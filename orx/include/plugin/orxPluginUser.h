/**
 * \file orxPluginUser.h
 * This header is used to define structures & functions for plugin registration.
 */

/*
 begin                : 23/04/2003
 author               : (C) Gdp
 email                : iarwain@ifrance.com
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef _orxPLUGIN_USER_H_
#define _orxPLUGIN_USER_H_


/*********************************************
 Constants
 *********************************************/

#include "orxInclude.h"

#include "plugin/orxPluginType.h"
#include "plugin/define/orxPlugin_CoreDefine.h"

#include "memory/orxMemory.h"
#include "utils/orxString.h"


/* Defines plugin registration info constants */
#define orxPLUGIN_USER_KZ_FUNCTION_INIT       "orxPlugin_Init" /**< Plugin init function name */

#define orxPLUGIN_KU32_NAME_SIZE              32
#define orxPLUGIN_KU32_FUNCTION_ARG_SIZE      128


/*********************************************
 Structures
 *********************************************/

/* Macro for Structure Handling */
#define _orxPLUGIN_USER_FUNCTION_ADD_LOW_LEVEL(FUNCTION, FUNCTION_ID, NAME, ARGS) \
  if(u32UserPluginFunctionCounter < u32UserPluginFunctionMaxNumber) \
  { \
    pstUserPluginFunctionInfo[u32UserPluginFunctionCounter].pfnFunction = (orxPLUGIN_FUNCTION) FUNCTION; \
    pstUserPluginFunctionInfo[u32UserPluginFunctionCounter].eFunctionID = ID; \
    orxString_Copy(pstUserPluginFunctionInfo[u32UserPluginFunctionCounter].zFunctionName, #NAME); \
    orxString_Copy(pstUserPluginFunctionInfo[u32UserPluginFunctionCounter].zFunctionArgs, ARGS); \
    u32UserPluginFunctionCounter++; \
  } \
  else \
  { \
    /* !!! MSG !!! */ \
  }

#define orxPLUGIN_USER_FUNCTION_START(STRUCTURE) \
{ \
  orxU32 u32UserPluginFunctionCounter = 0; \
  orxU32 u32UserPluginFunctionMaxNumber = sizeof(STRUCTURE) / sizeof(orxPLUGIN_USER_FUNCTION_INFO); \
  orxPLUGIN_USER_FUNCTION_INFO *pstUserPluginFunctionInfo = STRUCTURE; \
  orxMemory_Set(pstUserPluginFunctionInfo, 0, u32UserPluginFunctionMaxNumber * sizeof(orxPLUGIN_USER_FUNCTION_INFO));

#define orxPLUGIN_USER_FUNCTION_ADD(PLUGIN_ID, FUNCTION_BASE_ID, NAME) \
  _orxPLUGIN_USER_FUNCTION_ADD_LOW_LEVEL(FUNCTION, \
                                         orxPLUGIN_MAKE_FUNCTION_ID(PLUGIN_ID, FUNCTION_BASE_ID), \
                                         NAME, \
                                         orxSTRING_Empty)

#define orxPLUGIN_USER_FUNCTION_END(NUMBER_ADDRESS, STRUCTURE_ADDRESS) \
  *NUMBER_ADDRESS = u32UserPluginFunctionCounter; \
  *STRUCTURE_ADDRESS = pstUserPluginFunctionInfo; \
}

#define orxPLUGIN_USER_CORE_FUNCTION_ADD(PLUGIN_SUFFIX, FUNCTION, NAME_SUFFIX) \
  _orxPLUGIN_USER_FUNCTION_ADD_LOW_LEVEL(FUNCTION, \
                                         orxPLUGIN_MAKE_CORE_FUNCTION_ID(orxPLUGIN_CORE_ID_##PLUGIN_SUFFIX, orxPLUGIN_FUNCTION_BASE_ID_##PLUGIN_SUFFIX##_##NAME_SUFFIX), \
                                         PLUGIN_SUFFIX##NAME_SUFFIX, \
                                         orxSTRING_Empty)


/* Structure */
typedef struct __orxPLUGIN_USER_FUNCTION_INFO_t
{
  orxPLUGIN_FUNCTION pfnFunction;                           /**< Function Address */
  orxCHAR zFunctionArgs[orxPLUGIN_KU32_FUNCTION_ARG_SIZE];     /**< Function Argument Types */
  orxCHAR zFunctionName[orxPLUGIN_KU32_NAME_SIZE];          /**< Function Name */
  orxPLUGIN_FUNCTION_ID eFunctionID;                        /**< Function ID */

} orxPLUGIN_USER_FUNCTION_INFO;


/*********************************************
 Function prototypes
 *********************************************/

#endif /* _orxPLUGIN_USER_H_ */
