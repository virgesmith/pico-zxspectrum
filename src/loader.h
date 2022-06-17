#pragma once

#include "spec.h"

namespace loader
{

enum class Snapshot: byte { NONE, Z80, SNA };

const uint16_t SNA_LEN = 49179;
const uint16_t Z80_LEN = 49182; // v1 48k uncompressed

extern bool snapshot_pending;
extern bool screenshot_pending;
extern bool reset_pending;
extern Snapshot snapshot_type;
extern uint16_t image_size;
extern byte snapshot_buffer[Z80_LEN];

void load_image_sna(Z80& R);

//byte* save_image_sna(const Z80* R);

void load_image_z80(Z80& R);

void save_image_z80(const Z80& R);

}