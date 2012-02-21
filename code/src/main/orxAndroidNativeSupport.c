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
 * @file orxAndroidNativeSupport.c
 * @date 26/06/2011
 * @author simons.philippe@gmail.com
 *
 * Android native support
 *
 */

#include <android/log.h>
#include <android_native_app_glue.h>
#include <android/sensor.h>

#include "orxInclude.h"
#include "orxKernel.h"

#define  LOG_TAG    "orx"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static const char  *zOrxCommandLineKey = "orx.cmd_line";

orxS32              s32Animating;
struct android_app *pstApp;

ASensorManager     *poSensorManager;
const ASensor      *poAccelerometerSensor;
ASensorEventQueue  *poSensorEventQueue;
JNIEnv             *poJEnv;
jobject             oWakeLock;

orxU32              u32NbParams;
orxSTRING          *azParams;

/** Render inhibiter
 */
static orxSTATUS orxFASTCALL RenderInhibiter(const orxEVENT *_pstEvent)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

void orxAndroid_AttachThread()
{
  // retrieve the JNIEnv
  
  int error = (*pstApp->activity->vm)->AttachCurrentThread(pstApp->activity->vm, &poJEnv, NULL);
  if (error || !poJEnv)
  {
    LOGE("ERROR: could not attach thread to JVM!");
  }
}

void orxAndroid_DetachThread()
{
  (*pstApp->activity->vm)->DetachCurrentThread(pstApp->activity->vm);
}

void orxAndroid_AcquireWakeLock()
{
        /* Acquires wake lock */
        jclass contextClass = (*poJEnv)->FindClass(poJEnv, "android/content/Context");
        orxASSERT(contextClass != orxNULL);
        
        jmethodID getSystemService = (*poJEnv)->GetMethodID(poJEnv, contextClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
        orxASSERT(getSystemService != orxNULL);
        jfieldID POWER_SERVICE_ID = (*poJEnv)->GetStaticFieldID(poJEnv, contextClass, "POWER_SERVICE", "Ljava/lang/String;");
        orxASSERT(POWER_SERVICE_ID != orxNULL);
        jstring POWER_SERVICE = (*poJEnv)->GetStaticObjectField(poJEnv, contextClass, POWER_SERVICE_ID);
        orxASSERT(POWER_SERVICE != orxNULL);
        
        /* Retrieves the PowerManager service and wakelock */
        jobject powerManager = (*poJEnv)->CallObjectMethod(poJEnv, pstApp->activity->clazz, getSystemService, POWER_SERVICE);
        orxASSERT(powerManager != orxNULL);
        jclass powerManagerClass = (*poJEnv)->FindClass(poJEnv, "android/os/PowerManager");
        orxASSERT(powerManagerClass != orxNULL);
        jmethodID newWakeLock = (*poJEnv)->GetMethodID(poJEnv, powerManagerClass, "newWakeLock", "(ILjava/lang/String;)Landroid/os/PowerManager$WakeLock;");
        orxASSERT(newWakeLock != orxNULL);
        jfieldID SCREEN_BRIGHT_WAKE_LOCK_ID = (*poJEnv)->GetStaticFieldID(poJEnv, powerManagerClass, "SCREEN_BRIGHT_WAKE_LOCK", "I");
        orxASSERT(SCREEN_BRIGHT_WAKE_LOCK_ID != orxNULL);
        jfieldID ON_AFTER_RELEASE_ID = (*poJEnv)->GetStaticFieldID(poJEnv, powerManagerClass, "ON_AFTER_RELEASE", "I");
        orxASSERT(ON_AFTER_RELEASE_ID != orxNULL);
        jint SCREEN_BRIGHT_WAKE_LOCK = (*poJEnv)->GetStaticIntField(poJEnv, contextClass, SCREEN_BRIGHT_WAKE_LOCK_ID);
        jint ON_AFTER_RELEASE = (*poJEnv)->GetStaticIntField(poJEnv, contextClass, ON_AFTER_RELEASE_ID);
        jstring tag = (*poJEnv)->NewStringUTF(poJEnv, "orx");
        orxASSERT(tag != orxNULL);
        
        jobject wakelock = (*poJEnv)->CallObjectMethod(poJEnv, powerManager, newWakeLock, SCREEN_BRIGHT_WAKE_LOCK | ON_AFTER_RELEASE, tag);
        orxASSERT(wakelock != orxNULL);
        /* keep a global reference to release on Exit */
        oWakeLock = (*poJEnv)->NewGlobalRef(poJEnv, wakelock);

        /* acquire wake lock */
        jclass wakeLockClass = (*poJEnv)->GetObjectClass(poJEnv, wakelock);
        orxASSERT(wakeLockClass != orxNULL);
        jmethodID acquire = (*poJEnv)->GetMethodID(poJEnv, wakeLockClass, "acquire", "()V");
        orxASSERT(acquire != orxNULL);
        (*poJEnv)->CallVoidMethod(poJEnv, wakelock, acquire);
        
        jthrowable exc = (*poJEnv)->ExceptionOccurred(poJEnv);
        if(exc)
        {
          orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "java.lang.SecurityException: android.permission.WAKE_LOCK missing");
          (*poJEnv)->ExceptionDescribe(poJEnv);
          (*poJEnv)->ExceptionClear(poJEnv);
        }
}

void orxAndroid_ReleaseWakeLock()
{
        /* acquire wake lock */
        jclass wakeLockClass = (*poJEnv)->GetObjectClass(poJEnv, oWakeLock);
        orxASSERT(wakeLockClass != orxNULL);
        jmethodID release = (*poJEnv)->GetMethodID(poJEnv, wakeLockClass, "release", "()V");
        orxASSERT(release != orxNULL);
        (*poJEnv)->CallVoidMethod(poJEnv, oWakeLock, release);

        jthrowable exc = (*poJEnv)->ExceptionOccurred(poJEnv);
        if(exc)
        {
          orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "java.lang.SecurityException: android.permission.WAKE_LOCK missing");
          (*poJEnv)->ExceptionDescribe(poJEnv);
          (*poJEnv)->ExceptionClear(poJEnv);
        }
        
        /* delete global reference */
        (*poJEnv)->DeleteGlobalRef(poJEnv, oWakeLock);
        oWakeLock = orxNULL;
}

#define SKIP(p) while (*p && isspace (*p)) p++
#define WANT(p) *p && !isspace (*p)

/* Count the number of arguments. */

static int count_args (const char * input)
{
    const char * p;
    int argc = 0;
    p = input;
    while (*p) {
        SKIP (p);
        if (WANT (p)) {
            argc++;
            while (WANT (p))
                p++;
        }
    }
    return argc;
}

/* Copy each non-whitespace argument into its own allocated space. */

static int copy_args (const char * input, int argc, char ** argv)
{
    int i = 0;
    const char *p;

    p = input;
    while (*p) {
        SKIP (p);
        if (WANT (p)) {
            const char * end = p;
            char * copy;
            while (WANT (end))
                end++;
            copy = argv[i] = malloc (end - p + 1);
            if (! argv[i])
                return -1;
            while (WANT (p))
                *copy++ = *p++;
            *copy = 0;
            i++;
        }
    }
    if (i != argc) 
        return -1;
    return 0;
}

#undef SKIP
#undef WANT

static int argc_argv (const char * input, int * argc_ptr, char *** argv_ptr)
{
    int argc;
    char ** argv;

    argc = count_args (input);
    if (argc == 0)
        return -1;
    argv = malloc (sizeof (char *) * argc);
    if (! argv)
        return -1;
    if (copy_args (input, argc, argv) == -1)
        return -1;
    *argc_ptr = argc;
    *argv_ptr = argv;
    return 0;
}

void orxAndroid_GetMainArgs()
{
	jclass activityClass = (*poJEnv)->GetObjectClass(poJEnv, pstApp->activity->clazz);
	orxASSERT(activityClass != orxNULL);

	/* retrieve intent */
	jmethodID getIntent = (*poJEnv)->GetMethodID(poJEnv, activityClass, "getIntent", "()Landroid/content/Intent;");
	orxASSERT(getIntent != orxNULL);
	jobject intent = (*poJEnv)->CallObjectMethod(poJEnv, pstApp->activity->clazz, getIntent);
	orxASSERT(intent != orxNULL);
	jclass intentClass = (*poJEnv)->GetObjectClass(poJEnv, intent);
	orxASSERT(intentClass != orxNULL);
	
	/* retrieve componentName */
	jmethodID getComponent = (*poJEnv)->GetMethodID(poJEnv, intentClass, "getComponent", "()Landroid/content/ComponentName;");
	orxASSERT(getComponent != orxNULL);
	jobject componentName = (*poJEnv)->CallObjectMethod(poJEnv, intent, getComponent);
	orxASSERT(componentName != orxNULL);
	
	/* retrieve PackageManager */
	jmethodID getPackageManager = (*poJEnv)->GetMethodID(poJEnv, activityClass, "getPackageManager", "()Landroid/content/pm/PackageManager;");
	orxASSERT(getPackageManager != orxNULL);
	jobject packageManager = (*poJEnv)->CallObjectMethod(poJEnv, pstApp->activity->clazz, getPackageManager);
	orxASSERT(packageManager != orxNULL);
	jclass packageManagerClass = (*poJEnv)->GetObjectClass(poJEnv, packageManager);
	orxASSERT(packageManagerClass != orxNULL);
	
	/* retrieve GET_META_DATA static field */
	jclass abstractPackageManagerClass = (*poJEnv)->FindClass(poJEnv, "android/content/pm/PackageManager");
	orxASSERT(abstractPackageManagerClass != orxNULL);
	jfieldID GET_META_DATA_ID = (*poJEnv)->GetStaticFieldID(poJEnv, abstractPackageManagerClass, "GET_META_DATA", "I");
	orxASSERT(GET_META_DATA_ID != orxNULL);
	jint GET_META_DATA = (*poJEnv)->GetStaticIntField(poJEnv, abstractPackageManagerClass, GET_META_DATA_ID);
	
	/* retrive ActivityInfo */
	jmethodID getActivityInfo = (*poJEnv)->GetMethodID(poJEnv, packageManagerClass, "getActivityInfo", "(Landroid/content/ComponentName;I)Landroid/content/pm/ActivityInfo;");
	orxASSERT(getActivityInfo != orxNULL);
	jobject activityInfo = (*poJEnv)->CallObjectMethod(poJEnv, packageManager, getActivityInfo, componentName, GET_META_DATA);
	orxASSERT(activityInfo != orxNULL);
	jclass activityInfoClass = (*poJEnv)->GetObjectClass(poJEnv, activityInfo);
	orxASSERT(activityInfoClass != orxNULL);
	
	/* retrieve metaData Bundle */
	jfieldID metaDataID = (*poJEnv)->GetFieldID(poJEnv, activityInfoClass, "metaData", "Landroid/os/Bundle;");
	orxASSERT(metaDataID != orxNULL);
	jobject metaData = (*poJEnv)->GetObjectField(poJEnv, activityInfo, metaDataID);
	orxASSERT(metaData != orxNULL);
	jclass metaDataClass = (*poJEnv)->GetObjectClass(poJEnv, metaData);
	orxASSERT(metaDataClass != orxNULL);
	
	/* retrieve cmd_line String */
	jmethodID getString = (*poJEnv)->GetMethodID(poJEnv, metaDataClass, "getString", "(Ljava/lang/String;)Ljava/lang/String;");
	orxASSERT(getString != orxNULL);
	jstring key = (*poJEnv)->NewStringUTF(poJEnv, zOrxCommandLineKey);
	orxASSERT(key != orxNULL);
	jstring orx_cmd_line = (*poJEnv)->CallObjectMethod(poJEnv, metaData, getString, key);
	if(orx_cmd_line != orxNULL)
	{
    const char* cmd_line = (*poJEnv)->GetStringUTFChars(poJEnv, orx_cmd_line, NULL);
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "orx.cmd_line = %s", cmd_line);
    
    if (argc_argv (cmd_line, (int *) &u32NbParams, (char***) &azParams) == -1)
    {
      LOGE("Something went wrong.");
    }
    
    (*poJEnv)->ReleaseStringUTFChars(poJEnv, orx_cmd_line, cmd_line);
	}
	else
	{
	  u32NbParams = 0;
	  azParams = orxNULL;
	  LOGI("orx.cmd_line not defined");
	}
}

void orxAndroid_ReleaseMainArgs()
{
  int i;
  for(i = 0; i < u32NbParams; i++)
  {
    free(azParams[i]);
  }
  
  free(azParams);
}

static void engine_handle_cmd(struct android_app* app, int32_t cmd)
{
  struct engine* engine = (struct engine*)app->userData;
  switch (cmd)
  {
    case APP_CMD_INIT_WINDOW:
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "APP_CMD_INIT_WINDOW\n");
      // The window is being shown, get it ready.
      orxEvent_SendShort(orxEVENT_TYPE_DISPLAY, orxDISPLAY_EVENT_RESTORE_CONTEXT);
      break;
    case APP_CMD_TERM_WINDOW:
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "APP_CMD_TERM_WINDOW\n");
      // The window is being hidden or closed, clean it up.
      orxEvent_SendShort(orxEVENT_TYPE_DISPLAY, orxDISPLAY_EVENT_SAVE_CONTEXT);
      break;
    case APP_CMD_LOST_FOCUS:
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "APP_CMD_LOST_FOCUS\n");
      /* Sends event */
      if(orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_BACKGROUND) != orxSTATUS_FAILURE)
      {
        /* Adds render inhibiter */
        orxEvent_AddHandler(orxEVENT_TYPE_RENDER, RenderInhibiter);
      }
      break;
    case APP_CMD_GAINED_FOCUS:
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "APP_CMD_GAINED_FOCUS\n");
      /* Sends event */
      if(orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_FOREGROUND) != orxSTATUS_FAILURE)
      {
        /* Removes render inhibiter */
        orxEvent_RemoveHandler(orxEVENT_TYPE_RENDER, RenderInhibiter);
      }
      break;
    case APP_CMD_DESTROY:
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "APP_CMD_DESTROY\n");
      orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_CLOSE);
      break;
  }
}

/**
 * Process the next input event.
 */
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
    struct engine* engine = (struct engine*)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
    	orxSYSTEM_EVENT_PAYLOAD stPayload;
    	
      /* read event data */
      int32_t x = AMotionEvent_getX(event, 0);
      int32_t y = AMotionEvent_getY(event, 0);
      float pressure = AMotionEvent_getPressure(event, 0);

      /* Inits event's payload */
      orxMemory_Zero(&stPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
      stPayload.stTouch.fX = (orxFLOAT)x;
      stPayload.stTouch.fY = (orxFLOAT)y;
      stPayload.stTouch.fPressure = (orxFLOAT)pressure;

      int32_t action = AMotionEvent_getAction(event);
      switch(action)
      {
        case AMOTION_EVENT_ACTION_DOWN:
        {
          orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_BEGIN, orxNULL, orxNULL, &stPayload);
          break;
        }
        
        case AMOTION_EVENT_ACTION_UP:
        {
          orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_END, orxNULL, orxNULL, &stPayload);
          break;
        }
        
        case AMOTION_EVENT_ACTION_MOVE:
        {
          orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_MOVE, orxNULL, orxNULL, &stPayload);
          break;
        }

      }
      return 1;
    }
    return 0;
}

extern int main(int argc, char *argv[]);

void android_main(struct android_app *_pstApp)
{
  /* Makes sure glue isn't stripped */
  app_dummy();

  /* Inits app */
  pstApp                = _pstApp;
  pstApp->onAppCmd      = engine_handle_cmd;
  pstApp->onInputEvent  = engine_handle_input;

  /* Retrieves Java environment */
  orxAndroid_AttachThread();
  orxAndroid_GetMainArgs();

  #ifdef __orxDEBUG__

  LOGI("about to call main()");

  #endif

  main(u32NbParams, azParams);

  #ifdef __orxDEBUG__

  LOGI("main() returned");

  #endif

  orxAndroid_ReleaseMainArgs();
  orxAndroid_DetachThread();

  return;
}

