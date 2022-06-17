#pragma once

namespace sound {

void init();
void playSound(int chan, int volume, int freq);
void playBuzz(int size, int val);

}