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
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/

#ifndef _orxCONFIG_H_
#define _orxCONFIG_H_


#include "orxInclude.h"

#include "math/orxVector.h"

/** Config module setup
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

/** Writes config to given file. Will overwrite any existing file, including all comments.
 * @param[in] _zFileName        File name, if null or empty the default file name will be used
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS              orxConfig_Save(orxCONST orxSTRING _zFileName);

/** Selects current working section
 * @param[in] _zSectionName     Section name to select
 */
extern orxDLLAPI orxSTATUS              orxConfig_SelectSection(orxCONST orxSTRING _zSectionName);

/** Has specified value for the given key?
 * @param[in] _zKey             Key name
 * @return orxTRUE / orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL    orxConfig_HasValue(orxCONST orxSTRING _zKey);

/** Reads an integer value from config
 * @param[in] _zKey             Key name
 * @return The value
 */
extern orxDLLAPI orxS32 orxFASTCALL     orxConfig_GetS32(orxCONST orxSTRING _zKey);

/** Reads a float value from config
 * @param[in] _zKey             Key name
 * @return The value
 */
extern orxDLLAPI orxFLOAT orxFASTCALL   orxConfig_GetFloat(orxCONST orxSTRING _zKey);

/** Reads a string value from config
 * @param[in] _zKey             Key name
 * @return The value
 */
extern orxDLLAPI orxSTRING orxFASTCALL  orxConfig_GetString(orxCONST orxSTRING _zKey);

/** Reads a boolean value from config
 * @param[in] _zKey             Key name
 * @return The value
 */
extern orxDLLAPI orxBOOL orxFASTCALL    orxConfig_GetBool(orxCONST orxSTRING _zKey);

/** Reads a vector value from config
 * @param[in]   _zKey             Key name
 * @param[out]  _pstVector        Storage for vector value  
 * @return The value
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL orxConfig_GetVector(orxCONST orxSTRING _zKey, orxVECTOR *_pstVector);


/** Writes an integer value to config
 * @param[in] _zKey             Key name
 * @param[in] _s32Value         Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxConfig_SetS32(orxCONST orxSTRING _zKey, orxS32 _s32Value);

/** Writes a float value to config
 * @param[in] _zKey             Key name
 * @param[in] _fValue           Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxConfig_SetFloat(orxCONST orxSTRING _zKey, orxFLOAT _fValue);

/** Writes a string value to config
 * @param[in] _zKey             Key name
 * @param[in] _fValue           Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxConfig_SetString(orxCONST orxSTRING _zKey, orxCONST orxSTRING _zValue);

/** Writes a boolean value to config
 * @param[in] _zKey             Key name
 * @param[in] _fValue           Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxConfig_SetBool(orxCONST orxSTRING _zKey, orxBOOL _bValue);

/** Writes a vector value to config
 * @param[in] _zKey             Key name
 * @param[in] _pstValue         Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxConfig_SetVector(orxCONST orxSTRING _zKey, orxCONST orxVECTOR *_pstValue);

#endif /*_orxCONFIG_H_*/
