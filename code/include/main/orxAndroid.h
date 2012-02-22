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

extern orxMODULE_RUN_FUNCTION  pfnRun;

void MainLoop();

static orxINLINE void orx_Execute(orxU32 _u32NbParams, orxSTRING _azParams[], const orxMODULE_INIT_FUNCTION _pfnInit, const orxMODULE_RUN_FUNCTION _pfnRun, const orxMODULE_EXIT_FUNCTION _pfnExit)
{
  /* Inits the Debug System */
  orxDEBUG_INIT();

  /* Checks */
//  orxASSERT(_u32NbParams > 0);
//  orxASSERT(_azParams != orxNULL);
  orxASSERT(_pfnRun != orxNULL);

  /* register run function */
  pfnRun = _pfnRun;

  /* Registers main module */
  orxModule_Register(orxMODULE_ID_MAIN, orx_MainSetup, _pfnInit, _pfnExit);

  /* Registers all other modules */
  orxModule_RegisterAll();

  /* Calls all modules setup */
  orxModule_SetupAll();

  /* Sends the command line arguments to orxParam module */
  if(orxParam_SetArgs(_u32NbParams, _azParams) != orxSTATUS_FAILURE)
  {
    MainLoop();
  }

  /* Exits from the Debug system */

  orxDEBUG_EXIT();
}

#elif defined (__orxANDROID_NATIVE__)

  #include <android/log.h>
  #include <android_native_app_glue.h>
  #include <android/sensor.h>

#define  LOG_TAG    "orxAndroid.h"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

orxS32              s32Animating;
const ASensor       *poAccelerometerSensor;
ASensorEventQueue   *poSensorEventQueue;

static struct android_app *pstApp;

/* Defined in orxAndroidNativeSupport.c */
extern orxS32     u32NbParams;
extern orxSTRING *azParams;

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
}

/** Render inhibiter
 */
static orxSTATUS orxFASTCALL RenderInhibiter(const orxEVENT *_pstEvent)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

static void engine_handle_cmd(struct android_app* app, int32_t cmd)
{
  /* struct engine* engine = (struct engine*)app->userData; */
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
    /* struct engine* engine = (struct engine*)app->userData; */
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

void orxAndroid_EnableAccelerometer(orxFLOAT _fFrequency)
{
  ASensorManager *poSensorManager = ASensorManager_getInstance();
  poAccelerometerSensor = ASensorManager_getDefaultSensor(poSensorManager, ASENSOR_TYPE_ACCELEROMETER);
  poSensorEventQueue = ASensorManager_createEventQueue(poSensorManager, pstApp->looper, LOOPER_ID_USER, NULL, NULL);

  /* enable accelerometer */
  ASensorEventQueue_enableSensor(poSensorEventQueue, poAccelerometerSensor);
  ASensorEventQueue_setEventRate(poSensorEventQueue, poAccelerometerSensor, (1000L/_fFrequency)*1000);
}

void orxAndroid_WaitForWindow()
{
    // waiting for window to be ready
    // Read all pending events.
    int ident;
    int events;
    struct android_poll_source* source;

    while ((ident=ALooper_pollAll(pstApp->window != NULL ? 0 : -1, NULL, &events, (void**)&source)) >= 0)
    {
      // Process this event.
      if (source != NULL)
      {
        source->process(pstApp, source);
      }

      if (ident == LOOPER_ID_USER)
      // consume sensorevents
      {
        if (poAccelerometerSensor != NULL)
        {
          ASensorEvent event;
          while (ASensorEventQueue_getEvents(poSensorEventQueue, &event, 1) > 0);
        }
      }
    }
}

ANativeActivity* orxAndroid_GetNativeActivity()
{
  return pstApp->activity;
}

ANativeWindow* orxAndroid_GetNativeWindow()
{
  return pstApp->window;
}

int main(int argc, char *argv[]);
void orxAndroid_SetJavaVM(JavaVM* vm);
void orxAndroid_GetMainArgs();
void orxAndroid_ReleaseMainArgs();
void orxAndroid_DetachThread();

void android_main(struct android_app *_pstApp)
{
  /* Makes sure glue isn't stripped */
  app_dummy();

  /* Inits app */
  pstApp                = _pstApp;
  pstApp->onAppCmd      = engine_handle_cmd;
  pstApp->onInputEvent  = engine_handle_input;

  /* Retrieves Java environment */
  orxAndroid_SetJavaVM(_pstApp->activity->vm);
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


#endif

#endif
/** @} */
