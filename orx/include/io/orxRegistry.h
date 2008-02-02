/**
 * @file orxRegistry.h
 * 
 * Module for registry/config.
 */ 

 /***************************************************************************
 orxRegistry.h
 Registry/config management
 
 begin                : 09/12/2007
 author               : (C) Arcallians
 email                : cursor@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _orxREGISTRY_H_
#define _orxREGISTRY_H_
 
#include "orxInclude.h"
#include "plugin/orxPluginCore.h"

/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/** File module setup */
extern orxDLLAPI orxVOID                orxRegistry_Setup();

/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

orxPLUGIN_DECLARE_CORE_FUNCTION(orxRegistry_Init, orxSTATUS);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxRegistry_Exit, orxVOID);

orxPLUGIN_DECLARE_CORE_FUNCTION(orxRegistry_Load, orxSTATUS);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxRegistry_Save, orxSTATUS);

orxPLUGIN_DECLARE_CORE_FUNCTION(orxRegistry_GetInt32, orxS32, orxCONST orxSTRING, orxS32);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxRegistry_GetFloat, orxFLOAT, orxCONST orxSTRING, orxFLOAT);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxRegistry_GetString, orxSTRING, orxCONST orxSTRING, orxCONST orxSTRING);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxRegistry_GetBool, orxBOOL, orxCONST orxSTRING, orxBOOL);

orxPLUGIN_DECLARE_CORE_FUNCTION(orxRegistry_SetInt32, orxVOID, orxCONST orxSTRING, orxU32);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxRegistry_SetFloat, orxVOID, orxCONST orxSTRING, orxFLOAT);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxRegistry_SetString, orxVOID, orxCONST orxSTRING, orxCONST orxSTRING);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxRegistry_SetBool, orxVOID, orxCONST orxSTRING, orxBOOL);


/** Initializes the Registry Module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATIC orxINLINE orxSTATUS orxRegistry_Init()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRegistry_Init)();
}

/** Exits from the Registry Module
 */
orxSTATIC orxINLINE orxVOID orxRegistry_Exit()
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRegistry_Exit)();
}

/** Loads config registry from source
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATIC orxINLINE orxSTATUS orxRegistry_Load()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRegistry_Load)();
}

/** Saves config registry to source
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATIC orxINLINE orxSTATUS orxRegistry_Save()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRegistry_Save)();
}

/** Reads an integer value from registry
 * @param[in] _zKey             Key name
 * @param[in] _s32DefaultValue  Default value if key is not found
 * @return The value
 */
orxSTATIC orxINLINE orxS32 orxRegistry_GetInt32(orxCONST orxSTRING _zKey, orxS32 _s32DefaultValue)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRegistry_GetInt32)(_zKey, _s32DefaultValue);
}

/** Reads a float value from registry
 * @param[in] _zKey             Key name
 * @param[in] _fDefaultValue    Default value if key is not found
 * @return The value
 */
orxSTATIC orxINLINE orxFLOAT orxRegistry_GetFloat(orxCONST orxSTRING _zKey, orxFLOAT _fDefaultValue)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRegistry_GetInt32)(_zKey, _fDefaultValue);
}

/** Reads a string value from registry
 * @param[in] _zKey             Key name
 * @param[in] _zDefaultValue    Default value if key is not found
 * @return The value
 */
orxSTATIC orxINLINE orxSTRING orxRegistry_GetString(orxCONST orxSTRING _zKey, orxCONST orxSTRING _zDefaultValue)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRegistry_GetString)(_zKey, _zDefaultValue);
}

/** Reads a boolean value from registry
 * @param[in] _zKey             Key name
 * @param[in] _bDefaultValue    Default value if key is not found
 * @return The value
 */
orxSTATIC orxINLINE orxBOOL orxRegistry_GetBool(orxCONST orxSTRING _zKey, orxBOOL _bDefaultValue)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRegistry_GetBool)(_zKey, _bDefaultValue);
}

/** Writes an integer value to registry
 * @param[in] _zKey             Key name
 * @param[in] _s32Value         Value
 */
orxSTATIC orxINLINE orxVOID orxRegistry_SetInt32(orxCONST orxSTRING _zKey, orxS32 _s32Value)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRegistry_SetInt32)(_zKey, _s32Value);
}

/** Writes a float value to registry
 * @param[in] _zKey             Key name
 * @param[in] _fValue           Value
 */
orxSTATIC orxINLINE orxVOID orxRegistry_SetFloat(orxCONST orxSTRING _zKey, orxFLOAT _fValue)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRegistry_SetInt32)(_zKey, _fValue);
}

/** Writes a string value to registry
 * @param[in] _zKey             Key name
 * @param[in] _fValue           Value
 */
orxSTATIC orxINLINE orxVOID orxRegistry_SetString(orxCONST orxSTRING _zKey, orxCONST orxSTRING _zValue)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRegistry_SetString)(_zKey, _zValue);
}

/** Writes a boolean value to registry
 * @param[in] _zKey             Key name
 * @param[in] _fValue           Value
 */
orxSTATIC orxINLINE orxVOID orxRegistry_SetBool(orxCONST orxSTRING _zKey, orxBOOL _bValue)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRegistry_SetBool)(_zKey, _bValue);
}

#endif /*_orxREGISTRY_H_*/
