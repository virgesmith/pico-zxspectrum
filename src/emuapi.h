#pragma once

#define EXTRA_HEAP  0x10

// Title:     <                        >
#define TITLE "    SPECTRUM Emulator"
#define ROMSDIR "spec"

#define PALETTE_SIZE         16
#define VID_FRAME_SKIP       0x0
#define TFT_VBUFFER_YCROP    0
#define SINGLELINE_RENDERING 1

#define ACTION_NONE          0
#define ACTION_MAXKBDVAL     225
#define ACTION_EXITKBD       128
#define ACTION_RUNTFT        129
#define ACTION_RUNVGA        130


#define MASK_JOY2_RIGHT 0x0001
#define MASK_JOY2_LEFT  0x0002
#define MASK_JOY2_UP    0x0004
#define MASK_JOY2_DOWN  0x0008
#define MASK_JOY2_BTN   0x0010
#define MASK_KEY_USER1  0x0020
#define MASK_KEY_USER2  0x0040
#define MASK_KEY_USER3  0x0080
#define MASK_JOY1_RIGHT 0x0100
#define MASK_JOY1_LEFT  0x0200
#define MASK_JOY1_UP    0x0400
#define MASK_JOY1_DOWN  0x0800
#define MASK_JOY1_BTN   0x1000
#define MASK_KEY_USER4  0x2000
#define MASK_OSKB       0x8000

#define RGBVAL32(r,g,b)  ( (r<<16) | (g<<8) | b )
#define RGBVAL16(r,g,b)  ( (((r>>3)&0x1f)<<11) | (((g>>2)&0x3f)<<5) | (((b>>3)&0x1f)<<0) )
#define RGBVAL8(r,g,b)   ( (((r>>5)&0x07)<<5) | (((g>>5)&0x07)<<2) | (((b>>6)&0x3)<<0) )
#define R16(rgb) ((rgb>>8)&0xf8)
#define G16(rgb) ((rgb>>3)&0xfc)
#define B16(rgb) ((rgb<<3)&0xf8)



extern volatile bool vbl;

namespace emu {

int fileOpen(const char * filepath, const char * mode);
int fileRead(void * buf, int size, int handler);
int fileGetc(int handler);
int fileSeek(int handler, int seek, int origin);
int fileTell(int handler);
void fileClose(int handler);

unsigned int fileSize(const char * filepath);
unsigned int loadFile(const char * filepath, void * buf, int size);

void setPaletteEntry(unsigned char r, unsigned char g, unsigned char b, int index);
void drawLine(unsigned char * VBuf, int width, int height, int line);
void drawVsync();
int frameSkip();
void* lineBuffer(int line);

void drawText(unsigned short x, unsigned short y, const char * text, unsigned short fgcolor, unsigned short bgcolor, int doublesize);

void initJoysticks();
int swapJoysticks(int statusOnly);
unsigned short debounceLocalKeys();
int readKeys();
int getPad();
int readAnalogJoyX(int min, int max);
int readAnalogJoyY(int min, int max);
int readI2CKeyboard();

unsigned char readUsbSerial();

void sndPlaySound(int chan, int volume, int freq);
void sndPlayBuzz(int size, int val);
void sndInit();
void resetus();
int us();

//extern int emu_setKeymap(int index);

void fileTempInit();
void fileTempRead(int addr, unsigned char * val, int n);
void fileTempWrite(int addr, unsigned char val);

}