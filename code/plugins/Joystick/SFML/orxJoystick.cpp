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

/** Event handler
 */
static orxSTATUS orxFASTCALL orxJoystick_SFML_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Is a button pressed or released? */
  if(((_pstEvent->eType == orxEVENT_TYPE_FIRST_RESERVED + sf::Event::JoyButtonPressed)
   && (_pstEvent->eID == sf::Event::JoyButtonPressed))
  || ((_pstEvent->eType == orxEVENT_TYPE_FIRST_RESERVED + sf::Event::JoyButtonReleased)
   && (_pstEvent->eID == sf::Event::JoyButtonReleased)))
  {
    orxJOYSTICK_EVENT_PAYLOAD stPayload;
    sf::Event                *poEvent;

    /* Gets SFML event */
    poEvent = (sf::Event *)(_pstEvent->pstPayload);

    /* Inits payload */
    stPayload.u32JoystickID = poEvent->JoyButton.JoystickId;
    stPayload.eButton       = (orxJOYSTICK_BUTTON)poEvent->JoyButton.Button;

    /* Sends event */
    orxEVENT_SEND(orxEVENT_TYPE_JOYSTICK, (_pstEvent->eID == sf::Event::JoyButtonPressed) ? orxJOYSTICK_EVENT_BUTTON_PRESSED : orxJOYSTICK_EVENT_BUTTON_RELEASED, orxNULL, orxNULL, &stPayload);
  }

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxJoystick_SFML_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Wasn't already initialized? */
  if(!(sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstJoystick, sizeof(orxJOYSTICK_STATIC));

    /* Registers our keyboard event handlers */
    if((orxEvent_AddHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::JoyButtonPressed), orxJoystick_SFML_EventHandler) != orxSTATUS_FAILURE)
    && (orxEvent_AddHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::JoyButtonReleased), orxJoystick_SFML_EventHandler) != orxSTATUS_FAILURE))
    {
      orxEVENT stEvent;

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
    else
    {
      /* Removes event handlers */
      orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::JoyButtonPressed), orxJoystick_SFML_EventHandler);
      orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::JoyButtonReleased), orxJoystick_SFML_EventHandler);
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
    /* Removes event handlers */
    orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::JoyButtonPressed), orxJoystick_SFML_EventHandler);
    orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::JoyButtonReleased), orxJoystick_SFML_EventHandler);

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
