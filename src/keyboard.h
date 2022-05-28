#pragma once

#include <cstdint>

const uint16_t MASK_JOY2_RIGHT = 0x0001;
const uint16_t MASK_JOY2_LEFT  = 0x0002;
const uint16_t MASK_JOY2_UP    = 0x0004;
const uint16_t MASK_JOY2_DOWN  = 0x0008;
const uint16_t MASK_JOY2_BTN   = 0x0010;
const uint16_t MASK_KEY_USER1  = 0x0020;
const uint16_t MASK_KEY_USER2  = 0x0040;
const uint16_t MASK_KEY_USER3  = 0x0080;
const uint16_t MASK_JOY1_RIGHT = 0x0100;
const uint16_t MASK_JOY1_LEFT  = 0x0200;
const uint16_t MASK_JOY1_UP    = 0x0400;
const uint16_t MASK_JOY1_DOWN  = 0x0800;
const uint16_t MASK_JOY1_BTN   = 0x1000;
const uint16_t MASK_KEY_USER4  = 0x2000;

typedef uint8_t byte;

namespace emu::keyboard {

uint16_t readKeys();

uint16_t debounceLocalKeys();

byte readUsbSerial();

}


