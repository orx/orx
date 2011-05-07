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
 * @file orx.h
 * @date 02/09/2005
 * @author
 *
 * @todo
 */

/**
 * @addtogroup Orx
 *
 * Main orx include
 *
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef _orx_H_
#define _orx_H_

#define __orxEXTERN__ /* Not compiling orx library */


#include "orxInclude.h"
#include "orxKernel.h"
#include "orxUtils.h"

#ifndef __orxPLUGIN__

/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Should stop execution by default event handling?
 */
static orxBOOL sbStopByEvent = orxFALSE;


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Orx default basic event handler
 * @param[in]   _pstEvent                     Sent event
 * @return      orxSTATUS_SUCCESS if handled / orxSTATUS_FAILURE otherwise
 */
static orxSTATUS orxFASTCALL orx_DefaultEventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(_pstEvent->eType == orxEVENT_TYPE_SYSTEM);

  /* Depending on event ID */
  switch(_pstEvent->eID)
  {
    /* Close event */
    case orxSYSTEM_EVENT_CLOSE:
    {
      /* Updates status */
      sbStopByEvent = orxTRUE;

      break;
    }

    default:
    {
      break;
    }
  }

  /* Done! */
  return eResult;
}

/** Default main setup (module dependencies)
*/
static void orxFASTCALL orx_MainSetup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_MAIN, orxMODULE_ID_PARAM);
  orxModule_AddDependency(orxMODULE_ID_MAIN, orxMODULE_ID_CLOCK);
  orxModule_AddDependency(orxMODULE_ID_MAIN, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_MAIN, orxMODULE_ID_INPUT);
  orxModule_AddDependency(orxMODULE_ID_MAIN, orxMODULE_ID_EVENT);
  orxModule_AddDependency(orxMODULE_ID_MAIN, orxMODULE_ID_FILE);
  orxModule_AddDependency(orxMODULE_ID_MAIN, orxMODULE_ID_LOCALE);
  orxModule_AddDependency(orxMODULE_ID_MAIN, orxMODULE_ID_PLUGIN);
  orxModule_AddDependency(orxMODULE_ID_MAIN, orxMODULE_ID_OBJECT);
  orxModule_AddDependency(orxMODULE_ID_MAIN, orxMODULE_ID_PROFILER);

  orxModule_AddOptionalDependency(orxMODULE_ID_MAIN, orxMODULE_ID_SCREENSHOT);

  return;
}

#ifdef __orxANDROID__

#include <jni.h>
#include <android/log.h>

/** JNI environment
 *  This will be loaded in android-support.cpp.
 */
extern JNIEnv *mEnv;
extern JavaVM *mVM;
extern JNIEnv *globalEnv;

/****************in android there is no actual main, so define it as orxMain will be called by jni******/
#define main orxMain
/****************end******************/

#endif /* __orxANDROID__ */

#ifdef __orxIPHONE__

  #ifdef __orxOBJC__

#import <UIKit/UIKit.h>

/** Orx application interface
 */
@interface orxAppDelegate : NSObject <UIAccelerometerDelegate>
{
  UIWindow *poWindow;
  orxView  *poView;
}

@property (nonatomic, retain) UIWindow *poWindow;
@property (nonatomic, retain) UIView   *poView;

- (void)  MainLoop;

@end

extern orxSTATUS (orxFASTCALL *spfnRun)();

/** Orx main execution function
 * @param[in]   _u32NbParams                  Main function parameters number (argc)
 * @param[in]   _azParams                     Main function parameter list (argv)
 * @param[in]   _pfnInit                      Main init function (should init all the main stuff and register the main event handler to override the default one)
 * @param[in]   _pfnRun                       Main run function (will be called once per frame, should return orxSTATUS_SUCCESS to continue processing)
 * @param[in]   _pfnExit                      Main exit function (should clean all the main stuff)
 */
static orxINLINE void orx_Execute(orxU32 _u32NbParams, orxSTRING _azParams[], const orxMODULE_INIT_FUNCTION _pfnInit, const orxMODULE_RUN_FUNCTION _pfnRun, const orxMODULE_EXIT_FUNCTION _pfnExit)
{
  /* Inits the Debug System */
  orxDEBUG_INIT();

  /* Checks */
  orxASSERT(_u32NbParams > 0);
  orxASSERT(_azParams != orxNULL);
  orxASSERT(_pfnRun != orxNULL);

  /* Registers main module */
  orxModule_Register(orxMODULE_ID_MAIN, orx_MainSetup, _pfnInit, _pfnExit);

  /* Stores run callback */
  spfnRun = _pfnRun;

  /* Registers all other modules */
  orxModule_RegisterAll();

  /* Calls all modules setup */
  orxModule_SetupAll();

  /* Sends the command line arguments to orxParam module */
  if(orxParam_SetArgs(_u32NbParams, _azParams) != orxSTATUS_FAILURE)
  {
    NSAutoreleasePool *poPool;

    /* Allocates memory pool */
    poPool = [[NSAutoreleasePool alloc] init];

    /* Launches application */
    UIApplicationMain(_u32NbParams, _azParams, nil, @"orxAppDelegate");

    /* Releases memory pool */
    [poPool release];
  }

  /* Done! */
  return;
}

  #endif /* __orxOBJC__ */

#else /* __orxIPHONE__ */

  #ifdef __orxANDROID_NATIVE__

  #include <android/log.h>
  #include <android_native_app_glue.h>
  #include <android/sensor.h>


/* Defined in orxAndroidSupport.c */
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
        orxBOOL   bStop;

        /* Main loop */
        for(bStop = orxFALSE, sbStopByEvent = orxFALSE;
            bStop == orxFALSE;
            bStop = ((sbStopByEvent != orxFALSE) || (eMainStatus == orxSTATUS_FAILURE) || (eClockStatus == orxSTATUS_FAILURE)) ? orxTRUE : orxFALSE)
        {
          /* Reads all pending events */
          orxS32 s32Ident, s32Events;
          struct android_poll_source *pstSource;
  
          while((s32Ident = ALooper_pollAll((s32Animating || pstApp->destroyRequested) ? 0 : -1, NULL, (int*) &s32Events, (void **)&pstSource)) >= 0)
          {
             /* Processes this event */
             if(pstSource != NULL)
             {
               pstSource->process(pstApp, pstSource);
             }

            /* If a sensor has data, process it now */
            if(s32Ident == LOOPER_ID_USER)
            {
              if(poAccelerometerSensor != NULL)
              {
              	orxSYSTEM_EVENT_PAYLOAD stPayload;

                /* Inits event's payload */
                orxMemory_Zero(&stPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));

                ASensorEvent oEvent;
                while(ASensorEventQueue_getEvents(poSensorEventQueue, &oEvent, 1) > 0)
                {
                  stPayload.stAccelerometer.pAccelerometer = &oEvent;
                  stPayload.stAccelerometer.fX = (orxFLOAT)oEvent.acceleration.x;
                  stPayload.stAccelerometer.fY = (orxFLOAT)oEvent.acceleration.y;
                  stPayload.stAccelerometer.fZ = (orxFLOAT)oEvent.acceleration.z;

                  /* Sends event */
                  orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_ACCELERATE, orxNULL, orxNULL, &stPayload);
                }
              }
            }
          }

          if(s32Animating || pstApp->destroyRequested)
          {
            /* Runs the engine */
            eMainStatus = _pfnRun();

            /* Updates clock system */
            eClockStatus = orxClock_Update();
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

  #else /* __orxANDROID_NATIVE__ */

/** Orx main execution function
 * @param[in]   _u32NbParams                  Main function parameters number (argc)
 * @param[in]   _azParams                     Main function parameter list (argv)
 * @param[in]   _pfnInit                      Main init function (should init all the main stuff and register the main event handler to override the default one)
 * @param[in]   _pfnRun                       Main run function (will be called once per frame, should return orxSTATUS_SUCCESS to continue processing)
 * @param[in]   _pfnExit                      Main exit function (should clean all the main stuff)
 */
static orxINLINE void orx_Execute(orxU32 _u32NbParams, orxSTRING _azParams[], const orxMODULE_INIT_FUNCTION _pfnInit, const orxMODULE_RUN_FUNCTION _pfnRun, const orxMODULE_EXIT_FUNCTION _pfnExit)
{
  /* Inits the Debug System */
  orxDEBUG_INIT();

  /* Checks */
  orxASSERT(_u32NbParams > 0);
  orxASSERT(_azParams != orxNULL);
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
        orxSYSTEM_EVENT_PAYLOAD stPayload;
        orxSTATUS               eClockStatus, eMainStatus;
        orxBOOL                 bStop;

        /* Clears payload */
        orxMemory_Zero(&stPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));

        /* Main loop */
        for(bStop = orxFALSE, sbStopByEvent = orxFALSE;
            bStop == orxFALSE;
            bStop = ((sbStopByEvent != orxFALSE) || (eMainStatus == orxSTATUS_FAILURE) || (eClockStatus == orxSTATUS_FAILURE)) ? orxTRUE : orxFALSE)
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

    #ifdef __orxMSVC__

    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>

/** Orx main execution function (console-less windows application)
 * @param[in]   _pfnInit                      Main init function (should init all the main stuff and register the main event handler to override the default one)
 * @param[in]   _pfnRun                       Main run function (will be called once per frame, should return orxSTATUS_SUCCESS to continue processing)
 * @param[in]   _pfnExit                      Main exit function (should clean all the main stuff)
 */
static orxINLINE void orx_WinExecute(const orxMODULE_INIT_FUNCTION _pfnInit, const orxMODULE_RUN_FUNCTION _pfnRun, const orxMODULE_EXIT_FUNCTION _pfnExit)
{
  #define orxMAX_ARGS 256

  int   argc;
  char *argv[orxMAX_ARGS];
  char *pcToken, *pcNextToken, *pcFirstDelimiters;
  LPSTR lpFullCmdLine;

  /* Gets full command line */
  lpFullCmdLine = GetCommandLineA();

  /* Starts with a double quote? */
  if(*orxString_SkipWhiteSpaces(lpFullCmdLine) == '"')
  {
    /* Gets first delimiters */
    pcFirstDelimiters = "\"";
  }
  else
  {
    /* Gets first delimiters */
    pcFirstDelimiters = " ";
  }

  /* Process command line */
  for(argc = 0, pcNextToken = NULL, pcToken = strtok_s(lpFullCmdLine, pcFirstDelimiters, &pcNextToken);
      pcToken && (argc < orxMAX_ARGS);
      pcToken = strtok_s(NULL, " ", &pcNextToken))
  {
    argv[argc++] = pcToken;
  }

  /* Inits and executes orx */
  orx_Execute(argc, argv, _pfnInit, _pfnRun, _pfnExit);
}

    #endif /* __orxMSVC__ */

  #endif /* __orxANDROID_NATIVE__ */

#endif /* __orxIPHONE__ */

#endif /* __orxPLUGIN__ */

#endif /*_orx_H_*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/** @} */
