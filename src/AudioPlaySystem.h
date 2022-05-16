#pragma once

// TODO this could be a singleton
class AudioPlaySystem
{
public:
  AudioPlaySystem() { };
  void begin();
  void setSampleParameters(float clockfreq, float samplerate);
  void reset();
  void start();
  void stop();
  bool isPlaying();
  void sound(int C, int F, int V);
  void buzz(int size, int val);
  void step();
  static void snd_Mixer(short* stream, int len);
};


