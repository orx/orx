/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2015 Orx-Project
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
  orxU32            u32KeyReadIndex, u32KeyWriteIndex, u32CharReadIndex, u32CharWriteIndex;
  orxU32            au32KeyBuffer[orxKEYBOARD_KU32_BUFFER_SIZE];
  orxU32            au32CharBuffer[orxKEYBOARD_KU32_BUFFER_SIZE];
  orxCHAR           acStringBuffer[orxKEYBOARD_KU32_STRING_BUFFER_SIZE];
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
    case GLFW_KEY_ESC:              {eResult = orxKEYBOARD_KEY_ESCAPE; break;}
    case GLFW_KEY_SPACE:            {eResult = orxKEYBOARD_KEY_SPACE; break;}
    case GLFW_KEY_ENTER:            {eResult = orxKEYBOARD_KEY_RETURN; break;}
    case GLFW_KEY_BACKSPACE:        {eResult = orxKEYBOARD_KEY_BACKSPACE; break;}
    case GLFW_KEY_TAB:              {eResult = orxKEYBOARD_KEY_TAB; break;}
    case GLFW_KEY_PAGEUP:           {eResult = orxKEYBOARD_KEY_PAGEUP; break;}
    case GLFW_KEY_PAGEDOWN:         {eResult = orxKEYBOARD_KEY_PAGEDOWN; break;}
    case GLFW_KEY_END:              {eResult = orxKEYBOARD_KEY_END; break;}
    case GLFW_KEY_HOME:             {eResult = orxKEYBOARD_KEY_HOME; break;}
    case GLFW_KEY_INSERT:           {eResult = orxKEYBOARD_KEY_INSERT; break;}
    case GLFW_KEY_DEL:              {eResult = orxKEYBOARD_KEY_DELETE; break;}
    case GLFW_KEY_KP_ADD:           {eResult = orxKEYBOARD_KEY_ADD; break;}
    case GLFW_KEY_KP_SUBTRACT:      {eResult = orxKEYBOARD_KEY_SUBTRACT; break;}
    case GLFW_KEY_KP_MULTIPLY:      {eResult = orxKEYBOARD_KEY_MULTIPLY; break;}
    case GLFW_KEY_KP_DIVIDE:        {eResult = orxKEYBOARD_KEY_DIVIDE; break;}
    case GLFW_KEY_PAUSE:            {eResult = orxKEYBOARD_KEY_PAUSE; break;}
    case GLFW_KEY_RALT:             {eResult = orxKEYBOARD_KEY_RALT; break;}
    case GLFW_KEY_RCTRL:            {eResult = orxKEYBOARD_KEY_RCTRL; break;}
    case GLFW_KEY_RSHIFT:           {eResult = orxKEYBOARD_KEY_RSHIFT; break;}
    case GLFW_KEY_RSUPER:           {eResult = orxKEYBOARD_KEY_RSYSTEM; break;}
    case GLFW_KEY_LALT:             {eResult = orxKEYBOARD_KEY_LALT; break;}
    case GLFW_KEY_LCTRL:            {eResult = orxKEYBOARD_KEY_LCTRL; break;}
    case GLFW_KEY_LSHIFT:           {eResult = orxKEYBOARD_KEY_LSHIFT; break;}
    case GLFW_KEY_LSUPER:           {eResult = orxKEYBOARD_KEY_LSYSTEM; break;}
    case GLFW_KEY_MENU:             {eResult = orxKEYBOARD_KEY_MENU; break;}
    case '[':                       {eResult = orxKEYBOARD_KEY_LBRACKET; break;}
    case ']':                       {eResult = orxKEYBOARD_KEY_RBRACKET; break;}
    case ';':                       {eResult = orxKEYBOARD_KEY_SEMICOLON; break;}
    case ',':                       {eResult = orxKEYBOARD_KEY_COMMA; break;}
    case '.':                       {eResult = orxKEYBOARD_KEY_PERIOD; break;}
    case '\'':                      {eResult = orxKEYBOARD_KEY_QUOTE; break;}
    case '/':                       {eResult = orxKEYBOARD_KEY_SLASH; break;}
    case '\\':                      {eResult = orxKEYBOARD_KEY_BACKSLASH; break;}
    case '`':                       {eResult = orxKEYBOARD_KEY_BACKQUOTE; break;}
    case '=':                       {eResult = orxKEYBOARD_KEY_EQUAL; break;}
    case '-':                       {eResult = orxKEYBOARD_KEY_DASH; break;}
    case GLFW_KEY_F1:               {eResult = orxKEYBOARD_KEY_F1; break;}
    case GLFW_KEY_F2:               {eResult = orxKEYBOARD_KEY_F2; break;}
    case GLFW_KEY_F3:               {eResult = orxKEYBOARD_KEY_F3; break;}
    case GLFW_KEY_F4:               {eResult = orxKEYBOARD_KEY_F4; break;}
    case GLFW_KEY_F5:               {eResult = orxKEYBOARD_KEY_F5; break;}
    case GLFW_KEY_F6:               {eResult = orxKEYBOARD_KEY_F6; break;}
    case GLFW_KEY_F7:               {eResult = orxKEYBOARD_KEY_F7; break;}
    case GLFW_KEY_F8:               {eResult = orxKEYBOARD_KEY_F8; break;}
    case GLFW_KEY_F9:               {eResult = orxKEYBOARD_KEY_F9; break;}
    case GLFW_KEY_F10:              {eResult = orxKEYBOARD_KEY_F10; break;}
    case GLFW_KEY_F11:              {eResult = orxKEYBOARD_KEY_F11; break;}
    case GLFW_KEY_F12:              {eResult = orxKEYBOARD_KEY_F12; break;}
    case GLFW_KEY_F13:              {eResult = orxKEYBOARD_KEY_F13; break;}
    case GLFW_KEY_F14:              {eResult = orxKEYBOARD_KEY_F14; break;}
    case GLFW_KEY_F15:              {eResult = orxKEYBOARD_KEY_F15; break;}
    case GLFW_KEY_UP:               {eResult = orxKEYBOARD_KEY_UP; break;}
    case GLFW_KEY_RIGHT:            {eResult = orxKEYBOARD_KEY_RIGHT; break;}
    case GLFW_KEY_DOWN:             {eResult = orxKEYBOARD_KEY_DOWN; break;}
    case GLFW_KEY_LEFT:             {eResult = orxKEYBOARD_KEY_LEFT; break;}
    case GLFW_KEY_KP_0:             {eResult = orxKEYBOARD_KEY_NUMPAD_0; break;}
    case GLFW_KEY_KP_1:             {eResult = orxKEYBOARD_KEY_NUMPAD_1; break;}
    case GLFW_KEY_KP_2:             {eResult = orxKEYBOARD_KEY_NUMPAD_2; break;}
    case GLFW_KEY_KP_3:             {eResult = orxKEYBOARD_KEY_NUMPAD_3; break;}
    case GLFW_KEY_KP_4:             {eResult = orxKEYBOARD_KEY_NUMPAD_4; break;}
    case GLFW_KEY_KP_5:             {eResult = orxKEYBOARD_KEY_NUMPAD_5; break;}
    case GLFW_KEY_KP_6:             {eResult = orxKEYBOARD_KEY_NUMPAD_6; break;}
    case GLFW_KEY_KP_7:             {eResult = orxKEYBOARD_KEY_NUMPAD_7; break;}
    case GLFW_KEY_KP_8:             {eResult = orxKEYBOARD_KEY_NUMPAD_8; break;}
    case GLFW_KEY_KP_9:             {eResult = orxKEYBOARD_KEY_NUMPAD_9; break;}
    case GLFW_KEY_KP_ENTER:         {eResult = orxKEYBOARD_KEY_NUMPAD_RETURN; break;}
    case GLFW_KEY_KP_DECIMAL:       {eResult = orxKEYBOARD_KEY_NUMPAD_DECIMAL; break;}
    case GLFW_KEY_KP_NUM_LOCK:      {eResult = orxKEYBOARD_KEY_NUM_LOCK; break;}
    case GLFW_KEY_SCROLL_LOCK:      {eResult = orxKEYBOARD_KEY_SCROLL_LOCK; break;}
    case GLFW_KEY_CAPS_LOCK:        {eResult = orxKEYBOARD_KEY_CAPS_LOCK; break;}
    case 'A':                       {eResult = orxKEYBOARD_KEY_A; break;}
    case 'Z':                       {eResult = orxKEYBOARD_KEY_Z; break;}
    case 'E':                       {eResult = orxKEYBOARD_KEY_E; break;}
    case 'R':                       {eResult = orxKEYBOARD_KEY_R; break;}
    case 'T':                       {eResult = orxKEYBOARD_KEY_T; break;}
    case 'Y':                       {eResult = orxKEYBOARD_KEY_Y; break;}
    case 'U':                       {eResult = orxKEYBOARD_KEY_U; break;}
    case 'I':                       {eResult = orxKEYBOARD_KEY_I; break;}
    case 'O':                       {eResult = orxKEYBOARD_KEY_O; break;}
    case 'P':                       {eResult = orxKEYBOARD_KEY_P; break;}
    case 'Q':                       {eResult = orxKEYBOARD_KEY_Q; break;}
    case 'S':                       {eResult = orxKEYBOARD_KEY_S; break;}
    case 'D':                       {eResult = orxKEYBOARD_KEY_D; break;}
    case 'F':                       {eResult = orxKEYBOARD_KEY_F; break;}
    case 'G':                       {eResult = orxKEYBOARD_KEY_G; break;}
    case 'H':                       {eResult = orxKEYBOARD_KEY_H; break;}
    case 'J':                       {eResult = orxKEYBOARD_KEY_J; break;}
    case 'K':                       {eResult = orxKEYBOARD_KEY_K; break;}
    case 'L':                       {eResult = orxKEYBOARD_KEY_L; break;}
    case 'M':                       {eResult = orxKEYBOARD_KEY_M; break;}
    case 'W':                       {eResult = orxKEYBOARD_KEY_W; break;}
    case 'X':                       {eResult = orxKEYBOARD_KEY_X; break;}
    case 'C':                       {eResult = orxKEYBOARD_KEY_C; break;}
    case 'V':                       {eResult = orxKEYBOARD_KEY_V; break;}
    case 'B':                       {eResult = orxKEYBOARD_KEY_B; break;}
    case 'N':                       {eResult = orxKEYBOARD_KEY_N; break;}
    case '0':                       {eResult = orxKEYBOARD_KEY_0; break;}
    case '1':                       {eResult = orxKEYBOARD_KEY_1; break;}
    case '2':                       {eResult = orxKEYBOARD_KEY_2; break;}
    case '3':                       {eResult = orxKEYBOARD_KEY_3; break;}
    case '4':                       {eResult = orxKEYBOARD_KEY_4; break;}
    case '5':                       {eResult = orxKEYBOARD_KEY_5; break;}
    case '6':                       {eResult = orxKEYBOARD_KEY_6; break;}
    case '7':                       {eResult = orxKEYBOARD_KEY_7; break;}
    case '8':                       {eResult = orxKEYBOARD_KEY_8; break;}
    case '9':                       {eResult = orxKEYBOARD_KEY_9; break;}
    default:                        {eResult = orxKEYBOARD_KEY_NONE; break;}
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
    case orxKEYBOARD_KEY_BACKQUOTE:     {eResult = (GLFWKey)'`'; break;}
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
    case orxKEYBOARD_KEY_K:             {eResult = (GLFWKey)'K'; break;}
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

/** Key callback
 */
static void GLFWCALL orxKeyboard_GLFW_KeyCallback(int _iKey, int _iAction)
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
static void GLFWCALL orxKeyboard_GLFW_CharCallback(int _iKey, int _iAction)
{
  /* Stores it */
  sstKeyboard.au32CharBuffer[sstKeyboard.u32CharWriteIndex] = (orxU32)_iKey;
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

  /* Registers key callback */
  glfwSetKeyCallback(orxKeyboard_GLFW_KeyCallback);

  /* Registers char callback */
  glfwSetCharCallback(orxKeyboard_GLFW_CharCallback);

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

    /* Is GLFW window opened? */
    if(glfwGetWindowParam(GLFW_OPENED) != GL_FALSE)
    {
      /* Adds event handler */
      orxEvent_AddHandler(orxEVENT_TYPE_DISPLAY, orxKeyboard_GLFW_EventHandler);

      /* Registers key callback */
      glfwSetKeyCallback(orxKeyboard_GLFW_KeyCallback);

      /* Registers char callback */
      glfwSetCharCallback(orxKeyboard_GLFW_CharCallback);

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
    /* Unregisters key callback */
    glfwSetKeyCallback(NULL);

    /* Unregisters char callback */
    glfwSetCharCallback(NULL);

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
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_GLFW_ReadKey, KEYBOARD, READ_KEY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_GLFW_ReadString, KEYBOARD, READ_STRING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_GLFW_ClearBuffer, KEYBOARD, CLEAR_BUFFER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_GLFW_Show, KEYBOARD, SHOW);
orxPLUGIN_USER_CORE_FUNCTION_END();
