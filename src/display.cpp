#include "display.h"

#include "emuapi.h"

#include <cstring>


namespace {

// TFT is 320x240
const int SCREEN_WIDTH  = 320;
const int SCREEN_HEIGHT = 240;

// ZXSpectrum is 256x192
const int DISP_WIDTH = 256; // pass in to ctor...
const int DISP_HEIGHT = 192; // pass in to ctor...

const int HBORDER = (SCREEN_WIDTH - DISP_WIDTH) / 2;
const int VBORDER = (SCREEN_HEIGHT - DISP_HEIGHT) / 2;

int skip = 0;
volatile bool vbl = true;

TFT_T_DMA tft;

// map zx spectrum colour codes to 5+6+5 bit RGB
uint16_t palette16[PALETTE_SIZE];

byte* volatile VRAM; // pass in to ctor...
byte scanline_buffer[SCREEN_WIDTH]; // TODO use tft getlinebuffer? uint16_t* tft.getLineBuffer(line);

void drawLine(byte *VBuf, int width, int height, int line)
{
  if (skip == 0)
  {
    tft.writeLine(width, height, line, VBuf, palette16);
  }
}

void drawVsync()
{
  skip += 1;
  skip &= VID_FRAME_SKIP;
  volatile bool vb = vbl;
  while (vbl == vb) { };
}


void displayScanline(int y, int f_flash)
{
  int col = 0, pixeles, tinta, papel, atributos;

  if (y < VBORDER || y >= SCREEN_HEIGHT - VBORDER)
  {
    memset(scanline_buffer, emu::display::bordercolor, SCREEN_WIDTH);
    drawLine(scanline_buffer, SCREEN_WIDTH, SCREEN_HEIGHT, y);
    return;
  }

  memset(scanline_buffer, emu::display::bordercolor, HBORDER);
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

  memset(scanline_buffer + col, emu::display::bordercolor, HBORDER);

  drawLine(scanline_buffer, SCREEN_WIDTH, SCREEN_HEIGHT, y);
}

}

byte emu::display::bordercolor;


void emu::display::init(byte* const pram, const RGB (&palette)[16])
{
  VRAM = pram;

  for (size_t i = 0; i < 16; ++i)
    palette16[i] = RGBVAL16(palette[i].R, palette[i].G, palette[i].B);

  tft.begin();
  tft.fillScreenNoDma(RGBVAL16(0x00, 0x00, 0x00));
  tft.startDMA();
}

// void emu::display::setPaletteEntry(byte r, byte g, byte b, int index)
// {
//   if (index < PALETTE_SIZE)
//   {

//   }
// }

void emu::display::toggle_vbl()
{
  vbl = !vbl;
}


void emu::display::render() {
  int y;
  static int f_flash = 1, f_flash2 = 0;
  f_flash2 = (f_flash2 + 1) % 32;
  if (f_flash2 < 16)
    f_flash = 1;
  else
    f_flash = 0;

  for (y = 0; y < SCREEN_HEIGHT; y++)
    displayScanline(y, f_flash);

  drawVsync();
}
