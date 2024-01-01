/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008- Orx-Project
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
 * @author hacker.danielsson@gmail.com
 *
 * Android support
 *
 */


#if defined(TARGET_OS_ANDROID)

#include <android/log.h>

#ifdef __orxDEBUG__

#define MODULE "orxAndroidSupport"
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, MODULE, __VA_ARGS__)
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, MODULE, __VA_ARGS__)
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO, MODULE, __VA_ARGS__)
#define LOGV(...)  __android_log_print(ANDROID_LOG_VERBOSE, MODULE, __VA_ARGS__)

#else /* __orxDEBUG__ */

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
#define orxANDROID_KU32_KEY_BUFFER_SIZE         (AKEYCODE_PROFILE_SWITCH + 1)

#define orxANDROID_GET_ACTION_INDEX(ACTION)     (((ACTION) & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT)
#define orxANDROID_GET_AXIS_X(EV, INDEX)        GameActivityPointerAxes_getX(&(EV)->pointers[INDEX])
#define orxANDROID_GET_AXIS_Y(EV, INDEX)        GameActivityPointerAxes_getY(&(EV)->pointers[INDEX])

#define orxANDROID_KEY_ACTION_NONE              0
#define orxANDROID_KEY_ACTION_BASE              0x10000000
#define orxANDROID_KEY_ACTION_DOWN              (orxANDROID_KEY_ACTION_BASE | AKEY_EVENT_ACTION_DOWN)
#define orxANDROID_KEY_ACTION_UP                (orxANDROID_KEY_ACTION_BASE | AKEY_EVENT_ACTION_UP)

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxANDROID_STATIC_t
{
  orxBOOL              bPaused;
  orxBOOL              bHasFocus;
  orxFLOAT             fSurfaceScale;
  uint64_t             activeAxisIds;
  orxCHAR              zArguments[orxANDROID_KU32_ARGUMENT_BUFFER_SIZE];
  orxU32               au32PendingKeyActions[orxANDROID_KU32_KEY_BUFFER_SIZE];
  struct android_app  *app;
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

  JNIEnv *pstEnv;
  int status = jVM->AttachCurrentThread(&pstEnv, NULL);
  if(status < 0)
  {
    LOGE("failed to attach current thread");
    return NULL;
  }

  return pstEnv;
}

static void orxAndroid_JNI_ThreadDestroyed(void *_pVvalue)
{
  /* The thread is being destroyed, detach it from the Java VM and set the mThreadKey _pVvalue to NULL as required */
  JNIEnv *pstEnv = (JNIEnv *)_pVvalue;
  if(pstEnv != NULL)
  {
    jVM->DetachCurrentThread();
    pthread_setspecific(sThreadKey, NULL);
  }
}

static orxS32 orxAndroid_GetSdkVersion()
{
  return sstAndroid.app->activity->sdkVersion;
}

static jobject orxAndroid_JNI_getDisplay(JNIEnv *_pstEnv)
{
  jobject display;
  jobject instance = sstAndroid.app->activity->javaGameActivity;

  _pstEnv->PushLocalFrame(16);

  /* Note : WindowManager.getDefaultDisplay() was deprecated in Android R */
  if(orxAndroid_GetSdkVersion() >= __ANDROID_API_R__)
  {
    /* Finds classes */
    jclass contextClass = _pstEnv->FindClass("android/content/Context");

    /* Finds methods */
    jmethodID getDisplayMethod = _pstEnv->GetMethodID(contextClass, "getDisplay", "()Landroid/view/Display;");

    /* Calls methods and stores display object */
    display = _pstEnv->CallObjectMethod(instance, getDisplayMethod);
  }
  else
  {
    /* Finds classes */
    jclass activityClass = _pstEnv->GetObjectClass(instance);
    jclass windowManagerClass = _pstEnv->FindClass("android/view/WindowManager");

    /* Finds methods */
    jmethodID getWindowManagerMethod = _pstEnv->GetMethodID(activityClass, "getWindowManager", "()Landroid/view/WindowManager;");
    jmethodID getDefaultDisplayMethod = _pstEnv->GetMethodID(windowManagerClass,"getDefaultDisplay", "()Landroid/view/Display;");

    /* Calls methods and stores display object */
    jobject windowManager = _pstEnv->CallObjectMethod(instance, getWindowManagerMethod);
    display = _pstEnv->CallObjectMethod(windowManager, getDefaultDisplayMethod);
  }

  /* Frees all the local references except display object */
  return _pstEnv->PopLocalFrame(display);
}

static jobject orxAndroid_JNI_getActivityMetaData(JNIEnv *_pstEnv)
{
  jobject instance = sstAndroid.app->activity->javaGameActivity;

  _pstEnv->PushLocalFrame(16);

  /* Finds classes */
  jclass activityClass = _pstEnv->GetObjectClass(instance);
  jclass activityInfoClass = _pstEnv->FindClass("android/content/pm/ActivityInfo");
  jclass packageManagerClass = _pstEnv->FindClass("android/content/pm/PackageManager");

  /* Finds methods */
  jmethodID getPackageManagerMethod = _pstEnv->GetMethodID(activityClass, "getPackageManager", "()Landroid/content/pm/PackageManager;");
  jmethodID getComponentNameMethod = _pstEnv->GetMethodID(activityClass, "getComponentName", "()Landroid/content/ComponentName;");
  jmethodID getActivityInfoMethod = _pstEnv->GetMethodID(packageManagerClass, "getActivityInfo", "(Landroid/content/ComponentName;I)Landroid/content/pm/ActivityInfo;");

  /* Finds fields */
  jfieldID metaDataConstant = _pstEnv->GetStaticFieldID(packageManagerClass, "GET_META_DATA", "I");
  jfieldID metaDataField = _pstEnv->GetFieldID(activityInfoClass, "metaData", "Landroid/os/Bundle;");

  /* Calls fields and methods */
  jobject componentName = _pstEnv->CallObjectMethod(instance, getComponentNameMethod);
  jobject packageManager = _pstEnv->CallObjectMethod(instance, getPackageManagerMethod);
  jint GET_META_DATA = _pstEnv->GetStaticIntField(packageManagerClass, metaDataConstant);
  jobject activityInfo = _pstEnv->CallObjectMethod(packageManager, getActivityInfoMethod, componentName, GET_META_DATA);

  /* Stores the meta data */
  jobject metaData = _pstEnv->GetObjectField(activityInfo, metaDataField);

  /* Frees all the local references except metadata object */
  return _pstEnv->PopLocalFrame(metaData);
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

  pthread_setspecific(sThreadKey, (void *)orxAndroid_JNI_GetEnv());

  return orxSTATUS_SUCCESS;
}

extern "C" ANativeWindow *orxAndroid_GetNativeWindow()
{
  LOGI("orxAndroid_GetNativeWindow()");

  while((sstAndroid.app->window == NULL) && !sstAndroid.app->destroyRequested)
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
  JNIEnv *pstEnv = orxAndroid_JNI_GetEnv();

  pstEnv->PushLocalFrame(16);

  /* Gets meta data  */
  jobject metaData = orxAndroid_JNI_getActivityMetaData(pstEnv);
  if(metaData != NULL)
  {
    const char *zArguments;

    /* Finds classes */
    jclass bundleClass = pstEnv->FindClass("android/os/Bundle");

    /* Finds methods */
    jmethodID getStringMethod = pstEnv->GetMethodID(bundleClass, "getString", "(Ljava/lang/String;)Ljava/lang/String;");

    /* Reads arguments meta data */
    jstring arguments = (jstring)pstEnv->CallObjectMethod(metaData, getStringMethod, pstEnv->NewStringUTF("org.orx.lib.arguments"));
    if(arguments == NULL)
    {
      /* Use lib name as fallback */
      arguments = (jstring)pstEnv->CallObjectMethod(metaData, getStringMethod, pstEnv->NewStringUTF("android.app.lib_name"));
    }

    if(arguments == NULL)
    {
      /* Clears the arguments */
      *sstAndroid.zArguments = orxNULL;
    }
    else
    {
      zArguments = pstEnv->GetStringUTFChars(arguments, 0);

      /* Stores arguments */
      orxString_NPrint(sstAndroid.zArguments, sizeof(sstAndroid.zArguments) - 1, zArguments);

      pstEnv->ReleaseStringUTFChars(arguments, zArguments);
    }
  }
  else
  {
    /* Clears the arguments */
    *sstAndroid.zArguments = orxNULL;
  }

  /* Frees all the local references */
  pstEnv->PopLocalFrame(NULL);
}

extern "C" orxU32 orxAndroid_JNI_GetRotation()
{
  orxU32 rotation;
  JNIEnv *pstEnv = orxAndroid_JNI_GetEnv();

  pstEnv->PushLocalFrame(16);

  /* Gets display structure */
  jobject display = orxAndroid_JNI_getDisplay(pstEnv);

  /* Finds classes */
  jclass displayClass = pstEnv->FindClass("android/view/Display");

  /* Finds methods */
  jmethodID getRotationMethod = pstEnv->GetMethodID(displayClass, "getRotation", "()I");

  /* Calls method and stores rotation */
  rotation = (orxU32)pstEnv->CallIntMethod(display, getRotationMethod);

  /* Frees all the local references */
  pstEnv->PopLocalFrame(NULL);

  return rotation;
}

extern "C" void orxAndroid_SetKeyFilter(android_key_event_filter _pfnFilter)
{
  android_app_set_key_event_filter(sstAndroid.app, _pfnFilter);
}

static void orxAndroid_CheckForNewAxis()
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

static void orxAndroid_SendKey(orxU32 _u32KeyCode, orxU32 _u32Action)
{
  orxANDROID_KEY_EVENT stKeyEvent;

  /* Checks */
  orxASSERT((_u32Action == AKEY_EVENT_ACTION_DOWN) || (_u32Action == AKEY_EVENT_ACTION_UP));

  /* Inits event payload */
  orxMemory_Zero(&stKeyEvent, sizeof(orxANDROID_KEY_EVENT));
  stKeyEvent.u32KeyCode = _u32KeyCode;
  stKeyEvent.u32Action  = (_u32Action == AKEY_EVENT_ACTION_DOWN)
                          ? orxANDROID_EVENT_KEYBOARD_DOWN
                          : orxANDROID_EVENT_KEYBOARD_UP;

  orxEVENT_SEND(orxANDROID_EVENT_TYPE_KEYBOARD, 0, orxNULL, orxNULL, &stKeyEvent);
}

static void orxAndroid_HandleGameInput(struct android_app *_pstApp)
{
  orxU32 u32Key, u32Action;
  orxU32 au32HandledKeyActions[orxANDROID_KU32_KEY_BUFFER_SIZE];

  /* Handles any pending key events */
  for(u32Key = 0; u32Key < orxANDROID_KU32_KEY_BUFFER_SIZE; u32Key++)
  {
    u32Action = sstAndroid.au32PendingKeyActions[u32Key];
    sstAndroid.au32PendingKeyActions[u32Key] = orxANDROID_KEY_ACTION_NONE;

    if(u32Action != orxANDROID_KEY_ACTION_NONE)
    {
      orxAndroid_SendKey(u32Key, u32Action & ~orxANDROID_KEY_ACTION_BASE);
    }

    au32HandledKeyActions[u32Key] = u32Action;
  }

  orxAndroid_CheckForNewAxis();

  /* Swap input buffers so we don't miss any events while processing input buffer. */
  android_input_buffer *ib = android_app_swap_input_buffers(_pstApp);
  /* Early exit if no events. */
  if(ib == NULL)
  {
    return;
  }

  if(ib->keyEventsCount != 0)
  {
    uint64_t i;

    for(i = 0; i < ib->keyEventsCount; i++)
    {
      GameActivityKeyEvent *event = &ib->keyEvents[i];
      if(Paddleboat_processGameActivityKeyInputEvent(event, sizeof(GameActivityKeyEvent)) == 0)
      {
        /** In orx, key state transitions occur between two frames. In that case Paddleboat
         * delivers paired down/up events, we must schedule the last event until the next frame.
         * We do not buffer more than one event; any ludicrously fast key presses are ignored,
         * resulting in a single key press. Note that all down/up events are sequentially
         * delivered by Paddleboat.
         *
         * E.g. Handling a specific key, iterating the key events for the current frame:
         *
         * Down Up          -> Invoke Down, Schedule Up
         * Down Up Down     -> Invoke Down
         * Down Up Down Up  -> Invoke Down, Schedule Up
         *
         * And vice versa for sequences starting with an Up event.
         */

        /* Checks */
        orxASSERT(event->keyCode < orxANDROID_KU32_KEY_BUFFER_SIZE);

        if(event->action != AKEY_EVENT_ACTION_MULTIPLE)
        {
          /* Key not handled? */
          if(au32HandledKeyActions[event->keyCode] == orxANDROID_KEY_ACTION_NONE)
          {
            orxAndroid_SendKey(event->keyCode, event->action);
            au32HandledKeyActions[event->keyCode] = orxANDROID_KEY_ACTION_BASE | event->action;
          }
          else
          {
            /* Key not scheduled? */
            if(sstAndroid.au32PendingKeyActions[event->keyCode] == orxANDROID_KEY_ACTION_NONE)
            {
              /* Schedules key action for next frame */
              sstAndroid.au32PendingKeyActions[event->keyCode] = orxANDROID_KEY_ACTION_BASE | event->action;
            }
          }
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
          {
            iIndex = orxANDROID_GET_ACTION_INDEX(event->action);
            stPayload.stTouch.u32ID = event->pointers[iIndex].id;
            stPayload.stTouch.fX = sstAndroid.fSurfaceScale * orxANDROID_GET_AXIS_X(event, iIndex);
            stPayload.stTouch.fY = sstAndroid.fSurfaceScale * orxANDROID_GET_AXIS_Y(event, iIndex);
            orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_BEGIN, orxNULL, orxNULL, &stPayload);
            break;
          }
          case AMOTION_EVENT_ACTION_POINTER_UP:
          {
            iIndex = orxANDROID_GET_ACTION_INDEX(event->action);
            stPayload.stTouch.u32ID = event->pointers[iIndex].id;
            stPayload.stTouch.fX = sstAndroid.fSurfaceScale * orxANDROID_GET_AXIS_X(event, iIndex);
            stPayload.stTouch.fY = sstAndroid.fSurfaceScale * orxANDROID_GET_AXIS_Y(event, iIndex);
            orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_END, orxNULL, orxNULL, &stPayload);
            break;
          }
          case AMOTION_EVENT_ACTION_DOWN:
          {
            stPayload.stTouch.u32ID = event->pointers[0].id;
            stPayload.stTouch.fX = sstAndroid.fSurfaceScale * orxANDROID_GET_AXIS_X(event, 0);
            stPayload.stTouch.fY = sstAndroid.fSurfaceScale * orxANDROID_GET_AXIS_Y(event, 0);
            orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_BEGIN, orxNULL, orxNULL, &stPayload);
            break;
          }
          case AMOTION_EVENT_ACTION_UP:
          case AMOTION_EVENT_ACTION_CANCEL:
          {
            stPayload.stTouch.u32ID = event->pointers[0].id;
            stPayload.stTouch.fX = sstAndroid.fSurfaceScale * orxANDROID_GET_AXIS_X(event, 0);
            stPayload.stTouch.fY = sstAndroid.fSurfaceScale * orxANDROID_GET_AXIS_Y(event, 0);
            orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_END, orxNULL, orxNULL, &stPayload);
            break;
          }
          case AMOTION_EVENT_ACTION_MOVE:
          {
            for(iIndex = 0; iIndex < event->pointerCount; iIndex++)
            {
              stPayload.stTouch.u32ID = event->pointers[iIndex].id;
              stPayload.stTouch.fX = sstAndroid.fSurfaceScale * orxANDROID_GET_AXIS_X(event, iIndex);
              stPayload.stTouch.fY = sstAndroid.fSurfaceScale * orxANDROID_GET_AXIS_Y(event, iIndex);
              orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_MOVE, orxNULL, orxNULL, &stPayload);
            }
            break;
          }
        }
      }
    }

    android_app_clear_motion_events(ib);
  }
}

static void orxAndroid_handleCmd(struct android_app *_pstApp, int32_t _s32Cmd)
{
  switch(_s32Cmd)
  {
    case APP_CMD_INIT_WINDOW:
    {
      LOGI("APP_CMD_INIT_WINDOW");
      SwappyGL_setWindow(_pstApp->window);
      orxEVENT_SEND(orxANDROID_EVENT_TYPE_SURFACE, orxANDROID_EVENT_SURFACE_CREATED, orxNULL, orxNULL, orxNULL);
      break;
    }
    case APP_CMD_TERM_WINDOW:
    {
      LOGI("APP_CMD_TERM_WINDOW");
      SwappyGL_setWindow(nullptr);
      sstAndroid.fSurfaceScale = orxFLOAT_0;
      orxEVENT_SEND(orxANDROID_EVENT_TYPE_SURFACE, orxANDROID_EVENT_SURFACE_DESTROYED, orxNULL, orxNULL, orxNULL);
      break;
    }
    case APP_CMD_WINDOW_RESIZED:
    {
      LOGI("APP_CMD_WINDOW_RESIZED");
      break;
    }
    case APP_CMD_WINDOW_REDRAW_NEEDED:
    {
      LOGI("APP_CMD_WINDOW_REDRAW_NEEDED");
      break;
    }
    case APP_CMD_CONTENT_RECT_CHANGED:
    {
      LOGI("APP_CMD_CONTENT_RECT_CHANGED");
      orxANDROID_SURFACE_CHANGED_EVENT stSurfaceChangedEvent;
      stSurfaceChangedEvent.u32Width = _pstApp->contentRect.right - _pstApp->contentRect.left;
      stSurfaceChangedEvent.u32Height = _pstApp->contentRect.bottom - _pstApp->contentRect.top;

      orxEVENT_SEND(orxANDROID_EVENT_TYPE_SURFACE, orxANDROID_EVENT_SURFACE_CHANGED, orxNULL, orxNULL, &stSurfaceChangedEvent);
      sstAndroid.fSurfaceScale = orxFLOAT_0;
      break;
    }
    case APP_CMD_GAINED_FOCUS:
    {
      LOGI("APP_CMD_GAINED_FOCUS");
      sstAndroid.bHasFocus = orxTRUE;
      orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_FOCUS_GAINED);
      break;
    }
    case APP_CMD_LOST_FOCUS:
    {
      LOGI("APP_CMD_LOST_FOCUS");
      sstAndroid.bHasFocus = orxFALSE;
      orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_FOCUS_LOST);
      break;
    }
    case APP_CMD_CONFIG_CHANGED:
    {
      LOGI("APP_CMD_CONFIG_CHANGED");
      break;
    }
    case APP_CMD_LOW_MEMORY:
    {
      LOGI("APP_CMD_LOW_MEMORY");
      break;
    }
    case APP_CMD_START:
    {
      LOGI("APP_CMD_START");
      Paddleboat_onStart(orxAndroid_JNI_GetEnv());
      break;
    }
    case APP_CMD_RESUME:
    {
      LOGI("APP_CMD_RESUME");
      sstAndroid.bPaused = orxFALSE;
      orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_FOREGROUND);
      break;
    }
    case APP_CMD_SAVE_STATE:
    {
      LOGI("APP_CMD_SAVE_STATE");
      break;
    }
    case APP_CMD_PAUSE:
    {
      LOGI("APP_CMD_PAUSE");
      sstAndroid.bPaused = orxTRUE;
      orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_BACKGROUND);
      break;
    }
    case APP_CMD_STOP:
    {
      LOGI("APP_CMD_STOP");
      Paddleboat_onStop(orxAndroid_JNI_GetEnv());
      break;
    }
    case APP_CMD_DESTROY:
    {
      LOGI("APP_CMD_DESTROY");
      orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_CLOSE);
      break;
    }
    case APP_CMD_WINDOW_INSETS_CHANGED:
    {
      LOGI("APP_CMD_WINDOW_INSETS_CHANGED");
      break;
    }
    default:
    {
      break;
    }
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

  /* Check if we are exiting. */
  if(sstAndroid.app->destroyRequested != 0)
  {
    return;
  }

  /* If not animating, we will block forever waiting for events.
   * If animating, we loop until all events are read, then continue
   * to draw the next frame of animation.
   */
  while((id = ALooper_pollAll(orxAndroid_IsInteractible() ? 0 : -1, NULL, &events, (void **) &source)) >= 0)
  {
    /* Process this event. */
    if(source != NULL)
    {
      source->process(sstAndroid.app, source);
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

  orxAndroid_HandleGameInput(sstAndroid.app);
}

/* Main function to call */
extern int main(int argc, char *argv[]);

void android_main(android_app *_pstState)
{
  char *argv[orxANDROID_KU32_MAX_ARGUMENT_COUNT];

  _pstState->onAppCmd = orxAndroid_handleCmd;

  android_app_set_motion_event_filter(_pstState, NULL);

  /* Cleans static controller */
  orxMemory_Zero(&sstAndroid, sizeof(orxANDROID_STATIC));

  sstAndroid.app = _pstState;
  sstAndroid.bPaused = orxTRUE;
  sstAndroid.bHasFocus = orxFALSE;
  sstAndroid.fSurfaceScale = orxFLOAT_0;

  jVM = _pstState->activity->vm;

  /*
   * Create sThreadKey so we can keep track of the JNIEnv assigned to each thread
   * Refer to http://developer.android.com/guide/practices/design/jni.html for the rationale behind this
   */
  if(pthread_key_create(&sThreadKey, orxAndroid_JNI_ThreadDestroyed))
  {
    sThreadKey = 0;
    LOGE("Error initializing pthread key");
  }
  else
  {
    orxAndroid_JNI_SetupThread(orxNULL);
  }

  /* Initializes joystick support */
  JNIEnv *pstEnv = orxAndroid_JNI_GetEnv();
  Paddleboat_init(pstEnv, _pstState->activity->javaGameActivity);

  /* Initializes SwappyGL */
  SwappyGL_init(pstEnv, _pstState->activity->javaGameActivity);
  SwappyGL_setAutoSwapInterval(false);
  SwappyGL_setAutoPipelineMode(false);

  /* Gets arguments from manifest */
  orxAndroid_JNI_GetArguments();

  /* Parses the arguments */
  int argc = 0;

  char *pc = strtok(sstAndroid.zArguments, " ");
  while(pc && argc < orxANDROID_KU32_MAX_ARGUMENT_COUNT - 1)
  {
    argv[argc++] = pc;
    pc = strtok(0, " ");
  }
  argv[argc] = NULL;

  /* Run the application code! */
  main(argc, argv);

  if(_pstState->destroyRequested == 0)
  {
    GameActivity_finish(_pstState->activity);

    /* pumps final events */
    int id;
    int events;
    android_poll_source *source;

    _pstState->onAppCmd = NULL;

    while((id = ALooper_pollAll(-1, NULL, &events, (void **)&source)) >= 0)
    {
      /* Process this event. */
      if(source != NULL)
      {
        source->process(_pstState, source);
      }

      /* Check if we are exiting. */
      if(_pstState->destroyRequested != 0)
      {
        break;
      }
    }
  }

  Paddleboat_destroy(pstEnv);
  SwappyGL_destroy();
}

/* APK orxRESOURCE */

static const orxSTRING orxRESOURCE_KZ_TYPE_TAG_APK =  "apk";                          /**< Resource type apk tag */
#define orxRESOURCE_KZ_DEFAULT_STORAGE                "."                             /**< Default storage */
#define orxRESOURCE_KU32_BUFFER_SIZE                  256                             /**< Buffer size */
static orxCHAR s_acFileLocationBuffer[orxRESOURCE_KU32_BUFFER_SIZE];                  /**< File location buffer size */

static const orxSTRING orxFASTCALL orxResource_APK_Locate(const orxSTRING _zGroup, const orxSTRING _zStorage, const orxSTRING _zName, orxBOOL _bRequireExistence)
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
  stAPKTypeInfo.zTag       = (orxCHAR *)orxRESOURCE_KZ_TYPE_TAG_APK;
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
