#pragma once

namespace emu::sound {

void init();
void playSound(int chan, int volume, int freq);
void playBuzz(int size, int val);

}