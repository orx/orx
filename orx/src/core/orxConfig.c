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
 * @file orxConfig.c
 * @date 09/12/2007
 * @author iarwain@orx-project.org
 *
 * @todo
 */


#include<stdio.h>


#include "orxInclude.h"

#include "core/orxConfig.h"
#include "debug/orxDebug.h"
#include "memory/orxBank.h"
#include "math/orxMath.h"
#include "utils/orxString.h"


/** Module flags
 */
#define orxCONFIG_KU32_STATIC_FLAG_NONE     0x00000000  /**< No flags */

#define orxCONFIG_KU32_STATIC_FLAG_READY    0x00000001  /**< Ready flag */
#define orxCONFIG_KU32_STATIC_FLAG_HISTORY  0x00000002  /**< Keep history flag */

#define orxCONFIG_KU32_STATIC_MASK_ALL      0xFFFFFFFF  /**< All mask */


/** Defines
 */
#define orxCONFIG_KU32_SECTION_BANK_SIZE    16          /**< Default section bank size */
#define orxCONFIG_KU32_ENTRY_BANK_SIZE      16          /**< Default entry bank size */
#define orxCONFIG_KU32_HISTORY_BANK_SIZE    4           /**< Default history bank size */
#define orxCONFIG_KU32_BASE_FILENAME_LENGTH 64          /**< Base file name length */

#define orxCONFIG_KU32_BUFFER_SIZE          4096        /**< Buffer size */

#define orxCONFIG_KC_SECTION_START          '['         /**< Section start character */
#define orxCONFIG_KC_SECTION_END            ']'         /**< Section end character */
#define orxCONFIG_KC_ASSIGN                 '='         /**< Assign character */
#define orxCONFIG_KC_COMMENT                ';'         /**< Comment character */
#define orxCONFIG_KC_RANDOM_SEPARATOR       '~'         /**< Random number separator character */
#define orxCONFIG_KC_INHERITANCE_SEPARATOR  '@'         /**< Inheritance separator character */

#define orxCONFIG_KZ_CONFIG_SECTION         "Config"    /**< Config section name */
#define orxCONFIG_KZ_CONFIG_HISTORY         "History"   /**< History config entry name */

#ifdef __orxDEBUG__

  #define orxCONFIG_KZ_DEFAULT_FILE         "orxd.ini"  /**< Default config file name */

#else /* __orxDEBUG__ */

  #define orxCONFIG_KZ_DEFAULT_FILE         "orx.ini"   /**< Default config file name */

#endif /* __orxDEBUG__ */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Config node structure
 */
typedef struct __orxCONFIG_ENTRY_t
{
  orxSTRING         zKey;                   /**< Entry key: 4 */
  orxSTRING         zValue;                 /**< Entry value : 8 */
  orxU32            u32ID;                  /**< Key ID (CRC) : 12 */

  orxPAD(12)

} orxCONFIG_ENTRY;

/** Config section structure
 */
typedef struct __orxCONFIG_SECTION_t
{
  orxBANK    *pstBank;                      /**< Bank of entries : 4 */
  orxSTRING   zName;                        /**< Section name : 8 */
  orxU32      u32ID;                        /**< Section CRC : 12 */
  orxU32      u32ParentID;                  /**< Parent ID (RCR) : 16 */

  orxPAD(16)

} orxCONFIG_SECTION;

/** Static structure
 */
typedef struct __orxCONFIG_STATIC_t
{
  orxBANK            *pstSectionBank;       /**< Bank of sections */
  orxCONFIG_SECTION  *pstCurrentSection;    /**< Current working section */
  orxBANK            *pstHistoryBank;       /**< History bank */
  orxU32              u32Flags;             /**< Control flags */
  orxCHAR             zBaseFile[orxCONFIG_KU32_BASE_FILENAME_LENGTH]; /**< Base file name */

} orxCONFIG_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** static data
 */
orxSTATIC orxCONFIG_STATIC sstConfig;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Gets an entry from the current section
 * @param[in] _u32KeyID         Entry key ID
 * @return                      orxCONFIG_ENTRY / orxNULL
 */
orxSTATIC orxINLINE orxCONFIG_ENTRY *orxConfig_GetEntry(orxU32 _u32KeyID)
{
  orxCONFIG_ENTRY *pstResult = orxNULL, *pstEntry;

  /* Checks */
  orxASSERT(sstConfig.pstCurrentSection != orxNULL);

  /* For all entries */
  for(pstEntry = orxBank_GetNext(sstConfig.pstCurrentSection->pstBank, orxNULL);
      pstEntry != orxNULL;
      pstEntry = orxBank_GetNext(sstConfig.pstCurrentSection->pstBank, pstEntry))
  {
    /* Found? */
    if(_u32KeyID == pstEntry->u32ID)
    {
      /* Updates result */
      pstResult = pstEntry;

      break;
    }
  }

  /* Done! */
  return pstResult;
}

/** Gets a value from the current section, using inheritance
 * @param[in] _u32KeyID         Entry key ID
 * @return                      orxSTRING / orxNULL
 */
orxSTATIC orxINLINE orxSTRING orxConfig_GetValue(orxU32 _u32KeyID)
{
  orxCONFIG_ENTRY  *pstEntry;
  orxSTRING         zResult = orxNULL;

  /* Checks */
  orxASSERT(sstConfig.pstCurrentSection != orxNULL);

  /* Gets corresponding entry */
  pstEntry = orxConfig_GetEntry(_u32KeyID);

  /* Valid? */
  if(pstEntry != orxNULL)
  {
    /* Has local inheritance? */
    if(*(pstEntry->zValue) == orxCONFIG_KC_INHERITANCE_SEPARATOR)
    {
      orxCONFIG_SECTION *pstPreviousSection;

      /* Backups current section */
      pstPreviousSection = sstConfig.pstCurrentSection;

      /* Selects parent section */
      orxConfig_SelectSection(pstEntry->zValue + 1);

      /* Gets its inherited value */
      zResult = orxConfig_GetValue(_u32KeyID);

      /* Restores current section */
      sstConfig.pstCurrentSection = pstPreviousSection;
    }
    else
    {
      /* Updates result */
      zResult = pstEntry->zValue;
    }
  }
  else
  {
    /* Has parent? */
    if(sstConfig.pstCurrentSection->u32ParentID != 0)
    {
      orxCONFIG_SECTION *pstSection;

      /* For all the sections */
      for(pstSection = orxBank_GetNext(sstConfig.pstSectionBank, orxNULL);
          pstSection != orxNULL;
          pstSection = orxBank_GetNext(sstConfig.pstSectionBank, pstSection))
      {
        /* Found? */
        if(pstSection->u32ID == sstConfig.pstCurrentSection->u32ParentID)
        {
          orxCONFIG_SECTION *pstPreviousSection;

          /* Backups current section */
          pstPreviousSection = sstConfig.pstCurrentSection;

          /* Sets parent as current section */
          sstConfig.pstCurrentSection = pstSection;

          /* Gets inherited value */
          zResult = orxConfig_GetValue(_u32KeyID);

          /* Restores current section */
          sstConfig.pstCurrentSection = pstPreviousSection;

          break;
        }
      }
    }
  }

  /* Done! */
  return zResult;
}

/** Adds an entry in the current section
 * @param[in] _zKey             Entry key
 * @param[in] _zValue           Entry value
 * @return                      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATIC orxINLINE orxSTATUS orxConfig_AddEntry(orxCONST orxSTRING _zKey, orxCONST orxSTRING _zValue)
{
  orxCONFIG_ENTRY  *pstEntry;
  orxSTATUS         eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstConfig.pstCurrentSection != orxNULL);
  orxASSERT(_zKey != orxNULL);
  orxASSERT(*_zKey != *orxSTRING_EMPTY);
  orxASSERT(_zValue != orxNULL);

  /* Creates entry */
  pstEntry = orxBank_Allocate(sstConfig.pstCurrentSection->pstBank);

  /* Valid? */
  if(pstEntry != orxNULL)
  {
    /* Stores value */
    pstEntry->zValue = orxString_Duplicate(_zValue);

    /* Valid? */
    if(pstEntry->zValue != orxNULL)
    {
      /* Stores key */
      pstEntry->zKey = orxString_Duplicate(_zKey);

      /* Valid? */
      if(pstEntry->zKey != orxNULL)
      {
        /* Sets its ID */
        pstEntry->u32ID = orxString_ToCRC(pstEntry->zKey);

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
      else
      {
        /* !!! MSG !!! */

        /* Deletes allocated string */
        orxString_Delete(pstEntry->zValue);

        /* Deletes entry */
        orxBank_Free(sstConfig.pstCurrentSection->pstBank, pstEntry);
      }
    }
    else
    {
      /* !!! MSG !!! */

      /* Deletes entry */
      orxBank_Free(sstConfig.pstCurrentSection->pstBank, pstEntry);
    }
  }

  /* Done! */
  return eResult;
}

/** Deletes an entry
 * @param[in] _pstSection       Concerned section
 * @param[in] _pstEntry         Entry to delete
 */
orxSTATIC orxINLINE orxVOID orxConfig_DeleteEntry(orxCONFIG_SECTION *_pstSection, orxCONFIG_ENTRY *_pstEntry)
{
  /* Checks */
  orxASSERT(_pstEntry != orxNULL);

  /* Deletes key & value */
  orxString_Delete(_pstEntry->zKey);
  orxString_Delete(_pstEntry->zValue);

  /* Deletes the entry */
  orxBank_Free(_pstSection->pstBank, _pstEntry);

  return;
}

/** Creates a section
 * @param[in] _zSectionName     Name of the section to create
 * @param[in] _u32SectionID     ID of the section to create
 * @param[in] _u32ParentID      ID of the parent of the section to create
 */
orxSTATIC orxINLINE orxCONFIG_SECTION *orxConfig_CreateSection(orxCONST orxSTRING _zSectionName, orxU32 _u32SectionID, orxU32 _u32ParentID)
{
  orxCONFIG_SECTION *pstSection;

  /* Checks */
  orxASSERT(_zSectionName != orxNULL);
  orxASSERT(*_zSectionName != *orxSTRING_EMPTY);

  /* Allocates it */
  pstSection = orxBank_Allocate(sstConfig.pstSectionBank);

  /* Valid? */
  if(pstSection != orxNULL)
  {
    /* Creates its bank */
    pstSection->pstBank = orxBank_Create(orxCONFIG_KU32_ENTRY_BANK_SIZE, sizeof(orxCONFIG_ENTRY), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_CONFIG);

    /* Valid? */
    if(pstSection->pstBank != orxNULL)
    {
      /* Duplicates its name */
      pstSection->zName = orxString_Duplicate(_zSectionName);

      /* Valid? */
      if(pstSection->zName != orxNULL)
      {
        /* Sets its ID */
        pstSection->u32ID = _u32SectionID;

        /* Cleans its parent ID */
        pstSection->u32ParentID = _u32ParentID;
      }
      else
      {
        /* !!! MSG !!! */

        /* Deletes its bank */
        orxBank_Delete(pstSection->pstBank);

        /* Deletes it */
        orxBank_Free(sstConfig.pstSectionBank, pstSection);

        /* Updates result */
        pstSection = orxNULL;
      }
    }
    else
    {
      /* !!! MSG !!! */

      /* Deletes the section */
      orxBank_Free(sstConfig.pstSectionBank, pstSection);

      /* Updates result */
      pstSection = orxNULL;
    }
  }

  /* Done! */
  return pstSection;
}

/** Deletes a section
 * @param[in] _pstSection       Section to delete
 */
orxSTATIC orxINLINE orxVOID orxConfig_DeleteSection(orxCONFIG_SECTION *_pstSection)
{
  orxCONFIG_ENTRY *pstEntry;

  /* Checks */
  orxASSERT(_pstSection != orxNULL);

  /* While there is still an entry */
  while((pstEntry = (orxCONFIG_ENTRY *)orxBank_GetNext(_pstSection->pstBank, orxNULL)) != orxNULL)
  {
    /* Deletes entry */
    orxConfig_DeleteEntry(_pstSection, pstEntry);
  }

  /* Removes section */
  orxBank_Free(sstConfig.pstSectionBank, _pstSection);

  return;
}

/** Clears all config data
 */
orxSTATIC orxINLINE orxVOID orxConfig_Clear()
{
  orxCONFIG_SECTION *pstSection;

  /* While there's still a section */
  while((pstSection = orxBank_GetNext(sstConfig.pstSectionBank, orxNULL)) != orxNULL)
  {
    /* Deletes it */
    orxConfig_DeleteSection(pstSection);
  }

  return;
}

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Config module setup
 */
orxVOID orxConfig_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_CONFIG, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_CONFIG, orxMODULE_ID_BANK);

  return;
}

/** Inits the config module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxConfig_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY))
  {
    orxCHAR zBackupBaseFile[orxCONFIG_KU32_BASE_FILENAME_LENGTH];

    /* Backups base file name */
    orxMemory_Copy(zBackupBaseFile, sstConfig.zBaseFile, orxCONFIG_KU32_BASE_FILENAME_LENGTH);

    /* Cleans control structure */
    orxMemory_Zero(&sstConfig, sizeof(orxCONFIG_STATIC));

    /* Valid base file name? */
    if(*zBackupBaseFile != orxCHAR_NULL)
    {
      /* Restores base file name */
      orxMemory_Copy(sstConfig.zBaseFile, zBackupBaseFile, orxCONFIG_KU32_BASE_FILENAME_LENGTH);
    }
    else
    {
      /* Stores default base file name */
      orxString_Copy(sstConfig.zBaseFile, orxCONFIG_KZ_DEFAULT_FILE);
    }

    /* Creates section bank */
    sstConfig.pstSectionBank = orxBank_Create(orxCONFIG_KU32_SECTION_BANK_SIZE, sizeof(orxCONFIG_SECTION), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_CONFIG);

    /* Valid? */
    if(sstConfig.pstSectionBank != orxNULL)
    {
      /* Inits Flags */
      orxFLAG_SET(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY, orxCONFIG_KU32_STATIC_MASK_ALL);

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;

      /* Loads default config file */
      orxConfig_Load(sstConfig.zBaseFile);

      /* Selects config section */
      orxConfig_SelectSection(orxCONFIG_KZ_CONFIG_SECTION);

      /* Should keep history? */
      if(orxConfig_GetBool(orxCONFIG_KZ_CONFIG_HISTORY) != orxFALSE)
      {
        /* Creates history bank */
        sstConfig.pstHistoryBank = orxBank_Create(orxCONFIG_KU32_HISTORY_BANK_SIZE, sizeof(orxSTRING), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_CONFIG);

        /* Valid? */
        if(sstConfig.pstHistoryBank != orxNULL)
        {
          /* Updates flags */
          orxFLAG_SET(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_HISTORY, orxCONFIG_KU32_STATIC_FLAG_NONE);
        }
        else
        {
          /* !!! MSG !!! */

          /* Updates result */
          eResult = orxSTATUS_FAILURE;

          /* Clears Flags */
          orxFLAG_SET(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_NONE, orxCONFIG_KU32_STATIC_MASK_ALL);

          /* Deletes created bank */
          orxBank_Delete(sstConfig.pstSectionBank);
        }
      }
    }
  }
  else
  {
    /* !!! MSG !!! */

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Exits from the config module
 */
orxVOID orxConfig_Exit()
{
  /* Initialized? */
  if(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY))
  {
    /* Clears all data */
    orxConfig_Clear();

    /* Deletes section bank */
    orxBank_Delete(sstConfig.pstSectionBank);
    sstConfig.pstSectionBank = orxNULL;

    /* Has history bank? */
    if(sstConfig.pstHistoryBank != orxNULL)
    {
      /* Deletes it */
      orxBank_Delete(sstConfig.pstHistoryBank);
      sstConfig.pstHistoryBank = orxNULL;
    }

    /* Updates flags */
    orxFLAG_SET(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_NONE, orxCONFIG_KU32_STATIC_MASK_ALL);
  }

  return;
}

/** Sets config base name
 * @param[in] _zBaseName        Base name used for default config file
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_SetBaseName(orxCONST orxSTRING _zBaseName)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(orxString_GetLength(_zBaseName) < orxCONFIG_KU32_BASE_FILENAME_LENGTH - 1);

  /* Valid? */
  if((_zBaseName != orxNULL) && (*_zBaseName != *orxSTRING_EMPTY))
  {
    /* Copies it */
    orxString_Print(sstConfig.zBaseFile, "%s.ini", _zBaseName);
  }
  else
  {
    /* Uses default name */
    orxString_Copy(sstConfig.zBaseFile, orxCONFIG_KZ_DEFAULT_FILE);
  }

  /* Done! */
  return eResult;
}

/** Selects current working section
 * @param[in] _zSectionName     Section name to select
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxConfig_SelectSection(orxCONST orxSTRING _zSectionName)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zSectionName != orxNULL);

  /* Valid? */
  if(*_zSectionName != *orxSTRING_EMPTY)
  {
    orxCONFIG_SECTION  *pstSection;
    orxU32              u32SectionID;
    orxS32              s32MarkerIndex;

    /* Looks for inheritance index */
    s32MarkerIndex = orxString_SearchCharIndex(_zSectionName, orxCONFIG_KC_INHERITANCE_SEPARATOR, 0);

    /* Found? */
    if(s32MarkerIndex >= 0)
    {
      /* Cut the name */
      *(_zSectionName + s32MarkerIndex) = orxCHAR_NULL;
    }

    /* Gets the section ID */
    u32SectionID = orxString_ToCRC(_zSectionName);

    /* For all the sections */
    for(pstSection = orxBank_GetNext(sstConfig.pstSectionBank, orxNULL);
        pstSection != orxNULL;
        pstSection = orxBank_GetNext(sstConfig.pstSectionBank, pstSection))
    {
      /* Found? */
      if(pstSection->u32ID == u32SectionID)
      {
        /* Selects it */
        sstConfig.pstCurrentSection = pstSection;

        break;
      }
    }

    /* Not found? */
    if(pstSection == orxNULL)
    {
      orxU32 u32ParentID;

      /* Has inheritance? */
      if(s32MarkerIndex >= 0)
      {
        /* Gets its parent ID */
        u32ParentID = orxString_ToCRC(_zSectionName + s32MarkerIndex + 1);
      }
      else
      {
        /* Clears parent ID */
        u32ParentID = 0;
      }

      /* Creates it */
      pstSection = orxConfig_CreateSection(_zSectionName, u32SectionID, u32ParentID);

      /* Success? */
      if(pstSection != orxNULL)
      {
        /* Selects it */
        sstConfig.pstCurrentSection = pstSection;
      }
      else
      {
        /* !!! MSG !!! */

        /* Updates result */
        eResult = orxSTATUS_FAILURE;
      }
    }

    /* Had inheritance marker? */
    if(s32MarkerIndex >= 0)
    {
      /* Restores it */
      *(_zSectionName + s32MarkerIndex) = orxCONFIG_KC_INHERITANCE_SEPARATOR;
    }
  }
  else
  {
    /* !!! MSG !!! */

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets current working section
 * @return Current selected section
 */
orxSTRING orxConfig_GetCurrentSection()
{
  orxSTRING zResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));

  /* Has selected section? */
  if(sstConfig.pstCurrentSection != orxNULL)
  {
    /* Updates result */
    zResult = sstConfig.pstCurrentSection->zName;
  }
  else
  {
    /* Updates result */
    zResult = orxSTRING_EMPTY;
  }

  /* Done! */
  return zResult;
}

/** Read config config from source.
 * @param[in] _zFileName        File name
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_Load(orxCONST orxSTRING _zFileName)
{
  FILE     *pstFile;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zFileName != orxNULL);

  /* Valid file to open? */
  if((*_zFileName != *orxSTRING_EMPTY) && ((pstFile = fopen(_zFileName, "r")) != orxNULL))
  {
    orxCHAR   acBuffer[orxCONFIG_KU32_BUFFER_SIZE];
    orxU32    u32Size, u32Offset;
    orxSTRING zPreviousSection;

    /* Gets previous config section */
    zPreviousSection = orxConfig_GetCurrentSection();

    /* While file isn't empty */
    for(u32Size = (orxU32)fread(acBuffer, sizeof(orxCHAR), orxCONFIG_KU32_BUFFER_SIZE, pstFile), u32Offset = 0;
        u32Size > 0;
        u32Size = (orxU32)fread(acBuffer + u32Offset, sizeof(orxCHAR), orxCONFIG_KU32_BUFFER_SIZE - u32Offset, pstFile) + u32Offset)
    {
      orxCHAR *pc, *pcKeyEnd, *pcValueStart, *pcLineStart;

      /* For all buffered characters */
      for(pc = pcLineStart = acBuffer, pcKeyEnd = pcValueStart = orxNULL;
          pc < acBuffer + u32Size;
          pc++)
      {
        /* Comment character? */
        if(*pc == orxCONFIG_KC_COMMENT)
        {
          /* Has key & value? */
          if((pcKeyEnd != orxNULL) && (pcValueStart != orxNULL))
          {
            orxU32            u32KeyID;
            orxSTRING         pcValueEnd;
            orxCONFIG_ENTRY  *pstEntry;

            /* Finds end of value position */
            for(pcValueEnd = pc - 1;
                (pcValueEnd > pcValueStart) && ((*pcValueEnd == ' ') || (*pcValueEnd == '\t') || (*pcValueEnd == orxCHAR_CR) || (*pcValueEnd == orxCHAR_LF));
                pcValueEnd--);

            /* Cuts the strings */
            *pcKeyEnd = *(++pcValueEnd) = orxCHAR_NULL;

            /* Gets key ID */
            u32KeyID = orxString_ToCRC(pcLineStart);

            /* Already defined? */
            if((pstEntry = orxConfig_GetEntry(u32KeyID)) != orxNULL)
            {
              /* Logs */
              orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Config entry [%s::%s]: Replacing value \"%s\" with new value \"%s\" from <%s>.", sstConfig.pstCurrentSection->zName, pstEntry->zKey, pstEntry->zValue, pcValueStart, _zFileName);

              /* Deletes entry */
              orxConfig_DeleteEntry(sstConfig.pstCurrentSection, pstEntry);
            }

            /* Adds entry */
            orxConfig_AddEntry(pcLineStart, pcValueStart);

            /* Updates pointers */
            pcKeyEnd = pcValueStart = orxNULL;
          }

          /* Sets temporary line start */
          pcLineStart = pc;

          /* Skips the whole line */
          while((pc < acBuffer + u32Size) && (*pc != orxCHAR_EOL))
          {
            pc++;
          }

          /* Updates line start pointer */
          pcLineStart = pc + 1;
        }
        /* Beginning of line? */
        else if(pc == pcLineStart)
        {
          /* Skips all spaces */
          while((pc < acBuffer + u32Size) && ((*pc == orxCHAR_CR) || (*pc == orxCHAR_LF) || (*pc == ' ') || (*pc == '\t')))
          {
            /* Updates pointers */
            pcLineStart++, pc++;
          }

          /* Section start? */
          if(*pc == orxCONFIG_KC_SECTION_START)
          {
            /* Finds section end */
            while((pc < acBuffer + u32Size) && (*pc != orxCONFIG_KC_SECTION_END))
            {
              /* End of line? */
              if(*pc == orxCHAR_EOL)
              {
                /* !!! MSG !!! */
                orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Section name <%*s> incomplete, closing character '%c' not found.", pc - (pcLineStart + 1), pcLineStart + 1, orxCONFIG_KC_SECTION_END);

                /* Updates new line start */
                pcLineStart = pc + 1;

                break;
              }

              /* Updates pointer */
              pc++;
            }

            /* Valid? */
            if((pc < acBuffer + u32Size) && (*pc == orxCONFIG_KC_SECTION_END))
            {
              /* Cuts string */
              *pc = orxCHAR_NULL;

              /* Selects section */
              orxConfig_SelectSection(pcLineStart + 1);

              /* Skips the whole line */
              while((pc < acBuffer + u32Size) && (*pc != orxCHAR_EOL))
              {
                pc++;
              }

              /* Updates line start pointer */
              pcLineStart = pc + 1;
            }
          }
          /* Comment character? */
          else if(*pc == orxCONFIG_KC_COMMENT)
          {
            /* Skips the whole line */
            while((pc < acBuffer + u32Size) && (*pc != orxCHAR_EOL))
            {
              pc++;
            }

            /* Updates line start pointer */
            pcLineStart = pc + 1;
          }
          else
          {
            /* Finds assign character */
            while((pc < acBuffer + u32Size) && (*pc != orxCONFIG_KC_ASSIGN))
            {
              /* Updates pointer */
              pc++;
            }

            /* Found? */
            if((pc < acBuffer + u32Size) && (*pc == orxCONFIG_KC_ASSIGN))
            {
              /* Finds end of key position */
              for(pcKeyEnd = pc - 1;
                  (pcKeyEnd > pcLineStart) && ((*pcKeyEnd == ' ') || (*pcKeyEnd == '\t') || (*pcKeyEnd == orxCHAR_CR) || (*pcKeyEnd == orxCHAR_LF));
                  pcKeyEnd--);

              /* Updates key end pointer */
              pcKeyEnd += 1;

              /* Checks */
              orxASSERT(pcKeyEnd > pcLineStart);

              /* Finds start of value position */
              for(pcValueStart = pc + 1;
                  (pcValueStart < acBuffer + u32Size) && ((*pcValueStart == ' ') || (*pcValueStart == '\t') || (*pcValueStart == orxCHAR_CR) || (*pcValueStart == orxCHAR_LF));
                  pcValueStart++);
            }
          }
        }
      }

      /* Has remaining buffer? */
      if((pcLineStart != acBuffer) && (pc > pcLineStart))
      {
        /* Updates offset */
        u32Offset = (orxU32)(orxMIN(pc, acBuffer + u32Size) - pcLineStart);

        /* Copies it at the beginning of the buffer */
        orxMemory_Copy(acBuffer, pcLineStart, u32Offset);
      }
      else
      {
        /* Clears offset */
        u32Offset = 0;
      }
    }

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;

    /* Should keep history? */
    if(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_HISTORY))
    {
      orxSTRING *pzEntry;

      /* Add an history entry */
      pzEntry = (orxSTRING *)orxBank_Allocate(sstConfig.pstHistoryBank);

      /* Valid? */
      if(pzEntry != orxNULL)
      {
        /* Stores the file name */
        *pzEntry = orxString_Duplicate(_zFileName);
      }
    }

    /* Restores previous section */
    orxConfig_SelectSection(zPreviousSection);
  }

  /* Done! */
  return eResult;
}

/** Reloads config files from history
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_ReloadHistory()
{
  orxSTRING  *pzHistoryEntry;
  orxSTATUS   eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));

  /* Has history? */
  if(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_HISTORY))
  {
    /* Removes history flag */
    orxFLAG_SET(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_NONE, orxCONFIG_KU32_STATIC_FLAG_HISTORY);

    /* Clears all data */
    orxConfig_Clear();

    /* Reloads default file */
    eResult = orxConfig_Load(sstConfig.zBaseFile);

    /* Logs */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Config file [%s] has been reloaded.", sstConfig.zBaseFile);

    /* For all entries in history */
    for(pzHistoryEntry = orxBank_GetNext(sstConfig.pstHistoryBank, orxNULL);
        (pzHistoryEntry != orxNULL) && (eResult != orxSTATUS_FAILURE);
        pzHistoryEntry = orxBank_GetNext(sstConfig.pstHistoryBank, pzHistoryEntry))
    {
      /* Reloads it */
      eResult = orxConfig_Load(*pzHistoryEntry);

      /* Logs */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Config file [%s] has been reloaded.", *pzHistoryEntry);
    }

    /* Restores history flag */
    orxFLAG_SET(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_HISTORY, orxCONFIG_KU32_STATIC_FLAG_NONE);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Config history isn't stored. Please check your config file under the [Config] section.");

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Writes config to given file. Will overwrite any existing file, including all comments.
 * @param[in] _zFileName        File name, if null or empty the default file name will be used
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxConfig_Save(orxCONST orxSTRING _zFileName)
{
  FILE     *pstFile;
  orxSTRING zFileName;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));

  /* Is given file name valid? */
  if((_zFileName != orxNULL) && (*_zFileName != *orxSTRING_EMPTY))
  {
    /* Uses it */
    zFileName = _zFileName;
  }
  else
  {
    /* Uses default file */
    zFileName = sstConfig.zBaseFile;
  }

  /* Opens file */
  pstFile = fopen(zFileName, "w+");

  /* Valid? */
  if(pstFile != orxNULL)
  {
    orxCONFIG_SECTION *pstSection;

    /* For all sections */
    for(pstSection = orxBank_GetNext(sstConfig.pstSectionBank, orxNULL);
        pstSection != orxNULL;
        pstSection = orxBank_GetNext(sstConfig.pstSectionBank, pstSection))
    {
      orxCONFIG_ENTRY *pstEntry;

      /* Writes section name */
      fprintf(pstFile, "%c%s%c\n", orxCONFIG_KC_SECTION_START, pstSection->zName, orxCONFIG_KC_SECTION_END);

      /* For all entries */
      for(pstEntry = orxBank_GetNext(pstSection->pstBank, orxNULL);
          pstEntry != orxNULL;
          pstEntry = orxBank_GetNext(pstSection->pstBank, pstEntry))
      {
        /* Writes it */
        fprintf(pstFile, "%s%c%s%c\n", pstEntry->zKey, orxCONFIG_KC_ASSIGN, pstEntry->zValue, orxCONFIG_KC_COMMENT);
      }

      /* Adds a new line */
      fprintf(pstFile, "\n");
    }

    /* Flushes & closes the file */
    if((fflush(pstFile) == 0) && (fclose(pstFile) == 0))
    {
      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
  }

  /* Done! */
  return eResult;
}

/** Has specified value for the given key?
 * @param[in] _zKey             Key name
 * @return orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxConfig_HasValue(orxCONST orxSTRING _zKey)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(*_zKey != *orxSTRING_EMPTY);

  /* Updates result */
  bResult = (orxConfig_GetValue(orxString_ToCRC(_zKey)) != orxNULL) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

/** Has section for the given section name?
 * @param[in] _zSectionName     Section name
 * @return orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxConfig_HasSection(orxCONST orxSTRING _zSectionName)
{
  orxCONFIG_SECTION  *pstSection;
  orxU32              u32ID;
  orxBOOL             bResult = orxFALSE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zSectionName != orxNULL);
  orxASSERT(*_zSectionName != *orxSTRING_EMPTY);

  /* Gets section name ID */
  u32ID = orxString_ToCRC(_zSectionName);

  /* For all the sections */
  for(pstSection = orxBank_GetNext(sstConfig.pstSectionBank, orxNULL);
      pstSection != orxNULL;
      pstSection = orxBank_GetNext(sstConfig.pstSectionBank, pstSection))
  {
    /* Found? */
    if(pstSection->u32ID == u32ID)
    {
      /* Updates result */
      bResult = orxTRUE;

      break;
    }
  }

  /* Done! */
  return bResult;
}

/** Reads a signed integer value from config
 * @param[in] _zKey             Key name
 * @param[in] _s32DefaultValue  Default value if key is not found
 * @return The value
 */
orxS32 orxFASTCALL orxConfig_GetS32(orxCONST orxSTRING _zKey)
{
  orxSTRING zValue;
  orxS32    s32Result = 0;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(*_zKey != *orxSTRING_EMPTY);

  /* Gets corresponding value */
  zValue = orxConfig_GetValue(orxString_ToCRC(_zKey));

  /* Found? */
  if(zValue != orxNULL)
  {
    orxS32    s32Value;
    orxSTRING zRemainder;

    /* Gets value */
    if(orxString_ToS32(zValue, &s32Value, &zRemainder) != orxSTATUS_FAILURE)
    {
      orxS32 s32RandomSeparatorIndex, s32OtherValue;

      /* Searches for the random separator */
      s32RandomSeparatorIndex = orxString_SearchCharIndex(zRemainder, orxCONFIG_KC_RANDOM_SEPARATOR, 0);

      /* Found and has another value? */
      if((s32RandomSeparatorIndex >= 0)
      && (orxString_ToS32(zRemainder + s32RandomSeparatorIndex + 1, &s32OtherValue, orxNULL) != orxSTATUS_FAILURE))
      {
        /* Updates result */
        s32Result = orxS32RAND(s32Value, s32OtherValue);
      }
      else
      {
        /* Updates result */
        s32Result = s32Value;
      }
    }
  }

  /* Done! */
  return s32Result;
}

/** Reads an unsigned integer value from config
 * @param[in] _zKey             Key name
 * @param[in] _s32DefaultValue  Default value if key is not found
 * @return The value
 */
orxU32 orxFASTCALL orxConfig_GetU32(orxCONST orxSTRING _zKey)
{
  orxSTRING zValue;
  orxU32    u32Result = 0;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(*_zKey != *orxSTRING_EMPTY);

  /* Gets corresponding value */
  zValue = orxConfig_GetValue(orxString_ToCRC(_zKey));

  /* Found? */
  if(zValue != orxNULL)
  {
    orxU32    u32Value;
    orxSTRING zRemainder;

    /* Gets value */
    if(orxString_ToU32(zValue, &u32Value, &zRemainder) != orxSTATUS_FAILURE)
    {
      orxS32 s32RandomSeparatorIndex;
      orxU32 u32OtherValue;

      /* Searches for the random separator */
      s32RandomSeparatorIndex = orxString_SearchCharIndex(zRemainder, orxCONFIG_KC_RANDOM_SEPARATOR, 0);

      /* Found and has another value? */
      if((s32RandomSeparatorIndex >= 0)
      && (orxString_ToU32(zRemainder + s32RandomSeparatorIndex + 1, &u32OtherValue, orxNULL) != orxSTATUS_FAILURE))
      {
        /* Updates result */
        u32Result = orxU32RAND(u32Value, u32OtherValue);
      }
      else
      {
        /* Updates result */
        u32Result = u32Value;
      }
    }
  }

  /* Done! */
  return u32Result;
}

/** Reads a float value from config
 * @param[in] _zKey             Key name
 * @param[in] _fDefaultValue    Default value if key is not found
 * @return The value
 */
orxFLOAT orxFASTCALL orxConfig_GetFloat(orxCONST orxSTRING _zKey)
{
  orxSTRING zValue;
  orxFLOAT  fResult = orxFLOAT_0;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(*_zKey != *orxSTRING_EMPTY);

  /* Gets corresponding value */
  zValue = orxConfig_GetValue(orxString_ToCRC(_zKey));

  /* Found? */
  if(zValue != orxNULL)
  {
    orxFLOAT  fValue;
    orxSTRING zRemainder;

    /* Gets value */
    if(orxString_ToFloat(zValue, &fValue, &zRemainder) != orxSTATUS_FAILURE)
    {
      orxS32    s32RandomSeparatorIndex;
      orxFLOAT  fOtherValue;

      /* Searches for the random separator */
      s32RandomSeparatorIndex = orxString_SearchCharIndex(zRemainder, orxCONFIG_KC_RANDOM_SEPARATOR, 0);

      /* Found and has another value? */
      if((s32RandomSeparatorIndex >= 0)
      && (orxString_ToFloat(zRemainder + s32RandomSeparatorIndex + 1, &fOtherValue, orxNULL) != orxSTATUS_FAILURE))
      {
        /* Updates result */
        fResult = orxFRAND(fValue, fOtherValue);
      }
      else
      {
        /* Updates result */
        fResult = fValue;
      }
    }
  }

  /* Done! */
  return fResult;
}

/** Reads a string value from config
 * @param[in] _zKey             Key name
 * @param[in] _zDefaultValue    Default value if key is not found
 * @return The value
 */
orxSTRING orxFASTCALL orxConfig_GetString(orxCONST orxSTRING _zKey)
{
  orxSTRING zValue, zResult = orxSTRING_EMPTY;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(*_zKey != *orxSTRING_EMPTY);

  /* Gets corresponding value */
  zValue = orxConfig_GetValue(orxString_ToCRC(_zKey));

  /* Found? */
  if(zValue != orxNULL)
  {
    /* Updates result */
    zResult = zValue;
  }

  /* Done! */
  return zResult;
}

/** Reads a boolean value from config
 * @param[in] _zKey             Key name
 * @param[in] _bDefaultValue    Default value if key is not found
 * @return The value
 */
orxBOOL orxFASTCALL orxConfig_GetBool(orxCONST orxSTRING _zKey)
{
  orxSTRING zValue;
  orxBOOL   bResult = orxFALSE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(*_zKey != *orxSTRING_EMPTY);

  /* Gets corresponding value */
  zValue = orxConfig_GetValue(orxString_ToCRC(_zKey));

  /* Found? */
  if(zValue != orxNULL)
  {
    orxBOOL bValue;

    /* Gets value */
    if(orxString_ToBool(zValue, &bValue, orxNULL) != orxSTATUS_FAILURE)
    {
      /* Updates result */
      bResult = bValue;
    }
  }

  /* Done! */
  return bResult;
}

/** Reads a vector value from config
 * @param[in]   _zKey             Key name
 * @param[out]  _pvVector         Storage for vector value
 * @return The value
 */
orxVECTOR *orxFASTCALL orxConfig_GetVector(orxCONST orxSTRING _zKey, orxVECTOR *_pvVector)
{
  orxSTRING   zValue;
  orxVECTOR  *pvResult = orxNULL;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(*_zKey != *orxSTRING_EMPTY);
  orxASSERT(_pvVector != orxNULL);

  /* Gets corresponding value */
  zValue = orxConfig_GetValue(orxString_ToCRC(_zKey));

  /* Found? */
  if(zValue != orxNULL)
  {
    orxSTRING zRemainder;

    /* Gets value */
    if(orxString_ToVector(zValue, _pvVector, &zRemainder) != orxSTATUS_FAILURE)
    {
      orxS32 s32RandomSeparatorIndex;
      orxVECTOR vOtherValue;

      /* Searches for the random separator */
      s32RandomSeparatorIndex = orxString_SearchCharIndex(zRemainder, orxCONFIG_KC_RANDOM_SEPARATOR, 0);

      /* Found and has another value? */
      if((s32RandomSeparatorIndex >= 0)
      && (orxString_ToVector(zRemainder + s32RandomSeparatorIndex + 1, &vOtherValue, orxNULL) != orxSTATUS_FAILURE))
      {
        /* Updates result */
        _pvVector->fX = orxFRAND(_pvVector->fX, vOtherValue.fX);
        _pvVector->fY = orxFRAND(_pvVector->fY, vOtherValue.fY);
        _pvVector->fZ = orxFRAND(_pvVector->fZ, vOtherValue.fZ);
      }

      /* Updates result */
      pvResult = _pvVector;
    }
  }

  /* Done! */
  return pvResult;
}

/** Writes an integer value to config
 * @param[in] _zKey             Key name
 * @param[in] _s32Value         Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_SetS32(orxCONST orxSTRING _zKey, orxS32 _s32Value)
{
  orxCONFIG_ENTRY  *pstEntry;
  orxCHAR           zValue[16];
  orxSTATUS         eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(*_zKey != *orxSTRING_EMPTY);

  /* Clears buffer */
  orxMemory_Zero(zValue, 16 * sizeof(orxCHAR));

  /* Gets literal value */
  orxString_Print(zValue, "%d", _s32Value);

  /* Gets entry */
  pstEntry = orxConfig_GetEntry(orxString_ToCRC(_zKey));

  /* Found? */
  if(pstEntry != orxNULL)
  {
    /* Deletes it */
    orxConfig_DeleteEntry(sstConfig.pstCurrentSection, pstEntry);
  }

  /* Adds new entry */
  eResult = orxConfig_AddEntry(_zKey, zValue);

  /* Done! */
  return eResult;
}

/** Writes an unsigned integer value to config
 * @param[in] _zKey             Key name
 * @param[in] _u32Value         Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_SetU32(orxCONST orxSTRING _zKey, orxU32 _u32Value)
{
  orxCONFIG_ENTRY  *pstEntry;
  orxCHAR           zValue[16];
  orxSTATUS         eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(*_zKey != *orxSTRING_EMPTY);

  /* Clears buffer */
  orxMemory_Zero(zValue, 16 * sizeof(orxCHAR));

  /* Gets literal value */
  orxString_Print(zValue, "%u", _u32Value);

  /* Gets entry */
  pstEntry = orxConfig_GetEntry(orxString_ToCRC(_zKey));

  /* Found? */
  if(pstEntry != orxNULL)
  {
    /* Deletes it */
    orxConfig_DeleteEntry(sstConfig.pstCurrentSection, pstEntry);
  }

  /* Adds new entry */
  eResult = orxConfig_AddEntry(_zKey, zValue);

  /* Done! */
  return eResult;
}

/** Writes a float value to config
 * @param[in] _zKey             Key name
 * @param[in] _fValue           Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_SetFloat(orxCONST orxSTRING _zKey, orxFLOAT _fValue)
{
  orxCONFIG_ENTRY  *pstEntry;
  orxCHAR           zValue[16];
  orxSTATUS         eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(*_zKey != *orxSTRING_EMPTY);

  /* Clears buffer */
  orxMemory_Zero(zValue, 16 * sizeof(orxCHAR));

  /* Gets literal value */
  orxString_Print(zValue, "%g", _fValue);

  /* Gets entry */
  pstEntry = orxConfig_GetEntry(orxString_ToCRC(_zKey));

  /* Found? */
  if(pstEntry != orxNULL)
  {
    /* Deletes it */
    orxConfig_DeleteEntry(sstConfig.pstCurrentSection, pstEntry);
  }

  /* Adds new entry */
  eResult = orxConfig_AddEntry(_zKey, zValue);

  /* Done! */
  return eResult;
}

/** Writes a string value to config
 * @param[in] _zKey             Key name
 * @param[in] _fValue           Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_SetString(orxCONST orxSTRING _zKey, orxCONST orxSTRING _zValue)
{
  orxCONFIG_ENTRY  *pstEntry;
  orxSTATUS         eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(*_zKey != *orxSTRING_EMPTY);
  orxASSERT(_zValue != orxNULL);

  /* Gets entry */
  pstEntry = orxConfig_GetEntry(orxString_ToCRC(_zKey));

  /* Found? */
  if(pstEntry != orxNULL)
  {
    /* Deletes it */
    orxConfig_DeleteEntry(sstConfig.pstCurrentSection, pstEntry);
  }

  /* Adds new entry */
  eResult = orxConfig_AddEntry(_zKey, _zValue);

  /* Done! */
  return eResult;
}

/** Writes a boolean value to config
 * @param[in] _zKey             Key name
 * @param[in] _fValue           Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_SetBool(orxCONST orxSTRING _zKey, orxBOOL _bValue)
{
  orxCONFIG_ENTRY  *pstEntry;
  orxSTATUS         eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(*_zKey != *orxSTRING_EMPTY);

  /* Gets entry */
  pstEntry = orxConfig_GetEntry(orxString_ToCRC(_zKey));

  /* Found? */
  if(pstEntry != orxNULL)
  {
    /* Deletes it */
    orxConfig_DeleteEntry(sstConfig.pstCurrentSection, pstEntry);
  }

  /* Adds new entry */
  eResult = orxConfig_AddEntry(_zKey, (_bValue == orxFALSE) ? orxSTRING_FALSE : orxSTRING_TRUE);

  /* Done! */
  return eResult;
}

/** Writes a vector value to config
 * @param[in] _zKey             Key name
 * @param[in] _pvValue         Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_SetVector(orxCONST orxSTRING _zKey, orxCONST orxVECTOR *_pvValue)
{
  orxCONFIG_ENTRY  *pstEntry;
  orxCHAR           zValue[64];
  orxSTATUS         eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(*_zKey != *orxSTRING_EMPTY);
  orxASSERT(_pvValue != orxNULL);

  /* Clears buffer */
  orxMemory_Zero(zValue, 64 * sizeof(orxCHAR));

  /* Gets literal value */
  orxString_Print(zValue, "%c%g%c %g%c %g%c", orxSTRING_KC_VECTOR_START, _pvValue->fX, orxSTRING_KC_VECTOR_SEPARATOR, _pvValue->fY, orxSTRING_KC_VECTOR_SEPARATOR, _pvValue->fZ, orxSTRING_KC_VECTOR_END);

  /* Gets entry */
  pstEntry = orxConfig_GetEntry(orxString_ToCRC(_zKey));

  /* Found? */
  if(pstEntry != orxNULL)
  {
    /* Deletes it */
    orxConfig_DeleteEntry(sstConfig.pstCurrentSection, pstEntry);
  }

  /* Adds new entry */
  eResult = orxConfig_AddEntry(_zKey, zValue);

  /* Done! */
  return eResult;
}
