
//cRSID SID emulation engine

#include "SID_ADSR.c"
#include "SID_OscWaves.c"
#include "SID_Outputs.c"


unsigned short cRSID_getSIDbase (int sid_number) {
 return cRSID_C64.SID[ sid_number ].BaseAddress;
}

unsigned short cRSID_getSIDmodel (int sid_number) {
 return cRSID_C64.SID[ sid_number ].ChipModel;
}
unsigned short cRSID_setSIDmodel (int sid_number, unsigned short value) {
 return ( cRSID_C64.SID[ sid_number ].ChipModel = value );
}

unsigned char cRSID_getSIDchannel (int sid_number) { //channel in stereo field (left/right/middle)
 return cRSID_C64.SID[ sid_number ].Channel;
}

int cRSID_getSIDlevel (int sid_number) {
 return cRSID_C64.SID[ sid_number ].Level;
}


void cRSID_createSIDchip (cRSID_SIDinstance* SID, unsigned short model, char channel, unsigned short baseaddress) {
 //static cRSID_C64instance* C64 = &cRSID_C64;

 //SID->C64 = C64;
 SID->ChipModel = model; SID->Channel=channel;
 if( baseaddress>=0xD400 && (baseaddress<0xD800 || (0xDE00<=baseaddress && baseaddress<=0xDFE0)) ) { //check valid address, avoid Color-RAM
  SID->BaseAddress = baseaddress; SID->BasePtr = &cRSID_C64.IObankWR[baseaddress]; SID->BasePtrRD = &cRSID_C64.IObankRD[baseaddress];
 }
 else { SID->BaseAddress=0x0000;
 SID->BasePtr = SID->BasePtrRD = &cRSID_C64.IObankWR[CRSID_SID_SAFE_ADDRESS]; } //NULL; } //NULL-ing not preferred as it can cause Segfault in sample-thread
 cRSID_initSIDchip(SID);           //(and guarding against NULL BasePtr would take some precious cycles in SID-emulation functions)
}


void cRSID_initSIDchip (cRSID_SIDinstance* SID) {
 static unsigned char Channel;
 for (Channel = 0; Channel < 21; Channel+=7) {
  SID->ADSRstate[Channel] = 0; SID->RateCounter[Channel] = 0;
  SID->EnvelopeCounter[Channel] = 0; SID->ExponentCounter[Channel] = 0;
  SID->PhaseAccu[Channel] = 0; SID->PrevPhaseAccu[Channel] = 0;
  SID->NoiseLFSR[Channel] = 0x7FFFFF;
  SID->PrevWavGenOut[Channel] = 0; SID->PrevWavData[Channel] = 0;
  SID->PrevSounDemonDigiWF[Channel] = 0x00;
 }
 SID->SyncSourceMSBrise = 0; SID->RingSourceMSB = 0;
 SID->PrevLowPass = SID->PrevBandPass = SID->PrevVolume = 0;
}


static INLINE int cRSID_emulateHQresampledSID (FASTVAR cRSID_SIDinstance *const FASTPTR SID, FASTVAR char cycles) {
 //cause immediate stopping in audio-buffer thread, so SID-baseaddress changes during tune-switching won't give segfaults
 //if ( RARELY (cRSID.Paused || SID->BasePtr == NULL) ) return 0; //avoid some segfaults when NULL-ing SID4
 SID->Output = cRSID_emulateHQresampledSIDoutputStage( SID, cRSID_emulateHQwaves( SID, cycles ) );  // * SID->Volume;
 return SID->Output;
}

