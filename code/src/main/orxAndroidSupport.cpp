/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2012 Orx-Project
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
 * @file orxAndSupport.c
 * @date 26/06/2011
 * @author simons.philippe@gmail.com
 *
 * Android support
 *
 */

#if defined(__orxANDROID__)

#include <jni.h>
#include <android/log.h>

#define MODULE "orxAndroidSupport"
#define DEBUG(args...) __android_log_print(ANDROID_LOG_DEBUG, MODULE, ## args)

#include "orxInclude.h"
#include "orxKernel.h"

#include "main/orxAndroid.h"

int32_t s_winWidth = 1;
int32_t s_winHeight = 1;

/* Main function pointer */
orxMODULE_RUN_FUNCTION  pfnRun;

static orxSTATUS               seClockStatus, seMainStatus;
static orxSYSTEM_EVENT_PAYLOAD sstPayload;

static void renderFrame()
{
  /* Clears payload */
  orxMemory_Zero(&sstPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));

  /* Sends frame start event */
  orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_GAME_LOOP_START, orxNULL, orxNULL, &sstPayload);

  /* Runs the engine */
  seMainStatus = pfnRun();

  /* Updates clock system */
  seClockStatus = orxClock_Update();

  /* Sends frame stop event */
  orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_GAME_LOOP_STOP, orxNULL, orxNULL, &sstPayload);

  /* Updates frame counter */
  sstPayload.u32FrameCounter++;
}

static void nativeExit()
{
  /* Removes event handler */
  orxEvent_RemoveHandler(orxEVENT_TYPE_SYSTEM, orx_DefaultEventHandler);

  /* Exits from engine */
  orxModule_Exit(orxMODULE_ID_MAIN);

  /* Exits from all modules */
  orxModule_ExitAll();

  /* Exits from the Debug system */

  orxDEBUG_EXIT();
}

/** Should stop execution by default event handling?
 */
static orxBOOL sbStopByEvent = orxFALSE;

jobject oActivity;

/* Main function to call */
extern int main(int argc, char *argv[]);

extern "C" {

    jint JNI_OnLoad(JavaVM *vm, void *reserved)
    {
        orxAndroid_ThreadInit(vm);
        orxAndroid_APKInit();
        return JNI_VERSION_1_4;
    }

    JNIEXPORT void JNICALL Java_org_orx_lib_OrxActivity_nativeInit(JNIEnv * env, jobject thiz) {
      oActivity = env->NewGlobalRef(thiz);
    }

    JNIEXPORT void JNICALL Java_org_orx_lib_OrxRenderer_nativeExit(JNIEnv * env, jobject thiz) {
      nativeExit();
    }

    JNIEXPORT void JNICALL Java_org_orx_lib_OrxRenderer_nativeInit(JNIEnv* env, jobject thiz, jint width, jint height) {
        static int running = 0;
        s_winWidth = width;
        s_winHeight = height;

        if(running == 0)
        {
          running = 1;

          /* Call main function */
          main(0, orxNULL);
        }
    }

    JNIEXPORT void JNICALL Java_org_orx_lib_OrxRenderer_nativeRender(JNIEnv* env, jobject thiz) {
        renderFrame();
    }

    JNIEXPORT void JNICALL Java_org_orx_lib_OrxRenderer_nativeOnPause(JNIEnv* env, jobject thiz) {
    }

    JNIEXPORT void JNICALL Java_org_orx_lib_OrxRenderer_nativeOnResume(JNIEnv* env, jobject thiz) {
    }

    JNIEXPORT void JNICALL Java_org_orx_lib_OrxRenderer_nativeTouchesBegin(JNIEnv * env, jobject thiz, jint id, jfloat x, jfloat y) {
    }

    JNIEXPORT void JNICALL Java_org_orx_lib_OrxRenderer_nativeTouchesEnd(JNIEnv * env, jobject thiz, jint id, jfloat x, jfloat y) {
    }

    JNIEXPORT void JNICALL Java_org_orx_lib_OrxRenderer_nativeTouchesMove(JNIEnv * env, jobject thiz, jintArray ids, jfloatArray xs, jfloatArray ys) {
        int size = env->GetArrayLength(ids);
        jint id[size];
        jfloat x[size];
        jfloat y[size];

        env->GetIntArrayRegion(ids, 0, size, id);
        env->GetFloatArrayRegion(xs, 0, size, x);
        env->GetFloatArrayRegion(ys, 0, size, y);
    }

    JNIEXPORT void JNICALL Java_org_orx_lib_OrxRenderer_nativeTouchesCancel(JNIEnv * env, jobject thiz, jintArray ids, jfloatArray xs, jfloatArray ys) {
        int size = env->GetArrayLength(ids);
        jint id[size];
        jfloat x[size];
        jfloat y[size];

        env->GetIntArrayRegion(ids, 0, size, id);
        env->GetFloatArrayRegion(xs, 0, size, x);
        env->GetFloatArrayRegion(ys, 0, size, y);
    }

    #define KEYCODE_BACK 0x04
    #define KEYCODE_MENU 0x52

    JNIEXPORT jboolean JNICALL Java_org_orx_lib_OrxRenderer_nativeKeyDown(JNIEnv * env, jobject thiz, jint keyCode) {
        return JNI_FALSE;
    }

    JNIEXPORT jboolean JNICALL Java_org_orx_lib_OrxRenderer_nativeKeyUp(JNIEnv * env, jobject thiz, jint keyCode) {
        return JNI_FALSE;
    }

static JavaVM* s_vm = NULL;
static pthread_key_t s_jniEnvKey = 0;

void orxAndroid_ThreadInit(JavaVM* vm)
{
	s_vm = vm;
}

JNIEnv* orxAndroid_ThreadGetCurrentJNIEnv()
{
	JNIEnv* env = NULL;
    if (s_jniEnvKey)
	{
		env = (JNIEnv*)pthread_getspecific(s_jniEnvKey);
	}
	else
	{
		pthread_key_create(&s_jniEnvKey, NULL);
	}

	if (!env)
	{
		// do we have a VM cached?
		if (!s_vm)
		{
			__android_log_print(ANDROID_LOG_DEBUG, MODULE,  "Error - could not find JVM!");
			return NULL;
		}

		// Hmm - no env for this thread cached yet
		int error = s_vm->AttachCurrentThread(&env, NULL);
		__android_log_print(ANDROID_LOG_DEBUG, MODULE,  "AttachCurrentThread: %d, 0x%p", error, env);
		if (error || !env)
		{
			__android_log_print(ANDROID_LOG_DEBUG, MODULE,  "Error - could not attach thread to JVM!");
			return NULL;
		}

		pthread_setspecific(s_jniEnvKey, env);
	}

	return env;
}

typedef struct ThreadInitStruct
{
	void* m_arg;
	void *(*m_startRoutine)(void *);
} ThreadInitStruct;

static void* orxAndroid_ThreadSpawnProc(void* arg)
{
	ThreadInitStruct* init = (ThreadInitStruct*)arg;
	void *(*start_routine)(void *) = init->m_startRoutine;
	void* data = init->m_arg;
	void* ret;

	free(arg);

	orxAndroid_ThreadGetCurrentJNIEnv();

	ret = start_routine(data);

	if (s_vm)
		s_vm->DetachCurrentThread();

	return ret;
}

int orxAndroid_ThreadSpawnJNIThread(pthread_t *thread, pthread_attr_t const * attr,
    void *(*start_routine)(void *), void * arg)
{
	if (!start_routine)
		return -1;

	ThreadInitStruct* initData = (ThreadInitStruct*) malloc(sizeof(ThreadInitStruct));

	initData->m_startRoutine = start_routine;
	initData->m_arg = arg;

	int err = pthread_create(thread, attr, orxAndroid_ThreadSpawnProc, initData);

	// If the thread was not started, then we need to delete the init data ourselves
	if (err)
	{
		free(initData);
	}

	return err;
}

// on linuces, signals can interrupt sleep functions, so you might need to 
// retry to get the full sleep going. I'm not entirely sure this is necessary
// *here* clients could retry themselves when the exposed function returns
// nonzero
inline int __sleep(const struct timespec *req, struct timespec *rem)
{
	int ret = 1;
	int i;
	static const int sleepTries = 2;

	struct timespec req_tmp={0}, rem_tmp={0};

	rem_tmp = *req;
	for(i = 0; i < sleepTries; ++i)
	{
		req_tmp = rem_tmp;
		int ret = nanosleep(&req_tmp, &rem_tmp);
		if(ret == 0)
		{
			ret = 0;
			break;
		}
	}
	if(rem)
		*rem = rem_tmp;
	return ret;
}

int orxAndroid_ThreadSleep(unsigned long millisec)
{
    struct timespec req={0},rem={0};
    time_t sec  =(int)(millisec/1000);

    millisec     = millisec-(sec*1000);
    req.tv_sec  = sec;
    req.tv_nsec = millisec*1000000L;
    return __sleep(&req,&rem);
}

static jobject s_globalThiz;
static jclass APKFileClass;
static jclass fileHelper;
static jmethodID s_openFile;
static jmethodID s_closeFile;
static jfieldID s_lengthId;
static jmethodID s_seekFile;
static jfieldID s_positionId;
static jmethodID s_readFile;
static jfieldID s_dataId;

void orxAndroid_APKInit()
{
  __android_log_print(ANDROID_LOG_DEBUG, "apk",  "apk init\n");
  JNIEnv *env = orxAndroid_ThreadGetCurrentJNIEnv();
  __android_log_print(ANDROID_LOG_DEBUG, "apk",  "env = %p\n", env);
  fileHelper = env->FindClass("org/orx/lib/APKFileHelper");
  __android_log_print(ANDROID_LOG_DEBUG, "apk",  "class = %d\n", fileHelper);
  jmethodID getInstance = env->GetStaticMethodID(fileHelper, "getInstance", "()Lorg/orx/lib/APKFileHelper;");
  __android_log_print(ANDROID_LOG_DEBUG, "apk",  "inst = %d\n", getInstance);
  APKFileClass = env->FindClass("org/orx/lib/APKFileHelper$APKFile");
	
  s_openFile = env->GetMethodID(fileHelper, "openFileAndroid", "(Ljava/lang/String;)Lorg/orx/lib/APKFileHelper$APKFile;");
  s_closeFile = env->GetMethodID(fileHelper, "closeFileAndroid", "(Lorg/orx/lib/APKFileHelper$APKFile;)V");
  s_lengthId = env->GetFieldID(APKFileClass, "length", "I");
  s_seekFile = env->GetMethodID(fileHelper, "seekFileAndroid", "(Lorg/orx/lib/APKFileHelper$APKFile;I)J");
  s_positionId = env->GetFieldID(APKFileClass, "position", "I");
  s_readFile = env->GetMethodID(fileHelper, "readFileAndroid", "(Lorg/orx/lib/APKFileHelper$APKFile;I)V");
  s_dataId = env->GetFieldID(APKFileClass, "data", "[B");

  jobject thiz = env->CallStaticObjectMethod(fileHelper, getInstance);
  s_globalThiz = env->NewGlobalRef(thiz);
}

APKFile* orxAndroid_APKOpen(char const* path)
{
  JNIEnv *env = orxAndroid_ThreadGetCurrentJNIEnv();
  jstring test = env->NewStringUTF(path);
  jobject localfileHandle = env->CallObjectMethod(s_globalThiz, s_openFile, test);
  jobject fileHandle = env->NewGlobalRef(localfileHandle);
  env->DeleteLocalRef((jobject)test);
  env->DeleteLocalRef((jobject)localfileHandle);
    
  return (APKFile *) fileHandle;
}

void orxAndroid_APKClose(APKFile* file)
{
  JNIEnv *env = orxAndroid_ThreadGetCurrentJNIEnv();
  env->CallVoidMethod(s_globalThiz, s_closeFile, (jobject) file);
  env->DeleteGlobalRef((jobject)file);
}

int orxAndroid_APKGetc(APKFile *stream)
{
  char buff;
  orxAndroid_APKRead(&buff,1,1,stream);
  return buff;
}

char* orxAndroid_APKGets(char* s, int size, APKFile* stream)
{
  int i;
  char *d=s;
  for(i = 0; (size > 1) && (!orxAndroid_APKEOF(stream)); i++, size--, d++)
  {
    orxAndroid_APKRead(d,1,1,stream);
    if(*d==10)
    {
      size=1;
    }
  }
  *d=0;

  return s;
}

size_t orxAndroid_APKSize(APKFile* stream)
{
  JNIEnv *env = orxAndroid_ThreadGetCurrentJNIEnv();
  jint len = env->GetIntField((jobject) stream, s_lengthId);

  return len;
}

long orxAndroid_APKSeek(APKFile* stream, long offset, int type)
{
  JNIEnv *env = orxAndroid_ThreadGetCurrentJNIEnv();
  switch (type)
  {
  case SEEK_CUR:
    offset += orxAndroid_APKTell(stream);
    break;
  case SEEK_END:
    offset += orxAndroid_APKSize(stream);
    break;
  case SEEK_SET:
    // No need to change the offset..
    break;
  }

  jlong ret = env->CallLongMethod(s_globalThiz, s_seekFile, (jobject) stream, (jint) offset);

  return ret;
}

long orxAndroid_APKTell(APKFile* stream)
{
  JNIEnv *env = orxAndroid_ThreadGetCurrentJNIEnv();
  jint pos = env->GetIntField((jobject) stream, s_positionId);

  return pos;
}

size_t orxAndroid_APKRead(void* ptr, size_t size, size_t nmemb, APKFile* stream)
{
  JNIEnv *env = orxAndroid_ThreadGetCurrentJNIEnv();
  jint readLength = size*nmemb;

  int avail = orxAndroid_APKSize(stream)-orxAndroid_APKTell(stream);
  if(readLength>avail)
  {
    readLength = avail;
    nmemb = readLength/size;
  }

  env->CallVoidMethod(s_globalThiz, s_readFile, (jobject) stream, (jint) readLength);

  jbyteArray data = (jbyteArray) env->GetObjectField((jobject) stream, s_dataId);
  char *data2 = (char *) env->GetByteArrayElements(data, NULL);
  memcpy(ptr,data2,readLength);
  env->ReleaseByteArrayElements(data, (jbyte*) data2, 0);

  env->DeleteLocalRef(data);

  return nmemb;
}

int orxAndroid_APKEOF(APKFile *stream)
{
  int rv = (orxAndroid_APKTell(stream) >= orxAndroid_APKSize(stream)) ? 1 : 0;
  return rv;
}


}



#endif
