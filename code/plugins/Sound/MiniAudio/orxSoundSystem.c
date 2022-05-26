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
#undef MA_ON_THREAD_ENTRY
#undef MA_ON_THREAD_EXIT
#undef MA_NO_FLAC
#undef MA_NO_GENERATION
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
#define orxSOUNDSYSTEM_KU32_STATIC_FLAG_EXIT              0x00000010 /**< Exit flag */

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
#define orxSOUNDSYSTEM_KU32_TASK_PARAM_LIST_SIZE          512
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

  orxBOOL                         bBuffer   : 1;
  orxBOOL                         bReady    : 1;
};

/** Internal sound structure
 */
struct __orxSOUNDSYSTEM_SOUND_t
{
  ma_data_source_base             stBase;
  ma_sound                        stSound;
  orxHANDLE                       hUserData;
  const orxSTRING                 zName;
  orxHANDLE                       hOwner;
  ma_node_base                   *pstFilterNode;
  orxU32                          u32ChannelNumber;
  orxU32                          u32SampleRate;

  union
  {
    struct
    {
      ma_resource_manager_data_source stDataSource;
      orxFLOAT                   *afPendingSampleList;
      orxU32                      u32PendingSampleNumber;
      orxS32                      s32PacketID;

    } stStream;

    struct
    {
      ma_audio_buffer_ref         stDataSource;
      const orxSOUNDSYSTEM_SAMPLE *pstSample;

    } stSample;
  };

  orxBOOL                         bStream   : 1;
  orxBOOL                         bStopped  : 1;
  orxBOOL                         bStopping : 1;
  orxBOOL                         bReady    : 1;
};

/** Internal bus structure
 */
typedef struct __orxSOUNDSYSTEM_BUS_t
{
  ma_sound_group                  stGroup;
  orxSTRINGID                     stID;
  ma_node_base                   *pstFilterNode;

} orxSOUNDSYSTEM_BUS;

/** Internal custom node structure
 */
typedef struct __orxSOUNDSYSTEM_CUSTOM_NODE_t
{
  ma_node_base                    stNode;
  orxSOUND_FILTER_FUNCTION        pfnCallback;
  void                           *pContext;
  orxSTRINGID                     stNameID;
  orxU32                          u32ChannelNumber;
  orxU32                          u32SampleRate;

} orxSOUNDSYSTEM_CUSTOM_NODE;

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
    orxSOUNDSYSTEM_CUSTOM_NODE    stCustomNode;

  } stNode;

  orxSOUND_FILTER_DATA            stData;
  const orxSOUNDSYSTEM_SOUND     *pstSound;
  const orxSOUNDSYSTEM_BUS       *pstBus;
  orxBOOL                         bUseCustomParam : 1;
  orxBOOL                         bReady          : 1;

} orxSOUNDSYSTEM_FILTER;

/** Internal task parameter structure
 */
typedef struct __orxSOUNDSYSTEM_TASK_PARAM_t
{
  union
  {
    orxVECTOR               vPosition;
    struct {
      orxFLOAT              fPanning;
      orxBOOL               bMix;
    } stPanning;
    struct {
      orxFLOAT              fMinDistance;
      orxFLOAT              fMaxDistance;
      orxFLOAT              fMinGain;
      orxFLOAT              fMaxGain;
      orxFLOAT              fRollOff;
    } stSpatialization;
    const orxSTRING         zLocation;
    orxSOUNDSYSTEM_BUS *    pstBus;
    orxFLOAT                fVolume;
    orxFLOAT                fPitch;
    orxFLOAT                fTime;
    orxSOUNDSYSTEM_FILTER  *pstFilter;
    orxBOOL                 bLoop;
  };

  union
  {
    orxHANDLE               hStructure;
    orxSOUNDSYSTEM_SOUND   *pstSound;
    orxSOUNDSYSTEM_SAMPLE  *pstSample;
  };

} orxSOUNDSYSTEM_TASK_PARAM;

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
  ma_node_vtable                  stCustomNodeVTable;     /**< Custom node VTable */
  orxBANK                        *pstSampleBank;          /**< Sample bank */
  orxBANK                        *pstSoundBank;           /**< Sound bank */
  orxBANK                        *pstBusBank;             /**< Bus bank */
  orxBANK                        *pstFilterBank;          /**< Filter bank */
  volatile orxHANDLE              hRecordingResource;     /**< Recording resource */
  orxSOUNDSYSTEM_TASK_PARAM       astTaskParamList[orxSOUNDSYSTEM_KU32_TASK_PARAM_LIST_SIZE]; /**< Task parameters */
  orxFLOAT                        fDimensionRatio;        /**< Dimension ration */
  orxFLOAT                        fRecDimensionRatio;     /**< Reciprocal dimension ratio */
  orxFLOAT                        fForegroundVolume;      /**< Foreground volume */
  orxU32                          u32TaskParamIndex;      /**< Task param index */
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

static void orxSoundSystem_MiniAudio_ProcessCustomNode(ma_node *_pstNode, const float **_aafFramesIn, ma_uint32 *_pu32FrameCountIn, float **_aafFramesOut, ma_uint32 *_pu32FrameCountOut)
{
  orxSOUNDSYSTEM_CUSTOM_NODE *pstNode;

  /* Gets custom node */
  pstNode = (orxSOUNDSYSTEM_CUSTOM_NODE *)_pstNode;

  /* Calls callback */
  pstNode->pfnCallback(_aafFramesOut[0], _aafFramesIn[0], *_pu32FrameCountOut * pstNode->u32ChannelNumber, pstNode->u32ChannelNumber, pstNode->u32SampleRate, pstNode->stNameID, pstNode->pContext);

  /* Done! */
  return;

  //ma_delay_process_pcm_frames(&pDelayNode->delay, ppFramesOut[0], ppFramesIn[0], *pFrameCountOut);
}

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

static void orxFASTCALL orxSoundSystem_MiniAudio_Update(const orxCLOCK_INFO *_pstClockInfo, void *_pContext)
{
  orxEVENT                stEvent;                                                 \
  orxSOUND_EVENT_PAYLOAD  stPayload;
  orxSOUNDSYSTEM_FILTER  *pstFilter;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxSoundSystem_Update");

  /* Inits event */
  orxEVENT_INIT(stEvent, orxEVENT_TYPE_SOUND, orxSOUND_EVENT_SET_FILTER_PARAM, orxNULL, orxNULL, &stPayload);

  /* Clears payload */
  orxMemory_Zero(&stPayload, sizeof(orxSOUND_EVENT_PAYLOAD));

  /* For all filters */
  for(pstFilter = (orxSOUNDSYSTEM_FILTER *)orxBank_GetNext(sstSoundSystem.pstFilterBank, orxNULL);
      pstFilter != orxNULL;
      pstFilter = (orxSOUNDSYSTEM_FILTER *)orxBank_GetNext(sstSoundSystem.pstFilterBank, pstFilter))
  {
    /* Uses custom param and is ready? */
    if((pstFilter->bUseCustomParam != orxFALSE) && (pstFilter->bReady != orxFALSE))
    {
      const ma_engine_node *pstEngineNode;
      const orxSTRING       zName;
      orxSTATUS             eResult;

      /* Has sound? */
      if(pstFilter->pstSound != orxNULL)
      {
        /* Stores sound */
        stPayload.pstSound = orxSOUND(pstFilter->pstSound->hUserData);

        /* Clears bus ID */
        stPayload.stFilter.stBusID = orxSTRINGID_UNDEFINED;

        /* Doesn't have its owner yet? */
        if(pstFilter->pstSound->hOwner == orxNULL)
        {
          orxSTRUCTURE *pOwner;

          /* Stores it */
          if((pOwner = orxStructure_GetOwner(pstFilter->pstSound->hUserData)) != orxNULL)
          {
            ((orxSOUNDSYSTEM_SOUND *)pstFilter->pstSound)->hOwner = orxStructure_GetOwner(pOwner);
          }
        }

        /* Updates event */
        stEvent.hSender = pstFilter->pstSound->hOwner;

        /* Gets engine node */
        pstEngineNode = &(pstFilter->pstSound->stSound.engineNode);

        /* Gets name */
        zName = pstFilter->pstSound->zName;
      }
      /* Has bus */
      else
      {
        /* Stores bus ID */
        stPayload.stFilter.stBusID = pstFilter->pstBus->stID;

        /* Clears sound */
        stPayload.pstSound = orxNULL;

        /* Gets engine node */
        pstEngineNode = &(pstFilter->pstBus->stGroup.engineNode);

        /* Gets name */
        zName = orxString_GetFromID(pstFilter->pstBus->stID);
      }

      /* Updates payload filter data */
      orxMemory_Copy(&(stPayload.stFilter.stData), &(pstFilter->stData), sizeof(orxSOUND_FILTER_DATA));

      /* Sends event */
      eResult = orxEvent_Send(&stEvent);

      /* Success? */
      if(eResult != orxSTATUS_FAILURE)
      {
        /* Changed? */
        if(orxMemory_Compare(&(pstFilter->stData), &(stPayload.stFilter.stData), sizeof(orxSOUND_FILTER_DATA)) != 0)
        {
          ma_result hResult = MA_ERROR;

          /* Depending on its type */
          switch(pstFilter->stData.eType)
          {
            /* Biquad */
            case orxSOUND_FILTER_TYPE_BIQUAD:
            {
              ma_biquad_config stConfig;

              /* Inits its config */
              stConfig = ma_biquad_config_init(ma_format_f32, ma_engine_get_channels(&(sstSoundSystem.stEngine)), stPayload.stFilter.stData.stBiquad.fB0, stPayload.stFilter.stData.stBiquad.fB1, stPayload.stFilter.stData.stBiquad.fB2, stPayload.stFilter.stData.stBiquad.fA0, stPayload.stFilter.stData.stBiquad.fA1, stPayload.stFilter.stData.stBiquad.fA2);

              /* Reinits it */
              hResult = ma_biquad_node_reinit(&stConfig, &(pstFilter->stNode.stBiquadNode));

              break;
            }

            /* Low Pass */
            case orxSOUND_FILTER_TYPE_LOW_PASS:
            {
              /* Has order changed? */
              if(pstFilter->stData.stLowPass.u32Order != stPayload.stFilter.stData.stLowPass.u32Order)
              {
                /* Logs message */
                orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "[%s] Can't update parameter \"order\" for filter <%s> at runtime (%u -> %u), aborting.", zName, orxString_GetFromID(pstFilter->stData.stNameID), pstFilter->stData.stLowPass.u32Order, stPayload.stFilter.stData.stLowPass.u32Order);
              }
              else
              {
                ma_lpf_config stConfig;

                /* Inits its config */
                stConfig = ma_lpf_config_init(ma_format_f32, ma_engine_get_channels(&(sstSoundSystem.stEngine)), pstEngineNode->sampleRate, stPayload.stFilter.stData.stLowPass.fFrequency, stPayload.stFilter.stData.stLowPass.u32Order);

                /* Reinits it */
                hResult = ma_lpf_node_reinit(&stConfig, &(pstFilter->stNode.stLowPassNode));
              }

              break;
            }

            /* High Pass */
            case orxSOUND_FILTER_TYPE_HIGH_PASS:
            {
              /* Has order changed? */
              if(pstFilter->stData.stHighPass.u32Order != stPayload.stFilter.stData.stHighPass.u32Order)
              {
                /* Logs message */
                orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "[%s] Can't update parameter \"order\" for filter <%s> at runtime (%u -> %u), aborting.", zName, orxString_GetFromID(pstFilter->stData.stNameID), pstFilter->stData.stHighPass.u32Order, stPayload.stFilter.stData.stHighPass.u32Order);
              }
              else
              {
                ma_hpf_config stConfig;

                /* Inits its config */
                stConfig = ma_hpf_config_init(ma_format_f32, ma_engine_get_channels(&(sstSoundSystem.stEngine)), pstEngineNode->sampleRate, stPayload.stFilter.stData.stHighPass.fFrequency, stPayload.stFilter.stData.stHighPass.u32Order);

                /* Reinits it */
                hResult = ma_hpf_node_reinit(&stConfig, &(pstFilter->stNode.stHighPassNode));
              }

              break;
            }

            /* Band Pass */
            case orxSOUND_FILTER_TYPE_BAND_PASS:
            {
              /* Has order changed? */
              if(pstFilter->stData.stBandPass.u32Order != stPayload.stFilter.stData.stBandPass.u32Order)
              {
                /* Logs message */
                orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "[%s] Can't update parameter \"order\" for filter <%s> at runtime (%u -> %u), aborting.", zName, orxString_GetFromID(pstFilter->stData.stNameID), pstFilter->stData.stBandPass.u32Order, stPayload.stFilter.stData.stBandPass.u32Order);
              }
              else
              {
                ma_bpf_config stConfig;

                /* Inits its config */
                stConfig = ma_bpf_config_init(ma_format_f32, ma_engine_get_channels(&(sstSoundSystem.stEngine)), pstEngineNode->sampleRate, stPayload.stFilter.stData.stBandPass.fFrequency, stPayload.stFilter.stData.stBandPass.u32Order);

                /* Reinits it */
                hResult = ma_bpf_node_reinit(&stConfig, &(pstFilter->stNode.stBandPassNode));
              }

              break;
            }

            /* Low Shelf */
            case orxSOUND_FILTER_TYPE_LOW_SHELF:
            {
              ma_loshelf_config stConfig;

              /* Inits its config */
              stConfig = ma_loshelf2_config_init(ma_format_f32, ma_engine_get_channels(&(sstSoundSystem.stEngine)), pstEngineNode->sampleRate, stPayload.stFilter.stData.stLowShelf.fGain, stPayload.stFilter.stData.stLowShelf.fQ, stPayload.stFilter.stData.stLowShelf.fFrequency);

              /* Reinits it */
              hResult = ma_loshelf_node_reinit(&stConfig, &(pstFilter->stNode.stLowShelfNode));

              break;
            }

            /* High Shelf */
            case orxSOUND_FILTER_TYPE_HIGH_SHELF:
            {
              ma_hishelf_config stConfig;

              /* Inits its config */
              stConfig = ma_hishelf2_config_init(ma_format_f32, ma_engine_get_channels(&(sstSoundSystem.stEngine)), pstEngineNode->sampleRate, stPayload.stFilter.stData.stHighShelf.fGain, stPayload.stFilter.stData.stHighShelf.fQ, stPayload.stFilter.stData.stHighShelf.fFrequency);

              /* Reinits it */
              hResult = ma_hishelf_node_reinit(&stConfig, &(pstFilter->stNode.stHighShelfNode));

              break;
            }

            /* Notch */
            case orxSOUND_FILTER_TYPE_NOTCH:
            {
              ma_notch_config stConfig;

              /* Inits its config */
              stConfig = ma_notch2_config_init(ma_format_f32, ma_engine_get_channels(&(sstSoundSystem.stEngine)), pstEngineNode->sampleRate, stPayload.stFilter.stData.stNotch.fQ, stPayload.stFilter.stData.stNotch.fFrequency);

              /* Reinits it */
              hResult = ma_notch_node_reinit(&stConfig, &(pstFilter->stNode.stNotchNode));

              break;
            }

            /* Peaking */
            case orxSOUND_FILTER_TYPE_PEAKING:
            {
              ma_peak_config stConfig;

              /* Inits its config */
              stConfig = ma_peak2_config_init(ma_format_f32, ma_engine_get_channels(&(sstSoundSystem.stEngine)), pstEngineNode->sampleRate, stPayload.stFilter.stData.stPeaking.fGain, stPayload.stFilter.stData.stPeaking.fQ, stPayload.stFilter.stData.stPeaking.fFrequency);

              /* Reinits it */
              hResult = ma_peak_node_reinit(&stConfig, &(pstFilter->stNode.stPeakingNode));

              break;
            }

            /* Delay */
            case orxSOUND_FILTER_TYPE_DELAY:
            {
              /* Has delay changed? */
              if(pstFilter->stData.stDelay.fDelay != stPayload.stFilter.stData.stDelay.fDelay)
              {
                /* Logs message */
                orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "[%s] Can't update parameter \"delay\" for filter <%s> at runtime (%g -> %g), aborting.", zName, orxString_GetFromID(pstFilter->stData.stNameID), pstFilter->stData.stDelay.fDelay, stPayload.stFilter.stData.stDelay.fDelay);
              }

              /* Updates its decay */
              ma_delay_node_set_decay(&(pstFilter->stNode.stDelayNode), stPayload.stFilter.stData.stDelay.fDecay);

              /* Updates result */
              hResult = MA_SUCCESS;

              break;
            }

            /* Custom */
            case orxSOUND_FILTER_TYPE_CUSTOM:
            {
              /* Nothing to do */
              hResult = MA_SUCCESS;

              break;
            }

            default:
            {
              break;
            }
          }

          /* Success? */
          if(hResult == MA_SUCCESS)
          {
            /* Updates new filter data */
            orxMemory_Copy(&(pstFilter->stData), &(stPayload.stFilter.stData), sizeof(orxSOUND_FILTER_DATA));
          }
        }
      }
    }
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return;
}

static ma_result orxSoundSystem_MiniAudio_Recording_Write(ma_encoder *_pstEncoder, const void *_pBufferIn, size_t _sBytesToWrite, size_t *_psBytesWritten)
{
  /* Writes data */
  *_psBytesWritten = (size_t)orxResource_Write((orxHANDLE)_pstEncoder->pUserData, _sBytesToWrite, _pBufferIn, orxNULL, orxNULL);

  /* Done! */
  return (*_psBytesWritten == _sBytesToWrite) ? MA_SUCCESS : MA_ERROR;
}

static ma_result orxSoundSystem_MiniAudio_Recording_Seek(ma_encoder *_pstEncoder, ma_int64 _s64ByteOffset, ma_seek_origin _eOrigin)
{
  /* Seeks */
  return (orxResource_Seek((orxHANDLE)_pstEncoder->pUserData, (orxS64)_s64ByteOffset, (orxSEEK_OFFSET_WHENCE)_eOrigin) >= 0) ? MA_SUCCESS : MA_ERROR;
}

static orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_OpenRecordingFile()
{
  const orxSTRING zResourceLocation;
  orxHANDLE       hResource;
  orxSTATUS       eResult;

  /* Valid file to open? */
  if(((zResourceLocation = orxResource_LocateInStorage(orxSOUND_KZ_RESOURCE_GROUP, orxRESOURCE_KZ_DEFAULT_STORAGE, sstSoundSystem.stRecordingPayload.stStream.stInfo.zName)) != orxNULL)
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

static ma_result orxSoundSystem_MiniAudio_Stream_Read(ma_data_source *_pstDataSource, void *_pFramesOut, ma_uint64 _u64FrameCount, ma_uint64 *_pu64FramesRead)
{
  orxSOUNDSYSTEM_SOUND *pstSound;
  orxU32                u32CopySampleNumber = 0;
  ma_result             hResult = MA_SUCCESS;

  /* Retrieves associated sound */
  pstSound = (orxSOUNDSYSTEM_SOUND *)_pstDataSource;

  /* Checks */
  orxASSERT(pstSound->bStream != orxFALSE);

  /* Inits frames count */
  *_pu64FramesRead = 0;

  /* Not stopping? */
  if(pstSound->bStopping == orxFALSE)
  {
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
        if(pstSound->hOwner == orxNULL)
        {
          orxSTRUCTURE *pOwner;

          /* Stores it */
          if((pOwner = orxStructure_GetOwner(pstSound->hUserData)) != orxNULL)
          {
            pstSound->hOwner = orxStructure_GetOwner(pOwner);
          }
        }

        /* Sends event */
        orxEVENT_SEND(orxEVENT_TYPE_SOUND, orxSOUND_EVENT_PACKET, pstSound->hOwner, orxNULL, &stPayload);

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
        /* Not last packet? */
        else if(stPayload.stStream.stPacket.bLast == orxFALSE)
        {
          /* Silences a single frame */
          ma_silence_pcm_frames(_pFramesOut, 1, orxSOUNDSYSTEM_KE_DEFAULT_FORMAT, pstSound->u32ChannelNumber);

          /* Updates status */
          *_pu64FramesRead = 1;
        }
        else
        {
          /* Updates status */
          *_pu64FramesRead = 0;
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
            /* Has pending samples? */
            if((*_pu64FramesRead != 0) || (u32CopySampleNumber != 0))
            {
              /* Updates status */
              pstSound->bStopping = orxTRUE;
            }
            else
            {
              /* Updates result */
              hResult = MA_AT_END;
            }
          }
        }
      }
    }

    /* Adjusts available samples number (Since 0.11.3+, miniaudio doesn't accept data when MA_AT_END is reached) */
    *_pu64FramesRead += u32CopySampleNumber / pstSound->u32ChannelNumber;
  }
  else
  {
    /* Updates result */
    hResult = MA_AT_END;
  }

  /* Done! */
  return hResult;
}

static ma_result orxSoundSystem_MiniAudio_Stream_Seek(ma_data_source *_pstDataSource, ma_uint64 _u64FrameIndex)
{
  orxSOUNDSYSTEM_SOUND *pstSound;
  ma_result             hResult;

  /* Retrieves associated sound */
  pstSound = (orxSOUNDSYSTEM_SOUND *)_pstDataSource;

  /* Checks */
  orxASSERT(pstSound->bStream != orxFALSE);

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

static ma_result orxSoundSystem_MiniAudio_Stream_GetDataFormat(ma_data_source *_pstDataSource, ma_format *_peFormat, ma_uint32 *_pu32Channels, ma_uint32* _pu32SampleRate, ma_channel *_peChannelMap, size_t _sChannelMapCap)
{
  orxSOUNDSYSTEM_SOUND *pstSound;
  ma_result             hResult = MA_SUCCESS;

  /* Retrieves associated sound */
  pstSound = (orxSOUNDSYSTEM_SOUND *)_pstDataSource;

  /* Checks */
  orxASSERT(pstSound->bStream != orxFALSE);

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

static ma_result orxSoundSystem_MiniAudio_Stream_GetCursor(ma_data_source *_pstDataSource, ma_uint64 *_pu64Cursor)
{
  orxSOUNDSYSTEM_SOUND *pstSound;
  ma_result             hResult;

  /* Retrieves associated sound */
  pstSound = (orxSOUNDSYSTEM_SOUND *)_pstDataSource;

  /* Checks */
  orxASSERT(pstSound->bStream != orxFALSE);

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

static ma_result orxSoundSystem_MiniAudio_Stream_GetLength(ma_data_source *_pstDataSource, ma_uint64 *_pu64Length)
{
  orxSOUNDSYSTEM_SOUND *pstSound;
  ma_result             hResult;

  /* Retrieves associated sound */
  pstSound = (orxSOUNDSYSTEM_SOUND *)_pstDataSource;

  /* Checks */
  orxASSERT(pstSound->bStream != orxFALSE);

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

static ma_result orxSoundSystem_MiniAudio_Stream_SetLooping(ma_data_source* _pstDataSource, ma_bool32 _bIsLooping)
{
  orxSOUNDSYSTEM_SOUND *pstSound;
  ma_result             hResult;

  /* Retrieves associated sound */
  pstSound = (orxSOUNDSYSTEM_SOUND *)_pstDataSource;

  /* Checks */
  orxASSERT(pstSound->bStream != orxFALSE);

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
static ma_result orxSoundSystem_MiniAudio_InitVorbis(ma_read_proc _pfnRead, ma_seek_proc _pfnSeek, ma_tell_proc _pfnTell, void *_pReadSeekTellUserData, const ma_decoding_backend_config *_pstConfig, const ma_allocation_callbacks *_pstAllocationCallbacks, ma_stbvorbis *_pstVorbis)
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
static ma_result orxSoundSystem_MiniAudio_InitVorbisBackend(void *_pUserData, ma_read_proc _pfnRead, ma_seek_proc _pfnSeek, ma_tell_proc _pfnTell, void *_pReadSeekTellUserData, const ma_decoding_backend_config *_pstConfig, const ma_allocation_callbacks *_pstAllocationCallbacks, ma_data_source **_ppstBackend)
{
  ma_result     hResult;
  ma_stbvorbis *pstVorbis;

  /* Allocates the decoder backend */
  pstVorbis = (ma_stbvorbis *)ma_malloc(sizeof(ma_stbvorbis), _pstAllocationCallbacks);

  /* Success? */
  if(pstVorbis != NULL)
  {
    /* Inits decoder backend */
    hResult = orxSoundSystem_MiniAudio_InitVorbis(_pfnRead, _pfnSeek, _pfnTell, _pReadSeekTellUserData, _pstConfig, _pstAllocationCallbacks, pstVorbis);

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

static void orxSoundSystem_MiniAudio_UninitVorbisBackend(void *_pUserData, ma_data_source *_pstBackend, const ma_allocation_callbacks *_pstAllocationCallbacks)
{
  /* Uninits decoder backend */
  ma_stbvorbis_uninit((ma_stbvorbis *)_pstBackend, _pstAllocationCallbacks);

  /* Frees it */
  ma_free(_pstBackend, _pstAllocationCallbacks);

  /* Done! */
  return;
}

static orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_ProcessJobs(void *_pContext)
{
  /* Not exiting? */
  if(!orxFLAG_TEST(sstSoundSystem.u32Flags, orxSOUNDSYSTEM_KU32_STATIC_FLAG_EXIT))
  {
    ma_result hResult;

    /* Profiles */
    orxPROFILER_PUSH_MARKER("orxSoundSystem_ProcessTask");

    /* Processes next job */
    hResult = ma_resource_manager_process_next_job((ma_resource_manager *)_pContext);

    /* Profiles */
    orxPROFILER_POP_MARKER();

    /* No job left? */
    if(hResult == MA_NO_DATA_AVAILABLE)
    {
     /* Sleeps */
     orxSystem_Delay(orxSOUNDSYSTEM_KF_DEFAULT_THREAD_SLEEP_TIME);
    }
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

static ma_result orxSoundSystem_MiniAudio_Open(ma_vfs *_pstVFS, const char *_zFilename, ma_uint32 _u32OpenMode, ma_vfs_file *_pstFile)
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

static ma_result orxSoundSystem_MiniAudio_Close(ma_vfs *_pstVFS, ma_vfs_file _stFile)
{
  /* Closes resource */
  orxResource_Close((orxHANDLE)_stFile);

  /* Done! */
  return MA_SUCCESS;
}

static ma_result orxSoundSystem_MiniAudio_Read(ma_vfs *_pstVFS, ma_vfs_file _stFile, void *_pDst, size_t _sSizeInBytes, size_t *_psBytesRead)
{
  /* Reads data */
  *_psBytesRead = (size_t)orxResource_Read((orxHANDLE)_stFile, (orxS64)_sSizeInBytes, _pDst, orxNULL, orxNULL);

  /* Done! */
  return (*_psBytesRead == 0) ? MA_AT_END : MA_SUCCESS;
}

static ma_result orxSoundSystem_MiniAudio_Write(ma_vfs *_pstVFS, ma_vfs_file _stFile, const void *_pSrc, size_t _sSizeInBytes, size_t *_psBytesWritten)
{
  /* Writes data */
  *_psBytesWritten = (size_t)orxResource_Write((orxHANDLE)_stFile, _sSizeInBytes, _pSrc, orxNULL, orxNULL);

  /* Done! */
  return (*_psBytesWritten != _sSizeInBytes) ? MA_ACCESS_DENIED : MA_SUCCESS;
}

static ma_result orxSoundSystem_MiniAudio_Seek(ma_vfs *_pstVFS, ma_vfs_file _stFile, ma_int64 _s64Offset, ma_seek_origin _eOrigin)
{
  ma_result hResult;

  /* Seeks */
  hResult = (orxResource_Seek((orxHANDLE)_stFile, _s64Offset, (orxSEEK_OFFSET_WHENCE)_eOrigin) >= 0) ? MA_SUCCESS : MA_ERROR;

  /* Done! */
  return hResult;
}

static ma_result orxSoundSystem_MiniAudio_Tell(ma_vfs *_pstVFS, ma_vfs_file _stFile, ma_int64 *_ps64Cursor)
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

static ma_result orxSoundSystem_MiniAudio_Info(ma_vfs *_pstVFS, ma_vfs_file _stFile, ma_file_info *_pstInfo)
{
  /* Gets size */
  _pstInfo->sizeInBytes = (ma_uint64)orxResource_GetSize((orxHANDLE)_stFile);

  /* Done! */
  return MA_SUCCESS;
}

static orxINLINE orxSOUNDSYSTEM_TASK_PARAM *orxSoundSystem_MiniAudio_GetTaskParam(orxHANDLE _hStructure)
{
  volatile orxSOUNDSYSTEM_TASK_PARAM *pstResult;

  /* Gets current param */
  pstResult = &(sstSoundSystem.astTaskParamList[sstSoundSystem.u32TaskParamIndex]);

  /* Waits for it to be free */
  while(pstResult->hStructure != orxNULL)
    ;

  /* Stores structure */
  pstResult->hStructure = _hStructure;

  /* Updates index */
  sstSoundSystem.u32TaskParamIndex = (sstSoundSystem.u32TaskParamIndex + 1) & (orxSOUNDSYSTEM_KU32_TASK_PARAM_LIST_SIZE - 1);

  /* Done! */
  return (orxSOUNDSYSTEM_TASK_PARAM *)pstResult;
}

static orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_FreeSound(void *_pContext)
{
  orxSOUNDSYSTEM_SOUND *pstSound;
  orxSTATUS             eResult = orxSTATUS_SUCCESS;

  /* Gets sound */
  pstSound = (orxSOUNDSYSTEM_SOUND *)_pContext;

  /* Deletes it */
  orxBank_Free(sstSoundSystem.pstSoundBank, pstSound);

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_FreeSample(void *_pContext)
{
  orxSOUNDSYSTEM_SAMPLE  *pstSample;
  orxSTATUS               eResult = orxSTATUS_SUCCESS;

  /* Gets sample */
  pstSample = (orxSOUNDSYSTEM_SAMPLE *)_pContext;

  /* Is a buffer? */
  if(pstSample->bBuffer != orxFALSE)
  {
    /* Frees it */
    orxMemory_Free(pstSample->stBuffer.afSampleList);
  }
  else
  {
    /* Deletes location */
    orxString_Delete(pstSample->stResource.zLocation);
  }

  /* Deletes it */
  orxBank_Free(sstSoundSystem.pstSampleBank, pstSample);

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_FreeFilter(void *_pContext)
{
  orxSOUNDSYSTEM_FILTER  *pstFilter;
  orxSTATUS               eResult = orxSTATUS_SUCCESS;

  /* Gets filter */
  pstFilter = (orxSOUNDSYSTEM_FILTER *)_pContext;

  /* Deletes it */
  orxBank_Free(sstSoundSystem.pstFilterBank, pstFilter);

  /* Done! */
  return eResult;
}

static ma_result orxSoundSystem_MiniAudio_InitFilter(orxSOUNDSYSTEM_FILTER *_pstFilter, const orxSOUND_FILTER_DATA *_pstFilterData, const ma_engine_node *_pstEngineNode)
{
  ma_result hResult = MA_ERROR;

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
      hResult = ma_biquad_node_init(ma_engine_get_node_graph(&(sstSoundSystem.stEngine)), &stConfig, &(sstSoundSystem.stResourceManagerConfig.allocationCallbacks), &(_pstFilter->stNode.stBiquadNode));

      break;
    }

    /* Low Pass */
    case orxSOUND_FILTER_TYPE_LOW_PASS:
    {
      ma_lpf_node_config stConfig;

      /* Inits its config */
      stConfig = ma_lpf_node_config_init(ma_engine_get_channels(&(sstSoundSystem.stEngine)), _pstEngineNode->sampleRate, _pstFilterData->stLowPass.fFrequency, _pstFilterData->stLowPass.u32Order);

      /* Inits it */
      hResult = ma_lpf_node_init(ma_engine_get_node_graph(&(sstSoundSystem.stEngine)), &stConfig, &(sstSoundSystem.stResourceManagerConfig.allocationCallbacks), &(_pstFilter->stNode.stLowPassNode));

      break;
    }

    /* High Pass */
    case orxSOUND_FILTER_TYPE_HIGH_PASS:
    {
      ma_hpf_node_config stConfig;

      /* Inits its config */
      stConfig = ma_hpf_node_config_init(ma_engine_get_channels(&(sstSoundSystem.stEngine)), _pstEngineNode->sampleRate, _pstFilterData->stHighPass.fFrequency, _pstFilterData->stHighPass.u32Order);

      /* Inits it */
      hResult = ma_hpf_node_init(ma_engine_get_node_graph(&(sstSoundSystem.stEngine)), &stConfig, &(sstSoundSystem.stResourceManagerConfig.allocationCallbacks), &(_pstFilter->stNode.stHighPassNode));

      break;
    }

    /* Band Pass */
    case orxSOUND_FILTER_TYPE_BAND_PASS:
    {
      ma_bpf_node_config stConfig;

      /* Clamps order */
      ((orxSOUND_FILTER_DATA *)_pstFilterData)->stBandPass.u32Order = orxMAX(_pstFilterData->stBandPass.u32Order, 2);

      /* Inits its config */
      stConfig = ma_bpf_node_config_init(ma_engine_get_channels(&(sstSoundSystem.stEngine)), _pstEngineNode->sampleRate, _pstFilterData->stBandPass.fFrequency, _pstFilterData->stBandPass.u32Order);

      /* Inits it */
      hResult = ma_bpf_node_init(ma_engine_get_node_graph(&(sstSoundSystem.stEngine)), &stConfig, &(sstSoundSystem.stResourceManagerConfig.allocationCallbacks), &(_pstFilter->stNode.stBandPassNode));

      break;
    }

    /* Low Shelf */
    case orxSOUND_FILTER_TYPE_LOW_SHELF:
    {
      ma_loshelf_node_config stConfig;

      /* Inits its config */
      stConfig = ma_loshelf_node_config_init(ma_engine_get_channels(&(sstSoundSystem.stEngine)), _pstEngineNode->sampleRate, _pstFilterData->stLowShelf.fGain, _pstFilterData->stLowShelf.fQ, _pstFilterData->stLowShelf.fFrequency);

      /* Inits it */
      hResult = ma_loshelf_node_init(ma_engine_get_node_graph(&(sstSoundSystem.stEngine)), &stConfig, &(sstSoundSystem.stResourceManagerConfig.allocationCallbacks), &(_pstFilter->stNode.stLowShelfNode));

      break;
    }

    /* High Shelf */
    case orxSOUND_FILTER_TYPE_HIGH_SHELF:
    {
      ma_hishelf_node_config stConfig;

      /* Inits its config */
      stConfig = ma_hishelf_node_config_init(ma_engine_get_channels(&(sstSoundSystem.stEngine)), _pstEngineNode->sampleRate, _pstFilterData->stHighShelf.fGain, _pstFilterData->stHighShelf.fQ, _pstFilterData->stHighShelf.fFrequency);

      /* Inits it */
      hResult = ma_hishelf_node_init(ma_engine_get_node_graph(&(sstSoundSystem.stEngine)), &stConfig, &(sstSoundSystem.stResourceManagerConfig.allocationCallbacks), &(_pstFilter->stNode.stHighShelfNode));

      break;
    }

    /* Notch */
    case orxSOUND_FILTER_TYPE_NOTCH:
    {
      ma_notch_node_config stConfig;

      /* Inits its config */
      stConfig = ma_notch_node_config_init(ma_engine_get_channels(&(sstSoundSystem.stEngine)), _pstEngineNode->sampleRate, _pstFilterData->stNotch.fQ, _pstFilterData->stNotch.fFrequency);

      /* Inits it */
      hResult = ma_notch_node_init(ma_engine_get_node_graph(&(sstSoundSystem.stEngine)), &stConfig, &(sstSoundSystem.stResourceManagerConfig.allocationCallbacks), &(_pstFilter->stNode.stNotchNode));

      break;
    }

    /* Peaking */
    case orxSOUND_FILTER_TYPE_PEAKING:
    {
      ma_peak_node_config stConfig;

      /* Inits its config */
      stConfig = ma_peak_node_config_init(ma_engine_get_channels(&(sstSoundSystem.stEngine)), _pstEngineNode->sampleRate, _pstFilterData->stPeaking.fGain, _pstFilterData->stPeaking.fQ, _pstFilterData->stPeaking.fFrequency);

      /* Inits it */
      hResult = ma_peak_node_init(ma_engine_get_node_graph(&(sstSoundSystem.stEngine)), &stConfig, &(sstSoundSystem.stResourceManagerConfig.allocationCallbacks), &(_pstFilter->stNode.stPeakingNode));

      break;
    }

    /* Delay */
    case orxSOUND_FILTER_TYPE_DELAY:
    {
      /* Valid delay? */
      if(_pstFilterData->stDelay.fDelay > orxFLOAT_0)
      {
        ma_delay_node_config stConfig;

        /* Inits its config */
        stConfig = ma_delay_node_config_init(ma_engine_get_channels(&(sstSoundSystem.stEngine)), _pstEngineNode->sampleRate, orxF2U(orxU2F(_pstEngineNode->sampleRate) * _pstFilterData->stDelay.fDelay), _pstFilterData->stDelay.fDecay);

        /* Inits it */
        hResult = ma_delay_node_init(ma_engine_get_node_graph(&(sstSoundSystem.stEngine)), &stConfig, &(sstSoundSystem.stResourceManagerConfig.allocationCallbacks), &(_pstFilter->stNode.stDelayNode));
      }

      break;
    }

    /* Custom */
    case orxSOUND_FILTER_TYPE_CUSTOM:
    {
      /* Valid callback? */
      if(_pstFilterData->stCustom.pfnCallback != orxNULL)
      {
        ma_node_config stConfig;

        /* Stores info */
        _pstFilter->stNode.stCustomNode.pfnCallback       = _pstFilterData->stCustom.pfnCallback;
        _pstFilter->stNode.stCustomNode.pContext          = _pstFilterData->stCustom.pContext;
        _pstFilter->stNode.stCustomNode.stNameID          = _pstFilterData->stNameID;
        _pstFilter->stNode.stCustomNode.u32ChannelNumber  = ma_engine_get_channels(&(sstSoundSystem.stEngine));
        _pstFilter->stNode.stCustomNode.u32SampleRate     = _pstEngineNode->sampleRate;

        /* Inits its config */
        stConfig                  = ma_node_config_init();
        stConfig.vtable           = &(sstSoundSystem.stCustomNodeVTable);
        stConfig.pInputChannels   =
        stConfig.pOutputChannels  = (ma_uint32 *)&(_pstFilter->stNode.stCustomNode.u32ChannelNumber);

        /* Inits it */
        hResult = ma_node_init(ma_engine_get_node_graph(&(sstSoundSystem.stEngine)), &stConfig, &(sstSoundSystem.stResourceManagerConfig.allocationCallbacks), &(_pstFilter->stNode.stCustomNode.stNode));
      }

      break;
    }

    default:
    {
      break;
    }
  }

  /* Failure? */
  if(hResult != MA_SUCCESS)
  {
    /* Updates status */
    _pstFilter->stData.eType = orxSOUND_FILTER_TYPE_NONE;

    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't initialize sound filter <%s>: invalid parameters.", orxString_GetFromID(_pstFilterData->stNameID));
  }

  /* Done! */
  return hResult;
}

static void orxSoundSystem_MiniAudio_DeleteFilter(orxSOUNDSYSTEM_FILTER *_pstFilter)
{
  /* Depending on its type */
  switch(_pstFilter->stData.eType)
  {
    /* Biquad */
    case orxSOUND_FILTER_TYPE_BIQUAD:
    {
      /* Uninits its node */
      ma_biquad_node_uninit(&(_pstFilter->stNode.stBiquadNode), &(sstSoundSystem.stResourceManagerConfig.allocationCallbacks));

      break;
    }

    /* Low Pass */
    case orxSOUND_FILTER_TYPE_LOW_PASS:
    {
      /* Uninits its node */
      ma_lpf_node_uninit(&(_pstFilter->stNode.stLowPassNode), &(sstSoundSystem.stResourceManagerConfig.allocationCallbacks));

      break;
    }

    /* High Pass */
    case orxSOUND_FILTER_TYPE_HIGH_PASS:
    {
      /* Uninits its node */
      ma_hpf_node_uninit(&(_pstFilter->stNode.stHighPassNode), &(sstSoundSystem.stResourceManagerConfig.allocationCallbacks));

      break;
    }

    /* Band Pass */
    case orxSOUND_FILTER_TYPE_BAND_PASS:
    {
      /* Uninits its node */
      ma_bpf_node_uninit(&(_pstFilter->stNode.stBandPassNode), &(sstSoundSystem.stResourceManagerConfig.allocationCallbacks));

      break;
    }

    /* Low Shelf */
    case orxSOUND_FILTER_TYPE_LOW_SHELF:
    {
      /* Uninits its node */
      ma_loshelf_node_uninit(&(_pstFilter->stNode.stLowShelfNode), &(sstSoundSystem.stResourceManagerConfig.allocationCallbacks));

      break;
    }

    /* High Shelf */
    case orxSOUND_FILTER_TYPE_HIGH_SHELF:
    {
      /* Uninits its node */
      ma_hishelf_node_uninit(&(_pstFilter->stNode.stHighShelfNode), &(sstSoundSystem.stResourceManagerConfig.allocationCallbacks));

      break;
    }

    /* Notch */
    case orxSOUND_FILTER_TYPE_NOTCH:
    {
      /* Uninits its node */
      ma_notch_node_uninit(&(_pstFilter->stNode.stNotchNode), &(sstSoundSystem.stResourceManagerConfig.allocationCallbacks));

      break;
    }

    /* Peaking */
    case orxSOUND_FILTER_TYPE_PEAKING:
    {
      /* Uninits its node */
      ma_peak_node_uninit(&(_pstFilter->stNode.stPeakingNode), &(sstSoundSystem.stResourceManagerConfig.allocationCallbacks));

      break;
    }

    /* Delay */
    case orxSOUND_FILTER_TYPE_DELAY:
    {
      /* Uninits its node */
      ma_delay_node_uninit(&(_pstFilter->stNode.stDelayNode), &(sstSoundSystem.stResourceManagerConfig.allocationCallbacks));

      break;
    }

    /* Custom */
    case orxSOUND_FILTER_TYPE_CUSTOM:
    {
      /* Uninits its node */
      ma_node_uninit(&(_pstFilter->stNode.stCustomNode.stNode), &(sstSoundSystem.stResourceManagerConfig.allocationCallbacks));

      break;
    }

    default:
    {
      break;
    }
  }

  /* Deletes it */
  orxThread_RunTask(orxNULL, &orxSoundSystem_MiniAudio_FreeFilter, &orxSoundSystem_MiniAudio_FreeFilter, _pstFilter);

  /* Done! */
  return;
}

static orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_LoadSampleTask(void *_pContext)
{
  orxSOUNDSYSTEM_TASK_PARAM  *pstTaskParam;
  orxSOUNDSYSTEM_SAMPLE      *pstSample;
  orxSTATUS                   eResult = orxSTATUS_FAILURE;
  ma_result                   hResult;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxSoundSystem_LoadSampleTask");

  /* Gets task param */
  pstTaskParam = (orxSOUNDSYSTEM_TASK_PARAM *)_pContext;

  /* Gets sample */
  pstSample = pstTaskParam->pstSample;

  /* Inits data source */
  hResult = ma_resource_manager_data_source_init(&(sstSoundSystem.stResourceManager), pstTaskParam->zLocation, MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_WAIT_INIT | MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_DECODE | MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_ASYNC, NULL, &(pstSample->stResource.stDataSource));

  /* Success? */
  if(hResult == MA_SUCCESS)
  {
    ma_format eFormat;
    ma_uint64 u64Length;

    /* Retrieves channel number & sample rate */
    hResult = ma_resource_manager_data_source_get_data_format((ma_resource_manager_data_source *)&(pstSample->stResource.stDataSource), &(eFormat), (ma_uint32 *)&(pstSample->u32ChannelNumber), (ma_uint32 *)&(pstSample->u32SampleRate), NULL, 0);
    orxASSERT(hResult == MA_SUCCESS);

    /* Gets length */
    hResult = ma_resource_manager_data_source_get_length_in_pcm_frames((ma_resource_manager_data_source *)&(pstSample->stResource.stDataSource), &u64Length);
    orxASSERT(hResult == MA_SUCCESS);

    /* Stores frame number */
    pstSample->u32FrameNumber = (orxU32)u64Length;

    /* Updates status */
    orxMEMORY_BARRIER();
    pstSample->bReady = orxTRUE;

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't load sound sample <%s>: can't process resource [%s].", pstSample->zName, pstTaskParam->zLocation);
  }

  /* Updates task param */
  orxMEMORY_BARRIER();
  pstTaskParam->pstSample = orxNULL;

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_LinkSampleTask(void *_pContext)
{
  orxSOUNDSYSTEM_SOUND *pstSound;
  ma_sound_config       stSoundConfig;
  orxSTATUS             eResult = orxSTATUS_FAILURE;
  ma_result             hResult;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxSoundSystem_LinkSampleTask");

  /* Gets sound */
  pstSound = (orxSOUNDSYSTEM_SOUND *)_pContext;

  /* Inits sound config */
  stSoundConfig             = ma_sound_config_init();
  stSoundConfig.channelsOut = ma_engine_get_channels(&(sstSoundSystem.stEngine));
  stSoundConfig.flags       = MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC | ((sstSoundSystem.u32ListenerNumber == 0) ? MA_SOUND_FLAG_NO_SPATIALIZATION : 0);
  stSoundConfig.pFilePath   = pstSound->stSample.pstSample->stResource.zLocation;

  /* Creates sound */
  hResult = ma_sound_init_ex(&(sstSoundSystem.stEngine), &stSoundConfig, &(pstSound->stSound));

  /* Success? */
  if(hResult == MA_SUCCESS)
  {
    /* Stores filter node */
    pstSound->pstFilterNode     = (ma_node_base *)&(pstSound->stSound.engineNode);

    /* Stores channel number & sample rate */
    pstSound->u32ChannelNumber  = pstSound->stSample.pstSample->u32ChannelNumber;
    pstSound->u32SampleRate     = pstSound->stSample.pstSample->u32SampleRate;

    /* Updates status */
    orxMEMORY_BARRIER();
    pstSound->bReady            = orxTRUE;

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't create sound from sample [%s].", pstSound->zName);
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_CreateStreamTask(void *_pContext)
{
  orxSOUNDSYSTEM_TASK_PARAM  *pstTaskParam;
  orxSOUNDSYSTEM_SOUND       *pstSound;
  orxSTATUS                   eResult = orxSTATUS_FAILURE;
  ma_result                   hResult;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxSoundSystem_CreateStreamTask");

  /* Gets task param */
  pstTaskParam = (orxSOUNDSYSTEM_TASK_PARAM *)_pContext;

  /* Gets sound */
  pstSound = pstTaskParam->pstSound;

  /* Inits data source */
  hResult = ma_resource_manager_data_source_init(&(sstSoundSystem.stResourceManager), pstTaskParam->zLocation, MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_WAIT_INIT | MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_STREAM, NULL, &(pstSound->stStream.stDataSource));

  /* Success? */
  if(hResult == MA_SUCCESS)
  {
    ma_data_source_config stBaseConfig;

    /* Inits base data source config */
    stBaseConfig        = ma_data_source_config_init();
    stBaseConfig.vtable = &(sstSoundSystem.stStreamVTable);

    /* Inits base data source */
    hResult = ma_data_source_init(&stBaseConfig, &(pstSound->stBase));

    /* Success? */
    if(hResult == MA_SUCCESS)
    {
      ma_sound_config stSoundConfig;

      /* Inits sound config */
      stSoundConfig             = ma_sound_config_init();
      stSoundConfig.pDataSource = &(pstSound->stBase);
      stSoundConfig.channelsOut = ma_engine_get_channels(&(sstSoundSystem.stEngine));
      stSoundConfig.flags       = (sstSoundSystem.u32ListenerNumber == 0) ? MA_SOUND_FLAG_NO_SPATIALIZATION : 0;

      /* Stores info */
      hResult = ma_data_source_get_data_format(&(pstSound->stStream.stDataSource), NULL, (ma_uint32 *)&(pstSound->u32ChannelNumber), (ma_uint32 *)&(pstSound->u32SampleRate), NULL, 0);
      orxASSERT(hResult == MA_SUCCESS);

      /* Creates sound */
      hResult = ma_sound_init_ex(&(sstSoundSystem.stEngine), &stSoundConfig, &(pstSound->stSound));

      /* Success? */
      if(hResult == MA_SUCCESS)
      {
        /* Stores filter node */
        pstSound->pstFilterNode = (ma_node_base *)&(pstSound->stSound.engineNode);

        /* Updates status */
        orxMEMORY_BARRIER();
        pstSound->bReady = orxTRUE;

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
      else
      {
        /* Uninits base data source */
        ma_data_source_uninit(&(pstSound->stBase));

        /* Uninits data source */
        ma_resource_manager_data_source_uninit(&(pstSound->stStream.stDataSource));

        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't load sound stream <%s>: can't process resource [%s].", pstSound->zName, pstTaskParam->zLocation);
      }
    }
    else
    {
      /* Uninits data source */
      ma_resource_manager_data_source_uninit(&(pstSound->stStream.stDataSource));

      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't load sound stream <%s>: can't initialize internal data source.", pstSound->zName);
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't load sound stream <%s>: can't initialize resource data source for location [%s].", pstSound->zName, pstTaskParam->zLocation);
  }

  /* Updates task param */
  orxMEMORY_BARRIER();
  pstTaskParam->pstSound = orxNULL;

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_DeleteSampleTask(void *_pContext)
{
  orxSOUNDSYSTEM_SAMPLE  *pstSample;
  orxSTATUS               eResult = orxSTATUS_FAILURE;

  /* Gets sample */
  pstSample = (orxSOUNDSYSTEM_SAMPLE *)_pContext;

  /* Is ready? */
  if(pstSample->bReady != orxFALSE)
  {
    /* Isn't a buffer? */
    if(pstSample->bBuffer == orxFALSE)
    {
      /* Uninits data source */
      ma_resource_manager_data_source_uninit(&(pstSample->stResource.stDataSource));
    }

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_DeleteTask(void *_pContext)
{
  orxSOUNDSYSTEM_SOUND *pstSound;
  orxSTATUS             eResult = orxSTATUS_FAILURE;

  /* Gets sound */
  pstSound = (orxSOUNDSYSTEM_SOUND *)_pContext;

  /* Ready? */
  if(pstSound->bReady != orxFALSE)
  {
    /* Removes all filters */
    orxSoundSystem_RemoveAllFilters(pstSound);

    /* Uninits sound */
    ma_sound_uninit(&(pstSound->stSound));

    /* Has base data source? */
    if(pstSound->stBase.vtable != NULL)
    {
      /* Uninits base data source */
      ma_data_source_uninit(&(pstSound->stBase));
    }

    /* Is stream? */
    if(pstSound->bStream != orxFALSE)
    {
      /* Has data source? */
      if(pstSound->stStream.stDataSource.flags != 0)
      {
        /* Uninits it */
        ma_resource_manager_data_source_uninit(&(pstSound->stStream.stDataSource));
      }
    }
    else
    {
      /* Has data source? */
      if(pstSound->stSample.stDataSource.pData != NULL)
      {
        /* Uninits it */
        ma_audio_buffer_ref_uninit(&(pstSound->stSample.stDataSource));
      }
    }

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_PlayTask(void *_pContext)
{
  orxSOUNDSYSTEM_SOUND *pstSound;
  orxSTATUS             eResult = orxSTATUS_FAILURE;

  /* Gets sound */
  pstSound = (orxSOUNDSYSTEM_SOUND *)_pContext;

  /* Ready? */
  if(pstSound->bReady != orxFALSE)
  {
    /* Plays sound */
    eResult = (ma_sound_start(&(pstSound->stSound)) == MA_SUCCESS) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;

    /* Success? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Updates status */
      pstSound->bStopped = orxFALSE;
    }
  }

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_PauseTask(void *_pContext)
{
  orxSOUNDSYSTEM_SOUND *pstSound;
  orxSTATUS             eResult = orxSTATUS_FAILURE;

  /* Gets sound */
  pstSound = (orxSOUNDSYSTEM_SOUND *)_pContext;

  /* Ready? */
  if(pstSound->bReady != orxFALSE)
  {
    /* Pauses sound */
    eResult = (ma_sound_stop(&(pstSound->stSound)) == MA_SUCCESS) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_StopTask(void *_pContext)
{
  orxSOUNDSYSTEM_SOUND *pstSound;
  orxSTATUS             eResult = orxSTATUS_FAILURE;

  /* Gets sound */
  pstSound = (orxSOUNDSYSTEM_SOUND *)_pContext;

  /* Stops sound */
  if((pstSound->bReady != orxFALSE) && (ma_sound_stop(&(pstSound->stSound)) == MA_SUCCESS))
  {
    /* Rewinds it */
    eResult = (ma_sound_seek_to_pcm_frame(&(pstSound->stSound), 0) == MA_SUCCESS) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;

    /* Success? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Updates engine node's local time */
      ma_node_set_time((ma_node *)&(pstSound->stSound.engineNode), 0);

      /* Is a stream? */
      if(pstSound->bStream != orxFALSE)
      {
        /* Clears pending samples */
        pstSound->stStream.u32PendingSampleNumber = 0;
      }

      /* Updates status */
      pstSound->bStopped = orxTRUE;
    }
  }

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_AddFilterTask(void *_pContext)
{
  orxSOUNDSYSTEM_TASK_PARAM  *pstTaskParam;
  orxSOUNDSYSTEM_SOUND       *pstSound;
  orxSTATUS                   eResult = orxSTATUS_FAILURE;

  /* Gets task param */
  pstTaskParam = (orxSOUNDSYSTEM_TASK_PARAM *)_pContext;

  /* Gets sound */
  pstSound = pstTaskParam->pstSound;

  /* Ready? */
  if(pstSound->bReady != orxFALSE)
  {
    orxSOUNDSYSTEM_FILTER  *pstFilter;
    ma_result               hResult;

    /* Gets filter */
    pstFilter = pstTaskParam->pstFilter;

    /* Inits it */
    hResult = orxSoundSystem_MiniAudio_InitFilter(pstFilter, &(pstFilter->stData), &(pstSound->stSound.engineNode));

    /* Success? */
    if(hResult == MA_SUCCESS)
    {
      ma_node *pstOutputNode;

      /* Retrieves output node */
      orxASSERT(pstSound->pstFilterNode->outputBusCount > 0);
      pstOutputNode = pstSound->pstFilterNode->pOutputBuses[0].pInputNode;

      /* Attaches filter output */
      hResult = ma_node_attach_output_bus(&(pstFilter->stNode), 0, pstOutputNode, 0);

      /* Success? */
      if(hResult == MA_SUCCESS)
      {
        /* Attaches filter input */
        hResult = ma_node_attach_output_bus(pstSound->pstFilterNode, 0, &(pstFilter->stNode), 0);

        /* Success? */
        if(hResult == MA_SUCCESS)
        {
          /* Updates filter node */
          pstSound->pstFilterNode = (ma_node_base *)(&pstFilter->stNode);

          /* Updates status */
          orxMEMORY_BARRIER();
          pstFilter->bReady = orxTRUE;

          /* Updates result */
          eResult = orxSTATUS_SUCCESS;
        }
        else
        {
          /* Restores previous connection */
          hResult = ma_node_attach_output_bus(pstSound->pstFilterNode, 0, pstOutputNode, 0);
          orxASSERT(hResult == MA_SUCCESS);

          /* Deletes filter */
          orxSoundSystem_MiniAudio_DeleteFilter(pstFilter);
        }
      }
      else
      {
        /* Deletes filter */
        orxSoundSystem_MiniAudio_DeleteFilter(pstFilter);
      }
    }
    else
    {
      /* Deletes filter */
      orxSoundSystem_MiniAudio_DeleteFilter(pstFilter);
    }
  }

  /* Updates task param */
  orxMEMORY_BARRIER();
  pstTaskParam->pstSound = orxNULL;

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_RemoveLastFilterTask(void *_pContext)
{
  orxSOUNDSYSTEM_SOUND *pstSound;
  orxSTATUS             eResult = orxSTATUS_FAILURE;

  /* Gets sound */
  pstSound = (orxSOUNDSYSTEM_SOUND *)_pContext;

  /* Ready? */
  if(pstSound->bReady != orxFALSE)
  {
    /* Has filter(s)? */
    if(pstSound->pstFilterNode != (ma_node_base *)&(pstSound->stSound.engineNode))
    {
      ma_node  *pstInputNode, *pstOutputNode;
      ma_result hResult;

      /* Retrieves input & output nodes */
      orxASSERT(pstSound->pstFilterNode->inputBusCount > 0);
      orxASSERT(pstSound->pstFilterNode->outputBusCount > 0);
      pstInputNode  = pstSound->pstFilterNode->pInputBuses[0].head.pNext->pNode;
      pstOutputNode = pstSound->pstFilterNode->pOutputBuses[0].pInputNode;

      /* Deletes its filter (miniaudio will handle the detaching of the node) */
      orxSoundSystem_MiniAudio_DeleteFilter(orxSTRUCT_GET_FROM_FIELD(orxSOUNDSYSTEM_FILTER, stNode, pstSound->pstFilterNode));

      /* Attaches them together */
      hResult = ma_node_attach_output_bus(pstInputNode, 0, pstOutputNode, 0);
      orxASSERT(hResult == MA_SUCCESS);

      /* Updates filter node */
      pstSound->pstFilterNode = (ma_node_base *)pstInputNode;

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
  }

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_RemoveAllFiltersTask(void *_pContext)
{
  orxSOUNDSYSTEM_SOUND *pstSound;
  orxSTATUS             eResult = orxSTATUS_FAILURE;

  /* Gets sound */
  pstSound = (orxSOUNDSYSTEM_SOUND *)_pContext;

  /* Ready? */
  if(pstSound->bReady != orxFALSE)
  {
    /* Has filter(s)? */
    if((pstSound->pstFilterNode != orxNULL) && (pstSound->pstFilterNode != (ma_node_base *)&(pstSound->stSound.engineNode)))
    {
      ma_node_base *pstFilterNode;
      ma_node      *pstOutputNode;
      ma_result     hResult;

      /* Retrieves last output node */
      orxASSERT(pstSound->pstFilterNode->outputBusCount > 0);
      pstOutputNode = pstSound->pstFilterNode->pOutputBuses[0].pInputNode;

      /* For all filter nodes */
      for(pstFilterNode = pstSound->pstFilterNode; pstFilterNode != (ma_node_base *)&(pstSound->stSound.engineNode);)
      {
        ma_node *pstInputNode;

        /* Retrieves its input node */
        orxASSERT(pstFilterNode->inputBusCount > 0);
        pstInputNode = pstFilterNode->pInputBuses[0].head.pNext->pNode;

        /* Deletes its filter (miniaudio will handle the detaching of the node) */
        orxSoundSystem_MiniAudio_DeleteFilter(orxSTRUCT_GET_FROM_FIELD(orxSOUNDSYSTEM_FILTER, stNode, pstFilterNode));

        /* Updates it */
        pstFilterNode = (ma_node_base *)pstInputNode;
      }

      /* Updates filter node */
      pstSound->pstFilterNode = (ma_node_base *)&(pstSound->stSound.engineNode);

      /* Reattaches sound node */
      hResult = ma_node_attach_output_bus(pstSound->pstFilterNode, 0, pstOutputNode, 0);
      orxASSERT(hResult == MA_SUCCESS);
    }

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_SetBusTask(void *_pContext)
{
  orxSOUNDSYSTEM_TASK_PARAM  *pstTaskParam;
  orxSOUNDSYSTEM_SOUND       *pstSound;
  orxSTATUS                   eResult = orxSTATUS_FAILURE;

  /* Gets task param */
  pstTaskParam = (orxSOUNDSYSTEM_TASK_PARAM *)_pContext;

  /* Gets sound */
  pstSound = pstTaskParam->pstSound;

  /* Ready? */
  if(pstSound->bReady != orxFALSE)
  {
    ma_result hResult;

    /* Attaches sound to bus input */
    hResult = ma_node_attach_output_bus(pstSound->pstFilterNode, 0, &(pstTaskParam->pstBus->stGroup.engineNode), 0);

    /* Success? */
    if(hResult == MA_SUCCESS)
    {
      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
  }

  /* Updates task param */
  orxMEMORY_BARRIER();
  pstTaskParam->pstSound = orxNULL;

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_SetVolumeTask(void *_pContext)
{
  orxSOUNDSYSTEM_TASK_PARAM  *pstTaskParam;
  orxSOUNDSYSTEM_SOUND       *pstSound;
  orxSTATUS                   eResult = orxSTATUS_FAILURE;

  /* Gets task param */
  pstTaskParam = (orxSOUNDSYSTEM_TASK_PARAM *)_pContext;

  /* Gets sound */
  pstSound = pstTaskParam->pstSound;

  /* Ready? */
  if(pstSound->bReady != orxFALSE)
  {
    /* Sets volume */
    ma_sound_set_volume(&(pstSound->stSound), pstTaskParam->fVolume);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Updates task param */
  orxMEMORY_BARRIER();
  pstTaskParam->pstSound = orxNULL;

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_SetPitchTask(void *_pContext)
{
  orxSOUNDSYSTEM_TASK_PARAM  *pstTaskParam;
  orxSOUNDSYSTEM_SOUND       *pstSound;
  orxSTATUS                   eResult = orxSTATUS_FAILURE;

  /* Gets task param */
  pstTaskParam = (orxSOUNDSYSTEM_TASK_PARAM *)_pContext;

  /* Gets sound */
  pstSound = pstTaskParam->pstSound;

  /* Ready? */
  if(pstSound->bReady != orxFALSE)
  {
    /* Sets pitch */
    ma_sound_set_pitch(&(pstSound->stSound), pstTaskParam->fPitch);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Updates task param */
  orxMEMORY_BARRIER();
  pstTaskParam->pstSound = orxNULL;

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_SetTimeTask(void *_pContext)
{
  orxSOUNDSYSTEM_TASK_PARAM  *pstTaskParam;
  orxSOUNDSYSTEM_SOUND       *pstSound;
  orxSTATUS                   eResult = orxSTATUS_FAILURE;

  /* Gets task param */
  pstTaskParam = (orxSOUNDSYSTEM_TASK_PARAM *)_pContext;

  /* Gets sound */
  pstSound = pstTaskParam->pstSound;

  /* Ready? */
  if(pstSound->bReady != orxFALSE)
  {
    ma_uint64 u64Time;

    /* Computes time in frames */
    u64Time = orxF2U(pstTaskParam->fTime * orxU2F(pstSound->stSound.engineNode.sampleRate));

    /* Sets it */
    if(ma_sound_seek_to_pcm_frame(&(pstSound->stSound), u64Time) == MA_SUCCESS)
    {
      /* Updates engine node's local time */
      if(ma_node_set_time((ma_node *)&(pstSound->stSound.engineNode), u64Time) == MA_SUCCESS)
      {
        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
    }
  }

  /* Updates task param */
  orxMEMORY_BARRIER();
  pstTaskParam->pstSound = orxNULL;

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_SetPositionTask(void *_pContext)
{
  orxSOUNDSYSTEM_TASK_PARAM  *pstTaskParam;
  orxSOUNDSYSTEM_SOUND       *pstSound;
  orxSTATUS                   eResult = orxSTATUS_FAILURE;

  /* Gets task param */
  pstTaskParam = (orxSOUNDSYSTEM_TASK_PARAM *)_pContext;

  /* Gets sound */
  pstSound = pstTaskParam->pstSound;

  /* Ready? */
  if(pstSound->bReady != orxFALSE)
  {
    /* Updates position */
    ma_sound_set_position(&(pstSound->stSound), sstSoundSystem.fDimensionRatio * pstTaskParam->vPosition.fX, sstSoundSystem.fDimensionRatio * pstTaskParam->vPosition.fY, sstSoundSystem.fDimensionRatio * pstTaskParam->vPosition.fZ);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Updates task param */
  orxMEMORY_BARRIER();
  pstTaskParam->pstSound = orxNULL;

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_SetSpatializationTask(void *_pContext)
{
  orxSOUNDSYSTEM_TASK_PARAM  *pstTaskParam;
  orxSOUNDSYSTEM_SOUND       *pstSound;
  orxSTATUS                   eResult = orxSTATUS_FAILURE;

  /* Gets task param */
  pstTaskParam = (orxSOUNDSYSTEM_TASK_PARAM *)_pContext;

  /* Gets sound */
  pstSound = pstTaskParam->pstSound;

  /* Ready? */
  if(pstSound->bReady != orxFALSE)
  {
    /* Enable? */
    if((pstTaskParam->stSpatialization.fMinDistance >= orxFLOAT_0) && (pstTaskParam->stSpatialization.fMaxDistance >= orxFLOAT_0))
    {
      /* Updates status */
      ma_sound_set_spatialization_enabled(&(pstSound->stSound), orxTRUE);

      /* Updates properties */
      ma_sound_set_min_distance(&(pstSound->stSound), orxMAX(sstSoundSystem.fDimensionRatio * pstTaskParam->stSpatialization.fMinDistance, orxFLOAT_1));
      ma_sound_set_max_distance(&(pstSound->stSound), orxMAX(sstSoundSystem.fDimensionRatio * pstTaskParam->stSpatialization.fMaxDistance, orxFLOAT_1));
      ma_sound_set_min_gain(&(pstSound->stSound), pstTaskParam->stSpatialization.fMinGain);
      ma_sound_set_max_gain(&(pstSound->stSound), pstTaskParam->stSpatialization.fMaxGain);
      ma_sound_set_rolloff(&(pstSound->stSound), pstTaskParam->stSpatialization.fRollOff);
    }
    else
    {
      /* Updates status */
      ma_sound_set_spatialization_enabled(&(pstSound->stSound), orxFALSE);
    }

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Updates task param */
  orxMEMORY_BARRIER();
  pstTaskParam->pstSound = orxNULL;

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_SetPanningTask(void *_pContext)
{
  orxSOUNDSYSTEM_TASK_PARAM  *pstTaskParam;
  orxSOUNDSYSTEM_SOUND       *pstSound;
  orxSTATUS                   eResult = orxSTATUS_FAILURE;

  /* Gets task param */
  pstTaskParam = (orxSOUNDSYSTEM_TASK_PARAM *)_pContext;

  /* Gets sound */
  pstSound = pstTaskParam->pstSound;

  /* Ready? */
  if(pstSound->bReady != orxFALSE)
  {
    /* Updates sound */
    ma_sound_set_pan(&(pstSound->stSound), pstTaskParam->stPanning.fPanning);
    ma_sound_set_pan_mode(&(pstSound->stSound), (pstTaskParam->stPanning.bMix != orxFALSE) ? ma_pan_mode_pan : ma_pan_mode_balance);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Updates task param */
  orxMEMORY_BARRIER();
  pstTaskParam->pstSound = orxNULL;

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_LoopTask(void *_pContext)
{
  orxSOUNDSYSTEM_TASK_PARAM  *pstTaskParam;
  orxSOUNDSYSTEM_SOUND       *pstSound;
  orxSTATUS                   eResult = orxSTATUS_FAILURE;

  /* Gets task param */
  pstTaskParam = (orxSOUNDSYSTEM_TASK_PARAM *)_pContext;

  /* Gets sound */
  pstSound = pstTaskParam->pstSound;

  /* Ready? */
  if(pstSound->bReady != orxFALSE)
  {
    /* Updates sound's status */
    ma_sound_set_looping(&(pstSound->stSound), (pstTaskParam->bLoop != orxFALSE) ? MA_TRUE : MA_FALSE);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Updates task param */
  orxMEMORY_BARRIER();
  pstTaskParam->pstSound = orxNULL;

  /* Done! */
  return eResult;
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
    sstSoundSystem.u32ListenerNumber = (orxConfig_HasValue(orxSOUNDSYSTEM_KZ_CONFIG_LISTENERS) != orxFALSE) ? orxConfig_GetU32(orxSOUNDSYSTEM_KZ_CONFIG_LISTENERS) : orxSOUNDSYSTEM_KU32_DEFAULT_LISTENER_NUMBER;

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
    sstSoundSystem.stVorbisVTable.onInit                                  = &orxSoundSystem_MiniAudio_InitVorbisBackend;
    sstSoundSystem.stVorbisVTable.onUninit                                = &orxSoundSystem_MiniAudio_UninitVorbisBackend;
    sstSoundSystem.apstVTable[0]                                          = &(sstSoundSystem.stVorbisVTable);

    /* Inits data source VTable */
    sstSoundSystem.stStreamVTable.onRead                                  = &orxSoundSystem_MiniAudio_Stream_Read;
    sstSoundSystem.stStreamVTable.onSeek                                  = &orxSoundSystem_MiniAudio_Stream_Seek;
    sstSoundSystem.stStreamVTable.onGetDataFormat                         = &orxSoundSystem_MiniAudio_Stream_GetDataFormat;
    sstSoundSystem.stStreamVTable.onGetCursor                             = &orxSoundSystem_MiniAudio_Stream_GetCursor;
    sstSoundSystem.stStreamVTable.onGetLength                             = &orxSoundSystem_MiniAudio_Stream_GetLength;
    sstSoundSystem.stStreamVTable.onSetLooping                            = &orxSoundSystem_MiniAudio_Stream_SetLooping;

    /* Inits custom node VTable */
    sstSoundSystem.stCustomNodeVTable.onProcess                           = &orxSoundSystem_MiniAudio_ProcessCustomNode;
    sstSoundSystem.stCustomNodeVTable.onGetRequiredInputFrameCount        = NULL;
    sstSoundSystem.stCustomNodeVTable.inputBusCount                       = 1;
    sstSoundSystem.stCustomNodeVTable.outputBusCount                      = 1;
    sstSoundSystem.stCustomNodeVTable.flags                               = MA_NODE_FLAG_CONTINUOUS_PROCESSING;

    /* Inits resource callbacks */
    sstSoundSystem.stCallbacks.onOpen                                     = &orxSoundSystem_MiniAudio_Open;
    sstSoundSystem.stCallbacks.onClose                                    = &orxSoundSystem_MiniAudio_Close;
    sstSoundSystem.stCallbacks.onRead                                     = &orxSoundSystem_MiniAudio_Read;
    sstSoundSystem.stCallbacks.onWrite                                    = &orxSoundSystem_MiniAudio_Write;
    sstSoundSystem.stCallbacks.onSeek                                     = &orxSoundSystem_MiniAudio_Seek;
    sstSoundSystem.stCallbacks.onTell                                     = &orxSoundSystem_MiniAudio_Tell;
    sstSoundSystem.stCallbacks.onInfo                                     = &orxSoundSystem_MiniAudio_Info;

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
        ma_device_info *apstDeviceList;
        ma_uint32       u32DeviceNumber;

        /* No available playback devices? */
        if((ma_context_get_devices(&(sstSoundSystem.stContext), &apstDeviceList, &u32DeviceNumber, NULL, NULL) != MA_SUCCESS)
        || (u32DeviceNumber == 0))
        {
          ma_backend eNullBackend = ma_backend_null;

          /* Uninits context */
          ma_context_uninit(&(sstSoundSystem.stContext));

          /* Re-inits it with the null backend */
          hResult = ma_context_init(&eNullBackend, 1, &stContextConfig, &(sstSoundSystem.stContext));
        }
      }

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
          sstSoundSystem.pstBusBank     = orxBank_Create(orxSOUNDSYSTEM_KU32_BANK_SIZE, sizeof(orxSOUNDSYSTEM_BUS), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_AUDIO);
          sstSoundSystem.pstFilterBank  = orxBank_Create(orxSOUNDSYSTEM_KU32_BANK_SIZE, sizeof(orxSOUNDSYSTEM_FILTER), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_AUDIO);

          /* Valid? */
          if((sstSoundSystem.pstSampleBank != orxNULL) && (sstSoundSystem.pstSoundBank != orxNULL) && (sstSoundSystem.pstBusBank != orxNULL) && (sstSoundSystem.pstFilterBank != orxNULL))
          {
            /* Adds job thread */
            sstSoundSystem.u32WorkerThread = orxThread_Start(&orxSoundSystem_MiniAudio_ProcessJobs, orxSOUNDSYSTEM_KZ_THREAD_NAME, &(sstSoundSystem.stResourceManager));

            /* Valid? */
            if(sstSoundSystem.u32WorkerThread != orxU32_UNDEFINED)
            {
              orxU32 i;

              /* Adds event handler */
              eResult = orxEvent_AddHandler(orxEVENT_TYPE_SYSTEM, orxSoundSystem_MiniAudio_EventHandler);
              orxASSERT(eResult != orxSTATUS_FAILURE);

              /* Registers update  callback */
              eResult = orxClock_Register(orxClock_Get(orxCLOCK_KZ_CORE), orxSoundSystem_MiniAudio_Update, orxNULL, orxMODULE_ID_SOUNDSYSTEM, orxCLOCK_PRIORITY_LOW);
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
              orxBank_Delete(sstSoundSystem.pstBusBank);
              orxBank_Delete(sstSoundSystem.pstFilterBank);
              sstSoundSystem.pstSampleBank  = orxNULL;
              sstSoundSystem.pstSoundBank   = orxNULL;
              sstSoundSystem.pstBusBank     = orxNULL;
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
            if(sstSoundSystem.pstBusBank != orxNULL)
            {
              orxBank_Delete(sstSoundSystem.pstBusBank);
              sstSoundSystem.pstBusBank = orxNULL;
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
    /* Waits for all pending operations */
    while(orxThread_GetTaskCount() != 0)
      ;

    /* Updates status */
    orxFLAG_SET(sstSoundSystem.u32Flags, orxSOUNDSYSTEM_KU32_STATIC_FLAG_EXIT, orxSOUNDSYSTEM_KU32_STATIC_FLAG_NONE);

    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_SYSTEM, orxSoundSystem_MiniAudio_EventHandler);

    /* Unregisters update callback */
    orxClock_Unregister(orxClock_Get(orxCLOCK_KZ_CORE), orxSoundSystem_MiniAudio_Update);

    /* Stops any recording */
    orxSoundSystem_StopRecording();

    /* Uninits engine */
    ma_engine_uninit(&(sstSoundSystem.stEngine));

    /* Uninits context */
    ma_context_uninit(&(sstSoundSystem.stContext));

    /* Uninits resource manager */
    ma_resource_manager_uninit(&(sstSoundSystem.stResourceManager));

    /* Uninits log */
    ma_log_uninit(&(sstSoundSystem.stLog));

    /* Joins worker thread */
    orxThread_Join(sstSoundSystem.u32WorkerThread);

    /* Deletes banks */
    orxBank_Delete(sstSoundSystem.pstSampleBank);
    orxBank_Delete(sstSoundSystem.pstSoundBank);
    orxBank_Delete(sstSoundSystem.pstBusBank);
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
        pstResult->bBuffer           = orxTRUE;
        pstResult->bReady            = orxTRUE;
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
  const orxSTRING zResourceLocation;
  orxSOUNDSYSTEM_SAMPLE *pstResult;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxSoundSystem_LoadSample");

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_zFilename != orxNULL);

  /* Locates resource */
  zResourceLocation = orxResource_Locate(orxSOUND_KZ_RESOURCE_GROUP, _zFilename);

  /* Success? */
  if(zResourceLocation != orxNULL)
  {
    /* Allocates sample */
    pstResult = (orxSOUNDSYSTEM_SAMPLE *)orxBank_Allocate(sstSoundSystem.pstSampleBank);

    /* Valid? */
    if(pstResult != orxNULL)
    {
      orxSOUNDSYSTEM_TASK_PARAM *pstTaskParam;

      /* Clears it */
      orxMemory_Zero(pstResult, sizeof(orxSOUNDSYSTEM_SAMPLE));

      /* Stores name */
      pstResult->zName = orxString_Store(_zFilename);

      /* Stores location */
      pstResult->stResource.zLocation = orxString_Duplicate(zResourceLocation);

      /* Updates status */
      pstResult->bBuffer = orxFALSE;

      /* Prepares task parameter */
      pstTaskParam            = orxSoundSystem_MiniAudio_GetTaskParam(pstResult);
      pstTaskParam->zLocation = zResourceLocation;

      /* Runs load sample task */
      orxThread_RunTask(&orxSoundSystem_MiniAudio_LoadSampleTask, orxNULL, orxNULL, pstTaskParam);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't load sound sample <%s>: can't allocate sample structure.", _zFilename);

      /* Updates result */
      pstResult = orxNULL;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't load sound sample <%s>: can't locate resource.", _zFilename);

    /* Updates result */
    pstResult = orxNULL;
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return pstResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_DeleteSample(orxSOUNDSYSTEM_SAMPLE *_pstSample)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSample != orxNULL);

  /* Runs delete sample task */
  eResult = orxThread_RunTask(&orxSoundSystem_MiniAudio_DeleteSampleTask, &orxSoundSystem_MiniAudio_FreeSample, &orxSoundSystem_MiniAudio_FreeSample, _pstSample);

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
  if(_pstSample->bBuffer != orxFALSE)
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
  orxSOUNDSYSTEM_SOUND *pstResult;

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
    if(_pstSample->bBuffer != orxFALSE)
    {
      /* Updates sound config */
      stSoundConfig.flags       = (sstSoundSystem.u32ListenerNumber == 0) ? MA_SOUND_FLAG_NO_SPATIALIZATION : 0;
      stSoundConfig.pDataSource = &(pstResult->stSample.stDataSource);

      /* Creates buffer data source */
      hResult = ma_audio_buffer_ref_init(orxSOUNDSYSTEM_KE_DEFAULT_FORMAT, _pstSample->u32ChannelNumber, _pstSample->stBuffer.afSampleList, (ma_uint64)_pstSample->u32FrameNumber, &(pstResult->stSample.stDataSource));

      /* Enforces its sample rate */
      pstResult->stSample.stDataSource.sampleRate = _pstSample->u32SampleRate;

      /* Checks */
      orxASSERT(_pstSample->bReady != orxFALSE);
    }
    else
    {
      /* Updates sound config */
      stSoundConfig.flags     = MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC | ((sstSoundSystem.u32ListenerNumber == 0) ? MA_SOUND_FLAG_NO_SPATIALIZATION : 0);
      stSoundConfig.pFilePath = _pstSample->stResource.zLocation;
    }

    /* Success? */
    if(hResult == MA_SUCCESS)
    {
      /* Stores user data */
      pstResult->hUserData = _hUserData;

      /* Stores name */
      pstResult->zName = _pstSample->zName;

      /* Stores sample */
      pstResult->stSample.pstSample = _pstSample;

      /* Updates status */
      pstResult->bStream = orxFALSE;

      /* Ready? */
      if(_pstSample->bReady != orxFALSE)
      {
        /* Creates sound */
        hResult = ma_sound_init_ex(&(sstSoundSystem.stEngine), &stSoundConfig, &(pstResult->stSound));

        /* Success? */
        if(hResult == MA_SUCCESS)
        {
          /* Is buffer? */
          if(_pstSample->bBuffer != orxFALSE)
          {
            //! TEMP: Checks sample rates match until miniaudio supports audio buffers with sample rate
            orxASSERT(pstResult->stSound.engineNode.sampleRate == _pstSample->u32SampleRate && "Sound Sample rate [%u] does not match engine's [%u]: This is currently not supported, until miniaudio adds this feature.", _pstSample->u32SampleRate, pstResult->stSound.engineNode.sampleRate);
          }

          /* Stores filter node */
          pstResult->pstFilterNode    = (ma_node_base *)&(pstResult->stSound.engineNode);

          /* Stores channel number & sample rate */
          pstResult->u32ChannelNumber = _pstSample->u32ChannelNumber;
          pstResult->u32SampleRate    = _pstSample->u32SampleRate;

          /* Updates status */
          pstResult->bReady           = orxTRUE;
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
        /* Runs link sample task */
        orxThread_RunTask(&orxSoundSystem_MiniAudio_LinkSampleTask, orxNULL, orxNULL, pstResult);
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
        pstResult->bStream          = orxTRUE;

        /* Creates sound */
        hResult = ma_sound_init_ex(&(sstSoundSystem.stEngine), &stSoundConfig, &(pstResult->stSound));

        /* Success? */
        if(hResult == MA_SUCCESS)
        {
          /* Stores filter node */
          pstResult->pstFilterNode = (ma_node_base *)&(pstResult->stSound.engineNode);

          /* Stores user data */
          pstResult->hUserData = _hUserData;

          /* Stores name */
          pstResult->zName = orxSTRING_EMPTY;

          /* Updates status */
          pstResult->bReady = orxTRUE;
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

orxSOUNDSYSTEM_SOUND *orxFASTCALL orxSoundSystem_MiniAudio_LoadStream(orxHANDLE _hUserData, const orxSTRING _zFilename)
{
  const orxSTRING       zResourceLocation;
  orxSOUNDSYSTEM_SOUND *pstResult;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxSoundSystem_LoadStream");

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
      orxSOUNDSYSTEM_TASK_PARAM *pstTaskParam;

      /* Clears it */
      orxMemory_Zero(pstResult, sizeof(orxSOUNDSYSTEM_SOUND));

      /* Stores user data */
      pstResult->hUserData = _hUserData;

      /* Stores name */
      pstResult->zName = orxString_Store(_zFilename);

      /* Updates status */
      pstResult->bStream = orxTRUE;

      /* Prepares task parameter */
      pstTaskParam            = orxSoundSystem_MiniAudio_GetTaskParam(pstResult);
      pstTaskParam->zLocation = zResourceLocation;

      /* Runs create stream task */
      orxThread_RunTask(&orxSoundSystem_MiniAudio_CreateStreamTask, orxNULL, orxNULL, pstTaskParam);
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

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return pstResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_Delete(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Runs delete task */
  eResult = orxThread_RunTask(&orxSoundSystem_MiniAudio_DeleteTask, &orxSoundSystem_MiniAudio_FreeSound, &orxSoundSystem_MiniAudio_FreeSound, _pstSound);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_Play(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Ready? */
  if(_pstSound->bReady != orxFALSE)
  {
    /* Plays sound */
    eResult = (ma_sound_start(&(_pstSound->stSound)) == MA_SUCCESS) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;

    /* Success? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Updates status */
      _pstSound->bStopped = orxFALSE;
    }
  }
  else
  {
    /* Runs play task */
    eResult = orxThread_RunTask(&orxSoundSystem_MiniAudio_PlayTask, orxNULL, orxNULL, _pstSound);
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

  /* Ready? */
  if(_pstSound->bReady != orxFALSE)
  {
    /* Pauses sound */
    eResult = (ma_sound_stop(&(_pstSound->stSound)) == MA_SUCCESS) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
  }
  else
  {
    /* Runs pause task */
    eResult = orxThread_RunTask(&orxSoundSystem_MiniAudio_PauseTask, orxNULL, orxNULL, _pstSound);
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_Stop(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Ready? */
  if(_pstSound->bReady != orxFALSE)
  {
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
        if(_pstSound->bStream != orxFALSE)
        {
          /* Clears pending samples */
          _pstSound->stStream.u32PendingSampleNumber = 0;
        }

        /* Updates status */
        _pstSound->bStopped = orxTRUE;
      }
    }
    else
    {
      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* Runs stop task */
    eResult = orxThread_RunTask(&orxSoundSystem_MiniAudio_StopTask, orxNULL, orxNULL, _pstSound);
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
    orxSOUNDSYSTEM_TASK_PARAM *pstTaskParam;

    /* Clears it */
    orxMemory_Zero(pstFilter, sizeof(orxSOUNDSYSTEM_FILTER));

    /* Prepares task parameter */
    pstTaskParam                = orxSoundSystem_MiniAudio_GetTaskParam(_pstSound);
    pstTaskParam->pstFilter     = pstFilter;
    pstFilter->bUseCustomParam  = _bUseCustomParam;
    pstFilter->bReady           = orxFALSE;
    pstFilter->pstSound         = _pstSound;
    orxMemory_Copy(&(pstFilter->stData), _pstFilterData, sizeof(orxSOUND_FILTER_DATA));

    /* Runs add filter task */
    eResult = orxThread_RunTask(&orxSoundSystem_MiniAudio_AddFilterTask, orxNULL, orxNULL, pstTaskParam);
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_RemoveLastFilter(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Ready? */
  if(_pstSound->bReady != orxFALSE)
  {
    /* Has filter(s)? */
    if(_pstSound->pstFilterNode != (ma_node_base *)&(_pstSound->stSound.engineNode))
    {
      /* Updates its status */
      orxSTRUCT_GET_FROM_FIELD(orxSOUNDSYSTEM_FILTER, stNode, _pstSound->pstFilterNode)->bReady = orxFALSE;
    }
  }

  /* Runs remove last filter task */
  eResult = orxThread_RunTask(&orxSoundSystem_MiniAudio_RemoveLastFilterTask, orxNULL, orxNULL, _pstSound);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_RemoveAllFilters(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Ready? */
  if(_pstSound->bReady != orxFALSE)
  {
    /* Has filter(s)? */
    if((_pstSound->pstFilterNode != orxNULL) && (_pstSound->pstFilterNode != (ma_node_base *)&(_pstSound->stSound.engineNode)))
    {
      ma_node_base *pstFilterNode;

      /* For all filter nodes */
      for(pstFilterNode = _pstSound->pstFilterNode;
          pstFilterNode != (ma_node_base *)&(_pstSound->stSound.engineNode);
          pstFilterNode = (ma_node_base *)pstFilterNode->pInputBuses[0].head.pNext->pNode)
      {
        /* Updates its status */
        orxSTRUCT_GET_FROM_FIELD(orxSOUNDSYSTEM_FILTER, stNode, pstFilterNode)->bReady = orxFALSE;
      }
    }
  }

  /* Runs remove all filters task */
  eResult = orxThread_RunTask(&orxSoundSystem_MiniAudio_RemoveAllFiltersTask, orxNULL, orxNULL, _pstSound);

  /* Done! */
  return eResult;
}

orxHANDLE orxFASTCALL orxSoundSystem_MiniAudio_CreateBus(orxSTRINGID _stBusID)
{
  orxSOUNDSYSTEM_BUS *pstBus;
  orxHANDLE           hResult = orxHANDLE_UNDEFINED;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  /* Allocates bus */
  pstBus = (orxSOUNDSYSTEM_BUS *)orxBank_Allocate(sstSoundSystem.pstBusBank);

  /* Valid? */
  if(pstBus != orxNULL)
  {
    ma_sound_group_config stGroupConfig;

    /* Clears it */
    orxMemory_Zero(pstBus, sizeof(orxSOUNDSYSTEM_BUS));

    /* Inits sound group config */
    stGroupConfig             = ma_sound_group_config_init();
    stGroupConfig.channelsOut = ma_engine_get_channels(&(sstSoundSystem.stEngine));

    /* Creates sound group */
    if(ma_sound_group_init_ex(&(sstSoundSystem.stEngine), &stGroupConfig, &(pstBus->stGroup)) == MA_SUCCESS)
    {
      /* Stores filter node */
      pstBus->pstFilterNode = (ma_node_base *)&(pstBus->stGroup.engineNode);

      /* Stores its ID */
      pstBus->stID = _stBusID;

      /* Updates result */
      hResult = (orxHANDLE)pstBus;
    }
    else
    {
      /* Deletes bus */
      orxBank_Free(sstSoundSystem.pstBusBank, pstBus);

      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't create sound bus: can't initialize sound group.");
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't create sound bus: can't allocate memory.");
  }

  /* Done! */
  return hResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_DeleteBus(orxHANDLE _hBus)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  /* Valid? */
  if((_hBus != 0) && (_hBus != orxHANDLE_UNDEFINED))
  {
    orxSOUNDSYSTEM_BUS *pstBus;
    ma_node_output_bus *pstInput;
    ma_node            *pstOutputNode;
    ma_result           hResult;

    /* Gets it */
    pstBus = (orxSOUNDSYSTEM_BUS *)_hBus;

    /* Removes all filters */
    orxSoundSystem_RemoveAllBusFilters(_hBus);

    /* Retrieves output node */
    orxASSERT(pstBus->pstFilterNode->outputBusCount > 0);
    pstOutputNode = pstBus->pstFilterNode->pOutputBuses[0].pInputNode;

    /* For all inputs */
    orxASSERT(pstBus->pstFilterNode->inputBusCount > 0);
    for(pstInput = pstBus->pstFilterNode->pInputBuses[0].head.pNext;
        pstInput != NULL;
        pstInput = pstBus->pstFilterNode->pInputBuses[0].head.pNext)
    {
      /* Attaches it to output */
      hResult = ma_node_attach_output_bus(pstInput->pNode, 0, pstOutputNode, 0);
      orxASSERT(hResult == MA_SUCCESS);
    }

    /* Uninits sound group */
    ma_sound_group_uninit(&(pstBus->stGroup));

    /* Deletes bus */
    orxBank_Free(sstSoundSystem.pstBusBank, pstBus);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_SetBus(orxSOUNDSYSTEM_SOUND *_pstSound, orxHANDLE _hBus)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Valid? */
  if((_hBus != 0) && (_hBus != orxHANDLE_UNDEFINED))
  {
    /* Ready? */
    if(_pstSound->bReady != orxFALSE)
    {
      orxSOUNDSYSTEM_BUS *pstBus;
      ma_result           hResult;

      /* Gets bus */
      pstBus = (orxSOUNDSYSTEM_BUS *)_hBus;

      /* Attaches sound to bus input */
      hResult = ma_node_attach_output_bus(_pstSound->pstFilterNode, 0, &(pstBus->stGroup.engineNode), 0);

      /* Success? */
      if(hResult == MA_SUCCESS)
      {
        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
    }
    else
    {
      orxSOUNDSYSTEM_TASK_PARAM *pstTaskParam;

      /* Prepares task parameter */
      pstTaskParam          = orxSoundSystem_MiniAudio_GetTaskParam(_pstSound);
      pstTaskParam->pstBus  = (orxSOUNDSYSTEM_BUS *)_hBus;

      /* Runs set bus task */
      eResult = orxThread_RunTask(&orxSoundSystem_MiniAudio_SetBusTask, orxNULL, orxNULL, pstTaskParam);
    }
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_SetBusParent(orxHANDLE _hBus, orxHANDLE _hParentBus)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  /* Valid? */
  if((_hBus != 0) && (_hBus != orxHANDLE_UNDEFINED)
  && (_hParentBus != 0) && (_hParentBus != orxHANDLE_UNDEFINED))
  {
    orxSOUNDSYSTEM_BUS *pstBus, *pstParentBus;
    ma_result           hResult;

    /* Gets buses */
    pstBus        = (orxSOUNDSYSTEM_BUS *)_hBus;
    pstParentBus  = (orxSOUNDSYSTEM_BUS *)_hParentBus;

    /* Attaches sound to bus input */
    hResult = ma_node_attach_output_bus(pstBus->pstFilterNode, 0, &(pstParentBus->stGroup.engineNode), 0);

    /* Success? */
    if(hResult == MA_SUCCESS)
    {
      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_AddBusFilter(orxHANDLE _hBus, const orxSOUND_FILTER_DATA *_pstFilterData, orxBOOL _bUseCustomParam)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstFilterData != orxNULL);

  /* Valid? */
  if((_hBus != 0) && (_hBus != orxHANDLE_UNDEFINED))
  {
    orxSOUNDSYSTEM_FILTER *pstFilter;

    /* Allocates filter */
    pstFilter = (orxSOUNDSYSTEM_FILTER *)orxBank_Allocate(sstSoundSystem.pstFilterBank);

    /* Success? */
    if(pstFilter != orxNULL)
    {
      orxSOUNDSYSTEM_BUS *pstBus;
      ma_result           hResult;

      /* Clears it */
      orxMemory_Zero(pstFilter, sizeof(orxSOUNDSYSTEM_FILTER));

      /* Gets bus */
      pstBus = (orxSOUNDSYSTEM_BUS *)_hBus;

      /* Inits filter */
      hResult = orxSoundSystem_MiniAudio_InitFilter(pstFilter, _pstFilterData, &(pstBus->stGroup.engineNode));

      /* Success? */
      if(hResult == MA_SUCCESS)
      {
        ma_node *pstOutputNode;

        /* Retrieves output node */
        orxASSERT(pstBus->pstFilterNode->outputBusCount > 0);
        pstOutputNode = pstBus->pstFilterNode->pOutputBuses[0].pInputNode;

        /* Attaches filter output */
        hResult = ma_node_attach_output_bus(&(pstFilter->stNode), 0, pstOutputNode, 0);

        /* Success? */
        if(hResult == MA_SUCCESS)
        {
          /* Attaches filter input */
          hResult = ma_node_attach_output_bus(pstBus->pstFilterNode, 0, &(pstFilter->stNode), 0);

          /* Success? */
          if(hResult == MA_SUCCESS)
          {
            /* Updates filter node */
            pstBus->pstFilterNode = (ma_node_base *)(&pstFilter->stNode);

            /* Updates filter */
            orxMemory_Copy(&(pstFilter->stData), _pstFilterData, sizeof(orxSOUND_FILTER_DATA));
            pstFilter->bUseCustomParam  = _bUseCustomParam;
            pstFilter->pstBus           = pstBus;
            orxMEMORY_BARRIER();
            pstFilter->bReady           = orxTRUE;

            /* Updates result */
            eResult = orxSTATUS_SUCCESS;
          }
          else
          {
            /* Restores previous connection */
            hResult = ma_node_attach_output_bus(pstBus->pstFilterNode, 0, pstOutputNode, 0);
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
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_RemoveLastBusFilter(orxHANDLE _hBus)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  /* Valid? */
  if((_hBus != 0) && (_hBus != orxHANDLE_UNDEFINED))
  {
    orxSOUNDSYSTEM_BUS *pstBus;

    /* Gets it */
    pstBus = (orxSOUNDSYSTEM_BUS *)_hBus;

    /* Has filter(s)? */
    if(pstBus->pstFilterNode != (ma_node_base *)&(pstBus->stGroup.engineNode))
    {
      ma_node  *pstInputNode, *pstOutputNode;
      ma_result hResult;

      /* Retrieves input & output nodes */
      orxASSERT(pstBus->pstFilterNode->inputBusCount > 0);
      orxASSERT(pstBus->pstFilterNode->outputBusCount > 0);
      pstInputNode  = pstBus->pstFilterNode->pInputBuses[0].head.pNext->pNode;
      pstOutputNode = pstBus->pstFilterNode->pOutputBuses[0].pInputNode;

      /* Deletes its filter (miniaudio will handle the detaching of the node) */
      orxSoundSystem_MiniAudio_DeleteFilter(orxSTRUCT_GET_FROM_FIELD(orxSOUNDSYSTEM_FILTER, stNode, pstBus->pstFilterNode));

      /* Attaches them together */
      hResult = ma_node_attach_output_bus(pstInputNode, 0, pstOutputNode, 0);
      orxASSERT(hResult == MA_SUCCESS);

      /* Updates filter node */
      pstBus->pstFilterNode = (ma_node_base *)pstInputNode;

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_RemoveAllBusFilters(orxHANDLE _hBus)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  /* Valid? */
  if((_hBus != 0) && (_hBus != orxHANDLE_UNDEFINED))
  {
    orxSOUNDSYSTEM_BUS *pstBus;

    /* Gets it */
    pstBus = (orxSOUNDSYSTEM_BUS *)_hBus;

    /* Has filter(s)? */
    if(pstBus->pstFilterNode != (ma_node_base *)&(pstBus->stGroup.engineNode))
    {
      ma_node_base *pstFilterNode;
      ma_node      *pstOutputNode;
      ma_result     hResult;

      /* Retrieves last output node */
      orxASSERT(pstBus->pstFilterNode->outputBusCount > 0);
      pstOutputNode = pstBus->pstFilterNode->pOutputBuses[0].pInputNode;

      /* For all filter nodes */
      for(pstFilterNode = pstBus->pstFilterNode; pstFilterNode != (ma_node_base *)&(pstBus->stGroup.engineNode);)
      {
        ma_node *pstInputNode;

        /* Retrieves its input node */
        orxASSERT(pstFilterNode->inputBusCount > 0);
        pstInputNode = pstFilterNode->pInputBuses[0].head.pNext->pNode;

        /* Deletes its filter (miniaudio will handle the detaching of the node) */
        orxSoundSystem_MiniAudio_DeleteFilter(orxSTRUCT_GET_FROM_FIELD(orxSOUNDSYSTEM_FILTER, stNode, pstFilterNode));

        /* Updates it */
        pstFilterNode = (ma_node_base *)pstInputNode;
      }

      /* Updates filter node */
      pstBus->pstFilterNode = (ma_node_base *)&(pstBus->stGroup.engineNode);

      /* Reattaches sound node */
      hResult = ma_node_attach_output_bus(pstBus->pstFilterNode, 0, pstOutputNode, 0);
      orxASSERT(hResult == MA_SUCCESS);
    }
  }

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

  /* Ready? */
  if(_pstSound->bReady != orxFALSE)
  {
    /* Sets volume */
    ma_sound_set_volume(&(_pstSound->stSound), _fVolume);
  }
  else
  {
    orxSOUNDSYSTEM_TASK_PARAM *pstTaskParam;

    /* Prepares task parameter */
    pstTaskParam          = orxSoundSystem_MiniAudio_GetTaskParam(_pstSound);
    pstTaskParam->fVolume = _fVolume;

    /* Runs set volume task */
    eResult = orxThread_RunTask(&orxSoundSystem_MiniAudio_SetVolumeTask, orxNULL, orxNULL, pstTaskParam);
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_SetPitch(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fPitch)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Ready? */
  if(_pstSound->bReady != orxFALSE)
  {
    /* Sets pitch */
    ma_sound_set_pitch(&(_pstSound->stSound), _fPitch);
  }
  else
  {
    orxSOUNDSYSTEM_TASK_PARAM *pstTaskParam;

    /* Prepares task parameter */
    pstTaskParam          = orxSoundSystem_MiniAudio_GetTaskParam(_pstSound);
    pstTaskParam->fPitch  = _fPitch;

    /* Runs set pitch task */
    eResult = orxThread_RunTask(&orxSoundSystem_MiniAudio_SetPitchTask, orxNULL, orxNULL, pstTaskParam);
  }

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

  /* Ready? */
  if(_pstSound->bReady != orxFALSE)
  {
    /* Computes time in frames */
    u64Time = orxF2U(_fTime * orxU2F(_pstSound->stSound.engineNode.sampleRate));

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
  }
  else
  {
    orxSOUNDSYSTEM_TASK_PARAM *pstTaskParam;

    /* Prepares task parameter */
    pstTaskParam        = orxSoundSystem_MiniAudio_GetTaskParam(_pstSound);
    pstTaskParam->fTime = _fTime;

    /* Runs set time task */
    eResult = orxThread_RunTask(&orxSoundSystem_MiniAudio_SetTimeTask, orxNULL, orxNULL, pstTaskParam);
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

  /* Ready? */
  if(_pstSound->bReady != orxFALSE)
  {
    /* Updates position */
    ma_sound_set_position(&(_pstSound->stSound), sstSoundSystem.fDimensionRatio * _pvPosition->fX, sstSoundSystem.fDimensionRatio * _pvPosition->fY, sstSoundSystem.fDimensionRatio * _pvPosition->fZ);
  }
  else
  {
    orxSOUNDSYSTEM_TASK_PARAM *pstTaskParam;

    /* Prepares task parameter */
    pstTaskParam = orxSoundSystem_MiniAudio_GetTaskParam(_pstSound);
    orxVector_Copy(&(pstTaskParam->vPosition), _pvPosition);

    /* Runs set position task */
    eResult = orxThread_RunTask(&orxSoundSystem_MiniAudio_SetPositionTask, orxNULL, orxNULL, pstTaskParam);
  }

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
    /* Ready? */
    if(_pstSound->bReady != orxFALSE)
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
      orxSOUNDSYSTEM_TASK_PARAM *pstTaskParam;

      /* Prepares task parameter */
      pstTaskParam                                = orxSoundSystem_MiniAudio_GetTaskParam(_pstSound);
      pstTaskParam->stSpatialization.fMinDistance = _fMinDistance;
      pstTaskParam->stSpatialization.fMaxDistance = _fMaxDistance;
      pstTaskParam->stSpatialization.fMinGain     = _fMinGain;
      pstTaskParam->stSpatialization.fMaxGain     = _fMaxGain;
      pstTaskParam->stSpatialization.fRollOff     = _fRollOff;

      /* Runs set spatialization task */
      eResult = orxThread_RunTask(&orxSoundSystem_MiniAudio_SetSpatializationTask, orxNULL, orxNULL, pstTaskParam);
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

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_SetPanning(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fPanning, orxBOOL _bMix)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Ready? */
  if(_pstSound->bReady != orxFALSE)
  {
    /* Updates sound */
    ma_sound_set_pan(&(_pstSound->stSound), _fPanning);
    ma_sound_set_pan_mode(&(_pstSound->stSound), (_bMix != orxFALSE) ? ma_pan_mode_pan : ma_pan_mode_balance);
  }
  else
  {
    orxSOUNDSYSTEM_TASK_PARAM *pstTaskParam;

    /* Prepares task parameter */
    pstTaskParam                      = orxSoundSystem_MiniAudio_GetTaskParam(_pstSound);
    pstTaskParam->stPanning.fPanning  = _fPanning;
    pstTaskParam->stPanning.bMix      = _bMix;

    /* Runs set panning task */
    eResult = orxThread_RunTask(&orxSoundSystem_MiniAudio_SetPanningTask, orxNULL, orxNULL, pstTaskParam);
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_MiniAudio_Loop(orxSOUNDSYSTEM_SOUND *_pstSound, orxBOOL _bLoop)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Ready? */
  if(_pstSound->bReady != orxFALSE)
  {
    /* Updates sound's status */
    ma_sound_set_looping(&(_pstSound->stSound), (_bLoop != orxFALSE) ? MA_TRUE : MA_FALSE);
  }
  else
  {
    orxSOUNDSYSTEM_TASK_PARAM *pstTaskParam;

    /* Prepares task parameter */
    pstTaskParam        = orxSoundSystem_MiniAudio_GetTaskParam(_pstSound);
    pstTaskParam->bLoop =_bLoop;

    /* Runs loop task */
    eResult = orxThread_RunTask(&orxSoundSystem_MiniAudio_LoopTask, orxNULL, orxNULL, pstTaskParam);
  }

  /* Done! */
  return eResult;
}

orxFLOAT orxFASTCALL orxSoundSystem_MiniAudio_GetVolume(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Ready? */
  if(_pstSound->bReady != orxFALSE)
  {
    /* Updates result */
    fResult = ma_node_get_output_bus_volume((ma_node *)&(_pstSound->stSound), 0);
  }
  else
  {
    /* Updates result */
    fResult = -orxFLOAT_1;
  }

  /* Done! */
  return fResult;
}

orxFLOAT orxFASTCALL orxSoundSystem_MiniAudio_GetPitch(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Ready? */
  if(_pstSound->bReady != orxFALSE)
  {
    /* Updates result */
    fResult = _pstSound->stSound.engineNode.pitch;
  }
  else
  {
    /* Updates result */
    fResult = -orxFLOAT_1;
  }

  /* Done! */
  return fResult;
}

orxFLOAT orxFASTCALL orxSoundSystem_MiniAudio_GetTime(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Ready? */
  if(_pstSound->bReady != orxFALSE)
  {
    ma_uint64 u64Time;

    /* Gets time */
    u64Time = ma_sound_get_time_in_pcm_frames(&(_pstSound->stSound));

    /* Updates result */
    fResult = orxU2F(u64Time) / orxU2F(_pstSound->stSound.engineNode.sampleRate);
  }
  else
  {
    /* Updates result */
    fResult = -orxFLOAT_1;
  }

  /* Done! */
  return fResult;
}

orxVECTOR *orxFASTCALL orxSoundSystem_MiniAudio_GetPosition(const orxSOUNDSYSTEM_SOUND *_pstSound, orxVECTOR *_pvPosition)
{
  orxVECTOR *pvResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);
  orxASSERT(_pvPosition != orxNULL);

  /* Ready? */
  if(_pstSound->bReady != orxFALSE)
  {
    ma_vec3f vPosition;

    /* Gets position */
    vPosition = ma_sound_get_position(&(_pstSound->stSound));

    /* Updates result */
    pvResult = _pvPosition;
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
  if((_pstSound->bReady != orxFALSE) && (sstSoundSystem.u32ListenerNumber > 0))
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
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);
  orxASSERT(_pfPanning != orxNULL);
  orxASSERT(_pbMix != orxNULL);

  /* Playing? */
  if((_pstSound->bReady == orxFALSE) || (ma_sound_is_playing(&(_pstSound->stSound)) != MA_FALSE))
  {
    /* Updates values */
    *_pfPanning = _pstSound->stSound.engineNode.panner.pan;
    *_pbMix     = (_pstSound->stSound.engineNode.panner.mode == ma_pan_mode_pan) ? orxTRUE : orxFALSE;

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Updates values */
    *_pfPanning = orxFLOAT_0;
    *_pbMix     = orxFALSE;

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

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
  bResult = ((_pstSound->bReady != orxFALSE) && (ma_sound_is_looping(&(_pstSound->stSound)) != MA_FALSE)) ? orxTRUE : orxFALSE;

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

  /* Ready & not an empty stream? */
  if((_pstSound->bReady != orxFALSE)
  && ((_pstSound->bStream == orxFALSE) || (_pstSound->stStream.stDataSource.flags != 0)))
  {
    /* Gets length */
    hResult = ma_sound_get_length_in_pcm_frames((ma_sound *)&(_pstSound->stSound), &u64Length);

    /* Success? */
    if(hResult == MA_SUCCESS)
    {
      /* Updates result */
      fResult = orxU2F(u64Length) / orxU2F(_pstSound->stSound.engineNode.sampleRate);
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
  if((_pstSound->bReady == orxFALSE) || (ma_sound_is_playing(&(_pstSound->stSound)) != MA_FALSE))
  {
    /* Updates result */
    eResult = orxSOUNDSYSTEM_STATUS_PLAY;
  }
  /* Paused/Stopped */
  else
  {
    /* Updates result */
    eResult = ((_pstSound->bStopped != orxFALSE) || (ma_sound_get_time_in_pcm_frames(&(_pstSound->stSound)) == 0) || (ma_sound_at_end(&(_pstSound->stSound)) != MA_FALSE)) ? orxSOUNDSYSTEM_STATUS_STOP : orxSOUNDSYSTEM_STATUS_PAUSE;
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
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_LoadStream, SOUNDSYSTEM, LOAD_STREAM);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_Delete, SOUNDSYSTEM, DELETE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_Play, SOUNDSYSTEM, PLAY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_Pause, SOUNDSYSTEM, PAUSE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_Stop, SOUNDSYSTEM, STOP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_AddFilter, SOUNDSYSTEM, ADD_FILTER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_RemoveLastFilter, SOUNDSYSTEM, REMOVE_LAST_FILTER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_RemoveAllFilters, SOUNDSYSTEM, REMOVE_ALL_FILTERS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_CreateBus, SOUNDSYSTEM, CREATE_BUS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_DeleteBus, SOUNDSYSTEM, DELETE_BUS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_SetBus, SOUNDSYSTEM, SET_BUS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_SetBusParent, SOUNDSYSTEM, SET_BUS_PARENT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_AddBusFilter, SOUNDSYSTEM, ADD_BUS_FILTER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_RemoveLastBusFilter, SOUNDSYSTEM, REMOVE_LAST_BUS_FILTER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_MiniAudio_RemoveAllBusFilters, SOUNDSYSTEM, REMOVE_ALL_BUS_FILTERS);
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
