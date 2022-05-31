#pragma once

#include "spec.h"

const uint16_t SNA_LEN = 49179;

void load_image_sna(Z80 *R, const uint8_t* data, uint16_t length);

byte* save_image_sna();

void load_image_z80(Z80 *R, const uint8_t *data, uint16_t length);

