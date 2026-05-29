// cRSID lightweight RealSID (integer-only) library-header (with API-calls) by Hermit (Mihaly Horvath)

#ifndef LIBCRSID_HEADER
#define LIBCRSID_HEADER //guard used to prevent double inclusion of this header-file


// #include <stdlib.h>
// #include <unistd.h>

#include "Config.h"
#include "Optimize.h"


#ifdef CRSID_PLATFORM_PC
 #define CRSID_SID_FILE_EXTENSION ".sid"
 #define CRSID_SID_FILE_EXTENSION_ALTERNATIVE ".sid.seq"
#endif


enum cRSID_StatusCodes    { CRSID_STATUS_OK=0, CRSID_ERROR_INIT=-1, CRSID_ERROR_LOAD=-2 };
enum cRSID_Limitations    { CRSID_SUBTUNE_AMOUNT_MAX=256, //32,
                            CRSID_PLAYTIMESTRING_LENGTH_MAX = 20/*10*/,
                            CRSID_PLAYLIST_ENTRIES_MAX=2000,
                            CRSID_PATH_LENGTH_MAX=1024, CRSID_FILENAME_LEN_MAX = 255,
                            CRSID_PLAYLIST_ENTRY_SIZE=CRSID_PATH_LENGTH_MAX+2
                          };
enum cRSID_ChannelModes { CRSID_CHANNELMODE_MONO = 0, CRSID_CHANNELMODE_STEREO = 1, CRSID_CHANNELMODE_NARROW = 3 };


typedef struct cRSID_SIDheader {              //Offset:   default/info:
 unsigned char MagicString[4];            //$00 - "PSID" or "RSID" (RSID must provide Reset-circumstances & CIA/VIC-interrupts)
 unsigned char VersionH00;                //$04
 unsigned char Version;                   //$05 - 1 for PSID v1, 2..4 for PSID v2..4 or RSID v2..4 (3/4 has 2SID/3SID support), 0x4E for 4SID (WebSID-format)
 unsigned char HeaderSizeH00;             //$06
 unsigned char HeaderSize;                //$07 - $76 for v1, $7C for v2..4, with WebSID-format: $7E for 2SID, $80 for 3SID, $82 for 4SID (depends on number of SIDs)
 unsigned char LoadAddressH,LoadAddressL; //$08 - if 0 it's a PRG and its loadaddress is used (RSID: 0, PRG-loadaddress>=$07E8)
 unsigned char InitAddressH,InitAddressL; //$0A - if 0 it's taken from load-address (but should be set) (RSID: don't point to ROM, 0 if BASICflag set)
 unsigned char PlayAddressH,PlayAddressL; //$0C - if 0 play-routine-call is set by the initializer (always true for RSID)
 unsigned char SubtuneAmountH00;          //$0E
 unsigned char SubtuneAmount;             //$0F - 1..256
 unsigned char DefaultSubtuneH00;         //$10
 unsigned char DefaultSubtune;            //$11 - 1..256 (optional, defaults to 1)
 unsigned char SubtuneTimeSources[4];     //$12 - 0:Vsync / 1:CIA1 (for PSID) (LSB is subtune1, MSB above 32) , always 0 for RSID
 char          Title[32];                 //$16 - strings are using 1252 codepage
 char          Author[32];                //$36
 char          ReleaseInfo[32];           //$56
 //SID v2 additions:                              (if SID2/SID3 model is set to unknown, they're set to the same model as SID1)
 unsigned char ModelFormatStandardH;      //$76 - bit9&8/7&6/5&4: SID3/2/1 model (00:?,01:6581,10:8580,11:both) (4SID:bit6=SID1-channel), bit3&2:VideoStandard..
 unsigned char ModelFormatStandard;       //$77 ..(01:PAL,10:NTSC,11:both), bit1:(0:C64,1:PlaySIDsamples/RSID_BASICflag), bit0:(0:builtin-player,1:MUS)
 unsigned char RelocStartPage;            //$78 - v2NG specific, if 0 the SID doesn't write outside its data-range, if $FF there's no place for driver
 unsigned char RelocFreePages;            //$79 - size of area from RelocStartPage for driver-relocation (RSID: must not contain ROM or 0..$3FF)
 union {
  unsigned char SID2baseAddress;           //$7A - (SID2BASE-$d000)/16 //SIDv3-relevant, only $42..$FE values are valid ($d420..$DFE0), else no SID2
  unsigned char SID2flagsH;                //$7A: address of SID2 in WebSID-format too (same format as SID2baseAddress in HVSC format)
 };
 union {
  unsigned char SID3baseAddress;           //$7B - (SID3BASE-$d000)/16 //SIDv4-relevant, only $42..$FE values are valid ($d420..$DFE0), else no SID3
  unsigned char SID2flagsL;                //$7B: flags for WebSID-format, bit6: output-channel (0(default):left, 1:right, ?:both?), bit5..4:SIDmodel(00:setting,01:6581,10:8580,11:both)
                                             //   my own (implemented in SID-Wizard too) proposal for channel-info: bit7 should be 'middle' channel-flag (overriding bit6 left/right)
 };
 //WebSID-format (with 4 and more SIDs -support) additional fields: for each extra SID there's an 'nSIDflags' byte-pair
 unsigned char SID3flagsH, SID3flagsL; //$7C,$7D: the same address/flag-layout for SID3 as with SID2
 union { unsigned char SID4flagsH; unsigned char SID4baseAddress; }; //$7E
 unsigned char SID4flagsL; //$7F: the same address/flag-layout for SID4 as with SID2
 //... repeated for more SIDs, and end the list with $00,$00 (this determines the amount of SIDs)
} cRSID_SIDheader; //music-program follows right after the header


typedef struct cRSID_Interface { //public API variables aimed for being used from outside by a SID-player app
 //platform-related:
 unsigned char     CLImode; //boolean, if set, it disables GUI and select interactive command-line interface
 unsigned int      SampleBufferLength; //BufferSize; //amount of buffered samples (frames) for sound-playback, 2048 is usually a good choice
 unsigned char     HighQualitySID; //if set, it uses the cycle-based waveform-emulation with oversampling (with more CPU usage)
 unsigned char     HighQualityResampler; //use high-quality Sinc-based FIR resampler instead of fast averaging resampler
 unsigned char     Stereo; //boolean to set 2SID/3SID/4SID tunes as stereo
 unsigned char     PlaybackSpeed; //(1x is the default playback, 4x is a good choice for fast-forward)
 unsigned char     Paused; //boolean to pause SID-playback
 //C64-machine related:
 unsigned char     VideoStandard; //0:NTSC, 1:PAL (based on the SID-header field)
 unsigned short    SelectedSIDmodel; //can be set to 6581 or 8580, if set, this model will be used for every SID, no matter what
 unsigned char     MainVolume; //Sets the volume for the final audio output
 //SID-file related:
 int               SIDfileSize; //Read-Only
 union { //Read-Only
  cRSID_SIDheader*  SIDheader;
  char*             SIDfileData;
 };
 char              RealSIDmode; //Read-Only //has a shadowed copy in audio-thread, but only this one is to be written by main thread
 char              PSIDdigiMode; //Read-Only
 unsigned char     SubTune; //Read-Only (set internally by subtune-related API functions below)
 unsigned short    LoadAddress; //Read-Only
 unsigned short    InitAddress; //Read-Only
 unsigned short    PlayAddress; //Read-Only
 unsigned short    EndAddress; //Read-Only
 char              TimerSource; //Read-Only //for current subtune, 0:VIC, 1:CIA (as in SID-header)
 //PSID-playback related:
 int               FrameCycles; //Read-Only (can be used to calculate and display PSID framespeed)
 //playlist-related
 unsigned short    SubtuneDurations [CRSID_SUBTUNE_AMOUNT_MAX+1]; //Read-Only
 short             PlayTime; //Read-Only (playtime of curernt tune, to be displayed)
 char              PlaytimeExpired; //Read-Only (to detect if playtime of tune expired)
 char              AutoAdvance; //boolean to tell whether to advance to the next tune when tune playtime expires
 char              AutoExit; //boolean to tell whether to exit app when tune playtime expires
 char              FadeOut; //boolean to tell whether to fade out tune before advarce
 char              BuiltInMusic; //variable that tells if need to fall back to built-in music (e.g. when no arguments are passed to the app or no SID-file found)
 int               BuiltInMusicSize; //if 0, no built-in music processed
 const unsigned char* BuiltInMusicData; //pointer to the built-in musicdata array (if NULL, no musicdata)
 char              OpenedMusic; //a music was opened but not added to playlist, so playing it but not advancing to next tune in playlist, only next subtune if exists
 int               PlayListSize; //set/increased to the amount of items currently in playlist
 int               PlayListPlayPosition; //currently played playlist-entry
 char              PlayListAdvance; //Read-Only (playlist-initiated auto-advance after playtime-expiration)
 #ifdef CRSID_PLATFORM_PC //don't use this memory if no playlist-support (due to likely missing standard file-operations on embedded systems)
 int               PlayListDisplayPosition; //top of the playlist in GUI-display
 unsigned int      PlayListNumbering [CRSID_PLAYLIST_ENTRIES_MAX]; //the position-numbers preceding titles in the displayed playlist
 char              PlayList [CRSID_PLAYLIST_ENTRIES_MAX] [CRSID_PLAYLIST_ENTRY_SIZE];
 char              PlayTimeMinutes [CRSID_PLAYLIST_ENTRIES_MAX]; //these playtimes are only for giving in playlist-file, subtune-playtimes can come from a SID-Length txt database file if present
 char              PlayTimeSeconds [CRSID_PLAYLIST_ENTRIES_MAX];
 char              ChangingDirectory; //tell auto-advancer that directory is temporarily changed, and wait until it's changed back to where the app was ran from
 #endif
 int               FallbackPlayTime; //if no timing info is found, this can be used as a fallback playtime
 unsigned int      PlayListNumber; //variable for keeping current playlist-number (zeroed at startup)
 char*             SongLengths; //if a SongLengths database file is loaded, set this pointer to its memory-location (otherwise to an empty string)
 unsigned char*    KERNALfileData; //if KERNAL-ROM content is loaded from a file, its 8kbyte data resides here
 unsigned char*    BASICfileData; //if BASIC-ROM content is loaded from a file, its 8kbyte data resides here
} cRSID_Interface;


typedef struct cRSID_Output {
 signed int L;
 signed int R;
} cRSID_Output;


// Main public API functions (mainly in libcRSID.c)
void*              cRSID_init           (unsigned short samplerate, unsigned short buflen); //init emulation objects and sound
cRSID_SIDheader* cRSID_processSIDfileData (unsigned char* filedata, int filesize); //in host/file.c, copy SID-data to C64 memory
void               cRSID_initSIDtune    (cRSID_SIDheader* SIDheader, char subtune); //init tune/subtune
void               cRSID_generateSound  (FASTVAR unsigned char *buf, FASTVAR unsigned short len);
void               cRSID_syncGenSamples (int frametime); //if ALSA is initialized, call this periodically, and it emulates and sends samples or waits for the given 'frametime'
cRSID_Output       cRSID_generateSample (); //in host/audio.c, calculate a single sample
void               cRSID_close          (void); //close sound etc.

char*           cRSID_setSongLengthData (char* filedata);
unsigned char*     cRSID_setKERNALdata  (unsigned char* filedata);
unsigned char*     cRSID_setBASICdata   (unsigned char* filedata);

void               cRSID_playSIDtune    (void); //start/continue playback (enable playing audio-buffer)
void               cRSID_pauseSIDtune   (void); //pause playback (disable playing audio-buffer)
void               cRSID_startSubtune   (unsigned char subtune);
void               cRSID_nextSubtune    ();
void               cRSID_prevSubtune    ();

void               cRSID_autoAdvance    (); //called/callable periodically to check for playtime and if expired call callback function for auto-advance/etc
void               cRSID_setCallBack__autoAdvance ( void (*callback) (char subtunestepping, void* data), void* data ); //can trigger e.g. a SID-info update in the app if assigned

unsigned short     cRSID_getSIDbase     (int sid_number); //argument values: 1..4
unsigned short     cRSID_getSIDmodel    (int sid_number); //value: 6581 or 8580 (or 0/etc.)
unsigned short     cRSID_setSIDmodel    (int sid_number, unsigned short value); //value: 6581 or 8580 (or 0/etc.)
void               cRSID_setSIDmodels   (); //based on SIDheader-data
unsigned char      cRSID_getSIDchannel  (int sid_number); //channel in stereo field (left/right/middle)
int                cRSID_getSIDlevel    (int sid_number); //average level (for VU-meter display)

#ifdef CRSID_PLATFORM_PC
cRSID_SIDheader*   cRSID_playSIDfile    (char* filename, char subtune); //simple single-call SID playback
cRSID_SIDheader*   cRSID_loadSIDtune    (char* filename); //load and process SID-filedata to C64 memory
void              cRSID_setBuiltInMusic (const unsigned char* array, size_t size);
char            cRSID_startPlayListItem (int itemnumber);
void               cRSID_nextTune       ();
void               cRSID_prevTune       ();
unsigned int cRSID_findNextPlayableItem (int itemnumber);
unsigned int cRSID_findPreviousPlayableItem (int itemnumber);
void               cRSID_removeNewline  (char *name);
//in host/file.c
char*              cRSID_fileNameOnly   (char* path);
char*              cRSID_folderNameOnly (char* path);
char         cRSID_compareFileExtension (char *filename, char *extension);
char            cRSID_playableExtension (char *name);
#endif


extern cRSID_Interface cRSID; //the only public global object (for faster & simpler access than with struct-pointers, in some places)


#ifdef LIBCRSID_SOURCE //private/internal functions / variables


//void               cRSID_autoAdvance ();

#ifdef CRSID_PLATFORM_PC
//unsigned int       cRSID_findNextPlayableItem (int itemnumber);
//unsigned int       cRSID_findPreviousPlayableItem (int itemnumber);
//void               cRSID_removeNewline     (char *name);
#endif


#endif //LIBCRSID_SOURCE


#endif //LIBCRSID_HEADER
