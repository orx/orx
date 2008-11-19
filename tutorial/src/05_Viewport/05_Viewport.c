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
 * @file 05_Viewport.c
 * @date 11/08/2008
 * @author iarwain@orx-project.org
 *
 * Viewport tutorial
 */


#include "orx.h"


/* This is a basic C tutorial creating viewports and cameras.
 * As we are using the default executable for this tutorial, this code
 * will be loaded and executed as a runtime plugin.
 *
 * In addition, some basics are handled for us by the main executable.
 * First of all, it will load all available plugins and modules. If you
 * require only some of those, then it's better to write your own executable
 * instead of a plugin. This will be covered in a later tutorial.
 *
 * The main executable also handles some keys:
 * - F11 as vertical sync toggler
 * - Escape as exit key
 * - Backspace to reload all configuration files (provided that config history is turned on)
 * It also exits if the orxSYSTEM_EVENT_CLOSE signal is sent.
 *
 * See previous tutorials for more info about the basic object creation, clock and animation handling.
 *
 * This tutorial shows how to use multiple viewports with multiple cameras.
 * By default, it'll create 4 viewports.
 *
 * The top left corner one (1) and the bottom right corner one (4)
 * shares the same camera (1). To achieve this, we just need to use the same name in the config file.
 * Furthermore, when manipulating this camera using left & right mouse buttons to rotate it,
 * arrow keys to move it and '+' & '-' to zoom it, these two viewports will be affected.
 *
 * The top right viewport (2) is based on another camera (2) which frustum is narrower than the first one,
 * resulting in a display twice as big. You can't affect this viewport through keys for this tutorial.
 * 
 * The last viewport (3) is based on another camera (3) which has the exact same settings than the first one.
 * This viewport will display what you originally had in the viewport 1 & 4.
 * You can also interact directly with the first viewport properties, using WASD to move it and 'Q' & 'E' to resize it.
 * 
 * When two viewports overlap, the oldest one (ie. the one created before the other) will be displayed on top.
 *
 * Lastly, we have a box that doesn't move at all, and a little soldier whose world position will be determined
 * by the current mouse on-screen position.
 * In other words, no matter which viewport your mouse is on, and no matter how is set the camera for this viewport
 * the soldier will always have his feet at the same position than your mouse on screen (provided it's in a viewport).
 *
 * Viewports and objects are created with random colors and sizes using the character '~' in config file.
 *
 * NB: Cameras store their position/zoom/rotation in an orxFRAME structure, thus allowing them to be part of the orxFRAME
 * hierarchy. (cf. tutorial 03_Frame)
 * For example, object auto-following can be achieved by setting object's frame as camera's frame parent.
 */


/** Tutorial objects
 */
orxOBJECT   *pstSoldier;
orxVIEWPORT *pstViewport;


/** Update callback
 */
orxVOID orxFASTCALL Update(orxCONST orxCLOCK_INFO *_pstClockInfo, orxVOID *_pstContext)
{
  orxVECTOR vPos;
  orxCAMERA *pstCamera;
  orxFLOAT  fWidth, fHeight, fX, fY;

  /* *** SOLDIER MOVE UPDATE *** */

  /* Has mouse world position? */
  if(orxRender_GetWorldPosition(orxMouse_GetPosition(&vPos), &vPos) != orxNULL)
  {
    orxVECTOR vSoldierPos;

    /* Gets object current position */
    orxObject_GetWorldPosition(pstSoldier, &vSoldierPos);

    /* Keeps Z value */
    vPos.fZ = vSoldierPos.fZ;

    /* Moves the soldier under the cursor */
    orxObject_SetPosition(pstSoldier, &vPos);
  }

  /* *** CAMERA CONTROLS *** */

  /* Gets first viewport camera */
  pstCamera = orxViewport_GetCamera(pstViewport);

  /* Is left button pressed? */
  if(orxMouse_IsButtonPressed(orxMOUSE_BUTTON_LEFT))
  {
    /* Rotates camera CCW */
    orxCamera_SetRotation(pstCamera, orxCamera_GetRotation(pstCamera) + orx2F(-4.0f) * _pstClockInfo->fDT);
  }
  /* Is right button pressed? */
  if(orxMouse_IsButtonPressed(orxMOUSE_BUTTON_RIGHT))
  {
    /* Rotates camera CW */
    orxCamera_SetRotation(pstCamera, orxCamera_GetRotation(pstCamera) + orx2F(4.0f) * _pstClockInfo->fDT);
  }

  /* Is '+' pressed? */
  if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_ADD))
  {
    /* Camera zoom in */
    orxCamera_SetZoom(pstCamera, orxCamera_GetZoom(pstCamera) * orx2F(1.02f));
  }
  /* Is '-' pressed? */
  if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_SUBTRACT))
  {
    /* Camera zoom out */
    orxCamera_SetZoom(pstCamera, orxCamera_GetZoom(pstCamera) * orx2F(0.98f));
  }

  /* Gets camera position */
  orxCamera_GetPosition(pstCamera, &vPos);

  /* Is right arrow pressed? */
  if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_RIGHT))
  {
    /* Updates position */
    vPos.fX += orx2F(500) * _pstClockInfo->fDT;
  }
  /* Is left arrow pressed? */
  if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_LEFT))
  {
    /* Updates position */
    vPos.fX -= orx2F(500) * _pstClockInfo->fDT;
  }
  /* Is down arrow pressed? */
  if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_DOWN))
  {
    /* Updates position */
    vPos.fY += orx2F(500) * _pstClockInfo->fDT;
  }
  /* Is up arrow pressed? */
  if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_UP))
  {
    /* Updates position */
    vPos.fY -= orx2F(500) * _pstClockInfo->fDT;
  }

  /* Updates camera position */
  orxCamera_SetPosition(pstCamera, &vPos);


  /* *** VIEWPORT CONTROLS *** */

  /* Gets viewport size */
  orxViewport_GetRelativeSize(pstViewport, &fWidth, &fHeight);

  /* Is 'e' pressed? */
  if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_E))
  {
    /* Scales viewport up */
    fWidth *= orx2F(1.02f);
    fHeight*= orx2F(1.02f);
  }
  /* Is 'q' pressed? */
  if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_Q))
  {
    /* Scales viewport down */
    fWidth *= orx2F(0.98f);
    fHeight*= orx2F(0.98f);
  }

  /* Updates viewport size */
  orxViewport_SetRelativeSize(pstViewport, fWidth, fHeight);

  /* Gets viewport position */
  orxViewport_GetPosition(pstViewport, &fX, &fY);

  /* Is 'd' pressed? */
  if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_D))
  {
    /* Updates position */
    fX += orx2F(500) * _pstClockInfo->fDT;
  }
  /* Is 'a' pressed? */
  if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_A))
  {
    /* Updates position */
    fX -= orx2F(500) * _pstClockInfo->fDT;
  }
  /* Is 's' pressed? */
  if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_S))
  {
    /* Updates position */
    fY += orx2F(500) * _pstClockInfo->fDT;
  }
  /* Is 'w' pressed? */
  if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_W))
  {
    /* Updates position */
    fY -= orx2F(500) * _pstClockInfo->fDT;
  }

  /* Updates viewport position */
  orxViewport_SetPosition(pstViewport, fX, fY);
}


/** Inits the tutorial
 */
orxSTATUS Init()
{
  orxCLOCK *pstClock;

  /* Displays a small hint in console */
  orxLOG("\n* Worskpaces 1 & 3 display camera 1 content"
         "\n* Workspace 2 displays camera 2 (by default it's twice as close as the other cameras)"
         "\n* Workspace 3 displays camera 3"
         "\n- Soldier will be positioned (in the world) so as to be always displayed under the mouse"
         "\n- Arrow keys control camera 1 positioning, left & right buttons its rotation and '+' & '-' its zoom"
         "\n- WASD keys control viewport 1 positioning, A & E keys its size");

  /* Loads config file and selects main section */
  orxConfig_Load("../05_Viewport.ini");

  /* Creates all viewports */
  pstViewport = orxViewport_CreateFromConfig("Viewport1");
  orxViewport_CreateFromConfig("Viewport2");
  orxViewport_CreateFromConfig("Viewport3");
  orxViewport_CreateFromConfig("Viewport4");

  /* Creates objects */
  orxObject_CreateFromConfig("Box");
  pstSoldier = orxObject_CreateFromConfig("Soldier");

  /* Creates a 100 Hz clock */
  pstClock = orxClock_Create(orx2F(0.01f), orxCLOCK_TYPE_USER);

  /* Registers our update callback */
  orxClock_Register(pstClock, Update, orxNULL, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_NORMAL);

  /* Deactivates vertical sync */
  orxDisplay_EnableVSync(orxFALSE);

  /* Done! */
  return orxSTATUS_SUCCESS;
}

/* Registers plugin entry */
orxPLUGIN_DECLARE_ENTRY_POINT(Init);
