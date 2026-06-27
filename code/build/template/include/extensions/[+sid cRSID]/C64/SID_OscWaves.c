
#include "SID_CombiWaves.h" //"SIDwaves.h"


static INLINE unsigned short getPW (FASTVAR unsigned char* channelptr) {
 return ( ((channelptr[3]&0xF) << 8) | channelptr[2] ) << 4; //PW=0000..FFF0 from SID-register (000..FFF)
}

static INLINE unsigned short getCombinedPW (FASTVAR unsigned char* channelptr) {
 return ( ((channelptr[3]&0xF) << 8) | channelptr[2] ); //PW=000..FFF (range for combined-waveform lookup) from SID-register (000..FFF)
}


static INLINE unsigned short combinedWF (FASTVAR cRSID_SIDinstance *const FASTPTR SID, FASTVAR const unsigned char *const FASTPTR WFarray, FASTVAR unsigned short oscval, FASTVAR unsigned char channel, FASTVAR unsigned char* FASTPTR channelptr) {
 static enum {
  COMBINEDWF_SAMPLE_RESOLUTION = 12, COMBINEDWF_FILT_RESOLUTION = 16, //bits
  COMBINEDWF_WAVE_RESOLUTION = 8, CRSID_WAVE_RESOLUTION = 16, //bits
  COMBINEDWF_OSC_MSB_OFF_MASK = (1 << (COMBINEDWF_SAMPLE_RESOLUTION - 1) ) - 1, //0x7FFF
  COMBINEDWF_FILTMUL_MAX = ( (1 << COMBINEDWF_FILT_RESOLUTION) - 1 ), //0xFFFF
  COMBINEDWF_FILT_FRACTION_SHIFTS = 16,
  COMBINEDWF_WAVE_SHIFTS = (CRSID_WAVE_RESOLUTION - COMBINEDWF_WAVE_RESOLUTION) //8
 } Specs;
 FASTVAR unsigned char Pitch;
 FASTVAR unsigned short Filt;
 if ( RARELY (SID->ChipModel==6581 && WFarray!=cRSID_PulseTriangle) ) oscval &= COMBINEDWF_OSC_MSB_OFF_MASK; //0x7FF; //0x7FFF;
 Pitch = MOSTLY (channelptr[1]) ? channelptr[1] : 1; //avoid division by zero
 Filt = 0x7777 + (0x8888/Pitch);
 SID->PrevWavData[channel] = ( WFarray[oscval]*Filt + SID->PrevWavData[channel]*(COMBINEDWF_FILTMUL_MAX-Filt) ) >> COMBINEDWF_FILT_FRACTION_SHIFTS; //16;
 return SID->PrevWavData[channel] << COMBINEDWF_WAVE_SHIFTS; //8;
}



INLINE int cRSID_emulateSID_light (FASTVAR cRSID_SIDinstance *const FASTPTR SID) {
 static enum { NOISE_BITVAL=0x80, PULSE_BITVAL=0x40, SAW_BITVAL=0x20, TRI_BITVAL=0x10,
               PULSAWTRI_VAL=0x70, PULSAW_VAL=0x60, PULTRI_VAL=0x50, SAWTRI_VAL=0x30 } WaveFormBits;
 static enum { TEST_BITVAL=0x08, RING_BITVAL=0x04, SYNC_BITVAL=0x02, GATE_BITVAL=0x01 } ControlBits;
 static enum { OFF3_BITVAL=0x80 } FilterBits;
 static enum {
  SID_CHANNEL_SPACING = 7, SID_CHANNEL_COUNT = 3, CHANNEL2_INDEX = (2*SID_CHANNEL_SPACING), SID_CHANNELS_RANGE = (SID_CHANNEL_SPACING * SID_CHANNEL_COUNT),
  SID_PHASEACCU_RESOLUTION = 24, SID_WAVE_RESOLUTION = 12, CRSID_WAVE_RESOLUTION = 16, OSC3_WAVE_RESOLUTION = 8, //bits
  COMBINEDWF_SAMPLE_RESOLUTION = 12, //bits
  PHASEACCU_SID__RANGE = (1 << SID_PHASEACCU_RESOLUTION), //0x1000000, //in SID phase-accumulator coves 24 bit value-range
  PHASEACCU_RANGE = (PHASEACCU_SID__RANGE << CRSID_CLOCK_FRACTIONAL_BITS), //, but we use higher range for higher precision
  PHASEACCU_MAX = (PHASEACCU_RANGE - 1), PHASEACCU_ANDMASK = (PHASEACCU_MAX), //0xFFFFFFF
  PHASEACCU_MSB_BITVAL = ( (PHASEACCU_SID__RANGE >> 1) << CRSID_CLOCK_FRACTIONAL_BITS ), //0x8000000
  CRSID_WAVE_SHIFTS = ( (SID_PHASEACCU_RESOLUTION - CRSID_WAVE_RESOLUTION) + CRSID_CLOCK_FRACTIONAL_BITS ), //12
  CRSID_WAVE_RANGE = (1 << CRSID_WAVE_RESOLUTION), CRSID_WAVE_MAX = ( CRSID_WAVE_RANGE - 1 ), //0xFFFF
  CRSID_WAVE_MASK = (CRSID_WAVE_MAX), CRSID_WAVE_MID = (CRSID_WAVE_RANGE / 2), CRSID_WAVE_MIN = 0x0000,
  SOUNDEMON_DIGI_SEEK_WAVEFORM=0x01, SOUNDEMON_DIGI_RESOLUTION=8, SOUNDEMON_DIGI_SHIFTS = (CRSID_WAVE_RESOLUTION - SOUNDEMON_DIGI_RESOLUTION),
  SOUNDEMON_CARRIER_ELIMINATION_SAMPLECOUNT = 2, //4,
  SID_NOISE_CLOCK_BITVAL = 0x100000, NOISE_CLOCK = (SID_NOISE_CLOCK_BITVAL << CRSID_CLOCK_FRACTIONAL_BITS), //0x1000000 //value of phaseaccu-bit clocking the Noise-LFSR
  COMBINEDWF_SAMPLE_SHIFTS = ( (SID_PHASEACCU_RESOLUTION - COMBINEDWF_SAMPLE_RESOLUTION) + CRSID_CLOCK_FRACTIONAL_BITS ), //16
  WAVE_OSC3_SHIFTS = (CRSID_WAVE_RESOLUTION - OSC3_WAVE_RESOLUTION), //8
  STEEPNESS_FRACTION_SHIFTS = 16, STEEPNESS_STEPLIMIT = (256 << CRSID_CLOCK_FRACTIONAL_BITS), //4096 //high-pitch waveform anti-aliasing by frequency-dependent wave-edge steepness control
  SID_ENVELOPE_RESOLUTION = 8 /*bits*/, SID_ENVELOPE_MAGNITUDE = (1 << SID_ENVELOPE_RESOLUTION), //256
  ENVELOPE_MAGNITUDE_DIV = (SID_ENVELOPE_MAGNITUDE * CRSID_WAVGEN_PREDIV) //256 * 1..16 = 256..4096
 } Specs;

 static const unsigned char FilterSwitchVal[] = {1,1,1,1,1,1,1,2,2,2,2,2,2,2,4};

 FASTVAR unsigned char Channel;
 FASTVAR unsigned char * FASTPTR ChannelPtr;
 //static char MainVolume;
 FASTVAR unsigned char WF, FilterSwitchReso, VolumeBand;
 static unsigned char TestBit, Envelope;
 FASTVAR unsigned int Utmp, WavGenOut, PW;
 static unsigned int PhaseAccuStep, MSB;
 FASTVAR int Tmp, Feedback;
 static int Steepness, PulsePeak;
 //static int FilterInput, Cutoff, Resonance, FilterOutput, NonFilted, Output;
 FASTVAR int * FASTPTR PhaseAccuPtr;

 //'Paused' causes immediate stopping in audio-buffer thread, so SID-baseaddress changes during tune-switching won't give segfaults
 //if ( RARELY (cRSID.Paused || SID->BasePtr == NULL) ) return 0; //avoid some segfaults when NULL-ing SID4

 SID->NonFiltedSample = SID->FilterInputSample = 0;
 FilterSwitchReso = SID->BasePtr[0x17]; VolumeBand=SID->BasePtr[0x18];

 //Waveform-generator //(phase accumulator and waveform-selector)

 for (Channel=0; Channel < SID_CHANNELS_RANGE; Channel += SID_CHANNEL_SPACING) {
  ChannelPtr=&(SID->BasePtr[Channel]);

  WF = ChannelPtr[4]; TestBit = RARELY ( (WF & TEST_BITVAL) != 0 );
  PhaseAccuPtr = &(SID->PhaseAccu[Channel]);

  PhaseAccuStep = ( (ChannelPtr[1]<<8) | ChannelPtr[0] ) * cRSID_C64.SampleClockRatio; //SID->cRSID_C64.SampleClockRatio;
  if ( RARELY (TestBit || ((WF & SYNC_BITVAL) && SID->SyncSourceMSBrise)) ) *PhaseAccuPtr = 0;
  else { //stepping phase-accumulator (oscillator)
   *PhaseAccuPtr += PhaseAccuStep;
   if ( RARELY (*PhaseAccuPtr >= PHASEACCU_RANGE) ) *PhaseAccuPtr -= PHASEACCU_RANGE; //0x10000000
  }
  *PhaseAccuPtr &= PHASEACCU_ANDMASK; //0xFFFFFFF;
  MSB = *PhaseAccuPtr & PHASEACCU_MSB_BITVAL; //0x8000000;
  SID->SyncSourceMSBrise = RARELY (MSB > (SID->PrevPhaseAccu[Channel] & PHASEACCU_MSB_BITVAL)) ? 1 : 0;

  switch (WF & 0xF0) { //switch-case encourages computed-goto compiler-optimization
   case NOISE_BITVAL: { //if (WF & NOISE_BITVAL) { //noise waveform
    Tmp = SID->NoiseLFSR[Channel]; //clock LFSR all time if clockrate exceeds observable at given samplerate (last term):
    if ( RARELY (((*PhaseAccuPtr & NOISE_CLOCK) != (SID->PrevPhaseAccu[Channel] & NOISE_CLOCK)) || PhaseAccuStep >= NOISE_CLOCK) ) {
     Feedback = ( (Tmp & 0x400000) ^ ((Tmp & 0x20000) << 5) ) != 0;
     Tmp = ( (Tmp << 1) | Feedback|TestBit ) & 0x7FFFFF; //TEST-bit turns all bits in noise LFSR to 1 (on real SID slowly, in approx. 8000 microseconds ~ 300 samples)
     SID->NoiseLFSR[Channel] = Tmp;
    } //we simply zero output when other waveform is mixed with noise. On real SID LFSR continuously gets filled by zero and locks up. ($C1 waveform with pw<8 can keep it for a while.)
    WavGenOut = /*(WF & 0x70) ? 0 :*/ ((Tmp & 0x100000) >> 5) | ((Tmp & 0x40000) >> 4) | ((Tmp & 0x4000) >> 1) | ((Tmp & 0x800) << 1)
                                  | ((Tmp & 0x200) << 2) | ((Tmp & 0x20) << 5) | ((Tmp & 0x04) << 7) | ((Tmp & 0x01) << 8);
   } break;

   case PULSE_BITVAL: { //else if (WF & PULSE_BITVAL) { //simple pulse
    PW = getPW(ChannelPtr); //( ((ChannelPtr[3]&0xF) << 8) + ChannelPtr[2] ) << 4; //PW=0000..FFF0 from SID-register
    Utmp = (int) (PhaseAccuStep >> (CRSID_WAVE_SHIFTS+1)); if ( RARELY (0 < PW && PW < Utmp) ) PW = Utmp; //Too thin pulsewidth? Correct...
    Utmp ^= CRSID_WAVE_MAX;  if ( RARELY (PW > Utmp) ) PW = Utmp; //Too thin pulsewidth? Correct it to a value representable at the current samplerate
    Utmp = *PhaseAccuPtr >> CRSID_WAVE_SHIFTS; //12;
    //if ( (WF&0xF0) == PULSE_BITVAL ) { //simple pulse, most often used waveform, make it sound as clean as possible (by making it trapezoid)
    Steepness = (PhaseAccuStep>=STEEPNESS_STEPLIMIT) ? PHASEACCU_MAX/PhaseAccuStep : CRSID_WAVE_MAX; //0xFFFF; //rising/falling-edge steepness (add/sub at samples)
    if ( RARELY (TestBit) ) WavGenOut = CRSID_WAVE_MAX; //0xFFFF;
    else if (Utmp<PW) { //rising edge (interpolation)
     PulsePeak = (CRSID_WAVE_MAX-PW) * Steepness; //very thin pulses don't make a full swing between 0 and max but make a little spike
     if (PulsePeak>CRSID_WAVE_MAX) PulsePeak=CRSID_WAVE_MAX; //0xFFFF; //but adequately thick trapezoid pulses reach the maximum level
     Tmp = PulsePeak - (PW-Utmp)*Steepness; //draw the slope from the peak
     WavGenOut = MOSTLY (Tmp < CRSID_WAVE_MIN) ? CRSID_WAVE_MIN : Tmp; //but stop at 0-level
    }
    else { //falling edge (interpolation)
     PulsePeak = PW*Steepness; //very thin pulses don't make a full swing between 0 and max but make a little spike
     if (PulsePeak>CRSID_WAVE_MAX) PulsePeak=CRSID_WAVE_MAX; //adequately thick trapezoid pulses reach the maximum level
     Tmp = (CRSID_WAVE_MAX-Utmp)*Steepness - PulsePeak; //draw the slope from the peak
     WavGenOut = MOSTLY (Tmp>=0) ? CRSID_WAVE_MAX : Tmp;         //but stop at max-level
    } //}
   } break;

   case PULSAWTRI_VAL: { //else { //combined pulse  //pulse+saw+triangle (waveform nearly identical to tri+saw)
    Utmp = *PhaseAccuPtr >> COMBINEDWF_SAMPLE_SHIFTS; //16; //12;
    WavGenOut = Utmp >= getCombinedPW(ChannelPtr) || RARELY(TestBit)
                ? combinedWF( SID, cRSID_PulseSawTriangle, Utmp, Channel, ChannelPtr ) : CRSID_WAVE_MIN; //0;
   } break;
   case PULSAW_VAL: { //else if (WF & SAW_BITVAL) { //pulse+saw
    Utmp = *PhaseAccuPtr >> COMBINEDWF_SAMPLE_SHIFTS; //16; //12;
    WavGenOut = Utmp >= getCombinedPW(ChannelPtr) //|| RARELY(TestBit)
                ? combinedWF( SID, cRSID_PulseSawtooth, Utmp, Channel, ChannelPtr ) : CRSID_WAVE_MIN; //0;
   } break;
   case PULTRI_VAL: { // else { //pulse+triangle
    Tmp = *PhaseAccuPtr ^ ( (WF&RING_BITVAL)? SID->RingSourceMSB : 0 );
    WavGenOut = (*PhaseAccuPtr >> COMBINEDWF_SAMPLE_SHIFTS) >= getCombinedPW(ChannelPtr) || RARELY(TestBit)
     ? combinedWF( SID, cRSID_PulseTriangle, Tmp >> COMBINEDWF_SAMPLE_SHIFTS, Channel, ChannelPtr ) : CRSID_WAVE_MIN; //0;
   } break;

   case SAWTRI_VAL: {  //if (WF & TRI_BITVAL)
    WavGenOut = combinedWF( SID, cRSID_SawTriangle, *PhaseAccuPtr >> COMBINEDWF_SAMPLE_SHIFTS, Channel, ChannelPtr ); //saw+triangle
   } break;    //else { //simple cleaned (bandlimited) saw:
   case SAW_BITVAL: { //else if (WF & SAW_BITVAL) { //sawtooth
    WavGenOut = *PhaseAccuPtr >> CRSID_WAVE_SHIFTS; //12; //saw (this row would be enough for simple but aliased-at-high-pitch saw)
    Steepness = (PhaseAccuStep>>CRSID_CLOCK_FRACTIONAL_BITS) / 288; if ( RARELY (Steepness==0) ) Steepness=1; //avoid division by zero in next steps
    WavGenOut += (WavGenOut * Steepness) >> STEEPNESS_FRACTION_SHIFTS; //16; //1st half (rising edge) of asymmetric triangle-like saw waveform
    if ( RARELY (WavGenOut>CRSID_WAVE_MAX) )
       WavGenOut = CRSID_WAVE_MAX - ( ((WavGenOut-CRSID_WAVE_RANGE)<<STEEPNESS_FRACTION_SHIFTS) / Steepness ); //2nd half (falling edge, reciprocal steepness
   } break;
   case TRI_BITVAL: { //else if (WF & TRI_BITVAL) { //triangle (this waveform has no harsh edges, so it doesn't suffer from strong aliasing at high pitches)
    if ( MOSTLY (!cRSID_C64.RealSIDmode || SID->PrevSounDemonDigiWF[Channel] <= 0) ) { // != SOUNDEMON_DIGI_SEEK_WAVEFORM) ) {
     Tmp = *PhaseAccuPtr ^ ( RARELY(WF&RING_BITVAL) ? SID->RingSourceMSB : 0 );
     WavGenOut = ( Tmp ^ (Tmp&PHASEACCU_MSB_BITVAL? PHASEACCU_MAX:0) ) >> (CRSID_WAVE_SHIFTS-1); //11;
    }  //SounDemon digi hack: if previous waveform was 01, don't modify output in this round:
    else { WavGenOut = SID->PrevWavGenOut[Channel]; --SID->PrevSounDemonDigiWF[Channel]; } //(so carrier noise won't be heard due to non 1MHz emulation)
   } break;

   case 0x00: //emulate waveform 00 floating wave-DAC (utilized by SounDemon digis) (on real SID waveform00 decays after about 5 seconds, here we just simply keep the value to avoid clicks)
    //(Our jittery 'seeking' waveform=$01 part of SounDemon-digi is substituted directly by frequency-high register's value (as in SwinSID))
    if (cRSID_C64.RealSIDmode && WF == SOUNDEMON_DIGI_SEEK_WAVEFORM) {   //WavGenOut = ( !cRSID_C64.RealSIDmode || WF != SOUNDEMON_DIGI_SEEK_WAVEFORM /*|| ChannelPtr[1]==0*/ ) ? SID->PrevWavGenOut[Channel] : (unsigned int)(ChannelPtr[1] << SOUNDEMON_DIGI_SHIFTS);
     WavGenOut = ChannelPtr[1] << SOUNDEMON_DIGI_SHIFTS;
     SID->PrevSounDemonDigiWF[Channel] = SOUNDEMON_CARRIER_ELIMINATION_SAMPLECOUNT;
    }
    else WavGenOut = SID->PrevWavGenOut[Channel];
    break;
   default: WavGenOut = CRSID_WAVE_MIN; break; //noise plus pulse/saw/triangle mostly yields silence
  }
  //SID->PrevSounDemonDigiWF[Channel] = WF;

  WavGenOut &= CRSID_WAVE_MASK; //0xFFFF;
  /*if (WF&0xF0)*/ SID->PrevWavGenOut[Channel] = WavGenOut; //emulate waveform 00 floating wave-DAC (utilized by SounDemon digis)
  //else WavGenOut = SID->PrevWavGenOut[Channel];  //(on real SID waveform00 decays, we just simply keep the value to avoid clicks)
  SID->PrevPhaseAccu[Channel] = *PhaseAccuPtr;
  SID->RingSourceMSB = MSB;

  //routing the channel signal to either the filter or the unfiltered master output depending on filter-switch SID-registers
  Envelope = MOSTLY (SID->ChipModel==8580) ?  SID->EnvelopeCounter[Channel] : cRSID_ADSR_DAC_6581[SID->EnvelopeCounter[Channel]];
  if ( RARELY (FilterSwitchReso & FilterSwitchVal[Channel]) ) {
   SID->FilterInputSample += ( ((int)WavGenOut-CRSID_WAVE_MID) * Envelope ) / ENVELOPE_MAGNITUDE_DIV; // >> 8;
  }
  else if ( MOSTLY (Channel!=14 || !(VolumeBand & OFF3_BITVAL)) ) {
   SID->NonFiltedSample += ( ((int)WavGenOut-CRSID_WAVE_MID) * Envelope ) / ENVELOPE_MAGNITUDE_DIV; // >> 8;
  }

 }
 //update readable SID1-registers (some SID tunes might use 3rd channel ENV3/OSC3 value as control)
 SID->BasePtrRD[0x1B] /*cRSID_C64.IObankRD[SID->BaseAddress+0x1B]*/ = WavGenOut >> WAVE_OSC3_SHIFTS; //8; //OSC3, ENV3 (some players rely on it, unfortunately even for timing)
 SID->BasePtrRD[0x1C] /*cRSID_C64.IObankRD[SID->BaseAddress+0x1C]*/ = SID->EnvelopeCounter[CHANNEL2_INDEX]; //14]; //Envelope
 //cRSID_C64.IObankRD[SID->BaseAddress+0x1F] = (cRSID.SelectedSIDmodel==8580); //this doesn't exist in real SID but SID-Wizard code removes comment-marks and uses it as identification workaround

 return cRSID_emulateSIDoutputStage( SID );
}



//----------------------- High-quality (oversampled) waveform-generation --------------------------



static INLINE unsigned short HQcombinedWF (FASTVAR cRSID_SIDinstance *const FASTPTR SID, FASTVAR const unsigned char *const WFarray, FASTVAR unsigned short oscval) {
 static enum {
  COMBINEDWF_SAMPLE_RESOLUTION = 12, //bits
  COMBINEDWF_WAVE_RESOLUTION = 8, CRSID_WAVE_RESOLUTION = 16, //bits
  COMBINEDWF_OSC_MSB_OFF_MASK = (1 << (COMBINEDWF_SAMPLE_RESOLUTION - 1) ) - 1, //0x7FFF
  COMBINEDWF_WAVE_SHIFTS = (CRSID_WAVE_RESOLUTION - COMBINEDWF_WAVE_RESOLUTION) //8
 } Specs;
 if ( RARELY (SID->ChipModel==6581 && WFarray!=cRSID_PulseTriangle) ) oscval &= COMBINEDWF_OSC_MSB_OFF_MASK; //0x7FF; //0x7FFF;
 return WFarray[ oscval ] << COMBINEDWF_WAVE_SHIFTS; //8
}


INLINE cRSID_SIDwavOutput cRSID_emulateHQwaves (FASTVAR cRSID_SIDinstance *const FASTPTR SID, FASTVAR char cycles) { //, FASTVAR char filter) {
 static enum { NOISE_BITVAL=0x80, PULSE_BITVAL=0x40, SAW_BITVAL=0x20, TRI_BITVAL=0x10,
               PULSAWTRI_VAL=0x70, PULSAW_VAL=0x60, PULTRI_VAL=0x50, SAWTRI_VAL=0x30 } WaveFormBits;
 static enum { TEST_BITVAL=0x08, RING_BITVAL=0x04, SYNC_BITVAL=0x02, GATE_BITVAL=0x01 } ControlBits;
 static enum { OFF3_BITVAL=0x80 } FilterBits;
 static enum {
  SID_CHANNEL_SPACING = 7, SID_CHANNEL_COUNT = 3, CHANNEL2_INDEX = (2*SID_CHANNEL_SPACING), SID_CHANNELS_RANGE = (SID_CHANNEL_SPACING * SID_CHANNEL_COUNT),
  SID_PHASEACCU_RESOLUTION = 24, SID_WAVE_RESOLUTION = 12, CRSID_WAVE_RESOLUTION = 16, OSC3_WAVE_RESOLUTION = 8, //bits
  PHASEACCU_RANGE = (1 << SID_PHASEACCU_RESOLUTION), //0x1000000, //in SID phase-accumulator coves 24 bit value-range
  PHASEACCU_MAX = (PHASEACCU_RANGE - 1), PHASEACCU_ANDMASK = PHASEACCU_MAX, PHASEACCU_MSB_BITVAL = (PHASEACCU_RANGE >> 1), //0x800000
  CRSID_WAVE_SHIFTS = (SID_PHASEACCU_RESOLUTION - CRSID_WAVE_RESOLUTION),
  CRSID_WAVE_RANGE = (1 << CRSID_WAVE_RESOLUTION), CRSID_WAVE_MAX = ( CRSID_WAVE_RANGE - 1 ),
  CRSID_WAVE_MASK = (CRSID_WAVE_MAX), CRSID_WAVE_MID = (CRSID_WAVE_RANGE / 2), CRSID_WAVE_MIN = 0x0000,
  SOUNDEMON_DIGI_SEEK_WAVEFORM=0x01, SOUNDEMON_DIGI_RESOLUTION=8, SOUNDEMON_DIGI_SHIFTS = (CRSID_WAVE_RESOLUTION - SOUNDEMON_DIGI_RESOLUTION),
  SOUNDEMON_CARRIER_ELIMINATION_CYCLECOUNT = 24,
  COMBINEDWF_SAMPLE_RESOLUTION = 12, COMBINEDWF_SAMPLE_SHIFTS = (SID_PHASEACCU_RESOLUTION - COMBINEDWF_SAMPLE_RESOLUTION),
  NOISE_CLOCK = 0x100000, WAVE_OSC3_SHIFTS = (CRSID_WAVE_RESOLUTION - OSC3_WAVE_RESOLUTION),
  SID_ENVELOPE_RESOLUTION = 8 /*bits*/, SID_ENVELOPE_MAGNITUDE = (1 << SID_ENVELOPE_RESOLUTION), //256
  ENVELOPE_MAGNITUDE_DIV = (SID_ENVELOPE_MAGNITUDE * CRSID_WAVGEN_PREDIV) //256 * 1..16 = 256..4096
 } Specs;

 //#include "SIDwaves.h"

 FASTVAR unsigned char Channel, WF, FilterSwitchReso, VolumeBand;
 static unsigned char TestBit, Envelope;
 FASTVAR unsigned int Utmp, WavGenOut, PW;
 static unsigned int PhaseAccuStep, MSB;
 FASTVAR int Tmp, Feedback;
 //static int FilterInput, Cutoff, Resonance; //, FilterOutput, NonFilted, Output;
 FASTVAR unsigned char * FASTPTR ChannelPtr;
 FASTVAR int * FASTPTR PhaseAccuPtr;
 static cRSID_SIDwavOutput SIDwavOutput;

 static const unsigned char FilterSwitchVal[] = {1,1,1,1,1,1,1,2,2,2,2,2,2,2,4};

 //'Paused' causes immediate stopping in audio-buffer thread, so SID-baseaddress changes during tune-switching won't give segfaults
 //if ( RARELY (cRSID.Paused || SID->BasePtr == NULL) ) return (cRSID_SIDwavOutput) {0,0}; //{{0},0}; //avoid some segfaults when NULL-ing SID4

 SIDwavOutput.FilterInput = SIDwavOutput.NonFilted = 0;
 FilterSwitchReso = SID->BasePtr[0x17]; VolumeBand=SID->BasePtr[0x18];

 for (Channel=0; Channel < SID_CHANNELS_RANGE; Channel += SID_CHANNEL_SPACING) {
  ChannelPtr = &( SID->BasePtr[Channel] );

  WF = ChannelPtr[4]; TestBit = RARELY ( (WF & TEST_BITVAL) != 0 );
  PhaseAccuPtr = &(SID->PhaseAccu[Channel]);

  PhaseAccuStep = ( (ChannelPtr[1]<<8) | ChannelPtr[0] ) * cycles; //oscillator pitch
  if ( RARELY (TestBit || ((WF & SYNC_BITVAL) && SID->SyncSourceMSBrise)) ) *PhaseAccuPtr = 0; //oscillator-sync
  else { //stepping phase-accumulator (oscillator)
   *PhaseAccuPtr += PhaseAccuStep;
   if ( RARELY (*PhaseAccuPtr >= PHASEACCU_RANGE) ) *PhaseAccuPtr -= PHASEACCU_RANGE; //0x1000000;
  }
  *PhaseAccuPtr &= PHASEACCU_ANDMASK; //0xFFFFFF;
  MSB = *PhaseAccuPtr & PHASEACCU_MSB_BITVAL; //0x800000;
  SID->SyncSourceMSBrise = RARELY (MSB > (SID->PrevPhaseAccu[Channel] & PHASEACCU_MSB_BITVAL)) ? 1 : 0;

  switch (WF & 0xF0) { //switch-case encourages computed-goto compiler-optimization
   case NOISE_BITVAL: {  //if (WF & NOISE_BITVAL) { //noise waveform
    Tmp = SID->NoiseLFSR[Channel]; //clock LFSR all time if clockrate exceeds observable at given samplerate (last term):
    if ( RARELY ((*PhaseAccuPtr & NOISE_CLOCK) != (SID->PrevPhaseAccu[Channel] & NOISE_CLOCK)) ) {
     Feedback = ( (Tmp & 0x400000) ^ ((Tmp & 0x20000) << 5) ) != 0;
     Tmp = ( (Tmp << 1) | Feedback|TestBit ) & 0x7FFFFF; //TEST-bit turns all bits in noise LFSR to 1 (on real SID slowly, in approx. 8000 microseconds ~ 300 samples)
     SID->NoiseLFSR[Channel] = Tmp;
    } //we simply zero output below when other waveform is mixed with noise. On real SID LFSR continuously gets filled by zero and locks up. ($C1 waveform with pw<8 can keep it for a while.)
    SID->PrevWavGenOut[Channel] = /*(WF & 0x70) ? 0 :*/ ((Tmp & 0x100000) >> 5) | ((Tmp & 0x40000) >> 4) | ((Tmp & 0x4000) >> 1) | ((Tmp & 0x800) << 1)
                                  | ((Tmp & 0x200) << 2) | ((Tmp & 0x20) << 5) | ((Tmp & 0x04) << 7) | ((Tmp & 0x01) << 8);
   } break;

   case PULSAWTRI_VAL: { //pulse+saw+triangle (waveform nearly identical to tri+saw)
    Utmp = *PhaseAccuPtr >> COMBINEDWF_SAMPLE_SHIFTS; //12; //8;
    SID->PrevWavGenOut[Channel] = Utmp >= getCombinedPW(ChannelPtr) || RARELY(TestBit)
                                  ? HQcombinedWF( SID, cRSID_PulseSawTriangle, Utmp ) : CRSID_WAVE_MIN; //0;
   } break;
   case PULSAW_VAL: { //pulse+saw
    Utmp = *PhaseAccuPtr >> COMBINEDWF_SAMPLE_SHIFTS; //12; //8;
    SID->PrevWavGenOut[Channel] = Utmp >= getCombinedPW(ChannelPtr) || RARELY(TestBit)
                                  ? HQcombinedWF( SID, cRSID_PulseSawtooth, Utmp ) : CRSID_WAVE_MIN; //0;
   } break;
   case PULTRI_VAL: { //pulse+triangle
    Tmp = *PhaseAccuPtr ^ ( (WF&RING_BITVAL)? SID->RingSourceMSB : 0 );
    SID->PrevWavGenOut[Channel] = (*PhaseAccuPtr >> COMBINEDWF_SAMPLE_SHIFTS) >= getCombinedPW(ChannelPtr) || RARELY(TestBit) ?
                                  HQcombinedWF( SID, cRSID_PulseTriangle, Tmp >> COMBINEDWF_SAMPLE_SHIFTS ) : CRSID_WAVE_MIN; //0;
   } break;

   case PULSE_BITVAL: {  //else if (WF & PULSE_BITVAL) { //simple pulse or pulse+combined
    SID->PrevWavGenOut[Channel] = (*PhaseAccuPtr >> CRSID_WAVE_SHIFTS) >= getPW(ChannelPtr) || RARELY(TestBit)
                                  ? CRSID_WAVE_MAX : CRSID_WAVE_MIN; //0xFFFF : 0;
   } break;
   case SAWTRI_VAL: SID->PrevWavGenOut[Channel] = HQcombinedWF( SID, cRSID_SawTriangle, *PhaseAccuPtr >> COMBINEDWF_SAMPLE_SHIFTS ); break; //saw+triangle
   case SAW_BITVAL: {  //else if (WF & SAW_BITVAL) { //sawtooth
    SID->PrevWavGenOut[Channel] = *PhaseAccuPtr >> CRSID_WAVE_SHIFTS; //8;  //if (WF & TRI_BITVAL) WavGenOut = HQcombinedWF( SID, cRSID_SawTriangle, WavGenOut ); //saw+triangle
   } break;
   case TRI_BITVAL: {  //else if (WF & TRI_BITVAL) { //triangle (this waveform has no harsh edges, so it doesn't suffer from strong aliasing at high pitches)
    if ( MOSTLY (!cRSID_C64.RealSIDmode || SID->PrevSounDemonDigiWF[Channel] <= 0) ) { // != SOUNDEMON_DIGI_SEEK_WAVEFORM) ) {
     Tmp = *PhaseAccuPtr ^ ( RARELY(WF&RING_BITVAL) ? SID->RingSourceMSB : 0 );
     SID->PrevWavGenOut[Channel] = ( ( Tmp ^ (Tmp&PHASEACCU_MSB_BITVAL? PHASEACCU_MAX:0) ) >> (CRSID_WAVE_SHIFTS-1) ) & CRSID_WAVE_MASK; //0xFFFF;
    }  //SounDemon digi hack: if previous waveform was 01, don't modify output in these rounds:
    else SID->PrevSounDemonDigiWF[Channel] -= cycles;  //(so carrier noise won't be heard in Fanta_in_Space.sid/etc due to non-1MHz emulation)
   } break;

   case 0x00: //emulate waveform 00 floating wave-DAC (utilized by SounDemon digis) (on real SID waveform00 decays, we just simply keep the value to avoid clicks)
    //(Our jittery 'seeking' waveform=$01 part of SounDemon-digi is substituted directly by frequency-high register's value (as in SwinSID))
    if ( TIGHTLY ( cRSID_C64.RealSIDmode && WF == SOUNDEMON_DIGI_SEEK_WAVEFORM) ) {
     SID->PrevWavGenOut[Channel] = (ChannelPtr[1] << SOUNDEMON_DIGI_SHIFTS);
     SID->PrevSounDemonDigiWF[Channel] = SOUNDEMON_CARRIER_ELIMINATION_CYCLECOUNT;
    }
    break;
   default: SID->PrevWavGenOut[Channel] = CRSID_WAVE_MIN; break;  //noise with pulse/saw/triangle mostly yields silence
  }
  WavGenOut = SID->PrevWavGenOut[Channel];

  SID->PrevPhaseAccu[Channel] = *PhaseAccuPtr;
  SID->RingSourceMSB = MSB;

  //routing the channel signal to either the filter or the unfiltered master output depending on filter-switch SID-registers
  Envelope = MOSTLY (SID->ChipModel==8580) ?  SID->EnvelopeCounter[Channel] : cRSID_ADSR_DAC_6581[SID->EnvelopeCounter[Channel]];
  if ( RARELY (FilterSwitchReso & FilterSwitchVal[Channel]) ) {
   SIDwavOutput.FilterInput += ( ((int)WavGenOut-CRSID_WAVE_MID) * Envelope ) / ENVELOPE_MAGNITUDE_DIV; // >> 8;
  }
  else if ( MOSTLY (Channel!=14 || !(VolumeBand & OFF3_BITVAL)) ) {
   SIDwavOutput.NonFilted += ( ((int)WavGenOut-CRSID_WAVE_MID) * Envelope ) / ENVELOPE_MAGNITUDE_DIV; // >> 8;
  }

 }
 //update readable SID1-registers (some SID tunes might use 3rd channel ENV3/OSC3 value as control)
 SID->BasePtrRD[0x1B] /*cRSID_C64.IObankRD[SID->BaseAddress+0x1B]*/ = WavGenOut >> WAVE_OSC3_SHIFTS; //8; //OSC3, ENV3 (some players rely on it, unfortunately even for timing)
 SID->BasePtrRD[0x1C] /*cRSID_C64.IObankRD[SID->BaseAddress+0x1C]*/ = SID->EnvelopeCounter[CHANNEL2_INDEX]; //14]; //Envelope
 //cRSID_C64.IObankRD[SID->BaseAddress+0x1F] = (cRSID.SelectedSIDmodel==8580); //this doesn't exist in real SID but SID-Wizard code removes comment-marks and uses it as identification workaround

 //if (filter) {
  //cRSID_emulateHQresampledSIDfilter( SID );
  //SIDwavOutput.Mix = (SIDwavOutput.NonFilted + FilterOutput) * (VolumeBand & 0xF);
 //}

 return SIDwavOutput; //NonFilted; //+FilterInput; //WavGenOut; //(*PhaseAccuPtr)>>8;
}



