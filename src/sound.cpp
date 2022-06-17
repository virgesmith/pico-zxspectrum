#include "sound.h"

extern "C"
{
#include "iopins.h"
}

#include <cstdio>
#include <cstring>

#include "AudioPlaySystem.h"
#include "hardware/pwm.h"

AudioPlaySystem mymixer;

void sound::init()
{
  // uses core1
  // tft.begin_audio(256, mymixer.snd_Mixer);
  // mymixer.start();
  // gpio_init(AUDIO_PIN);
  // gpio_set_dir(AUDIO_PIN, GPIO_OUT);
}

void sound::playSound(int chan, int volume, int freq)
{
  if (chan < 6)
  {
    mymixer.sound(chan, freq, volume);
  }
}

void sound::playBuzz(int size, int val)
{
#ifndef CUSTOM_SND
  // gpio_put(AUDIO_PIN, (val?1:0));
  pwm_set_gpio_level(AUDIO_PIN, (val ? 255 : 128));
#endif
}
