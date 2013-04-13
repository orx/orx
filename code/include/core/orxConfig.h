/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2013 Orx-Project
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 */

/**
 * @file orxConfig.h
 * @date 02/02/2008
 * @author iarwain@orx-project.org
 *
 * @todo
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


/** Event enum
 */
typedef enum __orxCONFIG_EVENT_t
{
  orxCONFIG_EVENT_RELOAD_START = 0,                     /**< Event sent when reloading config starts */
  orxCONFIG_EVENT_RELOAD_STOP,                          /**< Event sent when reloading config stops */
  orxCONFIG_EVENT_INVALID_TYPE_ACCESS,                  /**< Event sent when accessing a value with an invalid type getter */

  orxCONFIG_EVENT_NUMBER,

  orxCONFIG_EVENT_NONE = orxENUM_NONE

} orxCONFIG_EVENT;


/** Config callback function type to use with save function */
typedef orxBOOL (orxFASTCALL *orxCONFIG_SAVE_FUNCTION)(const orxSTRING _zSectionName, const orxSTRING _zKeyName, const orxSTRING _zFileName, orxBOOL _bUseEncryption);


/** Config module setup
 */
extern orxDLLAPI void orxFASTCALL             orxConfig_Setup();

/** Initializes the Config Module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxConfig_Init();

/** Exits from the Config Module
 */
extern orxDLLAPI void orxFASTCALL             orxConfig_Exit();


/** Sets encryption key
 * @param[in] _zEncryptionKey  Encryption key to use, orxNULL to clear
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxConfig_SetEncryptionKey(const orxSTRING _zEncryptionKey);

/** Gets encryption key
 * @return Current encryption key / orxSTRING_EMPTY
 */
extern orxDLLAPI const orxSTRING orxFASTCALL  orxConfig_GetEncryptionKey();

/** Sets config base name
 * @param[in] _zBaseName        Base name used for default config file
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxConfig_SetBaseName(const orxSTRING _zBaseName);

/** Gets config main file name
 * @return Config main file name / orxSTRING_EMPTY
 */
extern orxDLLAPI const orxSTRING orxFASTCALL  orxConfig_GetMainFileName();


/** Loads config file from source
 * @param[in] _zFileName        File name
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxConfig_Load(const orxSTRING _zFileName);

/** Reloads config files from history
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxConfig_ReloadHistory();

/** Writes config to given file. Will overwrite any existing file, including all comments.
 * @param[in] _zFileName        File name, if null or empty the default file name will be used
 * @param[in] _bUseEncryption   Use file encryption to make it human non-readable?
 * @param[in] _pfnSaveCallback  Callback used to filter section/key to save. If NULL is passed, all section/keys will be saved
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxConfig_Save(const orxSTRING _zFileName, orxBOOL _bUseEncryption, const orxCONFIG_SAVE_FUNCTION _pfnSaveCallback);

/** Copies a file with optional encryption
 * @param[in] _zDstFileName     Name of the destination file
 * @param[in] _zSrcFileName     Name of the source file
 * @param[in] _zEncryptionKey   Encryption key to use when writing destination file, orxNULL for no encryption
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxConfig_CopyFile(const orxSTRING _zDstFileName, const orxSTRING _zSrcFileName, const orxSTRING _zEncryptionKey);

/** Selects current working section
 * @param[in] _zSectionName     Section name to select
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxConfig_SelectSection(const orxSTRING _zSectionName);

/** Renames a section
 * @param[in] _zSectionName     Section to rename
 * @param[in] _zNewSectionName  New name for the section
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxConfig_RenameSection(const orxSTRING _zSectionName, const orxSTRING _zNewSectionName);

/** Sets a section's parent
 * @param[in] _zSectionName     Concerned section, if the section doesn't exist, it will be created
 * @param[in] _zParentName      Parent section's name, if the section doesn't exist, it will be created, if orxNULL is provided, the former parent will be erased, if orxSTRING_EMPTY is provided, "no default parent" will be enforced
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxConfig_SetParent(const orxSTRING _zSectionName, const orxSTRING _zParentName);

/** Gets a section's parent
 * @param[in] _zSectionName     Concerned section, if the section doesn't exist, it will be created
 * @return Section's parent name / orxNULL
 */
extern orxDLLAPI const orxSTRING orxFASTCALL  orxConfig_GetParent(const orxSTRING _zSectionName);

/** Sets default parent for all sections
 * @param[in] _zSectionName     Section name that will be used as an implicit default parent section for all config sections, if orxNULL is provided, default parent will be removed
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxConfig_SetDefaultParent(const orxSTRING _zSectionName);

/** Gets current working section
 * @return Current selected section
 */
extern orxDLLAPI const orxSTRING orxFASTCALL  orxConfig_GetCurrentSection();

/** Pushes a section (storing the current one on section stack)
 * @param[in] _zSectionName     Section name to push
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxConfig_PushSection(const orxSTRING _zSectionName);

/** Pops last section from section stack
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxConfig_PopSection();

/** Has section for the given section name?
 * @param[in] _zSectionName     Section name
 * @return orxTRUE / orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL          orxConfig_HasSection(const orxSTRING _zSectionName);

/** Protects/unprotects a section from deletion (content might still be changed or deleted, but the section itself will resist delete/clear calls)
 * @param[in] _zSectionName     Section name to protect
 * @param[in] _bProtect         orxTRUE for protecting the section, orxFALSE to remove the protection
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxConfig_ProtectSection(const orxSTRING _zSectionName, orxBOOL _bProtect);

/** Gets section origin (ie. the file where it was defined for the first time or orxSTRING_EMPTY if not defined via a file)
 * @param[in] _zSectionName     Concerned section name
 * @return orxSTRING if found, orxSTRING_EMPTY otherwise
 */
extern orxDLLAPI const orxSTRING orxFASTCALL  orxConfig_GetSectionOrigin(const orxSTRING _zSectionName);

/** Gets section counter
 * @return Section counter
 */
extern orxDLLAPI orxU32 orxFASTCALL           orxConfig_GetSectionCounter();

/** Gets section at the given index
 * @param[in] _u32SectionIndex  Index of the desired section
 * @return orxSTRING if exist, orxSTRING_EMPTY otherwise
 */
extern orxDLLAPI const orxSTRING orxFASTCALL  orxConfig_GetSection(orxU32 _u32SectionIndex);


/** Clears all config info
* @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxConfig_Clear();

/** Clears section
 * @param[in] _zSectionName     Section name to clear
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxConfig_ClearSection(const orxSTRING _zSectionName);

/** Clears a value from current selected section
 * @param[in] _zKey             Key name
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxConfig_ClearValue(const orxSTRING _zKey);


/** Is this value inherited from another one?
 * @param[in] _zKey             Key name
 * @return orxTRUE / orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL          orxConfig_IsInheritedValue(const orxSTRING _zKey);

/** Has specified value for the given key?
 * @param[in] _zKey             Key name
 * @return orxTRUE / orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL          orxConfig_HasValue(const orxSTRING _zKey);

/** Reads a signed integer value from config (will take a random value if a list is provided for this key)
 * @param[in] _zKey             Key name
 * @return The value
 */
extern orxDLLAPI orxS32 orxFASTCALL           orxConfig_GetS32(const orxSTRING _zKey);

/** Reads an unsigned integer value from config (will take a random value if a list is provided for this key)
 * @param[in] _zKey             Key name
 * @return The value
 */
extern orxDLLAPI orxU32 orxFASTCALL           orxConfig_GetU32(const orxSTRING _zKey);

/** Reads a signed integer value from config (will take a random value if a list is provided for this key)
 * @param[in] _zKey             Key name
 * @return The value
 */
extern orxDLLAPI orxS64 orxFASTCALL           orxConfig_GetS64(const orxSTRING _zKey);

/** Reads an unsigned integer value from config (will take a random value if a list is provided for this key)
 * @param[in] _zKey             Key name
 * @return The value
 */
extern orxDLLAPI orxU64 orxFASTCALL           orxConfig_GetU64(const orxSTRING _zKey);

/** Reads a float value from config (will take a random value if a list is provided for this key)
 * @param[in] _zKey             Key name
 * @return The value
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxConfig_GetFloat(const orxSTRING _zKey);

/** Reads a string value from config (will take a random value if a list is provided for this key)
 * @param[in] _zKey             Key name
 * @return The value
 */
extern orxDLLAPI const orxSTRING orxFASTCALL  orxConfig_GetString(const orxSTRING _zKey);

/** Reads a boolean value from config (will take a random value if a list is provided for this key)
 * @param[in] _zKey             Key name
 * @return The value
 */
extern orxDLLAPI orxBOOL orxFASTCALL          orxConfig_GetBool(const orxSTRING _zKey);

/** Reads a vector value from config (will take a random value if a list is provided for this key)
 * @param[in]   _zKey             Key name
 * @param[out]  _pvVector         Storage for vector value
 * @return The value
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL       orxConfig_GetVector(const orxSTRING _zKey, orxVECTOR *_pvVector);

/** Duplicates a raw value (string) from config
 * @param[in] _zKey             Key name
 * @return The value. If non-null, needs to be deleted by the caller with orxString_Delete()
 */
extern orxDLLAPI orxSTRING orxFASTCALL        orxConfig_DuplicateRawValue(const orxSTRING _zKey);


/** Writes a signed integer value to config
 * @param[in] _zKey             Key name
 * @param[in] _s32Value         Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxConfig_SetS32(const orxSTRING _zKey, orxS32 _s32Value);

/** Writes an unsigned integer value to config
 * @param[in] _zKey             Key name
 * @param[in] _u32Value         Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxConfig_SetU32(const orxSTRING _zKey, orxU32 _u32Value);

/** Writes a signed integer value to config
 * @param[in] _zKey             Key name
 * @param[in] _s64Value         Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxConfig_SetS64(const orxSTRING _zKey, orxS64 _s64Value);

/** Writes an unsigned integer value to config
 * @param[in] _zKey             Key name
 * @param[in] _u64Value         Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxConfig_SetU64(const orxSTRING _zKey, orxU64 _u64Value);

/** Writes a float value to config
 * @param[in] _zKey             Key name
 * @param[in] _fValue           Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxConfig_SetFloat(const orxSTRING _zKey, orxFLOAT _fValue);

/** Writes a string value to config
 * @param[in] _zKey             Key name
 * @param[in] _zValue           Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxConfig_SetString(const orxSTRING _zKey, const orxSTRING _zValue);

/** Writes a string value to config, in block mode
 * @param[in] _zKey             Key name
 * @param[in] _zValue           Value to write in block mode
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxConfig_SetStringBlock(const orxSTRING _zKey, const orxSTRING _zValue);

/** Writes a boolean value to config
 * @param[in] _zKey             Key name
 * @param[in] _bValue           Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxConfig_SetBool(const orxSTRING _zKey, orxBOOL _bValue);

/** Writes a vector value to config
 * @param[in] _zKey             Key name
 * @param[in] _pvValue          Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxConfig_SetVector(const orxSTRING _zKey, const orxVECTOR *_pvValue);


/** Is value a list for the given key?
 * @param[in] _zKey             Key name
 * @return orxTRUE / orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL          orxConfig_IsList(const orxSTRING _zKey);

/** Gets list counter for a given key
 * @param[in] _zKey             Key name
 * @return List counter if it's a valid list, 0 otherwise
 */
extern orxDLLAPI orxS32 orxFASTCALL           orxConfig_GetListCounter(const orxSTRING _zKey);

/** Reads a signed integer value from config list
 * @param[in] _zKey             Key name
 * @param[in] _s32ListIndex     Index of desired item in list / -1 for random
 * @return The value
 */
extern orxDLLAPI orxS32 orxFASTCALL           orxConfig_GetListS32(const orxSTRING _zKey, orxS32 _s32ListIndex);

/** Reads an unsigned integer value from config list
 * @param[in] _zKey             Key name
 * @param[in] _s32ListIndex     Index of desired item in list / -1 for random
 * @return The value
 */
extern orxDLLAPI orxU32 orxFASTCALL           orxConfig_GetListU32(const orxSTRING _zKey, orxS32 _s32ListIndex);

/** Reads a signed integer value from config list
 * @param[in] _zKey             Key name
 * @param[in] _s32ListIndex     Index of desired item in list / -1 for random
 * @return The value
 */
extern orxDLLAPI orxS64 orxFASTCALL           orxConfig_GetListS64(const orxSTRING _zKey, orxS32 _s32ListIndex);

/** Reads an unsigned integer value from config list
 * @param[in] _zKey             Key name
 * @param[in] _s32ListIndex     Index of desired item in list / -1 for random
 * @return The value
 */
extern orxDLLAPI orxU64 orxFASTCALL           orxConfig_GetListU64(const orxSTRING _zKey, orxS32 _s32ListIndex);

/** Reads a float value from config list
 * @param[in] _zKey             Key name
 * @param[in] _s32ListIndex     Index of desired item in list / -1 for random
 * @return The value
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxConfig_GetListFloat(const orxSTRING _zKey, orxS32 _s32ListIndex);

/** Reads a string value from config list
 * @param[in] _zKey             Key name
 * @param[in] _s32ListIndex     Index of desired item in list / -1 for random
 * @return The value
 */
extern orxDLLAPI const orxSTRING orxFASTCALL  orxConfig_GetListString(const orxSTRING _zKey, orxS32 _s32ListIndex);

/** Reads a boolean value from config list
 * @param[in] _zKey             Key name
 * @param[in] _s32ListIndex     Index of desired item in list / -1 for random
 * @return The value
 */
extern orxDLLAPI orxBOOL orxFASTCALL          orxConfig_GetListBool(const orxSTRING _zKey, orxS32 _s32ListIndex);

/** Reads a vector value from config list
 * @param[in]   _zKey             Key name
 * @param[in]   _s32ListIndex     Index of desired item in list / -1 for random
 * @param[out]  _pvVector         Storage for vector value
 * @return The value
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL       orxConfig_GetListVector(const orxSTRING _zKey, orxS32 _s32ListIndex, orxVECTOR *_pvVector);

/** Writes a list of string values to config
 * @param[in] _zKey             Key name
 * @param[in] _azValue          Values
 * @param[in] _u32Number        Number of values
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxConfig_SetListString(const orxSTRING _zKey, const orxSTRING _azValue[], orxU32 _u32Number);

/** Gets key counter of the current section
 * @return Key counter of the current section if valid, 0 otherwise
 */
extern orxDLLAPI orxU32 orxFASTCALL           orxConfig_GetKeyCounter();

/** Gets key for the current section at the given index
 * @param[in] _u32KeyIndex      Index of the desired key
 * @return orxSTRING if exist, orxSTRING_EMPTY otherwise
 */
extern orxDLLAPI const orxSTRING orxFASTCALL  orxConfig_GetKey(orxU32 _u32KeyIndex);

#endif /*_orxCONFIG_H_*/

/** @} */
