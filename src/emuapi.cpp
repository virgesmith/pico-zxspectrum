#include "emuapi.h"

extern "C" {
#include "iopins.h"
}


#include "pico.h"
#include "pico/stdlib.h"
//#include "hardware/adc.h"

#include <cstdio>
#include <cstring>


// #define MAX_FILENAME_PATH   64
// #define NB_FILE_HANDLER     4
// #define AUTORUN_FILENAME    "autorun.txt"


namespace {

unsigned char keymatrix[6];
int keymatrix_hitrow=-1;
bool key_fn=false;
bool key_alt=false;
uint32_t keypress_t_ms=0;
uint32_t last_t_ms=0;
uint32_t hundred_ms_cnt=0;
bool ledflash_toggle=false;

uint16_t bLastState;

// TODO int -> size_t

// char malbuf[EXTRA_HEAP];
// int malbufpt = malbuf;

}

// namespace emu {
// void drawText(unsigned short x, unsigned short y, const char * text, unsigned short fgcolor, unsigned short bgcolor, int doublesize)
// {
//   tft.drawText(x, y, text, fgcolor, bgcolor, doublesize?true:false);
// }

// }

int emu::readKeys()
{
  uint16_t retval = 0;

  keymatrix_hitrow = -1;
  unsigned char row;
  unsigned short cols[6]={KCOLOUT1,KCOLOUT2,KCOLOUT3,KCOLOUT4,KCOLOUT5,KCOLOUT6};
  unsigned char keymatrixtmp[6];

  for (int i=0;i<6;i++){
    gpio_set_dir(cols[i], GPIO_OUT);
    gpio_put(cols[i], 0);
#ifdef SWAP_ALT_DEL
    sleep_us(1);
    //__asm volatile ("nop\n"); // 4-8ns
#endif
    row=0;
    row |= (gpio_get(KROWIN2) ? 0 : 0x01);
    row |= (gpio_get(KROWIN2) ? 0 : 0x01);
    row |= (gpio_get(KROWIN2) ? 0 : 0x01);
    row |= (gpio_get(KROWIN2) ? 0 : 0x01);
    row |= (gpio_get(KROWIN4) ? 0 : 0x02);
    row |= (gpio_get(KROWIN1) ? 0 : 0x04);
    row |= (gpio_get(KROWIN3) ? 0 : 0x08);
    row |= (gpio_get(KROWIN5) ? 0 : 0x10);
    row |= (gpio_get(KROWIN6) ? 0 : 0x20);
    //gpio_set_dir(cols[i], GPIO_OUT);
    gpio_put(cols[i], 1);
    gpio_set_dir(cols[i], GPIO_IN);
    gpio_disable_pulls(cols[i]);
    keymatrixtmp[i] = row;
  }

#ifdef MULTI_DEBOUNCE
  for (int i=0;i<6;i++){
    gpio_set_dir(cols[i], GPIO_OUT);
    gpio_put(cols[i], 0);
#ifdef SWAP_ALT_DEL
    sleep_us(1);
    //__asm volatile ("nop\n"); // 4-8ns
#endif
    row=0;
    row |= (gpio_get(KROWIN2) ? 0 : 0x01);
    row |= (gpio_get(KROWIN2) ? 0 : 0x01);
    row |= (gpio_get(KROWIN2) ? 0 : 0x01);
    row |= (gpio_get(KROWIN2) ? 0 : 0x01);
    row |= (gpio_get(KROWIN4) ? 0 : 0x02);
    row |= (gpio_get(KROWIN1) ? 0 : 0x04);
    row |= (gpio_get(KROWIN3) ? 0 : 0x08);
    row |= (gpio_get(KROWIN5) ? 0 : 0x10);
    row |= (gpio_get(KROWIN6) ? 0 : 0x20);
    //gpio_set_dir(cols[i], GPIO_OUT);
    gpio_put(cols[i], 1);
    gpio_set_dir(cols[i], GPIO_IN);
    gpio_disable_pulls(cols[i]);
    keymatrixtmp[i] |= row;
  }

  for (int i=0;i<6;i++){
    gpio_set_dir(cols[i], GPIO_OUT);
    gpio_put(cols[i], 0);
#ifdef SWAP_ALT_DEL
    sleep_us(1);
    //__asm volatile ("nop\n"); // 4-8ns
#endif
    row=0;
    row |= (gpio_get(KROWIN2) ? 0 : 0x01);
    row |= (gpio_get(KROWIN2) ? 0 : 0x01);
    row |= (gpio_get(KROWIN2) ? 0 : 0x01);
    row |= (gpio_get(KROWIN2) ? 0 : 0x01);
    row |= (gpio_get(KROWIN4) ? 0 : 0x02);
    row |= (gpio_get(KROWIN1) ? 0 : 0x04);
    row |= (gpio_get(KROWIN3) ? 0 : 0x08);
    row |= (gpio_get(KROWIN5) ? 0 : 0x10);
    row |= (gpio_get(KROWIN6) ? 0 : 0x20);
    //gpio_set_dir(cols[i], GPIO_OUT);
    gpio_put(cols[i], 1);
    gpio_set_dir(cols[i], GPIO_IN);
    gpio_disable_pulls(cols[i]);
    keymatrixtmp[i] |= row;
  }
#endif

#ifdef SWAP_ALT_DEL
  // Swap ALT and DEL
  unsigned char alt = keymatrixtmp[0] & 0x02;
  unsigned char del = keymatrixtmp[5] & 0x20;
  keymatrixtmp[0] &= ~0x02;
  keymatrixtmp[5] &= ~0x20;
  if (alt) keymatrixtmp[5] |= 0x20;
  if (del) keymatrixtmp[0] |= 0x02;
#endif

  bool alt_pressed=false;
  if ( keymatrixtmp[5] & 0x20 ) {alt_pressed=true; keymatrixtmp[5] &= ~0x20;};

  for (int i=0;i<6;i++){
    row = keymatrixtmp[i];
    if (row) keymatrix_hitrow=i;
    keymatrix[i] = row;
  }

  //6,9,15,8,7,22
#if INVX
  if ( row & 0x2  ) retval |= MASK_JOY2_LEFT;
  if ( row & 0x1  ) retval |= MASK_JOY2_RIGHT;
#else
  if ( row & 0x1  ) retval |= MASK_JOY2_LEFT;
  if ( row & 0x2  ) retval |= MASK_JOY2_RIGHT;
#endif
#if INVY
  if ( row & 0x8  ) retval |= MASK_JOY2_DOWN;
  if ( row & 0x4  ) retval |= MASK_JOY2_UP;
#else
  if ( row & 0x4  ) retval |= MASK_JOY2_DOWN;
  if ( row & 0x8  ) retval |= MASK_JOY2_UP;
#endif
  if ( row & 0x10 ) retval |= MASK_JOY2_BTN;

  // Handle LED flash
  uint32_t time_ms=to_ms_since_boot (get_absolute_time());
  if ((time_ms-last_t_ms) > 100) {
    last_t_ms = time_ms;
    if (ledflash_toggle == false) {
      ledflash_toggle = true;
    }
    else {
      ledflash_toggle = false;
    }
  }

  if ( alt_pressed ) {
    if (key_fn == false)
    {
      // Release to Press transition
      if (hundred_ms_cnt == 0) {
        keypress_t_ms=time_ms;
        hundred_ms_cnt += 1; // 1
      }
      else {
        hundred_ms_cnt += 1; // 2
        if (hundred_ms_cnt >= 2)
        {
          hundred_ms_cnt = 0;
          /*
          if ( (time_ms-keypress_t_ms) < 500)
          {
            if (key_alt == false)
            {
              key_alt = true;
            }
            else
            {
              key_alt = false;
            }
          }
          */
        }
      }
    }
    else {
      // Keep press
      if (hundred_ms_cnt == 1) {
        if ((to_ms_since_boot (get_absolute_time())-keypress_t_ms) > 2000)
        {
          if (key_alt == false)
          {
            key_alt = true;
          }
          else
          {
            key_alt = false;
          }
          hundred_ms_cnt = 0;
        }
      }
    }
    key_fn = true;
  }
  else  {
    key_fn = false;
  }

  // Handle LED
  if (key_alt == true) {
    gpio_put(KLED, (ledflash_toggle?1:0));
  }
  else {
    if (key_fn == true) {
      gpio_put(KLED, 1);
    }
    else {
      gpio_put(KLED, 0);
    }
  }

  if ( key_fn ) retval |= MASK_KEY_USER2;
  if ( ( key_fn ) && (keymatrix[0] == 0x02 )) retval |= MASK_KEY_USER1;
//#endif

  //Serial.println(retval,HEX);

  if ( ((retval & (MASK_KEY_USER1+MASK_KEY_USER2)) == (MASK_KEY_USER1+MASK_KEY_USER2))
     || (retval & MASK_KEY_USER4 ) )
  {
  }

  return (retval);
}

unsigned short emu::debounceLocalKeys()
{
  uint16_t bCurState = readKeys();
  uint16_t bClick = bCurState & ~bLastState;
  bLastState = bCurState;

  return (bClick);
}


unsigned char emu::readUsbSerial() {
  // mapping handled on client side now
  unsigned char c = getchar_timeout_us(0);
  if (c == 255)
    c = 0;
  return c;

  // const char UPPER = 128;
  // if (c != 255) printf("%c %d\n", c, c);
  // switch (c)
  // {
  //   case '1': return 30;
  //   case '2': return 31;
  //   case '3': return 32;
  //   case '4': return 33;
  //   case '5': return 34;
  //   case '6': return 35;
  //   case '7': return 36;
  //   case '8': return 37;
  //   case '9': return 38;
  //   case '0': return 39;
  //   case ')': return 39 + UPPER; // delete

  //   case 'q': return 20;
  //   case 'Q': return 20 + UPPER;
  //   case 'w': return 26;
  //   case 'e': return 8;
  //   case 'r': return 21;
  //   case 't': return 23;
  //   case 'y': return 28;
  //   case 'u': return 24;
  //   case 'i': return 12;
  //   case 'o': return 18;
  //   case 'p': return 19;

  //   case 'a': return 4;
  //   case 's': return 22;
  //   case 'd': return 7;
  //   case 'f': return 9;
  //   case 'g': return 10;
  //   case 'h': return 11;
  //   case 'j': return 13;
  //   case 'k': return 14;
  //   case 'l': return 15;
  //   case '#': return 40;

  //   //case '?': return 224; caps shift
  //   case 'z': return 29;
  //   case 'x': return 27;
  //   case 'c': return 6;
  //   case 'v': return 25;
  //   case 'b': return 5;
  //   case 'n': return 17;
  //   case 'm': return 16;
  //   //case ' ': return 225; symbol shift
  //   case ' ': return 44;

  //   default: return 0;
  // }

  // int retval = getchar_timeout_us(0);
  // if (retval != -1) {
  //   // static int k = 32;
  //   // emu_printf(k);
  //   return retval;
  // }
  // return 0;
}


// int emu_setKeymap(int index) {
// }


#include "AudioPlaySystem.h"
AudioPlaySystem mymixer;
#include "hardware/pwm.h"

void emu::sndInit()
{
    // uses core1
    // tft.begin_audio(256, mymixer.snd_Mixer);
    // mymixer.start();
    // gpio_init(AUDIO_PIN);
    // gpio_set_dir(AUDIO_PIN, GPIO_OUT);
}

void emu::sndPlaySound(int chan, int volume, int freq)
{
    if (chan < 6)
    {
        mymixer.sound(chan, freq, volume);
    }
}

void emu::sndPlayBuzz(int size, int val)
{
#ifndef CUSTOM_SND
    // gpio_put(AUDIO_PIN, (val?1:0));
    pwm_set_gpio_level(AUDIO_PIN, (val ? 255 : 128));
#endif
}

