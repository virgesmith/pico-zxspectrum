#pragma once

extern "C" {
#include "Z80.h"
}
#include "pico/time.h"

#include <cstdint>
typedef uint8_t byte;


namespace spectrum {

extern Z80 z80;
extern byte ram[0xC000];

void init();
void step();
void start();
void input();

}