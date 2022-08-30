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
 * GLFW keyboard plugin implementation
 *
 */


#include "orxPluginAPI.h"

#include "GLFW/glfw3.h"

#ifndef __orxEMBEDDED__
  #ifdef __orxMSVC__
    #pragma message("!!WARNING!! This plugin will only work in non-embedded mode when linked against a *DYNAMIC* version of GLFW!")
  #else /* __orxMSVC__ */
    #warning !!WARNING!! This plugin will only work in non-embedded mode when linked against a *DYNAMIC* version of GLFW!
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


/** Misc
 */
#define orxKEYBOARD_KU32_BUFFER_SIZE            64
#define orxKEYBOARD_KU32_STRING_BUFFER_SIZE     (orxKEYBOARD_KU32_BUFFER_SIZE * 4 + 1)
#define orxKEYBOARD_KU32_NAME_BUFFER_SIZE       8


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxKEYBOARD_STATIC_t
{
  orxU32            u32KeyReadIndex, u32KeyWriteIndex, u32CharReadIndex, u32CharWriteIndex;
  orxU32            au32KeyBuffer[orxKEYBOARD_KU32_BUFFER_SIZE];
  orxU32            au32CharBuffer[orxKEYBOARD_KU32_BUFFER_SIZE];
  orxCHAR           acStringBuffer[orxKEYBOARD_KU32_STRING_BUFFER_SIZE];
  orxCHAR           acNameBuffer[orxKEYBOARD_KU32_NAME_BUFFER_SIZE];
  GLFWwindow       *pstWindow;
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

static orxKEYBOARD_KEY orxFASTCALL orxKeyboard_GLFW_GetKey(GLFWKey _eKey)
{
  orxKEYBOARD_KEY eResult;

  /* Depending on key */
  switch(_eKey)
  {
    case GLFW_KEY_0:                {eResult = orxKEYBOARD_KEY_0;               break;}
    case GLFW_KEY_1:                {eResult = orxKEYBOARD_KEY_1;               break;}
    case GLFW_KEY_2:                {eResult = orxKEYBOARD_KEY_2;               break;}
    case GLFW_KEY_3:                {eResult = orxKEYBOARD_KEY_3;               break;}
    case GLFW_KEY_4:                {eResult = orxKEYBOARD_KEY_4;               break;}
    case GLFW_KEY_5:                {eResult = orxKEYBOARD_KEY_5;               break;}
    case GLFW_KEY_6:                {eResult = orxKEYBOARD_KEY_6;               break;}
    case GLFW_KEY_7:                {eResult = orxKEYBOARD_KEY_7;               break;}
    case GLFW_KEY_8:                {eResult = orxKEYBOARD_KEY_8;               break;}
    case GLFW_KEY_9:                {eResult = orxKEYBOARD_KEY_9;               break;}
    case GLFW_KEY_A:                {eResult = orxKEYBOARD_KEY_A;               break;}
    case GLFW_KEY_B:                {eResult = orxKEYBOARD_KEY_B;               break;}
    case GLFW_KEY_C:                {eResult = orxKEYBOARD_KEY_C;               break;}
    case GLFW_KEY_D:                {eResult = orxKEYBOARD_KEY_D;               break;}
    case GLFW_KEY_E:                {eResult = orxKEYBOARD_KEY_E;               break;}
    case GLFW_KEY_F:                {eResult = orxKEYBOARD_KEY_F;               break;}
    case GLFW_KEY_G:                {eResult = orxKEYBOARD_KEY_G;               break;}
    case GLFW_KEY_H:                {eResult = orxKEYBOARD_KEY_H;               break;}
    case GLFW_KEY_I:                {eResult = orxKEYBOARD_KEY_I;               break;}
    case GLFW_KEY_J:                {eResult = orxKEYBOARD_KEY_J;               break;}
    case GLFW_KEY_K:                {eResult = orxKEYBOARD_KEY_K;               break;}
    case GLFW_KEY_L:                {eResult = orxKEYBOARD_KEY_L;               break;}
    case GLFW_KEY_M:                {eResult = orxKEYBOARD_KEY_M;               break;}
    case GLFW_KEY_N:                {eResult = orxKEYBOARD_KEY_N;               break;}
    case GLFW_KEY_O:                {eResult = orxKEYBOARD_KEY_O;               break;}
    case GLFW_KEY_P:                {eResult = orxKEYBOARD_KEY_P;               break;}
    case GLFW_KEY_Q:                {eResult = orxKEYBOARD_KEY_Q;               break;}
    case GLFW_KEY_R:                {eResult = orxKEYBOARD_KEY_R;               break;}
    case GLFW_KEY_S:                {eResult = orxKEYBOARD_KEY_S;               break;}
    case GLFW_KEY_T:                {eResult = orxKEYBOARD_KEY_T;               break;}
    case GLFW_KEY_U:                {eResult = orxKEYBOARD_KEY_U;               break;}
    case GLFW_KEY_V:                {eResult = orxKEYBOARD_KEY_V;               break;}
    case GLFW_KEY_W:                {eResult = orxKEYBOARD_KEY_W;               break;}
    case GLFW_KEY_X:                {eResult = orxKEYBOARD_KEY_X;               break;}
    case GLFW_KEY_Y:                {eResult = orxKEYBOARD_KEY_Y;               break;}
    case GLFW_KEY_Z:                {eResult = orxKEYBOARD_KEY_Z;               break;}
    case GLFW_KEY_SPACE:            {eResult = orxKEYBOARD_KEY_SPACE;           break;}
    case GLFW_KEY_APOSTROPHE:       {eResult = orxKEYBOARD_KEY_QUOTE;           break;}
    case GLFW_KEY_COMMA:            {eResult = orxKEYBOARD_KEY_COMMA;           break;}
    case GLFW_KEY_MINUS:            {eResult = orxKEYBOARD_KEY_DASH;            break;}
    case GLFW_KEY_PERIOD:           {eResult = orxKEYBOARD_KEY_PERIOD;          break;}
    case GLFW_KEY_SLASH:            {eResult = orxKEYBOARD_KEY_SLASH;           break;}
    case GLFW_KEY_SEMICOLON:        {eResult = orxKEYBOARD_KEY_SEMICOLON;       break;}
    case GLFW_KEY_EQUAL:            {eResult = orxKEYBOARD_KEY_EQUAL;           break;}
    case GLFW_KEY_LEFT_BRACKET:     {eResult = orxKEYBOARD_KEY_LBRACKET;        break;}
    case GLFW_KEY_BACKSLASH:        {eResult = orxKEYBOARD_KEY_BACKSLASH;       break;}
    case GLFW_KEY_RIGHT_BRACKET:    {eResult = orxKEYBOARD_KEY_RBRACKET;        break;}
    case GLFW_KEY_GRAVE_ACCENT:     {eResult = orxKEYBOARD_KEY_BACKQUOTE;       break;}
    case GLFW_KEY_WORLD_1:          {eResult = orxKEYBOARD_KEY_WORLD_1;         break;}
    case GLFW_KEY_WORLD_2:          {eResult = orxKEYBOARD_KEY_WORLD_2;         break;}
    case GLFW_KEY_ESCAPE:           {eResult = orxKEYBOARD_KEY_ESCAPE;          break;}
    case GLFW_KEY_ENTER:            {eResult = orxKEYBOARD_KEY_ENTER;           break;}
    case GLFW_KEY_TAB:              {eResult = orxKEYBOARD_KEY_TAB;             break;}
    case GLFW_KEY_BACKSPACE:        {eResult = orxKEYBOARD_KEY_BACKSPACE;       break;}
    case GLFW_KEY_INSERT:           {eResult = orxKEYBOARD_KEY_INSERT;          break;}
    case GLFW_KEY_DELETE:           {eResult = orxKEYBOARD_KEY_DELETE;          break;}
    case GLFW_KEY_RIGHT:            {eResult = orxKEYBOARD_KEY_RIGHT;           break;}
    case GLFW_KEY_LEFT:             {eResult = orxKEYBOARD_KEY_LEFT;            break;}
    case GLFW_KEY_DOWN:             {eResult = orxKEYBOARD_KEY_DOWN;            break;}
    case GLFW_KEY_UP:               {eResult = orxKEYBOARD_KEY_UP;              break;}
    case GLFW_KEY_PAGE_UP:          {eResult = orxKEYBOARD_KEY_PAGE_UP;         break;}
    case GLFW_KEY_PAGE_DOWN:        {eResult = orxKEYBOARD_KEY_PAGE_DOWN;       break;}
    case GLFW_KEY_HOME:             {eResult = orxKEYBOARD_KEY_HOME;            break;}
    case GLFW_KEY_END:              {eResult = orxKEYBOARD_KEY_END;             break;}
    case GLFW_KEY_CAPS_LOCK:        {eResult = orxKEYBOARD_KEY_CAPS_LOCK;       break;}
    case GLFW_KEY_SCROLL_LOCK:      {eResult = orxKEYBOARD_KEY_SCROLL_LOCK;     break;}
    case GLFW_KEY_NUM_LOCK:         {eResult = orxKEYBOARD_KEY_NUM_LOCK;        break;}
    case GLFW_KEY_PRINT_SCREEN:     {eResult = orxKEYBOARD_KEY_PRINT_SCREEN;    break;}
    case GLFW_KEY_PAUSE:            {eResult = orxKEYBOARD_KEY_PAUSE;           break;}
    case GLFW_KEY_F1:               {eResult = orxKEYBOARD_KEY_F1;              break;}
    case GLFW_KEY_F2:               {eResult = orxKEYBOARD_KEY_F2;              break;}
    case GLFW_KEY_F3:               {eResult = orxKEYBOARD_KEY_F3;              break;}
    case GLFW_KEY_F4:               {eResult = orxKEYBOARD_KEY_F4;              break;}
    case GLFW_KEY_F5:               {eResult = orxKEYBOARD_KEY_F5;              break;}
    case GLFW_KEY_F6:               {eResult = orxKEYBOARD_KEY_F6;              break;}
    case GLFW_KEY_F7:               {eResult = orxKEYBOARD_KEY_F7;              break;}
    case GLFW_KEY_F8:               {eResult = orxKEYBOARD_KEY_F8;              break;}
    case GLFW_KEY_F9:               {eResult = orxKEYBOARD_KEY_F9;              break;}
    case GLFW_KEY_F10:              {eResult = orxKEYBOARD_KEY_F10;             break;}
    case GLFW_KEY_F11:              {eResult = orxKEYBOARD_KEY_F11;             break;}
    case GLFW_KEY_F12:              {eResult = orxKEYBOARD_KEY_F12;             break;}
    case GLFW_KEY_F13:              {eResult = orxKEYBOARD_KEY_F13;             break;}
    case GLFW_KEY_F14:              {eResult = orxKEYBOARD_KEY_F14;             break;}
    case GLFW_KEY_F15:              {eResult = orxKEYBOARD_KEY_F15;             break;}
    case GLFW_KEY_F16:              {eResult = orxKEYBOARD_KEY_F16;             break;}
    case GLFW_KEY_F17:              {eResult = orxKEYBOARD_KEY_F17;             break;}
    case GLFW_KEY_F18:              {eResult = orxKEYBOARD_KEY_F18;             break;}
    case GLFW_KEY_F19:              {eResult = orxKEYBOARD_KEY_F19;             break;}
    case GLFW_KEY_F20:              {eResult = orxKEYBOARD_KEY_F20;             break;}
    case GLFW_KEY_F21:              {eResult = orxKEYBOARD_KEY_F21;             break;}
    case GLFW_KEY_F22:              {eResult = orxKEYBOARD_KEY_F22;             break;}
    case GLFW_KEY_F23:              {eResult = orxKEYBOARD_KEY_F23;             break;}
    case GLFW_KEY_F24:              {eResult = orxKEYBOARD_KEY_F24;             break;}
    case GLFW_KEY_F25:              {eResult = orxKEYBOARD_KEY_F25;             break;}
    case GLFW_KEY_KP_0:             {eResult = orxKEYBOARD_KEY_NUMPAD_0;        break;}
    case GLFW_KEY_KP_1:             {eResult = orxKEYBOARD_KEY_NUMPAD_1;        break;}
    case GLFW_KEY_KP_2:             {eResult = orxKEYBOARD_KEY_NUMPAD_2;        break;}
    case GLFW_KEY_KP_3:             {eResult = orxKEYBOARD_KEY_NUMPAD_3;        break;}
    case GLFW_KEY_KP_4:             {eResult = orxKEYBOARD_KEY_NUMPAD_4;        break;}
    case GLFW_KEY_KP_5:             {eResult = orxKEYBOARD_KEY_NUMPAD_5;        break;}
    case GLFW_KEY_KP_6:             {eResult = orxKEYBOARD_KEY_NUMPAD_6;        break;}
    case GLFW_KEY_KP_7:             {eResult = orxKEYBOARD_KEY_NUMPAD_7;        break;}
    case GLFW_KEY_KP_8:             {eResult = orxKEYBOARD_KEY_NUMPAD_8;        break;}
    case GLFW_KEY_KP_9:             {eResult = orxKEYBOARD_KEY_NUMPAD_9;        break;}
    case GLFW_KEY_KP_DECIMAL:       {eResult = orxKEYBOARD_KEY_NUMPAD_DECIMAL;  break;}
    case GLFW_KEY_KP_DIVIDE:        {eResult = orxKEYBOARD_KEY_NUMPAD_DIVIDE;   break;}
    case GLFW_KEY_KP_MULTIPLY:      {eResult = orxKEYBOARD_KEY_NUMPAD_MULTIPLY; break;}
    case GLFW_KEY_KP_SUBTRACT:      {eResult = orxKEYBOARD_KEY_NUMPAD_SUBTRACT; break;}
    case GLFW_KEY_KP_ADD:           {eResult = orxKEYBOARD_KEY_NUMPAD_ADD;      break;}
    case GLFW_KEY_KP_ENTER:         {eResult = orxKEYBOARD_KEY_NUMPAD_ENTER;    break;}
    case GLFW_KEY_KP_EQUAL:         {eResult = orxKEYBOARD_KEY_NUMPAD_EQUAL;    break;}
    case GLFW_KEY_LEFT_SHIFT:       {eResult = orxKEYBOARD_KEY_LSHIFT;          break;}
    case GLFW_KEY_LEFT_CONTROL:     {eResult = orxKEYBOARD_KEY_LCTRL;           break;}
    case GLFW_KEY_LEFT_ALT:         {eResult = orxKEYBOARD_KEY_LALT;            break;}
    case GLFW_KEY_LEFT_SUPER:       {eResult = orxKEYBOARD_KEY_LSYSTEM;         break;}
    case GLFW_KEY_RIGHT_SHIFT:      {eResult = orxKEYBOARD_KEY_RSHIFT;          break;}
    case GLFW_KEY_RIGHT_CONTROL:    {eResult = orxKEYBOARD_KEY_RCTRL;           break;}
    case GLFW_KEY_RIGHT_ALT:        {eResult = orxKEYBOARD_KEY_RALT;            break;}
    case GLFW_KEY_RIGHT_SUPER:      {eResult = orxKEYBOARD_KEY_RSYSTEM;         break;}
    case GLFW_KEY_MENU:             {eResult = orxKEYBOARD_KEY_MENU;            break;}
    default:                        {eResult = orxKEYBOARD_KEY_NONE;            break;}
  }

  /* Done! */
  return eResult;
}

static GLFWKey orxFASTCALL orxKeyboard_GLFW_GetGLFWKey(orxKEYBOARD_KEY _eKey)
{
  GLFWKey eResult;

  /* Depending on key */
  switch(_eKey)
  {
    case orxKEYBOARD_KEY_0:               {eResult =  GLFW_KEY_0;               break;}
    case orxKEYBOARD_KEY_1:               {eResult =  GLFW_KEY_1;               break;}
    case orxKEYBOARD_KEY_2:               {eResult =  GLFW_KEY_2;               break;}
    case orxKEYBOARD_KEY_3:               {eResult =  GLFW_KEY_3;               break;}
    case orxKEYBOARD_KEY_4:               {eResult =  GLFW_KEY_4;               break;}
    case orxKEYBOARD_KEY_5:               {eResult =  GLFW_KEY_5;               break;}
    case orxKEYBOARD_KEY_6:               {eResult =  GLFW_KEY_6;               break;}
    case orxKEYBOARD_KEY_7:               {eResult =  GLFW_KEY_7;               break;}
    case orxKEYBOARD_KEY_8:               {eResult =  GLFW_KEY_8;               break;}
    case orxKEYBOARD_KEY_9:               {eResult =  GLFW_KEY_9;               break;}
    case orxKEYBOARD_KEY_A:               {eResult =  GLFW_KEY_A;               break;}
    case orxKEYBOARD_KEY_B:               {eResult =  GLFW_KEY_B;               break;}
    case orxKEYBOARD_KEY_C:               {eResult =  GLFW_KEY_C;               break;}
    case orxKEYBOARD_KEY_D:               {eResult =  GLFW_KEY_D;               break;}
    case orxKEYBOARD_KEY_E:               {eResult =  GLFW_KEY_E;               break;}
    case orxKEYBOARD_KEY_F:               {eResult =  GLFW_KEY_F;               break;}
    case orxKEYBOARD_KEY_G:               {eResult =  GLFW_KEY_G;               break;}
    case orxKEYBOARD_KEY_H:               {eResult =  GLFW_KEY_H;               break;}
    case orxKEYBOARD_KEY_I:               {eResult =  GLFW_KEY_I;               break;}
    case orxKEYBOARD_KEY_J:               {eResult =  GLFW_KEY_J;               break;}
    case orxKEYBOARD_KEY_K:               {eResult =  GLFW_KEY_K;               break;}
    case orxKEYBOARD_KEY_L:               {eResult =  GLFW_KEY_L;               break;}
    case orxKEYBOARD_KEY_M:               {eResult =  GLFW_KEY_M;               break;}
    case orxKEYBOARD_KEY_N:               {eResult =  GLFW_KEY_N;               break;}
    case orxKEYBOARD_KEY_O:               {eResult =  GLFW_KEY_O;               break;}
    case orxKEYBOARD_KEY_P:               {eResult =  GLFW_KEY_P;               break;}
    case orxKEYBOARD_KEY_Q:               {eResult =  GLFW_KEY_Q;               break;}
    case orxKEYBOARD_KEY_R:               {eResult =  GLFW_KEY_R;               break;}
    case orxKEYBOARD_KEY_S:               {eResult =  GLFW_KEY_S;               break;}
    case orxKEYBOARD_KEY_T:               {eResult =  GLFW_KEY_T;               break;}
    case orxKEYBOARD_KEY_U:               {eResult =  GLFW_KEY_U;               break;}
    case orxKEYBOARD_KEY_V:               {eResult =  GLFW_KEY_V;               break;}
    case orxKEYBOARD_KEY_W:               {eResult =  GLFW_KEY_W;               break;}
    case orxKEYBOARD_KEY_X:               {eResult =  GLFW_KEY_X;               break;}
    case orxKEYBOARD_KEY_Y:               {eResult =  GLFW_KEY_Y;               break;}
    case orxKEYBOARD_KEY_Z:               {eResult =  GLFW_KEY_Z;               break;}
    case orxKEYBOARD_KEY_SPACE:           {eResult =  GLFW_KEY_SPACE;           break;}
    case orxKEYBOARD_KEY_QUOTE:           {eResult =  GLFW_KEY_APOSTROPHE;      break;}
    case orxKEYBOARD_KEY_COMMA:           {eResult =  GLFW_KEY_COMMA;           break;}
    case orxKEYBOARD_KEY_DASH:            {eResult =  GLFW_KEY_MINUS;           break;}
    case orxKEYBOARD_KEY_PERIOD:          {eResult =  GLFW_KEY_PERIOD;          break;}
    case orxKEYBOARD_KEY_SLASH:           {eResult =  GLFW_KEY_SLASH;           break;}
    case orxKEYBOARD_KEY_SEMICOLON:       {eResult =  GLFW_KEY_SEMICOLON;       break;}
    case orxKEYBOARD_KEY_EQUAL:           {eResult =  GLFW_KEY_EQUAL;           break;}
    case orxKEYBOARD_KEY_LBRACKET:        {eResult =  GLFW_KEY_LEFT_BRACKET;    break;}
    case orxKEYBOARD_KEY_BACKSLASH:       {eResult =  GLFW_KEY_BACKSLASH;       break;}
    case orxKEYBOARD_KEY_RBRACKET:        {eResult =  GLFW_KEY_RIGHT_BRACKET;   break;}
    case orxKEYBOARD_KEY_BACKQUOTE:       {eResult =  GLFW_KEY_GRAVE_ACCENT;    break;}
    case orxKEYBOARD_KEY_WORLD_1:         {eResult =  GLFW_KEY_WORLD_1;         break;}
    case orxKEYBOARD_KEY_WORLD_2:         {eResult =  GLFW_KEY_WORLD_2;         break;}
    case orxKEYBOARD_KEY_ESCAPE:          {eResult =  GLFW_KEY_ESCAPE;          break;}
    case orxKEYBOARD_KEY_ENTER:           {eResult =  GLFW_KEY_ENTER;           break;}
    case orxKEYBOARD_KEY_TAB:             {eResult =  GLFW_KEY_TAB;             break;}
    case orxKEYBOARD_KEY_BACKSPACE:       {eResult =  GLFW_KEY_BACKSPACE;       break;}
    case orxKEYBOARD_KEY_INSERT:          {eResult =  GLFW_KEY_INSERT;          break;}
    case orxKEYBOARD_KEY_DELETE:          {eResult =  GLFW_KEY_DELETE;          break;}
    case orxKEYBOARD_KEY_RIGHT:           {eResult =  GLFW_KEY_RIGHT;           break;}
    case orxKEYBOARD_KEY_LEFT:            {eResult =  GLFW_KEY_LEFT;            break;}
    case orxKEYBOARD_KEY_DOWN:            {eResult =  GLFW_KEY_DOWN;            break;}
    case orxKEYBOARD_KEY_UP:              {eResult =  GLFW_KEY_UP;              break;}
    case orxKEYBOARD_KEY_PAGE_UP:         {eResult =  GLFW_KEY_PAGE_UP;         break;}
    case orxKEYBOARD_KEY_PAGE_DOWN:       {eResult =  GLFW_KEY_PAGE_DOWN;       break;}
    case orxKEYBOARD_KEY_HOME:            {eResult =  GLFW_KEY_HOME;            break;}
    case orxKEYBOARD_KEY_END:             {eResult =  GLFW_KEY_END;             break;}
    case orxKEYBOARD_KEY_CAPS_LOCK:       {eResult =  GLFW_KEY_CAPS_LOCK;       break;}
    case orxKEYBOARD_KEY_SCROLL_LOCK:     {eResult =  GLFW_KEY_SCROLL_LOCK;     break;}
    case orxKEYBOARD_KEY_NUM_LOCK:        {eResult =  GLFW_KEY_NUM_LOCK;        break;}
    case orxKEYBOARD_KEY_PRINT_SCREEN:    {eResult =  GLFW_KEY_PRINT_SCREEN;    break;}
    case orxKEYBOARD_KEY_PAUSE:           {eResult =  GLFW_KEY_PAUSE;           break;}
    case orxKEYBOARD_KEY_F1:              {eResult =  GLFW_KEY_F1;              break;}
    case orxKEYBOARD_KEY_F2:              {eResult =  GLFW_KEY_F2;              break;}
    case orxKEYBOARD_KEY_F3:              {eResult =  GLFW_KEY_F3;              break;}
    case orxKEYBOARD_KEY_F4:              {eResult =  GLFW_KEY_F4;              break;}
    case orxKEYBOARD_KEY_F5:              {eResult =  GLFW_KEY_F5;              break;}
    case orxKEYBOARD_KEY_F6:              {eResult =  GLFW_KEY_F6;              break;}
    case orxKEYBOARD_KEY_F7:              {eResult =  GLFW_KEY_F7;              break;}
    case orxKEYBOARD_KEY_F8:              {eResult =  GLFW_KEY_F8;              break;}
    case orxKEYBOARD_KEY_F9:              {eResult =  GLFW_KEY_F9;              break;}
    case orxKEYBOARD_KEY_F10:             {eResult =  GLFW_KEY_F10;             break;}
    case orxKEYBOARD_KEY_F11:             {eResult =  GLFW_KEY_F11;             break;}
    case orxKEYBOARD_KEY_F12:             {eResult =  GLFW_KEY_F12;             break;}
    case orxKEYBOARD_KEY_F13:             {eResult =  GLFW_KEY_F13;             break;}
    case orxKEYBOARD_KEY_F14:             {eResult =  GLFW_KEY_F14;             break;}
    case orxKEYBOARD_KEY_F15:             {eResult =  GLFW_KEY_F15;             break;}
    case orxKEYBOARD_KEY_F16:             {eResult =  GLFW_KEY_F16;             break;}
    case orxKEYBOARD_KEY_F17:             {eResult =  GLFW_KEY_F17;             break;}
    case orxKEYBOARD_KEY_F18:             {eResult =  GLFW_KEY_F18;             break;}
    case orxKEYBOARD_KEY_F19:             {eResult =  GLFW_KEY_F19;             break;}
    case orxKEYBOARD_KEY_F20:             {eResult =  GLFW_KEY_F20;             break;}
    case orxKEYBOARD_KEY_F21:             {eResult =  GLFW_KEY_F21;             break;}
    case orxKEYBOARD_KEY_F22:             {eResult =  GLFW_KEY_F22;             break;}
    case orxKEYBOARD_KEY_F23:             {eResult =  GLFW_KEY_F23;             break;}
    case orxKEYBOARD_KEY_F24:             {eResult =  GLFW_KEY_F24;             break;}
    case orxKEYBOARD_KEY_F25:             {eResult =  GLFW_KEY_F25;             break;}
    case orxKEYBOARD_KEY_NUMPAD_0:        {eResult =  GLFW_KEY_KP_0;            break;}
    case orxKEYBOARD_KEY_NUMPAD_1:        {eResult =  GLFW_KEY_KP_1;            break;}
    case orxKEYBOARD_KEY_NUMPAD_2:        {eResult =  GLFW_KEY_KP_2;            break;}
    case orxKEYBOARD_KEY_NUMPAD_3:        {eResult =  GLFW_KEY_KP_3;            break;}
    case orxKEYBOARD_KEY_NUMPAD_4:        {eResult =  GLFW_KEY_KP_4;            break;}
    case orxKEYBOARD_KEY_NUMPAD_5:        {eResult =  GLFW_KEY_KP_5;            break;}
    case orxKEYBOARD_KEY_NUMPAD_6:        {eResult =  GLFW_KEY_KP_6;            break;}
    case orxKEYBOARD_KEY_NUMPAD_7:        {eResult =  GLFW_KEY_KP_7;            break;}
    case orxKEYBOARD_KEY_NUMPAD_8:        {eResult =  GLFW_KEY_KP_8;            break;}
    case orxKEYBOARD_KEY_NUMPAD_9:        {eResult =  GLFW_KEY_KP_9;            break;}
    case orxKEYBOARD_KEY_NUMPAD_DECIMAL:  {eResult =  GLFW_KEY_KP_DECIMAL;      break;}
    case orxKEYBOARD_KEY_NUMPAD_DIVIDE:   {eResult =  GLFW_KEY_KP_DIVIDE;       break;}
    case orxKEYBOARD_KEY_NUMPAD_MULTIPLY: {eResult =  GLFW_KEY_KP_MULTIPLY;     break;}
    case orxKEYBOARD_KEY_NUMPAD_SUBTRACT: {eResult =  GLFW_KEY_KP_SUBTRACT;     break;}
    case orxKEYBOARD_KEY_NUMPAD_ADD:      {eResult =  GLFW_KEY_KP_ADD;          break;}
    case orxKEYBOARD_KEY_NUMPAD_ENTER:    {eResult =  GLFW_KEY_KP_ENTER;        break;}
    case orxKEYBOARD_KEY_NUMPAD_EQUAL:    {eResult =  GLFW_KEY_KP_EQUAL;        break;}
    case orxKEYBOARD_KEY_LSHIFT:          {eResult =  GLFW_KEY_LEFT_SHIFT;      break;}
    case orxKEYBOARD_KEY_LCTRL:           {eResult =  GLFW_KEY_LEFT_CONTROL;    break;}
    case orxKEYBOARD_KEY_LALT:            {eResult =  GLFW_KEY_LEFT_ALT;        break;}
    case orxKEYBOARD_KEY_LSYSTEM:         {eResult =  GLFW_KEY_LEFT_SUPER;      break;}
    case orxKEYBOARD_KEY_RSHIFT:          {eResult =  GLFW_KEY_RIGHT_SHIFT;     break;}
    case orxKEYBOARD_KEY_RCTRL:           {eResult =  GLFW_KEY_RIGHT_CONTROL;   break;}
    case orxKEYBOARD_KEY_RALT:            {eResult =  GLFW_KEY_RIGHT_ALT;       break;}
    case orxKEYBOARD_KEY_RSYSTEM:         {eResult =  GLFW_KEY_RIGHT_SUPER;     break;}
    case orxKEYBOARD_KEY_MENU:            {eResult =  GLFW_KEY_MENU;            break;}
    default:                              {eResult =  GLFW_KEY_UNKNOWN;         break;}
  }

  /* Done! */
  return eResult;
}

/** Key callback
 */
static void orxKeyboard_GLFW_KeyCallback(GLFWwindow *_pstWindow, int _iKey, int _iScanCode, int _iAction, int _iMod)
{
  /* Pressed? */
  if(_iAction == GLFW_PRESS)
  {
    /* Stores it */
    sstKeyboard.au32KeyBuffer[sstKeyboard.u32KeyWriteIndex] = (orxU32)_iKey;
    sstKeyboard.u32KeyWriteIndex = (sstKeyboard.u32KeyWriteIndex + 1) & (orxKEYBOARD_KU32_BUFFER_SIZE - 1);

    /* Full? */
    if(sstKeyboard.u32KeyReadIndex == sstKeyboard.u32KeyWriteIndex)
    {
      /* Bounces read index */
      sstKeyboard.u32KeyReadIndex = (sstKeyboard.u32KeyReadIndex + 1) & (orxKEYBOARD_KU32_BUFFER_SIZE - 1);
    }
  }

  /* Done! */
  return;
}

/** Char callback
 */
static void orxKeyboard_GLFW_CharCallback(GLFWwindow *_pstWindow, unsigned int _uiCodePoint)
{
  /* Stores it */
  sstKeyboard.au32CharBuffer[sstKeyboard.u32CharWriteIndex] = (orxU32)_uiCodePoint;
  sstKeyboard.u32CharWriteIndex = (sstKeyboard.u32CharWriteIndex + 1) & (orxKEYBOARD_KU32_BUFFER_SIZE - 1);

  /* Full? */
  if(sstKeyboard.u32CharReadIndex == sstKeyboard.u32CharWriteIndex)
  {
    /* Bounces read index */
    sstKeyboard.u32CharReadIndex = (sstKeyboard.u32CharReadIndex + 1) & (orxKEYBOARD_KU32_BUFFER_SIZE - 1);
  }

  /* Done! */
  return;
}

/** Event handler
 */
static orxSTATUS orxFASTCALL orxKeyboard_GLFW_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(_pstEvent->eType == orxEVENT_TYPE_DISPLAY);

  /* Retrieves current window */
  orxEVENT_SEND(orxEVENT_TYPE_FIRST_RESERVED, 0, orxNULL, orxNULL, &(sstKeyboard.pstWindow));

  /* Registers key callback */
  glfwSetKeyCallback(sstKeyboard.pstWindow, orxKeyboard_GLFW_KeyCallback);

  /* Registers char callback */
  glfwSetCharCallback(sstKeyboard.pstWindow, orxKeyboard_GLFW_CharCallback);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxKeyboard_GLFW_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Wasn't already initialized? */
  if(!(sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstKeyboard, sizeof(orxKEYBOARD_STATIC));

    /* Retrieves current window */
    orxEVENT_SEND(orxEVENT_TYPE_FIRST_RESERVED, 0, orxNULL, orxNULL, &(sstKeyboard.pstWindow));

    /* Success? */
    if(sstKeyboard.pstWindow != orxNULL)
    {
      /* Adds event handler */
      orxEvent_AddHandler(orxEVENT_TYPE_DISPLAY, orxKeyboard_GLFW_EventHandler);
      orxEvent_SetHandlerIDFlags(orxKeyboard_GLFW_EventHandler, orxEVENT_TYPE_DISPLAY, orxNULL, orxEVENT_GET_FLAG(orxDISPLAY_EVENT_SET_VIDEO_MODE), orxEVENT_KU32_MASK_ID_ALL);

      /* Registers key callback */
      glfwSetKeyCallback(sstKeyboard.pstWindow, orxKeyboard_GLFW_KeyCallback);

      /* Registers char callback */
      glfwSetCharCallback(sstKeyboard.pstWindow, orxKeyboard_GLFW_CharCallback);

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
  /* Wasn't initialized? */
  if(sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY)
  {
    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_DISPLAY, orxKeyboard_GLFW_EventHandler);

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
    bResult = (glfwGetKey(sstKeyboard.pstWindow, (int)eGLFWKey) != GLFW_RELEASE) ? orxTRUE : orxFALSE;
  }
  else
  {
    /* Updates result */
    bResult = orxFALSE;
  }

  /* Done! */
  return bResult;
}

const orxSTRING orxFASTCALL orxKeyboard_GLFW_GetKeyDisplayName(orxKEYBOARD_KEY _eKey)
{
  GLFWKey         eGLFWKey;
  const orxSTRING zResult = orxNULL;

  /* Checks */
  orxASSERT(_eKey < orxKEYBOARD_KEY_NUMBER);
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  /* Gets GLFW key enum */
  eGLFWKey = orxKeyboard_GLFW_GetGLFWKey(_eKey);

  /* Valid? */
  if(eGLFWKey != GLFW_KEY_UNKNOWN)
  {
    const orxSTRING zName;

    /* Gets its name */
    zName = glfwGetKeyName(eGLFWKey, 0);

    /* Success? */
    if(zName != orxNULL)
    {
      /* Prints it */
      orxString_NPrint(sstKeyboard.acNameBuffer, sizeof(sstKeyboard.acNameBuffer) - 1, "%s", zName);

      /* Gets uppercase version */
      orxString_UpperCase(sstKeyboard.acNameBuffer);

      /* Updates result */
      zResult = sstKeyboard.acNameBuffer;
    }
  }

  /* Not found? */
  if(zResult == orxNULL)
  {
    /* Gets key name */
    zResult = orxKeyboard_GetKeyName(_eKey);

    /* Success? */
    if(zResult != orxSTRING_EMPTY)
    {
      /* Skips prefix */
      zResult += 4;
    }
  }

  /* Done! */
  return zResult;
}

orxKEYBOARD_KEY orxFASTCALL orxKeyboard_GLFW_ReadKey()
{
  orxKEYBOARD_KEY eResult;

  /* Checks */
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  /* Not empty? */
  if(sstKeyboard.u32KeyReadIndex != sstKeyboard.u32KeyWriteIndex)
  {
    /* Updates result */
    eResult = orxKeyboard_GLFW_GetKey((GLFWKey)sstKeyboard.au32KeyBuffer[sstKeyboard.u32KeyReadIndex]);

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

const orxSTRING orxFASTCALL orxKeyboard_GLFW_ReadString()
{
  orxU32          u32BufferSize;
  orxCHAR        *pc;
  const orxSTRING zResult;

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

void orxFASTCALL orxKeyboard_GLFW_ClearBuffer()
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

orxSTATUS orxFASTCALL orxKeyboard_GLFW_Show(orxBOOL _bShow)
{
  return orxSTATUS_FAILURE;
}

/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(KEYBOARD);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_GLFW_Init, KEYBOARD, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_GLFW_Exit, KEYBOARD, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_GLFW_IsKeyPressed, KEYBOARD, IS_KEY_PRESSED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_GLFW_GetKeyDisplayName, KEYBOARD, GET_KEY_DISPLAY_NAME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_GLFW_ReadKey, KEYBOARD, READ_KEY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_GLFW_ReadString, KEYBOARD, READ_STRING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_GLFW_ClearBuffer, KEYBOARD, CLEAR_BUFFER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_GLFW_Show, KEYBOARD, SHOW);
orxPLUGIN_USER_CORE_FUNCTION_END();
