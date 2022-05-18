#include "Z80.h"
#include "spectrum_rom.h"
#include "emuapi.h"
#include "zx_filetyp_z80.h"

#include <string.h>

// ZXSpectrum is 256x192
static const int DISP_WIDTH = 256;
static const int DISP_HEIGHT = 192;
// TFT is 320x240
static const int SCREEN_WIDTH  = 320;
static const int SCREEN_HEIGHT = 240;

static const int HBORDER = (SCREEN_WIDTH - DISP_WIDTH) / 2; // ZXSpectrums is 256x192
static const int VBORDER = (SCREEN_HEIGHT - DISP_HEIGHT) / 2;


#define CYCLES_PER_FRAME 69888 //3500000/50

#define NBLINES (1) //(48+192+56+16) //(32+256+32)
#define CYCLES_PER_STEP (CYCLES_PER_FRAME/NBLINES)

typedef struct {
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
} HWOptions ;

static HWOptions hwopt = { 0xFF, 24, 128, 24, 48, 224, 16, 48, 192, 48, 8 };
//224, 64, 192, 56, 24, 128, 72};

struct { unsigned char R,G,B; } Palette[16] = {
  {   0,   0,   0},
  {   0,   0, 205},
  { 205,   0,   0},
  { 205,   0, 205},
  {   0, 205,   0},
  {   0, 205, 205},
  { 205, 205,   0},
  { 212, 212, 212},
  {   0,   0,   0},
  {   0,   0, 255},
  { 255,   0,   0},
  { 255,   0, 255},
  {   0, 255,   0},
  {   0, 255, 255},
  { 255, 255,   0},
  { 255, 255, 255}
};

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

static byte Z80_RAM[0xC000];                    // 48k RAM
static Z80 myCPU;
static byte * volatile VRAM=Z80_RAM;            // What will be displayed. Generally ZX VRAM, can be changed for alt screens.

//extern const byte rom_zx48_rom[];        // 16k ROM
static byte key_ram[8]={
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}; // Keyboard buffer
byte out_ram;                            // Output (fe port)
static byte kempston_ram;                       // Kempston-Joystick Buffer

static int bordercolor = 0;
static byte* scanline_buffer = NULL;

static int ik;
static int ihk;

void spec_Input(int bClick) {
  //ik  = emu_GetPad();
  //ihk = emu_ReadI2CKeyboard();
  ihk = emu_ReadUsbSerial();

}

void displayscanline(int y, int f_flash)
{
  int col = 0, pixeles, tinta, papel, atributos;

  if (y < VBORDER || y >= SCREEN_HEIGHT - VBORDER)
  {
    memset(scanline_buffer, bordercolor, SCREEN_WIDTH);
    emu_DrawLine(scanline_buffer, SCREEN_WIDTH, SCREEN_HEIGHT, y);
    return;
  }

  memset(scanline_buffer, bordercolor, HBORDER);
  col += HBORDER;

  int row = y - VBORDER;

  int dir_p = ((row & 0xC0) << 5) + ((row & 0x07) << 8) + ((row & 0x38) << 2);
  int dir_a = 0x1800 + (32 * (row >> 3));

  for (int x = 0; x < 32; x++)
  {
    pixeles=  VRAM[dir_p++];
    atributos=VRAM[dir_a++];

    if (((atributos & 0x80) == 0) || (f_flash == 0))
    {
      tinta = (atributos & 0x07) + ((atributos & 0x40) >> 3);
      papel = (atributos & 0x78) >> 3;
    }
    else
    {
      papel = (atributos & 0x07) + ((atributos & 0x40) >> 3);
      tinta = (atributos & 0x78) >> 3;
    }
    scanline_buffer[col++] = ((pixeles & 0x80) ? tinta : papel);
    scanline_buffer[col++] = ((pixeles & 0x40) ? tinta : papel);
    scanline_buffer[col++] = ((pixeles & 0x20) ? tinta : papel);
    scanline_buffer[col++] = ((pixeles & 0x10) ? tinta : papel);
    scanline_buffer[col++] = ((pixeles & 0x08) ? tinta : papel);
    scanline_buffer[col++] = ((pixeles & 0x04) ? tinta : papel);
    scanline_buffer[col++] = ((pixeles & 0x02) ? tinta : papel);
    scanline_buffer[col++] = ((pixeles & 0x01) ? tinta : papel);
  }

  memset(scanline_buffer + col, bordercolor, HBORDER);

  emu_DrawLine(scanline_buffer, SCREEN_WIDTH, SCREEN_HEIGHT, y);
}


static void displayScreen(void) {
  int y;
  static int f_flash = 1, f_flash2 = 0;
  f_flash2 = (f_flash2++) % 32;
  if (f_flash2 < 16)
    f_flash = 1;
  else
    f_flash = 0;

  for (y = 0; y < SCREEN_HEIGHT; y++)
    displayscanline (y, f_flash);

  emu_DrawVsync();
}


#ifdef CUSTOM_SND
#define SAMSIZE 32768
static unsigned char sam[SAMSIZE];
static int rdsam=0;
static int wrsam=SAMSIZE/2;
#endif

static int lastBuzzCycle=0;
//static byte lastBuzzVal;

#ifdef CUSTOM_SND
void  SND_Process( short * stream, int len )
{
    len = len >> 1;
    for (int i=0;i<len;i++)
    {
      short s = sam[rdsam]?32767:0;
      rdsam += 180;
      rdsam &= SAMSIZE-1;
      *stream++ = (short)(s);
      *stream++ = (short)(s);
    }
}
#endif


static void InitKeyboard(void){
  memset(key_ram, 0xff, sizeof(key_ram));
}

static void UpdateKeyboard (void)
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


#define MAX_Z80SIZE 49152


int endsWith(const char * s, const char * suffix)
{
  int retval = 0;
  int len = strlen(s);
  int slen = strlen(suffix);
  if (len > slen ) {
    if (!strcmp(&s[len-slen], suffix)) {
      retval = 1;
    }
  }
   return (retval);
}


void emu_KeyboardOnDown(int keymodifer, int key) {
}

void emu_KeyboardOnUp(int keymodifer, int key) {
}

void spec_Start(char * filename) {

  if (!filename) {
    ZX_ReadFromFlash_Z80(&myCPU, ZX48_ROM, 16384);
  }

  memset(Z80_RAM, 0, 0xC000);
  emu_sndInit();
}



//static AY8910 ay;

void spec_Init(void) {
  int J;
  /* Set up the palette */
  for(J=0;J<16;J++)
    emu_SetPaletteEntry(Palette[J].R,Palette[J].G,Palette[J].B, J);

  InitKeyboard();

  //Reset8910(&ay,3500000,0);


  if (scanline_buffer == NULL) scanline_buffer = (byte *)emu_Malloc(SCREEN_WIDTH);
  VRAM = Z80_RAM;
  memset(Z80_RAM, 0, sizeof(Z80_RAM));

  ResetZ80(&myCPU, CYCLES_PER_FRAME);
#if ALT_Z80CORE
  myCPU.RAM = Z80_RAM;
  Z80FlagTables();
#endif
}


void spec_Step(void) {
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
  displayScreen();

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


  UpdateKeyboard();

  //Loop8910(&ay,20);
}



#define BASERAM 0x4000


void WrZ80(register word Addr,register byte Value)
{
  if (Addr >= BASERAM)
    Z80_RAM[Addr-BASERAM]=Value;
}

byte RdZ80(register word addr)
{
  if (addr<BASERAM)
    return ZX48_ROM[addr];
  else
    return Z80_RAM[addr-BASERAM];
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
  emu_sndPlayBuzz(pulse_size,val);
#endif
}

void OutZ80(register word Port,register byte Value)
{
/*  if ((Port & 0xC002) == 0xC000) {
    WrCtrl8910(&ay,(Value &0x0F));
  }
  else if ((Port & 0xC002) == 0x8000) {
    WrData8910(&ay,Value);
  }
  else*/ if (!(Port & 0x01)) {
    bordercolor = (Value & 0x07);
    //byte mic = (Value & 0x08);
    byte ear = (Value & 0x10);
    buzz(((ear)?1:0), CYCLES_PER_STEP-myCPU.ICount);
    // TODO check if anything appears over serial....
    //printf("%c", Value);
  }
  else if((Port&0xFF)==0xFE) {
    out_ram=Value; // update it
    // TODO check if anything appears over serial....
    //printf("%c", Value);
  }
}

byte InZ80(register word port)
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


void PatchZ80(register Z80 *R)
{
  // nothing to do
}
