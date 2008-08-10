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


#include "orx.h"


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
 * NB: Here we use a config value to move the parent node, as we requesting this value from the config
 * module every time we need it, we can update this from the config file on the fly.
 * To affect the current execution, just press backspace and the move speed will be updated from file.
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

  /* Is left button pressed? */
  if(orxMouse_IsButtonPressed(orxMOUSE_BUTTON_LEFT))
  {
    /* Rotates Parent object CCW */
    orxObject_SetRotation(pstParentObject, orxObject_GetRotation(pstParentObject) + orx2F(-4.0f) * _pstClockInfo->fDT);
  }    
  /* Is right button pressed? */
  if(orxMouse_IsButtonPressed(orxMOUSE_BUTTON_RIGHT))
  {
    /* Rotates Parent object CW */
    orxObject_SetRotation(pstParentObject, orxObject_GetRotation(pstParentObject) + orx2F(4.0f) * _pstClockInfo->fDT);
  }    

  /* Is '+' pressed? */
  if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_ADD))
  {
    /* Scales up the Parent object */
    orxObject_SetScale(pstParentObject, orxVector_Mulf(&vScale, orxObject_GetScale(pstParentObject, &vScale), orx2F(1.02f)));
  }
  /* Is '-' pressed? */
  else if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_SUBTRACT))
  {
    /* Scales down the Parent object */
    orxObject_SetScale(pstParentObject, orxVector_Mulf(&vScale, orxObject_GetScale(pstParentObject, &vScale), orx2F(0.98f)));
  }

  /* Selects parent object config section */
  orxConfig_SelectSection("ParentObject");

  /* Gets object position */
  orxObject_GetPosition(pstParentObject, &vPosition);

  /* Is left arrow pressed? */
  if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_LEFT))
  {
    /* Moves it on left */
    vPosition.fX -= orxConfig_GetFloat("MoveSpeed") * _pstClockInfo->fDT;
  }
  /* Is right arrow pressed? */
  else if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_RIGHT))
  {
    /* Moves it on right */
    vPosition.fX += orxConfig_GetFloat("MoveSpeed") * _pstClockInfo->fDT;
  }
  /* Is up arrow pressed? */
  if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_UP))
  {
    /* Moves it on left */
    vPosition.fY -= orxConfig_GetFloat("MoveSpeed") * _pstClockInfo->fDT;
  }
  /* Is down arrow pressed? */
  else if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_DOWN))
  {
    /* Moves it on right */
    vPosition.fY += orxConfig_GetFloat("MoveSpeed") * _pstClockInfo->fDT;
  }

  /* Updates object with new position */
  orxObject_SetPosition(pstParentObject, &vPosition);
}


/** Inits the tutorial
 */
orxSTATUS Init()
{
  orxCLOCK *pstClock;

  /* Displays a small hint in console */
  orxLOG("\n- Left & right buttons will rotate the Parent object\n- '+' & '-' will scale it\n- Arrow keys will move it.");

  /* Loads config file and selects main section */
  orxConfig_Load("../../03_Frame/03_Frame.ini");

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
  orxClock_Register(pstClock, Update, orxNULL, orxMODULE_ID_MAIN);

  /* Done! */
  return orxSTATUS_SUCCESS;
}

/* Registers plugin entry */
orxPLUGIN_DECLARE_ENTRY_POINT(Init);
