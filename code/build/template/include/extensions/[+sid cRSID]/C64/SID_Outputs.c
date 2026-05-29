
#include "SID_FilterCurves.h" //"SIDfilter.h"



static INLINE int cRSID_emulateSIDoutputStage (FASTVAR cRSID_SIDinstance *const FASTPTR SID) { //, FASTVAR char nofilter) {
 static enum { FRACTIONAL_BITS = 12, FRACTIONAL_SHIFTS = (FRACTIONAL_BITS) } Specs;
 static enum { /*CRSID_FILTERTABLE_RESOLUTION = 12,*/ CRSID_FILTERTABLE_SHIFTS = (CRSID_FILTERTABLE_RESOLUTION),
               CRSID_FILTERTABLE_MAGNITUDE = (1 << CRSID_FILTERTABLE_RESOLUTION) } FilterSpecs;

 static enum {
  CHANNELS=3+1, VOLUME_MAX=0xF, D418_DIGI_VOL=1 *16, D418_DIGI_MUL = (D418_DIGI_VOL / CRSID_WAVGEN_PREDIV), //digi-channel is counted too as full-volume 4th channel in attenuation/volume vs SIDcount
  SID_CUTOFF_BITS = 11, SID_CUTOFF_RANGE = (1 << SID_CUTOFF_BITS), SID_CUTOFF_MAX = (SID_CUTOFF_RANGE - 1)
 } SIDspecs;
 static enum { OFF3_BITVAL=0x80, HIGHPASS_BITVAL=0x40, BANDPASS_BITVAL=0x20, LOWPASS_BITVAL=0x10 } FilterBits;

 FASTVAR char MainVolume;
 FASTVAR unsigned char FilterSwitchReso, VolumeBand;
 FASTVAR int Tmp, NonFilted, FilterInput, Cutoff, Resonance, FilterOutput, Output;

 //cause immediate stopping in audio-buffer thread, so SID-baseaddress changes during tune-switching won't give segfaults
 //if ( RARELY (cRSID.Paused || SID->BasePtr == NULL) ) return 0; //avoid some segfaults when NULL-ing SID4

 FilterSwitchReso = SID->BasePtr[0x17]; VolumeBand=SID->BasePtr[0x18];
 Cutoff = (SID->BasePtr[0x16] << 3) + (SID->BasePtr[0x15] & 7);
 Resonance = FilterSwitchReso >> 4;

 NonFilted=SID->NonFiltedSample; FilterInput=SID->FilterInputSample;

 //Filter
 FilterOutput=0;
 //if ( CALMLY (!nofilter) ) {
  if ( MOSTLY (SID->ChipModel == 8580) ) {
   Cutoff = cRSID_CutoffMul8580_44100Hz[Cutoff];
   Resonance = cRSID_Resonances8580[Resonance];
  }
  else { //6581
   Cutoff += (FilterInput*105)>>16; //MOSFET-VCR control-voltage calculation (resistance-modulation aka 6581 filter distortion) emulation
    if ( RARELY (Cutoff > SID_CUTOFF_MAX) ) Cutoff=SID_CUTOFF_MAX; else if ( RARELY(Cutoff<0) ) Cutoff=0;  //can really go below 0 when FilterInput is negative
   Cutoff = cRSID_CutoffMul6581_44100Hz[Cutoff];
   Resonance = cRSID_Resonances6581[Resonance];
  }
  //shifting negative integers in C is implementation-dependent, so using normal division by power of 2, that might luckily be optimized as arithmetic-shift by the compiler
  Tmp = FilterInput + ( (SID->PrevBandPass * Resonance) / CRSID_FILTERTABLE_MAGNITUDE ) + SID->PrevLowPass; // >> CRSID_FILTERTABLE_SHIFTS ) + SID->PrevLowPass;
  if (VolumeBand & HIGHPASS_BITVAL) FilterOutput -= Tmp;
  Tmp = SID->PrevBandPass - ( (Tmp * Cutoff) / CRSID_FILTERTABLE_MAGNITUDE ); // >> CRSID_FILTERTABLE_SHIFTS ); //12 );
  SID->PrevBandPass = Tmp;
  if (VolumeBand & BANDPASS_BITVAL) FilterOutput -= Tmp;
  Tmp = SID->PrevLowPass + ( (Tmp * Cutoff) / CRSID_FILTERTABLE_MAGNITUDE ); // >> CRSID_FILTERTABLE_SHIFTS ); // 12 );
  SID->PrevLowPass = Tmp;
  if (VolumeBand & LOWPASS_BITVAL) FilterOutput += Tmp;
 //}

 //Output-mixing (main-volume / attenuator) stage

 //For $D418 volume-register digi playback: an AC / DC separation for $D418 value at low (20Hz or so) cutoff-frequency,
 //sending AC (highpass) value to a 4th 'digi' channel mixed to the master output, and set ONLY the DC (lowpass) value to the volume-control.
 //This solved 2 issues: Thanks to the lowpass filtering of the volume-control, SID tunes where digi is played together with normal SID channels,
 //won't sound distorted anymore, and the volume-clicks disappear when setting SID-volume. (This is useful for fade-in/out tunes like Hades Nebula, where clicking ruins the intro.)
 if ( TIGHTLY (cRSID_C64.RealSIDmode) ) {
  Tmp = (signed int) ( (VolumeBand&0xF) << FRACTIONAL_SHIFTS ); //12 );
  SID->Digi = (Tmp - SID->PrevVolume) * D418_DIGI_MUL; //highpass is digi, adding it to output must be before digifilter-code
  SID->PrevVolume += (Tmp - SID->PrevVolume) / 1024; // >> (FRACTIONAL_SHIFTS-2); //10; //arithmetic shift amount determines digi lowpass-frequency
  MainVolume = SID->PrevVolume >> FRACTIONAL_SHIFTS; //12; //lowpass is main volume
 }
 else MainVolume = VolumeBand & 0xF;

 SID->Output = (NonFilted+FilterOutput) * MainVolume + SID->Digi;

 //Output = SID->Output / cRSID_C64.Attenuation; //( (CHANNELS*VOLUME_MAX) + cRSID_C64.Attenuation ); //faster being done once outside, not in all individual SIDs

 return SID->Output; //Output; // master output of a SID
}




static INLINE void cRSID_precalculateHQoutputParameters (FASTVAR cRSID_SIDinstance *const FASTPTR SID) { //called by resampler at samplerate-pace
 static enum { FRACTIONAL_BITS = 12, FRACTIONAL_SHIFTS = (FRACTIONAL_BITS) } OutputStageSpecs;
 static enum { OFF3_BITVAL=0x80, HIGHPASS_BITVAL=0x40, BANDPASS_BITVAL=0x20, LOWPASS_BITVAL=0x10 } FilterBits;
 static enum { D418_DIGI_VOL=1 *16, D418_DIGI_MUL = (D418_DIGI_VOL / CRSID_WAVGEN_PREDIV), VOLUME_DIGI_SEPARATION_CUTOFF_DIV = (1 << 10) } SIDspecs;

 //cause immediate stopping in audio-buffer thread, so SID-baseaddress changes during tune-switching won't give segfaults
 //if ( RARELY (cRSID.Paused || SID->BasePtr == NULL) ) return; //0; //avoid some segfaults when NULL-ing SID4

 FASTVAR unsigned char VolumeBand; //, FilterSwitchReso;
 FASTVAR int Tmp, Cutoff, Resonance;

 VolumeBand = SID->BasePtr[0x18]; //FilterSwitchReso = SID->BasePtr[0x17];
 //Resonance = SID->BasePtr[0x17] >> 4; //FilterSwitchReso >> 4;
 Cutoff = (SID->BasePtr[0x16] << 3) + (SID->BasePtr[0x15] & 7);

 if ( MOSTLY (SID->ChipModel == 8580) ) {
  SID->Cutoff = cRSID_CutoffMul8580_OverSampleRate[ Cutoff ];
  SID->Resonance = cRSID_Resonances8580[ SID->BasePtr[0x17] >> 4 ];
 }
 else { //6581
  SID->Cutoff = cRSID_CutoffMul6581_OverSampleRate[ Cutoff ];
  SID->Resonance = cRSID_Resonances6581[ SID->BasePtr[0x17] >> 4 ];
 }

 SID->HighPassBit = VolumeBand & HIGHPASS_BITVAL;
 SID->BandPassBit = VolumeBand & BANDPASS_BITVAL;
 SID->LowPassBit = VolumeBand & LOWPASS_BITVAL;

 //For $D418 volume-register digi playback: an AC / DC separation for $D418 value at low (20Hz or so) cutoff-frequency,
 //sending AC (highpass) value to a 4th 'digi' channel mixed to the master output, and set ONLY the DC (lowpass) value to the volume-control.
 //This solved 2 issues: Thanks to the lowpass filtering of the volume-control, SID tunes where digi is played together with normal SID channels,
 //won't sound distorted anymore, and the volume-clicks disappear when setting SID-volume. (This is useful for fade-in/out tunes like Hades Nebula, where clicking ruins the intro.)
 if ( TIGHTLY (cRSID_C64.RealSIDmode) ) {
  Tmp = (signed int) ( (VolumeBand & 0xF) << FRACTIONAL_SHIFTS ); //12 );
  SID->Digi = (Tmp - SID->PrevVolume) * D418_DIGI_MUL; //highpass is digi, adding it to output must be before digifilter-code
  SID->PrevVolume += (Tmp - SID->PrevVolume) / VOLUME_DIGI_SEPARATION_CUTOFF_DIV; /// 1024; // >> (FRACTIONAL_SHIFTS-2); //10; //arithmetic shift amount determines digi lowpass-frequency
  SID->Volume = SID->PrevVolume >> FRACTIONAL_SHIFTS; //12; //lowpass is main volume
 }
 else  SID->Volume = VolumeBand & 0xF;
}



static INLINE int cRSID_emulateHQresampledSIDoutputStage (FASTVAR cRSID_SIDinstance *const FASTPTR SID, FASTVAR cRSID_SIDwavOutput waves) { //called by resampler at oversample-rate
 static enum { //FRACTIONAL_BITS = 12, FRACTIONAL_SHIFTS = (FRACTIONAL_BITS),
  CRSID_FILTERTABLE_SHIFTS = (CRSID_FILTERTABLE_RESOLUTION), CRSID_FILTERTABLE_MAGNITUDE = (1 << CRSID_FILTERTABLE_RESOLUTION),
  /*CRSID_OVERSAMPLING_FILTERTABLE_RESOLUTION = 12,*/ CRSID_OVERSAMPLING_FILTERTABLE_SHIFTS = (CRSID_OVERSAMPLING_FILTERTABLE_RESOLUTION),
  CRSID_OVERSAMPLING_FILTERTABLE_MAGNITUDE = (1 << CRSID_OVERSAMPLING_FILTERTABLE_RESOLUTION),
  //HQ_6581_CUTOFF_MAX = 0xA82 //0x1505 //0x541 //cutoff-max from HQ-filter oversampled cutoff-table
 } FilterSpecs;

 //cause immediate stopping in audio-buffer thread, so SID-baseaddress changes during tune-switching won't give segfaults
 //if ( RARELY (cRSID.Paused || SID->BasePtr == NULL) ) return 0; //avoid some segfaults when NULL-ing SID4

 FASTVAR int Tmp; //, FilterInput;
 FASTVAR int FilterOutput, Cutoff;

 //FilterInput = waves->FilterInput;

 if ( MOSTLY (SID->ChipModel == 8580) ) Cutoff = SID->Cutoff;
 else {
  Cutoff = SID->Cutoff + (waves.FilterInput/1024); //MOSFET-VCR control-voltage calculation (resistance-modulation aka 6581 filter distortion) emulation
  /*if ( RARELY (Cutoff > HQ_6581_CUTOFF_MAX) ) Cutoff=HQ_6581_CUTOFF_MAX; else*/ if ( RARELY(Cutoff<0) ) Cutoff=0;  //can really go below 0 when FilterInput is negative
 }

 FilterOutput = 0; //shifting negative integers in C is implementation-dependent, so using normal division by power of 2, that might luckily be optimized as arithmetic-shift by the compiler
 Tmp = waves.FilterInput + ( (SID->PrevBandPass * SID->Resonance) / CRSID_FILTERTABLE_MAGNITUDE ) + SID->PrevLowPass; // >> CRSID_FILTERTABLE_SHIFTS ) + SID->PrevLowPass;
 if (SID->HighPassBit) FilterOutput -= Tmp;
 Tmp = SID->PrevBandPass - ( (Tmp * Cutoff) / CRSID_OVERSAMPLING_FILTERTABLE_MAGNITUDE ); // >> CRSID_OVERSAMPLING_FILTERTABLE_SHIFTS );
 SID->PrevBandPass = Tmp;
 if (SID->BandPassBit) FilterOutput -= Tmp;
 Tmp = SID->PrevLowPass + ( (Tmp * Cutoff) / CRSID_OVERSAMPLING_FILTERTABLE_MAGNITUDE ); // >> CRSID_OVERSAMPLING_FILTERTABLE_SHIFTS );
 SID->PrevLowPass = Tmp;
 if (SID->LowPassBit) FilterOutput += Tmp;

 return (waves.NonFilted + FilterOutput) * SID->Volume;
}



static INLINE void cRSID_emulateHQresampledSIDdigi (FASTVAR cRSID_SIDinstance *const FASTPTR SID, FASTVAR cRSID_Output *const FASTPTR signal) { //called by resampler at samplerate-pace, only digis
 //static enum { FRACTIONAL_BITS = 12, FRACTIONAL_SHIFTS = (FRACTIONAL_BITS) } Specs;
 //static enum { D418_DIGI_VOLUME=2 } SIDspecs;

 //FASTVAR unsigned char VolumeBand;
 //FASTVAR int Tmp, Digi; //, Output;

 //cause immediate stopping in audio-buffer thread, so SID-baseaddress changes during tune-switching won't give segfaults
 //if ( RARELY (cRSID.Paused || SID->BasePtr == NULL) ) return; //0; //avoid some segfaults when NULL-ing SID4

 //VolumeBand=SID->BasePtr[0x18];

 //Output-mixing (attenuator) stage

 //For $D418 volume-register digi playback: an AC / DC separation for $D418 value at low (20Hz or so) cutoff-frequency,
 //sending AC (highpass) value to a 4th 'digi' channel mixed to the master output, and set ONLY the DC (lowpass) value to the volume-control.
 //This solved 2 issues: Thanks to the lowpass filtering of the volume-control, SID tunes where digi is played together with normal SID channels,
 //won't sound distorted anymore, and the volume-clicks disappear when setting SID-volume. (This is useful for fade-in/out tunes like Hades Nebula, where clicking ruins the intro.)
 if ( TIGHTLY (cRSID_C64.RealSIDmode) ) { //only processing digi here
  /*Tmp = (signed int) ( (VolumeBand&0xF) << FRACTIONAL_SHIFTS ); //12 );
  Digi = (Tmp - SID->PrevVolume) * D418_DIGI_VOLUME * (SID->PrevVolume >> FRACTIONAL_SHIFTS); //S12); //highpass is digi, adding it to output must be before digifilter-code
  SID->PrevVolume += (Tmp - SID->PrevVolume) >> 10; //arithmetic shift amount determines digi lowpass-frequency*/

  if ( RARELY (SID->Channel == CRSID_CHANNEL_LEFT) ) signal->L += SID->Digi * 2;
  else if ( RARELY (SID->Channel == CRSID_CHANNEL_RIGHT) ) signal->R += SID->Digi * 2;
  else { signal->L += SID->Digi; signal->R += SID->Digi; }
 }

}


