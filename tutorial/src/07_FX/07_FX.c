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
 * @file 07_FX.c
 * @date 13/08/2008
 * @author iarwain@orx-project.org
 *
 * FX tutorial
 */


#include "orxPluginAPI.h"


/* This is a basic C tutorial creating visual FXs.
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
 * See previous tutorials for more info about the basic object creation, clock, animation, viewport and sound handling.
 *
 * This tutorial shows what FXs are and how to create them.
 *
 * FXs are based on a combination of curves based on sine, triangle, square or linear shape,
 * applied on different parameters such as scale, rotation, position, speed, alpha and color.
 *
 * FXs are set through config file requiring only one line of code to apply them on an object.
 * There can be up to 8 curves of any type combined to form a single FX. Up to 4 FXs can be applied
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
orxSTRING zSelectedFX = "WobbleFX";


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

      /* Has a multi-input info? */
      if(pstPayload->aeType[1] != orxINPUT_TYPE_NONE)
      {
        /* Logs info */
        orxLOG("[%s] triggered by '%s' + '%s'.", pstPayload->zInputName, orxInput_GetBindingName(pstPayload->aeType[0], pstPayload->aeID[0]), orxInput_GetBindingName(pstPayload->aeType[1], pstPayload->aeID[1]));
      }
      else
      {
        /* Logs info */
        orxLOG("[%s] triggered by '%s'.", pstPayload->zInputName, orxInput_GetBindingName(pstPayload->aeType[0], pstPayload->aeID[0]));
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
        orxLOG("FX <%s>@<%s> has stoped!", pstPayload->zFXName, orxObject_GetName(pstObject));

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
  /* *** FX CONTROLS *** */

  /* Select multiFX? */
  if(orxInput_IsActive("SelectMultiFX"))
  {
    /* Selects wobble FX */
    zSelectedFX = "MultiFX";
  }
  /* Select wobble? */
  if(orxInput_IsActive("SelectWobble"))
  {
    /* Selects wobble FX */
    zSelectedFX = "WobbleFX";
  }
  /* Select circle? */
  if(orxInput_IsActive("SelectCircle"))
  {
    /* Selects circle FX */
    zSelectedFX = "CircleFX";
  }
  /* Select fade? */
  if(orxInput_IsActive("SelectFade"))
  {
    /* Selects fade FX */
    zSelectedFX = "FadeFX";
  }
  /* Select flash? */
  if(orxInput_IsActive("SelectFlash"))
  {
    /* Selects flash FX */
    zSelectedFX = "FlashFX";
  }
  /* Select move? */
  if(orxInput_IsActive("SelectMove"))
  {
    /* Selects move FX */
    zSelectedFX = "MoveFX";
  }
  /* Select flip? */
  if(orxInput_IsActive("SelectFlip"))
  {
    /* Selects flip FX */
    zSelectedFX = "FlipFX";
  }

  /* Soldier not locked? */
  if(!((MyObject *)orxObject_GetUserData(pstSoldier))->bLock)
  {
    /* Apply FX? */
    if(orxInput_IsActive("ApplyFX") && orxInput_HasNewStatus("ApplyFX"))
    {
      /* Plays FX on soldier */
      orxObject_AddFX(pstSoldier, zSelectedFX);
    }
  }
}


/** Inits the tutorial
 */
orxSTATUS Init()
{
  orxCLOCK       *pstClock;
  MyObject       *pstMyObject;
  orxINPUT_TYPE   eType;
  orxENUM         eID;
  const orxSTRING zInputSelectWobble;
  const orxSTRING zInputSelectCircle;
  const orxSTRING zInputSelectFade;
  const orxSTRING zInputSelectFlash;
  const orxSTRING zInputSelectMove;
  const orxSTRING zInputSelectFlip;
  const orxSTRING zInputSelectMultiFX;
  const orxSTRING zInputApplyFX;

  /* Loads config file */
  orxConfig_Load("../07_FX.ini");

  /* Reloads inputs */
  orxInput_Load(orxSTRING_EMPTY);

  /* Gets input binding names */
  orxInput_GetBinding("SelectWobble", 0, &eType, &eID);
  zInputSelectWobble  = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("SelectCircle", 0, &eType, &eID);
  zInputSelectCircle  = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("SelectFade", 0, &eType, &eID);
  zInputSelectFade    = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("SelectFlash", 0, &eType, &eID);
  zInputSelectFlash   = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("SelectMove", 0, &eType, &eID);
  zInputSelectMove    = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("SelectFlip", 0, &eType, &eID);
  zInputSelectFlip    = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("SelectMultiFX", 0, &eType, &eID);
  zInputSelectMultiFX = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("ApplyFX", 0, &eType, &eID);
  zInputApplyFX       = orxInput_GetBindingName(eType, eID);

  /* Displays a small hint in console */
  orxLOG("\n- To select the FX to apply:"
         "\n . '%s' => Wobble"
         "\n . '%s' => Circle"
         "\n . '%s' => Fade"
         "\n . '%s' => Flash"
         "\n . '%s' => Move"
         "\n . '%s' => Flip"
         "\n . '%s' => MultiFX that contains the slots of 4 of the above FXs"
         "\n- '%s' will apply the current selected FX on soldier"
         "\n* Only once FX will be applied at a time in this tutorial"
         "\n* However an object can support up to 4 FXs at the same time"
         "\n* Box has a looping rotating FX applied directly from config, requiring no code",
         zInputSelectWobble, zInputSelectCircle, zInputSelectFade, zInputSelectFlash, zInputSelectMove, zInputSelectFlip, zInputSelectMultiFX, zInputApplyFX);

  /* Creates viewport */
  orxViewport_CreateFromConfig("Viewport");

  /* Gets main clock */
  pstClock = orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE);

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

/* Registers plugin entry */
orxPLUGIN_DECLARE_ENTRY_POINT(Init);
