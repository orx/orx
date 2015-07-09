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
 * @file 09_Scrolling.c
 * @date 26/08/2008
 * @author iarwain@orx-project.org
 *
 * Scrolling tutorial
 */


#include "orx.h"


/* This is a basic C tutorial showing how to do a parallax scrolling.
 *
 * See previous tutorials for more info about the basic object creation, clock, animation, viewport, sound, FX and physics/collision handling.
 *
 * This tutorial shows how to display a parallax scrolling.
 *
 * As you can see, there's no special code for the parallax scrolling. Actually,
 * orx's default 2D render plugin will take care of this for you, depending on how you
 * set the objects' properties in the config file. By default, AutoScroll is set to 'both'.
 *
 * This means a parallax scrolling will happen on both X and Y axis when the camera moves.
 * You can try to set this value to x, y or even remove it.
 *
 * Along the AutoScroll property, you can find the DepthScale one. This one is used to automatically
 * adjust objects' scale depending on how far they are from the camera.
 * The smaller the camera frustum is, the faster this autoscale will apply. You can try to play with
 * object positionning and camera near & far planes to achieve the desired scrolling and depth scale you want.
 *
 * You can change the scrolling speed (ie. the camera move speed) in the config file. As usual, you can
 * modify its value in real time and ask for a config history reload.
 *
 * As you can see, our update code simply moves the camera in the 3D space.
 * Pressing arrows will move it along X and Y axis, and pressing control & alt keys will move it along the Z one.
 * As told before, all the parallax scrolling will happen because objects have been flagged appropriately.
 * Your code merely needs to move your camera in your scenery, without having to bother about any scrolling effect.
 * This gives you a full control about how many scrolling planes you want, and which objects should be affected by it.
 *
 * The last point concerns the sky. As seen in the tutorial 03_Frame, we set the sky object's frame as a child
 * of the camera one. This means the position set for the sky object in the config file will always be
 * relative to the camera one.
 * In other words, the sky will always follow the camera, and as we put it, by default, at a depth of 1000 (ie. the
 * same value as the camera far frustum plane), it'll stay in the background.
 */


/** Tutorial objects
 */
orxCAMERA *pstCamera;


/** Update callback
 */
void orxFASTCALL Update(const orxCLOCK_INFO *_pstClockInfo, void *_pstContext)
{
  orxVECTOR vMove, vPosition, vScrollSpeed;

  /* *** SCROLLING UPDATE *** */

  /* Clears move vector */
  orxVector_Copy(&vMove, &orxVECTOR_0);

  /* Pushes tutorial config section */
  orxConfig_PushSection("Tutorial");

  /* Gets scroll speed */
  orxConfig_GetVector("ScrollSpeed", &vScrollSpeed);

  /* Pops config section */
  orxConfig_PopSection();

  /* Updates scroll speed with our current DT */
  orxVector_Mulf(&vScrollSpeed, &vScrollSpeed, _pstClockInfo->fDT);

  /* Going right? */
  if(orxInput_IsActive("CameraRight"))
  {
    /* Updates move vector */
    vMove.fX += vScrollSpeed.fX;
  }
  /* Going left? */
  if(orxInput_IsActive("CameraLeft"))
  {
    /* Updates move vector */
    vMove.fX -= vScrollSpeed.fX;
  }
  /* Going down? */
  if(orxInput_IsActive("CameraDown"))
  {
    /* Updates move vector */
    vMove.fY += vScrollSpeed.fY;
  }
  /* Going up? */
  if(orxInput_IsActive("CameraUp"))
  {
    /* Updates move vector */
    vMove.fY -= vScrollSpeed.fY;
  }
  /* Zoom in? */
  if(orxInput_IsActive("CameraZoomIn"))
  {
    /* Updates move vector */
    vMove.fZ += vScrollSpeed.fZ;
  }
  /* Zoom out? */
  if(orxInput_IsActive("CameraZoomOut"))
  {
    /* Updates move vector */
    vMove.fZ -= vScrollSpeed.fZ;
  }

  /* Updates camera position */
  orxCamera_SetPosition(pstCamera, orxVector_Add(&vPosition, orxCamera_GetPosition(pstCamera, &vPosition), &vMove));
}


/** Inits the tutorial
 */
orxSTATUS orxFASTCALL Init()
{
  orxVIEWPORT    *pstViewport;
  orxCLOCK       *pstClock;
  orxOBJECT      *pstSky;
  orxU32          i;
  orxINPUT_TYPE   eType;
  orxENUM         eID;
  orxINPUT_MODE   eMode;
  const orxSTRING zInputCameraLeft;
  const orxSTRING zInputCameraRight;
  const orxSTRING zInputCameraUp;
  const orxSTRING zInputCameraDown;
  const orxSTRING zInputCameraZoomIn;
  const orxSTRING zInputCameraZoomOut;

  /* Gets input binding names */
  orxInput_GetBinding("CameraLeft", 0, &eType, &eID, &eMode);
  zInputCameraLeft = orxInput_GetBindingName(eType, eID, eMode);

  orxInput_GetBinding("CameraRight", 0, &eType, &eID, &eMode);
  zInputCameraRight = orxInput_GetBindingName(eType, eID, eMode);

  orxInput_GetBinding("CameraUp", 0, &eType, &eID, &eMode);
  zInputCameraUp = orxInput_GetBindingName(eType, eID, eMode);

  orxInput_GetBinding("CameraDown", 0, &eType, &eID, &eMode);
  zInputCameraDown = orxInput_GetBindingName(eType, eID, eMode);

  orxInput_GetBinding("CameraZoomIn", 0, &eType, &eID, &eMode);
  zInputCameraZoomIn = orxInput_GetBindingName(eType, eID, eMode);

  orxInput_GetBinding("CameraZoomOut", 0, &eType, &eID, &eMode);
  zInputCameraZoomOut = orxInput_GetBindingName(eType, eID, eMode);

  /* Displays a small hint in console */
  orxLOG("\n- '%s', '%s', '%s' & '%s' will move the camera"
         "\n- '%s' & '%s' will zoom in/out"
         "\n* The scrolling and auto-scaling of objects is data-driven, no code required"
         "\n* The sky background will follow the camera (parent/child frame relation)",
         zInputCameraUp, zInputCameraLeft, zInputCameraDown, zInputCameraRight,
         zInputCameraZoomIn, zInputCameraZoomOut);

  /* Creates viewport */
  pstViewport = orxViewport_CreateFromConfig("Viewport");

  /* Gets camera */
  pstCamera = orxViewport_GetCamera(pstViewport);

  /* Gets main clock */
  pstClock = orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE);

  /* Registers our update callback */
  orxClock_Register(pstClock, Update, orxNULL, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_NORMAL);

  /* Creates sky */
  pstSky = orxObject_CreateFromConfig("Sky");

  /* Pushes tutorial config section */
  orxConfig_PushSection("Tutorial");

  /* For all requested clouds */
  for(i = 0; i < orxConfig_GetU32("CloudNumber"); i++)
  {
    /* Creates it */
    orxObject_CreateFromConfig("Cloud");
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
