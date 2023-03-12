/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2022 Orx-Project
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
 * @file orxAndroidSupport.cpp
 * @date 26/06/2011
 * @author simons.philippe@gmail.com
 *
 * Android support
 *
 */
#if defined(TARGET_OS_ANDROID)

#include <android/log.h>
#include <sys/system_properties.h>

#ifdef __orxDEBUG__

#define MODULE "orxAndroidSupport"
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,MODULE,__VA_ARGS__)
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,MODULE,__VA_ARGS__)
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,MODULE,__VA_ARGS__)
#define LOGV(...)  __android_log_print(ANDROID_LOG_VERBOSE,MODULE,__VA_ARGS__)

#else

#define LOGE(...)
#define LOGD(...)
#define LOGI(...)
#define LOGV(...)

#endif /* __orxDEBUG__ */

#include "orxInclude.h"
#include "orxKernel.h"
#include "orxAndroid.h"
#include "orxAndroidActivity.h"

/** Defines
 */
#define orxANDROID_KU32_ARGUMENT_BUFFER_SIZE    256    /**< Argument buffer size */
#define orxANDROID_KU32_MAX_ARGUMENT_COUNT      16     /**< Maximum number of arguments */

#define GET_ACTION_INDEX(ACTION)  (((ACTION) & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT)
#define GET_AXIS_X(EV, INDEX)     GameActivityPointerAxes_getX(&(EV)->pointers[INDEX])
#define GET_AXIS_Y(EV, INDEX)     GameActivityPointerAxes_getY(&(EV)->pointers[INDEX])

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxANDROID_STATIC_t
{
  orxBOOL bPaused;
  orxBOOL bHasFocus;
  orxFLOAT fSurfaceScale;
  uint64_t activeAxisIds;
  orxCHAR zArguments[orxANDROID_KU32_ARGUMENT_BUFFER_SIZE];

  struct android_app *app;
} orxANDROID_STATIC;

/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

static orxANDROID_STATIC sstAndroid;
static pthread_key_t sThreadKey;
static JavaVM *jVM;

/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

static JNIEnv *orxAndroid_JNI_GetEnv()
{
  /* From http://developer.android.com/guide/practices/jni.html
   * All threads are Linux threads, scheduled by the kernel.
   * They're usually started from managed code (using Thread.start), but they can also be created elsewhere and then
   * attached to the JavaVM. For example, a thread started with pthread_create can be attached with the
   * JNI AttachCurrentThread or AttachCurrentThreadAsDaemon functions. Until a thread is attached, it has no JNIEnv,
   * and cannot make JNI calls.
   * Attaching a natively-created thread causes a java.lang.Thread object to be constructed and added to the "main"
   * ThreadGroup, making it visible to the debugger. Calling AttachCurrentThread on an already-attached thread
   * is a no-op.
   * Note: You can call this function any number of times for the same thread, there's no harm in it
   */

  JNIEnv *env;
  int status = jVM->AttachCurrentThread(&env, NULL);
  if(status < 0)
  {
    LOGE("failed to attach current thread");
    return NULL;
  }

  return env;
}

static void orxAndroid_JNI_ThreadDestroyed(void *value)
{
  /* The thread is being destroyed, detach it from the Java VM and set the mThreadKey value to NULL as required */
  JNIEnv *env = (JNIEnv*)value;
  if(env != NULL)
  {
    jVM->DetachCurrentThread();
    pthread_setspecific(sThreadKey, NULL);
  }
}

static orxS32 orxAndroid_GetSdkVersion()
{
  orxS32 s32Version;

  char sdkVersion[PROP_VALUE_MAX + 1];
  int bufferLen = __system_property_get("ro.build.version.sdk", sdkVersion);

  if(bufferLen > 0 && orxString_ToS32Base(sdkVersion, 10, &s32Version, orxNULL) != orxSTATUS_FAILURE)
  {
    return s32Version;
  }

  return 0;
}

static jobject orxAndroid_JNI_getDisplay(JNIEnv *env)
{
  jobject display;
  jobject instance = sstAndroid.app->activity->javaGameActivity;

  env->PushLocalFrame(16);

  /* Note : WindowManager.getDefaultDisplay() was deprecated in Android R */
  if (orxAndroid_GetSdkVersion() >= __ANDROID_API_R__)
  {
    /* Finds classes */
    jclass contextClass = env->FindClass("android/content/Context");

    /* Finds methods */
    jmethodID getDisplayMethod = env->GetMethodID(contextClass, "getDisplay", "()Landroid/view/Display;");

    /* Calls methods and stores display object */
    display = env->CallObjectMethod(instance, getDisplayMethod);
  }
  else
  {
    /* Finds classes */
    jclass activityClass = env->GetObjectClass(instance);
    jclass windowManagerClass = env->FindClass("android/view/WindowManager");

    /* Finds methods */
    jmethodID getWindowManagerMethod = env->GetMethodID(activityClass, "getWindowManager", "()Landroid/view/WindowManager;");
    jmethodID getDefaultDisplayMethod = env->GetMethodID(windowManagerClass,"getDefaultDisplay", "()Landroid/view/Display;");

    /* Calls methods and stores display object */
    jobject windowManager = env->CallObjectMethod(instance, getWindowManagerMethod);
    display = env->CallObjectMethod(windowManager, getDefaultDisplayMethod);
  }

  /* Frees all the local references except display object */
  return env->PopLocalFrame(display);
}

static jobject orxAndroid_JNI_getActivityMetaData(JNIEnv *env)
{
  jobject instance = sstAndroid.app->activity->javaGameActivity;

  env->PushLocalFrame(16);

  /* Finds classes */
  jclass activityClass = env->GetObjectClass(instance);
  jclass activityInfoClass = env->FindClass("android/content/pm/ActivityInfo");
  jclass packageManagerClass = env->FindClass("android/content/pm/PackageManager");

  /* Finds methods */
  jmethodID getPackageManagerMethod = env->GetMethodID(activityClass, "getPackageManager", "()Landroid/content/pm/PackageManager;");
  jmethodID getComponentNameMethod = env->GetMethodID(activityClass, "getComponentName", "()Landroid/content/ComponentName;");
  jmethodID getActivityInfoMethod = env->GetMethodID(packageManagerClass, "getActivityInfo", "(Landroid/content/ComponentName;I)Landroid/content/pm/ActivityInfo;");

  /* Finds fields */
  jfieldID metaDataConstant = env->GetStaticFieldID(packageManagerClass, "GET_META_DATA", "I");
  jfieldID metaDataField = env->GetFieldID(activityInfoClass, "metaData", "Landroid/os/Bundle;");

  /* Calls fields and methods */
  jobject componentName = env->CallObjectMethod(instance, getComponentNameMethod);
  jobject packageManager = env->CallObjectMethod(instance, getPackageManagerMethod);
  jint GET_META_DATA = env->GetStaticIntField(packageManagerClass, metaDataConstant);
  jobject activityInfo = env->CallObjectMethod(packageManager, getActivityInfoMethod, componentName, GET_META_DATA);

  /* Stores the meta data */
  jobject metaData = env->GetObjectField(activityInfo, metaDataField);

  /* Frees all the local references except metadata object */
  return env->PopLocalFrame(metaData);
}

orxSTATUS orxFASTCALL orxAndroid_JNI_SetupThread(void *_pContext)
{
  /* From http://developer.android.com/guide/practices/jni.html
   * Threads attached through JNI must call DetachCurrentThread before they exit. If coding this directly is awkward,
   * in Android 2.0 (Eclair) and higher you can use pthread_key_create to define a destructor function that will be
   * called before the thread exits, and call DetachCurrentThread from there. (Use that key with pthread_setspecific
   * to store the JNIEnv in thread-local-storage; that way it'll be passed into your destructor as the argument.)
   * Note: The destructor is not called unless the stored value is != NULL
   * Note: You can call this function any number of times for the same thread, there's no harm in it
   *       (except for some lost CPU cycles)
   */

  JNIEnv *env = orxAndroid_JNI_GetEnv();
  pthread_setspecific(sThreadKey, (void*) env);

  return orxSTATUS_SUCCESS;
}

extern "C" ANativeWindow *orxAndroid_GetNativeWindow()
{
  LOGI("orxAndroid_GetNativeWindow()");

  while(sstAndroid.app->window == NULL && !sstAndroid.app->destroyRequested)
  {
    LOGI("no window received yet");
    orxAndroid_PumpEvents();
  }

  return sstAndroid.app->window;
}

extern "C" GameActivity *orxAndroid_GetGameActivity()
{
  return sstAndroid.app->activity;
}

extern "C" const char *orxAndroid_GetInternalStoragePath()
{
  return sstAndroid.app->activity->internalDataPath;
}

extern "C" void orxAndroid_JNI_GetArguments()
{
  JNIEnv *env = orxAndroid_JNI_GetEnv();

  env->PushLocalFrame(16);

  /* Gets meta data  */
  jobject metaData = orxAndroid_JNI_getActivityMetaData(env);
  if (metaData != NULL)
  {
    const char *zArguments;

    /* Finds classes */
    jclass bundleClass = env->FindClass("android/os/Bundle");

    /* Finds methods */
    jmethodID getStringMethod = env->GetMethodID(bundleClass, "getString", "(Ljava/lang/String;)Ljava/lang/String;");

    /* Reads arguments meta data */
    jstring arguments = (jstring)env->CallObjectMethod(metaData, getStringMethod, env->NewStringUTF("org.orx.lib.arguments"));
    if (arguments == NULL)
    {
      /* Use lib name as fallback */
      arguments = (jstring)env->CallObjectMethod(metaData, getStringMethod, env->NewStringUTF("android.app.lib_name"));
    }

    if (arguments == NULL)
    {
      /* Clears the arguments */
      *sstAndroid.zArguments = orxNULL;
    }
    else
    {
      zArguments = env->GetStringUTFChars(arguments, 0);

      /* Stores arguments */
      orxString_NPrint(sstAndroid.zArguments, sizeof(sstAndroid.zArguments) - 1, zArguments);

      env->ReleaseStringUTFChars(arguments, zArguments);
    }
  }
  else
  {
    /* Clears the arguments */
    *sstAndroid.zArguments = orxNULL;
  }

  /* Frees all the local references */
  env->PopLocalFrame(NULL);
}

extern "C" orxU32 orxAndroid_JNI_GetRotation()
{
  orxU32 rotation;
  JNIEnv *env = orxAndroid_JNI_GetEnv();

  env->PushLocalFrame(16);

  /* Gets display structure */
  jobject display = orxAndroid_JNI_getDisplay(env);

  /* Finds classes */
  jclass displayClass = env->FindClass("android/view/Display");

  /* Finds methods */
  jmethodID getRotationMethod = env->GetMethodID(displayClass, "getRotation", "()I");

  /* Calls method and stores rotation */
  rotation = (orxU32)env->CallIntMethod(display, getRotationMethod);

  /* Frees all the local references */
  env->PopLocalFrame(NULL);

  return rotation;
}

static void Android_CheckForNewAxis()
{
  /* Tell GameActivity about any new axis ids so it reports their events */
  const uint64_t activeAxisIds = Paddleboat_getActiveAxisMask();
  uint64_t newAxisIds = activeAxisIds ^ sstAndroid.activeAxisIds;
  if(newAxisIds != 0)
  {
    sstAndroid.activeAxisIds = activeAxisIds;
    int32_t currentAxisId = 0;
    while(newAxisIds != 0)
    {
      if((newAxisIds & 1) != 0)
      {
        LOGI("Enable Axis: %d", currentAxisId);
        GameActivityPointerAxes_enableAxis(currentAxisId);
      }
      ++currentAxisId;
      newAxisIds >>= 1;
    }
  }
}

static void Android_HandleGameInput(struct android_app* app)
{
  Android_CheckForNewAxis();

  /* Swap input buffers so we don't miss any events while processing inputBuffer. */
  android_input_buffer *ib = android_app_swap_input_buffers(app);
  /* Early exit if no events. */
  if(ib == NULL)
  {
      return;
  }

  if(ib->keyEventsCount != 0)
  {
    uint64_t i;

    for (i = 0; i < ib->keyEventsCount; i++)
    {
      GameActivityKeyEvent *event = &ib->keyEvents[i];
      if(Paddleboat_processGameActivityKeyInputEvent(event, sizeof(GameActivityKeyEvent)) == 0)
      {
        /* Didn't belong to a game controller, let's process it ourselves. */

        orxANDROID_KEY_EVENT stKeyEvent;
        /* Inits event payload */
        orxMemory_Zero(&stKeyEvent, sizeof(orxANDROID_KEY_EVENT));
        stKeyEvent.u32KeyCode = event->keyCode;
        stKeyEvent.u32Action = event->action == AKEY_EVENT_ACTION_DOWN ? orxANDROID_EVENT_KEYBOARD_DOWN
                                                                       : orxANDROID_EVENT_KEYBOARD_UP;
        if(event->action != AKEY_EVENT_ACTION_MULTIPLE)
        {
          orxEVENT_SEND(orxANDROID_EVENT_TYPE_KEYBOARD, 0, orxNULL, orxNULL, &stKeyEvent);
        }
      }
    }

    android_app_clear_key_events(ib);
  }

  if(ib->motionEventsCount != 0)
  {
    uint64_t i;

    for(i = 0; i < ib->motionEventsCount; i++)
    {
      GameActivityMotionEvent *event = &ib->motionEvents[i];
      if(Paddleboat_processGameActivityMotionInputEvent(event, sizeof(GameActivityMotionEvent)) == 0)
      {
        /* Didn't belong to a game controller, let's process it ourselves. */

        orxSYSTEM_EVENT_PAYLOAD stPayload;

        if(sstAndroid.fSurfaceScale == orxFLOAT_0)
        {
          orxConfig_PushSection(KZ_CONFIG_ANDROID);
          sstAndroid.fSurfaceScale = orxConfig_GetFloat(KZ_CONFIG_SURFACE_SCALE);
          orxConfig_PopSection();
        }

        /* Inits event payload */
        orxMemory_Zero(&stPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
        stPayload.stTouch.fPressure = orxFLOAT_0;

        int32_t iIndex;

        switch (event->action & AMOTION_EVENT_ACTION_MASK)
        {
          case AMOTION_EVENT_ACTION_POINTER_DOWN:
            iIndex = GET_ACTION_INDEX(event->action);
            stPayload.stTouch.u32ID = event->pointers[iIndex].id;
            stPayload.stTouch.fX = sstAndroid.fSurfaceScale * GET_AXIS_X(event, iIndex);
            stPayload.stTouch.fY = sstAndroid.fSurfaceScale * GET_AXIS_Y(event, iIndex);
            orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_BEGIN, orxNULL, orxNULL, &stPayload);
            break;
          case AMOTION_EVENT_ACTION_POINTER_UP:
            iIndex = GET_ACTION_INDEX(event->action);
            stPayload.stTouch.u32ID = event->pointers[iIndex].id;
            stPayload.stTouch.fX = sstAndroid.fSurfaceScale * GET_AXIS_X(event, iIndex);
            stPayload.stTouch.fY = sstAndroid.fSurfaceScale * GET_AXIS_Y(event, iIndex);
            orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_END, orxNULL, orxNULL, &stPayload);
            break;
          case AMOTION_EVENT_ACTION_DOWN:
            iIndex = GET_ACTION_INDEX(event->action);
            stPayload.stTouch.u32ID = event->pointers[iIndex].id;
            stPayload.stTouch.fX = sstAndroid.fSurfaceScale * GET_AXIS_X(event, iIndex);
            stPayload.stTouch.fY = sstAndroid.fSurfaceScale * GET_AXIS_Y(event, iIndex);
            orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_BEGIN, orxNULL, orxNULL, &stPayload);
            break;
          case AMOTION_EVENT_ACTION_UP:
          case AMOTION_EVENT_ACTION_CANCEL:
          {
            iIndex = GET_ACTION_INDEX(event->action);
            stPayload.stTouch.u32ID = event->pointers[iIndex].id;
            stPayload.stTouch.fX = sstAndroid.fSurfaceScale * GET_AXIS_X(event, iIndex);
            stPayload.stTouch.fY = sstAndroid.fSurfaceScale * GET_AXIS_Y(event, iIndex);
            orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_END, orxNULL, orxNULL, &stPayload);
            break;
          }
          case AMOTION_EVENT_ACTION_MOVE:
            for(iIndex = 0; iIndex < event->pointerCount; iIndex++)
            {
              stPayload.stTouch.u32ID = event->pointers[iIndex].id;
              stPayload.stTouch.fX = sstAndroid.fSurfaceScale * GET_AXIS_X(event, iIndex);
              stPayload.stTouch.fY = sstAndroid.fSurfaceScale * GET_AXIS_Y(event, iIndex);
              orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_MOVE, orxNULL, orxNULL, &stPayload);
            }
            break;
        }
      }
    }

    android_app_clear_motion_events(ib);
  }
}

static void orxAndroid_handleCmd(struct android_app *app, int32_t cmd)
{
  switch(cmd)
  {
    case APP_CMD_PAUSE:
      LOGI("APP_CMD_PAUSE");

      sstAndroid.bPaused = orxTRUE;
      orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_BACKGROUND);
      break;
    case APP_CMD_RESUME:
      LOGI("APP_CMD_RESUME");

      sstAndroid.bPaused = orxFALSE;
      orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_FOREGROUND);
      break;
    case APP_CMD_START:
      LOGI("APP_CMD_START");
      Paddleboat_onStart(orxAndroid_JNI_GetEnv());
      break;
    case APP_CMD_STOP:
      LOGI("APP_CMD_STOP");
      Paddleboat_onStop(orxAndroid_JNI_GetEnv());
      break;
    case APP_CMD_TERM_WINDOW:
      LOGI("APP_CMD_TERM_WINDOW");

      SwappyGL_setWindow(nullptr);
      sstAndroid.fSurfaceScale = orxFLOAT_0;
      orxEVENT_SEND(orxANDROID_EVENT_TYPE_SURFACE, orxANDROID_EVENT_SURFACE_DESTROYED, orxNULL, orxNULL, orxNULL);
      break;
    case APP_CMD_INIT_WINDOW:
      LOGI("APP_CMD_INIT_WINDOW");

      SwappyGL_setWindow(app->window);
      orxEVENT_SEND(orxANDROID_EVENT_TYPE_SURFACE, orxANDROID_EVENT_SURFACE_CREATED, orxNULL, orxNULL, orxNULL);
      break;
    case APP_CMD_CONTENT_RECT_CHANGED:
      {
        LOGI("APP_CMD_CONTENT_RECT_CHANGED");

        orxANDROID_SURFACE_CHANGED_EVENT stSurfaceChangedEvent;
        stSurfaceChangedEvent.u32Width = app->contentRect.right - app->contentRect.left;
        stSurfaceChangedEvent.u32Height = app->contentRect.bottom - app->contentRect.top;

        orxEVENT_SEND(orxANDROID_EVENT_TYPE_SURFACE, orxANDROID_EVENT_SURFACE_CHANGED, orxNULL, orxNULL, &stSurfaceChangedEvent);
        sstAndroid.fSurfaceScale = orxFLOAT_0;
      }
      break;
    case APP_CMD_DESTROY:
      LOGI("APP_CMD_DESTROY");
      orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_CLOSE);
      break;
    case APP_CMD_GAINED_FOCUS:
      LOGI("APP_CMD_GAINED_FOCUS");

      sstAndroid.bHasFocus = orxTRUE;
      orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_FOCUS_GAINED);
      break;
    case APP_CMD_LOST_FOCUS:
      LOGI("APP_CMD_LOST_FOCUS");

      sstAndroid.bHasFocus = orxFALSE;
      orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_FOCUS_LOST);
      break;
    default:
      break;
  }
}

static inline orxBOOL orxAndroid_IsInteractible()
{
  return (sstAndroid.app->window && !sstAndroid.bPaused && sstAndroid.bHasFocus);
}

extern "C" void orxAndroid_PumpEvents()
{
  /* Read all pending events. */
  int id;
  int events;
  android_poll_source *source;

  /* If not animating, we will block forever waiting for events.
   * If animating, we loop until all events are read, then continue
   * to draw the next frame of animation.
   */
  while( (id = ALooper_pollAll(orxAndroid_IsInteractible() ? 0 : -1, NULL, &events, (void**) &source)) >= 0 )
  {
    /* Process this event. */
    if(source != NULL)
    {
      source->process( sstAndroid.app, source );
    }

    if(id == LOOPER_ID_SENSOR)
    {
      orxEvent_SendShort(orxANDROID_EVENT_TYPE_ACCELERATE, 0);
    }

    /* Check if we are exiting. */
    if(sstAndroid.app->destroyRequested != 0)
    {
      return;
    }
  }

  /* Triggers any registered controller or mouse status callbacks */
  Paddleboat_update(orxAndroid_JNI_GetEnv());

  Android_HandleGameInput(sstAndroid.app);
}

/* Main function to call */
extern int main(int argc, char *argv[]);

void android_main(android_app* state)
{
  char *argv[orxANDROID_KU32_MAX_ARGUMENT_COUNT];

  state->onAppCmd = orxAndroid_handleCmd;

  android_app_set_motion_event_filter(state, NULL);

  /* Cleans static controller */
  orxMemory_Zero(&sstAndroid, sizeof(orxANDROID_STATIC));

  sstAndroid.app = state;
  sstAndroid.bPaused = orxTRUE;
  sstAndroid.bHasFocus = orxFALSE;
  sstAndroid.fSurfaceScale = orxFLOAT_0;

  jVM = state->activity->vm;

  /*
   * Create sThreadKey so we can keep track of the JNIEnv assigned to each thread
   * Refer to http://developer.android.com/guide/practices/design/jni.html for the rationale behind this
   */
  if(pthread_key_create(&sThreadKey, orxAndroid_JNI_ThreadDestroyed))
  {
    __android_log_print(ANDROID_LOG_ERROR, "Orx", "Error initializing pthread key");
  }
  else
  {
    orxAndroid_JNI_SetupThread(orxNULL);
  }

  /* Initializes joystick support */
  JNIEnv *env = orxAndroid_JNI_GetEnv();
  Paddleboat_init(env, state->activity->javaGameActivity);

  /* Initializes SwappyGL */
  SwappyGL_init(env, state->activity->javaGameActivity);
  SwappyGL_setAutoSwapInterval(false);
  SwappyGL_setAutoPipelineMode(false);
  SwappyGL_enableStats(false);

  /* Gets arguments from manifest */
  orxAndroid_JNI_GetArguments();

  /* Parses the arguments */
  int argc = 0;

  char *pc = strtok(sstAndroid.zArguments, " ");
  while (pc && argc < orxANDROID_KU32_MAX_ARGUMENT_COUNT - 1)
  {
    argv[argc++] = pc;
    pc = strtok(0, " ");
  }
  argv[argc] = NULL;

  /* Run the application code! */
  main(argc, argv);

  if(state->destroyRequested == 0)
  {
    GameActivity_finish(state->activity);

    /* pumps final events */
    int id;
    int events;
    android_poll_source *source;

    state->onAppCmd = NULL;

    while((id = ALooper_pollAll(-1, NULL, &events, (void**) &source )) >= 0)
    {
      /* Process this event. */
      if(source != NULL)
      {
        source->process(state, source);
      }

      /* Check if we are exiting. */
      if(state->destroyRequested != 0)
      {
        break;
      }
    }
  }

  Paddleboat_destroy(env);
  SwappyGL_destroy();
}

/* APK orxRESOURCE */

static const orxSTRING orxRESOURCE_KZ_TYPE_TAG_APK = "apk";                           /**< Resource type apk tag */
#define orxRESOURCE_KZ_DEFAULT_STORAGE                "."                             /**< Default storage */
#define orxRESOURCE_KU32_BUFFER_SIZE                  256                             /**< Buffer size */
static orxCHAR s_acFileLocationBuffer[orxRESOURCE_KU32_BUFFER_SIZE];                  /**< File location buffer size */

static const orxSTRING orxFASTCALL orxResource_APK_Locate(const orxSTRING _zStorage, const orxSTRING _zName, orxBOOL _bRequireExistence)
{
  const orxSTRING zResult = orxNULL;
  AAsset   *poAsset;

  /* Default storage? */
  if(orxString_Compare(_zStorage, orxRESOURCE_KZ_DEFAULT_STORAGE) == 0)
  {
    /* Uses name as path */
    orxString_NPrint(s_acFileLocationBuffer, orxRESOURCE_KU32_BUFFER_SIZE - 1, "%s", _zName);
  }
  else
  {
    /* Composes full name */
    orxString_NPrint(s_acFileLocationBuffer, orxRESOURCE_KU32_BUFFER_SIZE - 1, "%s%c%s", _zStorage, orxCHAR_DIRECTORY_SEPARATOR_LINUX, _zName);
  }

  /* Exist? */
  poAsset = AAssetManager_open(sstAndroid.app->activity->assetManager, s_acFileLocationBuffer, AASSET_MODE_RANDOM);
  if(poAsset != NULL)
  {
    /* Updates result */
    zResult = s_acFileLocationBuffer;
    AAsset_close(poAsset);
  }

  /* Done! */
  return zResult;
}

static orxHANDLE orxFASTCALL orxResource_APK_Open(const orxSTRING _zLocation, orxBOOL _bEraseMode)
{
  orxHANDLE hResult;

  /* Not in erase mode? */
  if(_bEraseMode == orxFALSE)
  {
    AAsset *poAsset;

    /* Opens Asset */
    poAsset = AAssetManager_open(sstAndroid.app->activity->assetManager, _zLocation, AASSET_MODE_RANDOM);

    /* Updates result */
    hResult = (poAsset != orxNULL) ? (orxHANDLE)poAsset : orxHANDLE_UNDEFINED;
  }
  else
  {
    /* Updates result */
    hResult = orxHANDLE_UNDEFINED;
  }

  /* Done! */
  return hResult;
}

static void orxFASTCALL orxResource_APK_Close(orxHANDLE _hResource)
{
  AAsset   *poAsset;

  /* Gets asset */
  poAsset = (AAsset *)_hResource;

  /* Closes it */
  AAsset_close(poAsset);
}

static orxS64 orxFASTCALL orxResource_APK_GetSize(orxHANDLE _hResource)
{
  AAsset   *poAsset;
  orxS64    s64Result;

  /* Gets asset */
  poAsset = (AAsset *)_hResource;

  /* Updates result */
  s64Result = (orxS64)AAsset_getLength(poAsset);

  /* Done! */
  return s64Result;
}

static orxS64 orxFASTCALL orxResource_APK_Seek(orxHANDLE _hResource, orxS64 _s64Offset, orxSEEK_OFFSET_WHENCE _eWhence)
{
  AAsset   *poAsset;
  orxS64    s64Result;

  /* Gets asset */
  poAsset = (AAsset *)_hResource;

  /* Updates result */
  s64Result = (orxS64)AAsset_seek(poAsset, (off_t)_s64Offset, _eWhence);

  /* Done! */
  return s64Result;
}

static orxS64 orxFASTCALL orxResource_APK_Tell(orxHANDLE _hResource)
{
  AAsset   *poAsset;
  orxS64    s64Result;

  /* Gets asset */
  poAsset = (AAsset *)_hResource;

  /* Updates result */
  s64Result = (orxS64)AAsset_getLength(poAsset) - (orxS64)AAsset_getRemainingLength(poAsset);

  /* Done! */
  return s64Result;
}

static orxS64 orxFASTCALL orxResource_APK_Read(orxHANDLE _hResource, orxS64 _s64Size, void *_pBuffer)
{
  AAsset   *poAsset;
  orxS64    s64Result;

  /* Gets asset */
  poAsset = (AAsset *)_hResource;

  /* Updates result */
  s64Result = (orxS64)AAsset_read(poAsset, _pBuffer, (size_t)(sizeof(orxCHAR) * _s64Size)) / sizeof(orxCHAR);

  /* Done! */
  return s64Result;
}

orxSTATUS orxAndroid_RegisterAPKResource()
{
  orxSTATUS eResult;
  orxRESOURCE_TYPE_INFO stAPKTypeInfo;

  /* Inits apk type */
  orxMemory_Zero(&stAPKTypeInfo, sizeof(orxRESOURCE_TYPE_INFO));
  stAPKTypeInfo.zTag       = (orxCHAR*) orxRESOURCE_KZ_TYPE_TAG_APK;
  stAPKTypeInfo.pfnLocate  = orxResource_APK_Locate;
  stAPKTypeInfo.pfnOpen    = orxResource_APK_Open;
  stAPKTypeInfo.pfnClose   = orxResource_APK_Close;
  stAPKTypeInfo.pfnGetSize = orxResource_APK_GetSize;
  stAPKTypeInfo.pfnSeek    = orxResource_APK_Seek;
  stAPKTypeInfo.pfnTell    = orxResource_APK_Tell;
  stAPKTypeInfo.pfnRead    = orxResource_APK_Read;
  stAPKTypeInfo.pfnWrite   = orxNULL;
  stAPKTypeInfo.pfnDelete  = orxNULL;

  /* Registers it */
  eResult = orxResource_RegisterType(&stAPKTypeInfo);

  return eResult;
}
#endif