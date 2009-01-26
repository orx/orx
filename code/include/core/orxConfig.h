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
 * @file orxConfig.h
 * @date 02/02/2008
 * @author iarwain@orx-project.org
 *
 * @todo
 * - Needs to refactor value getter (lot of duplicated code to handle random and lists)
 */

/**
 * @addtogroup orxConfig
 * 
 * Config module
 * Module that handles configuration files
 *
 * @{
 */


#ifndef _orxCONFIG_H_
#define _orxCONFIG_H_


#include "orxInclude.h"

#include "math/orxVector.h"


/** Config callback function type to use with save function */
typedef orxBOOL (orxFASTCALL *orxCONFIG_SAVE_FUNCTION)(orxCONST orxSTRING _zSectionName, orxCONST orxSTRING _zKeyName, orxBOOL _bUseEncryption);


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


/** Sets encryption key
 * @param[in] _zEncryptionKey  Encryption key to use, orxNULL to clear
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxConfig_SetEncryptionKey(orxCONST orxSTRING _zEncryptionKey);

/** Sets config base name
 * @param[in] _zBaseName        Base name used for default config file
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxConfig_SetBaseName(orxCONST orxSTRING _zBaseName);


/** Loads config file from source
 * @param[in] _zFileName        File name
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxConfig_Load(orxCONST orxSTRING _zFileName);

/** Reloads config files from history
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS              orxConfig_ReloadHistory();

/** Writes config to given file. Will overwrite any existing file, including all comments.
 * @param[in] _zFileName        File name, if null or empty the default file name will be used
 * @param[in] _bUseEncryption   Use file encryption to make it human non-readable?
 * @param[in] _pfnSaveCallback  Callback used to filter section/key to save. If NULL is passed, all section/keys will be saved
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxConfig_Save(orxCONST orxSTRING _zFileName, orxBOOL _bUseEncryption, orxCONST orxCONFIG_SAVE_FUNCTION _pfnSaveCallback);

/** Selects current working section
 * @param[in] _zSectionName     Section name to select
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxConfig_SelectSection(orxCONST orxSTRING _zSectionName);

/** Gets current working section
 * @return Current selected section
 */
extern orxDLLAPI orxSTRING              orxConfig_GetCurrentSection();

/** Has section for the given section name?
 * @param[in] _zSectionName     Section name
 * @return orxTRUE / orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL    orxConfig_HasSection(orxCONST orxSTRING _zSectionName);

/** Clears section
 * @param[in] _zSectionName     Section name to clear
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxConfig_ClearSection(orxCONST orxSTRING _zSectionName);

/** Clears a value from current selected section
 * @param[in] _zKey             Key name
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxConfig_ClearValue(orxCONST orxSTRING _zKey);


/** Has specified value for the given key?
 * @param[in] _zKey             Key name
 * @return orxTRUE / orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL    orxConfig_HasValue(orxCONST orxSTRING _zKey);

/** Reads a signed integer value from config (will take a random value if a list is provided for this key)
 * @param[in] _zKey             Key name
 * @return The value
 */
extern orxDLLAPI orxS32 orxFASTCALL     orxConfig_GetS32(orxCONST orxSTRING _zKey);

/** Reads an unsigned integer value from config (will take a random value if a list is provided for this key)
 * @param[in] _zKey             Key name
 * @return The value
 */
extern orxDLLAPI orxU32 orxFASTCALL     orxConfig_GetU32(orxCONST orxSTRING _zKey);

/** Reads a float value from config (will take a random value if a list is provided for this key)
 * @param[in] _zKey             Key name
 * @return The value
 */
extern orxDLLAPI orxFLOAT orxFASTCALL   orxConfig_GetFloat(orxCONST orxSTRING _zKey);

/** Reads a string value from config (will take a random value if a list is provided for this key)
 * @param[in] _zKey             Key name
 * @return The value
 */
extern orxDLLAPI orxSTRING orxFASTCALL  orxConfig_GetString(orxCONST orxSTRING _zKey);

/** Reads a boolean value from config (will take a random value if a list is provided for this key)
 * @param[in] _zKey             Key name
 * @return The value
 */
extern orxDLLAPI orxBOOL orxFASTCALL    orxConfig_GetBool(orxCONST orxSTRING _zKey);

/** Reads a vector value from config (will take a random value if a list is provided for this key)
 * @param[in]   _zKey             Key name
 * @param[out]  _pvVector         Storage for vector value
 * @return The value
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL orxConfig_GetVector(orxCONST orxSTRING _zKey, orxVECTOR *_pvVector);


/** Writes a signed integer value to config
 * @param[in] _zKey             Key name
 * @param[in] _s32Value         Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxConfig_SetS32(orxCONST orxSTRING _zKey, orxS32 _s32Value);

/** Writes an unsigned integer value to config
 * @param[in] _zKey             Key name
 * @param[in] _u32Value         Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxConfig_SetU32(orxCONST orxSTRING _zKey, orxU32 _u32Value);

/** Writes a float value to config
 * @param[in] _zKey             Key name
 * @param[in] _fValue           Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxConfig_SetFloat(orxCONST orxSTRING _zKey, orxFLOAT _fValue);

/** Writes a string value to config
 * @param[in] _zKey             Key name
 * @param[in] _zValue           Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxConfig_SetString(orxCONST orxSTRING _zKey, orxCONST orxSTRING _zValue);

/** Writes a boolean value to config
 * @param[in] _zKey             Key name
 * @param[in] _bValue           Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxConfig_SetBool(orxCONST orxSTRING _zKey, orxBOOL _bValue);

/** Writes a vector value to config
 * @param[in] _zKey             Key name
 * @param[in] _pvValue          Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxConfig_SetVector(orxCONST orxSTRING _zKey, orxCONST orxVECTOR *_pvValue);


/** Is value a list for the given key?
 * @param[in] _zKey             Key name
 * @return orxTRUE / orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL    orxConfig_IsList(orxCONST orxSTRING _zKey);

/** Gets list counter for a given key
 * @param[in] _zKey             Key name
 * @return List counter if it's a valid list, 0 otherwise
 */
extern orxDLLAPI orxS32 orxFASTCALL     orxConfig_GetListCounter(orxCONST orxSTRING _zKey);

/** Reads a signed integer value from config list
 * @param[in] _zKey             Key name
 * @param[in] _s32ListIndex     Index of desired item in list / -1 for random
 * @return The value
 */
extern orxDLLAPI orxS32 orxFASTCALL     orxConfig_GetListS32(orxCONST orxSTRING _zKey, orxS32 _s32ListIndex);

/** Reads an unsigned integer value from config list
 * @param[in] _zKey             Key name
 * @param[in] _s32ListIndex     Index of desired item in list / -1 for random
 * @return The value
 */
extern orxDLLAPI orxU32 orxFASTCALL     orxConfig_GetListU32(orxCONST orxSTRING _zKey, orxS32 _s32ListIndex);

/** Reads a float value from config list
 * @param[in] _zKey             Key name
 * @param[in] _s32ListIndex     Index of desired item in list / -1 for random
 * @return The value
 */
extern orxDLLAPI orxFLOAT orxFASTCALL   orxConfig_GetListFloat(orxCONST orxSTRING _zKey, orxS32 _s32ListIndex);

/** Reads a string value from config list
 * @param[in] _zKey             Key name
 * @param[in] _s32ListIndex     Index of desired item in list / -1 for random
 * @return The value
 */
extern orxDLLAPI orxSTRING orxFASTCALL  orxConfig_GetListString(orxCONST orxSTRING _zKey, orxS32 _s32ListIndex);

/** Reads a boolean value from config list
 * @param[in] _zKey             Key name
 * @param[in] _s32ListIndex     Index of desired item in list / -1 for random
 * @return The value
 */
extern orxDLLAPI orxBOOL orxFASTCALL    orxConfig_GetListBool(orxCONST orxSTRING _zKey, orxS32 _s32ListIndex);

/** Reads a vector value from config list
 * @param[in]   _zKey             Key name
 * @param[in]   _s32ListIndex     Index of desired item in list / -1 for random
 * @param[out]  _pvVector         Storage for vector value
 * @return The value
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL orxConfig_GetListVector(orxCONST orxSTRING _zKey, orxS32 _s32ListIndex, orxVECTOR *_pvVector);

/** Writes a list of string values to config
 * @param[in] _zKey             Key name
 * @param[in] _azValue          Values
 * @param[in] _u32Number        Number of values
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxConfig_SetStringList(orxCONST orxSTRING _zKey, orxCONST orxSTRING _azValue[], orxU32 _u32Number);

#endif /*_orxCONFIG_H_*/

/** @} */
