#include "spectrum.h"
#include "display.h"
#include "keyboard.h"
#include "buttons.h"

#include "pico.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "hardware/clocks.h"
#include "hardware/vreg.h"

#include <cstdio>

bool repeating_timer_callback(struct repeating_timer* t)
{
  spectrum::input();
  display::toggle_vbl();
  return true;
}

int main()
{
  //    vreg_set_voltage(VREG_VOLTAGE_1_05);
  //    set_sys_clock_khz(125000, true);
  //    set_sys_clock_khz(150000, true);
  // set_sys_clock_khz(133000, true);
  //    set_sys_clock_khz(200000, true);
  //    set_sys_clock_khz(210000, true);
  set_sys_clock_khz(230400, true);
  //    set_sys_clock_khz(225000, true);
  //    set_sys_clock_khz(250000, true);
  stdio_init_all();

  spectrum::init();
  display::init();

  repeating_timer timer;
  add_repeating_timer_ms(15, repeating_timer_callback, nullptr, &timer);

  for (;;)
  {
    spectrum::step();
  }
}
