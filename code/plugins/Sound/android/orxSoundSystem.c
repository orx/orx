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
 * @file orxSoundSystem.cpp
 * @date 19/07/2008
 * @author iarwain@orx-project.org
 *
 * ANDROID sound system plugin implementation
 *
 */

#include "orxPluginAPI.h"

JavaVM* globalVM;
JNIEnv* globalEnv;

typedef struct MethodList {
	jclass cMediaPlayer;
	jclass cActivityInstance;

	jmethodID mInit;
	jmethodID mPlay;
	jmethodID mStop;
	jmethodID mUnLoad;
	jmethodID mLoad;
	jmethodID mPause;

	//only used in play music
	jmethodID mSetDataSource;
	jmethodID mSetLoop;
	jmethodID mGetDuration;
	jmethodID mIslooping;
	jmethodID mSetVolumn;
	//~

	orxBOOL isInited;

} MethodList;

/** Module flags
 */
#define orxSOUNDSYSTEM_KU32_STATIC_FLAG_NONE      0x00000000 /**< No flags */

#define orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY     0x00000001 /**< Ready flag */

#define orxSOUNDSYSTEM_KU32_STATIC_MASK_ALL       0xFFFFFFFF /**< All mask */

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/
/** Internal sample structure
 */
struct __orxSOUNDSYSTEM_SAMPLE_t {
	orxFLOAT fDuration;
	/**
	 * this object will set mediaplayer when the bIsUsingMediaPlayer is true
	 */
	jobject oPlayer;
	orxBOOL bIsUsingMediaPlayer;

	/**
	 * if the bIsUsingMediaPlayer is true, this ID is the media player ID
	 * otherwise it is returned by soundpool load
	 * two kinds of ID is totally distinct;
	 */
	int bufferID;

};
/** Internal sound structure
 */
struct __orxSOUNDSYSTEM_SOUND_t {
	/**
	 * no mater using soundpool or mediaplayer, they all have no
	 * soundID or streamID before it plays.
	 * But for soundpool there is a stream ID when play it.
	 */
	orxSOUNDSYSTEM_SAMPLE* sampleBuffer;
	int soundPoolStreamID;
	float pitch; //the rate in soundpool
	float volumn; // It accepts a value between 0.0 and 1.0, default being 1.0.
	// the default volumn is 1.0f in both of side
};

static MethodList soundpoolMethodList;
static MethodList mediaPlayerMethodList;

static char baseFolder[32];

/** Static structure
 */
typedef struct __orxSOUNDSYSTEM_STATIC_t {
	//	orxFLOAT fDimensionRatio; /**< Dimension ratio */
	//	orxFLOAT fRecDimensionRatio; /**< Reciprocal dimension ratio */
	//	sf::SoundBuffer *poDummyBuffer; /**< Dummy sound buffer to prevent ANDROID from crashing on MacOS X 10.4 */
	orxU32 u32Flags;
	jobject iSoundpool;

} orxSOUNDSYSTEM_STATIC;

/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxSOUNDSYSTEM_STATIC sstSoundSystem;

#ifdef __cplusplus
extern "C" {
#endif

//#ifdef __dylib__
//jint JNI_OnLoad(JavaVM* vm, void* reserved) {
//#endif
//#ifdef __stlib__
//jint JNI_OnStaticLoad_ORX(JavaVM* vm, void* reserved) {
//#endif
//globalVM = vm;
//jint result = -1;
//
//if ((*vm)->GetEnv(vm, (void **) &globalEnv, JNI_VERSION_1_2) != JNI_OK) {
//	orxLOG("sound orx jni load error");
//	return result;
//}
//
//return JNI_VERSION_1_2;
//}

static JNIEnv* GetEnv() {
	JNIEnv* env = NULL;
	if (globalVM)
		(*globalVM)->GetEnv(globalVM, (void**) &env, JNI_VERSION_1_2);
	return env;
}

//typedef struct __orxSOUNDSYSTEM_SOUND_t   orxSOUNDSYSTEM_SOUND;
//typedef struct __orxSOUNDSYSTEM_SAMPLE_t  orxSOUNDSYSTEM_SAMPLE;
/**
 * to stop the sound  and music after the game is pausing
 */
static orxSTATUS orxFASTCALL orxAndroid_SoundSystemEventHandler(
		const orxEVENT *_pstEvent) {
	orxSTATUS eResult = orxSTATUS_SUCCESS;

	/* Checks */
	orxASSERT(_pstEvent->eType == orxEVENT_TYPE_SYSTEM);
	/* Colliding? */
	if (_pstEvent->eID == orxSYSTEM_EVENT_BACKGROUND) {
		orxSOUND *pstSound;

		/* Gets first sound */
		pstSound = orxSOUND(orxStructure_GetFirst(orxSTRUCTURE_ID_SOUND));

		/* Non empty? */
		while (pstSound != orxNULL) {

			if (orxSound_IsLooping(pstSound)) {
				orxSound_Pause(pstSound);
			}

			/* Gets first sound */
			pstSound = orxSOUND(orxStructure_GetNext(pstSound));
		}
	}

	if (_pstEvent->eID == orxSYSTEM_EVENT_FOREGROUND) {
		orxSOUND *pstSound;

		/* Gets first sound */
		pstSound = orxSOUND(orxStructure_GetFirst(orxSTRUCTURE_ID_SOUND));

		/* Non empty? */
		while (pstSound != orxNULL) {
			if (orxSound_IsLooping(pstSound)) {
				orxSound_Play(pstSound);
			}

			/* Gets first sound */
			pstSound = orxSOUND(orxStructure_GetNext(pstSound));
		}
	}
	/* Done! */
	return eResult;
}

/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

orxSTATUS /*orxFASTCALL*/orxSoundSystem_ANDROID_Init() {
	orxSTATUS eResult = orxSTATUS_FAILURE;
	JNIEnv* env = GetEnv();
	globalEnv = env;

	/* Was already initialized? */
	if (!(sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY)) {
		orxFLOAT fRatio;

		/* Cleans static controller */
		orxMemory_Zero(&sstSoundSystem, sizeof(orxSOUNDSYSTEM_STATIC));

		/* Gets dimension ratio */
		orxConfig_PushSection(orxSOUNDSYSTEM_KZ_CONFIG_SECTION);
		//		fRatio = orxConfig_GetFloat(orxSOUNDSYSTEM_KZ_CONFIG_RATIO);
		//
		//		/* Valid? */
		//		if(fRatio > orxFLOAT_0)
		//		{
		//			/* Stores it */
		//			sstSoundSystem.fDimensionRatio = fRatio;
		//		}
		//		else
		//		{
		//			/* Stores default one */
		//			sstSoundSystem.fDimensionRatio = (orxFLOAT)orxSoundSystem::sfDefaultDimensionRatio;
		//		}
		//
		//		/* Stores reciprocal dimenstion ratio */
		//		sstSoundSystem.fRecDimensionRatio = orxFLOAT_1 / sstSoundSystem.fDimensionRatio;

		/**
		 * the ratio and DimensionRatio can not be set when using mediaplayer and soundpool
		 *
		 */

		/* Updates status */
		orxFLAG_SET(sstSoundSystem.u32Flags, orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY, orxSOUNDSYSTEM_KU32_STATIC_MASK_ALL);

		/* Pops config section */
		orxConfig_PopSection();

		jclass cls = (*env)->FindClass(env, "org/orxproject/lib/AnOrxActivity");
		mediaPlayerMethodList.cActivityInstance = cls;

		//assign the mediaPlayer and soundpool method list
		if (!mediaPlayerMethodList.isInited) {
			mediaPlayerMethodList.cMediaPlayer = (*env)->FindClass(env,
					"android/media/MediaPlayer");

			if (!mediaPlayerMethodList.cMediaPlayer) {
				orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "android.media.mediaplayer class is not found. Are you running at least 1.5 version?");
				return orxSTATUS_FAILURE;
			}
			mediaPlayerMethodList.cMediaPlayer = (*env)->NewGlobalRef(env,
					mediaPlayerMethodList.cMediaPlayer);
			mediaPlayerMethodList.mInit = (*env)->GetMethodID(env,
					mediaPlayerMethodList.cMediaPlayer, "<init>", "()V");

			mediaPlayerMethodList.mLoad = (*env)->GetMethodID(env,
					mediaPlayerMethodList.cMediaPlayer, "prepare", "()V");
			mediaPlayerMethodList.mUnLoad = (*env)->GetMethodID(env,
					mediaPlayerMethodList.cMediaPlayer, "release", "()V");
			//			mediaPlayerMethodList.mSetDataSource = (*env)->GetMethodID(env,
			//					mediaPlayerMethodList.cMediaPlayer, "setDataSource",
			//					"(Ljava/lang/String;)V");

			mediaPlayerMethodList.mSetDataSource = (*env)->GetStaticMethodID(
					env, cls, "setMediaPlayerDataSource",
					"(Landroid/media/MediaPlayer;Ljava/lang/String;)Z");

			mediaPlayerMethodList.mPlay = (*env)->GetMethodID(env,
					mediaPlayerMethodList.cMediaPlayer, "start", "()V");
			mediaPlayerMethodList.mStop = (*env)->GetMethodID(env,
					mediaPlayerMethodList.cMediaPlayer, "stop", "()V");

			mediaPlayerMethodList.mPause = (*env)->GetMethodID(env,
					mediaPlayerMethodList.cMediaPlayer, "pause", "()V");

			mediaPlayerMethodList.mGetDuration = (*env)->GetMethodID(env,
					mediaPlayerMethodList.cMediaPlayer, "getDuration", "()I");
			mediaPlayerMethodList.mSetLoop = (*env)->GetMethodID(env,
					mediaPlayerMethodList.cMediaPlayer, "setLooping", "(Z)V");
			mediaPlayerMethodList.mIslooping = (*env)->GetMethodID(env,
					mediaPlayerMethodList.cMediaPlayer, "isLooping", "()Z");
			mediaPlayerMethodList.mSetVolumn = (*env)->GetMethodID(env,
					mediaPlayerMethodList.cMediaPlayer, "setVolume", "(FF)V");

			mediaPlayerMethodList.isInited = orxTRUE;
		}

		if (!soundpoolMethodList.isInited) {
			soundpoolMethodList.cMediaPlayer = (*env)->FindClass(env,
					"android/media/SoundPool");

			if (!soundpoolMethodList.cMediaPlayer) {
				orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "android.media.mediaplayer class is not found. Are you running at least 1.5 version?");
				return orxSTATUS_FAILURE;
			}
			soundpoolMethodList.cMediaPlayer = (*env)->NewGlobalRef(env,
					soundpoolMethodList.cMediaPlayer);
			soundpoolMethodList.mInit = (*env)->GetMethodID(env,
					soundpoolMethodList.cMediaPlayer, "<init>", "(III)V");

			soundpoolMethodList.mLoad = (*env)->GetStaticMethodID(env, cls,
					"loadSound",
					"(Landroid/media/SoundPool;Ljava/lang/String;)I");

			//			soundpoolMethodList.mLoad = (*env)->GetMethodID(env,
			//								soundpoolMethodList.cMediaPlayer, "load",
			//								"(Ljava/lang/String;I)I");
			soundpoolMethodList.mUnLoad = (*env)->GetMethodID(env,
					soundpoolMethodList.cMediaPlayer, "unload", "(I)Z");

			soundpoolMethodList.mPlay = (*env)->GetMethodID(env,
					soundpoolMethodList.cMediaPlayer, "play", "(IFFIIF)I");
			soundpoolMethodList.mPause = (*env)->GetMethodID(env,
					soundpoolMethodList.cMediaPlayer, "pause", "(I)V");
			soundpoolMethodList.mStop = (*env)->GetMethodID(env,
					soundpoolMethodList.cMediaPlayer, "stop", "(I)V");

			soundpoolMethodList.isInited = orxTRUE;

			//build the soundpool instance
			sstSoundSystem.iSoundpool = (*env)->NewObject(env,
					soundpoolMethodList.cMediaPlayer,
					soundpoolMethodList.mInit, 20, 3, 0);

		}

		getcwd(baseFolder, 32);

		//set the systemfocus event handler
		orxEvent_AddHandler(orxEVENT_TYPE_SYSTEM,
				orxAndroid_SoundSystemEventHandler);

		/* Updates result */
		eResult = orxSTATUS_SUCCESS;
	}

	/* Done! */
	return eResult;
}
void orxFASTCALL orxSoundSystem_ANDROID_Exit() {
	/* Was initialized? */
	if (sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) {
		/* Deletes dummy buffer */
		//		delete sstSoundSystem.poDummyBuffer;

		/* Cleans static controller */
		orxMemory_Zero(&sstSoundSystem, sizeof(orxSOUNDSYSTEM_STATIC));

		mediaPlayerMethodList.isInited = orxFALSE;
		soundpoolMethodList.isInited = orxFALSE;
	}

	return;
}

orxSOUNDSYSTEM_SAMPLE *orxFASTCALL orxSoundSystem_ANDROID_LoadSample(
		const orxSTRING _zFilename) {
	orxSOUNDSYSTEM_SAMPLE *pstResult;

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);orxASSERT(_zFilename != orxNULL);

	static char filenamex[128] = { 0 };

	sprintf(filenamex, "%s", _zFilename);
	orxLOG("load sample %s\n", filenamex);

	jstring filenameStr = (*globalEnv)->NewStringUTF(globalEnv, filenamex);

	//	jint soundId = (*globalEnv)->CallNonvirtualIntMethod(globalEnv,
	//			sstSoundSystem.iSoundpool, soundpoolMethodList.cMediaPlayer,
	//			soundpoolMethodList.mLoad, filenameStr, 1);

	jint soundId = (*globalEnv)->CallStaticIntMethod(globalEnv,
			mediaPlayerMethodList.cActivityInstance, soundpoolMethodList.mLoad,
			sstSoundSystem.iSoundpool, filenameStr);
	//	mEnv->CallStaticVoidMethod(mActivityInstance, midFlipBuffers);
	if (soundId == 0) {
		orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "can not load the file %s", _zFilename);
		pstResult = (orxSOUNDSYSTEM_SAMPLE *) orxNULL;
		return pstResult;
	}

	pstResult = orxMemory_Allocate(sizeof(orxSOUNDSYSTEM_SAMPLE),
			orxMEMORY_TYPE_MAIN);
	pstResult->bufferID = soundId;
	orxLOG("the sample is %d %x\n", pstResult->bufferID, pstResult );

	pstResult->bIsUsingMediaPlayer = orxFALSE;
	//	usleep(5);


	/* Done! */
	return pstResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_ANDROID_UnloadSample(
		orxSOUNDSYSTEM_SAMPLE *_pstSample) {

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);orxASSERT(_pstSample != orxNULL);

	if (_pstSample->bIsUsingMediaPlayer) {
		(*globalEnv)->CallNonvirtualVoidMethod(globalEnv, _pstSample->oPlayer,
				mediaPlayerMethodList.cMediaPlayer,
				mediaPlayerMethodList.mUnLoad);
		orxMemory_Zero(_pstSample, sizeof(orxSOUNDSYSTEM_SAMPLE));
		orxMemory_Free(_pstSample);
		return orxSTATUS_SUCCESS;
	}

	(*globalEnv)->CallNonvirtualBooleanMethod(globalEnv,
			sstSoundSystem.iSoundpool, soundpoolMethodList.cMediaPlayer,
			soundpoolMethodList.mUnLoad, _pstSample->bufferID);

	/* Done! */
	return orxSTATUS_SUCCESS;
}

orxSOUNDSYSTEM_SOUND *orxFASTCALL orxSoundSystem_ANDROID_CreateFromSample(
		const orxSOUNDSYSTEM_SAMPLE *_pstSample) {
	orxSOUNDSYSTEM_SOUND *pstResult;

	orxLOG("sound orx create from sample %d %x", _pstSample->bufferID,_pstSample);
	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);orxASSERT(_pstSample != orxNULL);

	pstResult = orxMemory_Allocate(sizeof(orxSOUNDSYSTEM_SOUND),
			orxMEMORY_TYPE_MAIN);
	pstResult->sampleBuffer = _pstSample;
	pstResult->pitch = 1.0f;
	pstResult->volumn = 1.0f;

	/* Done! */
	return pstResult;
}

orxSOUNDSYSTEM_SOUND *orxFASTCALL orxSoundSystem_ANDROID_CreateStreamFromFile(
		const orxSTRING _zFilename, const orxSTRING _zReference) {
	orxSOUNDSYSTEM_SOUND *pstResult;

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);orxASSERT(_zFilename != orxNULL);

	static char filenamex[128] = { 0 };

	sprintf(filenamex, "%s", _zFilename);
	orxLOG("load sample from file %s\n", filenamex);

	jstring filenameStr = (*globalEnv)->NewStringUTF(globalEnv, filenamex);

	orxSOUNDSYSTEM_SAMPLE *pstSample = orxMemory_Allocate(
			sizeof(orxSOUNDSYSTEM_SAMPLE), orxMEMORY_TYPE_MAIN);

	pstSample->bIsUsingMediaPlayer = orxTRUE;
	pstSample->oPlayer = (*globalEnv)->NewObject(globalEnv,
			mediaPlayerMethodList.cMediaPlayer, mediaPlayerMethodList.mInit);

	//	(*globalEnv)->CallNonvirtualVoidMethod(globalEnv, pstSample->oPlayer,
	//			mediaPlayerMethodList.cMediaPlayer,
	//			mediaPlayerMethodList.mSetDataSource, filenameStr);
	(*globalEnv)->CallStaticBooleanMethod(globalEnv,
			mediaPlayerMethodList.cActivityInstance,
			mediaPlayerMethodList.mSetDataSource, pstSample->oPlayer,
			filenameStr);

	(*globalEnv)->CallNonvirtualVoidMethod(globalEnv, pstSample->oPlayer,
			mediaPlayerMethodList.cMediaPlayer, mediaPlayerMethodList.mLoad);
	pstSample->fDuration = (float) (*globalEnv)->CallNonvirtualIntMethod(
			globalEnv, pstSample->oPlayer, mediaPlayerMethodList.cMediaPlayer,
			mediaPlayerMethodList.mGetDuration) / 60000.0f;

	pstResult = orxMemory_Allocate(sizeof(orxSOUNDSYSTEM_SOUND),
			orxMEMORY_TYPE_MAIN);
	pstResult->sampleBuffer = pstSample;
	pstResult->pitch = 1.0f;
	pstResult->volumn = 1.0f;

	/* Done! */
	return pstResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_ANDROID_Delete(
		orxSOUNDSYSTEM_SOUND *_pstSound) {
	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);orxASSERT(_pstSound != orxNULL);

	orxLOG("sound orx delete from sample");
	/* Is a music? */
	if (_pstSound->sampleBuffer->bIsUsingMediaPlayer) {
		(*globalEnv)->CallNonvirtualVoidMethod(globalEnv,
				_pstSound->sampleBuffer->oPlayer,
				mediaPlayerMethodList.cMediaPlayer, mediaPlayerMethodList.mStop);
		(*globalEnv)->CallNonvirtualVoidMethod(globalEnv,
				_pstSound->sampleBuffer->oPlayer,
				mediaPlayerMethodList.cMediaPlayer,
				mediaPlayerMethodList.mUnLoad);
		orxMemory_Zero(_pstSound->sampleBuffer, sizeof(orxSOUNDSYSTEM_SAMPLE));
		orxMemory_Free(_pstSound->sampleBuffer);
	} else {
		/* Deletes its sound */
		(*globalEnv)->CallNonvirtualVoidMethod(globalEnv,
				sstSoundSystem.iSoundpool, soundpoolMethodList.cMediaPlayer,
				soundpoolMethodList.mStop, _pstSound->soundPoolStreamID);
	}

	/* Deletes it */
	orxMemory_Free(_pstSound);
	/* Done! */
	return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL orxSoundSystem_ANDROID_Play(
		orxSOUNDSYSTEM_SOUND *_pstSound) {
	orxSTATUS eResult = orxSTATUS_SUCCESS;

	orxLOG("sound orx play from sample %d %x",_pstSound->sampleBuffer->bufferID, _pstSound->sampleBuffer );
	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);orxASSERT(_pstSound != orxNULL);

	/* Is a music? */
	if (_pstSound->sampleBuffer->bIsUsingMediaPlayer) {

		(*globalEnv)->CallNonvirtualVoidMethod(globalEnv,
				_pstSound->sampleBuffer->oPlayer,
				mediaPlayerMethodList.cMediaPlayer, mediaPlayerMethodList.mPlay);
	} else {
		//set the volumn and pitch when calling play
		jint streamId = (*globalEnv)->CallNonvirtualIntMethod(globalEnv,
				sstSoundSystem.iSoundpool, soundpoolMethodList.cMediaPlayer,
				soundpoolMethodList.mPlay, _pstSound->sampleBuffer->bufferID,
				_pstSound->volumn, _pstSound->volumn, 1, 0, _pstSound->pitch);
		if (streamId == 0) {
			orxLOG("failure maybe it is not ready");
			return orxSTATUS_FAILURE;
		}
		_pstSound->soundPoolStreamID = streamId;
	}

	/* Done! */
	return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_ANDROID_Pause(
		orxSOUNDSYSTEM_SOUND *_pstSound) {
	orxSTATUS eResult = orxSTATUS_SUCCESS;

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);orxASSERT(_pstSound != orxNULL);

	/* Is a music? */
	if (_pstSound->sampleBuffer->bIsUsingMediaPlayer) {
		/* Pauses it */
		(*globalEnv)->CallNonvirtualVoidMethod(globalEnv,
				_pstSound->sampleBuffer->oPlayer,
				mediaPlayerMethodList.cMediaPlayer,
				mediaPlayerMethodList.mPause);
	} else {
		/* Pauses it */
		(*globalEnv)->CallNonvirtualVoidMethod(globalEnv,
				sstSoundSystem.iSoundpool, soundpoolMethodList.cMediaPlayer,
				soundpoolMethodList.mPause, _pstSound->soundPoolStreamID);
	}

	/* Done! */
	return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_ANDROID_Stop(
		orxSOUNDSYSTEM_SOUND *_pstSound) {
	orxSTATUS eResult = orxSTATUS_SUCCESS;

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);orxASSERT(_pstSound != orxNULL);

	/* Is a music? */
	if (_pstSound->sampleBuffer->bIsUsingMediaPlayer) {
		/* Pauses it */
		(*globalEnv)->CallNonvirtualVoidMethod(globalEnv,
				_pstSound->sampleBuffer->oPlayer,
				mediaPlayerMethodList.cMediaPlayer, mediaPlayerMethodList.mStop);
	} else {
		/* Pauses it */
		(*globalEnv)->CallNonvirtualVoidMethod(globalEnv,
				sstSoundSystem.iSoundpool, soundpoolMethodList.cMediaPlayer,
				soundpoolMethodList.mStop, _pstSound->soundPoolStreamID);
	}

	/* Done! */
	return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_ANDROID_SetVolume(
		orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fVolume) {
	orxSTATUS eResult = orxSTATUS_SUCCESS;

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);orxASSERT(_pstSound != orxNULL);

	_pstSound->volumn = _fVolume;
	if (_pstSound->sampleBuffer->bIsUsingMediaPlayer) {
		//set it immediately
		(*globalEnv)->CallNonvirtualVoidMethod(globalEnv,
				_pstSound->sampleBuffer->oPlayer,
				mediaPlayerMethodList.cMediaPlayer,
				mediaPlayerMethodList.mSetVolumn, _pstSound->volumn,
				_pstSound->volumn);
	} else {
		orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "it may not work at all. only before play the sound effect can set the volumn");
	}

	/* Done! */
	return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_ANDROID_SetPitch(
		orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fPitch) {
	orxSTATUS eResult = orxSTATUS_SUCCESS;

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

	if (_pstSound->sampleBuffer->bIsUsingMediaPlayer) {
		//orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "do not support set the pitch for music");
	} else {
		_pstSound->pitch = _fPitch;
	}

	/* Done! */
	return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_ANDROID_SetPosition(
		orxSOUNDSYSTEM_SOUND *_pstSound, const orxVECTOR *_pvPosition) {
	orxSTATUS eResult = orxSTATUS_SUCCESS;

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

	//orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "NO support Set Position");

	/* Done! */
	return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_ANDROID_SetAttenuation(
		orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fAttenuation) {
	orxSTATUS eResult = orxSTATUS_SUCCESS;

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);//orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "NO support Set Attenuation");

	/* Done! */
	return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_ANDROID_SetReferenceDistance(
		orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fDistance) {
	orxSTATUS eResult = orxSTATUS_SUCCESS;

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);//orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "NO support Set Reference Distance");

	/* Done! */
	return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_ANDROID_Loop(
		orxSOUNDSYSTEM_SOUND *_pstSound, orxBOOL _bLoop) {
	orxSTATUS eResult = orxSTATUS_SUCCESS;

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);orxASSERT(_pstSound != orxNULL);

	/* Is a music? */
	if (_pstSound->sampleBuffer->bIsUsingMediaPlayer) {

		(*globalEnv)->CallNonvirtualVoidMethod(globalEnv,
				_pstSound->sampleBuffer->oPlayer,
				mediaPlayerMethodList.cMediaPlayer,
				mediaPlayerMethodList.mSetLoop, _bLoop ? 1 : 0);
	} else {
		//orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "NO support set loop for sound effect");
	}

	/* Done! */
	return eResult;
}

orxFLOAT orxFASTCALL orxSoundSystem_ANDROID_GetVolume(
		const orxSOUNDSYSTEM_SOUND *_pstSound) {

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);orxASSERT(_pstSound != orxNULL);

	/* Done! */
	return _pstSound->volumn;
}

orxFLOAT orxFASTCALL orxSoundSystem_ANDROID_GetPitch(
		const orxSOUNDSYSTEM_SOUND *_pstSound) {
	orxFLOAT fResult = -1;

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
	if (_pstSound->sampleBuffer->bIsUsingMediaPlayer) {
		//orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "do not support set the pitch for music");
	} else {
		fResult = _pstSound->pitch;
	}
	/* Done! */
	return fResult;
}

orxVECTOR *orxFASTCALL orxSoundSystem_ANDROID_GetPosition(
		const orxSOUNDSYSTEM_SOUND *_pstSound, orxVECTOR *_pvPosition) {
	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);//orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "do not support get position, please wait for new version");
	/* Done! */
	return orxSTATUS_SUCCESS;
}

orxFLOAT orxFASTCALL orxSoundSystem_ANDROID_GetAttenuation(
		const orxSOUNDSYSTEM_SOUND *_pstSound) {
	orxFLOAT fResult;

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);//orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "NO support get Attenuation");

	/* Done! */
	return orxSTATUS_SUCCESS;
}

orxFLOAT orxFASTCALL orxSoundSystem_ANDROID_GetReferenceDistance(
		const orxSOUNDSYSTEM_SOUND *_pstSound) {
	orxFLOAT fResult;

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);//orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "NO support get Reference Distance");
	/* Done! */
	return orxSTATUS_SUCCESS;
}

orxBOOL orxFASTCALL orxSoundSystem_ANDROID_IsLooping(
		const orxSOUNDSYSTEM_SOUND *_pstSound) {
	orxBOOL bResult = orxFALSE;

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);orxASSERT(_pstSound != orxNULL);

	/* Is a music? */
	if (_pstSound->sampleBuffer->bIsUsingMediaPlayer) {
		bResult = (*globalEnv)->CallNonvirtualBooleanMethod(globalEnv,
				_pstSound->sampleBuffer->oPlayer,
				mediaPlayerMethodList.cMediaPlayer,
				mediaPlayerMethodList.mIslooping);
	} else {
		//orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "NO support query looping for sound effect");

	}

	/* Done! */
	return bResult;
}

orxFLOAT orxFASTCALL orxSoundSystem_ANDROID_GetDuration(
		const orxSOUNDSYSTEM_SOUND *_pstSound) {
	orxFLOAT fResult = 0;

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);orxASSERT(_pstSound != orxNULL);

	/* Is a music? */
	if (_pstSound->sampleBuffer->bIsUsingMediaPlayer) {
		fResult = _pstSound->sampleBuffer->fDuration;
	} else {
		//orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "NO support set loop for sound effect");
	}

	/* Done! */
	return fResult;
}

orxSOUNDSYSTEM_STATUS orxFASTCALL orxSoundSystem_ANDROID_GetStatus(
		const orxSOUNDSYSTEM_SOUND *_pstSound) {
	orxSOUNDSYSTEM_STATUS eResult;

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);orxASSERT(_pstSound != orxNULL);

	//	/* Is a music? */
	//	if (_pstSound->bIsMusic != false) {
	//		/* Depending on music status */
	//		switch (_pstSound->poMusic->GetStatus()) {
	//case		sf::Music::Playing:
	//		{
	//			/* Updates result */
	//			eResult = orxSOUNDSYSTEM_STATUS_PLAY;
	//
	//			break;
	//		}
	//
	//		case sf::Music::Paused:
	//		{
	//			/* Updates result */
	//			eResult = orxSOUNDSYSTEM_STATUS_PAUSE;
	//
	//			break;
	//		}
	//
	//		case sf::Music::Stopped:
	//		default:
	//		{
	//			/* Updates result */
	//			eResult = orxSOUNDSYSTEM_STATUS_STOP;
	//
	//			break;
	//		}
	//	}
	//}
	//else
	//{
	//	/* Depending on sound status */
	//	switch(_pstSound->poSound->GetStatus())
	//	{
	//		case sf::Sound::Playing:
	//		{
	//			/* Updates result */
	//			eResult = orxSOUNDSYSTEM_STATUS_PLAY;
	//
	//			break;
	//		}
	//
	//		case sf::Sound::Paused:
	//		{
	//			/* Updates result */
	//			eResult = orxSOUNDSYSTEM_STATUS_PAUSE;
	//
	//			break;
	//		}
	//
	//		case sf::Sound::Stopped:
	//		default:
	//		{
	//			/* Updates result */
	//			eResult = orxSOUNDSYSTEM_STATUS_STOP;
	//
	//			break;
	//		}
	//	}
	//}

	/* Done! */
	return eResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_ANDROID_SetGlobalVolume(orxFLOAT _fVolume) {
	orxSTATUS eResult = orxSTATUS_SUCCESS;

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
	//orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "NO support to set global volumn, you can use android system button to replace it ");
	/* Done! */
	return eResult;
}

orxFLOAT orxFASTCALL orxSoundSystem_ANDROID_GetGlobalVolume() {
	orxFLOAT fResult = 0;

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);//orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "NO support to Get global volumn, you can use android system button to replace it ");
	/* Done! */
	return fResult;
}

orxSTATUS orxFASTCALL orxSoundSystem_ANDROID_SetListenerPosition(
		const orxVECTOR *_pvPosition) {
	orxSTATUS eResult = orxSTATUS_SUCCESS;

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);//orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "NO support to set Listener Position ");

	/* Done! */
	return eResult;
}

orxVECTOR *orxFASTCALL orxSoundSystem_ANDROID_GetListenerPosition(
		orxVECTOR *_pvPosition) {
	orxVECTOR *pvResult = orxSTATUS_SUCCESS;

	/* Checks */
	orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);//orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "NO support to get Listener Position ");
	/* Done! */
	return pvResult;
}
/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/
orxPLUGIN_USER_CORE_FUNCTION_START( SOUNDSYSTEM)
		;
		orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_ANDROID_Init, SOUNDSYSTEM, INIT);
		orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_ANDROID_Exit, SOUNDSYSTEM, EXIT);
		orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_ANDROID_LoadSample, SOUNDSYSTEM,
				LOAD_SAMPLE);
		orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_ANDROID_UnloadSample,
				SOUNDSYSTEM, UNLOAD_SAMPLE);
		orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_ANDROID_CreateFromSample,
				SOUNDSYSTEM, CREATE_FROM_SAMPLE);
		orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_ANDROID_CreateStreamFromFile,
				SOUNDSYSTEM, CREATE_STREAM_FROM_FILE);
		orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_ANDROID_Delete, SOUNDSYSTEM,
				DELETE);
		orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_ANDROID_Play, SOUNDSYSTEM, PLAY);
		orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_ANDROID_Pause,
				SOUNDSYSTEM, PAUSE);
		orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_ANDROID_Stop, SOUNDSYSTEM, STOP);
		orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_ANDROID_SetVolume, SOUNDSYSTEM,
				SET_VOLUME);
		orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_ANDROID_SetPitch, SOUNDSYSTEM,
				SET_PITCH);
		orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_ANDROID_SetPosition, SOUNDSYSTEM,
				SET_POSITION);
		orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_ANDROID_SetAttenuation,
				SOUNDSYSTEM, SET_ATTENUATION);
		orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_ANDROID_SetReferenceDistance,
				SOUNDSYSTEM, SET_REFERENCE_DISTANCE);
		orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_ANDROID_Loop, SOUNDSYSTEM, LOOP);
		orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_ANDROID_GetVolume, SOUNDSYSTEM,
				GET_VOLUME);
		orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_ANDROID_GetPitch, SOUNDSYSTEM,
				GET_PITCH);
		orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_ANDROID_GetPosition, SOUNDSYSTEM,
				GET_POSITION);
		orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_ANDROID_GetAttenuation,
				SOUNDSYSTEM, GET_ATTENUATION);
		orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_ANDROID_GetReferenceDistance,
				SOUNDSYSTEM, GET_REFERENCE_DISTANCE);
		orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_ANDROID_IsLooping, SOUNDSYSTEM,
				IS_LOOPING);
		orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_ANDROID_GetDuration, SOUNDSYSTEM,
				GET_DURATION);
		orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_ANDROID_GetStatus, SOUNDSYSTEM,
				GET_STATUS);
		orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_ANDROID_SetGlobalVolume,
				SOUNDSYSTEM, SET_GLOBAL_VOLUME);
		orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_ANDROID_GetGlobalVolume,
				SOUNDSYSTEM, GET_GLOBAL_VOLUME);
		orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_ANDROID_SetListenerPosition,
				SOUNDSYSTEM, SET_LISTENER_POSITION);
		orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_ANDROID_GetListenerPosition,
				SOUNDSYSTEM, GET_LISTENER_POSITION);
		orxPLUGIN_USER_CORE_FUNCTION_END()
;

#ifdef __cplusplus
}
#endif
