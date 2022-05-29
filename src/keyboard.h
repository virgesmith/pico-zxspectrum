#pragma once

#include <cstdint>

const uint16_t MASK_JOY2_RIGHT = 0x0001;
const uint16_t MASK_JOY2_LEFT  = 0x0002;
const uint16_t MASK_JOY2_UP    = 0x0004;
const uint16_t MASK_JOY2_DOWN  = 0x0008;
const uint16_t MASK_JOY2_BTN   = 0x0010;

typedef uint8_t byte;

namespace emu::keyboard {

void readUsbSerial(byte* kbd_ram);

}


