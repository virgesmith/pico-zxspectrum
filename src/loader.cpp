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

loader::Snapshot loader::snapshot_type = loader::Snapshot::NONE;
uint16_t loader::image_size = 0;
byte loader::snapshot_buffer[Z80_LEN];
bool loader::snapshot_pending = false;
bool loader::screenshot_pending = false;
bool loader::reset_pending = false;


void loader::load_image_sna(Z80& regs)
{
  // Load Z80 registers from SNA
  regs.I       = snapshot_buffer[ 0];
  regs.HL1.B.l = snapshot_buffer[ 1];
  regs.HL1.B.h = snapshot_buffer[ 2];
  regs.DE1.B.l = snapshot_buffer[ 3];
  regs.DE1.B.h = snapshot_buffer[ 4];
  regs.BC1.B.l = snapshot_buffer[ 5];
  regs.BC1.B.h = snapshot_buffer[ 6];
  regs.AF1.B.l = snapshot_buffer[ 7];
  regs.AF1.B.h = snapshot_buffer[ 8];
  regs.HL.B.l  = snapshot_buffer[ 9];
  regs.HL.B.h  = snapshot_buffer[10];
  regs.DE.B.l  = snapshot_buffer[11];
  regs.DE.B.h  = snapshot_buffer[12];
  regs.BC.B.l  = snapshot_buffer[13];
  regs.BC.B.h  = snapshot_buffer[14];
  regs.IY.B.l  = snapshot_buffer[15];
  regs.IY.B.h  = snapshot_buffer[16];
  regs.IX.B.l  = snapshot_buffer[17];
  regs.IX.B.h  = snapshot_buffer[18];
//#define IFF_1       0x01       /* IFF1 flip-flop             */
//#define IFF_IM1     0x02       /* 1: IM1 mode                */
//#define IFF_IM2     0x04       /* 1: IM2 mode                */
//#define IFF_2       0x08       /* IFF2 flip-flop             */
//#define IFF_EI      0x20       /* 1: EI pending              */
//#define IFF_HALT    0x80       /* 1: CPU HALTed              */
  regs.R = snapshot_buffer[20]; //R.W
  regs.AF.B.l = snapshot_buffer[21];
  regs.AF.B.h = snapshot_buffer[22];
  regs.SP.B.l =snapshot_buffer[23];
  regs.SP.B.h =snapshot_buffer[24];
  regs.IFF = 0;
  regs.IFF |= (((snapshot_buffer[19] & 0x04) >> 2) ? IFF_1 : 0); //regs->IFF1 = regs->IFF2 = ...
  regs.IFF |= (((snapshot_buffer[19] & 0x04) >> 2) ? IFF_2 : 0);
  regs.IFF |= (snapshot_buffer[25]<< 1); // regs->IM = snapshot_buffer[25];
  display::bordercolor = snapshot_buffer[26] & 0x07;

  // load RAM from SNA
  for (int i = 0; i < 0xc000; ++i)
  {
    WrZ80(i + 0x4000, snapshot_buffer[27 + i]);
  }
  // SP to PC for SNA run
  regs.PC.B.l = RdZ80(regs.SP.W);
  regs.SP.W++;
  regs.PC.B.h = RdZ80(regs.SP.W);
  regs.SP.W++;

  loader::snapshot_type = loader::Snapshot::SNA;
}

// byte* loader::save_image_sna()
// {
//   byte* data = snapshot_buffer;

//   // // PC to SP for SNA run?
//   // WrZ80(spec::myCPU.SP.W, spec::myCPU.PC.B.l);
//   // spec::myCPU.SP.W++;
//   // WrZ80(spec::myCPU.SP.W, spec::myCPU.PC.B.h);
//   // spec::myCPU.SP.W++;

//   // Save Z80 registers into SNA
//   data[ 0] = spec::myCPU.I      ;
//   data[ 1] = spec::myCPU.HL1.B.l;
//   data[ 2] = spec::myCPU.HL1.B.h;
//   data[ 3] = spec::myCPU.DE1.B.l;
//   data[ 4] = spec::myCPU.DE1.B.h;
//   data[ 5] = spec::myCPU.BC1.B.l;
//   data[ 6] = spec::myCPU.BC1.B.h;
//   data[ 7] = spec::myCPU.AF1.B.l;
//   data[ 8] = spec::myCPU.AF1.B.h;
//   data[ 9] = spec::myCPU.HL.B.l ;
//   data[10] = spec::myCPU.HL.B.h ;
//   data[11] = spec::myCPU.DE.B.l;
//   data[12] = spec::myCPU.DE.B.h;
//   data[13] = spec::myCPU.BC.B.l;
//   data[14] = spec::myCPU.BC.B.h;
//   data[15] = spec::myCPU.IY.B.l;
//   data[16] = spec::myCPU.IY.B.h;
//   data[17] = spec::myCPU.IX.B.l;
//   data[18] = spec::myCPU.IX.B.h;
//   data[19] = (spec::myCPU.IFF & IFF_EI) ? 0x4 : 0;
//   data[20] = spec::myCPU.R; //R.W
//   data[21] = spec::myCPU.AF.B.l;
//   data[22] = spec::myCPU.AF.B.h;
//   data[23] = spec::myCPU.SP.B.l;
//   data[24] = spec::myCPU.SP.B.h;
//   data[25] = (spec::myCPU.IFF & 0b110) >> 1;
//   data[26] = display::bordercolor;

//   // save RAM to SNA
//   for (uint16_t i = 0; i < 0xc000; ++i)
//   {
//     data[27 + i] = RdZ80(i + 0x4000);
//   }
//   return snapshot_buffer;
// }`

//--------------------------------------------------------------
// Unpack data from a file ( type = * .Z80 ) from flash
// And copy them to the memory of the ZX - Spectrum
//
// Data = pointer to the start of data
// Length = number of bytes
//--------------------------------------------------------------
void loader::load_image_z80(Z80& regs)
{
  const uint8_t* ptr = snapshot_buffer;
  const uint8_t* akt_block,*next_block;
  uint8_t value1,value2;
  uint8_t flag_version=0;
  uint8_t flag_compressed=0;
  uint16_t header_len;
  uint16_t cur_addr;

  //----------------------------------
  // parsing header
  // Byte : [0...29]
  //----------------------------------

  regs.AF.B.h=*(ptr++); // A [0]
  regs.AF.B.l=*(ptr++); // F [1]
  regs.BC.B.l=*(ptr++); // C [2]
  regs.BC.B.h=*(ptr++); // B [3]
  regs.HL.B.l=*(ptr++); // L [4]
  regs.HL.B.h=*(ptr++); // H [5]

  // PC [6+7]
  value1=*(ptr++);
  value2=*(ptr++);
  regs.PC.W=(value2<<8)|value1;
  if(regs.PC.W==0x0000) {
    flag_version=1;
  }
  else {
    flag_version=0;
  }

  // SP [8+9]
  value1=*(ptr++);
  value2=*(ptr++);
  regs.SP.W=(value2<<8)|value1;

  regs.I=*(ptr++); // I [10]
  regs.R=*(ptr++); // R [11]

  // Comressed-Flag & Border [12]
  value1=*(ptr++);
  value2=((value1&0x0E)>>1);
  OutZ80(0xFE, value2); // BorderColor
  if((value1&0x20)!=0) {
    flag_compressed=1;
  } else {
    flag_compressed=0;
  }

  regs.DE.B.l=*(ptr++); // E [13]
  regs.DE.B.h=*(ptr++); // D [14]
  regs.BC1.B.l=*(ptr++); // C1 [15]
  regs.BC1.B.h=*(ptr++); // B1 [16]
  regs.DE1.B.l=*(ptr++); // E1 [17]
  regs.DE1.B.h=*(ptr++); // D1 [18]
  regs.HL1.B.l=*(ptr++); // L1 [19]
  regs.HL1.B.h=*(ptr++); // H1 [20]
  regs.AF1.B.h=*(ptr++); // A1 [21]
  regs.AF1.B.l=*(ptr++); // F1 [22]
  regs.IY.B.l=*(ptr++); // Y [23]
  regs.IY.B.h=*(ptr++); // I [24]
  regs.IX.B.l=*(ptr++); // X [25]
  regs.IX.B.h=*(ptr++); // I [26]

  // Interrupt-Flag [27]
  value1=*(ptr++);
  if(value1!=0) {
    // EI
    regs.IFF|=IFF_2|IFF_EI;
  }
  else {
    // DI
    regs.IFF&=~(IFF_1|IFF_2|IFF_EI);
  }
  value1=*(ptr++); // nc [28]
  // Interrupt-Mode [29]
  value1=*(ptr++);
  if((value1&0x01)!=0) {
    regs.IFF|=IFF_IM1;
  }
  else {
    regs.IFF&=~IFF_IM1;
  }
  if((value1&0x02)!=0) {
    regs.IFF|=IFF_IM2;
  }
  else {
    regs.IFF&=~IFF_IM2;
  }

  // restliche Register
  regs.ICount   = regs.IPeriod;
  regs.IRequest = INT_NONE;
  regs.IBackup  = 0;

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
      while(ptr < (snapshot_buffer + loader::image_size)) {
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
      while(ptr<(snapshot_buffer + loader::image_size)) {
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
    regs.PC.W=(value2<<8)|value1;

    //------------------------
    // 1st block parsing
    //------------------------
    next_block=p_decompFlashBlock(akt_block);
    //------------------------
    // all other parsing
    //------------------------
    while(next_block<snapshot_buffer + loader::image_size) {
      akt_block=next_block;
      next_block=p_decompFlashBlock(akt_block);
    }
  }
  loader::snapshot_type = loader::Snapshot::Z80;
}


void loader::save_image_z80(const Z80& regs)
{
  snapshot_buffer[0] = regs.AF.B.h; // A [0]
  snapshot_buffer[1] = regs.AF.B.l; // F [1]
  snapshot_buffer[2] = regs.BC.B.l; // C [2]
  snapshot_buffer[3] = regs.BC.B.h; // B [3]
  snapshot_buffer[4] = regs.HL.B.l; // L [4]
  snapshot_buffer[5] = regs.HL.B.h; // H [5]

  snapshot_buffer[6] = regs.PC.B.l; // PC
  snapshot_buffer[7] = regs.PC.B.h; //
  snapshot_buffer[8] = regs.SP.B.l; // SP
  snapshot_buffer[9] = regs.SP.B.h; //

  snapshot_buffer[10] = regs.I; // I [10]
  snapshot_buffer[11] = regs.R & 0x7f; // R [11]

  // Comressed-Flag & Border [12]
  snapshot_buffer[12] = ((regs.R & 0x80) >> 7) | (display::bordercolor << 1);

  snapshot_buffer[13] = regs.DE.B.l; // E [13]
  snapshot_buffer[14] = regs.DE.B.h; // D [14]
  snapshot_buffer[15] = regs.BC1.B.l; // C1 [15]
  snapshot_buffer[16] = regs.BC1.B.h; // B1 [16]
  snapshot_buffer[17] = regs.DE1.B.l; // E1 [17]
  snapshot_buffer[18] = regs.DE1.B.h; // D1 [18]
  snapshot_buffer[19] = regs.HL1.B.l; // L1 [19]
  snapshot_buffer[20] = regs.HL1.B.h; // H1 [20]
  snapshot_buffer[21] = regs.AF1.B.h; // A1 [21]
  snapshot_buffer[22] = regs.AF1.B.l; // F1 [22]
  snapshot_buffer[23] = regs.IY.B.l; // Y [23]
  snapshot_buffer[24] = regs.IY.B.h; // I [24]
  snapshot_buffer[25] = regs.IX.B.l; // X [25]
  snapshot_buffer[26] = regs.IX.B.h; // I [26]

  // Interrupt-Flag [27]
  snapshot_buffer[27] = (regs.IFF | (IFF_2|IFF_EI)) ? 1 : 0;

  snapshot_buffer[28] = (regs.IFF & IFF_IM2); // "IFF2 (not particularly important...)"

  // Interrupt-Mode
  snapshot_buffer[29] = ((regs.IFF & IFF_IM1) ? 0x01 : 0) | ((regs.IFF & IFF_IM2) ? 0x02 : 0);

  //-----------------------
  // old Version 1 raw (uncompressed),
  //-----------------------
  for (uint16_t i = 0; i < 0xc000; ++i)
    snapshot_buffer[i + 30] = RdZ80(i + 0x4000);

  loader::snapshot_type = loader::Snapshot::Z80;
  loader::image_size = loader::Z80_LEN;
}



