/* Orx - Portable Game Engine
 *
 * Orx is the legal property of its developers, whose names
 * are listed in the COPYRIGHT file distributed
 * with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * @file orxTest.m
 * @date 28/01/2010
 * @author iarwain@orx-project.org
 *
 * iPhone test executable
 *
 */


#import <UIKit/UIKit.h>
#include "orx.h"

static orxSTATUS orxFASTCALL (*spfnRun)() = orxNULL;

@interface orxAppDelegate : NSObject <UIAccelerometerDelegate>
{
  UIWindow *Window;
  UIView   *View;
}

@property (nonatomic, retain) IBOutlet UIWindow *Window;
@property (nonatomic, retain) IBOutlet UIView *View;

- (void)Loop;

@end

@implementation orxAppDelegate

@synthesize Window;
@synthesize View;

- (void) applicationDidFinishLaunching: (UIApplication *)_poApplication
{
  [NSThread detachNewThreadSelector:@selector(Loop) toTarget:self withObject:nil];  

  [Window makeKeyAndVisible];
}

- (void) dealloc
{
  /* Removes event handler */
  orxEvent_RemoveHandler(orxEVENT_TYPE_SYSTEM, orx_DefaultEventHandler);

  /* Exits from engine */
  orxModule_Exit(orxMODULE_ID_MAIN);

  /* Exits from all modules */
  orxModule_ExitAll();

  /* Exits from the Debug system */
  orxDEBUG_EXIT();

  [View release];
  [Window release];
  [super dealloc];
}

- (void)Loop
{
  orxSTATUS eClockStatus, eMainStatus;
  orxBOOL   bStop;

  /* Main loop */
  for(bStop = orxFALSE;
      bStop == orxFALSE;
      bStop = ((sbStopByEvent != orxFALSE) || (eMainStatus == orxSTATUS_FAILURE) || (eClockStatus == orxSTATUS_FAILURE)) ? orxTRUE : orxFALSE)
  {
    /* Runs the engine */
    eMainStatus = spfnRun();

    /* Updates clock system */
    eClockStatus = orxClock_Update();
  }
}

@end

static orxINLINE void orxExec(orxU32 _u32NbParams, orxSTRING _azParams[], const orxMODULE_INIT_FUNCTION _pfnInit, const orxMODULE_RUN_FUNCTION _pfnRun, const orxMODULE_EXIT_FUNCTION _pfnExit)
{
  /* Inits the Debug System */
  orxDEBUG_INIT();
  
  /* Checks */
  orxASSERT(_u32NbParams > 0);
  orxASSERT(_azParams != orxNULL);
  orxASSERT(_pfnInit != orxNULL);
  orxASSERT(_pfnRun != orxNULL);
  orxASSERT(_pfnExit != orxNULL);
  
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
    /* Inits the engine */
    if(orxModule_Init(orxMODULE_ID_MAIN) != orxSTATUS_FAILURE)
    {      
      /* Registers default event handler */
      orxEvent_AddHandler(orxEVENT_TYPE_SYSTEM, orx_DefaultEventHandler);
      
      /* Displays help */
      if(orxParam_DisplayHelp() != orxSTATUS_FAILURE)
      {
        NSAutoreleasePool *poPool;

        /* Allocates memory pool */
        poPool = [[NSAutoreleasePool alloc] init];

        /* Launches application */
        UIApplicationMain(_u32NbParams, _azParams, nil, nil);

        /* Releases memory pool */
        [poPool release];
      }
    }
  }

  /* Done! */
  return;
}
      
static orxSTATUS orxFASTCALL Init()
{
  return orxSTATUS_SUCCESS;
}

static orxSTATUS orxFASTCALL Run()
{
  return orxSTATUS_SUCCESS;
}

static void orxFASTCALL Exit()
{
}

int main(int argc, char *argv[])
{
  /* Launches application */
  orxExec(argc, argv, Init, Run, Exit);

  /* Done! */
  return EXIT_SUCCESS;
}
