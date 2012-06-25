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
 * @file orxiOS.m
 * @date 03/03/2010
 * @author iarwain@orx-project.org
 *
 * iOS orx main application
 *
 */


/** Includes
 */
#include "orx.h"


#if defined(__orxIOS__) && defined(__orxOBJC__)

#define orxIOS_KZ_CONFIG_SECTION                      "iOS"
#define orxIOS_KZ_CONFIG_ACCELEROMETER_FREQUENCY      "AccelerometerFrequency"


/** Main function pointer
 */
orxSTATUS (orxFASTCALL *spfnRun)() = orxNULL;

/** Render inhibiter
 */
static orxSTATUS orxFASTCALL RenderInhibiter(const orxEVENT *_pstEvent)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

/** Orx application implementation
 */
@implementation orxAppDelegate

@synthesize poWindow;
@synthesize poViewController;

- (void) applicationDidFinishLaunching:(UIApplication *)_poApplication
{
  CGRect stFrame;

  /* Gets application's size */
  stFrame = [[UIScreen mainScreen] applicationFrame];

  /* Creates main window */
  self.poWindow = [[UIWindow alloc] initWithFrame:stFrame]; 

  /* Creates orx view */
  stFrame.origin.y = 0.0;

  /* Creates view controller */
  self.poViewController = [[orxViewController alloc] init];

  /* Sets black background color */
  [poWindow setBackgroundColor: [UIColor blackColor]];

  /* Attaches controller to window */
  [poWindow setRootViewController:poViewController];

  /* Activates window */
  [poWindow makeKeyAndVisible];

  /* Assigns main loop to a new thread */
  [NSThread detachNewThreadSelector:@selector(MainLoop) toTarget:self withObject:nil];
}

- (void) dealloc
{
  /* Releases view controller & window */
  [poViewController release];
  [poWindow release];

  /* Calls parent method */
  [super dealloc];
}

- (void) applicationWillTerminate:(UIApplication *)_poApplication
{
  /* Sends event */
  orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_CLOSE);
}

- (void) applicationDidEnterBackground:(UIApplication *)_poApplication
{
  /* Sends event */
  if(orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_BACKGROUND) != orxSTATUS_FAILURE)
  {
    /* Adds render inhibiter */
    orxEvent_AddHandler(orxEVENT_TYPE_RENDER, RenderInhibiter);
  }
}

- (void) applicationWillEnterForeground:(UIApplication *)_poApplication
{
  /* Sends event */
  if(orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_FOREGROUND) != orxSTATUS_FAILURE)
  {
    /* Removes render inhibiter */
    orxEvent_RemoveHandler(orxEVENT_TYPE_RENDER, RenderInhibiter);
  }
}

- (void) applicationWillResignActive:(UIApplication *)_poApplication
{
  /* Sends event */
  orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_FOCUS_LOST);
}

- (void) applicationDidBecomeActive:(UIApplication *)_poApplication
{
  static orxBOOL sbFirstActivation = orxTRUE;

  /* First activation? */
  if(sbFirstActivation != orxFALSE)
  {
    /* Updates status */
    sbFirstActivation = orxFALSE;
  }
  else
  {
    /* Sends event */
    orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_FOCUS_GAINED);
  }
}

- (void) accelerometer:(UIAccelerometer *)_poAccelerometer didAccelerate:(UIAcceleration *)_poAcceleration
{
  orxSYSTEM_EVENT_PAYLOAD stPayload;
  
  /* Inits event's payload */
  orxMemory_Zero(&stPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
  stPayload.stAccelerometer.dTime = orx2D(_poAcceleration.timestamp);
  orxVector_Set(&(stPayload.stAccelerometer.vAcceleration), orx2F(_poAcceleration.x), orx2F(-_poAcceleration.y), orx2F(-_poAcceleration.z));

  /* Sends it */
  orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_ACCELERATE, self, orxNULL, &stPayload);
}

- (void) MainLoop
{
  NSAutoreleasePool  *poMainPool;

  /* Allocates main memory pool */
  poMainPool = [[NSAutoreleasePool alloc] init];

  /* Inits the engine */
  if(orxModule_Init(orxMODULE_ID_MAIN) != orxSTATUS_FAILURE)
  {
    /* Displays help */
    if(orxParam_DisplayHelp() != orxSTATUS_FAILURE)
    {
      orxSTATUS               eClockStatus, eMainStatus;
      orxBOOL                 bStop;
      orxSYSTEM_EVENT_PAYLOAD stPayload;

      /* Registers default event handler */
      orxEvent_AddHandler(orxEVENT_TYPE_SYSTEM, orx_DefaultEventHandler);

      /* Pushes config section */
      orxConfig_PushSection(orxIOS_KZ_CONFIG_SECTION);

      /* Has valid accelerometer frequency? */
      if(orxConfig_HasValue(orxIOS_KZ_CONFIG_ACCELEROMETER_FREQUENCY) != orxFALSE)
      {
        orxFLOAT fFrequency;

        /* Valid? */
        if((fFrequency = orxConfig_GetFloat(orxIOS_KZ_CONFIG_ACCELEROMETER_FREQUENCY)) > orxFLOAT_0)
        {
          /* Applies it */
          [[UIAccelerometer sharedAccelerometer] setUpdateInterval: 1.0f / fFrequency];

          /* Binds accelerometer */
          [[UIAccelerometer sharedAccelerometer] setDelegate: (orxAppDelegate *)[[UIApplication sharedApplication] delegate]];
        }
      }
      else
      {
        /* Binds accelerometer */
        [[UIAccelerometer sharedAccelerometer] setDelegate: (orxAppDelegate *)[[UIApplication sharedApplication] delegate]];
      }

      /* Pops config section */
      orxConfig_PopSection();

      /* Main loop */
      for(bStop = orxFALSE;
          bStop == orxFALSE;
          bStop = ((sbStopByEvent != orxFALSE) || (eMainStatus == orxSTATUS_FAILURE) || (eClockStatus == orxSTATUS_FAILURE)) ? orxTRUE : orxFALSE)
      {
        NSAutoreleasePool *poPool;

        /* Allocates memory pool */
        poPool = [[NSAutoreleasePool alloc] init];

        /* Sends frame start event */
        orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_GAME_LOOP_START, orxNULL, orxNULL, &stPayload);

        /* Runs the engine */
        eMainStatus = spfnRun();

        /* Updates clock system */
        eClockStatus = orxClock_Update();

        /* Sends frame stop event */
        orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_GAME_LOOP_STOP, orxNULL, orxNULL, &stPayload);

        /* Updates frame counter */
        stPayload.u32FrameCounter++;

        /* Releases memory pool */
        [poPool release];
      }

      /* Removes event handler */
      orxEvent_RemoveHandler(orxEVENT_TYPE_SYSTEM, orx_DefaultEventHandler);
    }

    /* Exits from engine */
    orxModule_Exit(orxMODULE_ID_MAIN);
  }

  /* Exits from all modules */
  orxModule_ExitAll();

  /* Releases main pool */
  [poMainPool release];

  /* Exits from the Debug system */
  orxDEBUG_EXIT();
}

@end

#endif /* __orxIOS__ && __orxOBJC__ */
