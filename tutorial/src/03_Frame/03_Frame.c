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
 * - Backspace to reload all configuration files (provided that config history is turned on)
 * It also exits if the orxSYSTEM_EVENT_CLOSE signal is sent.
 *
 * See previous tutorials for more info about the basic object creation and clock handling.
 * All positions/scales/rotations are stored in orxFRAME for orx objects.
 * These frame are assembled in a hierarchy graph, meaning that changing a parent frame
 * properties will affect all its children.
 * Here we have four objects that we link to a common parent (an empty object) and a fifth one which has no parent.
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
orxOBJECT *pstObjectList[5];
orxOBJECT *pstParentObject;


/** Update callback
 */
orxVOID orxFASTCALL Update(orxCONST orxCLOCK_INFO *_pstClockInfo, orxVOID *_pstContext)
{
  orxVECTOR vScale, vPosition;
  orxU32    i;

  /* For all objects */
  for(i = 0; i < 5; i++)
  {
    /* Rotates object on self */
    orxObject_SetRotation(pstObjectList[i], orxMATH_KF_PI * _pstClockInfo->fTime);
  }

  /* Is rotate left input active ? */
  if(orxInput_IsActive("RotateLeft"))
  {
    /* Rotates Parent object CCW */
    orxObject_SetRotation(pstParentObject, orxObject_GetRotation(pstParentObject) + orx2F(-4.0f) * _pstClockInfo->fDT);
  }    
  /* Is rotate right input active? */
  if(orxInput_IsActive("RotateRight"))
  {
    /* Rotates Parent object CW */
    orxObject_SetRotation(pstParentObject, orxObject_GetRotation(pstParentObject) + orx2F(4.0f) * _pstClockInfo->fDT);
  }    

  /* Is scale up input active? */
  if(orxInput_IsActive("ScaleUp"))
  {
    /* Scales up the Parent object */
    orxObject_SetScale(pstParentObject, orxVector_Mulf(&vScale, orxObject_GetScale(pstParentObject, &vScale), orx2F(1.02f)));
  }
  /* Is '-' pressed? */
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
  orxINPUT_TYPE   eType;
  orxENUM         eID;
  orxSTRING       zInputRotateLeft, zInputRotateRight, zInputScaleUp, zInputScaleDown;

  /* Loads config file and selects main section */
  orxConfig_Load("../03_Frame.ini");

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

  /* Creates all 5 test objects */
  pstObjectList[0] = orxObject_CreateFromConfig("Object0");
  pstObjectList[1] = orxObject_CreateFromConfig("Object1");
  pstObjectList[2] = orxObject_CreateFromConfig("Object2");
  pstObjectList[3] = orxObject_CreateFromConfig("Object3");
  pstObjectList[4] = orxObject_CreateFromConfig("Object4");

  /* Links the four last to our parent object */
  orxObject_SetParent(pstObjectList[1], pstParentObject);
  orxObject_SetParent(pstObjectList[2], pstParentObject);
  orxObject_SetParent(pstObjectList[3], pstParentObject);
  orxObject_SetParent(pstObjectList[4], pstParentObject);

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
