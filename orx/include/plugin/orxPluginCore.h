/**
 * \file orxPluginCore.h
 * This header is used to define structures & functions for core plugins info.
 */

/*
 begin                : 24/04/2003
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef _orxPLUGIN_CORE_H_
#define _orxPLUGIN_CORE_H_


/*********************************************
 Constants
 *********************************************/

#include "orxInclude.h"

#include "plugin/orxPluginType.h"
#include "plugin/define/orxPlugin_CoreDefine.h"


/*********************************************
 Structures
 *********************************************/
 
typedef struct __orxPLUGIN_CORE_FUNCTION_t
{
  orxPLUGIN_FUNCTION *pfnFunction;        /**< Function Address */
  orxPLUGIN_FUNCTION_ID eFunctionID;      /**< Function ID */

} orxPLUGIN_CORE_FUNCTION;


/*********************************************
 Function prototypes
 *********************************************/

/** Adds an info structure for the given core module.
 * Has to be called during a core module init.
 * \param ePluginCoreID the numeric id of the core plugin
 * \param _astCoreFunction the pointer on the core functions info array
 * \return nothing.
 */
extern orxVOID orxFASTCALL  orxPlugin_AddCoreInfo(orxPLUGIN_CORE_ID _ePluginCoreID, orxCONST orxPLUGIN_CORE_FUNCTION *_astCoreFunction, orxU32 _u32CoreFunctionNumber);

/** Default core plugin function.
 * Needs to be referenced by all core functions at module init.
 */
extern orxVOID *orxFASTCALL orxPlugin_DefaultCoreFunction(orxCONST orxSTRING _zFunctionName, orxCONST orxSTRING _zFileName, orxU32 _u32Line);


/*** Plugin Core Function Macro Definition *** */
#define orxPLUGIN_DEFINE_CORE_FUNCTION(FUNCTION_NAME, RETURN, ...)                  \
    orxSTATIC orxVOID *_DEFAULT_CORE_FUNCTION_##FUNCTION_NAME()                     \
    {                                                                               \
        return(orxPlugin_DefaultCoreFunction(#FUNCTION_NAME, __FILE__, __LINE__));  \
    }                                                                               \
                                                                                    \
    RETURN (*FUNCTION_NAME) (__VA_ARGS__) = (RETURN (*) (__VA_ARGS__)) (&_DEFAULT_CORE_FUNCTION_##FUNCTION_NAME)


#endif /* _orxPLUGIN_CORE_H_ */
