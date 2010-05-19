/* Orx - Portable Game Engine
 *
 * Copyright (c) 2010 Orx-Project
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
 * @file orxJoystick.cpp
 * @date 08/07/2008
 * @author iarwain@orx-project.org
 *
 * SFML joystick plugin implementation
 *
 */


#include "orxPluginAPI.h"

#include <SFML/Graphics.hpp>


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
  orxU32            u32Flags;
  sf::Input        *poInput;

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

extern "C" orxSTATUS orxFASTCALL orxJoystick_SFML_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Wasn't already initialized? */
  if(!(sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY))
  {
    orxEVENT stEvent;

    /* Cleans static controller */
    orxMemory_Zero(&sstJoystick, sizeof(orxJOYSTICK_STATIC));

    /* Inits event for getting SFML input */
    orxEVENT_INIT(stEvent, orxEVENT_TYPE_FIRST_RESERVED, orxEVENT_TYPE_FIRST_RESERVED, orxNULL, orxNULL, &(sstJoystick.poInput));

    /* Sends it */
    orxEvent_Send(&stEvent);

    /* Valid? */
    if(sstJoystick.poInput != orxNULL)
    {
      /* Updates status */
      sstJoystick.u32Flags |= orxJOYSTICK_KU32_STATIC_FLAG_READY;

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
  }

  /* Done! */
  return eResult;
}

extern "C" void orxFASTCALL orxJoystick_SFML_Exit()
{
  /* Was initialized? */
  if(sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY)
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstJoystick, sizeof(orxJOYSTICK_STATIC));
  }

  return;
}

extern "C" orxFLOAT orxFASTCALL orxJoystick_SFML_GetAxisValue(orxJOYSTICK_AXIS _eAxis)
{
  sf::Joy::Axis eSFMLAxis;
  orxFLOAT      fResult;
  orxU32        u32ID;

  /* Checks */
  orxASSERT((sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY) == orxJOYSTICK_KU32_STATIC_FLAG_READY);
  orxASSERT(_eAxis < orxJOYSTICK_AXIS_NUMBER);

  /* Gets ID */
  u32ID = (orxU32)_eAxis / orxJOYSTICK_AXIS_SINGLE_NUMBER;

  /* Depending on axis */
  switch(_eAxis)
  {
    case orxJOYSTICK_AXIS_X_1:
    case orxJOYSTICK_AXIS_X_2:
    case orxJOYSTICK_AXIS_X_3:
    case orxJOYSTICK_AXIS_X_4:
    {
      /* Gets SFML axis */
      eSFMLAxis = sf::Joy::AxisX;
      break;
    }

    case orxJOYSTICK_AXIS_Y_1:
    case orxJOYSTICK_AXIS_Y_2:
    case orxJOYSTICK_AXIS_Y_3:
    case orxJOYSTICK_AXIS_Y_4:
    {
      /* Gets SFML axis */
      eSFMLAxis = sf::Joy::AxisY;
      break;
    }

    case orxJOYSTICK_AXIS_Z_1:
    case orxJOYSTICK_AXIS_Z_2:
    case orxJOYSTICK_AXIS_Z_3:
    case orxJOYSTICK_AXIS_Z_4:
    {
      /* Gets SFML axis */
      eSFMLAxis = sf::Joy::AxisZ;
      break;
    }

    case orxJOYSTICK_AXIS_R_1:
    case orxJOYSTICK_AXIS_R_2:
    case orxJOYSTICK_AXIS_R_3:
    case orxJOYSTICK_AXIS_R_4:
    {
      /* Gets SFML axis */
      eSFMLAxis = sf::Joy::AxisR;
      break;
    }

    case orxJOYSTICK_AXIS_U_1:
    case orxJOYSTICK_AXIS_U_2:
    case orxJOYSTICK_AXIS_U_3:
    case orxJOYSTICK_AXIS_U_4:
    {
      /* Gets SFML axis */
      eSFMLAxis = sf::Joy::AxisU;
      break;
    }

    case orxJOYSTICK_AXIS_V_1:
    case orxJOYSTICK_AXIS_V_2:
    case orxJOYSTICK_AXIS_V_3:
    case orxJOYSTICK_AXIS_V_4:
    {
      /* Gets SFML axis */
      eSFMLAxis = sf::Joy::AxisV;
      break;
    }

    case orxJOYSTICK_AXIS_POV_1:
    case orxJOYSTICK_AXIS_POV_2:
    case orxJOYSTICK_AXIS_POV_3:
    case orxJOYSTICK_AXIS_POV_4:
    {
      /* Gets SFML axis */
      eSFMLAxis = sf::Joy::AxisPOV;
      break;
    }

    default:
    {
      /* Updates result */
      eSFMLAxis = sf::Joy::Count;
      break;
    }
  }

  /* Valid? */
  if(eSFMLAxis < sf::Joy::Count)
  {
    /* Updates result */
    fResult = sstJoystick.poInput->GetJoystickAxis(u32ID, eSFMLAxis);
  }
  else
  {
    /* Updates result */
    fResult = orxFLOAT_0;
  }

  /* Done! */
  return fResult;
}

extern "C" orxBOOL orxFASTCALL orxJoystick_SFML_IsButtonPressed(orxJOYSTICK_BUTTON _eButton)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT((sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY) == orxJOYSTICK_KU32_STATIC_FLAG_READY);
  orxASSERT(_eButton < orxJOYSTICK_BUTTON_NUMBER);

  /* Updates result */
  bResult = sstJoystick.poInput->IsJoystickButtonDown((orxU32)_eButton / orxJOYSTICK_BUTTON_SINGLE_NUMBER, (orxU32)_eButton % orxJOYSTICK_BUTTON_SINGLE_NUMBER) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(JOYSTICK);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_SFML_Init, JOYSTICK, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_SFML_Exit, JOYSTICK, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_SFML_GetAxisValue, JOYSTICK, GET_AXIS_VALUE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_SFML_IsButtonPressed, JOYSTICK, IS_BUTTON_PRESSED);
orxPLUGIN_USER_CORE_FUNCTION_END();
