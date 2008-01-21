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

orxPLUGIN_DECLARE_CORE_FUNCTION(orxRegistry_Fill, orxBOOL);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxRegistry_Flush, orxBOOL);

orxPLUGIN_DECLARE_CORE_FUNCTION(orxRegistry_GetInt32, orxS32, orxCONST orxSTRING, orxS32);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxRegistry_GetString, orxSTRING, orxCONST orxSTRING, orxCONST orxSTRING);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxRegistry_GetBool, orxBOOL, orxCONST orxSTRING, orxBOOL);

orxPLUGIN_DECLARE_CORE_FUNCTION(orxRegistry_SetInt32, orxVOID, orxCONST orxSTRING, orxU32);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxRegistry_SetString, orxVOID, orxCONST orxSTRING, orxCONST orxSTRING);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxRegistry_SetBool, orxVOID, orxCONST orxSTRING, orxBOOL);

/** Initialize the Registry Module
 */
orxSTATIC orxINLINE orxSTATUS orxRegistry_Init()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRegistry_Init)();
}

/** Uninitialize the Registry Module
 */
orxSTATIC orxINLINE orxVOID orxRegistry_Exit()
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRegistry_Exit)();
}

/** Read config registry from source.
 * @return orxFALSE if an error has occur.
 */
orxSTATIC orxINLINE orxBOOL orxRegistry_Fill()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRegistry_Fill)();
}

/** Write config registry to source.
 * @return orxFALSE if an error has occur.
 */
orxSTATIC orxINLINE orxBOOL orxRegistry_Flush()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRegistry_Flush)();
}

/** Read an integer value from registry.
 * @param _zKey            (IN) Key name
 * @param _s32DefaultValue (IN) Default value if key is not found.
 * @return The value.
 */
orxSTATIC orxINLINE orxS32 orxRegistry_GetInt32(orxCONST orxSTRING _zKey, orxS32 _s32DefaultValue)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRegistry_GetInt32)(_zKey, _s32DefaultValue);
}

/** Read a string value from registry.
 * @param _zKey          (IN) Key name
 * @param _zDefaultValue (IN) Default value if key is not found.
 * @return The value.
 */
orxSTATIC orxINLINE orxSTRING orxRegistry_GetString(orxCONST orxSTRING _zKey, orxCONST orxSTRING _zDefaultValue)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRegistry_GetString)(_zKey, _zDefaultValue);
}

/** Read a boolean value from registry.
 * @param _zKey          (IN) Key name
 * @param _bDefaultValue (IN) Default value if key is not found.
 * @return The value.
 */
orxSTATIC orxINLINE orxBOOL orxRegistry_GetBool(orxCONST orxSTRING _zKey, orxBOOL _bDefaultValue)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRegistry_GetBool)(_zKey, _bDefaultValue);
}

/** Write an integer value to registry.
 * @param _zKey     (IN) Key name
 * @param _s32Value (IN) Value.
 */
orxSTATIC orxINLINE orxVOID orxRegistry_SetInt32(orxCONST orxSTRING _zKey, orxS32 _s32Value)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRegistry_SetInt32)(_zKey, _s32Value);
}

/** Write a string value to registry.
 * @param _zKey   (IN) Key name
 * @param _zValue (IN) Value.
 */
orxSTATIC orxINLINE orxVOID orxRegistry_SetString(orxCONST orxSTRING _zKey, orxCONST orxSTRING _zValue)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRegistry_SetString)(_zKey, _zValue);
}

/** Write a boolean value from registry.
 * @param _zKey   (IN) Key name
 * @param _bValue (IN) Value.
 */
orxSTATIC orxINLINE orxVOID orxRegistry_SetBool(orxCONST orxSTRING _zKey, orxBOOL _bValue)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRegistry_SetBool)(_zKey, _bValue);
}

#endif /*_orxREGISTRY_H_*/
