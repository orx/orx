/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2018 Orx-Project
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
 
/* Structure */
typedef struct __orxPLUGIN_CORE_FUNCTION_t
{
  orxPLUGIN_FUNCTION_ID eFunctionID;        /**< Function ID : 4 */
  orxPLUGIN_FUNCTION   *pfnFunction;        /**< Function Address : 8 */
  orxPLUGIN_FUNCTION    pfnDefaultFunction; /**< Default Function : 12 */

} orxPLUGIN_CORE_FUNCTION;

/* Structure */
typedef struct __orxPLUGIN_USER_FUNCTION_INFO_t
{
  orxPLUGIN_FUNCTION_ID eFunctionID;        /**< Function ID */
  orxPLUGIN_FUNCTION    pfnFunction;        /**< Function Address */
  orxCHAR               zFunctionArgs[orxPLUGIN_KU32_FUNCTION_ARG_SIZE]; /**< Function Argument Types */
  const orxSTRING       zFunctionName;      /**< Function Name */

} orxPLUGIN_USER_FUNCTION_INFO;


/** Plugin init function prototype
 */
typedef orxSTATUS (*orxPLUGIN_INIT_FUNCTION)(orxU32 *_peUserFunctionNumber, orxPLUGIN_USER_FUNCTION_INFO **_pastUserFunctionInfo);
typedef orxSTATUS (*orxPLUGIN_SWAP_FUNCTION)(orxPLUGIN_ENTRY_MODE _eMode);
typedef orxSTATUS (*orxPLUGIN_EXIT_FUNCTION)(orxPLUGIN_ENTRY_MODE _eMode);


/*********************************************
 Function prototypes
 *********************************************/

/** Adds an info structure for the given core module
 * Has to be called during a core module init
 * @param[in] _ePluginCoreID          The numeric id of the core plugin
 * @param[in] _eModuleID              Corresponding module ID
 * @param[in] _astCoreFunction        The pointer on the core functions info array
 * @param[in] _u32CoreFunctionNumber  Number of functions in the array
 * @return nothing.
 */
extern orxDLLAPI void orxFASTCALL     orxPlugin_AddCoreInfo(orxPLUGIN_CORE_ID _ePluginCoreID, orxMODULE_ID _eModuleID, const orxPLUGIN_CORE_FUNCTION *_astCoreFunction, orxU32 _u32CoreFunctionNumber);

#ifdef __orxEMBEDDED__

/** Binds a core plugin to its embedded implementation
 * Has to be called during a core module init
 * @param[in] _ePluginCoreID          The numeric id of the core plugin
 * @param[in] _pfnPluginInit          Embedded plug-in init function
 * @return nothing
 */
extern orxDLLAPI void orxFASTCALL     orxPlugin_BindCoreInfo(orxPLUGIN_CORE_ID _ePluginCoreID, orxPLUGIN_INIT_FUNCTION _pfnPluginInit);

#endif /* __orxEMBEDDED__ */

/** Default core plugin function
 * Needs to be referenced by all core functions at module init.
 */
extern orxDLLAPI void *orxFASTCALL    orxPlugin_DefaultCoreFunction(const orxSTRING _zFunctionName, const orxSTRING _zFileName, orxU32 _u32Line);


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
#define orxPLUGIN_DEFAULT_CORE_FUNCTION_DEFINE(FUNCTION_NAME)                               \
  extern orxDLLAPI void *orxFASTCALL orxPLUGIN_DEFAULT_CORE_FUNCTION_NAME(FUNCTION_NAME)()  \
  {                                                                                         \
    return(orxPlugin_DefaultCoreFunction(#FUNCTION_NAME, __FILE__, __LINE__));              \
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

#define orxPLUGIN_DEFINE_CORE_FUNCTION(FUNCTION_NAME, RETURN, ...)          \
  orxPLUGIN_DEFAULT_CORE_FUNCTION_DEFINE(FUNCTION_NAME)                     \
                                                                            \
  RETURN (orxFASTCALL *orxPLUGIN_CORE_FUNCTION_POINTER_NAME(FUNCTION_NAME))(__VA_ARGS__) = (RETURN(orxFASTCALL *)(__VA_ARGS__)) (&orxPLUGIN_DEFAULT_CORE_FUNCTION_NAME(FUNCTION_NAME))


/***************************************************************************
 ***************************************************************************
 ******                   CORE INFO ARRAY HANDLING                    ******
 ***************************************************************************
 ***************************************************************************/
 
/* *** Core info array begin macro *** */
#define orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(PLUGIN_SUFFIX)                                                                            \
  static const orxPLUGIN_CORE_FUNCTION sastPluginFunctionInfo_##PLUGIN_SUFFIX[orxPLUGIN_FUNCTION_BASE_ID_##PLUGIN_SUFFIX##_NUMBER] =  \
  {

/* *** Core info array add macro *** */
#define orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PLUGIN_SUFFIX, FUNCTION_SUFFIX, FUNCTION_NAME) \
  {(orxPLUGIN_FUNCTION_ID)orxPLUGIN_FUNCTION_BASE_ID_##PLUGIN_SUFFIX##_##FUNCTION_SUFFIX, (orxPLUGIN_FUNCTION *)&orxPLUGIN_CORE_FUNCTION_POINTER_NAME(FUNCTION_NAME), (orxPLUGIN_FUNCTION)&orxPLUGIN_DEFAULT_CORE_FUNCTION_NAME(FUNCTION_NAME)},

/* *** Core info array end macro *** */
#define orxPLUGIN_END_CORE_FUNCTION_ARRAY(PLUGIN_SUFFIX)                        \
  };                                                                            \
  extern orxIMPORT orxSTATUS orxPLUGIN_K_CORE_INIT_FUNCTION_NAME(PLUGIN_SUFFIX)(orxU32 *, orxPLUGIN_USER_FUNCTION_INFO **); \
  extern orxDLLAPI void orxPLUGIN_CORE_REGISTER_FUNCTION_NAME(PLUGIN_SUFFIX)()  \
  {                                                                             \
    orxPLUGIN_REGISTER_CORE_INFO(PLUGIN_SUFFIX);                                \
    return;                                                                     \
  }


/***************************************************************************
 ***************************************************************************
 ******               CORE INFO REGISTRATION HANDLING                 ******
 ***************************************************************************
 ***************************************************************************/

  #ifdef __orxEMBEDDED__

  /* *** Core info register macro *** */
  #define orxPLUGIN_REGISTER_CORE_INFO(PLUGIN_SUFFIX)                             \
    orxPlugin_AddCoreInfo(orxPLUGIN_CORE_ID_##PLUGIN_SUFFIX, orxMODULE_ID_##PLUGIN_SUFFIX, sastPluginFunctionInfo_##PLUGIN_SUFFIX, sizeof(sastPluginFunctionInfo_##PLUGIN_SUFFIX) / sizeof(orxPLUGIN_CORE_FUNCTION)); \
    orxPlugin_BindCoreInfo(orxPLUGIN_CORE_ID_##PLUGIN_SUFFIX, orxPLUGIN_K_CORE_INIT_FUNCTION_NAME(PLUGIN_SUFFIX))

  #else /* __orxEMBEDDED__ */

  /* *** Core info register macro *** */
  #define orxPLUGIN_REGISTER_CORE_INFO(PLUGIN_SUFFIX)                             \
    orxPlugin_AddCoreInfo(orxPLUGIN_CORE_ID_##PLUGIN_SUFFIX, orxMODULE_ID_##PLUGIN_SUFFIX, sastPluginFunctionInfo_##PLUGIN_SUFFIX, sizeof(sastPluginFunctionInfo_##PLUGIN_SUFFIX) / sizeof(orxPLUGIN_CORE_FUNCTION));

  #endif /* __orxEMBEDDED__ */

#endif /* _orxPLUGIN_CORE_H_ */

/** @} */
