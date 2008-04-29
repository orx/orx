/**
 * @file orxScript.h
 * 
 * Module for core script extension management.
 * 
 * @note WARNING This API will certainly change. This module is higly work in progress
 * @note Do not make modules that depends on it while the API is not stable enough
 */ 
 
 /***************************************************************************
 orxScript.h
 Module for core script extension management.
 
 begin                : 03/05/2005
 author               : (C) Arcallians
 email                : bestel@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/
 
#ifndef _orxSCRIPT_H_
#define _orxSCRIPT_H_
 
#include "orxInclude.h"
#include "plugin/orxPluginCore.h"

#define SCRIPT_MAX_PARAMS 16

typedef enum __orxSCRIPT_TYPE_t
{
  orxSCRIPT_TYPE_NULL,
  orxSCRIPT_TYPE_POINTER,
  orxSCRIPT_TYPE_S32,
  orxSCRIPT_TYPE_FLOAT,
  orxSCRIPT_TYPE_DOUBLE,
/*  orxSCRIPT_TYPE_STRING,*/
  orxSCRIPT_TYPE_VOID,
  
  /* Sentinel */
  orxSCRIPT_TYPE_NUMBER,

  orxSCRIPT_TYPE_NONE = orxENUM_NONE,
} orxSCRIPT_TYPE;

typedef union __orxSCRIPT_PARAM_t
{
  orxSCRIPT_TYPE eType;   /**< Parameter type */
  orxVOID   *pValue;
  
} orxSCRIPT_PARAM;

typedef orxSTATUS (*orxSCRIPT_FUNCTION_PTR)();

typedef struct __orxSCRIPT_FUNCTION_t
{
  orxSTRING zFunctionName;                          /**< String value of the function. Will be the name called from script */
  orxS32 s32NbParams;                               /**< Number of parameters */
  orxSCRIPT_TYPE aeParamsType[SCRIPT_MAX_PARAMS];   /**< List of parameters for each exported function */
  orxSCRIPT_FUNCTION_PTR pfnFunction;               /**< pointer on the function to call */
  orxS32 s32StackSize;                              /**< Stack size */
  
  /******* OPTIONAL *********/

/*  orxSTRING zFunctionDesc;           // Function description
    orxSTRING zParamsDesc[MAX_PARAMS]; // Parameters description
    orxSTRING zModuleName;             // Name of the module, for example : "Math" if function registered is orxMath_Sin
    orxSTRING zFunctionShortName;      // Name of the function without the prefix. for example : "Sin" if function registered is orxMath_Sin
*/
} orxSCRIPT_FUNCTION;


/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/** Script module setup. */
extern orxDLLAPI orxVOID orxScript_Setup();

/** Initialize the Script Module
 * @return Returns the status of the operation
 */
extern orxDLLAPI orxSTATUS orxScript_Init();

/** Uninitialize the Script Module
 */
extern orxDLLAPI orxVOID orxScript_Exit();

/** Function to register a new function in the system. 
 * This function sotre the parameters in the global list, gets the new entry index and call the plugin register function
 * @param _zFunctionName  (IN)  String value of the function
 * @param _pfnFunction    (IN)  Function pointer
 * @param _zParamTypes    (IN)  List of type for the function (the first parameter is the return type)
 */
extern orxDLLAPI orxSTATUS orxScript_RegisterFunctionGlobal(orxCONST orxSTRING _zFunctionName, orxSCRIPT_FUNCTION_PTR _pfnFunction, orxCONST orxSTRING _zParamTypes);

/** Executes a callback with input parameters, returns the result of the called function in the output
 * @param _pstFunctionInfo   (IN)  Function info
 * @param _pstInputValues    (IN)  Array of input parameters
 * @param _pstOutputValue    (OUT) Returned value from the function
 * @return Return orxSTATUS_SUCCESS if the function has been correctly executed, else orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxScript_ExecuteFunction(orxSCRIPT_FUNCTION *_pstFunctionInfo, orxCONST orxSCRIPT_PARAM *_pstInputValues, orxSCRIPT_PARAM *_pstOutputValue);

/** Returns function info from an index or orxNULL if not found
 * @param _s32Index (IN)  parameter index
 * @return  Returns the pointer on the function info if valid index, else returns orxNULL.
 */
extern orxDLLAPI orxSCRIPT_FUNCTION *orxScript_GetFunctionInfo(orxS32 _s32Index);

#define orxSCRIPT_REGISTER_FUNCTION(FUNCTION, ...) orxScript_RegisterFunctionGlobal(#FUNCTION, FUNCTION, #__VA_ARGS__)


/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

orxPLUGIN_DECLARE_CORE_FUNCTION(orxScript_PluginInit, orxSTATUS);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxScript_PluginExit, orxVOID);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxScript_RunFile, orxSTATUS, orxCONST orxSTRING);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxScript_RunString, orxSTATUS, orxCONST orxSTRING);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxScript_GetType, orxSCRIPT_TYPE, orxCONST orxSTRING);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxScript_GetS32Value, orxSTATUS, orxCONST orxSTRING, orxS32*);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxScript_GetFloatValue, orxSTATUS, orxCONST orxSTRING, orxFLOAT*);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxScript_GetStringValue, orxSTATUS, orxCONST orxSTRING, orxSTRING*);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxScript_SetS32Value, orxSTATUS, orxCONST orxSTRING, orxS32);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxScript_SetFloatValue, orxSTATUS, orxCONST orxSTRING, orxFLOAT);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxScript_SetStringValue, orxSTATUS, orxCONST orxSTRING, orxSTRING);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxScript_RegisterFunction, orxSTATUS, orxS32);

/** Initialize the Script Module
 * @return Returns the status of the operation
 */
orxSTATIC orxINLINE orxSTATUS orxScript_PluginInit()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxScript_PluginInit)();
}

/** Uninitialize the Script Module
 */
orxSTATIC orxINLINE orxVOID orxScript_PluginExit()
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxScript_PluginExit)();
}

/** Parse and run a script file
 * @param _zFileName  (IN)  File name
 * @return Returns Success if valid parsing/execution, else returns orxSTATUS_FAILURE
 */
orxSTATIC orxINLINE orxSTATUS orxScript_RunFile(orxCONST orxSTRING _zFileName)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxScript_RunFile)(_zFileName);
}

/** Parse and run a script string
 * @param _zScript    (IN)  script to parse
 * @return Returns Success if valid parsing/execution, else returns orxSTATUS_FAILURE
 */
orxSTATIC orxINLINE orxSTATUS orxScript_RunString(orxCONST orxSTRING _zScript)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxScript_RunString)(_zScript);
}


/** Gets a global variable type
 * @param _zVar       (IN) Variable name
 * @return Returns the type of the given variable. Returns orxSCRIPT_TYPE_NONE if variable not found.
 */
orxSTATIC orxINLINE orxSCRIPT_TYPE orxScript_GetType(orxCONST orxSTRING _zVar)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxScript_GetType)(_zVar);
}

/** Gets a global script variable value
 * @param _zVar       (IN)  Variable name
 * @param _bOutValue  (OUT) Signed 32 bits returns value
 * @return Returns orxSTATUS_SUCCESS is variable exists and is of the right type, else returns orxSTATUS_FAILURE
 */
orxSTATIC orxINLINE orxSTATUS orxScript_GetS32Value(orxCONST orxSTRING _zVar, orxS32 *_s32OutValue)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxScript_GetS32Value)(_zVar, _s32OutValue);
}

/** Gets a global script variable value
 * @param _zVar       (IN)  Variable name
 * @param _fOutValue  (OUT) Float returns value
 * @return Returns orxSTATUS_SUCCESS is variable exists and is of the right type, else returns orxSTATUS_FAILURE
 */
orxSTATIC orxINLINE orxSTATUS orxScript_GetFloatValue(orxCONST orxSTRING _zVar, orxFLOAT *_fOutValue)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxScript_GetFloatValue)(_zVar, _fOutValue);
}

/** Gets a global script variable value
 * @param _zVar       (IN)  Variable name
 * @param _zOutValue  (OUT) String returns value
 * @return Returns orxSTATUS_SUCCESS is variable exists and is of the right type, else returns orxSTATUS_FAILURE
 */
orxSTATIC orxINLINE orxSTATUS orxScript_GetStringValue(orxCONST orxSTRING _zVar, orxSTRING *_zOutValue)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxScript_GetStringValue)(_zVar, _zOutValue);
}

/** Sets a global script variable value
 * @param _zVar       (IN) Variable name
 * @param _s32Value   (IN) Signed 32 bits value
 * @return Returns orxSTATUS_SUCCESS is variable exists and is of the right type, else returns orxSTATUS_FAILURE
 */
orxSTATIC orxINLINE orxSTATUS orxScript_SetS32Value(orxCONST orxSTRING _zVar, orxS32 _s32Value)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxScript_SetS32Value)(_zVar, _s32Value);
}

/** Sets a global script variable value
 * @param _zVar       (IN) Variable name
 * @param _fValue     (IN) Float value
 * @return Returns orxSTATUS_SUCCESS is variable exists and is of the right type, else returns orxSTATUS_FAILURE
 */
orxSTATIC orxINLINE orxSTATUS orxScript_SetFloatValue(orxCONST orxSTRING _zVar, orxFLOAT _fValue)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxScript_SetFloatValue)(_zVar, _fValue);
}

/** Sets a global script variable value
 * @param _zVar       (IN) Variable name
 * @param _zValue     (IN) String value
 * @return Returns orxSTATUS_SUCCESS is variable exists and is of the right type, else returns orxSTATUS_FAILURE
 */
orxSTATIC orxINLINE orxSTATUS orxScript_SetStringValue(orxCONST orxSTRING _zVar, orxSTRING _zValue)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxScript_SetStringValue)(_zVar, _zValue);
}

/** Sets a global script variable value
 * @param _s32Index   (IN) Index of the function data from the global list
 * @return Returns orxSTATUS_SUCCESS if valid registration, else orxSTATUS_FAILURE
 */
orxSTATIC orxINLINE orxSTATUS orxScript_RegisterFunction(orxS32 _s32Index)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxScript_RegisterFunction)(_s32Index);
}

#endif /* _orxSCRIPT_H_ */
