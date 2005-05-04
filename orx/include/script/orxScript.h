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

typedef struct __orxSCRIPT_t orxSCRIPT;

/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/** Function that initialize the File plugin module
 */
extern orxDLLAPI orxVOID orxScript_Plugin_Init();

/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

/** Initialize the Script Module
 */
extern orxDLLAPI orxSTATUS (*orxScript_Init)();

/** Uninitialize the Script Module
 */
extern orxDLLAPI orxVOID (*orxScript_Exit)();

/** Create a new script instance
 * @return A pointer on a new script instance
 */
extern orxDLLAPI orxSCRIPT* (*orxScript_Create)();

/** Delete a script instance
 * @param _pstScript      (IN)     Pointer on the script instance
 */
extern orxDLLAPI orxVOID (*orxScript_Delete)(orxSCRIPT *_pstScript);

/** Load and execute a script file.
 * @param _pstScript      (IN)     Current script instance to use
 * @param _zFile          (IN)     File script to use
 * @return The status of the operation
 */
extern orxDLLAPI orxSTATUS (*orxScript_LoadFile)(orxSCRIPT *_pstScript, orxSTRING _zFile);

/** Call a function from a loaded script.
 * @param _pstScript      (IN)     Script instance to use
 * @param _zFunction      (IN)     Function name to execute
 * @param _zArgs          (IN)     in and out variables type (List of attributes similar to printf/scanf. See detailed documentation)
 * @param va_list         (IN/OUT) Variable list of variables that match to attributes defined in _zFunction
 * @return The status of the operation
 */
extern orxDLLAPI orxSTATUS (*orxScript_CallFunc)(orxSCRIPT *_pstScript, orxSTRING _zFunction, orxSTRING _zArgs, ...);

/** Execute arbitraty script code
 * @param _zScriptCode    (IN)     Script code to execute
 * @return The status of the operation
 * @note This function parse the content of the sent script code, translate it and send it to Script VM.
 * @note It might be a bit slower and must not been used for critical section.
 */
extern orxDLLAPI orxSTATUS (*orxScript_Execute)(orxSCRIPT *_pstScript, orxSTRING _zScriptCode);

#endif /* _orxSCRIPT_H_ */
