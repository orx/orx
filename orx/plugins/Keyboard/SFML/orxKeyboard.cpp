/**
 * @file orxKeyboard.cpp
 *
 * SFML keyboard plugin
 */

 /***************************************************************************
 begin                : 12/01/2008
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/

extern "C"
{
  #include "orxInclude.h"

  #include "plugin/orxPluginUser.h"
  #include "plugin/orxPlugin.h"

  #include "io/orxKeyboard.h"
  #include "display/orxDisplay.h"
}

#include <SFML/Graphics.hpp>


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
  sf::Input        *poInput;
} orxKEYBOARD_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
orxSTATIC orxKEYBOARD_STATIC sstKeyboard;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

orxSTATIC orxFASTCALL sf::Key::Code orxKeyboard_SFML_GetSFMLKey(orxKEYBOARD_KEY _eKey)
{
  sf::Key::Code eResult;

  /* Depending on key */
  switch(_eKey)
  {
    case orxKEYBOARD_KEY_ESCAPE:        {eResult = sf::Key::Escape; break;}
    case orxKEYBOARD_KEY_SPACE:         {eResult = sf::Key::Space; break;}
    case orxKEYBOARD_KEY_RETURN:        {eResult = sf::Key::Return; break;}
    case orxKEYBOARD_KEY_BACK:          {eResult = sf::Key::Back; break;}
    case orxKEYBOARD_KEY_TAB:           {eResult = sf::Key::Tab; break;}
    case orxKEYBOARD_KEY_PAGEUP:        {eResult = sf::Key::PageUp; break;}
    case orxKEYBOARD_KEY_PAGEDOWN:      {eResult = sf::Key::PageDown; break;}
    case orxKEYBOARD_KEY_END:           {eResult = sf::Key::End; break;}
    case orxKEYBOARD_KEY_HOME:          {eResult = sf::Key::Home; break;}
    case orxKEYBOARD_KEY_INSERT:        {eResult = sf::Key::Insert; break;}
    case orxKEYBOARD_KEY_DELETE:        {eResult = sf::Key::Delete; break;}
    case orxKEYBOARD_KEY_ADD:           {eResult = sf::Key::Add; break;}
    case orxKEYBOARD_KEY_SUBTRACT:      {eResult = sf::Key::Subtract; break;}
    case orxKEYBOARD_KEY_MULTIPLY:      {eResult = sf::Key::Multiply; break;}
    case orxKEYBOARD_KEY_DIVIDE:        {eResult = sf::Key::Divide; break;}
    case orxKEYBOARD_KEY_PAUSE:         {eResult = sf::Key::Pause; break;}
    case orxKEYBOARD_KEY_LEFT_CONTROL:  {eResult = sf::Key::LControl; break;}
    case orxKEYBOARD_KEY_LEFT_SHIFT:    {eResult = sf::Key::LShift; break;}
    case orxKEYBOARD_KEY_LEFT_ALT:      {eResult = sf::Key::LAlt; break;}
    case orxKEYBOARD_KEY_LEFT_SYSTEM:   {eResult = sf::Key::LSystem; break;}
    case orxKEYBOARD_KEY_RIGHT_CONTROL: {eResult = sf::Key::RControl; break;}
    case orxKEYBOARD_KEY_RIGHT_SHIFT:   {eResult = sf::Key::RShift; break;}
    case orxKEYBOARD_KEY_RIGHT_ALT:     {eResult = sf::Key::RAlt; break;}
    case orxKEYBOARD_KEY_RIGHT_SYSTEM:  {eResult = sf::Key::RSystem; break;}
    case orxKEYBOARD_KEY_MENU:          {eResult = sf::Key::Menu; break;}
    case orxKEYBOARD_KEY_LEFT_BRACKET:  {eResult = sf::Key::LBracket; break;}
    case orxKEYBOARD_KEY_RIGHT_BRACKET: {eResult = sf::Key::RBracket; break;}
    case orxKEYBOARD_KEY_SEMICOLON:     {eResult = sf::Key::SemiColon; break;}
    case orxKEYBOARD_KEY_COMMA:         {eResult = sf::Key::Comma; break;}
    case orxKEYBOARD_KEY_PERIOD:        {eResult = sf::Key::Period; break;}
    case orxKEYBOARD_KEY_QUOTE:         {eResult = sf::Key::Quote; break;}
    case orxKEYBOARD_KEY_SLASH:         {eResult = sf::Key::Slash; break;}
    case orxKEYBOARD_KEY_BACKSLASH:     {eResult = sf::Key::BackSlash; break;}
    case orxKEYBOARD_KEY_TILDE:         {eResult = sf::Key::Tilde; break;}
    case orxKEYBOARD_KEY_EQUAL:         {eResult = sf::Key::Equal; break;}
    case orxKEYBOARD_KEY_DASH:          {eResult = sf::Key::Dash; break;}
    case orxKEYBOARD_KEY_F1:            {eResult = sf::Key::F1; break;}
    case orxKEYBOARD_KEY_F2:            {eResult = sf::Key::F2; break;}
    case orxKEYBOARD_KEY_F3:            {eResult = sf::Key::F3; break;}
    case orxKEYBOARD_KEY_F4:            {eResult = sf::Key::F4; break;}
    case orxKEYBOARD_KEY_F5:            {eResult = sf::Key::F5; break;}
    case orxKEYBOARD_KEY_F6:            {eResult = sf::Key::F6; break;}
    case orxKEYBOARD_KEY_F7:            {eResult = sf::Key::F7; break;}
    case orxKEYBOARD_KEY_F8:            {eResult = sf::Key::F8; break;}
    case orxKEYBOARD_KEY_F9:            {eResult = sf::Key::F9; break;}
    case orxKEYBOARD_KEY_F10:           {eResult = sf::Key::F10; break;}
    case orxKEYBOARD_KEY_F11:           {eResult = sf::Key::F11; break;}
    case orxKEYBOARD_KEY_F12:           {eResult = sf::Key::F12; break;}
    case orxKEYBOARD_KEY_F13:           {eResult = sf::Key::F13; break;}
    case orxKEYBOARD_KEY_F14:           {eResult = sf::Key::F14; break;}
    case orxKEYBOARD_KEY_F15:           {eResult = sf::Key::F15; break;}
    case orxKEYBOARD_KEY_LEFT:          {eResult = sf::Key::Left; break;}
    case orxKEYBOARD_KEY_RIGHT:         {eResult = sf::Key::Right; break;}
    case orxKEYBOARD_KEY_UP:            {eResult = sf::Key::Up; break;}
    case orxKEYBOARD_KEY_DOWN:          {eResult = sf::Key::Down; break;}
    case orxKEYBOARD_KEY_NUMPAD0:       {eResult = sf::Key::Numpad0; break;}
    case orxKEYBOARD_KEY_NUMPAD1:       {eResult = sf::Key::Numpad1; break;}
    case orxKEYBOARD_KEY_NUMPAD2:       {eResult = sf::Key::Numpad2; break;}
    case orxKEYBOARD_KEY_NUMPAD3:       {eResult = sf::Key::Numpad3; break;}
    case orxKEYBOARD_KEY_NUMPAD4:       {eResult = sf::Key::Numpad4; break;}
    case orxKEYBOARD_KEY_NUMPAD5:       {eResult = sf::Key::Numpad5; break;}
    case orxKEYBOARD_KEY_NUMPAD6:       {eResult = sf::Key::Numpad6; break;}
    case orxKEYBOARD_KEY_NUMPAD7:       {eResult = sf::Key::Numpad7; break;}
    case orxKEYBOARD_KEY_NUMPAD8:       {eResult = sf::Key::Numpad8; break;}
    case orxKEYBOARD_KEY_NUMPAD9:       {eResult = sf::Key::Numpad9; break;}
    case orxKEYBOARD_KEY_A:             {eResult = sf::Key::A; break;}
    case orxKEYBOARD_KEY_Z:             {eResult = sf::Key::Z; break;}
    case orxKEYBOARD_KEY_E:             {eResult = sf::Key::E; break;}
    case orxKEYBOARD_KEY_R:             {eResult = sf::Key::R; break;}
    case orxKEYBOARD_KEY_T:             {eResult = sf::Key::T; break;}
    case orxKEYBOARD_KEY_Y:             {eResult = sf::Key::Y; break;}
    case orxKEYBOARD_KEY_U:             {eResult = sf::Key::U; break;}
    case orxKEYBOARD_KEY_I:             {eResult = sf::Key::I; break;}
    case orxKEYBOARD_KEY_O:             {eResult = sf::Key::O; break;}
    case orxKEYBOARD_KEY_P:             {eResult = sf::Key::P; break;}
    case orxKEYBOARD_KEY_Q:             {eResult = sf::Key::Q; break;}
    case orxKEYBOARD_KEY_S:             {eResult = sf::Key::S; break;}
    case orxKEYBOARD_KEY_D:             {eResult = sf::Key::D; break;}
    case orxKEYBOARD_KEY_F:             {eResult = sf::Key::F; break;}
    case orxKEYBOARD_KEY_G:             {eResult = sf::Key::G; break;}
    case orxKEYBOARD_KEY_H:             {eResult = sf::Key::H; break;}
    case orxKEYBOARD_KEY_J:             {eResult = sf::Key::J; break;}
    case orxKEYBOARD_KEY_K:             {eResult = sf::Key::K; break;}
    case orxKEYBOARD_KEY_L:             {eResult = sf::Key::L; break;}
    case orxKEYBOARD_KEY_M:             {eResult = sf::Key::M; break;}
    case orxKEYBOARD_KEY_W:             {eResult = sf::Key::W; break;}
    case orxKEYBOARD_KEY_X:             {eResult = sf::Key::X; break;}
    case orxKEYBOARD_KEY_C:             {eResult = sf::Key::C; break;}
    case orxKEYBOARD_KEY_V:             {eResult = sf::Key::V; break;}
    case orxKEYBOARD_KEY_B:             {eResult = sf::Key::B; break;}
    case orxKEYBOARD_KEY_N:             {eResult = sf::Key::N; break;}
    case orxKEYBOARD_KEY_0:             {eResult = sf::Key::Num0; break;}
    case orxKEYBOARD_KEY_1:             {eResult = sf::Key::Num1; break;}
    case orxKEYBOARD_KEY_2:             {eResult = sf::Key::Num2; break;}
    case orxKEYBOARD_KEY_3:             {eResult = sf::Key::Num3; break;}
    case orxKEYBOARD_KEY_4:             {eResult = sf::Key::Num4; break;}
    case orxKEYBOARD_KEY_5:             {eResult = sf::Key::Num5; break;}
    case orxKEYBOARD_KEY_6:             {eResult = sf::Key::Num6; break;}
    case orxKEYBOARD_KEY_7:             {eResult = sf::Key::Num7; break;}
    case orxKEYBOARD_KEY_8:             {eResult = sf::Key::Num8; break;}
    case orxKEYBOARD_KEY_9:             {eResult = sf::Key::Num9; break;}
    default:                            {eResult = sf::Key::Count; break;}
  }

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxKeyboard_SFML_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Was already initialized. */
  if(!(sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstKeyboard, sizeof(orxKEYBOARD_STATIC));

    /* Terrible hack : gets application input from SFML display plugin */
    sstKeyboard.poInput = (sf::Input *)orxDisplay_GetApplicationInput();

    /* Valid? */
    if(sstKeyboard.poInput != orxNULL)
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

extern "C" orxVOID orxKeyboard_SFML_Exit()
{
  /* Was initialized? */
  if(sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY)
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstKeyboard, sizeof(orxKEYBOARD_STATIC));
  }

  return;
}

extern "C" orxBOOL orxKeyboard_SFML_IsKeyPressed(orxKEYBOARD_KEY _eKey)
{
  orxBOOL       bResult;
  sf::Key::Code eSFMLKey;

  /* Checks */
  orxASSERT(_eKey < orxKEYBOARD_KEY_NUMBER);
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  /* Gets SFML key enum */
  eSFMLKey = orxKeyboard_SFML_GetSFMLKey(_eKey);

  /* Updates result */
  bResult = sstKeyboard.poInput->IsKeyDown(eSFMLKey) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

extern "C" orxKEYBOARD_KEY orxKeyboard_SFML_Read()
{
  orxU32          i;
  orxKEYBOARD_KEY eResult = orxKEYBOARD_KEY_NONE;

  /* Checks */
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  /* For all keys */
  for(i = 0; i < orxKEYBOARD_KEY_NUMBER; i++)
  {
    /* Is pressed? */
    if(orxKeyboard_SFML_IsKeyPressed((orxKEYBOARD_KEY)i) != orxFALSE)
    {
      /* Updates result */
      eResult = (orxKEYBOARD_KEY)i;

      break;
    }
  }

  /* Done! */
  return eResult;
}

extern "C" orxBOOL orxKeyboard_SFML_Hit()
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  /* Updates result */
  bResult = (orxKeyboard_SFML_Read() != orxKEYBOARD_KEY_NONE) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

extern "C" orxVOID orxKeyboard_SFML_ClearBuffer()
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
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_SFML_Init, KEYBOARD, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_SFML_Exit, KEYBOARD, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_SFML_IsKeyPressed, KEYBOARD, IS_KEY_PRESSED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_SFML_Hit, KEYBOARD, HIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_SFML_Read, KEYBOARD, READ);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_SFML_ClearBuffer, KEYBOARD, CLEAR_BUFFER);
orxPLUGIN_USER_CORE_FUNCTION_END();
