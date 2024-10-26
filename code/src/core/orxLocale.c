/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008- Orx-Project
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
#include "utils/orxHashTable.h"
#include "utils/orxString.h"


/** Module flags
 */
#define orxLOCALE_KU32_STATIC_FLAG_NONE           0x00000000  /**< No flags */

#define orxLOCALE_KU32_STATIC_FLAG_READY          0x00000001  /**< Ready flag */

#define orxLOCALE_KU32_STATIC_MASK_ALL            0xFFFFFFFF  /**< All mask */


/** Defines
 */
#define orxLOCALE_KU32_TABLE_SIZE                 8

#define orxLOCALE_KZ_CONFIG_SECTION               "Locale"
#define orxLOCALE_KZ_CONFIG_LANGUAGE              "Language"
#define orxLOCALE_KZ_CONFIG_LANGUAGE_LIST         "LanguageList"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxLOCALE_STATIC_t
{
  orxHASHTABLE       *pstGroupTable;        /**< Group table */
  orxU32              u32Flags;             /**< Control flags */

} orxLOCALE_STATIC;

/** Key callback context
 */
typedef struct __orxLOCALE_KEY_CONTEXT_t
{
  orxLOCALE_KEY_FUNCTION  pfnKeyCallback;
  const orxSTRING         zGroup;
  void                   *pContext;

} orxLOCALE_KEY_CONTEXT;


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
  orxSTATUS eResult;

  /* Selects it */
  eResult = orxLocale_SelectLanguage(_astArgList[0].zValue, (_u32ArgNumber > 1) ? _astArgList[1].zValue : orxNULL);

  /* Updates result */
  _pstResult->zValue = (eResult != orxSTATUS_FAILURE) ? orxLocale_GetCurrentLanguage((_u32ArgNumber > 1) ? _astArgList[1].zValue : orxNULL) : orxSTRING_EMPTY;

  /* Done! */
  return;
}

/** Command: GetCurrentLanguage
 */
void orxFASTCALL orxLocale_CommandGetCurrentLanguage(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->zValue = orxLocale_GetCurrentLanguage((_u32ArgNumber > 0) ? _astArgList[0].zValue : orxNULL);

  /* Done! */
  return;
}

/** Command: SetString
 */
void orxFASTCALL orxLocale_CommandSetString(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Sets string */
  orxLocale_SetString(_astArgList[0].zValue, _astArgList[1].zValue, (_u32ArgNumber > 2) ? _astArgList[2].zValue : orxNULL);

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
  _pstResult->zValue = orxLocale_GetString(_astArgList[0].zValue, (_u32ArgNumber > 1) ? _astArgList[1].zValue : orxNULL);

  /* Done! */
  return;
}

/** Registers all the locale commands
 */
static orxINLINE void orxLocale_RegisterCommands()
{
  /* Command: SelectLanguage */
  orxCOMMAND_REGISTER_CORE_COMMAND(Locale, SelectLanguage, "Language", orxCOMMAND_VAR_TYPE_STRING, 1, 1, {"Language", orxCOMMAND_VAR_TYPE_STRING}, {"Group = <void>", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: GetCurrentLanguage */
  orxCOMMAND_REGISTER_CORE_COMMAND(Locale, GetCurrentLanguage, "Language", orxCOMMAND_VAR_TYPE_STRING, 0, 1, {"Group = <void>", orxCOMMAND_VAR_TYPE_STRING});

  /* Command: SetString */
  orxCOMMAND_REGISTER_CORE_COMMAND(Locale, SetString, "Key", orxCOMMAND_VAR_TYPE_STRING, 2, 1, {"Key", orxCOMMAND_VAR_TYPE_STRING}, {"String", orxCOMMAND_VAR_TYPE_STRING}, {"Group = <void>", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: GetString */
  orxCOMMAND_REGISTER_CORE_COMMAND(Locale, GetString, "String", orxCOMMAND_VAR_TYPE_STRING, 1, 1, {"Key", orxCOMMAND_VAR_TYPE_STRING}, {"Group = <void>", orxCOMMAND_VAR_TYPE_STRING});

  /* Done! */
  return;
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

  /* Done! */
  return;
}

/** Key callback
 */
static orxBOOL orxFASTCALL orxLocale_KeyCallback(const orxSTRING _zKeyName, const orxSTRING _zSectionName, void *_pContext)
{
  orxLOCALE_KEY_CONTEXT  *pstContext;
  orxBOOL                 bResult;

  /* Gets context */
  pstContext = (orxLOCALE_KEY_CONTEXT *)_pContext;

  /* Runs callback */
  bResult = pstContext->pfnKeyCallback(_zKeyName, pstContext->zGroup, pstContext->pContext);

  /* Done! */
  return bResult;
}

static orxBOOL orxFASTCALL orxLocale_LoadGroupCallback(const orxSTRING _zKeyName, const orxSTRING _zSectionName, void *_pContext)
{
  orxBOOL bResult = orxTRUE;

  /* Is a group? */
  if((orxString_Compare(_zKeyName, orxLOCALE_KZ_CONFIG_LANGUAGE_LIST) != 0)
  && (orxString_Compare(_zKeyName, orxLOCALE_KZ_CONFIG_LANGUAGE) != 0))
  {
    /* Selects its language */
    orxLocale_SelectLanguage(orxConfig_GetString(_zKeyName), _zKeyName);
  }

  /* Done! */
  return bResult;
}

/** Loads locale languages selection from config
 */
static orxINLINE void orxLocale_Load()
{
  const orxSTRING zLanguage = orxNULL;

  /* Pushes locale config section */
  orxConfig_PushSection(orxLOCALE_KZ_CONFIG_SECTION);

  /* Has language? */
  if(orxConfig_HasValue(orxLOCALE_KZ_CONFIG_LANGUAGE) != orxFALSE)
  {
    /* Picks it */
    zLanguage = orxConfig_GetString(orxLOCALE_KZ_CONFIG_LANGUAGE);
  }
  /* Has language list? */
  else if(orxConfig_HasValue(orxLOCALE_KZ_CONFIG_LANGUAGE_LIST) != orxFALSE)
  {
    /* Picks its first entry */
    zLanguage = orxConfig_GetListString(orxLOCALE_KZ_CONFIG_LANGUAGE_LIST, 0);
  }

  /* Has language? */
  if(zLanguage != orxNULL)
  {
    /* Selects it by default */
    orxLocale_SelectLanguage(zLanguage, orxNULL);
  }

  /* Loads all groups */
  orxConfig_ForAllKeys(orxLocale_LoadGroupCallback, orxTRUE, orxNULL);

  /* Pops config section */
  orxConfig_PopSection();

  /* Done! */
  return;
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

  /* Done! */
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
    /* Clears static controller */
    orxMemory_Zero(&sstLocale, sizeof(orxLOCALE_STATIC));

    /* Creates group table */
    sstLocale.pstGroupTable = orxHashTable_Create(orxLOCALE_KU32_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Success? */
    if(sstLocale.pstGroupTable != orxNULL)
    {
      /* Inits Flags */
      orxFLAG_SET(sstLocale.u32Flags, orxLOCALE_KU32_STATIC_FLAG_READY, orxLOCALE_KU32_STATIC_MASK_ALL);

      /* Loads language selection from config */
      orxLocale_Load();

      /* Registers commands */
      orxLocale_RegisterCommands();

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
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

    /* Deletes group table */
    orxHashTable_Delete(sstLocale.pstGroupTable);
    sstLocale.pstGroupTable = orxNULL;

    /* Updates flags */
    orxFLAG_SET(sstLocale.u32Flags, orxLOCALE_KU32_STATIC_FLAG_NONE, orxLOCALE_KU32_STATIC_MASK_ALL);
  }

  /* Done! */
  return;
}

/** Selects current working language
 * @param[in] _zLanguage        Language to select
 * @param[in] _zGroup           Concerned group, orxNULL for default/fallback one
 */
orxSTATUS orxFASTCALL orxLocale_SelectLanguage(const orxSTRING _zLanguage, const orxSTRING _zGroup)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstLocale.u32Flags, orxLOCALE_KU32_STATIC_FLAG_READY));
  orxASSERT(_zLanguage != orxNULL);

  /* Is language valid? */
  if(*_zLanguage != orxCHAR_NULL)
  {
    /* Pushes its section */
    eResult = orxConfig_PushSection(_zLanguage);

    /* Success? */
    if(eResult != orxSTATUS_FAILURE)
    {
      const orxSTRING zLanguage;
      orxU64          u64GroupKey;

      /* Gets language */
      zLanguage = orxConfig_GetCurrentSection();

      /* Pops config section */
      orxConfig_PopSection();

      /* Gets group key */
      u64GroupKey = (_zGroup != orxNULL) ? orxString_Hash(_zGroup) : 0;

      /* Stores language */
      eResult = orxHashTable_Set(sstLocale.pstGroupTable, u64GroupKey, (void *)zLanguage);

      /* Success? */
      if(eResult != orxSTATUS_FAILURE)
      {
        orxLOCALE_EVENT_PAYLOAD stPayload;
        orxS32                  i, s32LanguageCount;
        orxBOOL                 bFound = orxFALSE;

        /* Pushes locale config section */
        orxConfig_PushSection(orxLOCALE_KZ_CONFIG_SECTION);

        /* For all languages */
        for(i = 0, s32LanguageCount = orxConfig_GetListCount(orxLOCALE_KZ_CONFIG_LANGUAGE_LIST); i < s32LanguageCount; i++)
        {
          /* Found? */
          if(orxString_Compare(_zLanguage, orxConfig_GetListString(orxLOCALE_KZ_CONFIG_LANGUAGE_LIST, i)) == 0)
          {
            /* Updates status */
            bFound = orxTRUE;
            break;
          }
        }

        /* New language? */
        if(bFound == orxFALSE)
        {
          const orxSTRING azLanguage[] = {_zLanguage};

          /* Adds it to the list */
          orxConfig_AppendListString(orxLOCALE_KZ_CONFIG_LANGUAGE_LIST, azLanguage, 1);
        }

        /* Stores it in config */
        orxConfig_SetString((_zGroup != orxNULL) ? _zGroup : orxLOCALE_KZ_CONFIG_LANGUAGE, _zLanguage);

        /* Pops config section */
        orxConfig_PopSection();

        /* Inits event payload */
        orxMemory_Zero(&stPayload, sizeof(orxLOCALE_EVENT_PAYLOAD));
        stPayload.zLanguage = zLanguage;
        stPayload.zGroup    = _zGroup;

        /* Sends it */
        orxEVENT_SEND(orxEVENT_TYPE_LOCALE, orxLOCALE_EVENT_SELECT_LANGUAGE, orxNULL, orxNULL, &stPayload);
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Gets current language
 * @param[in] _zGroup           Concerned group, orxNULL for default/fallback one
 * @return Current selected language
 */
const orxSTRING orxFASTCALL orxLocale_GetCurrentLanguage(const orxSTRING _zGroup)
{
  orxU64          u64GroupKey;
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstLocale.u32Flags, orxLOCALE_KU32_STATIC_FLAG_READY));

  /* Gets group key */
  u64GroupKey = (_zGroup != orxNULL) ? orxString_Hash(_zGroup) : 0;

  /* Updates result */
  zResult = (const orxSTRING)orxHashTable_Get(sstLocale.pstGroupTable, u64GroupKey);

  /* Not found and was group requested? */
  if((zResult == orxNULL) && (_zGroup != orxNULL))
  {
    /* Falls back to default group */
    zResult = (const orxSTRING)orxHashTable_Get(sstLocale.pstGroupTable, 0);
  }

  /* Not found? */
  if(zResult == orxNULL)
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

    /* For all languages */
    for(i = 0, s32LanguageCount = orxConfig_GetListCount(orxLOCALE_KZ_CONFIG_LANGUAGE_LIST); i < s32LanguageCount; i++)
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
 * @param[in] _zGroup           Concerned group, orxNULL for default/fallback one
 * @return orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxLocale_HasString(const orxSTRING _zKey, const orxSTRING _zGroup)
{
  const orxSTRING zLanguage;
  orxBOOL         bResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstLocale.u32Flags, orxLOCALE_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);

  /* Gets current language */
  zLanguage = orxLocale_GetCurrentLanguage(_zGroup);

  /* Found? */
  if(zLanguage != orxSTRING_EMPTY)
  {
    /* Pushes its section */
    orxConfig_PushSection(zLanguage);

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
 * @param[in] _zGroup           Concerned group, orxNULL for default/fallback one
 * @return The value
 */
const orxSTRING orxFASTCALL orxLocale_GetString(const orxSTRING _zKey, const orxSTRING _zGroup)
{
  const orxSTRING zLanguage;
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstLocale.u32Flags, orxLOCALE_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);

  /* Gets current language */
  zLanguage = orxLocale_GetCurrentLanguage(_zGroup);

  /* Found? */
  if(zLanguage != orxSTRING_EMPTY)
  {
    /* Pushes its section */
    orxConfig_PushSection(zLanguage);

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
 * @param[in] _zGroup           Concerned group, orxNULL for default/fallback one
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxLocale_SetString(const orxSTRING _zKey, const orxSTRING _zValue, const orxSTRING _zGroup)
{
  const orxSTRING zLanguage;
  orxSTATUS       eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstLocale.u32Flags, orxLOCALE_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);
  orxASSERT(_zValue != orxNULL);

  /* Gets current language */
  zLanguage = orxLocale_GetCurrentLanguage(_zGroup);

  /* Found? */
  if(zLanguage != orxSTRING_EMPTY)
  {
    orxLOCALE_EVENT_PAYLOAD stPayload;

    /* Pushes its section */
    orxConfig_PushSection(zLanguage);

    /* Updates result */
    eResult = orxConfig_SetString(_zKey, _zValue);

    /* Pops config section */
    orxConfig_PopSection();

    /* Inits event payload */
    orxMemory_Zero(&stPayload, sizeof(orxLOCALE_EVENT_PAYLOAD));
    stPayload.zLanguage     = zLanguage;
    stPayload.zGroup        = _zGroup;
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

/** Runs a callback for all keys of the current language
 * @param[in] _pfnKeyCallback   Function to run for each key. If this function returns orxFALSE, no other keys will be processed (ie. early exit)
 * @param[in] _zGroup           Concerned group, orxNULL for default/fallback one
 * @param[in] _pContext         User defined context, passed to the callback
 * @return orxSTATUS_SUCCESS if a current language is present and all keys were processed without interruption, orxSTATUS_FAILURE otherwise
 */
orxSTATUS orxFASTCALL orxLocale_ForAllKeys(const orxLOCALE_KEY_FUNCTION _pfnKeyCallback, const orxSTRING _zGroup, void *_pContext)
{
  const orxSTRING zLanguage;
  orxSTATUS       eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstLocale.u32Flags, orxLOCALE_KU32_STATIC_FLAG_READY));
  orxASSERT(_pfnKeyCallback != orxNULL);

  /* Gets current language */
  zLanguage = orxLocale_GetCurrentLanguage(_zGroup);

  /* Found? */
  if(zLanguage != orxSTRING_EMPTY)
  {
    orxLOCALE_KEY_CONTEXT stContext;

    /* Inits context */
    stContext.pfnKeyCallback  = _pfnKeyCallback;
    stContext.zGroup          = _zGroup;
    stContext.pContext        = _pContext;

    /* Pushes its section */
    orxConfig_PushSection(zLanguage);

    /* Runs callback on all keys */
    eResult = orxConfig_ForAllKeys(orxLocale_KeyCallback, orxTRUE, &stContext);

    /* Pops config section */
    orxConfig_PopSection();
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}
