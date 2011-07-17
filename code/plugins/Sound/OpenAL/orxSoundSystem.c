/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2011 Orx-Project
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

#if defined(__orxMAC__) || defined(__orxIPHONE__)

  #include <OpenAL/al.h>
  #include <OpenAL/alc.h>

#else /* __orxMAC__ || __orxIPHONE__ */

  #include <AL/al.h>
  #include <AL/alc.h>

#endif /* __orxMAC__ || __orxIPHONE__ */

#ifdef __orxMSVC__

  #pragma warning(disable : 4996)

#endif /* __orxMSVC__ */

#include "stb_vorbis.c"
#include "sndfile.h"


/** Module flags
 */
#define orxSOUNDSYSTEM_KU32_STATIC_FLAG_NONE      0x00000000 /**< No flags */

#define orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY     0x00000001 /**< Ready flag */
#define orxSOUNDSYSTEM_KU32_STATIC_FLAG_RECORDING 0x00000002 /**< Recording flag */

#define orxSOUNDSYSTEM_KU32_STATIC_MASK_ALL       0xFFFFFFFF /**< All mask */


/** Misc defines
 */
#define orxSOUNDSYSTEM_KU32_BANK_SIZE                   32
#define orxSOUNDSYSTEM_KU32_STREAM_BUFFER_NUMBER        4
#define orxSOUNDSYSTEM_KU32_DEFAULT_RECORDING_FREQUENCY 44100
#define orxSOUNDSYSTEM_KU32_STREAM_BUFFER_SIZE          8192
#define orxSOUNDSYSTEM_KU32_RECORDING_BUFFER_SIZE       orxSOUNDSYSTEM_KU32_STREAM_BUFFER_SIZE
#define orxSOUNDSYSTEM_KF_STREAM_TIMER_DELAY            orx2F(0.05f)
#define orxSOUNDSYSTEM_KF_DEFAULT_DIMENSION_RATIO       orx2F(0.01f)

#ifdef __orxDEBUG__

#define alASSERT()                                                      \
do                                                                      \
{                                                                       \
  ALenum eError = alGetError();                                         \
  orxASSERT(eError == AL_NO_ERROR && "OpenAL error code: 0x%X", eError);\
} while(orxFALSE)

#else /* __orxDEBUG__ */

#define alASSERT()                                                      \
do                                                                      \
{                                                                       \
  alGetError();                                                         \
} while(orxFALSE)

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
  orxBOOL             bVorbis;

  orxSOUNDSYSTEM_INFO stInfo;

  union
  {
    struct
    {
      stb_vorbis     *pstFile;
    } vorbis;

    struct
    {
      SNDFILE        *pstFile;
    } sndfile;
  };

} orxSOUNDSYSTEM_DATA;

/** Internal sample structure
 */
struct __orxSOUNDSYSTEM_SAMPLE_t
{
  ALuint              uiBuffer;
  orxFLOAT            fDuration;
  orxSOUNDSYSTEM_INFO stInfo;
};

/** Internal sound structure
 */
struct __orxSOUNDSYSTEM_SOUND_t
{
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
      orxBOOL                 bLoop;
      orxBOOL                 bStop;
      orxBOOL                 bPause;
      const orxSTRING         zReference;
      ALuint                  auiBufferList[orxSOUNDSYSTEM_KU32_STREAM_BUFFER_NUMBER];

      orxSOUNDSYSTEM_DATA     stData;
    };
  };

  orxBOOL   bIsStream;
  ALuint    uiSource;
  orxFLOAT  fDuration;
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
  orxU32                  u32Flags;           /**< Status flags */
  SNDFILE                *pstRecordingFile;   /**< Recording file */
  orxLINKLIST             stStreamList;       /**< Stream list */
  orxSOUND_EVENT_PAYLOAD  stRecordingPayload; /**< Recording payload */
  orxS16                  as16StreamBuffer[orxSOUNDSYSTEM_KU32_STREAM_BUFFER_SIZE]; /**< Stream buffer */
  orxS16                  as16RecordingBuffer[orxSOUNDSYSTEM_KU32_RECORDING_BUFFER_SIZE]; /**< Recording buffer */

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

static orxSTATUS orxFASTCALL orxSoundSystem_OpenAL_OpenRecordingFile()
{
  SF_INFO         stFileInfo;
  orxU32          u32Length;
  const orxCHAR  *zExtension;
  orxSTATUS       eResult;

  /* Clears file info */
  orxMemory_Zero(&stFileInfo, sizeof(SF_INFO));

  /* Gets file name's length */
  u32Length = orxString_GetLength(sstSoundSystem.stRecordingPayload.zSoundName);

  /* Gets extension */
  zExtension = (u32Length > 4) ? sstSoundSystem.stRecordingPayload.zSoundName + u32Length - 4 : orxSTRING_EMPTY;

  /* WAV? */
  if(orxString_ICompare(zExtension, ".wav") == 0)
  {
    /* Updates format */
    stFileInfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
  }
  /* CAF? */
  else if(orxString_ICompare(zExtension, ".caf") == 0)
  {
    /* Updates format */
    stFileInfo.format = SF_FORMAT_CAF | SF_FORMAT_PCM_16;
  }
  /* VOC? */
  else if(orxString_ICompare(zExtension, ".voc") == 0)
  {
    /* Updates format */
    stFileInfo.format = SF_FORMAT_VOC | SF_FORMAT_PCM_16;
  }
  /* AIFF? */
  else if(orxString_ICompare(zExtension, "aiff") == 0)
  {
    /* Updates format */
    stFileInfo.format = SF_FORMAT_AIFF | SF_FORMAT_PCM_16;
  }
  /* AU/SND? */
  else if((orxString_ICompare(zExtension + 1, ".au") == 0)
       || (orxString_ICompare(zExtension, ".snd") == 0))
  {
    /* Updates format */
    stFileInfo.format = SF_FORMAT_AU | SF_FORMAT_PCM_16;
  }
  /* IFF/SVX? */
  else if((orxString_ICompare(zExtension, ".iff") == 0)
       || (orxString_ICompare(zExtension, ".svx") == 0))
  {
    /* Updates format */
    stFileInfo.format = SF_FORMAT_SVX | SF_FORMAT_PCM_16;
  }
  /* RAW */
  else
  {
    /* Updates format */
    stFileInfo.format = SF_FORMAT_RAW | SF_FORMAT_PCM_16;
  }

  /* Inits it */
  stFileInfo.samplerate = sstSoundSystem.stRecordingPayload.stStream.stInfo.u32SampleRate;
  stFileInfo.channels   = (sstSoundSystem.stRecordingPayload.stStream.stInfo.u32ChannelNumber == 2) ? 2 : 1;

  /* Opens file */
  sstSoundSystem.pstRecordingFile = sf_open(sstSoundSystem.stRecordingPayload.zSoundName, SFM_WRITE, &stFileInfo);

  /* Success? */
  if(sstSoundSystem.pstRecordingFile)
  {
    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;

    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't open file <%s> to write recorded audio data. Sound recording is still in progress.", sstSoundSystem.stRecordingPayload.zSoundName);
  }

  /* Done! */
  return eResult;
}

static orxINLINE orxSTATUS orxSoundSystem_OpenAL_OpenFile(const orxSTRING _zFilename, orxSOUNDSYSTEM_DATA *_pstData)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(_zFilename != orxNULL);
  orxASSERT(_pstData != orxNULL);

  /* Opens file with vorbis */
  _pstData->vorbis.pstFile = stb_vorbis_open_filename((char *)_zFilename, NULL, NULL);

  /* Success? */
  if(_pstData->vorbis.pstFile != NULL)
  {
    stb_vorbis_info stFileInfo;

    /* Gets file info */
    stFileInfo  = stb_vorbis_get_info(_pstData->vorbis.pstFile);

    /* Stores info */
    _pstData->stInfo.u32ChannelNumber = (orxU32)stFileInfo.channels;
    _pstData->stInfo.u32FrameNumber   = (orxU32)stb_vorbis_stream_length_in_samples(_pstData->vorbis.pstFile);
    _pstData->stInfo.u32SampleRate    = (orxU32)stFileInfo.sample_rate;

    /* Updates status */
    _pstData->bVorbis                 = orxTRUE;

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    SF_INFO stFileInfo;

    /* Opens file with sndfile */
    _pstData->sndfile.pstFile = sf_open(_zFilename, SFM_READ, &stFileInfo);

    /* Success? */
    if(_pstData->sndfile.pstFile != NULL)
    {
      /* Stores info */
      _pstData->stInfo.u32ChannelNumber = (orxU32)stFileInfo.channels;
      _pstData->stInfo.u32FrameNumber   = (orxU32)stFileInfo.frames;
      _pstData->stInfo.u32SampleRate    = (orxU32)stFileInfo.samplerate;

      /* Updates status */
      _pstData->bVorbis                 = orxFALSE;

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }

  /* Done! */
  return eResult;
}

static orxINLINE void orxSoundSystem_OpenAL_CloseFile(orxSOUNDSYSTEM_DATA *_pstData)
{
  /* Checks */
  orxASSERT(_pstData != orxNULL);

  /* vorbis? */
  if(_pstData->bVorbis != orxFALSE)
  {
    /* Has valid file? */
    if(_pstData->vorbis.pstFile != orxNULL)
    {
      /* Closes file */
      stb_vorbis_close(_pstData->vorbis.pstFile);
      _pstData->vorbis.pstFile = orxNULL;
    }
  }
  /* sndfile */
  else
  {
    /* Has valid file? */
    if(_pstData->sndfile.pstFile != orxNULL)
    {
      /* Closes file */
      sf_close(_pstData->sndfile.pstFile);
      _pstData->sndfile.pstFile = orxNULL;
    }
  }

  /* Done! */
  return;
}

static orxINLINE orxU32 orxSoundSystem_OpenAL_Read(orxSOUNDSYSTEM_DATA *_pstData, orxU32 _u32FrameNumber, void *_pBuffer)
{
  orxU32 u32Result;

  /* Checks */
  orxASSERT(_pstData != orxNULL);

  /* vorbis? */
  if(_pstData->bVorbis != orxFALSE)
  {
    /* Has valid file? */
    if(_pstData->vorbis.pstFile != orxNULL)
    {
      /* Reads frames */
      u32Result = (orxU32)stb_vorbis_get_samples_short_interleaved(_pstData->vorbis.pstFile, _pstData->stInfo.u32ChannelNumber, (short *)_pBuffer, _u32FrameNumber * _pstData->stInfo.u32ChannelNumber);
    }
    else
    {
      /* Clears buffer */
      orxMemory_Zero(_pBuffer, _u32FrameNumber * _pstData->stInfo.u32ChannelNumber * sizeof(orxS16));

      /* Updates result */
      u32Result = _u32FrameNumber;
    }
  }
  /* sndfile */
  else
  {
    /* Has valid file? */
    if(_pstData->sndfile.pstFile != orxNULL)
    {
      /* Reads frames */
      u32Result = (orxU32)sf_read_short(_pstData->sndfile.pstFile, (short *)_pBuffer, _u32FrameNumber * _pstData->stInfo.u32ChannelNumber) / _pstData->stInfo.u32ChannelNumber;
    }
    else
    {
      /* Clears buffer */
      orxMemory_Zero(_pBuffer, _u32FrameNumber * _pstData->stInfo.u32ChannelNumber * sizeof(orxS16));

      /* Updates result */
      u32Result = _u32FrameNumber;
    }
  }

  /* Done! */
  return u32Result;
}

static orxINLINE void orxSoundSystem_OpenAL_Rewind(orxSOUNDSYSTEM_DATA *_pstData)
{
  /* Checks */
  orxASSERT(_pstData != orxNULL);

  /* vorbis? */
  if(_pstData->bVorbis != orxFALSE)
  {
    /* Has valid file? */
    if(_pstData->vorbis.pstFile != orxNULL)
    {
      /* Seeks start */
      stb_vorbis_seek_start(_pstData->vorbis.pstFile);
    }
  }
  /* sndfile */
  else
  {
    /* Has valid file? */
    if(_pstData->sndfile.pstFile != orxNULL)
    {
      /* Seeks start */
      sf_seek(_pstData->sndfile.pstFile, 0, SEEK_SET);
    }
  }

  /* Done! */
  return;
}

static void orxFASTCALL orxSoundSystem_OpenAL_FillStream(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  /* Checks */
  orxASSERT(_pstSound != orxNULL);

  /* Not stopped? */
  if(_pstSound->bStop == orxFALSE)
  {
    ALint   iBufferNumber = 0;
    ALuint *puiBufferList;
    ALuint  auiLocalBufferList[orxSOUNDSYSTEM_KU32_STREAM_BUFFER_NUMBER];

    /* Gets number of queued buffers */
    alGetSourcei(_pstSound->uiSource, AL_BUFFERS_QUEUED, &iBufferNumber);
    alASSERT();

    /* None found? */
    if(iBufferNumber == 0)
    {
      /* Uses initial buffer list */
      puiBufferList = _pstSound->auiBufferList;

      /* Updates buffer number */
      iBufferNumber = orxSOUNDSYSTEM_KU32_STREAM_BUFFER_NUMBER;
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
        puiBufferList = auiLocalBufferList;

        /* Unqueues them all */
        alSourceUnqueueBuffers(_pstSound->uiSource, orxMIN(iBufferNumber, orxSOUNDSYSTEM_KU32_STREAM_BUFFER_NUMBER), puiBufferList);
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
      u32BufferFrameNumber = orxSOUNDSYSTEM_KU32_STREAM_BUFFER_SIZE / _pstSound->stData.stInfo.u32ChannelNumber;

      /* For all processed buffers */
      for(i = 0, u32FrameNumber = u32BufferFrameNumber; i < (orxU32)iBufferNumber; i++)
      {
        orxBOOL bEOF = orxFALSE;

        /* Fills buffer */
        u32FrameNumber = orxSoundSystem_OpenAL_Read(&(_pstSound->stData), u32BufferFrameNumber, sstSoundSystem.as16StreamBuffer);

        /* Inits packet */
        stPayload.stStream.stPacket.u32SampleNumber = u32FrameNumber * _pstSound->stData.stInfo.u32ChannelNumber;
        stPayload.stStream.stPacket.as16SampleList  = sstSoundSystem.as16StreamBuffer;
        stPayload.stStream.stPacket.bDiscard        = orxFALSE;

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
            /* Updates status */
            bEOF = orxTRUE;
          }

          /* Ends of file? */
          if(bEOF != orxFALSE)
          {
            /* Rewinds file */
            orxSoundSystem_OpenAL_Rewind(&(_pstSound->stData));

            /* Not looping? */
            if(_pstSound->bLoop == orxFALSE)
            {
              /* Stops */
              _pstSound->bStop = orxTRUE;
              break;
            }
          }
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

      /* Stopped? */
      if((iState == AL_STOPPED) || (iState == AL_INITIAL))
      {
        /* Resumes play */
        alSourcePlay(_pstSound->uiSource);
        alASSERT();
      }
    }
  }
  else
  {
    ALint iState;

    /* Gets actual state */
    alGetSourcei(_pstSound->uiSource, AL_SOURCE_STATE, &iState);
    alASSERT();

    /* Stopped? */
    if((iState == AL_STOPPED) || (iState == AL_INITIAL))
    {
      ALint iQueuedBufferNumber = 0;

      /* Gets queued buffer number */
      alGetSourcei(_pstSound->uiSource, AL_BUFFERS_QUEUED, &iQueuedBufferNumber);
      alASSERT();

      /* Checks */
      orxASSERT(iQueuedBufferNumber <= orxSOUNDSYSTEM_KU32_STREAM_BUFFER_NUMBER);

      /* Found any? */
      if(iQueuedBufferNumber > 0)
      {
        ALuint auiDummy[orxSOUNDSYSTEM_KU32_STREAM_BUFFER_NUMBER];

        /* Unqueues them */
        alSourceUnqueueBuffers(_pstSound->uiSource, orxMIN(iQueuedBufferNumber, orxSOUNDSYSTEM_KU32_STREAM_BUFFER_NUMBER), auiDummy);
        alASSERT();
      }
    }
  }

  /* Done! */
  return;
}

static void orxFASTCALL orxSoundSystem_OpenAL_UpdateRecording()
{
  ALCint iSampleNumber;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(orxFLAG_TEST(sstSoundSystem.u32Flags, orxSOUNDSYSTEM_KU32_STATIC_FLAG_RECORDING));

  /* Gets the number of captured samples */
  alcGetIntegerv(sstSoundSystem.poCaptureDevice, ALC_CAPTURE_SAMPLES, 1, &iSampleNumber);

  /* For all packets */
  while(iSampleNumber > 0)
  {
    orxU32 u32PacketSampleNumber;

    /* Gets sample number for this packet */
    u32PacketSampleNumber = orxMIN((orxU32)iSampleNumber, orxSOUNDSYSTEM_KU32_RECORDING_BUFFER_SIZE);

    /* Inits packet */
    sstSoundSystem.stRecordingPayload.stStream.stPacket.u32SampleNumber  = u32PacketSampleNumber;
    sstSoundSystem.stRecordingPayload.stStream.stPacket.as16SampleList   = sstSoundSystem.as16RecordingBuffer;

    /* Gets the captured samples */
    alcCaptureSamples(sstSoundSystem.poCaptureDevice, (ALCvoid *)sstSoundSystem.as16RecordingBuffer, (ALCsizei)sstSoundSystem.stRecordingPayload.stStream.stPacket.u32SampleNumber);

    /* Sends event */
    orxEVENT_SEND(orxEVENT_TYPE_SOUND, orxSOUND_EVENT_RECORDING_PACKET, orxNULL, orxNULL, &(sstSoundSystem.stRecordingPayload));

    /* Should write the packet? */
    if(sstSoundSystem.stRecordingPayload.stStream.stPacket.bDiscard == orxFALSE)
    {
      /* No file opened yet? */
      if(sstSoundSystem.pstRecordingFile == orxNULL)
      {
        /* Opens it */
        orxSoundSystem_OpenAL_OpenRecordingFile();
      }

      /* Has a valid file opened? */
      if(sstSoundSystem.pstRecordingFile != orxNULL)
      {
        /* Writes data */
        sf_write_short(sstSoundSystem.pstRecordingFile, (const short *)sstSoundSystem.stRecordingPayload.stStream.stPacket.as16SampleList, sstSoundSystem.stRecordingPayload.stStream.stPacket.u32SampleNumber);
      } 
    }

    /* Updates remaining sample number */
    iSampleNumber -= (ALCint)u32PacketSampleNumber;

    /* Updates timestamp */
    sstSoundSystem.stRecordingPayload.stStream.stPacket.fTimeStamp += orxU2F(sstSoundSystem.stRecordingPayload.stStream.stPacket.u32SampleNumber) / orxU2F(sstSoundSystem.stRecordingPayload.stStream.stInfo.u32SampleRate * sstSoundSystem.stRecordingPayload.stStream.stInfo.u32ChannelNumber);
  }

  /* Updates packet's timestamp */
  sstSoundSystem.stRecordingPayload.stStream.stPacket.fTimeStamp = (orxFLOAT)orxSystem_GetTime();

  /* Done! */
  return;
}

static void orxFASTCALL orxSoundSystem_OpenAL_UpdateStreaming(const orxCLOCK_INFO *_pstInfo, void *_pContext)
{
  orxSOUNDSYSTEM_SOUND *pstSound;

  /* Is recording? */
  if(orxFLAG_TEST(sstSoundSystem.u32Flags, orxSOUNDSYSTEM_KU32_STATIC_FLAG_RECORDING))
  {
    /* Updates recording */
    orxSoundSystem_OpenAL_UpdateRecording();
  }

  /* For all streams */
  for(pstSound = (orxSOUNDSYSTEM_SOUND *)orxLinkList_GetFirst(&(sstSoundSystem.stStreamList));
      pstSound != orxNULL;
      pstSound = (orxSOUNDSYSTEM_SOUND *)orxLinkList_GetNext(&(pstSound->stNode)))
  {
    /* Fills its stream */
    orxSoundSystem_OpenAL_FillStream(pstSound);
  }
}

orxSTATUS orxFASTCALL orxSoundSystem_OpenAL_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Was already initialized? */
  if(!(sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY))
  {
    orxFLOAT fRatio;

    /* Cleans static controller */
    orxMemory_Zero(&sstSoundSystem, sizeof(orxSOUNDSYSTEM_STATIC));

    /* Opens device */
    sstSoundSystem.poDevice = alcOpenDevice(NULL);

    /* Valid? */
    if(sstSoundSystem.poDevice != NULL)
    {
      /* Creates associated context */
      sstSoundSystem.poContext = alcCreateContext(sstSoundSystem.poDevice, NULL);

      /* Valid? */
      if(sstSoundSystem.poContext != NULL)
      {
        /* Creates banks */
        sstSoundSystem.pstSampleBank  = orxBank_Create(orxSOUNDSYSTEM_KU32_BANK_SIZE, sizeof(orxSOUNDSYSTEM_SAMPLE), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
        sstSoundSystem.pstSoundBank   = orxBank_Create(orxSOUNDSYSTEM_KU32_BANK_SIZE, sizeof(orxSOUNDSYSTEM_SOUND), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

        /* Valid? */
        if((sstSoundSystem.pstSampleBank != orxNULL) && (sstSoundSystem.pstSoundBank))
        {
          /* Adds streaming timer */
          if(orxClock_AddGlobalTimer(orxSoundSystem_OpenAL_UpdateStreaming, orxSOUNDSYSTEM_KF_STREAM_TIMER_DELAY, -1, orxNULL) != orxSTATUS_FAILURE)
          {
            ALfloat afOrientation[] = {0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f};

            /* Selects it */
            alcMakeContextCurrent(sstSoundSystem.poContext);

            /* Sets 2D listener target */
            alListenerfv(AL_ORIENTATION, afOrientation);
            alASSERT();

            /* Gets dimension ratio */
            orxConfig_PushSection(orxSOUNDSYSTEM_KZ_CONFIG_SECTION);
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

            /* Pops config section */
            orxConfig_PopSection();

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
        }
      }
      else
      {
        /* Closes openAL device */
        alcCloseDevice(sstSoundSystem.poDevice);
        sstSoundSystem.poDevice = NULL;
      }
    }
  }
  
  /* Done! */
  return eResult;
}

void orxFASTCALL orxSoundSystem_OpenAL_Exit()
{
  /* Was initialized? */
  if(sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY)
  {
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

orxSOUNDSYSTEM_SAMPLE *orxFASTCALL orxSoundSystem_OpenAL_CreateSample(orxU32 _u32ChannelNumber, orxU32 _u32FrameNumber, orxU32 _u32SampleRate)
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
        alGenBuffers(1, &(pstResult->uiBuffer));
        alASSERT();

        /* Transfers the data */
        alBufferData(pstResult->uiBuffer, (_u32ChannelNumber > 1) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, pBuffer, (ALsizei)u32BufferSize, (ALsizei)_u32SampleRate);
        alASSERT();

        /* Stores info */
        pstResult->stInfo.u32ChannelNumber  = _u32ChannelNumber;
        pstResult->stInfo.u32FrameNumber    = _u32FrameNumber;
        pstResult->stInfo.u32SampleRate     = _u32SampleRate;

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

orxSOUNDSYSTEM_SAMPLE *orxFASTCALL orxSoundSystem_OpenAL_LoadSample(const orxSTRING _zFilename)
{
  orxSOUNDSYSTEM_DATA     stData;
  orxSOUNDSYSTEM_SAMPLE  *pstResult = NULL;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_zFilename != orxNULL);

  /* Opens file */
  if(orxSoundSystem_OpenAL_OpenFile(_zFilename, &stData) != orxSTATUS_FAILURE)
  {
    /* Allocates sample */
    pstResult = (orxSOUNDSYSTEM_SAMPLE *)orxBank_Allocate(sstSoundSystem.pstSampleBank);

    /* Valid? */
    if(pstResult != orxNULL)
    {
      orxU32  u32BufferSize;
      void   *pBuffer;

      /* Gets buffer size */
      u32BufferSize = stData.stInfo.u32FrameNumber * stData.stInfo.u32ChannelNumber * sizeof(orxS16);

      /* Allocates buffer */
      if((pBuffer = orxMemory_Allocate(u32BufferSize, orxMEMORY_TYPE_MAIN)) != orxNULL)
      {
        orxU32 u32ReadFrameNumber;

        /* Reads data */
        u32ReadFrameNumber = orxSoundSystem_OpenAL_Read(&stData, stData.stInfo.u32FrameNumber, pBuffer);

        /* Success? */
        if(u32ReadFrameNumber == stData.stInfo.u32FrameNumber)
        {
          /* Generates an OpenAL buffer */
          alGenBuffers(1, &(pstResult->uiBuffer));
          alASSERT();

          /* Transfers the data */
          alBufferData(pstResult->uiBuffer, (stData.stInfo.u32ChannelNumber > 1) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, pBuffer, (ALsizei)u32BufferSize, (ALsizei)stData.stInfo.u32SampleRate);
          alASSERT();

          /* Stores info */
          orxMemory_Copy(&(pstResult->stInfo), &(stData.stInfo), sizeof(orxSOUNDSYSTEM_INFO));

          /* Stores duration */
          pstResult->fDuration = orxU2F(stData.stInfo.u32FrameNumber) / orx2F(stData.stInfo.u32SampleRate);
        }
        else
        {
          /* Deletes sample */
          orxBank_Free(sstSoundSystem.pstSampleBank, pstResult);

          /* Updates result */
          pstResult = orxNULL;

          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't load sound sample <%s>: can't read data from file.", _zFilename);
        }

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
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't load sound sample <%s>: can't allocate memory for data.", _zFilename);
      }
    }

    /* Closes file */
    orxSoundSystem_OpenAL_CloseFile(&stData);
  }

  /* Done! */
  return pstResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_OpenAL_DeleteSample(orxSOUNDSYSTEM_SAMPLE *_pstSample)
{
  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSample != orxNULL);

  /* Deletes openAL buffer */
  alDeleteBuffers(1, &(_pstSample->uiBuffer));
  alASSERT();

  /* Deletes sample */
  orxBank_Free(sstSoundSystem.pstSampleBank, _pstSample);

  /* Done! */
  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL orxSoundSystem_OpenAL_GetSampleInfo(const orxSOUNDSYSTEM_SAMPLE *_pstSample, orxU32 *_pu32ChannelNumber, orxU32 *_pu32FrameNumber, orxU32 *_pu32SampleRate)
{
  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSample != orxNULL);
  orxASSERT(_pu32ChannelNumber != orxNULL);
  orxASSERT(_pu32FrameNumber != orxNULL);
  orxASSERT(_pu32SampleRate != orxNULL);

  /* Updates info */
  *_pu32ChannelNumber = _pstSample->stInfo.u32ChannelNumber;
  *_pu32FrameNumber   = _pstSample->stInfo.u32FrameNumber;
  *_pu32SampleRate    = _pstSample->stInfo.u32SampleRate;

  /* Done! */
  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL orxSoundSystem_OpenAL_SetSampleData(orxSOUNDSYSTEM_SAMPLE *_pstSample, const orxS16 *_as16Data, orxU32 _u32SampleNumber)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSample != orxNULL);
  orxASSERT(_as16Data != orxNULL);

  /* Valid size? */
  if(_u32SampleNumber == _pstSample->stInfo.u32ChannelNumber * _pstSample->stInfo.u32FrameNumber)
  {
    /* Transfers the data */
    alBufferData(_pstSample->uiBuffer, (_pstSample->stInfo.u32ChannelNumber > 1) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, (const ALvoid *)_as16Data, (ALsizei)(_u32SampleNumber * sizeof(orxS16)), (ALsizei)_pstSample->stInfo.u32SampleRate);
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

orxSOUNDSYSTEM_SOUND *orxFASTCALL orxSoundSystem_OpenAL_CreateFromSample(const orxSOUNDSYSTEM_SAMPLE *_pstSample)
{
  orxSOUNDSYSTEM_SOUND *pstResult = orxNULL;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSample != orxNULL);

  /* Allocates sound */
  pstResult = (orxSOUNDSYSTEM_SOUND *)orxBank_Allocate(sstSoundSystem.pstSoundBank);

  /* Valid? */
  if(pstResult != orxNULL)
  {
    /* Creates source */
    alGenSources(1, &(pstResult->uiSource));
    alASSERT();

    /* Inits it */
    alSourcei(pstResult->uiSource, AL_BUFFER, _pstSample->uiBuffer);
    alASSERT();
    alSourcef(pstResult->uiSource, AL_PITCH, 1.0f);
    alASSERT();
    alSourcef(pstResult->uiSource, AL_GAIN, 1.0f);
    alASSERT();

    /* Links sample */
    pstResult->pstSample = (orxSOUNDSYSTEM_SAMPLE *)_pstSample;

    /* Updates duration */
    pstResult->fDuration = _pstSample->fDuration;

    /* Updates status */
    pstResult->bIsStream = orxFALSE;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't allocate memory for creating sound.");
  }

  /* Done! */
  return pstResult;
}

orxSOUNDSYSTEM_SOUND *orxFASTCALL orxSoundSystem_OpenAL_CreateStream(orxU32 _u32ChannelNumber, orxU32 _u32SampleRate, const orxSTRING _zReference)
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
      alGenBuffers(orxSOUNDSYSTEM_KU32_STREAM_BUFFER_NUMBER, pstResult->auiBufferList);
      alASSERT();

      /* Stores information */
      pstResult->stData.stInfo.u32ChannelNumber = _u32ChannelNumber;
      pstResult->stData.stInfo.u32FrameNumber   = orxSOUNDSYSTEM_KU32_STREAM_BUFFER_SIZE / _u32ChannelNumber;
      pstResult->stData.stInfo.u32SampleRate    = _u32SampleRate;

      /* Stores duration */
      pstResult->fDuration = orx2F(-1.0f);

      /* Updates status */
      pstResult->bIsStream  = orxTRUE;
      pstResult->bStop      = orxTRUE;
      pstResult->bPause     = orxFALSE;
      pstResult->zReference = _zReference;

      /* Adds it to the list */
      orxLinkList_AddEnd(&(sstSoundSystem.stStreamList), &(pstResult->stNode));
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

orxSOUNDSYSTEM_SOUND *orxFASTCALL orxSoundSystem_OpenAL_CreateStreamFromFile(const orxSTRING _zFilename, const orxSTRING _zReference)
{
  orxSOUNDSYSTEM_SOUND *pstResult = orxNULL;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_zFilename != orxNULL);

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

    /* Opens file */
    if(orxSoundSystem_OpenAL_OpenFile(_zFilename, &(pstResult->stData)) != orxSTATUS_FAILURE)
    {
      /* Generates all openAL buffers */
      alGenBuffers(orxSOUNDSYSTEM_KU32_STREAM_BUFFER_NUMBER, pstResult->auiBufferList);
      alASSERT();

      /* Stores duration */
      pstResult->fDuration = orxU2F(pstResult->stData.stInfo.u32FrameNumber) / orx2F(pstResult->stData.stInfo.u32SampleRate);

      /* Updates status */
      pstResult->bIsStream  = orxTRUE;
      pstResult->bStop      = orxTRUE;
      pstResult->bPause     = orxFALSE;
      pstResult->zReference = _zReference;

      /* Adds it to the list */
      orxLinkList_AddEnd(&(sstSoundSystem.stStreamList), &(pstResult->stNode));
    }
    else
    {
      /* Deletes openAL source */
      alDeleteSources(1, &(pstResult->uiSource));
      alASSERT();

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

orxSTATUS orxFASTCALL orxSoundSystem_OpenAL_Delete(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Deletes source */
  alDeleteSources(1, &(_pstSound->uiSource));
  alASSERT();

  /* Stream? */
  if(_pstSound->bIsStream != orxFALSE)
  {
    /* Dispose audio file */
    orxSoundSystem_OpenAL_CloseFile(&(_pstSound->stData));

    /* Clears buffers */
    alDeleteBuffers(orxSOUNDSYSTEM_KU32_STREAM_BUFFER_NUMBER, _pstSound->auiBufferList);
    alASSERT();

    /* Removes it from list */
    orxLinkList_Remove(&(_pstSound->stNode));
  }

  /* Deletes sound */
  orxBank_Free(sstSoundSystem.pstSoundBank, _pstSound);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_OpenAL_Play(orxSOUNDSYSTEM_SOUND *_pstSound)
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

      /* Fills stream */
      orxSoundSystem_OpenAL_FillStream(_pstSound);
    }

    /* Updates status */
    _pstSound->bPause = orxFALSE;
  }

  /* Plays source */
  alSourcePlay(_pstSound->uiSource);
  alASSERT();

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_OpenAL_Pause(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Pauses source */
  alSourcePause(_pstSound->uiSource);
  alASSERT();

  /* Is a stream? */
  if(_pstSound->bIsStream != orxFALSE)
  {
    /* Updates status */
    _pstSound->bPause = orxTRUE;
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_OpenAL_Stop(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Stops source */
  alSourceStop(_pstSound->uiSource);
  alASSERT();

  /* Is a stream? */
  if(_pstSound->bIsStream != orxFALSE)
  {
    /* Rewinds file */
    orxSoundSystem_OpenAL_Rewind(&(_pstSound->stData));

    /* Updates status */
    _pstSound->bStop  = orxTRUE;
    _pstSound->bPause = orxFALSE;
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_OpenAL_StartRecording(const orxSTRING _zName, orxBOOL _bWriteToFile, orxU32 _u32SampleRate, orxU32 _u32ChannelNumber)
{
  ALCenum   eALFormat;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_zName != orxNULL);

  /* Not already recording? */
  if(!orxFLAG_TEST(sstSoundSystem.u32Flags, orxSOUNDSYSTEM_KU32_STATIC_FLAG_RECORDING))
  {
    /* Clears recording payload */
    orxMemory_Zero(&(sstSoundSystem.stRecordingPayload), sizeof(orxSOUND_EVENT_PAYLOAD));

    /* Stores recording name */
    sstSoundSystem.stRecordingPayload.zSoundName = orxString_Duplicate(_zName);

    /* Stores stream info */
    sstSoundSystem.stRecordingPayload.stStream.stInfo.u32SampleRate    = (_u32SampleRate > 0) ? _u32SampleRate : orxSOUNDSYSTEM_KU32_DEFAULT_RECORDING_FREQUENCY;
    sstSoundSystem.stRecordingPayload.stStream.stInfo.u32ChannelNumber = (_u32ChannelNumber == 2) ? 2 : 1;

    /* Stores discard status */
    sstSoundSystem.stRecordingPayload.stStream.stPacket.bDiscard = (_bWriteToFile != orxFALSE) ? orxFALSE : orxTRUE;

    /* Updates format based on the number of desired channels */
    eALFormat = (_u32ChannelNumber == 2) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;

    /* Opens the default capture device */
    sstSoundSystem.poCaptureDevice = alcCaptureOpenDevice(NULL, (ALCuint)sstSoundSystem.stRecordingPayload.stStream.stInfo.u32SampleRate, eALFormat, (ALCsizei)sstSoundSystem.stRecordingPayload.stStream.stInfo.u32SampleRate);

    /* Success? */
    if(sstSoundSystem.poCaptureDevice != NULL)
    {
      /* Should record? */
      if(_bWriteToFile != orxFALSE)
      {
        /* Opens file for recording */
        eResult = orxSoundSystem_OpenAL_OpenRecordingFile();
      }

      /* Success? */
      if(eResult != orxSTATUS_FAILURE)
      {
        /* Starts capture device */
        alcCaptureStart(sstSoundSystem.poCaptureDevice);

        /* Updates packet's timestamp */
        sstSoundSystem.stRecordingPayload.stStream.stPacket.fTimeStamp = (orxFLOAT)orxSystem_GetTime();

        /* Updates status */
        orxFLAG_SET(sstSoundSystem.u32Flags, orxSOUNDSYSTEM_KU32_STATIC_FLAG_RECORDING, orxSOUNDSYSTEM_KU32_STATIC_FLAG_NONE);

        /* Sends event */
        orxEVENT_SEND(orxEVENT_TYPE_SOUND, orxSOUND_EVENT_RECORDING_START, orxNULL, orxNULL, &(sstSoundSystem.stRecordingPayload));
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't start recording <%s>: failed to open file, aborting.", _zName);

        /* Deletes capture device */
        alcCaptureCloseDevice(sstSoundSystem.poCaptureDevice);
        sstSoundSystem.poCaptureDevice = orxNULL;
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't start recording of <%s>: failed to open sound capture device.", _zName);

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't start recording <%s> as the recording of <%s> is still in progress.", _zName, sstSoundSystem.stRecordingPayload.zSoundName);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_OpenAL_StopRecording()
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  /* Recording right now? */
  if(orxFLAG_TEST(sstSoundSystem.u32Flags, orxSOUNDSYSTEM_KU32_STATIC_FLAG_RECORDING))
  {
    /* Processes the remaining samples */
    orxSoundSystem_OpenAL_UpdateRecording();

    /* Has a recording file? */
    if(sstSoundSystem.pstRecordingFile != orxNULL)
    {
      /* Closes it */
      sf_close(sstSoundSystem.pstRecordingFile);
      sstSoundSystem.pstRecordingFile = orxNULL;
    }

    /* Reinits the packet */
    sstSoundSystem.stRecordingPayload.stStream.stPacket.u32SampleNumber  = 0;
    sstSoundSystem.stRecordingPayload.stStream.stPacket.as16SampleList   = sstSoundSystem.as16RecordingBuffer;

    /* Stops the capture device */
    alcCaptureStop(sstSoundSystem.poCaptureDevice);

    /* Closes it */
    alcCaptureCloseDevice(sstSoundSystem.poCaptureDevice);
    sstSoundSystem.poCaptureDevice = orxNULL;

    /* Updates status */
    orxFLAG_SET(sstSoundSystem.u32Flags, orxSOUNDSYSTEM_KU32_STATIC_FLAG_NONE, orxSOUNDSYSTEM_KU32_STATIC_FLAG_RECORDING);

    /* Sends event */
    orxEVENT_SEND(orxEVENT_TYPE_SOUND, orxSOUND_EVENT_RECORDING_STOP, orxNULL, orxNULL, &(sstSoundSystem.stRecordingPayload));

    /* Deletes name */
    orxString_Delete((orxSTRING)sstSoundSystem.stRecordingPayload.zSoundName);
    sstSoundSystem.stRecordingPayload.zSoundName = orxNULL;

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

orxBOOL orxFASTCALL orxSoundSystem_OpenAL_HasRecordingSupport()
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

orxSTATUS orxFASTCALL orxSoundSystem_OpenAL_SetVolume(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fVolume)
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

orxSTATUS orxFASTCALL orxSoundSystem_OpenAL_SetPitch(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fPitch)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Sets source's pitch */
  alSourcef(_pstSound->uiSource, AL_PITCH, _fPitch);
  alASSERT();

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_OpenAL_SetPosition(orxSOUNDSYSTEM_SOUND *_pstSound, const orxVECTOR *_pvPosition)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

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

orxSTATUS orxFASTCALL orxSoundSystem_OpenAL_SetAttenuation(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fAttenuation)
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

orxSTATUS orxFASTCALL orxSoundSystem_OpenAL_SetReferenceDistance(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fDistance)
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

orxSTATUS orxFASTCALL orxSoundSystem_OpenAL_Loop(orxSOUNDSYSTEM_SOUND *_pstSound, orxBOOL _bLoop)
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

orxFLOAT orxFASTCALL orxSoundSystem_OpenAL_GetVolume(const orxSOUNDSYSTEM_SOUND *_pstSound)
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

orxFLOAT orxFASTCALL orxSoundSystem_OpenAL_GetPitch(const orxSOUNDSYSTEM_SOUND *_pstSound)
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

orxVECTOR *orxFASTCALL orxSoundSystem_OpenAL_GetPosition(const orxSOUNDSYSTEM_SOUND *_pstSound, orxVECTOR *_pvPosition)
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

orxFLOAT orxFASTCALL orxSoundSystem_OpenAL_GetAttenuation(const orxSOUNDSYSTEM_SOUND *_pstSound)
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

orxFLOAT orxFASTCALL orxSoundSystem_OpenAL_GetReferenceDistance(const orxSOUNDSYSTEM_SOUND *_pstSound)
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

orxBOOL orxFASTCALL orxSoundSystem_OpenAL_IsLooping(const orxSOUNDSYSTEM_SOUND *_pstSound)
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

orxFLOAT orxFASTCALL orxSoundSystem_OpenAL_GetDuration(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxFLOAT fResult = orxFLOAT_0;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Updates result */
  fResult = _pstSound->fDuration;

  /* Done! */
  return fResult;
}

orxSOUNDSYSTEM_STATUS orxFASTCALL orxSoundSystem_OpenAL_GetStatus(const orxSOUNDSYSTEM_SOUND *_pstSound)
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
    case AL_INITIAL:
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

orxSTATUS orxFASTCALL orxSoundSystem_OpenAL_SetGlobalVolume(orxFLOAT _fVolume)
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

orxFLOAT orxFASTCALL orxSoundSystem_OpenAL_GetGlobalVolume()
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

orxSTATUS orxFASTCALL orxSoundSystem_OpenAL_SetListenerPosition(const orxVECTOR *_pvPosition)
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

orxVECTOR *orxFASTCALL orxSoundSystem_OpenAL_GetListenerPosition(orxVECTOR *_pvPosition)
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
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_Init, SOUNDSYSTEM, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_Exit, SOUNDSYSTEM, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_CreateSample, SOUNDSYSTEM, CREATE_SAMPLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_LoadSample, SOUNDSYSTEM, LOAD_SAMPLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_DeleteSample, SOUNDSYSTEM, DELETE_SAMPLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_GetSampleInfo, SOUNDSYSTEM, GET_SAMPLE_INFO);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_SetSampleData, SOUNDSYSTEM, SET_SAMPLE_DATA);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_CreateFromSample, SOUNDSYSTEM, CREATE_FROM_SAMPLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_CreateStream, SOUNDSYSTEM, CREATE_STREAM);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_CreateStreamFromFile, SOUNDSYSTEM, CREATE_STREAM_FROM_FILE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_Delete, SOUNDSYSTEM, DELETE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_Play, SOUNDSYSTEM, PLAY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_Pause, SOUNDSYSTEM, PAUSE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_Stop, SOUNDSYSTEM, STOP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_StartRecording, SOUNDSYSTEM, START_RECORDING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_StopRecording, SOUNDSYSTEM, STOP_RECORDING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_HasRecordingSupport, SOUNDSYSTEM, HAS_RECORDING_SUPPORT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_SetVolume, SOUNDSYSTEM, SET_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_SetPitch, SOUNDSYSTEM, SET_PITCH);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_SetPosition, SOUNDSYSTEM, SET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_SetAttenuation, SOUNDSYSTEM, SET_ATTENUATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_SetReferenceDistance, SOUNDSYSTEM, SET_REFERENCE_DISTANCE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_Loop, SOUNDSYSTEM, LOOP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_GetVolume, SOUNDSYSTEM, GET_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_GetPitch, SOUNDSYSTEM, GET_PITCH);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_GetPosition, SOUNDSYSTEM, GET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_GetAttenuation, SOUNDSYSTEM, GET_ATTENUATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_GetReferenceDistance, SOUNDSYSTEM, GET_REFERENCE_DISTANCE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_IsLooping, SOUNDSYSTEM, IS_LOOPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_GetDuration, SOUNDSYSTEM, GET_DURATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_GetStatus, SOUNDSYSTEM, GET_STATUS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_SetGlobalVolume, SOUNDSYSTEM, SET_GLOBAL_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_GetGlobalVolume, SOUNDSYSTEM, GET_GLOBAL_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_SetListenerPosition, SOUNDSYSTEM, SET_LISTENER_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_OpenAL_GetListenerPosition, SOUNDSYSTEM, GET_LISTENER_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_END();

#ifdef __orxMSVC__

  #pragma warning(default : 4996)

#endif /* __orxMSVC__ */
