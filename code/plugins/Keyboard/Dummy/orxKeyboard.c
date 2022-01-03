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
 * @file orxKeyboard.c
 * @date 25/01/2010
 * @author iarwain@orx-project.org
 *
 * Dummy keyboard plugin implementation
 *
 */


#include "orxPluginAPI.h"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

orxSTATUS orxFASTCALL orxKeyboard_Dummy_Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Done! */
  return eResult;
}

void orxFASTCALL orxKeyboard_Dummy_Exit()
{
  /* Done! */
  return;
}

orxBOOL orxFASTCALL orxKeyboard_Dummy_IsKeyPressed(orxKEYBOARD_KEY _eKey)
{
  orxBOOL bResult = orxFALSE;

  /* Done! */
  return bResult;
}

const orxSTRING orxFASTCALL orxKeyboard_Dummy_GetKeyDisplayName(orxKEYBOARD_KEY _eKey)
{
  const orxSTRING zResult = orxSTRING_EMPTY;

  /* Done! */
  return zResult;
}

orxKEYBOARD_KEY orxFASTCALL orxKeyboard_Dummy_ReadKey()
{
  orxKEYBOARD_KEY eResult = orxKEYBOARD_KEY_NONE;

  /* Done! */
  return eResult;
}

const orxSTRING orxFASTCALL orxKeyboard_Dummy_ReadString()
{
  const orxSTRING zResult = orxSTRING_EMPTY;

  /* Done! */
  return zResult;
}

void orxFASTCALL orxKeyboard_Dummy_ClearBuffer()
{
  /* Done! */
  return;
}

orxSTATUS orxFASTCALL orxKeyboard_Dummy_Show(orxBOOL _bShow)
{
  return orxSTATUS_FAILURE;
}

/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(KEYBOARD);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Dummy_Init, KEYBOARD, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Dummy_Exit, KEYBOARD, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Dummy_IsKeyPressed, KEYBOARD, IS_KEY_PRESSED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Dummy_GetKeyDisplayName, KEYBOARD, GET_KEY_DISPLAY_NAME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Dummy_ReadKey, KEYBOARD, READ_KEY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Dummy_ReadString, KEYBOARD, READ_STRING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Dummy_ClearBuffer, KEYBOARD, CLEAR_BUFFER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Dummy_Show, KEYBOARD, SHOW);
orxPLUGIN_USER_CORE_FUNCTION_END();
