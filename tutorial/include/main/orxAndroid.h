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
#include <android/log.h>

/* defined in orxModule.c */
extern orxMODULE_RUN_FUNCTION  spfnRun;
extern orxSYSTEM_EVENT_PAYLOAD sstPayload;

/* defined in orxDisplay.c */
extern int32_t s32DisplayWidth;
extern int32_t s32DisplayHeight;

/* defined in orxAndSupport.c */
extern jobject     oActivity;
extern JNIEnv     *poJEnv;
extern orxS32      s32NbParams;
extern orxSTRING  *azParams;

void orxAndroid_GetMainArgs();
void orxAndroid_ReleaseMainArgs();

/* Main function to call */
extern int main(int argc, char *argv[]);

static orxINLINE void orx_Init()
{
  /* retrieve orx cmd-line arguments */
  orxAndroid_GetMainArgs();

  /* Call main function */
  main(s32NbParams,azParams);
}

static orxINLINE orxBOOL orx_Step()
{
  orxSTATUS eClockStatus, eMainStatus;
  orxBOOL   bStop;
  
  /* Sends frame start event */
  orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_GAME_LOOP_START, orxNULL, orxNULL, &sstPayload);

  /* Runs the engine */
  eMainStatus = spfnRun();

  /* Updates clock system */
  eClockStatus = orxClock_Update();

  /* Sends frame stop event */
  orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_GAME_LOOP_STOP, orxNULL, orxNULL, &sstPayload);

  /* Updates frame counter */
  sstPayload.u32FrameCounter++;

  /* Updates stop condition */
  bStop = ((sbStopByEvent != orxFALSE) || (eMainStatus == orxSTATUS_FAILURE) || (eClockStatus == orxSTATUS_FAILURE)) ? orxTRUE : orxFALSE;

  /* Done! */
  return bStop;
}

static orxINLINE void orx_Exit()
{
  /* Removes event handler */
  orxEvent_RemoveHandler(orxEVENT_TYPE_SYSTEM, orx_DefaultEventHandler);

  /* Exits from engine */
  orxModule_Exit(orxMODULE_ID_MAIN);

  /* Exits from all modules */
  orxModule_ExitAll();

  /* Exits from the Debug system */
  orxDEBUG_EXIT();
  
  /* free cmd-line arguments */
  orxAndroid_ReleaseMainArgs();
}

  #ifdef __cplusplus
  extern "C" {
  #endif

    JNIEXPORT void JNICALL Java_org_orx_android_OrxLib_init(JNIEnv * env, jobject obj, jobject activity, jint width, jint height);
    JNIEXPORT jboolean JNICALL Java_org_orx_android_OrxLib_step(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_org_orx_android_OrxLib_exit(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_org_orx_android_OrxLib_send_1orxSYSTEM_1EVENT_1CLOSE(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_org_orx_android_OrxLib_send_1orxDISPLAY_1EVENT_1SAVE_1CONTEXT(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_org_orx_android_OrxLib_send_1orxDISPLAY_1EVENT_1RESTORE_1CONTEXT(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL Java_org_orx_android_OrxLib_onNativeTouch(JNIEnv* env, jobject obj, jint iAction, jint iActionPointer, jint iPointerCount, jintArray aiIdList,  jfloatArray afXList, jfloatArray afYList, jfloatArray afPressureList);
    JNIEXPORT void JNICALL Java_org_orx_android_OrxLib_onNativeAccel(JNIEnv* env, jobject obj, jfloat x, jfloat y, jfloat z);
    JNIEXPORT void JNICALL Java_org_orx_android_OrxLib_send_1orxSYSTEM_1EVENT_1BACKGROUND(JNIEnv* env, jobject obj);
    JNIEXPORT void JNICALL Java_org_orx_android_OrxLib_send_1orxSYSTEM_1EVENT_1FOREGROUND(JNIEnv* env, jobject obj);
    
  #ifdef __cplusplus
  };
  #endif

JNIEXPORT void JNICALL Java_org_orx_android_OrxLib_init(JNIEnv * env, jobject obj, jobject activity, jint s32width, jint s32height)
{
	s32DisplayWidth = s32width;
	s32DisplayHeight = s32height;
	
	/* save env and activity */
	poJEnv = env;
  #ifdef __cplusplus
    oActivity = env->NewGlobalRef(activity);
  #else /* __cplusplus */
	  oActivity = (*env)->NewGlobalRef(env, activity);
  #endif /* __cplusplus */
	
	orx_Init();
}

JNIEXPORT jboolean JNICALL Java_org_orx_android_OrxLib_step(JNIEnv * env, jobject obj)
{
	return (orx_Step() == orxTRUE ? JNI_FALSE : JNI_TRUE);
}

JNIEXPORT void JNICALL Java_org_orx_android_OrxLib_exit(JNIEnv * env, jobject obj)
{
  
	orx_Exit();
	
	/* release oActivity reference */
#ifdef __cplusplus
  env->DeleteGlobalRef(oActivity);
#else
	(*env)->DeleteGlobalRef(env, oActivity);
#endif
}

JNIEXPORT void JNICALL Java_org_orx_android_OrxLib_send_1orxSYSTEM_1EVENT_1CLOSE(JNIEnv * env, jobject obj)
{
  orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_CLOSE);
}

JNIEXPORT void JNICALL Java_org_orx_android_OrxLib_send_1orxDISPLAY_1EVENT_1SAVE_1CONTEXT(JNIEnv * env, jobject obj)
{
  orxEvent_SendShort(orxEVENT_TYPE_DISPLAY, orxDISPLAY_EVENT_SAVE_CONTEXT);
}

JNIEXPORT void JNICALL Java_org_orx_android_OrxLib_send_1orxDISPLAY_1EVENT_1RESTORE_1CONTEXT(JNIEnv * env, jobject obj)
{
  orxEvent_SendShort(orxEVENT_TYPE_DISPLAY, orxDISPLAY_EVENT_RESTORE_CONTEXT);
}

JNIEXPORT void JNICALL Java_org_orx_android_OrxLib_onNativeTouch(JNIEnv* env, jobject obj, jint iAction, jint iActionPointer, jint iPointerCount, jintArray aiIdList,  jfloatArray afXList, jfloatArray afYList, jfloatArray afPressureList)
{
	orxSYSTEM_EVENT_PAYLOAD stPayload;

	/* Inits event's type */
	orxSYSTEM_EVENT android_event;
	switch (iAction) {
	case 0:
		android_event = orxSYSTEM_EVENT_TOUCH_BEGIN;
		break;
	case 1:
		android_event = orxSYSTEM_EVENT_TOUCH_MOVE;
		break;
	case 2:
		android_event = orxSYSTEM_EVENT_TOUCH_END;
		break;
	default:
		//error.....
		return;
	}

	/* Inits event's payload */
	stPayload.stTouch.iPointerCount = iPointerCount;
	stPayload.stTouch.iActionPointer= iActionPointer;

	/* Get array from Java environment */
#ifdef __cplusplus
	stPayload.stTouch.aiIdList = env->GetIntArrayElements(aiIdList, NULL);
	stPayload.stTouch.afXList = env->GetFloatArrayElements(afXList, NULL);
	stPayload.stTouch.afYList = env->GetFloatArrayElements(afYList, NULL);
	stPayload.stTouch.afPressureList = env->GetFloatArrayElements(afPressureList, NULL);
#else /* __cplusplus */
	stPayload.stTouch.aiIdList = (*env)->GetIntArrayElements(env, aiIdList, NULL);
	stPayload.stTouch.afXList = (*env)->GetFloatArrayElements(env, afXList, NULL);
	stPayload.stTouch.afYList = (*env)->GetFloatArrayElements(env, afYList, NULL);
	stPayload.stTouch.afPressureList = (*env)->GetFloatArrayElements(env, afPressureList, NULL);
#endif /* __cplusplus */

	/* Sends it */
	orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, android_event, orxNULL, orxNULL, &stPayload);

	/* Clear java arrays */
#ifdef __cplusplus
	env->ReleaseIntArrayElements(aiIdList,stPayload.stTouch.aiIdList, 0);
	env->ReleaseFloatArrayElements(afXList, stPayload.stTouch.afXList, 0);
	env->ReleaseFloatArrayElements(afYList, stPayload.stTouch.afYList, 0);
	env->ReleaseFloatArrayElements(afPressureList, stPayload.stTouch.afPressureList, 0);
#else /* __cplusplus */
	(*env)->ReleaseIntArrayElements(env, aiIdList,stPayload.stTouch.aiIdList, 0);   
	(*env)->ReleaseFloatArrayElements(env, afXList, stPayload.stTouch.afXList, 0);   
	(*env)->ReleaseFloatArrayElements(env, afYList, stPayload.stTouch.afYList, 0);   
	(*env)->ReleaseFloatArrayElements(env, afPressureList, stPayload.stTouch.afPressureList, 0);
#endif /* __cplusplus */
}

JNIEXPORT void JNICALL Java_org_orx_android_OrxLib_onNativeAccel(JNIEnv* env, jobject obj, jfloat x, jfloat y, jfloat z)
{
  orxSYSTEM_EVENT_PAYLOAD stPayload;

	/* Inits event's payload */
	orxMemory_Zero(&stPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
	stPayload.stAccelerometer.pAccelerometer = orxNULL;
	stPayload.stAccelerometer.fX = (orxFLOAT) x;
	stPayload.stAccelerometer.fY = (orxFLOAT) y;
	stPayload.stAccelerometer.fZ = (orxFLOAT) z;

	/* Sends it */
	orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_ACCELERATE,	orxNULL, orxNULL, &stPayload);
}

JNIEXPORT void JNICALL Java_org_orx_android_OrxLib_send_1orxSYSTEM_1EVENT_1BACKGROUND(JNIEnv * env, jobject obj)
{
  orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_BACKGROUND);
}

JNIEXPORT void JNICALL Java_org_orx_android_OrxLib_send_1orxSYSTEM_1EVENT_1FOREGROUND(JNIEnv * env, jobject obj)
{
  orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_FOREGROUND);
}

#elif defined (__orxANDROID_NATIVE__)

  #include <android/log.h>
  #include <android_native_app_glue.h>
  #include <android/sensor.h>


/* Defined in orxAndroidNativeSupport.c */
extern orxS32 s32Animating;
extern struct android_app *pstApp;
extern void (*ptonAppCmd)(struct android_app *app, int32_t cmd);
extern int32_t (*ptonInputEvent)(struct android_app* app, AInputEvent* event);
extern const ASensor *poAccelerometerSensor;
extern ASensorEventQueue *poSensorEventQueue;
void orxAndroid_AttachThread();
void orxAndroid_DetachThread();
void orxAndroid_GetMainArgs();
void orxAndroid_ReleaseMainArgs();

extern orxS32     s32NbParams;
extern orxSTRING *azParams;


static orxINLINE void orx_AndroidExecute(struct android_app *_pstApp, const orxMODULE_INIT_FUNCTION _pfnInit, const orxMODULE_RUN_FUNCTION _pfnRun, const orxMODULE_EXIT_FUNCTION _pfnExit)
{
  /* Checks */
  orxASSERT(_pstApp != orxNULL);
  orxASSERT(_pfnRun != orxNULL);

  /* Inits app */
  pstApp                = _pstApp;
  pstApp->onAppCmd      = ptonAppCmd;
  pstApp->onInputEvent  = ptonInputEvent;
  
  /* Makes sure glue isn't stripped */
  app_dummy();

  /* Inits the Debug System */
  orxDEBUG_INIT();

  /* Retrieves Java environment */
  orxAndroid_AttachThread();
  orxAndroid_GetMainArgs();

  /* Registers main module */
  orxModule_Register(orxMODULE_ID_MAIN, orx_MainSetup, _pfnInit, _pfnExit);

  /* Registers all other modules */
  orxModule_RegisterAll();

  /* Calls all modules setup */
  orxModule_SetupAll();

  /* Sends the command line arguments to orxParam module */
  if(orxParam_SetArgs(s32NbParams, azParams) != orxSTATUS_FAILURE)
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
          while((s32Ident = ALooper_pollAll(((s32Animating != 0) || (pstApp->destroyRequested != 0)) ? 0 : -1, NULL, (int *)&s32Events, (void **)&pstSource)) >= 0)
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
                  stAccelPayload.stAccelerometer.pAccelerometer = &oEvent;
                  stAccelPayload.stAccelerometer.fX = (orxFLOAT)oEvent.acceleration.x;
                  stAccelPayload.stAccelerometer.fY = (orxFLOAT)oEvent.acceleration.y;
                  stAccelPayload.stAccelerometer.fZ = (orxFLOAT)oEvent.acceleration.z;

                  /* Sends event */
                  orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_ACCELERATE, orxNULL, orxNULL, &stAccelPayload);
                }
              }
            }
          }

          /* Should update? */
          if((s32Animating != 0) || (pstApp->destroyRequested != 0))
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

  orxAndroid_ReleaseMainArgs();
  orxAndroid_DetachThread();
}

#endif

#endif
/** @} */
