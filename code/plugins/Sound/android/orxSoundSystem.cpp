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
 * @date 13/01/2011
 * @author simons.philippe@gmail.com
 *
 * Android sound system plugin implementation
 *
 */

#include "orxPluginAPI.h"

#include <jni.h>
#include <nv_event/nv_event.h>
#include <nv_thread/nv_thread.h>

/** Module flags
 */
#define orxSOUNDSYSTEM_KU32_STATIC_FLAG_NONE      0x00000000 /**< No flags */

#define orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY     0x00000001 /**< Ready flag */

#define orxSOUNDSYSTEM_KU32_STATIC_MASK_ALL       0xFFFFFFFF /**< All mask */

extern jobject oActivity;

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/
/** Internal sample structure
 */
struct __orxSOUNDSYSTEM_SAMPLE_t {

	jint soundId;

};
/** Internal sound structure
 */
struct __orxSOUNDSYSTEM_SOUND_t {
	orxSOUNDSYSTEM_SAMPLE* sampleBuffer;
	jfloat  pitch; //the rate in soundpool
	jfloat  volumn; // It accepts a value between 0.0 and 1.0, default being 1.0.
	orxBOOL bLoop;
	orxBOOL bPaused;
	
	orxBOOL bIsUsingMediaPlayer;
	
	jint    streamID;            // bIsUsingMediaPlayer == orxFALSE
	
	jobject oMediaPlayer;        // bIsUsingMediaPlayer == orxTRUE
	jobject oAssetFileDescriptor; // bIsUsingMediaPlayer == orxTRUE
};

/** Static structure
 */
typedef struct __orxSOUNDSYSTEM_STATIC_t {
	orxU32          u32Flags;

  /* AssetManager jni stuff */
  jobject         oAssetManager;
  jmethodID       mIDAssetManager_openFd;
  jmethodID       mIDAssetFileDescriptor_getFileDescriptor;
  jmethodID       mIDAssetFileDescriptor_close;
  jmethodID       mIDAssetFileDescriptor_getStartOffset;
  jmethodID       mIDAssetFileDescriptor_getLength;

  /* SoundPool jni stuff */
	jobject         oSoundPool;
  jmethodID       mIDSoundPool_load;
  jmethodID       mIDSoundPool_unload;
  jmethodID       mIDSoundPool_play;
  jmethodID       mIDSoundPool_pause;
  jmethodID       mIDSoundPool_stop;
  jmethodID       mIDSoundPool_setVolume;
  jmethodID       mIDSoundPool_setRate;
  jmethodID       mIDSoundPool_setLoop;
  
  /* MediaPlayer jni stuff */
  jclass          clsMediaPlayer;
  jmethodID       mIDMediaPlayer_init;
	jmethodID       mIDMediaPlayer_setDataSource;
	jmethodID       mIDMediaPlayer_prepare;
	jmethodID       mIDMediaPlayer_start;
	jmethodID       mIDMediaPlayer_stop;
	jmethodID       mIDMediaPlayer_pause;
	jmethodID       mIDMediaPlayer_release;
	jmethodID       mIDMediaPlayer_setLooping;
	jmethodID       mIDMediaPlayer_getDuration;
	jmethodID       mIDMediaPlayer_isPlaying;
	jmethodID       mIDMediaPlayer_setVolume;

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

static orxSTATUS orxFASTCALL orxSoundSystem_Android_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(_pstEvent->eType == orxEVENT_TYPE_SYSTEM);
  
  if(_pstEvent->eID == orxSYSTEM_EVENT_BACKGROUND)
  {
    orxSOUND *pstSound;
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "orxSYSTEM_EVENT_BACKGROUND");
    
    /* Gets first sound */
    pstSound = orxSOUND(orxStructure_GetFirst(orxSTRUCTURE_ID_SOUND));
    while(pstSound != orxNULL)
    {
      if(orxSound_IsStream(pstSound) && orxSound_GetStatus(pstSound) == orxSOUND_STATUS_PLAY)
      {
        orxSound_Pause(pstSound);
      }

      /* Gets next sound */
      pstSound = orxSOUND(orxStructure_GetNext(pstSound));
    }
  }
  
  if(_pstEvent->eID == orxSYSTEM_EVENT_FOREGROUND)
  {
    orxSOUND *pstSound;
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "orxSYSTEM_EVENT_FOREGROUND");
    
    /* Gets first sound */
    pstSound = orxSOUND(orxStructure_GetFirst(orxSTRUCTURE_ID_SOUND));
    while(pstSound != orxNULL)
    {
      if(orxSound_IsStream(pstSound) && orxSound_GetStatus(pstSound) == orxSOUND_STATUS_PAUSE)
      {
        orxSound_Play(pstSound);
      }

      /* Gets next sound */
      pstSound = orxSOUND(orxStructure_GetNext(pstSound));
    }
  }
  
  return eResult;
}  

extern "C"  orxSTATUS orxFASTCALL orxSoundSystem_Android_Init() {
	orxSTATUS eResult = orxSTATUS_FAILURE;

	/* Was already initialized? */
	if (!(sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY)) {
		orxFLOAT fRatio;

		/* Cleans static controller */
		orxMemory_Zero(&sstSoundSystem, sizeof(orxSOUNDSYSTEM_STATIC));

		/* Gets dimension ratio */
		orxConfig_PushSection(orxSOUNDSYSTEM_KZ_CONFIG_SECTION);

		/* Updates status */
		orxFLAG_SET(sstSoundSystem.u32Flags, orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY, orxSOUNDSYSTEM_KU32_STATIC_MASK_ALL);

		/* Pops config section */
		orxConfig_PopSection();
		
    JNIEnv *poJEnv = NVThreadGetCurrentJNIEnv();
		
		/* retrieve the AssetManager */
		jclass clsActivity = poJEnv->FindClass("android/app/Activity");
		orxASSERT(clsActivity != orxNULL);
		jmethodID mIDgetAssets = poJEnv->GetMethodID(clsActivity, "getAssets", "()Landroid/content/res/AssetManager;");
		orxASSERT(mIDgetAssets != orxNULL);
		jobject oAssetManager = poJEnv->CallObjectMethod(oActivity, mIDgetAssets);
		orxASSERT(oAssetManager != orxNULL);
		sstSoundSystem.oAssetManager = poJEnv->NewGlobalRef(oAssetManager);
		jclass clsAssetManager = poJEnv->GetObjectClass(sstSoundSystem.oAssetManager);
		orxASSERT(clsAssetManager != orxNULL);
		sstSoundSystem.mIDAssetManager_openFd = poJEnv->GetMethodID(clsAssetManager, "openFd", "(Ljava/lang/String;)Landroid/content/res/AssetFileDescriptor;");
		orxASSERT(sstSoundSystem.mIDAssetManager_openFd != orxNULL);
		
		/* retrieve the AssetFileDescriptor */
		jclass clsAssetFileDescriptor = poJEnv->FindClass("android/content/res/AssetFileDescriptor");
		orxASSERT(clsAssetFileDescriptor != orxNULL);
		sstSoundSystem.mIDAssetFileDescriptor_getFileDescriptor = poJEnv->GetMethodID(clsAssetFileDescriptor, "getFileDescriptor", "()Ljava/io/FileDescriptor;");
		orxASSERT(sstSoundSystem.mIDAssetFileDescriptor_getFileDescriptor != orxNULL);
		sstSoundSystem.mIDAssetFileDescriptor_close = poJEnv->GetMethodID(clsAssetFileDescriptor, "close", "()V");
		orxASSERT(sstSoundSystem.mIDAssetFileDescriptor_close != orxNULL);
		sstSoundSystem.mIDAssetFileDescriptor_getStartOffset = poJEnv->GetMethodID(clsAssetFileDescriptor, "getStartOffset", "()J");
		orxASSERT(sstSoundSystem.mIDAssetFileDescriptor_getStartOffset != orxNULL);
		sstSoundSystem.mIDAssetFileDescriptor_getLength = poJEnv->GetMethodID(clsAssetFileDescriptor, "getLength", "()J");
		orxASSERT(sstSoundSystem.mIDAssetFileDescriptor_getLength != orxNULL);

    /* instanciate the SoundPool */
		jclass clsSoundPool = poJEnv->FindClass("android/media/SoundPool");
		orxASSERT(clsSoundPool != orxNULL);
		jmethodID mIDSoundPool_init = poJEnv->GetMethodID(clsSoundPool, "<init>", "(III)V");
		orxASSERT(mIDSoundPool_init != orxNULL);
		jobject oSoundPool = poJEnv->NewObject(clsSoundPool, mIDSoundPool_init, 16, 3, 0);
    orxASSERT(oSoundPool != orxNULL);
    sstSoundSystem.oSoundPool = poJEnv->NewGlobalRef(oSoundPool);
    /* retrieve methodID */
		sstSoundSystem.mIDSoundPool_load = poJEnv->GetMethodID(clsSoundPool, "load", "(Landroid/content/res/AssetFileDescriptor;I)I");
		orxASSERT(sstSoundSystem.mIDSoundPool_load != orxNULL);
		sstSoundSystem.mIDSoundPool_unload = poJEnv->GetMethodID(clsSoundPool, "unload", "(I)Z");
		orxASSERT(sstSoundSystem.mIDSoundPool_unload != orxNULL);
		sstSoundSystem.mIDSoundPool_play = poJEnv->GetMethodID(clsSoundPool, "play", "(IFFIIF)I");
		orxASSERT(sstSoundSystem.mIDSoundPool_play != orxNULL);
		sstSoundSystem.mIDSoundPool_pause = poJEnv->GetMethodID(clsSoundPool, "pause", "(I)V");
		orxASSERT(sstSoundSystem.mIDSoundPool_pause != orxNULL);
		sstSoundSystem.mIDSoundPool_stop = poJEnv->GetMethodID(clsSoundPool, "stop", "(I)V");
		orxASSERT(sstSoundSystem.mIDSoundPool_stop != orxNULL);
		sstSoundSystem.mIDSoundPool_setVolume = poJEnv->GetMethodID(clsSoundPool, "setVolume", "(IFF)V");
		orxASSERT(sstSoundSystem.mIDSoundPool_setVolume != orxNULL);
		sstSoundSystem.mIDSoundPool_setRate = poJEnv->GetMethodID(clsSoundPool, "setRate", "(IF)V");
		orxASSERT(sstSoundSystem.mIDSoundPool_setRate != orxNULL);
		sstSoundSystem.mIDSoundPool_setLoop = poJEnv->GetMethodID(clsSoundPool, "setLoop", "(II)V");
		orxASSERT(sstSoundSystem.mIDSoundPool_setLoop != orxNULL);
		
		/* retrieve MediaPlayer class and methodID */
		sstSoundSystem.clsMediaPlayer = poJEnv->FindClass("android/media/MediaPlayer");
		orxASSERT(sstSoundSystem.clsMediaPlayer != orxNULL);
		sstSoundSystem.mIDMediaPlayer_init = poJEnv->GetMethodID(sstSoundSystem.clsMediaPlayer, "<init>", "()V");
		orxASSERT(sstSoundSystem.mIDMediaPlayer_init != orxNULL);
		sstSoundSystem.mIDMediaPlayer_setDataSource = poJEnv->GetMethodID(sstSoundSystem.clsMediaPlayer, "setDataSource", "(Ljava/io/FileDescriptor;JJ)V");
    orxASSERT(sstSoundSystem.mIDMediaPlayer_setDataSource != orxNULL);
    sstSoundSystem.mIDMediaPlayer_prepare = poJEnv->GetMethodID(sstSoundSystem.clsMediaPlayer, "prepare", "()V");
    orxASSERT(sstSoundSystem.mIDMediaPlayer_prepare != orxNULL);
    sstSoundSystem.mIDMediaPlayer_start = poJEnv->GetMethodID(sstSoundSystem.clsMediaPlayer, "start", "()V");
    orxASSERT(sstSoundSystem.mIDMediaPlayer_start != orxNULL);
    sstSoundSystem.mIDMediaPlayer_stop = poJEnv->GetMethodID(sstSoundSystem.clsMediaPlayer, "stop", "()V");
    orxASSERT(sstSoundSystem.mIDMediaPlayer_stop != orxNULL);
    sstSoundSystem.mIDMediaPlayer_pause = poJEnv->GetMethodID(sstSoundSystem.clsMediaPlayer, "pause", "()V");
    orxASSERT(sstSoundSystem.mIDMediaPlayer_pause != orxNULL);
    sstSoundSystem.mIDMediaPlayer_release = poJEnv->GetMethodID(sstSoundSystem.clsMediaPlayer, "release", "()V");
    orxASSERT(sstSoundSystem.mIDMediaPlayer_release != orxNULL);
    sstSoundSystem.mIDMediaPlayer_setLooping = poJEnv->GetMethodID(sstSoundSystem.clsMediaPlayer, "setLooping", "(Z)V");
    orxASSERT(sstSoundSystem.mIDMediaPlayer_setLooping != orxNULL);
    sstSoundSystem.mIDMediaPlayer_getDuration = poJEnv->GetMethodID(sstSoundSystem.clsMediaPlayer, "getDuration", "()I");
    orxASSERT(sstSoundSystem.mIDMediaPlayer_getDuration != orxNULL);
    sstSoundSystem.mIDMediaPlayer_isPlaying = poJEnv->GetMethodID(sstSoundSystem.clsMediaPlayer, "isPlaying", "()Z");
    orxASSERT(sstSoundSystem.mIDMediaPlayer_isPlaying != orxNULL);
    sstSoundSystem.mIDMediaPlayer_setVolume = poJEnv->GetMethodID(sstSoundSystem.clsMediaPlayer, "setVolume", "(FF)V");
    orxASSERT(sstSoundSystem.mIDMediaPlayer_setVolume != orxNULL);

    /* set the systemfocus event handler */
    eResult = orxEvent_AddHandler(orxEVENT_TYPE_SYSTEM, orxSoundSystem_Android_EventHandler);
	}

	/* Done! */
	return eResult;
}
extern "C"  void orxFASTCALL orxSoundSystem_Android_Exit() {
	/* Was initialized? */
	if (sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) {

    /* set the systemfocus event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_SYSTEM, orxSoundSystem_Android_EventHandler);

    JNIEnv *poJEnv = NVThreadGetCurrentJNIEnv();
	  
	  /* release SoundPool resources */
		jclass clsSoundPool = poJEnv->FindClass("android/media/SoundPool");
		orxASSERT(clsSoundPool != orxNULL);
		jmethodID mIDSoundPool_release = poJEnv->GetMethodID(clsSoundPool, "release", "()V");
		orxASSERT(mIDSoundPool_release != orxNULL);
		poJEnv->CallVoidMethod(sstSoundSystem.oSoundPool, mIDSoundPool_release);

	  /* release global references */
	  poJEnv->DeleteGlobalRef(sstSoundSystem.oAssetManager);
	  poJEnv->DeleteGlobalRef(sstSoundSystem.oSoundPool);

		/* Cleans static controller */
		orxMemory_Zero(&sstSoundSystem, sizeof(orxSOUNDSYSTEM_STATIC));
	}

	return;
}

extern "C" orxSOUNDSYSTEM_SAMPLE *orxFASTCALL orxSoundSystem_Android_CreateSample(orxU32 _u32ChannelNumber, orxU32 _u32FrameNumber, orxU32 _u32SampleRate)
{
  orxSOUNDSYSTEM_SAMPLE *pstResult = orxNULL;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  //! TODO: Not implemented yet

  /* Done! */
  return pstResult;
}

extern "C" orxSOUNDSYSTEM_SAMPLE *orxFASTCALL orxSoundSystem_Android_LoadSample(const orxSTRING _zFilename) 
{
	orxSOUNDSYSTEM_SAMPLE *pstResult;

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
	orxASSERT(_zFilename != orxNULL);

  JNIEnv *poJEnv = NVThreadGetCurrentJNIEnv();

	jstring filenameString = poJEnv->NewStringUTF(_zFilename);
  orxASSERT(filenameString != orxNULL);
	jobject assetFd = poJEnv->CallObjectMethod(sstSoundSystem.oAssetManager, sstSoundSystem.mIDAssetManager_openFd, filenameString);
  orxASSERT(assetFd != orxNULL);
	jint soundId = poJEnv->CallIntMethod(sstSoundSystem.oSoundPool, sstSoundSystem.mIDSoundPool_load,	assetFd);
	
	if (soundId == 0) 
	{
		orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "can not load the file %s", _zFilename);
		pstResult = (orxSOUNDSYSTEM_SAMPLE *) orxNULL;
		return pstResult;
	}

	pstResult = (orxSOUNDSYSTEM_SAMPLE *) orxMemory_Allocate(sizeof(orxSOUNDSYSTEM_SAMPLE),	orxMEMORY_TYPE_MAIN);
	pstResult->soundId = soundId;

	/* Done! */
	return pstResult;
}

extern "C" orxSTATUS orxFASTCALL orxSoundSystem_Android_DeleteSample(orxSOUNDSYSTEM_SAMPLE *_pstSample)
{
	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
	orxASSERT(_pstSample != orxNULL);

  JNIEnv *poJEnv = NVThreadGetCurrentJNIEnv();

  /* unload */
  poJEnv->CallBooleanMethod(sstSoundSystem.oSoundPool, sstSoundSystem.mIDSoundPool_unload, _pstSample->soundId);

	/* Deletes it */
  orxMemory_Free(_pstSample);

	/* Done! */
	return orxSTATUS_SUCCESS;
}

extern "C" orxSTATUS orxFASTCALL orxSoundSystem_Android_GetSampleInfo(const orxSOUNDSYSTEM_SAMPLE *_pstSample, orxU32 *_pu32ChannelNumber, orxU32 *_pu32FrameNumber, orxU32 *_pu32SampleRate)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  //! TODO: Not implemented yet

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxSoundSystem_Android_SetSampleData(orxSOUNDSYSTEM_SAMPLE *_pstSample, const orxS16 *_as16Data, orxU32 _u32SampleNumber)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  //! TODO: Not implemented yet

  /* Done! */
  return eResult;
}

extern "C" orxSOUNDSYSTEM_SOUND *orxFASTCALL orxSoundSystem_Android_CreateFromSample(const orxSOUNDSYSTEM_SAMPLE *_pstSample)
{
	orxSOUNDSYSTEM_SOUND *pstResult;

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);orxASSERT(_pstSample != orxNULL);

	pstResult = (orxSOUNDSYSTEM_SOUND *) orxMemory_Allocate(sizeof(orxSOUNDSYSTEM_SOUND), orxMEMORY_TYPE_MAIN);
	pstResult->sampleBuffer = (orxSOUNDSYSTEM_SAMPLE *)_pstSample;
	pstResult->pitch = 1.0f;
	pstResult->volumn = 1.0f;
	pstResult->bLoop = orxFALSE;
	pstResult->bPaused = orxFALSE;
	pstResult->streamID = -1;
	pstResult->bIsUsingMediaPlayer = orxFALSE;

	/* Done! */
	return pstResult;
}

extern "C" orxSOUNDSYSTEM_SOUND *orxFASTCALL orxSoundSystem_Android_CreateStream(orxU32 _u32ChannelNumber, orxU32 _u32SampleRate, const orxSTRING _zReference)
{
  orxSOUNDSYSTEM_SOUND *pstResult = orxNULL;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  //! TODO: Not implemented yet

  /* Done! */
  return pstResult;
}

extern "C" orxSOUNDSYSTEM_SOUND *orxFASTCALL orxSoundSystem_Android_CreateStreamFromFile(const orxSTRING _zFilename, const orxSTRING _zReference)
{
	orxSOUNDSYSTEM_SOUND *pstResult;

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
	orxASSERT(_zFilename != orxNULL);

  JNIEnv *poJEnv = NVThreadGetCurrentJNIEnv();
	
	pstResult = (orxSOUNDSYSTEM_SOUND *) orxMemory_Allocate(sizeof(orxSOUNDSYSTEM_SOUND), orxMEMORY_TYPE_MAIN);

	jstring filenameString = poJEnv->NewStringUTF(_zFilename);
  orxASSERT(filenameString != orxNULL);
	jobject assetFd = poJEnv->CallObjectMethod(sstSoundSystem.oAssetManager, sstSoundSystem.mIDAssetManager_openFd, filenameString);
  orxASSERT(assetFd != orxNULL);
  pstResult->oAssetFileDescriptor = poJEnv->NewGlobalRef(assetFd);
  jobject fd = poJEnv->CallObjectMethod(assetFd, sstSoundSystem.mIDAssetFileDescriptor_getFileDescriptor);
  orxASSERT(fd != orxNULL);
  jlong offset = poJEnv->CallLongMethod(assetFd, sstSoundSystem.mIDAssetFileDescriptor_getStartOffset);
  jlong length = poJEnv->CallLongMethod(assetFd, sstSoundSystem.mIDAssetFileDescriptor_getLength);
  
  jobject oMediaPlayer = poJEnv->NewObject(sstSoundSystem.clsMediaPlayer, sstSoundSystem.mIDMediaPlayer_init);
  orxASSERT(oMediaPlayer != orxNULL);
  pstResult->oMediaPlayer = poJEnv->NewGlobalRef(oMediaPlayer);
  poJEnv->CallVoidMethod(pstResult->oMediaPlayer, sstSoundSystem.mIDMediaPlayer_setDataSource, fd, offset, length);
  poJEnv->CallVoidMethod(pstResult->oMediaPlayer, sstSoundSystem.mIDMediaPlayer_prepare);
	
	pstResult->sampleBuffer = orxNULL;
	pstResult->pitch = 1.0f;
	pstResult->volumn = 1.0f;
	pstResult->bLoop = orxFALSE;
	pstResult->bPaused = orxFALSE;
	pstResult->streamID = -1;
	pstResult->bIsUsingMediaPlayer = orxTRUE;

	/* Done! */
	return pstResult;
}

extern "C" orxSTATUS orxFASTCALL orxSoundSystem_Android_Delete(orxSOUNDSYSTEM_SOUND *_pstSound)
{
	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
	orxASSERT(_pstSound != orxNULL);

  JNIEnv *poJEnv = NVThreadGetCurrentJNIEnv();

  /* stop */
  if(_pstSound->bIsUsingMediaPlayer == orxTRUE)
  {
    poJEnv->CallVoidMethod(_pstSound->oMediaPlayer, sstSoundSystem.mIDMediaPlayer_release);
    poJEnv->CallVoidMethod(_pstSound->oAssetFileDescriptor, sstSoundSystem.mIDAssetFileDescriptor_close);
    poJEnv->DeleteGlobalRef(_pstSound->oMediaPlayer);
    poJEnv->DeleteGlobalRef(_pstSound->oAssetFileDescriptor);
  }
  else
  {
    poJEnv->CallVoidMethod(sstSoundSystem.oSoundPool, sstSoundSystem.mIDSoundPool_stop, _pstSound->streamID);
  }
  
	/* Deletes it */
	orxMemory_Free(_pstSound);

	/* Done! */
	return orxSTATUS_SUCCESS;
}

extern "C" orxSTATUS orxFASTCALL orxSoundSystem_Android_Play(orxSOUNDSYSTEM_SOUND *_pstSound)
{
	orxSTATUS eResult = orxSTATUS_FAILURE;

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
	orxASSERT(_pstSound != orxNULL);

  JNIEnv *poJEnv = NVThreadGetCurrentJNIEnv();

  /* play */
  if(_pstSound->bIsUsingMediaPlayer == orxTRUE)
  {
    orxASSERT(_pstSound->oMediaPlayer != orxNULL);
    poJEnv->CallVoidMethod(_pstSound->oMediaPlayer, sstSoundSystem.mIDMediaPlayer_start);
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
	  orxASSERT(_pstSound->sampleBuffer != orxNULL);
    jint streamID = poJEnv->CallIntMethod(sstSoundSystem.oSoundPool, sstSoundSystem.mIDSoundPool_play, _pstSound->sampleBuffer->soundId, _pstSound->volumn, _pstSound->volumn, 1, (_pstSound->bLoop ? -1 : 0), _pstSound->pitch);
    if(streamID != 0)
    {
      _pstSound->streamID = streamID;
      eResult = orxSTATUS_SUCCESS;
    }
  }
  
  _pstSound->bPaused = orxFALSE;
  
	/* Done! */
	return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxSoundSystem_Android_Pause(orxSOUNDSYSTEM_SOUND *_pstSound)
{
	orxSTATUS eResult = orxSTATUS_SUCCESS;

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
	orxASSERT(_pstSound != orxNULL);

  JNIEnv *poJEnv = NVThreadGetCurrentJNIEnv();
	
	_pstSound->bPaused = orxTRUE;

  /* pause */
  if(_pstSound->bIsUsingMediaPlayer == orxTRUE)
  {
    poJEnv->CallVoidMethod(_pstSound->oMediaPlayer, sstSoundSystem.mIDMediaPlayer_pause);
  }
  else
  {
    poJEnv->CallVoidMethod(sstSoundSystem.oSoundPool, sstSoundSystem.mIDSoundPool_pause, _pstSound->streamID);
  }
  
	/* Done! */
	return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxSoundSystem_Android_Stop(orxSOUNDSYSTEM_SOUND *_pstSound)
{
	orxSTATUS eResult = orxSTATUS_SUCCESS;

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
	orxASSERT(_pstSound != orxNULL);

  JNIEnv *poJEnv = NVThreadGetCurrentJNIEnv();

  /* stop */
  if(_pstSound->bIsUsingMediaPlayer == orxTRUE)
  {
    poJEnv->CallVoidMethod(_pstSound->oMediaPlayer, sstSoundSystem.mIDMediaPlayer_stop);
  }
  else
  {
    poJEnv->CallVoidMethod(sstSoundSystem.oSoundPool, sstSoundSystem.mIDSoundPool_stop, _pstSound->streamID);
  }

	/* Done! */
	return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxSoundSystem_Android_SetVolume(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fVolume)
{
	orxSTATUS eResult = orxSTATUS_SUCCESS;

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
	orxASSERT(_pstSound != orxNULL);

  JNIEnv *poJEnv = NVThreadGetCurrentJNIEnv();

	_pstSound->volumn = _fVolume;
	
  /* setVolume */
  if(_pstSound->bIsUsingMediaPlayer == orxTRUE)
  {
    poJEnv->CallVoidMethod(_pstSound->oMediaPlayer, sstSoundSystem.mIDMediaPlayer_setVolume, _fVolume, _fVolume);
  }
  else
  {
    poJEnv->CallVoidMethod(sstSoundSystem.oSoundPool, sstSoundSystem.mIDSoundPool_setVolume, _pstSound->streamID, _fVolume, _fVolume);
  }

	/* Done! */
	return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxSoundSystem_Android_SetPitch(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fPitch)
{
	orxSTATUS eResult = orxSTATUS_SUCCESS;

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
	orxASSERT(_pstSound != orxNULL);

  JNIEnv *poJEnv = NVThreadGetCurrentJNIEnv();

  /* setRate */
  if(_pstSound->bIsUsingMediaPlayer == orxTRUE)
  {
    eResult = orxSTATUS_FAILURE;
  }
  else
  {
    _pstSound->pitch = _fPitch;
    poJEnv->CallVoidMethod(sstSoundSystem.oSoundPool, sstSoundSystem.mIDSoundPool_setRate, _pstSound->streamID, _fPitch);
  }

	/* Done! */
	return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxSoundSystem_Android_SetPosition(orxSOUNDSYSTEM_SOUND *_pstSound, const orxVECTOR *_pvPosition)
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

extern "C" orxSTATUS orxFASTCALL orxSoundSystem_Android_SetAttenuation(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fAttenuation)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  //! TODO: Not implemented yet

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxSoundSystem_Android_SetReferenceDistance(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fDistance)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  //! TODO: Not implemented yet

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxSoundSystem_Android_Loop(orxSOUNDSYSTEM_SOUND *_pstSound, orxBOOL _bLoop)
{
	orxSTATUS eResult = orxSTATUS_SUCCESS;

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
	orxASSERT(_pstSound != orxNULL);

  JNIEnv *poJEnv = NVThreadGetCurrentJNIEnv();

  _pstSound->bLoop = _bLoop;
  
  /* loop */
  if(_pstSound->bIsUsingMediaPlayer == orxTRUE)
  {
    poJEnv->CallVoidMethod(_pstSound->oMediaPlayer, sstSoundSystem.mIDMediaPlayer_setLooping, _bLoop);
  }
  else
  {
    poJEnv->CallVoidMethod(sstSoundSystem.oSoundPool, sstSoundSystem.mIDSoundPool_setLoop, _pstSound->streamID, (_bLoop ? -1 : 0));
  }

	/* Done! */
	return eResult;
}

extern "C" orxFLOAT orxFASTCALL orxSoundSystem_Android_GetVolume(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
	orxASSERT(_pstSound != orxNULL);

	/* Done! */
	return _pstSound->volumn;
}

extern "C" orxFLOAT orxFASTCALL orxSoundSystem_Android_GetPitch(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
	orxFLOAT fResult;

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
	orxASSERT(_pstSound != orxNULL);

	fResult = _pstSound->pitch;

	/* Done! */
	return fResult;
}

extern "C" orxVECTOR *orxFASTCALL orxSoundSystem_Android_GetPosition(const orxSOUNDSYSTEM_SOUND *_pstSound, orxVECTOR *_pvPosition)
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

extern "C" orxFLOAT orxFASTCALL orxSoundSystem_Android_GetAttenuation(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  //! TODO: Not implemented yet

  /* Done! */
  return fResult;
}

extern "C" orxFLOAT orxFASTCALL orxSoundSystem_Android_GetReferenceDistance(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  //! TODO: Not implemented yet

  /* Done! */
  return fResult;
}

extern "C" orxBOOL orxFASTCALL orxSoundSystem_Android_IsLooping(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
	orxBOOL bResult = orxFALSE;

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
	orxASSERT(_pstSound != orxNULL);

  bResult = _pstSound->bLoop;

	/* Done! */
	return bResult;
}

extern "C" orxFLOAT orxFASTCALL orxSoundSystem_Android_GetDuration(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
	orxFLOAT fResult = orxFLOAT_0;

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
	orxASSERT(_pstSound != orxNULL);

  JNIEnv *poJEnv = NVThreadGetCurrentJNIEnv();
	
  if(_pstSound->bIsUsingMediaPlayer == orxTRUE)
  {
    fResult = poJEnv->CallFloatMethod(_pstSound->oMediaPlayer, sstSoundSystem.mIDMediaPlayer_getDuration);
  }

	/* Done! */
	return fResult;
}

extern "C" orxSOUNDSYSTEM_STATUS orxFASTCALL orxSoundSystem_Android_GetStatus(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
	orxSOUNDSYSTEM_STATUS eResult = orxSOUNDSYSTEM_STATUS_NONE;

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
	orxASSERT(_pstSound != orxNULL);
	
  JNIEnv *poJEnv = NVThreadGetCurrentJNIEnv();

	if(_pstSound->bIsUsingMediaPlayer == orxTRUE)
	{
	  jboolean playing = poJEnv->CallBooleanMethod(_pstSound->oMediaPlayer, sstSoundSystem.mIDMediaPlayer_isPlaying);
	  
	  if(playing == JNI_TRUE)
	  {
	  	eResult = orxSOUNDSYSTEM_STATUS_PLAY;
	  }
	  else
	  {
      if(_pstSound->bPaused == orxTRUE)
      {
        eResult = orxSOUNDSYSTEM_STATUS_PAUSE;
      }
	  	else
	  	{
        eResult = orxSOUNDSYSTEM_STATUS_STOP;
	  	}
	  }
	}
	else
	{
	  eResult = orxSOUNDSYSTEM_STATUS_PLAY;
	}

	/* Done! */
	return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxSoundSystem_Android_SetGlobalVolume(orxFLOAT _fVolume)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  //! not supported

  /* Done! */
  return eResult;
}

extern "C" orxFLOAT orxFASTCALL orxSoundSystem_Android_GetGlobalVolume()
{
  orxFLOAT fResult = orxFLOAT_0;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  //! not supported

  /* Done! */
  return fResult;
}

extern "C" orxSTATUS orxFASTCALL orxSoundSystem_Android_SetListenerPosition(const orxVECTOR *_pvPosition)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvPosition != orxNULL);

  //! TODO: Not implemented yet

  /* Done! */
  return eResult;
}

extern "C" orxVECTOR *orxFASTCALL orxSoundSystem_Android_GetListenerPosition(orxVECTOR *_pvPosition)
{
  orxVECTOR *pvResult = _pvPosition;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvPosition != orxNULL);

  //! TODO: Not implemented yet

  /* Done! */
  return pvResult;
}

extern "C" orxSTATUS orxFASTCALL orxSoundSystem_Android_StartRecording(const orxSTRING _zName, orxBOOL _bWriteToFile, orxU32 _u32SampleRate, orxU32 _u32ChannelNumber)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_zName != orxNULL);

  //! TODO: Not implemented yet

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxSoundSystem_Android_StopRecording()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  //! TODO: Not implemented yet

  /* Done! */
  return eResult;
}

extern "C" orxBOOL orxFASTCALL orxSoundSystem_Android_HasRecordingSupport()
{
  orxBOOL bResult = orxFALSE;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  //! TODO: Not implemented yet

  /* Done! */
  return bResult;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/
orxPLUGIN_USER_CORE_FUNCTION_START( SOUNDSYSTEM);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_Init, SOUNDSYSTEM, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_Exit, SOUNDSYSTEM, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_CreateSample, SOUNDSYSTEM,	CREATE_SAMPLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_LoadSample, SOUNDSYSTEM,	LOAD_SAMPLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_DeleteSample, SOUNDSYSTEM, DELETE_SAMPLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_GetSampleInfo, SOUNDSYSTEM, GET_SAMPLE_INFO);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_SetSampleData, SOUNDSYSTEM, SET_SAMPLE_DATA);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_CreateFromSample, SOUNDSYSTEM, CREATE_FROM_SAMPLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_CreateStream, SOUNDSYSTEM, CREATE_STREAM);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_CreateStreamFromFile, SOUNDSYSTEM, CREATE_STREAM_FROM_FILE);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_Delete, SOUNDSYSTEM,	DELETE);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_Play, SOUNDSYSTEM, PLAY);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_Pause,	SOUNDSYSTEM, PAUSE);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_Stop, SOUNDSYSTEM, STOP);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_SetVolume, SOUNDSYSTEM, SET_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_SetPitch, SOUNDSYSTEM,	SET_PITCH);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_SetPosition, SOUNDSYSTEM, SET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_SetAttenuation, SOUNDSYSTEM, SET_ATTENUATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_SetReferenceDistance, SOUNDSYSTEM, SET_REFERENCE_DISTANCE);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_Loop, SOUNDSYSTEM, LOOP);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_GetVolume, SOUNDSYSTEM, GET_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_GetPitch, SOUNDSYSTEM,	GET_PITCH);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_GetPosition, SOUNDSYSTEM, GET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_GetAttenuation, SOUNDSYSTEM, GET_ATTENUATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_GetReferenceDistance, SOUNDSYSTEM, GET_REFERENCE_DISTANCE);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_IsLooping, SOUNDSYSTEM, IS_LOOPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_GetDuration, SOUNDSYSTEM, GET_DURATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_GetStatus, SOUNDSYSTEM, GET_STATUS);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_SetGlobalVolume,	SOUNDSYSTEM, SET_GLOBAL_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_GetGlobalVolume,	SOUNDSYSTEM, GET_GLOBAL_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_SetListenerPosition,	SOUNDSYSTEM, SET_LISTENER_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_GetListenerPosition, SOUNDSYSTEM, GET_LISTENER_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_StartRecording, SOUNDSYSTEM, START_RECORDING);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_StopRecording, SOUNDSYSTEM, STOP_RECORDING);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Android_HasRecordingSupport, SOUNDSYSTEM, HAS_RECORDING_SUPPORT);
orxPLUGIN_USER_CORE_FUNCTION_END();
