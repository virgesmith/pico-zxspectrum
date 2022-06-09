#include "keyboard.h"
#include "loader.h"

extern "C"
{
#include "iopins.h"
}

#include "pico/stdlib.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>

void emu::keyboard::readUsbSerial(byte* kbd_ram)
{
  byte c = getchar_timeout_us(0);
  if (c == 0)
  {
    for (int i = 0; i < 8; ++i)
      kbd_ram[i] = getchar_timeout_us(0);
  }
  if (c == 1)
  {
    loader::save_image_z80(&spec::myCPU);
    spec::dump_sna = true;
  }
  if (c == 2)
  {
    spec::init();
  }
}
