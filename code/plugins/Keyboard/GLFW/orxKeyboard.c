/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2010 Orx-Project
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
 * GLFW keyboard plugin implementation
 *
 */


#include "orxPluginAPI.h"

#include "GL/glfw.h"

#ifndef __orxEMBEDDED__
  #ifdef __orxMSVC__
    #pragma message("!!WARNING!! This plugin will only work in non-embedded mode when linked against a *DYNAMIC* version of GLFW!")
  #else /* __orxMSVC__ */
    #warning !!WARNING!! This plugin will only work in non-embedded mode when linked against a *DYNAMIC* version of SDL!
  #endif /* __orxMSVC__ */
#endif /* __orxEMBEDDED__ */


/** Module flags
 */
#define orxKEYBOARD_KU32_STATIC_FLAG_NONE       0x00000000 /**< No flags */

#define orxKEYBOARD_KU32_STATIC_FLAG_READY      0x00000001 /**< Ready flag */

#define orxKEYBOARD_KU32_STATIC_MASK_ALL        0xFFFFFFFF /**< All mask */

/** GLFWKey typedef
 */
typedef int GLFWKey;

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxKEYBOARD_STATIC_t
{
  orxU32            u32Flags;
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

static GLFWKey orxFASTCALL orxKeyboard_GLFW_GetGLFWKey(orxKEYBOARD_KEY _eKey)
{
  GLFWKey eResult;

  /* Depending on key */
  switch(_eKey)
  {
    case orxKEYBOARD_KEY_ESCAPE:        {eResult = GLFW_KEY_ESC; break;}
    case orxKEYBOARD_KEY_SPACE:         {eResult = GLFW_KEY_SPACE; break;}
    case orxKEYBOARD_KEY_RETURN:        {eResult = GLFW_KEY_ENTER; break;}
    case orxKEYBOARD_KEY_BACKSPACE:     {eResult = GLFW_KEY_BACKSPACE; break;}
    case orxKEYBOARD_KEY_TAB:           {eResult = GLFW_KEY_TAB; break;}
    case orxKEYBOARD_KEY_PAGEUP:        {eResult = GLFW_KEY_PAGEUP; break;}
    case orxKEYBOARD_KEY_PAGEDOWN:      {eResult = GLFW_KEY_PAGEDOWN; break;}
    case orxKEYBOARD_KEY_END:           {eResult = GLFW_KEY_END; break;}
    case orxKEYBOARD_KEY_HOME:          {eResult = GLFW_KEY_HOME; break;}
    case orxKEYBOARD_KEY_INSERT:        {eResult = GLFW_KEY_INSERT; break;}
    case orxKEYBOARD_KEY_DELETE:        {eResult = GLFW_KEY_DEL; break;}
    case orxKEYBOARD_KEY_ADD:           {eResult = GLFW_KEY_KP_ADD; break;}
    case orxKEYBOARD_KEY_SUBTRACT:      {eResult = GLFW_KEY_KP_SUBTRACT; break;}
    case orxKEYBOARD_KEY_MULTIPLY:      {eResult = GLFW_KEY_KP_MULTIPLY; break;}
    case orxKEYBOARD_KEY_DIVIDE:        {eResult = GLFW_KEY_KP_DIVIDE; break;}
    case orxKEYBOARD_KEY_PAUSE:         {eResult = GLFW_KEY_PAUSE; break;}
    case orxKEYBOARD_KEY_RALT:          {eResult = GLFW_KEY_RALT; break;}
    case orxKEYBOARD_KEY_RCTRL:         {eResult = GLFW_KEY_RCTRL; break;}
    case orxKEYBOARD_KEY_RSHIFT:        {eResult = GLFW_KEY_RSHIFT; break;}
    case orxKEYBOARD_KEY_RSYSTEM:       {eResult = GLFW_KEY_RSUPER; break;}
    case orxKEYBOARD_KEY_LALT:          {eResult = GLFW_KEY_LALT; break;}
    case orxKEYBOARD_KEY_LCTRL:         {eResult = GLFW_KEY_LCTRL; break;}
    case orxKEYBOARD_KEY_LSHIFT:        {eResult = GLFW_KEY_LSHIFT; break;}
    case orxKEYBOARD_KEY_LSYSTEM:       {eResult = GLFW_KEY_LSUPER; break;}
    case orxKEYBOARD_KEY_MENU:          {eResult = GLFW_KEY_MENU; break;}
    case orxKEYBOARD_KEY_LBRACKET:      {eResult = (GLFWKey)'['; break;}
    case orxKEYBOARD_KEY_RBRACKET:      {eResult = (GLFWKey)']'; break;}
    case orxKEYBOARD_KEY_SEMICOLON:     {eResult = (GLFWKey)';'; break;}
    case orxKEYBOARD_KEY_COMMA:         {eResult = (GLFWKey)','; break;}
    case orxKEYBOARD_KEY_PERIOD:        {eResult = (GLFWKey)'.'; break;}
    case orxKEYBOARD_KEY_QUOTE:         {eResult = (GLFWKey)'\''; break;}
    case orxKEYBOARD_KEY_SLASH:         {eResult = (GLFWKey)'/'; break;}
    case orxKEYBOARD_KEY_BACKSLASH:     {eResult = (GLFWKey)'\\'; break;}
    case orxKEYBOARD_KEY_TILDE:         {eResult = (GLFWKey)'`'; break;}
    case orxKEYBOARD_KEY_EQUAL:         {eResult = (GLFWKey)'='; break;}
    case orxKEYBOARD_KEY_DASH:          {eResult = (GLFWKey)'-'; break;}
    case orxKEYBOARD_KEY_F1:            {eResult = GLFW_KEY_F1; break;}
    case orxKEYBOARD_KEY_F2:            {eResult = GLFW_KEY_F2; break;}
    case orxKEYBOARD_KEY_F3:            {eResult = GLFW_KEY_F3; break;}
    case orxKEYBOARD_KEY_F4:            {eResult = GLFW_KEY_F4; break;}
    case orxKEYBOARD_KEY_F5:            {eResult = GLFW_KEY_F5; break;}
    case orxKEYBOARD_KEY_F6:            {eResult = GLFW_KEY_F6; break;}
    case orxKEYBOARD_KEY_F7:            {eResult = GLFW_KEY_F7; break;}
    case orxKEYBOARD_KEY_F8:            {eResult = GLFW_KEY_F8; break;}
    case orxKEYBOARD_KEY_F9:            {eResult = GLFW_KEY_F9; break;}
    case orxKEYBOARD_KEY_F10:           {eResult = GLFW_KEY_F10; break;}
    case orxKEYBOARD_KEY_F11:           {eResult = GLFW_KEY_F11; break;}
    case orxKEYBOARD_KEY_F12:           {eResult = GLFW_KEY_F12; break;}
    case orxKEYBOARD_KEY_F13:           {eResult = GLFW_KEY_F13; break;}
    case orxKEYBOARD_KEY_F14:           {eResult = GLFW_KEY_F14; break;}
    case orxKEYBOARD_KEY_F15:           {eResult = GLFW_KEY_F15; break;}
    case orxKEYBOARD_KEY_UP:            {eResult = GLFW_KEY_UP; break;}
    case orxKEYBOARD_KEY_RIGHT:         {eResult = GLFW_KEY_RIGHT; break;}
    case orxKEYBOARD_KEY_DOWN:          {eResult = GLFW_KEY_DOWN; break;}
    case orxKEYBOARD_KEY_LEFT:          {eResult = GLFW_KEY_LEFT; break;}
    case orxKEYBOARD_KEY_NUMPAD_0:      {eResult = GLFW_KEY_KP_0; break;}
    case orxKEYBOARD_KEY_NUMPAD_1:      {eResult = GLFW_KEY_KP_1; break;}
    case orxKEYBOARD_KEY_NUMPAD_2:      {eResult = GLFW_KEY_KP_2; break;}
    case orxKEYBOARD_KEY_NUMPAD_3:      {eResult = GLFW_KEY_KP_3; break;}
    case orxKEYBOARD_KEY_NUMPAD_4:      {eResult = GLFW_KEY_KP_4; break;}
    case orxKEYBOARD_KEY_NUMPAD_5:      {eResult = GLFW_KEY_KP_5; break;}
    case orxKEYBOARD_KEY_NUMPAD_6:      {eResult = GLFW_KEY_KP_6; break;}
    case orxKEYBOARD_KEY_NUMPAD_7:      {eResult = GLFW_KEY_KP_7; break;}
    case orxKEYBOARD_KEY_NUMPAD_8:      {eResult = GLFW_KEY_KP_8; break;}
    case orxKEYBOARD_KEY_NUMPAD_9:      {eResult = GLFW_KEY_KP_9; break;}
    case orxKEYBOARD_KEY_NUMPAD_RETURN: {eResult = GLFW_KEY_KP_ENTER; break;}
    case orxKEYBOARD_KEY_NUMPAD_DECIMAL:{eResult = GLFW_KEY_KP_DECIMAL; break;}
    case orxKEYBOARD_KEY_NUM_LOCK:      {eResult = GLFW_KEY_KP_NUM_LOCK; break;}
    case orxKEYBOARD_KEY_SCROLL_LOCK:   {eResult = GLFW_KEY_SCROLL_LOCK; break;}
    case orxKEYBOARD_KEY_CAPS_LOCK:     {eResult = GLFW_KEY_CAPS_LOCK; break;}
    case orxKEYBOARD_KEY_A:             {eResult = (GLFWKey)'A'; break;}
    case orxKEYBOARD_KEY_Z:             {eResult = (GLFWKey)'Z'; break;}
    case orxKEYBOARD_KEY_E:             {eResult = (GLFWKey)'E'; break;}
    case orxKEYBOARD_KEY_R:             {eResult = (GLFWKey)'R'; break;}
    case orxKEYBOARD_KEY_T:             {eResult = (GLFWKey)'T'; break;}
    case orxKEYBOARD_KEY_Y:             {eResult = (GLFWKey)'Y'; break;}
    case orxKEYBOARD_KEY_U:             {eResult = (GLFWKey)'U'; break;}
    case orxKEYBOARD_KEY_I:             {eResult = (GLFWKey)'I'; break;}
    case orxKEYBOARD_KEY_O:             {eResult = (GLFWKey)'O'; break;}
    case orxKEYBOARD_KEY_P:             {eResult = (GLFWKey)'P'; break;}
    case orxKEYBOARD_KEY_Q:             {eResult = (GLFWKey)'Q'; break;}
    case orxKEYBOARD_KEY_S:             {eResult = (GLFWKey)'S'; break;}
    case orxKEYBOARD_KEY_D:             {eResult = (GLFWKey)'D'; break;}
    case orxKEYBOARD_KEY_F:             {eResult = (GLFWKey)'F'; break;}
    case orxKEYBOARD_KEY_G:             {eResult = (GLFWKey)'G'; break;}
    case orxKEYBOARD_KEY_H:             {eResult = (GLFWKey)'H'; break;}
    case orxKEYBOARD_KEY_J:             {eResult = (GLFWKey)'J'; break;}
    case orxKEYBOARD_KEY_K:             {eResult = (GLFWKey)'k'; break;}
    case orxKEYBOARD_KEY_L:             {eResult = (GLFWKey)'L'; break;}
    case orxKEYBOARD_KEY_M:             {eResult = (GLFWKey)'M'; break;}
    case orxKEYBOARD_KEY_W:             {eResult = (GLFWKey)'W'; break;}
    case orxKEYBOARD_KEY_X:             {eResult = (GLFWKey)'X'; break;}
    case orxKEYBOARD_KEY_C:             {eResult = (GLFWKey)'C'; break;}
    case orxKEYBOARD_KEY_V:             {eResult = (GLFWKey)'V'; break;}
    case orxKEYBOARD_KEY_B:             {eResult = (GLFWKey)'B'; break;}
    case orxKEYBOARD_KEY_N:             {eResult = (GLFWKey)'N'; break;}
    case orxKEYBOARD_KEY_0:             {eResult = (GLFWKey)'0'; break;}
    case orxKEYBOARD_KEY_1:             {eResult = (GLFWKey)'1'; break;}
    case orxKEYBOARD_KEY_2:             {eResult = (GLFWKey)'2'; break;}
    case orxKEYBOARD_KEY_3:             {eResult = (GLFWKey)'3'; break;}
    case orxKEYBOARD_KEY_4:             {eResult = (GLFWKey)'4'; break;}
    case orxKEYBOARD_KEY_5:             {eResult = (GLFWKey)'5'; break;}
    case orxKEYBOARD_KEY_6:             {eResult = (GLFWKey)'6'; break;}
    case orxKEYBOARD_KEY_7:             {eResult = (GLFWKey)'7'; break;}
    case orxKEYBOARD_KEY_8:             {eResult = (GLFWKey)'8'; break;}
    case orxKEYBOARD_KEY_9:             {eResult = (GLFWKey)'9'; break;}
    default:                            {eResult = GLFW_KEY_UNKNOWN; break;}
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxKeyboard_GLFW_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Was already initialized. */
  if(!(sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstKeyboard, sizeof(orxKEYBOARD_STATIC));

    /* Is GLFW window opened? */
    if(glfwGetWindowParam(GLFW_OPENED) != GL_FALSE)
    {
      /* Updates status */
      sstKeyboard.u32Flags |= orxKEYBOARD_KU32_STATIC_FLAG_READY;

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
  }

  /* Done! */
  return eResult;
}

void orxFASTCALL orxKeyboard_GLFW_Exit()
{
  /* Was initialized? */
  if(sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY)
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstKeyboard, sizeof(orxKEYBOARD_STATIC));
  }

  return;
}

orxBOOL orxFASTCALL orxKeyboard_GLFW_IsKeyPressed(orxKEYBOARD_KEY _eKey)
{
  GLFWKey eGLFWKey;
  orxBOOL bResult;

  /* Checks */
  orxASSERT(_eKey < orxKEYBOARD_KEY_NUMBER);
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  /* Gets GLFW key enum */
  eGLFWKey = orxKeyboard_GLFW_GetGLFWKey(_eKey);

  /* Valid? */
  if(eGLFWKey != GLFW_KEY_UNKNOWN)
  {
    /* Updates result */
    bResult = (glfwGetKey((int)eGLFWKey) != GLFW_RELEASE) ? orxTRUE : orxFALSE;
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

orxKEYBOARD_KEY orxFASTCALL orxKeyboard_GLFW_Read()
{
  orxU32          i;
  orxKEYBOARD_KEY eResult = orxKEYBOARD_KEY_NONE;

  /* Checks */
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  /* For all keys */
  for(i = 0; i < orxKEYBOARD_KEY_NUMBER; i++)
  {
    GLFWKey eGLFWKey;

    /* Gets GLFW key enum */
    eGLFWKey = orxKeyboard_GLFW_GetGLFWKey((orxKEYBOARD_KEY)i);

    /* Is pressed? */
    if(glfwGetKey((int)eGLFWKey) != GLFW_RELEASE)
    {
      /* Updates result */
      eResult = (orxKEYBOARD_KEY)i;

      break;
    }
  }

  /* Done! */
  return eResult;
}

orxBOOL orxFASTCALL orxKeyboard_GLFW_Hit()
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  /* Updates result */
  bResult = (orxKeyboard_GLFW_Read() != orxKEYBOARD_KEY_NONE) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

void orxFASTCALL orxKeyboard_GLFW_ClearBuffer()
{
  /* Checks */
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  /* Not implemented yet */
  orxASSERT(orxFALSE && "Clear keyboard buffer is not supported by this plugin!");

  /* Done! */
  return;
}

/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(KEYBOARD);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_GLFW_Init, KEYBOARD, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_GLFW_Exit, KEYBOARD, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_GLFW_IsKeyPressed, KEYBOARD, IS_KEY_PRESSED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_GLFW_Hit, KEYBOARD, HIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_GLFW_Read, KEYBOARD, READ);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_GLFW_ClearBuffer, KEYBOARD, CLEAR_BUFFER);
orxPLUGIN_USER_CORE_FUNCTION_END();
