// cRSID lightweight (integer-only) RealSID library (with API-calls) by Hermit (Mihaly Horvath), Year 2026
// License: WTF - do what the frick you want with this code, but please mention me as the original author
#define LIBCRSID_SOURCE


// #include <stdlib.h>
// #include <stdio.h>
//#include <stdint.h>
// #include <unistd.h>

#ifdef CRSID_PLATFORM_PC
 #include <stdio.h>
 //#include <math.h>
 #include <libgen.h> //for dirname()
#endif

#include "libcRSID.h"

#include "Config.h"
#include "Optimize.h"

#include "C64/C64.c"
#include "host/file.c"
#include "host/audio.c"


cRSID_Interface cRSID; //the only public global object (for faster & simpler access than with struct-pointers, in some places)
cRSID_C64instance cRSID_C64; //the only private global object (for faster & simpler access than with struct-pointers, in some places)



static void cRSID_autoAdvance_Dummy_callBack (char arg, void* data) { return; }


void* cRSID_init (unsigned short samplerate, unsigned short buflen) {
 static cRSID_C64instance* C64 = &cRSID_C64;

 cRSID.RealSIDmode = 1;
 cRSID.Stereo = CRSID_CHANNELMODE_MONO;
 cRSID.HighQualitySID = 1;
 cRSID.HighQualityResampler = 0; //1;  //was problematic (noisy) with lowpass-filtered triangles on all SID-channels
 cRSID_C64.RealSIDmode = cRSID.RealSIDmode; cRSID_C64.Stereo = cRSID.Stereo; //init shadow-variables to defaults for audio-thread as well
 cRSID_C64.HighQualitySID = cRSID.HighQualitySID; cRSID_C64.HighQualityResampler = cRSID.HighQualityResampler;
 cRSID.SelectedSIDmodel = 0; //default model and mode selections
 cRSID.PlaybackSpeed = 1;
 cRSID.MainVolume = 255; //230; //255; //230; //204; //255;
 cRSID.CLImode = 0; cRSID.AutoExit = cRSID.BuiltInMusic = cRSID.BuiltInMusicSize = 0; cRSID.BuiltInMusicData = NULL;
 cRSID.OpenedMusic = 0;
 cRSID.AutoAdvance = cRSID.FadeOut = 1;
 C64->FadeLevel=0xF;
 cRSID.PlayListSize = 0; cRSID.PlayListNumber = 1;
 cRSID.PlayListPlayPosition = cRSID.PlayListAdvance = 0;
 cRSID.SongLengths = NULL;  cRSID.KERNALfileData = NULL; cRSID.BASICfileData = NULL;
//#ifdef CRSID_PLATFORM_PC
 cRSID_setCallBack__autoAdvance( cRSID_autoAdvance_Dummy_callBack, NULL ); //to prevent segfault if not assigned by user
//#endif

 C64 = cRSID_createC64( samplerate );
#ifdef CRSID_PLATFORM_PC
 cRSID.ChangingDirectory=0;
 if ( cRSID_initSound(samplerate,buflen) == NULL ) return NULL;
#else
 if (buflen) return C64; //this is here just to eliminate unused 'buflen' variable warning
#endif

 return C64;
}



void cRSID_initSIDtune (cRSID_SIDheader* SIDheader, char subtune) { //subtune: 1..255
 static cRSID_C64instance* C64 = &cRSID_C64;
 static const unsigned char PowersOf2[] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
 unsigned int InitTimeout=10000000; //allowed instructions, value should be selected to allow for long-running memory-copiers in init-routines (e.g. Synth Sample)

 cRSID.PlaytimeExpired=0; C64->FadeLevel=0xF; cRSID.Paused = 1;

 if (subtune==0) subtune = 1;
 else if (subtune > SIDheader->SubtuneAmount) subtune = SIDheader->SubtuneAmount;    //printf( "Subtune: %d\n", subtune );
 cRSID.SubTune = subtune; C64->SecondCnt = cRSID.PlayTime = 0; cRSID.PlaybackSpeed=1; //cRSID.Paused = 0; //don't unpause this early, before init!

 cRSID_setC64(); cRSID_initC64(); //cRSID_writeMemC64(C64,0xD418,0xF); //set C64 hardware and init (reset) it
 //cRSID.Paused = 0;

 //determine init-address:
 cRSID.InitAddress = ((SIDheader->InitAddressH)<<8) + (SIDheader->InitAddressL); //get info from BASIC-startupcode for some tunes
 if (C64->RAMbank[1] == 0x37) { //are there SIDs with routine under IO area? some PSIDs don't set bank-registers themselves
  if ( (0xA000 <= cRSID.InitAddress && cRSID.InitAddress < 0xC000)
       || (cRSID.LoadAddress < 0xC000 && cRSID.EndAddress >= 0xA000) ) C64->RAMbank[1] = 0x36;
  else if (cRSID.InitAddress >= 0xE000 || cRSID.EndAddress >=0xE000) C64->RAMbank[1] = 0x35;
 }
 cRSID_initCPU( cRSID.InitAddress ); //prepare init-routine call
 C64->CPU.A = subtune - 1;

 if (!cRSID.RealSIDmode) {
  //call init-routine:
  for (InitTimeout=10000000; InitTimeout>0; InitTimeout--) { if ( cRSID_emulateCPU()>=0xFE ) break; } //give error when timed out?
 }

 //determine timing-source, if CIA, replace FrameCycles previouisly set to VIC-timing
 if (subtune>32) cRSID.TimerSource = cRSID.SIDheader->SubtuneTimeSources[0] & 0x80; //subtunes above 32 should use subtune32's timing
 else cRSID.TimerSource = cRSID.SIDheader->SubtuneTimeSources[(32-subtune)>>3] & PowersOf2[(subtune-1)&7];
 if (cRSID.TimerSource || C64->IObankWR[0xDC05]!=0x40 || C64->IObankWR[0xDC04]!=0x24) { //CIA1-timing (probably multispeed tune)
  cRSID.FrameCycles = ( ( C64->IObankWR[0xDC04] + (C64->IObankWR[0xDC05]<<8) ) ); //<< 4) / C64->ClockRatio;
  cRSID.TimerSource = 1; //if init-routine changed DC04 or DC05, assume CIA-timing
 }

 //determine playaddress:
 cRSID.PlayAddress = (SIDheader->PlayAddressH<<8) + SIDheader->PlayAddressL;
 if (cRSID.PlayAddress) { //normal play-address called with JSR
  if (C64->RAMbank[1] == 0x37) { //are there SIDs with routine under IO area?
   if (0xA000 <= cRSID.PlayAddress && cRSID.PlayAddress < 0xC000) C64->RAMbank[1] = 0x36;
  }
  else if (cRSID.PlayAddress >= 0xE000) C64->RAMbank[1] = 0x35; //player under KERNAL (e.g. Crystal Kingdom Dizzy)
 }
 else { //IRQ-playaddress for multispeed-tunes set by init-routine (some tunes turn off KERNAL ROM but doesn't set IRQ-vector!)
  cRSID.PlayAddress = (C64->RAMbank[1] & 3) < 2 ? cRSID_readMem(0xFFFE) + (cRSID_readMem(0xFFFF)<<8) //cRSID_readMemC64(C64,0xFFFE) + (cRSID_readMemC64(C64,0xFFFF)<<8) //for PSID
                                                 : cRSID_readMem(0x314) + (cRSID_readMem(0x315)<<8); //: cRSID_readMemC64(C64,0x314) + (cRSID_readMemC64(C64,0x315)<<8);
  if (cRSID.PlayAddress==0) { //if 0, still try with RSID-mode fallback
   cRSID_initCPU( cRSID.PlayAddress ); //point CPU to play-routine
   C64->Finished=1; C64->Returned=1; cRSID.Paused = 0; return;
  }
 }

 if (!cRSID.RealSIDmode) {  //prepare (PSID) play-routine playback:
  cRSID_initCPU( cRSID.PlayAddress ); //point CPU to play-routine
  C64->FrameCycleCnt=0; C64->Finished=1; C64->SampleCycleCnt=0; //C64->CIAisSet=0;
 }
 else { C64->Finished=0; C64->Returned=0; }

 cRSID.Paused = 0;
}


char* cRSID_setSongLengthData (char* filedata) {
 if (filedata != NULL) { cRSID.SongLengths = filedata; return cRSID.SongLengths; }
 return filedata;
}

unsigned char* cRSID_setKERNALdata (unsigned char* filedata) {
 if (filedata != NULL) { cRSID.KERNALfileData = filedata; cRSID_setROMcontent(); return cRSID.KERNALfileData; }
 return filedata;
}

unsigned char* cRSID_setBASICdata (unsigned char* filedata) {
 if (filedata != NULL) { cRSID.BASICfileData = filedata; cRSID_setROMcontent(); return cRSID.BASICfileData; }
 return filedata;
}


void cRSID_close (void) {
 cRSID.Paused = 1;
 #ifdef CRSID_PLATFORM_PC
  cRSID_closeSound();
 #endif
}


void cRSID_playSIDtune (void) {
 #ifdef CRSID_PLATFORM_PC
  cRSID_startSound();
 #endif
 cRSID.Paused = 0;
}


void cRSID_pauseSIDtune (void) {
 cRSID.Paused = 1; //cause immediate stopping in audio-buffer thread, so SID-baseaddress changes during tune-switching won't give segfaults
 #ifdef CRSID_PLATFORM_PC
  cRSID_stopSound();
 #endif
 #if (USING_AUDIO_LIBRARY__SDL)
  usleep( 100000 ); //this is not a blocking function, so artificially wait a bit to avoid thread-clashes
 #endif
}


void cRSID_startSubtune (unsigned char subtune) {
 int i;
 cRSID_pauseSIDtune();
 cRSID_processSIDfileData( (unsigned char*) cRSID.SIDfileData, cRSID.SIDfileSize );
 cRSID_initSIDtune( cRSID.SIDheader, subtune ); //cRSID.PlaybackSpeed=1; /*cRSID.Paused=0;*/
 cRSID_playSIDtune();
}

void cRSID_nextSubtune() { if (cRSID.SubTune<CRSID_SUBTUNE_AMOUNT_MAX-1) cRSID_startSubtune(++cRSID.SubTune); }

void cRSID_prevSubtune() { if (cRSID.SubTune>0) cRSID_startSubtune(--cRSID.SubTune); }



void cRSID_setCallBack__autoAdvance ( void (*callback) (char subtunestepping, void* data), void* data ) {
 cRSID_C64.callBack__autoAdvance = callback; cRSID_C64.callBackData__autoAdvance = data;
}

static void cRSID_callBack_autoAdvance (char subtunestepping) {
 cRSID_C64.callBack__autoAdvance( subtunestepping, cRSID_C64.callBackData__autoAdvance );
}
void cRSID_autoAdvance () {
 if ( cRSID.SubtuneDurations[cRSID.SubTune] != 0 && cRSID.AutoAdvance
      && (cRSID.SIDheader->SubtuneAmount>1 || cRSID.AutoExit || cRSID.PlayListSize>1) ) {
  if (cRSID.PlayTime == cRSID.SubtuneDurations[cRSID.SubTune]) {
   if (cRSID.SubTune < cRSID.SIDheader->SubtuneAmount) {
    if (cRSID.CLImode) {
     cRSID_nextSubtune();
     //#ifndef CRSID_LIBRARY
      cRSID_callBack_autoAdvance(1);
     //#endif
    }
    else cRSID.PlaytimeExpired=1;
   }
   #ifdef CRSID_PLATFORM_PC
   else if (!cRSID.ChangingDirectory) {
    if (cRSID.PlayListSize<=1 && cRSID.AutoExit) { printf("\n"); cRSID_close(); exit(0); }
    else if (cRSID.OpenedMusic==0) {
     if (cRSID.CLImode) {
      cRSID_nextTune();
      //#ifndef CRSID_LIBRARY
       cRSID_callBack_autoAdvance(0);
      //#endif
     }
     else cRSID.PlayListAdvance=1;
   }}
   #endif
  }
  else if ( cRSID.FadeOut && cRSID.PlayTime+1 == cRSID.SubtuneDurations[cRSID.SubTune]
            && cRSID.SubtuneDurations[cRSID.SubTune]>4 ) {
   cRSID_C64.FadeLevel = 0xF - 0xF * cRSID_C64.SecondCnt / cRSID_C64.SampleRate;
  }
  else cRSID_C64.FadeLevel=0xF;
 }
 else cRSID_C64.FadeLevel=0xF;
}



#ifdef CRSID_PLATFORM_PC



cRSID_SIDheader* cRSID_playSIDfile (char* filename, char subtune) {
 static cRSID_C64instance* C64 = &cRSID_C64;
 static cRSID_SIDheader* SIDheader;

 cRSID_pauseSIDtune();

 SIDheader = cRSID_loadSIDtune( filename );
 if (SIDheader==NULL) return NULL; //CRSID_ERROR_LOAD;

 cRSID_initSIDtune( SIDheader, subtune );
 cRSID_playSIDtune();

 return SIDheader; //CRSID_STATUS_OK;
}


cRSID_SIDheader* cRSID_loadSIDtune (char* filename) {
 static cRSID_C64instance* C64 = &cRSID_C64;
 enum SIDspecs { CRSID_FILESIZE_MAX = 100000 };
 //int FileSize;
 static unsigned char SIDfileData [CRSID_FILESIZE_MAX]; //use memset?

 cRSID_pauseSIDtune(); //stop if already playing

 cRSID.SIDfileSize/*FileSize*/ = cRSID_loadSIDfile( SIDfileData, filename, CRSID_FILESIZE_MAX );
 if ( cRSID.SIDfileSize/*FileSize*/ == CRSID_ERROR_LOAD ) return NULL;
 strcpy (C64->FileNameOnly, cRSID_fileNameOnly(filename));

 return cRSID_processSIDfileData( SIDfileData, cRSID.SIDfileSize/*FileSize*/ );
}

void cRSID_setBuiltInMusic (const unsigned char* array, size_t size) {
 cRSID.BuiltInMusicData = array; cRSID.BuiltInMusicSize = (int) size;
}



unsigned int cRSID_findNextPlayableItem (int itemnumber) {
 while (cRSID.PlayListSize>0 && !cRSID_playableExtension(cRSID.PlayList[itemnumber])
        && itemnumber<cRSID.PlayListSize-1) ++itemnumber; //skip folder-texts
 return itemnumber;
}

unsigned int cRSID_findPreviousPlayableItem (int itemnumber) {
 while (!cRSID_playableExtension(cRSID.PlayList[itemnumber]) && itemnumber>0) --itemnumber; //skip folder-texts
 return itemnumber;
}

char cRSID_startPlayListItem (int itemnumber) {
 cRSID_SIDheader *SIDheader=NULL;
 if (itemnumber < cRSID.PlayListSize) {
  itemnumber = cRSID_findNextPlayableItem (itemnumber); //skip folder-texts
  if ( cRSID_playableExtension(cRSID.PlayList[itemnumber]) ) {
   cRSID.PlayListPlayPosition = itemnumber; //so it's refreshed before tune starts (allowing handling data based on playlist-position)
   SIDheader = cRSID_playSIDfile( cRSID.PlayList[itemnumber], 1 ); //default subtune?
   if (SIDheader == NULL) { printf("Load error! (Playlist-item \"%s\" not found.)\n", cRSID.PlayList[itemnumber]); return CRSID_ERROR_LOAD; }
   else { cRSID.SIDheader=SIDheader; cRSID.PlayListPlayPosition = itemnumber; }
 }}
 return CRSID_STATUS_OK;
}

void cRSID_nextTune () {
 cRSID.PlayListAdvance = 0;
 if ( cRSID.PlayListPlayPosition+1 < cRSID.PlayListSize ) {
  cRSID_startPlayListItem( cRSID.PlayListPlayPosition+1 );
 }
 else cRSID_startPlayListItem(0); //loop over the playlist or end playback? (setting for this behaviour?)
}

void cRSID_prevTune () {
 if ( cRSID.PlayListPlayPosition-1 >= 0 ) {
  cRSID.PlayListPlayPosition = cRSID_findPreviousPlayableItem(cRSID.PlayListPlayPosition-1); //skip folder-texts
  cRSID_startPlayListItem( cRSID.PlayListPlayPosition );
 }
 else cRSID_startPlayListItem(cRSID.PlayListSize-1); //loop over the playlist or end playback? (setting for this behaviour?)
}



void cRSID_removeNewline (char *name) { //for fgets result containing newlines
 if (strrchr(name,'\n') != NULL) *strrchr(name,'\n') = '\0';
 if (strrchr(name,'\r') != NULL) *strrchr(name,'\r') = '\0';
}


#endif //CRSID_PLATFORM_PC


#undef LIBCRSID_SOURCE
