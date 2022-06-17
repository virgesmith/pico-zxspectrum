#include "keyboard.h"
#include "loader.h"
#include "serial.h"


void keyboard::readUsbSerial(byte (&kbd_ram)[8])
{
  Command c = serial::pop_command();
  switch (c)
  {
    case Command::KEYSTROKE:
      {
        serial::read_keyboard(kbd_ram);
      }
      break;
    case Command::SAVE:
      {
        loader::save_image_z80(spectrum::z80);
        loader::snapshot_pending = true;
      }
      break;
    case Command::SCREENSHOT:
      {
        loader::screenshot_pending = true;
      }
      break;
    case Command::RESET:
      {
        loader::reset_pending = true;
      }
      break;
    case Command::LOAD_SNA:
    case Command::LOAD_Z80:
      {
        serial::read_img(c);
        //loader::reset_pending = true;
      }
    default:
      break;
  }
}
