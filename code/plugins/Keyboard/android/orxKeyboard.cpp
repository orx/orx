/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2011 Orx-Project
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
 * @date 26/06/2010
 * @author iarwain@orx-project.org
 *
 * Android keyboard plugin implementation
 *
 */


#include "orxPluginAPI.h"

#include <nv_event/nv_event.h>

/** Module flags
 */
#define orxKEYBOARD_KU32_STATIC_FLAG_NONE       0x00000000 /**< No flags */

#define orxKEYBOARD_KU32_STATIC_FLAG_READY      0x00000001 /**< Ready flag */

#define orxKEYBOARD_KU32_STATIC_MASK_ALL        0xFFFFFFFF /**< All mask */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxKEYBOARD_STATIC_t
{
  orxU32            u32Flags;
  orxBOOL           abKeyPressed[NV_MAX_KEYCODE];
} orxKEYBOARD_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxKEYBOARD_STATIC sstKeyboard;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/
static NVKeyCode orxFASTCALL orxKeyboard_Android_GetNVKey(orxKEYBOARD_KEY _eKey)
{
  NVKeyCode eResult;

  /* Depending on key */
  switch(_eKey)
  {
    case orxKEYBOARD_KEY_ESCAPE:        {eResult = NV_KEYCODE_BACK; break;}
    case orxKEYBOARD_KEY_MENU:          {eResult = NV_KEYCODE_MENU; break;}

    case orxKEYBOARD_KEY_SPACE:         {eResult = NV_KEYCODE_SPACE; break;}
    case orxKEYBOARD_KEY_RETURN:        {eResult = NV_KEYCODE_ENTER; break;}
    case orxKEYBOARD_KEY_BACKSPACE:     {eResult = NV_KEYCODE_DEL; break;}
    case orxKEYBOARD_KEY_TAB:           {eResult = NV_KEYCODE_TAB; break;}
    case orxKEYBOARD_KEY_HOME:          {eResult = NV_KEYCODE_HOME; break;}
    case orxKEYBOARD_KEY_DELETE:        {eResult = NV_KEYCODE_DEL; break;}
    case orxKEYBOARD_KEY_ADD:           {eResult = NV_KEYCODE_PLUS; break;}
    case orxKEYBOARD_KEY_SUBTRACT:      {eResult = NV_KEYCODE_MINUS; break;}
    case orxKEYBOARD_KEY_MULTIPLY:      {eResult = NV_KEYCODE_STAR; break;}
    case orxKEYBOARD_KEY_DIVIDE:        {eResult = NV_KEYCODE_SLASH; break;}
    case orxKEYBOARD_KEY_RALT:          {eResult = NV_KEYCODE_ALT_RIGHT; break;}
    case orxKEYBOARD_KEY_RSHIFT:        {eResult = NV_KEYCODE_SHIFT_RIGHT; break;}
    case orxKEYBOARD_KEY_LALT:          {eResult = NV_KEYCODE_ALT_LEFT; break;}
    case orxKEYBOARD_KEY_LSHIFT:        {eResult = NV_KEYCODE_SHIFT_LEFT; break;}
    case orxKEYBOARD_KEY_LBRACKET:      {eResult = NV_KEYCODE_LEFT_BRACKET; break;}
    case orxKEYBOARD_KEY_RBRACKET:      {eResult = NV_KEYCODE_RIGHT_BRACKET; break;}
    case orxKEYBOARD_KEY_SEMICOLON:     {eResult = NV_KEYCODE_SEMICOLON; break;}
    case orxKEYBOARD_KEY_COMMA:         {eResult = NV_KEYCODE_COMMA; break;}
    case orxKEYBOARD_KEY_PERIOD:        {eResult = NV_KEYCODE_PERIOD; break;}
    case orxKEYBOARD_KEY_SLASH:         {eResult = NV_KEYCODE_SLASH; break;}
    case orxKEYBOARD_KEY_BACKSLASH:     {eResult = NV_KEYCODE_BACKSLASH; break;}
    case orxKEYBOARD_KEY_EQUAL:         {eResult = NV_KEYCODE_EQUALS; break;}
    case orxKEYBOARD_KEY_UP:            {eResult = NV_KEYCODE_DPAD_UP; break;}
    case orxKEYBOARD_KEY_RIGHT:         {eResult = NV_KEYCODE_DPAD_RIGHT; break;}
    case orxKEYBOARD_KEY_DOWN:          {eResult = NV_KEYCODE_DPAD_DOWN; break;}
    case orxKEYBOARD_KEY_LEFT:          {eResult = NV_KEYCODE_DPAD_LEFT; break;}
    case orxKEYBOARD_KEY_A:             {eResult = NV_KEYCODE_A; break;}
    case orxKEYBOARD_KEY_Z:             {eResult = NV_KEYCODE_Z; break;}
    case orxKEYBOARD_KEY_E:             {eResult = NV_KEYCODE_E; break;}
    case orxKEYBOARD_KEY_R:             {eResult = NV_KEYCODE_R; break;}
    case orxKEYBOARD_KEY_T:             {eResult = NV_KEYCODE_T; break;}
    case orxKEYBOARD_KEY_Y:             {eResult = NV_KEYCODE_Y; break;}
    case orxKEYBOARD_KEY_U:             {eResult = NV_KEYCODE_U; break;}
    case orxKEYBOARD_KEY_I:             {eResult = NV_KEYCODE_I; break;}
    case orxKEYBOARD_KEY_O:             {eResult = NV_KEYCODE_O; break;}
    case orxKEYBOARD_KEY_P:             {eResult = NV_KEYCODE_P; break;}
    case orxKEYBOARD_KEY_Q:             {eResult = NV_KEYCODE_Q; break;}
    case orxKEYBOARD_KEY_S:             {eResult = NV_KEYCODE_S; break;}
    case orxKEYBOARD_KEY_D:             {eResult = NV_KEYCODE_D; break;}
    case orxKEYBOARD_KEY_F:             {eResult = NV_KEYCODE_F; break;}
    case orxKEYBOARD_KEY_G:             {eResult = NV_KEYCODE_G; break;}
    case orxKEYBOARD_KEY_H:             {eResult = NV_KEYCODE_H; break;}
    case orxKEYBOARD_KEY_J:             {eResult = NV_KEYCODE_J; break;}
    case orxKEYBOARD_KEY_K:             {eResult = NV_KEYCODE_K; break;}
    case orxKEYBOARD_KEY_L:             {eResult = NV_KEYCODE_L; break;}
    case orxKEYBOARD_KEY_M:             {eResult = NV_KEYCODE_M; break;}
    case orxKEYBOARD_KEY_W:             {eResult = NV_KEYCODE_W; break;}
    case orxKEYBOARD_KEY_X:             {eResult = NV_KEYCODE_X; break;}
    case orxKEYBOARD_KEY_C:             {eResult = NV_KEYCODE_C; break;}
    case orxKEYBOARD_KEY_V:             {eResult = NV_KEYCODE_V; break;}
    case orxKEYBOARD_KEY_B:             {eResult = NV_KEYCODE_B; break;}
    case orxKEYBOARD_KEY_N:             {eResult = NV_KEYCODE_N; break;}
    case orxKEYBOARD_KEY_0:             {eResult = NV_KEYCODE_0; break;}
    case orxKEYBOARD_KEY_1:             {eResult = NV_KEYCODE_1; break;}
    case orxKEYBOARD_KEY_2:             {eResult = NV_KEYCODE_2; break;}
    case orxKEYBOARD_KEY_3:             {eResult = NV_KEYCODE_3; break;}
    case orxKEYBOARD_KEY_4:             {eResult = NV_KEYCODE_4; break;}
    case orxKEYBOARD_KEY_5:             {eResult = NV_KEYCODE_5; break;}
    case orxKEYBOARD_KEY_6:             {eResult = NV_KEYCODE_6; break;}
    case orxKEYBOARD_KEY_7:             {eResult = NV_KEYCODE_7; break;}
    case orxKEYBOARD_KEY_8:             {eResult = NV_KEYCODE_8; break;}
    case orxKEYBOARD_KEY_9:             {eResult = NV_KEYCODE_9; break;}
    default:                            {eResult = NV_KEYCODE_NULL; break;}
  }

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxKeyboard_Android_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  orxSYSTEM_EVENT_PAYLOAD *pstPayload;

  /* Gets payload */
  pstPayload = (orxSYSTEM_EVENT_PAYLOAD *) _pstEvent->pstPayload;

  /* Depending on ID */
  switch(_pstEvent->eID)
  {
  /* Keyboard? */
  case orxSYSTEM_EVENT_KEY_DOWN:
    sstKeyboard.abKeyPressed[pstPayload->stKeyboard.u32KeyCode] = orxTRUE;
    break;
  case orxSYSTEM_EVENT_KEY_UP:
    sstKeyboard.abKeyPressed[pstPayload->stKeyboard.u32KeyCode] = orxFALSE;
    break;
  }

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxKeyboard_Android_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Wasn't already initialized? */
  if(!(sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstKeyboard, sizeof(orxKEYBOARD_STATIC));
    
    /* Adds our mouse event handlers */
    if((eResult = orxEvent_AddHandler(orxEVENT_TYPE_SYSTEM, orxKeyboard_Android_EventHandler)) != orxSTATUS_FAILURE)
    {
      int i;
      for(i = 0; i < NV_MAX_KEYCODE; i++)
      {
        sstKeyboard.abKeyPressed[i] = orxFALSE;
      }

      /* Updates status */
      sstKeyboard.u32Flags |= orxKEYBOARD_KU32_STATIC_FLAG_READY;
    }
 }

  /* Done! */
  return eResult;
}

extern "C" void orxFASTCALL orxKeyboard_Android_Exit()
{
  /* Was initialized? */
  if(sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY)
  {
    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_SYSTEM, orxKeyboard_Android_EventHandler);

    /* Cleans static controller */
    orxMemory_Zero(&sstKeyboard, sizeof(orxKEYBOARD_STATIC));
  }

  return;
}

extern "C" orxBOOL orxFASTCALL orxKeyboard_Android_IsKeyPressed(orxKEYBOARD_KEY _eKey)
{
  NVKeyCode eNVKey;
  orxBOOL bResult;

  /* Checks */
  orxASSERT(_eKey < orxKEYBOARD_KEY_NUMBER);
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  /* Gets NvEvent key enum */
  eNVKey = orxKeyboard_Android_GetNVKey(_eKey);

  /* Valid? */
  if(eNVKey != NV_KEYCODE_NULL)
  {
    /* Updates result */
    bResult = sstKeyboard.abKeyPressed[eNVKey];
  }
  else
  {
    /* Logs message */
    orxLOG("Key <%s> is not handled by this plugin.", orxKeyboard_GetKeyName(_eKey));

    /* Updates result */
    bResult = orxFALSE;
  }

  /* Done! */
  return bResult;
}


extern "C" orxKEYBOARD_KEY orxFASTCALL orxKeyboard_Android_Read()
{
  orxU32          i;
  orxKEYBOARD_KEY eResult = orxKEYBOARD_KEY_NONE;

  /* Checks */
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  /* For all keys */
  for(i = 0; i < orxKEYBOARD_KEY_NUMBER; i++)
  {
    /* Is pressed? */
    if(orxKeyboard_Android_IsKeyPressed((orxKEYBOARD_KEY)i) != orxFALSE)
    {
      /* Updates result */
      eResult = (orxKEYBOARD_KEY)i;

      break;
    }
  }

  /* Done! */
  return eResult;
}

extern "C" orxBOOL orxFASTCALL orxKeyboard_Android_Hit()
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  /* Updates result */
  bResult = (orxKeyboard_Android_Read() != orxKEYBOARD_KEY_NONE) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

extern "C" void orxFASTCALL orxKeyboard_Android_ClearBuffer()
{
  /* Checks */
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  /* Not implemented yet */
  orxASSERT(orxFALSE && "Not implemented yet!");

  /* Done! */
  return;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(KEYBOARD);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_Init, KEYBOARD, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_Exit, KEYBOARD, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_IsKeyPressed, KEYBOARD, IS_KEY_PRESSED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_Hit, KEYBOARD, HIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_Read, KEYBOARD, READ);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_ClearBuffer, KEYBOARD, CLEAR_BUFFER);
orxPLUGIN_USER_CORE_FUNCTION_END();
