/**
 * \file orxPlugin_Joystick_NONE.c
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
#include "memory/orxMemory.h"

#define orxJOYSTICK_KU32_STATIC_FLAG_NONE   0x00000000  /**< No flags have been set */
#define orxJOYSTICK_KU32_STATIC_FLAG_READY  0x00000001  /**< The module has been initialized */

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

typedef struct __orxJOYSTICK_STATIC_t
{
  orxU32 u32Flags;         /**< Flags set by the joystick module */
} orxJOYSTICK_STATIC;

/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/
orxSTATIC orxJOYSTICK_STATIC sstJoystick;

/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Init joystick module
 * @return Returns the status of the operation
 */
orxSTATUS orxJoystick_Init()
{
  /* Module not already initialized ? */
  orxASSERT(!(sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY));

  /* Cleans static controller */
  orxMemory_Set(&sstJoystick, 0, sizeof(orxJOYSTICK_STATIC));
  
  /* Set module has ready */
  sstJoystick.u32Flags = orxJOYSTICK_KU32_STATIC_FLAG_READY;
  
  /* Module successfully initialized */
  return orxSTATUS_SUCCESS;
}

/** Exit joystick module
 */
orxVOID orxJoystick_Exit()
{
  /* Module initialized ? */
  orxASSERT((sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY) == orxJOYSTICK_KU32_STATIC_FLAG_READY);
  
  /* Module not ready now */
  sstJoystick.u32Flags = orxJOYSTICK_KU32_STATIC_FLAG_NONE;
}

/***************************************************************************
 * Plugin Related                                                          *
 ***************************************************************************/

orxSTATIC orxPLUGIN_USER_FUNCTION_INFO sastJoystick_Function[orxPLUGIN_FUNCTION_BASE_ID_JOYSTICK_NUMBER];

extern orxDLLEXPORT orxVOID orxPlugin_Init(orxS32 *_ps32Number, orxPLUGIN_USER_FUNCTION_INFO **_ppstInfo)
{
  orxPLUGIN_USER_FUNCTION_START(sastJoystick_Function);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_Init, JOYSTICK, INIT);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_Exit, JOYSTICK, EXIT);
  orxPLUGIN_USER_FUNCTION_END(_ps32Number, _ppstInfo);
  return;
}
