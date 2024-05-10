from __future__ import annotations
from typing import Optional
from pynput.keyboard import Key, KeyCode, Listener  # type: ignore
from serial import Serial  # type: ignore
from pathlib import Path
from datetime import datetime
import typer
import numpy as np
from PIL import Image  # type: ignore

DEVICE = "/dev/ttyACM0"
BAUD_RATE = 115200

keymap = {
    KeyCode(char="1"): ((3, 0),),
    KeyCode(char="2"): ((3, 1),),
    KeyCode(char="3"): ((3, 2),),
    KeyCode(char="4"): ((3, 3),),
    KeyCode(char="5"): ((3, 4),),
    KeyCode(char="6"): ((4, 4),),
    KeyCode(char="7"): ((4, 3),),
    KeyCode(char="8"): ((4, 2),),
    KeyCode(char="9"): ((4, 1),),
    KeyCode(char="0"): ((4, 0),),
    KeyCode(char="!"): ((3, 0),),
    KeyCode(char='"'): ((3, 1),),
    KeyCode(char="£"): ((3, 2),),
    KeyCode(char="$"): ((3, 3),),
    KeyCode(char="%"): ((3, 4),),
    KeyCode(char="^"): ((4, 4),),
    KeyCode(char="&"): ((4, 3),),
    KeyCode(char="*"): ((4, 2),),
    KeyCode(char="("): ((4, 1),),
    KeyCode(char=")"): ((4, 0),),
    KeyCode(char="q"): ((2, 0),),
    KeyCode(char="w"): ((2, 1),),
    KeyCode(char="e"): ((2, 2),),
    KeyCode(char="r"): ((2, 3),),
    KeyCode(char="t"): ((2, 4),),
    KeyCode(char="y"): ((5, 4),),
    KeyCode(char="u"): ((5, 3),),
    KeyCode(char="i"): ((5, 2),),
    KeyCode(char="o"): ((5, 1),),
    KeyCode(char="p"): ((5, 0),),
    KeyCode(char="Q"): ((2, 0),),
    KeyCode(char="W"): ((2, 1),),
    KeyCode(char="E"): ((2, 2),),
    KeyCode(char="R"): ((2, 3),),
    KeyCode(char="T"): ((2, 4),),
    KeyCode(char="Y"): ((5, 4),),
    KeyCode(char="U"): ((5, 3),),
    KeyCode(char="I"): ((5, 2),),
    KeyCode(char="O"): ((5, 1),),
    KeyCode(char="P"): ((5, 0),),
    KeyCode(char="a"): ((1, 0),),
    KeyCode(char="s"): ((1, 1),),
    KeyCode(char="d"): ((1, 2),),
    KeyCode(char="f"): ((1, 3),),
    KeyCode(char="g"): ((1, 4),),
    KeyCode(char="h"): ((6, 4),),
    KeyCode(char="j"): ((6, 3),),
    KeyCode(char="k"): ((6, 2),),
    KeyCode(char="l"): ((6, 1),),
    Key.enter: ((6, 0),),
    KeyCode(char="A"): ((1, 0),),
    KeyCode(char="S"): ((1, 1),),
    KeyCode(char="D"): ((1, 2),),
    KeyCode(char="F"): ((1, 3),),
    KeyCode(char="G"): ((1, 4),),
    KeyCode(char="H"): ((6, 4),),
    KeyCode(char="J"): ((6, 3),),
    KeyCode(char="K"): ((6, 2),),
    KeyCode(char="L"): ((6, 1),),
    Key.shift: ((0, 0),),  # Key.shift_l: (0, 0), Key.shift_r: (0, 0),
    KeyCode(char="z"): ((0, 1),),
    KeyCode(char="x"): ((0, 2),),
    KeyCode(char="c"): ((0, 3),),
    KeyCode(char="v"): ((0, 4),),
    KeyCode(char="b"): ((7, 4),),
    KeyCode(char="n"): ((7, 3),),
    KeyCode(char="m"): ((7, 2),),
    Key.ctrl_r: ((7, 1),),
    Key.space: ((7, 0),),
    KeyCode(char="Z"): ((0, 1),),
    KeyCode(char="X"): ((0, 2),),
    KeyCode(char="C"): ((0, 3),),
    KeyCode(char="V"): ((0, 4),),
    KeyCode(char="B"): ((7, 4),),
    KeyCode(char="N"): ((7, 3),),
    KeyCode(char="M"): ((7, 2),),
    # combo shortcuts
    Key.left: ((0, 0), (3, 4)),
    Key.down: ((0, 0), (4, 4)),
    Key.up: ((0, 0), (4, 3)),
    Key.right: ((0, 0), (4, 2)),
    Key.alt_gr: ((0, 0), (4, 1)),
    Key.backspace: ((0, 0), (4, 0)),
    KeyCode(char=","): ((7, 1), (7, 3)),
    KeyCode(char="."): ((7, 1), (7, 2)),
    KeyCode(char="/"): ((7, 1), (0, 4)),
    KeyCode(char=";"): ((7, 1), (0, 4)),
    KeyCode(char="-"): ((7, 1), (6, 3)),
    KeyCode(char="="): ((7, 1), (6, 1)),
    KeyCode(char="+"): ((7, 1), (6, 2)),
    KeyCode(char=";"): ((7, 1), (5, 1)),
    KeyCode(char=":"): ((7, 1), (0, 1)),
}

# enum class Command: byte { KEYSTROKE, SAVE, RESET, LOAD_SNA, LOAD_Z80, NONE=255 };


def get_mode(filename: str) -> int:
    # values must correspond with enum in keyboard.h
    if filename.endswith(".z80"):
        return 4
    elif filename.endswith(".sna"):
        return 3
    else:
        raise ValueError("invalid image format")


WIDTH = 320
HEIGHT = 240


def to_rgb(raw: bytes) -> np.ndarray:  # [int, np.dtype[int]]:
    img = np.empty((HEIGHT, WIDTH, 3), dtype=np.uint8)

    for i in range(0, len(raw), 2):
        b = (raw[i] & 0x1F) << 3
        g = ((raw[i] >> 5) | ((raw[i + 1] & 0x7) << 3)) << 2
        r = (raw[i + 1] >> 3) << 3

        x = (i // 2) % WIDTH
        y = (i // 2) // WIDTH
        img[y, x] = [r, g, b]

    return img


def upload_image(zxspectrum: Serial, filename: str) -> None:
    mode = get_mode(filename)
    zxspectrum.write(mode.to_bytes(1, "little"))

    with open(filename, "rb") as fh:
        data = fh.read()
        n = len(data)
        zxspectrum.write(n.to_bytes(2, "little"))
        zxspectrum.write(data)


def get_snapshot(zxspectrum: Serial) -> None:
    zxspectrum.write((1).to_bytes(1, "little"))
    filename = f'zx{datetime.now().strftime("%Y-%m-%dT%H:%M:%S")}.z80'
    with open(filename, "wb") as fh:
        hex = zxspectrum.readline().rstrip().decode("utf-8")
        z80 = bytes.fromhex(hex)
        if len(z80) != 49182:
            print(f"error received {len(z80)} bytes, expected 49182")
        else:
            print(f"wrote image ({len(z80)} bytes) to {filename}")
            fh.write(z80)


def get_screenshot(zxspectrum: Serial) -> None:
    zxspectrum.write((5).to_bytes(1, "little"))
    filename = f'zx{datetime.now().strftime("%Y-%m-%dT%H:%M:%S")}.png'
    hex = zxspectrum.readline().rstrip().decode("utf-8")
    raw = to_rgb(bytes.fromhex(hex))

    img = Image.fromarray(raw, "RGB")
    img.save(filename)
    print(f"wrote image to {filename}")


def reset(zxspectrum: Serial) -> None:
    zxspectrum.write((2).to_bytes(1, "little"))


def main(
    filename: Optional[str] = typer.Argument(
        None, help="a .z80 or .sna format image to load at startup"
    ),
) -> None:
    """Start the ZX Spectrum emulator

    Home saves a screenshot

    Ins saves a snapshot

    Del (or X button on screen) resets machine to last snapshot
    """
    if not Path(DEVICE).exists:
        raise FileNotFoundError("usb device not found")
    zxspectrum = Serial(DEVICE, BAUD_RATE)

    print("esc, ctrl-C to exit")

    kbd_ram = bytearray.fromhex("ffffffffffffffff")

    def on_press(key: Key) -> None:
        if key == Key.esc:
            exit(0)
        if key == Key.home:
            get_screenshot(zxspectrum)
            return
        if key == Key.insert:
            get_snapshot(zxspectrum)
            return
        if key == Key.delete:
            reset(zxspectrum)
            return

        codes = keymap.get(key, ())
        if not codes:
            print(f"{key} not mapped")
            return

        for port, bit in codes:
            kbd_ram[port] &= ~(1 << bit)
        zxspectrum.write((0).to_bytes(1, "little"))
        zxspectrum.write(kbd_ram)
        # print(f"{keymap[key]} {kbd_ram.hex()}")

    def on_release(key: Key) -> None:
        codes = keymap.get(key, ())
        if not codes:
            return

        for port, bit in codes:
            kbd_ram[port] |= 1 << bit
        zxspectrum.write((0).to_bytes(1, "little"))
        zxspectrum.write(kbd_ram)
        # print(f"{keymap[key]} {kbd_ram.hex()}")

    if filename:
        upload_image(zxspectrum, filename)
    else:
        zxspectrum.write((0).to_bytes(1, "little"))

    with Listener(on_press=on_press, on_release=on_release, suppress=True) as listener:
        while True:
            listener.join()


if __name__ == "__main__":
    typer.run(main)
