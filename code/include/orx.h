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
 * @file orx.h
 * @date 02/09/2005
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup Orx
 *
 * Main orx include, execution convenience helpers, freely modifiable by users
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
  orxASSERT(_pstEvent->eID == orxSYSTEM_EVENT_CLOSE);

  /* Updates status */
  sbStopByEvent = orxTRUE;

  /* Done! */
  return eResult;
}

/** Default main setup (module dependencies)
*/
static void orxFASTCALL orx_MainSetup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_MAIN, orxMODULE_ID_OBJECT);
  orxModule_AddDependency(orxMODULE_ID_MAIN, orxMODULE_ID_RENDER);

  orxModule_AddOptionalDependency(orxMODULE_ID_MAIN, orxMODULE_ID_SCREENSHOT);

  /* Done! */
  return;
}

#ifdef __orxIOS__

  #ifdef __orxOBJC__

#import <UIKit/UIKit.h>

/** Orx application interface
 */
@interface orxAppDelegate : NSObject <UIAccelerometerDelegate>
{
  UIWindow           *poWindow;
  orxViewController  *poViewController;
}

@property (nonatomic, retain) UIWindow         *poWindow;
@property (nonatomic, retain) UIViewController *poViewController;

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
  orxModule_Register(orxMODULE_ID_MAIN, "MAIN", orx_MainSetup, _pfnInit, _pfnExit);

  /* Stores run callback */
  spfnRun = _pfnRun;

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

#else /* __orxIOS__ */

  #if defined(__orxANDROID__) || defined(__orxANDROID_NATIVE__)

#include "main/orxAndroid.h"

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
  orxASSERT(_pfnRun != orxNULL);

  /* Registers main module */
  orxModule_Register(orxMODULE_ID_MAIN, "MAIN", orx_MainSetup, _pfnInit, _pfnExit);

  /* Sends the command line arguments to orxParam module */
  if(orxParam_SetArgs(_u32NbParams, _azParams) != orxSTATUS_FAILURE)
  {
    /* Sets thread callbacks */
    orxThread_SetCallbacks(orxAndroid_JNI_SetupThread, orxNULL, orxNULL);

    /* Inits the engine */
    if(orxModule_Init(orxMODULE_ID_MAIN) != orxSTATUS_FAILURE)
    {
      orxSYSTEM_EVENT_PAYLOAD stPayload;
      orxSTATUS               eClockStatus, eMainStatus;
      orxBOOL                 bStop;

      /* Registers default event handler */
      orxEvent_AddHandler(orxEVENT_TYPE_SYSTEM, orx_DefaultEventHandler);
      orxEvent_SetHandlerIDFlags(orx_DefaultEventHandler, orxEVENT_TYPE_SYSTEM, orxNULL, orxEVENT_GET_FLAG(orxSYSTEM_EVENT_CLOSE), orxEVENT_KU32_MASK_ID_ALL);

      /* Clears payload */
      orxMemory_Zero(&stPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));

      /* Main loop */
      for(bStop = orxFALSE, sbStopByEvent = orxFALSE;
          bStop == orxFALSE;
          bStop = ((sbStopByEvent != orxFALSE) || (eMainStatus == orxSTATUS_FAILURE) || (eClockStatus == orxSTATUS_FAILURE)) ? orxTRUE : orxFALSE)
      {
        orxAndroid_PumpEvents();

        /* Sends frame start event */
        orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_GAME_LOOP_START, orxNULL, orxNULL, &stPayload);

        /* Runs game specific code */
        eMainStatus = _pfnRun();

        /* Updates clock system */
        eClockStatus = orxClock_Update();

        /* Sends frame stop event */
        orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_GAME_LOOP_STOP, orxNULL, orxNULL, &stPayload);

        /* Updates frame count */
        stPayload.u32FrameCount++;
      }

      /* Removes event handler */
      orxEvent_RemoveHandler(orxEVENT_TYPE_SYSTEM, orx_DefaultEventHandler);

      /* Exits from the engine */
      orxModule_Exit(orxMODULE_ID_MAIN);
    }
  }

  /* Exits from the Debug system */
  orxDEBUG_EXIT();
}

  #else /* __orxANDROID__ || __orxANDROID_NATIVE__ */

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
  orxModule_Register(orxMODULE_ID_MAIN, "MAIN", orx_MainSetup, _pfnInit, _pfnExit);

  /* Sends the command line arguments to orxParam module */
  if(orxParam_SetArgs(_u32NbParams, _azParams) != orxSTATUS_FAILURE)
  {
    /* Inits the engine */
    if(orxModule_Init(orxMODULE_ID_MAIN) != orxSTATUS_FAILURE)
    {
      orxSYSTEM_EVENT_PAYLOAD stPayload;
      orxSTATUS               eClockStatus, eMainStatus;
      orxBOOL                 bStop;

      /* Registers default event handler */
      orxEvent_AddHandler(orxEVENT_TYPE_SYSTEM, orx_DefaultEventHandler);
      orxEvent_SetHandlerIDFlags(orx_DefaultEventHandler, orxEVENT_TYPE_SYSTEM, orxNULL, orxEVENT_GET_FLAG(orxSYSTEM_EVENT_CLOSE), orxEVENT_KU32_MASK_ID_ALL);

      /* Clears payload */
      orxMemory_Zero(&stPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));

      /* Main loop */
      for(bStop = orxFALSE, sbStopByEvent = orxFALSE;
          bStop == orxFALSE;
          bStop = ((sbStopByEvent != orxFALSE) || (eMainStatus == orxSTATUS_FAILURE) || (eClockStatus == orxSTATUS_FAILURE)) ? orxTRUE : orxFALSE)
      {
        /* Sends frame start event */
        orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_GAME_LOOP_START, orxNULL, orxNULL, &stPayload);

        /* Runs game specific code */
        eMainStatus = _pfnRun();

        /* Updates clock system */
        eClockStatus = orxClock_Update();

        /* Sends frame stop event */
        orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_GAME_LOOP_STOP, orxNULL, orxNULL, &stPayload);

        /* Updates frame count */
        stPayload.u32FrameCount++;
      }

      /* Removes event handler */
      orxEvent_RemoveHandler(orxEVENT_TYPE_SYSTEM, orx_DefaultEventHandler);

      /* Exits from the engine */
      orxModule_Exit(orxMODULE_ID_MAIN);
    }
  }

  /* Exits from the Debug system */
  orxDEBUG_EXIT();
}

  #endif /* __orxANDROID__ || __orxANDROID_NATIVE__ */

#endif /* __orxIOS__ */

#endif /* !__orxPLUGIN__ */

#endif /*_orx_H_*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/** @} */
