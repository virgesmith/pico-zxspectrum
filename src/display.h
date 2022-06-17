#pragma once

#include "tft_t_dma.h"

#include <cstdint>
typedef uint8_t byte;

namespace display
{

const uint16_t WIDTH = 320;
const uint16_t HEIGHT = 240;

struct RGB { byte R,G,B; };

extern byte bordercolor;

void init(byte* VRAM); // ==Z80RAM
void toggle_vbl();
void render();
const uint16_t* line(uint16_t y);

}