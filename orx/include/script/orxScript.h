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
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#ifndef _orxSCRIPT_H_
#define _orxSCRIPT_H_
 
#include "orxInclude.h"
#include "plugin/orxPluginCore.h"

typedef struct __orxSCRIPT_t orxSCRIPT;

/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

orxPLUGIN_DECLARE_CORE_FUNCTION_0(orxScript_Init, orxSTATUS);
orxPLUGIN_DECLARE_CORE_FUNCTION_0(orxScript_Exit, orxVOID);
orxPLUGIN_DECLARE_CORE_FUNCTION_0(orxScript_Create, orxSCRIPT *);
orxPLUGIN_DECLARE_CORE_FUNCTION_1(orxScript_Delete, orxVOID, orxSCRIPT *);
orxPLUGIN_DECLARE_CORE_FUNCTION_2(orxScript_LoadFile, orxSTATUS, orxSCRIPT *, orxCONST orxSTRING);
//orxPLUGIN_DECLARE_CORE_FUNCTION_4(orxScript_CallFunc, orxSTATUS, orxSCRIPT *, orxCONST orxSTRING, orxSTRING, ...);
orxPLUGIN_DECLARE_CORE_FUNCTION_2(orxScript_Execute, orxSTATUS, orxSCRIPT *, orxCONST orxSTRING);

/** Initialize the Script Module
 * @return Returns the status of the operation
 */
orxSTATIC orxINLINE orxSTATUS orxDLLAPI orxScript_Init()
{
  return orxPLUGIN_BODY_CORE_FUNCTION_NAME(orxScript_Init)();
}

/** Uninitialize the Script Module
 */
orxSTATIC orxINLINE orxVOID orxDLLAPI orxScript_Exit()
{
  orxPLUGIN_BODY_CORE_FUNCTION_NAME(orxScript_Exit)();
}

/** Create a new script instance
 * @return A pointer on a new script instance
 */
orxSTATIC orxINLINE orxSCRIPT* orxDLLAPI orxScript_Create()
{
  return orxPLUGIN_BODY_CORE_FUNCTION_NAME(orxScript_Create)();
}

/** Delete a script instance
 * @param _pstScript      (IN)     Pointer on the script instance
 */
orxSTATIC orxINLINE orxVOID orxDLLAPI orxScript_Delete(orxSCRIPT *_pstScript)
{
  orxPLUGIN_BODY_CORE_FUNCTION_NAME(orxScript_Delete)(_pstScript);
}

/** Load and execute a script file.
 * @param _pstScript      (IN)     Current script instance to use
 * @param _zFile          (IN)     File script to use
 * @return The status of the operation
 */
orxSTATIC orxINLINE orxSTATUS orxDLLAPI orxScript_LoadFile(orxSCRIPT *_pstScript, orxCONST orxSTRING _zFile)
{
  return orxPLUGIN_BODY_CORE_FUNCTION_NAME(orxScript_LoadFile)(_pstScript, _zFile);
}

/** Call a function from a loaded script.
 * @param _pstScript      (IN)     Script instance to use
 * @param _zFunction      (IN)     Function name to execute
 * @param _zArgs          (IN)     in and out variables type (List of attributes similar to printf/scanf. See detailed documentation)
 * @param va_list         (IN/OUT) Variable list of variables that match to attributes defined in _zFunction
 * @return The status of the operation
 */
/*
orxSTATIC orxINLINE orxSTATUS orxDLLAPI orxScript_CallFunc(orxSCRIPT *_pstScript, orxCONST orxSTRING _zFunction, orxSTRING _zArgs, ...)
{
  return orxPLUGIN_BODY_CORE_FUNCTION_NAME(orxScript_CallFunc)(_pstScript, _zFunction, _zArgs, ...);
}
*/

/** Execute arbitraty script code
 * @param _zScriptCode    (IN)     Script code to execute
 * @return The status of the operation
 * @note This function parse the content of the sent script code, translate it and send it to Script VM.
 * @note It might be a bit slower and must not been used for critical section.
 */
orxSTATIC orxINLINE orxSTATUS orxDLLAPI orxScript_Execute(orxSCRIPT *_pstScript, orxCONST orxSTRING _zScriptCode)
{
  return orxPLUGIN_BODY_CORE_FUNCTION_NAME(orxScript_Execute)(_pstScript, _zScriptCode);
}

#endif /* _orxSCRIPT_H_ */
