
//Emulation of C64 memories and memory bus (PLA & MUXes)

#include "C64.h"


extern cRSID_C64instance cRSID_C64;


void cRSID_generateMemoryBankPointers () {
 int i,j;
 for (i=0; i < 4; ++i) {
  for (j=0; j < 256; ++j) {
   if (j < 0xA0) cRSID_C64.MemoryBankPointersRD[i][j] = cRSID_C64.RAMbank;
   else if (0xD0 <= j && j < 0xE0 && i) cRSID_C64.MemoryBankPointersRD[i][j] = cRSID_C64.IObankRD;
   else if ( (j < 0xC0 && i == 3) || (0xE0 <= j && (i&2)) ) cRSID_C64.MemoryBankPointersRD[i][j] = cRSID_C64.ROMbanks;
   else cRSID_C64.MemoryBankPointersRD[i][j] = cRSID_C64.RAMbank;

   if (j < 0xD0 || 0xE0 <= j) cRSID_C64.MemoryBankPointersWR[i][j] = cRSID_C64.RAMbank;
   else if (i) { cRSID_C64.MemoryBankPointersWR[i][j] = cRSID_C64.IObankWR; }
   else cRSID_C64.MemoryBankPointersWR[i][j] = cRSID_C64.RAMbank;
  }
 }
}


static INLINE unsigned char* cRSID_getMemReadPtr (FASTVAR unsigned short address) {
 //cRSID_C64instance* const C64 = &cRSID_C64; //for faster (?) operation we use a global object as memory
 FASTVAR unsigned char * FASTPTR BankPointer = NULL;
 BankPointer = cRSID_C64.MemoryBankPointersRD[ cRSID_C64.RAMbank[1] & 3 ][ address >> 8 ];
 if ( MOSTLY (BankPointer != cRSID_C64.IObankRD) ) return &( BankPointer[ address ] );
 else if ( MOSTLY (address < 0xD400 || 0xD419 <= address) ) return &( BankPointer[ address ] );
 else return &cRSID_C64.IObankWR[address]; //emulate bitfading aka SID-read of last written reg (e.g. Lift Off ROR $D400,x)
 /*if (address<0xA000) return &cRSID_C64.RAMbank[address];
 else if ( 0xD000<=address && address<0xE000 && (cRSID_C64.RAMbank[1]&3) ) {
  if (0xD400 <= address && address < 0xD419) return &cRSID_C64.IObankWR[address]; //emulate bitfading aka SID-read of last written reg (e.g. Lift Off ROR $D400,x)
  return &cRSID_C64.IObankRD[address];
 }
 else if ( (address<0xC000 && (cRSID_C64.RAMbank[1]&3)==3)
           || (0xE000<=address && (cRSID_C64.RAMbank[1]&2)) ) return &cRSID_C64.ROMbanks[address];
 return &cRSID_C64.RAMbank[address];*/
}

//static INLINE unsigned char* cRSID_getMemReadPtrC64 (cRSID_C64instance* C64, FASTVAR unsigned short address) {
// return & ( MemoryBankPointersRD[ cRSID_C64.RAMbank[1] & 3 ][ address >> 8 ] ) [ address ];
 /*if (address<0xA000) return &C64->RAMbank[address];
 else if ( 0xD000<=address && address<0xE000 && (C64->RAMbank[1]&3) ) {
  if (0xD400 <= address && address < 0xD419) return &C64->IObankWR[address]; //emulate peculiar SID-read (e.g. Lift Off)
  return &C64->IObankRD[address];
 }
 else if ( (address<0xC000 && (C64->RAMbank[1]&3)==3)
           || (0xE000<=address && (C64->RAMbank[1]&2)) ) return &C64->ROMbanks[address];
 return &C64->RAMbank[address];*/
//}


static INLINE unsigned char* cRSID_getMemWritePtr (FASTVAR unsigned short address) {
 //cRSID_C64instance* const C64 = &cRSID_C64; //for faster (?) operation we use a global object as memory
 FASTVAR unsigned char * FASTPTR BankPointer;
 BankPointer = cRSID_C64.MemoryBankPointersWR[ cRSID_C64.RAMbank[1] & 3 ][ address >> 8 ];
 if ( MOSTLY (BankPointer != cRSID_C64.IObankWR) ) return &( BankPointer[ address ] );
 else if ( RARELY (0xD420 <= address && address < 0xD800) ) { //CIA/VIC mirrors needed?
  if ( LIKELY ( !(cRSID.PSIDdigiMode && 0xD418 <= address && address < 0xD500)
        && !(cRSID_C64.SID[2].BaseAddress <= address && address < cRSID_C64.SID[2].BaseAddress+0x20)
        && !(cRSID_C64.SID[3].BaseAddress <= address && address < cRSID_C64.SID[3].BaseAddress+0x20)
        && !(cRSID_C64.SID[4].BaseAddress <= address && address < cRSID_C64.SID[4].BaseAddress+0x20) ) ) {
   return &cRSID_C64.IObankWR[ 0xD400 + (address&0x1F) ]; //write to $D400..D41F if not in SID2/SID3 address-space
  }
 }
 return &( BankPointer[ address ] );
 /*if (address<0xD000 || 0xE000<=address) return &cRSID_C64.RAMbank[address];
 else if ( cRSID_C64.RAMbank[1]&3 ) { //handle SID-mirrors! (CJ in the USA workaround (writing above $d420, except SID2/SID3))
  if (0xD420 <= address && address < 0xD800) { //CIA/VIC mirrors needed?
   if ( !(cRSID_C64.PSIDdigiMode && 0xD418 <= address && address < 0xD500)
        && !(cRSID_C64.SID[2].BaseAddress <= address && address < cRSID_C64.SID[2].BaseAddress+0x20)
        && !(cRSID_C64.SID[3].BaseAddress <= address && address < cRSID_C64.SID[3].BaseAddress+0x20)
        && !(cRSID_C64.SID[4].BaseAddress <= address && address < cRSID_C64.SID[4].BaseAddress+0x20) ) {
    return &cRSID_C64.IObankWR[ 0xD400 + (address&0x1F) ]; //write to $D400..D41F if not in SID2/SID3 address-space
   }
   else return &cRSID_C64.IObankWR[address];
  }
  else return &cRSID_C64.IObankWR[address];
 }
 return &cRSID_C64.RAMbank[address];*/
}


//static INLINE unsigned char* cRSID_getMemWritePtrC64 (cRSID_C64instance* C64, FASTVAR unsigned short address) {
// return & ( MemoryBankPointersWR[ cRSID_C64.RAMbank[1] & 3 ][ address >> 8 ] ) [ address ];
 /*if (address<0xD000 || 0xE000<=address) return &C64->RAMbank[address];
 else if ( C64->RAMbank[1]&3 ) { //handle SID-mirrors! (CJ in the USA workaround (writing above $d420, except SID2/SID3/PSIDdigi))
  if (0xD420 <= address && address < 0xD800) { //CIA/VIC mirrors needed?
   if ( !(cRSID_C64.PSIDdigiMode && 0xD418 <= address && address < 0xD500)
        && !(C64->SID[2].BaseAddress <= address && address < C64->SID[2].BaseAddress+0x20)
        && !(C64->SID[3].BaseAddress <= address && address < C64->SID[3].BaseAddress+0x20)
        && !(C64->SID[4].BaseAddress <= address && address < C64->SID[4].BaseAddress+0x20) ) {
    return &C64->IObankWR[ 0xD400 + (address&0x1F) ]; //write to $D400..D41F if not in SID2/SID3 address-space
   }
   else return &C64->IObankWR[address];
  }
  else return &C64->IObankWR[address];
 }
 return &C64->RAMbank[address];*/
//}


static INLINE unsigned char cRSID_readMem (FASTVAR unsigned short address) {
 return *cRSID_getMemReadPtr(address);
}

/*static INLINE unsigned char cRSID_readMemC64 (cRSID_C64instance* C64, FASTVAR unsigned short address) {
 return *cRSID_getMemReadPtrC64(C64,address);
}*/


static INLINE void cRSID_writeMem (FASTVAR unsigned short address, FASTVAR unsigned char data) {
 *cRSID_getMemWritePtr(address)=data;
}

/*static INLINE void cRSID_writeMemC64 (cRSID_C64instance* C64, FASTVAR unsigned short address, FASTVAR unsigned char data) {
 *cRSID_getMemWritePtrC64(C64,address)=data;
}*/


void cRSID_setROMcontent () { //fill KERNAL/BASIC-ROM areas with content needed for SID-playback
 //static cRSID_C64instance* C64 = &cRSID_C64;
 int i;
 static const unsigned char ROM_IRQreturnCode[9] = {0xAD,0x0D,0xDC,0x68,0xA8,0x68,0xAA,0x68,0x40}; //CIA1-acknowledge IRQ-return
 static const unsigned char ROM_NMIstartCode[5] = {0x78,0x6c,0x18,0x03,0x40}; //SEI and jmp($0318)
 static const unsigned char ROM_IRQBRKstartCode[19] = { //Full IRQ-return (handling BRK with the same RAM vector as IRQ)
  0x48,0x8A,0x48,0x98,0x48,0xBA,0xBD,0x04,0x01,0x29,0x10,0xEA,0xEA,0xEA,0xEA,0xEA,0x6C,0x14,0x03
 };

 for (i=0xA000; i<0x10000; ++i) cRSID_C64.ROMbanks[i] = 0x60; //RTS (at least return if some unsupported call is made to ROM)

 if (cRSID.BASICfileData != NULL) { for (i = 0; i < 0x2000; ++i) cRSID_C64.ROMbanks[0xA000+i] = cRSID.BASICfileData[i]; }

 if (cRSID.KERNALfileData != NULL) { for (i = 0; i < 0x2000; ++i) cRSID_C64.ROMbanks[0xE000+i] = cRSID.KERNALfileData[i]; }   //for (i=0; i<sizeof(KERNAL); ++i) cRSID_C64.ROMbanks[0xE000+i] = KERNAL[i];
 else {
  for (i=0xEA31; i<0xEA7E; ++i) cRSID_C64.ROMbanks[i] = 0xEA; //NOP (full IRQ-return leading to simple IRQ-return without other tasks)
  for (i=0; i<9; ++i) cRSID_C64.ROMbanks [0xEA7E + i] = ROM_IRQreturnCode[i];
  for (i=0; i<4; ++i) cRSID_C64.ROMbanks [0xFE43 + i] = ROM_NMIstartCode[i];
  for (i=0; i<19; ++i) cRSID_C64.ROMbanks[0xFF48 + i] = ROM_IRQBRKstartCode[i];
  cRSID_C64.ROMbanks[0xFFFB] = 0xFE; cRSID_C64.ROMbanks[0xFFFA] = 0x43; //ROM NMI-vector
  cRSID_C64.ROMbanks[0xFFFF] = 0xFF; cRSID_C64.ROMbanks[0xFFFE] = 0x48; //ROM IRQ-vector
 }

 //copy KERNAL & BASIC ROM contents into the RAM under them? (So PSIDs that don't select bank correctly will work better.)
 for (i=0xA000; i<0x10000; ++i) cRSID_C64.RAMbank[i]=cRSID_C64.ROMbanks[i];
}


void cRSID_initMem () { //set default values that normally KERNEL ensures after startup/reset (only SID-playback related)
 //static cRSID_C64instance* C64 = &cRSID_C64;
 static int i;

 //data required by both PSID and RSID (according to HVSC SID_file_format.txt):
 cRSID_writeMem( 0x02A6, cRSID.VideoStandard  ); //cRSID_writeMemC64( C64, 0x02A6, cRSID_C64.VideoStandard  ); //$02A6 should be pre-set to: 0:NTSC / 1:PAL
 cRSID_writeMem( 0x0001, 0x37 ); //cRSID_writeMemC64( C64, 0x0001, 0x37 ); //initialize bank-reg. (ROM-banks and IO enabled)

 //if (cRSID_C64.ROMbanks[0xE000]==0) { //wasn't a KERNAL-ROM loaded? (e.g. PSID)
  cRSID_writeMem( 0x00CB, 0x40 ); //cRSID_writeMemC64( C64, 0x00CB, 0x40 ); //Some tunes might check for keypress here (e.g. Master Blaster Intro)
  //if(cRSID.RealSIDmode) {
   cRSID_writeMem( 0x0315, 0xEA ); cRSID_writeMem( 0x0314, 0x31 ); //cRSID_writeMemC64( C64, 0x0315, 0xEA ); cRSID_writeMemC64( C64, 0x0314, 0x31 ); //IRQ
   cRSID_writeMem( 0x0319, 0xEA/*0xFE*/ ); cRSID_writeMem( 0x0318, 0x81/*0x47*/ ); //cRSID_writeMemC64( C64, 0x0319, 0xEA/*0xFE*/ ); cRSID_writeMemC64( C64, 0x0318, 0x81/*0x47*/ ); //NMI
  //}

  for (i=0xD000; i<0xD7FF; ++i) cRSID_C64.IObankRD[i] = cRSID_C64.IObankWR[i] = 0; //initialize the whole IO area for a known base-state
  if(cRSID.RealSIDmode) {cRSID_C64.IObankWR[0xD012] = 0x37; cRSID_C64.IObankWR[0xD011] = 0x8B;} //else cRSID_C64.IObankWR[0xD012] = 0;
  //cRSID_C64.IObankWR[0xD019] = 0; //PSID: rasterrow: any value <= $FF, IRQ:enable later if there is VIC-timingsource

  cRSID_C64.IObankRD[0xDC00]=0x10; cRSID_C64.IObankRD[0xDC01]=0xFF; //Imitate CIA1 keyboard/joy port, some tunes check if buttons are not pressed
  if (cRSID.VideoStandard) { cRSID_C64.IObankWR[0xDC04]=0x24; cRSID_C64.IObankWR[0xDC05]=0x40; } //initialize CIAs
  else { cRSID_C64.IObankWR[0xDC04]=0x95; cRSID_C64.IObankWR[0xDC05]=0x42; }
  if (cRSID.RealSIDmode) cRSID_C64.IObankWR[0xDC0D] = 0x81; //Reset-default, but for PSID CIA1 TimerA IRQ should be enabled anyway if SID is CIA-timed
  cRSID_C64.IObankWR[0xDC0E] = 0x01; //some tunes (and PSID doc) expect already running CIA (Reset-default)
  cRSID_C64.IObankWR[0xDC0F] = 0x00; //All counters other than CIA1 TimerA should be disabled and set to 0xFF for PSID:
  cRSID_C64.IObankWR[0xDD00] = cRSID_C64.IObankRD[0xDD00] = 0x03; //VICbank-selector default
  cRSID_C64.IObankWR[0xDD04] = cRSID_C64.IObankWR[0xDD05] = 0xFF;
  //cRSID_C64.IObankWR[0xDD0E] = cRSID_C64.IObank[0xDD0F] = 0x08;
 //}

}

