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
 * @file orxMouse.c
 * @date 06/01/2009
 * @author iarwain@orx-project.org
 *
 * SDL mouse plugin implementation
 *
 */


#include "orxPluginAPI.h"

#include <SDL.h>


/** Module flags
 */
#define orxMOUSE_KU32_STATIC_FLAG_NONE        0x00000000 /**< No flags */

#define orxMOUSE_KU32_STATIC_FLAG_READY       0x00000001 /**< Ready flag */

#define orxMOUSE_KU32_STATIC_MASK_ALL         0xFFFFFFFF /**< All mask */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxMOUSE_STATIC_t
{
  orxU32      u32Flags;
  orxVECTOR   vMouseMove, vMouseBackup;
  orxFLOAT    fWheelMove;
  orxBOOL     bClearWheel;

} orxMOUSE_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxMOUSE_STATIC sstMouse;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Event handler
 */
static orxSTATUS orxFASTCALL orxMouse_SDL_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Is a mouse move? */
  if((_pstEvent->eType == orxEVENT_TYPE_FIRST_RESERVED + SDL_MOUSEMOTION)
  && (_pstEvent->eID == SDL_MOUSEMOTION))
  {
    SDL_Event *pstEvent;

    /* Gets SDL event */
    pstEvent = (SDL_Event *)(_pstEvent->pstPayload);

    /* Updates mouse move */
    sstMouse.vMouseMove.fX += orxS2F(pstEvent->motion.xrel) - sstMouse.vMouseBackup.fX;
    sstMouse.vMouseMove.fY += orxS2F(pstEvent->motion.yrel) - sstMouse.vMouseBackup.fY;

    /* Stores last mouse position */
    sstMouse.vMouseBackup.fX = orxS2F(pstEvent->motion.xrel);
    sstMouse.vMouseBackup.fY = orxS2F(pstEvent->motion.yrel);
  }

  /* Is a mouse wheel? */
  if((_pstEvent->eType == orxEVENT_TYPE_FIRST_RESERVED + SDL_MOUSEBUTTONDOWN)
  && (_pstEvent->eID == SDL_MOUSEBUTTONDOWN))
  {
    SDL_Event *pstEvent;

    /* Gets SDL event */
    pstEvent = (SDL_Event *)(_pstEvent->pstPayload);

    /* Should clear? */
    if(sstMouse.bClearWheel != orxFALSE)
    {
      /* Clears it */
      sstMouse.fWheelMove = orxFLOAT_0;
      sstMouse.bClearWheel = orxFALSE;
    }

    /* Updates wheel move */
    sstMouse.fWheelMove += (pstEvent->button.button == SDL_BUTTON_WHEELDOWN) ? 1 : -1;
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxMouse_SDL_ShowCursor(orxBOOL _bShow)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY) == orxMOUSE_KU32_STATIC_FLAG_READY);

  /* Shows cursor */
  SDL_ShowCursor(_bShow);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxMouse_SDL_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Was already initialized. */
  if(!(sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstMouse, sizeof(orxMOUSE_STATIC));

    /* Is SDL partly initialized? */
    if(SDL_WasInit(SDL_INIT_EVERYTHING) != 0)
    {
      /* Not already initialized? */
      if(SDL_WasInit(SDL_INIT_VIDEO) == 0)
      {
        /* Inits the mouse subsystem */
        eResult = (SDL_InitSubSystem(SDL_INIT_VIDEO) == 0) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
      }
      else
      {
        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
    }
    else
    {
      /* Inits SDL with video */
      eResult = (SDL_Init(SDL_INIT_VIDEO) == 0) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
    }

    /* Valid? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Registers our mouse event handler */
      if(orxEvent_AddHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + SDL_MOUSEMOTION), orxMouse_SDL_EventHandler) != orxSTATUS_FAILURE)
      {
        /* Registers our mouse wheell event handler */
        if(orxEvent_AddHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + SDL_MOUSEBUTTONDOWN), orxMouse_SDL_EventHandler) != orxSTATUS_FAILURE)
        {
          /* Updates status */
          sstMouse.u32Flags |= orxMOUSE_KU32_STATIC_FLAG_READY;

          /* Sets config section */
          orxConfig_PushSection(orxMOUSE_KZ_CONFIG_SECTION);

          /* Has show cursor value? */
          if(orxConfig_HasValue(orxMOUSE_KZ_CONFIG_SHOW_CURSOR) != orxFALSE)
          {
            /* Updates cursor status */
            orxMouse_SDL_ShowCursor(orxConfig_GetBool(orxMOUSE_KZ_CONFIG_SHOW_CURSOR));
          }

          /* Pops config section */
          orxConfig_PopSection();
        }
        else
        {
          /* Removes event handler */
          orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + SDL_MOUSEMOTION), orxMouse_SDL_EventHandler);

          /* Updates result */
          eResult = orxSTATUS_FAILURE;
        }
      }
    }
  }

  /* Done! */
  return eResult;
}

void orxFASTCALL orxMouse_SDL_Exit()
{
  /* Was initialized? */
  if(sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY)
  {
    /* Unregisters event handlers */
    orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + SDL_MOUSEMOTION), orxMouse_SDL_EventHandler);
    orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + SDL_MOUSEBUTTONDOWN), orxMouse_SDL_EventHandler);

    /* Cleans static controller */
    orxMemory_Zero(&sstMouse, sizeof(orxMOUSE_STATIC));
  }

  return;
}

orxSTATUS orxFASTCALL orxMouse_SDL_SetPosition(const orxVECTOR *_pvPosition)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY) == orxMOUSE_KU32_STATIC_FLAG_READY);

  /* Moves mouse */
  SDL_WarpMouse((orxS16)orxF2S(_pvPosition->fX), (orxS16)orxF2S(_pvPosition->fY));

  /* Done! */
  return eResult;
}

orxVECTOR *orxFASTCALL orxMouse_SDL_GetPosition(orxVECTOR *_pvPosition)
{
  orxS32      s32X, s32Y;
  orxVECTOR  *pvResult = _pvPosition;

  /* Checks */
  orxASSERT((sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY) == orxMOUSE_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvPosition != orxNULL);

  /* Gets mouse position */
  SDL_GetMouseState(&s32X, &s32Y);

  /* Updates result */
  _pvPosition->fX = orxS2F(s32X);
  _pvPosition->fY = orxS2F(s32Y);
  _pvPosition->fZ = orxFLOAT_0;

  /* Done! */
  return pvResult;
}

orxBOOL orxFASTCALL orxMouse_SDL_IsButtonPressed(orxMOUSE_BUTTON _eButton)
{
  orxU8   u8State;
  orxS32  s32X, s32Y;
  orxBOOL bResult;

  /* Checks */
  orxASSERT((sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY) == orxMOUSE_KU32_STATIC_FLAG_READY);
  orxASSERT(_eButton < orxMOUSE_BUTTON_NUMBER);

  /* Gets mouse state */
  u8State = SDL_GetMouseState(&s32X, &s32Y);

  /* Depending on button */
  switch(_eButton)
  {
    case orxMOUSE_BUTTON_LEFT:
    {
      /* Updates result */
      bResult = (u8State & SDL_BUTTON(SDL_BUTTON_LEFT)) ? orxTRUE : orxFALSE;
      break;
    }

    case orxMOUSE_BUTTON_RIGHT:
    {
      /* Updates result */
      bResult = (u8State & SDL_BUTTON(SDL_BUTTON_RIGHT)) ? orxTRUE : orxFALSE;
      break;
    }

    case orxMOUSE_BUTTON_MIDDLE:
    {
      /* Updates result */
      bResult = (u8State & SDL_BUTTON(SDL_BUTTON_MIDDLE)) ? orxTRUE : orxFALSE;
      break;
    }

    case orxMOUSE_BUTTON_EXTRA_1:
    {
      /* Updates result */
      bResult = (u8State & SDL_BUTTON(SDL_BUTTON_X1)) ? orxTRUE : orxFALSE;
      break;
    }

    case orxMOUSE_BUTTON_EXTRA_2:
    {
      /* Updates result */
      bResult = (u8State & SDL_BUTTON(SDL_BUTTON_X2)) ? orxTRUE : orxFALSE;
      break;
    }

    case orxMOUSE_BUTTON_WHEEL_UP:
    {
      /* Updates result */
      bResult = (u8State & SDL_BUTTON(SDL_BUTTON_WHEELUP)) ? orxTRUE : orxFALSE;
      break;
    }

    case orxMOUSE_BUTTON_WHEEL_DOWN:
    {
      /* Updates result */
      bResult = (u8State & SDL_BUTTON(SDL_BUTTON_WHEELDOWN)) ? orxTRUE : orxFALSE;
      break;
    }

    default:
    {
      /* Updates result */
      bResult = orxFALSE;
      break;
    }
  }

  /* Done! */
  return bResult;
}

orxVECTOR *orxFASTCALL orxMouse_SDL_GetMoveDelta(orxVECTOR *_pvMoveDelta)
{
  orxVECTOR *pvResult = _pvMoveDelta;

  /* Checks */
  orxASSERT((sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY) == orxMOUSE_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvMoveDelta != orxNULL);

  /* Updates result */
  orxVector_Copy(_pvMoveDelta, &(sstMouse.vMouseMove));

  /* Clears move */
  orxVector_Copy(&(sstMouse.vMouseMove), &orxVECTOR_0);

  /* Done! */
  return pvResult;
}

orxFLOAT orxFASTCALL orxMouse_SDL_GetWheelDelta()
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT((sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY) == orxMOUSE_KU32_STATIC_FLAG_READY);

  /* Updates result */
  fResult = sstMouse.fWheelMove;

  /* Clears wheel move on next update */
  sstMouse.bClearWheel = orxTRUE;

  /* Done! */
  return fResult;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(MOUSE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_SDL_Init, MOUSE, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_SDL_Exit, MOUSE, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_SDL_SetPosition, MOUSE, SET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_SDL_GetPosition, MOUSE, GET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_SDL_IsButtonPressed, MOUSE, IS_BUTTON_PRESSED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_SDL_GetMoveDelta, MOUSE, GET_MOVE_DELTA);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_SDL_GetWheelDelta, MOUSE, GET_WHEEL_DELTA);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_SDL_ShowCursor, MOUSE, SHOW_CURSOR);
orxPLUGIN_USER_CORE_FUNCTION_END();
