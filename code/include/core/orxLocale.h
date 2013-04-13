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
 * @file orxLocale.h
 * @date 15/07/2009
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxLocale
 *
 * Localization module
 * Module that handles localized strings
 *
 * @{
 */


#ifndef __orxLOCALE_H_
#define __orxLOCALE_H_


#include "orxInclude.h"


/** Event enum
 */
typedef enum __orxLOCALE_EVENT_t
{
  orxLOCALE_EVENT_SELECT_LANGUAGE = 0,                    /**< Event sent when selecting a language */
  orxLOCALE_EVENT_SET_STRING,                             /**< Event sent when setting a string */

  orxLOCALE_EVENT_NUMBER,

  orxLOCALE_EVENT_NONE = orxENUM_NONE

} orxLOCALE_EVENT;

/** Locale event payload
 */
typedef struct __orxLOCALE_EVENT_PAYLOAD_t
{
  const orxSTRING zLanguage;                              /**< Current language : 4 */
  const orxSTRING zStringKey;                             /**< String key : 8 */
  const orxSTRING zStringValue;                           /**< String value : 12 */

} orxLOCALE_EVENT_PAYLOAD;


/** Locale module setup
 */
extern orxDLLAPI void orxFASTCALL                         orxLocale_Setup();

/** Initializes the Locale Module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                    orxLocale_Init();

/** Exits from the Locale Module
 */
extern orxDLLAPI void orxFASTCALL                         orxLocale_Exit();


/** Selects current working language
 * @param[in] _zLanguage        Language to select
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                    orxLocale_SelectLanguage(const orxSTRING _zLanguage);

/** Gets current language
 * @return Current selected language
 */
extern orxDLLAPI const orxSTRING orxFASTCALL              orxLocale_GetCurrentLanguage();

/** Has given language? (if not correctly defined, false will be returned)
 * @param[in] _zLanguage        Concerned language
 * @return orxTRUE / orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL                      orxLocale_HasLanguage(const orxSTRING _zLanguage);

/** Gets language counter
 * @return Number of languages defined
 */
extern orxDLLAPI orxU32 orxFASTCALL                       orxLocale_GetLanguageCounter();

/** Gets language at the given index
 * @param[in] _u32LanguageIndex Index of the desired language
 * @return orxSTRING if exist, orxSTRING_EMPTY otherwise
 */
extern orxDLLAPI const orxSTRING orxFASTCALL              orxLocale_GetLanguage(orxU32 _u32LanguageIndex);


/** Has string for the given key?
 * @param[in] _zKey             Key name
 * @return orxTRUE / orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL                      orxLocale_HasString(const orxSTRING _zKey);

/** Reads a string in the current language for the given key
 * @param[in] _zKey             Key name
 * @return The value
 */
extern orxDLLAPI const orxSTRING orxFASTCALL              orxLocale_GetString(const orxSTRING _zKey);

/** Writes a string in the current language for the given key
 * @param[in] _zKey             Key name
 * @param[in] _zValue           Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                    orxLocale_SetString(const orxSTRING _zKey, const orxSTRING _zValue);

/** Gets key counter for the current language
 * @return Key counter the current language if valid, 0 otherwise
 */
extern orxDLLAPI orxU32 orxFASTCALL                       orxLocale_GetKeyCounter();

/** Gets key for the current language at the given index
 * @param[in] _u32KeyIndex      Index of the desired key
 * @return orxSTRING if exist, orxNULL otherwise
 */
extern orxDLLAPI const orxSTRING orxFASTCALL              orxLocale_GetKey(orxU32 _u32KeyIndex);

#endif /*__orxLOCALE_H_*/

/** @} */
