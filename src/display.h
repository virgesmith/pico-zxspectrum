#pragma once

#include "tft_t_dma.h"

#include <cstdint>
typedef uint8_t byte;

namespace emu::display
{

struct RGB { byte R,G,B; };

extern byte bordercolor;

void init(byte* VRAM, const RGB (&palette)[16]); // ==Z80RAM
void toggle_vbl();
void render();

}