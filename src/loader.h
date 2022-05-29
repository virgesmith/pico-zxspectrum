#pragma once

extern "C" {
#include "Z80.h"
}

void load_image_sna(Z80 *R, const uint8_t* data, uint16_t length);
void load_image_z80(Z80 *R, const uint8_t *data, uint16_t length);

