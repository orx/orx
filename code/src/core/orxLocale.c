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
 * @file orxLocale.c
 * @date 15/07/2009
 * @author iarwain@orx-project.org
 *
 */


#include "core/orxLocale.h"

#include "core/orxEvent.h"
#include "debug/orxDebug.h"
#include "core/orxConfig.h"
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
  orxSTRING           zCurrentLanguage;     /**< Current language */
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


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Locale module setup
 */
void orxFASTCALL orxLocale_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_LOCALE, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_LOCALE, orxMODULE_ID_EVENT);

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
    orxS32 s32LanguageCounter;

    /* Inits Flags */
    orxFLAG_SET(sstLocale.u32Flags, orxLOCALE_KU32_STATIC_FLAG_READY, orxLOCALE_KU32_STATIC_MASK_ALL);

    /* Pushes locale config section */
    orxConfig_PushSection(orxLOCALE_KZ_CONFIG_SECTION);

    /* Gets language counter */
    s32LanguageCounter = orxConfig_GetListCounter(orxLOCALE_KZ_CONFIG_LANGUAGE_LIST);

    /* Has any? */
    if(s32LanguageCounter > 0)
    {
      /* Selects the first one by default */
      orxLocale_SelectLanguage(orxConfig_GetListString(orxLOCALE_KZ_CONFIG_LANGUAGE_LIST, 0));
    }

    /* Pops config section */
    orxConfig_PopSection();

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
    /* Has selected language? */
    if(sstLocale.zCurrentLanguage != orxNULL)
    {
      /* Unprotects its config section */
      orxConfig_ProtectSection(sstLocale.zCurrentLanguage, orxFALSE);

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
    orxS32 i, s32LanguageCounter;

    /* Valid? */
    if(_zLanguage != orxSTRING_EMPTY)
    {
      /* Pushes locale config section */
      orxConfig_PushSection(orxLOCALE_KZ_CONFIG_SECTION);

      /* Gets language counter */
      s32LanguageCounter = orxConfig_GetListCounter(orxLOCALE_KZ_CONFIG_LANGUAGE_LIST);

      /* For all languages */
      for(i = 0; i < s32LanguageCounter; i++)
      {
        /* Found? */
        if(orxString_SearchString(orxConfig_GetListString(orxLOCALE_KZ_CONFIG_LANGUAGE_LIST, i), _zLanguage) != orxNULL)
        {
          /* Protects it */
          eResult = orxConfig_ProtectSection(_zLanguage, orxTRUE);

          /* Success? */
          if(eResult != orxSTATUS_FAILURE)
          {
            /* Pushes its section */
            eResult = orxConfig_PushSection(_zLanguage);

            /* Success? */
            if(eResult != orxSTATUS_FAILURE)
            {
              orxLOCALE_EVENT_PAYLOAD stPayload;

              /* Has selected language? */
              if(sstLocale.zCurrentLanguage != orxNULL)
              {
                /* Unprotects its config section */
                orxConfig_ProtectSection(sstLocale.zCurrentLanguage, orxFALSE);
              }

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
          }

          break;
        }
      }

      /* Pops config section */
      orxConfig_PopSection();
    }
  }

  /* Done! */
  return eResult;
}

/** Gets current language
 * @return Current selected language
 */
const orxSTRING orxFASTCALL orxLocale_GetCurrentLanguage()
{
  orxSTRING zResult;

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
    orxS32 i, s32LanguageCounter;

    /* Valid? */
    if(_zLanguage != orxSTRING_EMPTY)
    {
      /* Pushes locale config section */
      orxConfig_PushSection(orxLOCALE_KZ_CONFIG_SECTION);

      /* Gets language counter */
      s32LanguageCounter = orxConfig_GetListCounter(orxLOCALE_KZ_CONFIG_LANGUAGE_LIST);

      /* For all languages */
      for(i = 0; i < s32LanguageCounter; i++)
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
  }

  /* Done! */
  return bResult;
}

/** Gets language counter
 * @return Number of languages defined
 */
orxS32 orxFASTCALL orxLocale_GetLanguageCounter()
{
  orxS32 s32Result;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstLocale.u32Flags, orxLOCALE_KU32_STATIC_FLAG_READY));

  /* Pushes locale section */
  orxConfig_PushSection(orxLOCALE_KZ_CONFIG_SECTION);

  /* Updates result */
  s32Result = orxConfig_GetListCounter(orxLOCALE_KZ_CONFIG_LANGUAGE_LIST);

  /* Pops config section */
  orxConfig_PopSection();

  /* Done! */
  return s32Result;
}

/** Gets language at the given index
 * @param[in] _s32LanguageIndex Index of the desired language
 * @return orxSTRING if exist, orxSTRING_EMPTY otherwise
 */
const orxSTRING orxFASTCALL orxLocale_GetLanguage(orxS32 _s32LanguageIndex)
{
  orxSTRING zResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstLocale.u32Flags, orxLOCALE_KU32_STATIC_FLAG_READY));

  /* Pushes locale section */
  orxConfig_PushSection(orxLOCALE_KZ_CONFIG_SECTION);

  /* Updates result */
  zResult = orxConfig_GetListString(orxLOCALE_KZ_CONFIG_LANGUAGE_LIST, _s32LanguageIndex);

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
orxSTRING orxFASTCALL orxLocale_GetString(const orxSTRING _zKey)
{
  orxSTRING zResult;

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
    orxEVENT_SEND(orxEVENT_TYPE_LOCALE, orxLOCALE_EVENT_SELECT_LANGUAGE, orxNULL, orxNULL, &stPayload);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets key counter for the current language
 * @return Key counter the current language if valid, 0 otherwise
 */
orxS32 orxFASTCALL orxLocale_GetKeyCounter()
{
  orxS32 s32Result;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstLocale.u32Flags, orxLOCALE_KU32_STATIC_FLAG_READY));

  /* Has current language? */
  if(sstLocale.zCurrentLanguage != orxNULL)
  {
    /* Pushes its section */
    orxConfig_PushSection(sstLocale.zCurrentLanguage);

    /* Updates result */
    s32Result = orxConfig_GetKeyCounter();

    /* Pops config section */
    orxConfig_PopSection();
  }
  else
  {
    /* Updates result */
    s32Result = 0;
  }

  /* Done! */
  return s32Result;
}

/** Gets key for the current language at the given index
 * @param[in] _s32KeyIndex      Index of the desired key
 * @return orxSTRING if exist, orxNULL otherwise
 */
const orxSTRING orxFASTCALL orxLocale_GetKey(orxS32 _s32KeyIndex)
{
  orxSTRING zResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstLocale.u32Flags, orxLOCALE_KU32_STATIC_FLAG_READY));

  /* Has current language? */
  if(sstLocale.zCurrentLanguage != orxNULL)
  {
    /* Pushes its section */
    orxConfig_PushSection(sstLocale.zCurrentLanguage);

    /* Updates result */
    zResult = orxConfig_GetKey(_s32KeyIndex);

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
