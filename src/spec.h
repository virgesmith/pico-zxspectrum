#pragma once

#include <cstdint>
typedef uint8_t byte;


namespace spec {

byte* init();
void step();
void start();
void input(int bClick);

}