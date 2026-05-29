
//C64 emulation (SID-playback related)


#include "../libcRSID.h"

#include "MEM.c"
#include "CPU.c"
#include "CIA.c"
#include "VIC.c"
#include "SID.c"

#include "C64_SIDrouting.c"



cRSID_C64instance* cRSID_createC64 (unsigned short samplerate) { //init a basic PAL C64 instance
 //static cRSID_C64instance* C64 = &cRSID_C64;

 //enum C64clocks { C64_PAL_CPUCLK=985248, DEFAULT_SAMPLERATE=44100 };
 static enum { VOLUME_MAX=0xF, CHANNELS=3+1, SID_FULLVOLUME = (CHANNELS*VOLUME_MAX) /*64*/ } SIDspecs; //digi-channel is counted too in attenuation

 if(samplerate) cRSID_C64.SampleRate = samplerate;
 else cRSID_C64.SampleRate = samplerate = CRSID_DEFAULT_SAMPLERATE;
 cRSID_C64.SampleClockRatio = (CRSID_PAL_CPUCLK << CRSID_CLOCK_FRACTIONAL_BITS) / samplerate; //shifting (multiplication) enhances SampleClockRatio precision
 cRSID_C64.OversampleClockRatio = (samplerate << CRSID_RESAMPLER_FRACTIONAL_BITS) / CRSID_PAL_AUDIO_CLOCK; //round( SID_AUDIO_CLOCK / C64->SampleRate );
 cRSID_C64.OversampleClockRatioReciproc = CRSID_PAL_AUDIO_CLOCK / cRSID_C64.SampleRate; //round?

 cRSID_C64.SIDchipCount=cRSID_C64.AudioThread_SIDchipCount=1; //init audio-thread's version as well
 cRSID_C64.Attenuation = ((SID_FULLVOLUME+26) * CRSID_PRESAT_ATT_NOM) / (CRSID_PRESAT_ATT_DENOM * CRSID_WAVGEN_PREDIV);
 //cRSID_C64.CPU.C64 = C64;
 cRSID_createSIDchip ( &cRSID_C64.SID[1], 8580, CRSID_CHANNEL_BOTH, 0xD400 ); //default C64 setup with only 1 SID and 2 CIAs and 1 VIC
 cRSID_createCIAchip ( &cRSID_C64.CIA[1], 0xDC00 );
 cRSID_createCIAchip ( &cRSID_C64.CIA[2], 0xDD00 );
 cRSID_createVICchip ( 0xD000 );
 cRSID_generateMemoryBankPointers();
 //if(cRSID.RealSIDmode) {
  cRSID_setROMcontent();
 //}
 cRSID_initC64();
 return &cRSID_C64;
}



void cRSID_setSIDmodels () { //based on SIDheader-data
 short SIDmodel;

 SIDmodel = (cRSID.SIDheader->ModelFormatStandard&0x30) >= 0x20 ? 8580:6581;
 cRSID_C64.SID[1].ChipModel = cRSID.SelectedSIDmodel? cRSID.SelectedSIDmodel : SIDmodel;

 if (cRSID.SIDheader->Version != CRSID_FILEVERSION_WEBSID) {
  SIDmodel = cRSID.SIDheader->ModelFormatStandard & 0xC0;
  if (SIDmodel) SIDmodel = (SIDmodel >= 0x80) ? 8580:6581; else SIDmodel = cRSID_C64.SID[1].ChipModel;
  if (cRSID.SelectedSIDmodel) SIDmodel = cRSID.SelectedSIDmodel;
  cRSID_C64.SID[2].ChipModel = SIDmodel;

  SIDmodel = cRSID.SIDheader->ModelFormatStandardH & 0x03;
  if (SIDmodel) SIDmodel = (SIDmodel >= 0x02) ? 8580:6581; else SIDmodel = cRSID_C64.SID[1].ChipModel;
  if (cRSID.SelectedSIDmodel) SIDmodel = cRSID.SelectedSIDmodel;
  cRSID_C64.SID[3].ChipModel = SIDmodel;
 }
 else {
  SIDmodel = cRSID.SIDheader->SID2flagsL & 0x30;
  if (SIDmodel) SIDmodel = (SIDmodel >= 0x20) ? 8580:6581; else SIDmodel = cRSID_C64.SID[1].ChipModel;
  if (cRSID.SelectedSIDmodel) SIDmodel = cRSID.SelectedSIDmodel;
  cRSID_C64.SID[2].ChipModel = SIDmodel;

  SIDmodel = cRSID.SIDheader->SID3flagsL & 0x30;
  if (SIDmodel) SIDmodel = (SIDmodel >= 0x20) ? 8580:6581; else SIDmodel = cRSID_C64.SID[1].ChipModel;
  if (cRSID.SelectedSIDmodel) SIDmodel = cRSID.SelectedSIDmodel;
  cRSID_C64.SID[3].ChipModel = SIDmodel;

  SIDmodel = cRSID.SIDheader->SID4flagsL & 0x30;
  if (SIDmodel) SIDmodel = (SIDmodel >= 0x20) ? 8580:6581; else SIDmodel = cRSID_C64.SID[1].ChipModel;
  if (cRSID.SelectedSIDmodel) SIDmodel = cRSID.SelectedSIDmodel;
  cRSID_C64.SID[4].ChipModel = SIDmodel;
 }
}


void cRSID_setC64 () {   //set hardware-parameters (Models, SIDs) for playback of loaded SID-tune
 //static cRSID_C64instance* C64 = &cRSID_C64;

 //enum C64clocks { C64_PAL_CPUCLK=985248, C64_NTSC_CPUCLK=1022727 };
 enum C64scanlines { C64_PAL_SCANLINES = 312, C64_NTSC_SCANLINES = 263 };
 enum C64scanlineCycles { C64_PAL_SCANLINE_CYCLES = 63, C64_NTSC_SCANLINE_CYCLES = 65 };
 //enum C64framerates { PAL_FRAMERATE = 50, NTSC_FRAMERATE = 60 }; //Hz
 static enum { VOLUME_MAX=0xF, CHANNELS=3+1, SID_FULLVOLUME = (CHANNELS*VOLUME_MAX) /*64*/ } SIDspecs; //digi-channel is counted too in attenuation

 static const unsigned int CPUspeeds[] = { CRSID_NTSC_CPUCLK, CRSID_PAL_CPUCLK };
 static const unsigned short ScanLines[] = { C64_NTSC_SCANLINES, C64_PAL_SCANLINES };
 static const unsigned char ScanLineCycles[] = { C64_NTSC_SCANLINE_CYCLES, C64_PAL_SCANLINE_CYCLES };
 //unsigned char FrameRates[] = { NTSC_FRAMERATE, PAL_FRAMERATE };
 static const short Attenuations[]={ //manually tweaked attenuation, increase for 2SID (to 43) and 3SID (to 137) and 4SID (200)
  ((SID_FULLVOLUME+0  ) * CRSID_PRESAT_ATT_NOM) / (CRSID_PRESAT_ATT_DENOM * CRSID_WAVGEN_PREDIV), //CRSID_WAVGEN_PREDIV compensates for dividing by it in envelope-multiplier
  ((SID_FULLVOLUME+26 ) * CRSID_PRESAT_ATT_NOM) / (CRSID_PRESAT_ATT_DENOM * CRSID_WAVGEN_PREDIV),  //(64+26) =  90  *28/(16*1..16)=157..9
  ((SID_FULLVOLUME+43 ) * CRSID_PRESAT_ATT_NOM) / (CRSID_PRESAT_ATT_DENOM * CRSID_WAVGEN_PREDIV),  //(64+43) = 107  *28/(16*1..16)=187..11
  ((SID_FULLVOLUME+137) * CRSID_PRESAT_ATT_NOM) / (CRSID_PRESAT_ATT_DENOM * CRSID_WAVGEN_PREDIV), //(64+137) = 201  *28/(16*1..16)=351..21
  ((SID_FULLVOLUME+200) * CRSID_PRESAT_ATT_NOM) / (CRSID_PRESAT_ATT_DENOM * CRSID_WAVGEN_PREDIV)  //(64+200) = 264  *28/(16*1..16)=462..28
 };
 /*short SIDmodel;*/ char SIDchannel;

 cRSID.VideoStandard = ( (cRSID.SIDheader->ModelFormatStandard & 0x0C) >> 2 ) != 2;
 if (cRSID_C64.SampleRate==0) cRSID_C64.SampleRate = 44100;
 cRSID_C64.CPUfrequency = CPUspeeds[ cRSID.VideoStandard ];
 cRSID_C64.SampleClockRatio = (cRSID_C64.CPUfrequency << CRSID_CLOCK_FRACTIONAL_BITS) / cRSID_C64.SampleRate; //shifting (multiplication) enhances SampleClockRatio precision
 cRSID_C64.OversampleClockRatio = (cRSID_C64.SampleRate << CRSID_RESAMPLER_FRACTIONAL_BITS) / CRSID_PAL_AUDIO_CLOCK; //round( SID_AUDIO_CLOCK / C64->SampleRate );
 cRSID_C64.OversampleClockRatioReciproc = CRSID_PAL_AUDIO_CLOCK / cRSID_C64.SampleRate; //round?

 cRSID_C64.VIC.RasterLines = ScanLines[ cRSID.VideoStandard ];
 cRSID_C64.VIC.RasterRowCycles = ScanLineCycles[ cRSID.VideoStandard ];
 cRSID.FrameCycles = cRSID_C64.VIC.RasterLines * cRSID_C64.VIC.RasterRowCycles; ///cRSID_C64.SampleRate / PAL_FRAMERATE; //1x speed tune with VIC Vertical-blank timing

 cRSID_C64.PrevRasterLine=-1; //so if $d012 is set once only don't disturb FrameCycleCnt

 cRSID_setSIDmodels();

 if (cRSID.SIDheader->Version != CRSID_FILEVERSION_WEBSID) {
  cRSID_C64.SID[1].Channel = CRSID_CHANNEL_LEFT;

  cRSID_createSIDchip( &cRSID_C64.SID[2], cRSID_C64.SID[2].ChipModel, CRSID_CHANNEL_RIGHT, 0xD000 + cRSID.SIDheader->SID2baseAddress*16 );

  cRSID_createSIDchip( &cRSID_C64.SID[3], cRSID_C64.SID[3].ChipModel, CRSID_CHANNEL_BOTH, 0xD000 + cRSID.SIDheader->SID3baseAddress*16 );

 //ensure disabling SID4 in non-WebSID format:  //(NULL-ing not preferred as it can overwrite stuff and cause Segfault in sample-thread)
  cRSID_C64.SID[4].BaseAddress=0x0000;
  cRSID_C64.SID[4].BasePtr = cRSID_C64.SID[4].BasePtrRD = &cRSID_C64.IObankWR[CRSID_SID_SAFE_ADDRESS]; //NULL;

 }
 else {
  cRSID_C64.SID[1].Channel = (cRSID.SIDheader->ModelFormatStandardH & 0x40)? CRSID_CHANNEL_RIGHT:CRSID_CHANNEL_LEFT;
  if (cRSID.SIDheader->ModelFormatStandardH & 0x80) cRSID_C64.SID[1].Channel = CRSID_CHANNEL_BOTH; //my own proposal for 'middle' channel

  SIDchannel = (cRSID.SIDheader->SID2flagsL & 0x40) ? CRSID_CHANNEL_RIGHT:CRSID_CHANNEL_LEFT;
  if (cRSID.SIDheader->SID2flagsL & 0x80) SIDchannel = CRSID_CHANNEL_BOTH;
  cRSID_createSIDchip ( &cRSID_C64.SID[2], cRSID_C64.SID[2].ChipModel, SIDchannel, 0xD000 + cRSID.SIDheader->SID2baseAddress*16 );

  SIDchannel = (cRSID.SIDheader->SID3flagsL & 0x40) ? CRSID_CHANNEL_RIGHT:CRSID_CHANNEL_LEFT;
  if (cRSID.SIDheader->SID3flagsL & 0x80) SIDchannel = CRSID_CHANNEL_BOTH;
  cRSID_createSIDchip ( &cRSID_C64.SID[3], cRSID_C64.SID[3].ChipModel, SIDchannel, 0xD000 + cRSID.SIDheader->SID3flagsH*16 );

  SIDchannel = (cRSID.SIDheader->SID4flagsL & 0x40) ? CRSID_CHANNEL_RIGHT:CRSID_CHANNEL_LEFT;
  if (cRSID.SIDheader->SID4flagsL & 0x80) SIDchannel = CRSID_CHANNEL_BOTH;
  cRSID_createSIDchip ( &cRSID_C64.SID[4], cRSID_C64.SID[4].ChipModel, SIDchannel, 0xD000 + cRSID.SIDheader->SID4baseAddress*16 );
 }

 cRSID_C64.SIDchipCount = 1 + (cRSID_C64.SID[2].BaseAddress > 0) + (cRSID_C64.SID[3].BaseAddress > 0) + (cRSID_C64.SID[4].BaseAddress > 0);
 if (cRSID_C64.SIDchipCount == 1) cRSID_C64.SID[1].Channel = CRSID_CHANNEL_BOTH;
 cRSID_C64.Attenuation = Attenuations[cRSID_C64.SIDchipCount];
}



void cRSID_initC64 () { //C64 Reset
 enum { C64_RESET_VECTOR = 0xFFFC };
 //static cRSID_C64instance* C64 = &cRSID_C64;

 cRSID_initSIDchip( &cRSID_C64.SID[1] );
 cRSID_initCIAchip( &cRSID_C64.CIA[1] ); cRSID_initCIAchip( &cRSID_C64.CIA[2] );
 /*cRSID_setROMcontent();*/ cRSID_initMem();
 cRSID_initCPU( (cRSID_readMem(C64_RESET_VECTOR+1)<<8) + cRSID_readMem(C64_RESET_VECTOR) ); //cRSID_initCPU( &cRSID_C64.CPU, (cRSID_readMemC64(C64,0xFFFD)<<8) + cRSID_readMemC64(C64,0xFFFC) );
 cRSID_C64.IRQ = cRSID_C64.NMI = 0;
 if (cRSID.HighQualitySID) {
  cRSID_C64.SID[1].NonFiltedSample = cRSID_C64.SID[1].FilterInputSample = 0;
  cRSID_C64.SID[2].NonFiltedSample = cRSID_C64.SID[2].FilterInputSample = 0;
  cRSID_C64.SID[3].NonFiltedSample = cRSID_C64.SID[3].FilterInputSample = 0;
  cRSID_C64.SID[4].NonFiltedSample = cRSID_C64.SID[4].FilterInputSample = 0;
  cRSID_C64.SID[1].PrevNonFiltedSample = cRSID_C64.SID[1].PrevFilterInputSample = 0;
  cRSID_C64.SID[2].PrevNonFiltedSample = cRSID_C64.SID[2].PrevFilterInputSample = 0;
  cRSID_C64.SID[3].PrevNonFiltedSample = cRSID_C64.SID[3].PrevFilterInputSample = 0;
  cRSID_C64.SID[4].PrevNonFiltedSample = cRSID_C64.SID[4].PrevFilterInputSample = 0;
 }
 cRSID_C64.SampleCycleCnt = cRSID_C64.OverSampleCycleCnt = 0;
}



cRSID_Output* cRSID_emulateC64 () {
 //static cRSID_C64instance* C64 = &cRSID_C64;

 //static enum { VOLUME_MAX=0xF, CHANNELS=3+1 } SIDspecs; //digi-channel is counted too in attenuation
 //enum C64clocks { C64_PAL_CPUCLK=985248, DEFAULT_SAMPLERATE=44100 };
 //enum Oversampling { OVERSAMPLING_RATIO=5, OVERSAMPLING_CYCLES = ((C64_PAL_CPUCLK/DEFAULT_SAMPLERATE)/OVERSAMPLING_RATIO) };
 static enum { VUMETER_LOWPASS_DIV = 16, VUMETER_DIVSHIFTS = (4 - CRSID_WAVGEN_PRESHIFT) } VUmeterParameters;

 FASTVAR unsigned char InstructionCycles;
 static unsigned char VUmeterUpdateCounter;
 static cRSID_Output* Output;


 //Cycle-based/-paced part of emulations:


 while (cRSID_C64.SampleCycleCnt <= cRSID_C64.SampleClockRatio) {

  if ( CALMLY (!cRSID_C64.RealSIDmode) ) {
   if ( RARELY (cRSID_C64.FrameCycleCnt >= cRSID.FrameCycles) ) {
    cRSID_C64.FrameCycleCnt -= cRSID.FrameCycles;
    if ( RARELY (cRSID_C64.Finished) ) { //some tunes (e.g. Barbarian, A-Maze-Ing) don't always finish in 1 frame
     cRSID_initCPU( cRSID.PlayAddress ); //(PSID docs say bank-register should always be set for each call's region)
     cRSID_C64.Finished=0; //cRSID_C64.SampleCycleCnt=0; //PSID workaround for some tunes (e.g. Galdrumway):
     if ( LIKELY (cRSID.TimerSource==0) ) cRSID_C64.IObankRD[0xD019] = 0x81; //always simulate to player-calls that VIC-IRQ happened
     else cRSID_C64.IObankRD[0xDC0D] = 0x83; //always simulate to player-calls that CIA TIMERA/TIMERB-IRQ happened
   }}
   if ( TIGHTLY (cRSID_C64.Finished==0) ) {
    if ( RARELY ((InstructionCycles = cRSID_emulateCPU()) >= 0xFE) ) { InstructionCycles=6; cRSID_C64.Finished=1; }
   }
   else InstructionCycles=7; //idle between player-calls
   cRSID_C64.FrameCycleCnt += InstructionCycles;
   cRSID_C64.IObankRD[0xDC04] += InstructionCycles; //very simple CIA1 TimerA simulation for PSID (e.g. Delta-Mix_E-Load_loader)
  }

  else { //RealSID emulations:
   if ( RARELY (cRSID_handleCPUinterrupts()) ) { cRSID_C64.Finished=0; InstructionCycles=7; }
   else if ( MOSTLY (cRSID_C64.Finished==0) ) {
    if ( RARELY ((InstructionCycles = cRSID_emulateCPU()) >= 0xFE) ) {
     /*if (InstructionCycles!=0xFE && !(cRSID_C64.CPU.ST&I))*/ cRSID_C64.Finished=1;
     InstructionCycles=6;
    }
   }
   else InstructionCycles=7; //idle between IRQ-calls
   cRSID_C64.IRQ = cRSID_C64.NMI = 0; //prepare for collecting IRQ sources
   cRSID_C64.IRQ |= cRSID_emulateCIA( &cRSID_C64.CIA[1], InstructionCycles );
   cRSID_C64.NMI |= cRSID_emulateCIA( &cRSID_C64.CIA[2], InstructionCycles );
   cRSID_C64.IRQ |= cRSID_emulateVIC( InstructionCycles );
  }

  cRSID_C64.SampleCycleCnt += (InstructionCycles << CRSID_CLOCK_FRACTIONAL_BITS);

  cRSID_emulateADSRs( &cRSID_C64.SID[1], InstructionCycles );
  if (cRSID_C64.SID[2].BaseAddress != 0) cRSID_emulateADSRs( &cRSID_C64.SID[2], InstructionCycles );
  if (cRSID_C64.SID[3].BaseAddress != 0) cRSID_emulateADSRs( &cRSID_C64.SID[3], InstructionCycles );
  if (cRSID_C64.SID[4].BaseAddress != 0) cRSID_emulateADSRs( &cRSID_C64.SID[4], InstructionCycles );

 } //end of 1MHz cycle-based emulations (CPU, VIC, CIA, ADSR)
 cRSID_C64.SampleCycleCnt -= cRSID_C64.SampleClockRatio;


 if ( TIGHTLY (cRSID_C64.HighQualitySID) ) { //oversampled waveform-generation (although delayed ~22 cycles (~5 instructions) compared to CPU, shouldn't cause many issues (apart from cycle-exact SID-routines reading OSC3))
  if ( TIGHTLY (cRSID_C64.HighQualityResampler) ) cRSID_emulateHQresampledSIDs(); //high-quality but more CPU-hungry Sinc-based resampler (decimator)
  else cRSID_emulateOversampledSIDwaves(); //fast simple (but lower-quality) averager (box-filter) resampler
 }


 //Samplerate-based/-paced part of emulations:


 if ( CALMLY (!cRSID_C64.RealSIDmode) ) { //some PSID tunes use CIA TOD-clock (e.g. Kawasaki Synthesizer Demo)
  --cRSID_C64.TenthSecondCnt;
  if ( RARELY (cRSID_C64.TenthSecondCnt <= 0) ) {
   cRSID_C64.TenthSecondCnt = cRSID_C64.SampleRate / 10;
   ++( cRSID_C64.IObankRD[0xDC08] );
   if ( RARELY (cRSID_C64.IObankRD[0xDC08] >= 10) ) {
    cRSID_C64.IObankRD[0xDC08] = 0; ++( cRSID_C64.IObankRD[0xDC09] );
    //if(cRSID_C64.IObankRD[0xDC09]%
 }}}
 if ( MOSTLY (cRSID_C64.SecondCnt < cRSID_C64.SampleRate) ) ++cRSID_C64.SecondCnt;
 else { cRSID_C64.SecondCnt = 0; if ( MOSTLY (cRSID.PlayTime<3600) ) ++cRSID.PlayTime; }


 if ( TIGHTLY (cRSID_C64.HighQualitySID) ) { //SID output-stages and mono/stereo handling for High-Quality SID-emulation
  Output = TIGHTLY (cRSID_C64.HighQualityResampler) ? cRSID_emulateHQresampledSIDoutputs() : cRSID_emulateOversampledSIDoutputs();
 }
 else Output = cRSID_emulateLightSIDs(); //with special lightweight waveform-antialiasing code

 //Output.L /= cRSID_C64.Attenuation //( (CHANNELS*VOLUME_MAX) + cRSID_C64.Attenuation ); //* cRSID_C64.AudioThread_SIDchipCount;
 //Output.R /= cRSID_C64.Attenuation //( (CHANNELS*VOLUME_MAX) + cRSID_C64.Attenuation ); //* cRSID_C64.AudioThread_SIDchipCount;


 if ( RARELY (!cRSID.CLImode && !++VUmeterUpdateCounter) ) {
  //average level (for VU-meter)
  cRSID_C64.SID[1].Level += ( (abs(cRSID_C64.SID[1].Output)>>VUMETER_DIVSHIFTS) - cRSID_C64.SID[1].Level ) / VUMETER_LOWPASS_DIV; //16; //4; //1024;
  if ( TIGHTLY (cRSID_C64.SID[2].BaseAddress != 0) )
   cRSID_C64.SID[2].Level += ( (abs(cRSID_C64.SID[2].Output)>>VUMETER_DIVSHIFTS) - cRSID_C64.SID[2].Level ) / VUMETER_LOWPASS_DIV; //16; //1024;
  if ( TIGHTLY (cRSID_C64.SID[3].BaseAddress != 0) )
   cRSID_C64.SID[3].Level += ( (abs(cRSID_C64.SID[3].Output)>>VUMETER_DIVSHIFTS) - cRSID_C64.SID[3].Level ) / VUMETER_LOWPASS_DIV; //16; //1024;
  if ( TIGHTLY (cRSID_C64.SID[4].BaseAddress != 0) )
   cRSID_C64.SID[4].Level += ( (abs(cRSID_C64.SID[4].Output)>>VUMETER_DIVSHIFTS) - cRSID_C64.SID[4].Level ) / VUMETER_LOWPASS_DIV; //16; //1024;
 }

 return Output;
}



