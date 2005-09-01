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
extern orxVOID *orxDLLAPI orxFASTCALL orxPlugin_DefaultCoreFunction(orxCONST orxSTRING _zFunctionName, orxCONST orxSTRING _zFileName, orxU32 _u32Line);


/*** Plugin Core Function Macro Definition *** */


/* *** Core function default body name *** */
#define orxPLUGIN_BODY_CORE_FUNCTION_NAME(FUNCTION_NAME) _orxCoreFunctionPointer_##FUNCTION_NAME

/* *** Core function default name *** */
#define orxPLUGIN_DEFAULT_CORE_FUNCTION_NAME(FUNCTION_NAME) _orxDefaultCoreFunction_##FUNCTION_NAME

/* *** Default core function declaration *** */
#define orxPLUGIN_DEFAULT_CORE_FUNCTION(FUNCTION_NAME)                          \
  orxSTATIC orxVOID *orxPLUGIN_DEFAULT_CORE_FUNCTION_NAME(FUNCTION_NAME)()      \
  {                                                                             \
    return(orxPlugin_DefaultCoreFunction(#FUNCTION_NAME, __FILE__, __LINE__));  \
  }                                                                             \

/* *** 0 Arg declaration macro *** */
#define orxPLUGIN_DECLARE_CORE_FUNCTION_0(FUNCTION_NAME, RETURN)                \
  orxPLUGIN_DEFAULT_CORE_FUNCTION(FUNCTION_NAME)                                \
                                                                                \
  orxSTATIC RETURN (*orxPLUGIN_BODY_CORE_FUNCTION_NAME(FUNCTION_NAME))() = (RETURN (*)()) (&orxPLUGIN_DEFAULT_CORE_FUNCTION_NAME(FUNCTION_NAME));

/* *** 1 Arg declaration macro *** */
#define orxPLUGIN_DECLARE_CORE_FUNCTION_1(FUNCTION_NAME, RETURN, ARG1)          \
  orxPLUGIN_DEFAULT_CORE_FUNCTION(FUNCTION_NAME)                                \
                                                                                \
  orxSTATIC RETURN (*orxPLUGIN_BODY_CORE_FUNCTION_NAME(FUNCTION_NAME))(ARG1 _A1) = (RETURN (*)(ARG1)) (&orxPLUGIN_DEFAULT_CORE_FUNCTION_NAME(FUNCTION_NAME));

/* *** 2 Args declaration macro *** */
#define orxPLUGIN_DECLARE_CORE_FUNCTION_2(FUNCTION_NAME, RETURN, ARG1, ARG2)    \
  orxPLUGIN_DEFAULT_CORE_FUNCTION(FUNCTION_NAME)                                \
                                                                                \
  orxSTATIC RETURN (*orxPLUGIN_BODY_CORE_FUNCTION_NAME(FUNCTION_NAME))(ARG1 _A1, ARG2 _A2) = (RETURN (*)(ARG1, ARG2)) (&orxPLUGIN_DEFAULT_CORE_FUNCTION_NAME(FUNCTION_NAME));

/* *** 3 Args declaration macro *** */
#define orxPLUGIN_DECLARE_CORE_FUNCTION_3(FUNCTION_NAME, RETURN, ARG1, ARG2, ARG3) \
  orxPLUGIN_DEFAULT_CORE_FUNCTION(FUNCTION_NAME)                                \
                                                                                \
  orxSTATIC RETURN (*orxPLUGIN_BODY_CORE_FUNCTION_NAME(FUNCTION_NAME))(ARG1 _A1, ARG2 _A2, ARG3 _A3) = (RETURN (*)(ARG1, ARG2, ARG3)) (&orxPLUGIN_DEFAULT_CORE_FUNCTION_NAME(FUNCTION_NAME));

/* *** 4 Args declaration macro *** */
#define orxPLUGIN_DECLARE_CORE_FUNCTION_4(FUNCTION_NAME, RETURN, ARG1, ARG2, ARG3, ARG4) \
  orxPLUGIN_DEFAULT_CORE_FUNCTION(FUNCTION_NAME)                                \
                                                                                \
  orxSTATIC RETURN (*orxPLUGIN_BODY_CORE_FUNCTION_NAME(FUNCTION_NAME))(ARG1 _A1, ARG2 _A2, ARG3 _A3, ARG4 _A4) = (RETURN (*)(ARG1, ARG2, ARG3, ARG4)) (&orxPLUGIN_DEFAULT_CORE_FUNCTION_NAME(FUNCTION_NAME));

/* *** 5 Args declaration macro *** */
#define orxPLUGIN_DECLARE_CORE_FUNCTION_5(FUNCTION_NAME, RETURN, ARG1, ARG2, ARG3, ARG4, ARG5) \
  orxPLUGIN_DEFAULT_CORE_FUNCTION(FUNCTION_NAME)                                \
                                                                                \
  orxSTATIC RETURN (*orxPLUGIN_BODY_CORE_FUNCTION_NAME(FUNCTION_NAME))(ARG1 _A1, ARG2 _A2, ARG3 _A3, ARG4 _A4, ARG5 _A5) = (RETURN (*)(ARG1, ARG2, ARG3, ARG4, ARG5)) (&orxPLUGIN_DEFAULT_CORE_FUNCTION_NAME(FUNCTION_NAME));


/* *** Core info array begin macro *** */
#define orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(PLUGIN_SUFFIX)                      \
  orxSTATIC orxCONST orxPLUGIN_CORE_FUNCTION sastPluginFunctionInfo_##PLUGIN_SUFFIX[orxPLUGIN_FUNCTION_BASE_ID_##PLUGIN_SUFFIX##_NUMBER] = \
  {


/* *** Core info array add macro *** */
#define orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PLUGIN_SUFFIX, FUNCTION_SUFFIX, FUNCTION_NAME) \
  {(orxPLUGIN_FUNCTION *) &orxPLUGIN_BODY_CORE_FUNCTION_NAME(FUNCTION_NAME), orxPLUGIN_FUNCTION_BASE_ID_##PLUGIN_SUFFIX##_##FUNCTION_SUFFIX},


/* *** Core info array end macro *** */
#define orxPLUGIN_END_CORE_FUNCTION_ARRAY(PLUGIN_SUFFIX)                        \
  };                                                                            \
  orxVOID orxPLUGIN_CORE_REGISTER_FUNCTION_NAME(PLUGIN_SUFFIX)()                \
  {                                                                             \
    orxPLUGIN_REGISTER_CORE_INFO(PLUGIN_SUFFIX);                                \
    return;                                                                     \
  }


/* *** Core info register macro *** */
#define orxPLUGIN_REGISTER_CORE_INFO(PLUGIN_SUFFIX)                             \
  orxPlugin_AddCoreInfo(orxPLUGIN_CORE_ID_##PLUGIN_SUFFIX, sastPluginFunctionInfo_##PLUGIN_SUFFIX, sizeof(sastPluginFunctionInfo_##PLUGIN_SUFFIX) / sizeof(orxPLUGIN_CORE_FUNCTION));





#define orxPLUGIN_DEFINE_CORE_FUNCTION(FUNCTION_NAME, RETURN, ...)                  \
    orxSTATIC orxVOID *_DEFAULT_CORE_FUNCTION_##FUNCTION_NAME()                     \
    {                                                                               \
        return(orxPlugin_DefaultCoreFunction(#FUNCTION_NAME, __FILE__, __LINE__));  \
    }                                                                               \
                                                                                    \
    RETURN (*FUNCTION_NAME) (__VA_ARGS__) = (RETURN (*) (__VA_ARGS__)) (&_DEFAULT_CORE_FUNCTION_##FUNCTION_NAME)


#endif /* _orxPLUGIN_CORE_H_ */
