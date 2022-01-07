/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2022 Orx-Project
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 */

/**
 * @file orxSoundSystem.c
 * @date 26/09/2021
 * @author iarwain@orx-project.org
 *
 * MiniAudio sound system plugin implementation
 *
 */


#include "orxPluginAPI.h"

#define STB_VORBIS_HEADER_ONLY
#define FILE                              void
#include "stb_vorbis.c"
#undef STB_VORBIS_HEADER_ONLY
#undef FILE

#ifdef __APPLE__
  #define MA_NO_RUNTIME_LINKING
#endif /* __APPLE__ */
#ifdef APIENTRY
  #undef APIENTRY
#endif /* APIENTRY */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
  void rpmalloc_thread_initialize();
  void rpmalloc_thread_finalize(int);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#define MA_ON_THREAD_ENTRY                rpmalloc_thread_initialize();
#define MA_ON_THREAD_EXIT                 rpmalloc_thread_finalize(1);

#define MA_NO_FLAC
#define MA_NO_GENERATION

#define MINIAUDIO_IMPLEMENTATION

#include "miniaudio.h"

#undef MA_NO_RUNTIME_LINKING
#undef MA_NO_FLAC
#undef MA_NO_GENERATION
#undef MA_ON_THREAD_ENTRY
#undef MA_ON_THREAD_EXIT
#undef MINIAUDIO_IMPLEMENTATION

#define FILE                              void
#define fopen(NAME, MODE)                 orxResource_Open(NAME, orxFALSE)
#define fread(BUFFER, SIZE, COUNT, FILE)  (orxResource_Read(FILE, SIZE * COUNT, BUFFER, orxNULL, orxNULL) / (SIZE))
#define fgetc(FILE)                       (orxResource_Read(FILE, 1, &c, orxNULL, orxNULL) <= 0) ? EOF : c & 0xFF /* Context-sensitive, single call site in stb_vorbis */
#define ftell(FILE)                       (orxU32)orxResource_Tell(FILE)
#define fseek(FILE, OFFSET, WHENCE)       (orxResource_Seek(FILE, OFFSET, (orxSEEK_OFFSET_WHENCE)WHENCE) < 0) ? 1 : 0
#define fclose(FILE)                      orxResource_Close(FILE)

#define malloc(SIZE)                      orxMemory_Allocate((orxU32)SIZE, orxMEMORY_TYPE_AUDIO)
#define realloc(MEMORY, SIZE)             orxMemory_Reallocate(MEMORY, (orxU32)SIZE, orxMEMORY_TYPE_AUDIO)
#define free(MEMORY)                      orxMemory_Free(MEMORY)

#define fopen_s(PTR, FILE, MODE)          fopen(FILE, MODE)

#ifndef __orxDEBUG__
  #undef NDEBUG
  #define NDEBUG
#endif /* !__orxDEBUG__ */

#include "stb_vorbis.c"

#undef FILE
#undef fopen
#undef fread
#undef fgetc
#undef ftell
#undef fseek
#undef fclose

#undef malloc
#undef realloc
#undef free

#undef fopen_s


/** Module flags
 */
#define orxSOUNDSYSTEM_KU32_STATIC_FLAG_NONE              0x00000000 /**< No flags */

#define orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY             0x00000001 /**< Ready flag */
#define orxSOUNDSYSTEM_KU32_STATIC_FLAG_RECORDING         0x00000002 /**< Recording flag */
#define orxSOUNDSYSTEM_KU32_STATIC_FLAG_STOP_RECORDING    0x00000004 /**< Stop recording flag */
#define orxSOUNDSYSTEM_KU32_STATIC_FLAG_BACKGROUND_MUTED  0x00000008 /**< Background muted flag */

#define orxSOUNDSYSTEM_KU32_STATIC_MASK_ALL               0xFFFFFFFF /**< All mask */


/** Misc defines
 */
#define orxSOUNDSYSTEM_KU32_BANK_SIZE                     128
#define orxSOUNDSYSTEM_KU32_DEFAULT_SAMPLE_RATE           48000
#define orxSOUNDSYSTEM_KU32_DEFAULT_CHANNEL_NUMBER        2
#define orxSOUNDSYSTEM_KU32_DEFAULT_LISTENER_NUMBER       0
#define orxSOUNDSYSTEM_KE_DEFAULT_LOG_LEVEL               MA_LOG_LEVEL_WARNING
#define orxSOUNDSYSTEM_KE_DEFAULT_FORMAT                  ma_format_f32
#define orxSOUNDSYSTEM_KF_DEFAULT_DIMENSION_RATIO         orx2F(0.01f)
#define orxSOUNDSYSTEM_KF_DEFAULT_THREAD_SLEEP_TIME       orx2F(0.001f)
#define orxSOUNDSYSTEM_KZ_THREAD_NAME                     "Sound"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Internal sample structure
 */
struct __orxSOUNDSYSTEM_SAMPLE_t
{
  const orxSTRING                 zName;
  orxU32                          u32ChannelNumber;
  orxU32                          u32FrameNumber;
  orxU32                          u32SampleRate;

  union
  {
    struct
    {
      ma_resource_manager_data_source stDataSource;
      orxSTRING                   zLocation;

    } stResource;
    struct
    {
      orxFLOAT                   *afSampleList;

    } stBuffer;
  };

  orxBOOL                         bIsBuffer;
};

/** Internal sound structure
 */
struct __orxSOUNDSYSTEM_SOUND_t
{
  ma_data_source_base             stBase;
  ma_sound                        stSound;
  orxHANDLE                       hUserData;
  const orxSTRING                 zName;
  ma_node_base                   *pstFilterNode;
  orxU32                          u32ChannelNumber;
  orxU32                          u32SampleRate;

  union
  {
    struct
    {
      ma_resource_manager_data_source stDataSource;
      orxHANDLE                   hOwner;
      orxFLOAT                   *afPendingSampleList;
      orxU32                      u32PendingSampleNumber;
      orxS32                      s32PacketID;

    } stStream;

    struct
    {
      ma_audio_buffer_ref         stDataSource;

    } stSample;
  };

  orxBOOL                         bIsStream   : 1;
  orxBOOL                         bIsStopped  : 1;
};

/** Internal filter structure
 */
typedef struct __orxSOUNDSYSTEM_FILTER_t
{
  union
  {
    ma_biquad_node                stBiquadNode;
    ma_lpf_node                   stLowPassNode;
    ma_hpf_node                   stHighPassNode;
    ma_bpf_node                   stBandPassNode;
    ma_loshelf_node               stLowShelfNode;
    ma_hishelf_node               stHighShelfNode;
    ma_notch_node                 stNotchNode;
    ma_peak_node                  stPeakingNode;
    ma_delay_node                 stDelayNode;

  } stNode;

  orxSOUND_FILTER_DATA            stData;
  orxBOOL                         bUseCustomParam;

} orxSOUNDSYSTEM_FILTER;

/** Static structure
 */
typedef struct __orxSOUNDSYSTEM_STATIC_t
{
  orxSOUND_EVENT_PAYLOAD          stRecordingPayload;     /**< Recording payload */
  ma_log                          stLog;                  /**< Log */
  ma_log_callback                 stLogCallback;          /**< Log callback */
  ma_vfs_callbacks                stCallbacks;            /**< Resource callbacks */
  ma_encoder                      stEncoder;              /**< Encoder */
  ma_context                      stContext;              /**< Context */
  ma_resource_manager_config      stResourceManagerConfig;/** < Resource manager config */
  ma_resource_manager             stResourceManager;      /**< Resource manager */
  ma_engine                       stEngine;               /**< Engine */
  ma_context                      stCaptureContext;       /**< Context */
  ma_device                       stCaptureDevice;        /**< Caoture device */
  ma_data_source_vtable           stStreamVTable;         /**< Stream VTable */
  ma_decoding_backend_vtable      stVorbisVTable;         /**< Vorbis decoding backend VTable */
  ma_decoding_backend_vtable     *apstVTable[1];          /**< Decoding backend VTable */
  orxBANK                        *pstSampleBank;          /**< Sample bank */
  orxBANK                        *pstSoundBank;           /**< Sound bank */
  orxBANK                        *pstFilterBank;          /**< Filter bank */
  volatile orxHANDLE              hRecordingResource;     /**< Recording resource */
  orxFLOAT                        fDimensionRatio;        /**< Dimension ration */
  orxFLOAT                        fRecDimensionRatio;     /**< Reciprocal dimension ratio */
  orxFLOAT                        fForegroundVolume;      /**< Foreground volume */
  orxU32                          u32ListenerNumber;      /**< Listener number */
  orxU32                          u32WorkerThread;        /**< Worker thread */
  orxU32                          u32Flags;               /**< Status flags */

} orxSOUNDSYSTEM_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxSOUNDSYSTEM_STATIC sstSoundSystem;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

static orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Entering background? */
  if(_pstEvent->eID == orxSYSTEM_EVENT_BACKGROUND)
  {
    /* Pushes config section */
    orxConfig_PushSection(orxSOUNDSYSTEM_KZ_CONFIG_SECTION);

    /* Should mute? */
    if((orxConfig_HasValue(orxSOUNDSYSTEM_KZ_CONFIG_MUTE_IN_BACKGROUND) == orxFALSE)
    || (orxConfig_GetBool(orxSOUNDSYSTEM_KZ_CONFIG_MUTE_IN_BACKGROUND) != orxFALSE))
    {
      /* Backups foreground volume */
      sstSoundSystem.fForegroundVolume = orxSoundSystem_GetGlobalVolume();

      /* Updates volume */
      orxSoundSystem_SetGlobalVolume(orxFLOAT_0);

      /* Updates status */
      orxFLAG_SET(sstSoundSystem.u32Flags, orxSOUNDSYSTEM_KU32_STATIC_FLAG_BACKGROUND_MUTED, orxSOUNDSYSTEM_KU32_STATIC_FLAG_NONE);
    }

    /* Pops config section */
    orxConfig_PopSection();
  }
  else
  {
    /* Was muted? */
    if(orxFLAG_TEST(sstSoundSystem.u32Flags, orxSOUNDSYSTEM_KU32_STATIC_FLAG_BACKGROUND_MUTED))
    {
      /* Updates status */
      orxFLAG_SET(sstSoundSystem.u32Flags, orxSOUNDSYSTEM_KU32_STATIC_FLAG_NONE, orxSOUNDSYSTEM_KU32_STATIC_FLAG_BACKGROUND_MUTED);

      /* Restores foreground volume */
      orxSoundSystem_SetGlobalVolume(sstSoundSystem.fForegroundVolume);
    }
  }

  /* Done! */
  return eResult;
}

static size_t orxSoundSystem_MiniAudio_Recording_Write(ma_encoder *_pstEncoder, const void *_pBufferIn, size_t _sBytesToWrite)
{
  size_t sResult;

  /* Writes data */
  sResult = (size_t)orxResource_Write((orxHANDLE)_pstEncoder->pUserData, _sBytesToWrite, _pBufferIn, orxNULL, orxNULL);

  /* Done! */
  return sResult;
}

static ma_bool32 orxSoundSystem_MiniAudio_Recording_Seek(ma_encoder *_pstEncoder, int _iByteOffset, ma_seek_origin _eOrigin)
{
  /* Seeks */
  return (orxResource_Seek((orxHANDLE)_pstEncoder->pUserData, (orxS64)_iByteOffset, (orxSEEK_OFFSET_WHENCE)_eOrigin) >= 0) ? MA_TRUE : MA_FALSE;
}

static orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_OpenRecordingFile()
{
  const orxSTRING zResourceLocation;
  orxHANDLE       hResource;
  orxSTATUS       eResult;

  /* Valid file to open? */
  if(((zResourceLocation = orxResource_LocateInStorage(orxSOUND_KZ_RESOURCE_GROUP, orxNULL, sstSoundSystem.stRecordingPayload.stStream.stInfo.zName)) != orxNULL)
  && ((hResource = orxResource_Open(zResourceLocation, orxTRUE)) != orxHANDLE_UNDEFINED))
  {
    ma_encoder_config stEncoderConfig;

    /* Inits encoder config */
    stEncoderConfig = ma_encoder_config_init(ma_encoding_format_wav, orxSOUNDSYSTEM_KE_DEFAULT_FORMAT, sstSoundSystem.stRecordingPayload.stStream.stInfo.u32ChannelNumber, sstSoundSystem.stRecordingPayload.stStream.stInfo.u32SampleRate);

    /* Inits encoder */
    if(ma_encoder_init(&orxSoundSystem_MiniAudio_Recording_Write, &orxSoundSystem_MiniAudio_Recording_Seek, hResource, &stEncoderConfig, &(sstSoundSystem.stEncoder)) == MA_SUCCESS)
    {
      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't write to file <%s> to record audio data.", sstSoundSystem.stRecordingPayload.stStream.stInfo.zName);

      /* Closes resource */
      orxResource_Close(hResource);

      /* Updates result */
      eResult = orxSTATUS_FAILURE;

    }
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;

    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't open file <%s> to write recorded audio data.", sstSoundSystem.stRecordingPayload.stStream.stInfo.zName);
  }

  /* Updates recording handle */
  orxMEMORY_BARRIER();
  sstSoundSystem.hRecordingResource = (eResult != orxSTATUS_FAILURE) ? hResource : orxNULL;

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_OpenRecordingFileTask(void *_pContext)
{
  /* Opens file */
  return orxSoundSystem_MiniAudio_OpenRecordingFile();
}

static orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_StopRecordingTask(void *_pContext)
{
  /* Stops recording */
  orxSoundSystem_StopRecording();

  /* Done! */
  return orxSTATUS_SUCCESS;
}

static void orxSoundSystem_MiniAudio_UpdateRecording(ma_device *_pstDevice, void *_pOutput, const void *_pInput, ma_uint32 _u32FrameCount)
{
  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxSoundSystem_UpdateRecording");

  /* Recording? */
  if(orxFLAG_GET(sstSoundSystem.u32Flags, orxSOUNDSYSTEM_KU32_STATIC_FLAG_RECORDING | orxSOUNDSYSTEM_KU32_STATIC_FLAG_STOP_RECORDING) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_RECORDING)
  {
    /* Is engine still running? (As this is called by a non-orx thread, we need to make sure orx's still running) */
    if(orxModule_IsInitialized(orxMODULE_ID_MAIN) != orxFALSE)
    {
      orxFLOAT fDT;

      /* Inits packet */
      sstSoundSystem.stRecordingPayload.stStream.stPacket.u32SampleNumber = _u32FrameCount * sstSoundSystem.stRecordingPayload.stStream.stInfo.u32ChannelNumber;
      sstSoundSystem.stRecordingPayload.stStream.stPacket.afSampleList    = (orxFLOAT *)_pInput;

      /* Sends event */
      orxEVENT_SEND(orxEVENT_TYPE_SOUND, orxSOUND_EVENT_RECORDING_PACKET, orxNULL, orxNULL, &(sstSoundSystem.stRecordingPayload));

      /* Should write the packet? */
      if(sstSoundSystem.stRecordingPayload.stStream.stPacket.bDiscard == orxFALSE)
      {
        /* No recording resource yet? */
        if(sstSoundSystem.hRecordingResource == orxNULL)
        {
          /* Inits it */
          sstSoundSystem.hRecordingResource = orxHANDLE_UNDEFINED;
          orxMEMORY_BARRIER();

          /* Asks for its opening on main thread */
          orxThread_RunTask(orxNULL, orxSoundSystem_MiniAudio_OpenRecordingFileTask, orxNULL, orxNULL);

          /* Waits for outcome */
          while(sstSoundSystem.hRecordingResource == orxHANDLE_UNDEFINED)
            ;
        }

        /* Has a valid recording resource? */
        if(sstSoundSystem.hRecordingResource != orxNULL)
        {
          ma_uint64 u64FrameCount, u64WrittenFrameNumber;
          ma_result hResult;

          /* Computes frame count */
          u64FrameCount = (ma_uint64)(sstSoundSystem.stRecordingPayload.stStream.stPacket.u32SampleNumber / sstSoundSystem.stRecordingPayload.stStream.stInfo.u32ChannelNumber);

          /* Writes data */
          hResult = ma_encoder_write_pcm_frames(&(sstSoundSystem.stEncoder), _pInput, u64FrameCount, &u64WrittenFrameNumber);
          orxASSERT((hResult == MA_SUCCESS) && (u64WrittenFrameNumber == u64FrameCount));
        }
      }

      /* Updates time and timestamp */
      fDT = orxU2F(sstSoundSystem.stRecordingPayload.stStream.stPacket.u32SampleNumber) / orxU2F(sstSoundSystem.stRecordingPayload.stStream.stInfo.u32SampleRate * sstSoundSystem.stRecordingPayload.stStream.stInfo.u32ChannelNumber);
      sstSoundSystem.stRecordingPayload.stStream.stPacket.fTime      += fDT;
      sstSoundSystem.stRecordingPayload.stStream.stPacket.fTimeStamp  = (orxFLOAT)orxSystem_GetTime();

      /* Was last? */
      if(sstSoundSystem.stRecordingPayload.stStream.stPacket.bLast != orxFALSE)
      {
        /* Updates status */
        orxFLAG_SET(sstSoundSystem.u32Flags, orxSOUNDSYSTEM_KU32_STATIC_FLAG_STOP_RECORDING, orxSOUNDSYSTEM_KU32_STATIC_FLAG_NONE);

        /* Postpones recording stop on main thread */
        orxThread_RunTask(orxNULL, orxSoundSystem_MiniAudio_StopRecordingTask, orxNULL, orxNULL);
      }
    }
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return;
}

static ma_result SoundSystem_MiniAudio_Stream_Read(ma_data_source *_pstDataSource, void *_pFramesOut, ma_uint64 _u64FrameCount, ma_uint64 *_pu64FramesRead)
{
  orxSOUNDSYSTEM_SOUND *pstSound;
  orxU32                u32CopySampleNumber = 0;
  ma_result             hResult = MA_SUCCESS;

  /* Retrieves associated sound */
  pstSound = (orxSOUNDSYSTEM_SOUND *)_pstDataSource;

  /* Checks */
  orxASSERT(pstSound->bIsStream != orxFALSE);

  /* Inits frames count */
  *_pu64FramesRead = 0;

  /* Has pending samples? */
  if(pstSound->stStream.u32PendingSampleNumber > 0)
  {
    /* Gets number of samples to copy */
    u32CopySampleNumber = orxMIN(pstSound->stStream.u32PendingSampleNumber, (orxU32)_u64FrameCount * pstSound->u32ChannelNumber);

    /* Copies them */
    orxMemory_Copy(_pFramesOut, pstSound->stStream.afPendingSampleList, u32CopySampleNumber * sizeof(orxFLOAT));

    /* Updates pending sample numbers */
    pstSound->stStream.u32PendingSampleNumber -= u32CopySampleNumber;

    /* Updates output data */
    _pFramesOut     = (void *)(((orxFLOAT *)_pFramesOut) + u32CopySampleNumber);
    _u64FrameCount -= (ma_uint64)u32CopySampleNumber / (ma_uint64)pstSound->u32ChannelNumber;

    /* Empty? */
    if(pstSound->stStream.u32PendingSampleNumber == 0)
    {
      /* Removes pending sample list */
      pstSound->stStream.afPendingSampleList = orxNULL;
    }
    else
    {
      /* Updates sample list */
      pstSound->stStream.afPendingSampleList += u32CopySampleNumber;
    }
  }

  /* Need new data? */
  if(_u64FrameCount > 0)
  {
    /* Has data source? */
    if(pstSound->stStream.stDataSource.flags != 0)
    {
      /* Fetches audio content */
      hResult = ma_data_source_read_pcm_frames(&(pstSound->stStream.stDataSource), _pFramesOut, _u64FrameCount, _pu64FramesRead);
    }
    else
    {
      /* Silences audio content */
      ma_silence_pcm_frames(_pFramesOut, _u64FrameCount, orxSOUNDSYSTEM_KE_DEFAULT_FORMAT, pstSound->u32ChannelNumber);

      /* Updates status */
      *_pu64FramesRead = _u64FrameCount;
    }

    /* Success? */
    if((hResult == MA_SUCCESS) || (hResult == MA_AT_END))
    {
      orxSOUND_EVENT_PAYLOAD stPayload;

      /* Clears payload */
      orxMemory_Zero(&stPayload, sizeof(orxSOUND_EVENT_PAYLOAD));

      /* Stores sound */
      stPayload.pstSound                          = orxSOUND(pstSound->hUserData);

      /* Stores stream info */
      stPayload.stStream.stInfo.u32SampleRate     = pstSound->u32SampleRate;
      stPayload.stStream.stInfo.u32ChannelNumber  = pstSound->u32ChannelNumber;
      stPayload.stStream.stInfo.zName             = pstSound->zName;

      /* Inits packet */
      stPayload.stStream.stPacket.afSampleList    = (orxFLOAT *)_pFramesOut;
      stPayload.stStream.stPacket.u32SampleNumber = ((orxU32)*_pu64FramesRead) * pstSound->u32ChannelNumber;
      stPayload.stStream.stPacket.fTimeStamp      = (orxFLOAT)orxSystem_GetTime();
      stPayload.stStream.stPacket.fTime           = orxSoundSystem_GetTime(pstSound);;
      stPayload.stStream.stPacket.s32ID           = pstSound->stStream.s32PacketID++;
      stPayload.stStream.stPacket.bDiscard        = orxFALSE;
      stPayload.stStream.stPacket.bLast           = (hResult == MA_AT_END) ? orxTRUE : orxFALSE;

      /* Doesn't have its owner yet? */
      if(pstSound->stStream.hOwner == orxNULL)
      {
        /* Stores it */
        pstSound->stStream.hOwner = orxStructure_GetOwner(orxStructure_GetOwner(pstSound->hUserData));
      }

      /* Sends event */
      orxEVENT_SEND(orxEVENT_TYPE_SOUND, orxSOUND_EVENT_PACKET, pstSound->stStream.hOwner, orxNULL, &stPayload);

      /* Should proceed? */
      if((stPayload.stStream.stPacket.bDiscard == orxFALSE)
      && (stPayload.stStream.stPacket.u32SampleNumber > 0))
      {
        /* Should copy? */
        if(stPayload.stStream.stPacket.afSampleList != _pFramesOut)
        {
          orxU32 u32SampleNumber;

          /* Gets number of samples to send */
          u32SampleNumber = orxMIN(stPayload.stStream.stPacket.u32SampleNumber, (orxU32)*_pu64FramesRead * pstSound->u32ChannelNumber);

          /* Copies them */
          orxMemory_Copy(_pFramesOut, stPayload.stStream.stPacket.afSampleList, u32SampleNumber * sizeof(orxFLOAT));

          /* Updates sample numbers */
          pstSound->stStream.u32PendingSampleNumber = stPayload.stStream.stPacket.u32SampleNumber - u32SampleNumber;

          /* Has pending samples? */
          if(pstSound->stStream.u32PendingSampleNumber > 0)
          {
            /* Stores pending samples */
            pstSound->stStream.afPendingSampleList = stPayload.stStream.stPacket.afSampleList + u32SampleNumber;
          }
        }
        else
        {
          /* Checks */
          orxASSERT(stPayload.stStream.stPacket.u32SampleNumber <= (orxU32)*_pu64FramesRead * pstSound->u32ChannelNumber);

          /* Updates status */
          *_pu64FramesRead = (ma_uint64)stPayload.stStream.stPacket.u32SampleNumber / pstSound->u32ChannelNumber;
        }
      }
      else
      {
        /* Silences a single frame */
        ma_silence_pcm_frames(_pFramesOut, 1, orxSOUNDSYSTEM_KE_DEFAULT_FORMAT, pstSound->u32ChannelNumber);

        /* Updates status */
        *_pu64FramesRead = 1;
      }

      /* Updates result */
      hResult = MA_SUCCESS;

      /* Ends of file? */
      if(stPayload.stStream.stPacket.bLast != orxFALSE)
      {
        /* Resets time */
        orxSoundSystem_SetTime(pstSound, orxFLOAT_0);

        /* Not looping? */
        if(orxSoundSystem_IsLooping(pstSound) == orxFALSE)
        {
          /* Updates result */
          hResult = MA_AT_END;
        }
      }
    }
  }

  /* Adjusts available samples number */
  *_pu64FramesRead += (ma_uint64)u32CopySampleNumber / pstSound->u32ChannelNumber;

  /* Done! */
  return hResult;
}

static ma_result SoundSystem_MiniAudio_Stream_Seek(ma_data_source *_pstDataSource, ma_uint64 _u64FrameIndex)
{
  orxSOUNDSYSTEM_SOUND *pstSound;
  ma_result             hResult;

  /* Retrieves associated sound */
  pstSound = (orxSOUNDSYSTEM_SOUND *)_pstDataSource;

  /* Checks */
  orxASSERT(pstSound->bIsStream != orxFALSE);

  /* Has data source? */
  if(pstSound->stStream.stDataSource.flags != 0)
  {
    /* Updates result */
    hResult = ma_data_source_seek_to_pcm_frame(&(pstSound->stStream.stDataSource), _u64FrameIndex);
  }
  else
  {
    /* Updates result */
    hResult = MA_NOT_IMPLEMENTED;
  }

  /* Done! */
  return hResult;
}

static ma_result SoundSystem_MiniAudio_Stream_GetDataFormat(ma_data_source *_pstDataSource, ma_format *_peFormat, ma_uint32 *_pu32Channels, ma_uint32* _pu32SampleRate, ma_channel *_peChannelMap, size_t _sChannelMapCap)
{
  orxSOUNDSYSTEM_SOUND *pstSound;
  ma_result             hResult = MA_SUCCESS;

  /* Retrieves associated sound */
  pstSound = (orxSOUNDSYSTEM_SOUND *)_pstDataSource;

  /* Checks */
  orxASSERT(pstSound->bIsStream != orxFALSE);

  /* Updates values */
  if(_peFormat != NULL)
  {
    *_peFormat = orxSOUNDSYSTEM_KE_DEFAULT_FORMAT;
  }
  if(_pu32Channels != NULL)
  {
    *_pu32Channels = pstSound->u32ChannelNumber;
  }
  if(_pu32SampleRate != NULL)
  {
    *_pu32SampleRate = pstSound->u32SampleRate;
  }
  if(_peChannelMap != NULL)
  {
    orxMemory_Zero(_peChannelMap, (orxU32)(_sChannelMapCap * sizeof(ma_channel)));
  }

  /* Done! */
  return hResult;
}

static ma_result SoundSystem_MiniAudio_Stream_GetCursor(ma_data_source *_pstDataSource, ma_uint64 *_pu64Cursor)
{
  orxSOUNDSYSTEM_SOUND *pstSound;
  ma_result             hResult;

  /* Retrieves associated sound */
  pstSound = (orxSOUNDSYSTEM_SOUND *)_pstDataSource;

  /* Checks */
  orxASSERT(pstSound->bIsStream != orxFALSE);

  /* Has data source? */
  if(pstSound->stStream.stDataSource.flags != 0)
  {
    /* Updates result */
    hResult = ma_data_source_get_cursor_in_pcm_frames(&(pstSound->stStream.stDataSource), _pu64Cursor);
  }
  else
  {
    /* Updates result */
    hResult = MA_NOT_IMPLEMENTED;
  }

  /* Done! */
  return hResult;
}

static ma_result SoundSystem_MiniAudio_Stream_GetLength(ma_data_source *_pstDataSource, ma_uint64 *_pu64Length)
{
  orxSOUNDSYSTEM_SOUND *pstSound;
  ma_result             hResult;

  /* Retrieves associated sound */
  pstSound = (orxSOUNDSYSTEM_SOUND *)_pstDataSource;

  /* Checks */
  orxASSERT(pstSound->bIsStream != orxFALSE);

  /* Has data source? */
  if(pstSound->stStream.stDataSource.flags != 0)
  {
    /* Updates result */
    hResult = ma_data_source_get_length_in_pcm_frames(&(pstSound->stStream.stDataSource), _pu64Length);
  }
  else
  {
    /* Updates result */
    hResult = MA_NOT_IMPLEMENTED;
  }

  /* Done! */
  return hResult;
}

static ma_result SoundSystem_MiniAudio_Stream_SetLooping(ma_data_source* _pstDataSource, ma_bool32 _bIsLooping)
{
  orxSOUNDSYSTEM_SOUND *pstSound;
  ma_result             hResult;

  /* Retrieves associated sound */
  pstSound = (orxSOUNDSYSTEM_SOUND *)_pstDataSource;

  /* Checks */
  orxASSERT(pstSound->bIsStream != orxFALSE);

  /* Has data source? */
  if(pstSound->stStream.stDataSource.flags != 0)
  {
    /* Updates result */
    hResult = ma_data_source_set_looping(&(pstSound->stStream.stDataSource), _bIsLooping);
  }
  else
  {
    /* Updates result */
    hResult = MA_NOT_IMPLEMENTED;
  }

  /* Done! */
  return hResult;
}

/*
 * This function's logic has been lifted straight from 'ma_stbvorbis_init_file', replacing the internal call to `stb_vorbis_open_filename` with `stb_vorbis_open_file`, bypassing the use of the pushdata API.
 */
static ma_result SoundSystem_MiniAudio_InitVorbis(ma_read_proc _pfnRead, ma_seek_proc _pfnSeek, ma_tell_proc _pfnTell, void *_pReadSeekTellUserData, const ma_decoding_backend_config *_pstConfig, const ma_allocation_callbacks *_pstAllocationCallbacks, ma_stbvorbis *_pstVorbis)
{
  ma_result hResult;

  /* Checks */
  orxASSERT(_pfnRead != NULL);
  orxASSERT(_pfnSeek != NULL);

  /* VFS? */
  if(((ma_decoder *)_pReadSeekTellUserData)->onRead == &ma_decoder__on_read_vfs)
  {
    /* Inits internals */
    hResult = ma_stbvorbis_init_internal(_pstConfig, _pstVorbis);

    /* Success? */
    if(hResult == MA_SUCCESS)
    {
      /* Inits callbacks */
      _pstVorbis->onRead                = _pfnRead;
      _pstVorbis->onSeek                = _pfnSeek;
      _pstVorbis->onTell                = _pfnTell;
      _pstVorbis->pReadSeekTellUserData = _pReadSeekTellUserData;
      ma_allocation_callbacks_init_copy(&(_pstVorbis->allocationCallbacks), _pstAllocationCallbacks);

      /* Inits vorbis decoder */
      _pstVorbis->stb = stb_vorbis_open_file(((ma_decoder*)_pReadSeekTellUserData)->data.vfs.file, FALSE, NULL, NULL);

      /* Success? */
      if(_pstVorbis->stb != NULL)
      {
        /* Updates status */
        _pstVorbis->usingPushMode = MA_FALSE;

        /* Executes post-init */
        hResult = ma_stbvorbis_post_init(_pstVorbis);

        /* Failure? */
        if(hResult != MA_SUCCESS)
        {
          /* Closes decoder */
          stb_vorbis_close(_pstVorbis->stb);
        }
      }
      else
      {
        /* Updates result */
        hResult = MA_INVALID_FILE;
      }
    }
  }
  else
  {
    /* Updates result */
    hResult = MA_ERROR;
  }

  /* Done! */
  return hResult;
}

/*
 * This function's logic has been lifted straight from 'ma_decoding_backend_init__stbvorbis', replacing the internal call to 'ma_stbvorbis_init' with 'SoundSystem_MiniAudio_InitVorbis'.
 */
static ma_result SoundSystem_MiniAudio_InitVorbisBackend(void *_pUserData, ma_read_proc _pfnRead, ma_seek_proc _pfnSeek, ma_tell_proc _pfnTell, void *_pReadSeekTellUserData, const ma_decoding_backend_config *_pstConfig, const ma_allocation_callbacks *_pstAllocationCallbacks, ma_data_source **_ppstBackend)
{
  ma_result     hResult;
  ma_stbvorbis *pstVorbis;

  /* Allocates the decoder backend */
  pstVorbis = (ma_stbvorbis *)ma_malloc(sizeof(ma_stbvorbis), _pstAllocationCallbacks);

  /* Success? */
  if(pstVorbis != NULL)
  {
    /* Inits decoder backend */
    hResult = SoundSystem_MiniAudio_InitVorbis(_pfnRead, _pfnSeek, _pfnTell, _pReadSeekTellUserData, _pstConfig, _pstAllocationCallbacks, pstVorbis);

    /* Success? */
    if(hResult == MA_SUCCESS)
    {
      /* Stores it */
      *_ppstBackend = pstVorbis;
    }
    else
    {
      /* Frees decoder backend */
      ma_free(pstVorbis, _pstAllocationCallbacks);
    }
  }
  else
  {
    /* Updates result */
    hResult = MA_OUT_OF_MEMORY;
  }

  /* Done! */
  return hResult;
}

static void SoundSystem_MiniAudio_UninitVorbisBackend(void *_pUserData, ma_data_source *_pstBackend, const ma_allocation_callbacks *_pstAllocationCallbacks)
{
  /* Uninits decoder backend */
  ma_stbvorbis_uninit((ma_stbvorbis *)_pstBackend, _pstAllocationCallbacks);

  /* Frees it */
  ma_free(_pstBackend, _pstAllocationCallbacks);

  /* Done! */
  return;
}

static orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_ProcessTask(void *_pContext)
{
  /* Processes next job */
  if(ma_resource_manager_process_next_job((ma_resource_manager *)_pContext) == MA_NO_DATA_AVAILABLE)
  {
    /* Sleeps */
    orxSystem_Delay(orxSOUNDSYSTEM_KF_DEFAULT_THREAD_SLEEP_TIME);
  }

  /* Done! */
  return orxSTATUS_SUCCESS;
}

static void orxSoundSystem_MiniAudio_Log(void *_pUserData, ma_uint32 _u32Level, const char *_zMessage)
{
  /* Warning or error? */
  if(_u32Level <= orxSOUNDSYSTEM_KE_DEFAULT_LOG_LEVEL)
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, orxANSI_KZ_COLOR_FG_YELLOW "[%s]" orxANSI_KZ_COLOR_FG_DEFAULT " %s", ma_log_level_to_string(_u32Level), _zMessage);
  }

  /* Done! */
  return;
}

static void *orxSoundSystem_MiniAudio_Allocate(size_t _sSize, void *_pContext)
{
  /* Done! */
  return orxMemory_Allocate((orxU32)_sSize, orxMEMORY_TYPE_AUDIO);
}

static void *orxSoundSystem_MiniAudio_Reallocate(void *_pMem, size_t _sSize, void *_pContext)
{
  /* Done! */
  return orxMemory_Reallocate(_pMem, (orxU32)_sSize, orxMEMORY_TYPE_AUDIO);
}

static void orxSoundSystem_MiniAudio_Free(void *p, void *_pContext)
{
  /* Done! */
  orxMemory_Free(p);
}

static ma_result SoundSystem_MiniAudio_Open(ma_vfs *_pstVFS, const char *_zFilename, ma_uint32 _u32OpenMode, ma_vfs_file *_pstFile)
{
    orxHANDLE hResource;
    ma_result hResult;

    /* Opens resource */
    hResource = orxResource_Open(_zFilename, orxFALSE);

    /* Success? */
    if(hResource != orxHANDLE_UNDEFINED)
    {
      /* Stores file */
      *_pstFile = (ma_vfs_file)hResource;

      /* Updates result */
      hResult = MA_SUCCESS;
    }
    else
    {
      /* Updates result */
      hResult = MA_ACCESS_DENIED;
    }

    /* Done! */
    return hResult;
}

static ma_result SoundSystem_MiniAudio_Close(ma_vfs *_pstVFS, ma_vfs_file _stFile)
{
  /* Closes resource */
  orxResource_Close((orxHANDLE)_stFile);

  /* Done! */
  return MA_SUCCESS;
}

static ma_result SoundSystem_MiniAudio_Read(ma_vfs *_pstVFS, ma_vfs_file _stFile, void *_pDst, size_t _sSizeInBytes, size_t *_psBytesRead)
{
  /* Reads data */
  *_psBytesRead = (size_t)orxResource_Read((orxHANDLE)_stFile, (orxS64)_sSizeInBytes, _pDst, orxNULL, orxNULL);

  /* Done! */
  return (*_psBytesRead == 0) ? MA_AT_END : MA_SUCCESS;
}

static ma_result SoundSystem_MiniAudio_Write(ma_vfs *_pstVFS, ma_vfs_file _stFile, const void *_pSrc, size_t _sSizeInBytes, size_t *_psBytesWritten)
{
  /* Writes data */
  *_psBytesWritten = (size_t)orxResource_Write((orxHANDLE)_stFile, _sSizeInBytes, _pSrc, orxNULL, orxNULL);

  /* Done! */
  return (*_psBytesWritten != _sSizeInBytes) ? MA_ACCESS_DENIED : MA_SUCCESS;
}

static ma_result SoundSystem_MiniAudio_Seek(ma_vfs *_pstVFS, ma_vfs_file _stFile, ma_int64 _s64Offset, ma_seek_origin _eOrigin)
{
  ma_result hResult;

  /* Seeks */
  hResult = (orxResource_Seek((orxHANDLE)_stFile, _s64Offset, (orxSEEK_OFFSET_WHENCE)_eOrigin) >= 0) ? MA_SUCCESS : MA_ERROR;

  /* Done! */
  return hResult;
}

static ma_result SoundSystem_MiniAudio_Tell(ma_vfs *_pstVFS, ma_vfs_file _stFile, ma_int64 *_ps64Cursor)
{
  orxS64    s64Cursor;
  ma_result hResult;

  /* Tells */
  s64Cursor = orxResource_Tell((orxHANDLE)_stFile);

  /* Valid? */
  if(s64Cursor >= 0)
  {
    /* Updates cursor */
    *_ps64Cursor = s64Cursor;

    /* Updates result */
    hResult = MA_SUCCESS;
  }
  else
  {
    /* Updates result */
    hResult = MA_ERROR;
  }

  /* Done! */
  return hResult;
}

static ma_result SoundSystem_MiniAudio_Info(ma_vfs *_pstVFS, ma_vfs_file _stFile, ma_file_info *_pstInfo)
{
  /* Gets size */
  _pstInfo->sizeInBytes = (ma_uint64)orxResource_GetSize((orxHANDLE)_stFile);

  /* Done! */
  return MA_SUCCESS;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Was already initialized? */
  if(!(sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY))
  {
    ma_result hResult;
    orxFLOAT  fRatio;

    /* Cleans static controller */
    orxMemory_Zero(&sstSoundSystem, sizeof(orxSOUNDSYSTEM_STATIC));

    /* Pushes config section */
    orxConfig_PushSection(orxSOUNDSYSTEM_KZ_CONFIG_SECTION);

    /* Retrieves dimension ratio */
    fRatio                          = orxConfig_GetFloat(orxSOUNDSYSTEM_KZ_CONFIG_RATIO);
    sstSoundSystem.fDimensionRatio  = (fRatio > orxFLOAT_0) ? fRatio : orxSOUNDSYSTEM_KF_DEFAULT_DIMENSION_RATIO;

    /* Stores it */
    orxConfig_SetFloat(orxSOUNDSYSTEM_KZ_CONFIG_RATIO, sstSoundSystem.fDimensionRatio);

    /* Retrieves listener count */
    sstSoundSystem.u32ListenerNumber= (orxConfig_HasValue(orxSOUNDSYSTEM_KZ_CONFIG_LISTENERS) != orxFALSE) ? orxConfig_GetU32(orxSOUNDSYSTEM_KZ_CONFIG_LISTENERS) : orxSOUNDSYSTEM_KU32_DEFAULT_LISTENER_NUMBER;

    /* Should clamp? */
    if(sstSoundSystem.u32ListenerNumber > MA_ENGINE_MAX_LISTENERS)
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Too many listeners have been requested [%u]: using the maximum of [%u] listeners instead.", sstSoundSystem.u32ListenerNumber, MA_ENGINE_MAX_LISTENERS);

      /* Updates listener count */
      sstSoundSystem.u32ListenerNumber = MA_ENGINE_MAX_LISTENERS;
    }

    /* Stores it */
    orxConfig_SetU32(orxSOUNDSYSTEM_KZ_CONFIG_LISTENERS, sstSoundSystem.u32ListenerNumber);

    /* Gets reciprocal dimension ratio */
    sstSoundSystem.fRecDimensionRatio = orxFLOAT_1 / sstSoundSystem.fDimensionRatio;

    /* Inits vorbis decoding backend VTable */
    sstSoundSystem.stVorbisVTable.onInit                                  = &SoundSystem_MiniAudio_InitVorbisBackend;
    sstSoundSystem.stVorbisVTable.onUninit                                = &SoundSystem_MiniAudio_UninitVorbisBackend;
    sstSoundSystem.apstVTable[0]                                          = &(sstSoundSystem.stVorbisVTable);

    /* Inits data source VTable */
    sstSoundSystem.stStreamVTable.onRead                                  = &SoundSystem_MiniAudio_Stream_Read;
    sstSoundSystem.stStreamVTable.onSeek                                  = &SoundSystem_MiniAudio_Stream_Seek;
    sstSoundSystem.stStreamVTable.onGetDataFormat                         = &SoundSystem_MiniAudio_Stream_GetDataFormat;
    sstSoundSystem.stStreamVTable.onGetCursor                             = &SoundSystem_MiniAudio_Stream_GetCursor;
    sstSoundSystem.stStreamVTable.onGetLength                             = &SoundSystem_MiniAudio_Stream_GetLength;
    sstSoundSystem.stStreamVTable.onSetLooping                            = &SoundSystem_MiniAudio_Stream_SetLooping;

    /* Inits resource callbacks */
    sstSoundSystem.stCallbacks.onOpen                                     = &SoundSystem_MiniAudio_Open;
    sstSoundSystem.stCallbacks.onClose                                    = &SoundSystem_MiniAudio_Close;
    sstSoundSystem.stCallbacks.onRead                                     = &SoundSystem_MiniAudio_Read;
    sstSoundSystem.stCallbacks.onWrite                                    = &SoundSystem_MiniAudio_Write;
    sstSoundSystem.stCallbacks.onSeek                                     = &SoundSystem_MiniAudio_Seek;
    sstSoundSystem.stCallbacks.onTell                                     = &SoundSystem_MiniAudio_Tell;
    sstSoundSystem.stCallbacks.onInfo                                     = &SoundSystem_MiniAudio_Info;

    /* Inits resource manager configuration */
    sstSoundSystem.stResourceManagerConfig                                = ma_resource_manager_config_init();
    sstSoundSystem.stResourceManagerConfig.decodedFormat                  = orxSOUNDSYSTEM_KE_DEFAULT_FORMAT;
    sstSoundSystem.stResourceManagerConfig.jobThreadCount                 = 0;
    sstSoundSystem.stResourceManagerConfig.flags                          = MA_RESOURCE_MANAGER_FLAG_NON_BLOCKING;
    sstSoundSystem.stResourceManagerConfig.ppCustomDecodingBackendVTables = sstSoundSystem.apstVTable;
    sstSoundSystem.stResourceManagerConfig.customDecodingBackendCount     = orxARRAY_GET_ITEM_COUNT(sstSoundSystem.apstVTable);
    sstSoundSystem.stResourceManagerConfig.pVFS                           = &(sstSoundSystem.stCallbacks);
    sstSoundSystem.stResourceManagerConfig.allocationCallbacks.onMalloc   = &orxSoundSystem_MiniAudio_Allocate;
    sstSoundSystem.stResourceManagerConfig.allocationCallbacks.onRealloc  = &orxSoundSystem_MiniAudio_Reallocate;
    sstSoundSystem.stResourceManagerConfig.allocationCallbacks.onFree     = &orxSoundSystem_MiniAudio_Free;

    /* Inits log system */
    hResult                                                               = ma_log_init(&(sstSoundSystem.stResourceManagerConfig.allocationCallbacks), &(sstSoundSystem.stLog));
    orxASSERT(hResult == MA_SUCCESS);
    sstSoundSystem.stLogCallback                                          = ma_log_callback_init(&orxSoundSystem_MiniAudio_Log, NULL);
    hResult                                                               = ma_log_register_callback(&(sstSoundSystem.stLog), sstSoundSystem.stLogCallback);
    orxASSERT(hResult == MA_SUCCESS);
    sstSoundSystem.stResourceManagerConfig.pLog                           = &(sstSoundSystem.stLog);

    /* Inits resource manager */
    hResult = ma_resource_manager_init(&(sstSoundSystem.stResourceManagerConfig), &(sstSoundSystem.stResourceManager));

    /* Success? */
    if(hResult == MA_SUCCESS)
    {
      ma_context_config stContextConfig;

      /* Inits context */
      stContextConfig       = ma_context_config_init();
      stContextConfig.pLog  = &(sstSoundSystem.stLog);
      hResult               = ma_context_init(NULL, 0, &stContextConfig, &(sstSoundSystem.stContext));

      /* Success? */
      if(hResult == MA_SUCCESS)
      {
        ma_engine_config stEngineConfig;

        /* Inits engine */
        stEngineConfig                  = ma_engine_config_init();
        stEngineConfig.pContext         = &(sstSoundSystem.stContext);
        stEngineConfig.pLog             = &(sstSoundSystem.stLog);
        stEngineConfig.pResourceManager = &(sstSoundSystem.stResourceManager);
        stEngineConfig.listenerCount    = sstSoundSystem.u32ListenerNumber;
        ma_allocation_callbacks_init_copy(&(stEngineConfig.allocationCallbacks), &(sstSoundSystem.stResourceManagerConfig.allocationCallbacks));
        hResult                         = ma_engine_init(&stEngineConfig, &(sstSoundSystem.stEngine));

        /* Success? */
        if(hResult == MA_SUCCESS)
        {
          /* Creates banks */
          sstSoundSystem.pstSampleBank  = orxBank_Create(orxSOUNDSYSTEM_KU32_BANK_SIZE, sizeof(orxSOUNDSYSTEM_SAMPLE), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_AUDIO);
          sstSoundSystem.pstSoundBank   = orxBank_Create(orxSOUNDSYSTEM_KU32_BANK_SIZE, sizeof(orxSOUNDSYSTEM_SOUND), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_AUDIO);
          sstSoundSystem.pstFilterBank  = orxBank_Create(orxSOUNDSYSTEM_KU32_BANK_SIZE, sizeof(orxSOUNDSYSTEM_FILTER), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_AUDIO);

          /* Valid? */
          if((sstSoundSystem.pstSampleBank != orxNULL) && (sstSoundSystem.pstSoundBank != orxNULL) && (sstSoundSystem.pstFilterBank != orxNULL))
          {
            /* Adds job thread */
            sstSoundSystem.u32WorkerThread = orxThread_Start(&orxSoundSystem_MiniAudio_ProcessTask, orxSOUNDSYSTEM_KZ_THREAD_NAME, &(sstSoundSystem.stResourceManager));

            /* Valid? */
            if(sstSoundSystem.u32WorkerThread != orxU32_UNDEFINED)
            {
              orxU32 i;

              /* Adds event handler */
              eResult = orxEvent_AddHandler(orxEVENT_TYPE_SYSTEM, orxSoundSystem_MiniAudio_EventHandler);
              orxASSERT(eResult != orxSTATUS_FAILURE);

              /* Filters relevant event IDs */
              orxEvent_SetHandlerIDFlags(orxSoundSystem_MiniAudio_EventHandler, orxEVENT_TYPE_SYSTEM, orxNULL, orxEVENT_GET_FLAG(orxSYSTEM_EVENT_BACKGROUND) | orxEVENT_GET_FLAG(orxSYSTEM_EVENT_FOREGROUND), orxEVENT_KU32_MASK_ID_ALL);

              /* For all listeners */
              for(i = 0; i < sstSoundSystem.u32ListenerNumber; i++)
              {
                /* Inits it */
                ma_engine_listener_set_position(&(sstSoundSystem.stEngine), i, 0.0f, 0.0f, 0.0f);
                ma_engine_listener_set_direction(&(sstSoundSystem.stEngine), i, 0.0f, 0.0f, 1.0f);
                ma_engine_listener_set_velocity(&(sstSoundSystem.stEngine), i, 0.0f, 0.0f, 0.0f);
                ma_engine_listener_set_world_up(&(sstSoundSystem.stEngine), i, 0.0f, -1.0f, 0.0f);
                ma_engine_listener_set_cone(&(sstSoundSystem.stEngine), i, orxMATH_KF_2_PI, orxMATH_KF_2_PI, 1.0f);
              }

              /* Updates status */
              orxFLAG_SET(sstSoundSystem.u32Flags, orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY, orxSOUNDSYSTEM_KU32_STATIC_MASK_ALL);

              /* Updates result */
              eResult = orxSTATUS_SUCCESS;
            }
            else
            {
              /* Deletes banks */
              orxBank_Delete(sstSoundSystem.pstSampleBank);
              orxBank_Delete(sstSoundSystem.pstSoundBank);
              orxBank_Delete(sstSoundSystem.pstFilterBank);
              sstSoundSystem.pstSampleBank  = orxNULL;
              sstSoundSystem.pstSoundBank   = orxNULL;
              sstSoundSystem.pstFilterBank  = orxNULL;

              /* Uninits engine */
              ma_engine_uninit(&(sstSoundSystem.stEngine));

              /* Uninits context */
              ma_context_uninit(&(sstSoundSystem.stContext));

              /* Uninits resource manager */
              ma_resource_manager_uninit(&(sstSoundSystem.stResourceManager));

              /* Uninits log */
              ma_log_uninit(&(sstSoundSystem.stLog));
            }
          }
          else
          {
            /* Deletes banks */
            if(sstSoundSystem.pstSampleBank != orxNULL)
            {
              orxBank_Delete(sstSoundSystem.pstSampleBank);
              sstSoundSystem.pstSampleBank = orxNULL;
            }
            if(sstSoundSystem.pstSoundBank != orxNULL)
            {
              orxBank_Delete(sstSoundSystem.pstSoundBank);
              sstSoundSystem.pstSoundBank = orxNULL;
            }
            if(sstSoundSystem.pstFilterBank != orxNULL)
            {
              orxBank_Delete(sstSoundSystem.pstFilterBank);
              sstSoundSystem.pstFilterBank = orxNULL;
            }

            /* Uninits engine */
            ma_engine_uninit(&(sstSoundSystem.stEngine));

            /* Uninits context */
            ma_context_uninit(&(sstSoundSystem.stContext));

            /* Uninits resource manager */
            ma_resource_manager_uninit(&(sstSoundSystem.stResourceManager));

            /* Uninits log */
            ma_log_uninit(&(sstSoundSystem.stLog));
          }
        }
        else
        {
          /* Uninits context */
          ma_context_uninit(&(sstSoundSystem.stContext));

          /* Uninits resource manager */
          ma_resource_manager_uninit(&(sstSoundSystem.stResourceManager));

          /* Uninits log */
          ma_log_uninit(&(sstSoundSystem.stLog));
        }
      }
      else
      {
        /* Uninits resource manager */
        ma_resource_manager_uninit(&(sstSoundSystem.stResourceManager));

        /* Uninits log */
        ma_log_uninit(&(sstSoundSystem.stLog));
      }
    }
    else
    {
      /* Uninits log */
      ma_log_uninit(&(sstSoundSystem.stLog));
    }

    /* Pops config section */
    orxConfig_PopSection();
  }

  /* Done! */
  return eResult;
}

void orxFASTCALL orxSoundSystem_MiniAudio_Exit()
{
  /* Was initialized? */
  if(sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY)
  {
    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_SYSTEM, orxSoundSystem_MiniAudio_EventHandler);

    /* Stops any recording */
    orxSoundSystem_StopRecording();

    /* Joins worker thread */
    orxThread_Join(sstSoundSystem.u32WorkerThread);

    /* Uninits engine */
    ma_engine_uninit(&(sstSoundSystem.stEngine));

    /* Uninits context */
    ma_context_uninit(&(sstSoundSystem.stContext));

    /* Uninits resource manager */
    ma_resource_manager_uninit(&(sstSoundSystem.stResourceManager));

    /* Uninits log */
    ma_log_uninit(&(sstSoundSystem.stLog));

    /* Deletes banks */
    orxBank_Delete(sstSoundSystem.pstSampleBank);
    orxBank_Delete(sstSoundSystem.pstSoundBank);
    orxBank_Delete(sstSoundSystem.pstFilterBank);

    /* Cleans static controller */
    orxMemory_Zero(&sstSoundSystem, sizeof(orxSOUNDSYSTEM_STATIC));
  }

  /* Done! */
  return;
}

orxSOUNDSYSTEM_SAMPLE *orxFASTCALL orxSoundSystem_MiniAudio_CreateSample(orxU32 _u32ChannelNumber, orxU32 _u32FrameNumber, orxU32 _u32SampleRate)
{
  orxSOUNDSYSTEM_SAMPLE *pstResult = orxNULL;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  /* Valid parameters? */
  if((_u32ChannelNumber >= 1) && (_u32FrameNumber > 0) && (_u32SampleRate > 0))
  {
    /* Allocates sample */
    pstResult = (orxSOUNDSYSTEM_SAMPLE *)orxBank_Allocate(sstSoundSystem.pstSampleBank);

    /* Valid? */
    if(pstResult != orxNULL)
    {
      orxU32 u32BufferSize;

      /* Gets buffer size */
      u32BufferSize = _u32FrameNumber * _u32ChannelNumber * sizeof(orxFLOAT);

      /* Allocates buffer */
      pstResult->stBuffer.afSampleList = (orxFLOAT *)orxMemory_Allocate(u32BufferSize, orxMEMORY_TYPE_AUDIO);

      /* Success? */
      if(pstResult->stBuffer.afSampleList != orxNULL)
      {
        /* Clears it */
        orxMemory_Zero(pstResult->stBuffer.afSampleList, u32BufferSize);

        /* Clears name */
        pstResult->zName             = orxSTRING_EMPTY;

        /* Stores info */
        pstResult->u32ChannelNumber  = _u32ChannelNumber;
        pstResult->u32FrameNumber    = _u32FrameNumber;
        pstResult->u32SampleRate     = _u32SampleRate;

        /* Updates status */
        pstResult->bIsBuffer         = orxTRUE;
      }
      else
      {
        /* Deletes sample */
        orxBank_Free(sstSoundSystem.pstSampleBank, pstResult);

        /* Updates result */
        pstResult = orxNULL;

        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't create sound sample: can't allocate memory for data.");
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't create sound sample: can't allocate memory.");
    }
  }

  /* Done! */
  return pstResult;
}

orxSOUNDSYSTEM_SAMPLE *orxFASTCALL orxSoundSystem_MiniAudio_LoadSample(const orxSTRING _zFilename)
{
  orxSOUNDSYSTEM_SAMPLE *pstResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_zFilename != orxNULL);

  /* Allocates sample */
  pstResult = (orxSOUNDSYSTEM_SAMPLE *)orxBank_Allocate(sstSoundSystem.pstSampleBank);

  /* Valid? */
  if(pstResult != orxNULL)
  {
    const orxSTRING zResourceLocation;

    /* Clears data */
    orxMemory_Zero(pstResult, sizeof(orxSOUNDSYSTEM_SAMPLE));

    /* Locates resource */
    zResourceLocation = orxResource_Locate(orxSOUND_KZ_RESOURCE_GROUP, _zFilename);

    /* Success? */
    if(zResourceLocation != orxNULL)
    {
      ma_result hResult;

      /* Inits data source */
      hResult = ma_resource_manager_data_source_init(&(sstSoundSystem.stResourceManager), zResourceLocation, MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_WAIT_INIT | MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_DECODE | MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_ASYNC, NULL, &(pstResult->stResource.stDataSource));

      /* Success? */
      if(hResult == MA_SUCCESS)
      {
        ma_format eFormat;
        ma_uint64 u64Length;

        /* Stores name */
        pstResult->zName = orxString_Store(_zFilename);

        /* Stores location */
        pstResult->stResource.zLocation = orxString_Duplicate(zResourceLocation);

        /* Retrieves channel number & sample rate */
        hResult = ma_resource_manager_data_source_get_data_format((ma_resource_manager_data_source *)&(pstResult->stResource.stDataSource), &(eFormat), (ma_uint32 *)&(pstResult->u32ChannelNumber), (ma_uint32 *)&(pstResult->u32SampleRate), NULL, 0);
        orxASSERT(hResult == MA_SUCCESS);

        /* Gets length */
        hResult = ma_resource_manager_data_source_get_length_in_pcm_frames((ma_resource_manager_data_source *)&(pstResult->stResource.stDataSource), &u64Length);
        orxASSERT(hResult == MA_SUCCESS);

        /* Stores frame number */
        pstResult->u32FrameNumber = (orxU32)u64Length;

        /* Updates status */
        pstResult->bIsBuffer = orxFALSE;
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't load sound sample <%s>: can't process resource [%s].", _zFilename, zResourceLocation);

        /* Frees sample */
        orxBank_Free(sstSoundSystem.pstSampleBank, pstResult);

        /* Updates result */
        pstResult = orxNULL;
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't load sound sample <%s>: can't locate resource.", _zFilename);

      /* Frees sample */
      orxBank_Free(sstSoundSystem.pstSampleBank, pstResult);

      /* Updates result */
      pstResult = orxNULL;
    }
  }

  /* Done! */
  return pstResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_DeleteSample(orxSOUNDSYSTEM_SAMPLE *_pstSample)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSample != orxNULL);

  /* Is a buffer? */
  if(_pstSample->bIsBuffer != orxFALSE)
  {
    /* Frees it */
    orxMemory_Free(_pstSample->stBuffer.afSampleList);
  }
  else
  {
    /* Uninits data source */
    ma_resource_manager_data_source_uninit(&(_pstSample->stResource.stDataSource));

    /* Deletes location */
    orxString_Delete(_pstSample->stResource.zLocation);
  }

  /* Deletes sample */
  orxBank_Free(sstSoundSystem.pstSampleBank, _pstSample);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_GetSampleInfo(const orxSOUNDSYSTEM_SAMPLE *_pstSample, orxU32 *_pu32ChannelNumber, orxU32 *_pu32FrameNumber, orxU32 *_pu32SampleRate)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSample != orxNULL);
  orxASSERT(_pu32ChannelNumber != orxNULL);
  orxASSERT(_pu32FrameNumber != orxNULL);
  orxASSERT(_pu32SampleRate != orxNULL);

  /* Updates result */
  *_pu32ChannelNumber = _pstSample->u32ChannelNumber;
  *_pu32FrameNumber   = _pstSample->u32FrameNumber;
  *_pu32SampleRate    = _pstSample->u32SampleRate;

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_SetSampleData(orxSOUNDSYSTEM_SAMPLE *_pstSample, const orxFLOAT *_afData, orxU32 _u32SampleNumber)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSample != orxNULL);
  orxASSERT(_afData != orxNULL);

  /* Is buffer? */
  if(_pstSample->bIsBuffer != orxFALSE)
  {
    /* Valid size? */
    if(_u32SampleNumber == _pstSample->u32ChannelNumber * _pstSample->u32FrameNumber)
    {
      /* Copies data */
      orxMemory_Copy(_pstSample->stBuffer.afSampleList, _afData, _u32SampleNumber * sizeof(orxFLOAT));

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
  }

  /* Done! */
  return eResult;
}

orxSOUNDSYSTEM_SOUND *orxFASTCALL orxSoundSystem_MiniAudio_CreateFromSample(orxHANDLE _hUserData, const orxSOUNDSYSTEM_SAMPLE *_pstSample)
{
  orxSOUNDSYSTEM_SOUND *pstResult = orxNULL;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT((_hUserData != orxNULL) && (_hUserData != orxHANDLE_UNDEFINED));
  orxASSERT(_pstSample != orxNULL);

  /* Allocates sound */
  pstResult = (orxSOUNDSYSTEM_SOUND *)orxBank_Allocate(sstSoundSystem.pstSoundBank);

  /* Valid? */
  if(pstResult != orxNULL)
  {
    ma_sound_config stSoundConfig;
    ma_result       hResult = MA_SUCCESS;

    /* Clears it */
    orxMemory_Zero(pstResult, sizeof(orxSOUNDSYSTEM_SOUND));

    /* Inits sound config */
    stSoundConfig             = ma_sound_config_init();
    stSoundConfig.channelsOut = ma_engine_get_channels(&(sstSoundSystem.stEngine));

    /* Is buffer? */
    if(_pstSample->bIsBuffer != orxFALSE)
    {
      /* Updates sound config*/
      stSoundConfig.flags       = (sstSoundSystem.u32ListenerNumber == 0) ? MA_SOUND_FLAG_NO_SPATIALIZATION : 0;
      stSoundConfig.pDataSource = &(pstResult->stSample.stDataSource);

      /* Creates buffer data source */
      hResult = ma_audio_buffer_ref_init(orxSOUNDSYSTEM_KE_DEFAULT_FORMAT, _pstSample->u32ChannelNumber, _pstSample->stBuffer.afSampleList, (ma_uint64)_pstSample->u32FrameNumber, &(pstResult->stSample.stDataSource));
    }
    else
    {
      /* Updates sound config*/
      stSoundConfig.flags     = MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC | ((sstSoundSystem.u32ListenerNumber == 0) ? MA_SOUND_FLAG_NO_SPATIALIZATION : 0);
      stSoundConfig.pFilePath = _pstSample->stResource.zLocation;
    }

    /* Success? */
    if(hResult == MA_SUCCESS)
    {
      /* Creates sound */
      hResult = ma_sound_init_ex(&(sstSoundSystem.stEngine), &stSoundConfig, &(pstResult->stSound));

      /* Success? */
      if(hResult == MA_SUCCESS)
      {
        /* Stores filter node */
        pstResult->pstFilterNode    = (ma_node_base *)&(pstResult->stSound.engineNode);

        /* Stores user data */
        pstResult->hUserData        = _hUserData;

        /* Stores name */
        pstResult->zName            = _pstSample->zName;

        /* Stores channel number & sample rate */
        pstResult->u32ChannelNumber = _pstSample->u32ChannelNumber;
        pstResult->u32SampleRate    = _pstSample->u32SampleRate;

        /* Updates status */
        pstResult->bIsStream        = orxFALSE;
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't create sound from sample [%s].", _pstSample->zName);

        /* Deletes sound */
        orxBank_Free(sstSoundSystem.pstSoundBank, pstResult);

        /* Updates result */
        pstResult = orxNULL;
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't create sound from sample [%s]: can't initialize buffer data source.", _pstSample->zName);

      /* Deletes sound */
      orxBank_Free(sstSoundSystem.pstSoundBank, pstResult);

      /* Updates result */
      pstResult = orxNULL;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't allocate memory for creating sound.");
  }

  /* Done! */
  return pstResult;
}

orxSOUNDSYSTEM_SOUND *orxFASTCALL orxSoundSystem_MiniAudio_CreateStream(orxHANDLE _hUserData, orxU32 _u32ChannelNumber, orxU32 _u32SampleRate)
{
  orxSOUNDSYSTEM_SOUND *pstResult = orxNULL;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT((_hUserData != orxNULL) && (_hUserData != orxHANDLE_UNDEFINED));

  /* Valid parameters? */
  if((_u32ChannelNumber >= 1) && (_u32ChannelNumber <= 2) && (_u32SampleRate > 0))
  {
    /* Allocates sound */
    pstResult = (orxSOUNDSYSTEM_SOUND *)orxBank_Allocate(sstSoundSystem.pstSoundBank);

    /* Valid? */
    if(pstResult != orxNULL)
    {
      ma_data_source_config stBaseConfig;
      ma_result             hResult;

      /* Clears it */
      orxMemory_Zero(pstResult, sizeof(orxSOUNDSYSTEM_SOUND));

      /* Inits base data source config */
      stBaseConfig        = ma_data_source_config_init();
      stBaseConfig.vtable = &(sstSoundSystem.stStreamVTable);

      /* Inits base data source */
      hResult = ma_data_source_init(&stBaseConfig, &(pstResult->stBase));

      /* Success? */
      if(hResult == MA_SUCCESS)
      {
        ma_sound_config stSoundConfig;

        /* Inits sound config */
        stSoundConfig               = ma_sound_config_init();
        stSoundConfig.pDataSource   = &(pstResult->stBase);
        stSoundConfig.channelsOut   = ma_engine_get_channels(&(sstSoundSystem.stEngine));
        stSoundConfig.flags         = (sstSoundSystem.u32ListenerNumber == 0) ? MA_SOUND_FLAG_NO_SPATIALIZATION : 0;

        /* Stores info */
        pstResult->u32ChannelNumber = _u32ChannelNumber;
        pstResult->u32SampleRate    = _u32SampleRate;

        /* Updates status */
        pstResult->bIsStream = orxTRUE;

        /* Creates sound */
        hResult = ma_sound_init_ex(&(sstSoundSystem.stEngine), &stSoundConfig, &(pstResult->stSound));

        /* Success? */
        if(hResult == MA_SUCCESS)
        {
          /* Stores user data */
          pstResult->hUserData = _hUserData;

          /* Stores name */
          pstResult->zName = orxSTRING_EMPTY;
        }
        else
        {
          /* Uninits base data source */
          ma_data_source_uninit(&(pstResult->stBase));

          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't create sound stream with [%u] channels @ %u Hz: can't initialize sound structure.", _u32ChannelNumber, _u32SampleRate);

          /* Deletes sound */
          orxBank_Free(sstSoundSystem.pstSoundBank, pstResult);

          /* Updates result */
          pstResult = orxNULL;
        }
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't create sound stream with [%u] channels @ %u Hz: can't initialize internal data source.", _u32ChannelNumber, _u32SampleRate);

        /* Deletes sound */
        orxBank_Free(sstSoundSystem.pstSoundBank, pstResult);

        /* Updates result */
        pstResult = orxNULL;
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't create sound stream with [%u] channels @ %u Hz: can't allocate sound structure.", _u32ChannelNumber, _u32SampleRate);
    }
  }

  /* Done! */
  return pstResult;
}

orxSOUNDSYSTEM_SOUND *orxFASTCALL orxSoundSystem_MiniAudio_CreateStreamFromFile(orxHANDLE _hUserData, const orxSTRING _zFilename)
{
  const orxSTRING       zResourceLocation;
  orxSOUNDSYSTEM_SOUND *pstResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT((_hUserData != orxNULL) && (_hUserData != orxHANDLE_UNDEFINED));
  orxASSERT(_zFilename != orxNULL);

  /* Locates resource */
  zResourceLocation = orxResource_Locate(orxSOUND_KZ_RESOURCE_GROUP, _zFilename);

  /* Success? */
  if(zResourceLocation != orxNULL)
  {
    /* Allocates sound */
    pstResult = (orxSOUNDSYSTEM_SOUND *)orxBank_Allocate(sstSoundSystem.pstSoundBank);

    /* Valid? */
    if(pstResult != orxNULL)
    {
      ma_result hResult;

      /* Clears it */
      orxMemory_Zero(pstResult, sizeof(orxSOUNDSYSTEM_SOUND));

      /* Inits data source */
      hResult = ma_resource_manager_data_source_init(&(sstSoundSystem.stResourceManager), zResourceLocation, MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_WAIT_INIT | MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_STREAM, NULL, &(pstResult->stStream.stDataSource));

      /* Success? */
      if(hResult == MA_SUCCESS)
      {
        ma_data_source_config stBaseConfig;

        /* Inits base data source config */
        stBaseConfig        = ma_data_source_config_init();
        stBaseConfig.vtable = &(sstSoundSystem.stStreamVTable);

        /* Inits base data source */
        hResult = ma_data_source_init(&stBaseConfig, &(pstResult->stBase));

        /* Success? */
        if(hResult == MA_SUCCESS)
        {
          ma_sound_config stSoundConfig;

          /* Inits sound config */
          stSoundConfig             = ma_sound_config_init();
          stSoundConfig.pDataSource = &(pstResult->stBase);
          stSoundConfig.channelsOut = ma_engine_get_channels(&(sstSoundSystem.stEngine));
          stSoundConfig.flags       = (sstSoundSystem.u32ListenerNumber == 0) ? MA_SOUND_FLAG_NO_SPATIALIZATION : 0;

          /* Stores info */
          hResult = ma_data_source_get_data_format(&(pstResult->stStream.stDataSource), NULL, (ma_uint32 *)&(pstResult->u32ChannelNumber), (ma_uint32 *)&(pstResult->u32SampleRate), NULL, 0);
          orxASSERT(hResult == MA_SUCCESS);

          /* Updates status */
          pstResult->bIsStream = orxTRUE;

          /* Creates sound */
          hResult = ma_sound_init_ex(&(sstSoundSystem.stEngine), &stSoundConfig, &(pstResult->stSound));

          /* Success? */
          if(hResult == MA_SUCCESS)
          {
            /* Stores user data */
            pstResult->hUserData = _hUserData;

            /* Stores name */
            pstResult->zName = orxString_Store(_zFilename);
          }
          else
          {
            /* Uninits base data source */
            ma_data_source_uninit(&(pstResult->stBase));

            /* Uninits data source */
            ma_resource_manager_data_source_uninit(&(pstResult->stStream.stDataSource));

            /* Logs message */
            orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't load sound stream <%s>: can't process resource [%s].", _zFilename, zResourceLocation);

            /* Deletes sound */
            orxBank_Free(sstSoundSystem.pstSoundBank, pstResult);

            /* Updates result */
            pstResult = orxNULL;
          }
        }
        else
        {
          /* Uninits data source */
          ma_resource_manager_data_source_uninit(&(pstResult->stStream.stDataSource));

          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't load sound stream <%s>: can't initialize internal data source.", _zFilename);

          /* Deletes sound */
          orxBank_Free(sstSoundSystem.pstSoundBank, pstResult);

          /* Updates result */
          pstResult = orxNULL;
        }
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't load sound stream <%s>: can't initialize resource data source for location [%s].", _zFilename, zResourceLocation);

        /* Deletes sound */
        orxBank_Free(sstSoundSystem.pstSoundBank, pstResult);

        /* Updates result */
        pstResult = orxNULL;
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't load sound stream <%s>: can't allocate sound structure.", _zFilename);

      /* Updates result */
      pstResult = orxNULL;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't load sound stream <%s>: can't locate resource.", _zFilename);

    /* Updates result */
    pstResult = orxNULL;
  }

  /* Done! */
  return pstResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_Delete(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Uninits sound */
  ma_sound_uninit(&(_pstSound->stSound));

  /* Removes all filters */
  orxSoundSystem_RemoveAllFilters(_pstSound);

  /* Has base data source? */
  if(_pstSound->stBase.vtable != NULL)
  {
    /* Uninits base data source */
    ma_data_source_uninit(&(_pstSound->stBase));
  }

  /* Is stream? */
  if(_pstSound->bIsStream != orxFALSE)
  {
    /* Has data source? */
    if(_pstSound->stStream.stDataSource.flags != 0)
    {
      /* Uninits it */
      ma_resource_manager_data_source_uninit(&(_pstSound->stStream.stDataSource));
    }
  }
  else
  {
    /* Has data source? */
    if(_pstSound->stSample.stDataSource.pData != NULL)
    {
      /* Uninits it */
      ma_audio_buffer_ref_uninit(&(_pstSound->stSample.stDataSource));
    }
  }

  /* Deletes sound */
  orxBank_Free(sstSoundSystem.pstSoundBank, _pstSound);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_Play(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Plays sound */
  eResult = (ma_sound_start(&(_pstSound->stSound)) == MA_SUCCESS) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;

  /* Success? */
  if(eResult != orxSTATUS_FAILURE)
  {
    /* Updates status */
    _pstSound->bIsStopped = orxFALSE;
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_Pause(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Pauses sound */
  eResult = (ma_sound_stop(&(_pstSound->stSound)) == MA_SUCCESS) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_Stop(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Stops sound */
  if(ma_sound_stop(&(_pstSound->stSound)) == MA_SUCCESS)
  {
    /* Rewinds it */
    eResult = (ma_sound_seek_to_pcm_frame(&(_pstSound->stSound), 0) == MA_SUCCESS) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;

    /* Success? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Updates engine node's local time */
      ma_node_set_time((ma_node *)&(_pstSound->stSound.engineNode), 0);

      /* Is a stream? */
      if(_pstSound->bIsStream != orxFALSE)
      {
        /* Clears pending samples */
        _pstSound->stStream.u32PendingSampleNumber = 0;
      }

      /* Updates status */
      _pstSound->bIsStopped = orxTRUE;
    }
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_AddFilter(orxSOUNDSYSTEM_SOUND *_pstSound, const orxSOUND_FILTER_DATA *_pstFilterData, orxBOOL _bUseCustomParam)
{
  orxSOUNDSYSTEM_FILTER  *pstFilter;
  orxSTATUS               eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);
  orxASSERT(_pstFilterData != orxNULL);

  /* Allocates filter */
  pstFilter = (orxSOUNDSYSTEM_FILTER *)orxBank_Allocate(sstSoundSystem.pstFilterBank);

  /* Success? */
  if(pstFilter != orxNULL)
  {
    ma_result hResult;

    /* Depending on filter type */
    switch(_pstFilterData->eType)
    {
      /* Biquad */
      case orxSOUND_FILTER_TYPE_BIQUAD:
      {
        ma_biquad_node_config stConfig;

        /* Inits its config */
        stConfig = ma_biquad_node_config_init(ma_engine_get_channels(&(sstSoundSystem.stEngine)), _pstFilterData->stBiquad.fB0, _pstFilterData->stBiquad.fB1, _pstFilterData->stBiquad.fB2, _pstFilterData->stBiquad.fA0, _pstFilterData->stBiquad.fA1, _pstFilterData->stBiquad.fA2);

        /* Inits it */
        hResult = ma_biquad_node_init(ma_engine_get_node_graph(&(sstSoundSystem.stEngine)), &stConfig, &(sstSoundSystem.stResourceManagerConfig.allocationCallbacks), (ma_biquad_node *)(&pstFilter->stNode));

        break;
      }

      /* Low Pass */
      case orxSOUND_FILTER_TYPE_LOW_PASS:
      {
        ma_lpf_node_config stConfig;

        /* Inits its config */
        stConfig = ma_lpf_node_config_init(ma_engine_get_channels(&(sstSoundSystem.stEngine)), _pstSound->u32SampleRate, _pstFilterData->stLowPass.fFrequency, _pstFilterData->stLowPass.u32Order);

        /* Inits it */
        hResult = ma_lpf_node_init(ma_engine_get_node_graph(&(sstSoundSystem.stEngine)), &stConfig, &(sstSoundSystem.stResourceManagerConfig.allocationCallbacks), (ma_lpf_node *)(&pstFilter->stNode));

        break;
      }

      /* High Pass */
      case orxSOUND_FILTER_TYPE_HIGH_PASS:
      {
        ma_hpf_node_config stConfig;

        /* Inits its config */
        stConfig = ma_hpf_node_config_init(ma_engine_get_channels(&(sstSoundSystem.stEngine)), _pstSound->u32SampleRate, _pstFilterData->stHighPass.fFrequency, _pstFilterData->stHighPass.u32Order);

        /* Inits it */
        hResult = ma_hpf_node_init(ma_engine_get_node_graph(&(sstSoundSystem.stEngine)), &stConfig, &(sstSoundSystem.stResourceManagerConfig.allocationCallbacks), (ma_hpf_node *)(&pstFilter->stNode));

        break;
      }

      /* Band Pass */
      case orxSOUND_FILTER_TYPE_BAND_PASS:
      {
        ma_bpf_node_config stConfig;

        /* Inits its config */
        stConfig = ma_bpf_node_config_init(ma_engine_get_channels(&(sstSoundSystem.stEngine)), _pstSound->u32SampleRate, _pstFilterData->stBandPass.fFrequency, _pstFilterData->stBandPass.u32Order);

        /* Inits it */
        hResult = ma_bpf_node_init(ma_engine_get_node_graph(&(sstSoundSystem.stEngine)), &stConfig, &(sstSoundSystem.stResourceManagerConfig.allocationCallbacks), (ma_bpf_node *)(&pstFilter->stNode));

        break;
      }

      /* Low Shelf */
      case orxSOUND_FILTER_TYPE_LOW_SHELF:
      {
        ma_loshelf_node_config stConfig;

        /* Inits its config */
        stConfig = ma_loshelf_node_config_init(ma_engine_get_channels(&(sstSoundSystem.stEngine)), _pstSound->u32SampleRate, _pstFilterData->stLowShelf.fGain, _pstFilterData->stLowShelf.fQ, _pstFilterData->stLowShelf.fFrequency);

        /* Inits it */
        hResult = ma_loshelf_node_init(ma_engine_get_node_graph(&(sstSoundSystem.stEngine)), &stConfig, &(sstSoundSystem.stResourceManagerConfig.allocationCallbacks), (ma_loshelf_node *)(&pstFilter->stNode));

        break;
      }

      /* High Shelf */
      case orxSOUND_FILTER_TYPE_HIGH_SHELF:
      {
        ma_hishelf_node_config stConfig;

        /* Inits its config */
        stConfig = ma_hishelf_node_config_init(ma_engine_get_channels(&(sstSoundSystem.stEngine)), _pstSound->u32SampleRate, _pstFilterData->stHighShelf.fGain, _pstFilterData->stHighShelf.fQ, _pstFilterData->stHighShelf.fFrequency);

        /* Inits it */
        hResult = ma_hishelf_node_init(ma_engine_get_node_graph(&(sstSoundSystem.stEngine)), &stConfig, &(sstSoundSystem.stResourceManagerConfig.allocationCallbacks), (ma_hishelf_node *)(&pstFilter->stNode));

        break;
      }

      /* Notch */
      case orxSOUND_FILTER_TYPE_NOTCH:
      {
        ma_notch_node_config stConfig;

        /* Inits its config */
        stConfig = ma_notch_node_config_init(ma_engine_get_channels(&(sstSoundSystem.stEngine)), _pstSound->u32SampleRate, _pstFilterData->stNotch.fQ, _pstFilterData->stNotch.fFrequency);

        /* Inits it */
        hResult = ma_notch_node_init(ma_engine_get_node_graph(&(sstSoundSystem.stEngine)), &stConfig, &(sstSoundSystem.stResourceManagerConfig.allocationCallbacks), (ma_notch_node *)(&pstFilter->stNode));

        break;
      }

      /* Peaking */
      case orxSOUND_FILTER_TYPE_PEAKING:
      {
        ma_peak_node_config stConfig;

        /* Inits its config */
        stConfig = ma_peak_node_config_init(ma_engine_get_channels(&(sstSoundSystem.stEngine)), _pstSound->u32SampleRate, _pstFilterData->stPeaking.fGain, _pstFilterData->stPeaking.fQ, _pstFilterData->stPeaking.fFrequency);

        /* Inits it */
        hResult = ma_peak_node_init(ma_engine_get_node_graph(&(sstSoundSystem.stEngine)), &stConfig, &(sstSoundSystem.stResourceManagerConfig.allocationCallbacks), (ma_peak_node *)(&pstFilter->stNode));

        break;
      }

      /* Delay */
      case orxSOUND_FILTER_TYPE_DELAY:
      {
        ma_delay_node_config stConfig;

        /* Inits its config */
        stConfig = ma_delay_node_config_init(ma_engine_get_channels(&(sstSoundSystem.stEngine)), _pstSound->u32SampleRate, orxF2U(orxU2F(_pstSound->u32SampleRate) * _pstFilterData->stDelay.fDelay), _pstFilterData->stDelay.fDecay);

        /* Inits it */
        hResult = ma_delay_node_init(ma_engine_get_node_graph(&(sstSoundSystem.stEngine)), &stConfig, &(sstSoundSystem.stResourceManagerConfig.allocationCallbacks), (ma_delay_node *)(&pstFilter->stNode));

        break;
      }

      default:
      {
        /* Updates result */
        hResult = MA_ERROR;
        break;
      }
    }

    /* Success? */
    if(hResult == MA_SUCCESS)
    {
      ma_node *pstOutputNode;

      /* Retrieves output node */
      orxASSERT(_pstSound->pstFilterNode->outputBusCount > 0);
      pstOutputNode = _pstSound->pstFilterNode->pOutputBuses[0].pInputNode;

      /* Attaches filter output */
      hResult = ma_node_attach_output_bus(&(pstFilter->stNode), 0, pstOutputNode, 0);

      /* Success? */
      if(hResult == MA_SUCCESS)
      {
        /* Attaches filter input */
        hResult = ma_node_attach_output_bus(_pstSound->pstFilterNode, 0, &(pstFilter->stNode), 0);

        /* Success? */
        if(hResult == MA_SUCCESS)
        {
          /* Updates filter node */
          _pstSound->pstFilterNode = (ma_node_base *)(&pstFilter->stNode);

          /* Updates filter */
          orxMemory_Copy(&(pstFilter->stData), _pstFilterData, sizeof(orxSOUND_FILTER_DATA));
          pstFilter->bUseCustomParam = _bUseCustomParam;

          /* Updates result */
          eResult = orxSTATUS_SUCCESS;
        }
        else
        {
          /* Restores previous connection */
          hResult = ma_node_attach_output_bus(_pstSound->pstFilterNode, 0, pstOutputNode, 0);
          orxASSERT(hResult == MA_SUCCESS);

          /* Deletes filter */
          orxBank_Free(sstSoundSystem.pstFilterBank, pstFilter);
        }
      }
      else
      {
        /* Deletes filter */
        orxBank_Free(sstSoundSystem.pstFilterBank, pstFilter);
      }
    }
    else
    {
      /* Deletes filter */
      orxBank_Free(sstSoundSystem.pstFilterBank, pstFilter);
    }
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_RemoveLastFilter(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  //! TODO

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_RemoveAllFilters(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  //! TODO

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_StartRecording(const orxSTRING _zName, orxBOOL _bWriteToFile, orxU32 _u32SampleRate, orxU32 _u32ChannelNumber)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_zName != orxNULL);

  /* Not already recording? */
  if(!orxFLAG_TEST(sstSoundSystem.u32Flags, orxSOUNDSYSTEM_KU32_STATIC_FLAG_RECORDING))
  {
    ma_context_config stContextConfig;
    ma_result         hResult;

    /* Clears recording payload */
    orxMemory_Zero(&(sstSoundSystem.stRecordingPayload), sizeof(orxSOUND_EVENT_PAYLOAD));

    /* Stores recording name */
    sstSoundSystem.stRecordingPayload.stStream.stInfo.zName             = orxString_Duplicate(_zName);

    /* Stores stream info */
    sstSoundSystem.stRecordingPayload.stStream.stInfo.u32SampleRate     = (_u32SampleRate > 0)      ? _u32SampleRate    : orxSOUNDSYSTEM_KU32_DEFAULT_SAMPLE_RATE;
    sstSoundSystem.stRecordingPayload.stStream.stInfo.u32ChannelNumber  = (_u32ChannelNumber != 0)  ? _u32ChannelNumber : orxSOUNDSYSTEM_KU32_DEFAULT_CHANNEL_NUMBER;

    /* Stores status */
    sstSoundSystem.stRecordingPayload.stStream.stPacket.bDiscard        = (_bWriteToFile != orxFALSE) ? orxFALSE : orxTRUE;
    sstSoundSystem.stRecordingPayload.stStream.stPacket.bLast           = orxFALSE;

    /* Inits context */
    stContextConfig       = ma_context_config_init();
    stContextConfig.pLog  = &(sstSoundSystem.stLog);
    hResult               = ma_context_init(NULL, 0, &stContextConfig, &(sstSoundSystem.stCaptureContext));

    /* Success? */
    if(hResult == MA_SUCCESS)
    {
      ma_device_config stDeviceConfig;

      /* Inits device config */
      stDeviceConfig                  = ma_device_config_init(ma_device_type_capture);
      stDeviceConfig.sampleRate       = sstSoundSystem.stRecordingPayload.stStream.stInfo.u32SampleRate;
      stDeviceConfig.capture.channels = sstSoundSystem.stRecordingPayload.stStream.stInfo.u32ChannelNumber;
      stDeviceConfig.capture.format   = orxSOUNDSYSTEM_KE_DEFAULT_FORMAT;
      stDeviceConfig.dataCallback     = &orxSoundSystem_MiniAudio_UpdateRecording;

      /* Inits capture device */
      hResult = ma_device_init(&(sstSoundSystem.stCaptureContext), &stDeviceConfig, &(sstSoundSystem.stCaptureDevice));

      /* Success? */
      if(hResult == MA_SUCCESS)
      {
        /* Starts capture device */
        hResult = ma_device_start(&(sstSoundSystem.stCaptureDevice));

        /* Success? */
        if(hResult == MA_SUCCESS)
        {
          /* Should record? */
          if(_bWriteToFile != orxFALSE)
          {
            /* Opens file for recording */
            eResult = orxSoundSystem_MiniAudio_OpenRecordingFile();
          }
          else
          {
            /* Updates result */
            eResult = orxSTATUS_SUCCESS;
          }

          /* Success? */
          if(eResult != orxSTATUS_FAILURE)
          {
            /* Updates packet's timestamp and time */
            sstSoundSystem.stRecordingPayload.stStream.stPacket.fTimeStamp  = (orxFLOAT)orxSystem_GetTime();
            sstSoundSystem.stRecordingPayload.stStream.stPacket.fTime       = orxFLOAT_0;

            /* Updates status */
            orxFLAG_SET(sstSoundSystem.u32Flags, orxSOUNDSYSTEM_KU32_STATIC_FLAG_RECORDING, orxSOUNDSYSTEM_KU32_STATIC_FLAG_NONE);

            /* Sends event */
            orxEVENT_SEND(orxEVENT_TYPE_SOUND, orxSOUND_EVENT_RECORDING_START, orxNULL, orxNULL, &(sstSoundSystem.stRecordingPayload));
          }
          else
          {
            /* Stops capture device */
            ma_device_stop(&(sstSoundSystem.stCaptureDevice));

            /* Uninits capture device */
            ma_device_uninit(&(sstSoundSystem.stCaptureDevice));

            /* Uninits capture context */
            ma_context_uninit(&(sstSoundSystem.stCaptureContext));

            /* Logs message */
            orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't start recording <%s>: can't start capture device.", _zName);

            /* Updates result */
            eResult = orxSTATUS_FAILURE;
          }
        }
        else
        {
          /* Uninits capture device */
          ma_device_uninit(&(sstSoundSystem.stCaptureDevice));

          /* Uninits capture context */
          ma_context_uninit(&(sstSoundSystem.stCaptureContext));

          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't start recording <%s>: can't start capture device.", _zName);

          /* Updates result */
          eResult = orxSTATUS_FAILURE;
        }
      }
      else
      {
        /* Uninits capture context */
        ma_context_uninit(&(sstSoundSystem.stCaptureContext));

        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't start recording <%s>: can't init capture device.", _zName);

        /* Updates result */
        eResult = orxSTATUS_FAILURE;
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't start recording <%s>: can't init capture context.", _zName);

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't start recording <%s> as the recording of <%s> is still in progress.", _zName, sstSoundSystem.stRecordingPayload.stStream.stInfo.zName);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_StopRecording()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  /* Is recording? */
  if(orxFLAG_TEST(sstSoundSystem.u32Flags, orxSOUNDSYSTEM_KU32_STATIC_FLAG_RECORDING))
  {
    /* Updates status */
    orxFLAG_SET(sstSoundSystem.u32Flags, orxSOUNDSYSTEM_KU32_STATIC_FLAG_NONE, orxSOUNDSYSTEM_KU32_STATIC_FLAG_RECORDING | orxSOUNDSYSTEM_KU32_STATIC_FLAG_STOP_RECORDING);
    orxMEMORY_BARRIER();

    /* Has a recording file? */
    if(sstSoundSystem.hRecordingResource != orxNULL)
    {
      /* Closes it */
      orxResource_Close(sstSoundSystem.hRecordingResource);
      sstSoundSystem.hRecordingResource = orxNULL;
    }

    /* Stops capture device */
    ma_device_stop(&(sstSoundSystem.stCaptureDevice));

    /* Uninits capture device */
    ma_device_uninit(&(sstSoundSystem.stCaptureDevice));

    /* Uninits capture context */
    ma_context_uninit(&(sstSoundSystem.stCaptureContext));

    /* Resets the packet */
    sstSoundSystem.stRecordingPayload.stStream.stPacket.u32SampleNumber = 0;
    sstSoundSystem.stRecordingPayload.stStream.stPacket.afSampleList    = orxNULL;

    /* Sends event */
    orxEVENT_SEND(orxEVENT_TYPE_SOUND, orxSOUND_EVENT_RECORDING_STOP, orxNULL, orxNULL, &(sstSoundSystem.stRecordingPayload));

    /* Deletes name */
    orxString_Delete((orxSTRING)sstSoundSystem.stRecordingPayload.stStream.stInfo.zName);
    sstSoundSystem.stRecordingPayload.stStream.stInfo.zName = orxNULL;

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Updates status */
    orxFLAG_SET(sstSoundSystem.u32Flags, orxSOUNDSYSTEM_KU32_STATIC_FLAG_NONE, orxSOUNDSYSTEM_KU32_STATIC_FLAG_STOP_RECORDING);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

orxBOOL orxFASTCALL orxSoundSystem_MiniAudio_HasRecordingSupport()
{
  ma_context  stContext;
  orxBOOL     bResult = orxFALSE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  /* Inits context */
  if(ma_context_init(NULL, 0, NULL, &stContext) == MA_SUCCESS)
  {
    ma_device_info *apstCaptureDeviceList;
    ma_uint32       u32CaptureDeviceNumber;

    /* Gets available capture devices */
    if(ma_context_get_devices(&stContext, NULL, NULL, &apstCaptureDeviceList, &u32CaptureDeviceNumber) == MA_SUCCESS)
    {
      /* Updates result */
      bResult = (u32CaptureDeviceNumber != 0) ? orxTRUE : orxFALSE;
    }

    /* Uninits context */
    ma_context_uninit(&stContext);
  }

  /* Done! */
  return bResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_SetVolume(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fVolume)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Sets volume */
  ma_sound_set_volume(&(_pstSound->stSound), _fVolume);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_SetPitch(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fPitch)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Sets pitch */
  ma_sound_set_pitch(&(_pstSound->stSound), _fPitch);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_SetTime(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fTime)
{
  ma_uint64 u64Time;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Computes time in frames */
  u64Time = orxF2U(_fTime * orxU2F(ma_engine_get_sample_rate(ma_sound_get_engine(&(_pstSound->stSound)))));

  /* Sets it */
  if(ma_sound_seek_to_pcm_frame(&(_pstSound->stSound), u64Time) == MA_SUCCESS)
  {
    /* Updates engine node's local time */
    if(ma_node_set_time((ma_node *)&(_pstSound->stSound.engineNode), u64Time) == MA_SUCCESS)
    {
      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_SetPosition(orxSOUNDSYSTEM_SOUND *_pstSound, const orxVECTOR *_pvPosition)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);
  orxASSERT(_pvPosition != orxNULL);

  /* Updates position */
  ma_sound_set_position(&(_pstSound->stSound), sstSoundSystem.fDimensionRatio * _pvPosition->fX, sstSoundSystem.fDimensionRatio * _pvPosition->fY, sstSoundSystem.fDimensionRatio * _pvPosition->fZ);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_SetSpatialization(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fMinDistance, orxFLOAT _fMaxDistance, orxFLOAT _fMinGain, orxFLOAT _fMaxGain, orxFLOAT _fRollOff)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);
  orxASSERT(_fMaxDistance >= _fMinDistance);
  orxASSERT((_fMinGain >= orxFLOAT_0) && (_fMinGain <= orxFLOAT_1));
  orxASSERT((_fMaxGain >= orxFLOAT_0) && (_fMaxGain <= orxFLOAT_1));
  orxASSERT(_fRollOff >= orxFLOAT_0);

  /* Is spatialization enabled? */
  if(sstSoundSystem.u32ListenerNumber > 0)
  {
    /* Enable? */
    if((_fMinDistance >= orxFLOAT_0) && (_fMaxDistance >= orxFLOAT_0))
    {
      /* Updates status */
      ma_sound_set_spatialization_enabled(&(_pstSound->stSound), orxTRUE);

      /* Updates properties */
      ma_sound_set_min_distance(&(_pstSound->stSound), orxMAX(sstSoundSystem.fDimensionRatio * _fMinDistance, orxFLOAT_1));
      ma_sound_set_max_distance(&(_pstSound->stSound), orxMAX(sstSoundSystem.fDimensionRatio * _fMaxDistance, orxFLOAT_1));
      ma_sound_set_min_gain(&(_pstSound->stSound), _fMinGain);
      ma_sound_set_max_gain(&(_pstSound->stSound), _fMaxGain);
      ma_sound_set_rolloff(&(_pstSound->stSound), _fRollOff);
    }
    else
    {
      /* Updates status */
      ma_sound_set_spatialization_enabled(&(_pstSound->stSound), orxFALSE);
    }

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_SetPanning(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fPanning, orxBOOL _bMix)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Updates sound */
  ma_sound_set_pan(&(_pstSound->stSound), _fPanning);
  ma_sound_set_pan_mode(&(_pstSound->stSound), (_bMix != orxFALSE) ? ma_pan_mode_pan : ma_pan_mode_balance);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_Loop(orxSOUNDSYSTEM_SOUND *_pstSound, orxBOOL _bLoop)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Updates sound's status */
  ma_sound_set_looping(&(_pstSound->stSound), (_bLoop != orxFALSE) ? MA_TRUE : MA_FALSE);

  /* Done! */
  return eResult;
}

orxFLOAT orxFASTCALL orxSoundSystem_MiniAudio_GetVolume(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Updates result */
  fResult = ma_node_get_output_bus_volume((ma_node *)&(_pstSound->stSound), 0);

  /* Done! */
  return fResult;
}

orxFLOAT orxFASTCALL orxSoundSystem_MiniAudio_GetPitch(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Updates result */
  fResult = _pstSound->stSound.engineNode.pitch;

  /* Done! */
  return fResult;
}

orxFLOAT orxFASTCALL orxSoundSystem_MiniAudio_GetTime(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  ma_uint64 u64Time;
  orxFLOAT  fResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Gets time */
  u64Time = ma_sound_get_time_in_pcm_frames(&(_pstSound->stSound));

  /* Updates result */
  fResult = orxU2F(u64Time) / orxU2F(ma_engine_get_sample_rate(ma_sound_get_engine((ma_sound *)&(_pstSound->stSound))));

  /* Done! */
  return fResult;
}

orxVECTOR *orxFASTCALL orxSoundSystem_MiniAudio_GetPosition(const orxSOUNDSYSTEM_SOUND *_pstSound, orxVECTOR *_pvPosition)
{
  ma_vec3f    vPosition;
  orxVECTOR  *pvResult = _pvPosition;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);
  orxASSERT(_pvPosition != orxNULL);

  /* Gets position */
  vPosition = ma_sound_get_position(&(_pstSound->stSound));

  /* Updates result */
  orxVector_Set(pvResult, sstSoundSystem.fRecDimensionRatio * vPosition.x, sstSoundSystem.fRecDimensionRatio * vPosition.y, sstSoundSystem.fRecDimensionRatio * vPosition.z);

  /* Done! */
  return pvResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_GetSpatialization(const orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT *_pfMinDistance, orxFLOAT *_pfMaxDistance, orxFLOAT *_pfMinGain, orxFLOAT *_pfMaxGain, orxFLOAT *_pfRollOff)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);
  orxASSERT(_pfMinDistance != orxNULL);
  orxASSERT(_pfMaxDistance != orxNULL);
  orxASSERT(_pfMinGain != orxNULL);
  orxASSERT(_pfMaxGain != orxNULL);
  orxASSERT(_pfRollOff != orxNULL);

  /* Is spatialization enabled? */
  if(sstSoundSystem.u32ListenerNumber > 0)
  {
    /* Is sound spatialized? */
    if(_pstSound->stSound.engineNode.isSpatializationDisabled == MA_FALSE)
    {
      /* Updates values */
      *_pfMinDistance   = sstSoundSystem.fRecDimensionRatio * ma_sound_get_min_distance(&(_pstSound->stSound));
      *_pfMaxDistance   = sstSoundSystem.fRecDimensionRatio * ma_sound_get_max_distance(&(_pstSound->stSound));
      *_pfMinGain       = ma_sound_get_min_gain(&(_pstSound->stSound));
      *_pfMaxGain       = ma_sound_get_max_gain(&(_pstSound->stSound));
      *_pfRollOff       = ma_sound_get_rolloff(&(_pstSound->stSound));
    }
    else
    {
      /* Updates values */
      *_pfMinDistance   = *_pfMaxDistance = -orxFLOAT_1;
      *_pfMinGain       = orxFLOAT_0;
      *_pfMaxGain       = orxFLOAT_1;
      *_pfRollOff       = orxFLOAT_1;
    }

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Updates values */
    *_pfMinDistance   = *_pfMaxDistance = -orxFLOAT_1;
    *_pfMinGain       = orxFLOAT_0;
    *_pfMaxGain       = orxFLOAT_1;
    *_pfRollOff       = orxFLOAT_1;

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_GetPanning(const orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT *_pfPanning, orxBOOL *_pbMix)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);
  orxASSERT(_pfPanning != orxNULL);
  orxASSERT(_pbMix != orxNULL);

  /* Updates values */
  *_pfPanning = _pstSound->stSound.engineNode.panner.pan;
  *_pbMix     = (_pstSound->stSound.engineNode.panner.mode == ma_pan_mode_pan) ? orxTRUE : orxFALSE;

  /* Done! */
  return eResult;
}

orxBOOL orxFASTCALL orxSoundSystem_MiniAudio_IsLooping(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Updates result */
  bResult = (ma_sound_is_looping(&(_pstSound->stSound)) != MA_FALSE) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

orxFLOAT orxFASTCALL orxSoundSystem_MiniAudio_GetDuration(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  ma_uint64 u64Length;
  ma_result hResult;
  orxFLOAT  fResult = orxFLOAT_0;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Not an empty stream? */
  if((_pstSound->bIsStream == orxFALSE) || (_pstSound->stStream.stDataSource.flags != 0))
  {
    /* Gets length */
    hResult = ma_sound_get_length_in_pcm_frames((ma_sound *)&(_pstSound->stSound), &u64Length);

    /* Success? */
    if(hResult == MA_SUCCESS)
    {
      /* Updates result */
      fResult = orxU2F(u64Length) / orxU2F(ma_engine_get_sample_rate(ma_sound_get_engine((ma_sound *)&(_pstSound->stSound))));
    }
  }
  else
  {
    /* Updates result */
    fResult = -orxFLOAT_1;
  }

  /* Done! */
  return fResult;
}

orxSOUNDSYSTEM_STATUS orxFASTCALL orxSoundSystem_MiniAudio_GetStatus(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxSOUNDSYSTEM_STATUS eResult = orxSOUNDSYSTEM_STATUS_NONE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Playing? */
  if(ma_sound_is_playing(&(_pstSound->stSound)) != MA_FALSE)
  {
    /* Updates result */
    eResult = orxSOUNDSYSTEM_STATUS_PLAY;
  }
  /* Paused/Stopped */
  else
  {
    /* Updates result */
    eResult = ((_pstSound->bIsStopped != orxFALSE) || (ma_sound_get_time_in_pcm_frames(&(_pstSound->stSound)) == 0) || (ma_sound_at_end(&(_pstSound->stSound)) != MA_FALSE)) ? orxSOUNDSYSTEM_STATUS_STOP : orxSOUNDSYSTEM_STATUS_PAUSE;
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_SetGlobalVolume(orxFLOAT _fVolume)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  /* Is background muted? */
  if(orxFLAG_TEST(sstSoundSystem.u32Flags, orxSOUNDSYSTEM_KU32_STATIC_FLAG_BACKGROUND_MUTED))
  {
    /* Stores volume */
    sstSoundSystem.fForegroundVolume = _fVolume;

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Sets volume */
    eResult = (ma_engine_set_volume(&(sstSoundSystem.stEngine), _fVolume) == MA_SUCCESS) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

orxFLOAT orxFASTCALL orxSoundSystem_MiniAudio_GetGlobalVolume()
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  /* Updates result */
  fResult = ma_node_get_output_bus_volume(ma_engine_get_endpoint(&(sstSoundSystem.stEngine)), 0);

  /* Done! */
  return fResult;
}

orxU32 orxFASTCALL orxSoundSystem_MiniAudio_GetListenerCount()
{
  orxU32 u32Result;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  /* Updates result */
  u32Result = sstSoundSystem.u32ListenerNumber;

  /* Done! */
  return u32Result;
}

void orxFASTCALL orxSoundSystem_MiniAudio_EnableListener(orxU32 _u32ListenerIndex, orxBOOL _bEnable)
{
  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_u32ListenerIndex < sstSoundSystem.u32ListenerNumber);

  /* Updates listener */
  ma_engine_listener_set_enabled(&(sstSoundSystem.stEngine), _u32ListenerIndex, (_bEnable != orxFALSE) ? MA_TRUE : MA_FALSE);

  /* Done! */
  return;
}

orxBOOL orxFASTCALL orxSoundSystem_MiniAudio_IsListenerEnabled(orxU32 _u32ListenerIndex)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_u32ListenerIndex < sstSoundSystem.u32ListenerNumber);

  /* Updates listener */
  bResult = (ma_engine_listener_is_enabled(&(sstSoundSystem.stEngine), _u32ListenerIndex) != MA_FALSE) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_SetListenerPosition(orxU32 _u32Index, const orxVECTOR *_pvPosition)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvPosition != orxNULL);
  orxASSERT(_u32Index < sstSoundSystem.u32ListenerNumber);

  /* Valid? */
  if(_u32Index < sstSoundSystem.u32ListenerNumber)
  {
    /* Sets listener position */
    ma_engine_listener_set_position(&(sstSoundSystem.stEngine), _u32Index, sstSoundSystem.fDimensionRatio * _pvPosition->fX, sstSoundSystem.fDimensionRatio * _pvPosition->fY, sstSoundSystem.fDimensionRatio * _pvPosition->fZ);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

orxVECTOR *orxFASTCALL orxSoundSystem_MiniAudio_GetListenerPosition(orxU32 _u32Index, orxVECTOR *_pvPosition)
{
  ma_vec3f    vPosition;
  orxVECTOR  *pvResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvPosition != orxNULL);
  orxASSERT(_u32Index < sstSoundSystem.u32ListenerNumber);

  /* Valid? */
  if(_u32Index < sstSoundSystem.u32ListenerNumber)
  {
    /* Updates result */
    pvResult = _pvPosition;

    /* Gets listener position */
    vPosition = ma_engine_listener_get_position(&(sstSoundSystem.stEngine), _u32Index);

    /* Updates result */
    orxVector_Set(pvResult, sstSoundSystem.fRecDimensionRatio * vPosition.x, sstSoundSystem.fRecDimensionRatio * vPosition.y, sstSoundSystem.fRecDimensionRatio * vPosition.z);
  }
  else
  {
    /* Updates result */
    pvResult = orxNULL;
  }

  /* Done! */
  return pvResult;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(SOUNDSYSTEM);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_Init, SOUNDSYSTEM, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_Exit, SOUNDSYSTEM, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_CreateSample, SOUNDSYSTEM, CREATE_SAMPLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_LoadSample, SOUNDSYSTEM, LOAD_SAMPLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_DeleteSample, SOUNDSYSTEM, DELETE_SAMPLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_GetSampleInfo, SOUNDSYSTEM, GET_SAMPLE_INFO);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_SetSampleData, SOUNDSYSTEM, SET_SAMPLE_DATA);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_CreateFromSample, SOUNDSYSTEM, CREATE_FROM_SAMPLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_CreateStream, SOUNDSYSTEM, CREATE_STREAM);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_CreateStreamFromFile, SOUNDSYSTEM, CREATE_STREAM_FROM_FILE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_Delete, SOUNDSYSTEM, DELETE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_Play, SOUNDSYSTEM, PLAY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_Pause, SOUNDSYSTEM, PAUSE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_Stop, SOUNDSYSTEM, STOP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_AddFilter, SOUNDSYSTEM, ADD_FILTER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_RemoveLastFilter, SOUNDSYSTEM, REMOVE_LAST_FILTER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_RemoveAllFilters, SOUNDSYSTEM, REMOVE_ALL_FILTERS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_StartRecording, SOUNDSYSTEM, START_RECORDING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_StopRecording, SOUNDSYSTEM, STOP_RECORDING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_HasRecordingSupport, SOUNDSYSTEM, HAS_RECORDING_SUPPORT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_SetVolume, SOUNDSYSTEM, SET_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_SetPitch, SOUNDSYSTEM, SET_PITCH);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_SetTime, SOUNDSYSTEM, SET_TIME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_SetPosition, SOUNDSYSTEM, SET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_SetSpatialization, SOUNDSYSTEM, SET_SPATIALIZATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_SetPanning, SOUNDSYSTEM, SET_PANNING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_Loop, SOUNDSYSTEM, LOOP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_GetVolume, SOUNDSYSTEM, GET_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_GetPitch, SOUNDSYSTEM, GET_PITCH);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_GetTime, SOUNDSYSTEM, GET_TIME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_GetPosition, SOUNDSYSTEM, GET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_GetSpatialization, SOUNDSYSTEM, GET_SPATIALIZATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_GetPanning, SOUNDSYSTEM, GET_PANNING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_IsLooping, SOUNDSYSTEM, IS_LOOPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_GetDuration, SOUNDSYSTEM, GET_DURATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_GetStatus, SOUNDSYSTEM, GET_STATUS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_SetGlobalVolume, SOUNDSYSTEM, SET_GLOBAL_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_GetGlobalVolume, SOUNDSYSTEM, GET_GLOBAL_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_GetListenerCount, SOUNDSYSTEM, GET_LISTENER_COUNT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_EnableListener, SOUNDSYSTEM, ENABLE_LISTENER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_IsListenerEnabled, SOUNDSYSTEM, IS_LISTENER_ENABLED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_SetListenerPosition, SOUNDSYSTEM, SET_LISTENER_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_GetListenerPosition, SOUNDSYSTEM, GET_LISTENER_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_END();
