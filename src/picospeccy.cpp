#include "pico.h"
#include "pico/stdlib.h"

extern "C" {
#include "iopins.h"
#include "emuapi.h"
#include "spec.h"
}

#include "tft_t_dma.h"

#include <cstdio>

volatile bool vbl=true;

bool repeating_timer_callback(struct repeating_timer *t) {
    uint16_t bClick = emu_DebounceLocalKeys();
    emu_Input(bClick);
    if (vbl) {
        vbl = false;
    } else {
        vbl = true;
    }
    return true;
}
TFT_T_DMA tft;

static int skip=0;

#include "hardware/clocks.h"
#include "hardware/vreg.h"

int main(void) {
//    vreg_set_voltage(VREG_VOLTAGE_1_05);
//    set_sys_clock_khz(125000, true);
//    set_sys_clock_khz(150000, true);
//    set_sys_clock_khz(133000, true);
//    set_sys_clock_khz(200000, true);
//    set_sys_clock_khz(210000, true);
    set_sys_clock_khz(230000, true);
//    set_sys_clock_khz(225000, true);
//    set_sys_clock_khz(250000, true);
    stdio_init_all();

    tft.begin();
    emu_init();
    emu_start();

    emu_Init(nullptr);
    tft.fillScreenNoDma( RGBVAL16(0x00,0x00,0x00) );
    tft.startDMA();
    struct repeating_timer timer;
    add_repeating_timer_ms(25, repeating_timer_callback, NULL, &timer);

    while (true) {
        emu_Step();
    }
}

static unsigned char  palette8[PALETTE_SIZE];
static unsigned short palette16[PALETTE_SIZE];

void emu_SetPaletteEntry(unsigned char r, unsigned char g, unsigned char b, int index)
{
    if (index<PALETTE_SIZE) {
        palette8[index]  = RGBVAL8(r,g,b);
        palette16[index]  = RGBVAL16(r,g,b);
    }
}

void emu_DrawVsync(void)
{
    skip += 1;
    skip &= VID_FRAME_SKIP;
    volatile bool vb=vbl;
    while (vbl==vb) {};
}


void emu_DrawLine(unsigned char * VBuf, int width, int height, int line)
{
    if (skip == 0) {
         tft.writeLine(width,height,line, VBuf, palette16);
    }
}

void emu_DrawLine8(unsigned char * VBuf, int width, int height, int line)
{
    if (skip == 0) {
    }
}

void emu_DrawLine16(unsigned short * VBuf, int width, int height, int line)
{
    if (skip == 0) {
    }
}

void emu_DrawScreen(unsigned char * VBuf, int width, int height, int stride)
{
    if (skip == 0) {
    }
}

int emu_FrameSkip(void)
{
    return skip;
}

void * emu_LineBuffer(int line)
{
    return (void*)tft.getLineBuffer(line);
}


#include "AudioPlaySystem.h"
AudioPlaySystem mymixer;
#include "hardware/pwm.h"
void emu_sndInit() {
  tft.begin_audio(256, mymixer.snd_Mixer);
  mymixer.start();
  //gpio_init(AUDIO_PIN);
  //gpio_set_dir(AUDIO_PIN, GPIO_OUT);
}

void emu_sndPlaySound(int chan, int volume, int freq)
{
  if (chan < 6) {
    mymixer.sound(chan, freq, volume);
  }
}

void emu_sndPlayBuzz(int size, int val) {
#ifndef CUSTOM_SND
  //gpio_put(AUDIO_PIN, (val?1:0));
  pwm_set_gpio_level(AUDIO_PIN, (val?255:128));
#endif
}



