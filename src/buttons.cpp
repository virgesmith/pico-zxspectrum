#include "buttons.h"
#include "drivers/button/button.hpp"
#include "libraries/pico_display_2/pico_display_2.hpp"

namespace {

pimoroni::Button buttons[] = {
  pimoroni::Button(pimoroni::PicoDisplay2::A),
  pimoroni::Button(pimoroni::PicoDisplay2::B),
  pimoroni::Button(pimoroni::PicoDisplay2::X),
  pimoroni::Button(pimoroni::PicoDisplay2::Y),
};

}

bool button::pressed(button::Id id)
{
  return buttons[static_cast<int>(id)].raw();
}
