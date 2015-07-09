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
 * @file 04_Anim.c
 * @date 10/08/2008
 * @author iarwain@orx-project.org
 *
 * Frame tutorial
 */


#include "orx.h"


/* This is a basic C tutorial creating objects with animations.
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
      orxLOG("Animation <%s>@<%s> has stopped!", pstPayload->zAnimName, orxObject_GetName(orxOBJECT(_pstEvent->hRecipient)));

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

    case orxANIM_EVENT_CUSTOM_EVENT:
    {
       /* Logs info */
       orxLOG("Animation <%s>@<%s> has sent the event [%s]!", pstPayload->zAnimName, orxObject_GetName(orxOBJECT(_pstEvent->hRecipient)), pstPayload->stCustom.zName);

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
orxSTATUS orxFASTCALL Init()
{
  orxCLOCK       *pstClock;
  orxINPUT_TYPE   eType;
  orxENUM         eID;
  orxINPUT_MODE   eMode;
  const orxSTRING zInputWalkLeft;
  const orxSTRING zInputWalkRight;
  const orxSTRING zInputScaleUp;
  const orxSTRING zInputScaleDown;

  /* Gets input binding names */
  orxInput_GetBinding("GoLeft", 0, &eType, &eID, &eMode);
  zInputWalkLeft  = orxInput_GetBindingName(eType, eID, eMode);

  orxInput_GetBinding("GoRight", 0, &eType, &eID, &eMode);
  zInputWalkRight = orxInput_GetBindingName(eType, eID, eMode);

  orxInput_GetBinding("ScaleUp", 0, &eType, &eID, &eMode);
  zInputScaleUp   = orxInput_GetBindingName(eType, eID, eMode);

  orxInput_GetBinding("ScaleDown", 0, &eType, &eID, &eMode);
  zInputScaleDown = orxInput_GetBindingName(eType, eID, eMode);

  /* Displays a small hint in console */
  orxLOG("\n- '%s' & '%s' will change the soldier's animations\n- '%s' & '%s' will scale the soldier", zInputWalkLeft, zInputWalkRight, zInputScaleUp, zInputScaleDown);

  /* Creates viewport */
  orxViewport_CreateFromConfig("Viewport");

  /* Creates soldier */
  pstSoldier = orxObject_CreateFromConfig("Soldier");

  /* Gets main clock */
  pstClock = orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE);

  /* Registers our update callback */
  orxClock_Register(pstClock, Update, orxNULL, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_NORMAL);

  /* Registers event handler */
  orxEvent_AddHandler(orxEVENT_TYPE_ANIM, EventHandler);

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
