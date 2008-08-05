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
 * @file orxPluginCore.h
 * @date 24/04/2003
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxPlugin
 * 
 * Plugin core header
 * This header is used to define structures & functions for core plugins info.
 *
 * @{
 */


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
  orxPLUGIN_FUNCTION_ID eFunctionID;        /**< Function ID : 4 */
  orxPLUGIN_FUNCTION   *pfnFunction;        /**< Function Address : 8 */
  orxPLUGIN_FUNCTION    pfnDefaultFunction; /**< Default Function : 12 */

  orxPAD(12)                                /**< Padding */

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
extern orxDLLAPI orxVOID orxFASTCALL  orxPlugin_AddCoreInfo(orxPLUGIN_CORE_ID _ePluginCoreID, orxMODULE_ID _eModuleID, orxCONST orxPLUGIN_CORE_FUNCTION *_astCoreFunction, orxU32 _u32CoreFunctionNumber);

/** Default core plugin function.
 * Needs to be referenced by all core functions at module init.
 */
extern orxDLLAPI orxVOID *orxFASTCALL orxPlugin_DefaultCoreFunction(orxCONST orxSTRING _zFunctionName, orxCONST orxSTRING _zFileName, orxU32 _u32Line);


/***************************************************************************
 ***************************************************************************
 ******                 CORE FUNCTION NAME HANDLING                   ******
 ***************************************************************************
 ***************************************************************************/

/* *** Core function pointer name *** */
#define orxPLUGIN_CORE_FUNCTION_POINTER_NAME(FUNCTION_NAME) _orxCoreFunctionPointer_##FUNCTION_NAME

/* *** Core function default name *** */
#define orxPLUGIN_DEFAULT_CORE_FUNCTION_NAME(FUNCTION_NAME) _orxDefaultCoreFunction_##FUNCTION_NAME

/* *** Default core function declaration *** */
#define orxPLUGIN_DEFAULT_CORE_FUNCTION_DEFINE(FUNCTION_NAME)                     \
  extern orxDLLAPI orxVOID *orxPLUGIN_DEFAULT_CORE_FUNCTION_NAME(FUNCTION_NAME)() \
  {                                                                               \
    return(orxPlugin_DefaultCoreFunction(#FUNCTION_NAME, __FILE__, __LINE__));    \
  }


/***************************************************************************
 ***************************************************************************
 ******             CORE FUNCTION DEFINITION HANDLING                 ******
 ***************************************************************************
 ***************************************************************************/

/* *** Definition macro *** */
#ifdef __orxMSVC__
#pragma warning(disable : 4113)
#endif /* __orxMSVC__ */

#define orxPLUGIN_DEFINE_CORE_FUNCTION(FUNCTION_NAME, RETURN, ...)  \
  orxPLUGIN_DEFAULT_CORE_FUNCTION_DEFINE(FUNCTION_NAME)             \
                                                                    \
  RETURN (*orxPLUGIN_CORE_FUNCTION_POINTER_NAME(FUNCTION_NAME))(__VA_ARGS__) = (RETURN(*)(__VA_ARGS__)) (&orxPLUGIN_DEFAULT_CORE_FUNCTION_NAME(FUNCTION_NAME));


/***************************************************************************
 ***************************************************************************
 ******                   CORE INFO ARRAY HANDLING                    ******
 ***************************************************************************
 ***************************************************************************/
 
/* *** Core info array begin macro *** */
#define orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(PLUGIN_SUFFIX)                      \
  orxSTATIC orxCONST orxPLUGIN_CORE_FUNCTION sastPluginFunctionInfo_##PLUGIN_SUFFIX[orxPLUGIN_FUNCTION_BASE_ID_##PLUGIN_SUFFIX##_NUMBER] = \
  {

/* *** Core info array add macro *** */
#define orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PLUGIN_SUFFIX, FUNCTION_SUFFIX, FUNCTION_NAME) \
  {orxPLUGIN_FUNCTION_BASE_ID_##PLUGIN_SUFFIX##_##FUNCTION_SUFFIX, (orxPLUGIN_FUNCTION *)&orxPLUGIN_CORE_FUNCTION_POINTER_NAME(FUNCTION_NAME), (orxPLUGIN_FUNCTION)&orxPLUGIN_DEFAULT_CORE_FUNCTION_NAME(FUNCTION_NAME)},

/* *** Core info array end macro *** */
#define orxPLUGIN_END_CORE_FUNCTION_ARRAY(PLUGIN_SUFFIX)                        \
  };                                                                            \
  extern orxDLLAPI orxVOID orxPLUGIN_CORE_REGISTER_FUNCTION_NAME(PLUGIN_SUFFIX)()                \
  {                                                                             \
    orxPLUGIN_REGISTER_CORE_INFO(PLUGIN_SUFFIX);                                \
    return;                                                                     \
  }


/***************************************************************************
 ***************************************************************************
 ******               CORE INFO REGISTRATION HANDLING                 ******
 ***************************************************************************
 ***************************************************************************/

/* *** Core info register macro *** */
#define orxPLUGIN_REGISTER_CORE_INFO(PLUGIN_SUFFIX)                             \
  orxPlugin_AddCoreInfo(orxPLUGIN_CORE_ID_##PLUGIN_SUFFIX, orxMODULE_ID_##PLUGIN_SUFFIX, sastPluginFunctionInfo_##PLUGIN_SUFFIX, sizeof(sastPluginFunctionInfo_##PLUGIN_SUFFIX) / sizeof(orxPLUGIN_CORE_FUNCTION));

#endif /* _orxPLUGIN_CORE_H_ */

/** @} */
