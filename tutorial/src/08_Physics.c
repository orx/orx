/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2010 Orx-Project
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
 * @file 08_Physics.c
 * @date 21/08/2008
 * @author iarwain@orx-project.org
 *
 * Physics tutorial
 */


#include "orx.h"


/* This is a basic C tutorial creating physical bodies and playing with physics and collisions.
 *
 * See previous tutorials for more info about the basic object creation, clock, animation, viewport, sound and FX handling.
 *
 * This tutorial shows how to add physical properties to objects and handle collisions.
 *
 * As you can see, the physical properties are completely data-driven. So creating an object
 * with physical properties (ie. with a body) or without is the exact same line of code.
 *
 * Objects can be linked to a body which can be static or dynamic.
 * Each body can be made of up to 8 parts.
 *
 * A body part is defined by
 * - its shape (currently box, sphere and mesh (ie. convex polygon) are the only available)
 * - information about the shape size (corners for the box, center and radius for the sphere, vertices for the mesh)
 *   if no size data is specified, the shape will try to fill the complete body (using the
 *   object size and scale)
 * - collision "self" flags that defines this part
 * - collision "check" mask that defines with which other parts this one will collide
 *   (two parts in the same body will never collide)
 * - a flag (Solid) specifying if this shaped should only give information about collisions
 *   or if it should impact on the body physics simulation (bouncing, etc...)
 * - various attributes such as restitution, friction, density, ...
 *
 * Here we create static solid walls around our screen. We then spawn boxes in the middle.
 * The number of boxes created is tweakable through the config file and is 100 by default.
 *
 * The only interaction possible is using left and right mouse buttons to rotate the camera.
 * As we rotate it, we also update the gravity vector of our simulation.
 * Doing so, it gives the impression that the boxes will be always falling toward the bottom of our screen
 * no matter how the camera is rotated.
 *
 * We also register to the physics events to add a visual FXs on two colliding objects.
 * By default the FX is a fast color flash and is, as usual, tweakable in realtime (ie. reloading
 * the config history will apply the new settings immediately as the FX isn't kept in cache by default).
 * 
 * Updating an object scale (including changing its scale with FXs) will update its physical properties.
 * Keep in mind that scaling an object with a physical body is more expensive as we have to delete
 * the current shapes and recreate them at the correct size.
 * This is done as our current single physics plugin is based on Box2D which doesn't allow realtime
 * rescaling of shapes.
 *
 * This tutorial does only show basic physics and collision control, but, for example, you can also
 * be notified with events for object separating or keeping contact.
 *
 */


/** Tutorial objects
 */
orxCAMERA *pstCamera;


/** Event handler
 */
orxSTATUS orxFASTCALL EventHandler(const orxEVENT *_pstEvent)
{
  /* Is a new contact? */
  if(_pstEvent->eID == orxPHYSICS_EVENT_CONTACT_ADD)
  {
    orxOBJECT *pstObject1, *pstObject2;

    /* Gets colliding objects */
    pstObject1 = orxOBJECT(_pstEvent->hRecipient);
    pstObject2 = orxOBJECT(_pstEvent->hSender);

    /* Adds bump FX on both objects */
    orxObject_AddFX(pstObject1, "Bump");
    orxObject_AddFX(pstObject2, "Bump");
  }

  /* Done! */
  return orxSTATUS_SUCCESS;
}


/** Update callback
 */
void orxFASTCALL Update(const orxCLOCK_INFO *_pstClockInfo, void *_pstContext)
{
  orxFLOAT fDeltaRotation = orxFLOAT_0;

  /* Rotating left? */
  if(orxInput_IsActive("RotateLeft"))
  {
    /* Computes rotation delta */
    fDeltaRotation = orx2F(4.0f) * _pstClockInfo->fDT;
  }    
  /* Rotating right? */
  if(orxInput_IsActive("RotateRight"))
  {
    /* Computes rotation delta */
    fDeltaRotation = orx2F(-4.0f) * _pstClockInfo->fDT;
  }

  /* Turned? */
  if(fDeltaRotation != orxFLOAT_0)
  {
    orxVECTOR vGravity;

    /* Rotates camera */
    orxCamera_SetRotation(pstCamera, orxCamera_GetRotation(pstCamera) + fDeltaRotation);

    /* Gets gravity */
    if(orxPhysics_GetGravity(&vGravity))
    {
      /* Updates it */
      orxVector_2DRotate(&vGravity, &vGravity, fDeltaRotation);

      /* Applies it */
      orxPhysics_SetGravity(&vGravity);
    }
  }
}


/** Inits the tutorial
 */
orxSTATUS orxFASTCALL Init()
{
  orxCLOCK       *pstClock;
  orxVIEWPORT    *pstViewport;
  orxU32          i;
  orxINPUT_TYPE   eType;
  orxENUM         eID;
  orxINPUT_MODE   eMode;
  const orxSTRING zInputRotateLeft;
  const orxSTRING zInputRotateRight;

  /* Gets input binding names */
  orxInput_GetBinding("RotateLeft", 0, &eType, &eID, &eMode);
  zInputRotateLeft = orxInput_GetBindingName(eType, eID, eMode);

  orxInput_GetBinding("RotateRight", 0, &eType, &eID, &eMode);
  zInputRotateRight = orxInput_GetBindingName(eType, eID, eMode);

  /* Displays a small hint in console */
  orxLOG("\n- '%s' & '%s' will rotate the camera"
         "\n* Gravity will follow the camera"
         "\n* a bump visual FX is played on objects that collide", zInputRotateLeft, zInputRotateRight);

  /* Creates viewport */
  pstViewport = orxViewport_CreateFromConfig("Viewport");

  /* Gets camera */
  pstCamera = orxViewport_GetCamera(pstViewport);

  /* Gets main clock */
  pstClock = orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE);

  /* Registers our update callback */
  orxClock_Register(pstClock, Update, orxNULL, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_NORMAL);

  /* Registers event handler */
  orxEvent_AddHandler(orxEVENT_TYPE_PHYSICS, EventHandler);

  /* Creates sky */
  orxObject_CreateFromConfig("Sky");

  /* Creates walls */
  orxObject_CreateFromConfig("Walls");

  /* Pushes tutorial section */
  orxConfig_PushSection("Tutorial");

  /* For all requested boxes */
  for(i = 0; i < orxConfig_GetU32("BoxNumber"); i++)
  {
    /* Creates it */
    orxObject_CreateFromConfig("Box");
  }

  /* Pops config section */
  orxConfig_PopSection();

  /* Done! */
  return orxSTATUS_SUCCESS;
}

/** Run function
 */
orxSTATUS orxFASTCALL Run()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Should quit? */
  if(orxInput_IsActive("Quit"))
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Exit function
 */
void orxFASTCALL Exit()
{
  /* We're a bit lazy here so we let orx clean all our mess! :) */
}

/** Main function
 */
int main(int argc, char **argv)
{
  /* Executes a new instance of tutorial */
  orx_Execute(argc, argv, Init, Run, Exit);

  return EXIT_SUCCESS;
}


#ifdef __orxMSVC__

// Here's an example for a console-less program under windows with visual studio
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  // Inits and executes orx
  orx_WinExecute(Init, Run, Exit);

  // Done!
  return EXIT_SUCCESS;
}

#endif // __orxMSVC__
