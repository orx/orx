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
 * @date 07/01/2011
 * @author simons.philippe@gmail.com
 *
 * OpenSLES sound system plugin implementation
 *
 */


#include "orxPluginAPI.h"

// for native audio
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#define STB_VORBIS_NO_STDIO
#include "stb_vorbis.c"

/** Module flags
 */
#define orxSOUNDSYSTEM_KU32_STATIC_FLAG_NONE      0x00000000 /**< No flags */

#define orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY     0x00000001 /**< Ready flag */

#define orxSOUNDSYSTEM_KU32_STATIC_MASK_ALL       0xFFFFFFFF /**< All mask */

/** Misc defines
 */
#define orxSOUNDSYSTEM_KU32_BANK_SIZE                   32
#define orxSOUNDSYSTEM_KU32_DEFAULT_RECORDING_FREQUENCY 44100
#define orxSOUNDSYSTEM_KU32_RECORDING_BUFFER_SIZE       (orxSOUNDSYSTEM_KU32_STREAM_BUFFER_SIZE / sizeof(orxS16))
#define orxSOUNDSYSTEM_KF_DEFAULT_DIMENSION_RATIO       orx2F(0.01f)

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/
/** Internal data structure
 */
typedef struct __orxSOUNDSYSTEM_DATA_t
{
  orxU32          u32ChannelNumber;
  orxU32          u32FrameNumber;
  orxU32          u32SampleRate;

  stb_vorbis     *vorbis;
  void           *pAssetBuffer;
  
} orxSOUNDSYSTEM_DATA;

/** Internal sample structure
 */
struct __orxSOUNDSYSTEM_SAMPLE_t
{
  void     *pBuffer;
  orxU32    u32BufferSize;

  SLuint32  u32ChannelNumber;
  SLuint32  u32SampleRate;
};

/** Internal sound structure
 */
struct __orxSOUNDSYSTEM_SOUND_t
{
  SLObjectItf             PlayerObject;
  SLPlayItf               PlayerPlay;
  SLSeekItf               PlayerSeek;
  SLVolumeItf             PlayerVolume;
  SLPlaybackRateItf       PlayerRate;
  SLAndroidSimpleBufferQueueItf PlayerBufferQueue;

  orxBOOL                 isBufferQueue;
  orxBOOL                 isLooping;
  orxSOUNDSYSTEM_SAMPLE  *pstSample;
};

/** Static structure
 */
typedef struct __orxSOUNDSYSTEM_STATIC_t
{
  SLObjectItf             engineObject;       /**< OpenSL engine Object */
  SLEngineItf             engineEngine;       /**< OpenSL engine Interface */
  SLObjectItf             outputMixObject;    /**< OpenSL output mixer */
  
  orxBANK                *pstSampleBank;      /**< Sound bank */
  orxBANK                *pstSoundBank;       /**< Sound bank */
  orxFLOAT                fDimensionRatio;    /**< Dimension ratio */
  orxFLOAT                fRecDimensionRatio; /**< Reciprocal dimension ratio */
  orxU32                  u32Flags;           /**< Status flags */

} orxSOUNDSYSTEM_STATIC;



/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxSOUNDSYSTEM_STATIC sstSoundSystem;

static orxINLINE orxSTATUS orxSoundSystem_Android_OpenFile(const orxSTRING _zFileName, orxSOUNDSYSTEM_DATA *_pstData)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;;

  /* Checks */
  orxASSERT(_zFileName != orxNULL);
  orxASSERT(_pstData != orxNULL);

  /* open file from assets */
  AAsset* asset = AAssetManager_open(pstApp->activity->assetManager, _zFileName, AASSET_MODE_RANDOM);

  /* Valid? */
  if(asset != NULL)
  {
    off_t asset_size = AAsset_getLength(asset);
    
    if((_pstData->pAssetBuffer = orxMemory_Allocate(asset_size, orxMEMORY_TYPE_MAIN)) != orxNULL)
    {
      /* read asset in memory */
      AAsset_read(asset, _pstData->pAssetBuffer, asset_size);

      /* Opens file with vorbis */
      _pstData->vorbis = stb_vorbis_open_memory((unsigned char*)_pstData->pAssetBuffer, asset_size, NULL, NULL);

      /* Success? */
      if(_pstData->vorbis != NULL)
      {
        stb_vorbis_info stFileInfo;

        /* Gets file info */
        stFileInfo  = stb_vorbis_get_info(_pstData->vorbis);

        /* Stores info */
        _pstData->u32ChannelNumber  = (orxU32)stFileInfo.channels;
        _pstData->u32FrameNumber    = (orxU32)stb_vorbis_stream_length_in_samples(_pstData->vorbis);
        _pstData->u32SampleRate     = (orxU32)stFileInfo.sample_rate;

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
      
      AAsset_close(asset);
    }
  }

  /* Done! */
  return eResult;
}

static orxINLINE void orxSoundSystem_Android_CloseFile(orxSOUNDSYSTEM_DATA *_pstData)
{
  /* Checks */
  orxASSERT(_pstData != orxNULL);

  /* Closes file */
  stb_vorbis_close(_pstData->vorbis);
  
  /* free AssetBuffer */
  orxMemory_Free(_pstData->pAssetBuffer);
  
  /* Done! */
  return;
}

static orxINLINE orxU32 orxSoundSystem_Android_Read(orxSOUNDSYSTEM_DATA *_pstData, orxU32 _u32FrameNumber, void *_pBuffer)
{
  orxU32 u32Result;

  /* Checks */
  orxASSERT(_pstData != orxNULL);

  /* Reads frames */
  u32Result = (orxU32)stb_vorbis_get_samples_short_interleaved(_pstData->vorbis, _pstData->u32ChannelNumber, (short *)_pBuffer, _u32FrameNumber * _pstData->u32ChannelNumber);

  /* Done! */
  return u32Result;
}

orxSTATUS orxFASTCALL orxSoundSystem_Android_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Was already initialized? */
  if(!(sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY))
  {
    orxFLOAT fRatio;

    /* Cleans static controller */
    orxMemory_Zero(&sstSoundSystem, sizeof(orxSOUNDSYSTEM_STATIC));

    SLresult result;
    result = slCreateEngine(&sstSoundSystem.engineObject, 0, NULL, 0, NULL, NULL);
    orxASSERT(SL_RESULT_SUCCESS == result);
    
    // realize the engine
    result = (*sstSoundSystem.engineObject)->Realize(sstSoundSystem.engineObject, SL_BOOLEAN_FALSE);
    orxASSERT(SL_RESULT_SUCCESS == result);

    // get the engine interface, which is needed in order to create other objects
    result = (*sstSoundSystem.engineObject)->GetInterface(sstSoundSystem.engineObject, SL_IID_ENGINE, &sstSoundSystem.engineEngine);
    orxASSERT(SL_RESULT_SUCCESS == result);

    // create output mix
    result = (*sstSoundSystem.engineEngine)->CreateOutputMix(sstSoundSystem.engineEngine, &sstSoundSystem.outputMixObject, 0, NULL, NULL);
    orxASSERT(SL_RESULT_SUCCESS == result);

    // realize the output mix
    result = (*sstSoundSystem.outputMixObject)->Realize(sstSoundSystem.outputMixObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    
    /* Creates banks */
    sstSoundSystem.pstSampleBank  = orxBank_Create(orxSOUNDSYSTEM_KU32_BANK_SIZE, sizeof(orxSOUNDSYSTEM_SAMPLE), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
    sstSoundSystem.pstSoundBank   = orxBank_Create(orxSOUNDSYSTEM_KU32_BANK_SIZE, sizeof(orxSOUNDSYSTEM_SOUND), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if((sstSoundSystem.pstSampleBank != orxNULL) && (sstSoundSystem.pstSoundBank))
    {
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

      // destroy output mix object, and invalidate all associated interfaces
      if (sstSoundSystem.outputMixObject != NULL)
      {
        (*sstSoundSystem.outputMixObject)->Destroy(sstSoundSystem.outputMixObject);
        sstSoundSystem.outputMixObject = NULL;
      }

      // destroy engine object, and invalidate all associated interfaces
      if (sstSoundSystem.engineObject != NULL)
      {
        (*sstSoundSystem.engineObject)->Destroy(sstSoundSystem.engineObject);
        sstSoundSystem.engineObject = NULL;
        sstSoundSystem.engineEngine = NULL;
      }
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
    /* Deletes banks */
    orxBank_Delete(sstSoundSystem.pstSampleBank);
    orxBank_Delete(sstSoundSystem.pstSoundBank);

    // destroy output mix object, and invalidate all associated interfaces
    if (sstSoundSystem.outputMixObject != NULL)
    {
      (*sstSoundSystem.outputMixObject)->Destroy(sstSoundSystem.outputMixObject);
      sstSoundSystem.outputMixObject = NULL;
    }

    // destroy engine object, and invalidate all associated interfaces
    if (sstSoundSystem.engineObject != NULL)
    {
      (*sstSoundSystem.engineObject)->Destroy(sstSoundSystem.engineObject);
      sstSoundSystem.engineObject = NULL;
      sstSoundSystem.engineEngine = NULL;
    }

    /* Cleans static controller */
    orxMemory_Zero(&sstSoundSystem, sizeof(orxSOUNDSYSTEM_STATIC));
  }

  /* Done! */
  return;
}

orxSOUNDSYSTEM_SAMPLE *orxFASTCALL orxSoundSystem_Android_CreateSample(orxU32 _u32ChannelNumber, orxU32 _u32FrameNumber, orxU32 _u32SampleRate)
{
  orxSOUNDSYSTEM_SAMPLE *pstResult = orxNULL;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  //! TODO: Not implemented yet

  /* Done! */
  return pstResult;
}

orxSOUNDSYSTEM_SAMPLE *orxFASTCALL orxSoundSystem_Android_LoadSample(const orxSTRING _zFilename)
{
  orxSOUNDSYSTEM_DATA     stData;
  orxSOUNDSYSTEM_SAMPLE  *pstResult = NULL;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_zFilename != orxNULL);

  /* Opens file */
  if(orxSoundSystem_Android_OpenFile(_zFilename, &stData) != orxSTATUS_FAILURE)
  {
    /* Allocates sample */
    pstResult = (orxSOUNDSYSTEM_SAMPLE *)orxBank_Allocate(sstSoundSystem.pstSampleBank);

    /* Valid? */
    if(pstResult != orxNULL)
    {
      /* Gets buffer size */
      pstResult->u32BufferSize = stData.u32FrameNumber * stData.u32ChannelNumber * sizeof(orxS16);

      /* Allocates buffer */
      if((pstResult->pBuffer = orxMemory_Allocate(pstResult->u32BufferSize, orxMEMORY_TYPE_MAIN)) != orxNULL)
      {
        orxU32 u32ReadFrameNumber;

        /* Reads data */
        u32ReadFrameNumber = orxSoundSystem_Android_Read(&stData, stData.u32FrameNumber, pstResult->pBuffer);

        /* Success? */
        if(u32ReadFrameNumber == stData.u32FrameNumber)
        {
          /* Stores infos */
          pstResult->u32SampleRate = stData.u32SampleRate * 1000;
          pstResult->u32ChannelNumber = stData.u32ChannelNumber;
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
    orxSoundSystem_Android_CloseFile(&stData);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't load sound sample <%s>: unsupported format.", _zFilename);
  }

  /* Done! */
  return pstResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_Android_DeleteSample(orxSOUNDSYSTEM_SAMPLE *_pstSample)
{
  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSample != orxNULL);

  /* Frees buffer */
  orxMemory_Free(_pstSample->pBuffer);

  /* Deletes sample */
  orxBank_Free(sstSoundSystem.pstSampleBank, _pstSample);

  /* Done! */
  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL orxSoundSystem_Android_GetSampleInfo(const orxSOUNDSYSTEM_SAMPLE *_pstSample, orxU32 *_pu32ChannelNumber, orxU32 *_pu32FrameNumber, orxU32 *_pu32SampleRate)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  //! TODO: Not implemented yet

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_Android_SetSampleData(orxSOUNDSYSTEM_SAMPLE *_pstSample, const orxS16 *_as16Data, orxU32 _u32SampleNumber)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  //! TODO: Not implemented yet

  /* Done! */
  return eResult;
}

// this callback handler is called every time a buffer finishes playing
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
  SLresult result;
  orxSOUNDSYSTEM_SOUND *_pstSound = (orxSOUNDSYSTEM_SOUND*)context;
  
  if(_pstSound->isLooping == orxTRUE)
  {
    //enqueue the buffer
    result = (*_pstSound->PlayerBufferQueue)->Enqueue(_pstSound->PlayerBufferQueue, _pstSound->pstSample->pBuffer, _pstSound->pstSample->u32BufferSize);
    orxASSERT(SL_RESULT_SUCCESS == result);
  }
  else
  {
    result = (*_pstSound->PlayerPlay)->SetPlayState(_pstSound->PlayerPlay, SL_PLAYSTATE_STOPPED);
    orxASSERT(SL_RESULT_SUCCESS == result);
  }
}

orxSOUNDSYSTEM_SOUND *orxFASTCALL orxSoundSystem_Android_CreateFromSample(const orxSOUNDSYSTEM_SAMPLE *_pstSample)
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
    SLresult result;

    /* Clears it */
    orxMemory_Zero(pstResult, sizeof(orxSOUNDSYSTEM_SOUND));

    /* configure audio source */
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 1};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, _pstSample->u32ChannelNumber, _pstSample->u32SampleRate,
        SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, sstSoundSystem.outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    // create audio player
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME, SL_IID_PLAYBACKRATE};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    result = (*sstSoundSystem.engineEngine)->CreateAudioPlayer(sstSoundSystem.engineEngine, &pstResult->PlayerObject, &audioSrc, &audioSnk, 3, ids, req);
    if(SL_RESULT_SUCCESS != result)
    {
      pstResult->PlayerObject = NULL;
      orxBank_Free(sstSoundSystem.pstSoundBank, pstResult);
    	return orxNULL;
    }

    // realize the player
    result = (*pstResult->PlayerObject)->Realize(pstResult->PlayerObject, SL_BOOLEAN_FALSE);
    // this will always succeed on Android, but we check result for portability to other platforms
    if (SL_RESULT_SUCCESS != result) {
        (*pstResult->PlayerObject)->Destroy(pstResult->PlayerObject);
        pstResult->PlayerObject = NULL;
        orxBank_Free(sstSoundSystem.pstSoundBank, pstResult);
        return orxNULL;
    }

    // get the play interface
    result = (*pstResult->PlayerObject)->GetInterface(pstResult->PlayerObject, SL_IID_PLAY, &pstResult->PlayerPlay);
    orxASSERT(SL_RESULT_SUCCESS == result);

    // get the buffer queue interface
    result = (*pstResult->PlayerObject)->GetInterface(pstResult->PlayerObject, SL_IID_BUFFERQUEUE, &pstResult->PlayerBufferQueue);
    orxASSERT(SL_RESULT_SUCCESS == result);
    
    // get the volume interface
    result = (*pstResult->PlayerObject)->GetInterface(pstResult->PlayerObject, SL_IID_VOLUME, &pstResult->PlayerVolume);
    orxASSERT(SL_RESULT_SUCCESS == result);

    // get the rate interface
    result = (*pstResult->PlayerObject)->GetInterface(pstResult->PlayerObject, SL_IID_PLAYBACKRATE, &pstResult->PlayerRate);
    orxASSERT(SL_RESULT_SUCCESS == result);

    // register callback on the buffer queue
    result = (*pstResult->PlayerBufferQueue)->RegisterCallback(pstResult->PlayerBufferQueue, bqPlayerCallback, (void*) pstResult);
    assert(SL_RESULT_SUCCESS == result);

    pstResult->isBufferQueue = orxTRUE;
    pstResult->isLooping = orxFALSE;

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

orxSOUNDSYSTEM_SOUND *orxFASTCALL orxSoundSystem_Android_CreateStreamFromFile(const orxSTRING _zFileName, const orxSTRING _zReference)
{
  orxSOUNDSYSTEM_SOUND *pstResult = orxNULL;
  SLresult result;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_zFileName != orxNULL);

  /* Allocates sound */
  pstResult = (orxSOUNDSYSTEM_SOUND *)orxBank_Allocate(sstSoundSystem.pstSoundBank);

  /* Valid? */
  if(pstResult != orxNULL)
  {
    /* Clears it */
    orxMemory_Zero(pstResult, sizeof(orxSOUNDSYSTEM_SOUND));
    
    /* open file from assets */
    AAsset* asset = AAssetManager_open(pstApp->activity->assetManager, _zFileName, AASSET_MODE_RANDOM);

    /* Valid? */
    if(asset != NULL)
    {
      // open asset as file descriptor
      off_t start, length;
      int fd = AAsset_openFileDescriptor(asset, &start, &length);
      orxASSERT(0 <= fd);
      AAsset_close(asset);

      // configure audio source
      SLDataLocator_AndroidFD loc_fd = {SL_DATALOCATOR_ANDROIDFD, fd, start, length};
      SLDataFormat_MIME format_mime = {SL_DATAFORMAT_MIME, NULL, SL_CONTAINERTYPE_UNSPECIFIED};
      SLDataSource audioSrc = {&loc_fd, &format_mime};

      // configure audio sink
      SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, sstSoundSystem.outputMixObject};
      SLDataSink audioSnk = {&loc_outmix, NULL};

      // create audio player
      const SLInterfaceID ids[3] = {SL_IID_SEEK, SL_IID_VOLUME, SL_IID_PLAYBACKRATE};
      const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
      result = (*sstSoundSystem.engineEngine)->CreateAudioPlayer(sstSoundSystem.engineEngine, &pstResult->PlayerObject, &audioSrc, &audioSnk, 3, ids, req);
      orxASSERT(SL_RESULT_SUCCESS == result);

      // realize the player
      result = (*pstResult->PlayerObject)->Realize(pstResult->PlayerObject, SL_BOOLEAN_FALSE);
      orxASSERT(SL_RESULT_SUCCESS == result);

      // get the play interface
      result = (*pstResult->PlayerObject)->GetInterface(pstResult->PlayerObject, SL_IID_PLAY, &pstResult->PlayerPlay);
      orxASSERT(SL_RESULT_SUCCESS == result);

      // get the seek interface
      result = (*pstResult->PlayerObject)->GetInterface(pstResult->PlayerObject, SL_IID_SEEK, &pstResult->PlayerSeek);
      orxASSERT(SL_RESULT_SUCCESS == result);
      
      // get the volume interface
      result = (*pstResult->PlayerObject)->GetInterface(pstResult->PlayerObject, SL_IID_VOLUME, &pstResult->PlayerVolume);
      orxASSERT(SL_RESULT_SUCCESS == result);

      // get the rate interface
      result = (*pstResult->PlayerObject)->GetInterface(pstResult->PlayerObject, SL_IID_PLAYBACKRATE, &pstResult->PlayerRate);
      orxASSERT(SL_RESULT_SUCCESS == result);

      pstResult->isBufferQueue = orxFALSE;
      pstResult->isLooping = orxFALSE;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't load sound stream <%s>: can't sound structure.", _zFileName);
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

  /* delete player */
  (*_pstSound->PlayerObject)->Destroy(_pstSound->PlayerObject);
  _pstSound->PlayerObject = NULL;
  _pstSound->PlayerPlay = NULL;
  _pstSound->PlayerSeek = NULL;
  _pstSound->PlayerBufferQueue = NULL;

  /* Deletes sound */
  orxBank_Free(sstSoundSystem.pstSoundBank, _pstSound);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_Android_Play(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;
  SLresult result;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Start player */
  if(_pstSound->isBufferQueue == orxTRUE)
  {
    //enqueue the buffer
    result = (*_pstSound->PlayerBufferQueue)->Enqueue(_pstSound->PlayerBufferQueue, _pstSound->pstSample->pBuffer, _pstSound->pstSample->u32BufferSize);
    orxASSERT(SL_RESULT_SUCCESS == result);
  }
  
  result = (*_pstSound->PlayerPlay)->SetPlayState(_pstSound->PlayerPlay, SL_PLAYSTATE_PLAYING);
  orxASSERT(SL_RESULT_SUCCESS == result);
  
  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_Android_Pause(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;
  SLresult result;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Pauses Player */
  result = (*_pstSound->PlayerPlay)->SetPlayState(_pstSound->PlayerPlay, SL_PLAYSTATE_PAUSED);
  orxASSERT(SL_RESULT_SUCCESS == result);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_Android_Stop(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;
  SLresult result;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Stops Player */
  result = (*_pstSound->PlayerPlay)->SetPlayState(_pstSound->PlayerPlay, SL_PLAYSTATE_STOPPED);
  orxASSERT(SL_RESULT_SUCCESS == result);
  
  if(_pstSound->isBufferQueue == orxTRUE)
  {
    result = (*_pstSound->PlayerBufferQueue)->Clear(_pstSound->PlayerBufferQueue);
    orxASSERT(SL_RESULT_SUCCESS == result);
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

  //! TODO: Not implemented yet

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_Android_StopRecording()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  //! TODO: Not implemented yet

  /* Done! */
  return eResult;
}

orxBOOL orxFASTCALL orxSoundSystem_Android_HasRecordingSupport()
{
  orxBOOL bResult = orxFALSE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  //! TODO: Not implemented yet

  /* Done! */
  return bResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_Android_SetVolume(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fVolume)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  SLmillibel level;
  SLmillibel maxLevel;
  SLresult result;
  
  result = (*_pstSound->PlayerVolume)->GetMaxVolumeLevel(_pstSound->PlayerVolume, &maxLevel);
  orxASSERT(SL_RESULT_SUCCESS == result);
  
  level = _fVolume * maxLevel;

  result = (*_pstSound->PlayerVolume)->SetVolumeLevel(_pstSound->PlayerVolume, level);
  orxASSERT(SL_RESULT_SUCCESS == result);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_Android_SetPitch(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fPitch)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  SLresult result;
  SLpermille rate = _fPitch * 1000;
  
  result = (*_pstSound->PlayerRate)->SetRate(_pstSound->PlayerRate, rate);
  orxASSERT(SL_RESULT_SUCCESS == result);
  

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_Android_SetPosition(orxSOUNDSYSTEM_SOUND *_pstSound, const orxVECTOR *_pvPosition)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);
  orxASSERT(_pvPosition != orxNULL);

  //! TODO: Not implemented yet

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_Android_SetAttenuation(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fAttenuation)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  //! TODO: Not implemented yet

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_Android_SetReferenceDistance(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fDistance)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  //! TODO: Not implemented yet

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_Android_Loop(orxSOUNDSYSTEM_SOUND *_pstSound, orxBOOL _bLoop)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;
  SLresult result;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  if(_pstSound->isBufferQueue == orxFALSE)
  {
    // enable whole file looping
    result = (*_pstSound->PlayerSeek)->SetLoop(_pstSound->PlayerSeek, (_bLoop != orxFALSE) ? SL_BOOLEAN_TRUE : SL_BOOLEAN_FALSE, 0, SL_TIME_UNKNOWN);
    orxASSERT(SL_RESULT_SUCCESS == result);
    
  }
  _pstSound->isLooping = _bLoop;

  /* Done! */
  return eResult;
}

orxFLOAT orxFASTCALL orxSoundSystem_Android_GetVolume(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxFLOAT fResult = orxFLOAT_0;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  SLmillibel level;
  SLmillibel maxLevel;
  SLresult result;
  
  result = (*_pstSound->PlayerVolume)->GetVolumeLevel(_pstSound->PlayerVolume, &level);
  orxASSERT(SL_RESULT_SUCCESS == result);

  result = (*_pstSound->PlayerVolume)->GetMaxVolumeLevel(_pstSound->PlayerVolume, &maxLevel);
  orxASSERT(SL_RESULT_SUCCESS == result);

  fResult = orx2F(maxLevel) / orx2F(level);
  
  /* Done! */
  return fResult;
}

orxFLOAT orxFASTCALL orxSoundSystem_Android_GetPitch(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxFLOAT fResult = orxFLOAT_0;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  SLresult result;
  SLpermille rate;
  
  result = (*_pstSound->PlayerRate)->GetRate(_pstSound->PlayerRate, &rate);
  orxASSERT(SL_RESULT_SUCCESS == result);
  
  fResult = orx2F(rate) / orx2F(1000);

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

  //! TODO: Not implemented yet

  /* Done! */
  return pvResult;
}

orxFLOAT orxFASTCALL orxSoundSystem_Android_GetAttenuation(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  //! TODO: Not implemented yet

  /* Done! */
  return fResult;
}

orxFLOAT orxFASTCALL orxSoundSystem_Android_GetReferenceDistance(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  //! TODO: Not implemented yet

  /* Done! */
  return fResult;
}

orxBOOL orxFASTCALL orxSoundSystem_Android_IsLooping(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

   bResult = _pstSound->isLooping;

  /* Done! */
  return bResult;
}

orxFLOAT orxFASTCALL orxSoundSystem_Android_GetDuration(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxFLOAT fResult = orxFLOAT_0;
  SLresult result;
  SLmillisecond duration;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Updates result */
  result = (*_pstSound->PlayerPlay)->GetDuration(_pstSound->PlayerPlay, &duration);
  orxASSERT(SL_RESULT_SUCCESS == result);
  
  fResult = orxU2F(duration) / orxU2F(1000);

  /* Done! */
  return fResult;
}

orxSOUNDSYSTEM_STATUS orxFASTCALL orxSoundSystem_Android_GetStatus(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  SLresult result;
  SLuint32 state;
  orxSOUNDSYSTEM_STATUS eResult = orxSOUNDSYSTEM_STATUS_NONE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* get Player State */
  result = (*_pstSound->PlayerPlay)->GetPlayState(_pstSound->PlayerPlay, &state);
  orxASSERT(SL_RESULT_SUCCESS == result);

  switch(state)
  {
    case SL_PLAYSTATE_STOPPED:
    {
      eResult = orxSOUNDSYSTEM_STATUS_STOP;
      break;
    }
      
    case SL_PLAYSTATE_PAUSED:
    {
      eResult = orxSOUNDSYSTEM_STATUS_PAUSE;
      break;
    }
    
    case SL_PLAYSTATE_PLAYING:
    {
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

  //! not supported

  /* Done! */
  return eResult;
}

orxFLOAT orxFASTCALL orxSoundSystem_Android_GetGlobalVolume()
{
  orxFLOAT fResult = orxFLOAT_0;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  //! not supported

  /* Done! */
  return fResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_Android_SetListenerPosition(const orxVECTOR *_pvPosition)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvPosition != orxNULL);

  //! TODO: Not implemented yet

  /* Done! */
  return eResult;
}

orxVECTOR *orxFASTCALL orxSoundSystem_Android_GetListenerPosition(orxVECTOR *_pvPosition)
{
  orxVECTOR *pvResult = _pvPosition;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvPosition != orxNULL);

  //! TODO: Not implemented yet

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
