/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2018 Orx-Project
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

#include <jni.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/looper.h>
#include <android/sensor.h>
#include <android/native_window_jni.h>

#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>

#ifdef __orxDEBUG__

#define MODULE "orxAndroidSupport"
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,MODULE,__VA_ARGS__)
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,MODULE,__VA_ARGS__)

#else

#define LOGE(...)
#define LOGI(...)

#endif

#include "orxInclude.h"
#include "orxKernel.h"
#include "main/orxAndroid.h"

/** Static structure
 */
typedef struct __orxANDROID_STATIC_t {
        // Hosting Activity
        jobject mActivity;

        // method signatures
        jmethodID midGetRotation;
        jmethodID midGetDeviceIds;

        // AssetManager
        AAssetManager *poAssetManager;
        jobject jAssetManager;
        char *zAndroidInternalFilesPath;

        // looper stufs
        ALooper* looper;
        int pipeCmd[2];
        int pipeTouchEvent[2];
        int pipeKeyEvent[2];
        int pipeJoystickEvent[2];

        orxBOOL bPaused;
        orxBOOL bHasFocus;
        orxBOOL bDestroyRequested;
        orxFLOAT fSurfaceScale;

        ANativeWindow* pendingWindow;
        ANativeWindow* window;

        orxU32 u32SurfaceWidth;
        orxU32 u32SurfaceHeight;

        pthread_mutex_t mutex;
        pthread_cond_t cond;

} orxANDROID_STATIC;

/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxANDROID_STATIC sstAndroid;
static pthread_key_t mThreadKey;
static JavaVM* mJavaVM;

/*******************************************************************************
                               Globals
*******************************************************************************/

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
    int status = mJavaVM->AttachCurrentThread(&env, NULL);
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
        mJavaVM->DetachCurrentThread();
        pthread_setspecific(mThreadKey, NULL);
    }
}

void orxAndroid_JNI_SetupThread(void) {
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
    pthread_setspecific(mThreadKey, (void*) env);
}

// Library init
extern "C" jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv *env;
    mJavaVM = vm;
    LOGI("JNI_OnLoad called");
    if (mJavaVM->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGE("Failed to get the environment using GetEnv()");
        return -1;
    }
    /*
     * Create mThreadKey so we can keep track of the JNIEnv assigned to each thread
     * Refer to http://developer.android.com/guide/practices/design/jni.html for the rationale behind this
     */
    if (pthread_key_create(&mThreadKey, Android_JNI_ThreadDestroyed)) {
        __android_log_print(ANDROID_LOG_ERROR, "Orx", "Error initializing pthread key");
    }
    else {
        orxAndroid_JNI_SetupThread();
    }

    return JNI_VERSION_1_4;
}

int8_t app_read_cmd() {
    int8_t cmd;
    if (read(sstAndroid.pipeCmd[0], &cmd, sizeof(cmd)) == sizeof(cmd)) {
        return cmd;
    } else {
        LOGE("No data on command pipe!");
    }
    return -1;
}

static void app_write_cmd(int8_t cmd) {
    if(sstAndroid.pipeCmd[1] != -1) {
        if (write(sstAndroid.pipeCmd[1], &cmd, sizeof(cmd)) != sizeof(cmd)) {
            LOGE("Failure writing android_app cmd: %s\n", strerror(errno));
        }
    }
}

static void app_set_window(ANativeWindow* window) {
    pthread_mutex_lock(&sstAndroid.mutex);
    if (sstAndroid.pendingWindow != NULL) {
        app_write_cmd(APP_CMD_SURFACE_DESTROYED);
    }
    sstAndroid.pendingWindow = window;
    if (window != NULL) {
        app_write_cmd(APP_CMD_SURFACE_CREATED);
    }
    while (sstAndroid.window != sstAndroid.pendingWindow) {
        pthread_cond_wait(&sstAndroid.cond, &sstAndroid.mutex);
    }
    pthread_mutex_unlock(&sstAndroid.mutex);
}

// Called before main() to initialize JNI bindings
static void orxAndroid_Init(JNIEnv* mEnv, jobject jActivity)
{
    LOGI("orxAndroid_Init()");

    jclass objClass;

    orxAndroid_JNI_SetupThread();

    sstAndroid.mActivity = mEnv->NewGlobalRef(jActivity);

    objClass = mEnv->FindClass("org/orx/lib/OrxActivity");
    sstAndroid.midGetRotation = mEnv->GetMethodID(objClass, "getRotation","()I");
    sstAndroid.midGetDeviceIds = mEnv->GetMethodID(objClass, "getDeviceIds", "()[I");

    if(!sstAndroid.midGetRotation
       || !sstAndroid.midGetDeviceIds) {
        __android_log_print(ANDROID_LOG_WARN, "Orx", "Couldn't locate Java callbacks, check that they're named and typed correctly");
    }

    // setup AssetManager
    jmethodID midGetAssets = mEnv->GetMethodID(objClass, "getAssets", "()Landroid/content/res/AssetManager;");
    jobject jAssetManager = mEnv->CallObjectMethod(jActivity, midGetAssets);
    sstAndroid.jAssetManager = mEnv->NewGlobalRef(jAssetManager);
    sstAndroid.poAssetManager = AAssetManager_fromJava(mEnv, sstAndroid.jAssetManager);

    mEnv->DeleteLocalRef(objClass);
    mEnv->DeleteLocalRef(jActivity);
    mEnv->DeleteLocalRef(jAssetManager);

    sstAndroid.looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);

    ALooper_addFd(sstAndroid.looper, sstAndroid.pipeCmd[0], LOOPER_ID_MAIN, ALOOPER_EVENT_INPUT, NULL, NULL);
    ALooper_addFd(sstAndroid.looper, sstAndroid.pipeKeyEvent[0], LOOPER_ID_KEY_EVENT, ALOOPER_EVENT_INPUT, NULL, NULL);
    ALooper_addFd(sstAndroid.looper, sstAndroid.pipeTouchEvent[0], LOOPER_ID_TOUCH_EVENT, ALOOPER_EVENT_INPUT, NULL, NULL);
    ALooper_addFd(sstAndroid.looper, sstAndroid.pipeJoystickEvent[0], LOOPER_ID_JOYSTICK_EVENT, ALOOPER_EVENT_INPUT, NULL, NULL);

    LOGI("orxAndroid_Init() finished!");
}

static void orxAndroid_Exit(JNIEnv* env)
{
  // finish Activity
  jclass objClass = env->FindClass("android/app/Activity");
  jmethodID finish = env->GetMethodID(objClass, "finish", "()V");
  env->CallVoidMethod(sstAndroid.mActivity, finish);
  env->DeleteLocalRef(objClass);

  // wait for quit cmd
  int ident;
  int events;

  while ((ident=ALooper_pollAll(-1, NULL, &events, NULL)) >= 0)
  {
    if(ident == LOOPER_ID_MAIN)
    {
      int8_t cmd = app_read_cmd();

      if(cmd == APP_CMD_QUIT) {
        LOGI("APP_CMD_QUIT");
        break;
      }
      if(cmd == APP_CMD_SURFACE_DESTROYED) {
        LOGI("APP_CMD_SURFACE_DESTROYED");
        pthread_cond_broadcast(&sstAndroid.cond);

        pthread_mutex_lock(&sstAndroid.mutex);
        if(sstAndroid.window != NULL)
        {
          ANativeWindow_release(sstAndroid.window);
          sstAndroid.window = NULL;
        }
        pthread_cond_broadcast(&sstAndroid.cond);
        pthread_mutex_unlock(&sstAndroid.mutex);
      }
    }
  }

  env->DeleteGlobalRef(sstAndroid.mActivity);
  env->DeleteGlobalRef(sstAndroid.jAssetManager);

  if(sstAndroid.zAndroidInternalFilesPath)
  {
    free(sstAndroid.zAndroidInternalFilesPath);
  }

  if(sstAndroid.window != orxNULL)
  {
    ANativeWindow_release(sstAndroid.window);
    sstAndroid.window = orxNULL;
  }

  ALooper_removeFd(sstAndroid.looper, sstAndroid.pipeCmd[0]);
  ALooper_removeFd(sstAndroid.looper, sstAndroid.pipeKeyEvent[0]);
  ALooper_removeFd(sstAndroid.looper, sstAndroid.pipeTouchEvent[0]);
  ALooper_removeFd(sstAndroid.looper, sstAndroid.pipeJoystickEvent[0]);

  close(sstAndroid.pipeCmd[0]);
  close(sstAndroid.pipeCmd[1]);
  sstAndroid.pipeCmd[0] = -1;
  sstAndroid.pipeCmd[1] = -1;

  close(sstAndroid.pipeTouchEvent[0]);
  close(sstAndroid.pipeTouchEvent[1]);
  sstAndroid.pipeTouchEvent[0] = -1;
  sstAndroid.pipeTouchEvent[1] = -1;

  close(sstAndroid.pipeKeyEvent[0]);
  close(sstAndroid.pipeKeyEvent[1]);
  sstAndroid.pipeKeyEvent[0] = -1;
  sstAndroid.pipeKeyEvent[1] = -1;

  close(sstAndroid.pipeJoystickEvent[0]);
  close(sstAndroid.pipeJoystickEvent[1]);
  sstAndroid.pipeJoystickEvent[0] = -1;
  sstAndroid.pipeJoystickEvent[1] = -1;

  pthread_cond_destroy(&sstAndroid.cond);
  pthread_mutex_destroy(&sstAndroid.mutex);
}

/* Main function to call */
extern int main(int argc, char *argv[]);

extern "C" void JNICALL Java_org_orx_lib_OrxActivity_nativeOnCreate(JNIEnv *env, jobject thiz)
{
    LOGI("nativeCreate()");

    /* Cleans static controller */
    memset(&sstAndroid, 0, sizeof(orxANDROID_STATIC));

    sstAndroid.bPaused = orxFALSE;
    sstAndroid.bHasFocus = orxFALSE;
    sstAndroid.bDestroyRequested = orxFALSE;
    sstAndroid.window = orxNULL;
    sstAndroid.fSurfaceScale = orxFLOAT_0;

    // setup looper for commandes
    if (pipe(sstAndroid.pipeCmd)) {
        LOGE("could not create pipe: %s", strerror(errno));
        return;
    }

    // setup looper for key events
    if (pipe(sstAndroid.pipeKeyEvent)) {
        LOGE("could not create pipe: %s", strerror(errno));
        return;
    }

    // setup looper for joystick events
    if (pipe(sstAndroid.pipeJoystickEvent)) {
        LOGE("could not create pipe: %s", strerror(errno));
        return;
    }

    // setup looper for touch events
    if (pipe(sstAndroid.pipeTouchEvent)) {
        LOGE("could not create pipe: %s", strerror(errno));
        return;
    }

    pthread_mutex_init(&sstAndroid.mutex, NULL);
    pthread_cond_init(&sstAndroid.cond, NULL);
}

// Start up the Orx app
extern "C" void JNICALL Java_org_orx_lib_OrxActivity_startOrx(JNIEnv* env, jobject thiz, jobject activity)
{
    /* This interface could expand with ABI negotiation, calbacks, etc. */
    orxAndroid_Init(env, activity);

    /* Run the application code! */
    int status;
    status = main(0, orxNULL);

    orxAndroid_Exit(env);

    LOGI("startOrx() finished!");
}

// Keydown
extern "C" void JNICALL Java_org_orx_lib_OrxActivity_nativeOnKeyDown(JNIEnv* env, jobject thiz, jint keycode, jint unicode)
{
  orxANDROID_KEY_EVENT stKeyEvent;

  stKeyEvent.u32Action = orxANDROID_EVENT_KEYBOARD_DOWN;
  stKeyEvent.u32KeyCode = keycode;
  stKeyEvent.u32Unicode = unicode;

  if(sstAndroid.pipeKeyEvent[1] != -1)
  {
    if(write(sstAndroid.pipeKeyEvent[1], &stKeyEvent, sizeof(stKeyEvent)) != sizeof(stKeyEvent))
    {
      LOGE("Failure writing keycode: %s\n", strerror(errno));
    }
  }
}

// Keyup
extern "C" void JNICALL Java_org_orx_lib_OrxActivity_nativeOnKeyUp(JNIEnv* env, jobject thiz, jint keycode)
{
  orxANDROID_KEY_EVENT stKeyEvent;

  stKeyEvent.u32Action = orxANDROID_EVENT_KEYBOARD_UP;
  stKeyEvent.u32KeyCode = keycode;

  if(sstAndroid.pipeKeyEvent[1] != -1)
  {
    if (write(sstAndroid.pipeKeyEvent[1], &stKeyEvent, sizeof(stKeyEvent)) != sizeof(stKeyEvent))
    {
      LOGE("Failure writing keycode: %s\n", strerror(errno));
    }
  }
}

// Touch
extern "C" void JNICALL Java_org_orx_lib_OrxActivity_nativeOnTouch(
                                    JNIEnv* env, jobject thiz,
                                    jint touch_device_id_in, jint pointer_finger_id_in,
                                    jint action, jint x, jint y, jint p)
{
  orxANDROID_TOUCH_EVENT stTouchEvent;

  stTouchEvent.u32ID = pointer_finger_id_in;
  stTouchEvent.u32Action = action;
  stTouchEvent.fX = orx2F(x);
  stTouchEvent.fY = orx2F(y);

  if(sstAndroid.pipeTouchEvent[1] != -1)
  {
    if (write(sstAndroid.pipeTouchEvent[1], &stTouchEvent, sizeof(stTouchEvent)) != sizeof(stTouchEvent))
    {
      LOGE("Failure writing touch event: %s\n", strerror(errno));
    }
  }
}

// Quit
extern "C" void JNICALL Java_org_orx_lib_OrxActivity_stopOrx(JNIEnv* env, jobject thiz)
{
  app_write_cmd(APP_CMD_QUIT);
}

// Pause
extern "C" void JNICALL Java_org_orx_lib_OrxActivity_nativeOnPause(JNIEnv* env, jobject thiz)
{
  app_write_cmd(APP_CMD_PAUSE);
}

// Resume
extern "C" void JNICALL Java_org_orx_lib_OrxActivity_nativeOnResume(JNIEnv* env, jobject thiz)
{
  app_write_cmd(APP_CMD_RESUME);
}

// SurfaceDestroyed
extern "C" void JNICALL Java_org_orx_lib_OrxActivity_nativeOnSurfaceDestroyed(JNIEnv* env, jobject thiz)
{
  app_set_window(NULL);
}

// SurfaceCreated
extern "C" void JNICALL Java_org_orx_lib_OrxActivity_nativeOnSurfaceCreated(JNIEnv* env, jobject thiz, jobject surface)
{
  ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
  app_set_window(window);
}

// SurfaceChanged
extern "C" void JNICALL Java_org_orx_lib_OrxActivity_nativeOnSurfaceChanged(JNIEnv* env, jobject thiz, jint width, jint height)
{
  sstAndroid.u32SurfaceWidth = width;
  sstAndroid.u32SurfaceHeight = height;
  app_write_cmd(APP_CMD_SURFACE_CHANGED);
}

// Focus gained / lost
extern "C" void JNICALL Java_org_orx_lib_OrxActivity_nativeOnFocusChanged(JNIEnv* env, jobject thiz, jboolean hasFocus)
{
  if(hasFocus == JNI_TRUE)
  {
    app_write_cmd(APP_CMD_FOCUS_GAINED);
  }
  else
  {
    app_write_cmd(APP_CMD_FOCUS_LOST);
  }
}

extern "C" void JNICALL Java_org_orx_lib_OrxActivity_nativeOnInputDeviceAdded(JNIEnv* env, jobject thiz, jint deviceId)
{
  orxANDROID_JOYSTICK_EVENT stJoystickEvent;

  stJoystickEvent.u32Type = orxANDROID_EVENT_JOYSTICK_ADDED;
  stJoystickEvent.u32DeviceId = deviceId;

  if(sstAndroid.pipeJoystickEvent[1] != -1)
  {
    if (write(sstAndroid.pipeJoystickEvent[1], &stJoystickEvent, sizeof(stJoystickEvent)) != sizeof(stJoystickEvent))
    {
      LOGE("Failure writing joystick event: %s\n", strerror(errno));
    }
  }
}

extern "C" void JNICALL Java_org_orx_lib_OrxActivity_nativeOnInputDeviceChanged(JNIEnv* env, jobject thiz, jint deviceId)
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

extern "C" void JNICALL Java_org_orx_lib_OrxActivity_nativeOnInputDeviceRemoved(JNIEnv* env, jobject thiz, jint deviceId)
{
  orxANDROID_JOYSTICK_EVENT stJoystickEvent;

  stJoystickEvent.u32Type = orxANDROID_EVENT_JOYSTICK_REMOVED;
  stJoystickEvent.u32DeviceId = deviceId;

  if(sstAndroid.pipeJoystickEvent[1] != -1)
  {
    if (write(sstAndroid.pipeJoystickEvent[1], &stJoystickEvent, sizeof(stJoystickEvent)) != sizeof(stJoystickEvent))
    {
      LOGE("Failure writing joystick event: %s\n", strerror(errno));
    }
  }
}

extern "C" void JNICALL Java_org_orx_lib_OrxActivity_nativeOnJoystickMove(JNIEnv* env, jobject thiz, jint deviceId, jfloatArray axis)
{
  orxANDROID_JOYSTICK_EVENT stJoystickEvent;

  stJoystickEvent.u32Type = orxANDROID_EVENT_JOYSTICK_MOVE;
  stJoystickEvent.u32DeviceId = deviceId;

  env->GetFloatArrayRegion(axis, 0, 8, &stJoystickEvent.stAxisData.afValues[0]);

  if(sstAndroid.pipeJoystickEvent[1] != -1)
  {
    if (write(sstAndroid.pipeJoystickEvent[1], &stJoystickEvent, sizeof(stJoystickEvent)) != sizeof(stJoystickEvent))
    {
      LOGE("Failure writing joystick event: %s\n", strerror(errno));
    }
  }
}

extern "C" void JNICALL Java_org_orx_lib_OrxActivity_nativeOnJoystickDown(JNIEnv* env, jobject thiz, jint deviceId, jint keycode)
{
  orxANDROID_JOYSTICK_EVENT stJoystickEvent;

  stJoystickEvent.u32Type = orxANDROID_EVENT_JOYSTICK_DOWN;
  stJoystickEvent.u32DeviceId = deviceId;
  stJoystickEvent.u32KeyCode = keycode;

  if(sstAndroid.pipeJoystickEvent[1] != -1)
  {
    if (write(sstAndroid.pipeJoystickEvent[1], &stJoystickEvent, sizeof(stJoystickEvent)) != sizeof(stJoystickEvent))
    {
      LOGE("Failure writing joystick event: %s\n", strerror(errno));
    }
  }
}

extern "C" void JNICALL Java_org_orx_lib_OrxActivity_nativeOnJoystickUp(JNIEnv* env, jobject thiz, jint deviceId, jint keycode)
{
  orxANDROID_JOYSTICK_EVENT stJoystickEvent;

  stJoystickEvent.u32Type = orxANDROID_EVENT_JOYSTICK_UP;
  stJoystickEvent.u32DeviceId = deviceId;
  stJoystickEvent.u32KeyCode = keycode;

  if(sstAndroid.pipeJoystickEvent[1] != -1)
  {
    if (write(sstAndroid.pipeJoystickEvent[1], &stJoystickEvent, sizeof(stJoystickEvent)) != sizeof(stJoystickEvent))
    {
      LOGE("Failure writing joystick event: %s\n", strerror(errno));
    }
  }
}

extern "C" ANativeWindow* orxAndroid_GetNativeWindow()
{
  int ident;
  int events;

  LOGI("orxAndroid_GetNativeWindow()");

  while(sstAndroid.window == NULL && !sstAndroid.bDestroyRequested)
  {
    LOGI("no window received yet");
    orxAndroid_PumpEvents();
  }

  return sstAndroid.window;
}

extern "C" orxU32 orxAndroid_JNI_GetRotation()
{
  JNIEnv *env = Android_JNI_GetEnv();
  jint rotation = env->CallIntMethod(sstAndroid.mActivity, sstAndroid.midGetRotation);
  return rotation;
}

extern "C" void *orxAndroid_GetJNIEnv()
{
  return Android_JNI_GetEnv();
}

extern "C" jobject orxAndroid_GetActivity()
{
  return sstAndroid.mActivity;
}

extern "C" void orxAndroid_JNI_GetDeviceIds(orxS32 deviceIds[4])
{
  JNIEnv *env = Android_JNI_GetEnv();
  jintArray retval = (jintArray) env->CallObjectMethod(sstAndroid.mActivity, sstAndroid.midGetDeviceIds);
  env->GetIntArrayRegion(retval, 0, 4, (jint*) &deviceIds[0]);
  env->DeleteLocalRef(retval);
}

extern "C" const char * orxAndroid_GetInternalStoragePath()
{
  if (!sstAndroid.zAndroidInternalFilesPath)
  {
    jmethodID mid;
    jobject fileObject;
    jstring pathString;
    const char *path;
    jobject jActivity;

    JNIEnv *env = Android_JNI_GetEnv();

    mid = env->GetMethodID(env->GetObjectClass(sstAndroid.mActivity), "getFilesDir", "()Ljava/io/File;");
    fileObject = env->CallObjectMethod(sstAndroid.mActivity, mid);

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

static inline orxBOOL isInteractible()
{
  return (sstAndroid.window && !sstAndroid.bPaused && sstAndroid.bHasFocus);
}

extern "C" void orxAndroid_PumpEvents()
{
  int ident;
  int events;

  while ((ident=ALooper_pollAll(isInteractible() || sstAndroid.bDestroyRequested == orxTRUE ? 0 : -1, NULL, &events, NULL)) >= 0)
  {
    if(ident == LOOPER_ID_MAIN)
    {
      int8_t cmd = app_read_cmd();

      if(cmd == APP_CMD_PAUSE) {
        LOGI("APP_CMD_PAUSE");
        sstAndroid.bPaused = orxTRUE;
        orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_BACKGROUND);
      }
      if(cmd == APP_CMD_RESUME) {
        LOGI("APP_CMD_RESUME");
        sstAndroid.bPaused = orxFALSE;
        orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_FOREGROUND);
      }
      if(cmd == APP_CMD_SURFACE_DESTROYED) {
        LOGI("APP_CMD_SURFACE_DESTROYED");
        pthread_cond_broadcast(&sstAndroid.cond);

        sstAndroid.fSurfaceScale = orxFLOAT_0;
        orxEVENT_SEND(orxANDROID_EVENT_TYPE_SURFACE, orxANDROID_EVENT_SURFACE_DESTROYED, orxNULL, orxNULL, orxNULL);

        pthread_mutex_lock(&sstAndroid.mutex);
        if(sstAndroid.window != NULL)
        {
          ANativeWindow_release(sstAndroid.window);
          sstAndroid.window = NULL;
        }
        pthread_cond_broadcast(&sstAndroid.cond);
        pthread_mutex_unlock(&sstAndroid.mutex);
      }
      if(cmd == APP_CMD_SURFACE_CHANGED) {
        LOGI("APP_CMD_SURFACE_CHANGED");
        orxANDROID_SURFACE_CHANGED_EVENT stSurfaceChangedEvent;
        stSurfaceChangedEvent.u32Width = sstAndroid.u32SurfaceWidth;
        stSurfaceChangedEvent.u32Height = sstAndroid.u32SurfaceHeight;
        sstAndroid.fSurfaceScale = orxFLOAT_0;
        orxEVENT_SEND(orxANDROID_EVENT_TYPE_SURFACE, orxANDROID_EVENT_SURFACE_CHANGED, orxNULL, orxNULL, &stSurfaceChangedEvent);
      }
      if(cmd == APP_CMD_SURFACE_CREATED) {
        LOGI("APP_CMD_SURFACE_CREATED");
        pthread_mutex_lock(&sstAndroid.mutex);
        sstAndroid.window = sstAndroid.pendingWindow;
        pthread_cond_broadcast(&sstAndroid.cond);
        pthread_mutex_unlock(&sstAndroid.mutex);

        orxEVENT_SEND(orxANDROID_EVENT_TYPE_SURFACE, orxANDROID_EVENT_SURFACE_CREATED, orxNULL, orxNULL, orxNULL);
      }
      if(cmd == APP_CMD_QUIT) {
        LOGI("APP_CMD_QUIT");
        sstAndroid.bDestroyRequested = orxTRUE;
        orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_CLOSE);
      }
      if(cmd == APP_CMD_FOCUS_GAINED) {
        LOGI("APP_CMD_FOCUS_GAINED");
        sstAndroid.bHasFocus = orxTRUE;
        orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_FOCUS_GAINED);
      }
      if(cmd == APP_CMD_FOCUS_LOST) {
        LOGI("APP_CMD_FOCUS_LOST");
        sstAndroid.bHasFocus = orxFALSE;
        orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_FOCUS_LOST);
      }
    }

    if(ident == LOOPER_ID_SENSOR)
    {
      orxEvent_SendShort(orxANDROID_EVENT_TYPE_ACCELERATE, 0);
    }

    if(ident == LOOPER_ID_KEY_EVENT)
    {
      orxANDROID_KEY_EVENT stKeyEvent;

      if (read(sstAndroid.pipeKeyEvent[0], &stKeyEvent, sizeof(stKeyEvent)) == sizeof(stKeyEvent))
      {
        orxEVENT_SEND(orxANDROID_EVENT_TYPE_KEYBOARD, 0, orxNULL, orxNULL, &stKeyEvent);
      } else {
        LOGE("No data on command pipe!");
      }
    }

    if(ident == LOOPER_ID_TOUCH_EVENT)
    {
      orxANDROID_TOUCH_EVENT stTouchEvent;

      if (read(sstAndroid.pipeTouchEvent[0], &stTouchEvent, sizeof(stTouchEvent)) == sizeof(stTouchEvent))
      {
        orxSYSTEM_EVENT_PAYLOAD stPayload;

        if(sstAndroid.fSurfaceScale == orxFLOAT_0)
        {
          orxConfig_PushSection(KZ_CONFIG_ANDROID);
          sstAndroid.fSurfaceScale = orxConfig_GetFloat(KZ_CONFIG_SURFACE_SCALE);
          orxConfig_PopSection();
        }

        /* Inits event's payload */
        orxMemory_Zero(&stPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
        stPayload.stTouch.fPressure = orxFLOAT_0;
        stPayload.stTouch.fX = sstAndroid.fSurfaceScale * stTouchEvent.fX;
        stPayload.stTouch.fY = sstAndroid.fSurfaceScale * stTouchEvent.fY;
        stPayload.stTouch.u32ID = stTouchEvent.u32ID;

        switch(stTouchEvent.u32Action)
        {
        case 0: // MotionEvent.ACTION_DOWN
        case 5: // MotionEvent.ACTION_POINTER_DOWN
          orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_BEGIN, orxNULL, orxNULL, &stPayload);
          break;
        case 1: // MotionEvent.ACTION_UP
        case 6: // MotionEvent.ACTION_POINTER_UP
          orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_END, orxNULL, orxNULL, &stPayload);
          break;
        case 2: // MotionEvent.ACTION_MOVE
          orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_MOVE, orxNULL, orxNULL, &stPayload);
          break;
        }
      } else {
        LOGE("No data on command pipe!");
      }
    }

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
  poAsset = AAssetManager_open(sstAndroid.poAssetManager, s_acFileLocationBuffer, AASSET_MODE_RANDOM);
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
    poAsset = AAssetManager_open(sstAndroid.poAssetManager, _zLocation, AASSET_MODE_RANDOM);

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
