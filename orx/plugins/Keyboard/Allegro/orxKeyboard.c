/**
 * \file orxPlugin_Keyboard_Allegro.c
 */

/***************************************************************************
 begin                : 15/05/2005
 author               : (C) Arcallians
 email                : bestel@arcallians.org
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#include "orxInclude.h"
#include "plugin/orxPluginUser.h"
#include "debug/orxDebug.h"

#include <allegro.h>

#define orxKEYBOARD_KU32_STATIC_FLAG_NONE   0x00000000  /**< No flags have been set */
#define orxKEYBOARD_KU32_STATIC_FLAG_READY  0x00000001  /**< The module has been initialized */

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

typedef struct __orxKEYBOARD_STATIC_t
{
  orxU32 u32Flags;         /**< Flags set by the keyboard plugin module */
} orxKEYBOARD_STATIC;

/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/
orxSTATIC orxKEYBOARD_STATIC sstKeyboard;

/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Init the keyboard module
 * @return Returns the status of the operation
 */
orxSTATUS orxKeyboard_Init()
{
  /* Module not already initialized ? */
  orxASSERT(!(sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY));

  /* Cleans static controller */
  orxMemory_Set(&sstKeyboard, 0, sizeof(orxKEYBOARD_STATIC));
  
  /* Try to install allegro mouse */
  if(install_keyboard() != -1)
  {
    /* Set module has ready */
    sstKeyboard.u32Flags = orxKEYBOARD_KU32_STATIC_FLAG_READY;
  }
  
  /* Module successfully initialized ? */
  if(sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY == orxKEYBOARD_KU32_STATIC_FLAG_READY)
  {
    return orxSTATUS_SUCCESS;
  }
  else
  {
    return orxSTATUS_FAILURE;
  }
}

/** Exit the keyboard module
 */
orxVOID orxKeyboard_Exit()
{
  /* Module initialized ? */
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);
  
  /* Module not ready now */
  sstKeyboard.u32Flags = orxKEYBOARD_KU32_STATIC_FLAG_NONE;
}

/** Returns orxTRUE if there are keypresses waiting in the input buffer.
 * @return orxTRUE if keys have been pressed, else orxFALSE
 */
orxBOOL orxKeyboard_Hit()
{
  /* Module initialized ? */
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  return(orxS32)keypressed();
}

/** Returns the next character from the keyboard buffer, in ASCII format.
 * If the buffer is empty, it waits until a key is pressed.
 * The low byte of the return value contains the ASCII code of the key,
 * and the high byte the scancode.
 * @return Ascii and scancode value
 */
orxS32 orxKeyboard_Read()
{
  /* Module initialized ? */
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);
  
  return(orxS32)readkey();
}

/**  Empties the keyboard buffer.
 */
orxVOID orxKeyboard_ClearBuffer()
{
  /* Module initialized ? */
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);
  
  /* Clear buffer */
  clear_keybuf();
}


/***************************************************************************
 * Plugin Related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_FUNCTION_START(KEYBOARD);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Init, KEYBOARD, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Exit, KEYBOARD, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Hit, KEYBOARD, HIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Read, KEYBOARD, READ);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_ClearBuffer, KEYBOARD, CLEAR_BUFFER);
orxPLUGIN_USER_CORE_FUNCTION_END();
