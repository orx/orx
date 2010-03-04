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
 * @file orxMain.c
 * @date 03/03/2010
 * @author iarwain@orx-project.org
 *
 * iPhone orx application
 *
 */


/** Includes
 */
#include "orx.h"


#if defined(__orxIPHONE__) && defined(__orxOBJC__)

/** Main function pointer
 */
orxSTATUS (orxFASTCALL *spfnRun)() = orxNULL;


/** Orx application implementation
 */
@implementation orxAppDelegate

@synthesize poWindow;
@synthesize poView;

- (void) applicationDidFinishLaunching: (UIApplication *)_poApplication
{
  CGRect stFrame;
  
  /* Gets application's size */
  stFrame = [[UIScreen mainScreen] applicationFrame];
  
  /* Creates main window */
  self.poWindow = [[UIWindow alloc] initWithFrame:stFrame]; 
  
  /* Creates orx view */
  stFrame.origin.y = 0.0;
  poView = [[orxView alloc] initWithFrame:stFrame];
  
  /* Attaches it window */
  [poWindow addSubview:poView];
  
  /* Assigns main loop to a new thread */
  [NSThread detachNewThreadSelector:@selector(MainLoop) toTarget:self withObject:nil];
  
  /* Inits and binds accelerometer */
  [[UIAccelerometer sharedAccelerometer] setUpdateInterval: 1.0f / 60.0f];
  [[UIAccelerometer sharedAccelerometer] setDelegate: self];
  
  /* Activates window */
  [poWindow makeKeyAndVisible];
}

- (void) dealloc
{
  /* Releases view & window */
  [poView release];
  [poWindow release];
  
  /* Calls parent method */
  [super dealloc];
}

- (void) accelerometer:(UIAccelerometer *)_poAccelerometer didAccelerate:(UIAcceleration *)_poAcceleration
{
  orxIPHONE_EVENT_PAYLOAD stPayload;
  
  /* Inits event's payload */
  orxMemory_Zero(&stPayload, sizeof(orxIPHONE_EVENT_PAYLOAD));
  stPayload.poAccelerometer = _poAccelerometer;
  stPayload.poAcceleration  = _poAcceleration;
  
  /* Sends it */
  orxEVENT_SEND(orxEVENT_TYPE_IPHONE, orxIPHONE_EVENT_ACCELERATE, self, orxNULL, &stPayload);
}

- (void) MainLoop
{
  orxSTATUS           eClockStatus, eMainStatus;
  orxBOOL             bStop;
  NSAutoreleasePool  *poMainPool;
  
  /* Allocates main memory pool */
  poMainPool = [[NSAutoreleasePool alloc] init];
  
  /* Inits the engine */
  if(orxModule_Init(orxMODULE_ID_MAIN) != orxSTATUS_FAILURE)
  {      
    /* Displays help */
    if(orxParam_DisplayHelp() != orxSTATUS_FAILURE)
    {
      /* Registers default event handler */
      orxEvent_AddHandler(orxEVENT_TYPE_SYSTEM, orx_DefaultEventHandler);
      
      /* Main loop */
      for(bStop = orxFALSE;
          bStop == orxFALSE;
          bStop = ((sbStopByEvent != orxFALSE) || (eMainStatus == orxSTATUS_FAILURE) || (eClockStatus == orxSTATUS_FAILURE)) ? orxTRUE : orxFALSE)
      {
        NSAutoreleasePool *poPool;
        
        /* Allocates memory pool */
        poPool = [[NSAutoreleasePool alloc] init];
        
        /* Runs the engine */
        eMainStatus = spfnRun();
        
        /* Updates clock system */
        eClockStatus = orxClock_Update();
        
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

#endif /* __orxIPHONE__ && __orxOBJC__ */
