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
 * @file orxMouse.m
 * @date 31/01/2010
 * @author iarwain@orx-project.org
 *
 * iPhone mouse plugin implementation
 *
 */


#include "orxPluginAPI.h"


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
  orxBOOL     bIsClicked;
  orxVECTOR   vMouseMove, vMousePosition;

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

static orxSTATUS orxFASTCALL orxMouse_iPhone_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Depending on ID */
  switch(_pstEvent->eID)
  {
    /* Touch? */
    case orxIPHONE_EVENT_TOUCH_BEGIN:
    case orxIPHONE_EVENT_TOUCH_MOVE:
    case orxIPHONE_EVENT_TOUCH_END:
    case orxIPHONE_EVENT_TOUCH_CANCEL:
    {
      UITouch                  *poTouch;
      orxIPHONE_EVENT_PAYLOAD  *pstPayload;
      orxBOOL                   bActive = orxFALSE;
      orxVECTOR                 vNewPosition;
      CGPoint                   vViewPosition;

      /* Gets payload */
      pstPayload = (orxIPHONE_EVENT_PAYLOAD *)_pstEvent->pstPayload;

      /* Gets first touch */
      poTouch = [[pstPayload->poUIEvent allTouches] anyObject];

      /* Gets its position inside view */
      vViewPosition = [poTouch locationInView:(orxView *)_pstEvent->hSender];

      /* Gets new position */
      orxVector_Set(&vNewPosition, orx2F(vViewPosition.x), orx2F(vViewPosition.y), orxFLOAT_0);

      /* Updates mouse move */
      orxVector_Sub(&(sstMouse.vMouseMove), &(sstMouse.vMouseMove), &(sstMouse.vMousePosition));
      orxVector_Add(&(sstMouse.vMouseMove), &(sstMouse.vMouseMove), &vNewPosition);

      /* Updates mouse position */
      orxVector_Copy(&(sstMouse.vMousePosition), &vNewPosition);

      /* For all touches */
      for(poTouch in [pstPayload->poUIEvent allTouches])
      {
        /* Not a touch end? */
        if([poTouch phase] != UITouchPhaseEnded)
        {
          /* Updates status */
          bActive = orxTRUE;
          break;
        }
      }

      /* Updates click status */
      sstMouse.bIsClicked = bActive;

      break;
    }

    default:
    {
      break;
    }
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxMouse_iPhone_ShowCursor(orxBOOL _bShow)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY) == orxMOUSE_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_MOUSE, "Not available on this platform!");

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxMouse_iPhone_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Was already initialized. */
  if(!(sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstMouse, sizeof(orxMOUSE_STATIC));

    /* Adds our mouse event handlers */
    if((eResult = orxEvent_AddHandler(orxEVENT_TYPE_IPHONE, orxMouse_iPhone_EventHandler)) != orxSTATUS_FAILURE)
    {
      /* Updates status */
      sstMouse.u32Flags |= orxMOUSE_KU32_STATIC_FLAG_READY;

      /* Sets config section */
      orxConfig_PushSection(orxMOUSE_KZ_CONFIG_SECTION);

      /* Has show cursor value? */
      if(orxConfig_HasValue(orxMOUSE_KZ_CONFIG_SHOW_CURSOR) != orxFALSE)
      {
        /* Updates cursor status */
        orxMouse_iPhone_ShowCursor(orxConfig_GetBool(orxMOUSE_KZ_CONFIG_SHOW_CURSOR));
      }
    }

    /* Pops config section */
    orxConfig_PopSection();
  }

  /* Done! */
  return eResult;
}

void orxFASTCALL orxMouse_iPhone_Exit()
{
  /* Was initialized? */
  if(sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY)
  {
    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_IPHONE, orxMouse_iPhone_EventHandler);

    /* Cleans static controller */
    orxMemory_Zero(&sstMouse, sizeof(orxMOUSE_STATIC));
  }

  return;
}

orxSTATUS orxFASTCALL orxMouse_iPhone_SetPosition(const orxVECTOR *_pvPosition)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY) == orxMOUSE_KU32_STATIC_FLAG_READY);

  /* Updates mouse move */
  orxVector_Sub(&(sstMouse.vMouseMove), &(sstMouse.vMouseMove), &(sstMouse.vMousePosition));
  orxVector_Add(&(sstMouse.vMouseMove), &(sstMouse.vMouseMove), _pvPosition);

  /* Updates position */
  orxVector_Set(&(sstMouse.vMousePosition), _pvPosition->fX, _pvPosition->fY, orxFLOAT_0);

  /* Done! */
  return eResult;
}

orxVECTOR *orxFASTCALL orxMouse_iPhone_GetPosition(orxVECTOR *_pvPosition)
{
  orxVECTOR *pvResult = _pvPosition;

  /* Checks */
  orxASSERT((sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY) == orxMOUSE_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvPosition != orxNULL);

  /* Updates result */
  orxVector_Copy(_pvPosition, &(sstMouse.vMousePosition));

  /* Done! */
  return pvResult;
}

orxBOOL orxFASTCALL orxMouse_iPhone_IsButtonPressed(orxMOUSE_BUTTON _eButton)
{
  orxBOOL bResult = orxFALSE;

  /* Checks */
  orxASSERT((sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY) == orxMOUSE_KU32_STATIC_FLAG_READY);
  orxASSERT(_eButton < orxMOUSE_BUTTON_NUMBER);

  /* Left button? */
  if(_eButton == orxMOUSE_BUTTON_LEFT)
  {
    /* Updates result */
    bResult = sstMouse.bIsClicked;
  }

  /* Done! */
  return bResult;
}

orxVECTOR *orxFASTCALL orxMouse_iPhone_GetMoveDelta(orxVECTOR *_pvMoveDelta)
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

orxFLOAT orxFASTCALL orxMouse_iPhone_GetWheelDelta()
{
  orxFLOAT fResult = orxFLOAT_0;

  /* Checks */
  orxASSERT((sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY) == orxMOUSE_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_MOUSE, "Not available on this platform!");

  /* Done! */
  return fResult;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(MOUSE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_iPhone_Init, MOUSE, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_iPhone_Exit, MOUSE, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_iPhone_SetPosition, MOUSE, SET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_iPhone_GetPosition, MOUSE, GET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_iPhone_IsButtonPressed, MOUSE, IS_BUTTON_PRESSED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_iPhone_GetMoveDelta, MOUSE, GET_MOVE_DELTA);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_iPhone_GetWheelDelta, MOUSE, GET_WHEEL_DELTA);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_iPhone_ShowCursor, MOUSE, SHOW_CURSOR);
orxPLUGIN_USER_CORE_FUNCTION_END();
