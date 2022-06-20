#include "spectrum.h"

#include "spectrum_rom.h"
#include "sound.h"
#include "display.h"
#include "buttons.h"
#include "keyboard.h"
#include "loader.h"
#include "serial.h"

#include <pico/stdlib.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#define CYCLES_PER_FRAME 69888 //3500000/50

#define NBLINES (1) //(48+192+56+16) //(32+256+32)
#define CYCLES_PER_STEP (CYCLES_PER_FRAME/NBLINES)


Z80 spectrum::z80;
byte spectrum::ram[0xC000]; // 48k RAM

namespace {

const uint16_t BASERAM = 0x4000;

byte key_ram[8]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}; // Keyboard buffer
byte out_ram;                            // Output (fe port)
byte kempston_ram;                       // Kempston-Joystick Buffer

int lastBuzzCycle=0;

struct HWOptions
{
  int port_ff;      // 0xff = emulate the port,  0x00 alwais 0xFF
  // int ts_lebo;      // left border t states
  // int ts_grap;      // graphic zone t states
  // int ts_ribo;      // right border t states
  // int ts_hore;      // horizontal retrace t states
  // int ts_line;      // to speed the calc, the sum of 4 abobe
  // int line_poin;    // lines in retraze post interrup
  // int line_upbo;    // lines of upper border
  // int line_grap;    // lines of graphic zone = 192
  // int line_bobo;    // lines of bottom border
  // int line_retr;    // lines of the retrace
};

HWOptions hwopt = { 0xFF }; //, 24, 128, 24, 48, 224, 16, 48, 192, 48, 8 };

}


void spectrum::init()
{
  memset(spectrum::ram, 0, sizeof(spectrum::ram));
  ResetZ80(&spectrum::z80, CYCLES_PER_FRAME);

  Command mode = serial::wait_command();

  if ((mode == Command::LOAD_SNA) | (mode == Command::LOAD_Z80))
    serial::read_img(mode);

  sound::init();

  display::rgb_led(0, 64, 0);
}


void spectrum::step()
{
  for (int scanl = 0; scanl < NBLINES; scanl++)
  {
    lastBuzzCycle=0;
    ExecZ80(&z80,CYCLES_PER_STEP); // 3.5MHz ticks for 6 lines @ 30 kHz = 700 cycles
#ifdef CUSTOM_SND
    buzz(lastBuzzVal, CYCLES_PER_STEP);
#endif
    //busy_wait_us(1);
    //sleep_us(1);
  }

  if (loader::snapshot_pending)
  {
    display::rgb_led(32, 0, 32);
    serial::write_z80();
    loader::snapshot_pending = false;
    display::rgb_led(0, 64, 0);
  }

  if (loader::screenshot_pending)
  {
    display::rgb_led(0, 0, 64);
    serial::write_screen();
    loader::screenshot_pending = false;
    display::rgb_led(0, 64, 0);
  }

  if (loader::reset_pending)
  {
    display::rgb_led(64, 0, 0);
    loader::load_image_z80(spectrum::z80);
    loader::reset_pending = false;
    display::rgb_led(0, 64, 0);
  }

  IntZ80(&z80, INT_IRQ); // must be called every 20ms
  display::render();

  // int k = 0; // ik; //emu_GetPad();

  // kempston_ram = 0x00;
  // if (k & MASK_JOY2_BTN)
  //   kempston_ram |= 0x10; //Fire
  // if (k & MASK_JOY2_UP)
  //   kempston_ram |= 0x8; //Up
  // if (k & MASK_JOY2_DOWN)
  //   kempston_ram |= 0x4; //Down
  // if (k & MASK_JOY2_RIGHT)
  //   kempston_ram |= 0x2; //Right
  // if (k & MASK_JOY2_LEFT)
  //   kempston_ram |= 0x1; //Left
}

void spectrum::input()
{
  // client-side driver isn't expecting the data back...
  // if (button::pressed(button::Id::A))
  // {
  //   loader::snapshot_pending = true;
  // }

  // if (button::pressed(button::Id::B))
  // {
  //   loader::screenshot_pending = true;
  // }

  if (button::pressed(button::Id::X))
  {
    loader::reset_pending = true;
  }

  // if (button::pressed(button::Id::Y))
  // {
  // }

  keyboard::readUsbSerial(key_ram);
}

void WrZ80(word Addr, byte Value)
{
  if (Addr >= BASERAM)
    spectrum::ram[Addr-BASERAM]=Value;
}

byte RdZ80(word addr)
{
  if (addr<BASERAM)
    return ZX48_ROM[addr];
  else
    return spectrum::ram[addr-BASERAM];
}


void buzz(int val, int currentTstates)
{
  int pulse_size = currentTstates - lastBuzzCycle;
#ifdef CUSTOM_SND
  for (int i = 0; i<pulse_size; i++ )
  {
    sam[wrsam] = lastBuzzVal?0:1;
    wrsam += 1;
    wrsam &= SAMSIZE-1;
  }

  lastBuzzCycle = currentTstates;
  lastBuzzVal = val;
#else
  sound::playBuzz(pulse_size,val);
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


  if (!(port & 0x01))
  //if ((port & 0xFF) == 0xFE)
  {
    display::bordercolor = (value & 0x07);
    //byte mic = (value & 0x08) ? 1 : 0;
    byte ear = (value & 0x10) ? 1 : 0;
    // t = (CYCLES_PER_STEP-myCPU.ICount) - t;
    // printf("%c%c%c", (value & 0x1f), (byte)(t>>8), (byte)(t & 0xff));
    // printf("%c%c", (byte)(t >> 8), (byte)(t &0xff));
    buzz(ear, CYCLES_PER_STEP - spectrum::z80.ICount);
    // TODO check if anything appears over serial....
    // printf("%d %d %d\n", bordercolor, mic, ear);
  }
  else if((port & 0xFF) == 0xFE)
  {
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

  if ((port & 0xFF) == 0xFF)
  {
    if (hwopt.port_ff == 0xFF)
    {
      return 0xFF;
    }
    else
    {
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
