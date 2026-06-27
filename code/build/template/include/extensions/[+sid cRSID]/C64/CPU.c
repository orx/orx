
//cRSID CPU-emulation


enum cRSID_StatusFlagBitValues { N=0x80, V=0x40, B=0x10, D=0x08, I=0x04, Z=0x02, C=0x01 };


//static cRSID_C64instance* const C64 = &cRSID_C64; //could be a parameter but function-call is faster this way if only 1 main CPU exists
//static cRSID_CPUinstance* const CPU = &cRSID_C64.CPU; //&C64->CPU;

//#define PC cRSID_C64.CPU.PC  //static unsigned int PC;
//static unsigned char ST, X, Y;
//static short int A, SP;


void cRSID_initCPU (unsigned short mempos) {
 cRSID_C64.CPU.PC = mempos; cRSID_C64.CPU.A = 0; cRSID_C64.CPU.X = 0; cRSID_C64.CPU.Y = 0;
 cRSID_C64.CPU.ST = 0x04; cRSID_C64.CPU.SP = 0xFF; cRSID_C64.CPU.PrevNMI = 0;
}



/*static INLINE void loadReg (void) {
 PC = cRSID_C64.CPU.PC; SP = cRSID_C64.CPU.SP; ST = cRSID_C64.CPU.ST; A = cRSID_C64.CPU.A; X = cRSID_C64.CPU.X; Y = cRSID_C64.CPU.Y;
}
static INLINE void storeReg (void) {
 cRSID_C64.CPU.PC = PC; cRSID_C64.CPU.SP = SP; cRSID_C64.CPU.ST = ST; cRSID_C64.CPU.A = A; cRSID_C64.CPU.X = X; cRSID_C64.CPU.Y = Y;
}*/

static INLINE unsigned char rd (FASTVAR unsigned short address) {
 FASTVAR unsigned char value;
 value = *cRSID_getMemReadPtr(address);
 if ( TIGHTLY (cRSID_C64.RealSIDmode) ) {
  if ( LIKELY (cRSID_C64.RAMbank[1] & 3) ) {
   if ( RARELY (address==0xDC0D) ) { cRSID_acknowledgeCIAIRQ( &cRSID_C64.CIA[1] ); }
   else if ( RARELY (address==0xDD0D) ) { cRSID_acknowledgeCIAIRQ( &cRSID_C64.CIA[2] ); }
  }
 }
 return value;
}

static INLINE void wr (FASTVAR unsigned short address, FASTVAR unsigned char data) {
 *cRSID_getMemWritePtr(address)=data;
 if ( LIKELY (cRSID_C64.RealSIDmode && (cRSID_C64.RAMbank[1] & 3)) ) {
  //if(data&1) { //only writing 1 to $d019 bit0 would acknowledge, not any value (but RMW instructions write $d019 back before mod.)
   if ( RARELY (address==0xD019) ) { cRSID_acknowledgeVICrasterIRQ(); }
  //}
 }
}

static INLINE void wr2 (FASTVAR unsigned short address, FASTVAR unsigned char data) { //PSID-hack specific memory-write
 FASTVAR int Tmp;
 *cRSID_getMemWritePtr(address)=data;
 if ( LIKELY (cRSID_C64.RAMbank[1] & 3) ) {

  if ( TIGHTLY (cRSID_C64.RealSIDmode) ) {
   /*if (address<0xdc00 && 0xd800 <= address) cRSID_C64.IObankRD[address] = cRSID_C64.IObankWR[address];
   else*/ if ( RARELY ( (address & 0xFE00) == 0xDC00 ) ) {
    switch (address) {
     case 0xDC0D: cRSID_writeCIAIRQmask( &cRSID_C64.CIA[1], data ); break;
     case 0xDD0D: cRSID_writeCIAIRQmask( &cRSID_C64.CIA[2], data ); break;
     case 0xDC0C: cRSID_C64.IObankRD[address]=data;  break; //mirror WR to RD (e.g. if byte at DC0C is used as RTI)
     case 0xDD0C: cRSID_C64.IObankRD[address]=data;  break; //mirror WR to RD (e.g. Wonderland_XIII_tune_1.sid)
     case 0xDC05: cRSID_writeCIAlatchAhi( &cRSID_C64.CIA[1], data ); break;
     case 0xDC07: cRSID_writeCIAlatchBhi( &cRSID_C64.CIA[1], data ); break;
     case 0xDD05: cRSID_writeCIAlatchAhi( &cRSID_C64.CIA[2], data ); break;
     case 0xDD07: cRSID_writeCIAlatchBhi( &cRSID_C64.CIA[2], data ); break;
    }
   }
   //#ifdef CRSID_PLATFORM_PC //just for info displayer
   // else if (address==0xDC05 || address==0xDC04) cRSID_C64.FrameCycles = ( (cRSID_C64.IObankWR[0xDC04] + (cRSID_C64.IObankWR[0xDC05]<<8)) );
   //#endif
   else if( RARELY (address==0xD019 && data&1) ) { //only writing 1 to $d019 bit0 would acknowledge
    cRSID_acknowledgeVICrasterIRQ();
   }
  }

  else { //PSID-mode
   switch (address) {
    case 0xDC05: case 0xDC04:
     if ( RARELY (cRSID.TimerSource) ) { //dynamic CIA-setting (Galway/Rubicon workaround)
      cRSID.FrameCycles = ( (cRSID_C64.IObankWR[0xDC04] + (cRSID_C64.IObankWR[0xDC05]<<8)) ); //<< CRSID_CLOCK_FRACTIONAL_BITS) / cRSID_C64.SampleClockRatio;
     }
     break;
    case 0xDC08: cRSID_C64.IObankRD[0xDC08] = data; break; //refresh TOD-clock
    case 0xDC09: cRSID_C64.IObankRD[0xDC09] = data; break; //refresh TOD-clock
    case 0xD012: //dynamic VIC IRQ-rasterline setting (Microprose Soccer V1 workaround)
     if (cRSID_C64.PrevRasterLine >= 0) { //was $d012 set before? (or set only once?)
      if (cRSID_C64.IObankWR[0xD012] != cRSID_C64.PrevRasterLine) {
       Tmp = cRSID_C64.IObankWR[0xD012] - cRSID_C64.PrevRasterLine;
       if (Tmp<0) Tmp += cRSID_C64.VIC.RasterLines;
       cRSID_C64.FrameCycleCnt = cRSID.FrameCycles - Tmp * cRSID_C64.VIC.RasterRowCycles;
      }
     }
     cRSID_C64.PrevRasterLine = cRSID_C64.IObankWR[0xD012];
     break;
   }
  }

 }
}


static INLINE void addrModeImmediate (void)
{ ++cRSID_C64.CPU.PC; cRSID_C64.CPU.Addr = cRSID_C64.CPU.PC; cRSID_C64.CPU.Cycles=2; } //imm.

static INLINE void addrModeZeropage (void)
{ ++cRSID_C64.CPU.PC; cRSID_C64.CPU.Addr = rd(cRSID_C64.CPU.PC); cRSID_C64.CPU.Cycles=3; } //zp

static INLINE void addrModeAbsolute (void) {
 ++cRSID_C64.CPU.PC; cRSID_C64.CPU.Addr = rd(cRSID_C64.CPU.PC);
 ++cRSID_C64.CPU.PC; cRSID_C64.CPU.Addr += rd(cRSID_C64.CPU.PC)<<8; cRSID_C64.CPU.Cycles=4;
} //abs

static INLINE void addrModeZeropageXindexed (void)
{ ++cRSID_C64.CPU.PC; cRSID_C64.CPU.Addr = (rd(cRSID_C64.CPU.PC) + cRSID_C64.CPU.X) & 0xFF; cRSID_C64.CPU.Cycles=4; } //zp,x (with zeropage-wraparound of 6502)

static INLINE void addrModeZeropageYindexed (void)
{ ++cRSID_C64.CPU.PC; cRSID_C64.CPU.Addr = (rd(cRSID_C64.CPU.PC) + cRSID_C64.CPU.Y) & 0xFF; cRSID_C64.CPU.Cycles=4; } //zp,y (with zeropage-wraparound of 6502)

static INLINE void addrModeXindexed (void) { // abs,x (only STA is 5 cycles, others are 4 if page not crossed, RMW:7)
 ++cRSID_C64.CPU.PC; cRSID_C64.CPU.Addr = rd(cRSID_C64.CPU.PC) + cRSID_C64.CPU.X;
 ++cRSID_C64.CPU.PC; cRSID_C64.CPU.SamePage = (cRSID_C64.CPU.Addr <= 0xFF); cRSID_C64.CPU.Addr += rd(cRSID_C64.CPU.PC)<<8;
 cRSID_C64.CPU.Cycles = 5;
}

static INLINE void addrModeYindexed (void) { // abs,y (only STA is 5 cycles, others are 4 if page not crossed, RMW:7)
 ++cRSID_C64.CPU.PC; cRSID_C64.CPU.Addr = rd(cRSID_C64.CPU.PC) + cRSID_C64.CPU.Y; ++cRSID_C64.CPU.PC;
 cRSID_C64.CPU.SamePage = (cRSID_C64.CPU.Addr <= 0xFF); cRSID_C64.CPU.Addr += rd(cRSID_C64.CPU.PC)<<8; cRSID_C64.CPU.Cycles=5;
}

static INLINE void addrModeIndirectYindexed (void) { // (zp),y (only STA is 6 cycles, others are 5 if page not crossed, RMW:8)
 ++cRSID_C64.CPU.PC; cRSID_C64.CPU.Addr = rd(rd(cRSID_C64.CPU.PC)) + cRSID_C64.CPU.Y;
 cRSID_C64.CPU.SamePage = (cRSID_C64.CPU.Addr <= 0xFF); cRSID_C64.CPU.Addr += rd( (rd(cRSID_C64.CPU.PC)+1)&0xFF ) << 8;
 cRSID_C64.CPU.Cycles = 6;
}

static INLINE void addrModeXindexedIndirect (void) { // (zp,x)
 ++cRSID_C64.CPU.PC;
 cRSID_C64.CPU.Addr = ( rd( rd(cRSID_C64.CPU.PC) + cRSID_C64.CPU.X ) & 0xFF )
                      + ( ( rd( rd(cRSID_C64.CPU.PC) + cRSID_C64.CPU.X + 1 ) & 0xFF ) << 8 );
 cRSID_C64.CPU.Cycles = 6;
}


static INLINE void clrC (void) { cRSID_C64.CPU.ST &= ~C; } //clear Carry-flag
static INLINE void setC (FASTVAR unsigned char expr)
{ cRSID_C64.CPU.ST &= ~C; cRSID_C64.CPU.ST |= (expr!=0); } //set Carry-flag if expression is not zero, else clear it

static INLINE void clrNZC (void)
{ cRSID_C64.CPU.ST &= ~(N|Z|C); } //clear flags

static INLINE void clrNVZC (void)
{ cRSID_C64.CPU.ST &= ~(N|V|Z|C); } //clear flags

static INLINE void setNZbyA (void)
{ cRSID_C64.CPU.ST &= ~(N|Z); cRSID_C64.CPU.ST |= ((!cRSID_C64.CPU.A)<<1) | (cRSID_C64.CPU.A&N); } //set Negative-flag and Zero-flag based on result in Accumulator

static INLINE void setNZbyT (FASTVAR short int t)
{ t&=0xFF; cRSID_C64.CPU.ST &= ~(N|Z); cRSID_C64.CPU.ST |= ((!t)<<1) | (t&N); }

static INLINE void setNZbyX (void)
{ cRSID_C64.CPU.ST &= ~(N|Z); cRSID_C64.CPU.ST |= ((!cRSID_C64.CPU.X)<<1) | (cRSID_C64.CPU.X&N); } //set Negative-flag and Zero-flag based on result in X-register

static INLINE void setNZbyY (void)
{ cRSID_C64.CPU.ST &= ~(N|Z); cRSID_C64.CPU.ST |= ((!cRSID_C64.CPU.Y)<<1) | (cRSID_C64.CPU.Y&N); } //set Negative-flag and Zero-flag based on result in Y-register

static INLINE void setNZbyM (void)
{ cRSID_C64.CPU.ST &= ~(N|Z); cRSID_C64.CPU.ST |= ((!rd(cRSID_C64.CPU.Addr))<<1) | (rd(cRSID_C64.CPU.Addr)&N); } //set Negative-flag and Zero-flag based on result at Memory-Address

static INLINE void setNZCbyAdd (void)
{ cRSID_C64.CPU.ST &= ~(N|Z|C); cRSID_C64.CPU.ST |= (cRSID_C64.CPU.A&N)|(cRSID_C64.CPU.A>255); cRSID_C64.CPU.A&=0xFF; cRSID_C64.CPU.ST|=(!cRSID_C64.CPU.A)<<1; } //after increase/addition

static INLINE void setVbyAdd (FASTVAR unsigned char m, FASTVAR short int t) {
 cRSID_C64.CPU.ST &= ~V; cRSID_C64.CPU.ST |= ( (~(t^m)) & (t^cRSID_C64.CPU.A) & N ) >> 1;
} //calculate V-flag from A and T (previous A) and input2 (Memory)

static INLINE void setNZCbySub (FASTVAR signed short t)
{ cRSID_C64.CPU.ST &= ~(N|Z|C); cRSID_C64.CPU.ST |= (t&N) | (t>=0); /*t&=0xFF;*/ cRSID_C64.CPU.ST |= ((!(t&0xFF))<<1); }

static INLINE void push (FASTVAR unsigned char value)
{ cRSID_C64.RAMbank[0x100+cRSID_C64.CPU.SP] = value; --cRSID_C64.CPU.SP; cRSID_C64.CPU.SP&=0xFF; } //push a value to stack

static INLINE unsigned char pop (void)
{ ++cRSID_C64.CPU.SP; cRSID_C64.CPU.SP&=0xFF; return cRSID_C64.RAMbank[0x100+cRSID_C64.CPU.SP]; } //pop a value from stack




unsigned char cRSID_emulateCPU (void) { //the CPU emulation for SID/PRG playback (ToDo: CIA/VIC-IRQ/NMI/RESET vectors, BCD-mode)

 //enum cRSID_StatusFlagBitValues { N=0x80, V=0x40, B=0x10, D=0x08, I=0x04, Z=0x02, C=0x01 };

 static const unsigned char FlagSwitches[] = { 0x01, 0x21, 0x04, 0x24, 0x00, 0x40, 0x08, 0x28 },
                            BranchFlags[] = { 0x80, 0x40, 0x01, 0x02 };

 //static signed char /*Cycles,*/ SamePage;

 FASTVAR unsigned int /*Addr,*/ PrevPC;
 FASTVAR unsigned char IR;
 FASTVAR short int T;


 //loadReg();
 PrevPC = cRSID_C64.CPU.PC;    //if (cRSID_C64.CPU.PC==cRSID.InitAddress) printf ( "Init:$%4.4X, A:$%2.2X\n", cRSID.InitAddress, cRSID_C64.CPU.A );
 IR = rd(cRSID_C64.CPU.PC); cRSID_C64.CPU.Cycles=2; cRSID_C64.CPU.SamePage=0; //'Cycles': ensure smallest 6510 instruction runtime (for implied/register addressing-modes)


 if (IR&1) {  //nybble2:  1/5/9/D:accu.instructions, 3/7/B/F:illegal opcodes

  switch ( (IR & 0x1F) >> 1 ) { //value-forming to cause jump-table //PC wraparound not handled inside to save codespace
   case    0:  case    1: addrModeXindexedIndirect(); break; //(zp,x)
   case    2:  case    3: addrModeZeropage(); break;
   case    4:  case    5: addrModeImmediate(); break;
   case    6:  case    7: addrModeAbsolute(); break;
   case    8:  case    9: addrModeIndirectYindexed(); break; //(zp),y (5..6 cycles, 8 for R-M-W)
   case  0xA:             addrModeZeropageXindexed(); break; //zp,x
   case  0xB:             if ((IR&0xC0)!=0x80) addrModeZeropageXindexed(); //zp,x for illegal opcodes
                          else addrModeZeropageYindexed(); //zp,y for LAX/SAX illegal opcodes
                          break;
   case  0xC:  case  0xD: addrModeYindexed(); break;
   case  0xE:             addrModeXindexed(); break;
   case  0xF:             if ((IR&0xC0)!=0x80) addrModeXindexed(); //abs,x for illegal opcodes
                          else addrModeYindexed(); //abs,y for LAX/SAX illegal opcodes
                          break;
  }
  cRSID_C64.CPU.Addr &= 0xFFFF;

  switch ( (IR & 0xE0) >> 5 ) { //value-forming to cause gapless case-values and faster jump-table creation from switch-case

   case 0: if ( MOSTLY ((IR&0x1F) != 0xB) ) { //ORA / SLO(ASO)=ASL+ORA
            if ( RARELY ((IR&3) == 3) ) { clrNZC(); setC(rd(cRSID_C64.CPU.Addr)>=N); wr( cRSID_C64.CPU.Addr, rd(cRSID_C64.CPU.Addr)<<1 ); cRSID_C64.CPU.Cycles+=2; } //for SLO
            else cRSID_C64.CPU.Cycles -= cRSID_C64.CPU.SamePage;
            cRSID_C64.CPU.A |= rd(cRSID_C64.CPU.Addr); setNZbyA(); //ORA
           }
           else { cRSID_C64.CPU.A &= rd(cRSID_C64.CPU.Addr); setNZbyA(); setC( cRSID_C64.CPU.A >= N ); } //ANC (AND+Carry=bit7)
           break;

   case 1: if ( MOSTLY ((IR&0x1F) != 0xB) ) { //AND / RLA (ROL+AND)
            if ( RARELY ((IR&3) == 3) ) { //for RLA
             T = (rd(cRSID_C64.CPU.Addr)<<1) + (cRSID_C64.CPU.ST&C); clrNZC(); setC(T>255); T&=0xFF; wr(cRSID_C64.CPU.Addr,T); cRSID_C64.CPU.Cycles+=2;
            }
            else cRSID_C64.CPU.Cycles -= cRSID_C64.CPU.SamePage;
            cRSID_C64.CPU.A &= rd(cRSID_C64.CPU.Addr); setNZbyA(); //AND
           }
           else { cRSID_C64.CPU.A &= rd(cRSID_C64.CPU.Addr); setNZbyA(); setC( cRSID_C64.CPU.A >= N ); } //ANC (AND+Carry=bit7)
           break;

   case 2: if ( MOSTLY ((IR&0x1F) != 0xB) ) { //EOR / SRE(LSE)=LSR+EOR
            if ( RARELY ((IR&3) == 3) ) { clrNZC(); setC(rd(cRSID_C64.CPU.Addr)&1); wr(cRSID_C64.CPU.Addr,rd(cRSID_C64.CPU.Addr)>>1); cRSID_C64.CPU.Cycles+=2; } //for SRE
            else cRSID_C64.CPU.Cycles -= cRSID_C64.CPU.SamePage;
            cRSID_C64.CPU.A ^= rd(cRSID_C64.CPU.Addr); setNZbyA(); //EOR
           }
           else { cRSID_C64.CPU.A &= rd(cRSID_C64.CPU.Addr); setC( cRSID_C64.CPU.A & 1 ); cRSID_C64.CPU.A >>= 1; cRSID_C64.CPU.A &= 0xFF; setNZbyA(); } //ALR(ASR)=(AND+LSR)
           break;

   case 3: if ( MOSTLY ((IR&0x1F) != 0xB) ) { //RRA (ROR+ADC) / ADC
            if( RARELY ((IR&3) == 3) ) { //for RRA
             T = (rd(cRSID_C64.CPU.Addr)>>1) + ((cRSID_C64.CPU.ST&C)<<7); clrNZC(); setC(T&1); wr(cRSID_C64.CPU.Addr,T); cRSID_C64.CPU.Cycles+=2;
            }
            else cRSID_C64.CPU.Cycles -= cRSID_C64.CPU.SamePage;
            T = cRSID_C64.CPU.A; cRSID_C64.CPU.A += rd(cRSID_C64.CPU.Addr) + (cRSID_C64.CPU.ST & C);
            if ( RARELY ((cRSID_C64.CPU.ST & D) && (cRSID_C64.CPU.A&0xF)>9) ) { cRSID_C64.CPU.A+=0x10; cRSID_C64.CPU.A&=0xF0; } //BCD?
            setNZCbyAdd(); setVbyAdd(rd(cRSID_C64.CPU.Addr),T); //ADC
           }
           else { // ARR (AND+ROR, bit0 not going to C, but C and bit7 get exchanged.)
            cRSID_C64.CPU.A &= rd(cRSID_C64.CPU.Addr); //T = cRSID_C64.CPU.A; //T = cRSID_C64.CPU.A + rd(cRSID_C64.CPU.Addr) + (cRSID_C64.CPU.ST & C);
            //cRSID_C64.CPU.ST&=~V; cRSID_C64.CPU.ST |= ((T&N)>>1)^(T&V); //setVbyAdd(rd(cRSID_C64.CPU.Addr),T); //V-flag set by intermediate ADC mechanism: (A&mem)+mem ?!
            T = cRSID_C64.CPU.A; cRSID_C64.CPU.A = (cRSID_C64.CPU.A>>1) + ((cRSID_C64.CPU.ST&C)<<7); setC(T>=N); setNZbyA();
            cRSID_C64.CPU.ST &= ~V; cRSID_C64.CPU.ST |= (T & V) ^ (cRSID_C64.CPU.A & V); //corrected: V is set accoring to whether rotate changes Accu bit 6: Tbit6^Abit6)
           }
           break;

   case 4: if ( RARELY ((IR&0x1F) == 0xB) ) { cRSID_C64.CPU.A = cRSID_C64.CPU.X & rd(cRSID_C64.CPU.Addr); setNZbyA(); } //XAA (TXA+AND), highly unstable on real 6502!
           else if ( RARELY ((IR&0x1F) == 0x1B) ) { cRSID_C64.CPU.SP = cRSID_C64.CPU.A & cRSID_C64.CPU.X; wr( cRSID_C64.CPU.Addr, cRSID_C64.CPU.SP & ((cRSID_C64.CPU.Addr>>8)+1) ); } //TAS(SHS) (SP=A&X, mem=S&H} - unstable on real 6502
           else { wr2( cRSID_C64.CPU.Addr, cRSID_C64.CPU.A & (RARELY((IR&3)==3)? cRSID_C64.CPU.X:0xFF) ); } //STA / SAX (at times same as AHX/SHX/SHY) (illegal)
           break;

   case 5: if ( MOSTLY ((IR&0x1F) != 0x1B) ) { cRSID_C64.CPU.A=rd(cRSID_C64.CPU.Addr); if(RARELY((IR&3)==3)) cRSID_C64.CPU.X=cRSID_C64.CPU.A; } //LDA / LAX (illegal, used by my 1 rasterline player) (LAX #imm is unstable on C64)
           else { cRSID_C64.CPU.A=cRSID_C64.CPU.X=cRSID_C64.CPU.SP = rd(cRSID_C64.CPU.Addr) & cRSID_C64.CPU.SP; } //LAS(LAR)
           setNZbyA(); cRSID_C64.CPU.Cycles -= cRSID_C64.CPU.SamePage;
           break;

   case 6: if( MOSTLY ((IR&0x1F) != 0xB) ) { // CMP / DCP(DEC+CMP)
            if ( RARELY ((IR&3) == 3) ) { wr(cRSID_C64.CPU.Addr,rd(cRSID_C64.CPU.Addr)-1); cRSID_C64.CPU.Cycles+=2;} //DCP
            else cRSID_C64.CPU.Cycles -= cRSID_C64.CPU.SamePage;
            T = cRSID_C64.CPU.A - rd(cRSID_C64.CPU.Addr);
           }
           else { cRSID_C64.CPU.X = T = (cRSID_C64.CPU.A & cRSID_C64.CPU.X) - rd(cRSID_C64.CPU.Addr); cRSID_C64.CPU.X &= 0xFF; } //SBX(AXS)  //SBX (AXS) (CMP+DEX at the same time)
           setNZCbySub( T );
           break;

   case 7: if( RARELY ((IR&3)==3 && (IR&0x1F)!=0xB) ) { wr( cRSID_C64.CPU.Addr, rd(cRSID_C64.CPU.Addr)+1 ); cRSID_C64.CPU.Cycles+=2; } //ISC(ISB)=INC+SBC / SBC
           else cRSID_C64.CPU.Cycles -= cRSID_C64.CPU.SamePage;
           T = cRSID_C64.CPU.A; cRSID_C64.CPU.A -= rd(cRSID_C64.CPU.Addr) + !(cRSID_C64.CPU.ST & C);
           setNZCbySub( cRSID_C64.CPU.A ); cRSID_C64.CPU.A &= 0xFF; setVbyAdd( ~rd(cRSID_C64.CPU.Addr), T );
           break;
  }
 }


 else if (IR&2) {  //nybble2:  2:illegal/LDX, 6:A/X/INC/DEC, A:Accu-shift/reg.transfer/NOP, E:shift/X/INC/DEC

  switch (IR & 0x1F) { //Addressing modes
   case    2: addrModeImmediate(); break;
   case    6: addrModeZeropage(); break;
   case  0xE: addrModeAbsolute(); break;
   case 0x16: if ( (IR&0xC0) != 0x80 ) addrModeZeropageXindexed(); //zp,x
              else addrModeZeropageYindexed(); //zp,y
              break;
   case 0x1E: if ( (IR&0xC0) != 0x80 ) addrModeXindexed(); //abs,x
              else addrModeYindexed(); //abs,y
              break;
  }
  cRSID_C64.CPU.Addr&=0xFFFF;

  switch ( (IR & 0xE0) >> 5 ) {

   case 0: clrC(); //clear C for ASL //the rest of case 0 and 1 are identical but newer GCC gave notifications about 'fallthrough', so duplicated it
           if ( CALMLY ((IR&0xF)==0xA) ) { cRSID_C64.CPU.A = (cRSID_C64.CPU.A << 1) + (cRSID_C64.CPU.ST & C); setNZCbyAdd(); } //ASL/ROL (Accu)
           else { T = (rd(cRSID_C64.CPU.Addr)<<1) + (cRSID_C64.CPU.ST & C); setC(T>255); setNZbyT(T); wr(cRSID_C64.CPU.Addr,T); cRSID_C64.CPU.Cycles+=2; } //RMW (Read-Write-Modify)
           break;
   case 1: if ( CALMLY ((IR&0xF)==0xA) ) { cRSID_C64.CPU.A = (cRSID_C64.CPU.A << 1) + (cRSID_C64.CPU.ST & C); setNZCbyAdd(); } //ASL/ROL (Accu)
           else { T = (rd(cRSID_C64.CPU.Addr)<<1) + (cRSID_C64.CPU.ST & C); setC(T>255); setNZbyT(T); wr(cRSID_C64.CPU.Addr,T); cRSID_C64.CPU.Cycles+=2; } //RMW (Read-Write-Modify)
           break;

   case 2: clrC(); //clear C for LSR //the rest of case 2 and 3 are identical but newer GCC gave notifications about 'fallthrough', so duplicated it
           if ( CALMLY ((IR&0xF)==0xA) ) { T = cRSID_C64.CPU.A; cRSID_C64.CPU.A= (cRSID_C64.CPU.A >> 1) + ((cRSID_C64.CPU.ST&C) << 7); setC(T&1); cRSID_C64.CPU.A &= 0xFF; setNZbyA(); } //LSR/ROR (Accu)
           else { T = (rd(cRSID_C64.CPU.Addr)>>1) + ((cRSID_C64.CPU.ST&C) << 7); setC( rd(cRSID_C64.CPU.Addr) & 1 ); setNZbyT(T); wr(cRSID_C64.CPU.Addr,T); cRSID_C64.CPU.Cycles+=2; } //memory (RMW)
           break;
   case 3: if ( CALMLY ((IR&0xF)==0xA) ) { T = cRSID_C64.CPU.A; cRSID_C64.CPU.A= (cRSID_C64.CPU.A >> 1) + ((cRSID_C64.CPU.ST&C) << 7); setC(T&1); cRSID_C64.CPU.A &= 0xFF; setNZbyA(); } //LSR/ROR (Accu)
           else { T = (rd(cRSID_C64.CPU.Addr)>>1) + ((cRSID_C64.CPU.ST&C) << 7); setC( rd(cRSID_C64.CPU.Addr) & 1 ); setNZbyT(T); wr(cRSID_C64.CPU.Addr,T); cRSID_C64.CPU.Cycles+=2; } //memory (RMW)
           break;

   case 4: if ( MOSTLY (IR&4) ) { wr2( cRSID_C64.CPU.Addr, cRSID_C64.CPU.X ); } //STX
           else if ( RARELY (IR&0x10) ) cRSID_C64.CPU.SP = cRSID_C64.CPU.X; //TXS
           else { cRSID_C64.CPU.A = cRSID_C64.CPU.X; setNZbyA(); } //TXA
           break;

   case 5: if ( MOSTLY ((IR&0xF) != 0xA) ) { cRSID_C64.CPU.X = rd(cRSID_C64.CPU.Addr); cRSID_C64.CPU.Cycles -= cRSID_C64.CPU.SamePage; } //LDX
           else if ( RARELY (IR & 0x10) ) cRSID_C64.CPU.X = cRSID_C64.CPU.SP; //TSX
           else cRSID_C64.CPU.X = cRSID_C64.CPU.A; //TAX
           setNZbyX();
           break;

   case 6: if ( TIGHTLY (IR&4) ) { wr(cRSID_C64.CPU.Addr,rd(cRSID_C64.CPU.Addr)-1); setNZbyM(); cRSID_C64.CPU.Cycles+=2; } //DEC
           else { --cRSID_C64.CPU.X; setNZbyX(); } //DEX
           break;

   case 7: if ( TIGHTLY (IR&4) ) { wr(cRSID_C64.CPU.Addr,rd(cRSID_C64.CPU.Addr)+1); setNZbyM(); cRSID_C64.CPU.Cycles+=2; } //INC/NOP
           break;
  }
 }


 else if ( (IR & 0xC) == 8 ) {  //nybble2:  8:register/statusflag
  if ( IR&0x10 ) {
   if ( IR == 0x98 ) { cRSID_C64.CPU.A = cRSID_C64.CPU.Y; setNZbyA(); } //TYA
   else { //CLC/SEC/CLI/SEI/CLV/CLD/SED
    if (FlagSwitches[IR>>5] & 0x20) cRSID_C64.CPU.ST |= (FlagSwitches[IR>>5] & 0xDF);
    else cRSID_C64.CPU.ST &= ~( FlagSwitches[IR>>5] & 0xDF );
   }
  }
  else {
   switch ( (IR & 0xF0) >> 5 ) {
    case 0: push( cRSID_C64.CPU.ST ); cRSID_C64.CPU.Cycles=3; break; //PHP
    case 1: cRSID_C64.CPU.ST = pop(); cRSID_C64.CPU.Cycles=4; break; //PLP
    case 2: push( cRSID_C64.CPU.A ); cRSID_C64.CPU.Cycles=3; break; //PHA
    case 3: cRSID_C64.CPU.A = pop(); setNZbyA(); cRSID_C64.CPU.Cycles=4; break; //PLA
    case 4: --cRSID_C64.CPU.Y; setNZbyY(); break; //DEY
    case 5: cRSID_C64.CPU.Y = cRSID_C64.CPU.A; setNZbyY(); break; //TAY
    case 6: ++cRSID_C64.CPU.Y; setNZbyY(); break; //INY
    case 7: ++cRSID_C64.CPU.X; setNZbyX(); break; //INX
   }
  }
 }


 else {  //nybble2:  0: control/branch/Y/compare  4: Y/compare  C:Y/compare/JMP

  if ( (IR&0x1F) == 0x10 ) { //BPL/BMI/BVC/BVS/BCC/BCS/BNE/BEQ  relative branch
   ++cRSID_C64.CPU.PC;
   T = rd( cRSID_C64.CPU.PC ); if (T & 0x80) T -= 0x100;
   if (IR & 0x20) {
    if (cRSID_C64.CPU.ST & BranchFlags[IR>>6]) { cRSID_C64.CPU.PC += T; cRSID_C64.CPU.Cycles=3; }
   }
   else {
    if ( !(cRSID_C64.CPU.ST & BranchFlags[IR>>6]) ) { cRSID_C64.CPU.PC += T; cRSID_C64.CPU.Cycles=3; } //plus 1 cycle if page is crossed?
   }
  }

  else {  //nybble2:  0:Y/control/Y/compare  4:Y/compare  C:Y/compare/JMP
   switch (IR&0x1F) { //Addressing modes
    case    0: addrModeImmediate(); break; //imm. (or abs.low for JSR/BRK)
    case    4: addrModeZeropage(); break;
    case  0xC: addrModeAbsolute(); break;
    case 0x14: addrModeZeropageXindexed(); break; //zp,x
    case 0x1C: addrModeXindexed(); break; //abs,x
   }
   cRSID_C64.CPU.Addr &= 0xFFFF;

   switch ( (IR & 0xE0) >> 5 ) {

    case 0: if( TIGHTLY (!(IR&4)) ) { //BRK / NOP-absolute/abs,x/zp/zp,x
             push((cRSID_C64.CPU.PC+2-1) >> 8); push((cRSID_C64.CPU.PC+2-1) & 0xFF); push(cRSID_C64.CPU.ST|B); cRSID_C64.CPU.ST |= I; //BRK
             cRSID_C64.CPU.PC = rd(0xFFFE) + (rd(0xFFFF)<<8) - 1; cRSID_C64.CPU.Cycles=7;
            }
            else if ( RARELY (IR == 0x1C) ) cRSID_C64.CPU.Cycles -= cRSID_C64.CPU.SamePage; //NOP abs,x
            break;

    case 1: if ( CALMLY (IR & 0xF) ) { //BIT / NOP-abs,x/zp,x
             if ( MOSTLY (!(IR&0x10)) ) { cRSID_C64.CPU.ST &= 0x3D; cRSID_C64.CPU.ST |= (rd(cRSID_C64.CPU.Addr) & 0xC0) | ( (!(cRSID_C64.CPU.A & rd(cRSID_C64.CPU.Addr))) << 1 ); } //BIT
             else if ( RARELY (IR == 0x3C) ) cRSID_C64.CPU.Cycles -= cRSID_C64.CPU.SamePage; //NOP abs,x
            }
            else { //JSR
             push( (cRSID_C64.CPU.PC+2-1) >> 8 ); push( (cRSID_C64.CPU.PC+2-1) & 0xFF );
             cRSID_C64.CPU.PC = rd(cRSID_C64.CPU.Addr) + rd(cRSID_C64.CPU.Addr+1) * 256 - 1; cRSID_C64.CPU.Cycles=6;
            }
            break;

    case 2: if ( MOSTLY (IR & 0xF) ) { //JMP / NOP-abs,x/zp/zp,x
             if ( MOSTLY (IR == 0x4C) ) { //JMP
              cRSID_C64.CPU.PC = ( RARELY ( cRSID_C64.RealSIDmode && cRSID_C64.NMI && (PrevPC == 0xDC02 || PrevPC == 0xDC04) ) ) ?
                                 (unsigned int) ( ( (/*rd(cRSID_C64.CPU.PC+1)==2?0:*/0x800) + (cRSID_C64.CPU.Addr&0xFF) ) - 1 ) //Workaround: Hi_Fi_Sky.sid/WonderLand-XII/Hunters_Moon/File_Deleted/Storebror.sid and the like needs cycle/subcycle-exact emulation to work well. This value is OK (though jittery) for most tunes (as WebSID proved).
                                 : (cRSID_C64.CPU.Addr - 1);
              cRSID_C64.CPU.Cycles=3; rd(cRSID_C64.CPU.Addr+1); //a read from jump-address highbyte to acknowledge CIA-IRQ is used in some tunes with 'jmp DC0C' or 'jmp DD0C' (e.g. Wonderland_XIII_tune_1.sid or Hi_Fi_Sky.sid)
              //if (cRSID_C64.CPU.Addr==PrevPC) {storeReg(); cRSID_C64.Returned=1; return 0xFF;} //turn self-jump mainloop (after init) into idle time
             }
             else if ( RARELY (IR==0x5C) ) cRSID_C64.CPU.Cycles -= cRSID_C64.CPU.SamePage; //NOP abs,x
            }
            else { //RTI
             cRSID_C64.CPU.ST = pop(); T = pop(); cRSID_C64.CPU.PC = (pop() << 8) + T - 1; cRSID_C64.CPU.Cycles=6;
             if ( LIKELY (cRSID_C64.Returned && cRSID_C64.CPU.SP >= 0xFF) ) { ++cRSID_C64.CPU.PC; /*storeReg();*/ return 0xFE; }
            }
            break;

    case 3: if ( CALMLY (IR & 0xF) ) { //JMP() (indirect) / NOP-abs,x/zp/zp,x
             if ( MOSTLY (IR == 0x6C) ) { //JMP() (indirect)
              cRSID_C64.CPU.PC = rd( (cRSID_C64.CPU.Addr&0xFF00) + ((cRSID_C64.CPU.Addr+1)&0xFF) ); //(with highbyte-wraparound bug)
              cRSID_C64.CPU.PC = (cRSID_C64.CPU.PC << 8) + rd(cRSID_C64.CPU.Addr) - 1; cRSID_C64.CPU.Cycles=5;
             }
             else if ( RARELY (IR == 0x7C) ) cRSID_C64.CPU.Cycles -= cRSID_C64.CPU.SamePage; //NOP abs,x
            }
            else { //RTS
             if ( RARELY (cRSID_C64.CPU.SP >= 0xFF) ) {/*storeReg();*/ cRSID_C64.Returned=1; return 0xFF;} //Init returns, provide idle-time between IRQs
             T=pop(); cRSID_C64.CPU.PC = (pop() << 8) + T; cRSID_C64.CPU.Cycles=6;
            }
            break;

    case 4: if ( MOSTLY (IR & 4) ) { wr2( cRSID_C64.CPU.Addr, cRSID_C64.CPU.Y ); } //STY / NOP #imm
            break;

    case 5: cRSID_C64.CPU.Y = rd(cRSID_C64.CPU.Addr); setNZbyY(); cRSID_C64.CPU.Cycles -= cRSID_C64.CPU.SamePage; //LDY
            break;

    case 6: if ( MOSTLY (!(IR&0x10)) ) { //CPY / NOP abs,x/zp,x
             T = cRSID_C64.CPU.Y - rd(cRSID_C64.CPU.Addr); setNZCbySub( T ); //CPY
            }
            else if ( RARELY (IR==0xDC) ) cRSID_C64.CPU.Cycles -= cRSID_C64.CPU.SamePage; //NOP abs,x
            break;

    case 7: if ( MOSTLY (!(IR&0x10)) ) { //CPX / NOP abs,x/zp,x
             T = cRSID_C64.CPU.X - rd(cRSID_C64.CPU.Addr); setNZCbySub( T ); //CPX
            }
            else if ( RARELY (IR==0xFC) ) cRSID_C64.CPU.Cycles -= cRSID_C64.CPU.SamePage; //NOP abs,x
            break;
   }
  }
 }


 ++cRSID_C64.CPU.PC; //PC&=0xFFFF;

 //storeReg();


 if ( CALMLY (!cRSID_C64.RealSIDmode) ) { //substitute KERNAL IRQ-return in PSID (e.g. Microprose Soccer)
  if ( RARELY ( (cRSID_C64.RAMbank[1]&3)>1 && PrevPC<0xE000 && (cRSID_C64.CPU.PC==0xEA31 || cRSID_C64.CPU.PC==0xEA81 || cRSID_C64.CPU.PC==0xEA7E) ) ) return 0xFE;
 }


 return cRSID_C64.CPU.Cycles;
}



//INLINE void push (unsigned char value) { cRSID_C64.CPU.cRSID_C64.RAMbank[0x100+cRSID_C64.CPU.SP] = value; --cRSID_C64.CPU.SP; cRSID_C64.CPU.SP&=0xFF; } //push a value to stack

 //handle entering into IRQ and NMI interrupt
static INLINE char cRSID_handleCPUinterrupts () {
 enum StatusFlagBitValues { B=0x10, I=0x04 };

 if ( RARELY (cRSID_C64.NMI > cRSID_C64.CPU.PrevNMI) ) { //if IRQ and NMI at the same time, NMI is serviced first (or is it?!)
  //cRSID_C64.CPU.ST &= ~B;
  push(cRSID_C64.CPU.PC>>8); push(cRSID_C64.CPU.PC&0xFF); push(cRSID_C64.CPU.ST); cRSID_C64.CPU.ST |= I;
  cRSID_C64.CPU.PC = *cRSID_getMemReadPtr(0xFFFA) + (*cRSID_getMemReadPtr(0xFFFB)<<8); //NMI-vector
  cRSID_C64.CPU.PrevNMI = cRSID_C64.NMI;
  return 1;
 }
 else if ( RARELY (cRSID_C64.IRQ && !(cRSID_C64.CPU.ST&I) ) ) {
  //cRSID_C64.CPU.ST &= ~B;
  push(cRSID_C64.CPU.PC>>8); push(cRSID_C64.CPU.PC&0xFF); push(cRSID_C64.CPU.ST); cRSID_C64.CPU.ST |= I;
  cRSID_C64.CPU.PC = *cRSID_getMemReadPtr(0xFFFE) + (*cRSID_getMemReadPtr(0xFFFF)<<8); //maskable IRQ-vector
  cRSID_C64.CPU.PrevNMI = cRSID_C64.NMI;
  return 1;
 }
 cRSID_C64.CPU.PrevNMI = cRSID_C64.NMI; //prepare for NMI edge-detection

 return 0;
}
