#pragma once

#include "Z80.h"
#include "pico/time.h"
#include "types.h"

namespace spectrum {

extern Z80 z80;
extern byte ram[0xC000];

void init();
void step();
void input();

} // namespace spectrum