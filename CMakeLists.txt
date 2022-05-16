cmake_minimum_required(VERSION 3.12)

# Pull in PICO SDK (must be before project)
include(pico_sdk_import.cmake)

project(picozxspectrum C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the Pico SDK
pico_sdk_init()

add_subdirectory(src/FatFs_SPI build)

include_directories(src/config)
include_directories(src/tft_t)
include_directories(src/FatFs_SPI)


set(PICOSPECCY_SOURCES
		src/Z80.c
		src/spec.c
		src/zx_filetyp_z80.c
		src/spectrum_rom.c
		src/emuapi.cpp
		src/picospeccy.cpp
		src/AudioPlaySystem.cpp
	)

set(TFT_T_SOURCES
		src/tft_t/tft_t_dma.cpp
	)


add_executable(picozxspectrum
 	${PICOSPECCY_SOURCES}
	${TFT_T_SOURCES}
)

target_link_libraries(picozxspectrum pico_multicore
	    pico_stdlib
	    FatFs_SPI
#	    hardware_adc
	    hardware_pio
      hardware_pwm
)


#target_compile_options(picozxspectrum PUBLIC -O3)
target_compile_options(picozxspectrum PUBLIC -O3 -Wall -Wno-unused-function -Wno-unused-parameter)
IF (NOT DEFINED N_SD_CARDS)
    SET(N_SD_CARDS 1)
ENDIF()
target_compile_definitions(picozxspectrum PUBLIC N_SD_CARDS=${N_SD_CARDS})

pico_enable_stdio_uart(picozxspectrum 0)
pico_enable_stdio_usb(picozxspectrum 1)

pico_add_extra_outputs(picozxspectrum)
