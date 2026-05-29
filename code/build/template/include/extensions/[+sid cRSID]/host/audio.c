

#ifdef CRSID_PLATFORM_PC


#if (!USING_AUDIO_LIBRARY__SDL && !USING_AUDIO_LIBRARY__ALSA)
 #define USING_AUDIO_LIBRARY__SDL 1  //default fallback audio-library ('weak' define) if nothing is given outside
#endif


#if (USING_AUDIO_LIBRARY__SDL)
 #include <SDL.h>
#elif (USING_AUDIO_LIBRARY__ALSA)
 #include <stdio.h>
 #include <stdint.h>
 //#define _POSIX_C_SOURCE 199309L  //supposedly needed to bring clock_gettime() from time.h
 //#include <time.h>
 #include <sys/time.h>
 #include <alsa/asoundlib.h>
 #include <alsa/pcm.h>
 #if (ENABLED_NCURSES) //just for testing/debugging:
  #include <curses.h>  //#include "../../host/ncurses/include/curses.h"
 #endif
#endif

#include "host.h"

#include "../libcRSID.h"


enum {
 CRSID_SAMPLE_BYTES = 2 /*for the used S16 LE 16-bit format*/, CRSID_SAMPLE_CHANNELS = 2,
 CRSID_SAMPLE_FRAMESIZE = (CRSID_SAMPLE_BYTES * CRSID_SAMPLE_CHANNELS) /*bytes*/, CRSID_SILENT_SAMPLE_VALUE = 0 /*with signed S16*/
};



void cRSID_soundCallback (void* userdata, unsigned char *buf, int len) { //currently only SDL is callback-based, ALSA is 'simply' 'polled'
 cRSID_C64.SoundStarted=1; //set to inform the rest of the code that sound could be started already (buffer callback works)
 cRSID_generateSound( buf, len );
 cRSID_autoAdvance();
}


void* cRSID_initSound (unsigned short samplerate, unsigned short buflen) {

 cRSID_C64.SoundDevice = NULL;
 cRSID_C64.SampleBuffer = NULL;
 cRSID_C64.SampleRate = samplerate;
 cRSID.SampleBufferLength = buflen;
 cRSID_C64.SampleBufferSize = cRSID.SampleBufferLength * CRSID_SAMPLE_FRAMESIZE;
 cRSID_C64.SoundStarted=0;


 #if (USING_AUDIO_LIBRARY__SDL)

  static SDL_AudioSpec SoundSpec;

  if ( SDL_Init(SDL_INIT_AUDIO) < 0 ) {
   fprintf(stderr, "Couldn't initialize SDL-Audio: %s\n",SDL_GetError()); return NULL;
  }
  SoundSpec.freq=samplerate;
  SoundSpec.channels = CRSID_SAMPLE_CHANNELS;
  SoundSpec.format = AUDIO_S16;
  SoundSpec.samples = buflen; SoundSpec.userdata = &cRSID_C64; SoundSpec.callback = cRSID_soundCallback;
  if ( SDL_OpenAudio(&SoundSpec, NULL) < 0 ) {
   fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError()); return NULL;
  }

  //cRSID.Paused = 0;

  return (void*)&SoundSpec;


 #elif (USING_AUDIO_LIBRARY__ALSA)

  enum { DISABLE_RESAMPLER = 0, ENABLE_RESAMPLER = 1 }; //advised to turn on because many soundcards are at 48kHz, not 44100Hz

  if (cRSID_C64.SoundDevice != NULL) return (void*)cRSID_C64.SoundDevice; //already initialized? return to avoid corresponding warning

  if ( snd_pcm_open( (snd_pcm_t**) &cRSID_C64.SoundDevice, "default" /* "plughw:0,0" */ , SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK ) < 0 ) {
   fprintf( stderr, "Couldn't initialize ALSA-Audio!\n" ); cRSID_C64.SoundDevice=NULL; return NULL;
  }

  //cRSID.SampleBufferLength = buflen; cRSID_C64.SampleRate = samplerate;

  if ( snd_pcm_set_params( cRSID_C64.SoundDevice, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED,
       CRSID_SAMPLE_CHANNELS, samplerate, ENABLE_RESAMPLER, 1000000 * cRSID.SampleBufferLength / samplerate ) < 0 ) { //last parameter is latency in microseconds, but if not precise we'll set exact buffersize instead
   fprintf( stderr, "Couldn't set ALSA 44kHz S16-LE playback-format.\n" ); return NULL;
  }

  cRSID_C64.SampleBufferSize = cRSID.SampleBufferLength * CRSID_SAMPLE_FRAMESIZE; //in bytes
  cRSID_C64.SampleBuffer = malloc( cRSID_C64.SampleBufferSize );
  memset( cRSID_C64.SampleBuffer, CRSID_SILENT_SAMPLE_VALUE, cRSID_C64.SampleBufferSize ); if (cRSID_C64.SampleBuffer == NULL) { printf("Couldn't allocate sample-buffer!\n"); }

  //snd_pcm_nonblock( cRSID_C64.SoundDevice, 0 ); //switch to blocking-mode for playback (used in SDL but doesn't seem to have an effect for me)

  if ( snd_pcm_prepare( cRSID_C64.SoundDevice ) < 0 ) { printf("Couldn't prepare ALSA sound-device\n"); return NULL; }

  //cRSID.Paused = 0;

  return (void*)cRSID_C64.SoundDevice;


 #else

  return (void*)cRSID_initSound; //a non-NULL is still returned to caller to show everything's fine with purposedly empty audio-init

 #endif //end of sound-init selections

}


#if (USING_AUDIO_LIBRARY__ALSA)

static INLINE int64_t getEpochTimeMicroseconds () {
 static struct timeval /*struct timespec*/ CurrentTime;
 gettimeofday( &CurrentTime, NULL /*timezone*/ );
 return CurrentTime.tv_sec * 1000000 + CurrentTime.tv_usec;
}

void cRSID_syncGenSamples (int frametime) { //this synchronizer functioun should last till the next loop/frame by generating audio, or waiting if done with it
 enum { MICROSECONDS_PER_SECOND = 1000000, SYNC_TIME_GRANULARITY = 1000 /* microseconds */ };  //maximum allowed jitter (when there's no CPU-overload)
 //enum { SAMPLECHUNK_SIZE_SLACK = BUFFERSIZE_MIN /*100*/ /*samples*/ }; //generating some more samples than needed in current round by lowering send-threshold
 enum { SAMPLE_SEND_RETRY_DELAY = 1000 }; //microseconds  (so in case of error the machine doesn't run at 100% in loop)

 static int i, j; static char SyncPeriodCount, SyncPeriods;
 static int64_t LatestSyncTime = 0, EntryTime, NewSyncTime, SyncTime;
 static signed int SyncDiff = 0, WishedPeriod;
 static snd_pcm_sframes_t FramesToSend, SentFrames, FramesLeft, BytesLeft, Slack;
 static unsigned char* SamplePointer;
 static cRSID_Output Output;

 //if no CPU-overload, sync: wait for next inputhandler frame's time
 EntryTime = getEpochTimeMicroseconds();
 if (LatestSyncTime == 0) LatestSyncTime = EntryTime;  //bootstrapping
 NewSyncTime = LatestSyncTime + frametime - SyncDiff;
 WishedPeriod = NewSyncTime - LatestSyncTime;
 SyncPeriods = frametime / SYNC_TIME_GRANULARITY; SyncPeriodCount = 0;
 while ( getEpochTimeMicroseconds() < NewSyncTime && SyncPeriodCount++ <= SyncPeriods ) {
  usleep( SYNC_TIME_GRANULARITY ); //if (++SyncPeriodCount > frametime/SYNC_TIME_GRANULARITY /*SYNC_PERIODS_MAX*/) break; //workaround for a possible problematic gettimeofday() implementation to atleast run the C64 code
 }
 SyncTime = getEpochTimeMicroseconds(); SyncDiff = (SyncTime - LatestSyncTime) - WishedPeriod;  //int WorkLoad = EntryTime - LatestSyncTime; int Period = SyncTime - LatestSyncTime; static double AveragePeriod=10000/*INPUTSCAN_PERIOD*//*CLI_KEYCHECK_PERIOD*/+2; AveragePeriod += (Period - AveragePeriod) / 1000.0; /*printf("%d ",WorkLoad);*/ printf( "Last:..%d Entry:..%d  New:..%d Sync:..%d  Work:%d  Wished:%d Period:%d (average:%.0f) Diff:%d\n", (int)(LatestSyncTime&0xFFFFFFF), (int)(EntryTime&0xFFFFFFF), (int)(NewSyncTime&0xFFFFFFF), (int)(SyncTime&0xFFFFFFF),  WorkLoad, WishedPeriod, Period, AveragePeriod, SyncDiff );
 LatestSyncTime = SyncTime /*EntryTime*/; //getEpochTimeMicroseconds();  FrameTimeOverload = WorkLoad - frametime;

 //if (cRSID.Paused) return; //cRSID_C64.SoundStarted=1; //set to inform the rest of the code that sound could be started already (buffer callback works)

 //Refresh shadow registers that remain untouched during the audio-generation, to avoid discrepancies if main thread changes their originators (here in the single thread it won't happen btw)
 cRSID_C64.RealSIDmode = cRSID.RealSIDmode; //this one gets updated at SID-data rocessing too when the audio doesn't run, to let CPU routines use the up-to-date shadowed versions
 cRSID_C64.AudioThread_SIDchipCount = cRSID_C64.SIDchipCount;
 cRSID_C64.Stereo = cRSID_C64.AudioThread_SIDchipCount > 1 ? cRSID.Stereo : CRSID_CHANNELMODE_MONO; //this might be changed regularly during playback by the user, and maybe these too:
 cRSID_C64.HighQualitySID = cRSID.HighQualitySID; cRSID_C64.HighQualityResampler = cRSID.HighQualityResampler;

 FramesToSend = ( cRSID_C64.SampleRate * frametime / MICROSECONDS_PER_SECOND ) + 1; //ideally these amount of samples would be sent in every round (if no dropouts/whatever happened)
 Slack = SYNC_TIME_GRANULARITY*2 *cRSID_C64.SampleRate/MICROSECONDS_PER_SECOND;  if (Slack > cRSID.SampleBufferLength) Slack = cRSID.SampleBufferLength; //lower buffer-drain threshold to propose sending a bit more samples than needed
 if (cRSID_C64.SoundDevice != NULL) {
  FramesLeft = snd_pcm_avail_update( cRSID_C64.SoundDevice );  //snd_pcm_wait() contains embedded poll waiting implementation
  if (FramesLeft < 0) { //even if there's no audio for some reason, at least emulate a silent C64
   FramesLeft = FramesToSend; //if (!cRSID.Paused) fprintf( stderr, "Audio error %d (%s) ( returned from snd_pcm_avail_update() )\n", (int)FramesLeft, snd_strerror(FramesLeft) );
  }
  else if (FramesLeft > cRSID.SampleBufferLength) FramesLeft = cRSID.SampleBufferLength; //when stopping audio-updates for a timeframe, older ALSA might give strange big FramesLeft values here like ~18000
 }
 else FramesLeft = FramesToSend;

 if (/*SampleBuffer!=NULL &&*/ FramesLeft >= FramesToSend - Slack /*&& FramesLeft <= cRSID.SampleBufferLength*/) { //if below threshold, wait until next round to send samples

  BytesLeft = FramesLeft * CRSID_SAMPLE_FRAMESIZE;

  if (cRSID_C64.SampleBuffer != NULL) {
   for (i = 0; i < BytesLeft /*SampleBufferSize*/ ; i += CRSID_SAMPLE_FRAMESIZE) {  // frametime*SampleRate/1000000 ?
    for (j=0; j<cRSID.PlaybackSpeed; ++j) Output = cRSID_generateSample();
    cRSID_C64.SampleBuffer[i+0] = Output.L & 0xFF; cRSID_C64.SampleBuffer[i+1] = Output.L >> 8;
    cRSID_C64.SampleBuffer[i+2] = Output.R & 0xFF; cRSID_C64.SampleBuffer[i+3] = Output.R >> 8;
   }
  }
  else { for (i = 0; i < BytesLeft; i += CRSID_SAMPLE_FRAMESIZE) cRSID_generateSample(); }

  if (cRSID_C64.SoundDevice != NULL) {
   SamplePointer = cRSID_C64.SampleBuffer;
   while (FramesLeft > 0) {
    if ( (SentFrames = snd_pcm_writei( cRSID_C64.SoundDevice, SamplePointer, FramesLeft )) < 0 ) {
     if (SentFrames==-EAGAIN) { usleep( SAMPLE_SEND_RETRY_DELAY ); continue; }
     if ( (SentFrames = snd_pcm_recover( cRSID_C64.SoundDevice, SentFrames, 0 ) ) < 0 ) {
      fprintf( stderr, "ALSA-write failed (unrecoverable): %s\n", snd_strerror(SentFrames) ); return;
     }
    }
    else {
     FramesLeft -= SentFrames; SamplePointer += SentFrames * CRSID_SAMPLE_FRAMESIZE;
    }
   }
  }

 }
 else if (FramesLeft < 0) { //retry ALSA-sound totally in case of serious fault like broken pipe... (but if the code above still ensures machine-emulation this action is not relevant and won't happen)
  if (!cRSID.Paused) fprintf( stderr, "Audio error %d (%s) ( returned from snd_pcm_avail_update() )\n", (int)FramesLeft, snd_strerror(FramesLeft) );
  cRSID_stopSound(); cRSID.Paused = 1;
  cRSID_closeSound(); //with following cRSID_initSound() ensures complete revival of a possibly thrown-away ALSA (e.g. error -32 'broken pipe' which stops sound and C64 emulation completely)
  usleep( SAMPLE_SEND_RETRY_DELAY );
  cRSID_initSound(cRSID_C64.SampleRate, cRSID.SampleBufferLength);
  cRSID_startSound(); cRSID.Paused = 0;
 }

 cRSID_autoAdvance();
}

#endif


void cRSID_closeSound (void) {
 #if (USING_AUDIO_LIBRARY__SDL)
 SDL_PauseAudio(1);
 if (cRSID_C64.SoundStarted) { //SDL_CloseAudio of SDL might freeze if somehow sound-callback didn't start
  SDL_CloseAudio();
 }
 #elif (USING_AUDIO_LIBRARY__ALSA)
  if (cRSID_C64.SoundDevice != NULL) { //&& cRSID_C64.SoundStarted) {
   snd_pcm_drain( cRSID_C64.SoundDevice );
   snd_pcm_close( cRSID_C64.SoundDevice ); //usleep(10000);
   cRSID_C64.SoundDevice = NULL;
  }
  if (cRSID_C64.SampleBuffer != NULL) { free( cRSID_C64.SampleBuffer ); cRSID_C64.SampleBuffer = NULL; }
 #endif
}


void cRSID_startSound (void) {
 #if (USING_AUDIO_LIBRARY__SDL)
  SDL_PauseAudio(0);
 #elif (USING_AUDIO_LIBRARY__ALSA)
  //snd_pcm_pause( cRSID_C64.SoundDevice, 0 );
 #endif
}


void cRSID_stopSound (void) {
 cRSID.Paused = 1; //cause immediate stopping in audio-buffer thread, so SID-baseaddress changes during tune-switching won't give segfaults
 #if (USING_AUDIO_LIBRARY__SDL)
  SDL_PauseAudio(1);
 #elif (USING_AUDIO_LIBRARY__ALSA)
  //snd_pcm_pause( cRSID_C64.SoundDevice, 1 ); //not all devices support this ( snd_pcm_hw_params_can_pause() can tell )
 #endif
}


#endif //CRSID_PLATFORM_PC


void cRSID_generateSound (FASTVAR unsigned char *buf, FASTVAR unsigned short len) {
 //enum { FADE_SCALING = 16, MIX_SCALING = (256*FADE_SCALING) };
 FASTVAR unsigned short i; FASTVAR unsigned char j;
 static cRSID_Output Output;
 //FASTVAR signed short OutputL, OutputR;
 //FASTVAR signed int VolumeMul;

 //Refresh shadow registers that remain untouched during the audio-generation, to avoid discrepancies if main thread changes their originators
 cRSID_C64.RealSIDmode = cRSID.RealSIDmode; //this one gets updated at SID-data rocessing too when the audio doesn't run, to let CPU routines use the up-to-date shadowed versions
 cRSID_C64.AudioThread_SIDchipCount = cRSID_C64.SIDchipCount;
 cRSID_C64.Stereo = cRSID_C64.AudioThread_SIDchipCount > 1 ? cRSID.Stereo : CRSID_CHANNELMODE_MONO; //this might be changed regularly during playback by the user, and maybe these too:
 cRSID_C64.HighQualitySID = cRSID.HighQualitySID; cRSID_C64.HighQualityResampler = cRSID.HighQualityResampler;

 for (i=0; i<len; i+=4) {
  for(j=0; j<cRSID.PlaybackSpeed; ++j) Output = cRSID_generateSample();
  //VolumeMul = cRSID.MainVolume * cRSID_C64.FadeLevel;
  // /* *(signed short*) &buf[i]*/ Output.L = Output.L * VolumeMul / MIX_SCALING;   //can we expect little-endianness?
  // /* *(signed short*) &buf[i+2]*/ Output.R = Output.R * VolumeMul / MIX_SCALING; //can we expect little-endianness?
  buf[i+0] = Output.L & 0xFF; buf[i+1] = Output.L >> 8;
  buf[i+2] = Output.R & 0xFF; buf[i+3] = Output.R >> 8;
 }
}


INLINE cRSID_Output cRSID_generateSample () { //call this from custom buffer-filler
 enum { FADE_SCALING = 16, MIX_SCALING = (256*FADE_SCALING),
  SCRAMBLER_BITS = 8, DITHERNOISE_DIVSHIFTS = 6 /*5*/, DITHERNOISE_MID = ( ( 1 << (SCRAMBLER_BITS - DITHERNOISE_DIVSHIFTS) ) / 2 )
 };

 FASTVAR signed short VolumeMul; FASTVAR signed short Insert;
 static cRSID_Output Output;
 static int LFSR = 0x0055A5AA; FASTVAR int fb;

 if ( RARELY (cRSID.Paused) ) { Output.L = Output.R = 0; return Output; }

 //If calling cRSID_generateSample() directly, don't forget to keep the audio-thread shadow-variables updated, as seen above in cRSID_generateSound()
 //cRSID_C64.RealSIDmode = cRSID.RealSIDmode; //this one gets updated at SID-data rocessing too when the audio doesn't run, to let CPU routines use the up-to-date shadowed versions

 Output = *cRSID_emulateC64();

 if ( RARELY (cRSID.PSIDdigiMode) ) { Insert = cRSID_playPSIDdigi(); Output.L += Insert; Output.R += Insert; }

 VolumeMul = cRSID.MainVolume * cRSID_C64.FadeLevel;
 Output.L = (Output.L * VolumeMul) / MIX_SCALING; //Having the attenuator before saturation allows avoiding filter/many-SIDs overdrive-distortion
 Output.R = (Output.R * VolumeMul) / MIX_SCALING; //(By simply adjusting the main volume knob to a lower level. Real SID distorts as well in those cases btw.)

 //Add dithering noise for more natural sound and to override any small quantization/correlated/mixdown noises (might add C64 bus-noise in the future)
 //This is my fast LFSR+scrambler-routine generating perfect even white-noise spectrum (measured with 'ENT' tool)
 //23bit LFSR (pow(2,23)-1=8388607 states) with XOR-ed taps of bit22 and bit13 (realized by 24 bits = 3 bytes and bit23^bit14 (LFSR_HH[7],LFSR_H[6]) feedback to bit1)
 fb = ( (LFSR & 0x800000) ^ ( (LFSR & 0x004000) << 9 ) ); LFSR <<= 1; if (fb) LFSR |= 2; //LFSR&=0xFFFFFE;
 //scramble 24bit value into a 8bit output (e.g. make LFSR 8bit output non-linear and therefore more random)
 //We use AND-masks to combine into output (instead of free bit-to-bit mapping)
 //(this method has ENT-correlation=0.000002, PI-error=0.02%, a nice evenly distributed spectrum that fits between -45.4..-45.8dB)
 Insert = (signed char) ( (unsigned char) ( ( ( (LFSR >> 16) & 0x99 ) + ( (LFSR >> 8) & 0x72 ) + (LFSR & 0xB4) + 0x7C ) & 0xFF ) >> DITHERNOISE_DIVSHIFTS ) - DITHERNOISE_MID;
 Output.L += Insert; Output.R += Insert;

 if ( RARELY(Output.L>=32767) ) Output.L=32767; else if ( RARELY (Output.L<=-32768) ) Output.L=-32768; //saturation arithmetic on overflow
 if ( RARELY(Output.R>=32767) ) Output.R=32767; else if ( RARELY (Output.R<=-32768) ) Output.R=-32768; //saturation arithmetic on overflow
 return Output;
}

