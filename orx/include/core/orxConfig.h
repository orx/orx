/**
 * @file orxConfig.h
 * 
 * Module for config/config.
 */ 

 /***************************************************************************
 orxConfig.h
 Config/config management
 
 begin                : 02/02/2008
 author               : (C) Arcallians
 email                : cursor@arcallians.org / iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _orxCONFIG_H_
#define _orxCONFIG_H_


#include "orxInclude.h"


/** File module setup
 */
extern orxDLLAPI orxVOID                orxConfig_Setup();

/** Initializes the Config Module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS              orxConfig_Init();

/** Exits from the Config Module
 */
extern orxDLLAPI orxVOID                orxConfig_Exit();


/** Loads config config from source
 * @param[in] _zFileName        File name
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxConfig_Load(orxCONST orxSTRING _zFileName);

/** Saves config config to source
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS              orxConfig_Save();

/** Gets config file name
 * @return File name if loaded, orxSTRING_EMPTY otherwise
 */
extern orxDLLAPI orxSTRING              orxConfig_GetFileName();

/** Selects current working section
 * @param[in] _zSectionName     Section name to select
 */
extern orxDLLAPI orxVOID                orxConfig_SelectSection(orxCONST orxSTRING _zSectionName);

/** Reads an integer value from config
 * @param[in] _zKey             Key name
 * @param[in] _s32DefaultValue  Default value if key is not found
 * @return The value
 */
extern orxDLLAPI orxS32 orxFASTCALL     orxConfig_GetInt32(orxCONST orxSTRING _zKey);

/** Reads a float value from config
 * @param[in] _zKey             Key name
 * @param[in] _fDefaultValue    Default value if key is not found
 * @return The value
 */
extern orxDLLAPI orxFLOAT orxFASTCALL   orxConfig_GetFloat(orxCONST orxSTRING _zKey);

/** Reads a string value from config
 * @param[in] _zKey             Key name
 * @param[in] _zDefaultValue    Default value if key is not found
 * @return The value
 */
extern orxDLLAPI orxSTRING orxFASTCALL  orxConfig_GetString(orxCONST orxSTRING _zKey);

/** Reads a boolean value from config
 * @param[in] _zKey             Key name
 * @param[in] _bDefaultValue    Default value if key is not found
 * @return The value
 */
extern orxDLLAPI orxBOOL orxFASTCALL    orxConfig_GetBool(orxCONST orxSTRING _zKey);


/** Writes an integer value to config
 * @param[in] _zKey             Key name
 * @param[in] _s32Value         Value
 */
extern orxDLLAPI orxVOID orxFASTCALL    orxConfig_SetInt32(orxCONST orxSTRING _zKey, orxS32 _s32Value);

/** Writes a float value to config
 * @param[in] _zKey             Key name
 * @param[in] _fValue           Value
 */
extern orxDLLAPI orxVOID orxFASTCALL    orxConfig_SetFloat(orxCONST orxSTRING _zKey, orxFLOAT _fValue);

/** Writes a string value to config
 * @param[in] _zKey             Key name
 * @param[in] _fValue           Value
 */
extern orxDLLAPI orxVOID orxFASTCALL    orxConfig_SetString(orxCONST orxSTRING _zKey, orxCONST orxSTRING _zValue);

/** Writes a boolean value to config
 * @param[in] _zKey             Key name
 * @param[in] _fValue           Value
 */
extern orxDLLAPI orxVOID orxFASTCALL    orxConfig_SetBool(orxCONST orxSTRING _zKey, orxBOOL _bValue);

#endif /*_orxCONFIG_H_*/
