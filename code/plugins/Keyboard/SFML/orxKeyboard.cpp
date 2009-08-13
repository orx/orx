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
 * @file orxKeyboard.cpp
 * @date 12/01/2008
 * @author iarwain@orx-project.org
 *
 * SFML keyboard plugin implementation
 *
 */


#include "orxPluginAPI.h"

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
static orxKEYBOARD_STATIC sstKeyboard;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

static sf::Key::Code orxFASTCALL orxKeyboard_SFML_GetSFMLKey(orxKEYBOARD_KEY _eKey)
{
  sf::Key::Code eResult;

  /* Depending on key */
  switch(_eKey)
  {
    case orxKEYBOARD_KEY_ESCAPE:        {eResult = sf::Key::Escape; break;}
    case orxKEYBOARD_KEY_SPACE:         {eResult = sf::Key::Space; break;}
    case orxKEYBOARD_KEY_RETURN:        {eResult = sf::Key::Return; break;}
    case orxKEYBOARD_KEY_BACKSPACE:     {eResult = sf::Key::Back; break;}
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
    case orxKEYBOARD_KEY_RALT:          {eResult = sf::Key::RAlt; break;}
    case orxKEYBOARD_KEY_RCTRL:         {eResult = sf::Key::RControl; break;}
    case orxKEYBOARD_KEY_RSHIFT:        {eResult = sf::Key::RShift; break;}
    case orxKEYBOARD_KEY_RSYSTEM:       {eResult = sf::Key::RSystem; break;}
    case orxKEYBOARD_KEY_LALT:          {eResult = sf::Key::LAlt; break;}
    case orxKEYBOARD_KEY_LCTRL:         {eResult = sf::Key::LControl; break;}
    case orxKEYBOARD_KEY_LSHIFT:        {eResult = sf::Key::LShift; break;}
    case orxKEYBOARD_KEY_LSYSTEM:       {eResult = sf::Key::LSystem; break;}
    case orxKEYBOARD_KEY_MENU:          {eResult = sf::Key::Menu; break;}
    case orxKEYBOARD_KEY_LBRACKET:      {eResult = sf::Key::LBracket; break;}
    case orxKEYBOARD_KEY_RBRACKET:      {eResult = sf::Key::RBracket; break;}
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
    case orxKEYBOARD_KEY_UP:            {eResult = sf::Key::Up; break;}
    case orxKEYBOARD_KEY_RIGHT:         {eResult = sf::Key::Right; break;}
    case orxKEYBOARD_KEY_DOWN:          {eResult = sf::Key::Down; break;}
    case orxKEYBOARD_KEY_LEFT:          {eResult = sf::Key::Left; break;}
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

static orxKEYBOARD_KEY orxFASTCALL orxKeyboard_SFML_GetKeyID(sf::Key::Code _eKey)
{
  orxKEYBOARD_KEY eResult;

  /* Depending on key */
  switch(_eKey)
  {
    case sf::Key::Escape:     {eResult = orxKEYBOARD_KEY_ESCAPE; break;}
    case sf::Key::Space:      {eResult = orxKEYBOARD_KEY_SPACE; break;}
    case sf::Key::Return:     {eResult = orxKEYBOARD_KEY_RETURN; break;}
    case sf::Key::Back:       {eResult = orxKEYBOARD_KEY_BACKSPACE; break;}
    case sf::Key::Tab:        {eResult = orxKEYBOARD_KEY_TAB; break;}
    case sf::Key::PageUp:     {eResult = orxKEYBOARD_KEY_PAGEUP; break;}
    case sf::Key::PageDown:     {eResult = orxKEYBOARD_KEY_PAGEDOWN; break;}
    case sf::Key::End:        {eResult = orxKEYBOARD_KEY_END; break;}
    case sf::Key::Home:       {eResult = orxKEYBOARD_KEY_HOME; break;}
    case sf::Key::Insert:     {eResult = orxKEYBOARD_KEY_INSERT; break;}
    case sf::Key::Delete:     {eResult = orxKEYBOARD_KEY_DELETE; break;}
    case sf::Key::Add:        {eResult = orxKEYBOARD_KEY_ADD; break;}
    case sf::Key::Subtract:   {eResult = orxKEYBOARD_KEY_SUBTRACT; break;}
    case sf::Key::Multiply:   {eResult = orxKEYBOARD_KEY_MULTIPLY; break;}
    case sf::Key::Divide:     {eResult = orxKEYBOARD_KEY_DIVIDE; break;}
    case sf::Key::Pause:      {eResult = orxKEYBOARD_KEY_PAUSE; break;}
    case sf::Key::RAlt:       {eResult = orxKEYBOARD_KEY_RALT; break;}
    case sf::Key::RControl:   {eResult = orxKEYBOARD_KEY_RCTRL; break;}
    case sf::Key::RShift:     {eResult = orxKEYBOARD_KEY_RSHIFT; break;}
    case sf::Key::RSystem:    {eResult = orxKEYBOARD_KEY_RSYSTEM; break;}
    case sf::Key::LAlt:       {eResult = orxKEYBOARD_KEY_LALT; break;}
    case sf::Key::LControl:   {eResult = orxKEYBOARD_KEY_LCTRL; break;}
    case sf::Key::LShift:     {eResult = orxKEYBOARD_KEY_LSHIFT; break;}
    case sf::Key::LSystem:    {eResult = orxKEYBOARD_KEY_LSYSTEM; break;}
    case sf::Key::Menu:       {eResult = orxKEYBOARD_KEY_MENU; break;}
    case sf::Key::LBracket:   {eResult = orxKEYBOARD_KEY_LBRACKET; break;}
    case sf::Key::RBracket:   {eResult = orxKEYBOARD_KEY_RBRACKET; break;}
    case sf::Key::SemiColon:  {eResult = orxKEYBOARD_KEY_SEMICOLON; break;}
    case sf::Key::Comma:      {eResult = orxKEYBOARD_KEY_COMMA; break;}
    case sf::Key::Period:     {eResult = orxKEYBOARD_KEY_PERIOD; break;}
    case sf::Key::Quote:      {eResult = orxKEYBOARD_KEY_QUOTE; break;}
    case sf::Key::Slash:      {eResult = orxKEYBOARD_KEY_SLASH; break;}
    case sf::Key::BackSlash:  {eResult = orxKEYBOARD_KEY_BACKSLASH; break;}
    case sf::Key::Tilde:      {eResult = orxKEYBOARD_KEY_TILDE; break;}
    case sf::Key::Equal:      {eResult = orxKEYBOARD_KEY_EQUAL; break;}
    case sf::Key::Dash:       {eResult = orxKEYBOARD_KEY_DASH; break;}
    case sf::Key::F1:         {eResult = orxKEYBOARD_KEY_F1; break;}
    case sf::Key::F2:         {eResult = orxKEYBOARD_KEY_F2; break;}
    case sf::Key::F3:         {eResult = orxKEYBOARD_KEY_F3; break;}
    case sf::Key::F4:         {eResult = orxKEYBOARD_KEY_F4; break;}
    case sf::Key::F5:         {eResult = orxKEYBOARD_KEY_F5; break;}
    case sf::Key::F6:         {eResult = orxKEYBOARD_KEY_F6; break;}
    case sf::Key::F7:         {eResult = orxKEYBOARD_KEY_F7; break;}
    case sf::Key::F8:         {eResult = orxKEYBOARD_KEY_F8; break;}
    case sf::Key::F9:         {eResult = orxKEYBOARD_KEY_F9; break;}
    case sf::Key::F10:        {eResult = orxKEYBOARD_KEY_F10; break;}
    case sf::Key::F11:        {eResult = orxKEYBOARD_KEY_F11; break;}
    case sf::Key::F12:        {eResult = orxKEYBOARD_KEY_F12; break;}
    case sf::Key::F13:        {eResult = orxKEYBOARD_KEY_F13; break;}
    case sf::Key::F14:        {eResult = orxKEYBOARD_KEY_F14; break;}
    case sf::Key::F15:        {eResult = orxKEYBOARD_KEY_F15; break;}
    case sf::Key::Up:         {eResult = orxKEYBOARD_KEY_UP; break;}
    case sf::Key::Right:      {eResult = orxKEYBOARD_KEY_RIGHT; break;}
    case sf::Key::Down:       {eResult = orxKEYBOARD_KEY_DOWN; break;}
    case sf::Key::Left:       {eResult = orxKEYBOARD_KEY_LEFT; break;}
    case sf::Key::Numpad0:    {eResult = orxKEYBOARD_KEY_NUMPAD0; break;}
    case sf::Key::Numpad1:    {eResult = orxKEYBOARD_KEY_NUMPAD1; break;}
    case sf::Key::Numpad2:    {eResult = orxKEYBOARD_KEY_NUMPAD2; break;}
    case sf::Key::Numpad3:    {eResult = orxKEYBOARD_KEY_NUMPAD3; break;}
    case sf::Key::Numpad4:    {eResult = orxKEYBOARD_KEY_NUMPAD4; break;}
    case sf::Key::Numpad5:    {eResult = orxKEYBOARD_KEY_NUMPAD5; break;}
    case sf::Key::Numpad6:    {eResult = orxKEYBOARD_KEY_NUMPAD6; break;}
    case sf::Key::Numpad7:    {eResult = orxKEYBOARD_KEY_NUMPAD7; break;}
    case sf::Key::Numpad8:    {eResult = orxKEYBOARD_KEY_NUMPAD8; break;}
    case sf::Key::Numpad9:    {eResult = orxKEYBOARD_KEY_NUMPAD9; break;}
    case sf::Key::A:          {eResult = orxKEYBOARD_KEY_A; break;}
    case sf::Key::Z:          {eResult = orxKEYBOARD_KEY_Z; break;}
    case sf::Key::E:          {eResult = orxKEYBOARD_KEY_E; break;}
    case sf::Key::R:          {eResult = orxKEYBOARD_KEY_R; break;}
    case sf::Key::T:          {eResult = orxKEYBOARD_KEY_T; break;}
    case sf::Key::Y:          {eResult = orxKEYBOARD_KEY_Y; break;}
    case sf::Key::U:          {eResult = orxKEYBOARD_KEY_U; break;}
    case sf::Key::I:          {eResult = orxKEYBOARD_KEY_I; break;}
    case sf::Key::O:          {eResult = orxKEYBOARD_KEY_O; break;}
    case sf::Key::P:          {eResult = orxKEYBOARD_KEY_P; break;}
    case sf::Key::Q:          {eResult = orxKEYBOARD_KEY_Q; break;}
    case sf::Key::S:          {eResult = orxKEYBOARD_KEY_S; break;}
    case sf::Key::D:          {eResult = orxKEYBOARD_KEY_D; break;}
    case sf::Key::F:          {eResult = orxKEYBOARD_KEY_F; break;}
    case sf::Key::G:          {eResult = orxKEYBOARD_KEY_G; break;}
    case sf::Key::H:          {eResult = orxKEYBOARD_KEY_H; break;}
    case sf::Key::J:          {eResult = orxKEYBOARD_KEY_J; break;}
    case sf::Key::K:          {eResult = orxKEYBOARD_KEY_K; break;}
    case sf::Key::L:          {eResult = orxKEYBOARD_KEY_L; break;}
    case sf::Key::M:          {eResult = orxKEYBOARD_KEY_M; break;}
    case sf::Key::W:          {eResult = orxKEYBOARD_KEY_W; break;}
    case sf::Key::X:          {eResult = orxKEYBOARD_KEY_X; break;}
    case sf::Key::C:          {eResult = orxKEYBOARD_KEY_C; break;}
    case sf::Key::V:          {eResult = orxKEYBOARD_KEY_V; break;}
    case sf::Key::B:          {eResult = orxKEYBOARD_KEY_B; break;}
    case sf::Key::N:          {eResult = orxKEYBOARD_KEY_N; break;}
    case sf::Key::Num0:       {eResult = orxKEYBOARD_KEY_0; break;}
    case sf::Key::Num1:       {eResult = orxKEYBOARD_KEY_1; break;}
    case sf::Key::Num2:       {eResult = orxKEYBOARD_KEY_2; break;}
    case sf::Key::Num3:       {eResult = orxKEYBOARD_KEY_3; break;}
    case sf::Key::Num4:       {eResult = orxKEYBOARD_KEY_4; break;}
    case sf::Key::Num5:       {eResult = orxKEYBOARD_KEY_5; break;}
    case sf::Key::Num6:       {eResult = orxKEYBOARD_KEY_6; break;}
    case sf::Key::Num7:       {eResult = orxKEYBOARD_KEY_7; break;}
    case sf::Key::Num8:       {eResult = orxKEYBOARD_KEY_8; break;}
    case sf::Key::Num9:       {eResult = orxKEYBOARD_KEY_9; break;}
    default:                  {eResult = orxKEYBOARD_KEY_NUMBER;}
  }

  /* Done! */
  return eResult;
}

/** Event handler
 */
static orxSTATUS orxFASTCALL orxKeyboard_SFML_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Is a key pressed or released? */
  if(((_pstEvent->eType == orxEVENT_TYPE_FIRST_RESERVED + sf::Event::KeyPressed)
   && (_pstEvent->eID == sf::Event::KeyPressed))
  || ((_pstEvent->eType == orxEVENT_TYPE_FIRST_RESERVED + sf::Event::KeyReleased)
   && (_pstEvent->eID == sf::Event::KeyReleased)))
  {
    orxKEYBOARD_EVENT_PAYLOAD stPayload;
    sf::Event                *poEvent;

    /* Gets SFML event */
    poEvent = (sf::Event *)(_pstEvent->pstPayload);

    /* Inits payload */
    stPayload.eKey        = orxKeyboard_SFML_GetKeyID(poEvent->Key.Code);
    stPayload.u32Unicode  = 0;

    /* Sends event */
    orxEVENT_SEND(orxEVENT_TYPE_KEYBOARD, (_pstEvent->eID == sf::Event::KeyPressed) ? orxKEYBOARD_EVENT_KEY_PRESSED : orxKEYBOARD_EVENT_KEY_RELEASED, orxNULL, orxNULL, &stPayload);
  }
  /* Is a text entered? */
  else if((_pstEvent->eType == orxEVENT_TYPE_FIRST_RESERVED + sf::Event::TextEntered)
       && (_pstEvent->eID == sf::Event::TextEntered))
  {
    orxKEYBOARD_EVENT_PAYLOAD stPayload;
    sf::Event                *poEvent;

    /* Gets SFML event */
    poEvent = (sf::Event *)(_pstEvent->pstPayload);

    /* Inits payload */
    stPayload.eKey        = orxKEYBOARD_KEY_NONE;
    stPayload.u32Unicode  = poEvent->Text.Unicode;

    /* Sends event */
    orxEVENT_SEND(orxEVENT_TYPE_KEYBOARD, orxKEYBOARD_EVENT_KEY_PRESSED, orxNULL, orxNULL, &stPayload);
  }

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxKeyboard_SFML_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Wasn't already initialized? */
  if(!(sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstKeyboard, sizeof(orxKEYBOARD_STATIC));

    /* Registers our keyboard event handlers */
    if((orxEvent_AddHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::KeyPressed), orxKeyboard_SFML_EventHandler) != orxSTATUS_FAILURE)
    && (orxEvent_AddHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::KeyReleased), orxKeyboard_SFML_EventHandler) != orxSTATUS_FAILURE)
    && (orxEvent_AddHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::TextEntered), orxKeyboard_SFML_EventHandler) != orxSTATUS_FAILURE))
    {
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
    else
    {
      /* Removes event handlers */
      orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::KeyPressed), orxKeyboard_SFML_EventHandler);
      orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::KeyReleased), orxKeyboard_SFML_EventHandler);
      orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::TextEntered), orxKeyboard_SFML_EventHandler);
    }
  }

  /* Done! */
  return eResult;
}

extern "C" void orxFASTCALL orxKeyboard_SFML_Exit()
{
  /* Was initialized? */
  if(sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY)
  {
    /* Removes event handlers */
    orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::KeyPressed), orxKeyboard_SFML_EventHandler);
    orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::KeyReleased), orxKeyboard_SFML_EventHandler);
    orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::TextEntered), orxKeyboard_SFML_EventHandler);

    /* Cleans static controller */
    orxMemory_Zero(&sstKeyboard, sizeof(orxKEYBOARD_STATIC));
  }

  return;
}

extern "C" orxBOOL orxFASTCALL orxKeyboard_SFML_IsKeyPressed(orxKEYBOARD_KEY _eKey)
{
  orxBOOL       bResult;
  sf::Key::Code eSFMLKey;

  /* Checks */
  orxASSERT(_eKey < orxKEYBOARD_KEY_NUMBER);
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  /* Gets SFML key enum */
  eSFMLKey = orxKeyboard_SFML_GetSFMLKey(_eKey);

  /* Valid? */
  if(eSFMLKey < sf::Key::Count)
  {
    /* Updates result */
    bResult = sstKeyboard.poInput->IsKeyDown(eSFMLKey) ? orxTRUE : orxFALSE;
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

extern "C" orxKEYBOARD_KEY orxFASTCALL orxKeyboard_SFML_Read()
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

extern "C" orxBOOL orxFASTCALL orxKeyboard_SFML_Hit()
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  /* Updates result */
  bResult = (orxKeyboard_SFML_Read() != orxKEYBOARD_KEY_NONE) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

extern "C" void orxFASTCALL orxKeyboard_SFML_ClearBuffer()
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
