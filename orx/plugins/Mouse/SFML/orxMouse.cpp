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
 * @todo
 */


extern "C"
{
  #include "orxInclude.h"

  #include "plugin/orxPluginUser.h"
  #include "plugin/orxPlugin.h"

  #include "io/orxMouse.h"
  #include "display/orxDisplay.h"
}

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
  orxU32            u32Flags;
  sf::Input        *poInput;
} orxMOUSE_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
orxSTATIC orxMOUSE_STATIC sstMouse;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

extern "C" orxSTATUS orxMouse_SFML_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Was already initialized. */
  if(!(sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstMouse, sizeof(orxMOUSE_STATIC));

    /* Terrible hack : gets application input from display SFML plugin */
    sstMouse.poInput = (sf::Input *)orxDisplay_GetApplicationInput();

    /* Valid? */
    if(sstMouse.poInput != orxNULL)
    {
      /* Updates status */
      sstMouse.u32Flags |= orxMOUSE_KU32_STATIC_FLAG_READY;

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
  }

  /* Done! */
  return eResult;
}

extern "C" orxVOID orxMouse_SFML_Exit()
{
  /* Was initialized? */
  if(sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY)
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstMouse, sizeof(orxMOUSE_STATIC));
  }

  return;
}

extern "C" orxVECTOR *orxMouse_SFML_GetPosition(orxVECTOR *_pvPosition)
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

extern "C" orxBOOL orxMouse_SFML_IsButtonPressed(orxMOUSE_BUTTON _eButton)
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


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(MOUSE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_SFML_Init, MOUSE, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_SFML_Exit, MOUSE, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_SFML_GetPosition, MOUSE, GET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_SFML_IsButtonPressed, MOUSE, IS_BUTTON_PRESSED);
orxPLUGIN_USER_CORE_FUNCTION_END();
