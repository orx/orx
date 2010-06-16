/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2010 Orx-Project
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
 * SDL sound system plugin implementation
 *
 */


#include "orxPluginAPI.h"

#ifdef __orxWINDOWS__

  #include <AL/al.h>
  #include <AL/alc.h>

#else /* __orxWINDOWS__ */

  #include <OpenAL/al.h>
  #include <OpenAL/alc.h>

#endif /* __orxWINDOWS__ */

#ifdef __orxMSVC__

  #pragma warning(disable : 4996)

#endif /* __orxMSVC__ */

#include <stb_vorbis.c>


/** Module flags
 */
#define orxSOUNDSYSTEM_KU32_STATIC_FLAG_NONE      0x00000000 /**< No flags */

#define orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY     0x00000001 /**< Ready flag */

#define orxSOUNDSYSTEM_KU32_STATIC_MASK_ALL       0xFFFFFFFF /**< All mask */


/** Misc defines
 */
#define orxSOUNDSYSTEM_KU32_BANK_SIZE             32
#define orxSOUNDSYSTEM_KU32_STREAM_BUFFER_NUMBER  4
#define orxSOUNDSYSTEM_KU32_STREAM_BUFFER_SIZE    16384
#define orxSOUNDSYSTEM_KF_STREAM_TIMER_DELAY      orx2F(0.1f)
#define orxSOUNDSYSTEM_KF_DEFAULT_DIMENSION_RATIO orx2F(0.01f)

#ifdef __orxDEBUG__

#define alASSERT()                                                      \
do                                                                      \
{                                                                       \
  ALenum eError = alGetError();                                         \
  orxASSERT(eError == AL_NO_ERROR && "OpenAL error code: %ld", eError); \
} while(orxFALSE)

#else /* __orxDEBUG__ */

#define alASSERT()

#endif /* __orxDEBUG__ */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Internal sample structure
 */
struct __orxSOUNDSYSTEM_SAMPLE_t
{
  ALuint    uiBuffer;
  orxFLOAT  fDuration;
};

/** Internal sound structure
 */
struct __orxSOUNDSYSTEM_SOUND_t
{
  ALuint    uiSource;
  orxBOOL   bIsStream;
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
      ALuint                  auiBufferList[orxSOUNDSYSTEM_KU32_STREAM_BUFFER_NUMBER];
      orxBOOL                 bLoop;
      orxBOOL                 bStop;
      orxBOOL                 bPause;
      stb_vorbis             *pstFile;
      stb_vorbis_info         stFileInfo;
    };
  };
};

/** Static structure
 */
typedef struct __orxSOUNDSYSTEM_STATIC_t
{
  ALCdevice        *poDevice;           /**< OpenAL device */
  ALCcontext       *poContext;          /**< OpenAL context */
  orxBANK          *pstSampleBank;      /**< Sound bank */
  orxBANK          *pstSoundBank;       /**< Sound bank */
  orxFLOAT          fDimensionRatio;    /**< Dimension ratio */
  orxFLOAT          fRecDimensionRatio; /**< Reciprocal dimension ratio */
  orxU32            u32Flags;           /**< Status flags */
  orxU8             au8StreamBuffer[orxSOUNDSYSTEM_KU32_STREAM_BUFFER_SIZE]; /**< Stream buffer */

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

static void orxFASTCALL orxSoundSystem_SDL_FillStream(const orxCLOCK_INFO *_pstInfo, void *_pContext)
{
  orxSOUNDSYSTEM_SOUND *pstSound;

  /* Checks */
  orxASSERT(_pContext != orxNULL);

  /* Gets associated sound */
  pstSound = (orxSOUNDSYSTEM_SOUND *)_pContext;

  /* Not stopped? */
  if(pstSound->bStop == orxFALSE)
  {
    ALint   iBufferNumber = 0;
    ALuint *puiBufferList;
    ALuint  auiLocalBufferList[orxSOUNDSYSTEM_KU32_STREAM_BUFFER_NUMBER];

    /* Gets number of queued buffers */
    alGetSourcei(pstSound->uiSource, AL_BUFFERS_QUEUED, &iBufferNumber);
    alASSERT();

    /* Checks */
    orxASSERT((iBufferNumber == 0) || (iBufferNumber == orxSOUNDSYSTEM_KU32_STREAM_BUFFER_NUMBER));

    /* None found? */
    if(iBufferNumber == 0)
    {
      /* Uses initial buffer list */
      puiBufferList = pstSound->auiBufferList;

      /* Updates buffer number */
      iBufferNumber = orxSOUNDSYSTEM_KU32_STREAM_BUFFER_NUMBER;
    }
    else
    {
      /* Gets number of processed buffers */
      iBufferNumber = 0;
      alGetSourcei(pstSound->uiSource, AL_BUFFERS_PROCESSED, &iBufferNumber);
      alASSERT();

      /* Found any? */
      if(iBufferNumber > 0)
      {
        /* Uses local list */
        puiBufferList = auiLocalBufferList;

        /* Unqueues them all */
        alSourceUnqueueBuffers(pstSound->uiSource, orxMIN(iBufferNumber, orxSOUNDSYSTEM_KU32_STREAM_BUFFER_NUMBER), puiBufferList);
        alASSERT();
      }
    }

    /* Needs processing? */
    if(iBufferNumber > 0)
    {
      orxU32 u32BufferFrameNumber, u32FrameNumber, i;

      /* Gets buffer's frame number */
      u32BufferFrameNumber = (orxSOUNDSYSTEM_KU32_STREAM_BUFFER_SIZE / sizeof(orxU16)) / pstSound->stFileInfo.channels;

      /* For all processed buffers */
      for(i = 0, u32FrameNumber = u32BufferFrameNumber; i < (orxU32)iBufferNumber; i++)
      {
        orxBOOL bEOF = orxFALSE;

        /* Fills buffer */
        u32FrameNumber = (orxU32)stb_vorbis_get_samples_short_interleaved(pstSound->pstFile, pstSound->stFileInfo.channels, (short *)sstSoundSystem.au8StreamBuffer, u32BufferFrameNumber * pstSound->stFileInfo.channels);

        /* Success? */
        if(u32FrameNumber > 0)
        {
          /* Transfers its data */
          alBufferData(puiBufferList[i], (pstSound->stFileInfo.channels > 1) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, sstSoundSystem.au8StreamBuffer, (ALsizei)(u32FrameNumber * pstSound->stFileInfo.channels * sizeof(orxU16)), (ALsizei)pstSound->stFileInfo.sample_rate);
          alASSERT();

          /* Queues it */
          alSourceQueueBuffers(pstSound->uiSource, 1, &puiBufferList[i]);
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
          stb_vorbis_seek_start(pstSound->pstFile);

          /* Not looping? */
          if(pstSound->bLoop == orxFALSE)
          {
            /* Stops */
            pstSound->bStop = orxTRUE;
            break;
          }
        }
      }
    }

    /* Should continue */
    if(pstSound->bStop == orxFALSE)
    {
      ALint iState;

      /* Gets actual state */
      alGetSourcei(pstSound->uiSource, AL_SOURCE_STATE, &iState);
      alASSERT();

      /* Stopped? */
      if((iState == AL_STOPPED) || (iState == AL_INITIAL))
      {
        /* Resumes play */
        alSourcePlay(pstSound->uiSource);
        alASSERT();
      }
    }
  }
  else
  {
    ALint iState;

    /* Gets actual state */
    alGetSourcei(pstSound->uiSource, AL_SOURCE_STATE, &iState);
    alASSERT();

    /* Stopped? */
    if((iState == AL_STOPPED) || (iState == AL_INITIAL))
    {
      ALint iQueuedBufferNumber = 0;

      /* Gets queued buffer number */
      alGetSourcei(pstSound->uiSource, AL_BUFFERS_QUEUED, &iQueuedBufferNumber);
      alASSERT();

      /* Checks */
      orxASSERT(iQueuedBufferNumber <= orxSOUNDSYSTEM_KU32_STREAM_BUFFER_NUMBER);

      /* Found any? */
      if(iQueuedBufferNumber > 0)
      {
        ALuint auiDummy[orxSOUNDSYSTEM_KU32_STREAM_BUFFER_NUMBER];

        /* Unqueues them */
        alSourceUnqueueBuffers(pstSound->uiSource, orxMIN(iQueuedBufferNumber, orxSOUNDSYSTEM_KU32_STREAM_BUFFER_NUMBER), auiDummy);
        alASSERT();
      }
    }
  }
}

static orxINLINE orxSTATUS orxSoundSystem_SDL_OpenFile(const orxSTRING _zFilename, stb_vorbis **_ppstFile, stb_vorbis_info *_pstFileInfo, orxU32 *_pu32FrameNumber)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(_zFilename != orxNULL);
  orxASSERT(_ppstFile != orxNULL);
  orxASSERT(_pstFileInfo != orxNULL);
  orxASSERT(_pu32FrameNumber != orxNULL);

  /* Opens file */
  *_ppstFile = stb_vorbis_open_filename((char *)_zFilename, NULL, NULL);

  /* Success? */
  if(*_ppstFile != NULL)
  {
    /* Stores info */
    *_pstFileInfo     = stb_vorbis_get_info(*_ppstFile);
    *_pu32FrameNumber = stb_vorbis_stream_length_in_samples(*_ppstFile);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't load sound sample <%s>: can't find/load the file.", _zFilename);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_SDL_Init()
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
        /* Closes openAL device */
        alcCloseDevice(sstSoundSystem.poDevice);
        sstSoundSystem.poDevice = NULL;
      }
    }
  }
  
  /* Done! */
  return eResult;
}

void orxFASTCALL orxSoundSystem_SDL_Exit()
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

    /* Closes device */
    alcCloseDevice(sstSoundSystem.poDevice);

    /* Cleans static controller */
    orxMemory_Zero(&sstSoundSystem, sizeof(orxSOUNDSYSTEM_STATIC));
  }

  return;
}

orxSOUNDSYSTEM_SAMPLE *orxFASTCALL orxSoundSystem_SDL_LoadSample(const orxSTRING _zFilename)
{
  orxU32                  u32FrameNumber;
  stb_vorbis             *pstFile;
  stb_vorbis_info         stFileInfo;
  orxSOUNDSYSTEM_SAMPLE  *pstResult = NULL;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_zFilename != orxNULL);

  /* Opends file */
  if(orxSoundSystem_SDL_OpenFile(_zFilename, &pstFile, &stFileInfo, &u32FrameNumber) != orxSTATUS_FAILURE)
  {
    /* Allocates sample */
    pstResult = (orxSOUNDSYSTEM_SAMPLE *)orxBank_Allocate(sstSoundSystem.pstSampleBank);

    /* Valid? */
    if(pstResult != orxNULL)
    {
      orxU32  u32BufferSize;
      void   *pBuffer;

      /* Gets buffer size */
      u32BufferSize = u32FrameNumber * stFileInfo.channels * sizeof(orxU16);

      /* Allocates buffer */
      if((pBuffer = orxMemory_Allocate(u32BufferSize, orxMEMORY_TYPE_MAIN)) != orxNULL)
      {
        orxU32 u32Length;

        /* Reads data */
        u32Length = (orxU32)stb_vorbis_get_samples_short_interleaved(pstFile, stFileInfo.channels, (short *)pBuffer, u32FrameNumber * stFileInfo.channels);

        /* Success? */
        if(u32Length == u32FrameNumber)
        {
          /* Generates an OpenAL buffer */
          alGenBuffers(1, &(pstResult->uiBuffer));
          alASSERT();

          /* Transfers the data */
          alBufferData(pstResult->uiBuffer, (stFileInfo.channels > 1) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, pBuffer, (ALsizei)(u32FrameNumber * stFileInfo.channels * sizeof(orxU16)), (ALsizei)stFileInfo.sample_rate);
          alASSERT();

          /* Stores duration */
          pstResult->fDuration = orxU2F(u32FrameNumber) / orx2F(stFileInfo.sample_rate);
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
    stb_vorbis_close(pstFile);
  }

  /* Done! */
  return pstResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_SDL_UnloadSample(orxSOUNDSYSTEM_SAMPLE *_pstSample)
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

orxSOUNDSYSTEM_SOUND *orxFASTCALL orxSoundSystem_SDL_CreateFromSample(const orxSOUNDSYSTEM_SAMPLE *_pstSample)
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

orxSOUNDSYSTEM_SOUND *orxFASTCALL orxSoundSystem_SDL_CreateStreamFromFile(const orxSTRING _zFilename)
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
    orxU32 u32TotalFrameNumber;

    /* Clears it */
    orxMemory_Zero(pstResult, sizeof(orxSOUNDSYSTEM_SOUND));

    /* Generates openAL source */
    alGenSources(1, &(pstResult->uiSource));
    alASSERT();

    /* Opens file */
    if(orxSoundSystem_SDL_OpenFile(_zFilename, &(pstResult->pstFile), &(pstResult->stFileInfo), &u32TotalFrameNumber) != orxSTATUS_FAILURE)
    {
      /* Generates all openAL buffers */
      alGenBuffers(orxSOUNDSYSTEM_KU32_STREAM_BUFFER_NUMBER, pstResult->auiBufferList);
      alASSERT();

      /* Stores duration */
      pstResult->fDuration = orxU2F(u32TotalFrameNumber) / orx2F(pstResult->stFileInfo.sample_rate);

      /* Updates status */
      pstResult->bIsStream  = orxTRUE;
      pstResult->bStop      = orxTRUE;
      pstResult->bPause     = orxFALSE;

      /* Adds timer for this stream */
      orxClock_AddGlobalTimer(orxSoundSystem_SDL_FillStream, orxSOUNDSYSTEM_KF_STREAM_TIMER_DELAY, -1, pstResult);
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
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't load sound stream <%s>: can't sound structure.", _zFilename);
  }
  
  /* Done! */
  return pstResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_SDL_Delete(orxSOUNDSYSTEM_SOUND *_pstSound)
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
    stb_vorbis_close(_pstSound->pstFile);

    /* Clears buffers */
    alDeleteBuffers(orxSOUNDSYSTEM_KU32_STREAM_BUFFER_NUMBER, _pstSound->auiBufferList);
    alASSERT();

    /* Removes associated timer */
    orxClock_RemoveGlobalTimer(orxSoundSystem_SDL_FillStream, orxSOUNDSYSTEM_KF_STREAM_TIMER_DELAY, _pstSound);
  }

  /* Deletes sound */
  orxBank_Free(sstSoundSystem.pstSoundBank, _pstSound);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_SDL_Play(orxSOUNDSYSTEM_SOUND *_pstSound)
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
      orxSoundSystem_SDL_FillStream(orxNULL, _pstSound);
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

orxSTATUS orxFASTCALL orxSoundSystem_SDL_Pause(orxSOUNDSYSTEM_SOUND *_pstSound)
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

orxSTATUS orxFASTCALL orxSoundSystem_SDL_Stop(orxSOUNDSYSTEM_SOUND *_pstSound)
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
    stb_vorbis_seek_start(_pstSound->pstFile);

    /* Updates status */
    _pstSound->bStop  = orxTRUE;
    _pstSound->bPause = orxFALSE;
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_SDL_SetVolume(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fVolume)
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

orxSTATUS orxFASTCALL orxSoundSystem_SDL_SetPitch(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fPitch)
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

orxSTATUS orxFASTCALL orxSoundSystem_SDL_SetPosition(orxSOUNDSYSTEM_SOUND *_pstSound, const orxVECTOR *_pvPosition)
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

orxSTATUS orxFASTCALL orxSoundSystem_SDL_SetAttenuation(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fAttenuation)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Set source's roll off factor */
  alSourcef(_pstSound->uiSource, AL_ROLLOFF_FACTOR, sstSoundSystem.fRecDimensionRatio * _fAttenuation);
  alASSERT();

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_SDL_SetReferenceDistance(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fDistance)
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

orxSTATUS orxFASTCALL orxSoundSystem_SDL_Loop(orxSOUNDSYSTEM_SOUND *_pstSound, orxBOOL _bLoop)
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

orxFLOAT orxFASTCALL orxSoundSystem_SDL_GetVolume(const orxSOUNDSYSTEM_SOUND *_pstSound)
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

orxFLOAT orxFASTCALL orxSoundSystem_SDL_GetPitch(const orxSOUNDSYSTEM_SOUND *_pstSound)
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

orxVECTOR *orxFASTCALL orxSoundSystem_SDL_GetPosition(const orxSOUNDSYSTEM_SOUND *_pstSound, orxVECTOR *_pvPosition)
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

orxFLOAT orxFASTCALL orxSoundSystem_SDL_GetAttenuation(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Get source's roll off factor */
  alGetSourcef(_pstSound->uiSource, AL_ROLLOFF_FACTOR, &fResult);
  alASSERT();

  /* Updates result */
  fResult *= sstSoundSystem.fDimensionRatio;

  /* Done! */
  return fResult;
}

orxFLOAT orxFASTCALL orxSoundSystem_SDL_GetReferenceDistance(const orxSOUNDSYSTEM_SOUND *_pstSound)
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

orxBOOL orxFASTCALL orxSoundSystem_SDL_IsLooping(const orxSOUNDSYSTEM_SOUND *_pstSound)
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

orxFLOAT orxFASTCALL orxSoundSystem_SDL_GetDuration(const orxSOUNDSYSTEM_SOUND *_pstSound)
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

orxSOUNDSYSTEM_STATUS orxFASTCALL orxSoundSystem_SDL_GetStatus(const orxSOUNDSYSTEM_SOUND *_pstSound)
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

orxSTATUS orxFASTCALL orxSoundSystem_SDL_SetGlobalVolume(orxFLOAT _fVolume)
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

orxFLOAT orxFASTCALL orxSoundSystem_SDL_GetGlobalVolume()
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

orxSTATUS orxFASTCALL orxSoundSystem_SDL_SetListenerPosition(const orxVECTOR *_pvPosition)
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

orxVECTOR *orxFASTCALL orxSoundSystem_SDL_GetListenerPosition(orxVECTOR *_pvPosition)
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
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SDL_Init, SOUNDSYSTEM, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SDL_Exit, SOUNDSYSTEM, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SDL_LoadSample, SOUNDSYSTEM, LOAD_SAMPLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SDL_UnloadSample, SOUNDSYSTEM, UNLOAD_SAMPLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SDL_CreateFromSample, SOUNDSYSTEM, CREATE_FROM_SAMPLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SDL_CreateStreamFromFile, SOUNDSYSTEM, CREATE_STREAM_FROM_FILE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SDL_Delete, SOUNDSYSTEM, DELETE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SDL_Play, SOUNDSYSTEM, PLAY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SDL_Pause, SOUNDSYSTEM, PAUSE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SDL_Stop, SOUNDSYSTEM, STOP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SDL_SetVolume, SOUNDSYSTEM, SET_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SDL_SetPitch, SOUNDSYSTEM, SET_PITCH);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SDL_SetPosition, SOUNDSYSTEM, SET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SDL_SetAttenuation, SOUNDSYSTEM, SET_ATTENUATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SDL_SetReferenceDistance, SOUNDSYSTEM, SET_REFERENCE_DISTANCE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SDL_Loop, SOUNDSYSTEM, LOOP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SDL_GetVolume, SOUNDSYSTEM, GET_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SDL_GetPitch, SOUNDSYSTEM, GET_PITCH);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SDL_GetPosition, SOUNDSYSTEM, GET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SDL_GetAttenuation, SOUNDSYSTEM, GET_ATTENUATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SDL_GetReferenceDistance, SOUNDSYSTEM, GET_REFERENCE_DISTANCE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SDL_IsLooping, SOUNDSYSTEM, IS_LOOPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SDL_GetDuration, SOUNDSYSTEM, GET_DURATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SDL_GetStatus, SOUNDSYSTEM, GET_STATUS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SDL_SetGlobalVolume, SOUNDSYSTEM, SET_GLOBAL_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SDL_GetGlobalVolume, SOUNDSYSTEM, GET_GLOBAL_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SDL_SetListenerPosition, SOUNDSYSTEM, SET_LISTENER_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SDL_GetListenerPosition, SOUNDSYSTEM, GET_LISTENER_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_END();

#ifdef __orxMSVC__

  #pragma warning(default : 4996)

#endif /* __orxMSVC__ */
