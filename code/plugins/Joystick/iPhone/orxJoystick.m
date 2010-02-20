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
 * @file orxJoystick.m
 * @date 19/02/2010
 * @author iarwain@orx-project.org
 *
 * iPhone joystick plugin implementation
 *
 * @todo
 */


#include "orxPluginAPI.h"


/** Module flags
 */
#define orxJOYSTICK_KU32_STATIC_FLAG_NONE     0x00000000 /**< No flags */

#define orxJOYSTICK_KU32_STATIC_FLAG_READY    0x00000001 /**< Ready flag */

#define orxJOYSTICK_KU32_STATIC_MASK_ALL      0xFFFFFFFF /**< All mask */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxJOYSTICK_STATIC_t
{
  orxU32      u32Flags;
  orxVECTOR   vAcceleration;

} orxJOYSTICK_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxJOYSTICK_STATIC sstJoystick;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

static orxSTATUS orxFASTCALL orxJoystick_iPhone_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;
  
  /* Depending on ID */
  switch(_pstEvent->eID)
  {
    /* Accelerate? */
    case orxIPHONE_EVENT_ACCELERATE:
    {
      orxIPHONE_EVENT_PAYLOAD *pstPayload;

      /* Gets payload */
      pstPayload = (orxIPHONE_EVENT_PAYLOAD *)_pstEvent->pstPayload;

      /* Gets new position */
      orxVector_Set(&(sstJoystick.vAcceleration), orx2F(pstPayload->poAcceleration.x), orx2F(pstPayload->poAcceleration.y), orx2F(pstPayload->poAcceleration.z));

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

orxSTATUS orxFASTCALL orxJoystick_iPhone_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Wasn't already initialized? */
  if(!(sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstJoystick, sizeof(orxJOYSTICK_STATIC));

    /* Adds our joystick event handlers */
    if((eResult = orxEvent_AddHandler(orxEVENT_TYPE_IPHONE, orxJoystick_iPhone_EventHandler)) != orxSTATUS_FAILURE)
    {
      /* Updates status */
      sstJoystick.u32Flags |= orxJOYSTICK_KU32_STATIC_FLAG_READY;
    }
  }

  /* Done! */
  return eResult;
}

void orxFASTCALL orxJoystick_iPhone_Exit()
{
  /* Was initialized? */
  if(sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY)
  {
    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_IPHONE, orxJoystick_iPhone_EventHandler);
    
    /* Cleans static controller */
    orxMemory_Zero(&sstJoystick, sizeof(orxJOYSTICK_STATIC));
  }
  
  return;
}

orxFLOAT orxFASTCALL orxJoystick_iPhone_GetAxisValue(orxU32 _u32ID, orxJOYSTICK_AXIS _eAxis)
{
  orxFLOAT fResult = orxFLOAT_0;

  /* Depending on axis */
  switch(_eAxis)
  {
    case orxJOYSTICK_AXIS_X:
    {
      /* Updates result */
      fResult = sstJoystick.vAcceleration.fX;

      break;
    }

    case orxJOYSTICK_AXIS_Y:
    {
      /* Updates result */
      fResult = sstJoystick.vAcceleration.fY;

      break;
    }

    case orxJOYSTICK_AXIS_Z:
    {
      /* Updates result */
      fResult = sstJoystick.vAcceleration.fZ;

      break;
    }

    default:
    {
      /* Not available */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_JOYSTICK, "<%s> is not available on this platform!", orxJoystick_GetAxisName(_eAxis));

      break;
    }
  }

  /* Done! */
  return fResult;
}

orxBOOL orxFASTCALL orxJoystick_iPhone_IsButtonPressed(orxU32 _u32ID, orxJOYSTICK_BUTTON _eButton)
{
  orxBOOL bResult = orxFALSE;

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_JOYSTICK, "Not available on this platform!");

  /* Done! */
  return bResult;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(JOYSTICK);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_iPhone_Init, JOYSTICK, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_iPhone_Exit, JOYSTICK, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_iPhone_GetAxisValue, JOYSTICK, GET_AXIS_VALUE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_iPhone_IsButtonPressed, JOYSTICK, IS_BUTTON_PRESSED);
orxPLUGIN_USER_CORE_FUNCTION_END();
