/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2013 Orx-Project
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

static JavaVM* s_vm = NULL;
static pthread_key_t s_jniEnvKey = 0;

int32_t s_winWidth = 1;
int32_t s_winHeight = 1;

/* Main function pointer */
orxMODULE_RUN_FUNCTION  pfnRun;

static orxSYSTEM_EVENT_PAYLOAD sstPayload;

static orxBOOL renderFrame()
{
  orxSTATUS eClockStatus, eMainStatus;

  /* Sends frame start event */
  orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_GAME_LOOP_START, orxNULL, orxNULL, &sstPayload);

  /* Runs the engine */
  eMainStatus = pfnRun();

  /* Updates clock system */
  eClockStatus = orxClock_Update();

  /* Sends frame stop event */
  orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_GAME_LOOP_STOP, orxNULL, orxNULL, &sstPayload);

  /* Updates frame counter */
  sstPayload.u32FrameCounter++;

  return ((eMainStatus == orxSTATUS_FAILURE) || (eClockStatus == orxSTATUS_FAILURE)) ? orxTRUE : orxFALSE;
}

static void nativeExit()
{
  /* Exits from engine */
  orxModule_Exit(orxMODULE_ID_MAIN);

  /* Exits from all modules */
  orxModule_ExitAll();

  /* Exits from the Debug system */

  orxDEBUG_EXIT();

  s_jniEnvKey = 0;
}

jobject oActivity;

/* Main function to call */
extern int main(int argc, char *argv[]);

static int isRunning;

extern "C" {

jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
  orxAndroid_ThreadInit(vm);
  return JNI_VERSION_1_4;
}

static void orxFASTCALL RequireDepthBuffer_Setup()
{
  // Adds module dependencies
  orxModule_AddDependency(orxMODULE_ID_MAIN, orxMODULE_ID_CONFIG);
}

static orxSTATUS orxFASTCALL RequireDepthBuffer_Init()
{
  return orxSTATUS_SUCCESS;
}

static void orxFASTCALL RequireDepthBuffer_Exit()
{
}

static orxBOOL RequireDepthBuffer_Run()
{
  orxBOOL bResult;

  orxConfig_PushSection("Display");
  bResult = orxConfig_GetBool("DepthBuffer");
  orxConfig_PopSection();

  return bResult;
}

JNIEXPORT jboolean JNICALL Java_org_orx_lib_OrxActivity_requireDepthBuffer(JNIEnv * env, jobject thiz)
{
  jboolean jbResult = JNI_FALSE;
  s_jniEnvKey = 0;

  // Inits the Debug System
  orxDEBUG_INIT();

  // Registers main module
  orxModule_Register(orxMODULE_ID_MAIN, RequireDepthBuffer_Setup, RequireDepthBuffer_Init, RequireDepthBuffer_Exit);

  // Registers all other modules
  orxModule_RegisterAll();

  // Calls all modules setup
  orxModule_SetupAll();

  // Inits the engine
  if(orxModule_Init(orxMODULE_ID_MAIN) != orxSTATUS_FAILURE)
  {
    jbResult = RequireDepthBuffer_Run() == orxTRUE ? JNI_TRUE : JNI_FALSE;

    // Exits from engine
    orxModule_Exit(orxMODULE_ID_MAIN);

    // Exits from all modules
    orxModule_ExitAll();
  }

  // Exits from the Debug system
  orxDEBUG_EXIT();

  s_jniEnvKey = 0;

  return jbResult;
}

JNIEXPORT void JNICALL Java_org_orx_lib_OrxActivity_nativeInit(JNIEnv * env, jobject thiz)
{
  isRunning = 0;
  oActivity = env->NewGlobalRef(thiz);
}

JNIEXPORT void JNICALL Java_org_orx_lib_OrxRenderer_nativeExit(JNIEnv * env, jobject thiz)
{
  nativeExit();
  env->DeleteGlobalRef(oActivity);
}

JNIEXPORT void JNICALL Java_org_orx_lib_OrxRenderer_nativeInit(JNIEnv* env, jobject thiz, jint width, jint height)
{
  s_winWidth = width;
  s_winHeight = height;

  if(isRunning == 0)
  {
    isRunning = 1;

    /* Clears payload */
    orxMemory_Zero(&sstPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));

    /* Call main function */
    main(0, orxNULL);
  }
}

JNIEXPORT jboolean JNICALL Java_org_orx_lib_OrxRenderer_nativeRender(JNIEnv* env, jobject thiz)
{
  return (renderFrame() == orxTRUE) ? JNI_TRUE : JNI_FALSE;
}

/** Render inhibiter
 */
static orxSTATUS orxFASTCALL RenderInhibiter(const orxEVENT *_pstEvent)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

JNIEXPORT void JNICALL Java_org_orx_lib_OrxRenderer_nativeOnPause(JNIEnv* env, jobject thiz)
{
  if(isRunning == 1)
  {
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "nativeOnPause");
    if(orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_BACKGROUND) != orxSTATUS_FAILURE)
    {
      /* Adds render inhibiter */
      orxEvent_AddHandler(orxEVENT_TYPE_RENDER, RenderInhibiter);
    }
  }
}

JNIEXPORT void JNICALL Java_org_orx_lib_OrxRenderer_nativeOnResume(JNIEnv* env, jobject thiz)
{
  if(isRunning == 1)
  {
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "nativeOnResume");
    if(orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_FOREGROUND) != orxSTATUS_FAILURE)
    {
      /* Removes render inhibiter */
      orxEvent_RemoveHandler(orxEVENT_TYPE_RENDER, RenderInhibiter);
    }
  }
}

JNIEXPORT void JNICALL Java_org_orx_lib_OrxRenderer_nativeTouchesBegin(JNIEnv * env, jobject thiz, jint id, jfloat x, jfloat y)
{
  orxSYSTEM_EVENT_PAYLOAD stPayload;

  /* Inits event's payload */
  orxMemory_Zero(&stPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
  stPayload.stTouch.fX = (orxFLOAT)x;
  stPayload.stTouch.fY = (orxFLOAT)y;
  stPayload.stTouch.fPressure = orxFLOAT_0;
  stPayload.stTouch.u32ID = id;

  orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_BEGIN, orxNULL, orxNULL, &stPayload);
}

JNIEXPORT void JNICALL Java_org_orx_lib_OrxRenderer_nativeTouchesEnd(JNIEnv * env, jobject thiz, jint id, jfloat x, jfloat y)
{
  orxSYSTEM_EVENT_PAYLOAD stPayload;

  /* Inits event's payload */
  orxMemory_Zero(&stPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
  stPayload.stTouch.fX = (orxFLOAT)x;
  stPayload.stTouch.fY = (orxFLOAT)y;
  stPayload.stTouch.fPressure = orxFLOAT_0;
  stPayload.stTouch.u32ID = id;

  orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_END, orxNULL, orxNULL, &stPayload);
}

JNIEXPORT void JNICALL Java_org_orx_lib_OrxRenderer_nativeTouchesMove(JNIEnv * env, jobject thiz, jintArray ids, jfloatArray xs, jfloatArray ys)
{
  int size = env->GetArrayLength(ids);
  jint id[size];
  jfloat x[size];
  jfloat y[size];

  env->GetIntArrayRegion(ids, 0, size, id);
  env->GetFloatArrayRegion(xs, 0, size, x);
  env->GetFloatArrayRegion(ys, 0, size, y);

  orxSYSTEM_EVENT_PAYLOAD stPayload;

  /* Inits event's payload */
  orxMemory_Zero(&stPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
  stPayload.stTouch.fPressure = orxFLOAT_0;

  for(int i = 0; i < size; i++)
  {
    stPayload.stTouch.fX = (orxFLOAT)x[i];
    stPayload.stTouch.fY = (orxFLOAT)y[i];
    stPayload.stTouch.u32ID = id[i];
    orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_MOVE, orxNULL, orxNULL, &stPayload);
  }
}

JNIEXPORT void JNICALL Java_org_orx_lib_OrxRenderer_nativeTouchesCancel(JNIEnv * env, jobject thiz, jintArray ids, jfloatArray xs, jfloatArray ys)
{
  int size = env->GetArrayLength(ids);
  jint id[size];
  jfloat x[size];
  jfloat y[size];

  env->GetIntArrayRegion(ids, 0, size, id);
  env->GetFloatArrayRegion(xs, 0, size, x);
  env->GetFloatArrayRegion(ys, 0, size, y);

  orxSYSTEM_EVENT_PAYLOAD stPayload;

  /* Inits event's payload */
  orxMemory_Zero(&stPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
  stPayload.stTouch.fPressure = orxFLOAT_0;

  for(int i = 0; i < size; i++)
  {
    stPayload.stTouch.fX = (orxFLOAT)x[i];
    stPayload.stTouch.fY = (orxFLOAT)y[i];
    stPayload.stTouch.u32ID = id[i];
    orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_END, orxNULL, orxNULL, &stPayload);
  }
}

JNIEXPORT jboolean JNICALL Java_org_orx_lib_OrxRenderer_nativeKeyDown(JNIEnv * env, jobject thiz, jint keyCode)
{
  jboolean result = JNI_TRUE;
  orxEVENT stEvent;

  orxEVENT_INIT(stEvent, (orxEVENT_TYPE)orxEVENT_TYPE_FIRST_RESERVED + orxANDROID_EVENT_KEYBOARD, orxANDROID_EVENT_KEYBOARD_DOWN, orxNULL, orxNULL, &keyCode);

  if(orxEvent_Send(&stEvent) != orxSTATUS_SUCCESS)
  {
    result = JNI_FALSE;
  }

  return result;
}

JNIEXPORT jboolean JNICALL Java_org_orx_lib_OrxRenderer_nativeKeyUp(JNIEnv * env, jobject thiz, jint keyCode)
{
  jboolean result = JNI_TRUE;
  orxEVENT stEvent;

  orxEVENT_INIT(stEvent, (orxEVENT_TYPE)orxEVENT_TYPE_FIRST_RESERVED + orxANDROID_EVENT_KEYBOARD, orxANDROID_EVENT_KEYBOARD_UP, orxNULL, orxNULL, &keyCode);

  if(orxEvent_Send(&stEvent) != orxSTATUS_SUCCESS)
  {
    result = JNI_FALSE;
  }

  return result;
}

JNIEXPORT void JNICALL Java_org_orx_lib_OrxAccelerometer_onSensorChanged(JNIEnv*  env, jobject thiz, jfloat x, jfloat y, jfloat z, jlong timeStamp) {
  orxSYSTEM_EVENT_PAYLOAD stAccelPayload;

  if(isRunning == 1)
  {
    orxMemory_Zero(&stAccelPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));

    stAccelPayload.stAccelerometer.dTime = orxFLOAT_0;
  
    /* Set acceleration vector */
    orxVector_Set(&stAccelPayload.stAccelerometer.vAcceleration,orx2F(x),orx2F(y),orx2F(z));

    /* Sends event */
    orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_ACCELERATE, orxNULL, orxNULL, &stAccelPayload);
  }
}

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

int orxAndroid_ThreadSpawnJNIThread(pthread_t *thread, pthread_attr_t const * attr, void *(*start_routine)(void *), void * arg)
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

}

#endif
