


static INLINE cRSID_Output* cRSID_emulateLightSIDs () { //lightweight (all at samplerate-pace) SID-emulations: oscillators, waveforms, filter and complete output stages
 //static enum { VOLUME_MAX=0xF, CHANNELS=3+1 } SIDspecs; //digi-channel is counted too in attenuation

 FASTVAR signed int Tmp, Tmp2;
 static cRSID_Output Output;

 switch (cRSID_C64.Stereo) {
  case CRSID_CHANNELMODE_MONO: { //if ( MOSTLY (cRSID_C64.Stereo==CRSID_CHANNELMODE_MONO /*|| cRSID_C64.AudioThread_SIDchipCount==1*/) ) { //mono
   Output.L /*= Output.R*/ = cRSID_emulateSID_light( &cRSID_C64.SID[1] );
   if ( TIGHTLY (cRSID_C64.SID[2].BaseAddress != 0) ) Output.L += cRSID_emulateSID_light( &cRSID_C64.SID[2] );
   if ( TIGHTLY (cRSID_C64.SID[3].BaseAddress != 0) ) Output.L += cRSID_emulateSID_light( &cRSID_C64.SID[3] );
   if ( TIGHTLY (cRSID_C64.SID[4].BaseAddress != 0) ) Output.L += cRSID_emulateSID_light( &cRSID_C64.SID[4] );
   Output.R = Output.L /= cRSID_C64.Attenuation; //( (CHANNELS*VOLUME_MAX) + cRSID_C64.Attenuation );  // * cRSID_C64.AudioThread_SIDchipCount; //better being multiplied in a single place than for all SIDs
  } break;
  case CRSID_CHANNELMODE_STEREO: { //else { //stereo
   Tmp = cRSID_emulateSID_light( &cRSID_C64.SID[1] );
   if ( RARELY (cRSID_C64.SID[1].Channel == CRSID_CHANNEL_LEFT) )  { Output.L = Tmp * 2; Output.R=0; }
   else if ( RARELY (cRSID_C64.SID[1].Channel == CRSID_CHANNEL_RIGHT) ) { Output.R = Tmp * 2; Output.L=0; }
   else Output.L = Output.R = Tmp;
   if ( TIGHTLY (cRSID_C64.SID[2].BaseAddress != 0) ) {
    Tmp = cRSID_emulateSID_light( &cRSID_C64.SID[2] );
    if (cRSID_C64.SID[2].Channel == CRSID_CHANNEL_LEFT)  Output.L += Tmp * 2;
    else if (cRSID_C64.SID[2].Channel == CRSID_CHANNEL_RIGHT) Output.R += Tmp * 2;
    else { Output.L += Tmp; Output.R += Tmp; }
   }
   if ( TIGHTLY (cRSID_C64.SID[3].BaseAddress != 0) ) {
    Tmp = cRSID_emulateSID_light( &cRSID_C64.SID[3] );
    if ( UNLIKELY (cRSID_C64.SID[3].Channel == CRSID_CHANNEL_LEFT) )  Output.L += Tmp * 2;
    else if ( UNLIKELY (cRSID_C64.SID[3].Channel == CRSID_CHANNEL_RIGHT) ) Output.R += Tmp * 2;
    else { Output.L += Tmp; Output.R += Tmp; }
   }
   if ( TIGHTLY (cRSID_C64.SID[4].BaseAddress != 0) ) {
    Tmp = cRSID_emulateSID_light( &cRSID_C64.SID[4] );
    if (cRSID_C64.SID[4].Channel == CRSID_CHANNEL_LEFT)  Output.L += Tmp * 2;
    else if (cRSID_C64.SID[4].Channel == CRSID_CHANNEL_RIGHT) Output.R += Tmp * 2;
    else { Output.L += Tmp; Output.R += Tmp; }
   }
   Output.L /= cRSID_C64.Attenuation; //( (CHANNELS*VOLUME_MAX) + cRSID_C64.Attenuation );  // * cRSID_C64.AudioThread_SIDchipCount;
   Output.R /= cRSID_C64.Attenuation; //( (CHANNELS*VOLUME_MAX) + cRSID_C64.Attenuation );  // * cRSID_C64.AudioThread_SIDchipCount;
  } break;
  case CRSID_CHANNELMODE_NARROW: {
   Tmp = cRSID_emulateSID_light( &cRSID_C64.SID[1] );
   if ( RARELY (cRSID_C64.SID[1].Channel == CRSID_CHANNEL_LEFT) )  //Output.L = Tmp * 2;
   { Tmp2 = Tmp >> CRSID_CHANNELPANNING_DIVSHIFTS;  Output.L = Tmp + Tmp2; Output.R = Tmp - Tmp2; }
   else if ( RARELY (cRSID_C64.SID[1].Channel == CRSID_CHANNEL_RIGHT) )  //Output.R = Tmp * 2;
   { Tmp2 = Tmp >> CRSID_CHANNELPANNING_DIVSHIFTS;  Output.R = Tmp + Tmp2; Output.L = Tmp - Tmp2; }
   else Output.L = Output.R = Tmp;
   if ( TIGHTLY (cRSID_C64.SID[2].BaseAddress != 0) ) {
    Tmp = cRSID_emulateSID_light( &cRSID_C64.SID[2] );
    if (cRSID_C64.SID[2].Channel == CRSID_CHANNEL_LEFT)  //Output.L += Tmp * 2;
    { Tmp2 = Tmp >> CRSID_CHANNELPANNING_DIVSHIFTS;  Output.L += Tmp + Tmp2; Output.R += Tmp - Tmp2; }
    else if (cRSID_C64.SID[2].Channel == CRSID_CHANNEL_RIGHT)  //Output.R += Tmp * 2;
    { Tmp2 = Tmp >> CRSID_CHANNELPANNING_DIVSHIFTS;  Output.R += Tmp + Tmp2; Output.L += Tmp - Tmp2; }
    else { Output.L += Tmp; Output.R += Tmp; }
   }
   if ( TIGHTLY (cRSID_C64.SID[3].BaseAddress != 0) ) {
    Tmp = cRSID_emulateSID_light( &cRSID_C64.SID[3] );
    if ( UNLIKELY (cRSID_C64.SID[3].Channel == CRSID_CHANNEL_LEFT) )  //Output.L += Tmp * 2;
    { Tmp2 = Tmp >> CRSID_CHANNELPANNING_DIVSHIFTS;  Output.L += Tmp + Tmp2; Output.R += Tmp - Tmp2; }
    else if ( UNLIKELY (cRSID_C64.SID[3].Channel == CRSID_CHANNEL_RIGHT) )  //Output.R += Tmp * 2;
    { Tmp2 = Tmp >> CRSID_CHANNELPANNING_DIVSHIFTS;  Output.R += Tmp + Tmp2; Output.L += Tmp - Tmp2; }
    else { Output.L += Tmp; Output.R += Tmp; }
   }
   if ( TIGHTLY (cRSID_C64.SID[4].BaseAddress != 0) ) {
    Tmp = cRSID_emulateSID_light( &cRSID_C64.SID[4] );
    if (cRSID_C64.SID[4].Channel == CRSID_CHANNEL_LEFT)  //Output.L += Tmp * 2;
    { Tmp2 = Tmp >> CRSID_CHANNELPANNING_DIVSHIFTS;  Output.L += Tmp + Tmp2; Output.R += Tmp - Tmp2; }
    else if (cRSID_C64.SID[4].Channel == CRSID_CHANNEL_RIGHT)  //Output.R += Tmp * 2;
    { Tmp2 = Tmp >> CRSID_CHANNELPANNING_DIVSHIFTS;  Output.R += Tmp + Tmp2; Output.L += Tmp - Tmp2; }
    else { Output.L += Tmp; Output.R += Tmp; }
   }
   Output.L /= cRSID_C64.Attenuation; //( (CHANNELS*VOLUME_MAX) + cRSID_C64.Attenuation );  // * cRSID_C64.AudioThread_SIDchipCount;
   Output.R /= cRSID_C64.Attenuation; //( (CHANNELS*VOLUME_MAX) + cRSID_C64.Attenuation );  // * cRSID_C64.AudioThread_SIDchipCount;
  } break;
 }
 return &Output;
}



static INLINE void cRSID_emulateOversampledSIDwaves () { //averaging oversampler, only oscillators & waveforms (called at samplerate-pace)
 FASTVAR unsigned char HQsampleCount = 0;  FASTVAR int Temp;
 //2-pole Chebyshev filter: coefficients for 0.075f (cutoff is 18.4734kHz at 5.5x oversampling):
 //a0=3.869430E-02, a1=7.738860E-02, a2=3.869430E-02,  //fixedpoint (1.0=16384): a0 = 634, a1 = 1268,  a2 = 634
 //                 b1=1.392667E+00, b2=-5.474446E-01; //fixedpoint (1.0=16384): (b0 = 0), b1 = 22817, b2 = -8969
 //sample 'n': output[n] = input[n]*a0 + (input[n-1]*a1 + output[n-1]*b1) + (input[n-2]*a2 + output[n-2]*b2)
 //'a=a0+a1+a2=2536' -> simplified (faster) formula: output = (input*a + output*b1 + prevoutput*b2) / 16384
 //but b1 and b2 is compensated to signed-sum to 16384 and to be (sums of) powers of 2:
 // a = 2048 , b1 = 22528 = 16384 + 4096 + 2048 = 16384 + 2048 * 3 , b2 = -8192
 // -> output = (input*2048 + output*(16384+3*2048) - prevoutput * 8192) >> 14
 // -> output += ( (input << 11) + ((output*3) << 11) - (prevoutput << 13) ) >> 14
 //*** Resulting even more simplified formula: output += ( input + output * 3 - (prevoutput << 2) ) >> 3;
 //2-pole Chybyshev coefficients for 0.1f (cutoff is 10th of sample-freq. 24.6312kHz at 5.5x oversampling):
 //a0=6.372802E-02, a1=1.274560E-01, a2=6.372802E-02  //fixedpoint (1.0=16384): a0 = 1044, a1 = 2088,  a2 = 1044
 //                 b1=1.194365E+00, b2=-4.492774E-01 //fixedpoint (1.0=16384): (b0 = 0),  b1 = 19568, b2 = -7361
 //'a=a0+a1+a2=4176', b1 and b2 is compensated to signed-sum to 16384 and to be (sums of) powers of 2:
 // a = 4096 , b1 = 20480 = 16384 + 4096, b2 = -8192
 //*** Resulting very simplified formula: output += ( input + output - (prevoutput << 1) ) >> 2;
 //4-pole Chebyshev 0.1f: is much more complex and needs much more precision, not worth implementing for RPI0
 //2.780755E-03, 1.112302E-02, 1.668453E-02,  1.112302E-02, 2.780755E-03
  //fixedpoint (1.0=16384): 46,  184,    273,   184,    46   -> sum = 733 -> 1024
 //    0,        2.764031E+00, -3.122854E+00, 1.664554E+00, -3.502232E-01
  //fixedpoint (1.0=16384): 0,  45286, -51165, 27272, -5738  -> 0, 49152, -53278, 24576, -6144   for example
 static int NonFiltSample1=0, FiltSample1=0, PrevNonFiltSample1=0, PrevFiltSample1=0;
 static int NonFiltSample2=0, FiltSample2=0, PrevNonFiltSample2=0, PrevFiltSample2=0;
 static int NonFiltSample3=0, FiltSample3=0, PrevNonFiltSample3=0, PrevFiltSample3=0;
 static int NonFiltSample4=0, FiltSample4=0, PrevNonFiltSample4=0, PrevFiltSample4=0;
 static cRSID_SIDwavOutput SIDwavOutput;

 HQsampleCount=0;
 cRSID_C64.SID[1].NonFiltedSample = cRSID_C64.SID[1].FilterInputSample = 0;
 cRSID_C64.SID[2].NonFiltedSample = cRSID_C64.SID[2].FilterInputSample = 0;
 cRSID_C64.SID[3].NonFiltedSample = cRSID_C64.SID[3].FilterInputSample = 0;
 cRSID_C64.SID[4].NonFiltedSample = cRSID_C64.SID[4].FilterInputSample = 0;

 while (cRSID_C64.OverSampleCycleCnt <= cRSID_C64.SampleClockRatio) {

  SIDwavOutput = cRSID_emulateHQwaves( &cRSID_C64.SID[1], CRSID_OVERSAMPLING_CYCLES );
  Temp = PrevNonFiltSample1; PrevNonFiltSample1 = NonFiltSample1;
  NonFiltSample1 += ( SIDwavOutput.NonFilted + NonFiltSample1 * 3 - (Temp << 2) ) >> 3; //( SIDwavOutput.NonFilted + NonFiltSample1 - (Temp << 1) ) >> 2; //(SIDwavOutput.NonFilted - NonFiltSample1) >> 2;
  Temp = PrevFiltSample1; PrevFiltSample1 = FiltSample1;
  FiltSample1 += ( SIDwavOutput.FilterInput + FiltSample1 * 3 - (Temp << 2) ) >> 3; //( SIDwavOutput.FilterInput + FiltSample1 - (Temp << 1) ) >> 2; //(SIDwavOutput.FilterInput - FiltSample1) >> 2;
  cRSID_C64.SID[1].NonFiltedSample += NonFiltSample1; cRSID_C64.SID[1].FilterInputSample += FiltSample1;

  //2-pole Chebyshev-based fast integer-only fixed-point ~18kHz Nyquist/antialiasing-filters for all SIDs
  if ( TIGHTLY (cRSID_C64.SID[2].BaseAddress != 0) ) {
   SIDwavOutput = cRSID_emulateHQwaves( &cRSID_C64.SID[2], CRSID_OVERSAMPLING_CYCLES );
   Temp = PrevNonFiltSample2; PrevNonFiltSample2 = NonFiltSample2;
   NonFiltSample2 += ( SIDwavOutput.NonFilted + NonFiltSample2 * 3 - (Temp << 2) ) >> 3;
   Temp = PrevFiltSample2; PrevFiltSample2 = FiltSample2;
   FiltSample2 += ( SIDwavOutput.FilterInput + FiltSample2 * 3 - (Temp << 2) ) >> 3;
   cRSID_C64.SID[2].NonFiltedSample += NonFiltSample2; cRSID_C64.SID[2].FilterInputSample += FiltSample2; //SIDwavOutput.FilterInput;
  }
  if ( TIGHTLY (cRSID_C64.SID[3].BaseAddress != 0) ) {
   SIDwavOutput = cRSID_emulateHQwaves( &cRSID_C64.SID[3], CRSID_OVERSAMPLING_CYCLES );
   Temp = PrevNonFiltSample3; PrevNonFiltSample3 = NonFiltSample3;
   NonFiltSample3 += ( SIDwavOutput.NonFilted + NonFiltSample3 * 3 - (Temp << 2) ) >> 3;
   Temp = PrevFiltSample3; PrevFiltSample3 = FiltSample3;
   FiltSample3 += ( SIDwavOutput.FilterInput + FiltSample3 * 3 - (Temp << 2) ) >> 3;
   cRSID_C64.SID[3].NonFiltedSample += NonFiltSample3; cRSID_C64.SID[3].FilterInputSample += FiltSample3; //SIDwavOutput.FilterInput;
  }
  if ( TIGHTLY (cRSID_C64.SID[4].BaseAddress != 0) ) {
   SIDwavOutput = cRSID_emulateHQwaves( &cRSID_C64.SID[4], CRSID_OVERSAMPLING_CYCLES );
   Temp = PrevNonFiltSample4; PrevNonFiltSample4 = NonFiltSample4;
   NonFiltSample4 += ( SIDwavOutput.NonFilted + NonFiltSample4 * 3 - (Temp << 2) ) >> 3;
   Temp = PrevFiltSample4; PrevFiltSample4 = FiltSample4;
   FiltSample4 += ( SIDwavOutput.FilterInput + FiltSample4 * 3 - (Temp << 2) ) >> 3;
   cRSID_C64.SID[4].NonFiltedSample += NonFiltSample4; cRSID_C64.SID[4].FilterInputSample += FiltSample3; //SIDwavOutput.FilterInput;
  }
  ++HQsampleCount;
  cRSID_C64.OverSampleCycleCnt += (CRSID_OVERSAMPLING_CYCLES << CRSID_CLOCK_FRACTIONAL_BITS);
 }
 cRSID_C64.OverSampleCycleCnt -= cRSID_C64.SampleClockRatio;

 //fast resampler - averaging of accumulated samples, decreases sound-aliasing further:
 cRSID_C64.SID[1].NonFiltedSample /= HQsampleCount; cRSID_C64.SID[1].FilterInputSample /= HQsampleCount;
 if ( TIGHTLY (cRSID_C64.SID[2].BaseAddress != 0) ) {
  cRSID_C64.SID[2].NonFiltedSample /= HQsampleCount; cRSID_C64.SID[2].FilterInputSample /= HQsampleCount;
 }
 if ( TIGHTLY (cRSID_C64.SID[3].BaseAddress != 0) ) {
  cRSID_C64.SID[3].NonFiltedSample /= HQsampleCount; cRSID_C64.SID[3].FilterInputSample /= HQsampleCount;
 }
 if ( TIGHTLY (cRSID_C64.SID[4].BaseAddress != 0) ) {
  cRSID_C64.SID[4].NonFiltedSample /= HQsampleCount; cRSID_C64.SID[4].FilterInputSample /= HQsampleCount;
 }
}



static INLINE cRSID_Output* cRSID_emulateOversampledSIDoutputs () { //called at samplerate-pace, filters and complete output stages
 static enum { VOLUME_MAX=0xF, CHANNELS=3+1 } SIDspecs; //digi-channel is counted too in attenuation

 FASTVAR signed int Tmp, Tmp2;
 static cRSID_Output Output;

 switch (cRSID_C64.Stereo) {  //if ( MOSTLY (cRSID_C64.Stereo==CRSID_CHANNELMODE_MONO /*|| cRSID_C64.AudioThread_SIDchipCount==1*/) ) { //mono
  case CRSID_CHANNELMODE_MONO: {
   Output.L /*= Output.R*/ = cRSID_emulateSIDoutputStage( &cRSID_C64.SID[1] ); //, cRSID_C64.HighQualityResampler );
   if ( TIGHTLY (cRSID_C64.SID[2].BaseAddress != 0) ) Output.L += cRSID_emulateSIDoutputStage( &cRSID_C64.SID[2] ); //, cRSID_C64.HighQualityResampler );
   if ( TIGHTLY (cRSID_C64.SID[3].BaseAddress != 0) ) Output.L += cRSID_emulateSIDoutputStage( &cRSID_C64.SID[3] ); //, cRSID_C64.HighQualityResampler );
   if ( TIGHTLY (cRSID_C64.SID[4].BaseAddress != 0) ) Output.L += cRSID_emulateSIDoutputStage( &cRSID_C64.SID[4] ); //, cRSID_C64.HighQualityResampler );
   Output.R = Output.L /= cRSID_C64.Attenuation; //( (CHANNELS*VOLUME_MAX) + cRSID_C64.Attenuation );  // * cRSID_C64.AudioThread_SIDchipCount; //better being multiplied in a single place than for all SIDs
  } break;
  case CRSID_CHANNELMODE_STEREO: {  //else { //stereo
   Tmp = cRSID_emulateSIDoutputStage( &cRSID_C64.SID[1] ); //, cRSID_C64.HighQualityResampler );
   if ( RARELY (cRSID_C64.SID[1].Channel == CRSID_CHANNEL_LEFT) ) { Output.L = Tmp * 2; Output.R=0; }
   else if ( RARELY (cRSID_C64.SID[1].Channel == CRSID_CHANNEL_RIGHT) ) { Output.R = Tmp * 2; Output.L=0; }
   else Output.L = Output.R = Tmp;
   if ( TIGHTLY (cRSID_C64.SID[2].BaseAddress != 0) ) {
    Tmp = cRSID_emulateSIDoutputStage( &cRSID_C64.SID[2] ); //, cRSID_C64.HighQualityResampler );
    if (cRSID_C64.SID[2].Channel == CRSID_CHANNEL_LEFT)  Output.L += Tmp * 2;
    else if (cRSID_C64.SID[2].Channel == CRSID_CHANNEL_RIGHT) Output.R += Tmp * 2;
    else { Output.L += Tmp; Output.R += Tmp; }
   }
   if ( TIGHTLY (cRSID_C64.SID[3].BaseAddress != 0) ) {
    Tmp = cRSID_emulateSIDoutputStage( &cRSID_C64.SID[3] ); //, cRSID_C64.HighQualityResampler );
    if ( UNLIKELY (cRSID_C64.SID[3].Channel == CRSID_CHANNEL_LEFT) ) Output.L += Tmp * 2;
    else if ( UNLIKELY (cRSID_C64.SID[3].Channel == CRSID_CHANNEL_RIGHT) ) Output.R += Tmp * 2;
    else { Output.L += Tmp; Output.R += Tmp; }
   }
   if ( TIGHTLY (cRSID_C64.SID[4].BaseAddress != 0) ) {
    Tmp = cRSID_emulateSIDoutputStage( &cRSID_C64.SID[4] ); //, cRSID_C64.HighQualityResampler );
    if (cRSID_C64.SID[4].Channel == CRSID_CHANNEL_LEFT)  Output.L += Tmp * 2;
    else if (cRSID_C64.SID[4].Channel == CRSID_CHANNEL_RIGHT) Output.R += Tmp * 2;
    else { Output.L += Tmp; Output.R += Tmp; }
   }
   Output.L /= cRSID_C64.Attenuation; //( (CHANNELS*VOLUME_MAX) + cRSID_C64.Attenuation );  // * cRSID_C64.AudioThread_SIDchipCount;
   Output.R /= cRSID_C64.Attenuation; //( (CHANNELS*VOLUME_MAX) + cRSID_C64.Attenuation );  // * cRSID_C64.AudioThread_SIDchipCount;
  } break;
  case CRSID_CHANNELMODE_NARROW: {  //narrowed stereo (channels are closer to each other and the center)
   Tmp = cRSID_emulateSIDoutputStage( &cRSID_C64.SID[1] ); //, cRSID_C64.HighQualityResampler );
   if ( RARELY (cRSID_C64.SID[1].Channel == CRSID_CHANNEL_LEFT) ) {
    Tmp2 = Tmp >> CRSID_CHANNELPANNING_DIVSHIFTS;  Output.L = Tmp + Tmp2; Output.R = Tmp - Tmp2;
   }
   else if ( RARELY (cRSID_C64.SID[1].Channel == CRSID_CHANNEL_RIGHT) ) {
    Tmp2 = Tmp >> CRSID_CHANNELPANNING_DIVSHIFTS;  Output.R = Tmp + Tmp2; Output.L = Tmp - Tmp2;
   }
   else Output.L = Output.R = Tmp;
   if ( TIGHTLY (cRSID_C64.SID[2].BaseAddress != 0) ) {
    Tmp = cRSID_emulateSIDoutputStage( &cRSID_C64.SID[2] ); //, cRSID_C64.HighQualityResampler );
    if (cRSID_C64.SID[2].Channel == CRSID_CHANNEL_LEFT) {
     Tmp2 = Tmp >> CRSID_CHANNELPANNING_DIVSHIFTS;  Output.L += Tmp + Tmp2; Output.R += Tmp - Tmp2;
    }
    else if (cRSID_C64.SID[2].Channel == CRSID_CHANNEL_RIGHT) {
     Tmp2 = Tmp >> CRSID_CHANNELPANNING_DIVSHIFTS;  Output.R += Tmp + Tmp2; Output.L += Tmp - Tmp2;
    }
    else { Output.L += Tmp; Output.R += Tmp; }
   }
   if ( TIGHTLY (cRSID_C64.SID[3].BaseAddress != 0) ) {
    Tmp = cRSID_emulateSIDoutputStage( &cRSID_C64.SID[3] ); //, cRSID_C64.HighQualityResampler );
    if ( UNLIKELY (cRSID_C64.SID[3].Channel == CRSID_CHANNEL_LEFT) ) {
     Tmp2 = Tmp >> CRSID_CHANNELPANNING_DIVSHIFTS;  Output.L += Tmp + Tmp2; Output.R += Tmp - Tmp2;
    }
    else if ( UNLIKELY (cRSID_C64.SID[3].Channel == CRSID_CHANNEL_RIGHT) ) {
     Tmp2 = Tmp >> CRSID_CHANNELPANNING_DIVSHIFTS;  Output.R += Tmp + Tmp2; Output.L += Tmp - Tmp2;
    }
    else { Output.L += Tmp; Output.R += Tmp; }
   }
   if ( TIGHTLY (cRSID_C64.SID[4].BaseAddress != 0) ) {
    Tmp = cRSID_emulateSIDoutputStage( &cRSID_C64.SID[4] ); //, cRSID_C64.HighQualityResampler );
    if (cRSID_C64.SID[4].Channel == CRSID_CHANNEL_LEFT) {
     Tmp2 = Tmp >> CRSID_CHANNELPANNING_DIVSHIFTS;  Output.L += Tmp + Tmp2; Output.R += Tmp - Tmp2;
    }
    else if (cRSID_C64.SID[4].Channel == CRSID_CHANNEL_RIGHT) {
     Tmp2 = Tmp >> CRSID_CHANNELPANNING_DIVSHIFTS;  Output.R += Tmp + Tmp2; Output.L += Tmp - Tmp2;
    }
    else { Output.L += Tmp; Output.R += Tmp; }
   }
   Output.L /= cRSID_C64.Attenuation; //( (CHANNELS*VOLUME_MAX) + cRSID_C64.Attenuation );  // * cRSID_C64.AudioThread_SIDchipCount;
   Output.R /= cRSID_C64.Attenuation; //( (CHANNELS*VOLUME_MAX) + cRSID_C64.Attenuation );  // * cRSID_C64.AudioThread_SIDchipCount;
  } break;
 }
 return &Output;
}



static INLINE void cRSID_emulateHQresampledSIDs () { //oscillators, waveforms, filter and attenuation (main-volume) (called at samplerate-pace, but core at oversampled rate)
 static enum {
  //RESAMPLER_FRACTIONAL_BITS = 12,
  FRACTIONAL_MUL = (1 << CRSID_RESAMPLER_FRACTIONAL_BITS), //4096 $1000
  FRACTIONAL_AND = (FRACTIONAL_MUL-1), //4095 $0FFF
  INTEGER_AND = (-FRACTIONAL_MUL), //$F000
  RESAMPLEBUFFER_SIZE = 16, //entries
   RESAMPLEBUFFER_SIZE_MUL = (RESAMPLEBUFFER_SIZE << CRSID_RESAMPLER_FRACTIONAL_BITS),
  SINCWINDOW_PERIODS = (CRSID_RESAMPLER_SINCWINDOW_PERIODS), //12, //half-sines
   SINCWINDOW_HALF1_LAST_PERIOD = (SINCWINDOW_PERIODS/2-1),
  SINCPERIOD_SAMPLES = (CRSID_RESAMPLER_SINCPERIOD_SAMPLES), //128, //entries
   SINCWINDOW_SIZE = (SINCWINDOW_PERIODS*SINCPERIOD_SAMPLES), //1536
   SINCPERIOD_BITS = ( CRSID_BYTE_LOG2( SINCPERIOD_SAMPLES ) ), //7, //log2(SINCPERIOD_SAMPLES)
    SINCPERIOD_BITS_REVERSE = (CRSID_RESAMPLER_FRACTIONAL_BITS-SINCPERIOD_BITS),
  //SINCWINDOW_RESOLUTION = 11, //bits
   SINCWINDOW_MAGNITUDE = (CRSID_RESAMPLER_SINCWINDOW_MAGNITUDE) //(1 << SINCWINDOW_RESOLUTION) //SINCWINDOW_VALUE_MAX = 2048 // (2**SINCWINDOW_RESOLUTION)
 } AudioResamplerParameters;

 FASTVAR signed int Tmp, Left, Right;
 FASTVAR signed char Mono, ResampleBufWritePos = 0;
 FASTVAR int SincWindowPos;
 static int ResampleBufPos=0, NextResampleBufPos=FRACTIONAL_MUL;
 static signed int ResampleBufferL [RESAMPLEBUFFER_SIZE] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
 static signed int ResampleBufferR [RESAMPLEBUFFER_SIZE] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
 #include "SincWindow.h"

 cRSID_precalculateHQoutputParameters( &cRSID_C64.SID[1] );
 if ( TIGHTLY (cRSID_C64.SID[2].BaseAddress != 0) ) cRSID_precalculateHQoutputParameters( &cRSID_C64.SID[2] );
 if ( TIGHTLY (cRSID_C64.SID[3].BaseAddress != 0) ) cRSID_precalculateHQoutputParameters( &cRSID_C64.SID[3] );
 if ( TIGHTLY (cRSID_C64.SID[4].BaseAddress != 0) ) cRSID_precalculateHQoutputParameters( &cRSID_C64.SID[4] );
 Mono = cRSID_C64.Stereo==CRSID_CHANNELMODE_MONO /*|| cRSID_C64.AudioThread_SIDchipCount==1*/;

 while (ResampleBufPos < NextResampleBufPos) {
  ResampleBufWritePos = (ResampleBufPos >> CRSID_RESAMPLER_FRACTIONAL_BITS) - SINCWINDOW_HALF1_LAST_PERIOD;
   if (ResampleBufWritePos < 0) ResampleBufWritePos += RESAMPLEBUFFER_SIZE;
  SincWindowPos = SINCPERIOD_SAMPLES - ( (ResampleBufPos & FRACTIONAL_AND) >> SINCPERIOD_BITS_REVERSE );

  if ( MOSTLY (Mono) ) { //mono
   Left /*= Right*/ = cRSID_emulateHQresampledSID( &cRSID_C64.SID[1], CRSID_OVERSAMPLING_CYCLES ); //.Mix;
   if ( TIGHTLY (cRSID_C64.SID[2].BaseAddress != 0) ) Left += cRSID_emulateHQresampledSID( &cRSID_C64.SID[2], CRSID_OVERSAMPLING_CYCLES ); //.Mix;
   if ( TIGHTLY (cRSID_C64.SID[3].BaseAddress != 0) ) Left += cRSID_emulateHQresampledSID( &cRSID_C64.SID[3], CRSID_OVERSAMPLING_CYCLES ); //.Mix;
   if ( TIGHTLY (cRSID_C64.SID[4].BaseAddress != 0) ) Left += cRSID_emulateHQresampledSID( &cRSID_C64.SID[4], CRSID_OVERSAMPLING_CYCLES ); //.Mix;
   //Right = Left;
   while (SincWindowPos < SINCWINDOW_SIZE) { //Resampling subsequent stereo samples to output-sample-buffer
    ResampleBufferL[ResampleBufWritePos] += (Left * SincWindow[SincWindowPos]) / SINCWINDOW_MAGNITUDE; // >> SINCWINDOW_RESOLUTION;
    ++ResampleBufWritePos; if (ResampleBufWritePos >= RESAMPLEBUFFER_SIZE) ResampleBufWritePos=0;
    SincWindowPos += SINCPERIOD_SAMPLES;
   }
  }
  else { //stereo
   Tmp = cRSID_emulateHQresampledSID( &cRSID_C64.SID[1], CRSID_OVERSAMPLING_CYCLES ); //.Mix;
   if ( RARELY (cRSID_C64.SID[1].Channel == CRSID_CHANNEL_LEFT) ) { Left = Tmp * 2; Right=0; }
   else if ( RARELY (cRSID_C64.SID[1].Channel == CRSID_CHANNEL_RIGHT) ) { Right = Tmp * 2; Left=0; }
   else Left = Right = Tmp;
   if ( TIGHTLY (cRSID_C64.SID[2].BaseAddress != 0) ) {
    Tmp = cRSID_emulateHQresampledSID( &cRSID_C64.SID[2], CRSID_OVERSAMPLING_CYCLES ); //.Mix;
    if (cRSID_C64.SID[2].Channel == CRSID_CHANNEL_LEFT)  Left += Tmp * 2;
    else if (cRSID_C64.SID[2].Channel == CRSID_CHANNEL_RIGHT) Right += Tmp * 2;
    else { Left += Tmp; Right += Tmp; }
   }
   if ( TIGHTLY (cRSID_C64.SID[3].BaseAddress != 0) ) {
    Tmp = cRSID_emulateHQresampledSID( &cRSID_C64.SID[3], CRSID_OVERSAMPLING_CYCLES ); //.Mix;
    if ( UNLIKELY (cRSID_C64.SID[3].Channel == CRSID_CHANNEL_LEFT) ) Left += Tmp * 2;
    else if ( UNLIKELY (cRSID_C64.SID[3].Channel == CRSID_CHANNEL_RIGHT) ) Right += Tmp * 2;
    else { Left += Tmp; Right += Tmp; }
   }
   if ( TIGHTLY (cRSID_C64.SID[4].BaseAddress != 0) ) {
    Tmp = cRSID_emulateHQresampledSID( &cRSID_C64.SID[4], CRSID_OVERSAMPLING_CYCLES ); //.Mix;
    if (cRSID_C64.SID[4].Channel == CRSID_CHANNEL_LEFT)  Left += Tmp * 2;
    else if (cRSID_C64.SID[4].Channel == CRSID_CHANNEL_RIGHT) Right += Tmp * 2;
    else { Left += Tmp; Right += Tmp; }
   }
   while (SincWindowPos < SINCWINDOW_SIZE) { //Resampling subsequent stereo samples to output-sample-buffer
    ResampleBufferL[ResampleBufWritePos] += (Left * SincWindow[SincWindowPos]) / SINCWINDOW_MAGNITUDE; // >> SINCWINDOW_RESOLUTION;
    ResampleBufferR[ResampleBufWritePos] += (Right * SincWindow[SincWindowPos]) / SINCWINDOW_MAGNITUDE; // >> SINCWINDOW_RESOLUTION;
    ++ResampleBufWritePos; if (ResampleBufWritePos >= RESAMPLEBUFFER_SIZE) ResampleBufWritePos=0;
    SincWindowPos += SINCPERIOD_SAMPLES;
   }
  }

  ResampleBufPos += cRSID_C64.OversampleClockRatio;
 }

 if (ResampleBufPos >= RESAMPLEBUFFER_SIZE_MUL) ResampleBufPos -= RESAMPLEBUFFER_SIZE_MUL;
  NextResampleBufPos = (ResampleBufPos & INTEGER_AND) + FRACTIONAL_MUL;
 cRSID_C64.ResampledOutput.L = ResampleBufferL[ResampleBufWritePos] / cRSID_C64.OversampleClockRatioReciproc;
  ResampleBufferL[ResampleBufWritePos] = 0;
 if ( RARELY (!Mono) ) { //stereo
  cRSID_C64.ResampledOutput.R = ResampleBufferR[ResampleBufWritePos] / cRSID_C64.OversampleClockRatioReciproc;
   ResampleBufferR[ResampleBufWritePos] = 0;
 }
}



static INLINE cRSID_Output* cRSID_emulateHQresampledSIDoutputs () { //called at samplerate-pace, only adding digi and final attenuation
 static enum { VOLUME_MAX=0xF, CHANNELS=3+1 } SIDspecs; //digi-channel is counted too in attenuation

 FASTVAR signed int Tmp;
 static cRSID_Output Output;

 cRSID_emulateHQresampledSIDdigi( &cRSID_C64.SID[1], &cRSID_C64.ResampledOutput );
 if ( TIGHTLY (cRSID_C64.SID[2].BaseAddress != 0) ) cRSID_emulateHQresampledSIDdigi( &cRSID_C64.SID[2], &cRSID_C64.ResampledOutput );
 if ( TIGHTLY (cRSID_C64.SID[3].BaseAddress != 0) ) cRSID_emulateHQresampledSIDdigi( &cRSID_C64.SID[3], &cRSID_C64.ResampledOutput );
 if ( TIGHTLY (cRSID_C64.SID[4].BaseAddress != 0) ) cRSID_emulateHQresampledSIDdigi( &cRSID_C64.SID[4], &cRSID_C64.ResampledOutput );

 Output.L = cRSID_C64.ResampledOutput.L / cRSID_C64.Attenuation; //( ( (CHANNELS*VOLUME_MAX) + cRSID_C64.Attenuation ) );  // * cRSID_C64.AudioThread_SIDchipCount ); //better being multiplied in a single place than for all SIDs
 Output.R = TIGHTLY (cRSID_C64.Stereo >= CRSID_CHANNELMODE_STEREO /*&& cRSID_C64.AudioThread_SIDchipCount>1*/) ?
             cRSID_C64.ResampledOutput.R / cRSID_C64.Attenuation //( ( (CHANNELS*VOLUME_MAX) + cRSID_C64.Attenuation ) ) // * cRSID_C64.AudioThread_SIDchipCount )
             : Output.L;

 return &Output; //&cRSID_C64.ResampledOutput; //&Output;
}



static INLINE short cRSID_playPSIDdigi () {
 //static cRSID_C64instance* C64 = &cRSID_C64;

 enum PSIDdigiSpecs {
  D418_VOL_RANGE=0x10, DIGI_MAX = (D418_VOL_RANGE-1), DIGI_MASK=(DIGI_MAX), DIGI_MID = (D418_VOL_RANGE/2),
  OUTPUT_RESOLUTION=16, SID_CHANNELS=3+1, NOMINAL_DIGI_RANGE = ( (1 << OUTPUT_RESOLUTION) / SID_CHANNELS ), //16384
  DIGI_VOLUME=128 /*55*/, DIGI_VOLUME_DENOM=16, NOMINAL_DIGI_MUL = (NOMINAL_DIGI_RANGE / D418_VOL_RANGE), //1024
  DIGI_MUL = (NOMINAL_DIGI_MUL * DIGI_VOLUME / DIGI_VOLUME_DENOM) * CRSID_PRESAT_ATT_DENOM / CRSID_PRESAT_ATT_NOM
 };

 FASTVAR unsigned char Shifts;
 static unsigned char PlaybackEnabled=0, NybbleCounter=0, RepeatCounter=0;
 FASTVAR unsigned short RatePeriod;
 static unsigned short SampleAddress = 0;
 static int Output=0; //need to be static to keep level between calls
 static int PeriodCounter = 0;

 if ( cRSID_C64.IObankWR[0xD41D] ) {
  PlaybackEnabled = (cRSID_C64.IObankWR[0xD41D] >= 0xFE);
  PeriodCounter = 0; NybbleCounter = 0;
  SampleAddress = cRSID_C64.IObankWR[0xD41E] + (cRSID_C64.IObankWR[0xD41F]<<8);
  RepeatCounter = cRSID_C64.IObankWR[0xD43F];
 }
 cRSID_C64.IObankWR[0xD41D] = 0;

 if (PlaybackEnabled) {
  RatePeriod = cRSID_C64.IObankWR[0xD45D] + (cRSID_C64.IObankWR[0xD45E]<<8);
  if (RatePeriod) PeriodCounter += cRSID_C64.CPUfrequency / RatePeriod;
  if ( PeriodCounter >= cRSID_C64.SampleRate ) {
   PeriodCounter -= cRSID_C64.SampleRate;

   if ( SampleAddress < cRSID_C64.IObankWR[0xD43D] + (cRSID_C64.IObankWR[0xD43E]<<8) ) {
    if (NybbleCounter) {
     Shifts = cRSID_C64.IObankWR[0xD47D] ? 4:0;
     ++SampleAddress;
    }
    else Shifts = cRSID_C64.IObankWR[0xD47D] ? 0:4;
    Output = ( ( (cRSID_C64.RAMbank[SampleAddress]>>Shifts) & DIGI_MASK) - DIGI_MID ) * DIGI_MUL; //* DIGI_VOLUME; //* (cRSID_C64.IObankWR[0xD418]&0xF);
    NybbleCounter^=1;
   }
   else if (RepeatCounter) {
    SampleAddress = cRSID_C64.IObankWR[0xD47F] + (cRSID_C64.IObankWR[0xD47E]<<8);
    RepeatCounter--;
   }

  }
 }

 return (Output / cRSID_C64.Attenuation);
}

