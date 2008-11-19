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
 * @file 07_FX.c
 * @date 13/08/2008
 * @author iarwain@orx-project.org
 *
 * FX tutorial
 */


#include "orx.h"


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
 * - Backspace to reload all configuration files (provided that config history is turned on)
 * It also exits if the orxSYSTEM_EVENT_CLOSE signal is sent.
 *
 * See previous tutorials for more info about the basic object creation, clock, animation, viewport and sound handling.
 *
 * This tutorial shows what FXs are and how to create them.
 *
 * FXs are based on a combination of curves based on sine, saw or linear shape, applied on different parameters
 * suchs as scale, rotation, alpha, color and position.
 *
 * FXs are set through config file requiring only one line of code to apply them on an object.
 * There can be up to 5 curves of any type combined to form an FX. Up to 4 FXs can be applied
 * on the same object at a given time.
 *
 * FX can set absolute or relative values, depending on the attribute Absolute in its config.
 * Control over curve period, phasis, pow, amplification over time is also granted.
 * For rotation FXs, value can use object's orientation and/or scale on demand.
 * This allows the creation of pretty elaborated and great looking visual FXs.
 *
 * FX parameters can be tweaked in the config file and reloaded on-the-fly using backspace,
 * unless the FX was specified to be kept in cache memory (with the attribute KeepInCache).
 *
 * For example you won't be able to tweak on the fly the circle FX as it's defined
 * with this attribute in the default config file. All the other FXs can be updated while
 * the tutorial run, at their next application.
 *
 * As always, random parameters can be used from config allowing some variety for a single FX.
 * For example, the wobble scale, the flash color and the "attack" of the move FX are using limited random values.
 *
 * We also register to the FX events to display when FXs are played and stopped.
 * As the FX played on the box object is tagget as looping, it'll never stop and thus
 * the corresponding event (FX_STOP) won't ever be sent.
 *
 * We also show briefly how to add some personal user data to an orxOBJECT (here a structure containing a single boolean).
 * We retrieve it in the event callback to lock the object when an FX starts and unlock it when it stops.
 * We use this lock to allow only one FX at a time on the soldier. It's only written here as a didactic purpose.
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
orxSTATUS orxFASTCALL EventHandler(orxCONST orxEVENT *_pstEvent)
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

    case orxSOUND_EVENT_STOP:
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

  /* Done! */
  return orxSTATUS_SUCCESS;
}


/** Update callback
 */
orxVOID orxFASTCALL Update(orxCONST orxCLOCK_INFO *_pstClockInfo, orxVOID *_pstContext)
{
  /* *** FX CONTROLS *** */

  /* Is numpad 0 pressed? */
  if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_NUMPAD0))
  {
    /* Selects wobble FX */
    zSelectedFX = "MultiFX";
  }
  /* Is numpad 1 pressed? */
  if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_NUMPAD1))
  {
    /* Selects wobble FX */
    zSelectedFX = "WobbleFX";
  }
  /* Is numpad 2 pressed? */
  if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_NUMPAD2))
  {
    /* Selects circle FX */
    zSelectedFX = "CircleFX";
  }
  /* Is numpad 3 pressed? */
  if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_NUMPAD3))
  {
    /* Selects fade FX */
    zSelectedFX = "FadeFX";
  }
  /* Is numpad 4 pressed? */
  if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_NUMPAD4))
  {
    /* Selects flash FX */
    zSelectedFX = "FlashFX";
  }
  /* Is numpad 5 pressed? */
  if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_NUMPAD5))
  {
    /* Selects move FX */
    zSelectedFX = "MoveFX";
  }
  /* Is numpad 6 pressed? */
  if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_NUMPAD6))
  {
    /* Selects flip FX */
    zSelectedFX = "FlipFX";
  }

  /* Soldier not locked? */
  if(!((MyObject *)orxObject_GetUserData(pstSoldier))->bLock)
  {
    /* Is space pressed? */
    if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_SPACE))
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
  orxCLOCK *pstClock;
  MyObject *pstMyObject;

  /* Displays a small hint in console */
  orxLOG("\n- Numpad 1-6 selects an FX to apply (Wobble, Circle, Fade, Color, Move & Flip)"
         "\n- Numpad 0 will apply the MultiFX containing the slots of 4 of the above FXs"
         "\n- Space will apply the current selected FX on soldier"
         "\n* Only once FX will be applied at a time in this tutorial"
         "\n* However an object can support up to 4 FXs at the same time"
         "\n* Box has a looping rotating FX applied directly from config, requiring no code");

  /* Loads config file and selects main section */
  orxConfig_Load("../07_FX.ini");

  /* Creates viewport */
  orxViewport_CreateFromConfig("Viewport");

  /* Creates a 100 Hz clock */
  pstClock = orxClock_Create(orx2F(0.01f), orxCLOCK_TYPE_USER);

  /* Registers our update callback */
  orxClock_Register(pstClock, Update, orxNULL, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_NORMAL);

  /* Registers event handler */
  orxEvent_AddHandler(orxEVENT_TYPE_FX, EventHandler);

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
