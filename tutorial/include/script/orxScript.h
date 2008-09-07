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
 * @file orxScript.h
 * @date 03/05/2005
 * @author bestel@arcallians.org
 *
 * @note WARNING This API will certainly change. This module is higly work in progress
 * @note Do not make modules that depends on it while the API is not stable enough
 *
 * @todo
 * @deprecated WIP, not finished yet 
 */

/**
 * @addtogroup orxScript
 * 
 * Script module
 * Module that handles scripts
 *
 * @{
 */


#ifndef _orxSCRIPT_H_
#define _orxSCRIPT_H_
 
#include "orxInclude.h"
#include "plugin/orxPluginCore.h"

#define orxSCRIPT_MAX_PARAMS 16

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
  orxSCRIPT_TYPE aeParamsType[orxSCRIPT_MAX_PARAMS];   /**< List of parameters for each exported function */
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

/** Initializes the Script Module
 * @return Returns the status of the operation
 */
extern orxDLLAPI orxSTATUS orxScript_PluginInit();

/** Uninitializes the Script Module
 */
extern orxDLLAPI orxVOID orxScript_PluginExit();

/** Parses and runs a script file
 * @param _zFileName  (IN)  File name
 * @return Returns Success if valid parsing/execution, else returns orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxScript_RunFile(orxCONST orxSTRING _zFileName);

/** Parses and runs a script string
 * @param _zScript    (IN)  script to parse
 * @return Returns Success if valid parsing/execution, else returns orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxScript_RunString(orxCONST orxSTRING _zScript);

/** Gets a global variable type
 * @param _zVar       (IN) Variable name
 * @return Returns the type of the given variable. Returns orxSCRIPT_TYPE_NONE if variable not found.
 */
extern orxDLLAPI orxSCRIPT_TYPE orxScript_GetType(orxCONST orxSTRING _zVar);

/** Gets a global script variable value
 * @param _zVar       (IN)  Variable name
 * @param _s32OutValue (OUT) Signed 32 bits returns value
 * @return Returns orxSTATUS_SUCCESS is variable exists and is of the right type, else returns orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxScript_GetS32Value(orxCONST orxSTRING _zVar, orxS32 *_s32OutValue);

/** Gets a global script variable value
 * @param _zVar       (IN)  Variable name
 * @param _fOutValue  (OUT) Float returns value
 * @return Returns orxSTATUS_SUCCESS is variable exists and is of the right type, else returns orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxScript_GetFloatValue(orxCONST orxSTRING _zVar, orxFLOAT *_fOutValue);

/** Gets a global script variable value
 * @param _zVar       (IN)  Variable name
 * @param _zOutValue  (OUT) String returns value
 * @return Returns orxSTATUS_SUCCESS is variable exists and is of the right type, else returns orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxScript_GetStringValue(orxCONST orxSTRING _zVar, orxSTRING *_zOutValue);

/** Sets a global script variable value
 * @param _zVar       (IN) Variable name
 * @param _s32Value   (IN) Signed 32 bits value
 * @return Returns orxSTATUS_SUCCESS is variable exists and is of the right type, else returns orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxScript_SetS32Value(orxCONST orxSTRING _zVar, orxS32 _s32Value);

/** Sets a global script variable value
 * @param _zVar       (IN) Variable name
 * @param _fValue     (IN) Float value
 * @return Returns orxSTATUS_SUCCESS is variable exists and is of the right type, else returns orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxScript_SetFloatValue(orxCONST orxSTRING _zVar, orxFLOAT _fValue);

/** Sets a global script variable value
 * @param _zVar       (IN) Variable name
 * @param _zValue     (IN) String value
 * @return Returns orxSTATUS_SUCCESS is variable exists and is of the right type, else returns orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxScript_SetStringValue(orxCONST orxSTRING _zVar, orxSTRING _zValue);

/** Sets a global script variable value
 * @param _s32Index   (IN) Index of the function data from the global list
 * @return Returns orxSTATUS_SUCCESS if valid registration, else orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxScript_RegisterFunction(orxS32 _s32Index);

#endif /* _orxSCRIPT_H_ */

/** @} */
