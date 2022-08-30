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

/** Misc
 */
#define orxKEYBOARD_KU32_BUFFER_SIZE            64
#define orxKEYBOARD_KU32_STRING_BUFFER_SIZE     (orxKEYBOARD_KU32_BUFFER_SIZE * 4 + 1)

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxKEYBOARD_STATIC_t
{
  orxU32            u32Flags;
  orxBOOL           abKeyPressed[orxKEYBOARD_KEY_NUMBER];

  orxU32            u32KeyReadIndex, u32KeyWriteIndex, u32CharReadIndex, u32CharWriteIndex;
  orxU32            au32KeyBuffer[orxKEYBOARD_KU32_BUFFER_SIZE];
  orxU32            au32CharBuffer[orxKEYBOARD_KU32_BUFFER_SIZE];
  orxCHAR           acStringBuffer[orxKEYBOARD_KU32_STRING_BUFFER_SIZE];
#ifdef __orxANDROID__
  jmethodID         midShowKeyboard;
#endif
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

static orxKEYBOARD_KEY orxFASTCALL orxKeyboard_Android_GetKey(orxU32 _eKey)
{
  orxKEYBOARD_KEY eResult;

  /* Depending on key */
  switch(_eKey)
  {
    case AKEYCODE_BACK:               {eResult = orxKEYBOARD_KEY_ESCAPE; break;}
    case AKEYCODE_SPACE:              {eResult = orxKEYBOARD_KEY_SPACE; break;}
    case AKEYCODE_ENTER:              {eResult = orxKEYBOARD_KEY_ENTER; break;}
    case AKEYCODE_BUTTON_SELECT:      {eResult = orxKEYBOARD_KEY_ENTER; break;}
    case AKEYCODE_DEL:                {eResult = orxKEYBOARD_KEY_BACKSPACE; break;}
    case AKEYCODE_TAB:                {eResult = orxKEYBOARD_KEY_TAB; break;}
    case AKEYCODE_PAGE_UP:            {eResult = orxKEYBOARD_KEY_PAGE_UP; break;}
    case AKEYCODE_PAGE_DOWN:          {eResult = orxKEYBOARD_KEY_PAGE_DOWN; break;}
    case AKEYCODE_PLUS:               {eResult = orxKEYBOARD_KEY_NUMPAD_ADD; break;}
    case AKEYCODE_STAR:               {eResult = orxKEYBOARD_KEY_NUMPAD_MULTIPLY; break;}
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
    case AKEYCODE_VOLUME_DOWN:        {eResult = orxKEYBOARD_KEY_VOLUME_DOWN; break;}
    case AKEYCODE_VOLUME_UP:          {eResult = orxKEYBOARD_KEY_VOLUME_UP; break;}
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

static orxSTATUS orxFASTCALL orxKeyboard_Android_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;
  orxANDROID_KEY_EVENT *pstKeyEvent;
  orxKEYBOARD_KEY eKey;

  /* Gets payload */
  pstKeyEvent = (orxANDROID_KEY_EVENT *) _pstEvent->pstPayload;

  /* Depending on ID */
  switch(pstKeyEvent->u32Action)
  {
    case orxANDROID_EVENT_KEYBOARD_DOWN:
      eKey = orxKeyboard_Android_GetKey(pstKeyEvent->u32KeyCode);

      if(eKey != orxKEYBOARD_KEY_NONE)
      {
        sstKeyboard.abKeyPressed[eKey] = orxTRUE;

        /* Stores it */
        sstKeyboard.au32KeyBuffer[sstKeyboard.u32KeyWriteIndex] = pstKeyEvent->u32KeyCode;
        sstKeyboard.u32KeyWriteIndex = (sstKeyboard.u32KeyWriteIndex + 1) & (orxKEYBOARD_KU32_BUFFER_SIZE - 1);

        /* Full? */
        if(sstKeyboard.u32KeyReadIndex == sstKeyboard.u32KeyWriteIndex)
        {
          /* Bounces read index */
          sstKeyboard.u32KeyReadIndex = (sstKeyboard.u32KeyReadIndex + 1) & (orxKEYBOARD_KU32_BUFFER_SIZE - 1);
        }
      }

      /* has unicode */
      if(pstKeyEvent->u32Unicode != 0)
      {
        /* Stores it */
        sstKeyboard.au32CharBuffer[sstKeyboard.u32CharWriteIndex] = pstKeyEvent->u32Unicode;
        sstKeyboard.u32CharWriteIndex = (sstKeyboard.u32CharWriteIndex + 1) & (orxKEYBOARD_KU32_BUFFER_SIZE - 1);

        /* Full? */
        if(sstKeyboard.u32CharReadIndex == sstKeyboard.u32CharWriteIndex)
        {
          /* Bounces read index */
          sstKeyboard.u32CharReadIndex = (sstKeyboard.u32CharReadIndex + 1) & (orxKEYBOARD_KU32_BUFFER_SIZE - 1);
        }
      }

      break;

    case orxANDROID_EVENT_KEYBOARD_UP: // UP
      eKey = orxKeyboard_Android_GetKey(pstKeyEvent->u32KeyCode);

      if(eKey != orxKEYBOARD_KEY_NONE)
      {
        sstKeyboard.abKeyPressed[eKey] = orxFALSE;
      }
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
    if((eResult = orxEvent_AddHandler(orxANDROID_EVENT_TYPE_KEYBOARD, orxKeyboard_Android_EventHandler)) != orxSTATUS_FAILURE)
    {
      int i;
      for(i = 0; i < orxKEYBOARD_KEY_NUMBER; i++)
      {
        sstKeyboard.abKeyPressed[i] = orxFALSE;
      }

#ifdef __orxANDROID__
      JNIEnv *env = (JNIEnv*) orxAndroid_GetJNIEnv();
      jclass objClass = env->FindClass("org/orx/lib/OrxActivity");
      sstKeyboard.midShowKeyboard = env->GetMethodID(objClass, "showKeyboard","(Z)V");
      env->DeleteLocalRef(objClass);
#endif

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
    orxEvent_RemoveHandler(orxANDROID_EVENT_TYPE_KEYBOARD, orxKeyboard_Android_EventHandler);

    /* Cleans static controller */
    orxMemory_Zero(&sstKeyboard, sizeof(orxKEYBOARD_STATIC));
  }

  return;
}

extern "C" orxBOOL orxFASTCALL orxKeyboard_Android_IsKeyPressed(orxKEYBOARD_KEY _eKey)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT(_eKey < orxKEYBOARD_KEY_NUMBER);
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  /* Updates result */
  bResult = sstKeyboard.abKeyPressed[_eKey];

  /* Done! */
  return bResult;
}

extern "C" const orxSTRING orxFASTCALL orxKeyboard_Android_GetKeyDisplayName(orxKEYBOARD_KEY _eKey)
{
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(_eKey < orxKEYBOARD_KEY_NUMBER);
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  /* Gets key name */
  zResult = orxKeyboard_GetKeyName(_eKey);

  /* Success? */
  if(zResult != orxSTRING_EMPTY)
  {
    /* Skips prefix */
    zResult += 4;
  }

  /* Done! */
  return zResult;
}

extern "C" orxKEYBOARD_KEY orxFASTCALL orxKeyboard_Android_ReadKey()
{
  orxKEYBOARD_KEY eResult;

  /* Checks */
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  /* Not empty? */
  if(sstKeyboard.u32KeyReadIndex != sstKeyboard.u32KeyWriteIndex)
  {
    /* Updates result */
    eResult = orxKeyboard_Android_GetKey((orxS32)sstKeyboard.au32KeyBuffer[sstKeyboard.u32KeyReadIndex]);

    /* Updates read index */
    sstKeyboard.u32KeyReadIndex = (sstKeyboard.u32KeyReadIndex + 1) & (orxKEYBOARD_KU32_BUFFER_SIZE - 1);
  }
  else
  {
    /* Updates result */
    eResult = orxKEYBOARD_KEY_NONE;
  }

  /* Done! */
  return eResult;
}

extern "C" const orxSTRING orxFASTCALL orxKeyboard_Android_ReadString()
{
  orxU32          u32BufferSize;
  orxCHAR        *pc;
  const orxSTRING zResult = sstKeyboard.acStringBuffer;

  /* Checks */
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  /* For all characters */
  for(zResult = pc = sstKeyboard.acStringBuffer, u32BufferSize = orxKEYBOARD_KU32_STRING_BUFFER_SIZE - 1;
      sstKeyboard.u32CharReadIndex != sstKeyboard.u32CharWriteIndex;
      sstKeyboard.u32CharReadIndex = (sstKeyboard.u32CharReadIndex + 1) & (orxKEYBOARD_KU32_BUFFER_SIZE - 1))
  {
    orxU32 u32Size;

    /* Writes it */
    u32Size = orxString_PrintUTF8Character(pc, u32BufferSize, sstKeyboard.au32CharBuffer[sstKeyboard.u32CharReadIndex]);

    /* Updates buffer size */
    u32BufferSize -= u32Size;

    /* Updates char pointer */
    pc += u32Size;
  }

  /* Terminates string */
  *pc = orxCHAR_NULL;

  /* Done! */
  return zResult;
}

extern "C" void orxFASTCALL orxKeyboard_Android_ClearBuffer()
{
  /* Checks */
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  /* Clears all buffer indices */
  sstKeyboard.u32KeyReadIndex   =
  sstKeyboard.u32KeyWriteIndex  =
  sstKeyboard.u32CharReadIndex  =
  sstKeyboard.u32CharWriteIndex = 0;

  /* Done! */
  return;
}

extern "C" orxSTATUS orxFASTCALL orxKeyboard_Android_Show(orxBOOL _bShow)
{
#ifdef __orxANDROID__
  JNIEnv *env = (JNIEnv*) orxAndroid_GetJNIEnv();
  jobject jActivity = orxAndroid_GetActivity();
  env->CallVoidMethod(jActivity, sstKeyboard.midShowKeyboard, _bShow == orxTRUE ? JNI_TRUE : JNI_FALSE);
  env->DeleteLocalRef(jActivity);
#endif

  /* Done */
  return orxSTATUS_SUCCESS;
}

/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(KEYBOARD);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_Init, KEYBOARD, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_Exit, KEYBOARD, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_IsKeyPressed, KEYBOARD, IS_KEY_PRESSED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_GetKeyDisplayName, KEYBOARD, GET_KEY_DISPLAY_NAME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_ReadKey, KEYBOARD, READ_KEY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_ReadString, KEYBOARD, READ_STRING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_ClearBuffer, KEYBOARD, CLEAR_BUFFER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_Show, KEYBOARD, SHOW);
orxPLUGIN_USER_CORE_FUNCTION_END();
