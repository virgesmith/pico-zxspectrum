#include "spec.h"
#include "display.h"
#include "keyboard.h"

#include "pico.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "hardware/clocks.h"
#include "hardware/vreg.h"

#include <cstdio>


emu::display::RGB PALETTE[16] = {
  {   0,   0,   0},
  {   0,   0, 205},
  { 205,   0,   0},
  { 205,   0, 205},
  {   0, 205,   0},
  {   0, 205, 205},
  { 205, 205,   0},
  { 212, 212, 212},
  {   0,   0,   0},
  {   0,   0, 255},
  { 255,   0,   0},
  { 255,   0, 255},
  {   0, 255,   0},
  {   0, 255, 255},
  { 255, 255,   0},
  { 255, 255, 255}
};


bool repeating_timer_callback(struct repeating_timer *t)
{
  uint16_t bClick = emu::keyboard::debounceLocalKeys();
  spec::input(bClick);
  emu::display::toggle_vbl();
  return true;
}


int main()
{
  //    vreg_set_voltage(VREG_VOLTAGE_1_05);
  //    set_sys_clock_khz(125000, true);
  //    set_sys_clock_khz(150000, true);
  //    set_sys_clock_khz(133000, true);
  //    set_sys_clock_khz(200000, true);
  //    set_sys_clock_khz(210000, true);
  set_sys_clock_khz(230000, true);
  //    set_sys_clock_khz(225000, true);
  //    set_sys_clock_khz(250000, true);
  stdio_init_all();

  byte* ram_start = spec::init();
  spec::start();
  emu::display::init(ram_start, PALETTE);

  repeating_timer timer;
  add_repeating_timer_ms(25, repeating_timer_callback, NULL, &timer);

  for (;;)
  {
    spec::step();
  }
}
