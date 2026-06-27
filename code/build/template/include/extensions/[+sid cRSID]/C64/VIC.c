
//VIC-II emulation


void cRSID_createVICchip (unsigned short baseaddress) {
 //static cRSID_C64instance* C64 = &cRSID_C64;

 //VIC->C64 = C64;
 cRSID_C64.VIC.ChipModel = 0;
 cRSID_C64.VIC.BaseAddress = baseaddress;
 cRSID_C64.VIC.BasePtrWR = &cRSID_C64.IObankWR[baseaddress]; cRSID_C64.VIC.BasePtrRD = &cRSID_C64.IObankRD[baseaddress];
 cRSID_initVICchip();
}


void cRSID_initVICchip () {
 short i; //unsigned char i;  //compilers sometimes doesn't like 'char' as index
 for (i=0; i<0x3F; ++i) cRSID_C64.VIC.BasePtrWR[i] = cRSID_C64.VIC.BasePtrRD[i] = 0x00;
 cRSID_C64.VIC.RowCycleCnt=0;
}


static INLINE char cRSID_emulateVIC (FASTVAR char cycles) {

 FASTVAR unsigned short RasterRow;

 enum VICregisters {
  CONTROL = 0x11, RASTERROWL = 0x12, SPRITE_ENABLE=0x15, //CHARSET_VRAM_BANK = 0x18,
  INTERRUPT = 0x19, INTERRUPT_ENABLE = 0x1A, //SPRITE_SCALEX = 0x1D, BORDER_COLOR = 0x20, SPRITE0_COLOR = 0x27
 };

 enum ControlBitVal { RASTERROWMSB = 0x80, DISPLAY_ENABLE = 0x10, ROWS = 0x08, YSCROLL_MASK = 0x07 };

 enum InterruptBitVal { VIC_IRQ = 0x80, RASTERROW_MATCH_IRQ = 0x01 };


 cRSID_C64.VIC.RowCycleCnt += cycles;
 if ( RARELY (cRSID_C64.VIC.RowCycleCnt >= cRSID_C64.VIC.RasterRowCycles) ) {
  cRSID_C64.VIC.RowCycleCnt -= cRSID_C64.VIC.RasterRowCycles;

  RasterRow = ( (cRSID_C64.VIC.BasePtrRD[CONTROL]&RASTERROWMSB) << 1 ) + cRSID_C64.VIC.BasePtrRD[RASTERROWL];
  ++RasterRow; if ( RARELY (RasterRow >= cRSID_C64.VIC.RasterLines) ) RasterRow = 0;
  cRSID_C64.VIC.BasePtrRD[CONTROL] = ( cRSID_C64.VIC.BasePtrRD[CONTROL] & ~RASTERROWMSB ) | ((RasterRow&0x100)>>1);
  cRSID_C64.VIC.BasePtrRD[RASTERROWL] = RasterRow & 0xFF;

  if (cRSID_C64.VIC.BasePtrWR[INTERRUPT_ENABLE] & RASTERROW_MATCH_IRQ) {
   if ( RARELY (RasterRow == ( (cRSID_C64.VIC.BasePtrWR[CONTROL]&RASTERROWMSB) << 1 ) + cRSID_C64.VIC.BasePtrWR[RASTERROWL]) ) {
    cRSID_C64.VIC.BasePtrRD[INTERRUPT] |= VIC_IRQ | RASTERROW_MATCH_IRQ;
   }
  }

 }

 return cRSID_C64.VIC.BasePtrRD[INTERRUPT] & VIC_IRQ;
}


static INLINE void cRSID_acknowledgeVICrasterIRQ () {
 enum VICregisters { INTERRUPT = 0x19 };
 enum InterruptBitVal { VIC_IRQ = 0x80, RASTERROW_MATCH_IRQ = 0x01 };
 //An 1 is to be written into the IRQ-flag (bit0) of $d019 to clear it and deassert IRQ signal
 //if (cRSID_C64.VIC.BasePtrWR[INTERRUPT] & RASTERROW_MATCH_IRQ) { //acknowledge raster-interrupt by writing to $d019 bit0?
 //But oftentimes INC/LSR/etc. RMW commands are used to acknowledge VIC IRQ, they work on real
 //CPU because it writes the unmodified original value itself to memory before writing the modified there
  cRSID_C64.VIC.BasePtrWR[INTERRUPT] &= ~RASTERROW_MATCH_IRQ; //prepare for next acknowledge-detection
  cRSID_C64.VIC.BasePtrRD[INTERRUPT] &= ~(VIC_IRQ | RASTERROW_MATCH_IRQ); //remove IRQ flag and state
 //}
}

