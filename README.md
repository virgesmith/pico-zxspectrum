# pico-zxspectrum

It's been 40 years... I still remember the computery smell when I unboxed mine at Xmas (I *think* 1983)

So for old times' sake here's a ZX Spectrum emulator for the [Raspberry Pi pico](https://www.raspberrypi.com/documentation/microcontrollers/raspberry-pi-pico.html), using:

- screen: a [5cm 320x240 TFT display](https://shop.pimoroni.com/products/pico-display-pack-2-0?variant=39374122582099) as screen.
- keyboard: USB serial connection
- sound: not sure yet...
- cassette recorder: life's too short...

## Status

- [X] display: fully operational
- [X] keyboard: serial works using a basic python client
- [ ] sound
- [ ] real-time(?) tape loading
- [X] image loading over USB serial: Z80 and SNA formats supported
- [X] image saving over USB serial: Z80 format
- [X] save screenshot over USB serial: png format

![boot](./doc/boot.jpg)

![basic](./doc/basic.png) ![jetpac](./doc/jetpac.png)
![chegg](./doc/chegg.png) ![manic](./doc/manic.png)


## Dependencies

Requires [pico-sdk](https://github.com/raspberrypi/pico-sdk) (and its dependencies e.g. tinyUSB...), plus the [pimoroni-pico](https://github.com/pimoroni/pimoroni-pico) libraries. In the repo root:

Ensure tinyUSB sources are present in the `lib` subdir of the SDK (extract or symlink to a release here, or use git submodules), e.g.

```sh
cd pico-sdk-1.5.1/lib
rmdir tinyusb # will be empty initially
ln -s ../../tinyusb-0.14.0 tinyusb
```

Using tinyusb 0.15 or above results in a linker error that I don't know how to fix:

```txt
[100%] Linking CXX executable picozxspectrum.elf
/usr/lib/gcc/arm-none-eabi/10.3.1/../../../arm-none-eabi/bin/ld: picozxspectrum.elf section `.bss' will not fit in region `RAM'
/usr/lib/gcc/arm-none-eabi/10.3.1/../../../arm-none-eabi/bin/ld: region RAM overflowed
/usr/lib/gcc/arm-none-eabi/10.3.1/../../../arm-none-eabi/bin/ld: section .stack1_dummy VMA [0000000020040000,00000000200407ff] overlaps section .bss VMA [0000000020001828,000000002004005f]
/usr/lib/gcc/arm-none-eabi/10.3.1/../../../arm-none-eabi/bin/ld: region `RAM' overflowed by 96 bytes
collect2: error: ld returned 1 exit status
make[2]: *** [CMakeFiles/picozxspectrum.dir/build.make:1447: picozxspectrum.elf] Error 1
make[1]: *** [CMakeFiles/Makefile2:1541: CMakeFiles/picozxspectrum.dir/all] Error 2
make: *** [Makefile:91: all] Error 2
```

## Configuration

Symlink Pico SDK and Pimoroni Pico:

```sh
ln -s ../pico-sdk-1.5.1 pico-sdk # adjust version and location as necessary
ln -s pico-sdk/external/pico_sdk_import.cmake
ln -s ../pimoroni-pico # adjust to wherever you cloned it to
```

## Build

```sh
mkdir -p build && cd build
cmake -DPICO_SDK=../pico_sdk ..
make -j
```

## Install

Still in the build directory:

```sh
cp picozxspectrum.uf2 /media/$USER/RPI-RP2/
```

You should see at this point a blank screen with the LED bright white.

## Use

Interaction requires a (rudimentary) keyboard driver written in python and depends on pyserial and pynput. It can also load an image (Z80 or SNA) on startup:

```sh
python zx.py [image]
```

Keys map closely to the ZX Spectrum keyboard (below), with `LShift`as caps shift, `RCtrl` is symbol shift. `LShift+RCtrl` enters extended mode. Various unmapped keys also work, such as `Backspace` and the arrow keys which emulate caps-shifted 0,5,6,7,8 respectively.

~~`PrtScr`~~`Home` triggers a screenshot save (png format)

`Ins` triggers an image save (registers and RAM in Z80 format)

`Del` or the screen's `X` button resets the device to the last image loaded/saved, or ZX basic.

`Esc` followed by `Ctrl+C` exits the keyboard listener.

![spectrum-48-keyboard](./doc/spectrum-48-keyboard.png)

## Acknowledgements

Much of the C and C++ source code originates from: https://github.com/Jean-MarcHarvengt/MCUME. Copyright notices, where present, have been preserved.

These were also very useful:
http://www.breakintoprogram.co.uk/computers/zx-spectrum.
https://github.com/fruit-bat/pico-zxspectrum


Keyboard layout from https://dotneteer.github.io/spectnetide/getting-started/use-keyboard-tool
