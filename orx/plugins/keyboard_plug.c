/**
 * \file keyboard_plug.c
 */

/***************************************************************************
 begin                : 22/11/2003
 author               : (C) Gdp
 email                : iarwain@ifrance.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "include.h"

#include "plugin/plugin_user.h"

#include <allegro.h>


/********************
 *   Core Related   *
 ********************/


uint32 keyboard_init()
{
  return install_keyboard();
}

void keyboard_exit()
{
  return;
}

int32 keyboard_hit()
{
  return keypressed();
}

int32 keyboard_read()
{
  return readkey();
}

int32 keyboard_keystate_get(uint8 _uc_key)
{
  return 0;
}

void keyboard_buffer_clear()
{
  clear_keybuf();
}



/********************
 *  Plugin Related  *
 ********************/

static plugin_user_st_function_info keyboard_plug_spst_function[PLUGIN_KEYBOARD_KUL_FUNCTION_NUMBER];

void plugin_init(int32 *_pi_fn_number, plugin_user_st_function_info **_ppst_fn_info)
{
  PLUGIN_USER_FUNCTION_START(keyboard_plug_spst_function);


  PLUGIN_USER_CORE_FUNCTION_ADD(keyboard_init, KEYBOARD, INIT);

  PLUGIN_USER_CORE_FUNCTION_ADD(keyboard_exit, KEYBOARD, EXIT);

  PLUGIN_USER_CORE_FUNCTION_ADD(keyboard_hit, KEYBOARD, HIT);

  PLUGIN_USER_CORE_FUNCTION_ADD(keyboard_read, KEYBOARD, READ);

  PLUGIN_USER_CORE_FUNCTION_ADD(keyboard_keystate_get, KEYBOARD, KEYSTATE_GET);

  PLUGIN_USER_CORE_FUNCTION_ADD(keyboard_buffer_clear, KEYBOARD, BUFFER_CLEAR);


  PLUGIN_USER_FUNCTION_END(_pi_fn_number, _ppst_fn_info);

  return;
}
