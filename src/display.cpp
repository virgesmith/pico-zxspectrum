#include "display.h"

#include <cstring>


namespace {

// TFT is 320x240
const int SCREEN_WIDTH  = 320;
const int SCREEN_HEIGHT = 240;

// ZXSpectrum is 256x192
const int DISP_WIDTH = 256; // pass in to ctor...
const int DISP_HEIGHT = 192; // pass in to ctor...
const int PALETTE_SIZE = 16;

const int VID_FRAME_SKIP = 0x0;

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
  int col = 0, pixel, ink, paper, attr;

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

  for (int x = 0; x < 32; ++x)
  {
    pixel = VRAM[dir_p++];
    attr = VRAM[dir_a++];

    if (((attr & 0x80) == 0) || (f_flash == 0))
    {
      ink = (attr & 0x07) + ((attr & 0x40) >> 3);
      paper = (attr & 0x78) >> 3;
    }
    else
    {
      paper = (attr & 0x07) + ((attr & 0x40) >> 3);
      ink = (attr & 0x78) >> 3;
    }
    scanline_buffer[col++] = ((pixel & 0x80) ? ink : paper);
    scanline_buffer[col++] = ((pixel & 0x40) ? ink : paper);
    scanline_buffer[col++] = ((pixel & 0x20) ? ink : paper);
    scanline_buffer[col++] = ((pixel & 0x10) ? ink : paper);
    scanline_buffer[col++] = ((pixel & 0x08) ? ink : paper);
    scanline_buffer[col++] = ((pixel & 0x04) ? ink : paper);
    scanline_buffer[col++] = ((pixel & 0x02) ? ink : paper);
    scanline_buffer[col++] = ((pixel & 0x01) ? ink : paper);
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
