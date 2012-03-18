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

/* Defined in orxAndroidNativeSupport.c */
extern struct android_app *pstApp;
extern orxS32                     u32NbParams;
extern orxSTRING                 *azParams;
extern orxS32                     s32Animating;
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

#endif

#endif
/** @} */
