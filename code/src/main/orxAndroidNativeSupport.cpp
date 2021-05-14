/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2021 Orx-Project
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
 * @file orxAndroidNativeSupport.cpp
 * @date 26/06/2011
 * @author simons.philippe@gmail.com
 *
 * Android support
 *
 */

#if defined(TARGET_OS_ANDROID_NATIVE)

#include <android/log.h>
#include <android_native_app_glue.h>
#include <android/input.h>

#include <errno.h>
#include <unistd.h>

#define MODULE "orxAndroidNativeSupport"
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,MODULE,__VA_ARGS__)
#define LOGW(...)  __android_log_print(ANDROID_LOG_WARN,MODULE,__VA_ARGS__)

#ifdef __orxDEBUG__

#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,MODULE,__VA_ARGS__)
#define LOGV(...)  ((void)__android_log_print(ANDROID_LOG_VERBOSE, MODULE, __VA_ARGS__))

#else

#define LOGI(...)
#define LOGV(...)

#endif

#include "orxInclude.h"
#include "orxKernel.h"
#include "main/orxAndroid.h"

/** Static structure
 */
typedef struct __orxANDROID_STATIC_t {
        // AssetManager
        char *zAndroidInternalFilesPath;

        // looper fd-pipe
        ALooper* looper;
        int pipeJoystickEvent[2];

        orxBOOL bPaused;
        orxBOOL bHasFocus;

        int32_t lastWidth;
        int32_t lastHeight;
        orxFLOAT fSurfaceScale;

        struct android_app* app_;
} orxANDROID_STATIC;

static orxANDROID_STATIC sstAndroid;
static pthread_key_t sThreadKey;
static JavaVM* jVM;

static JNIEnv* Android_JNI_GetEnv() {
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
    if(status < 0) {
        LOGE("failed to attach current thread");
        return 0;
    }

    return env;
}

static void Android_JNI_ThreadDestroyed(void* value) {
    /* The thread is being destroyed, detach it from the Java VM and set the mThreadKey value to NULL as required */
    JNIEnv *env = (JNIEnv*) value;
    if (env != NULL) {
        jVM->DetachCurrentThread();
        pthread_setspecific(sThreadKey, NULL);
    }
}

orxSTATUS orxFASTCALL orxAndroid_JNI_SetupThread(void *_pContext) {
    /* From http://developer.android.com/guide/practices/jni.html
     * Threads attached through JNI must call DetachCurrentThread before they exit. If coding this directly is awkward,
     * in Android 2.0 (Eclair) and higher you can use pthread_key_create to define a destructor function that will be
     * called before the thread exits, and call DetachCurrentThread from there. (Use that key with pthread_setspecific
     * to store the JNIEnv in thread-local-storage; that way it'll be passed into your destructor as the argument.)
     * Note: The destructor is not called unless the stored value is != NULL
     * Note: You can call this function any number of times for the same thread, there's no harm in it
     *       (except for some lost CPU cycles)
     */
    JNIEnv *env = Android_JNI_GetEnv();
    pthread_setspecific(sThreadKey, (void*) env);

    return orxSTATUS_SUCCESS;
}

extern "C" ANativeWindow* orxAndroid_GetNativeWindow()
{
  LOGI("orxAndroid_GetNativeWindow()");

  while(sstAndroid.app_->window == NULL && !sstAndroid.app_->destroyRequested)
  {
    LOGI("no window received yet");
    orxAndroid_PumpEvents();
  }

  return sstAndroid.app_->window;
}

extern "C" void *orxAndroid_GetJNIEnv()
{
  return Android_JNI_GetEnv();
}

extern "C" jobject orxAndroid_GetActivity()
{
  return sstAndroid.app_->activity->clazz;
}

extern "C" struct android_app* orxAndroid_GetAndroidApp()
{
  return sstAndroid.app_;
}

// TODO: Duplicated in orxAndroid(Native)Support.cpp. Refactor to orxAndroidCommon or similar
extern "C" void orxAndroid_JNI_GetDeviceIds(orxS32 deviceIds[orxANDROID_KU32_MAX_JOYSTICK_NUMBER])
{
  JNIEnv *env = Android_JNI_GetEnv();
  jobject jActivity = orxAndroid_GetActivity();

  // Create Java-method access to the activity.getDeviceIds();
  jmethodID midGetDeviceIds = env->GetMethodID(env->GetObjectClass(jActivity), "getDeviceIds", "()[I");

  if(midGetDeviceIds)
  {
    jintArray retval = (jintArray) env->CallObjectMethod(jActivity, midGetDeviceIds);
    env->GetIntArrayRegion(retval, 0, orxANDROID_KU32_MAX_JOYSTICK_NUMBER, (jint*) &deviceIds[0]);
    env->DeleteLocalRef(retval);
  } else
  {
    env->ExceptionClear();
    LOGW("Couldn't locate Java method getDeviceIds, provided in OrxNativeActivity.java. Joystick devices in Orx is now disabled.");
  }
}

// Theses method are called from Activity if it implements InputDeviceListener
// Copied from orxAndroidSupport.cpp
// Local function names doesn't have to match in Activity because they are registered manually.
extern "C" void JNICALL Java_nativeOnInputDeviceAdded(JNIEnv* env, jobject thiz, jint deviceId)
{
  orxANDROID_JOYSTICK_EVENT stJoystickEvent;

  stJoystickEvent.u32Type = orxANDROID_EVENT_JOYSTICK_ADDED;
  stJoystickEvent.u32DeviceId = deviceId;

  if(sstAndroid.pipeJoystickEvent[1] != -1)
  {
    LOGV("nativeOnInputDeviceAdded(), deviceId: %d\n", deviceId);

    if (write(sstAndroid.pipeJoystickEvent[1], &stJoystickEvent, sizeof(stJoystickEvent)) != sizeof(stJoystickEvent))
    {
      LOGE("Failure writing joystick event: %s\n", strerror(errno));
    }
  }
}

extern "C" void JNICALL Java_nativeOnInputDeviceChanged(JNIEnv* env, jobject thiz, jint deviceId)
{
  orxANDROID_JOYSTICK_EVENT stJoystickEvent;

  stJoystickEvent.u32Type = orxANDROID_EVENT_JOYSTICK_CHANGED;
  stJoystickEvent.u32DeviceId = deviceId;

  if(sstAndroid.pipeJoystickEvent[1] != -1)
  {
    if (write(sstAndroid.pipeJoystickEvent[1], &stJoystickEvent, sizeof(stJoystickEvent)) != sizeof(stJoystickEvent))
    {
      LOGE("Failure writing joystick event: %s\n", strerror(errno));
    }
  }
}

extern "C" void JNICALL Java_nativeOnInputDeviceRemoved(JNIEnv* env, jobject thiz, jint deviceId)
{
  orxANDROID_JOYSTICK_EVENT stJoystickEvent;

  stJoystickEvent.u32Type = orxANDROID_EVENT_JOYSTICK_REMOVED;
  stJoystickEvent.u32DeviceId = deviceId;

  if(sstAndroid.pipeJoystickEvent[1] != -1)
  {
    LOGV("nativeOnInputDeviceRemoved(), deviceId: %d\n", deviceId);

    if (write(sstAndroid.pipeJoystickEvent[1], &stJoystickEvent, sizeof(stJoystickEvent)) != sizeof(stJoystickEvent))
    {
      LOGE("Failure writing joystick event: %s\n", strerror(errno));
    }
  }
}

extern "C" ANativeActivity* orxAndroid_GetNativeActivity()
{
  return sstAndroid.app_->activity;
}

// TODO: Duplicated in orxAndroid(Native)Support.cpp. Refactor to orxAndroidCommon or similar
extern "C" const char * orxAndroid_GetInternalStoragePath()
{
  if (!sstAndroid.zAndroidInternalFilesPath)
  {
    jmethodID mid;
    jobject fileObject;
    jstring pathString;
    const char *path;

    JNIEnv *env = Android_JNI_GetEnv();

    mid = env->GetMethodID(env->GetObjectClass(orxAndroid_GetActivity()), "getFilesDir", "()Ljava/io/File;");
    fileObject = env->CallObjectMethod(orxAndroid_GetActivity(), mid);

    if (!fileObject)
    {
      LOGE("Couldn't get internal directory");
      return NULL;
    }

    // path = fileObject.getAbsolutePath();
    mid = env->GetMethodID(env->GetObjectClass(fileObject), "getAbsolutePath", "()Ljava/lang/String;");
    pathString = (jstring)env->CallObjectMethod(fileObject, mid);
    env->DeleteLocalRef(fileObject);

    path = env->GetStringUTFChars(pathString, NULL);
    sstAndroid.zAndroidInternalFilesPath = strdup(path);
    env->ReleaseStringUTFChars(pathString, path);
    env->DeleteLocalRef(pathString);
  }

  return sstAndroid.zAndroidInternalFilesPath;
}

// TODO: Duplicated in orxAndroid(Native)Support.cpp. Refactor to orxAndroidCommon or similar
extern "C" orxSTATUS orxAndroid_JNI_GetInputDevice(orxU32 _u32DeviceId, orxANDROID_JOYSTICK_INFO *pstJoystickInfo)
{
    jmethodID mid;
    jobject deviceObject;
    jclass inputDeviceClass;
    jstring deviceNameString;
    const char *deviceName;

    JNIEnv *env = Android_JNI_GetEnv();

    inputDeviceClass = env->FindClass("android/view/InputDevice");
    mid = env->GetStaticMethodID(inputDeviceClass, "getDevice", "(I)Landroid/view/InputDevice;");
    deviceObject = env->CallStaticObjectMethod(inputDeviceClass, mid, (jint)_u32DeviceId);

    if (!deviceObject)
    {
      LOGE("Couldn't get InputDevice.getDevice(%d)", (int)_u32DeviceId);
      env->DeleteLocalRef(inputDeviceClass);
      return orxSTATUS_FAILURE;
    }

    pstJoystickInfo->u32DeviceId = _u32DeviceId;

    mid = env->GetMethodID(inputDeviceClass, "getVendorId", "()I");
    pstJoystickInfo->u32VendorId = env->CallIntMethod(deviceObject, mid);

    mid = env->GetMethodID(inputDeviceClass, "getProductId", "()I");
    pstJoystickInfo->u32ProductId = env->CallIntMethod(deviceObject, mid);

    mid = env->GetMethodID(inputDeviceClass, "getDescriptor", "()Ljava/lang/String;");
    deviceNameString = (jstring)env->CallObjectMethod(deviceObject, mid);
    deviceName = env->GetStringUTFChars(deviceNameString, NULL);
    strncpy(pstJoystickInfo->descriptor, deviceName, sizeof(pstJoystickInfo->descriptor) - 1);
    pstJoystickInfo->descriptor[sizeof(pstJoystickInfo->descriptor) - 1] = orxCHAR_NULL;
    env->ReleaseStringUTFChars(deviceNameString, deviceName);
    env->DeleteLocalRef(deviceNameString);

    mid = env->GetMethodID(inputDeviceClass, "getName", "()Ljava/lang/String;");
    deviceNameString = (jstring)env->CallObjectMethod(deviceObject, mid);
    deviceName = env->GetStringUTFChars(deviceNameString, NULL);
    strncpy(pstJoystickInfo->name, deviceName, sizeof(pstJoystickInfo->name) - 1);
    pstJoystickInfo->name[sizeof(pstJoystickInfo->name) - 1] = orxCHAR_NULL;
    env->ReleaseStringUTFChars(deviceNameString, deviceName);
    env->DeleteLocalRef(deviceNameString);

    env->DeleteLocalRef(deviceObject);
    env->DeleteLocalRef(inputDeviceClass);

    return orxSTATUS_SUCCESS;
}

extern "C" orxU32 orxAndroid_JNI_GetRotation()
{
    JNIEnv *env = Android_JNI_GetEnv();
    jclass contextClass, windowManagerClass, displayClass;
    jfieldID field;
    jstring WINDOW_SERVICE;
    jmethodID getSystemService, getDefaultDisplay, getRotation;
    jobject context, windowManager, defaultDisplay;
    int rotation;

    context = orxAndroid_GetActivity();
    contextClass = env->FindClass("android/content/Context");
    field = env->GetStaticFieldID(contextClass, "WINDOW_SERVICE", "Ljava/lang/String;");
    WINDOW_SERVICE = (jstring) env->GetStaticObjectField(contextClass, field);
    getSystemService = env->GetMethodID(contextClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    env->DeleteLocalRef(contextClass);

    windowManager = env->CallObjectMethod(context, getSystemService,  WINDOW_SERVICE);
    windowManagerClass = env->FindClass("android/view/WindowManager");
    getDefaultDisplay = env->GetMethodID(windowManagerClass, "getDefaultDisplay", "()Landroid/view/Display;");
    env->DeleteLocalRef(windowManagerClass);
    env->DeleteLocalRef(WINDOW_SERVICE);

    defaultDisplay = env->CallObjectMethod(windowManager, getDefaultDisplay);
    displayClass = env->FindClass("android/view/Display");
    getRotation = env->GetMethodID(displayClass, "getRotation", "()I");
    rotation = env->CallIntMethod(defaultDisplay, getRotation);
    env->DeleteLocalRef(windowManager);
    env->DeleteLocalRef(defaultDisplay);
    env->DeleteLocalRef(displayClass);

    return rotation;
}

static int32_t handleInput(struct android_app* app, AInputEvent* event)
{
    int32_t type = AInputEvent_getType(event);
    int32_t deviceId = AInputEvent_getDeviceId(event);

    switch(type)
    {
    case AINPUT_EVENT_TYPE_MOTION:
    {
        if(sstAndroid.fSurfaceScale == orxFLOAT_0)
        {
            orxConfig_PushSection(KZ_CONFIG_ANDROID);
            sstAndroid.fSurfaceScale = orxConfig_GetFloat(KZ_CONFIG_SURFACE_SCALE);
            orxConfig_PopSection();
        }

        int32_t action = AMotionEvent_getAction(event);
        int32_t source = AInputEvent_getSource(event);
        unsigned int flags = action & AMOTION_EVENT_ACTION_MASK;

        // Touch/mouse or joystick ?

        // TODO: What about AINPUT_SOURCE_TRACKBALL & AINPUT_SOURCE_TOUCHPAD ?
        if ((source & AINPUT_SOURCE_TOUCHSCREEN) == AINPUT_SOURCE_TOUCHSCREEN ||
            (source & AINPUT_SOURCE_MOUSE) == AINPUT_SOURCE_MOUSE)
        {
            // touch screen
            orxSYSTEM_EVENT_PAYLOAD stPayload;
            /* Inits event's payload */
            orxMemory_Zero(&stPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
            stPayload.stTouch.fPressure = orxFLOAT_0;

            // only the primary pointer sends move messages so get the other pointer positions while we are here
            switch( flags )
            {
                case AMOTION_EVENT_ACTION_DOWN:
                    stPayload.stTouch.fX = sstAndroid.fSurfaceScale * orx2F(AMotionEvent_getX(event, 0));
                    stPayload.stTouch.fY = sstAndroid.fSurfaceScale * orx2F(AMotionEvent_getY(event, 0));
                    stPayload.stTouch.u32ID = AMotionEvent_getPointerId(event, 0);
                    orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_BEGIN, orxNULL, orxNULL, &stPayload);
                    break;
                case AMOTION_EVENT_ACTION_UP:
                case AMOTION_EVENT_ACTION_CANCEL:
                    stPayload.stTouch.fX = sstAndroid.fSurfaceScale * orx2F(AMotionEvent_getX(event, 0));
                    stPayload.stTouch.fY = sstAndroid.fSurfaceScale * orx2F(AMotionEvent_getY(event, 0));
                    stPayload.stTouch.u32ID = AMotionEvent_getPointerId(event, 0);
                    orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_END, orxNULL, orxNULL, &stPayload);
                    break;
                case AMOTION_EVENT_ACTION_MOVE:
                {
                    int32_t count = AMotionEvent_getPointerCount(event);

                    for(int i = 0; i < count; i++)
                    {
                      stPayload.stTouch.fX = sstAndroid.fSurfaceScale * orx2F(AMotionEvent_getX(event, i));
                      stPayload.stTouch.fY = sstAndroid.fSurfaceScale * orx2F(AMotionEvent_getY(event, i));
                      stPayload.stTouch.u32ID = AMotionEvent_getPointerId(event, i);
                      orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_MOVE, orxNULL, orxNULL, &stPayload);
                    }
                    break;
                }
                case AMOTION_EVENT_ACTION_POINTER_DOWN:
                {
                    int32_t iIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
                    stPayload.stTouch.u32ID = AMotionEvent_getPointerId(event, iIndex);
                    stPayload.stTouch.fX = sstAndroid.fSurfaceScale * orx2F(AMotionEvent_getX(event, iIndex));
                    stPayload.stTouch.fY = sstAndroid.fSurfaceScale * orx2F(AMotionEvent_getY(event, iIndex));
                    orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_BEGIN, orxNULL, orxNULL, &stPayload);
                    break;
                }
                case AMOTION_EVENT_ACTION_POINTER_UP:
                {
                    int32_t iIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
                    stPayload.stTouch.u32ID = AMotionEvent_getPointerId(event, iIndex);
                    stPayload.stTouch.fX = sstAndroid.fSurfaceScale * orx2F(AMotionEvent_getX(event, iIndex));
                    stPayload.stTouch.fY = sstAndroid.fSurfaceScale * orx2F(AMotionEvent_getY(event, iIndex));
                    orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_END, orxNULL, orxNULL, &stPayload);
                    break;
                }
            }
        }
        else if ( (source & AINPUT_SOURCE_JOYSTICK) == AINPUT_SOURCE_JOYSTICK )
        {
            // joystick

            if (flags != AMOTION_EVENT_ACTION_MOVE)
            {
                LOGE("Joystick motion event NOT action AMOTION_EVENT_ACTION_MOVE deviceId: %d\n", deviceId);
                return 0;
            }

            orxANDROID_JOYSTICK_EVENT stJoystickEvent;
            // Inits event's payload
            orxMemory_Zero(&stJoystickEvent, sizeof(orxANDROID_JOYSTICK_EVENT));

            stJoystickEvent.u32DeviceId = deviceId;
            int32_t pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

            switch( flags )
            {
                case AMOTION_EVENT_ACTION_DOWN:
                    stJoystickEvent.u32Type = orxANDROID_EVENT_JOYSTICK_DOWN;
                    break;
                case AMOTION_EVENT_ACTION_UP:
                    stJoystickEvent.u32Type = orxANDROID_EVENT_JOYSTICK_UP;
                    break;
                case AMOTION_EVENT_ACTION_MOVE:
                    stJoystickEvent.u32Type = orxANDROID_EVENT_JOYSTICK_MOVE;
                    break;
            }

            stJoystickEvent.stAxisData.fX = orx2F(AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_X, pointerIndex));
            stJoystickEvent.stAxisData.fY = orx2F(AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_Y, pointerIndex));
            stJoystickEvent.stAxisData.fZ = orx2F(AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_Z, pointerIndex));
            stJoystickEvent.stAxisData.fRZ = orx2F(AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_RZ, pointerIndex));
            stJoystickEvent.stAxisData.fLTRIGGER = orx2F(AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_LTRIGGER, pointerIndex));
            stJoystickEvent.stAxisData.fRTRIGGER = orx2F(AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_RTRIGGER, pointerIndex));

            orxEVENT_SEND(orxANDROID_EVENT_TYPE_JOYSTICK, 0, orxNULL, orxNULL, &stJoystickEvent);
        } else
        {
            return 0;
        }

        return 1;
    }

    case AINPUT_EVENT_TYPE_KEY:
    {
        int32_t action = AKeyEvent_getAction(event);
        int32_t keyCode = AKeyEvent_getKeyCode(event);
        int32_t source = AInputEvent_getSource(event);

        // Key or joystick ?
        if(keyCode != AKEYCODE_BACK && // BACK is a keyboard event
                ((source & AINPUT_SOURCE_JOYSTICK) == AINPUT_SOURCE_JOYSTICK ||
                 (source & AINPUT_SOURCE_GAMEPAD) == AINPUT_SOURCE_GAMEPAD))
        {
            orxANDROID_JOYSTICK_EVENT stJoystickEvent;
            /* Inits event's payload */
            orxMemory_Zero(&stJoystickEvent, sizeof(orxANDROID_JOYSTICK_EVENT));

            stJoystickEvent.u32Type = action == AKEY_EVENT_ACTION_DOWN ? orxANDROID_EVENT_JOYSTICK_DOWN : orxANDROID_EVENT_JOYSTICK_UP;
            stJoystickEvent.u32DeviceId = deviceId;
            stJoystickEvent.u32KeyCode = keyCode;

            orxEVENT_SEND(orxANDROID_EVENT_TYPE_JOYSTICK, 0, orxNULL, orxNULL, &stJoystickEvent);

            LOGV("Joystick EVENT sent, type: %s, keyCode: %d\n", (action == AKEY_EVENT_ACTION_DOWN ? "DOWN":"UP"), (int)keyCode);
        }
        else if((source & AINPUT_SOURCE_KEYBOARD) == AINPUT_SOURCE_KEYBOARD ||
                (source & AINPUT_SOURCE_DPAD) == AINPUT_SOURCE_DPAD)
        {
            // Keyboard and Dpad

            orxANDROID_KEY_EVENT stKeyEvent;
            stKeyEvent.u32KeyCode = keyCode;
            stKeyEvent.u32Action = action == AKEY_EVENT_ACTION_DOWN ? orxANDROID_EVENT_KEYBOARD_DOWN : orxANDROID_EVENT_KEYBOARD_UP;
            stKeyEvent.u32Unicode = 0; // not in Ndk, only in Java KeyEvent getUnicodeChar()

            if(action != AKEY_EVENT_ACTION_MULTIPLE)
            {
                orxEVENT_SEND(orxANDROID_EVENT_TYPE_KEYBOARD, 0, orxNULL, orxNULL, &stKeyEvent);
            }
            else
            {
                // TODO See OrxActivity.onKey()
                // https://stackoverflow.com/questions/21124051/receive-complete-android-unicode-input-in-c-c
            }
        }

        if(keyCode == AKEYCODE_VOLUME_UP || keyCode == AKEYCODE_VOLUME_DOWN)
            return 0;

        return 1;
    }

    default:
        break;
    }
    return 0;
}

void handleCmd( struct android_app* app, int32_t cmd )
{
    switch( cmd )
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
    case APP_CMD_TERM_WINDOW:
        LOGI("APP_CMD_TERM_WINDOW");

        sstAndroid.fSurfaceScale = orxFLOAT_0;
        orxEVENT_SEND(orxANDROID_EVENT_TYPE_SURFACE, orxANDROID_EVENT_SURFACE_DESTROYED, orxNULL, orxNULL, orxNULL);
        break;
    case APP_CMD_INIT_WINDOW:
        LOGI("APP_CMD_INIT_WINDOW");

        orxEVENT_SEND(orxANDROID_EVENT_TYPE_SURFACE, orxANDROID_EVENT_SURFACE_CREATED, orxNULL, orxNULL, orxNULL);
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
    case APP_CMD_CONFIG_CHANGED:
        LOGI("APP_CMD_CONFIG_CHANGED");
        break;
    default:
        break;
    }
}

static inline orxBOOL isInteractible()
{
  return (sstAndroid.app_->window && !sstAndroid.bPaused && sstAndroid.bHasFocus);
}

// Called from within Orx
extern "C" void orxAndroid_PumpEvents()
{
    // Read all pending events.
    int ident;
    int events;
    android_poll_source* source;

    // If not animating, we will block forever waiting for events.
    // If animating, we loop until all events are read, then continue
    // to draw the next frame of animation.
    while( (ident = ALooper_pollAll( isInteractible() ? 0 : -1, NULL, &events, (void**) &source )) >= 0 )
    {
        // Process this event.
        if( source != NULL )
            source->process( sstAndroid.app_, source );

        if (ident == LOOPER_ID_SENSOR)
        {
            orxEvent_SendShort(orxANDROID_EVENT_TYPE_ACCELERATE, 0);
        }

        // read orxANDROID_JOYSTICK_EVENTs coming from device listener in Activity
        if(ident == LOOPER_ID_JOYSTICK_EVENT)
        {
          orxANDROID_JOYSTICK_EVENT stJoystickEvent;

          if (read(sstAndroid.pipeJoystickEvent[0], &stJoystickEvent, sizeof(stJoystickEvent)) == sizeof(stJoystickEvent))
          {
            orxEVENT_SEND(orxANDROID_EVENT_TYPE_JOYSTICK, 0, orxNULL, orxNULL, &stJoystickEvent);
          } else {
            LOGE("No data on command pipe!");
          }
        }

        // Check if we are exiting.
        if( sstAndroid.app_->destroyRequested != 0 )
        {
            return;
        }
    }

    if(sstAndroid.app_->window != NULL)
    {
      // Check if window size changed
      int32_t newWidth = ANativeWindow_getWidth(sstAndroid.app_->window);
      int32_t newHeight = ANativeWindow_getHeight(sstAndroid.app_->window);

      if(newWidth != sstAndroid.lastWidth || newHeight != sstAndroid.lastHeight)
      {
          orxANDROID_SURFACE_CHANGED_EVENT stSurfaceChangedEvent;

          stSurfaceChangedEvent.u32Width = newWidth;
          stSurfaceChangedEvent.u32Height = newHeight;

          orxEVENT_SEND(orxANDROID_EVENT_TYPE_SURFACE, orxANDROID_EVENT_SURFACE_CHANGED, orxNULL, orxNULL, &stSurfaceChangedEvent);

          sstAndroid.lastWidth = newWidth;
          sstAndroid.lastHeight = newHeight;
          sstAndroid.fSurfaceScale = orxFLOAT_0;
      }
    }
}

/* Main function to call */
extern int main(int argc, char *argv[]);

void android_main( android_app* state )
{
    LOGI("android_main()");

    state->onAppCmd = handleCmd;
    state->onInputEvent = handleInput;

    /* Cleans static controller */
    memset(&sstAndroid, 0, sizeof(orxANDROID_STATIC));
    sstAndroid.pipeJoystickEvent[0] = -1;
    sstAndroid.pipeJoystickEvent[1] = -1;

    sstAndroid.app_ = state;
    sstAndroid.lastWidth = 0;
    sstAndroid.lastHeight = 0;
    sstAndroid.bPaused = orxTRUE;
    sstAndroid.bHasFocus = orxFALSE;
    sstAndroid.fSurfaceScale = orxFLOAT_0;

    // setup Looper pipe for joystick lifecycle events
    if (pipe(sstAndroid.pipeJoystickEvent)) {
        LOGE("could not create pipe: %s", strerror(errno));
        return;
    }

    jVM = state->activity->vm;

    /*
     * Create mThreadKey so we can keep track of the JNIEnv assigned to each thread
     * Refer to http://developer.android.com/guide/practices/design/jni.html for the rationale behind this
     */
    if (pthread_key_create(&sThreadKey, Android_JNI_ThreadDestroyed)) {
        __android_log_print(ANDROID_LOG_ERROR, "Orx", "Error initializing pthread key");
    }
    else {
        orxAndroid_JNI_SetupThread(orxNULL);
    }

    JNIEnv *env = Android_JNI_GetEnv();

    jobject jActivity = state->activity->clazz;

    // Looper for Joystick lifecycle events
    sstAndroid.looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    ALooper_addFd(sstAndroid.looper, sstAndroid.pipeJoystickEvent[0], LOOPER_ID_JOYSTICK_EVENT, ALOOPER_EVENT_INPUT, NULL, NULL);

    // register jni methods manually because the automatic registration does not happens when Activity is not loading system library.
    JNINativeMethod methodTable[]=
      {
          {"nativeOnInputDeviceAdded", "(I)V", (void *)Java_nativeOnInputDeviceAdded},
          {"nativeOnInputDeviceChanged", "(I)V", (void *)Java_nativeOnInputDeviceChanged},
          {"nativeOnInputDeviceRemoved", "(I)V", (void *)Java_nativeOnInputDeviceRemoved}
      };
    int methodTableSize=sizeof(methodTable)/sizeof(methodTable[0]);
    env->RegisterNatives(env->GetObjectClass(jActivity), methodTable, methodTableSize);
    if (env->ExceptionCheck()) {
       env->ExceptionClear();
       LOGW("Couldn't locate Java method(s) nativeOnInputDevice[Added|Change|Removed], provided in OrxNativeActivity.java. Joystick add/remove tracking in Orx is now disabled.");
    }

    /* Run the application code! */
    main(0, orxNULL);

    if(sstAndroid.zAndroidInternalFilesPath)
    {
      free(sstAndroid.zAndroidInternalFilesPath);
    }

    ALooper_removeFd(sstAndroid.looper, sstAndroid.pipeJoystickEvent[0]);
    close(sstAndroid.pipeJoystickEvent[0]);
    close(sstAndroid.pipeJoystickEvent[1]);
    sstAndroid.pipeJoystickEvent[0] = -1;
    sstAndroid.pipeJoystickEvent[1] = -1;

    if(state->destroyRequested == 0)
    {
        ANativeActivity_finish(state->activity);

        // pumps final events
        int id;
        int events;
        android_poll_source* source;

        state->onAppCmd = NULL;
        state->onInputEvent = NULL;

        while( (id = ALooper_pollAll(-1, NULL, &events, (void**) &source )) >= 0 )
        {
            // Process this event.
            if( source != NULL )
                source->process( state, source );

            // Check if we are exiting.
            if( state->destroyRequested != 0 )
            {
                return;
            }
        }
    }
}


// APK orxRESOURCE

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
  poAsset = AAssetManager_open(sstAndroid.app_->activity->assetManager, s_acFileLocationBuffer, AASSET_MODE_RANDOM);
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
    poAsset = AAssetManager_open(sstAndroid.app_->activity->assetManager, _zLocation, AASSET_MODE_RANDOM);

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
