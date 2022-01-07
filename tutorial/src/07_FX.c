/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2022 Orx-Project
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
 * @file 07_FX.c
 * @date 13/08/2008
 * @author iarwain@orx-project.org
 *
 * FX tutorial
 */


#include "orx.h"


/* This is a basic C tutorial creating visual FXs.
 *
 * See previous tutorials for more info about the basic object creation, clock, animation, viewport and sound handling.
 *
 * This tutorial shows what FXs are and how to create them.
 *
 * FXs are based on a combination of curves based on sine, triangle, square or linear shape,
 * applied on different parameters such as scale, rotation, position, speed, alpha and color.
 *
 * FXs are set through config file requiring only one line of code to apply them on an object.
 * There can be up to 8 curves of any type combined to form a single FX. Up to 8 FXs can be applied
 * on the same object at the same time.
 *
 * FXs can use absolute or relative values, depending on the Absolute attribute in its config.
 * Control over curve period, phase, pow and amplification over time is also granted.
 * For position and speed FXs, the output value can use the object's orientation and/or scale
 * so as to be applied relatively to the object's current state.
 * This allows the creation of pretty elaborated and nice looking visual FXs.
 *
 * FX parameters can be tweaked in the config file and reloaded on-the-fly using backspace,
 * unless the FX was specified to be cached in memory (cf. the ''KeepInCache'' attribute).
 *
 * For example you won't be able to tweak on the fly the circle FX as it has been defined
 * with this attribute in the default config file.
 * All the other FXs can be updated while the tutorial run.
 *
 * As always, random parameters can be used from config allowing some variety for a single FX.
 * For example, the wobble scale, the flash color and the "attack" move FXs are using limited random values.
 *
 * We also register to the FX events so as to display when FXs are played and stopped.
 * As the FX played on the box object is tagged as looping, it'll never stop.
 * Therefore the corresponding event (orxFX_EVENT_STOP) will never be sent.
 *
 * We also show briefly how to add some personal user data to an orxOBJECT (here a structure containing a single boolean).
 * We retrieve it in the event callback to lock the object when an FX starts and unlock it when it stops.
 * We use this lock to allow only one FX at a time on the soldier. It's only written here for didactic purpose.
 *
 */


/** Tutorial structure
 */
typedef struct MyObject
{
  orxBOOL bLock;
} MyObject;


/** Tutorial objects
 */
orxOBJECT *pstSoldier;
orxOBJECT *pstBox;
const orxSTRING zSelectedFX = "WobbleFX";


/** Event handler
 */
orxSTATUS orxFASTCALL EventHandler(const orxEVENT *_pstEvent)
{
  /* Input event? */
  if(_pstEvent->eType == orxEVENT_TYPE_INPUT)
  {
    /* Activated? */
    if(_pstEvent->eID == orxINPUT_EVENT_ON)
    {
      orxINPUT_EVENT_PAYLOAD *pstPayload;

      /* Gets event payload */
      pstPayload = (orxINPUT_EVENT_PAYLOAD *)_pstEvent->pstPayload;

      /* Main input? */
      if(orxString_Compare(pstPayload->zSetName, orxINPUT_KZ_CONFIG_SECTION) == 0)
      {
        /* Has a multi-input info? */
        if(pstPayload->aeType[1] != orxINPUT_TYPE_NONE)
        {
          /* Logs info */
          orxLOG("<%s> selected [%s + %s]", pstPayload->zInputName, orxInput_GetBindingName(pstPayload->aeType[0], pstPayload->aeID[0], pstPayload->aeMode[0]), orxInput_GetBindingName(pstPayload->aeType[1], pstPayload->aeID[1], pstPayload->aeMode[1]));
        }
        else
        {
          /* Logs info */
          orxLOG("<%s> selected [%s]", pstPayload->zInputName, orxInput_GetBindingName(pstPayload->aeType[0], pstPayload->aeID[0], pstPayload->aeMode[0]));
        }
      }
    }
  }
  else
  {
    orxFX_EVENT_PAYLOAD *pstPayload;
    orxOBJECT           *pstObject;

    /* Gets event payload */
    pstPayload = _pstEvent->pstPayload;

    /* Gets event recipient */
    pstObject = orxOBJECT(_pstEvent->hRecipient);

    /* Depending on event type */
    switch(_pstEvent->eID)
    {
      case orxFX_EVENT_START:
      {
        /* Logs info */
        orxLOG("FX <%s>@<%s> has started!", pstPayload->zFXName, orxObject_GetName(pstObject));

        /* On soldier? */
        if(pstObject == pstSoldier)
        {
          /* Locks it */
          ((MyObject *)orxObject_GetUserData(pstObject))->bLock = orxTRUE;
        }

        break;
      }

      case orxFX_EVENT_STOP:
      {
        /* Logs info */
        orxLOG("FX <%s>@<%s> has stopped!", pstPayload->zFXName, orxObject_GetName(pstObject));

        /* On soldier? */
        if(pstObject == pstSoldier)
        {
          /* Unlocks it */
          ((MyObject *)orxObject_GetUserData(pstObject))->bLock = orxFALSE;
        }

        break;
      }
    }
  }

  /* Done! */
  return orxSTATUS_SUCCESS;
}


/** Update callback
 */
void orxFASTCALL Update(const orxCLOCK_INFO *_pstClockInfo, void *_pstContext)
{
  orxS32 i, s32Count;

  /* *** FX CONTROLS *** */

  /* Pushes main config section */
  orxConfig_PushSection("Main");

  /* For all inputs/FXs */
  for(i = 0, s32Count = orxConfig_GetListCount("FXList");
      i < s32Count;
      i++)
  {
    const orxSTRING zFX;

    /* Gets its name */
    zFX = orxConfig_GetListString("FXList", i);

    /* Is active? */
    if(orxInput_IsActive(zFX))
    {
      /* Selects it */
      zSelectedFX = zFX;
      break;
    }
  }

  /* Pops config section */
  orxConfig_PopSection();

  /* Soldier not locked? */
  if(!((MyObject *)orxObject_GetUserData(pstSoldier))->bLock)
  {
    /* Apply FX? */
    if(orxInput_HasBeenActivated("ApplyFX"))
    {
      /* Plays FX on soldier */
      orxObject_AddFX(pstSoldier, zSelectedFX);
    }
  }
}


/** Inits the tutorial
 */
orxSTATUS orxFASTCALL Init()
{
  orxCLOCK       *pstClock;
  MyObject       *pstMyObject;
  orxINPUT_TYPE   eType;
  orxENUM         eID;
  orxINPUT_MODE   eMode;
  orxS32          i, s32Count;
  const orxSTRING zInputApplyFX;
  orxCHAR         acBuffer[1024], *pc = acBuffer;

  /* Gets input binding names */
  orxInput_GetBinding("ApplyFX", 0, &eType, &eID, &eMode);
  zInputApplyFX = orxInput_GetBindingName(eType, eID, eMode);

  /* Displays a small hint in console */
  pc += orxString_NPrint(pc, (orxU32)(sizeof(acBuffer) - (pc - acBuffer) - 1), "\n- To select a FX:");
  orxConfig_PushSection("Main");
  for(i = 0, s32Count = orxConfig_GetListCount("FXList");
      i < s32Count;
      i++)
  {
    const orxSTRING zInput;
    zInput = orxConfig_GetListString("FXList", i);
    orxInput_GetBinding(zInput, 0, &eType, &eID, &eMode);
    pc += orxString_NPrint(pc, (orxU32)(sizeof(acBuffer) - (pc - acBuffer) - 1), "\n . %-8s => %s", orxInput_GetBindingName(eType, eID, eMode), zInput);
  }
  orxConfig_PopSection();
  pc += orxString_NPrint(pc, (orxU32)(sizeof(acBuffer) - (pc - acBuffer) - 1),
    "\n- %s will apply the current selected FX on the soldier"
    "\n* In this tutorial, only a single concurrent FX will be applied on the soldier"
    "\n* However an object can support up to %u simultaneous FXs"
    "\n* The box has a looping rotating FX applied directly from config upon its creation, requiring no code",
    zInputApplyFX, orxFXPOINTER_KU32_FX_NUMBER);
  orxLOG("%s", acBuffer);

  /* Creates viewport */
  orxViewport_CreateFromConfig("Viewport");

  /* Gets main clock */
  pstClock = orxClock_Get(orxCLOCK_KZ_CORE);

  /* Registers our update callback */
  orxClock_Register(pstClock, Update, orxNULL, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_NORMAL);

  /* Registers event handler */
  orxEvent_AddHandler(orxEVENT_TYPE_FX, EventHandler);
  orxEvent_AddHandler(orxEVENT_TYPE_INPUT, EventHandler);

  /* Creates objects */
  pstSoldier  = orxObject_CreateFromConfig("Soldier");
  pstBox      = orxObject_CreateFromConfig("Box");

  /* Allocates our own object data */
  pstMyObject = orxMemory_Allocate(sizeof(MyObject), orxMEMORY_TYPE_MAIN);

  /* Inits it */
  pstMyObject->bLock = orxFALSE;

  /* Links it to soldier */
  orxObject_SetUserData(pstSoldier, pstMyObject);

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
