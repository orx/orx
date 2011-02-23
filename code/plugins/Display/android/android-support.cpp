/*******************************************************************************
 This file links the Java side of Android with liborx
 *******************************************************************************/
#include <jni.h>
#include <sys/time.h>
#include <time.h>
#include <android/log.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <orx.h>
#include "string.h"
#include "orx_apk_file.h"

static jobject assetManager;

#define DEBUG
#define ORX_JAVA_PACKAGE_PATH org_orxproject_lib_AnOrxActivity
#define ORX_JAVA_ACTITY_PATH org/orxproject/lib/AnOrxActivity

#ifndef ORX_JAVA_PACKAGE_PATH
#error You have to define ORX_JAVA_PACKAGE_PATH to your package path with dots replaced with underscores, for example "com_orx_example"
#endif
#define JAVA_EXPORT_NAME2(name,package) Java_##package##_##name
#define JAVA_EXPORT_NAME1(name,package) JAVA_EXPORT_NAME2(name,package)
#define JAVA_EXPORT_NAME(name) JAVA_EXPORT_NAME1(name,ORX_JAVA_PACKAGE_PATH)

#define _STR(s)     #s
#define STR(s)      _STR(s)

/***********************OPENAL init**************/
//extern "C" jint JNI_OnStaticLoad_openal(JavaVM* vm, void* reserved);
/*************************************************/

/*******************************************************************************
 Globals
 *******************************************************************************/
static long _getTime(void) {
	struct timeval now;
	gettimeofday(&now, NULL);
	return (long) (now.tv_sec * 1000 + now.tv_usec / 1000);
}

//Main activity
jclass mActivityInstance;

//method signatures
jmethodID midCreateGLContext;
jmethodID midFlipBuffers;
jmethodID midLoadImage;
jmethodID midSaveScreenImage;
jmethodID midRemoveImage;
jmethodID midStartLoop;
jmethodID midEndLoop;

char basefolder[32];
char appName[32];



extern "C" int orxMain(int argc, char *argv[]);
//extern "C" int Android_OnKeyDown(int keycode);
//extern "C" int Android_OnKeyUp(int keycode);
//extern "C" void Android_SetScreenResolution(int width, int height);

extern void ANDROID_OnResize(int iScreenWidth, int iScreenHeight);

extern void ANDROID_SetShaderSupport(orxBOOL shaderSupport);

extern void ANDROID_OnTouch(unsigned char action, unsigned int pointId, float x,
    float y, float p);

extern void ANDROID_OnAccel(void *accelEventPtr, float x, float y, float z);

//If we're not the active app, don't try to render
bool bRenderingEnabled = false;

//Feature IDs
static const int FEATURE_AUDIO = 1;
static const int FEATURE_ACCEL = 2;

jstring jAppPath = NULL;
const char* mainAppPathStr;

//Start up the ORX app
extern "C" void JAVA_EXPORT_NAME(nativeInit)(JNIEnv* env, jobject obj) {

	orxLOG( "orx init 0x%x", jAppPath);

	mEnv = env;
	bRenderingEnabled = true;
	//	ANDROID_createGLContext();
	jboolean isCopy = false;
//	const char* mainAppPathStr = mEnv->GetStringUTFChars(jAppPath,NULL);

	char* argv = (char*) malloc(64 * sizeof(argv));
	sprintf(argv, "/sdcard/%s", mainAppPathStr);
	orxLOG("the app path is %s", argv);
//	mEnv->ReleaseStringUTFChars(jAppPath, mainAppPathStr);

	orxMain(1, &argv);
	free(argv);
}

extern "C" void JAVA_EXPORT_NAME(nativeSetShaderSupport)(JNIEnv* env,
		jobject obj, bool bShaderSupport) {
	if (bShaderSupport) {
		ANDROID_SetShaderSupport( orxTRUE);
	} else {
		ANDROID_SetShaderSupport( orxFALSE);
	}
}

extern "C" void JAVA_EXPORT_NAME(nativeSendFocusEvent)(JNIEnv* env,
		jobject obj, int focusEventID) {

	orxLOG("send event id is %d", focusEventID);
	if (focusEventID == 0) {
		//send background
		if (orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_BACKGROUND)
				!= orxSTATUS_FAILURE) {
			/* Adds render inhibiter */
		}
	} else if (focusEventID == 1) {
		//send foreground
		if (orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_FOREGROUND)
				!= orxSTATUS_FAILURE) {
			/* Adds render inhibiter */
		}

	}
}

//Keydown
extern "C" void JAVA_EXPORT_NAME(onNativeKeyDown)(JNIEnv* env, jobject obj,
		jint keycode) {

	//	int r = Android_OnKeyDown(keycode);
#ifdef DEBUG
	orxLOG(
			"ORX: native key down %d\n", keycode);
#endif

}

//Keyup
extern "C" void JAVA_EXPORT_NAME(onNativeKeyUp)(JNIEnv* env, jobject obj,
		jint keycode) {

	//	int r = Android_OnKeyUp(keycode);
#ifdef DEBUG
	orxLOG(
			"ORX: native key up %d\n", keycode);
#endif

}

//Touch
extern "C" void JAVA_EXPORT_NAME(onNativeTouch)(JNIEnv* env, jobject obj,
		jint action, int pointId, jfloat x, jfloat y, jfloat p) {

#ifdef DEBUG
	//	orxLOG( "ORX",
	//			"ORX: native touch event %d @ %f/%f, pressure %f\n",
	//			action, x, y, p);
#endif

	//TODO: Pass this off to the ORX multitouch stuff


	ANDROID_OnTouch((unsigned int) action, (unsigned int) pointId, x, y, p);

}

//Quit
extern "C" void JAVA_EXPORT_NAME(nativeQuit)(JNIEnv* env, jobject obj) {

	//Stop rendering as we're no longer in the foreground
	bRenderingEnabled = false;

	//Inject a ORX_QUIT event
	orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_CLOSE);

	orxLOG( "ORX: Native quit");
}

//Resize
extern "C" void JAVA_EXPORT_NAME(onNativeResize)(JNIEnv* env, jobject obj,
		jint width, jint height) {
	orxLOG(
			"ORX: Set screen size on init: %d/%d\n", width, height);
	ANDROID_OnResize(width, height);
}

extern "C" void JAVA_EXPORT_NAME(onNativeAccel)(JNIEnv* env, jobject obj,
		jfloat x, jfloat y, jfloat z) {
	//	fLastAccelerometer[0] = x;
	//	fLastAccelerometer[1] = y;
	//	fLastAccelerometer[2] = z;
	void *objPtr = (void *) obj;
	ANDROID_OnAccel(objPtr, x, y, z);
}

extern "C" void JAVA_EXPORT_NAME(nativeSetAssetManager)(JNIEnv* env,
		jclass clazz, jobject _assetManager) {
	assetManager = _assetManager;
}

extern "C" void JAVA_EXPORT_NAME(nativeSetMainAppPath)(JNIEnv* env,
		jclass clazz, jstring mainAppPath) {
	jAppPath = mainAppPath;
	mainAppPathStr = mEnv->GetStringUTFChars(jAppPath,NULL);
}

/*******************************************************************************
 Functions called by ORX into Java
 *******************************************************************************/
static orxSTATUS orxFASTCALL orxAndroid_DefaultEventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(_pstEvent->eType == orxEVENT_TYPE_SYSTEM);

  /* Depending on event ID */
  switch(_pstEvent->eID)
  {
    /* Frame start */
    case orxSYSTEM_EVENT_GAME_LOOP_START:
    {
      mEnv->CallStaticIntMethod(mActivityInstance, midStartLoop);

      break;
    }

    /* Frame stop */
    case orxSYSTEM_EVENT_GAME_LOOP_STOP:
    {
      mEnv->CallStaticIntMethod(mActivityInstance, midEndLoop);

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

orxBOOL ANDROID_createGLContext() {
	orxLOG( "ORX: ORX_create_context()\n");

  /* Registers default event handler */
  orxEvent_AddHandler(orxEVENT_TYPE_SYSTEM, orxAndroid_DefaultEventHandler);

	bRenderingEnabled = true;

	mEnv->CallStaticVoidMethod(mActivityInstance, midCreateGLContext);

	//no use just a test could be deleted

	return true; //TODO always return true; will change
}

void ANDROID_GL_SwapBuffer() {

	if (!bRenderingEnabled) {
		return;
	}

	//When we get here, we've accumulated a full frame
	mEnv->CallStaticVoidMethod(mActivityInstance, midFlipBuffers);
}

jobject ANDROID_loadImage(JNIEnv** env,
		const orxSTRING filename) {
	*env = mEnv;
	static char filenamex[128];
	sprintf(filenamex, "%s", filename);
	orxLOG("image path is %s\n", filenamex );
	jstring filePath = mEnv->NewStringUTF(filenamex);
	return mEnv->CallStaticObjectMethod(mActivityInstance, midLoadImage,
			filePath);
}

void ANDROID_removeImage() {
	mEnv->CallStaticVoidMethod(mActivityInstance, midRemoveImage);
}

void ANDROID_saveScreenImage(const orxSTRING filename,
		orxBOOL bPNG) {
	jstring filenamex = mEnv->NewStringUTF(filename);
	mEnv->CallStaticVoidMethod(mActivityInstance, midSaveScreenImage,
			filenamex, bPNG != 0);
}

/*******************************************************************************
 Functions called by JNI
 *******************************************************************************/

//define in sound plugin
extern JavaVM* globalVM;
extern JNIEnv* globalEnv;

//Library init
extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved) {

	JNIEnv* env = NULL;
	jint result = -1;

	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
		return result;
	}

	mEnv = env;

	globalEnv = mEnv;
	globalVM = vm;

	orxLOG( "JNI: OnLoad class is %s", STR(ORX_JAVA_ACTITY_PATH));

	//converte the ORX_JAVA_PACKAGE_PATH into string(the name of the method)
	jclass cls = mEnv->FindClass(STR(ORX_JAVA_ACTITY_PATH));
	mActivityInstance = cls;
	midCreateGLContext = mEnv->GetStaticMethodID(cls, "createGLContext", "()V");
	midFlipBuffers = mEnv->GetStaticMethodID(cls, "flipBuffers", "()V");
	orxLOG( "ORX:ok other\n");
	//	midUpdateAudio = mEnv->GetStaticMethodID(cls, "updateAudio", "([B)V");
	midLoadImage = mEnv->GetStaticMethodID(cls, "loadImage",
			"(Ljava/lang/String;)Ljava/lang/Object;");
	orxLOG( "ORX: loadImage\n");
	midSaveScreenImage = mEnv->GetStaticMethodID(cls, "saveScreenImage",
			"(Ljava/lang/String;Z)V");
	orxLOG( "ORX: saveScreenImage\n");
	midRemoveImage = mEnv->GetStaticMethodID(cls, "removeImage", "()V");
	orxLOG( "ORX: removeImage\n");
	midStartLoop = mEnv->GetStaticMethodID(cls, "startLoop", "()I");
	midEndLoop = mEnv->GetStaticMethodID(cls, "endLoop", "()I");

	if (!midCreateGLContext || !midFlipBuffers || !midLoadImage
			|| !midSaveScreenImage || !midStartLoop || !midEndLoop) {
		orxLOG( "ORX: Bad mids\n");
	} else {
#ifdef DEBUG
		orxLOG( "ORX: Good mids\n");
#endif
	}

	//init the asset
	OrxAPKInit();




//	ANDROID_GL_SwapBuffer = ANDROID_GL_SwapBuffer_impl;
//	ANDROID_createGLContext = ANDROID_createGLContext_impl;
//	ANDROID_loadImage = ANDROID_loadImage_impl;
//	ANDROID_removeImage = ANDROID_removeImage_impl;
//	ANDROID_saveScreenImage = ANDROID_saveScreenImage_impl;

	//	JNI_OnStaticLoad_openal(vm, reserved);
	//	globalEnv = mEnv;
	orxLOG("sound orx jni load end %x  %x ", globalEnv, mEnv);

	return JNI_VERSION_1_4;
}
