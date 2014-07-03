#if defined(__orxANDROID_NATIVE__)

#include <android/log.h>
#include <android_native_app_glue.h>

#ifdef __orxDEBUG__

#define MODULE "orxAndroidNativeSupport"
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

struct android_app;

/** Static structure
 */
typedef struct __orxANDROID_STATIC_t {
        char *s_AndroidInternalFilesPath;
        orxBOOL bPaused;
        android_app* app_;
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

void Android_JNI_SetupThread(void) {
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
}

extern "C" ANativeWindow* orxAndroid_GetNativeWindow()
{
    int ident;
    int events;
    android_poll_source* source;

    LOGI("orxAndroid_GetNativeWindow()");

    while(sstAndroid.app_->window == orxNULL)
    {
        LOGI("no window received yet");

        ident = ALooper_pollAll(-1, NULL, &events, (void**) &source );
        // Process this event.
        if( source != NULL )
            source->process( sstAndroid.app_, source );
    }

    return sstAndroid.app_->window;
}

extern "C" void orxAndroid_JNI_SetWindowFormat(orxU32 format)
{
    // NOOP on NativeActivity
}

extern "C" void *orxAndroid_GetJNIEnv()
{
  return Android_JNI_GetEnv();
}

extern "C" jobject orxAndroid_GetActivity()
{
  return sstAndroid.app_->activity->clazz;
}

static int32_t handleInput(struct android_app* app, AInputEvent* event)
{

    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION )
    {
        orxSYSTEM_EVENT_PAYLOAD stPayload;

        /* Inits event's payload */
        orxMemory_Zero(&stPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
        stPayload.stTouch.fPressure = orxFLOAT_0;
        stPayload.stTouch.fX = AMotionEvent_getX(event, 0);
        stPayload.stTouch.fY = AMotionEvent_getY(event, 0);

        int32_t action = AMotionEvent_getAction(event);
        unsigned int flags = action & AMOTION_EVENT_ACTION_MASK;
        switch( flags )
        {
            case AMOTION_EVENT_ACTION_DOWN:
                stPayload.stTouch.u32ID = AMotionEvent_getPointerId(event, 0);
                orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_BEGIN, orxNULL, orxNULL, &stPayload);
                break;
            case AMOTION_EVENT_ACTION_UP:
                stPayload.stTouch.u32ID = AMotionEvent_getPointerId(event, 0);
                orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_END, orxNULL, orxNULL, &stPayload);
                break;
            case AMOTION_EVENT_ACTION_MOVE:
            {
                int32_t count = AMotionEvent_getPointerCount(event);
                stPayload.stTouch.u32ID = AMotionEvent_getPointerId(event, 0);
                orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_MOVE, orxNULL, orxNULL, &stPayload);

                for(int i = 1; i < count; i++)
                {
                  stPayload.stTouch.fX = AMotionEvent_getX(event, i);
                  stPayload.stTouch.fY = AMotionEvent_getY(event, i);
                  stPayload.stTouch.u32ID = AMotionEvent_getPointerId(event, i);
                  orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_MOVE, orxNULL, orxNULL, &stPayload);
                }
                break;
            }
            case AMOTION_EVENT_ACTION_POINTER_DOWN:
            {
                int32_t iIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)
                    >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
                stPayload.stTouch.u32ID = AMotionEvent_getPointerId(event, iIndex);
                orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_BEGIN, orxNULL, orxNULL, &stPayload);
                break;
            }
            case AMOTION_EVENT_ACTION_POINTER_UP:
            {
                int32_t iIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)
                    >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
                stPayload.stTouch.u32ID = AMotionEvent_getPointerId(event, iIndex);
                orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_END, orxNULL, orxNULL, &stPayload);
                break;
            }
        }

        return 1;
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

        orxEVENT_SEND(orxANDROID_EVENT_TYPE_SURFACE, orxANDROID_EVENT_SURFACE_DESTROYED, orxNULL, orxNULL, orxNULL);
        break;
    case APP_CMD_WINDOW_RESIZED:
        LOGI("APP_CMD_WINDOW_RESIZED");

        orxANDROID_SURFACE_CHANGED_EVENT stSurfaceChangedEvent;

        // TODO
        stSurfaceChangedEvent.u32Width = 0;
        stSurfaceChangedEvent.u32Height = 0;

        orxEVENT_SEND(orxANDROID_EVENT_TYPE_SURFACE, orxANDROID_EVENT_SURFACE_CHANGED, orxNULL, orxNULL, &stSurfaceChangedEvent);
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

        orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_FOCUS_GAINED);
        break;
    case APP_CMD_LOST_FOCUS:
        LOGI("APP_CMD_LOST_FOCUS");

        orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_FOCUS_LOST);
        break;
    default:
        break;
    }
}

static inline orxBOOL isInteractible()
{
  return (sstAndroid.app_->window != orxNULL && sstAndroid.bPaused != orxTRUE);
}


extern "C" void orxAndroid_PumpEvents()
{
    // Read all pending events.
    int id;
    int events;
    android_poll_source* source;

    // If not animating, we will block forever waiting for events.
    // If animating, we loop until all events are read, then continue
    // to draw the next frame of animation.
    while( (id = ALooper_pollAll( isInteractible() ? 0 : -1, NULL, &events, (void**) &source )) >= 0 )
    {
        // Process this event.
        if( source != NULL )
            source->process( sstAndroid.app_, source );

        // Check if we are exiting.
        if( sstAndroid.app_->destroyRequested != 0 )
        {
            return;
        }
    }
}

/* Main function to call */
extern int main(int argc, char *argv[]);

void android_main( android_app* state )
{
    app_dummy();

    state->onAppCmd = handleCmd;
    state->onInputEvent = handleInput;

    /* Cleans static controller */
    memset(&sstAndroid, 0, sizeof(orxANDROID_STATIC));
    sstAndroid.app_ = state;
    jVM = state->activity->vm;

    /*
     * Create mThreadKey so we can keep track of the JNIEnv assigned to each thread
     * Refer to http://developer.android.com/guide/practices/design/jni.html for the rationale behind this
     */
    if (pthread_key_create(&sThreadKey, Android_JNI_ThreadDestroyed)) {
        __android_log_print(ANDROID_LOG_ERROR, "Orx", "Error initializing pthread key");
    }
    else {
        Android_JNI_SetupThread();
    }

    /* Run the application code! */
    int status;
    status = main(0, orxNULL);
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

  /* Registers it */
  eResult = orxResource_RegisterType(&stAPKTypeInfo);

  return eResult;
}
#endif
