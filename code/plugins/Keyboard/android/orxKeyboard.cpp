/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2013 Orx-Project
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

#include "main/orxAndroid.h"
#include "android/keycodes.h"

/** Module flags
 */
#define orxKEYBOARD_KU32_STATIC_FLAG_NONE       0x00000000 /**< No flags */

#define orxKEYBOARD_KU32_STATIC_FLAG_READY      0x00000001 /**< Ready flag */

#define orxKEYBOARD_KU32_STATIC_MASK_ALL        0xFFFFFFFF /**< All mask */

#define orxKEYBOARD_MAX_ANDROID_KEYCODE         256

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxKEYBOARD_STATIC_t
{
  orxU32            u32Flags;
  orxBOOL           abKeyPressed[orxKEYBOARD_MAX_ANDROID_KEYCODE];
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

/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

static orxKEYBOARD_KEY orxFASTCALL orxKeyboard_Android_GetKey(int _eKey)
{
  orxKEYBOARD_KEY eResult;

  /* Depending on key */
  switch(_eKey)
  {
    case AKEYCODE_BACK:               {eResult = orxKEYBOARD_KEY_ESCAPE; break;}
    case AKEYCODE_SPACE:              {eResult = orxKEYBOARD_KEY_SPACE; break;}
    case AKEYCODE_ENTER:              {eResult = orxKEYBOARD_KEY_RETURN; break;}
    case AKEYCODE_CLEAR:              {eResult = orxKEYBOARD_KEY_BACKSPACE; break;}
    case AKEYCODE_TAB:                {eResult = orxKEYBOARD_KEY_TAB; break;}
    case AKEYCODE_PAGE_UP:            {eResult = orxKEYBOARD_KEY_PAGEUP; break;}
    case AKEYCODE_PAGE_DOWN:          {eResult = orxKEYBOARD_KEY_PAGEDOWN; break;}
    case AKEYCODE_DEL:                {eResult = orxKEYBOARD_KEY_DELETE; break;}
    case AKEYCODE_PLUS:               {eResult = orxKEYBOARD_KEY_ADD; break;}
    case AKEYCODE_STAR:               {eResult = orxKEYBOARD_KEY_MULTIPLY; break;}
    case AKEYCODE_MEDIA_PLAY_PAUSE:   {eResult = orxKEYBOARD_KEY_PAUSE; break;}
    case AKEYCODE_ALT_RIGHT:          {eResult = orxKEYBOARD_KEY_RALT; break;}
    case AKEYCODE_SHIFT_RIGHT:        {eResult = orxKEYBOARD_KEY_RSHIFT; break;}
    case AKEYCODE_ALT_LEFT:           {eResult = orxKEYBOARD_KEY_LALT; break;}
    case AKEYCODE_SHIFT_LEFT:         {eResult = orxKEYBOARD_KEY_LSHIFT; break;}
    case AKEYCODE_MENU:               {eResult = orxKEYBOARD_KEY_MENU; break;}
    case AKEYCODE_LEFT_BRACKET:       {eResult = orxKEYBOARD_KEY_LBRACKET; break;}
    case AKEYCODE_RIGHT_BRACKET:      {eResult = orxKEYBOARD_KEY_RBRACKET; break;}
    case AKEYCODE_SEMICOLON:          {eResult = orxKEYBOARD_KEY_SEMICOLON; break;}
    case AKEYCODE_COMMA:              {eResult = orxKEYBOARD_KEY_COMMA; break;}
    case AKEYCODE_PERIOD:             {eResult = orxKEYBOARD_KEY_PERIOD; break;}
    case AKEYCODE_APOSTROPHE:         {eResult = orxKEYBOARD_KEY_QUOTE; break;}
    case AKEYCODE_SLASH:              {eResult = orxKEYBOARD_KEY_SLASH; break;}
    case AKEYCODE_BACKSLASH:          {eResult = orxKEYBOARD_KEY_BACKSLASH; break;}
    case AKEYCODE_EQUALS:             {eResult = orxKEYBOARD_KEY_EQUAL; break;}
    case AKEYCODE_MINUS:              {eResult = orxKEYBOARD_KEY_DASH; break;}
    case AKEYCODE_DPAD_UP:            {eResult = orxKEYBOARD_KEY_UP; break;}
    case AKEYCODE_DPAD_RIGHT:         {eResult = orxKEYBOARD_KEY_RIGHT; break;}
    case AKEYCODE_DPAD_DOWN:          {eResult = orxKEYBOARD_KEY_DOWN; break;}
    case AKEYCODE_DPAD_LEFT:          {eResult = orxKEYBOARD_KEY_LEFT; break;}
    case AKEYCODE_A:                  {eResult = orxKEYBOARD_KEY_A; break;}
    case AKEYCODE_Z:                  {eResult = orxKEYBOARD_KEY_Z; break;}
    case AKEYCODE_E:                  {eResult = orxKEYBOARD_KEY_E; break;}
    case AKEYCODE_R:                  {eResult = orxKEYBOARD_KEY_R; break;}
    case AKEYCODE_T:                  {eResult = orxKEYBOARD_KEY_T; break;}
    case AKEYCODE_Y:                  {eResult = orxKEYBOARD_KEY_Y; break;}
    case AKEYCODE_U:                  {eResult = orxKEYBOARD_KEY_U; break;}
    case AKEYCODE_I:                  {eResult = orxKEYBOARD_KEY_I; break;}
    case AKEYCODE_O:                  {eResult = orxKEYBOARD_KEY_O; break;}
    case AKEYCODE_P:                  {eResult = orxKEYBOARD_KEY_P; break;}
    case AKEYCODE_Q:                  {eResult = orxKEYBOARD_KEY_Q; break;}
    case AKEYCODE_S:                  {eResult = orxKEYBOARD_KEY_S; break;}
    case AKEYCODE_D:                  {eResult = orxKEYBOARD_KEY_D; break;}
    case AKEYCODE_F:                  {eResult = orxKEYBOARD_KEY_F; break;}
    case AKEYCODE_G:                  {eResult = orxKEYBOARD_KEY_G; break;}
    case AKEYCODE_H:                  {eResult = orxKEYBOARD_KEY_H; break;}
    case AKEYCODE_J:                  {eResult = orxKEYBOARD_KEY_J; break;}
    case AKEYCODE_K:                  {eResult = orxKEYBOARD_KEY_K; break;}
    case AKEYCODE_L:                  {eResult = orxKEYBOARD_KEY_L; break;}
    case AKEYCODE_M:                  {eResult = orxKEYBOARD_KEY_M; break;}
    case AKEYCODE_W:                  {eResult = orxKEYBOARD_KEY_W; break;}
    case AKEYCODE_X:                  {eResult = orxKEYBOARD_KEY_X; break;}
    case AKEYCODE_C:                  {eResult = orxKEYBOARD_KEY_C; break;}
    case AKEYCODE_V:                  {eResult = orxKEYBOARD_KEY_V; break;}
    case AKEYCODE_B:                  {eResult = orxKEYBOARD_KEY_B; break;}
    case AKEYCODE_N:                  {eResult = orxKEYBOARD_KEY_N; break;}
    case AKEYCODE_0:                  {eResult = orxKEYBOARD_KEY_0; break;}
    case AKEYCODE_1:                  {eResult = orxKEYBOARD_KEY_1; break;}
    case AKEYCODE_2:                  {eResult = orxKEYBOARD_KEY_2; break;}
    case AKEYCODE_3:                  {eResult = orxKEYBOARD_KEY_3; break;}
    case AKEYCODE_4:                  {eResult = orxKEYBOARD_KEY_4; break;}
    case AKEYCODE_5:                  {eResult = orxKEYBOARD_KEY_5; break;}
    case AKEYCODE_6:                  {eResult = orxKEYBOARD_KEY_6; break;}
    case AKEYCODE_7:                  {eResult = orxKEYBOARD_KEY_7; break;}
    case AKEYCODE_8:                  {eResult = orxKEYBOARD_KEY_8; break;}
    case AKEYCODE_9:                  {eResult = orxKEYBOARD_KEY_9; break;}
    default:                          {eResult = orxKEYBOARD_KEY_NONE; break;}
  }

  /* Done! */
  return eResult;
}

static int orxFASTCALL orxKeyboard_Android_GetAKey(orxKEYBOARD_KEY _eKey)
{
  int eResult;

  /* Depending on key */
  switch(_eKey)
  {
    case orxKEYBOARD_KEY_ESCAPE:        {eResult = AKEYCODE_BACK; break;}
    case orxKEYBOARD_KEY_SPACE:         {eResult = AKEYCODE_SPACE; break;}
    case orxKEYBOARD_KEY_RETURN:        {eResult = AKEYCODE_ENTER; break;}
    case orxKEYBOARD_KEY_BACKSPACE:     {eResult = AKEYCODE_CLEAR; break;}
    case orxKEYBOARD_KEY_TAB:           {eResult = AKEYCODE_TAB; break;}
    case orxKEYBOARD_KEY_PAGEUP:        {eResult = AKEYCODE_PAGE_UP; break;}
    case orxKEYBOARD_KEY_PAGEDOWN:      {eResult = AKEYCODE_PAGE_DOWN; break;}
    case orxKEYBOARD_KEY_DELETE:        {eResult = AKEYCODE_DEL; break;}
    case orxKEYBOARD_KEY_ADD:           {eResult = AKEYCODE_PLUS; break;}
    case orxKEYBOARD_KEY_MULTIPLY:      {eResult = AKEYCODE_STAR; break;}
    case orxKEYBOARD_KEY_PAUSE:         {eResult = AKEYCODE_MEDIA_PLAY_PAUSE; break;}
    case orxKEYBOARD_KEY_RALT:          {eResult = AKEYCODE_ALT_RIGHT; break;}
    case orxKEYBOARD_KEY_RSHIFT:        {eResult = AKEYCODE_SHIFT_RIGHT; break;}
    case orxKEYBOARD_KEY_LALT:          {eResult = AKEYCODE_ALT_LEFT; break;}
    case orxKEYBOARD_KEY_LSHIFT:        {eResult = AKEYCODE_SHIFT_LEFT; break;}
    case orxKEYBOARD_KEY_MENU:          {eResult = AKEYCODE_MENU; break;}
    case orxKEYBOARD_KEY_LBRACKET:      {eResult = AKEYCODE_LEFT_BRACKET; break;}
    case orxKEYBOARD_KEY_RBRACKET:      {eResult = AKEYCODE_RIGHT_BRACKET; break;}
    case orxKEYBOARD_KEY_SEMICOLON:     {eResult = AKEYCODE_SEMICOLON; break;}
    case orxKEYBOARD_KEY_COMMA:         {eResult = AKEYCODE_COMMA; break;}
    case orxKEYBOARD_KEY_PERIOD:        {eResult = AKEYCODE_PERIOD; break;}
    case orxKEYBOARD_KEY_QUOTE:         {eResult = AKEYCODE_APOSTROPHE; break;}
    case orxKEYBOARD_KEY_SLASH:         {eResult = AKEYCODE_SLASH; break;}
    case orxKEYBOARD_KEY_BACKSLASH:     {eResult = AKEYCODE_BACKSLASH; break;}
    case orxKEYBOARD_KEY_EQUAL:         {eResult = AKEYCODE_EQUALS; break;}
    case orxKEYBOARD_KEY_DASH:          {eResult = AKEYCODE_MINUS; break;}
    case orxKEYBOARD_KEY_UP:            {eResult = AKEYCODE_DPAD_UP; break;}
    case orxKEYBOARD_KEY_RIGHT:         {eResult = AKEYCODE_DPAD_RIGHT; break;}
    case orxKEYBOARD_KEY_DOWN:          {eResult = AKEYCODE_DPAD_DOWN; break;}
    case orxKEYBOARD_KEY_LEFT:          {eResult = AKEYCODE_DPAD_LEFT; break;}
    case orxKEYBOARD_KEY_A:             {eResult = AKEYCODE_A; break;}
    case orxKEYBOARD_KEY_Z:             {eResult = AKEYCODE_Z; break;}
    case orxKEYBOARD_KEY_E:             {eResult = AKEYCODE_E; break;}
    case orxKEYBOARD_KEY_R:             {eResult = AKEYCODE_R; break;}
    case orxKEYBOARD_KEY_T:             {eResult = AKEYCODE_T; break;}
    case orxKEYBOARD_KEY_Y:             {eResult = AKEYCODE_Y; break;}
    case orxKEYBOARD_KEY_U:             {eResult = AKEYCODE_U; break;}
    case orxKEYBOARD_KEY_I:             {eResult = AKEYCODE_I; break;}
    case orxKEYBOARD_KEY_O:             {eResult = AKEYCODE_O; break;}
    case orxKEYBOARD_KEY_P:             {eResult = AKEYCODE_P; break;}
    case orxKEYBOARD_KEY_Q:             {eResult = AKEYCODE_Q; break;}
    case orxKEYBOARD_KEY_S:             {eResult = AKEYCODE_S; break;}
    case orxKEYBOARD_KEY_D:             {eResult = AKEYCODE_D; break;}
    case orxKEYBOARD_KEY_F:             {eResult = AKEYCODE_F; break;}
    case orxKEYBOARD_KEY_G:             {eResult = AKEYCODE_G; break;}
    case orxKEYBOARD_KEY_H:             {eResult = AKEYCODE_H; break;}
    case orxKEYBOARD_KEY_J:             {eResult = AKEYCODE_J; break;}
    case orxKEYBOARD_KEY_K:             {eResult = AKEYCODE_K; break;}
    case orxKEYBOARD_KEY_L:             {eResult = AKEYCODE_L; break;}
    case orxKEYBOARD_KEY_M:             {eResult = AKEYCODE_M; break;}
    case orxKEYBOARD_KEY_W:             {eResult = AKEYCODE_W; break;}
    case orxKEYBOARD_KEY_X:             {eResult = AKEYCODE_X; break;}
    case orxKEYBOARD_KEY_C:             {eResult = AKEYCODE_C; break;}
    case orxKEYBOARD_KEY_V:             {eResult = AKEYCODE_V; break;}
    case orxKEYBOARD_KEY_B:             {eResult = AKEYCODE_B; break;}
    case orxKEYBOARD_KEY_N:             {eResult = AKEYCODE_N; break;}
    case orxKEYBOARD_KEY_0:             {eResult = AKEYCODE_0; break;}
    case orxKEYBOARD_KEY_1:             {eResult = AKEYCODE_1; break;}
    case orxKEYBOARD_KEY_2:             {eResult = AKEYCODE_2; break;}
    case orxKEYBOARD_KEY_3:             {eResult = AKEYCODE_3; break;}
    case orxKEYBOARD_KEY_4:             {eResult = AKEYCODE_4; break;}
    case orxKEYBOARD_KEY_5:             {eResult = AKEYCODE_5; break;}
    case orxKEYBOARD_KEY_6:             {eResult = AKEYCODE_6; break;}
    case orxKEYBOARD_KEY_7:             {eResult = AKEYCODE_7; break;}
    case orxKEYBOARD_KEY_8:             {eResult = AKEYCODE_8; break;}
    case orxKEYBOARD_KEY_9:             {eResult = AKEYCODE_9; break;}
    default:                            {eResult = AKEYCODE_UNKNOWN; break;}
  }

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxKeyboard_Android_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;
  orxANDROID_KEY_EVENT *pstKeyEvent;

  /* Gets payload */
  pstKeyEvent = (orxANDROID_KEY_EVENT *) _pstEvent->pstPayload;

  /* Depending on ID */
  switch(_pstEvent->eID)
  {
    case orxANDROID_EVENT_KEYBOARD_DOWN:
      sstKeyboard.abKeyPressed[pstKeyEvent->u32KeyCode] = orxTRUE;
      break;
    case orxANDROID_EVENT_KEYBOARD_UP:
      sstKeyboard.abKeyPressed[pstKeyEvent->u32KeyCode] = orxFALSE;
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
    if((eResult = orxEvent_AddHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + orxANDROID_EVENT_KEYBOARD), orxKeyboard_Android_EventHandler)) != orxSTATUS_FAILURE)
    {
      int i;
      for(i = 0; i < orxKEYBOARD_MAX_ANDROID_KEYCODE; i++)
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
    orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + orxANDROID_EVENT_KEYBOARD), orxKeyboard_Android_EventHandler);

    /* Cleans static controller */
    orxMemory_Zero(&sstKeyboard, sizeof(orxKEYBOARD_STATIC));
  }

  return;
}

extern "C" orxBOOL orxFASTCALL orxKeyboard_Android_IsKeyPressed(orxKEYBOARD_KEY _eKey)
{
  orxS32 s32KeyIdx;
  orxBOOL bResult;

  /* Checks */
  orxASSERT(_eKey < orxKEYBOARD_KEY_NUMBER);
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  /* Gets Event key enum */
  s32KeyIdx = orxKeyboard_Android_GetAKey(_eKey);

  /* Valid? */
  if(s32KeyIdx != AKEYCODE_UNKNOWN)
  {
    /* Updates result */
    bResult = sstKeyboard.abKeyPressed[s32KeyIdx];
  }
  else
  {
    /* Updates result */
    bResult = orxFALSE;
  }

  /* Done! */
  return bResult;
}


extern "C" orxKEYBOARD_KEY orxFASTCALL orxKeyboard_Android_ReadKey()
{
  orxKEYBOARD_KEY eResult = orxKEYBOARD_KEY_NONE;

  /* Not yet implemented */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_KEYBOARD, "Not yet implemented!");

  /* Done! */
  return eResult;
}

extern "C" const orxSTRING orxFASTCALL orxKeyboard_Android_ReadString()
{
  const orxSTRING zResult = orxSTRING_EMPTY;

  /* Not yet implemented */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_KEYBOARD, "Not yet implemented!");

  /* Done! */
  return zResult;
}

extern "C" void orxFASTCALL orxKeyboard_Android_ClearBuffer()
{
  /* Not yet implemented */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_KEYBOARD, "Not yet implemented!");

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
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_ReadKey, KEYBOARD, READ_KEY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_ReadString, KEYBOARD, READ_STRING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_ClearBuffer, KEYBOARD, CLEAR_BUFFER);
orxPLUGIN_USER_CORE_FUNCTION_END();
