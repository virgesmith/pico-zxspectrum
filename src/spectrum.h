#pragma once

#include "Z80.h"
#include "types.h"
#include "pico/time.h"


namespace spectrum {

extern Z80 z80;
extern byte ram[0xC000];

void init();
void step();
void start();
void input();

}