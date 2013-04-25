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
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/looper.h>
#include <android/sensor.h>

#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>

#define DEBUG_ANDROID_SUPPORT

#ifdef DEBUG_ANDROID_SUPPORT

#define MODULE "orxAndroidSupport"
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,MODULE,__VA_ARGS__)
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,MODULE,__VA_ARGS__)
#define DEBUG_JNI

#else

#define LOGE(...)
#define LOGI(...)

#endif

#include "orxInclude.h"
#include "orxKernel.h"
#include "main/orxAndroid.h"
#include <EGL/egl.h>

/** Static structure
 */
typedef struct __orxANDROID_STATIC_t {
	orxU32 u32Flags;

        // Main activity
        jobject mActivity;

        // method signatures
        jmethodID midCreateGLContext;
        jmethodID midFlipBuffers;
        jmethodID midGetRotation;

        // AssetManager
        AAssetManager *poAssetManager;
        jobject jAssetManager;
        char *s_AndroidInternalFilesPath;

        // looper stufs
        ALooper* looper;
        int pipeCmd[2];
        int pipeTouchEvent[2];
        int pipeKeyEvent[2];

        orxBOOL bSurfaceReady;
        orxBOOL bPaused;

} orxANDROID_STATIC;

typedef struct __orxANDROID_TOUCH_EVENT_t {
        orxU32   u32ID;
        orxFLOAT fX;
        orxFLOAT fY;
        orxU32   u32Action;

} orxANDROID_TOUCH_EVENT;

typedef struct __orxANDROID_KEY_EVENT_t {
       orxU32 u32Action;
       orxU32 u32KeyCode;

} orxANDROID_KEY_EVENT;

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

// TODO move this to Display plugin
int s_winWidth;
int s_winHeight;
// TODO move this to Joystick plugin
ASensorEventQueue* sensorEventQueue;

JNIEnv* Android_JNI_GetEnv(void) {
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

int Android_JNI_SetupThread(void) {
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
    return 1;
}

// Library init
extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved)
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
        Android_JNI_SetupThread();
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
    if (write(sstAndroid.pipeCmd[1], &cmd, sizeof(cmd)) != sizeof(cmd)) {
        LOGE("Failure writing android_app cmd: %s\n", strerror(errno));
    }
}

// Called before main() to initialize JNI bindings
static void orxAndroid_Init(JNIEnv* mEnv, jobject thiz)
{
    __android_log_print(ANDROID_LOG_INFO, "Orx", "orxAndroid_Init()");

    /* Cleans static controller */
    orxMemory_Zero(&sstAndroid, sizeof(orxANDROID_STATIC));

    Android_JNI_SetupThread();

    sstAndroid.mActivity = mEnv->NewGlobalRef(thiz);

    jclass objClass = mEnv->GetObjectClass(thiz);

    sstAndroid.midCreateGLContext = mEnv->GetMethodID(objClass, "createGLContext","(II[I)Z");
    sstAndroid.midFlipBuffers = mEnv->GetMethodID(objClass, "flipBuffers","()V");
    sstAndroid.midGetRotation = mEnv->GetMethodID(objClass, "getRotation","()I");

    if(!sstAndroid.midCreateGLContext || !sstAndroid.midFlipBuffers || !sstAndroid.midGetRotation) {
        __android_log_print(ANDROID_LOG_WARN, "Orx", "Orx: Couldn't locate Java callbacks, check that they're named and typed correctly");
    }

    // setup AssetManager
    jmethodID midGetAssets = mEnv->GetMethodID(objClass, "getAssets", "()Landroid/content/res/AssetManager;");
    jobject jAssetManager = mEnv->CallObjectMethod(thiz, midGetAssets);
    sstAndroid.jAssetManager = mEnv->NewGlobalRef(jAssetManager);
    sstAndroid.poAssetManager = AAssetManager_fromJava(mEnv, sstAndroid.jAssetManager);

    sstAndroid.bSurfaceReady = orxTRUE;
    sstAndroid.bPaused = orxFALSE;
    sstAndroid.looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);

    // setup looper for commandes
    if (pipe(sstAndroid.pipeCmd)) {
        LOGE("could not create pipe: %s", strerror(errno));
        return;
    }

    ALooper_addFd(sstAndroid.looper, sstAndroid.pipeCmd[0], LOOPER_ID_MAIN, ALOOPER_EVENT_INPUT, NULL, NULL);

    // setup looper for key events
    if (pipe(sstAndroid.pipeKeyEvent)) {
        LOGE("could not create pipe: %s", strerror(errno));
        return;
    }

    ALooper_addFd(sstAndroid.looper, sstAndroid.pipeKeyEvent[0], LOOPER_ID_KEY_EVENT, ALOOPER_EVENT_INPUT, NULL, NULL);

    // setup looper for touch events
    if (pipe(sstAndroid.pipeTouchEvent)) {
        LOGE("could not create pipe: %s", strerror(errno));
        return;
    }

    ALooper_addFd(sstAndroid.looper, sstAndroid.pipeTouchEvent[0], LOOPER_ID_TOUCH_EVENT, ALOOPER_EVENT_INPUT, NULL, NULL);

    __android_log_print(ANDROID_LOG_INFO, "Orx", "orxAndroid_Init() finished!");
}

static void orxAndroid_Exit(JNIEnv* env)
{
  env->DeleteGlobalRef(sstAndroid.mActivity);
  env->DeleteGlobalRef(sstAndroid.jAssetManager);
  free(sstAndroid.s_AndroidInternalFilesPath);

  // TODO close pipes fds
}

/* Main function to call */
extern int main(int argc, char *argv[]);

// Start up the Orx app
extern "C" void Java_org_orx_lib_OrxActivity_nativeInit(JNIEnv* env, jobject thiz)
{
    /* This interface could expand with ABI negotiation, calbacks, etc. */
    orxAndroid_Init(env, thiz);

    /* Run the application code! */
    int status;
    status = main(0, orxNULL);

    orxAndroid_Exit(env);
    /* Do not issue an exit or the whole application will terminate instead of just the Orx thread */
    //exit(status);
}

// Resize
extern "C" void Java_org_orx_lib_OrxActivity_onNativeResize(
                                    JNIEnv* env, jobject thiz,
                                    jint width, jint height)
{
// TODO send event here and move s_winWidth and s_winHeight to Display plugin
  s_winWidth = width;
  s_winHeight = height;
}

// Keydown
extern "C" void Java_org_orx_lib_OrxActivity_onNativeKeyDown(JNIEnv* env, jobject thiz, jint keycode)
{
  orxANDROID_KEY_EVENT stKeyEvent;

  stKeyEvent.u32Action = 0;
  stKeyEvent.u32KeyCode = keycode;
  if (write(sstAndroid.pipeKeyEvent[1], &stKeyEvent, sizeof(stKeyEvent)) != sizeof(stKeyEvent))
  {
    LOGE("Failure writing keycode: %s\n", strerror(errno));
  }
}

// Keyup
extern "C" void Java_org_orx_lib_OrxActivity_onNativeKeyUp(JNIEnv* env, jobject thiz, jint keycode)
{
  orxANDROID_KEY_EVENT stKeyEvent;

  stKeyEvent.u32Action = 1;
  stKeyEvent.u32KeyCode = keycode;
  if (write(sstAndroid.pipeKeyEvent[1], &stKeyEvent, sizeof(stKeyEvent)) != sizeof(stKeyEvent))
  {
    LOGE("Failure writing keycode: %s\n", strerror(errno));
  }
}

// Touch
extern "C" void Java_org_orx_lib_OrxActivity_onNativeTouch(
                                    JNIEnv* env, jobject thiz,
                                    jint touch_device_id_in, jint pointer_finger_id_in,
                                    jint action, jfloat x, jfloat y, jfloat p)
{
  orxANDROID_TOUCH_EVENT stTouchEvent;

  stTouchEvent.u32ID = pointer_finger_id_in;
  stTouchEvent.u32Action = action;
  stTouchEvent.fX = x;
  stTouchEvent.fY = y;

  if (write(sstAndroid.pipeTouchEvent[1], &stTouchEvent, sizeof(stTouchEvent)) != sizeof(stTouchEvent))
  {
    LOGE("Failure writing touch event: %s\n", strerror(errno));
  }
}

// Quit
extern "C" void Java_org_orx_lib_OrxActivity_nativeQuit(JNIEnv* env, jobject thiz)
{    
  app_write_cmd(APP_CMD_QUIT);
}

// Pause
extern "C" void Java_org_orx_lib_OrxActivity_nativePause(JNIEnv* env, jobject thiz)
{
  app_write_cmd(APP_CMD_PAUSE);
}

// Resume
extern "C" void Java_org_orx_lib_OrxActivity_nativeResume(JNIEnv* env, jobject thiz)
{
  app_write_cmd(APP_CMD_RESUME);
}

// SurfaceDestroyed
extern "C" void Java_org_orx_lib_OrxActivity_nativeSurfaceDestroyed(JNIEnv* env, jobject thiz)
{
  app_write_cmd(APP_CMD_SURFACE_DESTROYED);
}

// SurfaceCreated
extern "C" void Java_org_orx_lib_OrxActivity_nativeSurfaceCreated(JNIEnv* env, jobject thiz)
{
  app_write_cmd(APP_CMD_SURFACE_CREATED);
}

class LocalReferenceHolder
{
private:
    static int s_active;

public:
    static bool IsActive() {
        return s_active > 0;
    }

public:
    LocalReferenceHolder(const char *func) : m_env(NULL), m_func(func) {
#ifdef DEBUG_JNI
        LOGI("Entering function %s", m_func);
#endif
    }
    ~LocalReferenceHolder() {
#ifdef DEBUG_JNI
        LOGI("Leaving function %s", m_func);
#endif
        if (m_env) {
            m_env->PopLocalFrame(NULL);
            --s_active;
        }
    }

    bool init(JNIEnv *env, jint capacity = 16) {
        if (env->PushLocalFrame(capacity) < 0) {
            LOGE("Failed to allocate enough JVM local references");
            return false;
        }
        ++s_active;
        m_env = env;
        return true;
    }

protected:
    JNIEnv *m_env;
    const char *m_func;
};
int LocalReferenceHolder::s_active;

extern "C" orxU32 orxAndroid_JNI_GetRotation()
{
    JNIEnv *env = Android_JNI_GetEnv();
    
    jint rotation = env->CallIntMethod(sstAndroid.mActivity, sstAndroid.midGetRotation);
    return rotation;
}

extern "C" orxBOOL orxAndroid_JNI_CreateContext(int majorVersion, int minorVersion,
                                int red, int green, int blue, int alpha,
                                int buffer, int depth, int stencil,
                                int buffers, int samples)
{
    JNIEnv *env = Android_JNI_GetEnv();

    jint attribs[] = {
        EGL_RED_SIZE, red,
        EGL_GREEN_SIZE, green,
        EGL_BLUE_SIZE, blue,
        EGL_ALPHA_SIZE, alpha,
        EGL_BUFFER_SIZE, buffer,
        EGL_DEPTH_SIZE, depth,
        EGL_STENCIL_SIZE, stencil,
        EGL_SAMPLE_BUFFERS, buffers,
        EGL_SAMPLES, samples,
        EGL_RENDERABLE_TYPE, (majorVersion == 1 ? EGL_OPENGL_ES_BIT : EGL_OPENGL_ES2_BIT),
        EGL_NONE
    };
    int len = 21;

    jintArray array;

    array = env->NewIntArray(len);
    env->SetIntArrayRegion(array, 0, len, attribs);

    jboolean success = env->CallBooleanMethod(sstAndroid.mActivity, sstAndroid.midCreateGLContext, majorVersion, minorVersion, array);

    env->DeleteLocalRef(array);

    return success ? orxTRUE : orxFALSE;
}

extern "C" void orxAndroid_JNI_SwapWindow()
{
    JNIEnv *mEnv = Android_JNI_GetEnv();
    mEnv->CallVoidMethod(sstAndroid.mActivity, sstAndroid.midFlipBuffers); 
}


extern "C" void *orxAndroid_GetJNIEnv()
{
    return Android_JNI_GetEnv();
}

extern "C" const char * orxAndroid_GetInternalStoragePath()
{
    if (!sstAndroid.s_AndroidInternalFilesPath) {
        LocalReferenceHolder refs(__FUNCTION__);
        jmethodID mid;
        jobject fileObject;
        jstring pathString;
        const char *path;

        JNIEnv *env = Android_JNI_GetEnv();
        if (!refs.init(env)) {
            return NULL;
        }

        // fileObj = context.getFilesDir();
        mid = env->GetMethodID(env->GetObjectClass(sstAndroid.mActivity),
                "getFilesDir", "()Ljava/io/File;");
        fileObject = env->CallObjectMethod(sstAndroid.mActivity, mid);
        if (!fileObject) {
            LOGE("Couldn't get internal directory");
            return NULL;
        }

        // path = fileObject.getAbsolutePath();
        mid = env->GetMethodID(env->GetObjectClass(fileObject),
                "getAbsolutePath", "()Ljava/lang/String;");
        pathString = (jstring)env->CallObjectMethod(fileObject, mid);

        path = env->GetStringUTFChars(pathString, NULL);
        sstAndroid.s_AndroidInternalFilesPath = strdup(path);
        env->ReleaseStringUTFChars(pathString, path);
    }
    return sstAndroid.s_AndroidInternalFilesPath;
}

static inline orxBOOL isInteractible()
{
  return (sstAndroid.bSurfaceReady == orxTRUE && sstAndroid.bPaused == orxFALSE);
}

extern "C" void orxAndroid_PumpEvents()
{
  int ident;
  int events;

  while ((ident=ALooper_pollAll(isInteractible() == orxTRUE ? 0 : -1, NULL, &events, NULL)) >= 0)
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
        sstAndroid.bSurfaceReady = orxFALSE;
        orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_FOCUS_LOST);
      }
      if(cmd == APP_CMD_SURFACE_CREATED) {
        LOGI("APP_CMD_SURFACE_CREATED");
        sstAndroid.bSurfaceReady = orxTRUE;
        orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_FOCUS_GAINED);
      }
      if(cmd == APP_CMD_QUIT) {
        LOGI("APP_CMD_QUIT");
        orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_CLOSE);
      }
    }

    if(ident == LOOPER_ID_SENSOR)
    {
      orxSYSTEM_EVENT_PAYLOAD stAccelPayload;
      ASensorEvent event;

      while (ASensorEventQueue_getEvents(sensorEventQueue, &event, 1) > 0)
      {
        orxMemory_Zero(&stAccelPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));

        stAccelPayload.stAccelerometer.dTime = orxFLOAT_0;
  
        /* Set acceleration vector */
        orxVector_Set(&stAccelPayload.stAccelerometer.vAcceleration,
            orx2F(event.acceleration.x),
            orx2F(event.acceleration.y),
            orx2F(event.acceleration.z));

        /* Sends event */
        orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_ACCELERATE, orxNULL, orxNULL, &stAccelPayload);
      }
    }

    if(ident == LOOPER_ID_KEY_EVENT)
    {
      orxANDROID_KEY_EVENT stKeyEvent;

      if (read(sstAndroid.pipeKeyEvent[0], &stKeyEvent, sizeof(stKeyEvent)) == sizeof(stKeyEvent))
      {
        orxEVENT stEvent;

        switch(stKeyEvent.u32Action)
        {
        case 0: // down
          orxEVENT_INIT(stEvent, (orxEVENT_TYPE)orxEVENT_TYPE_FIRST_RESERVED + orxANDROID_EVENT_KEYBOARD, orxANDROID_EVENT_KEYBOARD_DOWN, orxNULL, orxNULL, &stKeyEvent.u32KeyCode);
          break;
        case 1: // up
          orxEVENT_INIT(stEvent, (orxEVENT_TYPE)orxEVENT_TYPE_FIRST_RESERVED + orxANDROID_EVENT_KEYBOARD, orxANDROID_EVENT_KEYBOARD_UP, orxNULL, orxNULL, &stKeyEvent.u32KeyCode);
          break;
        }

        orxEvent_Send(&stEvent);
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

        /* Inits event's payload */
        orxMemory_Zero(&stPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
        stPayload.stTouch.fPressure = orxFLOAT_0;
        stPayload.stTouch.fX = stTouchEvent.fX;
        stPayload.stTouch.fY = stTouchEvent.fY;
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
  AAsset   *poAsset;
  orxHANDLE hResult;

  /* Not in erase mode? */
  if(_bEraseMode == orxFALSE)
  {
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

static orxS32 orxFASTCALL orxResource_APK_GetSize(orxHANDLE _hResource)
{
  AAsset   *poAsset;
  orxS32    s32Result;

  /* Gets asset */
  poAsset = (AAsset *)_hResource;

  /* Updates result */
  s32Result = AAsset_getLength(poAsset);

  /* Done! */
  return s32Result;
}

static orxS32 orxFASTCALL orxResource_APK_Seek(orxHANDLE _hResource, orxS32 _s32Offset, orxSEEK_OFFSET_WHENCE _eWhence)
{
  AAsset   *poAsset;
  orxS32    s32Result;

  /* Gets asset */
  poAsset = (AAsset *)_hResource;

  /* Updates result */
  s32Result = AAsset_seek(poAsset, _s32Offset, _eWhence);

  /* Done! */
  return s32Result;
}

static orxS32 orxFASTCALL orxResource_APK_Tell(orxHANDLE _hResource)
{
  AAsset   *poAsset;
  orxS32    s32Result;

  /* Gets asset */
  poAsset = (AAsset *)_hResource;

  /* Updates result */
  s32Result = AAsset_getLength(poAsset) - AAsset_getRemainingLength(poAsset);

  /* Done! */
  return s32Result;
}

static orxS32 orxFASTCALL orxResource_APK_Read(orxHANDLE _hResource, orxS32 _s32Size, void *_pBuffer)
{
  AAsset   *poAsset;
  orxS32    s32Result;

  /* Gets asset */
  poAsset = (AAsset *)_hResource;

  /* Updates result */
  s32Result = AAsset_read(poAsset, _pBuffer, sizeof(orxCHAR) * _s32Size) / sizeof(orxCHAR);

  /* Done! */
  return s32Result;
}

orxSTATUS orxAndroid_RegisterAPKResource()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;
  orxRESOURCE_TYPE_INFO stAPKTypeInfo;

  /* Inits apk type */
  stAPKTypeInfo.zTag       = (orxCHAR*) orxRESOURCE_KZ_TYPE_TAG_APK;
  stAPKTypeInfo.pfnLocate  = orxResource_APK_Locate;
  stAPKTypeInfo.pfnOpen    = orxResource_APK_Open;
  stAPKTypeInfo.pfnClose   = orxResource_APK_Close;
  stAPKTypeInfo.pfnGetSize = orxResource_APK_GetSize;
  stAPKTypeInfo.pfnSeek    = orxResource_APK_Seek;
  stAPKTypeInfo.pfnTell    = orxResource_APK_Tell;
  stAPKTypeInfo.pfnRead    = orxResource_APK_Read;
  stAPKTypeInfo.pfnWrite   = orxNULL;

  /* Registers it */
  eResult = orxResource_RegisterType(&stAPKTypeInfo);

  return eResult;
}

#endif
