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
 * @file orxKeyboard.c
 * @date 06/01/2009
 * @author iarwain@orx-project.org
 *
 * SDL keyboard plugin implementation
 *
 */


#include "orxPluginAPI.h"

#include <SDL.h>


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

static SDLKey orxFASTCALL orxKeyboard_SDL_GetSDLKey(orxKEYBOARD_KEY _eKey)
{
  SDLKey eResult;

  /* Depending on key */
  switch(_eKey)
  {
    case orxKEYBOARD_KEY_ESCAPE:        {eResult = SDLK_ESCAPE; break;}
    case orxKEYBOARD_KEY_SPACE:         {eResult = SDLK_SPACE; break;}
    case orxKEYBOARD_KEY_RETURN:        {eResult = SDLK_RETURN; break;}
    case orxKEYBOARD_KEY_BACKSPACE:     {eResult = SDLK_BACKSPACE; break;}
    case orxKEYBOARD_KEY_TAB:           {eResult = SDLK_TAB; break;}
    case orxKEYBOARD_KEY_PAGEUP:        {eResult = SDLK_PAGEUP; break;}
    case orxKEYBOARD_KEY_PAGEDOWN:      {eResult = SDLK_PAGEDOWN; break;}
    case orxKEYBOARD_KEY_END:           {eResult = SDLK_END; break;}
    case orxKEYBOARD_KEY_HOME:          {eResult = SDLK_HOME; break;}
    case orxKEYBOARD_KEY_INSERT:        {eResult = SDLK_INSERT; break;}
    case orxKEYBOARD_KEY_DELETE:        {eResult = SDLK_DELETE; break;}
    case orxKEYBOARD_KEY_ADD:           {eResult = SDLK_KP_PLUS; break;}
    case orxKEYBOARD_KEY_SUBTRACT:      {eResult = SDLK_KP_MINUS; break;}
    case orxKEYBOARD_KEY_MULTIPLY:      {eResult = SDLK_KP_MULTIPLY; break;}
    case orxKEYBOARD_KEY_DIVIDE:        {eResult = SDLK_KP_DIVIDE; break;}
    case orxKEYBOARD_KEY_PAUSE:         {eResult = SDLK_PAUSE; break;}
    case orxKEYBOARD_KEY_RALT:          {eResult = SDLK_RALT; break;}
    case orxKEYBOARD_KEY_RCTRL:         {eResult = SDLK_RCTRL; break;}
    case orxKEYBOARD_KEY_RSHIFT:        {eResult = SDLK_RSHIFT; break;}
    case orxKEYBOARD_KEY_RSYSTEM:       {eResult = SDLK_RSUPER; break;}
    case orxKEYBOARD_KEY_LALT:          {eResult = SDLK_LALT; break;}
    case orxKEYBOARD_KEY_LCTRL:         {eResult = SDLK_LCTRL; break;}
    case orxKEYBOARD_KEY_LSHIFT:        {eResult = SDLK_LSHIFT; break;}
    case orxKEYBOARD_KEY_LSYSTEM:       {eResult = SDLK_LSUPER; break;}
    case orxKEYBOARD_KEY_MENU:          {eResult = SDLK_MENU; break;}
    case orxKEYBOARD_KEY_LBRACKET:      {eResult = SDLK_LEFTBRACKET; break;}
    case orxKEYBOARD_KEY_RBRACKET:      {eResult = SDLK_RIGHTBRACKET; break;}
    case orxKEYBOARD_KEY_SEMICOLON:     {eResult = SDLK_SEMICOLON; break;}
    case orxKEYBOARD_KEY_COMMA:         {eResult = SDLK_COMMA; break;}
    case orxKEYBOARD_KEY_PERIOD:        {eResult = SDLK_PERIOD; break;}
    case orxKEYBOARD_KEY_QUOTE:         {eResult = SDLK_QUOTE; break;}
    case orxKEYBOARD_KEY_SLASH:         {eResult = SDLK_SLASH; break;}
    case orxKEYBOARD_KEY_BACKSLASH:     {eResult = SDLK_BACKSLASH; break;}
    case orxKEYBOARD_KEY_TILDE:         {eResult = SDLK_BACKQUOTE; break;}
    case orxKEYBOARD_KEY_EQUAL:         {eResult = SDLK_EQUALS; break;}
    case orxKEYBOARD_KEY_DASH:          {eResult = SDLK_UNDERSCORE; break;}
    case orxKEYBOARD_KEY_F1:            {eResult = SDLK_F1; break;}
    case orxKEYBOARD_KEY_F2:            {eResult = SDLK_F2; break;}
    case orxKEYBOARD_KEY_F3:            {eResult = SDLK_F3; break;}
    case orxKEYBOARD_KEY_F4:            {eResult = SDLK_F4; break;}
    case orxKEYBOARD_KEY_F5:            {eResult = SDLK_F5; break;}
    case orxKEYBOARD_KEY_F6:            {eResult = SDLK_F6; break;}
    case orxKEYBOARD_KEY_F7:            {eResult = SDLK_F7; break;}
    case orxKEYBOARD_KEY_F8:            {eResult = SDLK_F8; break;}
    case orxKEYBOARD_KEY_F9:            {eResult = SDLK_F9; break;}
    case orxKEYBOARD_KEY_F10:           {eResult = SDLK_F10; break;}
    case orxKEYBOARD_KEY_F11:           {eResult = SDLK_F11; break;}
    case orxKEYBOARD_KEY_F12:           {eResult = SDLK_F12; break;}
    case orxKEYBOARD_KEY_F13:           {eResult = SDLK_F13; break;}
    case orxKEYBOARD_KEY_F14:           {eResult = SDLK_F14; break;}
    case orxKEYBOARD_KEY_F15:           {eResult = SDLK_F15; break;}
    case orxKEYBOARD_KEY_UP:            {eResult = SDLK_UP; break;}
    case orxKEYBOARD_KEY_RIGHT:         {eResult = SDLK_RIGHT; break;}
    case orxKEYBOARD_KEY_DOWN:          {eResult = SDLK_DOWN; break;}
    case orxKEYBOARD_KEY_LEFT:          {eResult = SDLK_LEFT; break;}
    case orxKEYBOARD_KEY_NUMPAD0:       {eResult = SDLK_KP0; break;}
    case orxKEYBOARD_KEY_NUMPAD1:       {eResult = SDLK_KP1; break;}
    case orxKEYBOARD_KEY_NUMPAD2:       {eResult = SDLK_KP2; break;}
    case orxKEYBOARD_KEY_NUMPAD3:       {eResult = SDLK_KP3; break;}
    case orxKEYBOARD_KEY_NUMPAD4:       {eResult = SDLK_KP4; break;}
    case orxKEYBOARD_KEY_NUMPAD5:       {eResult = SDLK_KP5; break;}
    case orxKEYBOARD_KEY_NUMPAD6:       {eResult = SDLK_KP6; break;}
    case orxKEYBOARD_KEY_NUMPAD7:       {eResult = SDLK_KP7; break;}
    case orxKEYBOARD_KEY_NUMPAD8:       {eResult = SDLK_KP8; break;}
    case orxKEYBOARD_KEY_NUMPAD9:       {eResult = SDLK_KP9; break;}
    case orxKEYBOARD_KEY_A:             {eResult = SDLK_a; break;}
    case orxKEYBOARD_KEY_Z:             {eResult = SDLK_z; break;}
    case orxKEYBOARD_KEY_E:             {eResult = SDLK_e; break;}
    case orxKEYBOARD_KEY_R:             {eResult = SDLK_r; break;}
    case orxKEYBOARD_KEY_T:             {eResult = SDLK_t; break;}
    case orxKEYBOARD_KEY_Y:             {eResult = SDLK_y; break;}
    case orxKEYBOARD_KEY_U:             {eResult = SDLK_u; break;}
    case orxKEYBOARD_KEY_I:             {eResult = SDLK_i; break;}
    case orxKEYBOARD_KEY_O:             {eResult = SDLK_o; break;}
    case orxKEYBOARD_KEY_P:             {eResult = SDLK_p; break;}
    case orxKEYBOARD_KEY_Q:             {eResult = SDLK_q; break;}
    case orxKEYBOARD_KEY_S:             {eResult = SDLK_s; break;}
    case orxKEYBOARD_KEY_D:             {eResult = SDLK_d; break;}
    case orxKEYBOARD_KEY_F:             {eResult = SDLK_f; break;}
    case orxKEYBOARD_KEY_G:             {eResult = SDLK_g; break;}
    case orxKEYBOARD_KEY_H:             {eResult = SDLK_h; break;}
    case orxKEYBOARD_KEY_J:             {eResult = SDLK_j; break;}
    case orxKEYBOARD_KEY_K:             {eResult = SDLK_k; break;}
    case orxKEYBOARD_KEY_L:             {eResult = SDLK_l; break;}
    case orxKEYBOARD_KEY_M:             {eResult = SDLK_m; break;}
    case orxKEYBOARD_KEY_W:             {eResult = SDLK_w; break;}
    case orxKEYBOARD_KEY_X:             {eResult = SDLK_x; break;}
    case orxKEYBOARD_KEY_C:             {eResult = SDLK_c; break;}
    case orxKEYBOARD_KEY_V:             {eResult = SDLK_v; break;}
    case orxKEYBOARD_KEY_B:             {eResult = SDLK_b; break;}
    case orxKEYBOARD_KEY_N:             {eResult = SDLK_n; break;}
    case orxKEYBOARD_KEY_0:             {eResult = SDLK_0; break;}
    case orxKEYBOARD_KEY_1:             {eResult = SDLK_1; break;}
    case orxKEYBOARD_KEY_2:             {eResult = SDLK_2; break;}
    case orxKEYBOARD_KEY_3:             {eResult = SDLK_3; break;}
    case orxKEYBOARD_KEY_4:             {eResult = SDLK_4; break;}
    case orxKEYBOARD_KEY_5:             {eResult = SDLK_5; break;}
    case orxKEYBOARD_KEY_6:             {eResult = SDLK_6; break;}
    case orxKEYBOARD_KEY_7:             {eResult = SDLK_7; break;}
    case orxKEYBOARD_KEY_8:             {eResult = SDLK_8; break;}
    case orxKEYBOARD_KEY_9:             {eResult = SDLK_9; break;}
    default:                            {eResult = SDLK_LAST; break;}
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxKeyboard_SDL_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Was already initialized. */
  if(!(sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstKeyboard, sizeof(orxKEYBOARD_STATIC));

    /* Is SDL partly initialized? */
    if(SDL_WasInit(SDL_INIT_EVERYTHING) != 0)
    {
      /* Not already initialized? */
      if(SDL_WasInit(SDL_INIT_VIDEO) == 0)
      {
        /* Inits the mouse subsystem */
        eResult = (SDL_InitSubSystem(SDL_INIT_VIDEO) == 0) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
      }
      else
      {
        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
    }
    else
    {
      /* Inits SDL with video */
      eResult = (SDL_Init(SDL_INIT_VIDEO) == 0) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
    }

    /* Valid? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Updates status */
      sstKeyboard.u32Flags |= orxKEYBOARD_KU32_STATIC_FLAG_READY;
    }
  }

  /* Done! */
  return eResult;
}

void orxFASTCALL orxKeyboard_SDL_Exit()
{
  /* Was initialized? */
  if(sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY)
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstKeyboard, sizeof(orxKEYBOARD_STATIC));
  }

  return;
}

orxBOOL orxFASTCALL orxKeyboard_SDL_IsKeyPressed(orxKEYBOARD_KEY _eKey)
{
  orxU8  *pu8KeyState;
  SDLKey  eSDLKey;
  orxBOOL bResult;

  /* Checks */
  orxASSERT(_eKey < orxKEYBOARD_KEY_NUMBER);
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  /* Gets SDL key enum */
  eSDLKey = orxKeyboard_SDL_GetSDLKey(_eKey);

  /* Valid? */
  if(eSDLKey != SDLK_LAST)
  {
    /* Gets key state */
    pu8KeyState = SDL_GetKeyState(orxNULL);

    /* Updates result */
    bResult = pu8KeyState[eSDLKey] ? orxTRUE : orxFALSE;
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

orxKEYBOARD_KEY orxFASTCALL orxKeyboard_SDL_Read()
{
  orxU8          *pu8KeyState;
  orxU32          i;
  orxKEYBOARD_KEY eResult = orxKEYBOARD_KEY_NONE;

  /* Checks */
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  /* Gets key state */
  pu8KeyState = SDL_GetKeyState(orxNULL);

  /* For all keys */
  for(i = 0; i < orxKEYBOARD_KEY_NUMBER; i++)
  {
    SDLKey eSDLKey;

    /* Gets SDL key enum */
    eSDLKey = orxKeyboard_SDL_GetSDLKey(i);

    /* Is pressed? */
    if(pu8KeyState[eSDLKey])
    {
      /* Updates result */
      eResult = (orxKEYBOARD_KEY)i;

      break;
    }
  }

  /* Done! */
  return eResult;
}

orxBOOL orxFASTCALL orxKeyboard_SDL_Hit()
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  /* Updates result */
  bResult = (orxKeyboard_SDL_Read() != orxKEYBOARD_KEY_NONE) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

void orxFASTCALL orxKeyboard_SDL_ClearBuffer()
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
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_SDL_Init, KEYBOARD, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_SDL_Exit, KEYBOARD, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_SDL_IsKeyPressed, KEYBOARD, IS_KEY_PRESSED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_SDL_Hit, KEYBOARD, HIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_SDL_Read, KEYBOARD, READ);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_SDL_ClearBuffer, KEYBOARD, CLEAR_BUFFER);
orxPLUGIN_USER_CORE_FUNCTION_END();
