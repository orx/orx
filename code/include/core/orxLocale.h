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
  orxLOCALE_EVENT_SELECT_LANGUAGE = 0,                  /**< Event sent when selecting a language */
  orxLOCALE_EVENT_SET_STRING,                           /**< Event sent when setting a string */

  orxLOCALE_EVENT_NUMBER,

  orxLOCALE_EVENT_NONE = orxENUM_NONE

} orxLOCALE_EVENT;

/** Locale event payload
 */
typedef struct __orxLOCALE_EVENT_PAYLOAD_t
{
  orxSTRING zLanguage;                                  /**< Current language : 4 */
  orxSTRING zStringKey;                                 /**< String key : 8 */
  orxSTRING zStringValue;                               /**< String value : 12 */

} orxLOCALE_EVENT_PAYLOAD;


/** Locale module setup
 */
extern orxDLLAPI void orxFASTCALL       orxLocale_Setup();

/** Initializes the Locale Module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxLocale_Init();

/** Exits from the Locale Module
 */
extern orxDLLAPI void orxFASTCALL       orxLocale_Exit();


/** Selects current working language
 * @param[in] _zLanguage        Language to select
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxLocale_SelectLanguage(const orxSTRING _zLanguage);

/** Gets current language
 * @return Current selected language
 */
extern orxDLLAPI const orxSTRING orxFASTCALL orxLocale_GetCurrentLanguage();

/** Has given language? (if not correctly defined, false will be returned)
 * @param[in] _zLanguage        Concerned language
 * @return orxTRUE / orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL    orxLocale_HasLanguage(const orxSTRING _zLanguage);

/** Gets language counter
 * @return Number of languages defined
 */
extern orxDLLAPI orxS32 orxFASTCALL     orxLocale_GetLanguageCounter();

/** Gets language at the given index
 * @param[in] _s32LanguageIndex Index of the desired language
 * @return orxSTRING if exist, orxSTRING_EMPTY otherwise
 */
extern orxDLLAPI const orxSTRING orxFASTCALL orxLocale_GetLanguage(orxS32 _s32LanguageIndex);


/** Has string for the given key?
 * @param[in] _zKey             Key name
 * @return orxTRUE / orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL    orxLocale_HasString(const orxSTRING _zKey);

/** Reads a string in the current language for the given key
 * @param[in] _zKey             Key name
 * @return The value
 */
extern orxDLLAPI orxSTRING orxFASTCALL  orxLocale_GetString(const orxSTRING _zKey);

/** Writes a string in the current language for the given key
 * @param[in] _zKey             Key name
 * @param[in] _zValue           Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxLocale_SetString(const orxSTRING _zKey, const orxSTRING _zValue);

/** Gets key counter for the current language
 * @return Key counter the current language if valid, 0 otherwise
 */
extern orxDLLAPI orxS32 orxFASTCALL     orxLocale_GetKeyCounter();

/** Gets key for the current language at the given index
 * @param[in] _s32KeyIndex      Index of the desired key
 * @return orxSTRING if exist, orxNULL otherwise
 */
extern orxDLLAPI const orxSTRING orxFASTCALL orxLocale_GetKey(orxS32 _s32KeyIndex);

#endif /*__orxLOCALE_H_*/

/** @} */
