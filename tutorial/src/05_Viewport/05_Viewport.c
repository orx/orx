/* Orx - Portable Game Engine
 *
 * Copyright (c) 2010 Orx-Project
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
 * @file 05_Viewport.c
 * @date 11/08/2008
 * @author iarwain@orx-project.org
 *
 * Viewport tutorial
 */


#include "orxPluginAPI.h"


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
 * - F12 to capture a screenshot
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
void orxFASTCALL Update(const orxCLOCK_INFO *_pstClockInfo, void *_pstContext)
{
  orxVECTOR vPos;
  orxCAMERA *pstCamera;
  orxFLOAT  fWidth, fHeight, fX, fY;

  /* *** CAMERA CONTROLS *** */

  /* Gets first viewport camera */
  pstCamera = orxViewport_GetCamera(pstViewport);

  /* Camera rotate left? */
  if(orxInput_IsActive("CameraRotateLeft"))
  {
    /* Rotates camera CCW */
    orxCamera_SetRotation(pstCamera, orxCamera_GetRotation(pstCamera) + orx2F(-4.0f) * _pstClockInfo->fDT);
  }
  /* Camera rotate right? */
  if(orxInput_IsActive("CameraRotateRight"))
  {
    /* Rotates camera CW */
    orxCamera_SetRotation(pstCamera, orxCamera_GetRotation(pstCamera) + orx2F(4.0f) * _pstClockInfo->fDT);
  }

  /* Camera zoom in? */
  if(orxInput_IsActive("CameraZoomIn"))
  {
    /* Camera zoom in */
    orxCamera_SetZoom(pstCamera, orxCamera_GetZoom(pstCamera) * orx2F(1.02f));
  }
  /* Camera zoom out? */
  if(orxInput_IsActive("CameraZoomOut"))
  {
    /* Camera zoom out */
    orxCamera_SetZoom(pstCamera, orxCamera_GetZoom(pstCamera) * orx2F(0.98f));
  }

  /* Gets camera position */
  orxCamera_GetPosition(pstCamera, &vPos);

  /* Camera right? */
  if(orxInput_IsActive("CameraRight"))
  {
    /* Updates position */
    vPos.fX += orx2F(500) * _pstClockInfo->fDT;
  }
  /* Camera left? */
  if(orxInput_IsActive("CameraLeft"))
  {
    /* Updates position */
    vPos.fX -= orx2F(500) * _pstClockInfo->fDT;
  }
  /* Camera down? */
  if(orxInput_IsActive("CameraDown"))
  {
    /* Updates position */
    vPos.fY += orx2F(500) * _pstClockInfo->fDT;
  }
  /* Camera up? */
  if(orxInput_IsActive("CameraUp"))
  {
    /* Updates position */
    vPos.fY -= orx2F(500) * _pstClockInfo->fDT;
  }

  /* Updates camera position */
  orxCamera_SetPosition(pstCamera, &vPos);


  /* *** VIEWPORT CONTROLS *** */

  /* Gets viewport size */
  orxViewport_GetRelativeSize(pstViewport, &fWidth, &fHeight);

  /* Viewport scale up? */
  if(orxInput_IsActive("ViewportScaleUp"))
  {
    /* Scales viewport up */
    fWidth *= orx2F(1.02f);
    fHeight*= orx2F(1.02f);
  }
  /* Viewport scale down? */
  if(orxInput_IsActive("ViewportScaleDown"))
  {
    /* Scales viewport down */
    fWidth *= orx2F(0.98f);
    fHeight*= orx2F(0.98f);
  }

  /* Updates viewport size */
  orxViewport_SetRelativeSize(pstViewport, fWidth, fHeight);

  /* Gets viewport position */
  orxViewport_GetPosition(pstViewport, &fX, &fY);

  /* Viewport right? */
  if(orxInput_IsActive("ViewportRight"))
  {
    /* Updates position */
    fX += orx2F(500) * _pstClockInfo->fDT;
  }
  /* Viewport left? */
  if(orxInput_IsActive("ViewportLeft"))
  {
    /* Updates position */
    fX -= orx2F(500) * _pstClockInfo->fDT;
  }
  /* Viewport down? */
  if(orxInput_IsActive("ViewportDown"))
  {
    /* Updates position */
    fY += orx2F(500) * _pstClockInfo->fDT;
  }
  /* Viewport up? */
  if(orxInput_IsActive("ViewportUp"))
  {
    /* Updates position */
    fY -= orx2F(500) * _pstClockInfo->fDT;
  }

  /* Updates viewport position */
  orxViewport_SetPosition(pstViewport, fX, fY);

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
}


/** Inits the tutorial
 */
orxSTATUS Init()
{
  orxCLOCK       *pstClock;
  orxINPUT_TYPE   eType;
  orxENUM         eID;
  orxSTRING       zInputCameraLeft, zInputCameraRight, zInputCameraUp, zInputCameraDown;
  orxSTRING       zInputCameraRotateLeft, zInputCameraRotateRight, zInputCameraZoomIn, zInputCameraZoomOut;
  orxSTRING       zInputViewportLeft, zInputViewportRight, zInputViewportUp, zInputViewportDown;
  orxSTRING       zInputViewportScaleUp, zInputViewportScaleDown;

  /* Loads config file and selects main section */
  orxConfig_Load("../05_Viewport.ini");

  /* Reloads inputs */
  orxInput_Load(orxSTRING_EMPTY);

  /* Gets input binding names */
  orxInput_GetBinding("CameraLeft", 0, &eType, &eID);
  zInputCameraLeft = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("CameraRight", 0, &eType, &eID);
  zInputCameraRight = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("CameraUp", 0, &eType, &eID);
  zInputCameraUp = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("CameraDown", 0, &eType, &eID);
  zInputCameraDown = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("CameraRotateLeft", 0, &eType, &eID);
  zInputCameraRotateLeft = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("CameraRotateRight", 0, &eType, &eID);
  zInputCameraRotateRight = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("CameraZoomIn", 0, &eType, &eID);
  zInputCameraZoomIn = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("CameraZoomOut", 0, &eType, &eID);
  zInputCameraZoomOut = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("ViewportLeft", 0, &eType, &eID);
  zInputViewportLeft = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("ViewportRight", 0, &eType, &eID);
  zInputViewportRight = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("ViewportUp", 0, &eType, &eID);
  zInputViewportUp = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("ViewportDown", 0, &eType, &eID);
  zInputViewportDown = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("ViewportScaleUp", 0, &eType, &eID);
  zInputViewportScaleUp = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("ViewportScaleDown", 0, &eType, &eID);
  zInputViewportScaleDown = orxInput_GetBindingName(eType, eID);

  /* Displays a small hint in console */
  orxLOG("\n* Worskpaces 1 & 3 display camera 1 content"
         "\n* Workspace 2 displays camera 2 (by default it's twice as close as the other cameras)"
         "\n* Workspace 3 displays camera 3"
         "\n- Soldier will be positioned (in the world) so as to be always displayed under the mouse"
         "\n- '%s', '%s', '%s' & '%s' control camera 1 positioning"
         "\n- '%s' & '%s' control camera 1 rotation"
         "\n- '%s' & '%s' control camera 1 zoom"
         "\n- '%s', '%s', '%s' & '%s' control viewport 1 positioning"
         "\n- '%s' & '%s' control viewport 1 size",
         zInputCameraUp, zInputCameraLeft, zInputCameraDown, zInputCameraRight,
         zInputCameraRotateLeft, zInputCameraRotateRight,
         zInputCameraZoomIn, zInputCameraZoomOut,
         zInputViewportUp, zInputViewportLeft, zInputViewportDown, zInputViewportRight,
         zInputViewportScaleUp, zInputViewportScaleDown);

  /* Creates all viewports */
  pstViewport = orxViewport_CreateFromConfig("Viewport1");
  orxViewport_CreateFromConfig("Viewport2");
  orxViewport_CreateFromConfig("Viewport3");
  orxViewport_CreateFromConfig("Viewport4");

  /* Creates objects */
  orxObject_CreateFromConfig("Box");
  pstSoldier = orxObject_CreateFromConfig("Soldier");

  /* Creates a 100 Hz clock */
  pstClock = orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE);

  /* Registers our update callback */
  orxClock_Register(pstClock, Update, orxNULL, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_NORMAL);

  /* Done! */
  return orxSTATUS_SUCCESS;
}

/* Registers plugin entry */
orxPLUGIN_DECLARE_ENTRY_POINT(Init);
