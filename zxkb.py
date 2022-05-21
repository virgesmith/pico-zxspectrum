from tkinter.tix import Tree
from pynput.keyboard import Key, KeyCode, Listener
from serial import Serial
from pathlib import Path
from time import sleep

DEVICE = "/dev/ttyACM0"
BAUD_RATE = 115200
CAPS_MASK = 128
SYM_MASK = 64

CAPS_SHIFT = 224
SYM_SHIFT = 225

caps = False
sym = False

keymap = {
  KeyCode(char='1'): 30,
  KeyCode(char='2'): 31,
  KeyCode(char='3'): 32,
  KeyCode(char='4'): 33,
  KeyCode(char='5'): 34,
  KeyCode(char='6'): 35,
  KeyCode(char='7'): 36,
  KeyCode(char='8'): 37,
  KeyCode(char='9'): 38,
  KeyCode(char='0'): 39,
  KeyCode(char='!'): 30 + CAPS_MASK,
  KeyCode(char='"'): 31 + CAPS_MASK,
  KeyCode(char='£'): 32 + CAPS_MASK,
  KeyCode(char='$'): 33 + CAPS_MASK,
  KeyCode(char='%'): 34 + CAPS_MASK, Key.left: 34 + CAPS_MASK,
  KeyCode(char='^'): 35 + CAPS_MASK, Key.down: 35 + CAPS_MASK,
  KeyCode(char='&'): 36 + CAPS_MASK, Key.up: 36 + CAPS_MASK,
  KeyCode(char='*'): 37 + CAPS_MASK, Key.right: 37 + CAPS_MASK,
  KeyCode(char='('): 38 + CAPS_MASK, Key.alt_gr: 38 + CAPS_MASK,
  KeyCode(char=')'): 39 + CAPS_MASK, Key.backspace: 39 + CAPS_MASK,

  KeyCode(char='q'): 20,
  KeyCode(char='w'): 26,
  KeyCode(char='e'): 8 ,
  KeyCode(char='r'): 21,
  KeyCode(char='t'): 23,
  KeyCode(char='y'): 28,
  KeyCode(char='u'): 24,
  KeyCode(char='i'): 12,
  KeyCode(char='o'): 18,
  KeyCode(char='p'): 19,

  KeyCode(char='Q'): 20 + CAPS_MASK,
  KeyCode(char='W'): 26 + CAPS_MASK,
  KeyCode(char='E'): 8  + CAPS_MASK,
  KeyCode(char='R'): 21 + CAPS_MASK,
  KeyCode(char='T'): 23 + CAPS_MASK,
  KeyCode(char='Y'): 28 + CAPS_MASK,
  KeyCode(char='U'): 24 + CAPS_MASK,
  KeyCode(char='I'): 12 + CAPS_MASK,
  KeyCode(char='O'): 18 + CAPS_MASK,
  KeyCode(char='P'): 19 + CAPS_MASK,

  KeyCode(char='a'): 4,
  KeyCode(char='s'): 22,
  KeyCode(char='d'): 7 ,
  KeyCode(char='f'): 9,
  KeyCode(char='g'): 10,
  KeyCode(char='h'): 11,
  KeyCode(char='j'): 13,
  KeyCode(char='k'): 14,
  KeyCode(char='l'): 15,
  Key.enter: 40,

  KeyCode(char='A'): 4  + CAPS_MASK,
  KeyCode(char='S'): 22 + CAPS_MASK,
  KeyCode(char='D'): 7  + CAPS_MASK,
  KeyCode(char='F'): 9  + CAPS_MASK,
  KeyCode(char='G'): 10 + CAPS_MASK,
  KeyCode(char='H'): 11 + CAPS_MASK,
  KeyCode(char='J'): 13 + CAPS_MASK,
  KeyCode(char='K'): 14 + CAPS_MASK,
  KeyCode(char='L'): 15 + CAPS_MASK,

  # Caps shift
  KeyCode(char='z'): 29,
  KeyCode(char='x'): 27,
  KeyCode(char='c'): 6 ,
  KeyCode(char='v'): 25,
  KeyCode(char='b'): 5 ,
  KeyCode(char='n'): 17,
  KeyCode(char='m'): 16,
  # Sym shift
  Key.space: 44,

  # Caps shift
  KeyCode(char='Z'): 29 + CAPS_MASK,
  KeyCode(char='X'): 27 + CAPS_MASK,
  KeyCode(char='C'): 6  + CAPS_MASK,
  KeyCode(char='V'): 25 + CAPS_MASK,
  KeyCode(char='B'): 5  + CAPS_MASK,
  KeyCode(char='N'): 17 + CAPS_MASK,
  KeyCode(char='M'): 16 + CAPS_MASK,
  # Sym shift
  Key.space: 44 + CAPS_MASK, # break

  # for convenience
  Key.caps_lock: 31 + CAPS_MASK,
  KeyCode(char=","): 17 + SYM_MASK,
  KeyCode(char="."): 16 + SYM_MASK,
  KeyCode(char="<"): 21 + SYM_MASK,
  KeyCode(char=">"): 23 + SYM_MASK,
  KeyCode(char="?"): 6 + SYM_MASK,
  KeyCode(char="/"): 25 + SYM_MASK,
  KeyCode(char=";"): 18 + SYM_MASK,
  KeyCode(char=":"): 29 + SYM_MASK,

  KeyCode(char="-"): 13 + SYM_MASK,
  KeyCode(char="="): 15 + SYM_MASK,
  KeyCode(char="_"): 39 + SYM_MASK,
  KeyCode(char="+"): 14 + SYM_MASK,

  # extended mode test
  #Key.tab: CAPS_MASK + SYM_MASK,
}

def on_press(key: Key):
  # print(key)
  global caps, sym
  if key == Key.esc:
    exit(0)
  if key == Key.shift:
    caps = True
  elif key == Key.ctrl_r:
    sym = True
    if caps and sym:
      device.write((CAPS_MASK + SYM_MASK).to_bytes(1, byteorder='little'))
  else:
    code = keymap.get(key, 0)
    if sym:
      code += SYM_MASK
    # print(f'{"shift-" if caps else ""}{"sym-" if sym else ""}{key} -> {code}')
    device.write(code.to_bytes(1, byteorder='little'))


def on_release(key: Key):
  global caps, sym
  if key == Key.shift:
    caps = False
    #device.write(CAPS_SHIFT.to_bytes(1, byteorder='little'))
  elif key == Key.ctrl_r:
    sym = False
    #device.write(SYM_SHIFT.to_bytes(1, byteorder='little'))

if not Path(DEVICE).exists:
  raise FileNotFoundError("usb device not found")

device = Serial(DEVICE, BAUD_RATE)

with Listener(on_press=on_press, on_release=on_release, suppress=True) as listener:
  while True:
    #c0, t0, c1, t1 = [b for b in device.read(4)]
    #print(f"{c0}, {t0}, {c1}, {t1}")
    b = device.read(3)
    #print(f"{int.from_bytes(b[0], 'big')}, {int.from_bytes(b[1], 'big')}, {int.from_bytes(b[2]), 'big'}")
    print(f"{b[0]}, {b[1]}, {b[2]}")

listener.join()
