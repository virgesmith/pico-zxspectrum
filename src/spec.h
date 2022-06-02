#pragma once

extern "C" {
#include "Z80.h"
}
#include "pico/time.h"

#include <cstdint>
typedef uint8_t byte;


namespace spec {

extern Z80 myCPU;
extern bool dump_sna;

byte* init();
void step();
void start();
void input(int bClick);

}