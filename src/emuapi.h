#pragma once

typedef unsigned char byte;

// #define EXTRA_HEAP  0x10

// // Title:     <                        >
// #define TITLE "    SPECTRUM Emulator"
// #define ROMSDIR "spec"

// #define VID_FRAME_SKIP       0x0
// #define TFT_VBUFFER_YCROP    0
// #define SINGLELINE_RENDERING 1

// #define ACTION_NONE          0
// #define ACTION_MAXKBDVAL     225
// #define ACTION_EXITKBD       128
// #define ACTION_RUNTFT        129
// #define ACTION_RUNVGA        130



namespace emu {

unsigned int fileSize(const char * filepath);
unsigned int loadFile(const char * filepath, void * buf, int size);


void initJoysticks();
int swapJoysticks(int statusOnly);
//unsigned short debounceLocalKeys();
//int readKeys();
int getPad();
int readAnalogJoyX(int min, int max);
int readAnalogJoyY(int min, int max);
int readI2CKeyboard();

//byte readUsbSerial();

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