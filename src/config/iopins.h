#ifndef IOPINS_H
#define IOPINS_H


// Speaker
#define AUDIO_PIN       0
// VGA
/* RRRGGGBB
   VSYNC and HSYNC */
#define VGA_COLORBASE   2
#define VGA_SYNCBASE    14
#define VGA_VSYNC       15

// TFT
#define TFT_SPIREG      spi0
#define TFT_SPIDREQ     DREQ_SPI0_TX
#define TFT_SCLK        18
#define TFT_MOSI        19
#define TFT_MISO        255 // Not required, used for DC...
#define TFT_DC          16
#define TFT_RST         21
#define TFT_CS          255
#define TFT_BACKLIGHT   20


// SD (see SPI0 in code!!!)
#define SD_SPIREG       spi1
#define SD_SCLK         10 //14
#define SD_MOSI         11 //15
#define SD_MISO         12
#define SD_CS           13
#define SD_DETECT       255 // 22

// PSRAM (exclusive with TFT)
#define PSRAM_SPIREG    spi0
#define PSRAM_SCLK      18
#define PSRAM_MOSI      19
#define PSRAM_MISO      16 // DC
#define PSRAM_CS        17


// Keyboard matrix
//Cols (out)
#define KCOLOUT1        1
#define KCOLOUT2        2
#define KCOLOUT3        3
#define KCOLOUT4        4
#define KCOLOUT5        5
#define KCOLOUT6        14
//Rows (in)
#define KROWIN1         6
#define KROWIN2         9
#define KROWIN3         15
#define KROWIN4         8
#define KROWIN5         7
#define KROWIN6         22

#define KLED            25

#endif

