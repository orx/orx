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
 * @file orxMouse.cpp
 * @date 26/11/2007
 * @author iarwain@orx-project.org
 *
 * SFML mouse plugin implementation
 *
 */


#include "orxPluginAPI.h"

#include <SFML/Graphics.hpp>


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
  sf::Input  *poInput;

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
static orxSTATUS orxFASTCALL orxMouse_SFML_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Is a mouse move? */
  if((_pstEvent->eType == orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseMoved)
  && (_pstEvent->eID == sf::Event::MouseMoved))
  {
    sf::Event *poEvent;

    /* Gets SFML event */
    poEvent = (sf::Event *)(_pstEvent->pstPayload);

    /* Updates mouse move */
    sstMouse.vMouseMove.fX += orxS2F(poEvent->MouseMove.X) - sstMouse.vMouseBackup.fX;
    sstMouse.vMouseMove.fY += orxS2F(poEvent->MouseMove.Y) - sstMouse.vMouseBackup.fY;

    /* Stores last mouse position */
    sstMouse.vMouseBackup.fX = orxS2F(poEvent->MouseMove.X);
    sstMouse.vMouseBackup.fY = orxS2F(poEvent->MouseMove.Y);
  }
  /* Is a mouse wheel? */
  else if((_pstEvent->eType == orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseWheelMoved)
       && (_pstEvent->eID == sf::Event::MouseWheelMoved))
  {
    sf::Event *poEvent;

    /* Gets SFML event */
    poEvent = (sf::Event *)(_pstEvent->pstPayload);

    /* Should clear wheel? */
    if(sstMouse.bClearWheel != orxFALSE)
    {
      /* Clears it */
      sstMouse.fWheelMove   = orxFLOAT_0;
      sstMouse.bClearWheel  = orxFALSE;
    }

    /* Updates wheel move */
    sstMouse.fWheelMove += orxS2F(poEvent->MouseWheel.Delta);

    /* Has delta? */
    if(poEvent->MouseWheel.Delta != 0)
    {
      orxMOUSE_EVENT_PAYLOAD stPayload;

      /* Inits payload */
      stPayload.eButton = (poEvent->MouseWheel.Delta > 0) ? orxMOUSE_BUTTON_WHEEL_UP : orxMOUSE_BUTTON_WHEEL_DOWN;

      /* Sends event */
      orxEVENT_SEND(orxEVENT_TYPE_MOUSE, orxMOUSE_EVENT_BUTTON_PRESSED, orxNULL, orxNULL, &stPayload);
    }
  }
  /* Is a mouse button pressed or released? */
  else if(((_pstEvent->eType == orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseButtonPressed)
        && (_pstEvent->eID == sf::Event::MouseButtonPressed))
       || ((_pstEvent->eType == orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseButtonReleased)
        && (_pstEvent->eID == sf::Event::MouseButtonReleased)))
  {
    orxMOUSE_EVENT_PAYLOAD  stPayload;
    sf::Event              *poEvent;

    /* Gets SFML event */
    poEvent = (sf::Event *)(_pstEvent->pstPayload);

    /* Depending on button */
    switch(poEvent->MouseButton.Button)
    {
      default:
      case sf::Mouse::Left:
      {
        /* Inits payload */
        stPayload.eButton = orxMOUSE_BUTTON_LEFT;
        break;
      }
      case sf::Mouse::Middle:
      {
        /* Inits payload */
        stPayload.eButton = orxMOUSE_BUTTON_MIDDLE;
        break;
      }
      case sf::Mouse::Right:
      {
        /* Inits payload */
        stPayload.eButton = orxMOUSE_BUTTON_RIGHT;
        break;
      }
      case sf::Mouse::XButton1:
      {
        /* Inits payload */
        stPayload.eButton = orxMOUSE_BUTTON_EXTRA_1;
        break;
      }
      case sf::Mouse::XButton2:
      {
        /* Inits payload */
        stPayload.eButton = orxMOUSE_BUTTON_EXTRA_2;
        break;
      }
    }

    /* Sends event */
    orxEVENT_SEND(orxEVENT_TYPE_MOUSE, (_pstEvent->eID == sf::Event::MouseButtonPressed) ? orxMOUSE_EVENT_BUTTON_PRESSED : orxMOUSE_EVENT_BUTTON_RELEASED, orxNULL, orxNULL, &stPayload);
  }

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxMouse_SFML_ShowCursor(orxBOOL _bShow)
{
  orxEVENT  stEvent;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY) == orxMOUSE_KU32_STATIC_FLAG_READY);

  /* Inits event */
  orxEVENT_INIT(stEvent, orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseButtonPressed, orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseButtonPressed, orxNULL, orxNULL, &_bShow);

  /* Sends system close event */
  eResult = orxEvent_Send(&stEvent);

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxMouse_SFML_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Wasn't already initialized? */
  if(!(sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstMouse, sizeof(orxMOUSE_STATIC));

    /* Registers our mouse event handlers */
    if((orxEvent_AddHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseMoved), orxMouse_SFML_EventHandler) != orxSTATUS_FAILURE)
    && (orxEvent_AddHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseWheelMoved), orxMouse_SFML_EventHandler) != orxSTATUS_FAILURE)
    && (orxEvent_AddHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseButtonPressed), orxMouse_SFML_EventHandler) != orxSTATUS_FAILURE)
    && (orxEvent_AddHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseButtonReleased), orxMouse_SFML_EventHandler) != orxSTATUS_FAILURE))
    {
      /* Terrible hack : gets application input from display SFML plugin */
      sstMouse.poInput = (sf::Input *)orxDisplay_GetApplicationInput();

      /* Valid? */
      if(sstMouse.poInput != orxNULL)
      {
        /* Updates status */
        sstMouse.u32Flags |= orxMOUSE_KU32_STATIC_FLAG_READY;

        /* Pushes config section */
        orxConfig_PushSection(orxMOUSE_KZ_CONFIG_SECTION);

        /* Has show cursor value? */
        if(orxConfig_HasValue(orxMOUSE_KZ_CONFIG_SHOW_CURSOR) != orxFALSE)
        {
          /* Updates cursor status */
          orxMouse_SFML_ShowCursor(orxConfig_GetBool(orxMOUSE_KZ_CONFIG_SHOW_CURSOR));
        }

        /* Pops config section */
        orxConfig_PopSection();

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
      else
      {
        /* Removes event handlers */
        orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseMoved), orxMouse_SFML_EventHandler);
        orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseWheelMoved), orxMouse_SFML_EventHandler);
        orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseButtonPressed), orxMouse_SFML_EventHandler);
        orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseButtonReleased), orxMouse_SFML_EventHandler);
      }
    }
    else
    {
      /* Removes event handlers */
      orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseMoved), orxMouse_SFML_EventHandler);
      orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseWheelMoved), orxMouse_SFML_EventHandler);
      orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseButtonPressed), orxMouse_SFML_EventHandler);
      orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseButtonReleased), orxMouse_SFML_EventHandler);
    }
  }

  /* Done! */
  return eResult;
}

extern "C" void orxFASTCALL orxMouse_SFML_Exit()
{
  /* Was initialized? */
  if(sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY)
  {
    /* Unregisters event handlers */
    orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseMoved), orxMouse_SFML_EventHandler);
    orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseWheelMoved), orxMouse_SFML_EventHandler);
    orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseButtonPressed), orxMouse_SFML_EventHandler);
    orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseButtonReleased), orxMouse_SFML_EventHandler);

    /* Cleans static controller */
    orxMemory_Zero(&sstMouse, sizeof(orxMOUSE_STATIC));
  }

  return;
}

extern "C" orxSTATUS orxFASTCALL orxMouse_SFML_SetPosition(const orxVECTOR *_pvPosition)
{
  orxEVENT  stEvent;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY) == orxMOUSE_KU32_STATIC_FLAG_READY);

  /* Inits event */
  orxEVENT_INIT(stEvent, orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseMoved, orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseMoved, orxNULL, orxNULL, _pvPosition);

  /* Sends system close event */
  eResult = orxEvent_Send(&stEvent);

  /* Done! */
  return eResult;
}

extern "C" orxVECTOR *orxFASTCALL orxMouse_SFML_GetPosition(orxVECTOR *_pvPosition)
{
  orxVECTOR *pvResult = _pvPosition;

  /* Checks */
  orxASSERT((sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY) == orxMOUSE_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvPosition != orxNULL);

  /* Gets mouse position */
  _pvPosition->fX = orxS2F(sstMouse.poInput->GetMouseX());
  _pvPosition->fY = orxS2F(sstMouse.poInput->GetMouseY());
  _pvPosition->fZ = orxFLOAT_0;

  /* Done! */
  return pvResult;
}

extern "C" orxBOOL orxFASTCALL orxMouse_SFML_IsButtonPressed(orxMOUSE_BUTTON _eButton)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT((sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY) == orxMOUSE_KU32_STATIC_FLAG_READY);
  orxASSERT(_eButton < orxMOUSE_BUTTON_NUMBER);

  /* Depending on button */
  switch(_eButton)
  {
    case orxMOUSE_BUTTON_LEFT:
    {
      /* Updates result */
      bResult = sstMouse.poInput->IsMouseButtonDown(sf::Mouse::Left) ? orxTRUE : orxFALSE;
      break;
    }

    case orxMOUSE_BUTTON_RIGHT:
    {
      /* Updates result */
      bResult = sstMouse.poInput->IsMouseButtonDown(sf::Mouse::Right) ? orxTRUE : orxFALSE;
      break;
    }

    case orxMOUSE_BUTTON_MIDDLE:
    {
      /* Updates result */
      bResult = sstMouse.poInput->IsMouseButtonDown(sf::Mouse::Middle) ? orxTRUE : orxFALSE;
      break;
    }

    case orxMOUSE_BUTTON_EXTRA_1:
    {
      /* Updates result */
      bResult = sstMouse.poInput->IsMouseButtonDown(sf::Mouse::XButton1) ? orxTRUE : orxFALSE;
      break;
    }

    case orxMOUSE_BUTTON_EXTRA_2:
    {
      /* Updates result */
      bResult = sstMouse.poInput->IsMouseButtonDown(sf::Mouse::XButton2) ? orxTRUE : orxFALSE;
      break;
    }

    case orxMOUSE_BUTTON_WHEEL_UP:
    case orxMOUSE_BUTTON_WHEEL_DOWN:
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_MOUSE, "The wheel button <%ld> can't be queried directly with this SFML plugin.", _eButton);
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

extern "C" orxVECTOR *orxFASTCALL orxMouse_SFML_GetMoveDelta(orxVECTOR *_pvMoveDelta)
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

extern "C" orxFLOAT orxFASTCALL orxMouse_SFML_GetWheelDelta()
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
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_SFML_Init, MOUSE, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_SFML_Exit, MOUSE, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_SFML_SetPosition, MOUSE, SET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_SFML_GetPosition, MOUSE, GET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_SFML_IsButtonPressed, MOUSE, IS_BUTTON_PRESSED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_SFML_GetMoveDelta, MOUSE, GET_MOVE_DELTA);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_SFML_GetWheelDelta, MOUSE, GET_WHEEL_DELTA);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_SFML_ShowCursor, MOUSE, SHOW_CURSOR);
orxPLUGIN_USER_CORE_FUNCTION_END();
