#include "serial.h"
#include "loader.h"

#include "pico/stdlib.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>


Command serial::pop_command()
{
  return (Command)getchar_timeout_us(0);
}

Command serial::wait_command()
{
  return (Command)getchar();
}

void serial::read_keyboard(byte (&kbd_ram)[8])
{
  for (int i = 0; i < 8; ++i)
    // TODO why does getchar() not work reliably?
    kbd_ram[i] = getchar_timeout_us(0); // conveniently returns 255 if no keystroke
}

void serial::read_img(Command mode)
{
  uint16_t len = (uint16_t)getchar() + (((uint16_t)getchar()) << 8);

  // check image isnt too big
  if (len > loader::Z80_LEN)
  {
    return;
  }

  loader::image_size = len;

  for (uint16_t i = 0; i < loader::image_size; ++i)
  {
    loader::snapshot_buffer[i] = getchar();
  }

  switch(mode)
  {
    case Command::LOAD_Z80:
      loader::load_image_z80(spec::z80);
      break;
    case Command::LOAD_SNA:
      loader::load_image_sna(spec::z80);
    default:
      break;
  }
}


void serial::write_z80()
{
  for (uint16_t i = 0; i < loader::Z80_LEN; ++i)
  {
    printf("%02x", loader::snapshot_buffer[i]);
  }
  printf("\n");
}
