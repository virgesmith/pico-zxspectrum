cmake_minimum_required(VERSION 3.12)


set(PICO_BOARD pico)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

# set(PICO_PLATFORM rp2350-riscv)
# set(PICO_TOOLCHAIN_PATH /opt/riscv-gcc-14)

set(PICO_SDK_PATH ../../pico-sdk-1.5.1_tusb-0.14) # NB relative to build dir
# set(PICO_SDK_PATH ../../pico-sdk) # NB relative to build dir
set(PIMORONI_PATH ../pimoroni-pico) # NB relative to project root

# ensure we have the correct pico_sdk_import.cmake
execute_process (COMMAND
  bash -c "cp ${PICO_SDK_PATH}/external/pico_sdk_import.cmake .."
)

# Pull in PICO SDK (must be before project)
include(pico_sdk_import.cmake)

project(picozxspectrum C CXX ASM)

# Initialize the Pico SDK
pico_sdk_init()


add_subdirectory(${PIMORONI_PATH}/drivers/button pimoroni/drivers/button EXCLUDE_FROM_ALL)
add_subdirectory(${PIMORONI_PATH}/drivers/rgbled pimoroni/drivers/rgbled EXCLUDE_FROM_ALL)

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

include_directories(${PIMORONI_PATH})

target_link_libraries(picozxspectrum
	pico_multicore
	pico_stdlib
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
