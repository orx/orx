#ifndef LIBCRSID_HEADER__C64
#define LIBCRSID_HEADER__C64 //used  to prevent double inclusion of this header-file


#include "../Config.h"
#include "../Optimize.h"

#include "../libcRSID.h"
#include "../host/host.h"


#define CRSID_BYTE_LOG2(x) ((x)<2? 0: ((x)<4? 1: ((x)<8? 2: ((x)<16? 3: ((x)<32? 4: ((x)<64? 5: ((x)<128? 6: ((x)<256? 7: 8))))))))


enum cRSID_Specifications__C64 {
 CRSID_PAL_CPUCLK=985248, CRSID_NTSC_CPUCLK=1022727,
 CRSID_DEFAULT_SAMPLERATE=44100, CRSID_CLOCK_FRACTIONAL_BITS = 4, CRSID_RESAMPLER_FRACTIONAL_BITS = 12,
 //CRSID_OVERSAMPLING_RATIO=5, //comes from external define now (in Config.h/Config.mk/makefile)
 CRSID_OVERSAMPLING_CYCLES = ((CRSID_PAL_CPUCLK/CRSID_DEFAULT_SAMPLERATE)/CRSID_OVERSAMPLING_RATIO),
 CRSID_PAL_AUDIO_CLOCK = (CRSID_PAL_CPUCLK / CRSID_OVERSAMPLING_CYCLES),
 CRSID_NTSC_AUDIO_CLOCK = (CRSID_NTSC_CPUCLK / CRSID_OVERSAMPLING_CYCLES),
 CRSID_SIDCOUNT_MAX=4, CRSID_CIACOUNT=2
};
enum cRSID_Channels { CRSID_CHANNEL_LEFT=1, CRSID_CHANNEL_RIGHT=2, CRSID_CHANNEL_BOTH=3,  CRSID_CHANNELPANNING_DIVSHIFTS = 2 };
enum cRSID_AudioLevels {
 //attenuates wave-generator output not to overdrive resampler-input (and maybe filter-input):
 CRSID_WAVGEN_PRESHIFT=3, CRSID_WAVGEN_PREDIV = (1 << CRSID_WAVGEN_PRESHIFT), //shift-value can be 1..4 (1..16x division)
 //increase cRSID.Attenuation by nom/denom ratio, (adjusting of signal-strength before saturating sound to 16bit signed short to avoid strong multi-channel filter distortions):
 CRSID_PRESAT_ATT_NOM = 20, CRSID_PRESAT_ATT_DENOM=16
};
enum cRSID_MemAddresses {
 CRSID_C64_MEMBANK_SIZE = 0x10000, CRSID_MEMBANK_SAFETY_ZONE_SIZE = 0x100, CRSID_SID_SAFETY_ZONE_SIZE = 0x100,
 CRSID_MEMBANK_SIZE = (CRSID_C64_MEMBANK_SIZE + CRSID_MEMBANK_SAFETY_ZONE_SIZE + CRSID_SID_SAFETY_ZONE_SIZE),
 CRSID_SID_SAFE_ADDRESS = (CRSID_C64_MEMBANK_SIZE + CRSID_MEMBANK_SAFETY_ZONE_SIZE)
};


typedef struct cRSID_CPUinstance {
 //cRSID_C64instance* C64; //reference to the containing C64
 unsigned int       PC;
 short int          A, SP;
 unsigned char      X, Y, ST;  //STATUS-flags: N V - B D I Z C
 //extra temporary variables:
 unsigned int       Addr;
 unsigned int       Cycles, SamePage;
 unsigned char      PrevNMI; //used for NMI leading edge detection
} cRSID_CPUinstance;

typedef struct cRSID_CIAinstance {
 //cRSID_C64instance* C64;         //reference to the containing C64
 char               ChipModel;   //old or new CIA? (have 1 cycle difference in cases)
 unsigned short     BaseAddress; //CIA-baseaddress location in C64-memory (IO)
 unsigned char*     BasePtrWR;   //CIA-baseaddress location in host's memory for writing
 unsigned char*     BasePtrRD;   //CIA-baseaddress location in host's memory for reading
} cRSID_CIAinstance;

typedef struct cRSID_VICinstance {
 //cRSID_C64instance* C64;         //reference to the containing C64
 char               ChipModel;   //(timing differences between models?)
 unsigned short     BaseAddress; //VIC-baseaddress location in C64-memory (IO)
 unsigned char*     BasePtrWR;   //VIC-baseaddress location in host's memory for writing
 unsigned char*     BasePtrRD;   //VIC-baseaddress location in host's memory for reading
 unsigned short     RasterLines;
 unsigned char      RasterRowCycles;
 unsigned char      RowCycleCnt;
} cRSID_VICinstance;


typedef struct cRSID_SIDinstance {
 //SID-chip data:
 //cRSID_C64instance* C64;           //reference to the containing C64
 unsigned short     ChipModel;     //values: 8580 / 6581
 unsigned char      Channel;       //1:left, 2:right, 3:both(middle)
 unsigned short     BaseAddress;   //SID-baseaddress location in C64-memory (IO)
 unsigned char*     BasePtr;       //SID-baseaddress location in host's memory (for writing to SID)
 unsigned char*     BasePtrRD;     //SID-baseaddress location in host's memory (SID OSC3/ENV3 written here for readback)
 //ADSR-related:
 unsigned char      ADSRstate[15];
 unsigned short     RateCounter[15];
 unsigned char      EnvelopeCounter[15];
 unsigned char      ExponentCounter[15];
 //Wave-related:
 int                PhaseAccu[15];       //28bit precision instead of 24bit
 int                PrevPhaseAccu[15];   //(integerized ClockRatio fractionals, WebSID has similar solution)
 unsigned char      SyncSourceMSBrise;
 unsigned int       RingSourceMSB;
 unsigned int       NoiseLFSR[15];
 signed char        PrevSounDemonDigiWF[15];
 unsigned int       PrevWavGenOut[15];
 unsigned char      PrevWavData[15];
 //Filter-related:
 int                PrevLowPass;
 int                PrevBandPass;
 unsigned char      Volume; //pre-calculated once, used by oversampled/HQ output-emulation many times
 int                Digi; //pre-calculated once, used by oversampled/HQ output-emulation many times
 int                Resonance, Cutoff; //pre-calculated once, used by oversampled/HQ-filter many times
 unsigned char      HighPassBit, BandPassBit, LowPassBit; //pre-calculated once, used by oversampled/HQ-filter many times
 //Output-stage:
 int                NonFiltedSample;
 int                FilterInputSample;
 int                PrevNonFiltedSample;
 int                PrevFilterInputSample;
 signed int         PrevVolume; //lowpass-filtered version of Volume-band register
 int                Output;     //not attenuated (range:0..0xFFFFF depending on SID's main-volume)
 int                Level;      //filtered version, good for VU-meter display
} cRSID_SIDinstance;

typedef struct cRSID_SIDwavOutput {
 //union {
  signed int NonFilted;
  //signed int Mix; //filted and non-filted together (before main-volume attenuation)
 //};
 signed int FilterInput;
} cRSID_SIDwavOutput;


typedef struct cRSID_C64instance {
 //platform-related:
 unsigned short    SampleRate;
 unsigned int      SampleBufferSize; //calculated (by audio-init) amount of bytes in the buffer
 unsigned char*    SampleBuffer; //malloc-ed/freed by audio init/close routines
 void*             SoundDevice; //handle to the audio-device returned by the opener function in audio-init
 unsigned char     SoundStarted;
 char              RealSIDmode; //audio-thread's read-only shadow/cache-register for cRSID.RealSIDmode (which is the only one to be changed by main thread)
 unsigned char     Stereo; //audio-thread's read-only shadow/cache-register for cRSID.Stereo //boolean to set 2SID/3SID/4SID tunes as stereo
 unsigned char     HighQualitySID; //audio-thread's read-only shadow/cache-register for cRSID.HighQualitySID //if set, it uses the cycle-based waveform-emulation with oversampling (with more CPU usage)
 unsigned char     HighQualityResampler; //audio-thread's read-only shadow/cache-register for cRSID.HighQualityResampler //use high-quality Sinc-based FIR resampler instead of fast averaging resampler
 //C64-machine related:
 unsigned char     SIDchipCount; //not likely to change during playback, but just in case the audio-thread version is used
 unsigned char     AudioThread_SIDchipCount; //audio-thread's read-only shadow/cache-register for SIDchipCount
 //unsigned char     VideoStandard; //0:NTSC, 1:PAL (based on the SID-header field)
 unsigned int      CPUfrequency;
 unsigned short    SampleClockRatio; //ratio of CPU-clock and samplerate (for CPU/CIA/VIC/ADSR timing)
 unsigned short    OversampleClockRatio; //ratio of oversampling SID-clock and samplerate (for SID-waveform resampling)
 unsigned short    OversampleClockRatioReciproc; //ratio oversampling SID-clock and samplerate (for SID-waveform resampling)
 char              Finished;
 char              Returned;
 unsigned char     IRQ; //collected IRQ line from devices
 unsigned char     NMI; //collected NMI line from devices
 cRSID_Output      ResampledOutput;
 //SID-file related:
 char              FileNameOnly [CRSID_FILENAME_LEN_MAX];
 unsigned short    Attenuation;
 //PSID-playback related:
 //char              CIAisSet; //for dynamic CIA setting from player-routine (RealSID substitution)
 //int               FrameCycles;
 int               FrameCycleCnt; //this is a substitution in PSID-mode for CIA/VIC counters
 short             PrevRasterLine;
 short             SampleCycleCnt;
 short             OverSampleCycleCnt;
 //playlist-related
 short             TenthSecondCnt;
 unsigned short    SecondCnt;
 void              (*callBack__autoAdvance) (char subtunestepping, void* data);
 void*               callBackData__autoAdvance;
 char              FadeLevel;
 //Hardware-elements:
 cRSID_CPUinstance CPU;
 cRSID_SIDinstance SID[CRSID_SIDCOUNT_MAX+1];
 cRSID_CIAinstance CIA[CRSID_CIACOUNT+1];
 cRSID_VICinstance VIC;
 unsigned char* FASTPTR MemoryBankPointersRD [4] [256]; //fast lookup for memory-bank address-multiplexing done by the PLA
 unsigned char* FASTPTR MemoryBankPointersWR [4] [256]; //fast lookup for memory-bank address-multiplexing done by the PLA
 //Overlapping system memories, which one is read/written in an address region depends on CPU-port bankselect-bits)
 //Address $00 and $01 - data-direction and data-register of port built into CPU (used as bank-selection) (overriding RAM on C64)
 unsigned char RAMbank [CRSID_MEMBANK_SIZE];  //$0000..$FFFF RAM (and RAM under IO/ROM/CPUport)
 unsigned char IObankWR [CRSID_MEMBANK_SIZE]; //$D000..$DFFF IO-RAM (registers) to write (VIC/SID/CIA/ColorRAM/IOexpansion)
 unsigned char IObankRD [CRSID_MEMBANK_SIZE]; //$D000..$DFFF IO-RAM (registers) to read from (VIC/SID/CIA/ColorRAM/IOexpansion)
 unsigned char ROMbanks [CRSID_MEMBANK_SIZE]; //$1000..$1FFF/$9000..$9FFF (CHARGEN), $A000..$BFFF (BASIC), $E000..$FFFF (KERNAL)
} cRSID_C64instance;


//cRSID_C64instance cRSID_C64; //the only global object (for faster & simpler access than with struct-pointers, in some places)


// C64/C64.c
cRSID_C64instance*  cRSID_createC64     (unsigned short samplerate);
void                cRSID_setC64        (); //configure hardware (SIDs) for SID-tune
void                cRSID_initC64       (); //hard-reset
cRSID_Output*       cRSID_emulateC64    ();
// C64/C64_SIDrouting.c
static INLINE short cRSID_playPSIDdigi  ();

// C64/MEM.c
static INLINE unsigned char* cRSID_getMemReadPtr     (FASTVAR unsigned short address); //for global cSID_C64 fast-access
//static INLINE unsigned char* cRSID_getMemReadPtrC64  (cRSID_C64instance* C64, FASTVAR unsigned short address); //maybe slower
static INLINE unsigned char* cRSID_getMemWritePtr    (FASTVAR unsigned short address); //for global cSID_C64 fast-access
//static INLINE unsigned char* cRSID_getMemWritePtrC64 (cRSID_C64instance* C64, FASTVAR unsigned short address); //maybe slower
static INLINE unsigned char  cRSID_readMem     (FASTVAR unsigned short address); //for global cSID_C64 fast-access
//static INLINE unsigned char  cRSID_readMemC64  (cRSID_C64instance* C64, FASTVAR unsigned short address); //maybe slower
static INLINE void           cRSID_writeMem    (FASTVAR unsigned short address, FASTVAR unsigned char data); //for global cSID_C64 fast-access
//static INLINE void           cRSID_writeMemC64 (cRSID_C64instance* C64, FASTVAR unsigned short address, FASTVAR unsigned char data); //maybe slower
void                         cRSID_setROMcontent (); //KERNAL, BASIC
void                         cRSID_initMem       ();
// C64/CPU.c
void               cRSID_initCPU       (unsigned short mempos);
unsigned char      cRSID_emulateCPU    (void); //direct instances inside for hopefully faster operation
static INLINE char cRSID_handleCPUinterrupts ();
// C64/SID.c
void               cRSID_createSIDchip (cRSID_SIDinstance* SID, unsigned short model, char channel, unsigned short baseaddress);
void               cRSID_initSIDchip   (cRSID_SIDinstance* SID);
static INLINE int  cRSID_emulateHQresampledSID (FASTVAR cRSID_SIDinstance *const FASTPTR SID, FASTVAR char cycles);
// C64/SID_ADSR.c
void               cRSID_emulateADSRs  (FASTVAR cRSID_SIDinstance *const FASTPTR SID, FASTVAR char cycles);
// C64/SID_OscWaves.c
int             cRSID_emulateSID_light (FASTVAR cRSID_SIDinstance *const FASTPTR SID); //calls output-stage too
cRSID_SIDwavOutput cRSID_emulateHQwaves(FASTVAR cRSID_SIDinstance *const FASTPTR SID, FASTVAR char cycles); //, FASTVAR char filter);
// C64/SID_Outputs.c
static INLINE int  cRSID_emulateSIDoutputStage (FASTVAR cRSID_SIDinstance *const FASTPTR SID); //, FASTVAR char nofilter);
static INLINE void cRSID_precalculateHQoutputParameters (FASTVAR cRSID_SIDinstance *const FASTPTR SID); //for faster oversampled filter & attenuation
static INLINE int  cRSID_emulateHQresampledSIDoutputStage (FASTVAR cRSID_SIDinstance *const FASTPTR SID, FASTVAR cRSID_SIDwavOutput waves);
static INLINE void cRSID_emulateHQresampledSIDdigi (FASTVAR cRSID_SIDinstance *const FASTPTR SID, FASTVAR cRSID_Output *const FASTPTR signal);
// C64/CIA.c
void               cRSID_createCIAchip (cRSID_CIAinstance* CIA, unsigned short baseaddress);
void               cRSID_initCIAchip   (cRSID_CIAinstance* CIA);
static INLINE char cRSID_emulateCIA    (FASTVAR cRSID_CIAinstance *const FASTPTR CIA, FASTVAR char cycles);
static INLINE void cRSID_writeCIAlatchAhi  (FASTVAR cRSID_CIAinstance *const FASTPTR CIA, FASTVAR unsigned char value);
static INLINE void cRSID_writeCIAlatchBhi  (FASTVAR cRSID_CIAinstance *const FASTPTR CIA, FASTVAR unsigned char value);
static INLINE void cRSID_writeCIAIRQmask   (FASTVAR cRSID_CIAinstance *const FASTPTR CIA, FASTVAR unsigned char value);
static INLINE void cRSID_acknowledgeCIAIRQ (FASTVAR cRSID_CIAinstance *const FASTPTR CIA);
// C64/VIC.c
void               cRSID_createVICchip (unsigned short baseaddress);
void               cRSID_initVICchip   ();
static INLINE char cRSID_emulateVIC    (FASTVAR char cycles);
static INLINE void cRSID_acknowledgeVICrasterIRQ ();


#endif //LIBCRSID_HEADER__C64