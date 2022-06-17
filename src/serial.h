#pragma once

#include "keyboard.h"

namespace serial
{

// check if a command has been issued
Command pop_command();

// wait for a command to be issued
Command wait_command();

void read_keyboard(byte (&kbd_ram)[8]);

void read_img(Command c);

void write_z80();

void write_screen();

}