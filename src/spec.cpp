#include "spec.h"

#include "spectrum_rom.h"
#include "sound.h"
#include "display.h"
#include "keyboard.h"

extern "C" {
#include "Z80.h"
#include "zx_filetyp_z80.h"
}

#include <pico/stdlib.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#define CYCLES_PER_FRAME 69888 //3500000/50

#define NBLINES (1) //(48+192+56+16) //(32+256+32)
#define CYCLES_PER_STEP (CYCLES_PER_FRAME/NBLINES)

#define BASERAM 0x4000


const byte map_qw[8][5] = {
    {25, 6,27,29,224}, // vcxz<caps shift=Lshift>
    {10, 9, 7,22, 4}, // gfdsa
    {23,21, 8,26,20}, // trewq
    {34,33,32,31,30}, // 54321
    {35,36,37,38,39}, // 67890
    {28,24,12,18,19}, // yuiop
    {11,13,14,15,40}, // hjkl<enter>
    { 5,17,16,225,44}, // bnm <symbshift=RSHift> <space>
};

namespace {

byte ZX_RAM[0xC000];                    // 48k RAM
Z80 myCPU;
byte key_ram[8]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}; // Keyboard buffer
byte out_ram;                            // Output (fe port)
byte kempston_ram;                       // Kempston-Joystick Buffer

int ik;
int ihk;

int lastBuzzCycle=0;

struct HWOptions
{
  int port_ff;      // 0xff = emulate the port,  0x00 alwais 0xFF
  int ts_lebo;      // left border t states
  int ts_grap;      // graphic zone t states
  int ts_ribo;      // right border t states
  int ts_hore;      // horizontal retrace t states
  int ts_line;      // to speed the calc, the sum of 4 abobe
  int line_poin;    // lines in retraze post interrup
  int line_upbo;    // lines of upper border
  int line_grap;    // lines of graphic zone = 192
  int line_bobo;    // lines of bottom border
  int line_retr;    // lines of the retrace
  /*
  int TSTATES_PER_LINE;
  int TOP_BORDER_LINES;
  int SCANLINES;
  int BOTTOM_BORDER_LINES;
  int tstate_border_left;
  int tstate_graphic_zone;
  int tstate_border_right;
  int hw_model;
  int int_type;
  int videopage;
  int BANKM;
  int BANK678;
  */
};


HWOptions hwopt = { 0xFF, 24, 128, 24, 48, 224, 16, 48, 192, 48, 8 };


void initKeyboard()
{
  memset(key_ram, 0xff, sizeof(key_ram));
}

static void updateKeyboard()
{
  //int k = ik; //emu_GetPad();
  int hk = ihk; //emu_ReadI2CKeyboard();

  memset(key_ram, 0xff, sizeof(key_ram));
  int shift = hk;
  if (hk >=128) hk -= 128;
  else if (hk >=64) hk -= 64;
  // scan all possibilities
  for (int j=0;j<8;j++) {
    for(int i=0;i<5;i++){
      if ( /*(k == map_qw[j][i]) ||*/ (hk == map_qw[j][i]) ) {
          key_ram[j] &= ~ (1<<(4-i));
      }
    }
  }
  if (shift & (1<<7)) key_ram[0] &= ~(1<<0);  // caps shift
  if (shift & (1<<6)) key_ram[7] &= ~(1<<1);  // symbol shift
}

}


namespace spec {

void start()
{
  ZX_ReadFromFlash_Z80(&myCPU, ZX48_ROM, 16384);

  memset(ZX_RAM, 0, 0xC000);
  uint16_t len = (uint16_t)getchar() + (((uint16_t)getchar()) << 8);
  if (len)
  {
    byte* buffer = (byte*)malloc(len);
    if (buffer)
    {
      for (uint16_t i = 0; i < len; ++i)
      {
          buffer[i] = getchar();
      }
      ZX_ReadFromFlash_SNA(&myCPU, buffer, len);
      free(buffer);
    }
  }

  emu::sound::init();
}

byte* init()
{
  initKeyboard();

  memset(ZX_RAM, 0, sizeof(ZX_RAM));

  ResetZ80(&myCPU, CYCLES_PER_FRAME);
  return ZX_RAM;
}


void step()
{
  int scanl;
  for (scanl = 0; scanl < NBLINES; scanl++) {
    lastBuzzCycle=0;
    ExecZ80(&myCPU,CYCLES_PER_STEP); // 3.5MHz ticks for 6 lines @ 30 kHz = 700 cycles
#ifdef CUSTOM_SND
    buzz(lastBuzzVal, CYCLES_PER_STEP);
#endif
    //busy_wait_us(1);
    //sleep_us(1);
  }

#if ALT_Z80CORE
#else
  IntZ80(&myCPU,INT_IRQ); // must be called every 20ms
#endif
  emu::display::render();

  int k=ik; //emu_GetPad();

  kempston_ram = 0x00;
  if (k & MASK_JOY2_BTN)
          kempston_ram |= 0x10; //Fire
  if (k & MASK_JOY2_UP)
          kempston_ram |= 0x8; //Up
  if (k & MASK_JOY2_DOWN)
          kempston_ram |= 0x4; //Down
  if (k & MASK_JOY2_RIGHT)
          kempston_ram |= 0x2; //Right
  if (k & MASK_JOY2_LEFT)
          kempston_ram |= 0x1; //Left


  updateKeyboard();

  //Loop8910(&ay,20);
}

void input(int bClick)
{
  ihk = emu::keyboard::readUsbSerial();
}

}


void WrZ80(word Addr, byte Value)
{
  if (Addr >= BASERAM)
    ZX_RAM[Addr-BASERAM]=Value;
}

byte RdZ80(word addr)
{
  if (addr<BASERAM)
    return ZX48_ROM[addr];
  else
    return ZX_RAM[addr-BASERAM];
}



void buzz(int val, int currentTstates)
{
  int pulse_size = (currentTstates-lastBuzzCycle);
#ifdef CUSTOM_SND
  for (int i = 0; i<pulse_size; i++ ) {
    sam[wrsam] = lastBuzzVal?0:1;
    wrsam += 1;
    wrsam &= SAMSIZE-1;
  }

  lastBuzzCycle = currentTstates;
  lastBuzzVal = val;
#else
  emu::sound::playBuzz(pulse_size,val);
#endif
}

void OutZ80(word port,byte value)
{
  // if ((Port & 0xC002) == 0xC000) {
  //   WrCtrl8910(&ay,(value &0x0F));
  // }
  // else if ((Port & 0xC002) == 0x8000) {
  //   WrData8910(&ay,value);
  // }
  // else
  // static int t = 0;
  // t = myCPU.ICount - t;


  //if (!(port & 0x01)) {
  if ((port & 0xFF) == 0xFE)
  {
    emu::display::bordercolor = (value & 0x07);
    //byte mic = (value & 0x08) ? 1 : 0;
    byte ear = (value & 0x10) ? 1 : 0;
    // t = (CYCLES_PER_STEP-myCPU.ICount) - t;
    // printf("%c%c%c", (value & 0x1f), (byte)(t>>8), (byte)(t & 0xff));
    // printf("%c%c", (byte)(t >> 8), (byte)(t &0xff));
    buzz(ear, CYCLES_PER_STEP-myCPU.ICount);
    // TODO check if anything appears over serial....
    // printf("%d %d %d\n", bordercolor, mic, ear);
  }
  else if((port & 0xFF)==0xFE) {
    out_ram=value; // update it
    // TODO check if anything appears over serial....
    //printf("Port 0xFE %d\n", value);
  }
}

byte InZ80(word port)
{
    // if (port == 0xFFFD) {
    //   return (RdData8910(&ay));
    // }

    if((port&0xFF)==0x1F) {
        // kempston RAM
        return kempston_ram;
    }

    if ((port&0xFF)==0xFE) {
        switch(port>>8) {
            case 0xFE : return key_ram[0]; break;
            case 0xFD : return key_ram[1]; break;
            case 0xFB : return key_ram[2]; break;
            case 0xF7 : return key_ram[3]; break;
            case 0xEF : return key_ram[4]; break;
            case 0xDF : return key_ram[5]; break;
            case 0xBF : return key_ram[6]; break;
            case 0x7F : return key_ram[7]; break;
        }
    }

    if ((port & 0xFF) == 0xFF) {
      if (hwopt.port_ff == 0xFF) {
       return 0xFF;
      }
      else {
       //code = 1;
       //if (code == 0xFF) code = 0x00;
       return 1;
      }
    }
    return 0xFF;
}


void PatchZ80(Z80 *R)
{
  // nothing to do
}
