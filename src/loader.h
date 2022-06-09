#pragma once

#include "spec.h"

namespace loader
{

const uint16_t SNA_LEN = 49179;
const uint16_t Z80_LEN = 49182; // v1 48k uncompressed

extern byte snapshot_buffer[Z80_LEN];

void load_image_sna(Z80 *R);

//byte* save_image_sna(const Z80* R);

void load_image_z80(Z80 *R);

byte* save_image_z80(const Z80* R);

}