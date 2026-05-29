

static const unsigned char cRSID_ADSR_DAC_6581[] = {
 0x00,0x02,0x03,0x04,0x05,0x06,0x07,0x09,0x09,0x0B,0x0B,0x0D,0x0D,0x0F,0x10,0x12,
 0x11,0x13,0x14,0x15,0x16,0x17,0x18,0x1A,0x1A,0x1C,0x1C,0x1E,0x1E,0x20,0x21,0x23, //used at output of ADSR envelope generator
 0x21,0x23,0x24,0x25,0x26,0x27,0x28,0x2A,0x2A,0x2C,0x2C,0x2E,0x2E,0x30,0x31,0x33, //(not used for wave-generator because of 8bit-only resolution)
 0x32,0x34,0x35,0x36,0x37,0x38,0x39,0x3B,0x3B,0x3D,0x3D,0x3F,0x3F,0x41,0x42,0x44,
 0x40,0x42,0x42,0x44,0x44,0x46,0x47,0x49,0x49,0x4A,0x4B,0x4D,0x4D,0x4F,0x50,0x52,
 0x51,0x53,0x53,0x55,0x55,0x57,0x58,0x5A,0x5A,0x5B,0x5C,0x5E,0x5E,0x60,0x61,0x63,
 0x61,0x62,0x63,0x65,0x65,0x67,0x68,0x6A,0x69,0x6B,0x6C,0x6E,0x6E,0x70,0x71,0x73,
 0x72,0x73,0x74,0x76,0x76,0x78,0x79,0x7B,0x7A,0x7C,0x7D,0x7F,0x7F,0x81,0x82,0x84,
 0x7B,0x7D,0x7E,0x80,0x80,0x82,0x83,0x85,0x84,0x86,0x87,0x89,0x89,0x8B,0x8C,0x8D,
 0x8C,0x8E,0x8F,0x91,0x91,0x93,0x94,0x96,0x95,0x97,0x98,0x9A,0x9A,0x9C,0x9D,0x9E,
 0x9C,0x9E,0x9F,0xA1,0xA1,0xA3,0xA4,0xA5,0xA5,0xA7,0xA8,0xAA,0xAA,0xAC,0xAC,0xAE,
 0xAD,0xAF,0xB0,0xB2,0xB2,0xB4,0xB5,0xB6,0xB6,0xB8,0xB9,0xBB,0xBB,0xBD,0xBD,0xBF,
 0xBB,0xBD,0xBE,0xC0,0xC0,0xC2,0xC2,0xC4,0xC4,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCD,
 0xCC,0xCE,0xCF,0xD1,0xD1,0xD3,0xD3,0xD5,0xD5,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDE,
 0xDC,0xDE,0xDF,0xE1,0xE1,0xE3,0xE3,0xE5,0xE5,0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xEE,
 0xED,0xEF,0xF0,0xF2,0xF2,0xF4,0xF4,0xF6,0xF6,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFF
};


INLINE void cRSID_emulateADSRs (FASTVAR cRSID_SIDinstance *const FASTPTR SID, FASTVAR char cycles) {
 static enum {
  SID_CHANNEL_SPACING = 7, SID_CHANNEL_COUNT = 3, CHANNEL2_INDEX = (2*SID_CHANNEL_SPACING), SID_CHANNELS_RANGE = (SID_CHANNEL_SPACING * SID_CHANNEL_COUNT),
 } Specs;
 static enum { GATE_BITVAL=0x01, ATTACK_BITVAL=0x80, DECAYSUSTAIN_BITVAL=0x40, HOLDZEROn_BITVAL=0x10 } ADSRstateBits;

 static const short ADSRprescalePeriods[16] = {
  9, 32, 63, 95, 149, 220, 267, 313, 392, 977, 1954, 3126, 3907, 11720, 19532, 31251
 };
 static const unsigned char ADSRexponentPeriods[256] = {
  1, 30, 30, 30, 30, 30, 30, 16, 16, 16, 16, 16, 16, 16, 16,
  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 4, 4, 4, 4, 4, //pos0:1  pos6:30  pos14:16  pos26:8
  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, //pos54:4 //pos93:2
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
 };

 FASTVAR unsigned char Channel;
 static unsigned char PrevGate, AD, SR;
 FASTVAR unsigned short PrescalePeriod;
 FASTVAR unsigned char * FASTPTR ChannelPtr, * FASTPTR ADSRstatePtr, * FASTPTR EnvelopeCounterPtr, * FASTPTR ExponentCounterPtr;
 FASTVAR unsigned short * FASTPTR RateCounterPtr;


 //'Paused' causes immediate stopping in audio-buffer thread, so SID-baseaddress changes during tune-switching won't give segfaults
 //if ( RARELY (cRSID.Paused || SID->BasePtr == NULL) ) return; //printf( "%p-", SID->BasePtr ); //avoid some segfaults when NULL-ing SID4

 for (Channel=0; Channel < SID_CHANNELS_RANGE; Channel += SID_CHANNEL_SPACING) {

  ChannelPtr=&SID->BasePtr[Channel]; AD=ChannelPtr[5]; SR=ChannelPtr[6];
  ADSRstatePtr = &(SID->ADSRstate[Channel]);
  RateCounterPtr = &(SID->RateCounter[Channel]);
  EnvelopeCounterPtr = &(SID->EnvelopeCounter[Channel]);
  ExponentCounterPtr = &(SID->ExponentCounter[Channel]);

  PrevGate = (*ADSRstatePtr & GATE_BITVAL);
  if ( RARELY (PrevGate != (ChannelPtr[4] & GATE_BITVAL)) ) { //gatebit-change?
   if (PrevGate) *ADSRstatePtr &= ~ (GATE_BITVAL | ATTACK_BITVAL | DECAYSUSTAIN_BITVAL); //falling edge
   else *ADSRstatePtr = (GATE_BITVAL | ATTACK_BITVAL | DECAYSUSTAIN_BITVAL | HOLDZEROn_BITVAL); //rising edge
  }

  if (*ADSRstatePtr & ATTACK_BITVAL) PrescalePeriod = ADSRprescalePeriods[ AD >> 4 ];
  else if (*ADSRstatePtr & DECAYSUSTAIN_BITVAL) PrescalePeriod = ADSRprescalePeriods[ AD & 0x0F ];
  else PrescalePeriod = ADSRprescalePeriods[ SR & 0x0F ];

  *RateCounterPtr += cycles; if ( RARELY (*RateCounterPtr >= 0x8000) ) *RateCounterPtr -= 0x8000; //*RateCounterPtr &= 0x7FFF; //can wrap around (ADSR delay-bug: short 1st frame)

  if ( RARELY (PrescalePeriod <= *RateCounterPtr && *RateCounterPtr < PrescalePeriod+cycles) ) { //ratecounter shot (matches rateperiod) (in genuine SID ratecounter is LFSR)
   *RateCounterPtr -= PrescalePeriod; //reset rate-counter on period-match
   if ( (*ADSRstatePtr & ATTACK_BITVAL) || ++(*ExponentCounterPtr) == ADSRexponentPeriods[*EnvelopeCounterPtr] ) {
    *ExponentCounterPtr = 0;
    if (*ADSRstatePtr & HOLDZEROn_BITVAL) {
     if (*ADSRstatePtr & ATTACK_BITVAL) {
      ++(*EnvelopeCounterPtr);
      if (*EnvelopeCounterPtr==0xFF) *ADSRstatePtr &= ~ATTACK_BITVAL;
     }
     else if ( !(*ADSRstatePtr & DECAYSUSTAIN_BITVAL) || *EnvelopeCounterPtr != (SR&0xF0)+(SR>>4) ) {
      --(*EnvelopeCounterPtr); //resid adds 1 cycle delay, we omit that mechanism here
      if (*EnvelopeCounterPtr==0) *ADSRstatePtr &= ~HOLDZEROn_BITVAL;
 }}}}}

}

