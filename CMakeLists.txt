cmake_minimum_required(VERSION 3.12)

# Pull in PICO SDK (must be before project)
include(pico_sdk_import.cmake)

project(picozxspectrum C CXX ASM)

set(PICO_BOARD pico)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the Pico SDK
pico_sdk_init()

add_subdirectory(pimoroni-pico/drivers/button EXCLUDE_FROM_ALL)
add_subdirectory(pimoroni-pico/drivers/rgbled EXCLUDE_FROM_ALL)

include_directories(src/config)
include_directories(src/z80)
include_directories(src/tft_t)

set(PICOZXSPECTRUM_SRCS
	src/spectrum.cpp
	src/spectrum_rom.cpp
	src/sound.cpp
	src/display.cpp
	src/keyboard.cpp
	src/buttons.cpp
	src/loader.cpp
	src/serial.cpp
	src/main.cpp
	src/AudioPlaySystem.cpp
)

set(Z80_SRCS
	src/z80/Z80.c
)

set(TFT_T_SOURCES
		src/tft_t/tft_t_dma.cpp
	)

add_executable(picozxspectrum
 	${PICOZXSPECTRUM_SRCS}
	${Z80_SRCS}
	${TFT_T_SOURCES}
)

include_directories(${CMAKE_SOURCE_DIR}/pimoroni-pico)

target_link_libraries(picozxspectrum
	pico_multicore
	pico_stdlib
#	hardware_adc
  hardware_dma
	hardware_spi
	hardware_pio
	hardware_pwm
	rgbled
	button
)

target_compile_options(picozxspectrum PUBLIC -O3 -Wall -Wno-unused-function -Wno-unused-parameter)

pico_enable_stdio_uart(picozxspectrum 0)
pico_enable_stdio_usb(picozxspectrum 1)

pico_add_extra_outputs(picozxspectrum)
