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
 * @file orxAndroid.h
 * @date 26/06/2011
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxAndroid
 * 
 * Android support module
 *
 * @{
 */


#ifndef _orxANDROID_H_
#define _orxANDROID_H_

#if defined(__orxANDROID__)

#include <jni.h>
#include <pthread.h>
#include <stdlib.h>

#if defined(__cplusplus)
extern "C"
{
#endif

/** @file thread.h
  The Thread library makes it easy to create native threads that can acess
  JNI objects.  By default, pthreads created in the Android NDK are NOT connected
  to the JVM and JNI calls will fail.  This library wraps thread creation in
  such a way that pthreads created using it will connect to and disconnect from
  the JVM as appropriate.  Applications creating all of their threads with these
  interfaces can use the provided ThreadGetCurrentJNIEnv() function to
  get the current thread's JNI context at any time.

  Note that native-created threads still have JNI limitations over threads
  that are calls down to native from Java.  The JNI function FindClass will
  NOT find application-specific classes when called from native threads.
  Native code that needs to call FindClass and record the indices of Java
  class members for later access must call FindClass and Get*FieldID/Get*MethodID
  in threads calling from Java, such as JNI_OnLoad
 */

/**
  Initializes the thread system by connecting it to the JVM.  This
  function must be called as early as possible in the native code's
  JNI_OnLoad function, so that the thread system is prepared for any
  JNI-dependent library initialization calls.  
  @param vm The VM pointer - should be the JavaVM pointer sent to JNI_OnLoad.
  */
void orxAndroid_ThreadInit(JavaVM* vm);

/**
  Retrieves the JNIEnv object associated with the current thread, allowing
  any thread that was creating with ThreadSpawnJNIThread() to access the
  JNI at will.  This JNIEnv is NOT usable across multiple calls or threads
  The function should be called in each function that requires a JNIEnv
  @return The current thread's JNIEnv, or NULL if the thread was not created
  by ThreadSpawnJNIThread
  @see ThreadSpawnJNIThread
  */
JNIEnv* orxAndroid_ThreadGetCurrentJNIEnv();

/**
  Spwans a new native thread that is registered for use with JNI.  Threads
  created with this function will have access to JNI data via the JNIEnv
  available from ThreadGetCurrentJNIEnv().
  @param thread is the same as in pthread_create
  @param attr is the same as in pthread_create
  @param start_routine is the same as in pthread_create
  @param arg is the same as in pthread_create
  @return 0 on success, -1 on failure
  @see ThreadGetCurrentJNIEnv
*/
int orxAndroid_ThreadSpawnJNIThread(pthread_t *thread, pthread_attr_t const * attr,
    void *(*start_routine)(void *), void * arg);

/**
  Sleeps the current thread for the specified number of milliseconds
  @param millisec Sleep time in ms
  @return 0 on success, -1 on failure
*/
int orxAndroid_ThreadSleep(unsigned long millisec);

#if defined(__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C"
{
#endif

/** @file apk_file.h
  <b>APK is now considered internal - File in file.h should be used
  by applications.</b>

  This library supports FILE*-like access to assets in the APK file.  These
  APIs should not be used directly.  Use the File APIs in file.h, which
  include all of this library's functionalities and support files from APK
  and from /data.
  @see file.h
*/

typedef void APKFile;



/**
  Initializes the library.  This function MUST be called from the application's
  JNI_OnLoad, from a function known to be called by JNI_OnLoad, or from a function
  in a Java-called thread.  thread-created native threads cannot call this
  initialization function.
  */
void        orxAndroid_APKInit();

APKFile*    orxAndroid_APKOpen(char const* path);
void        orxAndroid_APKClose(APKFile* file);
int         orxAndroid_APKGetc(APKFile *stream);
char*       orxAndroid_APKGets(char* s, int size, APKFile* stream);
size_t      orxAndroid_APKSize(APKFile* stream);
long        orxAndroid_APKSeek(APKFile* stream, long offset, int type);
long        orxAndroid_APKTell(APKFile* stream);
size_t      orxAndroid_APKRead(void* ptr, size_t size, size_t nmemb, APKFile* stream);
int         orxAndroid_APKEOF(APKFile *stream);

#if defined(__cplusplus)
}
#endif

#define orxANDROID_EVENT_KEYBOARD       0
#define orxANDROID_EVENT_KEYBOARD_DOWN  0
#define orxANDROID_EVENT_KEYBOARD_UP    1
#define KEYCODE_BACK 0x04
#define KEYCODE_MENU 0x52


#elif defined (__orxANDROID_NATIVE__)

  #include <android/log.h>
  #include <nv_native_app_glue.h>
  #include <android/sensor.h>

#define  LOG_TAG    "orxAndroid.h"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

/* Defined in orxAndroidNativeSupport.c */
extern struct android_app *pstApp;
extern orxS32                     u32NbParams;
extern orxSTRING                 *azParams;
extern const ASensor             *poAccelerometerSensor;
extern ASensorEventQueue         *poSensorEventQueue;

static orxINLINE void orx_Execute(orxU32 _u32NbParams, orxSTRING _azParams[], const orxMODULE_INIT_FUNCTION _pfnInit, const orxMODULE_RUN_FUNCTION _pfnRun, const orxMODULE_EXIT_FUNCTION _pfnExit)
{
  /* Inits the Debug System */
  orxDEBUG_INIT();

  /* Checks */
  orxASSERT(_pfnRun != orxNULL);

  /* Registers main module */
  orxModule_Register(orxMODULE_ID_MAIN, orx_MainSetup, _pfnInit, _pfnExit);

  /* Registers all other modules */
  orxModule_RegisterAll();

  /* Calls all modules setup */
  orxModule_SetupAll();

  /* Sends the command line arguments to orxParam module */
  if(orxParam_SetArgs(_u32NbParams, _azParams) != orxSTATUS_FAILURE)
  {
    /* Inits the engine */
    if(orxModule_Init(orxMODULE_ID_MAIN) != orxSTATUS_FAILURE)
    {
      /* Registers default event handler */
      orxEvent_AddHandler(orxEVENT_TYPE_SYSTEM, orx_DefaultEventHandler);

      /* Displays help */
      if(orxParam_DisplayHelp() != orxSTATUS_FAILURE)
      {
        orxSTATUS eClockStatus, eMainStatus;
        orxSYSTEM_EVENT_PAYLOAD stPayload;
        orxBOOL   bStop;

        /* Clears payload */
        orxMemory_Zero(&stPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));

        /* Main loop */
        for(bStop = orxFALSE, sbStopByEvent = orxFALSE;
            bStop == orxFALSE;
            bStop = ((sbStopByEvent != orxFALSE) || (eMainStatus == orxSTATUS_FAILURE) || (eClockStatus == orxSTATUS_FAILURE)) ? orxTRUE : orxFALSE)
        {
          /* Reads all pending events */
          orxS32 s32Ident, s32Events;
          struct android_poll_source *pstSource;

          /* For all system events */
          while((s32Ident = ALooper_pollAll((nv_app_status_interactable(pstApp) || sbStopByEvent != orxFALSE) ? 0 : -1, NULL, (int *)&s32Events, (void **)&pstSource)) >= 0)
          {
             /* Valid source? */
             if(pstSource != NULL)
             {
               /* Process its event */
               pstSource->process(pstApp, pstSource);
             }

            /* If a sensor has data, process it now */
            if(s32Ident == LOOPER_ID_USER)
            {
              /* Has accelerometer? */
              if(poAccelerometerSensor != NULL)
              {
              	orxSYSTEM_EVENT_PAYLOAD stAccelPayload;
                ASensorEvent            oEvent;

                /* Inits event's payload */
                orxMemory_Zero(&stAccelPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));

                /* For all accelerometer events */
                while(ASensorEventQueue_getEvents(poSensorEventQueue, &oEvent, 1) > 0)
                {
                  /* Inits event */
                  stAccelPayload.stAccelerometer.vAcceleration.fX = (orxFLOAT)oEvent.acceleration.x;
                  stAccelPayload.stAccelerometer.vAcceleration.fY = (orxFLOAT)oEvent.acceleration.y;
                  stAccelPayload.stAccelerometer.vAcceleration.fZ = (orxFLOAT)oEvent.acceleration.z;

                  /* Sends event */
                  orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_ACCELERATE, orxNULL, orxNULL, &stAccelPayload);
                }
              }
            }
          }
          /* Should update? */
          if(nv_app_status_interactable(pstApp))
          {
            /* Sends frame start event */
            orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_GAME_LOOP_START, orxNULL, orxNULL, &stPayload);

            /* Runs the engine */
            eMainStatus = _pfnRun();

            /* Updates clock system */
            eClockStatus = orxClock_Update();

            /* Sends frame stop event */
            orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_GAME_LOOP_STOP, orxNULL, orxNULL, &stPayload);

            /* Updates frame counter */
            stPayload.u32FrameCounter++;
          }
        }
      }
      /* Removes event handler */
      orxEvent_RemoveHandler(orxEVENT_TYPE_SYSTEM, orx_DefaultEventHandler);

      /* Exits from engine */
      orxModule_Exit(orxMODULE_ID_MAIN);
    }

    /* Exits from all modules */
    orxModule_ExitAll();
  }
  
  /* Exits from the Debug system */
  orxDEBUG_EXIT();
}

#endif

#endif
/** @} */
