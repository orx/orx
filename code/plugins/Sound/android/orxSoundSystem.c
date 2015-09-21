/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2015 Orx-Project
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
 * @date 07/01/2009
 * @author iarwain@orx-project.org
 *
 * OpenAL sound system plugin implementation
 *
 */


#include "orxPluginAPI.h"

#include <AL/al.h>
#include <AL/alc.h>
#include "ivorbiscodec.h"
#include "ivorbisfile.h"


/** Module flags
 */
#define orxSOUNDSYSTEM_KU32_STATIC_FLAG_NONE      0x00000000 /**< No flags */

#define orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY     0x00000001 /**< Ready flag */
#define orxSOUNDSYSTEM_KU32_STATIC_FLAG_RECORDING 0x00000002 /**< Recording flag */
#define orxSOUNDSYSTEM_KU32_STATIC_FLAG_PAUSED    0x00000004 /**< Paused flag */

#define orxSOUNDSYSTEM_KU32_STATIC_MASK_ALL       0xFFFFFFFF /**< All mask */


/** Misc defines
 */
#define orxSOUNDSYSTEM_KU32_BANK_SIZE                     32
#define orxSOUNDSYSTEM_KS32_DEFAULT_STREAM_BUFFER_NUMBER  4
#define orxSOUNDSYSTEM_KS32_DEFAULT_STREAM_BUFFER_SIZE    4096
#define orxSOUNDSYSTEM_KS32_DEFAULT_RECORDING_FREQUENCY   44100
#define orxSOUNDSYSTEM_KF_DEFAULT_DIMENSION_RATIO         orx2F(0.01f)
#define orxSOUNDSYSTEM_KF_DEFAULT_THREAD_SLEEP_TIME       orx2F(0.001f)
#define orxSOUNDSYSTEM_KZ_THREAD_NAME                     "Sound Streaming"

#ifdef __orxDEBUG__

#define alASSERT()                                                      \
do                                                                      \
{                                                                       \
  ALenum eError = alGetError();                                         \
  orxASSERT(eError == AL_NO_ERROR && "OpenAL error code: 0x%X", eError);\
} while(orxFALSE)

#else /* __orxDEBUG__ */

#define alASSERT()

#endif /* __orxDEBUG__ */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Internal info structure
 */
typedef struct __orxSOUNDSYSTEM_INFO_t
{
  orxU32 u32ChannelNumber;
  orxU32 u32FrameNumber;
  orxU32 u32SampleRate;

} orxSOUNDSYSTEM_INFO;

/** Internal data structure
 */
typedef struct __orxSOUNDSYSTEM_DATA_t
{
  orxHANDLE           hResource;
  OggVorbis_File      stVf;
  orxSOUNDSYSTEM_INFO stInfo;

#ifdef __orxDEBUG__
  orxU32              u32NameID;
#endif /* __orxDEBUG__ */

} orxSOUNDSYSTEM_DATA;

/** Internal sample structure
 */
struct __orxSOUNDSYSTEM_SAMPLE_t
{
  volatile ALuint     uiBuffer;
  orxFLOAT            fDuration;
  orxSOUNDSYSTEM_DATA stData;
};

/** Internal sound structure
 */
struct __orxSOUNDSYSTEM_SOUND_t
{
  orxBOOL   bIsStream;
  ALuint    uiSource;
  orxFLOAT  fDuration;

  union
  {
    /* Sample */
    struct
    {
      orxSOUNDSYSTEM_SAMPLE  *pstSample;
    };

    /* Stream */
    struct
    {
      orxLINKLIST_NODE        stNode;
      orxBOOL                 bDelete;
      orxBOOL                 bLoop;
      orxBOOL                 bStop;
      orxBOOL                 bPause;
      const orxSTRING         zReference;
      orxS32                  s32PacketID;
      orxSOUNDSYSTEM_DATA     stData;

      ALuint                  auiBufferList[0];
    };
  };
};

/** Static structure
 */
typedef struct __orxSOUNDSYSTEM_STATIC_t
{
  ALCdevice              *poDevice;           /**< OpenAL device */
  ALCdevice              *poCaptureDevice;    /**< OpenAL capture device */
  ALCcontext             *poContext;          /**< OpenAL context */
  orxBANK                *pstSampleBank;      /**< Sound bank */
  orxBANK                *pstSoundBank;       /**< Sound bank */
  orxFLOAT                fDimensionRatio;    /**< Dimension ratio */
  orxFLOAT                fRecDimensionRatio; /**< Reciprocal dimension ratio */
  orxU32                  u32StreamingThread; /**< Streaming thread */
  orxU32                  u32Flags;           /**< Status flags */
  orxLINKLIST             stStreamList;       /**< Stream list */
  orxSOUND_EVENT_PAYLOAD  stRecordingPayload; /**< Recording payload */
  orxS32                  s32StreamBufferSize;/**< Stream buffer size */
  orxS32                  s32StreamBufferNumber; /**< Stream buffer number */
  orxS16                 *as16StreamBuffer;   /**< Stream buffer */
  ALuint                 *auiWorkBufferList;  /**< Buffer list */
  orxTHREAD_SEMAPHORE    *pstStreamSemaphore; /**< Stream semaphore */

} orxSOUNDSYSTEM_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxSOUNDSYSTEM_STATIC sstSoundSystem;
static ov_callbacks sstOggVorbis_Callbacks;

/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

typedef void (ALC_APIENTRY*LPALCDEVICEPAUSESOFT)(ALCdevice *device);
typedef void (ALC_APIENTRY*LPALCDEVICERESUMESOFT)(ALCdevice *device);

static LPALCDEVICEPAUSESOFT        alcDevicePauseSOFT              = NULL;
static LPALCDEVICERESUMESOFT       alcDeviceResumeSOFT             = NULL;

static orxSTATUS orxFASTCALL orxSoundSystem_Android_EventHandler(const orxEVENT *_pstEvent)
{
  if(_pstEvent->eID == orxSYSTEM_EVENT_BACKGROUND && !orxFLAG_TEST(sstSoundSystem.u32Flags, orxSOUNDSYSTEM_KU32_STATIC_FLAG_PAUSED))
  {
    alcDevicePauseSOFT(sstSoundSystem.poDevice);
    orxThread_Enable(orxTHREAD_KU32_FLAG_NONE, orxTHREAD_GET_FLAG_FROM_ID(sstSoundSystem.u32StreamingThread));
    orxFLAG_SET(sstSoundSystem.u32Flags, orxSOUNDSYSTEM_KU32_STATIC_FLAG_PAUSED, orxSOUNDSYSTEM_KU32_STATIC_FLAG_NONE);
  }

  if(_pstEvent->eID == orxSYSTEM_EVENT_FOREGROUND && orxFLAG_TEST(sstSoundSystem.u32Flags, orxSOUNDSYSTEM_KU32_STATIC_FLAG_PAUSED))
  {
    alcDeviceResumeSOFT(sstSoundSystem.poDevice);
    orxThread_Enable(orxTHREAD_GET_FLAG_FROM_ID(sstSoundSystem.u32StreamingThread), orxTHREAD_KU32_FLAG_NONE);
    orxFLAG_SET(sstSoundSystem.u32Flags, orxSOUNDSYSTEM_KU32_STATIC_FLAG_NONE, orxSOUNDSYSTEM_KU32_STATIC_FLAG_PAUSED);
  }

  return orxSTATUS_SUCCESS;
}

static size_t read_func(void* ptr, size_t size, size_t nmemb, void* datasource)
{
  orxS64 s64Read = orxResource_Read((orxHANDLE) datasource, size * nmemb, ptr, orxNULL, orxNULL);

  return (size_t)s64Read;
}

static int seek_func(void* datasource, ogg_int64_t offset, int whence)
{
  orxResource_Seek((orxHANDLE)datasource, (orxS64)offset, (orxSEEK_OFFSET_WHENCE)whence);

  return 0;
}

static int close_func(void* datasource)
{
  orxResource_Close((orxHANDLE)datasource);

  return 0;
}

static long tell_func(void* datasource)
{
  return (long)orxResource_Tell((orxHANDLE)datasource);
}

static orxINLINE orxSTATUS orxSoundSystem_Android_OpenFile(orxSOUNDSYSTEM_DATA *_pstData)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  if(ov_open_callbacks(_pstData->hResource, &_pstData->stVf, NULL, 0, sstOggVorbis_Callbacks) == 0)
  {
    vorbis_info *info = ov_info(&_pstData->stVf, -1);

    /* Stores info */
    _pstData->stInfo.u32ChannelNumber = (orxU32)info->channels;
    _pstData->stInfo.u32FrameNumber   = (orxU32)ov_pcm_total(&_pstData->stVf, -1);
    _pstData->stInfo.u32SampleRate    = (orxU32)info->rate;

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Closes resource */
    orxResource_Close(_pstData->hResource);

    /* Removes data */
    _pstData->hResource = orxNULL;
  }

  /* Done! */
  return eResult;
}

static orxINLINE void orxSoundSystem_Android_CloseFile(orxSOUNDSYSTEM_DATA *_pstData)
{
  /* Checks */
  orxASSERT(_pstData != orxNULL);

  /* Has valid file? */
  if(_pstData->hResource != orxNULL)
  {
    /* Closes file */
    ov_clear(&_pstData->stVf);
    _pstData->hResource = orxNULL;
  }

  /* Done! */
  return;
}

static orxINLINE orxU32 orxSoundSystem_Android_Read(orxSOUNDSYSTEM_DATA *_pstData, orxU32 _u32FrameNumber, void *_pBuffer)
{
  orxU32 u32Result, u32RequestBytes, u32BytesRead, u32FrameSize;
  char *pBuffer;
  int current_section;
  
  /* Checks */
  orxASSERT(_pstData != orxNULL);

  /* Has valid file? */
  if(_pstData->hResource != orxNULL)
  {
    pBuffer = (char*) _pBuffer;
    u32FrameSize = _pstData->stInfo.u32ChannelNumber * sizeof(orxS16);
    u32RequestBytes = _u32FrameNumber * u32FrameSize;

    /* Reads frames */
    u32Result = 0;
    
    do {
      u32BytesRead = ov_read(&_pstData->stVf, pBuffer, u32RequestBytes, &current_section);
      u32Result += (u32BytesRead / u32FrameSize);
      pBuffer += u32BytesRead;
      u32RequestBytes -= u32BytesRead;
    } while(u32BytesRead != 0 && u32Result != _u32FrameNumber);
  }
  else
  {
    /* Clears buffer */
    orxMemory_Zero(_pBuffer, _u32FrameNumber * _pstData->stInfo.u32ChannelNumber * sizeof(orxS16));

    /* Updates result */
    u32Result = _u32FrameNumber;
  }

  /* Done! */
  return u32Result;
}

static orxINLINE void orxSoundSystem_Android_Rewind(orxSOUNDSYSTEM_DATA *_pstData)
{
  /* Checks */
  orxASSERT(_pstData != orxNULL);

  /* Has valid file? */
  if(_pstData->hResource != orxNULL)
  {
    /* Seeks start */
    ov_raw_seek(&_pstData->stVf, 0);
  }

  /* Done! */
  return;
}

static orxSTATUS orxFASTCALL orxSoundSystem_Android_FreeSound(void *_pContext)
{
  orxSOUNDSYSTEM_SOUND *pstSound;
  orxSTATUS             eResult = orxSTATUS_SUCCESS;

  /* Gets sound */
  pstSound = (orxSOUNDSYSTEM_SOUND *)_pContext;

  /* Deletes sound */
  orxBank_Free(sstSoundSystem.pstSoundBank, pstSound);

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxSoundSystem_Android_FreeSample(void *_pContext)
{
  orxSOUNDSYSTEM_SAMPLE  *pstSample;
  orxSTATUS               eResult = orxSTATUS_SUCCESS;

  /* Gets sample */
  pstSample = (orxSOUNDSYSTEM_SAMPLE *)_pContext;

  /* Deletes sample  */
  orxBank_Free(sstSoundSystem.pstSampleBank, pstSample);

  /* Done! */
  return eResult;
}

static void orxFASTCALL orxSoundSystem_Android_FillStream(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  /* Checks */
  orxASSERT(_pstSound != orxNULL);

  /* Valid? */
  if(_pstSound->fDuration > orxFLOAT_0)
  {
    /* Not stopped? */
    if(_pstSound->bStop == orxFALSE)
    {
      ALint   iBufferNumber = 0;
      ALuint *puiBufferList;

      /* Gets number of queued buffers */
      alGetSourcei(_pstSound->uiSource, AL_BUFFERS_QUEUED, &iBufferNumber);
      alASSERT();

      /* None found? */
      if(iBufferNumber == 0)
      {
        /* Uses initial buffer list */
        puiBufferList = _pstSound->auiBufferList;

        /* Updates buffer number */
        iBufferNumber = sstSoundSystem.s32StreamBufferNumber;
      }
      else
      {
        /* Gets number of processed buffers */
        iBufferNumber = 0;
        alGetSourcei(_pstSound->uiSource, AL_BUFFERS_PROCESSED, &iBufferNumber);
        alASSERT();

        /* Found any? */
        if(iBufferNumber > 0)
        {
          /* Uses local list */
          puiBufferList = sstSoundSystem.auiWorkBufferList;

          /* Unqueues them all */
          alSourceUnqueueBuffers(_pstSound->uiSource, orxMIN(iBufferNumber, sstSoundSystem.s32StreamBufferNumber), puiBufferList);
          alASSERT();
        }
      }

      /* Needs processing? */
      if(iBufferNumber > 0)
      {
        orxU32                 u32BufferFrameNumber, u32FrameNumber, i;
        orxSOUND_EVENT_PAYLOAD stPayload;

        /* Clears payload */
        orxMemory_Zero(&stPayload, sizeof(orxSOUND_EVENT_PAYLOAD));

        /* Stores recording name */
        stPayload.zSoundName = _pstSound->zReference;

        /* Stores stream info */
        stPayload.stStream.stInfo.u32SampleRate     = _pstSound->stData.stInfo.u32SampleRate;
        stPayload.stStream.stInfo.u32ChannelNumber  = _pstSound->stData.stInfo.u32ChannelNumber;

        /* Stores time stamp */
        stPayload.stStream.stPacket.fTimeStamp = (orxFLOAT)orxSystem_GetTime();

        /* Gets buffer's frame number */
        u32BufferFrameNumber = sstSoundSystem.s32StreamBufferSize / _pstSound->stData.stInfo.u32ChannelNumber;

        /* For all processed buffers */
        for(i = 0, u32FrameNumber = u32BufferFrameNumber; i < (orxU32)iBufferNumber; i++)
        {
          orxBOOL bEOF = orxFALSE;

          /* Fills buffer */
          u32FrameNumber = orxSoundSystem_Android_Read(&(_pstSound->stData), u32BufferFrameNumber, sstSoundSystem.as16StreamBuffer);

          /* Inits packet */
          stPayload.stStream.stPacket.u32SampleNumber = u32FrameNumber * _pstSound->stData.stInfo.u32ChannelNumber;
          stPayload.stStream.stPacket.as16SampleList  = sstSoundSystem.as16StreamBuffer;
          stPayload.stStream.stPacket.bDiscard        = orxFALSE;
          stPayload.stStream.stPacket.s32ID           = _pstSound->s32PacketID++;

          /* Sends event */
          orxEVENT_SEND(orxEVENT_TYPE_SOUND, orxSOUND_EVENT_PACKET, orxNULL, orxNULL, &stPayload);

          /* Should proceed? */
          if(stPayload.stStream.stPacket.bDiscard == orxFALSE)
          {
            /* Success? */
            if(u32FrameNumber > 0)
            {
              /* Transfers its data */
              alBufferData(puiBufferList[i], (_pstSound->stData.stInfo.u32ChannelNumber > 1) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, stPayload.stStream.stPacket.as16SampleList, (ALsizei)(stPayload.stStream.stPacket.u32SampleNumber * sizeof(orxS16)), (ALsizei)_pstSound->stData.stInfo.u32SampleRate);
              alASSERT();

              /* Queues it */
              alSourceQueueBuffers(_pstSound->uiSource, 1, &puiBufferList[i]);
              alASSERT();

              /* End of file? */
              if(u32FrameNumber < u32BufferFrameNumber)
              {
                /* Updates status */
                bEOF = orxTRUE;
              }
            }
            else
            {
              /* Clears its data */
              alBufferData(puiBufferList[i], (_pstSound->stData.stInfo.u32ChannelNumber > 1) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, stPayload.stStream.stPacket.as16SampleList, 0, (ALsizei)_pstSound->stData.stInfo.u32SampleRate);
              alASSERT();

              /* Queues it */
              alSourceQueueBuffers(_pstSound->uiSource, 1, &puiBufferList[i]);
              alASSERT();

              /* Updates status */
              bEOF = orxTRUE;
            }

            /* Ends of file? */
            if(bEOF != orxFALSE)
            {
              /* Rewinds file */
              orxSoundSystem_Android_Rewind(&(_pstSound->stData));

              /* Not looping? */
              if(_pstSound->bLoop == orxFALSE)
              {
                /* Stops */
                _pstSound->bStop = orxTRUE;
                break;
              }
            }
          }
          else
          {
            /* Clears its data */
            alBufferData(puiBufferList[i], (_pstSound->stData.stInfo.u32ChannelNumber > 1) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, stPayload.stStream.stPacket.as16SampleList, 0, (ALsizei)_pstSound->stData.stInfo.u32SampleRate);
            alASSERT();

            /* Queues it */
            alSourceQueueBuffers(_pstSound->uiSource, 1, &puiBufferList[i]);
            alASSERT();
          }
        }
      }

      /* Should continue */
      if(_pstSound->bStop == orxFALSE)
      {
        ALint iState;

        /* Gets actual state */
        alGetSourcei(_pstSound->uiSource, AL_SOURCE_STATE, &iState);
        alASSERT();

        /* Should pause? */
        if(_pstSound->bPause != orxFALSE)
        {
          /* Not paused? */
          if(iState != AL_PAUSED)
          {
            /* Pauses source */
            alSourcePause(_pstSound->uiSource);
            alASSERT();
          }
        }
        else
        {
          /* Stopped? */
          if((iState == AL_STOPPED) || (iState == AL_INITIAL) || (iState == AL_PAUSED))
          {
            /* Resumes play */
            alSourcePlay(_pstSound->uiSource);
            alASSERT();
          }
        }
      }
    }
    else
    {
      ALint iState;

      /* Gets actual state */
      alGetSourcei(_pstSound->uiSource, AL_SOURCE_STATE, &iState);
      alASSERT();

      /* Should stop */
      if((iState == AL_PLAYING) || (iState == AL_PAUSED))
      {
        /* Stops source */
        alSourceStop(_pstSound->uiSource);
        alASSERT();

        /* Rewinds file */
        orxSoundSystem_Android_Rewind(&(_pstSound->stData));

        /* Gets actual state */
        alGetSourcei(_pstSound->uiSource, AL_SOURCE_STATE, &iState);
        alASSERT();
      }

      /* Stopped? */
      if((iState == AL_STOPPED) || (iState == AL_INITIAL))
      {
        ALint iQueuedBufferNumber = 0, iProcessedBufferNumber;

        /* Gets queued & processed buffer numbers */
        alGetSourcei(_pstSound->uiSource, AL_BUFFERS_QUEUED, &iQueuedBufferNumber);
        alASSERT();
        alGetSourcei(_pstSound->uiSource, AL_BUFFERS_PROCESSED, &iProcessedBufferNumber);
        alASSERT();

        /* Checks */
        orxASSERT(iProcessedBufferNumber <= iQueuedBufferNumber);
        orxASSERT(iQueuedBufferNumber <= sstSoundSystem.s32StreamBufferNumber);

        /* Found any? */
        if(iQueuedBufferNumber > 0)
        {
          /* Updates sound packet ID */
          _pstSound->s32PacketID -= (orxS32)(iQueuedBufferNumber - iProcessedBufferNumber);

          /* Unqueues them */
          alSourceUnqueueBuffers(_pstSound->uiSource, orxMIN(iQueuedBufferNumber, sstSoundSystem.s32StreamBufferNumber), sstSoundSystem.auiWorkBufferList);
          alASSERT();
        }
      }
    }
  }

  /* Done! */
  return;
}

static orxSTATUS orxFASTCALL orxSoundSystem_Android_UpdateStreaming(void *_pContext)
{
  orxLINKLIST_NODE *pstNode;
  orxSTATUS         eResult = orxSTATUS_SUCCESS;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxSoundSystem_UpdateStreaming");

  /* For all streams nodes */
  for(pstNode = orxLinkList_GetFirst(&(sstSoundSystem.stStreamList));
      pstNode != orxNULL;
     )
  {
    orxSOUNDSYSTEM_SOUND *pstSound;

    /* Gets associated sound */
    pstSound = orxSTRUCT_GET_FROM_FIELD(orxSOUNDSYSTEM_SOUND, stNode, pstNode);

    /* Marked for deletion? */
    if(pstSound->bDelete != orxFALSE)
    {
      /* Deletes source */
      alDeleteSources(1, &(pstSound->uiSource));
      alASSERT();

      /* Closes audio file */
      orxSoundSystem_Android_CloseFile(&(pstSound->stData));

      /* Clears buffers */
      alDeleteBuffers(sstSoundSystem.s32StreamBufferNumber, pstSound->auiBufferList);
      alASSERT();

      /* Gets next node */
      pstNode = orxLinkList_GetNext(pstNode);

      /* Removes it from list */
      orxThread_WaitSemaphore(sstSoundSystem.pstStreamSemaphore);
      orxLinkList_Remove(&(pstSound->stNode));
      orxThread_SignalSemaphore(sstSoundSystem.pstStreamSemaphore);

      /* Postpones sound deletion from bank on main thread */
      orxThread_RunTask(orxNULL, orxSoundSystem_Android_FreeSound, orxNULL, pstSound);
    }
    else
    {
      /* Fills its stream */
      orxSoundSystem_Android_FillStream(pstSound);

      /* Gets next node */
      pstNode = orxLinkList_GetNext(pstNode);
    }
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Sleeps before next update */
  orxSystem_Delay(orxSOUNDSYSTEM_KF_DEFAULT_THREAD_SLEEP_TIME);

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxSoundSystem_Android_CreateStreamTask(void *_pContext)
{
  orxSOUNDSYSTEM_SOUND *pstSound;
  orxSTATUS             eResult = orxSTATUS_FAILURE;

  /* Gets sound */
  pstSound = (orxSOUNDSYSTEM_SOUND *)_pContext;

  /* Opens file */
  if(orxSoundSystem_Android_OpenFile(&(pstSound->stData)) != orxSTATUS_FAILURE)
  {
    /* Stores duration */
    pstSound->fDuration = orxU2F(pstSound->stData.stInfo.u32FrameNumber) / orx2F(pstSound->stData.stInfo.u32SampleRate);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Clears duration */
    pstSound->fDuration = orxFLOAT_0;

    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't load sound stream <%s>: invalid data.", orxString_GetFromID(pstSound->stData.u32NameID));
  }

  /* Adds it to the list */
  orxThread_WaitSemaphore(sstSoundSystem.pstStreamSemaphore);
  orxLinkList_AddEnd(&(sstSoundSystem.stStreamList), &(pstSound->stNode));
  orxThread_SignalSemaphore(sstSoundSystem.pstStreamSemaphore);

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxSoundSystem_Android_LoadSampleTask(void *_pContext)
{
  orxSOUNDSYSTEM_SAMPLE  *pstSample;
  ALuint                  uiBuffer = 0;
  orxSTATUS               eResult = orxSTATUS_FAILURE;

  /* Gets sample */
  pstSample = (orxSOUNDSYSTEM_SAMPLE *)_pContext;

  /* Generates an OpenAL buffer */
  alGenBuffers(1, &uiBuffer);
  alASSERT();

  /* Gets info from data */
  if(orxSoundSystem_Android_OpenFile(&(pstSample->stData)) != orxSTATUS_FAILURE)
  {
    void   *pBuffer;
    orxU32  u32BufferSize;

    /* Gets buffer size */
    u32BufferSize = pstSample->stData.stInfo.u32FrameNumber * pstSample->stData.stInfo.u32ChannelNumber * sizeof(orxS16);

    /* Allocates buffer */
    if((pBuffer = orxMemory_Allocate(u32BufferSize, orxMEMORY_TYPE_TEMP)) != orxNULL)
    {
      orxU32 u32ReadFrameNumber;

      /* Reads data */
      u32ReadFrameNumber = orxSoundSystem_Android_Read(&(pstSample->stData), pstSample->stData.stInfo.u32FrameNumber, pBuffer);

      /* Success? */
      if(u32ReadFrameNumber == pstSample->stData.stInfo.u32FrameNumber)
      {
        /* Transfers the data */
        alBufferData(uiBuffer, (pstSample->stData.stInfo.u32ChannelNumber > 1) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, pBuffer, (ALsizei)u32BufferSize, (ALsizei)pstSample->stData.stInfo.u32SampleRate);
        alASSERT();

        /* Stores duration */
        pstSample->fDuration = orxU2F(pstSample->stData.stInfo.u32FrameNumber) / orx2F(pstSample->stData.stInfo.u32SampleRate);

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
      else
      {
        /* Clears sample info */
        pstSample->stData.stInfo.u32FrameNumber = 0;
        pstSample->fDuration                    = orxFLOAT_0;

        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't load sound sample <%s>: can't read all data from file.", orxString_GetFromID(pstSample->stData.u32NameID));
      }

      /* Frees buffer */
      orxMemory_Free(pBuffer);
    }
    else
    {
      /* Clears sample info */
      pstSample->stData.stInfo.u32FrameNumber = 0;
      pstSample->fDuration                    = orxFLOAT_0;

      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't load sound sample <%s>: can't allocate memory for data.", orxString_GetFromID(pstSample->stData.u32NameID));
    }

    /* Closes file */
    orxSoundSystem_Android_CloseFile(&(pstSample->stData));
  }
  else
  {
    /* Clears sample info */
    pstSample->stData.stInfo.u32FrameNumber = 0;
    pstSample->fDuration                    = orxFLOAT_0;

    /* Closes resource */
    orxResource_Close(pstSample->stData.hResource);
    pstSample->stData.hResource = orxNULL;

    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't load sound sample <%s>: invalid data.", orxString_GetFromID(pstSample->stData.u32NameID));
  }

  /* Stores OpenAL buffer */
  orxMEMORY_BARRIER();
  pstSample->uiBuffer = uiBuffer;

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxSoundSystem_Android_LinkSampleTask(void *_pContext)
{
  orxSOUNDSYSTEM_SOUND *pstSound;
  orxSTATUS             eResult = orxSTATUS_SUCCESS;

  /* Gets sound */
  pstSound = (orxSOUNDSYSTEM_SOUND *)_pContext;

  /* Links buffer to source */
  alSourcei(pstSound->uiSource, AL_BUFFER, pstSound->pstSample->uiBuffer);
  alASSERT();

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxSoundSystem_Android_DeleteSampleTask(void *_pContext)
{
  orxSOUNDSYSTEM_SAMPLE  *pstSample;
  orxSTATUS               eResult = orxSTATUS_SUCCESS;

  /* Gets sample */
  pstSample = (orxSOUNDSYSTEM_SAMPLE *)_pContext;

  /* Deletes openAL buffer */
  alDeleteBuffers(1, (const ALuint *)&(pstSample->uiBuffer));
  alASSERT();

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxSoundSystem_Android_DeleteTask(void *_pContext)
{
  orxSOUNDSYSTEM_SOUND *pstSound;
  orxSTATUS             eResult = orxSTATUS_SUCCESS;

  /* Gets sound */
  pstSound = (orxSOUNDSYSTEM_SOUND *)_pContext;

  /* Deletes its source */
  alDeleteSources(1, &(pstSound->uiSource));
  alASSERT();

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxSoundSystem_Android_PlayTask(void *_pContext)
{
  orxSOUNDSYSTEM_SOUND *pstSound;
  orxSTATUS             eResult = orxSTATUS_SUCCESS;

  /* Gets sound */
  pstSound = (orxSOUNDSYSTEM_SOUND *)_pContext;

  /* Plays source */
  alSourcePlay(pstSound->uiSource);
  alASSERT();

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxSoundSystem_Android_PauseTask(void *_pContext)
{
  orxSOUNDSYSTEM_SOUND *pstSound;
  orxSTATUS             eResult = orxSTATUS_SUCCESS;

  /* Gets sound */
  pstSound = (orxSOUNDSYSTEM_SOUND *)_pContext;

  /* Pauses source */
  alSourcePause(pstSound->uiSource);
  alASSERT();

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxSoundSystem_Android_StopTask(void *_pContext)
{
  orxSOUNDSYSTEM_SOUND *pstSound;
  orxSTATUS             eResult = orxSTATUS_SUCCESS;

  /* Gets sound */
  pstSound = (orxSOUNDSYSTEM_SOUND *)_pContext;

  /* Stops source */
  alSourceStop(pstSound->uiSource);
  alASSERT();

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_Android_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Was already initialized? */
  if(!(sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY))
  {
#ifdef __orxDEBUG__
    putenv("ALSOFT_LOGLEVEL=3");
#endif

    /* Cleans static controller */
    orxMemory_Zero(&sstSoundSystem, sizeof(orxSOUNDSYSTEM_STATIC));

    /* Creates semaphore */
    sstSoundSystem.pstStreamSemaphore = orxThread_CreateSemaphore(1);

    /* Success? */
    if(sstSoundSystem.pstStreamSemaphore != orxNULL)
    {
      /* Cleans static callbacks */
      orxMemory_Zero(&sstOggVorbis_Callbacks, sizeof(ov_callbacks));

      sstOggVorbis_Callbacks.read_func = &read_func;
      sstOggVorbis_Callbacks.seek_func = &seek_func;
      sstOggVorbis_Callbacks.close_func = &close_func;
      sstOggVorbis_Callbacks.tell_func = &tell_func;

      /* Pushes config section */
      orxConfig_PushSection(orxSOUNDSYSTEM_KZ_CONFIG_SECTION);

      /* Opens device */
      sstSoundSystem.poDevice = alcOpenDevice(NULL);

      /* Valid? */
      if(sstSoundSystem.poDevice != NULL)
      {
        /* Creates associated context */
        sstSoundSystem.poContext = alcCreateContext(sstSoundSystem.poDevice, NULL);

        /* Has stream buffer size? */
        if(orxConfig_HasValue(orxSOUNDSYSTEM_KZ_CONFIG_STREAM_BUFFER_SIZE) != orxFALSE)
        {
          /* Stores it */
          sstSoundSystem.s32StreamBufferSize = orxConfig_GetU32(orxSOUNDSYSTEM_KZ_CONFIG_STREAM_BUFFER_SIZE) & 0xFFFFFFFC;
        }
        else
        {
          /* Uses default one */
          sstSoundSystem.s32StreamBufferSize = orxSOUNDSYSTEM_KS32_DEFAULT_STREAM_BUFFER_SIZE;
        }

        /* Has stream buffer number? */
        if(orxConfig_HasValue(orxSOUNDSYSTEM_KZ_CONFIG_STREAM_BUFFER_NUMBER) != orxFALSE)
        {
          /* Gets stream number */
          sstSoundSystem.s32StreamBufferNumber = orxMAX(2, orxConfig_GetU32(orxSOUNDSYSTEM_KZ_CONFIG_STREAM_BUFFER_NUMBER));
        }
        else
        {
          /* Uses default one */
          sstSoundSystem.s32StreamBufferNumber = orxSOUNDSYSTEM_KS32_DEFAULT_STREAM_BUFFER_NUMBER;
        }

        /* Valid? */
        if(sstSoundSystem.poContext != NULL)
        {
          /* Creates banks */
          sstSoundSystem.pstSampleBank  = orxBank_Create(orxSOUNDSYSTEM_KU32_BANK_SIZE, sizeof(orxSOUNDSYSTEM_SAMPLE), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
          sstSoundSystem.pstSoundBank   = orxBank_Create(orxSOUNDSYSTEM_KU32_BANK_SIZE, sizeof(orxSOUNDSYSTEM_SOUND) + sstSoundSystem.s32StreamBufferNumber * sizeof(ALuint), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

          /* Valid? */
          if((sstSoundSystem.pstSampleBank != orxNULL) && (sstSoundSystem.pstSoundBank))
          {
            /* Adds streaming thread */
            sstSoundSystem.u32StreamingThread = orxThread_Start(&orxSoundSystem_Android_UpdateStreaming, orxSOUNDSYSTEM_KZ_THREAD_NAME, orxNULL);

            /* Success? */
            if(sstSoundSystem.u32StreamingThread != orxU32_UNDEFINED)
            {
              ALfloat   afOrientation[] = {0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f};
              orxFLOAT  fRatio;

              /* Selects it */
              alcMakeContextCurrent(sstSoundSystem.poContext);

              /* Sets 2D listener target */
              alListenerfv(AL_ORIENTATION, afOrientation);
              alASSERT();

              /* Allocates stream buffers */
              sstSoundSystem.as16StreamBuffer     = (orxS16 *)orxMemory_Allocate(sstSoundSystem.s32StreamBufferSize * sizeof(orxS16), orxMEMORY_TYPE_AUDIO);

              /* Allocates working buffer list */
              sstSoundSystem.auiWorkBufferList    = (ALuint *)orxMemory_Allocate(sstSoundSystem.s32StreamBufferNumber * sizeof(ALuint), orxMEMORY_TYPE_AUDIO);

              /* Gets dimension ratio */
              fRatio = orxConfig_GetFloat(orxSOUNDSYSTEM_KZ_CONFIG_RATIO);

              /* Valid? */
              if(fRatio > orxFLOAT_0)
              {
                /* Stores it */
                sstSoundSystem.fDimensionRatio = fRatio;
              }
              else
              {
                /* Stores default one */
                sstSoundSystem.fDimensionRatio = orxSOUNDSYSTEM_KF_DEFAULT_DIMENSION_RATIO;
              }

              /* Stores reciprocal dimenstion ratio */
              sstSoundSystem.fRecDimensionRatio = orxFLOAT_1 / sstSoundSystem.fDimensionRatio;

              /* Updates status */
              orxFLAG_SET(sstSoundSystem.u32Flags, orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY, orxSOUNDSYSTEM_KU32_STATIC_MASK_ALL);

            alcDevicePauseSOFT = (LPALCDEVICEPAUSESOFT) alcGetProcAddress(sstSoundSystem.poDevice, "alcDevicePauseSOFT");
            alcDeviceResumeSOFT = (LPALCDEVICERESUMESOFT) alcGetProcAddress(sstSoundSystem.poDevice, "alcDeviceResumeSOFT");

            /* Add event handler */
            orxEvent_AddHandler(orxEVENT_TYPE_SYSTEM, orxSoundSystem_Android_EventHandler);

              /* Updates result */
              eResult = orxSTATUS_SUCCESS;
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

              /* Destroys openAL context */
              alcDestroyContext(sstSoundSystem.poContext);
              sstSoundSystem.poContext = NULL;

              /* Closes openAL device */
              alcCloseDevice(sstSoundSystem.poDevice);
              sstSoundSystem.poDevice = NULL;

              /* Deletes semaphore */
              orxThread_DeleteSemaphore(sstSoundSystem.pstStreamSemaphore);
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

            /* Destroys openAL context */
            alcDestroyContext(sstSoundSystem.poContext);
            sstSoundSystem.poContext = NULL;

            /* Closes openAL device */
            alcCloseDevice(sstSoundSystem.poDevice);
            sstSoundSystem.poDevice = NULL;

            /* Deletes semaphore */
            orxThread_DeleteSemaphore(sstSoundSystem.pstStreamSemaphore);
          }
        }
        else
        {
          /* Closes openAL device */
          alcCloseDevice(sstSoundSystem.poDevice);
          sstSoundSystem.poDevice = NULL;

          /* Deletes semaphore */
          orxThread_DeleteSemaphore(sstSoundSystem.pstStreamSemaphore);
        }
      }
      else
      {
        /* Deletes semaphore */
        orxThread_DeleteSemaphore(sstSoundSystem.pstStreamSemaphore);
      }

      /* Pops config section */
      orxConfig_PopSection();
    }
  }

  /* Done! */
  return eResult;
}

void orxFASTCALL orxSoundSystem_Android_Exit()
{
  /* Was initialized? */
  if(sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY)
  {
    /* Makes sure streaming thread is enabled */
    orxThread_Enable(orxTHREAD_GET_FLAG_FROM_ID(sstSoundSystem.u32StreamingThread), orxTHREAD_KU32_FLAG_NONE);

    /* Joins streaming thread */
    orxThread_Join(sstSoundSystem.u32StreamingThread);

    /* Stops any recording */
    orxSoundSystem_StopRecording();

    /* Waits for all tasks to be finished */
    while(orxThread_GetTaskCounter() != 0);

    /* Deletes semaphore */
    orxThread_DeleteSemaphore(sstSoundSystem.pstStreamSemaphore);

    /* Remove event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_SYSTEM, orxSoundSystem_Android_EventHandler);

    /* Deletes working buffer list */
    orxMemory_Free(sstSoundSystem.auiWorkBufferList);

    /* Deletes stream buffers */
    orxMemory_Free(sstSoundSystem.as16StreamBuffer);

    /* Deletes banks */
    orxBank_Delete(sstSoundSystem.pstSampleBank);
    orxBank_Delete(sstSoundSystem.pstSoundBank);

    /* Removes current context */
    alcMakeContextCurrent(NULL);

    /* Deletes context */
    alcDestroyContext(sstSoundSystem.poContext);

    /* Has capture device? */
    if(sstSoundSystem.poCaptureDevice != orxNULL)
    {
      /* Closes it */
      alcCaptureCloseDevice(sstSoundSystem.poCaptureDevice);
    }

    /* Closes device */
    alcCloseDevice(sstSoundSystem.poDevice);

    /* Cleans static controller */
    orxMemory_Zero(&sstSoundSystem, sizeof(orxSOUNDSYSTEM_STATIC));
  }

  /* Done! */
  return;
}

orxSOUNDSYSTEM_SAMPLE *orxFASTCALL orxSoundSystem_Android_CreateSample(orxU32 _u32ChannelNumber, orxU32 _u32FrameNumber, orxU32 _u32SampleRate)
{
  orxSOUNDSYSTEM_SAMPLE *pstResult = NULL;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  /* Valid parameters? */
  if((_u32ChannelNumber >= 1) && (_u32ChannelNumber <= 2) && (_u32FrameNumber > 0) && (_u32SampleRate > 0))
  {
    /* Allocates sample */
    pstResult = (orxSOUNDSYSTEM_SAMPLE *)orxBank_Allocate(sstSoundSystem.pstSampleBank);

    /* Valid? */
    if(pstResult != orxNULL)
    {
      orxU32  u32BufferSize;
      void   *pBuffer;

      /* Gets buffer size */
      u32BufferSize = _u32FrameNumber * _u32ChannelNumber * sizeof(orxS16);

      /* Allocates buffer */
      if((pBuffer = orxMemory_Allocate(u32BufferSize, orxMEMORY_TYPE_MAIN)) != orxNULL)
      {
        /* Clears it */
        orxMemory_Zero(pBuffer, u32BufferSize);

        /* Generates an OpenAL buffer */
        alGenBuffers(1, (ALuint *)&(pstResult->uiBuffer));
        alASSERT();

        /* Transfers the data */
        alBufferData(pstResult->uiBuffer, (_u32ChannelNumber > 1) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, pBuffer, (ALsizei)u32BufferSize, (ALsizei)_u32SampleRate);
        alASSERT();

        /* Stores info */
        pstResult->stData.stInfo.u32ChannelNumber  = _u32ChannelNumber;
        pstResult->stData.stInfo.u32FrameNumber    = _u32FrameNumber;
        pstResult->stData.stInfo.u32SampleRate     = _u32SampleRate;

        /* Stores duration */
        pstResult->fDuration = orxU2F(_u32FrameNumber) / orx2F(_u32SampleRate);

        /* Frees buffer */
        orxMemory_Free(pBuffer);
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
  }

  /* Done! */
  return pstResult;
}

orxSOUNDSYSTEM_SAMPLE *orxFASTCALL orxSoundSystem_Android_LoadSample(const orxSTRING _zFilename)
{
  orxSOUNDSYSTEM_SAMPLE *pstResult = NULL;

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
      orxHANDLE hResource;

      /* Opens it */
      hResource = orxResource_Open(zResourceLocation, orxFALSE);

      /* Success? */
      if(hResource != orxHANDLE_UNDEFINED)
      {
#ifdef __orxDEBUG__
        /* Stores name ID */
        pstResult->stData.u32NameID = orxString_GetID(_zFilename);
#endif /* __orxDEBUG__ */

        /* Stores resource */
        pstResult->stData.hResource = hResource;

        /* Runs sample load task */
        orxThread_RunTask(&orxSoundSystem_Android_LoadSampleTask, orxNULL, orxNULL, pstResult);
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't load sound sample <%s>: can't open resource [%s].", _zFilename, zResourceLocation);

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

orxSTATUS orxFASTCALL orxSoundSystem_Android_DeleteSample(orxSOUNDSYSTEM_SAMPLE *_pstSample)
{
  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSample != orxNULL);

  /* Runs delete sample task */
  orxThread_RunTask(&orxSoundSystem_Android_DeleteSampleTask, &orxSoundSystem_Android_FreeSample, &orxSoundSystem_Android_FreeSample, _pstSample);

  /* Done! */
  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL orxSoundSystem_Android_GetSampleInfo(const orxSOUNDSYSTEM_SAMPLE *_pstSample, orxU32 *_pu32ChannelNumber, orxU32 *_pu32FrameNumber, orxU32 *_pu32SampleRate)
{
  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSample != orxNULL);
  orxASSERT(_pu32ChannelNumber != orxNULL);
  orxASSERT(_pu32FrameNumber != orxNULL);
  orxASSERT(_pu32SampleRate != orxNULL);

  /* Updates info */
  *_pu32ChannelNumber = _pstSample->stData.stInfo.u32ChannelNumber;
  *_pu32FrameNumber   = _pstSample->stData.stInfo.u32FrameNumber;
  *_pu32SampleRate    = _pstSample->stData.stInfo.u32SampleRate;

  /* Done! */
  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL orxSoundSystem_Android_SetSampleData(orxSOUNDSYSTEM_SAMPLE *_pstSample, const orxS16 *_as16Data, orxU32 _u32SampleNumber)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSample != orxNULL);
  orxASSERT(_as16Data != orxNULL);

  /* Valid size? */
  if(_u32SampleNumber == _pstSample->stData.stInfo.u32ChannelNumber * _pstSample->stData.stInfo.u32FrameNumber)
  {
    /* Waits for pending load to complete */
    while(_pstSample->uiBuffer == 0);

    /* Transfers the data */
    alBufferData(_pstSample->uiBuffer, (_pstSample->stData.stInfo.u32ChannelNumber > 1) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, (const ALvoid *)_as16Data, (ALsizei)(_u32SampleNumber * sizeof(orxS16)), (ALsizei)_pstSample->stData.stInfo.u32SampleRate);
    alASSERT();

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

orxSOUNDSYSTEM_SOUND *orxFASTCALL orxSoundSystem_Android_CreateFromSample(const orxSOUNDSYSTEM_SAMPLE *_pstSample)
{
  orxSOUNDSYSTEM_SOUND *pstResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSample != orxNULL);

  /* Allocates sound */
  pstResult = (orxSOUNDSYSTEM_SOUND *)orxBank_Allocate(sstSoundSystem.pstSoundBank);

  /* Valid? */
  if(pstResult != orxNULL)
  {
    /* Links sample */
    pstResult->pstSample = (orxSOUNDSYSTEM_SAMPLE *)_pstSample;

    /* Updates duration */
    pstResult->fDuration = _pstSample->fDuration;

    /* Updates status */
    pstResult->bIsStream = orxFALSE;

    /* Creates source */
    alGenSources(1, &(pstResult->uiSource));
    alASSERT();

    /* Inits it */
    alSourcef(pstResult->uiSource, AL_PITCH, 1.0f);
    alASSERT();
    alSourcef(pstResult->uiSource, AL_GAIN, 1.0f);
    alASSERT();

    /* Not finished loading? */
    if(_pstSample->uiBuffer == 0)
    {
      /* Runs link task */
      orxThread_RunTask(&orxSoundSystem_Android_LinkSampleTask, orxNULL, orxNULL, pstResult);
    }
    else
    {
      /* Links it to data buffer */
      alSourcei(pstResult->uiSource, AL_BUFFER, _pstSample->uiBuffer);
      alASSERT();
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

orxSOUNDSYSTEM_SOUND *orxFASTCALL orxSoundSystem_Android_CreateStream(orxU32 _u32ChannelNumber, orxU32 _u32SampleRate, const orxSTRING _zReference)
{
  orxSOUNDSYSTEM_SOUND *pstResult = orxNULL;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_zReference != orxNULL);

  /* Valid parameters? */
  if((_u32ChannelNumber >= 1) && (_u32ChannelNumber <= 2) && (_u32SampleRate > 0))
  {
    /* Allocates sound */
    pstResult = (orxSOUNDSYSTEM_SOUND *)orxBank_Allocate(sstSoundSystem.pstSoundBank);

    /* Valid? */
    if(pstResult != orxNULL)
    {
      /* Clears it */
      orxMemory_Zero(pstResult, sizeof(orxSOUNDSYSTEM_SOUND));

      /* Generates openAL source */
      alGenSources(1, &(pstResult->uiSource));
      alASSERT();

      /* Generates all openAL buffers */
      alGenBuffers(sstSoundSystem.s32StreamBufferNumber, pstResult->auiBufferList);
      alASSERT();

      /* Stores information */
      pstResult->stData.stInfo.u32ChannelNumber = _u32ChannelNumber;
      pstResult->stData.stInfo.u32FrameNumber   = sstSoundSystem.s32StreamBufferSize / _u32ChannelNumber;
      pstResult->stData.stInfo.u32SampleRate    = _u32SampleRate;

      /* Stores duration */
      pstResult->fDuration = orx2F(-1.0f);

      /* Updates status */
      pstResult->bIsStream  = orxTRUE;
      pstResult->bStop      = orxTRUE;
      pstResult->zReference = _zReference;
      pstResult->s32PacketID= 0;

      /* Adds it to the list */
      orxThread_WaitSemaphore(sstSoundSystem.pstStreamSemaphore);
      orxLinkList_AddEnd(&(sstSoundSystem.stStreamList), &(pstResult->stNode));
      orxThread_SignalSemaphore(sstSoundSystem.pstStreamSemaphore);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't load create stream: can't allocate sound structure.");
    }
  }

  /* Done! */
  return pstResult;
}

orxSOUNDSYSTEM_SOUND *orxFASTCALL orxSoundSystem_Android_CreateStreamFromFile(const orxSTRING _zFilename, const orxSTRING _zReference)
{
  orxSOUNDSYSTEM_SOUND *pstResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_zFilename != orxNULL);

  /* Allocates sound */
  pstResult = (orxSOUNDSYSTEM_SOUND *)orxBank_Allocate(sstSoundSystem.pstSoundBank);

  /* Valid? */
  if(pstResult != orxNULL)
  {
    const orxSTRING zResourceLocation;

    /* Clears it */
    orxMemory_Zero(pstResult, sizeof(orxSOUNDSYSTEM_SOUND));

    /* Locates resource */
    zResourceLocation = orxResource_Locate(orxSOUND_KZ_RESOURCE_GROUP, _zFilename);

    /* Success? */
    if(zResourceLocation != orxNULL)
    {
      orxHANDLE hResource;

      /* Opens it */
      hResource = orxResource_Open(zResourceLocation, orxFALSE);

      /* Success? */
      if(hResource != orxHANDLE_UNDEFINED)
      {
#ifdef __orxDEBUG__
        /* Stores name ID */
        pstResult->stData.u32NameID = orxString_GetID(_zFilename);
#endif /* __orxDEBUG__ */

        /* Stores resource */
        pstResult->stData.hResource = hResource;

        /* Updates status */
        pstResult->bIsStream  = orxTRUE;
        pstResult->bStop      = orxTRUE;
        pstResult->zReference = _zReference;
        pstResult->s32PacketID= 0;

        /* Generates openAL source */
        alGenSources(1, &(pstResult->uiSource));
        alASSERT();

        /* Generates all openAL buffers */
        alGenBuffers(sstSoundSystem.s32StreamBufferNumber, pstResult->auiBufferList);
        alASSERT();

        /* Runs stream create task */
        orxThread_RunTask(&orxSoundSystem_Android_CreateStreamTask, orxNULL, orxNULL, pstResult);
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't load sound stream <%s>: can't open resource [%s].", _zFilename, zResourceLocation);

        /* Deletes sound */
        orxBank_Free(sstSoundSystem.pstSoundBank, pstResult);

        /* Updates result */
        pstResult = orxNULL;
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't load sound stream <%s>: can't locate resource.", _zFilename);

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
  }

  /* Done! */
  return pstResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_Android_Delete(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Stream? */
  if(_pstSound->bIsStream != orxFALSE)
  {
    /* Marks it for deletion */
    _pstSound->bDelete = orxTRUE;
  }
  else
  {
    /* Runs delete task */
    orxThread_RunTask(&orxSoundSystem_Android_DeleteTask, &orxSoundSystem_Android_FreeSound, &orxSoundSystem_Android_FreeSound, _pstSound);
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_Android_Play(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Is a stream? */
  if(_pstSound->bIsStream != orxFALSE)
  {
    /* Not already playing? */
    if(_pstSound->bStop != orxFALSE)
    {
      /* Updates status */
      _pstSound->bStop = orxFALSE;
    }

    /* Updates status */
    _pstSound->bPause = orxFALSE;
  }
  else
  {
    /* Not finished loading? */
    if(_pstSound->pstSample->uiBuffer == 0)
    {
      /* Runs play task */
      orxThread_RunTask(&orxSoundSystem_Android_PlayTask, orxNULL, orxNULL, _pstSound);
    }
    else
    {
      /* Plays source */
      alSourcePlay(_pstSound->uiSource);
      alASSERT();
    }
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_Android_Pause(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Is a stream? */
  if(_pstSound->bIsStream != orxFALSE)
  {
    /* Updates status */
    _pstSound->bPause = orxTRUE;
  }
  else
  {
    /* Not finished loading? */
    if(_pstSound->pstSample->uiBuffer == 0)
    {
      /* Runs pause task */
      orxThread_RunTask(&orxSoundSystem_Android_PauseTask, orxNULL, orxNULL, _pstSound);
    }
    else
    {
      /* Pauses source */
      alSourcePause(_pstSound->uiSource);
      alASSERT();
    }
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_Android_Stop(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Is a stream? */
  if(_pstSound->bIsStream != orxFALSE)
  {
    /* Updates status */
    _pstSound->bStop  = orxTRUE;
    _pstSound->bPause = orxFALSE;
  }
  else
  {
    /* Not finished loading? */
    if(_pstSound->pstSample->uiBuffer == 0)
    {
      /* Runs stop task */
      orxThread_RunTask(&orxSoundSystem_Android_StopTask, orxNULL, orxNULL, _pstSound);
    }
    else
    {
      /* Stops source */
      alSourceStop(_pstSound->uiSource);
      alASSERT();
    }
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_Android_StartRecording(const orxSTRING _zName, orxBOOL _bWriteToFile, orxU32 _u32SampleRate, orxU32 _u32ChannelNumber)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_zName != orxNULL);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_Android_StopRecording()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  /* Done! */
  return eResult;
}

orxBOOL orxFASTCALL orxSoundSystem_Android_HasRecordingSupport()
{
	const ALCchar  *zDeviceNameList;
  orxBOOL         bResult;

  /* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  /* Gets capture device name list */
  zDeviceNameList = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);

  /* Updates result */
  bResult = (zDeviceNameList[0] != orxCHAR_NULL) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_Android_SetVolume(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fVolume)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Sets source's gain */
  alSourcef(_pstSound->uiSource, AL_GAIN, _fVolume);
  alASSERT();

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_Android_SetPitch(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fPitch)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Sets source's pitch */
  alSourcef(_pstSound->uiSource, AL_PITCH, _fPitch);
  alASSERT();

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_Android_SetPosition(orxSOUNDSYSTEM_SOUND *_pstSound, const orxVECTOR *_pvPosition)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);
  orxASSERT(_pvPosition != orxNULL);

  /* Sets source position */
  alSource3f(_pstSound->uiSource, AL_POSITION, sstSoundSystem.fDimensionRatio * _pvPosition->fX, sstSoundSystem.fDimensionRatio * _pvPosition->fY, sstSoundSystem.fDimensionRatio * _pvPosition->fZ);
  alASSERT();

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_Android_SetAttenuation(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fAttenuation)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Set source's roll off factor */
  alSourcef(_pstSound->uiSource, AL_ROLLOFF_FACTOR, sstSoundSystem.fDimensionRatio * _fAttenuation);
  alASSERT();

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_Android_SetReferenceDistance(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fDistance)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Sets source's reference distance */
  alSourcef(_pstSound->uiSource, AL_REFERENCE_DISTANCE, sstSoundSystem.fDimensionRatio * _fDistance);
  alASSERT();

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_Android_Loop(orxSOUNDSYSTEM_SOUND *_pstSound, orxBOOL _bLoop)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Stream? */
  if(_pstSound->bIsStream != orxFALSE)
  {
    /* Updates status */
    _pstSound->bLoop = _bLoop;
  }
  else
  {
    /* Updates source */
    alSourcei(_pstSound->uiSource, AL_LOOPING, (_bLoop != orxFALSE) ? AL_TRUE : AL_FALSE);
    alASSERT();
  }

  /* Done! */
  return eResult;
}

orxFLOAT orxFASTCALL orxSoundSystem_Android_GetVolume(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxFLOAT fResult = orxFLOAT_0;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Updates result */
  alGetSourcef(_pstSound->uiSource, AL_GAIN, &fResult);
  alASSERT();

  /* Done! */
  return fResult;
}

orxFLOAT orxFASTCALL orxSoundSystem_Android_GetPitch(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxFLOAT fResult = orxFLOAT_0;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Updates result */
  alGetSourcef(_pstSound->uiSource, AL_PITCH, &fResult);
  alASSERT();

  /* Done! */
  return fResult;
}

orxVECTOR *orxFASTCALL orxSoundSystem_Android_GetPosition(const orxSOUNDSYSTEM_SOUND *_pstSound, orxVECTOR *_pvPosition)
{
  orxVECTOR *pvResult = _pvPosition;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);
  orxASSERT(_pvPosition != orxNULL);

  /* Gets source's position */
  alGetSource3f(_pstSound->uiSource, AL_POSITION, &(pvResult->fX), &(pvResult->fY), &(pvResult->fZ));
  alASSERT();

  /* Updates result */
  orxVector_Mulf(pvResult, pvResult, sstSoundSystem.fRecDimensionRatio);

  /* Done! */
  return pvResult;
}

orxFLOAT orxFASTCALL orxSoundSystem_Android_GetAttenuation(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Get source's roll off factor */
  alGetSourcef(_pstSound->uiSource, AL_ROLLOFF_FACTOR, &fResult);
  alASSERT();

  /* Updates result */
  fResult *= sstSoundSystem.fRecDimensionRatio;

  /* Done! */
  return fResult;
}

orxFLOAT orxFASTCALL orxSoundSystem_Android_GetReferenceDistance(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Gets source's reference distance */
  alGetSourcef(_pstSound->uiSource, AL_REFERENCE_DISTANCE, &fResult);
  alASSERT();

  /* Updates result */
  fResult *= sstSoundSystem.fRecDimensionRatio;

  /* Done! */
  return fResult;
}

orxBOOL orxFASTCALL orxSoundSystem_Android_IsLooping(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  ALint   iLooping;
  orxBOOL bResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Stream? */
  if(_pstSound->bIsStream != orxFALSE)
  {
    /* Updates result */
    bResult = _pstSound->bLoop;
  }
  else
  {
    /* Gets looping property */
    alGetSourcei(_pstSound->uiSource, AL_LOOPING, &iLooping);
    alASSERT();

    /* Updates result */
    bResult = (iLooping == AL_FALSE) ? orxFALSE : orxTRUE;
  }

  /* Done! */
  return bResult;
}

orxFLOAT orxFASTCALL orxSoundSystem_Android_GetDuration(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Updates result */
  fResult = _pstSound->fDuration;

  /* Done! */
  return fResult;
}

orxSOUNDSYSTEM_STATUS orxFASTCALL orxSoundSystem_Android_GetStatus(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  ALint                 iState;
  orxSOUNDSYSTEM_STATUS eResult = orxSOUNDSYSTEM_STATUS_NONE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Gets source's state */
  alGetSourcei(_pstSound->uiSource, AL_SOURCE_STATE, &iState);
  alASSERT();

  /* Depending on it */
  switch(iState)
  {
    case AL_STOPPED:
    {
      /* Is stream? */
      if(_pstSound->bIsStream != orxFALSE)
      {
        /* Updates result */
        eResult = (_pstSound->bStop != orxFALSE) ? orxSOUNDSYSTEM_STATUS_STOP : (_pstSound->bPause != orxFALSE) ? orxSOUNDSYSTEM_STATUS_PAUSE : orxSOUNDSYSTEM_STATUS_PLAY;
      }
      else
      {
        /* Updates result */
        eResult = orxSOUNDSYSTEM_STATUS_STOP;
      }

      break;
    }

    case AL_PAUSED:
    {
      /* Updates result */
      eResult = orxSOUNDSYSTEM_STATUS_PAUSE;

      break;
    }

    case AL_INITIAL:
    case AL_PLAYING:
    {
      /* Updates result */
      eResult = orxSOUNDSYSTEM_STATUS_PLAY;

      break;
    }

    default:
    {
      break;
    }
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_Android_SetGlobalVolume(orxFLOAT _fVolume)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  /* Sets listener's gain */
  alListenerf(AL_GAIN, _fVolume);
  alASSERT();

  /* Done! */
  return eResult;
}

orxFLOAT orxFASTCALL orxSoundSystem_Android_GetGlobalVolume()
{
  orxFLOAT fResult = orxFLOAT_0;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  /* Updates result */
  alGetListenerf(AL_GAIN, &fResult);
  alASSERT();

  /* Done! */
  return fResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_Android_SetListenerPosition(const orxVECTOR *_pvPosition)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvPosition != orxNULL);

  /* Sets listener's position */
  alListener3f(AL_POSITION, sstSoundSystem.fDimensionRatio * _pvPosition->fX, sstSoundSystem.fDimensionRatio * _pvPosition->fY, sstSoundSystem.fDimensionRatio * _pvPosition->fZ);
  alASSERT();

  /* Done! */
  return eResult;
}

orxVECTOR *orxFASTCALL orxSoundSystem_Android_GetListenerPosition(orxVECTOR *_pvPosition)
{
  orxVECTOR *pvResult = _pvPosition;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvPosition != orxNULL);

  /* Gets listener's position */
  alGetListener3f(AL_POSITION, &(pvResult->fX), &(pvResult->fY), &(pvResult->fZ));
  alASSERT();

  /* Updates result */
  orxVector_Mulf(pvResult, pvResult, sstSoundSystem.fRecDimensionRatio);

  /* Done! */
  return pvResult;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(SOUNDSYSTEM);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_Init, SOUNDSYSTEM, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_Exit, SOUNDSYSTEM, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_CreateSample, SOUNDSYSTEM, CREATE_SAMPLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_LoadSample, SOUNDSYSTEM, LOAD_SAMPLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_DeleteSample, SOUNDSYSTEM, DELETE_SAMPLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_GetSampleInfo, SOUNDSYSTEM, GET_SAMPLE_INFO);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_SetSampleData, SOUNDSYSTEM, SET_SAMPLE_DATA);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_CreateFromSample, SOUNDSYSTEM, CREATE_FROM_SAMPLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_CreateStream, SOUNDSYSTEM, CREATE_STREAM);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_CreateStreamFromFile, SOUNDSYSTEM, CREATE_STREAM_FROM_FILE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_Delete, SOUNDSYSTEM, DELETE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_Play, SOUNDSYSTEM, PLAY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_Pause, SOUNDSYSTEM, PAUSE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_Stop, SOUNDSYSTEM, STOP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_StartRecording, SOUNDSYSTEM, START_RECORDING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_StopRecording, SOUNDSYSTEM, STOP_RECORDING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_HasRecordingSupport, SOUNDSYSTEM, HAS_RECORDING_SUPPORT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_SetVolume, SOUNDSYSTEM, SET_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_SetPitch, SOUNDSYSTEM, SET_PITCH);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_SetPosition, SOUNDSYSTEM, SET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_SetAttenuation, SOUNDSYSTEM, SET_ATTENUATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_SetReferenceDistance, SOUNDSYSTEM, SET_REFERENCE_DISTANCE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_Loop, SOUNDSYSTEM, LOOP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_GetVolume, SOUNDSYSTEM, GET_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_GetPitch, SOUNDSYSTEM, GET_PITCH);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_GetPosition, SOUNDSYSTEM, GET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_GetAttenuation, SOUNDSYSTEM, GET_ATTENUATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_GetReferenceDistance, SOUNDSYSTEM, GET_REFERENCE_DISTANCE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_IsLooping, SOUNDSYSTEM, IS_LOOPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_GetDuration, SOUNDSYSTEM, GET_DURATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_GetStatus, SOUNDSYSTEM, GET_STATUS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_SetGlobalVolume, SOUNDSYSTEM, SET_GLOBAL_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_GetGlobalVolume, SOUNDSYSTEM, GET_GLOBAL_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_SetListenerPosition, SOUNDSYSTEM, SET_LISTENER_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Android_GetListenerPosition, SOUNDSYSTEM, GET_LISTENER_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_END();

#ifdef __orxMSVC__

  #pragma warning(default : 4996)

#endif /* __orxMSVC__ */
