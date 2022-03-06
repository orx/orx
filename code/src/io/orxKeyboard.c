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
 * @date 22/11/2003
 * @author iarwain@orx-project.org
 *
 */


#include "io/orxKeyboard.h"
#include "plugin/orxPluginCore.h"
#include "debug/orxDebug.h"


/** Misc defines
 */
#define orxKEYBOARD_KZ_LITERAL_PREFIX           "KEY_"


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Keyboard module setup
 */
void orxFASTCALL orxKeyboard_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_KEYBOARD, orxMODULE_ID_PLUGIN);
  orxModule_AddDependency(orxMODULE_ID_KEYBOARD, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_KEYBOARD, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_KEYBOARD, orxMODULE_ID_DISPLAY);

  return;
}

/** Gets key literal name
 * @param[in] _eKey       Concerned key
 * @return Key's name
 */
const orxSTRING orxFASTCALL orxKeyboard_GetKeyName(orxKEYBOARD_KEY _eKey)
{
  const orxSTRING zResult;

#define orxKEYBOARD_DECLARE_KEY_NAME(KEY)   case orxKEYBOARD_KEY_##KEY: zResult = orxKEYBOARD_KZ_LITERAL_PREFIX#KEY; break

  /* Checks */
  orxASSERT(_eKey < orxKEYBOARD_KEY_NUMBER);

  /* Depending on key */
  switch(_eKey)
  {
    orxKEYBOARD_DECLARE_KEY_NAME(0);
    orxKEYBOARD_DECLARE_KEY_NAME(1);
    orxKEYBOARD_DECLARE_KEY_NAME(2);
    orxKEYBOARD_DECLARE_KEY_NAME(3);
    orxKEYBOARD_DECLARE_KEY_NAME(4);
    orxKEYBOARD_DECLARE_KEY_NAME(5);
    orxKEYBOARD_DECLARE_KEY_NAME(6);
    orxKEYBOARD_DECLARE_KEY_NAME(7);
    orxKEYBOARD_DECLARE_KEY_NAME(8);
    orxKEYBOARD_DECLARE_KEY_NAME(9);
    orxKEYBOARD_DECLARE_KEY_NAME(A);
    orxKEYBOARD_DECLARE_KEY_NAME(B);
    orxKEYBOARD_DECLARE_KEY_NAME(C);
    orxKEYBOARD_DECLARE_KEY_NAME(D);
    orxKEYBOARD_DECLARE_KEY_NAME(E);
    orxKEYBOARD_DECLARE_KEY_NAME(F);
    orxKEYBOARD_DECLARE_KEY_NAME(G);
    orxKEYBOARD_DECLARE_KEY_NAME(H);
    orxKEYBOARD_DECLARE_KEY_NAME(I);
    orxKEYBOARD_DECLARE_KEY_NAME(J);
    orxKEYBOARD_DECLARE_KEY_NAME(K);
    orxKEYBOARD_DECLARE_KEY_NAME(L);
    orxKEYBOARD_DECLARE_KEY_NAME(M);
    orxKEYBOARD_DECLARE_KEY_NAME(N);
    orxKEYBOARD_DECLARE_KEY_NAME(O);
    orxKEYBOARD_DECLARE_KEY_NAME(P);
    orxKEYBOARD_DECLARE_KEY_NAME(Q);
    orxKEYBOARD_DECLARE_KEY_NAME(R);
    orxKEYBOARD_DECLARE_KEY_NAME(S);
    orxKEYBOARD_DECLARE_KEY_NAME(T);
    orxKEYBOARD_DECLARE_KEY_NAME(U);
    orxKEYBOARD_DECLARE_KEY_NAME(V);
    orxKEYBOARD_DECLARE_KEY_NAME(W);
    orxKEYBOARD_DECLARE_KEY_NAME(X);
    orxKEYBOARD_DECLARE_KEY_NAME(Y);
    orxKEYBOARD_DECLARE_KEY_NAME(Z);
    orxKEYBOARD_DECLARE_KEY_NAME(SPACE);
    orxKEYBOARD_DECLARE_KEY_NAME(QUOTE);
    orxKEYBOARD_DECLARE_KEY_NAME(COMMA);
    orxKEYBOARD_DECLARE_KEY_NAME(DASH);
    orxKEYBOARD_DECLARE_KEY_NAME(PERIOD);
    orxKEYBOARD_DECLARE_KEY_NAME(SLASH);
    orxKEYBOARD_DECLARE_KEY_NAME(SEMICOLON);
    orxKEYBOARD_DECLARE_KEY_NAME(EQUAL);
    orxKEYBOARD_DECLARE_KEY_NAME(LBRACKET);
    orxKEYBOARD_DECLARE_KEY_NAME(BACKSLASH);
    orxKEYBOARD_DECLARE_KEY_NAME(RBRACKET);
    orxKEYBOARD_DECLARE_KEY_NAME(BACKQUOTE);
    orxKEYBOARD_DECLARE_KEY_NAME(WORLD_1);
    orxKEYBOARD_DECLARE_KEY_NAME(WORLD_2);
    orxKEYBOARD_DECLARE_KEY_NAME(ESCAPE);
    orxKEYBOARD_DECLARE_KEY_NAME(ENTER);
    orxKEYBOARD_DECLARE_KEY_NAME(TAB);
    orxKEYBOARD_DECLARE_KEY_NAME(BACKSPACE);
    orxKEYBOARD_DECLARE_KEY_NAME(INSERT);
    orxKEYBOARD_DECLARE_KEY_NAME(DELETE);
    orxKEYBOARD_DECLARE_KEY_NAME(RIGHT);
    orxKEYBOARD_DECLARE_KEY_NAME(LEFT);
    orxKEYBOARD_DECLARE_KEY_NAME(DOWN);
    orxKEYBOARD_DECLARE_KEY_NAME(UP);
    orxKEYBOARD_DECLARE_KEY_NAME(PAGE_UP);
    orxKEYBOARD_DECLARE_KEY_NAME(PAGE_DOWN);
    orxKEYBOARD_DECLARE_KEY_NAME(HOME);
    orxKEYBOARD_DECLARE_KEY_NAME(END);
    orxKEYBOARD_DECLARE_KEY_NAME(CAPS_LOCK);
    orxKEYBOARD_DECLARE_KEY_NAME(SCROLL_LOCK);
    orxKEYBOARD_DECLARE_KEY_NAME(NUM_LOCK);
    orxKEYBOARD_DECLARE_KEY_NAME(PRINT_SCREEN);
    orxKEYBOARD_DECLARE_KEY_NAME(PAUSE);
    orxKEYBOARD_DECLARE_KEY_NAME(VOLUME_DOWN);
    orxKEYBOARD_DECLARE_KEY_NAME(VOLUME_UP);
    orxKEYBOARD_DECLARE_KEY_NAME(F1);
    orxKEYBOARD_DECLARE_KEY_NAME(F2);
    orxKEYBOARD_DECLARE_KEY_NAME(F3);
    orxKEYBOARD_DECLARE_KEY_NAME(F4);
    orxKEYBOARD_DECLARE_KEY_NAME(F5);
    orxKEYBOARD_DECLARE_KEY_NAME(F6);
    orxKEYBOARD_DECLARE_KEY_NAME(F7);
    orxKEYBOARD_DECLARE_KEY_NAME(F8);
    orxKEYBOARD_DECLARE_KEY_NAME(F9);
    orxKEYBOARD_DECLARE_KEY_NAME(F10);
    orxKEYBOARD_DECLARE_KEY_NAME(F11);
    orxKEYBOARD_DECLARE_KEY_NAME(F12);
    orxKEYBOARD_DECLARE_KEY_NAME(F13);
    orxKEYBOARD_DECLARE_KEY_NAME(F14);
    orxKEYBOARD_DECLARE_KEY_NAME(F15);
    orxKEYBOARD_DECLARE_KEY_NAME(F16);
    orxKEYBOARD_DECLARE_KEY_NAME(F17);
    orxKEYBOARD_DECLARE_KEY_NAME(F18);
    orxKEYBOARD_DECLARE_KEY_NAME(F19);
    orxKEYBOARD_DECLARE_KEY_NAME(F20);
    orxKEYBOARD_DECLARE_KEY_NAME(F21);
    orxKEYBOARD_DECLARE_KEY_NAME(F22);
    orxKEYBOARD_DECLARE_KEY_NAME(F23);
    orxKEYBOARD_DECLARE_KEY_NAME(F24);
    orxKEYBOARD_DECLARE_KEY_NAME(F25);
    orxKEYBOARD_DECLARE_KEY_NAME(NUMPAD_0);
    orxKEYBOARD_DECLARE_KEY_NAME(NUMPAD_1);
    orxKEYBOARD_DECLARE_KEY_NAME(NUMPAD_2);
    orxKEYBOARD_DECLARE_KEY_NAME(NUMPAD_3);
    orxKEYBOARD_DECLARE_KEY_NAME(NUMPAD_4);
    orxKEYBOARD_DECLARE_KEY_NAME(NUMPAD_5);
    orxKEYBOARD_DECLARE_KEY_NAME(NUMPAD_6);
    orxKEYBOARD_DECLARE_KEY_NAME(NUMPAD_7);
    orxKEYBOARD_DECLARE_KEY_NAME(NUMPAD_8);
    orxKEYBOARD_DECLARE_KEY_NAME(NUMPAD_9);
    orxKEYBOARD_DECLARE_KEY_NAME(NUMPAD_DECIMAL);
    orxKEYBOARD_DECLARE_KEY_NAME(NUMPAD_DIVIDE);
    orxKEYBOARD_DECLARE_KEY_NAME(NUMPAD_MULTIPLY);
    orxKEYBOARD_DECLARE_KEY_NAME(NUMPAD_SUBTRACT);
    orxKEYBOARD_DECLARE_KEY_NAME(NUMPAD_ADD);
    orxKEYBOARD_DECLARE_KEY_NAME(NUMPAD_ENTER);
    orxKEYBOARD_DECLARE_KEY_NAME(NUMPAD_EQUAL);
    orxKEYBOARD_DECLARE_KEY_NAME(LSHIFT);
    orxKEYBOARD_DECLARE_KEY_NAME(LCTRL);
    orxKEYBOARD_DECLARE_KEY_NAME(LALT);
    orxKEYBOARD_DECLARE_KEY_NAME(LSYSTEM);
    orxKEYBOARD_DECLARE_KEY_NAME(RSHIFT);
    orxKEYBOARD_DECLARE_KEY_NAME(RCTRL);
    orxKEYBOARD_DECLARE_KEY_NAME(RALT);
    orxKEYBOARD_DECLARE_KEY_NAME(RSYSTEM);
    orxKEYBOARD_DECLARE_KEY_NAME(MENU);

    default:
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_KEYBOARD, "No name defined for key #%d.", _eKey);

      /* Updates result */
      zResult = orxSTRING_EMPTY;
    }
  }

  /* Done! */
  return zResult;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION(orxKeyboard_Init, orxSTATUS, void);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxKeyboard_Exit, void, void);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxKeyboard_IsKeyPressed, orxBOOL, orxKEYBOARD_KEY);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxKeyboard_GetKeyDisplayName, const orxSTRING, orxKEYBOARD_KEY);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxKeyboard_ReadKey, orxKEYBOARD_KEY, void);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxKeyboard_ReadString, const orxSTRING, void);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxKeyboard_ClearBuffer, void, void);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxKeyboard_Show, orxSTATUS, orxBOOL);


/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(KEYBOARD)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(KEYBOARD, INIT, orxKeyboard_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(KEYBOARD, EXIT, orxKeyboard_Exit)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(KEYBOARD, IS_KEY_PRESSED, orxKeyboard_IsKeyPressed)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(KEYBOARD, GET_KEY_DISPLAY_NAME, orxKeyboard_GetKeyDisplayName)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(KEYBOARD, READ_KEY, orxKeyboard_ReadKey)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(KEYBOARD, READ_STRING, orxKeyboard_ReadString)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(KEYBOARD, CLEAR_BUFFER, orxKeyboard_ClearBuffer)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(KEYBOARD, SHOW, orxKeyboard_Show)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(KEYBOARD)


/* *** Core function implementations *** */

/** Inits the keyboard module
 */
orxSTATUS orxFASTCALL orxKeyboard_Init()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxKeyboard_Init)();
}

/** Exits from the keyboard module
 */
void orxFASTCALL orxKeyboard_Exit()
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxKeyboard_Exit)();
}

/** Is key pressed?
 * @param[in] _eKey       Key to check
 * @return orxTRUE if pressed / orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY _eKey)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxKeyboard_IsKeyPressed)(_eKey);
}

/** Gets key display name, layout-dependent
 * @param[in] _eKey       Concerned key
 * @return UTF-8 encoded key's name if valid, orxSTRING_EMPTY otherwise
 */
const orxSTRING orxFASTCALL orxKeyboard_GetKeyDisplayName(orxKEYBOARD_KEY _eKey)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxKeyboard_GetKeyDisplayName)(_eKey);
}

/** Gets the next key from the keyboard buffer and removes it from there
 * @return orxKEYBOARD_KEY, orxKEYBOARD_KEY_NONE if the buffer is empty
 */
orxKEYBOARD_KEY orxFASTCALL orxKeyboard_ReadKey()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxKeyboard_ReadKey)();
}

/** Gets the next UTF-8 encoded string from the keyboard buffer and removes it from there
 * @return UTF-8 encoded string
 */
const orxSTRING orxFASTCALL orxKeyboard_ReadString()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxKeyboard_ReadString)();
}

/**  Empties the keyboard buffer.
 */
void orxFASTCALL orxKeyboard_ClearBuffer()
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxKeyboard_ClearBuffer)();
}

/** Show/Hide the virtual keyboard
 * @param[in]   _bShow          Show/hide virtual keyboard
 * @return orxSTATUS_SUCCESS if supported by platform, orxSTATUS_FAILURE otherwise
 */
orxSTATUS orxFASTCALL orxKeyboard_Show(orxBOOL _bShow)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxKeyboard_Show)(_bShow);
}
