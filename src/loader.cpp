//--------------------------------------------------------------
// File derived from:
// Datum    : 27.01.2014
// Version  : 1.0
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
//--------------------------------------------------------------
#include "loader.h"
#include "display.h"

#include <cstring>

//-------------------------------------------------------------
extern uint8_t out_ram;

//--------------------------------------------------------------
// interne Funktionen
//--------------------------------------------------------------
namespace {

//--------------------------------------------------------------
// Internal function
// Unpack and store a block of data
// ( From flash ) the new version
//--------------------------------------------------------------
const uint8_t* p_decompFlashBlock(const uint8_t *block_adr)
{
  const uint8_t *ptr;
  const uint8_t *next_block;
  uint8_t value1,value2;
  uint16_t block_len;
  uint8_t flag_compressed=0;
  uint8_t flag_page=0;
  uint16_t cur_addr=0;

  // pointer auf Blockanfang setzen
  ptr=block_adr;

  // Laenge vom Block
  value1=*(ptr++);
  value2=*(ptr++);
  block_len=(value2<<8)|value1;
  if(block_len==0xFFFF) {
    block_len=0x4000;
    flag_compressed=0;
  }
  else {
    flag_compressed=1;
  }

  // Page vom Block
  flag_page=*(ptr++);

  // next Block ausrechnen
  next_block=(uint8_t*)(ptr+block_len);

  // Startadresse setzen
  if(flag_page==4) cur_addr=0x8000;
  if(flag_page==5) cur_addr=0xC000;
  if(flag_page==8) cur_addr=0x4000;

  if(flag_compressed==1) {
    //-----------------------
    // komprimiert
    //-----------------------
    while(ptr<(block_adr+3+block_len)) {
      value1=*(ptr++);
      if(value1!=0xED) {
        WrZ80(cur_addr++, value1);
      }
      else {
        value2=*(ptr++);
        if(value2!=0xED) {
          WrZ80(cur_addr++, value1);
          WrZ80(cur_addr++, value2);
        }
        else {
          value1=*(ptr++);
          value2=*(ptr++);
          while(value1--) {
            WrZ80(cur_addr++, value2);
          }
        }
      }
    }
  }
  else {
    //-----------------------
    // nicht komprimiert
    //-----------------------
    while(ptr<(block_adr+3+block_len)) {
      value1=*(ptr++);
      WrZ80(cur_addr++, value1);
    }
  }

  return(next_block);
}

}


void load_image_sna(Z80 *regs, const byte* data, uint16_t len)
{
  // Load Z80 registers from SNA
  regs->I       = data[ 0];
  regs->HL1.B.l = data[ 1];
  regs->HL1.B.h = data[ 2];
  regs->DE1.B.l = data[ 3];
  regs->DE1.B.h = data[ 4];
  regs->BC1.B.l = data[ 5];
  regs->BC1.B.h = data[ 6];
  regs->AF1.B.l = data[ 7];
  regs->AF1.B.h = data[ 8];
  regs->HL.B.l  = data[ 9];
  regs->HL.B.h  = data[10];
  regs->DE.B.l  = data[11];
  regs->DE.B.h  = data[12];
  regs->BC.B.l  = data[13];
  regs->BC.B.h  = data[14];
  regs->IY.B.l  = data[15];
  regs->IY.B.h  = data[16];
  regs->IX.B.l  = data[17];
  regs->IX.B.h  = data[18];
 //#define IFF_1       0x01       /* IFF1 flip-flop             */
//#define IFF_IM1     0x02       /* 1: IM1 mode                */
//#define IFF_IM2     0x04       /* 1: IM2 mode                */
//#define IFF_2       0x08       /* IFF2 flip-flop             */
//#define IFF_EI      0x20       /* 1: EI pending              */
//#define IFF_HALT    0x80       /* 1: CPU HALTed              */
  regs->R = data[20]; //R.W
  regs->AF.B.l = data[21];
  regs->AF.B.h = data[22];
  regs->SP.B.l =data[23];
  regs->SP.B.h =data[24];
  regs->IFF = 0;
  regs->IFF |= (((data[19] & 0x04) >> 2) ? IFF_1 : 0); //regs->IFF1 = regs->IFF2 = ...
  regs->IFF |= (((data[19] & 0x04) >> 2) ? IFF_2 : 0);
  regs->IFF |= (data[25]<< 1); // regs->IM = data[25];
  //regs->BorderColor = data[26];
  emu::display::bordercolor = data[26] & 0x07;

  // load RAM from SNA
  for (int i = 0; i != 0xbfff; ++i)
  {
    WrZ80(i + 0x4000, data[27 + i]);
  }
  // SP to PC for SNA run
  regs->PC.B.l = RdZ80(regs->SP.W);
  regs->SP.W++;
  regs->PC.B.h = RdZ80(regs->SP.W);
  regs->SP.W++;
}

byte* save_image_sna()
{
  byte* data = (byte*)malloc(SNA_LEN);
  if (!data)
    return nullptr;

  // // PC to SP for SNA run?
  // WrZ80(spec::myCPU.SP.W, spec::myCPU.PC.B.l);
  // spec::myCPU.SP.W++;
  // WrZ80(spec::myCPU.SP.W, spec::myCPU.PC.B.h);
  // spec::myCPU.SP.W++;

  // Save Z80 registers into SNA
  data[ 0] = spec::myCPU.I      ;
  data[ 1] = spec::myCPU.HL1.B.l;
  data[ 2] = spec::myCPU.HL1.B.h;
  data[ 3] = spec::myCPU.DE1.B.l;
  data[ 4] = spec::myCPU.DE1.B.h;
  data[ 5] = spec::myCPU.BC1.B.l;
  data[ 6] = spec::myCPU.BC1.B.h;
  data[ 7] = spec::myCPU.AF1.B.l;
  data[ 8] = spec::myCPU.AF1.B.h;
  data[ 9] = spec::myCPU.HL.B.l ;
  data[10] = spec::myCPU.HL.B.h ;
  data[11] = spec::myCPU.DE.B.l;
  data[12] = spec::myCPU.DE.B.h;
  data[13] = spec::myCPU.BC.B.l;
  data[14] = spec::myCPU.BC.B.h;
  data[15] = spec::myCPU.IY.B.l;
  data[16] = spec::myCPU.IY.B.h;
  data[17] = spec::myCPU.IX.B.l;
  data[18] = spec::myCPU.IX.B.h;
  data[19] = (spec::myCPU.IFF & IFF_EI) ? 0x4 : 0;
  data[20] = spec::myCPU.R; //R.W
  data[21] = spec::myCPU.AF.B.l;
  data[22] = spec::myCPU.AF.B.h;
  data[23] = spec::myCPU.SP.B.l;
  data[24] = spec::myCPU.SP.B.h;
  data[25] = (spec::myCPU.IFF & 0b110) >> 1;
  data[26] = emu::display::bordercolor;

  // save RAM to SNA
  for (uint16_t i = 0; i < 0xc000; ++i)
  {
    data[27 + i] = RdZ80(i + 0x4000);
  }
  return data;
}

//--------------------------------------------------------------
// Unpack data from a file ( type = * .Z80 ) from flash
// And copy them to the memory of the ZX - Spectrum
//
// Data = pointer to the start of data
// Length = number of bytes
//--------------------------------------------------------------
void load_image_z80(Z80 *R, const uint8_t *data, uint16_t length)
{
  const uint8_t *ptr;
  const uint8_t *akt_block,*next_block;
  uint8_t value1,value2;
  uint8_t flag_version=0;
  uint8_t flag_compressed=0;
  uint16_t header_len;
  uint16_t cur_addr;

  if(length==0) return;
  if(length>0xC020) return;

  //----------------------------------
  // parsing header
  // Byte : [0...29]
  //----------------------------------

  // Set pointer to data beginning
  ptr=data;

  R->AF.B.h=*(ptr++); // A [0]
  R->AF.B.l=*(ptr++); // F [1]
  R->BC.B.l=*(ptr++); // C [2]
  R->BC.B.h=*(ptr++); // B [3]
  R->HL.B.l=*(ptr++); // L [4]
  R->HL.B.h=*(ptr++); // H [5]

  // PC [6+7]
  value1=*(ptr++);
  value2=*(ptr++);
  R->PC.W=(value2<<8)|value1;
  if(R->PC.W==0x0000) {
    flag_version=1;
  }
  else {
    flag_version=0;
  }

  // SP [8+9]
  value1=*(ptr++);
  value2=*(ptr++);
  R->SP.W=(value2<<8)|value1;

  R->I=*(ptr++); // I [10]
  R->R=*(ptr++); // R [11]

  // Comressed-Flag & Border [12]
  value1=*(ptr++);
  value2=((value1&0x0E)>>1);
  OutZ80(0xFE, value2); // BorderColor
  if((value1&0x20)!=0) {
    flag_compressed=1;
  } else {
    flag_compressed=0;
  }

  R->DE.B.l=*(ptr++); // E [13]
  R->DE.B.h=*(ptr++); // D [14]
  R->BC1.B.l=*(ptr++); // C1 [15]
  R->BC1.B.h=*(ptr++); // B1 [16]
  R->DE1.B.l=*(ptr++); // E1 [17]
  R->DE1.B.h=*(ptr++); // D1 [18]
  R->HL1.B.l=*(ptr++); // L1 [19]
  R->HL1.B.h=*(ptr++); // H1 [20]
  R->AF1.B.h=*(ptr++); // A1 [21]
  R->AF1.B.l=*(ptr++); // F1 [22]
  R->IY.B.l=*(ptr++); // Y [23]
  R->IY.B.h=*(ptr++); // I [24]
  R->IX.B.l=*(ptr++); // X [25]
  R->IX.B.h=*(ptr++); // I [26]

  // Interrupt-Flag [27]
  value1=*(ptr++);
  if(value1!=0) {
    // EI
    R->IFF|=IFF_2|IFF_EI;
  }
  else {
    // DI
    R->IFF&=~(IFF_1|IFF_2|IFF_EI);
  }
  value1=*(ptr++); // nc [28]
  // Interrupt-Mode [29]
  value1=*(ptr++);
  if((value1&0x01)!=0) {
    R->IFF|=IFF_IM1;
  }
  else {
    R->IFF&=~IFF_IM1;
  }
  if((value1&0x02)!=0) {
    R->IFF|=IFF_IM2;
  }
  else {
    R->IFF&=~IFF_IM2;
  }

  // restliche Register
  R->ICount   = R->IPeriod;
  R->IRequest = INT_NONE;
  R->IBackup  = 0;

  //----------------------------------
  // save the data in RAM
  // Byte : [30...n]
  //----------------------------------

  cur_addr=0x4000; // RAM start


  if(flag_version==0) {
    //-----------------------
    // old Version
    //-----------------------
    if(flag_compressed==1) {
      //-----------------------
      // compressed
      //-----------------------
      while(ptr<(data+length)) {
        value1=*(ptr++);
        if(value1!=0xED) {
          WrZ80(cur_addr++, value1);
        }
        else {
          value2=*(ptr++);
          if(value2!=0xED) {
            WrZ80(cur_addr++, value1);
            WrZ80(cur_addr++, value2);
          }
          else {
            value1=*(ptr++);
            value2=*(ptr++);
            while(value1--) {
              WrZ80(cur_addr++, value2);
            }
          }
        }
      }
    }
    else {
      //-----------------------
      // raw (uncompressed)
      //-----------------------
      while(ptr<(data+length)) {
        value1=*(ptr++);
        WrZ80(cur_addr++, value1);
      }
    }
  }
  else {
    //-----------------------
    // new Version
    //-----------------------
    // Header Laenge [30+31]
    value1=*(ptr++);
    value2=*(ptr++);
    header_len=(value2<<8)|value1;
    akt_block=(uint8_t*)(ptr+header_len);
    // PC [32+33]
    value1=*(ptr++);
    value2=*(ptr++);
    R->PC.W=(value2<<8)|value1;

    //------------------------
    // 1st block parsing
    //------------------------
    next_block=p_decompFlashBlock(akt_block);
    //------------------------
    // all other parsing
    //------------------------
    while(next_block<data+length) {
      akt_block=next_block;
      next_block=p_decompFlashBlock(akt_block);
    }
  }
}


byte* save_image_z80(const Z80 *R, const uint8_t *ram)
{
  //----------------------------------
  // construct header
  // Byte : [0...29]
  //----------------------------------
  byte* data = (byte*)malloc(30 + 0xc000);

  data[0] = R->AF.B.h; // A [0]
  data[1] = R->AF.B.l; // F [1]
  data[2] = R->BC.B.l; // C [2]
  data[3] = R->BC.B.h; // B [3]
  data[4] = R->HL.B.l; // L [4]
  data[5] = R->HL.B.h; // H [5]

  data[6] = R->PC.B.l; // PC
  data[7] = R->PC.B.h; //
  data[8] = R->SP.B.l; // SP
  data[9] = R->SP.B.h; //

  // // SP [8+9]
  // value1=*(ptr++);
  // value2=*(ptr++);
  // R->SP.W=(value2<<8)|value1;

  data[10] = R->I; // I [10]
  data[11] = R->R & 0x7f; // R [11]

  // Comressed-Flag & Border [12]
  //data[12] = ((R->R & 0x80) >> 7) & (emu::display::bordercolor << 1);
  data[12] = ((R->R & 0x80) >> 7) & (InZ80(0xFE) << 1);

  // value1=*(ptr++);
  // value2=((value1&0x0E)>>1);
  // OutZ80(0xFE, value2); // BorderColor
  // if((value1&0x20)!=0) {
  //   flag_compressed=1;
  // } else {
  //   flag_compressed=0;
  // }

  data[13] = R->DE.B.l; // E [13]
  data[14] = R->DE.B.h; // D [14]
  data[15] = R->BC1.B.l; // C1 [15]
  data[16] = R->BC1.B.h; // B1 [16]
  data[17] = R->DE1.B.l; // E1 [17]
  data[18] = R->DE1.B.h; // D1 [18]
  data[19] = R->HL1.B.l; // L1 [19]
  data[20] = R->HL1.B.h; // H1 [20]
  data[21] = R->AF1.B.h; // A1 [21]
  data[22] = R->AF1.B.l; // F1 [22]
  data[23] = R->IY.B.l; // Y [23]
  data[24] = R->IY.B.h; // I [24]
  data[25] = R->IX.B.l; // X [25]
  data[26] = R->IX.B.h; // I [26]

  // Interrupt-Flag [27]
  data[27] = (R->IFF | (IFF_2|IFF_EI)) ? 1 : 0;

  data[28] = (R->IFF & IFF_IM2); // "IFF2 (not particularly important...)"

  // Interrupt-Mode
  data[29] = ((R->IFF & IFF_IM1) ? 0x01 : 0) | ((R->IFF & IFF_IM2) ? 0x02 : 0);

  // // restliche Register
  // R->ICount   = R->IPeriod;
  // R->IRequest = INT_NONE;
  // R->IBackup  = 0;

  //-----------------------
  // old Version 1 raw (uncompressed),
  //-----------------------
  memcpy(data + 30, ram, 0xc000);

  return data;
}



