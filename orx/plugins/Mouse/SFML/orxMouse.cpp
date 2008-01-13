/**
 * @file orxMouse.cpp
 * 
 * SFML mouse plugin
 * 
 */
 
 /***************************************************************************
 orxMouse.cpp
 SFML mouse plugin
 
 begin                : 26/11/2007
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
    orxMemory_Set(&sstMouse, 0, sizeof(orxMOUSE_STATIC));

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
    orxMemory_Set(&sstMouse, 0, sizeof(orxMOUSE_STATIC));
  }

  return;
}

extern "C" orxSTATUS orxMouse_SFML_GetPosition(orxS32 *_ps32X, orxS32 *_ps32Y)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(_ps32X != orxNULL);
  orxASSERT(_ps32Y != orxNULL);

  /* Gets mouse position */
  *_ps32X = sstMouse.poInput->GetMouseX();
  *_ps32Y = sstMouse.poInput->GetMouseY();

  /* Done! */
  return eResult;
}

extern "C" orxBOOL orxMouse_SFML_IsButtonPressed(orxMOUSE_BUTTON _eButton)
{
  orxBOOL bResult;

  /* Checks */
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


/********************
 *  Plugin Related  *
 ********************/

orxPLUGIN_USER_CORE_FUNCTION_CPP_START(MOUSE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_SFML_Init, MOUSE, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_SFML_Exit, MOUSE, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_SFML_GetPosition, MOUSE, GET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_SFML_IsButtonPressed, MOUSE, IS_BUTTON_PRESSED);
orxPLUGIN_USER_CORE_FUNCTION_END();
