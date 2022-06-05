#pragma once

extern "C" {
#include "Z80.h"
}
#include "pico/time.h"

#include <cstdint>
typedef uint8_t byte;


namespace spec {

extern Z80 z80;

byte* init();
void step();
void start();
void input();

}