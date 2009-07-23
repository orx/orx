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
 * @file 04_Anim.c
 * @date 10/08/2008
 * @author iarwain@orx-project.org
 *
 * Frame tutorial
 */


#include "orxPluginAPI.h"


/* This is a basic C tutorial creating objects with animations.
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
 *
 * This tutorial shows how to handle animation graphs.
 *
 * As graph could quickly become very complex, here we only have 4 animations here: IdleRight, IdleLeft, WalkRight and WalkLeft.
 *
 * All the possible transitions are defined in the config file. When you request an animation
 * as target, the engine will figure out how to chain the animations to arrive at the requested one.
 * In the same way, if no animation are requested, it'll follow the links according to their priorities.
 *
 * There's also loop and random control as WIP, so they won't be described in this tutorial.
 *
 * As for an example, if the soldier was in WalkRight and you stop pressing keys, it'll know
 * he has to go immediately to IdleRight, without having code to give this order.
 *
 * We also show how to subscribe to the animation events so as to know when animations are started, stopped or cut.
 */


/** Tutorial objects
 */
orxOBJECT *pstSoldier;


/** Event handler
 */
orxSTATUS orxFASTCALL EventHandler(const orxEVENT *_pstEvent)
{
  orxANIM_EVENT_PAYLOAD *pstPayload;

  /* Gets event payload */
  pstPayload = (orxANIM_EVENT_PAYLOAD *)_pstEvent->pstPayload;

  /* Depending on event type */
  switch(_pstEvent->eID)
  {
    case orxANIM_EVENT_START:
    {
      /* Logs info */
      orxLOG("Animation <%s>@<%s> has started!", pstPayload->zAnimName, orxObject_GetName(orxOBJECT(_pstEvent->hRecipient)));

      break;
    }

    case orxANIM_EVENT_STOP:
    {
      /* Logs info */
      orxLOG("Animation <%s>@<%s> has stoped!", pstPayload->zAnimName, orxObject_GetName(orxOBJECT(_pstEvent->hRecipient)));

      break;
    }

    case orxANIM_EVENT_CUT:
    {
      /* Logs info */
      orxLOG("Animation <%s>@<%s> has been cut!", pstPayload->zAnimName, orxObject_GetName(orxOBJECT(_pstEvent->hRecipient)));

      break;
    }

    case orxANIM_EVENT_LOOP:
    {
      /* Logs info */
      orxLOG("Animation <%s>@<%s> has looped!", pstPayload->zAnimName, orxObject_GetName(orxOBJECT(_pstEvent->hRecipient)));

      break;
    }
  }

  /* Done! */
  return orxSTATUS_SUCCESS;
}

/** Update callback
 */
void orxFASTCALL Update(const orxCLOCK_INFO *_pstClockInfo, void *_pstContext)
{
  orxVECTOR vScale;

  /* Is walk right active? */
  if(orxInput_IsActive("GoRight"))
  {
    /* Sets walk right as target anim */
    orxObject_SetTargetAnim(pstSoldier, "WalkRight");
  }
  /* Is walk left active? */
  else if(orxInput_IsActive("GoLeft"))
  {
    /* Sets walk left as target anim */
    orxObject_SetTargetAnim(pstSoldier, "WalkLeft");
  }
  /* No walk active */
  else
  {
    /* Removes target anim */
    orxObject_SetTargetAnim(pstSoldier, orxNULL);
  }

  /* Is scale up active ? */
  if(orxInput_IsActive("ScaleUp"))
  {
    /* Scales up the soldier */
    orxObject_SetScale(pstSoldier, orxVector_Mulf(&vScale, orxObject_GetScale(pstSoldier, &vScale), orx2F(1.02f)));
  }
  /* Is scale down active? */
  if(orxInput_IsActive("ScaleDown"))
  {
    /* Scales down the soldier */
    orxObject_SetScale(pstSoldier, orxVector_Mulf(&vScale, orxObject_GetScale(pstSoldier, &vScale), orx2F(0.98f)));
  }
}


/** Inits the tutorial
 */
orxSTATUS Init()
{
  orxCLOCK       *pstClock;
  orxINPUT_TYPE   eType;
  orxENUM         eID;
  orxSTRING       zInputWalkLeft, zInputWalkRight, zInputScaleUp, zInputScaleDown;

  /* Loads config file and selects main section */
  orxConfig_Load("../04_Anim.ini");

  /* Gets input binding names */
  orxInput_GetBinding("WalkLeft", 0, &eType, &eID);
  zInputWalkLeft  = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("WalkRight", 0, &eType, &eID);
  zInputWalkRight = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("ScaleUp", 0, &eType, &eID);
  zInputScaleUp   = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("ScaleDown", 0, &eType, &eID);
  zInputScaleDown = orxInput_GetBindingName(eType, eID);

  /* Displays a small hint in console */
  orxLOG("\n- '%s' & '%s' will change the soldier's animations\n- '%s' & '%s' will scale the soldier", zInputWalkLeft, zInputWalkRight, zInputScaleUp, zInputScaleDown);

  /* Creates viewport */
  orxViewport_CreateFromConfig("Viewport");

  /* Creates soldier */
  pstSoldier = orxObject_CreateFromConfig("Soldier");

  /* Creates a 100 Hz clock */
  pstClock = orxClock_Create(orx2F(0.01f), orxCLOCK_TYPE_USER);

  /* Registers our update callback */
  orxClock_Register(pstClock, Update, orxNULL, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_NORMAL);

  /* Registers event handler */
  orxEvent_AddHandler(orxEVENT_TYPE_ANIM, EventHandler);

  /* Done! */
  return orxSTATUS_SUCCESS;
}

/* Registers plugin entry */
orxPLUGIN_DECLARE_ENTRY_POINT(Init);
