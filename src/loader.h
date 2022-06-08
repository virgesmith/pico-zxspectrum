#pragma once

#include "spec.h"

const uint16_t SNA_LEN = 49179;
const uint16_t Z80_LEN = 49182; // v1 48k uncompressed

void load_image_sna(Z80 *R, const uint8_t* data, uint16_t length);

byte* save_image_sna();

void load_image_z80(Z80 *R, const uint8_t *data, uint16_t length);

byte* save_image_z80(const Z80 *R, const uint8_t *ram);
