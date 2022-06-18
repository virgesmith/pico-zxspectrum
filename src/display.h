#pragma once

#include "types.h"
#include "tft_t_dma.h"

namespace display
{

const uint16_t WIDTH = 320;
const uint16_t HEIGHT = 240;

struct RGB { byte R,G,B; };

extern byte bordercolor;

void init();
void toggle_vbl();
void render();
const uint16_t* line(uint16_t y);

void rgb_led(byte r, byte g, byte b);

}