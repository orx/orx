/* Orx - Portable Game Engine
 *
 * Orx is the legal property of its developers, whose names
 * are listed in the COPYRIGHT file distributed 
 * with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
#import <AudioToolbox/AudioToolbox.h>
#import <OpenAL/al.h>
#import <OpenAL/alc.h>


/** Module flags
 */
#define orxSOUNDSYSTEM_KU32_STATIC_FLAG_NONE      0x00000000 /**< No flags */

#define orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY     0x00000001 /**< Ready flag */

#define orxSOUNDSYSTEM_KU32_STATIC_MASK_ALL       0xFFFFFFFF /**< All mask */


/** Misc defines
 */
#define orxSOUNDSYSTEM_KU32_BANK_SIZE             32
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
  ALuint                  uiSource;
  orxSOUNDSYSTEM_SAMPLE  *pstSample;
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

static orxINLINE orxSTATUS orxSoundSystem_iPhone_OpenFile(const orxSTRING _zFilename, ExtAudioFileRef *_poFileRef, AudioStreamBasicDescription *_pstFileInfo, orxU32 *_pu32FrameNumber)
{
  NSString *poName;
  NSURL    *poURL;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(_zFilename != orxNULL);
  orxASSERT(_poFileRef != orxNULL);
  orxASSERT(_pstFileInfo != orxNULL);
  orxASSERT(_pu32FrameNumber != orxNULL);

  /* Gets NSString */
  poName = [NSString stringWithCString:_zFilename encoding:NSASCIIStringEncoding];

  /* Gets associated URL */
  poURL = [NSURL fileURLWithPath:poName];

  /* Opens file */
  if(ExtAudioFileOpenURL((CFURLRef)poURL, _poFileRef) == 0)
  {
    UInt32 u32InfoSize;

    /* Gets file info size  */
    u32InfoSize = sizeof(AudioStreamBasicDescription);

    /* Clears file info */
    orxMemory_Zero(_pstFileInfo, u32InfoSize);

    /* Gets file info */
    if(ExtAudioFileGetProperty(*_poFileRef, kExtAudioFileProperty_FileDataFormat, &u32InfoSize, _pstFileInfo) == 0)
    {
      /* Valid number of channels */
      if(_pstFileInfo->mChannelsPerFrame <= 2)
      {
        /* Updates file info for 16bit PCM data */
        _pstFileInfo->mFormatID         = kAudioFormatLinearPCM;
        _pstFileInfo->mBytesPerPacket   = 2 * _pstFileInfo->mChannelsPerFrame;
        _pstFileInfo->mFramesPerPacket  = 1;
        _pstFileInfo->mBytesPerFrame    = 2 * _pstFileInfo->mChannelsPerFrame;
        _pstFileInfo->mBitsPerChannel   = 16;
        _pstFileInfo->mFormatFlags      = kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked | kAudioFormatFlagIsSignedInteger;

        /* Applies it */
        if(ExtAudioFileSetProperty(*_poFileRef, kExtAudioFileProperty_ClientDataFormat, u32InfoSize, _pstFileInfo) == 0)
        {
          SInt64 s64FrameNumber;

          /* Gets frame number size */
          u32InfoSize = sizeof(SInt64);

          /* Get the frame number */
          if(ExtAudioFileGetProperty(*_poFileRef, kExtAudioFileProperty_FileLengthFrames, &u32InfoSize, &s64FrameNumber) == 0)
          {
            /* Updates frame number */
            *_pu32FrameNumber = s64FrameNumber;

            /* Updates result */
            eResult = orxSTATUS_SUCCESS;
          }
          else
          {
            /* Outputs log */
            orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't load sound sample <%s>: can't get file size.", _zFilename);
          }
        }
        else
        {
          /* Outputs log */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't load sound sample <%s>: can't convert to 16bit PCM.", _zFilename);
        }
      }
      else
      {
        /* Outputs log */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't load sound sample <%s>: too many channels.", _zFilename);
      }
    }
    else
    {
      /* Outputs log */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't load sound sample <%s>: invalid format.", _zFilename);
    }
  }
  else
  {
    /* Outputs log */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't load sound sample <%s>: can't find/load the file.", _zFilename);
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_iPhone_Init()
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
    alASSERT();

    /* Valid? */
    if(sstSoundSystem.poDevice != NULL)
    {
      /* Creates associated context */
      sstSoundSystem.poContext = alcCreateContext(sstSoundSystem.poDevice, NULL);
      alASSERT();

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
          alASSERT();

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

void orxFASTCALL orxSoundSystem_iPhone_Exit()
{
  /* Was initialized? */
  if(sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY)
  {
    /* Deletes banks */
    orxBank_Delete(sstSoundSystem.pstSampleBank);
    orxBank_Delete(sstSoundSystem.pstSoundBank);

    /* Deletes context */
    alcDestroyContext(sstSoundSystem.poContext);
    alASSERT();

    /* Closes device */
    alcCloseDevice(sstSoundSystem.poDevice);
    alASSERT();

    /* Cleans static controller */
    orxMemory_Zero(&sstSoundSystem, sizeof(orxSOUNDSYSTEM_STATIC));
  }

  return;
}

orxSOUNDSYSTEM_SAMPLE *orxFASTCALL orxSoundSystem_iPhone_LoadSample(const orxSTRING _zFilename)
{
  AudioStreamBasicDescription stFileInfo;
  ExtAudioFileRef             oFileRef;
  orxU32                      u32FrameNumber;
  orxSOUNDSYSTEM_SAMPLE      *pstResult = NULL;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_zFilename != orxNULL);

  /* Opends file */
  if(orxSoundSystem_iPhone_OpenFile(_zFilename, &oFileRef, &stFileInfo, &u32FrameNumber) != orxSTATUS_FAILURE)
  {
    /* Allocates sample */
    pstResult = (orxSOUNDSYSTEM_SAMPLE *)orxBank_Allocate(sstSoundSystem.pstSampleBank);

    /* Valid? */
    if(pstResult != orxNULL)
    {
      UInt32  u32BufferSize;
      void   *pBuffer;

      /* Gets buffer size */
      u32BufferSize = u32FrameNumber * stFileInfo.mBytesPerFrame;

      /* Allocates buffer */
      if((pBuffer = orxMemory_Allocate(u32BufferSize, orxMEMORY_TYPE_MAIN)) != orxNULL)
      {
        AudioBufferList stBufferInfo;

        /* Inits buffer info */
        stBufferInfo.mNumberBuffers               = 1;
        stBufferInfo.mBuffers[0].mDataByteSize    = u32BufferSize;
        stBufferInfo.mBuffers[0].mNumberChannels  = stFileInfo.mChannelsPerFrame;
        stBufferInfo.mBuffers[0].mData            = pBuffer;

        /* Reads data */
        if(ExtAudioFileRead(oFileRef, (UInt32 *)&u32FrameNumber, &stBufferInfo) == 0)
        {
          /* Generates an OpenAL buffer */
          alGenBuffers(1, &(pstResult->uiBuffer));
          alASSERT();

          /* Transfers the data */
          alBufferData(pstResult->uiBuffer, (stFileInfo.mChannelsPerFrame > 1) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, pBuffer, (ALsizei)u32BufferSize, (ALsizei)stFileInfo.mSampleRate);
          alASSERT();

          /* Stores duration */
          pstResult->fDuration = orxU2F(u32BufferSize) / orx2F(stFileInfo.mSampleRate);
        }
        else
        {
          /* Deletes sample */
          orxBank_Free(sstSoundSystem.pstSampleBank, pstResult);

          /* Updates result */
          pstResult = orxNULL;

          /* Outputs log */
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

        /* Outputs log */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't load sound sample <%s>: can't allocate memory for data.", _zFilename);
      }
    }

    /* Closes file */
    ExtAudioFileDispose(oFileRef);
  }

  /* Done! */
  return pstResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_iPhone_UnloadSample(orxSOUNDSYSTEM_SAMPLE *_pstSample)
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

orxSOUNDSYSTEM_SOUND *orxFASTCALL orxSoundSystem_iPhone_CreateFromSample(const orxSOUNDSYSTEM_SAMPLE *_pstSample)
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
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't allocate memory for creating sound.");
  }

  /* Done! */
  return pstResult;
}

orxSOUNDSYSTEM_SOUND *orxFASTCALL orxSoundSystem_iPhone_CreateStreamFromFile(const orxSTRING _zFilename)
{
  orxSOUNDSYSTEM_SOUND *pstResult = orxNULL;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_zFilename != orxNULL);

//! TODO

  /* Done! */
  return pstResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_iPhone_Delete(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Deletes source */
  alDeleteSources(1, &(_pstSound->uiSource));
  alASSERT();

  /* Deletes sound */
  orxBank_Free(sstSoundSystem.pstSoundBank, _pstSound);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_iPhone_Play(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Plays source */
  alSourcePlay(_pstSound->uiSource);
  alASSERT();

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_iPhone_Pause(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Pauses source */
  alSourcePause(_pstSound->uiSource);
  alASSERT();

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_iPhone_Stop(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Stops source */
  alSourceStop(_pstSound->uiSource);
  alASSERT();

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_iPhone_SetVolume(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fVolume)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Sets source's gain */
  alSourcef(_pstSound->uiSource, AL_GAIN, _fVolume);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_iPhone_SetPitch(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fPitch)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Sets source's pitch */
  alSourcef(_pstSound->uiSource, AL_PITCH, _fPitch);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_iPhone_SetPosition(orxSOUNDSYSTEM_SOUND *_pstSound, const orxVECTOR *_pvPosition)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);
  orxASSERT(_pvPosition != orxNULL);

  /* Sets source position */
  alSource3f(_pstSound->uiSource, AL_POSITION, sstSoundSystem.fDimensionRatio * _pvPosition->fX, sstSoundSystem.fDimensionRatio * _pvPosition->fY, sstSoundSystem.fDimensionRatio * _pvPosition->fZ);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_iPhone_SetAttenuation(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fAttenuation)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Set source's roll off factor */
  alSourcef(_pstSound->uiSource, AL_ROLLOFF_FACTOR, sstSoundSystem.fDimensionRatio * _fAttenuation);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_iPhone_SetReferenceDistance(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fDistance)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Sets source's reference distance */
  alSourcef(_pstSound->uiSource, AL_REFERENCE_DISTANCE, sstSoundSystem.fDimensionRatio * _fDistance);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_iPhone_Loop(orxSOUNDSYSTEM_SOUND *_pstSound, orxBOOL _bLoop)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Updates source */
  alSourcei(_pstSound->uiSource, AL_LOOPING, (_bLoop != orxFALSE) ? AL_TRUE : AL_FALSE);

  /* Done! */
  return eResult;
}

orxFLOAT orxFASTCALL orxSoundSystem_iPhone_GetVolume(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxFLOAT fResult = orxFLOAT_0;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Updates result */
  alGetSourcef(_pstSound->uiSource, AL_GAIN, &fResult);

  /* Done! */
  return fResult;
}

orxFLOAT orxFASTCALL orxSoundSystem_iPhone_GetPitch(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxFLOAT fResult = orxFLOAT_0;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Updates result */
  alGetSourcef(_pstSound->uiSource, AL_PITCH, &fResult);

  /* Done! */
  return fResult;
}

orxVECTOR *orxFASTCALL orxSoundSystem_iPhone_GetPosition(const orxSOUNDSYSTEM_SOUND *_pstSound, orxVECTOR *_pvPosition)
{
  orxVECTOR *pvResult = _pvPosition;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);
  orxASSERT(_pvPosition != orxNULL);

  /* Gets source's position */
  alGetSource3f(_pstSound->uiSource, AL_POSITION, &(pvResult->fX), &(pvResult->fY), &(pvResult->fZ));

  /* Updates result */
  orxVector_Mulf(pvResult, pvResult, sstSoundSystem.fRecDimensionRatio);

  /* Done! */
  return pvResult;
}

orxFLOAT orxFASTCALL orxSoundSystem_iPhone_GetAttenuation(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Get source's roll off factor */
  alGetSourcef(_pstSound->uiSource, AL_ROLLOFF_FACTOR, &fResult);

  /* Updates result */
  fResult *= sstSoundSystem.fRecDimensionRatio;

  /* Done! */
  return fResult;
}

orxFLOAT orxFASTCALL orxSoundSystem_iPhone_GetReferenceDistance(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Gets source's reference distance */
  alGetSourcef(_pstSound->uiSource, AL_REFERENCE_DISTANCE, &fResult);

  /* Updates result */
  fResult *= sstSoundSystem.fRecDimensionRatio;

  /* Done! */
  return fResult;
}

orxBOOL orxFASTCALL orxSoundSystem_iPhone_IsLooping(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  ALint   iLooping;
  orxBOOL bResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Gets looping property */
  alGetSourcei(_pstSound->uiSource, AL_LOOPING, &iLooping);

  /* Updates result */
  bResult = (iLooping == AL_FALSE) ? orxFALSE : orxTRUE;

  /* Done! */
  return bResult;
}

orxFLOAT orxFASTCALL orxSoundSystem_iPhone_GetDuration(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxFLOAT fResult = orxFLOAT_0;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Updates result */
  fResult = _pstSound->pstSample->fDuration;

  /* Done! */
  return fResult;
}

orxSOUNDSYSTEM_STATUS orxFASTCALL orxSoundSystem_iPhone_GetStatus(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  ALint                 iState;
  orxSOUNDSYSTEM_STATUS eResult = orxSOUNDSYSTEM_STATUS_NONE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Gets source's state */
  alGetSourcei(_pstSound->uiSource, AL_SOURCE_STATE, &iState);

  /* Depending on it */
  switch(iState)
  {
    case AL_INITIAL:
    case AL_STOPPED:
    {
      /* Updates result */
      eResult = orxSOUNDSYSTEM_STATUS_STOP;

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

orxSTATUS orxFASTCALL orxSoundSystem_iPhone_SetGlobalVolume(orxFLOAT _fVolume)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  /* Sets listener's gain */
  alListenerf(AL_GAIN, _fVolume);

  /* Done! */
  return eResult;
}

orxFLOAT orxFASTCALL orxSoundSystem_iPhone_GetGlobalVolume()
{
  orxFLOAT fResult = orxFLOAT_0;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  /* Updates result */
  alGetListenerf(AL_GAIN, &fResult);

  /* Done! */
  return fResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_iPhone_SetListenerPosition(const orxVECTOR *_pvPosition)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvPosition != orxNULL);

  /* Sets listener's position */
  alListener3f(AL_POSITION, sstSoundSystem.fDimensionRatio * _pvPosition->fX, sstSoundSystem.fDimensionRatio * _pvPosition->fY, sstSoundSystem.fDimensionRatio * _pvPosition->fZ);

  /* Done! */
  return eResult;
}

orxVECTOR *orxFASTCALL orxSoundSystem_iPhone_GetListenerPosition(orxVECTOR *_pvPosition)
{
  orxVECTOR *pvResult = _pvPosition;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvPosition != orxNULL);

  /* Gets listener's position */
  alGetListener3f(AL_POSITION, &(pvResult->fX), &(pvResult->fY), &(pvResult->fZ));

  /* Updates result */
  orxVector_Mulf(pvResult, pvResult, sstSoundSystem.fRecDimensionRatio);

  /* Done! */
  return pvResult;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(SOUNDSYSTEM);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_iPhone_Init, SOUNDSYSTEM, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_iPhone_Exit, SOUNDSYSTEM, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_iPhone_LoadSample, SOUNDSYSTEM, LOAD_SAMPLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_iPhone_UnloadSample, SOUNDSYSTEM, UNLOAD_SAMPLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_iPhone_CreateFromSample, SOUNDSYSTEM, CREATE_FROM_SAMPLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_iPhone_CreateStreamFromFile, SOUNDSYSTEM, CREATE_STREAM_FROM_FILE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_iPhone_Delete, SOUNDSYSTEM, DELETE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_iPhone_Play, SOUNDSYSTEM, PLAY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_iPhone_Pause, SOUNDSYSTEM, PAUSE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_iPhone_Stop, SOUNDSYSTEM, STOP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_iPhone_SetVolume, SOUNDSYSTEM, SET_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_iPhone_SetPitch, SOUNDSYSTEM, SET_PITCH);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_iPhone_SetPosition, SOUNDSYSTEM, SET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_iPhone_SetAttenuation, SOUNDSYSTEM, SET_ATTENUATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_iPhone_SetReferenceDistance, SOUNDSYSTEM, SET_REFERENCE_DISTANCE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_iPhone_Loop, SOUNDSYSTEM, LOOP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_iPhone_GetVolume, SOUNDSYSTEM, GET_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_iPhone_GetPitch, SOUNDSYSTEM, GET_PITCH);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_iPhone_GetPosition, SOUNDSYSTEM, GET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_iPhone_GetAttenuation, SOUNDSYSTEM, GET_ATTENUATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_iPhone_GetReferenceDistance, SOUNDSYSTEM, GET_REFERENCE_DISTANCE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_iPhone_IsLooping, SOUNDSYSTEM, IS_LOOPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_iPhone_GetDuration, SOUNDSYSTEM, GET_DURATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_iPhone_GetStatus, SOUNDSYSTEM, GET_STATUS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_iPhone_SetGlobalVolume, SOUNDSYSTEM, SET_GLOBAL_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_iPhone_GetGlobalVolume, SOUNDSYSTEM, GET_GLOBAL_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_iPhone_SetListenerPosition, SOUNDSYSTEM, SET_LISTENER_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_iPhone_GetListenerPosition, SOUNDSYSTEM, GET_LISTENER_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_END();
