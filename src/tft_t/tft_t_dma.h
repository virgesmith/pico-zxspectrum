#pragma once

#include "tft_t_dma_config.h"

#include "pico.h"

#include <cstdio>
#include <cmath>

#define RGBVAL32(r,g,b)  ( (r<<16) | (g<<8) | b )
#define RGBVAL16(r,g,b)  ( (((r>>3)&0x1f)<<11) | (((g>>2)&0x3f)<<5) | (((b>>3)&0x1f)<<0) )
#define RGBVAL8(r,g,b)   ( (((r>>5)&0x07)<<5) | (((g>>5)&0x07)<<2) | (((b>>6)&0x3)<<0) )
#define R16(rgb) ((rgb>>8)&0xf8)
#define G16(rgb) ((rgb>>3)&0xfc)
#define B16(rgb) ((rgb<<3)&0xf8)


#ifdef LOHRES
#define TFT_WIDTH      240
#define TFT_REALWIDTH  240
#else
#ifdef OVERRULE_WIDTH
#define TFT_WIDTH      OVERRULE_WIDTH
#else
#define TFT_WIDTH      320
#endif
#define TFT_REALWIDTH  320
#endif
#ifdef OVERRULE_HEIGHT
#define TFT_HEIGHT     OVERRULE_HEIGHT
#else
#define TFT_HEIGHT     240
#endif
#define TFT_REALHEIGHT 240


#define LINES_PER_BLOCK         64
#define NR_OF_BLOCK             4


#define ST7735_NOP     0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID   0x04
#define ST7735_RDDST   0x09

#define ST7735_SLPIN   0x10
#define ST7735_SLPOUT  0x11
#define ST7735_PTLON   0x12
#define ST7735_NORON   0x13

#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_RAMRD   0x2E

#define ST7735_PTLAR   0x30
#define ST7735_COLMOD  0x3A
#define ST7735_MADCTL  0x36

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5

#define ST7735_RDID1   0xDA
#define ST7735_RDID2   0xDB
#define ST7735_RDID3   0xDC
#define ST7735_RDID4   0xDD

#define ST7735_PWCTR6  0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

#define ST77XX_MADCTL_MY  0x80
#define ST77XX_MADCTL_MX  0x40
#define ST77XX_MADCTL_MV  0x20
#define ST77XX_MADCTL_ML  0x10
#define ST77XX_MADCTL_RGB 0x00
#define ST77XX_MADCTL_BGR 0x08
#define ST77XX_MADCTL_MH  0x04

#define TFT_CASET ST7735_CASET
#define TFT_PASET ST7735_RASET
#define TFT_RAMWR ST7735_RAMWR
#define TFT_MADCTL ST7735_MADCTL

class TFT_T_DMA final
{
public:
  TFT_T_DMA();

  void setArea(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2);
  void begin();
  void flipscreen(bool flip);
  bool isflipped();
  void startDMA();
  void stopDMA();
  int get_frame_buffer_size(int *width, int *height);
  void waitSync();

  void begin_audio(int samplesize, void (*callback)(short * stream, int len));
  void end_audio();

  // NoDMA functions
  void fillScreenNoDma(uint16_t color);
  void writeScreenNoDma(const uint16_t *pcolors);

    // DMA functions
  uint16_t * getLineBuffer(int j);
  void writeScreen(int width, int height, int stride, uint8_t *buffer, uint16_t *palette16);
  void writeLine(int width, int height, int stride, uint8_t *buffer, uint16_t *palette16);
  void writeLine(int width, int height, int y, uint16_t *buf);
  void fillScreen(uint16_t color);

private:
  uint8_t _rst, _cs, _dc;
  uint8_t _miso, _mosi, _sclk, _bkl;
  bool flipped=false;

  void wait();
};


