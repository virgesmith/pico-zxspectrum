#include "keyboard.h"

extern "C"
{
#include "iopins.h"
}

#include "pico/stdlib.h"

#include <cstdint>
#include <cstdio>

void emu::keyboard::readUsbSerial(byte* kbd_ram)
{
  byte c = getchar_timeout_us(0);
  if (c == 0)
  {
    for (int i = 0; i < 8; ++i)
      kbd_ram[i] = getchar_timeout_us(0);
  }
}
