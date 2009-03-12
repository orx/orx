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
 * @file orxSystem.c
 * @date 26/05/2005
 * @author iarwain@orx-project.org
 *
 * @todo
 */


#include "core/orxSystem.h"
#include "plugin/orxPluginCore.h"


/***************************************************************************
 orxSystem_Setup
 System module setup.

 returns: nothing
 ***************************************************************************/
void orxSystem_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_SYSTEM, orxMODULE_ID_PLUGIN);
  orxModule_AddDependency(orxMODULE_ID_SYSTEM, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_SYSTEM, orxMODULE_ID_BANK);

  return;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION(orxSystem_Init, orxSTATUS, void);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSystem_Exit, void, void);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxSystem_GetTime, orxFLOAT, void);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSystem_GetRealTime, orxS32, void);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSystem_Delay, void, orxFLOAT);


/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(SYSTEM)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SYSTEM, INIT, orxSystem_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SYSTEM, EXIT, orxSystem_Exit)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SYSTEM, GET_TIME, orxSystem_GetTime)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SYSTEM, GET_REAL_TIME, orxSystem_GetRealTime)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SYSTEM, DELAY, orxSystem_Delay)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(SYSTEM)


/* *** Core function implementations *** */

orxSTATUS orxSystem_Init()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSystem_Init)();
}

void orxSystem_Exit()
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSystem_Exit)();
}

orxFLOAT orxSystem_GetTime()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSystem_GetTime)();
}

orxS32 orxSystem_GetRealTime()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSystem_GetRealTime)();
}

void orxSystem_Delay(orxFLOAT _fSeconds)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSystem_Delay)(_fSeconds);
}
