#include "emuapi.h"
#include "spec.h"

extern "C"
{
#include "iopins.h"
}


#include "tft_t_dma.h"

#include "pico.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "hardware/clocks.h"
#include "hardware/vreg.h"


#include <cstdio>

TFT_T_DMA tft;


bool repeating_timer_callback(struct repeating_timer *t)
{
    uint16_t bClick = emu::debounceLocalKeys();
    spec::input(bClick);
    vbl = !vbl;
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

    tft.begin();

    spec::init();
    spec::start();
    tft.fillScreenNoDma(RGBVAL16(0x00, 0x00, 0x00));
    tft.startDMA();
    repeating_timer timer;
    add_repeating_timer_ms(25, repeating_timer_callback, NULL, &timer);

    for (;;)
    {
        spec::step();
    }
}

