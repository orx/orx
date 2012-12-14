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
 * @file 03_Frame.c
 * @date 10/08/2008
 * @author iarwain@orx-project.org
 *
 * Frame tutorial
 */


#include "orxPluginAPI.h"


/* This is a basic C tutorial creating objects with linked (hierarchy) frames.
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
 * See previous tutorials for more info about the basic object creation and clock handling.
 * All positions/scales/rotations are stored in orxFRAME for orx objects.
 * These frame are assembled in a hierarchy graph, meaning that changing a parent frame
 * properties will affect all its children.
 * Here we have four objects that we link to a common parent (an empty object) and a fifth one which has no parent.
 * The first two children are implicitely created using the object's config property ChildList whereas the two
 * others are created and linked in code (for didactic purposes).
 * You can use arrow keys to move the parent object, '+' & '-' to scale it, left & right mouse button to rotate it.
 * All these transformation will affect its children.
 * This provides us with an easy way to create grouped object or complex object assembly and transform them
 * (position, scale, rotation, speed, ...) very easily.
 *
 * NB: If you change the viewport size and position, the function orxRender_GetWorldPosition() will still
 * return a valid information. You can test it by changing the viewport properties in the config file.
 */


/** Tutorial objects
 */
orxOBJECT *pstParentObject;


/** Update callback
 */
void orxFASTCALL Update(const orxCLOCK_INFO *_pstClockInfo, void *_pstContext)
{
  orxVECTOR vScale, vPosition;

  /* Is rotate left input active ? */
  if(orxInput_IsActive("RotateLeft"))
  {
    /* Rotates Parent object CCW */
    orxObject_SetRotation(pstParentObject, orxObject_GetRotation(pstParentObject) - orxMATH_KF_PI * _pstClockInfo->fDT);
  }    
  /* Is rotate right input active? */
  if(orxInput_IsActive("RotateRight"))
  {
    /* Rotates Parent object CW */
    orxObject_SetRotation(pstParentObject, orxObject_GetRotation(pstParentObject) + orxMATH_KF_PI * _pstClockInfo->fDT);
  }    

  /* Is scale up input active? */
  if(orxInput_IsActive("ScaleUp"))
  {
    /* Scales up the Parent object */
    orxObject_SetScale(pstParentObject, orxVector_Mulf(&vScale, orxObject_GetScale(pstParentObject, &vScale), orx2F(1.02f)));
  }
  /* Is scale down input active? */
  if(orxInput_IsActive("ScaleDown"))
  {
    /* Scales down the Parent object */
    orxObject_SetScale(pstParentObject, orxVector_Mulf(&vScale, orxObject_GetScale(pstParentObject, &vScale), orx2F(0.98f)));
  }

  /* Is mouse in viewport? */
  if(orxRender_GetWorldPosition(orxMouse_GetPosition(&vPosition), &vPosition))
  {
    orxVECTOR vParentPosition;

    /* Gets current parent position */
    orxObject_GetWorldPosition(pstParentObject, &vParentPosition);

    /* Keeps Z value */
    vPosition.fZ = vParentPosition.fZ;

    /* Updates object with mouse position */
    orxObject_SetPosition(pstParentObject, &vPosition);
  }
}


/** Inits the tutorial
 */
orxSTATUS Init()
{
  orxCLOCK       *pstClock;
  orxOBJECT      *pstObject;
  orxINPUT_TYPE   eType;
  orxENUM         eID;
  const orxSTRING zInputRotateLeft;
  const orxSTRING zInputRotateRight;
  const orxSTRING zInputScaleUp;
  const orxSTRING zInputScaleDown;

  /* Loads config file */
  orxConfig_Load("../03_Frame.ini");

  /* Reloads inputs */
  orxInput_Load(orxSTRING_EMPTY);

  /* Gets input binding names */
  orxInput_GetBinding("RotateLeft", 0, &eType, &eID);
  zInputRotateLeft  = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("RotateRight", 0, &eType, &eID);
  zInputRotateRight = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("ScaleUp", 0, &eType, &eID);
  zInputScaleUp     = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("ScaleDown", 0, &eType, &eID);
  zInputScaleDown   = orxInput_GetBindingName(eType, eID);

  /* Displays a small hint in console */
  orxLOG("\n- The parent object will follow the mouse\n- '%s' & '%s' will rotate it\n- '%s' & '%s' will scale it", zInputRotateLeft, zInputRotateRight, zInputScaleUp, zInputScaleDown);

  /* Creates viewport */
  orxViewport_CreateFromConfig("Viewport");

  /* Creates Parent object */
  pstParentObject = orxObject_CreateFromConfig("ParentObject");

  /* Creates all 3 test objects and links the last two to our parent object */
  orxObject_CreateFromConfig("Object0");
  pstObject = orxObject_CreateFromConfig("Object1");
  orxObject_SetParent(pstObject, pstParentObject);
  pstObject = orxObject_CreateFromConfig("Object2");
  orxObject_SetParent(pstObject, pstParentObject);

  /* Gets main clock */
  pstClock = orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE);

  /* Registers our update callback */
  orxClock_Register(pstClock, Update, orxNULL, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_NORMAL);

  /* Done! */
  return orxSTATUS_SUCCESS;
}

/* Registers plugin entry */
orxPLUGIN_DECLARE_ENTRY_POINT(Init);
