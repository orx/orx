/**
 * \file orxPlugin_Mouse_Allegro.c
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

#include "orxInclude.h"
#include "plugin/orxPluginUser.h"
#include "debug/orxDebug.h"

#include <allegro.h>

#define orxMOUSE_KU32_FLAG_NONE  0x00000000  /**< No flags have been set */
#define orxMOUSE_KU32_FLAG_READY 0x00000001  /**< The module has been initialized */

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

typedef struct __orxMOUSE_STATIC_t
{
  orxU32 u32Flags;         /**< Flags set by the mouse plugin module */
} orxMOUSE_STATIC;

/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/
orxSTATIC orxMOUSE_STATIC sstMouse;

/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Init the mouse module
 * @return Returns the status of the operation
 */
orxSTATUS orxMouse_Init()
{
  /* Module not already initialized ? */
  orxASSERT(!(sstMouse.u32Flags & orxMOUSE_KU32_FLAG_READY));

  /* Cleans static controller */
  orxMemory_Set(&sstMouse, 0, sizeof(orxMOUSE_STATIC));
  
  /* Try to install allegro mouse */
  if (install_mouse() != -1)
  {
    /* Set module has ready */
    sstMouse.u32Flags = orxMOUSE_KU32_FLAG_READY;
  }
  
  /* Module successfully initialized ? */
  if (sstMouse.u32Flags & orxMOUSE_KU32_FLAG_READY == orxMOUSE_KU32_FLAG_READY)
  {
    return orxSTATUS_SUCCESS;
  }
  else
  {
    return orxSTATUS_FAILED;
  }
}

/** Exit the mouse module
 */
orxVOID orxMouse_Exit()
{
  /* Module initialized ? */
  orxASSERT((sstMouse.u32Flags & orxMOUSE_KU32_FLAG_READY) == orxMOUSE_KU32_FLAG_READY);
  
  /* Module not ready now */
  sstMouse.u32Flags = orxMOUSE_KU32_FLAG_NONE;
}

/** Get current mouse pointer screen coordinates
 * @param _s32x (OUT)   X coordinates
 * @param _s32y (OUT)   Y coordinates
 */
orxVOID orxMouse_GetMove(orxS32 *_ps32x, orxS32 *_ps32y)
{
  /* Module initialized ? */
  orxASSERT((sstMouse.u32Flags & orxMOUSE_KU32_FLAG_READY) == orxMOUSE_KU32_FLAG_READY);

  /* get move */
  get_mouse_mickeys((int *)_ps32x, (int *)_ps32y);
}

/***************************************************************************
 * Plugin Related                                                          *
 ***************************************************************************/

orxSTATIC orxPLUGIN_USER_FUNCTION_INFO sastMouse_Function[orxPLUGIN_FUNCTION_BASE_ID_MOUSE_NUMBER];

orxVOID orxPlugin_Init(orxS32 *_ps32Number, orxPLUGIN_USER_FUNCTION_INFO **_ppstInfos)
{
  orxPLUGIN_USER_FUNCTION_START(sastMouse_Function);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_Init, MOUSE, INIT);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_Exit, MOUSE, EXIT);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_GetMove, MOUSE, GET_MOVE);
  orxPLUGIN_USER_FUNCTION_END(_ps32Number, _ppstInfos);
  return;
}
