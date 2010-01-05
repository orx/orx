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
 */


#include "orxInclude.h"

#include "core/orxConfig.h"
#include "debug/orxDebug.h"
#include "memory/orxBank.h"
#include "math/orxMath.h"
#include "io/orxFile.h"
#include "utils/orxLinkList.h"
#include "utils/orxHashTable.h"
#include "utils/orxString.h"

#ifdef __orxMAC__

  #include <unistd.h>

#endif /* __orxMAC__ */


/** Module flags
 */
#define orxCONFIG_KU32_STATIC_FLAG_NONE           0x00000000  /**< No flags */

#define orxCONFIG_KU32_STATIC_FLAG_READY          0x00000001  /**< Ready flag */
#define orxCONFIG_KU32_STATIC_FLAG_HISTORY        0x00000002  /**< Keep history flag */

#define orxCONFIG_KU32_STATIC_MASK_ALL            0xFFFFFFFF  /**< All mask */


/** Config value flags */
#define orxCONFIG_VALUE_KU16_FLAG_NONE            0x0000      /**< No flags */

#define orxCONFIG_VALUE_KU16_FLAG_LIST            0x0001      /**< List flag */
#define orxCONFIG_VALUE_KU16_FLAG_RANDOM          0x0002      /**< Random flag */
#define orxCONFIG_VALUE_KU16_FLAG_INHERITANCE     0x0004      /**< Inheritance flag */
#define orxCONFIG_VALUE_KU16_FLAG_BLOCK_MODE      0x0008      /**< Block mode flag */

#define orxCONFIG_VALUE_KU16_MASK_ALL             0xFFFF      /**< All mask */

/** Defines
 */
#define orxCONFIG_KU32_SECTION_BANK_SIZE          32          /**< Default section bank size */
#define orxCONFIG_KU32_STACK_BANK_SIZE            8           /**< Default stack bank size */
#define orxCONFIG_KU32_ENTRY_BANK_SIZE            8           /**< Default entry bank size */
#define orxCONFIG_KU32_HISTORY_BANK_SIZE          4           /**< Default history bank size */
#define orxCONFIG_KU32_BASE_FILENAME_LENGTH       256         /**< Base file name length */

#define orxCONFIG_KU32_BUFFER_SIZE                4096        /**< Buffer size */

#define orxCONFIG_KC_SECTION_START                '['         /**< Section start character */
#define orxCONFIG_KC_SECTION_END                  ']'         /**< Section end character */
#define orxCONFIG_KC_ASSIGN                       '='         /**< Assign character */
#define orxCONFIG_KC_COMMENT                      ';'         /**< Comment character */
#define orxCONFIG_KC_RANDOM_SEPARATOR             '~'         /**< Random number separator character */
#define orxCONFIG_KC_LIST_SEPARATOR               '#'         /**< List separator */
#define orxCONFIG_KC_SECTION_SEPARATOR            '.'         /**< Section separator */
#define orxCONFIG_KC_INHERITANCE_MARKER           '@'         /**< Inheritance marker character */
#define orxCONFIG_KC_BLOCK                        '"'         /**< Block delimiter character */

#define orxCONFIG_KZ_CONFIG_SECTION               "Config"    /**< Config section name */
#define orxCONFIG_KZ_CONFIG_HISTORY               "History"   /**< History config entry name */

#define orxCONFIG_KZ_DEFAULT_ENCRYPTION_KEY       "Orx Default Encryption Key =)" /**< Orx default encryption key */
#define orxCONFIG_KZ_ENCRYPTION_TAG               "OECF"      /**< Encryption file tag */
#define orxCONFIG_KU32_ENCRYPTION_TAG_LENGTH      4           /**< Encryption file tag length */

#ifdef __orxDEBUG__

  #define orxCONFIG_KZ_DEFAULT_FILE               "orxd.ini"  /**< Default config file name */

#else /* __orxDEBUG__ */

  #define orxCONFIG_KZ_DEFAULT_FILE               "orx.ini"   /**< Default config file name */

#endif /* __orxDEBUG__ */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Config value type enum
 */
typedef enum __orxCONFIG_VALUE_TYPE_t
{
  orxCONFIG_VALUE_TYPE_STRING = 0,
  orxCONFIG_VALUE_TYPE_FLOAT,
  orxCONFIG_VALUE_TYPE_S32,
  orxCONFIG_VALUE_TYPE_U32,
  orxCONFIG_VALUE_TYPE_BOOL,
  orxCONFIG_VALUE_TYPE_VECTOR,

  orxCONFIG_VALUE_TYPE_NUMBER,

  orxCONFIG_VALUE_TYPE_NONE = orxENUM_NONE

} orxCONFIG_VALUE_TYPE;

/** Config value structure
 */
typedef struct __orxCONFIG_VALUE_t
{
  orxSTRING             zValue;             /**< Literal value : 4 */
  orxCONFIG_VALUE_TYPE  eType;              /**< Value type : 8 */
  orxU16                u16Flags;           /**< Status flags : 10 */
  orxU8                 u8ListCounter;      /**< List counter : 11 */
  orxU8                 u8CacheIndex;       /**< Cache index : 12 */

  union
  {
    orxVECTOR           vValue;             /**< Vector value : 24 */
    orxBOOL             bValue;             /**< Bool value : 16 */
    orxFLOAT            fValue;             /**< Float value : 16 */
    orxU32              u32Value;           /**< U32 value : 16 */
    orxS32              s32Value;           /**< S32 value : 16 */
  };                                        /**< Union value : 24 */

  union
  {
    orxVECTOR           vAltValue;          /**< Alternate vector value : 36 */
    orxBOOL             bAltValue;          /**< Alternate bool value : 28 */
    orxFLOAT            fAltValue;          /**< Alternate float value : 28 */
    orxU32              u32AltValue;        /**< Alternate U32 value : 28 */
    orxS32              s32AltValue;        /**< Alternate S32 value : 28 */
  };                                        /**< Union value : 36 */

} orxCONFIG_VALUE;

/** Config entry structure
 */
typedef struct __orxCONFIG_ENTRY_t
{
  orxLINKLIST_NODE  stNode;                 /**< List node : 12 */
  orxSTRING         zKey;                   /**< Entry key : 16 */
  orxU32            u32ID;                  /**< Key ID (CRC) : 20 */

  orxCONFIG_VALUE   stValue;                /**< Entry value : 56 */

  orxPAD(56)

} orxCONFIG_ENTRY;

/** Config section structure
 */
typedef struct __orxCONFIG_SECTION_t
{
  orxLINKLIST_NODE  stNode;                 /**< List node : 12 */
  orxBANK          *pstEntryBank;           /**< Entry bank : 16 */
  orxSTRING         zName;                  /**< Section name : 20 */
  orxU32            u32ID;                  /**< Section CRC : 24 */
  orxU32            u32ParentID;            /**< Parent ID (CRC) : 28 */
  orxS32            s32ProtectionCounter;   /**< Protection counter : 32 */
  orxLINKLIST       stEntryList;            /**< Entry list : 44 */

  orxPAD(44)

} orxCONFIG_SECTION;

/** Config stack entry structure
 */
typedef struct __orxCONFIG_STACK_ENTRY_t
{
  orxLINKLIST_NODE    stNode;               /**< Linklist node : 12 */
  orxCONFIG_SECTION  *pstSection;           /**< Section : 16 */

} orxCONFIG_STACK_ENTRY;

/** Static structure
 */
typedef struct __orxCONFIG_STATIC_t
{
  orxBANK            *pstSectionBank;       /**< Section bank */
  orxCONFIG_SECTION  *pstCurrentSection;    /**< Current working section */
  orxBANK            *pstHistoryBank;       /**< History bank */
  orxBANK            *pstStackBank;         /**< Stack bank */
  orxLINKLIST         stStackList;          /**< Stack list */
  orxU32              u32Flags;             /**< Control flags */
  orxU32              u32LoadCounter;       /**< Load counter */
  orxSTRING           zEncryptionKey;       /**< Encryption key */
  orxU32              u32EncryptionKeySize; /**< Encryption key size */
  orxCHAR            *pcEncryptionChar;     /**< Current encryption char */
  orxLINKLIST         stSectionList;        /**< Section list */
  orxHASHTABLE       *pstSectionTable;      /**< Section table */
  orxCHAR             zBaseFile[orxCONFIG_KU32_BASE_FILENAME_LENGTH]; /**< Base file name */

} orxCONFIG_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** static data
 */
static orxCONFIG_STATIC sstConfig;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

static orxINLINE orxSTRING orxConfig_DuplicateValue(const orxSTRING _zValue, orxBOOL _bBlockMode)
{
  orxSTRING zResult;

  /* Checks */
  orxASSERT(orxString_GetLength(_zValue) < orxCONFIG_KU32_BUFFER_SIZE);

  /* Not in block mode? */
  if(_bBlockMode == orxFALSE)
  {
    orxCHAR acBuffer[orxCONFIG_KU32_BUFFER_SIZE], *pcInput, *pcOutput;

    /* For all characters */
    for(pcInput = _zValue, pcOutput = acBuffer; *pcInput != orxCHAR_NULL;)
    {
      /* Not a space? */
      if(*pcInput != ' ' && *pcInput != '\t')
      {
        /* Copies it */
        *pcOutput++ = *pcInput++;

        /* Is a list separator? */
        if(*(pcInput - 1) == orxCONFIG_KC_LIST_SEPARATOR)
        {
          /* Skips all trailing and leading spaces */
          while((*pcInput == ' ') || (*pcInput == '\t'))
          {
            pcInput++;
          }
        }
      }
      else
      {
        orxCHAR *pcTest;

        /* Skips all the spaces */
        for(pcTest = pcInput + 1; (*pcTest == ' ') || (*pcTest == '\t'); pcTest++);

        /* Is a list separator or end of string? */
        if((*pcTest == orxCONFIG_KC_LIST_SEPARATOR) || (*pcTest == orxCHAR_NULL))
        {
          /* Skips all trailing spaces */
          pcInput = pcTest;
        }
        else
        {
          /* For all spaces */
          for(pcTest; pcInput < pcTest; pcInput++, pcOutput++)
          {
            /* Copies it */
            *pcOutput = *pcInput;
          }
        }
      }
    }

    /* Ends string */
    *pcOutput = orxCHAR_NULL;

    /* Updates result */
    zResult = orxString_Duplicate(acBuffer);
  }
  else
  {
    /* Updates result */
    zResult = orxString_Duplicate(_zValue);
  }

  /* Done! */
  return zResult;
}

/** Computes a working config value (process random, inheritance and list attributes)
 * @param[in] _pstValue         Concerned config value
 */
static orxINLINE void orxConfig_ComputeWorkingValue(orxCONFIG_VALUE *_pstValue)
{
  orxCHAR  *pc;
  orxU8     u8Counter;
  orxU16    u16Flags;

  /* Checks */
  orxASSERT(_pstValue != orxNULL);

  /* Has inheritance? */
  if((*(_pstValue->zValue) == orxCONFIG_KC_INHERITANCE_MARKER)
  && (*(_pstValue->zValue + 1) != orxCONFIG_KC_INHERITANCE_MARKER))
  {
    /* Updates flags */
    u16Flags = orxCONFIG_VALUE_KU16_FLAG_INHERITANCE;
  }
  else
  {
    /* Updates flags */
    u16Flags = orxCONFIG_VALUE_KU16_FLAG_NONE;
  }

  /* For all characters */
  for(pc = _pstValue->zValue, u8Counter = 1; *pc != orxCHAR_NULL; pc++)
  {
    /* Is a list separator? */
    if(*pc == orxCONFIG_KC_LIST_SEPARATOR)
    {
      /* Not too long? */
      if(u8Counter < 0xFF)
      {
        /* Sets an end of string here */
        *pc = orxCHAR_NULL;

        /* Updates list counter */
        u8Counter++;

        /* Updates flags */
        u16Flags |= orxCONFIG_VALUE_KU16_FLAG_LIST;
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "List for value <%s> is too long, more than 255 values have been found.", _pstValue->zValue);
      }
    }
    else if(*pc == orxCONFIG_KC_RANDOM_SEPARATOR)
    {
      /* Updates flags */
      u16Flags |= orxCONFIG_VALUE_KU16_FLAG_RANDOM;
    }
  }

  /* Updates value flags */
  _pstValue->u16Flags = u16Flags;

  /* Updates list counter */
  _pstValue->u8ListCounter = u8Counter;
}

/** Restores a processed config value to its literal (for printing/saving/deleting purposes)
 * @param[in] _pstValue         Concerned config value
 */
static orxINLINE void orxConfig_RestoreLiteralValue(orxCONFIG_VALUE *_pstValue)
{
  orxCHAR  *pc;
  orxU8     u8Counter;

  /* Checks */
  orxASSERT(_pstValue != orxNULL);

  /* For all characters */
  for(u8Counter = _pstValue->u8ListCounter - 1, pc = _pstValue->zValue; u8Counter > 0; pc++)
  {
    /* Null character? */
    if(*pc == orxCHAR_NULL)
    {
      /* Updates it */
      *pc = orxCONFIG_KC_LIST_SEPARATOR;

      /* Updates counter */
      u8Counter--;
    }
  }

  /* Cleans list status */
  _pstValue->u16Flags       &= ~orxCONFIG_VALUE_KU16_FLAG_LIST;
  _pstValue->u8ListCounter   = 1;
  _pstValue->u8CacheIndex    = 0;
}

/** Gets a list value
 * @param[in] _pstValue         Concerned config value
 * @param[in] _s32Index         Index of the desired value
 */
static orxINLINE orxSTRING orxConfig_GetListValue(orxCONFIG_VALUE *_pstValue, orxS32 _s32Index)
{
  orxSTRING zResult;
  orxS32    s32Counter;

  /* Checks */
  orxASSERT(_pstValue != orxNULL);
  orxASSERT(_s32Index >= 0);

  /* Gets to the correct item start */
  for(zResult = _pstValue->zValue, s32Counter = _s32Index; s32Counter > 0; zResult++)
  {
    /* Null character? */
    if(*zResult == orxCHAR_NULL)
    {
      /* Updates counter */
      s32Counter--;
    }
  }

  /* Done! */
  return zResult;
}

/** En/De-crypts a buffer using global encryption key
 * @param[in] _acBuffer         Buffer to en/de-crypt
 * @param[in] _u32BufferSize    Buffer size
 */
static orxINLINE void orxConfig_CryptBuffer(orxCHAR *_acBuffer, orxU32 _u32BufferSize)
{
  orxCHAR *pc, *pcKey, *pcEndKey, *pcStartKey;

  orxASSERT(sstConfig.zEncryptionKey != orxNULL);

  /* Gets current, first and last encryption character */
  pcKey       = sstConfig.pcEncryptionChar;
  pcStartKey  = sstConfig.zEncryptionKey;
  pcEndKey    = sstConfig.zEncryptionKey + sstConfig.u32EncryptionKeySize - 1;

  /* For all characters */
  for(pc = _acBuffer; pc < _acBuffer + _u32BufferSize; pc++)
  {
    /* En/De-crypts character */
    *pc ^= *pcKey;

    /* Updates key pointer */
    pcKey = (pcKey == pcEndKey) ? sstConfig.zEncryptionKey : pcKey + 1;
  }

  /* Updates global current encryption character */
  sstConfig.pcEncryptionChar = pcKey;

  return;
}

/** Gets an entry from the current section
 * @param[in] _u32KeyID         Entry key ID
 * @return                      orxCONFIG_ENTRY / orxNULL
 */
static orxINLINE orxCONFIG_ENTRY *orxConfig_GetEntry(orxU32 _u32KeyID)
{
  orxCONFIG_ENTRY *pstResult = orxNULL, *pstEntry;

  /* Checks */
  orxASSERT(sstConfig.pstCurrentSection != orxNULL);

  /* For all entries */
  for(pstEntry = (orxCONFIG_ENTRY *)orxLinkList_GetFirst(&(sstConfig.pstCurrentSection->stEntryList));
      pstEntry != orxNULL;
      pstEntry = (orxCONFIG_ENTRY *)orxLinkList_GetNext(&(pstEntry->stNode)))
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

/** Forward declaration of orxConfig_GetValue
 */
static orxINLINE orxCONFIG_VALUE *orxConfig_GetValue(const orxSTRING _zKey);

/** Gets a value from the current section, using inheritance
 * @param[in] _u32KeyID         Entry key ID
 * @return                      orxCONFIG_VALUE / orxNULL
 */
static orxINLINE orxCONFIG_VALUE *orxConfig_GetValueFromKey(orxU32 _u32KeyID)
{
  orxCONFIG_ENTRY  *pstEntry;
  orxCONFIG_VALUE  *pstResult = orxNULL;

  /* Checks */
  orxASSERT(sstConfig.pstCurrentSection != orxNULL);

  /* Gets corresponding entry */
  pstEntry = orxConfig_GetEntry(_u32KeyID);

  /* Valid? */
  if(pstEntry != orxNULL)
  {
    /* Has local inheritance? */
    if(orxFLAG_TEST(pstEntry->stValue.u16Flags, orxCONFIG_VALUE_KU16_FLAG_INHERITANCE))
    {
      orxCONFIG_SECTION  *pstPreviousSection;
      orxS32              s32SeparatorIndex;

      /* Backups current section */
      pstPreviousSection = sstConfig.pstCurrentSection;

      /* Looks for inheritance index */
      s32SeparatorIndex = orxString_SearchCharIndex(pstEntry->stValue.zValue, orxCONFIG_KC_SECTION_SEPARATOR, 0);

      /* Found? */
      if(s32SeparatorIndex >= 0)
      {
        /* Cut the name */
        *(pstEntry->stValue.zValue + s32SeparatorIndex) = orxCHAR_NULL;

        /* Selects parent section */
        orxConfig_SelectSection(pstEntry->stValue.zValue + 1);

        /* Gets its inherited value */
        pstResult = orxConfig_GetValue(pstEntry->stValue.zValue + s32SeparatorIndex + 1);

        /* Cut the name */
        *(pstEntry->stValue.zValue + s32SeparatorIndex) = orxCONFIG_KC_SECTION_SEPARATOR;
      }
      else
      {
        /* Selects parent section */
        orxConfig_SelectSection(pstEntry->stValue.zValue + 1);

        /* Gets its inherited value */
        pstResult = orxConfig_GetValueFromKey(_u32KeyID);
      }

      /* Restores current section */
      sstConfig.pstCurrentSection = pstPreviousSection;
    }
    else
    {
      /* Updates result */
      pstResult = &(pstEntry->stValue);
    }
  }
  else
  {
    /* Has parent? */
    if(sstConfig.pstCurrentSection->u32ParentID != 0)
    {
      orxCONFIG_SECTION *pstSection;

      /* Gets it from table */
      pstSection = (orxCONFIG_SECTION *)orxHashTable_Get(sstConfig.pstSectionTable, sstConfig.pstCurrentSection->u32ParentID);

      /* Valid? */
      if(pstSection != orxNULL)
      {
        orxCONFIG_SECTION *pstPreviousSection;

        /* Backups current section */
        pstPreviousSection = sstConfig.pstCurrentSection;

        /* Sets parent as current section */
        sstConfig.pstCurrentSection = pstSection;

        /* Gets inherited value */
        pstResult = orxConfig_GetValueFromKey(_u32KeyID);

        /* Restores current section */
        sstConfig.pstCurrentSection = pstPreviousSection;
      }
    }
  }

  /* Done! */
  return pstResult;
}

/** Gets a value from the current section, using inheritance
 * @param[in] _zKeyID           Entry key
 * @return                      orxCONFIG_VALUE / orxNULL
 */
static orxINLINE orxCONFIG_VALUE *orxConfig_GetValue(const orxSTRING _zKey)
{
  orxCONFIG_VALUE *pstResult = orxNULL;

  /* Checks */
  orxASSERT(sstConfig.pstCurrentSection != orxNULL);

  /* Valid? */
  if((_zKey != orxSTRING_EMPTY) && (_zKey != orxNULL))
  {
    /* Gets value */
    pstResult = orxConfig_GetValueFromKey(orxString_ToCRC(_zKey));
  }

  /* Done! */
  return pstResult;
}

/** Adds an entry in the current section
 * @param[in] _zKey             Entry key
 * @param[in] _zValue           Entry value
 * @param[in] _bBlockMode       Block mode (ie. ignore special characters)?
 * @return                      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS orxConfig_AddEntry(const orxSTRING _zKey, const orxSTRING _zValue, orxBOOL _bBlockMode)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstConfig.pstCurrentSection != orxNULL);
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zValue != orxNULL);

  /* Valid? */
  if(_zKey != orxSTRING_EMPTY)
  {
    orxCONFIG_ENTRY *pstEntry;

    /* Creates entry */
    pstEntry = (orxCONFIG_ENTRY *)orxBank_Allocate(sstConfig.pstCurrentSection->pstEntryBank);

    /* Valid? */
    if(pstEntry != orxNULL)
    {
      /* Stores key */
      pstEntry->zKey = orxString_Duplicate(_zKey);

      /* Valid? */
      if(pstEntry->zKey != orxNULL)
      {
        /* Stores value */
        pstEntry->stValue.zValue = orxConfig_DuplicateValue(_zValue, _bBlockMode);

        /* Valid? */
        if(pstEntry->stValue.zValue != orxNULL)
        {
          /* Not in block mode? */
          if(_bBlockMode == orxFALSE)
          {
            /* Computes working value */
            orxConfig_ComputeWorkingValue(&(pstEntry->stValue));
          }
          else
          {
            /* Block mode, no list nor random allowed */
            pstEntry->stValue.u16Flags      = orxCONFIG_VALUE_KU16_FLAG_BLOCK_MODE;
            pstEntry->stValue.u8ListCounter = 1;
          }

          /* Adds it to list */
          orxMemory_Zero(&(pstEntry->stNode), sizeof(orxLINKLIST_NODE));
          orxLinkList_AddEnd(&(sstConfig.pstCurrentSection->stEntryList), &(pstEntry->stNode));

          /* Sets its ID */
          pstEntry->u32ID = orxString_ToCRC(pstEntry->zKey);

          /* Inits its type */
          pstEntry->stValue.eType = orxCONFIG_VALUE_TYPE_STRING;

          /* Updates result */
          eResult = orxSTATUS_SUCCESS;
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Failed to duplicate key string(%s).", _zKey);

          /* Deletes allocated string */
          orxString_Delete(pstEntry->zKey);

          /* Deletes entry */
          orxBank_Free(sstConfig.pstCurrentSection->pstEntryBank, pstEntry);
        }
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Failed to duplicate zValue string(%s).", _zValue);

        /* Deletes entry */
        orxBank_Free(sstConfig.pstCurrentSection->pstEntryBank, pstEntry);
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Deletes an entry
 * @param[in] _pstSection       Concerned section
 * @param[in] _pstEntry         Entry to delete
 */
static orxINLINE void orxConfig_DeleteEntry(orxCONFIG_SECTION *_pstSection, orxCONFIG_ENTRY *_pstEntry)
{
  /* Checks */
  orxASSERT(_pstEntry != orxNULL);

  /* Not in block mode? */
  if(!orxFLAG_TEST(_pstEntry->stValue.u16Flags, orxCONFIG_VALUE_KU16_FLAG_BLOCK_MODE))
  {
    /* Restore literal value */
    orxConfig_RestoreLiteralValue(&(_pstEntry->stValue));
  }

  /* Deletes key & value */
  orxString_Delete(_pstEntry->zKey);
  orxString_Delete(_pstEntry->stValue.zValue);

  /* Removes it from list */
  orxLinkList_Remove(&(_pstEntry->stNode));

  /* Deletes the entry */
  orxBank_Free(_pstSection->pstEntryBank, _pstEntry);

  return;
}

/** Creates a section
 * @param[in] _zSectionName     Name of the section to create
 * @param[in] _u32SectionID     ID of the section to create
 * @param[in] _u32ParentID      ID of the parent of the section to create
 */
static orxINLINE orxCONFIG_SECTION *orxConfig_CreateSection(const orxSTRING _zSectionName, orxU32 _u32SectionID, orxU32 _u32ParentID)
{
  orxCONFIG_SECTION *pstSection;

  /* Checks */
  orxASSERT(_zSectionName != orxNULL);
  orxASSERT(_zSectionName != orxSTRING_EMPTY);

  /* Valid? */
  if((_u32SectionID != 0) && (_u32SectionID != orxU32_UNDEFINED))
  {
    /* Allocates it */
    pstSection = (orxCONFIG_SECTION *)orxBank_Allocate(sstConfig.pstSectionBank);

    /* Valid? */
    if(pstSection != orxNULL)
    {
      /* Creates its bank */
      pstSection->pstEntryBank = orxBank_Create(orxCONFIG_KU32_ENTRY_BANK_SIZE, sizeof(orxCONFIG_ENTRY), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_CONFIG);

      /* Valid? */
      if(pstSection->pstEntryBank != orxNULL)
      {
        /* Duplicates its name */
        pstSection->zName = orxString_Duplicate(_zSectionName);

        /* Valid? */
        if(pstSection->zName != orxNULL)
        {
          /* Clears its entry list */
          orxMemory_Zero(&(pstSection->stEntryList), sizeof(orxLINKLIST));

          /* Adds it to list */
          orxMemory_Zero(&(pstSection->stNode), sizeof(orxLINKLIST_NODE));
          orxLinkList_AddEnd(&(sstConfig.stSectionList), &(pstSection->stNode));

          /* Adds it to table */
          orxHashTable_Add(sstConfig.pstSectionTable, _u32SectionID, pstSection);

          /* Sets its ID */
          pstSection->u32ID = _u32SectionID;

          /* Inits its parent ID */
          pstSection->u32ParentID = _u32ParentID;

          /* Clears its protection counter */
          pstSection->s32ProtectionCounter = 0;
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Duplicating section name failed.");

          /* Deletes its bank */
          orxBank_Delete(pstSection->pstEntryBank);

          /* Deletes it */
          orxBank_Free(sstConfig.pstSectionBank, pstSection);

          /* Updates result */
          pstSection = orxNULL;
        }
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Failed to create config bank.");

        /* Deletes the section */
        orxBank_Free(sstConfig.pstSectionBank, pstSection);

        /* Updates result */
        pstSection = orxNULL;
      }
    }
  }
  else
  {
    /* Updates result */
    pstSection = orxNULL;
  }

  /* Done! */
  return pstSection;
}

/** Deletes a section
 * @param[in] _pstSection       Section to delete
 */
static orxINLINE void orxConfig_DeleteSection(orxCONFIG_SECTION *_pstSection)
{
  orxCONFIG_ENTRY        *pstEntry;
  orxCONFIG_STACK_ENTRY  *pstStackEntry;

  /* Checks */
  orxASSERT(_pstSection != orxNULL);

  /* While there is still an entry */
  while((pstEntry = (orxCONFIG_ENTRY *)orxLinkList_GetFirst(&(_pstSection->stEntryList))) != orxNULL)
  {
    /* Deletes entry */
    orxConfig_DeleteEntry(_pstSection, pstEntry);
  }

  /* Not protected? */
  if(_pstSection->s32ProtectionCounter == 0)
  {
    /* For all stack entries */
    for(pstStackEntry = (orxCONFIG_STACK_ENTRY *)orxLinkList_GetFirst(&(sstConfig.stStackList));
        pstStackEntry != orxNULL;
        pstStackEntry = (orxCONFIG_STACK_ENTRY *)orxLinkList_GetNext(&(pstStackEntry->stNode)))
    {
      /* Is deleted section? */
      if(pstStackEntry->pstSection == _pstSection)
      {
        /* Removes it from stack list */
        orxLinkList_Remove(&(pstStackEntry->stNode));

        /* Deletes it */
        orxBank_Free(sstConfig.pstStackBank, pstStackEntry);

        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Warning: deleted section <%s> was previously pushed and has to be removed from stack.", _pstSection->zName);
      }
    }

    /* Is the current selected one? */
    if(sstConfig.pstCurrentSection == _pstSection)
    {
      /* Deselects it */
      sstConfig.pstCurrentSection = orxNULL;
    }

    /* Deletes its name */
    orxString_Delete(_pstSection->zName);

    /* Removes it from list */
    orxLinkList_Remove(&(_pstSection->stNode));

    /* Removes it from table */
    orxHashTable_Remove(sstConfig.pstSectionTable, _pstSection->u32ID);

    /* Removes section */
    orxBank_Free(sstConfig.pstSectionBank, _pstSection);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Warning: section <%s> can't be deleted as it's protected by %d entities.", _pstSection->zName, _pstSection->s32ProtectionCounter);
  }

  return;
}

/** Reads a signed integer value from config value
 * @param[in]   _pstValue         Concerned config value
 * @param[in]   _s32ListIndex      List index
 * @return The value
 */
static orxINLINE orxS32 orxConfig_GetS32FromValue(orxCONFIG_VALUE *_pstValue, orxS32 _s32ListIndex)
{
  orxS32 s32Result = 0;

  /* Checks */
  orxASSERT(_pstValue != orxNULL);
  orxASSERT(_s32ListIndex < _pstValue->u8ListCounter);

  /* Random index? */
  if(_s32ListIndex < 0)
  {
    /* Not a list? */
    if(!orxFLAG_TEST(_pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_LIST))
    {
      /* Updates real index */
      _s32ListIndex = 0;
    }
    else
    {
      /* Updates real index */
      _s32ListIndex = (orxS32)orxMath_GetRandomU32(0, (orxU32)_pstValue->u8ListCounter - 1);
    }
  }

  /* Is it cached and on the same index? */
  if((_pstValue->eType == orxCONFIG_VALUE_TYPE_S32) && (_s32ListIndex == _pstValue->u8CacheIndex))
  {
    /* Random? */
    if(orxFLAG_TEST(_pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_RANDOM))
    {
      /* Updates result */
      s32Result = orxMath_GetRandomS32(_pstValue->s32Value, _pstValue->s32AltValue);
    }
    else
    {
      /* Updates result */
      s32Result = _pstValue->s32Value;
    }
  }
  else
  {
    orxS32    s32Value;
    orxSTRING zRemainder, zStart;

    /* Gets wanted value */
    zStart = orxConfig_GetListValue(_pstValue, _s32ListIndex);

    /* Gets value */
    if(orxString_ToS32(zStart, &s32Value, &zRemainder) != orxSTATUS_FAILURE)
    {
      orxS32  s32RandomSeparatorIndex = 0;
      orxBOOL bRandom = orxFALSE;

      /* Random? */
      if(orxFLAG_TEST(_pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_RANDOM))
      {
        /* Searches for the random separator */
        s32RandomSeparatorIndex = orxString_SearchCharIndex(zRemainder, orxCONFIG_KC_RANDOM_SEPARATOR, 0);

        /* Valid? */
        if((s32RandomSeparatorIndex >= 0) && (*(zRemainder + s32RandomSeparatorIndex + 1) != orxCONFIG_KC_RANDOM_SEPARATOR))
        {
          /* Updates random status */
          bRandom = orxTRUE;
        }
      }

      /* Random? */
      if(bRandom != orxFALSE)
      {
        orxS32 s32OtherValue;

        /* Has another value? */
        if(orxString_ToS32(zRemainder + s32RandomSeparatorIndex + 1, &s32OtherValue, orxNULL) != orxSTATUS_FAILURE)
        {
          /* Updates cache */
          _pstValue->eType        = orxCONFIG_VALUE_TYPE_S32;
          _pstValue->u8CacheIndex = (orxU8)_s32ListIndex;
          _pstValue->s32Value     = s32Value;
          _pstValue->s32AltValue  = s32OtherValue;

          /* Updates result */
          s32Result = orxMath_GetRandomS32(s32Value, s32OtherValue);
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Failed to get S32 random from config value <%s>.", _pstValue->zValue);

          /* Clears cache */
          _pstValue->eType = orxCONFIG_VALUE_TYPE_STRING;

          /* Updates result */
          s32Result = s32Value;
        }
      }
      else
      {
        /* Updates cache */
        _pstValue->eType        = orxCONFIG_VALUE_TYPE_S32;
        _pstValue->u8CacheIndex = (orxU8)_s32ListIndex;
        _pstValue->s32Value     = s32Value;

        /* Updates result */
        s32Result = s32Value;
      }
    }
  }

  /* Done! */
  return s32Result;
}

/** Reads an unsigned integer value from config value
 * @param[in]   _pstValue         Concerned config value
 * @param[in]   _s32ListIndex      List index
 * @return The value
 */
static orxINLINE orxU32 orxConfig_GetU32FromValue(orxCONFIG_VALUE *_pstValue, orxS32 _s32ListIndex)
{
  orxU32 u32Result = 0;

  /* Checks */
  orxASSERT(_pstValue != orxNULL);
  orxASSERT(_s32ListIndex < _pstValue->u8ListCounter);

  /* Random index? */
  if(_s32ListIndex < 0)
  {
    /* Not a list? */
    if(!orxFLAG_TEST(_pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_LIST))
    {
      /* Updates real index */
      _s32ListIndex = 0;
    }
    else
    {
      /* Updates real index */
      _s32ListIndex = (orxS32)orxMath_GetRandomU32(0, (orxS32)_pstValue->u8ListCounter - 1);
    }
  }

  /* Is it cached and on the same index? */
  if((_pstValue->eType == orxCONFIG_VALUE_TYPE_U32) && (_s32ListIndex == _pstValue->u8CacheIndex))
  {
    /* Random? */
    if(orxFLAG_TEST(_pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_RANDOM))
    {
      /* Updates result */
      u32Result = orxMath_GetRandomU32(_pstValue->u32Value, _pstValue->u32AltValue);
    }
    else
    {
      /* Updates result */
      u32Result = _pstValue->u32Value;
    }
  }
  else
  {
    orxU32    u32Value;
    orxSTRING zRemainder, zStart;

    /* Gets wanted value */
    zStart = orxConfig_GetListValue(_pstValue, _s32ListIndex);

    /* Gets value */
    if(orxString_ToU32(zStart, &u32Value, &zRemainder) != orxSTATUS_FAILURE)
    {
      orxS32  s32RandomSeparatorIndex = 0;
      orxBOOL bRandom = orxFALSE;

      /* Random? */
      if(orxFLAG_TEST(_pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_RANDOM))
      {
        /* Searches for the random separator */
        s32RandomSeparatorIndex = orxString_SearchCharIndex(zRemainder, orxCONFIG_KC_RANDOM_SEPARATOR, 0);

        /* Valid? */
        if((s32RandomSeparatorIndex >= 0) && (*(zRemainder + s32RandomSeparatorIndex + 1) != orxCONFIG_KC_RANDOM_SEPARATOR))
        {
          /* Updates random status */
          bRandom = orxTRUE;
        }
      }

      /* Random? */
      if(bRandom != orxFALSE)
      {
        orxU32 u32OtherValue;

        /* Has another value? */
        if(orxString_ToU32(zRemainder + s32RandomSeparatorIndex + 1, &u32OtherValue, orxNULL) != orxSTATUS_FAILURE)
        {
          /* Updates cache */
          _pstValue->eType        = orxCONFIG_VALUE_TYPE_U32;
          _pstValue->u8CacheIndex = (orxU8)_s32ListIndex;
          _pstValue->u32Value     = u32Value;
          _pstValue->u32AltValue  = u32OtherValue;

          /* Updates result */
          u32Result = orxMath_GetRandomU32(u32Value, u32OtherValue);
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Failed to get U32 random from config value <%s>.", _pstValue->zValue);

          /* Clears cache */
          _pstValue->eType = orxCONFIG_VALUE_TYPE_STRING;

          /* Updates result */
          u32Result = u32Value;
        }
      }
      else
      {
        /* Updates cache */
        _pstValue->eType        = orxCONFIG_VALUE_TYPE_U32;
        _pstValue->u8CacheIndex = (orxU8)_s32ListIndex;
        _pstValue->u32Value     = u32Value;

        /* Updates result */
        u32Result = u32Value;
      }
    }
  }

  /* Done! */
  return u32Result;
}

/** Reads a float value from config value
 * @param[in]   _pstValue         Concerned config value
 * @param[in]   _s32ListIndex      List index
 * @return The value
 */
static orxINLINE orxFLOAT orxConfig_GetFloatFromValue(orxCONFIG_VALUE *_pstValue, orxS32 _s32ListIndex)
{
  orxFLOAT fResult = orxFLOAT_0;

  /* Checks */
  orxASSERT(_pstValue != orxNULL);
  orxASSERT(_s32ListIndex < _pstValue->u8ListCounter);

  /* Random index? */
  if(_s32ListIndex < 0)
  {
    /* Not a list? */
    if(!orxFLAG_TEST(_pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_LIST))
    {
      /* Updates real index */
      _s32ListIndex = 0;
    }
    else
    {
      /* Updates real index */
      _s32ListIndex = (orxS32)orxMath_GetRandomU32(0, (orxU32)_pstValue->u8ListCounter - 1);
    }
  }

  /* Is it cached and on the same index? */
  if((_pstValue->eType == orxCONFIG_VALUE_TYPE_FLOAT) && (_s32ListIndex == _pstValue->u8CacheIndex))
  {
    /* Random? */
    if(orxFLAG_TEST(_pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_RANDOM))
    {
      /* Updates result */
      fResult = orxMath_GetRandomFloat(_pstValue->fValue, _pstValue->fAltValue);
    }
    else
    {
      /* Updates result */
      fResult = _pstValue->fValue;
    }
  }
  else
  {
    orxFLOAT  fValue;
    orxSTRING zRemainder, zStart;

    /* Gets wanted value */
    zStart = orxConfig_GetListValue(_pstValue, _s32ListIndex);

    /* Gets value */
    if(orxString_ToFloat(zStart, &fValue, &zRemainder) != orxSTATUS_FAILURE)
    {
      orxS32  s32RandomSeparatorIndex = 0;
      orxBOOL bRandom = orxFALSE;

      /* Random? */
      if(orxFLAG_TEST(_pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_RANDOM))
      {
        /* Searches for the random separator */
        s32RandomSeparatorIndex = orxString_SearchCharIndex(zRemainder, orxCONFIG_KC_RANDOM_SEPARATOR, 0);

        /* Valid? */
        if((s32RandomSeparatorIndex >= 0) && (*(zRemainder + s32RandomSeparatorIndex + 1) != orxCONFIG_KC_RANDOM_SEPARATOR))
        {
          /* Updates random status */
          bRandom = orxTRUE;
        }
      }

      /* Random? */
      if(bRandom != orxFALSE)
      {
        orxFLOAT fOtherValue;

        /* Has another value? */
        if(orxString_ToFloat(zRemainder + s32RandomSeparatorIndex + 1, &fOtherValue, orxNULL) != orxSTATUS_FAILURE)
        {
          /* Updates cache */
          _pstValue->eType        = orxCONFIG_VALUE_TYPE_FLOAT;
          _pstValue->u8CacheIndex = (orxU8)_s32ListIndex;
          _pstValue->fValue       = fValue;
          _pstValue->fAltValue    = fOtherValue;

          /* Updates result */
          fResult = orxMath_GetRandomFloat(fValue, fOtherValue);
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Failed to get FLOAT random from config value <%s>.", _pstValue->zValue);

          /* Clears cache */
          _pstValue->eType = orxCONFIG_VALUE_TYPE_STRING;

          /* Updates result */
          fResult = fValue;
        }
      }
      else
      {
        /* Updates cache */
        _pstValue->eType        = orxCONFIG_VALUE_TYPE_FLOAT;
        _pstValue->u8CacheIndex = (orxU8)_s32ListIndex;
        _pstValue->fValue       = fValue;

        /* Updates result */
        fResult = fValue;
      }
    }
  }

  /* Done! */
  return fResult;
}

/** Reads a string value from config value
 * @param[in]   _pstValue         Concerned config value
 * @param[in]   _s32ListIndex      List index
 * @return The value
 */
static orxINLINE orxSTRING orxConfig_GetStringFromValue(orxCONFIG_VALUE *_pstValue, orxS32 _s32ListIndex)
{
  orxSTRING zResult;

  /* Checks */
  orxASSERT(_pstValue != orxNULL);
  orxASSERT(_s32ListIndex < _pstValue->u8ListCounter);

  /* Random index? */
  if(_s32ListIndex < 0)
  {
    /* Not a list? */
    if(!orxFLAG_TEST(_pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_LIST))
    {
      /* Updates real index */
      _s32ListIndex = 0;
    }
    else
    {
      /* Updates real index */
      _s32ListIndex = (orxS32)orxMath_GetRandomU32(0, (orxU32)_pstValue->u8ListCounter - 1);
    }
  }

  /* Is it cached and on the same index? */
  if((_pstValue->eType == orxCONFIG_VALUE_TYPE_BOOL) && (_s32ListIndex == _pstValue->u8CacheIndex))
  {
    /* Updates result */
    zResult = _pstValue->zValue;
  }
  else
  {
    /* Gets wanted value */
    zResult = orxConfig_GetListValue(_pstValue, _s32ListIndex);
  }

  /* Done! */
  return zResult;
}

/** Reads a boolean value from config value
 * @param[in]   _pstValue         Concerned config value
 * @param[in]   _s32ListIndex      List index
 * @return The value
 */
static orxINLINE orxBOOL orxConfig_GetBoolFromValue(orxCONFIG_VALUE *_pstValue, orxS32 _s32ListIndex)
{
  orxBOOL bResult = orxFALSE;

  /* Checks */
  orxASSERT(_pstValue != orxNULL);
  orxASSERT(_s32ListIndex < _pstValue->u8ListCounter);

  /* Random index? */
  if(_s32ListIndex < 0)
  {
    /* Not a list? */
    if(!orxFLAG_TEST(_pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_LIST))
    {
      /* Updates real index */
      _s32ListIndex = 0;
    }
    else
    {
      /* Updates real index */
      _s32ListIndex = (orxS32)orxMath_GetRandomU32(0, (orxU32)_pstValue->u8ListCounter - 1);
    }
  }

  /* Is it cached and on the same index? */
  if((_pstValue->eType == orxCONFIG_VALUE_TYPE_BOOL) && (_s32ListIndex == _pstValue->u8CacheIndex))
  {
    /* Updates result */
    bResult = _pstValue->bValue;
  }
  else
  {
    orxBOOL   bValue;
    orxSTRING zStart;

    /* Gets wanted value */
    zStart = orxConfig_GetListValue(_pstValue, _s32ListIndex);

    /* Gets value */
    if(orxString_ToBool(zStart, &bValue, orxNULL) != orxSTATUS_FAILURE)
    {
      /* Updates cache */
      _pstValue->eType        = orxCONFIG_VALUE_TYPE_BOOL;
      _pstValue->u8CacheIndex = (orxU8)_s32ListIndex;
      _pstValue->bValue       = bValue;

      /* Updates result */
      bResult = bValue;
    }
  }

  /* Done! */
  return bResult;
}

/** Reads a vector value from config value
 * @param[in]   _pstValue         Concerned config value
 * @param[in]   _s32ListIndex      List index
 * @param[out]  _pvVector         Storage for vector value
 * @return The value
 */
static orxINLINE orxVECTOR *orxConfig_GetVectorFromValue(orxCONFIG_VALUE *_pstValue, orxS32 _s32ListIndex, orxVECTOR *_pvVector)
{
  orxVECTOR *pvResult = orxNULL;

  /* Checks */
  orxASSERT(_pstValue != orxNULL);
  orxASSERT(_s32ListIndex < _pstValue->u8ListCounter);
  orxASSERT(_pvVector != orxNULL);

  /* Random index? */
  if(_s32ListIndex < 0)
  {
    /* Not a list? */
    if(!orxFLAG_TEST(_pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_LIST))
    {
      /* Updates real index */
      _s32ListIndex = 0;
    }
    else
    {
      /* Updates real index */
      _s32ListIndex = (orxS32)orxMath_GetRandomU32(0, (orxU32)_pstValue->u8ListCounter - 1);
    }
  }

  /* Is it cached and on the same index? */
  if((_pstValue->eType == orxCONFIG_VALUE_TYPE_VECTOR) && (_s32ListIndex == _pstValue->u8CacheIndex))
  {
    /* Random? */
    if(orxFLAG_TEST(_pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_RANDOM))
    {
      /* Gets random values from cache */
      _pvVector->fX = orxMath_GetRandomFloat(_pstValue->vValue.fX, _pstValue->vAltValue.fX);
      _pvVector->fY = orxMath_GetRandomFloat(_pstValue->vValue.fY, _pstValue->vAltValue.fY);
      _pvVector->fZ = orxMath_GetRandomFloat(_pstValue->vValue.fZ, _pstValue->vAltValue.fZ);
    }
    else
    {
      /* Copies value from cache */
      orxVector_Copy(_pvVector, &(_pstValue->vValue));
    }

    /* Updates result */
    pvResult = _pvVector;
  }
  else
  {
    orxSTRING zRemainder, zStart;

    /* Gets wanted value */
    zStart = orxConfig_GetListValue(_pstValue, _s32ListIndex);

    /* Gets value */
    if(orxString_ToVector(zStart, _pvVector, &zRemainder) != orxSTATUS_FAILURE)
    {
      orxS32  s32RandomSeparatorIndex = 0;
      orxBOOL bRandom = orxFALSE;

      /* Random? */
      if(orxFLAG_TEST(_pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_RANDOM))
      {
        /* Searches for the random separator */
        s32RandomSeparatorIndex = orxString_SearchCharIndex(zRemainder, orxCONFIG_KC_RANDOM_SEPARATOR, 0);

        /* Valid? */
        if((s32RandomSeparatorIndex >= 0) && (*(zRemainder + s32RandomSeparatorIndex + 1) != orxCONFIG_KC_RANDOM_SEPARATOR))
        {
          /* Updates random status */
          bRandom = orxTRUE;
        }
      }

      /* Random? */
      if(bRandom != orxFALSE)
      {
        orxVECTOR vOtherValue;

        /* Has another value? */
        if(orxString_ToVector(zRemainder + s32RandomSeparatorIndex + 1, &vOtherValue, orxNULL) != orxSTATUS_FAILURE)
        {
          /* Updates cache */
          _pstValue->eType        = orxCONFIG_VALUE_TYPE_VECTOR;
          _pstValue->u8CacheIndex = (orxU8)_s32ListIndex;
          orxVector_Copy(&(_pstValue->vValue), _pvVector);
          orxVector_Copy(&(_pstValue->vAltValue), &vOtherValue);

          /* Updates result */
          _pvVector->fX = orxMath_GetRandomFloat(_pvVector->fX, vOtherValue.fX);
          _pvVector->fY = orxMath_GetRandomFloat(_pvVector->fY, vOtherValue.fY);
          _pvVector->fZ = orxMath_GetRandomFloat(_pvVector->fZ, vOtherValue.fZ);
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Failed to get VECTOR random from config value <%s>.", _pstValue->zValue);

          /* Clears cache */
          _pstValue->eType = orxCONFIG_VALUE_TYPE_STRING;
        }
      }
      else
      {
        /* Updates cache */
        _pstValue->eType        = orxCONFIG_VALUE_TYPE_VECTOR;
        _pstValue->u8CacheIndex = (orxU8)_s32ListIndex;
        orxVector_Copy(&(_pstValue->vValue), _pvVector);
      }

      /* Updates result */
      pvResult = _pvVector;
    }
  }

  /* Done! */
  return pvResult;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Config module setup
 */
void orxFASTCALL orxConfig_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_CONFIG, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_CONFIG, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_CONFIG, orxMODULE_ID_FILE);

  return;
}

/** Inits the config module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY))
  {
    orxCHAR   zBackupBaseFile[orxCONFIG_KU32_BASE_FILENAME_LENGTH];
    orxSTRING zBackupEncryptionKey;

    /* Backups base file name */
    orxMemory_Copy(zBackupBaseFile, sstConfig.zBaseFile, orxCONFIG_KU32_BASE_FILENAME_LENGTH);

    /* Backups encryption key */
    zBackupEncryptionKey = sstConfig.zEncryptionKey;

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

    /* Valid encryption key? */
    if(zBackupEncryptionKey != orxNULL)
    {
      /* Restores it */
      sstConfig.zEncryptionKey        = zBackupEncryptionKey;
      sstConfig.u32EncryptionKeySize  = orxString_GetLength(zBackupEncryptionKey);
      sstConfig.pcEncryptionChar      = sstConfig.zEncryptionKey;
    }
    else
    {
      /* Sets default encryption key */
      orxConfig_SetEncryptionKey(orxCONFIG_KZ_DEFAULT_ENCRYPTION_KEY);
    }

    /* Creates stack bank & section bank/table */
    sstConfig.pstStackBank    = orxBank_Create(orxCONFIG_KU32_STACK_BANK_SIZE, sizeof(orxCONFIG_STACK_ENTRY), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_CONFIG);
    sstConfig.pstSectionBank  = orxBank_Create(orxCONFIG_KU32_SECTION_BANK_SIZE, sizeof(orxCONFIG_SECTION), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_CONFIG);
    sstConfig.pstSectionTable = orxHashTable_Create(orxCONFIG_KU32_SECTION_BANK_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if((sstConfig.pstStackBank != orxNULL) && (sstConfig.pstSectionBank != orxNULL) && (sstConfig.pstSectionTable != orxNULL))
    {
      /* Inits Flags */
      orxFLAG_SET(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY, orxCONFIG_KU32_STATIC_MASK_ALL);

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;

      /* Loads default config file */
      orxConfig_Load(sstConfig.zBaseFile);

      /* Pushes config section */
      orxConfig_PushSection(orxCONFIG_KZ_CONFIG_SECTION);

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
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Failed to create history bank.");

          /* Updates result */
          eResult = orxSTATUS_FAILURE;

          /* Clears Flags */
          orxFLAG_SET(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_NONE, orxCONFIG_KU32_STATIC_MASK_ALL);

          /* Deletes created banks and table */
          orxBank_Delete(sstConfig.pstSectionBank);
          orxBank_Delete(sstConfig.pstStackBank);
          orxHashTable_Delete(sstConfig.pstSectionTable);
        }
      }

      /* Pops section */
      orxConfig_PopSection();
    }
    else
    {
      /* Should delete stack bank? */
      if(sstConfig.pstStackBank != orxNULL)
      {
        /* Deletes it */
        orxBank_Delete(sstConfig.pstStackBank);
      }

      /* Should delete section bank? */
      if(sstConfig.pstSectionBank != orxNULL)
      {
        /* Deletes it */
        orxBank_Delete(sstConfig.pstSectionBank);
      }

      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Can't allocate stack bank and/or section bank/table.");
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Tried to initialize config module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Exits from the config module
 */
void orxFASTCALL orxConfig_Exit()
{
  /* Initialized? */
  if(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY))
  {
    /* Clears all data */
    orxConfig_Clear();

    /* Clears section list */
    orxLinkList_Clean(&(sstConfig.stSectionList));

    /* Deletes section table */
    orxHashTable_Delete(sstConfig.pstSectionTable);
    sstConfig.pstSectionTable = orxNULL;

    /* Deletes section bank */
    orxBank_Delete(sstConfig.pstSectionBank);
    sstConfig.pstSectionBank = orxNULL;

    /* Deletes stack bank */
    orxBank_Delete(sstConfig.pstStackBank);
    sstConfig.pstStackBank = orxNULL;

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

/** Sets encryption key
 * @param[in] _zEncryption key  Encryption key to use, orxNULL to clear
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_SetEncryptionKey(const orxSTRING _zEncryptionKey)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Had encryption key? */
  if(sstConfig.zEncryptionKey != orxNULL)
  {
    /* Deletes it */
    orxString_Delete(sstConfig.zEncryptionKey);
  }

  /* Has new key? */
  if((_zEncryptionKey != orxNULL) && (_zEncryptionKey != orxSTRING_EMPTY))
  {
    /* Updates values */
    sstConfig.zEncryptionKey        = orxString_Duplicate(_zEncryptionKey);
    sstConfig.u32EncryptionKeySize  = orxString_GetLength(sstConfig.zEncryptionKey);
    sstConfig.pcEncryptionChar      = sstConfig.zEncryptionKey;
  }
  else
  {
    /* Updates values */
    sstConfig.zEncryptionKey        = orxNULL;
    sstConfig.u32EncryptionKeySize  = 0;
    sstConfig.pcEncryptionChar      = orxNULL;
  }

  /* Done! */
  return eResult;
}

/** Sets config base name
 * @param[in] _zBaseName        Base name used for default config file
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_SetBaseName(const orxSTRING _zBaseName)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(orxString_GetLength(_zBaseName) < orxCONFIG_KU32_BASE_FILENAME_LENGTH - 5);

  /* Valid? */
  if((_zBaseName != orxNULL) && (_zBaseName != orxSTRING_EMPTY))
  {
    /* Mac? */
    #ifdef __orxMAC__

      orxS32 s32Index, s32NextIndex;

      /* Finds last directory separator */
      for(s32Index = orxString_SearchCharIndex(_zBaseName, orxCHAR_DIRECTORY_SEPARATOR, 0);
          (s32Index >= 0) && ((s32NextIndex = orxString_SearchCharIndex(_zBaseName, orxCHAR_DIRECTORY_SEPARATOR, s32Index + 1)) > 0);
          s32Index = s32NextIndex);

      /* Found? */
      if(s32Index > 0)
      {
        /* Removes it */
        *(_zBaseName + s32Index) = orxCHAR_NULL;

        /* Sets current directory */
        chdir(_zBaseName);

        /* Restores separator */
        *(_zBaseName + s32Index) = orxCHAR_DIRECTORY_SEPARATOR;
      }

    #endif /* __orxMAC__ */

    /* Copies it */
    orxString_NPrint(sstConfig.zBaseFile, orxCONFIG_KU32_BASE_FILENAME_LENGTH - 1, "%s.ini", _zBaseName);
    sstConfig.zBaseFile[orxCONFIG_KU32_BASE_FILENAME_LENGTH - 1] = orxCHAR_NULL;
  }
  else
  {
    /* Uses default name */
    orxString_Copy(sstConfig.zBaseFile, orxCONFIG_KZ_DEFAULT_FILE);
  }

  /* Done! */
  return eResult;
}

/** Gets config main file name
 * @return Config main file name / orxSTRING_EMPTY
 */
const orxSTRING orxFASTCALL orxConfig_GetMainFileName()
{
  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));

  /* Done! */
  return sstConfig.zBaseFile;
}

/** Selects current working section
 * @param[in] _zSectionName     Section name to select
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_SelectSection(const orxSTRING _zSectionName)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zSectionName != orxNULL);

  /* Valid? */
  if(_zSectionName != orxSTRING_EMPTY)
  {
    orxCONFIG_SECTION  *pstSection;
    orxCHAR            *pcNameEnd;
    orxU32              u32SectionID, u32ParentID;
    orxS32              s32MarkerIndex;

    /* Looks for inheritance index */
    s32MarkerIndex = orxString_SearchCharIndex(_zSectionName, orxCONFIG_KC_INHERITANCE_MARKER, 0);

    /* Found? */
    if(s32MarkerIndex >= 0)
    {
      orxSTRING zParent;

      /* Cut the name */
      *(_zSectionName + s32MarkerIndex) = orxCHAR_NULL;

      /* Gets end of name */
      for(pcNameEnd = _zSectionName + s32MarkerIndex - 1; (pcNameEnd > _zSectionName) && (*pcNameEnd == ' '); pcNameEnd--);

      /* Should trim? */
      if(((++pcNameEnd) < _zSectionName + s32MarkerIndex) && (pcNameEnd > _zSectionName))
      {
        /* Ends name here */
        *pcNameEnd = orxCHAR_NULL;
      }
      else
      {
        /* Clears name end pointer */
        pcNameEnd = orxNULL;
      }

      /* Gets parent name */
      for(zParent = _zSectionName + s32MarkerIndex + 1; *zParent == ' '; zParent++);

      /* Gets its parent ID */
      u32ParentID = orxString_ToCRC(zParent);
    }
    else
    {
      /* Clears parent ID */
      u32ParentID = orxU32_UNDEFINED;

      /* Clears end of name */
      pcNameEnd = orxNULL;
    }

    /* Gets the section ID */
    u32SectionID = orxString_ToCRC(_zSectionName);

    /* Not already selected? */
    if((sstConfig.pstCurrentSection == orxNULL)
    || (sstConfig.pstCurrentSection->u32ID != u32SectionID))
    {
      /* Gets it from table */
      pstSection = (orxCONFIG_SECTION *)orxHashTable_Get(sstConfig.pstSectionTable, u32SectionID);

      /* Valid? */
      if(pstSection != orxNULL)
      {
        /* Selects it */
        sstConfig.pstCurrentSection = pstSection;
      }
    }
    else
    {
      /* Updates selection */
      pstSection = sstConfig.pstCurrentSection;
    }

    /* Not found? */
    if(pstSection == orxNULL)
    {
      /* Creates it */
      pstSection = orxConfig_CreateSection(_zSectionName, u32SectionID, (u32ParentID != orxU32_UNDEFINED) ? u32ParentID : 0);

      /* Success? */
      if(pstSection != orxNULL)
      {
        /* Selects it */
        sstConfig.pstCurrentSection = pstSection;
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Failed to create config section with parameters (%s, %0X, %0X).", _zSectionName, u32SectionID, u32ParentID);

        /* Updates result */
        eResult = orxSTATUS_FAILURE;
      }
    }
    else
    {
      /* Loading? */
      if(sstConfig.u32LoadCounter != 0)
      {
        /* Has new parent ID? */
        if(u32ParentID != orxU32_UNDEFINED)
        {
          /* Updates parent ID */
          pstSection->u32ParentID = u32ParentID;
        }
      }
    }

    /* Had trimmed name */
    if(pcNameEnd != orxNULL)
    {
      /* Restores space */
      *pcNameEnd = ' ';
    }

    /* Had inheritance marker? */
    if(s32MarkerIndex >= 0)
    {
      /* Restores it */
      *(_zSectionName + s32MarkerIndex) = orxCONFIG_KC_INHERITANCE_MARKER;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Failed to select config section (%s), invalid name.", _zSectionName);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets a section's parent
 * @param[in] _zSectionName     Concerned section, if the section doesn't exist, it will be created
 * @param[in] _zParentName      Parent section's name, if the section doesn't exist, it will be created, if orxNULL is provided, the former parent will be erased
 */
orxSTATUS orxFASTCALL orxConfig_SetParent(const orxSTRING _zSectionName, const orxSTRING _zParentName)
{
  orxCONFIG_SECTION  *pstPreviousSection;
  orxSTATUS           eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zSectionName != orxNULL);

  /* Backups current section */
  pstPreviousSection = sstConfig.pstCurrentSection;

  /* Selects concerned section */
  eResult = orxConfig_SelectSection(_zSectionName);

  /* Success? */
  if(eResult != orxSTATUS_FAILURE)
  {
    /* Has parent? */
    if(_zParentName != orxNULL)
    {
      orxCONFIG_SECTION *pstSection;

      /* Stores it */
      pstSection = sstConfig.pstCurrentSection;

      /* Selects parent section */
      eResult = orxConfig_SelectSection(_zParentName);

      /* Success? */
      if(eResult != orxSTATUS_FAILURE)
      {
        /* Updates concerned section's parent */
        pstSection->u32ParentID = sstConfig.pstCurrentSection->u32ID;
      }
    }
    else
    {
      /* Clears its parent */
      sstConfig.pstCurrentSection->u32ParentID = 0;
    }
  }

  /* Restores previous section */
  sstConfig.pstCurrentSection = pstPreviousSection;

  /* Done! */
  return eResult;
}

/** Gets a section's parent
 * @param[in] _zSectionName     Concerned section, if the section doesn't exist, it will be created
 * @return Section's parent name / orxNULL
 */
const orxSTRING orxFASTCALL orxConfig_GetParent(const orxSTRING _zSectionName)
{
  orxSTRING zResult = orxNULL;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zSectionName != orxNULL);

  /* Has section? */
  if(orxConfig_HasSection(_zSectionName) != orxFALSE)
  {
    orxCONFIG_SECTION *pstPreviousSection;

    /* Backups current section */
    pstPreviousSection = sstConfig.pstCurrentSection;

    /* Selects concerned section */
    if(orxConfig_SelectSection(_zSectionName) != orxSTATUS_FAILURE)
    {
      /* Has parent? */
      if(sstConfig.pstCurrentSection->u32ParentID != 0)
      {
        orxCONFIG_SECTION *pstParentSection;

        /* Gets it from table */
        pstParentSection = (orxCONFIG_SECTION *)orxHashTable_Get(sstConfig.pstSectionTable, sstConfig.pstCurrentSection->u32ParentID);

        /* Updates result */
        zResult = pstParentSection->zName;
      }
    }

    /* Restores previous section */
    sstConfig.pstCurrentSection = pstPreviousSection;
  }

  /* Done! */
  return zResult;
}

/** Gets current working section
 * @return Current selected section
 */
const orxSTRING orxFASTCALL orxConfig_GetCurrentSection()
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

/** Pushes a section (storing the current one on section stack)
 * @param[in] _zSectionName     Section name to push
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_PushSection(const orxSTRING _zSectionName)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zSectionName != orxNULL);

  /* Valid? */
  if(_zSectionName != orxSTRING_EMPTY)
  {
    orxCONFIG_SECTION *pstCurrentSection;

    /* Stores current section */
    pstCurrentSection = sstConfig.pstCurrentSection;

    /* Selects requested section */
    eResult = orxConfig_SelectSection(_zSectionName);

    /* Success? */
    if(eResult != orxSTATUS_FAILURE)
    {
      orxCONFIG_STACK_ENTRY *pstStackEntry;

      /* Allocates stack entry */
      pstStackEntry = (orxCONFIG_STACK_ENTRY *)orxBank_Allocate(sstConfig.pstStackBank);

      /* Valid? */
      if(pstStackEntry != orxNULL)
      {
        /* Updates it */
        pstStackEntry->pstSection = pstCurrentSection;

        /* Adds it at end of list */
        orxLinkList_AddEnd(&(sstConfig.stStackList), &(pstStackEntry->stNode));
      }
      else
      {
        /* Restores current section */
        sstConfig.pstCurrentSection = pstCurrentSection;

        /* Updates result */
        eResult = orxSTATUS_FAILURE;
      }
    }
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Pops last section from section stack
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_PopSection()
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));

  /* Has stacked entry? */
  if(orxLinkList_GetCounter(&(sstConfig.stStackList)) > 0)
  {
    orxCONFIG_STACK_ENTRY *pstStackEntry;

    /* Gets stack entry */
    pstStackEntry = (orxCONFIG_STACK_ENTRY *)orxLinkList_GetLast(&(sstConfig.stStackList));

    /* Updates current section */
    sstConfig.pstCurrentSection = pstStackEntry->pstSection;

    /* Removes it from stack list */
    orxLinkList_Remove(&(pstStackEntry->stNode));

    /* Deletes it */
    orxBank_Free(sstConfig.pstStackBank, pstStackEntry);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Read config config from source.
 * @param[in] _zFileName        File name
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_Load(const orxSTRING _zFileName)
{
  orxFILE  *pstFile;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zFileName != orxNULL);

  /* Should keep history? */
  if(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_HISTORY))
  {
    /* External call? */
    if(sstConfig.u32LoadCounter == 0)
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
  }

  /* Updates load counter */
  sstConfig.u32LoadCounter++;

  /* Valid file to open? */
  if((_zFileName != orxSTRING_EMPTY) && ((pstFile = orxFile_Open(_zFileName, orxFILE_KU32_FLAG_OPEN_READ | orxFILE_KU32_FLAG_OPEN_BINARY)) != orxNULL))
  {
    orxCHAR             acBuffer[orxCONFIG_KU32_BUFFER_SIZE], *pcPreviousEncryptionChar;
    orxU32              u32Size, u32Offset;
    orxBOOL             bUseEncryption = orxFALSE, bFirstTime;
    orxCONFIG_SECTION  *pstPreviousSection;

    /* Gets previous config section */
    pstPreviousSection = sstConfig.pstCurrentSection;

    /* Gets previous encryption character */
    pcPreviousEncryptionChar = sstConfig.pcEncryptionChar;

    /* Reinits current encryption character */
    sstConfig.pcEncryptionChar = sstConfig.zEncryptionKey;

    /* While file isn't empty */
    for(u32Size = orxFile_Read(acBuffer, sizeof(orxCHAR), orxCONFIG_KU32_BUFFER_SIZE, pstFile), u32Offset = 0, bFirstTime = orxTRUE;
        u32Size > 0;
        u32Size = orxFile_Read(acBuffer + u32Offset, sizeof(orxCHAR), orxCONFIG_KU32_BUFFER_SIZE - u32Offset, pstFile) + u32Offset, bFirstTime = orxFALSE)
    {
      orxCHAR  *pc, *pcKeyEnd, *pcValueStart, *pcLineStart;
      orxBOOL   bBlockMode;

      /* First time? */
      if(bFirstTime != orxFALSE)
      {
        /* Has encryption tag? */
        if(orxString_NCompare(acBuffer, orxCONFIG_KZ_ENCRYPTION_TAG, orxCONFIG_KU32_ENCRYPTION_TAG_LENGTH) == 0)
        {
          /* Updates encryption status */
          bUseEncryption = orxTRUE;

          /* Updates start of line */
          pcLineStart = acBuffer + orxCONFIG_KU32_ENCRYPTION_TAG_LENGTH;

          /* Updates offset */
          u32Offset = orxCONFIG_KU32_ENCRYPTION_TAG_LENGTH;
        }
        else
        {
          /* Updates encryption status */
          bUseEncryption = orxFALSE;

          /* Updates start of line */
          pcLineStart = acBuffer;
        }
      }
      else
      {
        /* Updates start of line */
        pcLineStart = acBuffer;
      }

      /* Uses encryption? */
      if(bUseEncryption != orxFALSE)
      {
        /* Decrypts all new characters */
        orxConfig_CryptBuffer(acBuffer + u32Offset, u32Size - u32Offset);
      }

      /* End of file reached? */
      if(u32Size < orxCONFIG_KU32_BUFFER_SIZE)
      {
        /* Adds an extra EOL */
        acBuffer[u32Size++] = orxCHAR_LF;
      }

      /* For all buffered characters */
      for(pc = pcLineStart, pcKeyEnd = pcValueStart = orxNULL, bBlockMode = orxFALSE;
          pc < acBuffer + u32Size;
          pc++)
      {
        /* Comment character or EOL out of block mode or block character in block mode? */
        if((((*pc == orxCONFIG_KC_COMMENT)
          || (*pc == orxCHAR_CR)
          || (*pc == orxCHAR_LF))
         && (bBlockMode == orxFALSE))
        || ((bBlockMode != orxFALSE)
         && (*pc == orxCONFIG_KC_BLOCK)))
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

            /* Skips the whole line */
            while((pc < acBuffer + u32Size) && (*pc != orxCHAR_CR) && (*pc != orxCHAR_LF))
            {
              pc++;
            }

            /* Empty? */
            if((bBlockMode == orxFALSE) && ((*pcValueStart == orxCONFIG_KC_COMMENT) || (*pcValueStart == orxCHAR_CR) || (*pcValueStart == orxCHAR_LF)))
            {
              /* Uses empty string */
              pcValueStart = (orxCHAR *)orxSTRING_EMPTY;
            }

            /* Cuts the strings */
            *pcKeyEnd = *(++pcValueEnd) = orxCHAR_NULL;

            /* Gets key ID */
            u32KeyID = orxString_ToCRC(pcLineStart);

            /* Already defined? */
            if((pstEntry = orxConfig_GetEntry(u32KeyID)) != orxNULL)
            {
              /* Not in block mode? */
              if(!orxFLAG_TEST(pstEntry->stValue.u16Flags, orxCONFIG_VALUE_KU16_FLAG_BLOCK_MODE))
              {
                /* Restores literal value */
                orxConfig_RestoreLiteralValue(&(pstEntry->stValue));
              }

              /* Logs */
              orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Config entry [%s::%s]: Replacing value \"%s\" with new value \"%s\" from <%s>.", sstConfig.pstCurrentSection->zName, pstEntry->zKey, pstEntry->stValue.zValue, pcValueStart, _zFileName);

              /* Deletes entry */
              orxConfig_DeleteEntry(sstConfig.pstCurrentSection, pstEntry);
            }

            /* Adds entry */
            orxConfig_AddEntry(pcLineStart, pcValueStart, bBlockMode);

            /* Updates pointers */
            pcKeyEnd = pcValueStart = orxNULL;
          }
          else
          {
            /* Skips the whole line */
            while((pc < acBuffer + u32Size) && (*pc != orxCHAR_CR) && (*pc != orxCHAR_LF))
            {
              pc++;
            }
          }

          /* Resets block mode */
          bBlockMode = orxFALSE;

          /* Valid? */
          if(pc < acBuffer + u32Size)
          {
            /* Updates line start pointer */
            pcLineStart = pc + 1;
          }
          else
          {
            /* Updates line start pointer */
            pcLineStart = pc - 1;

            /* Makes sure we don't mistake remaining partial comment for a new key */
            *pcLineStart = orxCONFIG_KC_COMMENT;
          }
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

          /* Inheritance marker? */
          if(*pc == orxCONFIG_KC_INHERITANCE_MARKER)
          {
            /* Updates pointer */
            pc++;

            /* Finds section end */
            while((pc < acBuffer + u32Size) && (*pc != orxCONFIG_KC_INHERITANCE_MARKER))
            {
              /* End of line? */
              if((*pc == orxCHAR_CR) || (*pc == orxCHAR_LF))
              {
                /* Logs message */
                orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "File name <%*s> incomplete, closing character '%c' not found.", pc - (pcLineStart + 1), pcLineStart + 1, orxCONFIG_KC_INHERITANCE_MARKER);

                /* Updates new line start */
                pcLineStart = pc + 1;

                break;
              }

              /* Updates pointer */
              pc++;
            }

            /* Valid? */
            if((pc < acBuffer + u32Size) && (*pc == orxCONFIG_KC_INHERITANCE_MARKER))
            {
              orxCONFIG_SECTION *pstCurrentSection;

              /* Gets current section */
              pstCurrentSection = sstConfig.pstCurrentSection;

              /* Cuts string */
              *pc = orxCHAR_NULL;

              /* Logs message */
              orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "[%s]: Begins the processing of included file %c%s%c.", _zFileName, orxCONFIG_KC_INHERITANCE_MARKER, pcLineStart + 1, orxCONFIG_KC_INHERITANCE_MARKER);

              /* Loads file */
              orxConfig_Load(pcLineStart + 1);

              /* Logs message */
              orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "[%s]: Ends the processing of included file %c%s%c.", _zFileName, orxCONFIG_KC_INHERITANCE_MARKER, pcLineStart + 1, orxCONFIG_KC_INHERITANCE_MARKER);

              /* Restores current section */
              sstConfig.pstCurrentSection = pstCurrentSection;

              /* Skips the whole line */
              while((pc < acBuffer + u32Size)  && (*pc != orxCHAR_CR) && (*pc != orxCHAR_LF))
              {
                pc++;
              }

              /* Valid? */
              if(pc < acBuffer + u32Size)
              {
                /* Updates line start pointer */
                pcLineStart = pc + 1;
              }
              else
              {
                /* Updates line start pointer */
                pcLineStart = pc - 1;

                /* Makes sure we don't mistake remaining partial comment for a new key */
                *pcLineStart = orxCONFIG_KC_COMMENT;
              }
            }
          }
          /* Section start? */
          else if(*pc == orxCONFIG_KC_SECTION_START)
          {
            /* Finds section end */
            while((pc < acBuffer + u32Size) && (*pc != orxCONFIG_KC_SECTION_END))
            {
              /* End of line? */
              if((*pc == orxCHAR_CR) || (*pc == orxCHAR_LF))
              {
                /* Logs message */
                *pc = orxCHAR_NULL;
                orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Section name <%s> incomplete, closing character '%c' not found.", pcLineStart + 1, orxCONFIG_KC_SECTION_END);

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
              while((pc < acBuffer + u32Size)  && (*pc != orxCHAR_CR) && (*pc != orxCHAR_LF))
              {
                pc++;
              }

              /* Valid? */
              if(pc < acBuffer + u32Size)
              {
                /* Updates line start pointer */
                pcLineStart = pc + 1;
              }
              else
              {
                /* Updates line start pointer */
                pcLineStart = pc - 1;

                /* Makes sure we don't mistake remaining partial comment for a new key */
                *pcLineStart = orxCONFIG_KC_COMMENT;
              }
            }
          }
          /* Comment character? */
          else if(*pc == orxCONFIG_KC_COMMENT)
          {
            /* Skips the whole line */
            while((pc < acBuffer + u32Size)  && (*pc != orxCHAR_CR) && (*pc != orxCHAR_LF))
            {
              pc++;
            }

            /* Valid? */
            if(pc < acBuffer + u32Size)
            {
              /* Updates line start pointer */
              pcLineStart = pc + 1;
            }
            else
            {
              /* Updates line start pointer */
              pcLineStart = pc - 1;

              /* Makes sure we don't mistake remaining partial comment for a new key */
              *pcLineStart = orxCONFIG_KC_COMMENT;
            }
          }
          else
          {
            /* Finds assign character */
            while((pc < acBuffer + u32Size) && (*pc != orxCONFIG_KC_ASSIGN) && (*pc != orxCHAR_CR) && (*pc != orxCHAR_LF))
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
                  (pcValueStart < acBuffer + u32Size) && ((*pcValueStart == ' ') || (*pcValueStart == '\t'));
                  pcValueStart++);

              /* Valid? */
              if(pcValueStart < acBuffer + u32Size)
              {
                /* Is it a block delimiter character? */
                if(*pcValueStart == orxCONFIG_KC_BLOCK)
                {
                  /* Updates value start pointer */
                  pcValueStart++;

                  /* Valid? */
                  if(pcValueStart < acBuffer + u32Size)
                  {
                    /* Is not a block delimiter? */
                    if(*pcValueStart != orxCONFIG_KC_BLOCK)
                    {
                      /* Activates block mode */
                      bBlockMode = orxTRUE;
                    }
                  }
                }
              }

              /* Updates current character */
              pc = pcValueStart;
            }
            else
            {
              /* Not at end of buffer */
              if(pc < acBuffer + u32Size)
              {
                /* Logs message */
                *pc = orxCHAR_NULL;
                orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Key <%s> has no value, assign character '%c' not found.", pcLineStart, orxCONFIG_KC_ASSIGN);
              }
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

    /* Pops previous section */
    sstConfig.pstCurrentSection = pstPreviousSection;

    /* Restores previous encryption character */
    sstConfig.pcEncryptionChar = pcPreviousEncryptionChar;
  }
  else
  {
    /* Logs */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Config file [%s] couldn't be loaded.", _zFileName);
  }

  /* Updates load counter */
  sstConfig.u32LoadCounter--;

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
    for(pzHistoryEntry = (orxSTRING *)orxBank_GetNext(sstConfig.pstHistoryBank, orxNULL);
        (pzHistoryEntry != orxNULL) && (eResult != orxSTATUS_FAILURE);
        pzHistoryEntry = (orxSTRING *)orxBank_GetNext(sstConfig.pstHistoryBank, pzHistoryEntry))
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
 * @param[in] _bUseEncryption   Use file encryption to make it human non-readable?
 * @param[in] _pfnSaveCallback  Callback used to filter section/key to save. If NULL is passed, all section/keys will be saved
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_Save(const orxSTRING _zFileName, orxBOOL _bUseEncryption, const orxCONFIG_SAVE_FUNCTION _pfnSaveCallback)
{
  orxFILE  *pstFile;
  orxSTRING zFileName;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));

  /* No encryption requested or has a valid key? */
  if((_bUseEncryption == orxFALSE) || (sstConfig.zEncryptionKey != orxNULL))
  {
    /* Is given file name invalid? */
    if((_zFileName == orxNULL) || (_zFileName == orxSTRING_EMPTY))
    {
      /* Uses default file */
      zFileName = sstConfig.zBaseFile;
    }
    else
    {
      /* Uses given one */
      zFileName = _zFileName;
    }

    /* Opens file */
    pstFile = orxFile_Open(zFileName, orxFILE_KU32_FLAG_OPEN_READ | orxFILE_KU32_FLAG_OPEN_WRITE | orxFILE_KU32_FLAG_OPEN_BINARY);

    /* Valid? */
    if(pstFile != orxNULL)
    {
      orxCONFIG_SECTION  *pstSection;
      orxCHAR             acBuffer[orxCONFIG_KU32_BUFFER_SIZE], *pcPreviousEncryptionChar = orxNULL;
      orxU32              u32BufferSize;

      /* Use encryption? */
      if(_bUseEncryption != orxFALSE)
      {
        /* Gets previous encryption character */
        pcPreviousEncryptionChar = sstConfig.pcEncryptionChar;

        /* Resets current encryption character */
        sstConfig.pcEncryptionChar = sstConfig.zEncryptionKey;

        /* Adds encryption tag */
        orxFile_Print(pstFile, "%s", orxCONFIG_KZ_ENCRYPTION_TAG);
      }

      /* For all sections */
      for(pstSection = (orxCONFIG_SECTION *)orxLinkList_GetFirst(&(sstConfig.stSectionList));
          pstSection != orxNULL;
          pstSection = (orxCONFIG_SECTION *)orxLinkList_GetNext(&(pstSection->stNode)))
      {
        /* No callback or should save it? */
        if((_pfnSaveCallback == orxNULL) || (_pfnSaveCallback(pstSection->zName, orxNULL, _bUseEncryption) != orxFALSE))
        {
          orxCONFIG_SECTION *pstParentSection = orxNULL;
          orxCONFIG_ENTRY   *pstEntry;

          /* Has a parent ID? */
          if(pstSection->u32ParentID != 0)
          {
            /* Gets it from table */
            pstParentSection = (orxCONFIG_SECTION *)orxHashTable_Get(sstConfig.pstSectionTable, pstSection->u32ParentID);
          }

          /* Has a parent section */
          if(pstParentSection != orxNULL)
          {
            /* Writes section name with inheritance */
            u32BufferSize = (orxU32)orxString_NPrint(acBuffer, orxCONFIG_KU32_BUFFER_SIZE - 1, "%c%s%c%s%c%s", orxCONFIG_KC_SECTION_START, pstSection->zName, orxCONFIG_KC_INHERITANCE_MARKER, pstParentSection->zName, orxCONFIG_KC_SECTION_END, orxSTRING_EOL);
            acBuffer[orxCONFIG_KU32_BUFFER_SIZE - 1] = orxCHAR_NULL;
          }
          else
          {
            /* Writes section name */
            u32BufferSize = (orxU32)orxString_NPrint(acBuffer, orxCONFIG_KU32_BUFFER_SIZE - 1, "%c%s%c%s", orxCONFIG_KC_SECTION_START, pstSection->zName, orxCONFIG_KC_SECTION_END, orxSTRING_EOL);
            acBuffer[orxCONFIG_KU32_BUFFER_SIZE - 1] = orxCHAR_NULL;
          }

          /* Encrypt? */
          if(_bUseEncryption != orxFALSE)
          {
            /* Encrypts buffer */
            orxConfig_CryptBuffer(acBuffer, u32BufferSize);
          }

          /* Saves it */
          orxFile_Write(acBuffer, sizeof(orxCHAR), u32BufferSize, pstFile);

          /* For all entries */
          for(pstEntry = (orxCONFIG_ENTRY *)orxLinkList_GetFirst(&(pstSection->stEntryList));
              pstEntry != orxNULL;
              pstEntry = (orxCONFIG_ENTRY *)orxLinkList_GetNext(&(pstEntry->stNode)))
          {
            /* No callback or should save it? */
            if((_pfnSaveCallback == orxNULL) || (_pfnSaveCallback(pstSection->zName, pstEntry->zKey, _bUseEncryption) != orxFALSE))
            {
              /* Not in block mode? */
              if(!orxFLAG_TEST(pstEntry->stValue.u16Flags, orxCONFIG_VALUE_KU16_FLAG_BLOCK_MODE))
              {
                /* Restores string */
                orxConfig_RestoreLiteralValue(&(pstEntry->stValue));

                /* Writes it */
                u32BufferSize = (orxU32)orxString_NPrint(acBuffer, orxCONFIG_KU32_BUFFER_SIZE - 1, "%s %c %s%s", pstEntry->zKey, orxCONFIG_KC_ASSIGN, pstEntry->stValue.zValue, orxSTRING_EOL);
                acBuffer[orxCONFIG_KU32_BUFFER_SIZE - 1] = orxCHAR_NULL;

                /* Computes working value */
                orxConfig_ComputeWorkingValue(&(pstEntry->stValue));
              }
              else
              {
                /* Writes it */
                u32BufferSize = (orxU32)orxString_NPrint(acBuffer, orxCONFIG_KU32_BUFFER_SIZE - 1, "%s %c %c%s%c%s", pstEntry->zKey, orxCONFIG_KC_ASSIGN, orxCONFIG_KC_BLOCK, pstEntry->stValue.zValue, orxCONFIG_KC_BLOCK, orxSTRING_EOL);
                acBuffer[orxCONFIG_KU32_BUFFER_SIZE - 1] = orxCHAR_NULL;
              }

              /* Encrypt? */
              if(_bUseEncryption != orxFALSE)
              {
                /* Encrypts buffer */
                orxConfig_CryptBuffer(acBuffer, u32BufferSize);
              }

              /* Saves it */
              orxFile_Write(acBuffer, sizeof(orxCHAR), u32BufferSize, pstFile);
            }
          }

          /* Adds a new line */
          u32BufferSize = (orxU32)orxString_Print(acBuffer, "%s", orxSTRING_EOL);

          /* Encrypt? */
          if(_bUseEncryption != orxFALSE)
          {
            /* Encrypts buffer */
            orxConfig_CryptBuffer(acBuffer, u32BufferSize);
          }

          /* Saves it */
          orxFile_Write(acBuffer, sizeof(orxCHAR), u32BufferSize, pstFile);
        }
      }

      /* Flushes & closes the file */
      eResult = orxFile_Close(pstFile);

      /* Use encryption? */
      if(_bUseEncryption != orxFALSE)
      {
        /* Restores previous encryption character */
        sstConfig.pcEncryptionChar = pcPreviousEncryptionChar;
      }
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Can't save config file <%s> with encryption: no valid encryption key provided!", _zFileName);
  }

  /* Done! */
  return eResult;
}

/** Has specified value for the given key?
 * @param[in] _zKey             Key name
 * @return orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxConfig_HasValue(const orxSTRING _zKey)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);

  /* Updates result */
  bResult = (orxConfig_GetValue(_zKey) != orxNULL) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

/** Has section for the given section name?
 * @param[in] _zSectionName     Section name
 * @return orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxConfig_HasSection(const orxSTRING _zSectionName)
{
  orxCONFIG_SECTION  *pstSection;
  orxU32              u32ID;
  orxBOOL             bResult = orxFALSE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zSectionName != orxNULL);

  /* Valid? */
  if(_zSectionName != orxSTRING_EMPTY)
  {
    /* Gets section name ID */
    u32ID = orxString_ToCRC(_zSectionName);

    /* Gets it from table */
    pstSection = (orxCONFIG_SECTION *)orxHashTable_Get(sstConfig.pstSectionTable, u32ID);

    /* Valid? */
    if(pstSection != orxNULL)
    {
      /* Updates result */
      bResult = orxTRUE;
    }
  }

  /* Done! */
  return bResult;
}

/** Protects/unprotects a section from deletion (content might still be changed or deleted, but the section itself will resist delete/clear calls)
 * @param[in] _zSectionName     Section name to protect
 * @param[in] _bProtect         orxTRUE for protecting the section, orxFALSE to remove the protection
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_ProtectSection(const orxSTRING _zSectionName, orxBOOL _bProtect)
{
  orxCONFIG_SECTION  *pstSection;
  orxU32              u32ID;
  orxSTATUS           eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zSectionName != orxNULL);
  orxASSERT(_zSectionName != orxSTRING_EMPTY);

  /* Gets section name ID */
  u32ID = orxString_ToCRC(_zSectionName);

  /* Gets it from table */
  pstSection = (orxCONFIG_SECTION *)orxHashTable_Get(sstConfig.pstSectionTable, u32ID);

  /* Valid? */
  if(pstSection != orxNULL)
  {
    /* Updates protection counter */
    pstSection->s32ProtectionCounter += (_bProtect != orxFALSE) ? 1 : -1;

    /* Checks */
    orxASSERT(pstSection->s32ProtectionCounter >= 0);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Clears all config data
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_Clear()
{
  orxCONFIG_SECTION *pstLastSection, *pstNewSection;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));

  /* For all sections */
  for(pstLastSection = orxNULL, pstNewSection = (orxCONFIG_SECTION *)orxLinkList_GetFirst(&(sstConfig.stSectionList));
      pstNewSection != orxNULL;
      pstNewSection = (pstLastSection != orxNULL) ? (orxCONFIG_SECTION *)orxLinkList_GetNext(&(pstLastSection->stNode)) : (orxCONFIG_SECTION *)orxLinkList_GetFirst(&(sstConfig.stSectionList)))
  {
    /* Checks */
    orxASSERT(pstNewSection->s32ProtectionCounter >= 0);

    /* Protected? */
    if(pstNewSection->s32ProtectionCounter > 0)
    {
      /* Updates last section */
      pstLastSection = pstNewSection;
    }

    /* Deletes section */
    orxConfig_DeleteSection(pstNewSection);
  }

  /* Done! */
  return orxSTATUS_SUCCESS;
}

/** Clears section
 * @param[in] _zSectionName     Section name to clear
 */
orxSTATUS orxFASTCALL orxConfig_ClearSection(const orxSTRING _zSectionName)
{
  orxCONFIG_SECTION  *pstSection;
  orxU32              u32ID;
  orxSTATUS           eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zSectionName != orxNULL);
  orxASSERT(_zSectionName != orxSTRING_EMPTY);

  /* Gets section name ID */
  u32ID = orxString_ToCRC(_zSectionName);

  /* Gets it from table */
  pstSection = (orxCONFIG_SECTION *)orxHashTable_Get(sstConfig.pstSectionTable, u32ID);

  /* Valid? */
  if(pstSection != orxNULL)
  {
    /* Deletes it */
    orxConfig_DeleteSection(pstSection);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Clears a value from current selected section
 * @param[in] _zKey             Key name
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_ClearValue(const orxSTRING _zKey)
{
  orxCONFIG_ENTRY  *pstEntry;
  orxSTATUS         eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);

  /* Gets entry */
  pstEntry = orxConfig_GetEntry(orxString_ToCRC(_zKey));

  /* Found? */
  if(pstEntry != orxNULL)
  {
    /* Deletes it */
    orxConfig_DeleteEntry(sstConfig.pstCurrentSection, pstEntry);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Reads a signed integer value from config (will take a random value if a list is provided for this key)
 * @param[in] _zKey             Key name
 * @return The value
 */
orxS32 orxFASTCALL orxConfig_GetS32(const orxSTRING _zKey)
{
  orxCONFIG_VALUE  *pstValue;
  orxS32            s32Result;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);

  /* Gets corresponding value */
  pstValue = orxConfig_GetValue(_zKey);

  /* Found? */
  if(pstValue != orxNULL)
  {
    /* Updates result */
    s32Result = orxConfig_GetS32FromValue(pstValue, -1);
  }
  else
  {
    /* Updates result */
    s32Result = 0;
  }

  /* Done! */
  return s32Result;
}

/** Reads an unsigned integer value from config (will take a random value if a list is provided for this key)
 * @param[in] _zKey             Key name
 * @return The value
 */
orxU32 orxFASTCALL orxConfig_GetU32(const orxSTRING _zKey)
{
  orxCONFIG_VALUE  *pstValue;
  orxU32            u32Result;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);

  /* Gets corresponding value */
  pstValue = orxConfig_GetValue(_zKey);

  /* Found? */
  if(pstValue != orxNULL)
  {
    /* Updates result */
    u32Result = orxConfig_GetU32FromValue(pstValue, -1);
  }
  else
  {
    /* Updates result */
    u32Result = 0;
  }

  /* Done! */
  return u32Result;
}

/** Reads a float value from config (will take a random value if a list is provided for this key)
 * @param[in] _zKey             Key name
 * @return The value
 */
orxFLOAT orxFASTCALL orxConfig_GetFloat(const orxSTRING _zKey)
{
  orxCONFIG_VALUE  *pstValue;
  orxFLOAT          fResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);

  /* Gets corresponding value */
  pstValue = orxConfig_GetValue(_zKey);

  /* Found? */
  if(pstValue != orxNULL)
  {
    /* Updates result */
    fResult = orxConfig_GetFloatFromValue(pstValue, -1);
  }
  else
  {
    /* Updates result */
    fResult = orxFLOAT_0;
  }

  /* Done! */
  return fResult;
}

/** Reads a string value from config (will take a random value if a list is provided for this key)
 * @param[in] _zKey             Key name
 * @return The value
 */
const orxSTRING orxFASTCALL orxConfig_GetString(const orxSTRING _zKey)
{
  orxCONFIG_VALUE  *pstValue;
  orxSTRING         zResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);

  /* Gets corresponding value */
  pstValue = orxConfig_GetValue(_zKey);

  /* Found? */
  if(pstValue != orxNULL)
  {
    /* Updates result */
    zResult = orxConfig_GetStringFromValue(pstValue, -1);
  }
  else
  {
    /* Updates result */
    zResult = orxSTRING_EMPTY;
  }

  /* Done! */
  return zResult;
}

/** Reads a boolean value from config (will take a random value if a list is provided for this key)
 * @param[in] _zKey             Key name
 * @return The value
 */
orxBOOL orxFASTCALL orxConfig_GetBool(const orxSTRING _zKey)
{
  orxCONFIG_VALUE  *pstValue;
  orxBOOL           bResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);

  /* Gets corresponding value */
  pstValue = orxConfig_GetValue(_zKey);

  /* Found? */
  if(pstValue != orxNULL)
  {
    /* Updates result */
    bResult = orxConfig_GetBoolFromValue(pstValue, -1);
  }
  else
  {
    /* Updates result */
    bResult = orxFALSE;
  }

  /* Done! */
  return bResult;
}

/** Reads a vector value from config (will take a random value if a list is provided for this key)
 * @param[in]   _zKey             Key name
 * @param[out]  _pvVector         Storage for vector value
 * @return The value
 */
orxVECTOR *orxFASTCALL orxConfig_GetVector(const orxSTRING _zKey, orxVECTOR *_pvVector)
{
  orxCONFIG_VALUE  *pstValue;
  orxVECTOR        *pvResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);
  orxASSERT(_pvVector != orxNULL);

  /* Gets corresponding value */
  pstValue = orxConfig_GetValue(_zKey);

  /* Found? */
  if(pstValue != orxNULL)
  {
    /* Updates result */
    pvResult = orxConfig_GetVectorFromValue(pstValue, -1, _pvVector);
  }
  else
  {
    /* Updates result */
    pvResult = orxNULL;
  }

  /* Done! */
  return pvResult;
}

/** Writes an integer value to config
 * @param[in] _zKey             Key name
 * @param[in] _s32Value         Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_SetS32(const orxSTRING _zKey, orxS32 _s32Value)
{
  orxCONFIG_ENTRY  *pstEntry;
  orxCHAR           zValue[16];
  orxSTATUS         eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);

  /* Clears buffer */
  orxMemory_Zero(zValue, 16 * sizeof(orxCHAR));

  /* Gets literal value */
  orxString_Print(zValue, "%ld", _s32Value);

  /* Gets entry */
  pstEntry = orxConfig_GetEntry(orxString_ToCRC(_zKey));

  /* Found? */
  if(pstEntry != orxNULL)
  {
    /* Deletes it */
    orxConfig_DeleteEntry(sstConfig.pstCurrentSection, pstEntry);
  }

  /* Adds new entry */
  eResult = orxConfig_AddEntry(_zKey, zValue, orxFALSE);

  /* Done! */
  return eResult;
}

/** Writes an unsigned integer value to config
 * @param[in] _zKey             Key name
 * @param[in] _u32Value         Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_SetU32(const orxSTRING _zKey, orxU32 _u32Value)
{
  orxCONFIG_ENTRY  *pstEntry;
  orxCHAR           zValue[16];
  orxSTATUS         eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);

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
  eResult = orxConfig_AddEntry(_zKey, zValue, orxFALSE);

  /* Done! */
  return eResult;
}

/** Writes a float value to config
 * @param[in] _zKey             Key name
 * @param[in] _fValue           Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_SetFloat(const orxSTRING _zKey, orxFLOAT _fValue)
{
  orxCONFIG_ENTRY  *pstEntry;
  orxCHAR           zValue[16];
  orxSTATUS         eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);

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
  eResult = orxConfig_AddEntry(_zKey, zValue, orxFALSE);

  /* Done! */
  return eResult;
}

/** Writes a string value to config
 * @param[in] _zKey             Key name
 * @param[in] _fValue           Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_SetString(const orxSTRING _zKey, const orxSTRING _zValue)
{
  orxCONFIG_ENTRY  *pstEntry;
  orxSTATUS         eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);
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
  eResult = orxConfig_AddEntry(_zKey, _zValue, orxFALSE);

  /* Done! */
  return eResult;
}

/** Writes a boolean value to config
 * @param[in] _zKey             Key name
 * @param[in] _fValue           Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_SetBool(const orxSTRING _zKey, orxBOOL _bValue)
{
  orxCONFIG_ENTRY  *pstEntry;
  orxSTATUS         eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);

  /* Gets entry */
  pstEntry = orxConfig_GetEntry(orxString_ToCRC(_zKey));

  /* Found? */
  if(pstEntry != orxNULL)
  {
    /* Deletes it */
    orxConfig_DeleteEntry(sstConfig.pstCurrentSection, pstEntry);
  }

  /* Adds new entry */
  eResult = orxConfig_AddEntry(_zKey, (_bValue == orxFALSE) ? orxSTRING_FALSE : orxSTRING_TRUE, orxFALSE);

  /* Done! */
  return eResult;
}

/** Writes a vector value to config
 * @param[in] _zKey             Key name
 * @param[in] _pvValue         Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_SetVector(const orxSTRING _zKey, const orxVECTOR *_pvValue)
{
  orxCONFIG_ENTRY  *pstEntry;
  orxCHAR           zValue[64];
  orxSTATUS         eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);
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
  eResult = orxConfig_AddEntry(_zKey, zValue, orxFALSE);

  /* Done! */
  return eResult;
}

/** Is value a list for the given key?
 * @param[in] _zKey             Key name
 * @return orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxConfig_IsList(const orxSTRING _zKey)
{
  orxCONFIG_VALUE  *pstValue;
  orxBOOL           bResult = orxFALSE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);

  /* Gets associated value */
  pstValue = orxConfig_GetValue(_zKey);

  /* Valid? */
  if(pstValue != orxNULL)
  {
    /* Updates result */
    bResult = orxFLAG_TEST(pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_LIST) ? orxTRUE : orxFALSE;
  }

  /* Done! */
  return bResult;
}

/** Gets list counter for a given key
 * @param[in] _zKey             Key name
 * @return List counter if it's a valid list, 0 otherwise
 */
orxS32 orxFASTCALL orxConfig_GetListCounter(const orxSTRING _zKey)
{
  orxCONFIG_VALUE  *pstValue;
  orxS32            s32Result = 0;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);

  /* Gets associated value */
  pstValue = orxConfig_GetValue(_zKey);

  /* Valid? */
  if(pstValue != orxNULL)
  {
    /* Updates result */
    s32Result = (orxS32)pstValue->u8ListCounter;
  }

  /* Done! */
  return s32Result;
}

/** Reads a signed integer value from config list
 * @param[in] _zKey             Key name
 * @param[in] _s32ListIndex     Index of desired item in list / -1 for random
 * @return The value
 */
orxS32 orxFASTCALL orxConfig_GetListS32(const orxSTRING _zKey, orxS32 _s32ListIndex)
{
  orxCONFIG_VALUE  *pstValue;
  orxS32            s32Result = 0;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);
  orxASSERT(_s32ListIndex < 0xFF);

  /* Gets value */
  pstValue = orxConfig_GetValue(_zKey);

  /* Valid? */
  if(pstValue != orxNULL)
  {
    /* Is index valid? */
    if(_s32ListIndex < (orxS32)pstValue->u8ListCounter)
    {
      /* Updates result */
      s32Result = orxConfig_GetS32FromValue(pstValue, _s32ListIndex);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Failed to get S32 list item value <%s.%s>, index invalid: %ld out of %ld item(s).", _zKey, pstValue->zValue, _s32ListIndex, (orxS32)pstValue->u8ListCounter);
    }
  }

  /* Done! */
  return s32Result;
}

/** Reads an unsigned integer value from config list
 * @param[in] _zKey             Key name
 * @param[in] _s32ListIndex     Index of desired item in list / -1 for random
 * @return The value
 */
orxU32 orxFASTCALL orxConfig_GetListU32(const orxSTRING _zKey, orxS32 _s32ListIndex)
{
  orxCONFIG_VALUE  *pstValue;
  orxU32            u32Result = 0;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);
  orxASSERT(_s32ListIndex < 0xFF);

  /* Gets value */
  pstValue = orxConfig_GetValue(_zKey);

  /* Valid? */
  if(pstValue != orxNULL)
  {
    /* Is index valid? */
    if(_s32ListIndex < (orxS32)pstValue->u8ListCounter)
    {
      /* Updates result */
      u32Result = orxConfig_GetU32FromValue(pstValue, _s32ListIndex);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Failed to get U32 list item config value <%s.%s>, index invalid: %ld out of %ld item(s).", _zKey, pstValue->zValue, _s32ListIndex, (orxS32)pstValue->u8ListCounter);
    }
  }

  /* Done! */
  return u32Result;
}

/** Reads a float value from config list
 * @param[in] _zKey             Key name
 * @param[in] _s32ListIndex     Index of desired item in list / -1 for random
 * @return The value
 */
orxFLOAT orxFASTCALL orxConfig_GetListFloat(const orxSTRING _zKey, orxS32 _s32ListIndex)
{
  orxCONFIG_VALUE  *pstValue;
  orxFLOAT          fResult = orxFLOAT_0;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);
  orxASSERT(_s32ListIndex < 0xFF);

  /* Gets value */
  pstValue = orxConfig_GetValue(_zKey);

  /* Valid? */
  if(pstValue != orxNULL)
  {
    /* Is index valid? */
    if(_s32ListIndex < (orxS32)pstValue->u8ListCounter)
    {
      /* Updates result */
      fResult = orxConfig_GetFloatFromValue(pstValue, _s32ListIndex);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Failed to get FLOAT list item config value <%s.%s>, index invalid: %ld out of %ld item(s).", _zKey, pstValue->zValue, _s32ListIndex, (orxS32)pstValue->u8ListCounter);
    }
  }

  /* Done! */
  return fResult;
}

/** Reads a string value from config list
 * @param[in] _zKey             Key name
 * @param[in] _s32ListIndex     Index of desired item in list / -1 for random
 * @return The value
 */
const orxSTRING orxFASTCALL orxConfig_GetListString(const orxSTRING _zKey, orxS32 _s32ListIndex)
{
  orxCONFIG_VALUE  *pstValue;
  orxSTRING         zResult = orxSTRING_EMPTY;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);
  orxASSERT(_s32ListIndex < 0xFF);

  /* Gets value */
  pstValue = orxConfig_GetValue(_zKey);

  /* Valid? */
  if(pstValue != orxNULL)
  {
    /* Is index valid? */
    if(_s32ListIndex < (orxS32)pstValue->u8ListCounter)
    {
      /* Updates result */
      zResult = orxConfig_GetStringFromValue(pstValue, _s32ListIndex);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Failed to get STRING list item config value <%s>, index invalid: %ld out of %ld item(s).", _zKey, _s32ListIndex, (orxS32)pstValue->u8ListCounter);
    }
  }

  /* Done! */
  return zResult;
}

/** Reads a boolean value from config list
 * @param[in] _zKey             Key name
 * @param[in] _s32ListIndex     Index of desired item in list / -1 for random
 * @return The value
 */
orxBOOL orxFASTCALL orxConfig_GetListBool(const orxSTRING _zKey, orxS32 _s32ListIndex)
{
  orxCONFIG_VALUE  *pstValue;
  orxBOOL           bResult = orxFALSE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);
  orxASSERT(_s32ListIndex < 0xFF);

  /* Gets value */
  pstValue = orxConfig_GetValue(_zKey);

  /* Valid? */
  if(pstValue != orxNULL)
  {
    /* Is index valid? */
    if(_s32ListIndex < (orxS32)pstValue->u8ListCounter)
    {
      /* Updates result */
      bResult = orxConfig_GetBoolFromValue(pstValue, _s32ListIndex);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Failed to get BOOL list item config value <%s.%s>, index invalid: %ld out of %ld item(s).", _zKey, pstValue->zValue, _s32ListIndex, (orxS32)pstValue->u8ListCounter);
    }
  }

  /* Done! */
  return bResult;
}

/** Reads a vector value from config list
 * @param[in]   _zKey             Key name
 * @param[in]   _s32ListIndex     Index of desired item in list / -1 for random
 * @param[out]  _pvVector         Storage for vector value
 * @return The value
 */
orxVECTOR *orxFASTCALL orxConfig_GetListVector(const orxSTRING _zKey, orxS32 _s32ListIndex, orxVECTOR *_pvVector)
{
  orxCONFIG_VALUE  *pstValue;
  orxVECTOR        *pvResult = orxNULL;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);
  orxASSERT(_s32ListIndex < 0xFF);

  /* Gets value */
  pstValue = orxConfig_GetValue(_zKey);

  /* Valid? */
  if(pstValue != orxNULL)
  {
    /* Is index valid? */
    if(_s32ListIndex < (orxS32)pstValue->u8ListCounter)
    {
      /* Updates result */
      pvResult = orxConfig_GetVectorFromValue(pstValue, _s32ListIndex, _pvVector);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Failed to get U32 list item config value <%s.%s>, index invalid: %ld out of %ld item(s).", _zKey, pstValue->zValue, _s32ListIndex, (orxS32)pstValue->u8ListCounter);
    }
  }

  /* Done! */
  return pvResult;
}

/** Writes a list of string values to config
 * @param[in] _zKey             Key name
 * @param[in] _azValue          Values
 * @param[in] _u32Number        Number of values
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_SetStringList(const orxSTRING _zKey, const orxSTRING _azValue[], orxU32 _u32Number)
{
  orxCONFIG_ENTRY  *pstEntry;
  orxCHAR           acBuffer[orxCONFIG_KU32_BUFFER_SIZE];
  orxU32            u32Index, i;
  orxSTATUS         eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);
  orxASSERT(_azValue != orxNULL);

  /* Valid? */
  if(_u32Number > 0)
  {
    /* Gets entry */
    pstEntry = orxConfig_GetEntry(orxString_ToCRC(_zKey));

    /* Found? */
    if(pstEntry != orxNULL)
    {
      /* Deletes it */
      orxConfig_DeleteEntry(sstConfig.pstCurrentSection, pstEntry);
    }

    /* For all values */
    for(i = 0, u32Index = 0; i < _u32Number; i++)
    {
      orxCHAR *pc;

      /* For all characters */
      for(pc = _azValue[i]; *pc != orxCHAR_NULL; pc++)
      {
        /* Copies it */
        acBuffer[u32Index++] = *pc;
      }

      /* Adds separator */
      acBuffer[u32Index++] = orxCONFIG_KC_LIST_SEPARATOR;
    }

    /* Removes last separator */
    acBuffer[u32Index - 1] = orxCHAR_NULL;

    /* Adds new entry */
    eResult = orxConfig_AddEntry(_zKey, acBuffer, orxFALSE);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Cannot write config string list as no item is provided.");

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets key counter for the current section
 * @return Key counter the current section if valid, 0 otherwise
 */
orxS32 orxFASTCALL orxConfig_GetKeyCounter()
{
  orxS32 s32Result;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));

  /* Has current section? */
  if(sstConfig.pstCurrentSection != orxNULL)
  {
    /* Updates result */
    s32Result = orxLinkList_GetCounter(&(sstConfig.pstCurrentSection->stEntryList));
  }
  else
  {
    /* Updates result */
    s32Result = 0;
  }

  /* Done! */
  return s32Result;
}

/** Gets key for the current section at the given index
 * @param[in] _s32KeyIndex      Index of the desired key
 * @return orxSTRING if exist, orxSTRING_EMPTY otherwise
 */
const orxSTRING orxFASTCALL orxConfig_GetKey(orxS32 _s32KeyIndex)
{
  orxSTRING zResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));

  /* Valid? */
  if(_s32KeyIndex < orxConfig_GetKeyCounter())
  {
    orxCONFIG_ENTRY  *pstEntry;
    orxS32            i;

    /* Finds correct entry */
    for(i = _s32KeyIndex, pstEntry = (orxCONFIG_ENTRY *)orxLinkList_GetFirst(&(sstConfig.pstCurrentSection->stEntryList));
        i > 0;
        i--, pstEntry = (orxCONFIG_ENTRY *)orxLinkList_GetNext(&(pstEntry->stNode)));

    /* Updates result */
    zResult = pstEntry->zKey;
  }
  else
  {
    /* Updates result */
    zResult = orxSTRING_EMPTY;
  }
  
  /* Done! */
  return zResult;
}
