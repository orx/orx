/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2021 Orx-Project
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
 * @file orxLocale.c
 * @date 15/07/2009
 * @author iarwain@orx-project.org
 *
 */


#include "core/orxLocale.h"

#include "debug/orxDebug.h"
#include "core/orxConfig.h"
#include "core/orxCommand.h"
#include "core/orxEvent.h"
#include "utils/orxString.h"


/** Module flags
 */
#define orxLOCALE_KU32_STATIC_FLAG_NONE           0x00000000  /**< No flags */

#define orxLOCALE_KU32_STATIC_FLAG_READY          0x00000001  /**< Ready flag */

#define orxLOCALE_KU32_STATIC_MASK_ALL            0xFFFFFFFF  /**< All mask */


/** Defines
 */
#define orxLOCALE_KZ_CONFIG_SECTION               "Locale"
#define orxLOCALE_KZ_CONFIG_LANGUAGE_LIST         "LanguageList"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxLOCALE_STATIC_t
{
  const orxSTRING     zCurrentLanguage;     /**< Current language */
  orxU32              u32Flags;             /**< Control flags */

} orxLOCALE_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** static data
 */
static orxLOCALE_STATIC sstLocale;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Command: SelectLanguage
 */
void orxFASTCALL orxLocale_CommandSelectLanguage(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Selects it */
  orxLocale_SelectLanguage(_astArgList[0].zValue);

  /* Updates result */
  _pstResult->zValue = (sstLocale.zCurrentLanguage != orxNULL) ? sstLocale.zCurrentLanguage : orxSTRING_EMPTY;

  /* Done! */
  return;
}

/** Command: GetCurrentLanguage
 */
void orxFASTCALL orxLocale_CommandGetCurrentLanguage(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->zValue = (sstLocale.zCurrentLanguage != orxNULL) ? sstLocale.zCurrentLanguage : orxSTRING_EMPTY;

  /* Done! */
  return;
}

/** Command: SetString
 */
void orxFASTCALL orxLocale_CommandSetString(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Sets string */
  orxLocale_SetString(_astArgList[0].zValue, _astArgList[1].zValue);

  /* Updates result */
  _pstResult->zValue = _astArgList[0].zValue;

  /* Done! */
  return;
}

/** Command: GetString
 */
void orxFASTCALL orxLocale_CommandGetString(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->zValue = orxLocale_GetString(_astArgList[0].zValue);

  /* Done! */
  return;
}

/** Registers all the locale commands
 */
static orxINLINE void orxLocale_RegisterCommands()
{
  /* Command: SelectLanguage */
  orxCOMMAND_REGISTER_CORE_COMMAND(Locale, SelectLanguage, "Language", orxCOMMAND_VAR_TYPE_STRING, 1, 0, {"Language", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: GetCurrentLanguage */
  orxCOMMAND_REGISTER_CORE_COMMAND(Locale, GetCurrentLanguage, "Language", orxCOMMAND_VAR_TYPE_STRING, 0, 0);

  /* Command: SetString */
  orxCOMMAND_REGISTER_CORE_COMMAND(Locale, SetString, "Key", orxCOMMAND_VAR_TYPE_STRING, 2, 0, {"Key", orxCOMMAND_VAR_TYPE_STRING}, {"String", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: GetString */
  orxCOMMAND_REGISTER_CORE_COMMAND(Locale, GetString, "String", orxCOMMAND_VAR_TYPE_STRING, 1, 0, {"Key", orxCOMMAND_VAR_TYPE_STRING});
}

/** Unregisters all the locale commands
 */
static orxINLINE void orxLocale_UnregisterCommands()
{
  /* Command: SelectLanguage */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Locale, SelectLanguage);
  /* Command: GetCurrentLanguage */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Locale, GetCurrentLanguage);

  /* Command: SetString */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Locale, SetString);
  /* Command: GetString */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Locale, GetString);
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Locale module setup
 */
void orxFASTCALL orxLocale_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_LOCALE, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_LOCALE, orxMODULE_ID_STRING);
  orxModule_AddDependency(orxMODULE_ID_LOCALE, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_LOCALE, orxMODULE_ID_EVENT);
  orxModule_AddDependency(orxMODULE_ID_LOCALE, orxMODULE_ID_COMMAND);

  return;
}

/** Inits the locale module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxLocale_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!orxFLAG_TEST(sstLocale.u32Flags, orxLOCALE_KU32_STATIC_FLAG_READY))
  {
    orxS32 s32LanguageCount;

    /* Inits Flags */
    orxFLAG_SET(sstLocale.u32Flags, orxLOCALE_KU32_STATIC_FLAG_READY, orxLOCALE_KU32_STATIC_MASK_ALL);

    /* Pushes locale config section */
    orxConfig_PushSection(orxLOCALE_KZ_CONFIG_SECTION);

    /* Gets language count */
    s32LanguageCount = orxConfig_GetListCount(orxLOCALE_KZ_CONFIG_LANGUAGE_LIST);

    /* Has any? */
    if(s32LanguageCount > 0)
    {
      /* Selects the first one by default */
      orxLocale_SelectLanguage(orxConfig_GetListString(orxLOCALE_KZ_CONFIG_LANGUAGE_LIST, 0));
    }

    /* Pops config section */
    orxConfig_PopSection();

    /* Registers commands */
    orxLocale_RegisterCommands();

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Tried to initialize locale module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Exits from the locale module
 */
void orxFASTCALL orxLocale_Exit()
{
  /* Initialized? */
  if(orxFLAG_TEST(sstLocale.u32Flags, orxLOCALE_KU32_STATIC_FLAG_READY))
  {
    /* Unregisters commands */
    orxLocale_UnregisterCommands();

    /* Has selected language? */
    if(sstLocale.zCurrentLanguage != orxNULL)
    {
      /* Clears internal reference */
      sstLocale.zCurrentLanguage = orxNULL;
    }

    /* Updates flags */
    orxFLAG_SET(sstLocale.u32Flags, orxLOCALE_KU32_STATIC_FLAG_NONE, orxLOCALE_KU32_STATIC_MASK_ALL);
  }

  return;
}

/** Selects current working language
 * @param[in] _zLanguage        Language to select
 */
orxSTATUS orxFASTCALL orxLocale_SelectLanguage(const orxSTRING _zLanguage)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstLocale.u32Flags, orxLOCALE_KU32_STATIC_FLAG_READY));
  orxASSERT(_zLanguage != orxNULL);

  /* Is language valid? */
  if(_zLanguage != orxSTRING_EMPTY)
  {
    orxS32 i, s32LanguageCount;

    /* Pushes locale config section */
    orxConfig_PushSection(orxLOCALE_KZ_CONFIG_SECTION);

    /* Gets language count */
    s32LanguageCount = orxConfig_GetListCount(orxLOCALE_KZ_CONFIG_LANGUAGE_LIST);

    /* For all languages */
    for(i = 0; i < s32LanguageCount; i++)
    {
      /* Found? */
      if(orxString_SearchString(orxConfig_GetListString(orxLOCALE_KZ_CONFIG_LANGUAGE_LIST, i), _zLanguage) != orxNULL)
      {
        /* Pushes its section */
        eResult = orxConfig_PushSection(_zLanguage);

        /* Success? */
        if(eResult != orxSTATUS_FAILURE)
        {
          orxLOCALE_EVENT_PAYLOAD stPayload;

          /* Stores its reference */
          sstLocale.zCurrentLanguage = orxConfig_GetCurrentSection();

          /* Pops config section */
          orxConfig_PopSection();

          /* Inits event payload */
          orxMemory_Zero(&stPayload, sizeof(orxLOCALE_EVENT_PAYLOAD));
          stPayload.zLanguage = sstLocale.zCurrentLanguage;

          /* Sends it */
          orxEVENT_SEND(orxEVENT_TYPE_LOCALE, orxLOCALE_EVENT_SELECT_LANGUAGE, orxNULL, orxNULL, &stPayload);
        }

        break;
      }
    }

    /* Pops config section */
    orxConfig_PopSection();
  }

  /* Done! */
  return eResult;
}

/** Gets current language
 * @return Current selected language
 */
const orxSTRING orxFASTCALL orxLocale_GetCurrentLanguage()
{
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstLocale.u32Flags, orxLOCALE_KU32_STATIC_FLAG_READY));

  /* Has selected language? */
  if(sstLocale.zCurrentLanguage != orxNULL)
  {
    /* Updates result */
    zResult = sstLocale.zCurrentLanguage;
  }
  else
  {
    /* Updates result */
    zResult = orxSTRING_EMPTY;
  }

  /* Done! */
  return zResult;
}

/** Has given language? (if not correctly defined, false will be returned)
 * @param[in] _zLanguage        Concerned language
 * @return orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxLocale_HasLanguage(const orxSTRING _zLanguage)
{
  orxBOOL bResult = orxFALSE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstLocale.u32Flags, orxLOCALE_KU32_STATIC_FLAG_READY));
  orxASSERT(_zLanguage != orxNULL);

  /* Valid? */
  if(_zLanguage != orxSTRING_EMPTY)
  {
    orxS32 i, s32LanguageCount;

    /* Pushes locale config section */
    orxConfig_PushSection(orxLOCALE_KZ_CONFIG_SECTION);

    /* Gets language count */
    s32LanguageCount = orxConfig_GetListCount(orxLOCALE_KZ_CONFIG_LANGUAGE_LIST);

    /* For all languages */
    for(i = 0; i < s32LanguageCount; i++)
    {
      /* Found? */
      if(orxString_SearchString(orxConfig_GetListString(orxLOCALE_KZ_CONFIG_LANGUAGE_LIST, i), _zLanguage) != orxNULL)
      {
        /* Updates result */
        bResult = orxConfig_HasSection(_zLanguage);

        break;
      }
    }

    /* Pops config section */
    orxConfig_PopSection();
  }

  /* Done! */
  return bResult;
}

/** Gets language count
 * @return Number of languages defined
 */
orxU32 orxFASTCALL orxLocale_GetLanguageCount()
{
  orxU32 u32Result;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstLocale.u32Flags, orxLOCALE_KU32_STATIC_FLAG_READY));

  /* Pushes locale section */
  orxConfig_PushSection(orxLOCALE_KZ_CONFIG_SECTION);

  /* Updates result */
  u32Result = (orxU32)orxConfig_GetListCount(orxLOCALE_KZ_CONFIG_LANGUAGE_LIST);

  /* Pops config section */
  orxConfig_PopSection();

  /* Done! */
  return u32Result;
}

/** Gets language at the given index
 * @param[in] _u32LanguageIndex Index of the desired language
 * @return orxSTRING if exist, orxSTRING_EMPTY otherwise
 */
const orxSTRING orxFASTCALL orxLocale_GetLanguage(orxU32 _u32LanguageIndex)
{
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstLocale.u32Flags, orxLOCALE_KU32_STATIC_FLAG_READY));

  /* Pushes locale section */
  orxConfig_PushSection(orxLOCALE_KZ_CONFIG_SECTION);

  /* Updates result */
  zResult = orxConfig_GetListString(orxLOCALE_KZ_CONFIG_LANGUAGE_LIST, _u32LanguageIndex);

  /* Pops config section */
  orxConfig_PopSection();

  /* Done! */
  return zResult;
}

/** Has string for the given key?
 * @param[in] _zKey             Key name
 * @return orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxLocale_HasString(const orxSTRING _zKey)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstLocale.u32Flags, orxLOCALE_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);

  /* Has current language? */
  if(sstLocale.zCurrentLanguage != orxNULL)
  {
    /* Pushes its section */
    orxConfig_PushSection(sstLocale.zCurrentLanguage);

    /* Updates result */
    bResult = orxConfig_HasValue(_zKey);

    /* Pops config section */
    orxConfig_PopSection();
  }
  else
  {
    /* Updates result */
    bResult = orxFALSE;
  }

  /* Done! */
  return bResult;
}

/** Reads a string in the current language for the given key
 * @param[in] _zKey             Key name
 * @return The value
 */
const orxSTRING orxFASTCALL orxLocale_GetString(const orxSTRING _zKey)
{
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstLocale.u32Flags, orxLOCALE_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);

  /* Has current language? */
  if(sstLocale.zCurrentLanguage != orxNULL)
  {
    /* Pushes its section */
    orxConfig_PushSection(sstLocale.zCurrentLanguage);

    /* Updates result */
    zResult = orxConfig_GetString(_zKey);

    /* Pops config section */
    orxConfig_PopSection();
  }
  else
  {
    /* Updates result */
    zResult = orxSTRING_EMPTY;
  }

  /* Done! */
  return zResult;
}

/** Writes a string in the current language for the given key
 * @param[in] _zKey             Key name
 * @param[in] _zValue           Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxLocale_SetString(const orxSTRING _zKey, const orxSTRING _zValue)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstLocale.u32Flags, orxLOCALE_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);
  orxASSERT(_zValue != orxNULL);

  /* Has current language? */
  if(sstLocale.zCurrentLanguage != orxNULL)
  {
    orxLOCALE_EVENT_PAYLOAD stPayload;

    /* Pushes its section */
    orxConfig_PushSection(sstLocale.zCurrentLanguage);

    /* Updates result */
    eResult = orxConfig_SetString(_zKey, _zValue);

    /* Pops config section */
    orxConfig_PopSection();

    /* Inits event payload */
    orxMemory_Zero(&stPayload, sizeof(orxLOCALE_EVENT_PAYLOAD));
    stPayload.zLanguage     = sstLocale.zCurrentLanguage;
    stPayload.zStringKey    = _zKey;
    stPayload.zStringValue  = _zValue;

    /* Sends it */
    orxEVENT_SEND(orxEVENT_TYPE_LOCALE, orxLOCALE_EVENT_SET_STRING, orxNULL, orxNULL, &stPayload);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets key count for the current language
 * @return Key count the current language if valid, 0 otherwise
 */
orxU32 orxFASTCALL orxLocale_GetKeyCount()
{
  orxU32 u32Result;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstLocale.u32Flags, orxLOCALE_KU32_STATIC_FLAG_READY));

  /* Has current language? */
  if(sstLocale.zCurrentLanguage != orxNULL)
  {
    /* Pushes its section */
    orxConfig_PushSection(sstLocale.zCurrentLanguage);

    /* Updates result */
    u32Result = orxConfig_GetKeyCount();

    /* Pops config section */
    orxConfig_PopSection();
  }
  else
  {
    /* Updates result */
    u32Result = 0;
  }

  /* Done! */
  return u32Result;
}

/** Gets key for the current language at the given index
 * @param[in] _u32KeyIndex      Index of the desired key
 * @return orxSTRING if exist, orxNULL otherwise
 */
const orxSTRING orxFASTCALL orxLocale_GetKey(orxU32 _u32KeyIndex)
{
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstLocale.u32Flags, orxLOCALE_KU32_STATIC_FLAG_READY));

  /* Has current language? */
  if(sstLocale.zCurrentLanguage != orxNULL)
  {
    /* Pushes its section */
    orxConfig_PushSection(sstLocale.zCurrentLanguage);

    /* Updates result */
    zResult = orxConfig_GetKey(_u32KeyIndex);

    /* Pops config section */
    orxConfig_PopSection();
  }
  else
  {
    /* Updates result */
    zResult = orxSTRING_EMPTY;
  }

  /* Done! */
  return zResult;
}
