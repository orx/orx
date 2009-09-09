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
 * @file orxPluginUser.h
 * @date 23/04/2003
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxPlugin
 * 
 * Plugin user header
 * This header is used to define structures & functions for plugin registration.
 *
 * @{
 */


#ifndef _orxPLUGIN_USER_H_
#define _orxPLUGIN_USER_H_


#include "orxInclude.h"

#include "plugin/orxPluginType.h"
#include "plugin/define/orxPlugin_CoreDefine.h"

#include "memory/orxMemory.h"
#include "utils/orxString.h"


/*********************************************
 Constants
 *********************************************/

/* Defines a user plugin entry function (that takes no arguments and return an orxSTATUS value) */
#define orxPLUGIN_DECLARE_ENTRY_POINT(ENTRY_FUNCTION)   \
extern orxIMPORT orxDLLEXPORT orxSTATUS orxPLUGIN_K_INIT_FUNCTION_NAME(orxS32 *_ps32Number, orxPLUGIN_USER_FUNCTION_INFO **_ppstInfo) \
{                                                       \
  orxSTATUS eResult;                                    \
                                                        \
  /* Calls entry point function */                      \
  eResult = ENTRY_FUNCTION();                           \
                                                        \
  /* Updates parameters */                              \
  *_ps32Number  = 0;                                    \
  *_ppstInfo    = orxNULL;                              \
                                                        \
  /* Done! */                                           \
  return eResult;                                       \
}


/*********************************************
 Structures
 *********************************************/

/* Macro for Structure Handling */
#define _orxPLUGIN_USER_FUNCTION_ADD_LOW_LEVEL(FUNCTION, FUNCTION_ID, NAME, ARGS) \
  if(u32UserPluginFunctionCounter < u32UserPluginFunctionMaxNumber) \
  { \
    pstUserPluginFunctionInfo[u32UserPluginFunctionCounter].pfnFunction   = (orxPLUGIN_FUNCTION) FUNCTION; \
    pstUserPluginFunctionInfo[u32UserPluginFunctionCounter].eFunctionID   = FUNCTION_ID; \
    pstUserPluginFunctionInfo[u32UserPluginFunctionCounter].zFunctionName = #NAME; \
    orxString_NCopy(pstUserPluginFunctionInfo[u32UserPluginFunctionCounter].zFunctionArgs, ARGS, orxPLUGIN_KU32_FUNCTION_ARG_SIZE - 1); \
    pstUserPluginFunctionInfo[u32UserPluginFunctionCounter].zFunctionArgs[orxPLUGIN_KU32_FUNCTION_ARG_SIZE - 1] = orxCHAR_NULL; \
    u32UserPluginFunctionCounter++; \
  } \
  else \
  { \
    /* Logs message */ \
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Maximum number of plugin functions reached."); \
  }

#define orxPLUGIN_USER_FUNCTION_START(STRUCTURE) \
{ \
  orxU32 u32UserPluginFunctionCounter = 0; \
  orxU32 u32UserPluginFunctionMaxNumber = sizeof(STRUCTURE) / sizeof(orxPLUGIN_USER_FUNCTION_INFO); \
  orxPLUGIN_USER_FUNCTION_INFO *pstUserPluginFunctionInfo = STRUCTURE; \
  orxMemory_Zero(pstUserPluginFunctionInfo, u32UserPluginFunctionMaxNumber * sizeof(orxPLUGIN_USER_FUNCTION_INFO));

#define orxPLUGIN_USER_FUNCTION_ADD(FUNCTION, ARGS, PLUGIN_ID, FUNCTION_BASE_ID, NAME) \
  _orxPLUGIN_USER_FUNCTION_ADD_LOW_LEVEL(&FUNCTION, \
                                         orxPLUGIN_MAKE_FUNCTION_ID(PLUGIN_ID, FUNCTION_BASE_ID), \
                                         NAME, \
                                         ARGS)

#define orxPLUGIN_USER_FUNCTION_END(NUMBER_ADDRESS, STRUCTURE_ADDRESS) \
  *NUMBER_ADDRESS = u32UserPluginFunctionCounter; \
  *STRUCTURE_ADDRESS = pstUserPluginFunctionInfo; \
}

#define orxPLUGIN_USER_CORE_FUNCTION_START(PLUGIN_SUFFIX) \
  static orxPLUGIN_USER_FUNCTION_INFO sau32##PLUGIN_SUFFIX##_Function[orxPLUGIN_FUNCTION_BASE_ID_##PLUGIN_SUFFIX##_NUMBER]; \
  extern orxIMPORT orxDLLEXPORT orxSTATUS orxPLUGIN_K_CORE_INIT_FUNCTION_NAME(PLUGIN_SUFFIX)(orxS32 *_ps32Number, orxPLUGIN_USER_FUNCTION_INFO **_ppstInfo) \
  { \
    orxSTATUS eResult = orxSTATUS_SUCCESS; \
    orxPLUGIN_USER_FUNCTION_START(sau32##PLUGIN_SUFFIX##_Function);

#define orxPLUGIN_USER_CORE_FUNCTION_ADD(FUNCTION, PLUGIN_SUFFIX, NAME_SUFFIX) \
  _orxPLUGIN_USER_FUNCTION_ADD_LOW_LEVEL(&FUNCTION, \
                                         orxPLUGIN_MAKE_CORE_FUNCTION_ID(orxPLUGIN_CORE_ID_##PLUGIN_SUFFIX, orxPLUGIN_FUNCTION_BASE_ID_##PLUGIN_SUFFIX##_##NAME_SUFFIX), \
                                         PLUGIN_SUFFIX##_##NAME_SUFFIX, \
                                         orxSTRING_EMPTY)

#define orxPLUGIN_USER_CORE_FUNCTION_END() \
  orxPLUGIN_USER_FUNCTION_END(_ps32Number, _ppstInfo); \
  return eResult; \
  }

#endif /* _orxPLUGIN_USER_H_ */

/** @} */
