#include "pico.h"
#include "pico/stdlib.h"

extern "C"
{
#include "iopins.h"
}

#include "emuapi.h"
#include "spec.h"

#include "tft_t_dma.h"

#include <cstdio>


bool repeating_timer_callback(struct repeating_timer *t)
{
    uint16_t bClick = emu_DebounceLocalKeys();
    spec::input(bClick);
    if (vbl)
    {
        vbl = false;
    }
    else
    {
        vbl = true;
    }
    return true;
}
TFT_T_DMA tft;


#include "hardware/clocks.h"
#include "hardware/vreg.h"

int main(void)
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

    tft.begin();
    emu_init();
    emu_start();

    spec::init();
    spec::start(nullptr);
    tft.fillScreenNoDma(RGBVAL16(0x00, 0x00, 0x00));
    tft.startDMA();
    repeating_timer timer;
    add_repeating_timer_ms(25, repeating_timer_callback, NULL, &timer);

    for (;;)
    {
        spec::step();
    }
}

