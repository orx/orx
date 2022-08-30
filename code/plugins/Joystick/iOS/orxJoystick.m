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
 * @file orxJoystick.m
 * @date 19/02/2010
 * @author iarwain@orx-project.org
 *
 * iOS joystick plugin implementation
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

static orxSTATUS orxFASTCALL orxJoystick_iOS_EventHandler(const orxEVENT *_pstEvent)
{
  orxSYSTEM_EVENT_PAYLOAD  *pstPayload;
  orxSTATUS                 eResult = orxSTATUS_SUCCESS;

  /* Accelerate event? */
  if(_pstEvent->eID == orxSYSTEM_EVENT_ACCELERATE)
  {
    /* Gets payload */
    pstPayload = (orxSYSTEM_EVENT_PAYLOAD *)_pstEvent->pstPayload;

    /* Gets new acceleration */
    orxVector_Copy(&(sstJoystick.vAcceleration), &(pstPayload->stAccelerometer.vAcceleration));
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxJoystick_iOS_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Wasn't already initialized? */
  if(!(sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstJoystick, sizeof(orxJOYSTICK_STATIC));

    /* Adds our joystick event handlers */
    if((eResult = orxEvent_AddHandler(orxEVENT_TYPE_SYSTEM, orxJoystick_iOS_EventHandler)) != orxSTATUS_FAILURE)
    {
      /* Filters relevant event IDs */
      orxEvent_SetHandlerIDFlags(orxJoystick_iOS_EventHandler, orxEVENT_TYPE_SYSTEM, orxNULL, orxEVENT_GET_FLAG(orxSYSTEM_EVENT_ACCELERATE), orxEVENT_KU32_MASK_ID_ALL);

      /* Updates status */
      sstJoystick.u32Flags |= orxJOYSTICK_KU32_STATIC_FLAG_READY;
    }
  }

  /* Done! */
  return eResult;
}

void orxFASTCALL orxJoystick_iOS_Exit()
{
  /* Was initialized? */
  if(sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY)
  {
    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_SYSTEM, orxJoystick_iOS_EventHandler);

    /* Cleans static controller */
    orxMemory_Zero(&sstJoystick, sizeof(orxJOYSTICK_STATIC));
  }

  return;
}

orxFLOAT orxFASTCALL orxJoystick_iOS_GetAxisValue(orxJOYSTICK_AXIS _eAxis)
{
  orxFLOAT fResult = orxFLOAT_0;

  /* Depending on axis */
  switch(_eAxis)
  {
    case orxJOYSTICK_AXIS_LX_1:
    case orxJOYSTICK_AXIS_LX_2:
    case orxJOYSTICK_AXIS_LX_3:
    case orxJOYSTICK_AXIS_LX_4:
    {
      /* Updates result */
      fResult = sstJoystick.vAcceleration.fX;

      break;
    }

    case orxJOYSTICK_AXIS_LY_1:
    case orxJOYSTICK_AXIS_LY_2:
    case orxJOYSTICK_AXIS_LY_3:
    case orxJOYSTICK_AXIS_LY_4:
    {
      /* Updates result */
      fResult = sstJoystick.vAcceleration.fY;

      break;
    }

    case orxJOYSTICK_AXIS_RX_1:
    case orxJOYSTICK_AXIS_RX_2:
    case orxJOYSTICK_AXIS_RX_3:
    case orxJOYSTICK_AXIS_RX_4:
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

orxBOOL orxFASTCALL orxJoystick_iOS_IsButtonPressed(orxJOYSTICK_BUTTON _eButton)
{
  orxBOOL bResult = orxFALSE;

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_JOYSTICK, "Not available on this platform!");

  /* Done! */
  return bResult;
}

orxBOOL orxFASTCALL orxJoystick_iOS_IsConnected(orxU32 _u32ID)
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
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_iOS_Init, JOYSTICK, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_iOS_Exit, JOYSTICK, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_iOS_GetAxisValue, JOYSTICK, GET_AXIS_VALUE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_iOS_IsButtonPressed, JOYSTICK, IS_BUTTON_PRESSED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_iOS_IsConnected, JOYSTICK, IS_CONNECTED);
orxPLUGIN_USER_CORE_FUNCTION_END();
