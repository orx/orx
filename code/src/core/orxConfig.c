/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2018 Orx-Project
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
 * @file orxConfig.c
 * @date 09/12/2007
 * @author iarwain@orx-project.org
 *
 */


#include "orxInclude.h"

#include "core/orxConfig.h"
#include "core/orxCommand.h"
#include "core/orxEvent.h"
#include "core/orxResource.h"
#include "debug/orxDebug.h"
#include "debug/orxProfiler.h"
#include "memory/orxBank.h"
#include "math/orxMath.h"
#include "io/orxFile.h"
#include "utils/orxLinkList.h"
#include "utils/orxHashTable.h"
#include "utils/orxString.h"

#ifdef __orxMSVC__

#include <direct.h>

#else /* __orxMSVC__ */

  #include <unistd.h>

#endif /* __orxMSVC__ */

#ifdef __orxGCC__

  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wpragmas"
  #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"

#endif /* __orxGCC__ */


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
#define orxCONFIG_VALUE_KU16_FLAG_SELF_VALUE      0x0010      /**< Self value flag */
#define orxCONFIG_VALUE_KU16_FLAG_COMMAND         0x0020      /**< Command flag */

#define orxCONFIG_VALUE_KU16_MASK_ALL             0xFFFF      /**< All mask */

/** Defines
 */
#define orxCONFIG_KU32_SECTION_BANK_SIZE          2048        /**< Default section bank size */
#define orxCONFIG_KU32_STACK_BANK_SIZE            32          /**< Default stack bank size */
#define orxCONFIG_KU32_ENTRY_BANK_SIZE            8192        /**< Default entry bank size */
#define orxCONFIG_KU32_HISTORY_BANK_SIZE          32          /**< Default history bank size */
#define orxCONFIG_KU32_BASE_FILENAME_LENGTH       256         /**< Base file name length */

#define orxCONFIG_KU32_BUFFER_SIZE                8192        /**< Buffer size */
#define orxCONFIG_KU32_LARGE_BUFFER_SIZE          524288      /**< Large buffer size */

#define orxCONFIG_KU32_COMMAND_BUFFER_SIZE        128         /**< Command buffer size */

#define orxCONFIG_KC_SECTION_START                '['         /**< Section start character */
#define orxCONFIG_KC_SECTION_END                  ']'         /**< Section end character */
#define orxCONFIG_KC_ASSIGN                       '='         /**< Assign character */
#define orxCONFIG_KC_APPEND                       '+'         /**< Append character */
#define orxCONFIG_KC_COMMENT                      ';'         /**< Comment character */
#define orxCONFIG_KC_COMMAND                      '%'         /**< Command character */
#define orxCONFIG_KC_RANDOM_SEPARATOR             '~'         /**< Random number separator character */
#define orxCONFIG_KC_LIST_SEPARATOR               '#'         /**< List separator */
#define orxCONFIG_KC_SECTION_SEPARATOR            '.'         /**< Section separator */
#define orxCONFIG_KC_INHERITANCE_MARKER           '@'         /**< Inheritance marker character */
#define orxCONFIG_KC_BLOCK                        '"'         /**< Block delimiter character */

#define orxCONFIG_KZ_CONFIG_SECTION               "Config"    /**< Config section name */
#define orxCONFIG_KZ_CONFIG_DEFAULT_PARENT        "DefaultParent" /**< Default parent for sections */

#define orxCONFIG_KZ_DEFAULT_ENCRYPTION_KEY       "Orx Default Encryption Key =)" /**< Orx default encryption key */
#define orxCONFIG_KZ_ENCRYPTION_TAG               "OECF"      /**< Encryption file tag */
#define orxCONFIG_KU32_ENCRYPTION_TAG_LENGTH      4           /**< Encryption file tag length */

#define orxCONFIG_KZ_UTF8_BOM                     "\xEF\xBB\xBF" /**< UTF-8 BOM */
#define orxCONFIG_KU32_UTF8_BOM_LENGTH            3           /**< UTF-8 BOM length */

#define orxCONFIG_KZ_CONFIG_MEMORY                "Memory"    /**< Memory buffer name */

#if defined(__orxDEBUG__)

  #define orxCONFIG_KZ_DEFAULT_FILE               "orxd.ini"  /**< Default config file name */

#elif defined(__orxPROFILER__)

  #define orxCONFIG_KZ_DEFAULT_FILE               "orxp.ini"  /**< Default config file name */

#else

  #define orxCONFIG_KZ_DEFAULT_FILE               "orx.ini"   /**< Default config file name */

#endif


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
  orxCONFIG_VALUE_TYPE_S64,
  orxCONFIG_VALUE_TYPE_U64,
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
  orxU16                u16Type;            /**< Value type : 6 */
  orxU16                u16Flags;           /**< Status flags : 8 */
  orxU16                u16ListCount;       /**< List count : 10 */
  orxU16                u16CacheIndex;      /**< Cache index : 12 */

  union
  {
    orxVECTOR           vValue;             /**< Vector value : 24 */
    orxU64              u64Value;           /**< U64 value : 20 */
    orxS64              s64Value;           /**< S64 value : 20 */
    orxU32              u32Value;           /**< U32 value : 16 */
    orxS32              s32Value;           /**< S32 value : 16 */
    orxFLOAT            fValue;             /**< Float value : 16 */
    orxBOOL             bValue;             /**< Bool value : 16 */
  };                                        /**< Union value : 24 */

  union
  {
    orxVECTOR           vAltValue;          /**< Alternate vector value : 36 */
    orxU64              u64AltValue;        /**< Alternate U64 value : 32 */
    orxS64              s64AltValue;        /**< Alternate S64 value : 32 */
    orxU32              u32AltValue;        /**< Alternate U32 value : 28 */
    orxS32              s32AltValue;        /**< Alternate S32 value : 28 */
    orxFLOAT            fAltValue;          /**< Alternate float value : 28 */
    orxBOOL             bAltValue;          /**< Alternate bool value : 28 */
  };                                        /**< Union value : 36 */

  orxU32               *au32ListIndexTable; /**< List index table : 40 */

} orxCONFIG_VALUE;

/** Config entry structure
 */
typedef struct __orxCONFIG_ENTRY_t
{
  orxLINKLIST_NODE  stNode;                 /**< List node : 12 */
  orxSTRINGID       stID;                   /**< Key ID : 16 */

  orxCONFIG_VALUE   stValue;                /**< Entry value : 52 */

  orxSTRINGID       stOriginID;             /**< Origin : 56 */

} orxCONFIG_ENTRY;

/** Config section structure
 */
typedef struct __orxCONFIG_SECTION_t
{
  orxLINKLIST_NODE  stNode;                 /**< List node : 12 */
  const orxSTRING   zName;                  /**< Name : 16 */
  struct __orxCONFIG_SECTION_t *pstParent;  /**< Parent section : 20 */
  orxLINKLIST       stEntryList;            /**< Entry list : 32 */
  orxS32            s32ProtectionCount;     /**< Protection count : 36 */
  orxSTRINGID       stOriginID;             /**< Origin : 40 */

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
  orxBANK            *pstEntryBank;         /**< Entry bank */
  orxCONFIG_SECTION  *pstCurrentSection;    /**< Current working section */
  orxBANK            *pstHistoryBank;       /**< History bank */
  orxBANK            *pstStackBank;         /**< Stack bank */
  orxLINKLIST         stStackList;          /**< Stack list */
  orxU32              u32Flags;             /**< Control flags */
  orxSTRINGID         stResourceGroupID;    /**< Resource group ID */
  orxU32              u32LoadCount;         /**< Load count */
  orxSTRING           zEncryptionKey;       /**< Encryption key */
  orxCONFIG_BOOTSTRAP_FUNCTION pfnBootstrap;/**< Bootstrap */
  orxSTRINGID         stLoadFileID;         /**< Loading file ID */
  orxU32              u32EncryptionKeySize; /**< Encryption key size */
  orxU32              u32BufferListSize;    /**< Buffer list size */
  orxCHAR            *pcEncryptionChar;     /**< Current encryption char */
  orxLINKLIST         stSectionList;        /**< Section list */
  orxHASHTABLE       *pstSectionTable;      /**< Section table */
  orxCONFIG_SECTION  *pstDefaultSection;    /**< Default parent section */
  orxCHAR             acCommandBuffer[orxCONFIG_KU32_COMMAND_BUFFER_SIZE]; /**< Command buffer */
  orxCHAR             zBaseFile[orxCONFIG_KU32_BASE_FILENAME_LENGTH]; /**< Base file name */
  orxCHAR             acValueBuffer[orxCONFIG_KU32_LARGE_BUFFER_SIZE]; /**< Value buffer */

} orxCONFIG_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** static data
 */
static orxCONFIG_STATIC sstConfig;


/** Unsupported BOMs
 */
static struct __orxCONFIG_BOM_DEFINITION_t
{
  const orxSTRING zBOM;
  orxU32          u32Length;
} sastUnsupportedBOMList[] =
{
  {"\xFE\xFF", 2},                          /**< UTF-16 BE */
  {"\xFF\xFE", 2},                          /**< UTF-16 LE */
  {"\x00\x00\xFE\xFF", 4},                  /**< UTF-32 BE */
  {"\xFF\xFE\x00\x00", 4},                  /**< UTF-32 LE */
  {"\x2B\x2F\x76\x38", 4},                  /**< UTF-7 #1 */
  {"\x2B\x2F\x76\x39", 4},                  /**< UTF-7 #2 */
  {"\x2B\x2F\x76\x2B", 4},                  /**< UTF-7 #3 */
  {"\x2B\x2F\x76\x2F", 4},                  /**< UTF-7 #4 */
  {"\xF7\x64\x4C", 3},                      /**< UTF-1 */
  {"\xDD\x73\x66\x73", 4},                  /**< UTF-EBCDIC */
  {"\x0E\xFE\xFF", 3},                      /**< SCSU */
  {"\xFB\xEE\x28", 3},                      /**< BOCU-1 */
  {"\x84\x31\x95\x33", 4},                  /**< GB-18030 */
};


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Cleans a value
 * @param[in] _pstValue         Concerned config value
 */
static orxINLINE void orxConfig_CleanValue(orxCONFIG_VALUE *_pstValue)
{
  /* Not in block mode? */
  if(!orxFLAG_TEST(_pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_BLOCK_MODE))
  {
    /* Is a list? */
    if(orxFLAG_TEST(_pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_LIST))
    {
      /* Deletes string */
      orxString_Delete(_pstValue->zValue);

      /* Deletes index table */
      orxMemory_Free(_pstValue->au32ListIndexTable);
      _pstValue->au32ListIndexTable = orxNULL;

      /* Cleans list status */
      _pstValue->u16Flags      &= ~orxCONFIG_VALUE_KU16_FLAG_LIST;
      _pstValue->u16ListCount   = 1;
      _pstValue->u16CacheIndex  = 0;
    }
  }

  /* Done! */
  return;
}

/** Deletes an entry
 * @param[in] _pstEntry         Entry to delete
 */
static orxINLINE void orxConfig_DeleteEntry(orxCONFIG_ENTRY *_pstEntry)
{
  /* Checks */
  orxASSERT(_pstEntry != orxNULL);

  /* Deletes value */
  orxConfig_CleanValue(&(_pstEntry->stValue));

  /* Removes it from list */
  orxLinkList_Remove(&(_pstEntry->stNode));

  /* Deletes the entry */
  orxBank_Free(sstConfig.pstEntryBank, _pstEntry);

  return;
}

/** Event handler
 */
static orxSTATUS orxFASTCALL orxConfig_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Add or update? */
  if((_pstEvent->eID == orxRESOURCE_EVENT_ADD) || (_pstEvent->eID == orxRESOURCE_EVENT_UPDATE))
  {
    orxRESOURCE_EVENT_PAYLOAD *pstPayload;

    /* Gets payload */
    pstPayload = (orxRESOURCE_EVENT_PAYLOAD *)_pstEvent->pstPayload;

    /* Is config group? */
    if(pstPayload->stGroupID == sstConfig.stResourceGroupID)
    {
      orxCONFIG_SECTION *pstSection;

      /* Finds correct entry */
      for(pstSection = (orxCONFIG_SECTION *)orxLinkList_GetFirst(&(sstConfig.stSectionList));
          pstSection != orxNULL;
          pstSection = (orxCONFIG_SECTION *)orxLinkList_GetNext(&(pstSection->stNode)))
      {
        orxCONFIG_ENTRY *pstEntry, *pstNextEntry;

        /* For all entries */
        for(pstEntry = (orxCONFIG_ENTRY *)orxLinkList_GetFirst(&(pstSection->stEntryList));
            pstEntry != orxNULL;
            pstEntry = pstNextEntry)
        {
          /* Gets next entry */
          pstNextEntry = (orxCONFIG_ENTRY *)orxLinkList_GetNext(&(pstEntry->stNode));

          /* Should get cleaned? */
          if(pstEntry->stOriginID == pstPayload->stNameID)
          {
            /* Deletes it */
            orxConfig_DeleteEntry(pstEntry);
          }
        }
      }

      /* Reloads file */
      orxConfig_Load(orxString_GetFromID(pstPayload->stNameID));
    }
  }

  /* Done! */
  return eResult;
}

/** Computes a working config value (process random, inheritance and list attributes)
 * @param[in] _pstValue         Concerned config value
 * @param[in] _u32Size          Size of contained list
 */
static void orxFASTCALL orxConfig_ComputeWorkingValue(orxCONFIG_VALUE *_pstValue, orxU32 _u32Size)
{
  orxU32 u32Count;
  orxU16 u16Flags;

  /* Checks */
  orxASSERT(_pstValue != orxNULL);

  /* Has inheritance? */
  if((*(_pstValue->zValue) == orxCONFIG_KC_INHERITANCE_MARKER)
  && (*(_pstValue->zValue + 1) != orxCONFIG_KC_INHERITANCE_MARKER))
  {
    /* Checks */
    orxASSERT(_pstValue->au32ListIndexTable == orxNULL);
    orxASSERT((_u32Size == orxU32_UNDEFINED) || (_u32Size <= 1));

    /* Self value? */
    if(*(_pstValue->zValue + 1) == orxCHAR_NULL)
    {
      /* Updates flags */
      u16Flags = orxCONFIG_VALUE_KU16_FLAG_SELF_VALUE;
    }
    else
    {
      /* Updates flags */
      u16Flags = orxCONFIG_VALUE_KU16_FLAG_INHERITANCE;
    }

    /* Sets count */
    u32Count = 1;
  }
  else
  {
    orxCHAR  *pc;
    orxU32    u32Index;

    /* Updates flags */
    u16Flags = ((*(_pstValue->zValue) == orxCONFIG_KC_COMMAND) && (*(_pstValue->zValue + 1) != orxCONFIG_KC_COMMAND)) ? orxCONFIG_VALUE_KU16_FLAG_COMMAND : orxCONFIG_VALUE_KU16_FLAG_NONE;

    /* Doesn't have an index table? */
    if(_pstValue->au32ListIndexTable == orxNULL)
    {
      /* Needs a table? */
      if((_u32Size != orxU32_UNDEFINED) && (_u32Size > 1))
      {
        /* Allocates it */
        _pstValue->au32ListIndexTable = (orxU32 *)orxMemory_Allocate((_u32Size - 1) * sizeof(orxU32), orxMEMORY_TYPE_CONFIG);

        /* Checks */
        orxASSERT(_pstValue->au32ListIndexTable != orxNULL);
      }
    }

    /* For all characters */
    for(pc = _pstValue->zValue, u32Count = 1, u32Index = 0; *pc != orxCHAR_NULL; pc++)
    {
      /* Is a list separator? */
      if(*pc == orxCONFIG_KC_LIST_SEPARATOR)
      {
        /* Not too long? */
        if(u32Count < 0xFFFF)
        {
          /* Is a command? */
          if((*(pc + 1) == orxCONFIG_KC_COMMAND) && (*(pc + 2) != orxCONFIG_KC_COMMAND))
          {
            /* Adds command flag */
            u16Flags |= orxCONFIG_VALUE_KU16_FLAG_COMMAND;
          }

          /* Sets an end of string here */
          *pc = orxCHAR_NULL;

          /* Updates list count */
          u32Count++;

          /* Checks */
          orxASSERT((_u32Size == orxU32_UNDEFINED) || (u32Count <= _u32Size));

          /* Stores index */
          _pstValue->au32ListIndexTable[u32Index++] = (orxU32)(pc + 1 - _pstValue->zValue);

          /* Updates flags */
          u16Flags |= orxCONFIG_VALUE_KU16_FLAG_LIST;
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "List for value <%s> is too long, more than 65535 values have been found.", _pstValue->zValue);
        }
      }
      else if(*pc == orxCONFIG_KC_RANDOM_SEPARATOR)
      {
        /* Updates flags */
        u16Flags |= orxCONFIG_VALUE_KU16_FLAG_RANDOM;
      }
    }
  }

  /* Updates value flags */
  _pstValue->u16Flags = u16Flags;

  /* Updates list count */
  _pstValue->u16ListCount = (orxU16)u32Count;
}

/** Restores a processed config value to its literal (for printing/saving/deleting purposes)
 * @param[in] _pstValue         Concerned config value
 */
static void orxFASTCALL orxConfig_RestoreLiteralValue(orxCONFIG_VALUE *_pstValue)
{
  orxCHAR  *pc;
  orxU16    u16Count;

  /* Checks */
  orxASSERT(_pstValue != orxNULL);

  /* For all characters */
  for(u16Count = _pstValue->u16ListCount - 1, pc = _pstValue->zValue; u16Count > 0; pc++)
  {
    /* Null character? */
    if(*pc == orxCHAR_NULL)
    {
      /* Updates it */
      *pc = orxCONFIG_KC_LIST_SEPARATOR;

      /* Updates count */
      u16Count--;
    }
  }

  /* Cleans list status */
  _pstValue->u16Flags      &= ~orxCONFIG_VALUE_KU16_FLAG_LIST;
  _pstValue->u16ListCount = 1;
  _pstValue->u16CacheIndex  = 0;
}

/** Initializes a value
 * @param[in] _pstValue         Concerned config value
 * @param[in] _zValue           Value to use for initialization
 * @param[in] _bBlockMode       In block mode?
 * @return                      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS orxConfig_InitValue(orxCONFIG_VALUE *_pstValue, const orxSTRING _zValue, orxBOOL _bBlockMode)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(orxString_GetLength(_zValue) < orxCONFIG_KU32_LARGE_BUFFER_SIZE);

  /* Clears index table */
  _pstValue->au32ListIndexTable = orxNULL;

  /* Not in block mode? */
  if(_bBlockMode == orxFALSE)
  {
    orxU32  u32Size;
    orxBOOL bNeedDuplication = orxFALSE;

    /* Buffer not already prepared? */
    if(sstConfig.acValueBuffer[0] == orxCHAR_NULL)
    {
      orxCHAR        *pcOutput;
      const orxCHAR  *pcInput;

      /* For all characters */
      for(pcInput = _zValue, pcOutput = sstConfig.acValueBuffer, u32Size = 1; *pcInput != orxCHAR_NULL;)
      {
        /* Not a space? */
        if((*pcInput != ' ') && (*pcInput != '\t'))
        {
          /* Copies it */
          *pcOutput++ = *pcInput++;

          /* Is a list separator? */
          if(*(pcInput - 1) == orxCONFIG_KC_LIST_SEPARATOR)
          {
            /* Asks for duplication */
            bNeedDuplication = orxTRUE;

            /* Updates size */
            u32Size++;

            /* Skips all trailing and leading spaces */
            while((*pcInput == ' ') || (*pcInput == '\t'))
            {
              pcInput++;
            }
          }
        }
        else
        {
          const orxCHAR *pcTest;

          /* Scans all the spaces */
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
            for(; pcInput < pcTest; pcInput++, pcOutput++)
            {
              /* Copies it */
              *pcOutput = *pcInput;
            }
          }
        }
      }

      /* Ends string */
      *pcOutput = orxCHAR_NULL;
    }
    else
    {
      /* Needs duplication */
      bNeedDuplication = orxTRUE;

      /* Gets size */
      u32Size = sstConfig.u32BufferListSize;
    }

    /* Needs duplication? */
    if(bNeedDuplication != orxFALSE)
    {
      /* Duplicates string */
      _pstValue->zValue = orxString_Duplicate(sstConfig.acValueBuffer);

      /* Valid? */
      if(_pstValue->zValue != orxNULL)
      {
        /* Computes working value */
        orxConfig_ComputeWorkingValue(_pstValue, u32Size);
      }
      else
      {
        /* Updates result */
        eResult = orxSTATUS_FAILURE;
      }
    }
    else
    {
      /* Stores value */
      _pstValue->zValue = (*sstConfig.acValueBuffer != orxCHAR_NULL) ? (orxSTRING)orxString_Store(sstConfig.acValueBuffer) : (orxSTRING)orxSTRING_EMPTY;

      /* Computes working value */
      orxConfig_ComputeWorkingValue(_pstValue, 1);
    }
  }
  else
  {
    orxCHAR        *pcOutput;
    const orxCHAR  *pcInput;

    /* For all characters */
    for(pcInput = _zValue, pcOutput = sstConfig.acValueBuffer; *pcInput != orxCHAR_NULL; pcInput++, pcOutput++)
    {
      /* Copies it */
      *pcOutput = *pcInput;

      /* First block character of two? */
      if((*pcInput == orxCONFIG_KC_BLOCK) && (*(pcInput + 1) == orxCONFIG_KC_BLOCK))
      {
        /* Skips it */
        pcInput++;
      }
    }

    /* Ends string */
    *pcOutput = orxCHAR_NULL;

    /* Stores value */
    _pstValue->zValue = (orxSTRING)orxString_Store(sstConfig.acValueBuffer);

    /* Block mode, no list nor random allowed */
    _pstValue->u16Flags       = orxCONFIG_VALUE_KU16_FLAG_BLOCK_MODE;
    _pstValue->u16ListCount = 1;
  }

  /* Clears value buffer */
  sstConfig.u32BufferListSize = 0;
  sstConfig.acValueBuffer[0]  = orxCHAR_NULL;

  /* Done! */
  return eResult;
}

/** Appends a value to an existing one
 * @param[in] _pstValue         Concerned config value
 * @param[in] _zValue           Value to append
 * @return                      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxSTATUS orxFASTCALL orxConfig_AppendValue(orxCONFIG_VALUE *_pstValue, const orxSTRING _zValue)
{
  orxCHAR  *pcOutput;
  orxU32   *pu32Index;
  orxU32    u32Size, u32BufferSize, i;
  orxU32    au32IndexTable[orxCONFIG_KU32_BUFFER_SIZE / sizeof(orxU32)];
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(orxString_GetLength(_zValue) < orxCONFIG_KU32_LARGE_BUFFER_SIZE);
  orxASSERT(!orxFLAG_TEST(_pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_SELF_VALUE | orxCONFIG_VALUE_KU16_FLAG_INHERITANCE));

  /* Buffer not already prepared? */
  if(sstConfig.acValueBuffer[0] == orxCHAR_NULL)
  {
    const orxCHAR *pcInput;

    /* For all characters */
    for(pcInput = _zValue, pcOutput = sstConfig.acValueBuffer, u32Size = 1, au32IndexTable[0] = 0;
        (*pcInput != orxCHAR_NULL) && (u32Size < orxARRAY_GET_ITEM_COUNT(au32IndexTable));
       )
    {
      /* Not a space? */
      if((*pcInput != ' ') && (*pcInput != '\t'))
      {
        /* Is a list separator? */
        if(*pcInput == orxCONFIG_KC_LIST_SEPARATOR)
        {
          /* Ends string */
          *pcOutput++ = orxCHAR_NULL;

          /* Stores index */
          au32IndexTable[u32Size] = (orxU32)(pcOutput - sstConfig.acValueBuffer);

          /* Updates size */
          u32Size++;

          /* Skips all trailing and leading spaces */
          do
          {
            pcInput++;
          } while((*pcInput == ' ') || (*pcInput == '\t'));
        }
        else
        {
          /* Copies it */
          *pcOutput++ = *pcInput++;
        }
      }
      else
      {
        const orxCHAR *pcTest;

        /* Scans all the spaces */
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
          for(; pcInput < pcTest; pcInput++, pcOutput++)
          {
            /* Copies it */
            *pcOutput = *pcInput;
          }
        }
      }
    }

    /* Ends string */
    *pcOutput++ = orxCHAR_NULL;
  }
  else
  {
    /* For all characters */
    for(pcOutput = sstConfig.acValueBuffer, u32Size = 1, au32IndexTable[0] = 0;
        (*pcOutput != orxCHAR_NULL) && (u32Size < orxARRAY_GET_ITEM_COUNT(au32IndexTable));
        pcOutput++)
    {
      /* Is a list separator? */
      if(*pcOutput == orxCONFIG_KC_LIST_SEPARATOR)
      {
        /* Ends string */
        *pcOutput = orxCHAR_NULL;

        /* Stores index */
        au32IndexTable[u32Size] = (orxU32)(pcOutput + 1 - sstConfig.acValueBuffer);

        /* Updates size */
        u32Size++;
      }
    }

    /* Updates pointer */
    pcOutput++;
  }

  /* Didn't finish? */
  if(u32Size == orxARRAY_GET_ITEM_COUNT(au32IndexTable))
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Couldn't append all config list items from [%s]: only the first %u items were appended.", _zValue, orxARRAY_GET_ITEM_COUNT(au32IndexTable));
  }

  /* Is current value a list? */
  if(orxFLAG_TEST(_pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_LIST))
  {
    /* Reallocates index table */
    _pstValue->au32ListIndexTable = (orxU32 *)orxMemory_Reallocate(_pstValue->au32ListIndexTable, (u32Size + (orxU32)_pstValue->u16ListCount - 1) * sizeof(orxU32));

    /* Checks */
    orxASSERT(_pstValue->au32ListIndexTable != orxNULL);

    /* Computes buffer size */
    u32BufferSize = _pstValue->au32ListIndexTable[_pstValue->u16ListCount - 2] + orxString_GetLength(_pstValue->zValue + _pstValue->au32ListIndexTable[_pstValue->u16ListCount - 2]) + 1;

    /* Reallocates buffer */
    _pstValue->zValue = (orxSTRING)orxMemory_Reallocate(_pstValue->zValue, u32BufferSize + (orxU32)(pcOutput - sstConfig.acValueBuffer));

    /* Checks */
    orxASSERT(_pstValue->zValue != orxNULL);
  }
  else
  {
    orxSTRING zNewValue;

    /* Allocates index table */
    _pstValue->au32ListIndexTable = (orxU32 *)orxMemory_Allocate(u32Size * sizeof(orxU32), orxMEMORY_TYPE_CONFIG);

    /* Computes buffer size */
    u32BufferSize = orxString_GetLength(_pstValue->zValue) + 1;

    /* Allocates new buffer */
    zNewValue = (orxSTRING)orxMemory_Allocate(u32BufferSize + (orxU32)(pcOutput - sstConfig.acValueBuffer), orxMEMORY_TYPE_TEXT);

    /* Checks */
    orxASSERT(_pstValue->au32ListIndexTable != orxNULL);
    orxASSERT(_pstValue->zValue != orxNULL);

    /* Copies original content */
    orxMemory_Copy(zNewValue, _pstValue->zValue, u32BufferSize);

    /* Stores it */
    _pstValue->zValue = zNewValue;
  }

  /* Copies new content */
  orxMemory_Copy(_pstValue->zValue + u32BufferSize, sstConfig.acValueBuffer, (orxU32)(pcOutput - sstConfig.acValueBuffer));

  /* For all new indices */
  for(pu32Index = _pstValue->au32ListIndexTable + _pstValue->u16ListCount - 1, i = 0;
      i < u32Size;
      i++, pu32Index++)
  {
    /* Copies it */
    *pu32Index = au32IndexTable[i] + u32BufferSize;
  }

  /* Updates value */
  _pstValue->u16ListCount  += (orxU16)u32Size;
  _pstValue->u16Flags      |= orxCONFIG_VALUE_KU16_FLAG_LIST;

  /* Clears value buffer */
  sstConfig.u32BufferListSize = 0;
  sstConfig.acValueBuffer[0]  = orxCHAR_NULL;

  /* Done! */
  return eResult;
}

/** Gets a list value
 * @param[in] _pstValue         Concerned config value
 * @param[in] _s32Index         Index of the desired value
 * @param[in] _bPermanentStore  Store the result permanently if it's a string from a command
 */
static orxINLINE const orxSTRING orxConfig_GetListValue(orxCONFIG_VALUE *_pstValue, orxS32 _s32Index, orxBOOL _bPermanentStore)
{
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(_pstValue != orxNULL);
  orxASSERT(_s32Index >= 0);

  /* Is self value? */
  if(orxFLAG_TEST(_pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_SELF_VALUE))
  {
    /* Updates result */
    zResult = sstConfig.pstCurrentSection->zName;
  }
  else
  {
    /* Updates result */
    zResult = (_s32Index == 0) ? _pstValue->zValue : _pstValue->zValue + _pstValue->au32ListIndexTable[_s32Index - 1];

    /* Command character? */
    if(*zResult == orxCONFIG_KC_COMMAND)
    {
      /* Real command? */
      if(*(zResult + 1) != orxCONFIG_KC_COMMAND)
      {
        orxCOMMAND_VAR stResult;

        /* Evaluates command */
        if(orxCommand_Evaluate(zResult + 1, &stResult) != orxNULL)
        {
          /* String? */
          if(stResult.eType == orxCOMMAND_VAR_TYPE_STRING)
          {
            /* Updates result */
            zResult = stResult.zValue;
          }
          else
          {
            /* Prints result */
            orxCommand_PrintVar(sstConfig.acCommandBuffer, sizeof(sstConfig.acCommandBuffer), &stResult);

            /* Updates result */
            zResult = (_bPermanentStore != orxFALSE) ? orxString_Store(sstConfig.acCommandBuffer) : sstConfig.acCommandBuffer;
          }
        }
      }
      else
      {
        /* Updates result */
        zResult++;
      }
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
  orxCHAR *pc, *pcKey, *pcEndKey;

  orxASSERT(sstConfig.zEncryptionKey != orxNULL);

  /* Gets current, first and last encryption character */
  pcKey     = sstConfig.pcEncryptionChar;
  pcEndKey  = sstConfig.zEncryptionKey + sstConfig.u32EncryptionKeySize - 1;

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
 * @param[in] _stKeyID          Entry key ID
 * @return                      orxCONFIG_ENTRY / orxNULL
 */
static orxINLINE orxCONFIG_ENTRY *orxConfig_GetEntry(orxSTRINGID _stKeyID)
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
    if(_stKeyID == pstEntry->stID)
    {
      /* Updates result */
      pstResult = pstEntry;

      break;
    }
  }

  /* Done! */
  return pstResult;
}

/** Gets an entry from the current section, using inheritance
 * @param[in] _stKeyID          Entry key ID
 * @return                      orxCONFIG_ENTRY / orxNULL
 */
static orxCONFIG_ENTRY *orxFASTCALL orxConfig_GetEntryFromKey(orxSTRINGID _stKeyID)
{
  orxCONFIG_ENTRY *pstResult = orxNULL;

  /* Checks */
  orxASSERT(sstConfig.pstCurrentSection != orxNULL);

  /* Gets corresponding entry */
  pstResult = orxConfig_GetEntry(_stKeyID);

  /* Not found? */
  if(pstResult == orxNULL)
  {
    orxCONFIG_SECTION *pstParent;

    /* Has parent? */
    if(sstConfig.pstCurrentSection->pstParent != 0)
    {
      /* Selects it */
      pstParent = sstConfig.pstCurrentSection->pstParent;
    }
    else
    {
      /* Isn't the default parent? */
      if(sstConfig.pstCurrentSection != sstConfig.pstDefaultSection)
      {
        /* Selects default parent */
        pstParent = sstConfig.pstDefaultSection;
      }
      else
      {
        /* No parent */
        pstParent = orxNULL;
      }
    }

    /* Valid parent ID */
    if((pstParent != orxNULL) && (pstParent != orxHANDLE_UNDEFINED))
    {
      orxCONFIG_SECTION *pstPreviousSection;

      /* Backups current section */
      pstPreviousSection = sstConfig.pstCurrentSection;

      /* Sets parent as current section */
      sstConfig.pstCurrentSection = pstParent;

      /* Gets inherited value */
      pstResult = orxConfig_GetEntryFromKey(_stKeyID);

      /* Restores current section */
      sstConfig.pstCurrentSection = pstPreviousSection;
    }
  }

  /* Done! */
  return pstResult;
}

/** Gets a value from the current section, using inheritance
 * @param[in] _stKeyID          Entry key ID
 * @param[in] _pstOrigin        Origin section for in-section forwarding
 * @param[out] _ppstSource      Source section where the value was found
 * @return                      orxCONFIG_VALUE / orxNULL
 */
static orxCONFIG_VALUE *orxFASTCALL orxConfig_GetValueFromKey(orxSTRINGID _stKeyID, orxCONFIG_SECTION *_pstOrigin, orxCONFIG_SECTION **_ppstSource)
{
  orxCONFIG_ENTRY  *pstEntry;
  orxCONFIG_VALUE  *pstResult = orxNULL;

  /* Checks */
  orxASSERT(sstConfig.pstCurrentSection != orxNULL);

  /* Gets corresponding entry */
  pstEntry = orxConfig_GetEntry(_stKeyID);

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
        orxSTRINGID stNewKeyID;

        /* Gets new key */
        stNewKeyID = orxString_ToCRC(pstEntry->stValue.zValue + s32SeparatorIndex + 1);

        /* Same section? */
        if(s32SeparatorIndex == 1)
        {
          /* Checks */
          orxASSERT(stNewKeyID != _stKeyID);

          /* Is origin different from current? */
          if(_pstOrigin != sstConfig.pstCurrentSection)
          {
            /* Restores it */
            sstConfig.pstCurrentSection = _pstOrigin;
          }

          /* Gets its inherited value */
          pstResult = orxConfig_GetValueFromKey(stNewKeyID, _pstOrigin, _ppstSource);
        }
        else
        {
          /* Cuts the name */
          *(pstEntry->stValue.zValue + s32SeparatorIndex) = orxCHAR_NULL;

          /* Checks */
          orxASSERT((stNewKeyID != _stKeyID) || (orxString_ToCRC(pstEntry->stValue.zValue + 1) != orxString_ToCRC(pstPreviousSection->zName)));

          /* Selects parent section */
          orxConfig_SelectSection(pstEntry->stValue.zValue + 1);

          /* Gets its inherited value */
          pstResult = orxConfig_GetValueFromKey(stNewKeyID, _pstOrigin, _ppstSource);

          /* Restores the name */
          *(pstEntry->stValue.zValue + s32SeparatorIndex) = orxCONFIG_KC_SECTION_SEPARATOR;
        }
      }
      else
      {
        /* Checks */
        orxASSERT(orxString_ToCRC(pstEntry->stValue.zValue + 1) != orxString_ToCRC(pstPreviousSection->zName));

        /* Selects parent section */
        orxConfig_SelectSection(pstEntry->stValue.zValue + 1);

        /* Gets its inherited value */
        pstResult = orxConfig_GetValueFromKey(_stKeyID, _pstOrigin, _ppstSource);
      }

      /* Restores current section */
      sstConfig.pstCurrentSection = pstPreviousSection;
    }
    else
    {
      /* Updates result */
      pstResult = &(pstEntry->stValue);

      /* Stores source section */
      *_ppstSource = sstConfig.pstCurrentSection;
    }
  }
  else
  {
    orxCONFIG_SECTION *pstParent;

    /* Has parent? */
    if(sstConfig.pstCurrentSection->pstParent != 0)
    {
      /* Selects it */
      pstParent = sstConfig.pstCurrentSection->pstParent;
    }
    else
    {
      /* Isn't the default parent? */
      if(sstConfig.pstCurrentSection != sstConfig.pstDefaultSection)
      {
        /* Selects default parent */
        pstParent = sstConfig.pstDefaultSection;
      }
      else
      {
        /* No parent */
        pstParent = orxNULL;
      }
    }

    /* Valid parent ID */
    if((pstParent != orxNULL) && (pstParent != orxHANDLE_UNDEFINED))
    {
      orxCONFIG_SECTION *pstPreviousSection;

      /* Backups current section */
      pstPreviousSection = sstConfig.pstCurrentSection;

      /* Sets parent as current section */
      sstConfig.pstCurrentSection = pstParent;

      /* Gets inherited value */
      pstResult = orxConfig_GetValueFromKey(_stKeyID, _pstOrigin, _ppstSource);

      /* Restores current section */
      sstConfig.pstCurrentSection = pstPreviousSection;
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

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxConfig_GetValue");

  /* Checks */
  orxASSERT(sstConfig.pstCurrentSection != orxNULL);

  /* Valid? */
  if((_zKey != orxSTRING_EMPTY) && (_zKey != orxNULL))
  {
    orxCONFIG_SECTION  *pstDummy = orxNULL;
    orxSTRINGID         stID;

    /* Gets its ID */
    stID = orxString_ToCRC(_zKey);

    /* Gets value */
    pstResult = orxConfig_GetValueFromKey(stID, sstConfig.pstCurrentSection, &pstDummy);

#ifdef __orxDEBUG__

    /* Not found? */
    if(pstResult == orxNULL)
    {
      orxCONFIG_ENTRY *pstEntry;

      /* For all entries in section */
      for(pstEntry = (orxCONFIG_ENTRY *)orxLinkList_GetFirst(&(sstConfig.pstCurrentSection->stEntryList));
          pstEntry != orxNULL;
          pstEntry = (orxCONFIG_ENTRY *)orxLinkList_GetNext(&(pstEntry->stNode)))
      {
        /* Identical? */
        if(pstEntry->stID == stID)
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, orxANSI_KZ_COLOR_FG_GREEN "[%s]" orxANSI_KZ_COLOR_FG_DEFAULT ": " orxANSI_KZ_COLOR_FG_YELLOW "<%s>" orxANSI_KZ_COLOR_FG_DEFAULT " inherits from " orxANSI_KZ_COLOR_FG_YELLOW "<%s> " orxANSI_KZ_COLOR_FG_DEFAULT orxANSI_KZ_COLOR_UNDERLINE_ON "however one of its ancestors was not found" orxANSI_KZ_COLOR_UNDERLINE_OFF ", " orxANSI_KZ_COLOR_BLINK_ON "typo" orxANSI_KZ_COLOR_BLINK_OFF "?", sstConfig.pstCurrentSection->zName, _zKey, pstEntry->stValue.zValue);

          break;
        }
        else
        {
          const orxSTRING zKey;

          /* Gets its key */
          zKey = orxString_GetFromID(pstEntry->stID);

          /* Case-only difference? */
          if(orxString_ICompare(zKey, _zKey) == 0)
          {
            /* Logs message */
            orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, orxANSI_KZ_COLOR_FG_GREEN "[%s]" orxANSI_KZ_COLOR_FG_DEFAULT ": " orxANSI_KZ_COLOR_FG_YELLOW "<%s>" orxANSI_KZ_COLOR_FG_DEFAULT orxANSI_KZ_COLOR_UNDERLINE_ON " was found instead of requested key" orxANSI_KZ_COLOR_FG_YELLOW orxANSI_KZ_COLOR_UNDERLINE_OFF " <%s>" orxANSI_KZ_COLOR_FG_DEFAULT ", " orxANSI_KZ_COLOR_BLINK_ON "typo" orxANSI_KZ_COLOR_BLINK_OFF "?", sstConfig.pstCurrentSection->zName, zKey, _zKey);

            break;
          }
        }
      }
    }

#endif /* __orxDEBUG__ */

  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return pstResult;
}

/** Sets an entry in the current section (adds it if need be)
 * @param[in] _zKey             Entry key
 * @param[in] _zValue           Entry value
 * @param[in] _bBlockMode       Block mode (ie. ignore special characters)?
 * @param[in] _bAppend          Append to list
 * @return                      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS orxConfig_SetEntry(const orxSTRING _zKey, const orxSTRING _zValue, orxBOOL _bBlockMode, orxBOOL _bAppend)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxConfig_SetEntry");

  /* Checks */
  orxASSERT(sstConfig.pstCurrentSection != orxNULL);
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zValue != orxNULL);
  orxASSERT((_bBlockMode && _bAppend) == orxFALSE);

  /* Valid? */
  if(_zKey != orxSTRING_EMPTY)
  {
    orxCONFIG_ENTRY  *pstEntry;
    orxSTRINGID       stKeyID;
    orxBOOL           bReuse;

    /* Gets key ID */
    stKeyID = orxString_GetID(_zKey);

    /* Gets existing entry */
    pstEntry = orxConfig_GetEntry(stKeyID);

    /* Found? */
    if(pstEntry != orxNULL)
    {
      /* Not appending? */
      if(_bAppend == orxFALSE)
      {
        /* Deletes value */
        orxConfig_CleanValue(&(pstEntry->stValue));
      }

      /* Updates status */
      bReuse = orxTRUE;
    }
    else
    {
      /* Creates entry */
      pstEntry = (orxCONFIG_ENTRY *)orxBank_Allocate(sstConfig.pstEntryBank);

      /* Updates status */
      bReuse = orxFALSE;
    }

    /* Valid? */
    if(pstEntry != orxNULL)
    {
      /* Really appending? */
      if((bReuse != orxFALSE) && (_bAppend != orxFALSE))
      {
        /* Not self or inheritance? */
        if(!orxFLAG_TEST(pstEntry->stValue.u16Flags, orxCONFIG_VALUE_KU16_FLAG_SELF_VALUE | orxCONFIG_VALUE_KU16_FLAG_INHERITANCE))
        {
          /* Appends value */
          eResult = orxConfig_AppendValue(&(pstEntry->stValue), _zValue);
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Failed to append [%s] to config entry <%s.%s>: can't append list item(s) to a value that contains an inheritance marker <%s>.", _zValue, sstConfig.pstCurrentSection->zName, _zKey, pstEntry->stValue.zValue);

          /* Updates result */
          eResult = orxSTATUS_FAILURE;
        }
      }
      else
      {
        /* Inits value */
        eResult = orxConfig_InitValue(&(pstEntry->stValue), _zValue, _bBlockMode);

        /* Stores origin */
        pstEntry->stOriginID = sstConfig.stLoadFileID;
      }

      /* Success? */
      if(eResult != orxSTATUS_FAILURE)
      {
        /* Not reusing entry? */
        if(bReuse == orxFALSE)
        {
          /* Adds it to list */
          orxMemory_Zero(&(pstEntry->stNode), sizeof(orxLINKLIST_NODE));
          orxLinkList_AddEnd(&(sstConfig.pstCurrentSection->stEntryList), &(pstEntry->stNode));

          /* Sets its ID */
          pstEntry->stID = stKeyID;
        }

        /* Inits its type */
        pstEntry->stValue.u16Type = (orxU16)orxCONFIG_VALUE_TYPE_STRING;
      }
      else
      {
        /* Not appending? */
        if(_bAppend == orxFALSE)
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Failed to set value [%s] on config entry <%s.%s>.", _zValue, sstConfig.pstCurrentSection->zName, _zKey);

          /* Reusing entry? */
          if(bReuse != orxFALSE)
          {
            /* Removes it from list */
            orxLinkList_Remove(&(pstEntry->stNode));
          }

          /* Deletes entry */
          orxBank_Free(sstConfig.pstEntryBank, pstEntry);
        }
      }
    }
  }

  /* Pops marker */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return eResult;
}

/** Creates a section
 * @param[in] _zSectionName     Name of the section to create
 * @param[in] _pstParent        Parent section
 */
static orxINLINE orxCONFIG_SECTION *orxConfig_CreateSection(const orxSTRING _zSectionName, orxCONFIG_SECTION *_pstParent)
{
  orxCONFIG_SECTION *pstSection;

  /* Allocates it */
  pstSection = (orxCONFIG_SECTION *)orxBank_Allocate(sstConfig.pstSectionBank);

  /* Valid? */
  if(pstSection != orxNULL)
  {
    /* Creates origin */
    pstSection->stOriginID = sstConfig.stLoadFileID;

    /* Clears its entry list */
    orxMemory_Zero(&(pstSection->stEntryList), sizeof(orxLINKLIST));

    /* Adds it to list */
    orxMemory_Zero(&(pstSection->stNode), sizeof(orxLINKLIST_NODE));
    orxLinkList_AddEnd(&(sstConfig.stSectionList), &(pstSection->stNode));

    /* Adds it to table */
    orxHashTable_Add(sstConfig.pstSectionTable, orxString_ToCRC(_zSectionName), pstSection);

    /* Stores its name */
    pstSection->zName = _zSectionName;

    /* Stores its parent */
    pstSection->pstParent = _pstParent;

    /* Clears its protection count */
    pstSection->s32ProtectionCount = 0;
  }

  /* Done! */
  return pstSection;
}

/** Deletes a section
 * @param[in] _pstSection       Section to delete
 */
static orxINLINE void orxConfig_DeleteSection(orxCONFIG_SECTION *_pstSection)
{
  orxCONFIG_ENTRY *pstEntry;

  /* Checks */
  orxASSERT(_pstSection != orxNULL);

  /* While there is still an entry */
  while((pstEntry = (orxCONFIG_ENTRY *)orxLinkList_GetFirst(&(_pstSection->stEntryList))) != orxNULL)
  {
    /* Deletes entry */
    orxConfig_DeleteEntry(pstEntry);
  }

  /* Not protected? */
  if(_pstSection->s32ProtectionCount == 0)
  {
    orxCONFIG_STACK_ENTRY *pstStackEntry, *pstNextStackEntry;

    /* For all stack entries */
    for(pstStackEntry = (orxCONFIG_STACK_ENTRY *)orxLinkList_GetFirst(&(sstConfig.stStackList));
        pstStackEntry != orxNULL;
        pstStackEntry = pstNextStackEntry)
    {
      /* Gets next entry */
      pstNextStackEntry = (orxCONFIG_STACK_ENTRY *)orxLinkList_GetNext(&(pstStackEntry->stNode));

      /* Is deleted section? */
      if(pstStackEntry->pstSection == _pstSection)
      {
        /* Removes it from stack list */
        orxLinkList_Remove(&(pstStackEntry->stNode));

        /* Deletes it */
        orxBank_Free(sstConfig.pstStackBank, pstStackEntry);

        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Warning: deleted section [%s] was previously pushed and has to be removed from stack.", _pstSection->zName);
      }
    }

    /* Is the current selected one? */
    if(sstConfig.pstCurrentSection == _pstSection)
    {
      /* Deselects it */
      sstConfig.pstCurrentSection = orxNULL;
    }

    /* Has parent? */
    if((_pstSection->pstParent != orxNULL) && (_pstSection->pstParent != orxHANDLE_UNDEFINED))
    {
      /* Unprotects it */
      _pstSection->pstParent->s32ProtectionCount--;

      /* Checks */
      orxASSERT(_pstSection->pstParent->s32ProtectionCount >= 0);
    }

    /* Removes it from list */
    orxLinkList_Remove(&(_pstSection->stNode));

    /* Removes it from table */
    orxHashTable_Remove(sstConfig.pstSectionTable, orxString_ToCRC(_pstSection->zName));

    /* Removes section */
    orxBank_Free(sstConfig.pstSectionBank, _pstSection);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Warning: section [%s] can't be deleted as it's protected by %d entities.", _pstSection->zName, _pstSection->s32ProtectionCount);
  }

  return;
}

/** Reads a signed integer value from config value
 * @param[in]   _pstValue         Concerned config value
 * @param[in]   _s32ListIndex     List index
 * @param[out]  _ps32Result       Result value
 * @return orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS orxConfig_GetS32FromValue(orxCONFIG_VALUE *_pstValue, orxS32 _s32ListIndex, orxS32 *_ps32Result)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

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
      _s32ListIndex = (orxS32)orxMath_GetRandomU32(0, (orxU32)_pstValue->u16ListCount - 1);
    }
  }

  /* Is it cached and on the same index? */
  if((_pstValue->u16Type == (orxU16)orxCONFIG_VALUE_TYPE_S32) && (_s32ListIndex == (orxS32)_pstValue->u16CacheIndex))
  {
    /* Random? */
    if(orxFLAG_TEST(_pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_RANDOM))
    {
      /* Updates result */
      *_ps32Result = orxMath_GetRandomS32(_pstValue->s32Value, _pstValue->s32AltValue);
    }
    else
    {
      /* Updates result */
      *_ps32Result = _pstValue->s32Value;
    }
  }
  else
  {
    orxS32          s32Value;
    const orxSTRING zRemainder;
    const orxSTRING zStart;

    /* Gets wanted value */
    zStart = orxConfig_GetListValue(_pstValue, _s32ListIndex, orxFALSE);

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
          _pstValue->u16Type        = (orxU16)orxCONFIG_VALUE_TYPE_S32;
          _pstValue->u16CacheIndex  = (orxU16)_s32ListIndex;
          _pstValue->s32Value       = s32Value;
          _pstValue->s32AltValue    = s32OtherValue;

          /* Updates result */
          *_ps32Result = orxMath_GetRandomS32(s32Value, s32OtherValue);
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Failed to get S32 random from config value <%s>.", zStart);

          /* Clears cache */
          _pstValue->u16Type = (orxU16)orxCONFIG_VALUE_TYPE_STRING;

          /* Updates result */
          *_ps32Result = s32Value;
        }
      }
      else
      {
        /* Updates cache */
        _pstValue->u16Type        = (orxU16)orxCONFIG_VALUE_TYPE_S32;
        _pstValue->u16CacheIndex  = (orxU16)_s32ListIndex;
        _pstValue->s32Value       = s32Value;
        _pstValue->s32AltValue    = s32Value;

        /* Updates result */
        *_ps32Result = s32Value;
      }

      /* Command? */
      if(orxFLAG_TEST(_pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_COMMAND))
      {
        /* Clears cache */
        _pstValue->u16Type = (orxU16)orxCONFIG_VALUE_TYPE_STRING;
      }
    }
    else
    {
      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }

  /* Done! */
  return eResult;
}

/** Reads an unsigned integer value from config value
 * @param[in]   _pstValue         Concerned config value
 * @param[in]   _s32ListIndex     List index
 * @param[out]  _pu32Result       Result value
 * @return orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS orxConfig_GetU32FromValue(orxCONFIG_VALUE *_pstValue, orxS32 _s32ListIndex, orxU32 *_pu32Result)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

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
      _s32ListIndex = (orxS32)orxMath_GetRandomU32(0, (orxS32)_pstValue->u16ListCount - 1);
    }
  }

  /* Is it cached and on the same index? */
  if((_pstValue->u16Type == (orxU16)orxCONFIG_VALUE_TYPE_U32) && (_s32ListIndex == (orxS32)_pstValue->u16CacheIndex))
  {
    /* Random? */
    if(orxFLAG_TEST(_pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_RANDOM))
    {
      /* Updates result */
      *_pu32Result = orxMath_GetRandomU32(_pstValue->u32Value, _pstValue->u32AltValue);
    }
    else
    {
      /* Updates result */
      *_pu32Result = _pstValue->u32Value;
    }
  }
  else
  {
    orxU32          u32Value;
    const orxSTRING zRemainder;
    const orxSTRING zStart;

    /* Gets wanted value */
    zStart = orxConfig_GetListValue(_pstValue, _s32ListIndex, orxFALSE);

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
          _pstValue->u16Type        = (orxU16)orxCONFIG_VALUE_TYPE_U32;
          _pstValue->u16CacheIndex  = (orxU16)_s32ListIndex;
          _pstValue->u32Value       = u32Value;
          _pstValue->u32AltValue    = u32OtherValue;

          /* Updates result */
          *_pu32Result = orxMath_GetRandomU32(u32Value, u32OtherValue);
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Failed to get U32 random from config value <%s>.", zStart);

          /* Clears cache */
          _pstValue->u16Type = (orxU16)orxCONFIG_VALUE_TYPE_STRING;

          /* Updates result */
          *_pu32Result = u32Value;
        }
      }
      else
      {
        /* Updates cache */
        _pstValue->u16Type        = (orxU16)orxCONFIG_VALUE_TYPE_U32;
        _pstValue->u16CacheIndex  = (orxU16)_s32ListIndex;
        _pstValue->u32Value       = u32Value;
        _pstValue->u32AltValue    = u32Value;

        /* Updates result */
        *_pu32Result = u32Value;
      }

      /* Command? */
      if(orxFLAG_TEST(_pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_COMMAND))
      {
        /* Clears cache */
        _pstValue->u16Type = (orxU16)orxCONFIG_VALUE_TYPE_STRING;
      }
    }
    else
    {
      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }

  /* Done! */
  return eResult;
}

/** Reads a signed integer value from config value
 * @param[in]   _pstValue         Concerned config value
 * @param[in]   _s32ListIndex     List index
 * @param[out]  _ps64Result       Result value
 * @return orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS orxConfig_GetS64FromValue(orxCONFIG_VALUE *_pstValue, orxS32 _s32ListIndex, orxS64 *_ps64Result)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

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
      _s32ListIndex = (orxS32)orxMath_GetRandomU32(0, (orxU32)_pstValue->u16ListCount - 1);
    }
  }

  /* Is it cached and on the same index? */
  if((_pstValue->u16Type == (orxU16)orxCONFIG_VALUE_TYPE_S64) && (_s32ListIndex == (orxS32)_pstValue->u16CacheIndex))
  {
    /* Random? */
    if(orxFLAG_TEST(_pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_RANDOM))
    {
      /* Updates result */
      *_ps64Result = orxMath_GetRandomS64(_pstValue->s64Value, _pstValue->s64AltValue);
    }
    else
    {
      /* Updates result */
      *_ps64Result = _pstValue->s64Value;
    }
  }
  else
  {
    orxS64          s64Value;
    const orxSTRING zRemainder;
    const orxSTRING zStart;

    /* Gets wanted value */
    zStart = orxConfig_GetListValue(_pstValue, _s32ListIndex, orxFALSE);

    /* Gets value */
    if(orxString_ToS64(zStart, &s64Value, &zRemainder) != orxSTATUS_FAILURE)
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
        orxS64 s64OtherValue;

        /* Has another value? */
        if(orxString_ToS64(zRemainder + s32RandomSeparatorIndex + 1, &s64OtherValue, orxNULL) != orxSTATUS_FAILURE)
        {
          /* Updates cache */
          _pstValue->u16Type        = (orxU16)orxCONFIG_VALUE_TYPE_S64;
          _pstValue->u16CacheIndex  = (orxU16)_s32ListIndex;
          _pstValue->s64Value       = s64Value;
          _pstValue->s64AltValue    = s64OtherValue;

          /* Updates result */
          *_ps64Result = orxMath_GetRandomS64(s64Value, s64OtherValue);
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Failed to get S64 random from config value <%s>.", zStart);

          /* Clears cache */
          _pstValue->u16Type = (orxU16)orxCONFIG_VALUE_TYPE_STRING;

          /* Updates result */
          *_ps64Result = s64Value;
        }
      }
      else
      {
        /* Updates cache */
        _pstValue->u16Type        = (orxU16)orxCONFIG_VALUE_TYPE_S64;
        _pstValue->u16CacheIndex  = (orxU16)_s32ListIndex;
        _pstValue->s64Value       = s64Value;
        _pstValue->s64AltValue    = s64Value;

        /* Updates result */
        *_ps64Result = s64Value;
      }

      /* Command? */
      if(orxFLAG_TEST(_pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_COMMAND))
      {
        /* Clears cache */
        _pstValue->u16Type = (orxU16)orxCONFIG_VALUE_TYPE_STRING;
      }
    }
    else
    {
      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }

  /* Done! */
  return eResult;
}

/** Reads an unsigned integer value from config value
 * @param[in]   _pstValue         Concerned config value
 * @param[in]   _s32ListIndex     List index
 * @param[out]  _pu64Result       Result value
 * @return orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS orxConfig_GetU64FromValue(orxCONFIG_VALUE *_pstValue, orxS32 _s32ListIndex, orxU64 *_pu64Result)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

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
      _s32ListIndex = (orxS32)orxMath_GetRandomU32(0, (orxS32)_pstValue->u16ListCount - 1);
    }
  }

  /* Is it cached and on the same index? */
  if((_pstValue->u16Type == (orxU16)orxCONFIG_VALUE_TYPE_U64) && (_s32ListIndex == (orxS32)_pstValue->u16CacheIndex))
  {
    /* Random? */
    if(orxFLAG_TEST(_pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_RANDOM))
    {
      /* Updates result */
      *_pu64Result = orxMath_GetRandomU64(_pstValue->u64Value, _pstValue->u64AltValue);
    }
    else
    {
      /* Updates result */
      *_pu64Result = _pstValue->u64Value;
    }
  }
  else
  {
    orxU64          u64Value;
    const orxSTRING zRemainder;
    const orxSTRING zStart;

    /* Gets wanted value */
    zStart = orxConfig_GetListValue(_pstValue, _s32ListIndex, orxFALSE);

    /* Gets value */
    if(orxString_ToU64(zStart, &u64Value, &zRemainder) != orxSTATUS_FAILURE)
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
        orxU64 u64OtherValue;

        /* Has another value? */
        if(orxString_ToU64(zRemainder + s32RandomSeparatorIndex + 1, &u64OtherValue, orxNULL) != orxSTATUS_FAILURE)
        {
          /* Updates cache */
          _pstValue->u16Type        = (orxU16)orxCONFIG_VALUE_TYPE_U64;
          _pstValue->u16CacheIndex  = (orxU16)_s32ListIndex;
          _pstValue->u64Value       = u64Value;
          _pstValue->u64AltValue    = u64OtherValue;

          /* Updates result */
          *_pu64Result = orxMath_GetRandomU64(u64Value, u64OtherValue);
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Failed to get U64 random from config value <%s>.", zStart);

          /* Clears cache */
          _pstValue->u16Type = (orxU16)orxCONFIG_VALUE_TYPE_STRING;

          /* Updates result */
          *_pu64Result = u64Value;
        }
      }
      else
      {
        /* Updates cache */
        _pstValue->u16Type        = (orxU16)orxCONFIG_VALUE_TYPE_U64;
        _pstValue->u16CacheIndex  = (orxU16)_s32ListIndex;
        _pstValue->u64Value       = u64Value;
        _pstValue->u64AltValue    = u64Value;

        /* Updates result */
        *_pu64Result = u64Value;
      }

      /* Command? */
      if(orxFLAG_TEST(_pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_COMMAND))
      {
        /* Clears cache */
        _pstValue->u16Type = (orxU16)orxCONFIG_VALUE_TYPE_STRING;
      }
    }
    else
    {
      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }

  /* Done! */
  return eResult;
}

/** Reads a float value from config value
 * @param[in]   _pstValue         Concerned config value
 * @param[in]   _s32ListIndex     List index
 * @param[out]  _pfResult         Result value
 * @return orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS orxConfig_GetFloatFromValue(orxCONFIG_VALUE *_pstValue, orxS32 _s32ListIndex, orxFLOAT *_pfResult)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

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
      _s32ListIndex = (orxS32)orxMath_GetRandomU32(0, (orxU32)_pstValue->u16ListCount - 1);
    }
  }

  /* Is it cached and on the same index? */
  if((_pstValue->u16Type == (orxU16)orxCONFIG_VALUE_TYPE_FLOAT) && (_s32ListIndex == (orxS32)_pstValue->u16CacheIndex))
  {
    /* Random? */
    if(orxFLAG_TEST(_pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_RANDOM))
    {
      /* Updates result */
      *_pfResult = orxMath_GetRandomFloat(_pstValue->fValue, _pstValue->fAltValue);
    }
    else
    {
      /* Updates result */
      *_pfResult = _pstValue->fValue;
    }
  }
  else
  {
    orxFLOAT        fValue;
    const orxSTRING zRemainder;
    const orxSTRING zStart;

    /* Gets wanted value */
    zStart = orxConfig_GetListValue(_pstValue, _s32ListIndex, orxFALSE);

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
          _pstValue->u16Type        = (orxU16)orxCONFIG_VALUE_TYPE_FLOAT;
          _pstValue->u16CacheIndex  = (orxU16)_s32ListIndex;
          _pstValue->fValue         = fValue;
          _pstValue->fAltValue      = fOtherValue;

          /* Updates result */
          *_pfResult = orxMath_GetRandomFloat(fValue, fOtherValue);
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Failed to get FLOAT random from config value <%s>.", zStart);

          /* Clears cache */
          _pstValue->u16Type = (orxU16)orxCONFIG_VALUE_TYPE_STRING;

          /* Updates result */
          *_pfResult = fValue;
        }
      }
      else
      {
        /* Updates cache */
        _pstValue->u16Type        = (orxU16)orxCONFIG_VALUE_TYPE_FLOAT;
        _pstValue->u16CacheIndex  = (orxU16)_s32ListIndex;
        _pstValue->fValue         = fValue;
        _pstValue->fAltValue      = fValue;

        /* Updates result */
        *_pfResult = fValue;
      }

      /* Command? */
      if(orxFLAG_TEST(_pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_COMMAND))
      {
        /* Clears cache */
        _pstValue->u16Type = (orxU16)orxCONFIG_VALUE_TYPE_STRING;
      }
    }
    else
    {
      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }

  /* Done! */
  return eResult;
}

/** Reads a string value from config value
 * @param[in]   _pstValue         Concerned config value
 * @param[in]   _s32ListIndex     List index
 * @param[out]  _pzResult         Result value
 * @return orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS orxConfig_GetStringFromValue(orxCONFIG_VALUE *_pstValue, orxS32 _s32ListIndex, const orxSTRING *_pzResult)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

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
      _s32ListIndex = (orxS32)orxMath_GetRandomU32(0, (orxU32)_pstValue->u16ListCount - 1);
    }
  }

  /* Gets wanted value */
  *_pzResult = orxConfig_GetListValue(_pstValue, _s32ListIndex, orxTRUE);

  /* Done! */
  return eResult;
}

/** Reads a boolean value from config value
 * @param[in]   _pstValue         Concerned config value
 * @param[in]   _s32ListIndex     List index
 * @param[out]  _pbResult         Result value
 * @return orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS orxConfig_GetBoolFromValue(orxCONFIG_VALUE *_pstValue, orxS32 _s32ListIndex, orxBOOL *_pbResult)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

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
      _s32ListIndex = (orxS32)orxMath_GetRandomU32(0, (orxU32)_pstValue->u16ListCount - 1);
    }
  }

  /* Is it cached and on the same index? */
  if((_pstValue->u16Type == (orxU16)orxCONFIG_VALUE_TYPE_BOOL) && (_s32ListIndex == (orxS32)_pstValue->u16CacheIndex))
  {
    /* Updates result */
    *_pbResult = _pstValue->bValue;
  }
  else
  {
    orxBOOL         bValue;
    const orxSTRING zStart;

    /* Gets wanted value */
    zStart = orxConfig_GetListValue(_pstValue, _s32ListIndex, orxFALSE);

    /* Gets value */
    if(orxString_ToBool(zStart, &bValue, orxNULL) != orxSTATUS_FAILURE)
    {
      /* Command? */
      if(orxFLAG_TEST(_pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_COMMAND))
      {
        /* Clears cache */
        _pstValue->u16Type = (orxU16)orxCONFIG_VALUE_TYPE_STRING;
      }
      else
      {
        /* Updates cache */
        _pstValue->u16Type        = (orxU16)orxCONFIG_VALUE_TYPE_BOOL;
        _pstValue->u16CacheIndex  = (orxU16)_s32ListIndex;
        _pstValue->bValue         = bValue;
      }

      /* Updates result */
      *_pbResult = bValue;
    }
    else
    {
      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }

  /* Done! */
  return eResult;
}

/** Reads a vector value from config value
 * @param[in]   _pstValue         Concerned config value
 * @param[in]   _s32ListIndex     List index
 * @param[out]  _pvResult         Result value
 * @return orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS orxConfig_GetVectorFromValue(orxCONFIG_VALUE *_pstValue, orxS32 _s32ListIndex, orxVECTOR *_pvResult)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

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
      _s32ListIndex = (orxS32)orxMath_GetRandomU32(0, (orxU32)_pstValue->u16ListCount - 1);
    }
  }

  /* Is it cached and on the same index? */
  if((_pstValue->u16Type == (orxU16)orxCONFIG_VALUE_TYPE_VECTOR) && (_s32ListIndex == (orxS32)_pstValue->u16CacheIndex))
  {
    /* Random? */
    if(orxFLAG_TEST(_pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_RANDOM))
    {
      /* Gets random values from cache */
      _pvResult->fX = orxMath_GetRandomFloat(_pstValue->vValue.fX, _pstValue->vAltValue.fX);
      _pvResult->fY = orxMath_GetRandomFloat(_pstValue->vValue.fY, _pstValue->vAltValue.fY);
      _pvResult->fZ = orxMath_GetRandomFloat(_pstValue->vValue.fZ, _pstValue->vAltValue.fZ);
    }
    else
    {
      /* Copies value from cache */
      orxVector_Copy(_pvResult, &(_pstValue->vValue));
    }
  }
  else
  {
    const orxSTRING zRemainder;
    const orxSTRING zStart;

    /* Gets wanted value */
    zStart = orxConfig_GetListValue(_pstValue, _s32ListIndex, orxFALSE);

    /* Gets value */
    if(orxString_ToVector(zStart, _pvResult, &zRemainder) != orxSTATUS_FAILURE)
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
          _pstValue->u16Type        = (orxU16)orxCONFIG_VALUE_TYPE_VECTOR;
          _pstValue->u16CacheIndex  = (orxU16)_s32ListIndex;
          orxVector_Copy(&(_pstValue->vValue), _pvResult);
          orxVector_Copy(&(_pstValue->vAltValue), &vOtherValue);

          /* Updates result */
          _pvResult->fX = orxMath_GetRandomFloat(_pvResult->fX, vOtherValue.fX);
          _pvResult->fY = orxMath_GetRandomFloat(_pvResult->fY, vOtherValue.fY);
          _pvResult->fZ = orxMath_GetRandomFloat(_pvResult->fZ, vOtherValue.fZ);
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Failed to get VECTOR random from config value <%s>.", _pstValue->zValue);

          /* Clears cache */
          _pstValue->u16Type = (orxU16)orxCONFIG_VALUE_TYPE_STRING;
        }
      }
      else
      {
        /* Updates cache */
        _pstValue->u16Type        = (orxU16)orxCONFIG_VALUE_TYPE_VECTOR;
        _pstValue->u16CacheIndex  = (orxU16)_s32ListIndex;
        orxVector_Copy(&(_pstValue->vValue), _pvResult);
        orxVector_Copy(&(_pstValue->vAltValue), _pvResult);
      }

      /* Command? */
      if(orxFLAG_TEST(_pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_COMMAND))
      {
        /* Clears cache */
        _pstValue->u16Type = (orxU16)orxCONFIG_VALUE_TYPE_STRING;
      }
    }
    else
    {
      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }

  /* Done! */
  return eResult;
}

/** Processes buffer (load)
 */
static orxU32 orxFASTCALL orxConfig_ProcessBuffer(const orxSTRING _zName, orxCHAR *_acBuffer, orxU32 _u32Size, orxU32 _u32Offset)
{
  orxCHAR  *pc, *pcKeyEnd, *pcLineStart, *pcValueStart;
  orxBOOL   bBlockMode, bAppend;
  orxU32    u32Result;

  /* For all buffered characters */
  for(pc = pcLineStart = _acBuffer + _u32Offset, pcKeyEnd = pcValueStart = orxNULL, bBlockMode = orxFALSE, bAppend = orxFALSE;
      pc < _acBuffer + _u32Size;
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
      /* Block mode? */
      if(bBlockMode != orxFALSE)
      {
        /* Not enough buffer? */
        if(pc + 1 >= _acBuffer + _u32Size)
        {
          /* Continues */
          continue;
        }
        /* Double block character? */
        else if(*(pc + 1) == orxCONFIG_KC_BLOCK)
        {
          /* Skips next character */
          pc++;

          /* Continues */
          continue;
        }
      }

      /* Has key & value? */
      if((pcKeyEnd != orxNULL) && (pcValueStart != orxNULL))
      {
        orxSTRING pcValueEnd;

        /* Not in block mode? */
        if(bBlockMode == orxFALSE)
        {
          /* Finds end of value position */
          for(pcValueEnd = pc - 1;
              (pcValueEnd > pcValueStart) && ((*pcValueEnd == ' ') || (*pcValueEnd == '\t'));
              pcValueEnd--);

          /* Is it a list separator? */
          if(*pcValueEnd == orxCONFIG_KC_LIST_SEPARATOR)
          {
            /* Is it a double list separator? */
            if((pcValueEnd > pcValueStart) && (*(pcValueEnd - 1) == orxCONFIG_KC_LIST_SEPARATOR))
            {
              /* Ignores last list separator */
              pcValueEnd--;
            }
            else
            {
              /* Erases the whole line */
              for(pcValueEnd++;
                  (pcValueEnd < _acBuffer + _u32Size) && (*pcValueEnd != orxCHAR_CR) && (*pcValueEnd != orxCHAR_LF);
                  pcValueEnd++)
              {
                *pcValueEnd = ' ';
              }

              /* Valid? */
              if(pcValueEnd < _acBuffer + _u32Size)
              {
                /* Removes current line stopper */
                *pcValueEnd = ' ';
              }
              else
              {
                /* Makes sure we don't mistake remaining partial comment for a new key */
                *(pcValueEnd - 1) = orxCONFIG_KC_COMMENT;
              }

              /* Updates current character */
              pc = pcValueEnd;

              /* Continues */
              continue;
            }
          }
        }
        else
        {
          /* Gets last block characters */
          pcValueEnd = pc - 1;
        }

        /* Skips the whole line */
        while((pc < _acBuffer + _u32Size) && (*pc != orxCHAR_CR) && (*pc != orxCHAR_LF))
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

#ifdef __orxDEBUG__
        {
          orxCONFIG_ENTRY  *pstEntry;
          orxSTRINGID       stKeyID;

          /* Gets key ID */
          stKeyID = orxString_ToCRC(pcLineStart);

          /* Already defined? */
          if((pstEntry = orxConfig_GetEntry(stKeyID)) != orxNULL)
          {
            /* Not in block mode? */
            if(!orxFLAG_TEST(pstEntry->stValue.u16Flags, orxCONFIG_VALUE_KU16_FLAG_BLOCK_MODE))
            {
              /* Restores literal value */
              orxConfig_RestoreLiteralValue(&(pstEntry->stValue));

              /* Appending? */
              if(bAppend != orxFALSE)
              {
                /* Logs */
                orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "[%s]: <%s.%s> += %s", _zName, sstConfig.pstCurrentSection->zName, orxString_GetFromID(pstEntry->stID), pcValueStart);
              }
              /* Different? */
              else if(orxString_Compare(pcValueStart, pstEntry->stValue.zValue) != 0)
              {
                /* Logs */
                orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "[%s]: <%s.%s> = %s -> %s", _zName, sstConfig.pstCurrentSection->zName, orxString_GetFromID(pstEntry->stID), pstEntry->stValue.zValue, pcValueStart);
              }

              /* Recomputes working value */
              orxConfig_ComputeWorkingValue(&(pstEntry->stValue), orxU32_UNDEFINED);
            }
            else
            {
              /* Different? */
              if(orxString_Compare(pcValueStart, pstEntry->stValue.zValue) != 0)
              {
                /* Logs */
                orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "[%s]: <%s.%s> = %s -> %s", _zName, sstConfig.pstCurrentSection->zName, orxString_GetFromID(pstEntry->stID), pstEntry->stValue.zValue, pcValueStart);
              }
            }
          }
        }
#endif /* __orxDEBUG__ */

        /* Adds/replaces entry */
        orxConfig_SetEntry(pcLineStart, pcValueStart, bBlockMode, bAppend);

        /* Updates pointers */
        pcKeyEnd = pcValueStart = orxNULL;
      }
      else
      {
        /* Skips the whole line */
        while((pc < _acBuffer + _u32Size) && (*pc != orxCHAR_CR) && (*pc != orxCHAR_LF))
        {
          pc++;
        }
      }

      /* Resets block mode */
      bBlockMode = orxFALSE;

      /* Valid? */
      if(pc < _acBuffer + _u32Size)
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
      while((pc < _acBuffer + _u32Size) && ((*pc == orxCHAR_CR) || (*pc == orxCHAR_LF) || (*pc == ' ') || (*pc == '\t')))
      {
        /* Updates pointers */
        pcLineStart++, pc++;
      }

      /* Depending on first character */
      switch(*pc)
      {
        /* Inheritance marker? */
        case orxCONFIG_KC_INHERITANCE_MARKER:
        {
          /* Updates pointer */
          pc++;

          /* Finds section end */
          while((pc < _acBuffer + _u32Size) && (*pc != orxCONFIG_KC_INHERITANCE_MARKER))
          {
            /* End of line? */
            if((*pc == orxCHAR_CR) || (*pc == orxCHAR_LF))
            {
              /* Logs message */
              orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "[%s]: Incomplete file name <%*s>, closing character '%c' not found", _zName, pc - (pcLineStart + 1), pcLineStart + 1, orxCONFIG_KC_INHERITANCE_MARKER);

              /* Updates new line start */
              pcLineStart = pc + 1;

              break;
            }

            /* Updates pointer */
            pc++;
          }

          /* Valid? */
          if((pc < _acBuffer + _u32Size) && (*pc == orxCONFIG_KC_INHERITANCE_MARKER))
          {
            orxCONFIG_SECTION *pstCurrentSection;

            /* Gets current section */
            pstCurrentSection = sstConfig.pstCurrentSection;

            /* Cuts string */
            *pc = orxCHAR_NULL;

            /* Logs message */
            orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "[%s]: Begin include %c%s%c", _zName, orxCONFIG_KC_INHERITANCE_MARKER, pcLineStart + 1, orxCONFIG_KC_INHERITANCE_MARKER);

            /* Loads file */
            orxConfig_Load(pcLineStart + 1);

            /* Logs message */
            orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "[%s]: End include %c%s%c", _zName, orxCONFIG_KC_INHERITANCE_MARKER, pcLineStart + 1, orxCONFIG_KC_INHERITANCE_MARKER);

            /* Restores current section */
            sstConfig.pstCurrentSection = pstCurrentSection;

            /* Skips the whole line */
            while((pc < _acBuffer + _u32Size) && (*pc != orxCHAR_CR) && (*pc != orxCHAR_LF))
            {
              pc++;
            }

            /* Valid? */
            if(pc < _acBuffer + _u32Size)
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

          break;
        }

        /* Section start? */
        case orxCONFIG_KC_SECTION_START:
        {
          /* Finds section end */
          while((pc < _acBuffer + _u32Size) && (*pc != orxCONFIG_KC_SECTION_END))
          {
            /* End of line? */
            if((*pc == orxCHAR_CR) || (*pc == orxCHAR_LF))
            {
              /* Logs message */
              *pc = orxCHAR_NULL;
              orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "[%s]: Incomplete section name <%s>, closing character '%c' not found", _zName, pcLineStart + 1, orxCONFIG_KC_SECTION_END);

              /* Updates new line start */
              pcLineStart = pc + 1;

              break;
            }

            /* Updates pointer */
            pc++;
          }

          /* Valid? */
          if((pc < _acBuffer + _u32Size) && (*pc == orxCONFIG_KC_SECTION_END))
          {
            /* Cuts string */
            *pc = orxCHAR_NULL;

            /* Selects section */
            orxConfig_SelectSection(pcLineStart + 1);

            /* Skips the whole line */
            while((pc < _acBuffer + _u32Size) && (*pc != orxCHAR_CR) && (*pc != orxCHAR_LF))
            {
              pc++;
            }

            /* Valid? */
            if(pc < _acBuffer + _u32Size)
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

          break;
        }

        /* Comment character? */
        case orxCONFIG_KC_COMMENT:
        {
          /* Skips the whole line */
          while((pc < _acBuffer + _u32Size) && (*pc != orxCHAR_CR) && (*pc != orxCHAR_LF))
          {
            pc++;
          }

          /* Valid? */
          if(pc < _acBuffer + _u32Size)
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

          break;
        }

        /* Command character */
        case orxCONFIG_KC_COMMAND:
        {
          /* Finds command end */
          while((pc < _acBuffer + _u32Size) && (*pc != orxCHAR_CR) && (*pc != orxCHAR_LF) && (*pc != orxCONFIG_KC_COMMENT))
          {
            /* Updates pointers */
            pc++;
          }

          /* Valid? */
          if(pc < _acBuffer + _u32Size)
          {
            orxCOMMAND_VAR stDummy;

            /* Cuts string */
            *pc = orxCHAR_NULL;

            /* Evaluates command */
            orxCommand_Evaluate(pcLineStart + 1, &stDummy);

            /* Skips the whole line */
            while((pc < _acBuffer + _u32Size) && (*pc != orxCHAR_CR) && (*pc != orxCHAR_LF))
            {
              pc++;
            }

            /* Valid? */
            if(pc < _acBuffer + _u32Size)
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

          break;
        }

        default:
        {
          /* Finds assign character */
          while((pc < _acBuffer + _u32Size) && (*pc != orxCONFIG_KC_ASSIGN) && (*pc != orxCHAR_CR) && (*pc != orxCHAR_LF))
          {
            /* Updates pointer */
            pc++;
          }

          /* Found? */
          if((pc < _acBuffer + _u32Size) && (*pc == orxCONFIG_KC_ASSIGN))
          {
            orxCHAR *pcStart;

            /* Append? */
            if(*(pc - 1) == orxCONFIG_KC_APPEND)
            {
              /* Inits end of key search */
              pcStart = pc - 2;
              bAppend = orxTRUE;
            }
            else
            {
              /* Inits end of key search */
              pcStart = pc - 1;
              bAppend = orxFALSE;
            }

            /* Finds end of key position */
            for(pcKeyEnd = pcStart;
                (pcKeyEnd > pcLineStart) && ((*pcKeyEnd == ' ') || (*pcKeyEnd == '\t') || (*pcKeyEnd == orxCHAR_CR) || (*pcKeyEnd == orxCHAR_LF));
                pcKeyEnd--);

            /* Updates key end pointer */
            pcKeyEnd += 1;

            /* Checks */
            orxASSERT(pcKeyEnd > pcLineStart);

            /* Finds start of value position */
            for(pcValueStart = pc + 1;
                (pcValueStart < _acBuffer + _u32Size) && ((*pcValueStart == ' ') || (*pcValueStart == '\t'));
                pcValueStart++);

            /* Valid? */
            if(pcValueStart < _acBuffer + _u32Size)
            {
              /* Is it a block delimiter character? */
              if(*pcValueStart == orxCONFIG_KC_BLOCK)
              {
                /* Updates value start pointer */
                pcValueStart++;

                /* Valid? */
                if(pcValueStart < _acBuffer + _u32Size)
                {
                  /* Is not a block delimiter or triple block delimiter? */
                  if((*pcValueStart != orxCONFIG_KC_BLOCK)
                  || ((pcValueStart + 1 < _acBuffer + _u32Size)
                   && (*(pcValueStart + 1) == orxCONFIG_KC_BLOCK)))
                  {
                    /* Activates block mode */
                    bBlockMode = orxTRUE;
                  }
                }
              }
            }

            /* Updates current character */
            pc = pcValueStart - 1;
          }
          else
          {
            /* Not at end of buffer */
            if(pc < _acBuffer + _u32Size)
            {
              /* Logs message */
              *pc = orxCHAR_NULL;
              orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "[%s]: No value for key <%s>, assign character '%c' not found", _zName, pcLineStart, orxCONFIG_KC_ASSIGN);
            }
          }

          break;
        }
      }
    }
  }

  /* Has remaining buffer? */
  if((pcLineStart != _acBuffer) && (pc > pcLineStart))
  {
    /* Updates result */
    u32Result = (orxU32)(_acBuffer + _u32Size - pcLineStart);
  }
  else
  {
    /* Clears result */
    u32Result = 0;
  }

  /* Done! */
  return u32Result;
}

/** Select section (internal)
 */
static orxSTATUS orxConfig_SelectSectionInternal(const orxSTRING _zSectionName)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Valid? */
  if(_zSectionName != orxSTRING_EMPTY)
  {
    const orxSTRING     zSectionName;
    orxCONFIG_SECTION  *pstSection, *pstParent = orxNULL;
    orxCHAR            *pcNameEnd;
    orxBOOL             bNewParent = orxFALSE;
    orxS32              s32MarkerIndex;

    /* Looks for inheritance index */
    s32MarkerIndex = orxString_SearchCharIndex(_zSectionName, orxCONFIG_KC_INHERITANCE_MARKER, 0);

    /* Found? */
    if(s32MarkerIndex >= 0)
    {
      /* Cuts the name */
      *((orxSTRING)_zSectionName + s32MarkerIndex) = orxCHAR_NULL;

      /* Gets end of name */
      for(pcNameEnd = (orxSTRING)_zSectionName + s32MarkerIndex - 1; (pcNameEnd > _zSectionName) && (*pcNameEnd == ' '); pcNameEnd--);

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

      if(_zSectionName[s32MarkerIndex + 1] == orxCONFIG_KC_INHERITANCE_MARKER)
      {
        /* Forces 'no default' parent ID */
        pstParent = (orxCONFIG_SECTION *)orxHANDLE_UNDEFINED;

        /* Asks for new parent to be set */
        bNewParent = orxTRUE;
      }
      else
      {
        const orxSTRING     zParent;
        orxCONFIG_SECTION  *pstPreviousSection;

        /* Backups current section */
        pstPreviousSection = sstConfig.pstCurrentSection;

        /* Gets parent name */
        for(zParent = _zSectionName + s32MarkerIndex + 1; *zParent == ' '; zParent++);

        /* Selects it */
        if(orxConfig_SelectSectionInternal(zParent) != orxSTATUS_FAILURE)
        {
          /* Protects it */
          sstConfig.pstCurrentSection->s32ProtectionCount++;

          /* Checks */
          orxASSERT(sstConfig.pstCurrentSection->s32ProtectionCount >= 0);

          /* Stores it */
          pstParent = sstConfig.pstCurrentSection;

          /* Asks for new parent to be set */
          bNewParent = orxTRUE;
        }

        /* Restores previous section */
        sstConfig.pstCurrentSection = pstPreviousSection;
      }
    }
    else
    {
      /* Clears end of name */
      pcNameEnd = orxNULL;
    }

    /* Stores its name */
    zSectionName = orxString_Store(_zSectionName);

    /* Not already selected? */
    if((sstConfig.pstCurrentSection == orxNULL)
    || (sstConfig.pstCurrentSection->zName != zSectionName))
    {
      /* Gets it from table */
      pstSection = (orxCONFIG_SECTION *)orxHashTable_Get(sstConfig.pstSectionTable, orxString_ToCRC(_zSectionName));

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
      pstSection = orxConfig_CreateSection(zSectionName, pstParent);

      /* Success? */
      if(pstSection != orxNULL)
      {
        /* Selects it */
        sstConfig.pstCurrentSection = pstSection;
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Failed to create config section [%s].", _zSectionName);

        /* Updates result */
        eResult = orxSTATUS_FAILURE;
      }
    }
    else
    {
      /* Loading? */
      if(sstConfig.u32LoadCount != 0)
      {
        /* Has new parent ID? */
        if(bNewParent != orxFALSE)
        {
          /* Updates parent ID */
          pstSection->pstParent = pstParent;
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
      *((orxSTRING)_zSectionName + s32MarkerIndex) = orxCONFIG_KC_INHERITANCE_MARKER;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Failed to select config section (%s), invalid name.", _zSectionName);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}
/** Origin save callback
 */
orxBOOL orxFASTCALL orxConfig_OriginSaveCallback(const orxSTRING _zSectionName, const orxSTRING _zKeyName, const orxSTRING _zFileName, orxBOOL _bUseEncryption)
{
  orxBOOL bResult;

#ifdef __orxWINDOWS__

  /* Updates result */
  bResult = ((_zKeyName != orxNULL) || (orxString_ICompare(_zFileName, orxConfig_GetOrigin(_zSectionName)) == 0)) ? orxTRUE : orxFALSE;

#else /* __orxWINDOWS__ */

  /* Updates result */
  bResult = ((_zKeyName != orxNULL) || (orxString_Compare(_zFileName, orxConfig_GetOrigin(_zSectionName)) == 0)) ? orxTRUE : orxFALSE;

#endif /* __orxWINDOWS__ */

  /* Done! */
  return bResult;
}

/** Command: Load
 */
void orxFASTCALL orxConfig_CommandLoad(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->bValue = (orxConfig_Load(_astArgList[0].zValue) != orxSTATUS_FAILURE) ? orxTRUE : orxFALSE;

  /* Done! */
  return;
}

/** Command: Save
 */
void orxFASTCALL orxConfig_CommandSave(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->bValue = (orxConfig_Save(_astArgList[0].zValue, (_u32ArgNumber > 2) ? _astArgList[2].bValue : orxFALSE, ((_u32ArgNumber > 1) && (_astArgList[1].bValue != orxFALSE)) ? orxConfig_OriginSaveCallback : orxNULL) != orxSTATUS_FAILURE) ? orxTRUE : orxFALSE;

  /* Done! */
  return;
}

/** Command: Reload
 */
void orxFASTCALL orxConfig_CommandReload(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->bValue = (orxConfig_ReloadHistory() != orxSTATUS_FAILURE) ? orxTRUE : orxFALSE;

  /* Done! */
  return;
}

/** Command: GetOrigin
 */
void orxFASTCALL orxConfig_CommandGetOrigin(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->zValue = orxConfig_GetOrigin(_astArgList[0].zValue);

  /* Done! */
  return;
}

/** Command: GetParent
 */
void orxFASTCALL orxConfig_CommandGetParent(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->zValue = orxConfig_GetParent(_astArgList[0].zValue);
  if(_pstResult->zValue == orxNULL)
  {
    _pstResult->zValue = orxSTRING_EMPTY;
  }

  /* Done! */
  return;
}

/** Command: SetParent
 */
void orxFASTCALL orxConfig_CommandSetParent(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->zValue = (orxConfig_SetParent(_astArgList[0].zValue, (_u32ArgNumber > 1) ? ((_astArgList[1].zValue[0] != orxCONFIG_KC_INHERITANCE_MARKER) ? _astArgList[1].zValue : orxSTRING_EMPTY) : orxNULL) != orxSTATUS_FAILURE) ? _astArgList[0].zValue : orxSTRING_EMPTY;

  /* Done! */
  return;
}

/** Command: CreateSection
 */
void orxFASTCALL orxConfig_CommandCreateSection(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->zValue = (orxConfig_SelectSection(_astArgList[0].zValue) != orxSTATUS_FAILURE) ? _astArgList[0].zValue : orxSTRING_EMPTY;

  /* Done! */
  return;
}

/** Command: HasSection
 */
void orxFASTCALL orxConfig_CommandHasSection(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->bValue = (orxConfig_HasSection(_astArgList[0].zValue));

  /* Done! */
  return;
}

/** Command: ClearSection
 */
void orxFASTCALL orxConfig_CommandClearSection(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Clears section */
   orxConfig_ClearSection(_astArgList[0].zValue);

  /* Updates result */
  _pstResult->zValue = _astArgList[0].zValue;

  /* Done! */
  return;
}

/** Command: GetCurrentSection
 */
void orxFASTCALL orxConfig_CommandGetCurrentSection(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->zValue = orxConfig_GetCurrentSection();

  /* Done! */
  return;
}

/** Command: ClearValue
 */
void orxFASTCALL orxConfig_CommandClearValue(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Pushes section */
  orxConfig_PushSection(_astArgList[0].zValue);

  /* Clears value */
  orxConfig_ClearValue(_astArgList[1].zValue);

  /* Pops section */
  orxConfig_PopSection();

  /* Updates result */
  _pstResult->zValue = _astArgList[1].zValue;

  /* Done! */
  return;
}

/** Command: HasValue
 */
void orxFASTCALL orxConfig_CommandHasValue(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Pushes section */
  orxConfig_PushSection(_astArgList[0].zValue);

  /* Updates result */
  _pstResult->bValue = (orxConfig_HasValue(_astArgList[1].zValue));

  /* Pops section */
  orxConfig_PopSection();

  /* Done! */
  return;
}

/** Command: GetValue
 */
void orxFASTCALL orxConfig_CommandGetValue(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxS32 s32Index;

  /* Gets index */
  s32Index = (_u32ArgNumber > 2) ? _astArgList[2].s32Value : -1;

  /* Pushes section */
  orxConfig_PushSection(_astArgList[0].zValue);

  /* Raw request? */
  if((_u32ArgNumber > 3) && (_astArgList[3].bValue != orxFALSE))
  {
    /* Updates result */
    _pstResult->zValue = orxConfig_GetListString(_astArgList[1].zValue, s32Index);
  }
  else
  {
    orxCONFIG_VALUE *pstValue;

    /* Gets corresponding value */
    pstValue = orxConfig_GetValue(_astArgList[1].zValue);

    /* Success? */
    if(pstValue != orxNULL)
    {
      const orxSTRING zValue;

      /* Random? */
      if(s32Index == -1)
      {
        /* Updates real index */
        s32Index = orxFLAG_TEST(pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_LIST) ? (orxS32)orxMath_GetRandomU32(0, (orxU32)pstValue->u16ListCount - 1) : 0;
      }
      else
      {
        /* Invalid index? */
        if(s32Index >= (orxS32)pstValue->u16ListCount)
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "<%s.%s> can't access list value at index [%d]: out of bound access, returning last value at index [%u]!", _astArgList[0].zValue, _astArgList[1].zValue, s32Index, (orxU32)pstValue->u16ListCount - 1);

          /* Updates index */
          s32Index = (orxS32)pstValue->u16ListCount - 1;
        }
      }

      /* Is a command? */
      zValue = (s32Index == 0) ? pstValue->zValue : pstValue->zValue + pstValue->au32ListIndexTable[s32Index - 1];
      if((*zValue == orxCONFIG_KC_COMMAND) && (*(zValue + 1) != orxCONFIG_KC_COMMAND))
      {
        /* Gets string value */
        _pstResult->zValue = orxConfig_GetListValue(pstValue, s32Index, orxFALSE);
      }
      /* Is index valid? */
      else if(s32Index < (orxS32)pstValue->u16ListCount)
      {
        orxVECTOR vResult;
        orxBOOL   bConfigLevelEnabled;

        /* Gets config debug level state */
        bConfigLevelEnabled = orxDEBUG_IS_LEVEL_ENABLED(orxDEBUG_LEVEL_CONFIG);

        /* Deactivates config debug level */
        orxDEBUG_ENABLE_LEVEL(orxDEBUG_LEVEL_CONFIG, orxFALSE);

        /* Gets vector value */
        if(orxConfig_GetVectorFromValue(pstValue, s32Index, &vResult) != orxSTATUS_FAILURE)
        {
          /* Prints it */
          orxString_NPrint(sstConfig.acCommandBuffer, orxCONFIG_KU32_COMMAND_BUFFER_SIZE - 1, "%c%g%c %g%c %g%c", orxSTRING_KC_VECTOR_START, vResult.fX, orxSTRING_KC_VECTOR_SEPARATOR, vResult.fY, orxSTRING_KC_VECTOR_SEPARATOR, vResult.fZ, orxSTRING_KC_VECTOR_END);

          /* Updates result */
          _pstResult->zValue = sstConfig.acCommandBuffer;
        }
        else
        {
          orxBOOL bSuccess = orxFALSE;

          /* Random? */
          if(orxFLAG_TEST(pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_RANDOM))
          {
            orxFLOAT fValue;

            /* Gets random float value */
            if(orxConfig_GetFloatFromValue(pstValue, s32Index, &fValue) != orxSTATUS_FAILURE)
            {
              /* Prints it */
              orxString_NPrint(sstConfig.acCommandBuffer, orxCONFIG_KU32_COMMAND_BUFFER_SIZE - 1, "%g", fValue);

              /* Updates result */
              _pstResult->zValue = sstConfig.acCommandBuffer;

              /* Updates status */
              bSuccess = orxTRUE;
            }
          }

          /* Need string value? */
          if(bSuccess == orxFALSE)
          {
            /* Gets string value */
            _pstResult->zValue = orxConfig_GetListValue(pstValue, s32Index, orxFALSE);
          }
        }

        /* Restores config debug level state */
        orxDEBUG_ENABLE_LEVEL(orxDEBUG_LEVEL_CONFIG, bConfigLevelEnabled);
      }
      else
      {
        /* Updates result */
        _pstResult->zValue = orxSTRING_EMPTY;
      }
    }
    else
    {
      /* Updates result */
      _pstResult->zValue = orxSTRING_EMPTY;
    }
  }

  /* Pops section */
  orxConfig_PopSection();

  /* Done! */
  return;
}

/** Command: SetValue
 */
void orxFASTCALL orxConfig_CommandSetValue(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Pushes section */
  orxConfig_PushSection(_astArgList[0].zValue);

  /* Updates result */
  _pstResult->zValue = (orxConfig_SetString(_astArgList[1].zValue, _astArgList[2].zValue) != orxSTATUS_FAILURE) ? _astArgList[2].zValue : orxSTRING_EMPTY;

  /* Pops section */
  orxConfig_PopSection();

  /* Done! */
  return;
}

/** Command: AppendValue
 */
void orxFASTCALL orxConfig_CommandAppendValue(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Pushes section */
  orxConfig_PushSection(_astArgList[0].zValue);

  /* Updates result */
  _pstResult->zValue = (orxConfig_SetEntry(_astArgList[1].zValue, _astArgList[2].zValue, orxFALSE, orxTRUE) != orxSTATUS_FAILURE) ? _astArgList[2].zValue : orxSTRING_EMPTY;

  /* Pops section */
  orxConfig_PopSection();

  /* Done! */
  return;
}

/** Command: GetRawValue
 */
void orxFASTCALL orxConfig_CommandGetRawValue(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCONFIG_ENTRY *pstEntry;

  /* Pushes section */
  orxConfig_PushSection(_astArgList[0].zValue);

  /* Gets corresponding entry */
  pstEntry = orxConfig_GetEntry(orxString_ToCRC(_astArgList[1].zValue));

  /* Found? */
  if(pstEntry != orxNULL)
  {
    /* Not in block mode? */
    if(!orxFLAG_TEST(pstEntry->stValue.u16Flags, orxCONFIG_VALUE_KU16_FLAG_BLOCK_MODE))
    {
      /* Restores string */
      orxConfig_RestoreLiteralValue(&(pstEntry->stValue));

      /* Prints it */
      orxString_NPrint(sstConfig.acCommandBuffer, orxCONFIG_KU32_COMMAND_BUFFER_SIZE - 1, "%s", pstEntry->stValue.zValue);

      /* Updates result */
      _pstResult->zValue = sstConfig.acCommandBuffer;

      /* Computes working value */
      orxConfig_ComputeWorkingValue(&(pstEntry->stValue), orxU32_UNDEFINED);
    }
    else
    {
      /* Updates result */
      _pstResult->zValue = pstEntry->stValue.zValue;
    }
  }
  else
  {
    /* Updates result */
    _pstResult->zValue = orxSTRING_EMPTY;
  }

  /* Pops section */
  orxConfig_PopSection();

  /* Done! */
  return;
}

/** Command: GetListCount
 */
void orxFASTCALL orxConfig_CommandGetListCount(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Pushes section */
  orxConfig_PushSection(_astArgList[0].zValue);

  /* Updates result */
  _pstResult->s32Value = orxConfig_GetListCount(_astArgList[1].zValue);

  /* Pops section */
  orxConfig_PopSection();

  /* Done! */
  return;
}

/** Registers all the config commands
 */
static orxINLINE void orxConfig_RegisterCommands()
{
  /* Command: Load */
  orxCOMMAND_REGISTER_CORE_COMMAND(Config, Load, "Success?", orxCOMMAND_VAR_TYPE_BOOL, 1, 0, {"FileName", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: Save */
  orxCOMMAND_REGISTER_CORE_COMMAND(Config, Save, "Success?", orxCOMMAND_VAR_TYPE_BOOL, 1, 2, {"FileName", orxCOMMAND_VAR_TYPE_STRING}, {"OnlyOrigin = false", orxCOMMAND_VAR_TYPE_BOOL}, {"Encrypt = false", orxCOMMAND_VAR_TYPE_BOOL});

  /* Command: Reload */
  orxCOMMAND_REGISTER_CORE_COMMAND(Config, Reload, "Success?", orxCOMMAND_VAR_TYPE_BOOL, 0, 0);

  /* Command: GetOrigin */
  orxCOMMAND_REGISTER_CORE_COMMAND(Config, GetOrigin, "Origin", orxCOMMAND_VAR_TYPE_STRING, 1, 0, {"Section", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: GetParent */
  orxCOMMAND_REGISTER_CORE_COMMAND(Config, GetParent, "Parent", orxCOMMAND_VAR_TYPE_STRING, 1, 0, {"Section", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: SetParent */
  orxCOMMAND_REGISTER_CORE_COMMAND(Config, SetParent, "Section", orxCOMMAND_VAR_TYPE_STRING, 1, 1, {"Section", orxCOMMAND_VAR_TYPE_STRING}, {"Parent = <void>", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: CreateSection */
  orxCOMMAND_REGISTER_CORE_COMMAND(Config, CreateSection, "Section", orxCOMMAND_VAR_TYPE_STRING, 1, 0, {"Section", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: HasSection */
  orxCOMMAND_REGISTER_CORE_COMMAND(Config, HasSection, "Section?", orxCOMMAND_VAR_TYPE_BOOL, 1, 0, {"Section", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: ClearSection */
  orxCOMMAND_REGISTER_CORE_COMMAND(Config, ClearSection, "Section", orxCOMMAND_VAR_TYPE_STRING, 1, 0, {"Section", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: GetCurrentSection */
  orxCOMMAND_REGISTER_CORE_COMMAND(Config, GetCurrentSection, "Section", orxCOMMAND_VAR_TYPE_STRING, 0, 0);

  /* Command: HasValue */
  orxCOMMAND_REGISTER_CORE_COMMAND(Config, HasValue, "Value?", orxCOMMAND_VAR_TYPE_BOOL, 2, 0, {"Section", orxCOMMAND_VAR_TYPE_STRING}, {"Key", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: ClearValue */
  orxCOMMAND_REGISTER_CORE_COMMAND(Config, ClearValue, "Value", orxCOMMAND_VAR_TYPE_STRING, 2, 0, {"Section", orxCOMMAND_VAR_TYPE_STRING}, {"Key", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: GetValue */
  orxCOMMAND_REGISTER_CORE_COMMAND(Config, GetValue, "Value", orxCOMMAND_VAR_TYPE_STRING, 2, 2, {"Section", orxCOMMAND_VAR_TYPE_STRING}, {"Key", orxCOMMAND_VAR_TYPE_STRING}, {"Index = -1", orxCOMMAND_VAR_TYPE_S32}, {"Verbatim = false", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: SetValue */
  orxCOMMAND_REGISTER_CORE_COMMAND(Config, SetValue, "Value", orxCOMMAND_VAR_TYPE_STRING, 3, 0, {"Section", orxCOMMAND_VAR_TYPE_STRING}, {"Key", orxCOMMAND_VAR_TYPE_STRING}, {"Value", orxCOMMAND_VAR_TYPE_NUMERIC});
  /* Command: AppendValue */
  orxCOMMAND_REGISTER_CORE_COMMAND(Config, AppendValue, "Value", orxCOMMAND_VAR_TYPE_STRING, 3, 0, {"Section", orxCOMMAND_VAR_TYPE_STRING}, {"Key", orxCOMMAND_VAR_TYPE_STRING}, {"Value", orxCOMMAND_VAR_TYPE_NUMERIC});
  /* Command: GetRawValue */
  orxCOMMAND_REGISTER_CORE_COMMAND(Config, GetRawValue, "RawValue", orxCOMMAND_VAR_TYPE_STRING, 2, 0, {"Section", orxCOMMAND_VAR_TYPE_STRING}, {"Key", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: GetListCount */
  orxCOMMAND_REGISTER_CORE_COMMAND(Config, GetListCount, "Count", orxCOMMAND_VAR_TYPE_S32, 2, 0, {"Section", orxCOMMAND_VAR_TYPE_STRING}, {"Key", orxCOMMAND_VAR_TYPE_STRING});

  /* Alias: Load */
  orxCommand_AddAlias("Load", "Config.Load", orxNULL);
  /* Alias: Save */
  orxCommand_AddAlias("Save", "Config.Save", orxNULL);
  /* Alias: Reload */
  orxCommand_AddAlias("Reload", "Config.Reload", orxNULL);

  /* Alias: Set */
  orxCommand_AddAlias("Set", "Config.SetValue", orxNULL);
  /* Alias: Get */
  orxCommand_AddAlias("Get", "Config.GetValue", orxNULL);
  /* Alias: GetRaw */
  orxCommand_AddAlias("GetRaw", "Config.GetRawValue", orxNULL);

  /* Alias: @ */
  orxCommand_AddAlias("@", "Config.GetCurrentSection", orxNULL);
}

/** Unregisters all the config commands
 */
static orxINLINE void orxConfig_UnregisterCommands()
{
  /* Alias: Load */
  orxCommand_RemoveAlias("Load");
  /* Alias: Save */
  orxCommand_RemoveAlias("Save");
  /* Alias: Reload */
  orxCommand_RemoveAlias("Reload");

  /* Alias: Set */
  orxCommand_RemoveAlias("Set");
  /* Alias: Get */
  orxCommand_RemoveAlias("Get");
  /* Alias: GetRaw */
  orxCommand_RemoveAlias("GetRaw");

  /* Alias: @ */
  orxCommand_RemoveAlias("@");

  /* Command: Load */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Config, Load);
  /* Command: Save */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Config, Save);
  /* Command: Reload */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Config, Reload);

  /* Command: GetOrigin */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Config, GetOrigin);
  /* Command: GetParent */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Config, GetParent);
  /* Command: SetParent */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Config, SetParent);
  /* Command: CreateSection */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Config, CreateSection);
  /* Command: HasSection */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Config, HasSection);
  /* Command: ClearSection */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Config, ClearSection);
  /* Command: GetCurrentSection */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Config, GetCurrentSection);

  /* Command: HasValue */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Config, HasValue);
  /* Command: ClearValue */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Config, ClearValue);
  /* Command: GetValue */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Config, GetValue);
  /* Command: SetValue */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Config, SetValue);
  /* Command: AppendValue */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Config, AppendValue);
  /* Command: GetRawValue */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Config, GetRawValue);
  /* Command: GetListCount */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Config, GetListCount);
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
  orxModule_AddDependency(orxMODULE_ID_CONFIG, orxMODULE_ID_STRING);
  orxModule_AddDependency(orxMODULE_ID_CONFIG, orxMODULE_ID_PROFILER);
  orxModule_AddDependency(orxMODULE_ID_CONFIG, orxMODULE_ID_FILE);
  orxModule_AddDependency(orxMODULE_ID_CONFIG, orxMODULE_ID_EVENT);
  orxModule_AddDependency(orxMODULE_ID_CONFIG, orxMODULE_ID_COMMAND);
  orxModule_AddDependency(orxMODULE_ID_CONFIG, orxMODULE_ID_RESOURCE);

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
    orxCHAR                       zBackupBaseFile[orxCONFIG_KU32_BASE_FILENAME_LENGTH];
    orxSTRING                     zBackupEncryptionKey;
    orxCONFIG_BOOTSTRAP_FUNCTION  pfnBackupBootstrap;

    /* Backups base file name */
    orxMemory_Copy(zBackupBaseFile, sstConfig.zBaseFile, orxCONFIG_KU32_BASE_FILENAME_LENGTH);

    /* Backups encryption key */
    zBackupEncryptionKey = sstConfig.zEncryptionKey;

    /* Backups bootstrap */
    pfnBackupBootstrap = sstConfig.pfnBootstrap;

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

    /* Restores bootstrap */
    sstConfig.pfnBootstrap = pfnBackupBootstrap;

    /* Creates stack bank, history bank & section bank/table */
    sstConfig.pstStackBank    = orxBank_Create(orxCONFIG_KU32_STACK_BANK_SIZE, sizeof(orxCONFIG_STACK_ENTRY), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_CONFIG);
    sstConfig.pstHistoryBank  = orxBank_Create(orxCONFIG_KU32_HISTORY_BANK_SIZE, sizeof(orxU32), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_CONFIG);
    sstConfig.pstSectionBank  = orxBank_Create(orxCONFIG_KU32_SECTION_BANK_SIZE, sizeof(orxCONFIG_SECTION), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_CONFIG);
    sstConfig.pstEntryBank    = orxBank_Create(orxCONFIG_KU32_ENTRY_BANK_SIZE, sizeof(orxCONFIG_ENTRY), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_CONFIG);

    sstConfig.pstSectionTable = orxHashTable_Create(orxCONFIG_KU32_SECTION_BANK_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_CONFIG);

    /* Valid? */
    if((sstConfig.pstStackBank != orxNULL) && (sstConfig.pstHistoryBank != orxNULL) && (sstConfig.pstSectionBank != orxNULL) && (sstConfig.pstEntryBank != orxNULL) && (sstConfig.pstSectionTable != orxNULL))
    {
      orxBOOL bLoadDefault;

      /* Inits values */
      sstConfig.stLoadFileID      = orxSTRINGID_UNDEFINED;
      sstConfig.stResourceGroupID = orxString_GetID(orxCONFIG_KZ_RESOURCE_GROUP);

      /* Inits flags */
      orxFLAG_SET(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY | orxCONFIG_KU32_STATIC_FLAG_HISTORY, orxCONFIG_KU32_STATIC_MASK_ALL);

      /* Registers commands */
      orxConfig_RegisterCommands();

      /* Has bootstrap? */
      if(sstConfig.pfnBootstrap != orxNULL)
      {
        /* Calls it */
        bLoadDefault = (sstConfig.pfnBootstrap() != orxSTATUS_FAILURE) ? orxTRUE : orxFALSE;
      }
      else
      {
        /* Asks for default loading */
        bLoadDefault = orxTRUE;
      }

      /* Should load default? */
      if(bLoadDefault != orxFALSE)
      {
        /* Loads default config file */
        orxConfig_Load(sstConfig.zBaseFile);
      }

      /* Pushes config section */
      orxConfig_PushSection(orxCONFIG_KZ_CONFIG_SECTION);

      /* Sets default parent */
      orxConfig_SetDefaultParent(orxConfig_GetString(orxCONFIG_KZ_CONFIG_DEFAULT_PARENT));

      /* Pops section */
      orxConfig_PopSection();

      /* Adds event handler */
      orxEvent_AddHandler(orxEVENT_TYPE_RESOURCE, orxConfig_EventHandler);
      orxEvent_SetHandlerIDFlags(orxConfig_EventHandler, orxEVENT_TYPE_RESOURCE, orxNULL, orxEVENT_GET_FLAG(orxRESOURCE_EVENT_ADD) | orxEVENT_GET_FLAG(orxRESOURCE_EVENT_UPDATE), orxEVENT_KU32_MASK_ID_ALL);

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* Should delete stack bank? */
      if(sstConfig.pstStackBank != orxNULL)
      {
        /* Deletes it */
        orxBank_Delete(sstConfig.pstStackBank);
      }

      /* Should delete history bank? */
      if(sstConfig.pstHistoryBank != orxNULL)
      {
        /* Deletes it */
        orxBank_Delete(sstConfig.pstHistoryBank);
      }

      /* Should delete section bank? */
      if(sstConfig.pstSectionBank != orxNULL)
      {
        /* Deletes it */
        orxBank_Delete(sstConfig.pstSectionBank);
      }

      /* Should delete entry bank? */
      if(sstConfig.pstEntryBank != orxNULL)
      {
        /* Deletes it */
        orxBank_Delete(sstConfig.pstEntryBank);
      }

      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Can't allocate stack bank and/or section bank/table.");
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Tried to initialize config module when it was already initialized.");

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
    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_RESOURCE, orxConfig_EventHandler);

    /* Unregisters commands */
    orxConfig_UnregisterCommands();

    /* Removes default parent section */
    orxConfig_SetDefaultParent(orxNULL);

    /* Clears all data */
    orxConfig_Clear();

    /* Clears section list */
    orxLinkList_Clean(&(sstConfig.stSectionList));

    /* Deletes section table */
    orxHashTable_Delete(sstConfig.pstSectionTable);

    /* Deletes section bank */
    orxBank_Delete(sstConfig.pstSectionBank);

    /* Deletes entry bank */
    orxBank_Delete(sstConfig.pstEntryBank);

    /* Deletes history bank */
    orxBank_Delete(sstConfig.pstHistoryBank);

    /* Deletes stack bank */
    orxBank_Delete(sstConfig.pstStackBank);

    /* Deletes encryption key */
    orxConfig_SetEncryptionKey(orxNULL);

    /* Cleans control structure */
    orxMemory_Zero(&sstConfig, sizeof(orxCONFIG_STATIC));
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
    free(sstConfig.zEncryptionKey);
  }

  /* Has new key? */
  if((_zEncryptionKey != orxNULL) && (_zEncryptionKey != orxSTRING_EMPTY))
  {
    /* Updates values */
    sstConfig.u32EncryptionKeySize  = (orxU32)strlen(_zEncryptionKey);
    sstConfig.zEncryptionKey        = (orxSTRING)malloc((size_t)(sstConfig.u32EncryptionKeySize + 1));
    memcpy(sstConfig.zEncryptionKey, _zEncryptionKey, (size_t)(sstConfig.u32EncryptionKeySize + 1));
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

/** Gets encryption key
 * @return Current encryption key / orxSTRING_EMPTY
 */
const orxSTRING orxFASTCALL orxConfig_GetEncryptionKey()
{
  const orxSTRING zResult;

  /* Updates result */
  zResult = (sstConfig.zEncryptionKey != orxNULL) ? sstConfig.zEncryptionKey : orxSTRING_EMPTY;

  /* Done! */
  return zResult;
}

/** Sets config bootstrap function: this function will get called when the config menu is initialized, before any config file is loaded.
 *  The only available APIs within the bootstrap function are those of orxConfig and its dependencies (orxMemory, orxString, orxFile, orxEvent, orxResource, ...)
 * @param[in] _pfnBootstrap     Bootstrap function that will get called at module init, before loading any config file.
                                If this function returns orxSTATUS_FAILURE, the default config file will be skipped, otherwise the regular load sequence will happen
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_SetBootstrap(const orxCONFIG_BOOTSTRAP_FUNCTION _pfnBootstrap)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Stores it */
  sstConfig.pfnBootstrap = _pfnBootstrap;

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
    orxS32 s32Index, s32NextIndex;

    /* Finds last directory separator */
    for(s32Index = 0;
        (s32Index >= 0) && (((s32NextIndex = orxString_SearchCharIndex(_zBaseName, orxCHAR_DIRECTORY_SEPARATOR_WINDOWS, s32Index + 1)) > 0) || ((s32NextIndex = orxString_SearchCharIndex(_zBaseName, orxCHAR_DIRECTORY_SEPARATOR_LINUX, s32Index + 1)) > 0));
        s32Index = s32NextIndex)
    {
      /* Enforces native directory separator */
      *((orxSTRING)_zBaseName + s32NextIndex) = orxCHAR_DIRECTORY_SEPARATOR;
    }

    /* Found? */
    if(s32Index > 0)
    {
      /* Removes it */
      *((orxSTRING)_zBaseName + s32Index) = orxCHAR_NULL;

#ifdef __orxMSVC__

      /* Sets current directory */
      if(_chdir(_zBaseName) < 0)
      {
        /* Restores separator */
        *((orxSTRING)_zBaseName + s32Index) = orxCHAR_DIRECTORY_SEPARATOR;

        /* Clears index */
        s32Index = -1;
      }
      else
      {
        /* Restores separator */
        *((orxSTRING)_zBaseName + s32Index) = orxCHAR_DIRECTORY_SEPARATOR;
      }

#else /* __orxMSVC__ */

      /* Sets current directory */
      if(chdir(_zBaseName) < 0)
      {
        /* Restores separator */
        *((orxSTRING)_zBaseName + s32Index) = orxCHAR_DIRECTORY_SEPARATOR;

        /* Clears index */
        s32Index = -1;
      }
      else
      {
        /* Restores separator */
        *((orxSTRING)_zBaseName + s32Index) = orxCHAR_DIRECTORY_SEPARATOR;
      }

#endif /* __orxMSVC__ */
    }
    else
    {
      /* Clears index */
      s32Index = -1;
    }

    /* Gets config base file */
    orxString_NPrint(sstConfig.zBaseFile, orxCONFIG_KU32_BASE_FILENAME_LENGTH - 1, "%s.ini", _zBaseName + s32Index + 1);
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

/** Read config config from source.
 * @param[in] _zFileName        File name
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_Load(const orxSTRING _zFileName)
{
  orxSTRINGID     stPreviousLoadFileID;
  const orxSTRING zResourceLocation;
  orxHANDLE       hResource;
  orxSTATUS       eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zFileName != orxNULL);

  /* External call? */
  if(sstConfig.u32LoadCount == 0)
  {
    /* Profiles */
    orxPROFILER_PUSH_MARKER("orxConfig_Load");

    /* Should keep history? */
    if(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_HISTORY))
    {
      orxSTRINGID *pstFileID;

      /* Adds an history entry */
      pstFileID = (orxSTRINGID *)orxBank_Allocate(sstConfig.pstHistoryBank);

      /* Valid? */
      if(pstFileID != orxNULL)
      {
        /* Stores the file name */
        *pstFileID = orxString_GetID(_zFileName);
      }
    }
  }

  /* Updates load count */
  sstConfig.u32LoadCount++;

  /* Stores previously loaded file */
  stPreviousLoadFileID = sstConfig.stLoadFileID;

  /* Sets current loaded file */
  sstConfig.stLoadFileID = orxString_GetID(_zFileName);

  /* Valid file to open? */
  if((_zFileName != orxSTRING_EMPTY)
  && ((zResourceLocation = orxResource_Locate(orxCONFIG_KZ_RESOURCE_GROUP, _zFileName)) != orxNULL)
  && ((hResource = orxResource_Open(zResourceLocation, orxFALSE)) != orxHANDLE_UNDEFINED))
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
    for(u32Size = (orxU32)orxResource_Read(hResource, orxCONFIG_KU32_BUFFER_SIZE, acBuffer, orxNULL, orxNULL), u32Offset = 0, bFirstTime = orxTRUE;
        u32Size > 0;
        u32Size = (orxU32)orxResource_Read(hResource, (orxS64)(orxCONFIG_KU32_BUFFER_SIZE - u32Offset), acBuffer + u32Offset, orxNULL, orxNULL) + u32Offset, bFirstTime = orxFALSE)
    {
      /* First time? */
      if(bFirstTime != orxFALSE)
      {
        /* Has encryption tag? */
        if(orxString_NCompare(acBuffer, orxCONFIG_KZ_ENCRYPTION_TAG, orxCONFIG_KU32_ENCRYPTION_TAG_LENGTH) == 0)
        {
          /* Updates encryption status */
          bUseEncryption = orxTRUE;

          /* Updates offset */
          u32Offset = orxCONFIG_KU32_ENCRYPTION_TAG_LENGTH;

          /* Decrypts all new characters */
          orxConfig_CryptBuffer(acBuffer + u32Offset, u32Size - u32Offset);
        }
        else
        {
          /* Updates encryption status */
          bUseEncryption = orxFALSE;
        }

        /* Has UTF-8 BOM? */
        if(orxString_NCompare(acBuffer + u32Offset, orxCONFIG_KZ_UTF8_BOM, orxCONFIG_KU32_UTF8_BOM_LENGTH) == 0)
        {
          /* Skips it */
          u32Offset += orxCONFIG_KU32_UTF8_BOM_LENGTH;
        }
        else
        {
          orxU32 i;

          /* For all invalid BOMs */
          for(i = 0; i < sizeof(sastUnsupportedBOMList) / sizeof(struct __orxCONFIG_BOM_DEFINITION_t); i++)
          {
            /* Matches? */
            if(orxString_NCompare(acBuffer + u32Offset, sastUnsupportedBOMList[i].zBOM, sastUnsupportedBOMList[i].u32Length) == 0)
            {
              /* Logs message */
              orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "[%s]: Can't load config file, invalid text encoding. Only ANSI & UTF-8 are supported.", _zFileName);

              /* Updates result */
              eResult = orxSTATUS_FAILURE;

              break;
            }
          }
        }
      }

      /* Success? */
      if(eResult != orxSTATUS_FAILURE)
      {
        /* Not first time? */
        if(bFirstTime == orxFALSE)
        {
          /* Uses encryption? */
          if(bUseEncryption != orxFALSE)
          {
            /* Decrypts all new characters */
            orxConfig_CryptBuffer(acBuffer + u32Offset, u32Size - u32Offset);
          }

          /* Resets offset */
          u32Offset = 0;
        }

        /* End of file reached? */
        if(u32Size < orxCONFIG_KU32_BUFFER_SIZE)
        {
          /* Adds an extra EOL */
          acBuffer[u32Size++] = orxCHAR_LF;
        }

        /* Processes buffer */
        u32Offset = orxConfig_ProcessBuffer(_zFileName, acBuffer, u32Size, u32Offset);

        /* Should keep remainder of buffer? */
        if(u32Offset != 0)
        {
          /* Moves it at the beginning of the buffer */
          orxMemory_Move(acBuffer, acBuffer + u32Size - u32Offset, u32Offset);
          acBuffer[u32Offset] = orxCHAR_NULL;
        }
      }
    }

    /* Pops previous section */
    sstConfig.pstCurrentSection = pstPreviousSection;

    /* Restores previous encryption character */
    sstConfig.pcEncryptionChar = pcPreviousEncryptionChar;

    /* Closes file */
    orxResource_Close(hResource);
  }
  else
  {
    /* Logs */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "[%s]: Can't load file: invalid / file not found", _zFileName);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Restores previously loading file */
  sstConfig.stLoadFileID = stPreviousLoadFileID;

  /* Updates load count */
  sstConfig.u32LoadCount--;

  /* External call? */
  if(sstConfig.u32LoadCount == 0)
  {
    /* Optimizes the section table */
    orxHashTable_Optimize(sstConfig.pstSectionTable);

    /* Profiles */
    orxPROFILER_POP_MARKER();
  }

  /* Done! */
  return eResult;
}

/** Loads config data from a memory buffer. NB: the buffer will be modified during processing!
 * @param[in] _acBuffer         Buffer to process, will be modified during processing
 * @param[in] _u32BufferSize    Size of the buffer
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_LoadFromMemory(orxCHAR *_acBuffer, orxU32 _u32BufferSize)
{
  orxU32    u32Offset;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(_acBuffer != orxNULL);
  orxASSERT(_u32BufferSize > 0);

  /* Valid? */
  if((_acBuffer != orxNULL)
  && (_u32BufferSize > 0))
  {
    orxBOOL bProcess = orxTRUE;

    /* Has encryption tag? */
    if(orxString_NCompare(_acBuffer, orxCONFIG_KZ_ENCRYPTION_TAG, orxCONFIG_KU32_ENCRYPTION_TAG_LENGTH) == 0)
    {
      /* Updates offset */
      u32Offset = orxCONFIG_KU32_ENCRYPTION_TAG_LENGTH;

      /* Valid? */
      if(u32Offset < _u32BufferSize)
      {
        /* Decrypts all new characters */
        orxConfig_CryptBuffer(_acBuffer + u32Offset, _u32BufferSize - u32Offset);
      }
      else
      {
        /* Stops processing */
        bProcess = orxFALSE;
      }
    }
    else
    {
      /* Clears offset */
      u32Offset = 0;
    }

    /* Has UTF-8 BOM? */
    if(orxString_NCompare(_acBuffer + u32Offset, orxCONFIG_KZ_UTF8_BOM, orxCONFIG_KU32_UTF8_BOM_LENGTH) == 0)
    {
      /* Skips it */
      u32Offset += orxCONFIG_KU32_UTF8_BOM_LENGTH;
    }
    else
    {
      orxU32 i;

      /* For all invalid BOMs */
      for(i = 0; i < sizeof(sastUnsupportedBOMList) / sizeof(struct __orxCONFIG_BOM_DEFINITION_t); i++)
      {
        /* Matches? */
        if(orxString_NCompare(_acBuffer + u32Offset, sastUnsupportedBOMList[i].zBOM, sastUnsupportedBOMList[i].u32Length) == 0)
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Can't load memory buffer, invalid text encoding. Only ANSI & UTF-8 are supported.");

          /* Don't process */
          bProcess = orxFALSE;

          break;
        }
      }
    }

    /* Success? */
    if((bProcess != orxFALSE)
    && (u32Offset <= _u32BufferSize))
    {
      /* Loads it */
      if(orxConfig_ProcessBuffer(orxCONFIG_KZ_CONFIG_MEMORY, _acBuffer, _u32BufferSize, u32Offset) == 0)
      {
        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Reloads config files from history
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_ReloadHistory()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));

  /* Has history? */
  if(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_HISTORY))
  {
    orxU32 *pu32HistoryEntry;

    /* Sends event */
    orxEvent_SendShort(orxEVENT_TYPE_CONFIG, orxCONFIG_EVENT_RELOAD_START);

    /* Removes history flag */
    orxFLAG_SET(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_NONE, orxCONFIG_KU32_STATIC_FLAG_HISTORY);

    /* Clears all data */
    orxConfig_Clear();

    /* Reloads default file */
    eResult = orxConfig_Load(sstConfig.zBaseFile);

    /* Logs */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "[%s]: Config file has been reloaded", sstConfig.zBaseFile);

    /* For all entries in history */
    for(pu32HistoryEntry = (orxU32 *)orxBank_GetNext(sstConfig.pstHistoryBank, orxNULL);
        (pu32HistoryEntry != orxNULL) && (eResult != orxSTATUS_FAILURE);
        pu32HistoryEntry = (orxU32 *)orxBank_GetNext(sstConfig.pstHistoryBank, pu32HistoryEntry))
    {
      const orxSTRING zFileName;

      /* Gets its filename */
      zFileName = orxString_GetFromID(*pu32HistoryEntry);

      /* Reloads it */
      eResult = orxConfig_Load(zFileName);

      /* Logs */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "[%s]: Config file has been reloaded", zFileName);
    }

    /* Restores history flag */
    orxFLAG_SET(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_HISTORY, orxCONFIG_KU32_STATIC_FLAG_NONE);

    /* Sends event */
    orxEvent_SendShort(orxEVENT_TYPE_CONFIG, orxCONFIG_EVENT_RELOAD_STOP);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Config history isn't stored. Please check your config file under the [Config] section");

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
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxConfig_Save");

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));

  /* No encryption requested or has a valid key? */
  if((_bUseEncryption == orxFALSE) || (sstConfig.zEncryptionKey != orxNULL))
  {
    const orxSTRING zResourceLocation;
    const orxSTRING zFileName;
    orxHANDLE       hResource;

    /* Is given an invalid file name? */
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

    /* Valid file to open? */
    if(((zResourceLocation = orxResource_LocateInStorage(orxCONFIG_KZ_RESOURCE_GROUP, orxNULL, zFileName)) != orxNULL)
    && ((hResource = orxResource_Open(zResourceLocation, orxTRUE)) != orxHANDLE_UNDEFINED))
    {
      orxCONFIG_SECTION  *pstSection;
      orxCHAR             acBuffer[orxCONFIG_KU32_BUFFER_SIZE], *pcPreviousEncryptionChar = orxNULL;
      orxU32              u32BufferSize;

      /* Inits buffer */
      acBuffer[orxCONFIG_KU32_BUFFER_SIZE - 1] = orxCHAR_NULL;

      /* Use encryption? */
      if(_bUseEncryption != orxFALSE)
      {
        /* Gets previous encryption character */
        pcPreviousEncryptionChar = sstConfig.pcEncryptionChar;

        /* Resets current encryption character */
        sstConfig.pcEncryptionChar = sstConfig.zEncryptionKey;

        /* Adds encryption tag */
        orxResource_Write(hResource, orxCONFIG_KU32_ENCRYPTION_TAG_LENGTH, orxCONFIG_KZ_ENCRYPTION_TAG, orxNULL, orxNULL);
      }

      /* For all sections */
      for(pstSection = (orxCONFIG_SECTION *)orxLinkList_GetFirst(&(sstConfig.stSectionList));
          pstSection != orxNULL;
          pstSection = (orxCONFIG_SECTION *)orxLinkList_GetNext(&(pstSection->stNode)))
      {
        /* No callback or should save it? */
        if((_pfnSaveCallback == orxNULL) || (_pfnSaveCallback(pstSection->zName, orxNULL, zFileName, _bUseEncryption) != orxFALSE))
        {
          orxCONFIG_ENTRY *pstEntry;

          /* Forces default section? */
          if(pstSection->pstParent == orxHANDLE_UNDEFINED)
          {
            /* Writes section name */
            u32BufferSize = (orxU32)orxString_NPrint(acBuffer, orxCONFIG_KU32_BUFFER_SIZE - 1, "%c%s%c%c%c%s", orxCONFIG_KC_SECTION_START, pstSection->zName, orxCONFIG_KC_INHERITANCE_MARKER, orxCONFIG_KC_INHERITANCE_MARKER, orxCONFIG_KC_SECTION_END, orxSTRING_EOL);
          }
          /* Has a valid parent section */
          else if(pstSection->pstParent != orxNULL)
          {
            /* Writes section name with inheritance */
            u32BufferSize = (orxU32)orxString_NPrint(acBuffer, orxCONFIG_KU32_BUFFER_SIZE - 1, "%c%s%c%s%c%s", orxCONFIG_KC_SECTION_START, pstSection->zName, orxCONFIG_KC_INHERITANCE_MARKER, pstSection->pstParent->zName, orxCONFIG_KC_SECTION_END, orxSTRING_EOL);
          }
          else
          {
            /* Writes section name */
            u32BufferSize = (orxU32)orxString_NPrint(acBuffer, orxCONFIG_KU32_BUFFER_SIZE - 1, "%c%s%c%s", orxCONFIG_KC_SECTION_START, pstSection->zName, orxCONFIG_KC_SECTION_END, orxSTRING_EOL);
          }

          /* Encrypt? */
          if(_bUseEncryption != orxFALSE)
          {
            /* Encrypts buffer */
            orxConfig_CryptBuffer(acBuffer, u32BufferSize);
          }

          /* Saves it */
          orxResource_Write(hResource, (orxS64)u32BufferSize, acBuffer, orxNULL, orxNULL);

          /* For all entries */
          for(pstEntry = (orxCONFIG_ENTRY *)orxLinkList_GetFirst(&(pstSection->stEntryList));
              pstEntry != orxNULL;
              pstEntry = (orxCONFIG_ENTRY *)orxLinkList_GetNext(&(pstEntry->stNode)))
          {
            const orxSTRING zKey;

            /* Gets key */
            zKey = orxString_GetFromID(pstEntry->stID);

            /* No callback or should save it? */
            if((_pfnSaveCallback == orxNULL) || (_pfnSaveCallback(pstSection->zName, zKey, zFileName, _bUseEncryption) != orxFALSE))
            {
              /* Not in block mode? */
              if(!orxFLAG_TEST(pstEntry->stValue.u16Flags, orxCONFIG_VALUE_KU16_FLAG_BLOCK_MODE))
              {
                /* Writes it */
                u32BufferSize = (orxU32)orxString_NPrint(acBuffer, orxCONFIG_KU32_BUFFER_SIZE - 1, "%s %c %s", zKey, orxCONFIG_KC_ASSIGN, pstEntry->stValue.zValue);

                /* Is a list? */
                if(orxFLAG_TEST(pstEntry->stValue.u16Flags, orxCONFIG_VALUE_KU16_FLAG_LIST))
                {
                  orxCHAR  *pcSrc;
                  orxU32    i;

                  /* For all remaining items */
                  for(i = (orxU32)(pstEntry->stValue.u16ListCount - 1), pcSrc = pstEntry->stValue.zValue + orxString_GetLength(pstEntry->stValue.zValue);
                      i > 0;
                      i--)
                  {
                    orxCHAR *pcDst;

                    /* Skips all 'hidden separators' */
                    for(; *pcSrc == orxCHAR_NULL; pcSrc++);

                    /* Writes separator */
                    u32BufferSize += (orxU32)orxString_NPrint(acBuffer + u32BufferSize, orxCONFIG_KU32_BUFFER_SIZE - 1 - u32BufferSize, " %c ", orxCONFIG_KC_LIST_SEPARATOR);

                    /* For all characters */
                    for(pcDst = acBuffer + u32BufferSize; (*pcSrc != orxCHAR_NULL) && (pcDst < acBuffer + orxCONFIG_KU32_BUFFER_SIZE - 1); pcSrc++, pcDst++, u32BufferSize++)
                    {
                      /* Copies it */
                      *pcDst = *pcSrc;
                    }
                  }
                }

                /* Writes EOL */
                u32BufferSize += (orxU32)orxString_NPrint(acBuffer + u32BufferSize, orxCONFIG_KU32_BUFFER_SIZE - 1 - u32BufferSize, "%s", orxSTRING_EOL);
              }
              else
              {
                orxCHAR *pcSrc, *pcDst;

                /* Writes lead in */
                u32BufferSize = (orxU32)orxString_NPrint(acBuffer, orxCONFIG_KU32_BUFFER_SIZE - 1, "%s %c %c", zKey, orxCONFIG_KC_ASSIGN, orxCONFIG_KC_BLOCK);

                /* For all characters */
                for(pcSrc = pstEntry->stValue.zValue, pcDst = acBuffer + u32BufferSize; (*pcSrc != orxCHAR_NULL) && (pcDst < acBuffer + orxCONFIG_KU32_BUFFER_SIZE - 1); pcSrc++, pcDst++, u32BufferSize++)
                {
                  /* Copies character */
                  *pcDst = *pcSrc;

                  /* Is a block marker? */
                  if(*pcSrc == orxCONFIG_KC_BLOCK)
                  {
                    /* Doubles it */
                    *(++pcDst) = orxCONFIG_KC_BLOCK;
                    u32BufferSize++;
                  }
                }

                /* Writes lead out */
                u32BufferSize += (orxU32)orxString_NPrint(pcDst, orxCONFIG_KU32_BUFFER_SIZE - 1 - u32BufferSize, "%c%s", orxCONFIG_KC_BLOCK, orxSTRING_EOL);
              }

              /* Encrypt? */
              if(_bUseEncryption != orxFALSE)
              {
                /* Encrypts buffer */
                orxConfig_CryptBuffer(acBuffer, u32BufferSize);
              }

              /* Saves it */
              orxResource_Write(hResource, (orxS64)u32BufferSize, acBuffer, orxNULL, orxNULL);
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
          orxResource_Write(hResource, (orxS64)u32BufferSize, acBuffer, orxNULL, orxNULL);
        }
      }

      /* Flushes & closes the resource */
      orxResource_Close(hResource);

      /* Use encryption? */
      if(_bUseEncryption != orxFALSE)
      {
        /* Restores previous encryption character */
        sstConfig.pcEncryptionChar = pcPreviousEncryptionChar;
      }

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "[%s]: Can't save file, can't open file on disk!", zFileName);
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "[%s]: Can't save file with encryption, no valid encryption key provided!", _zFileName);
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return eResult;
}

/** Copies a file with optional encryption
 * @param[in] _zDstFileName     Name of the destination file
 * @param[in] _zSrcFileName     Name of the source file
 * @param[in] _zEncryptionKey   Encryption key to use when writing destination file, orxNULL for no encryption
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_CopyFile(const orxSTRING _zDstFileName, const orxSTRING _zSrcFileName, const orxSTRING _zEncryptionKey)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zDstFileName != orxNULL);
  orxASSERT(_zSrcFileName != orxNULL);
  orxASSERT(*_zDstFileName != orxCHAR_NULL);
  orxASSERT(*_zSrcFileName != orxCHAR_NULL);

  /* Valid names */
  if(orxString_Compare(_zDstFileName, _zSrcFileName) != 0)
  {
    /* Copies file */
    eResult = orxConfig_MergeFiles(_zDstFileName, &_zSrcFileName, 1, _zEncryptionKey);
  }

  /* Done! */
  return eResult;
}

/** Merges multiple files into a single one, with optional encryption
 * @param[in] _zDstFileName     Name of the destination file
 * @param[in] _azSrcFileName    List of the names of the source files
 * @param[in] _u32Number        Number of source file names
 * @param[in] _zEncryptionKey   Encryption key to use when writing destination file, orxNULL for no encryption
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_MergeFiles(const orxSTRING _zDstFileName, const orxSTRING *_azSrcFileName, orxU32 _u32Number, const orxSTRING _zEncryptionKey)
{
  orxFILE  *pstDstFile;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zDstFileName != orxNULL);
  orxASSERT(_azSrcFileName != orxNULL);
  orxASSERT(*_zDstFileName != orxCHAR_NULL);
  orxASSERT(*_azSrcFileName != orxNULL);
  orxASSERT(_u32Number > 0);

  /* Opens destination file */
  pstDstFile = orxFile_Open(_zDstFileName, orxFILE_KU32_FLAG_OPEN_WRITE | orxFILE_KU32_FLAG_OPEN_BINARY);

  /* Success? */
  if(pstDstFile != orxNULL)
  {
    orxCHAR  *pcEncryptionCharBackup, *pcEncryptionChar;
    orxSTRING zPreviousEncryptionKey;
    orxU32    u32PreviousEncryptionKeySize, i;

    /* Backups encryption char */
    pcEncryptionCharBackup = sstConfig.pcEncryptionChar;

    /* Uses encryption? */
    if(_zEncryptionKey != orxNULL)
    {
      /* Stores previous encryption key */
      zPreviousEncryptionKey        = sstConfig.zEncryptionKey;
      u32PreviousEncryptionKeySize  = sstConfig.u32EncryptionKeySize;

      /* Inits encryption char */
      pcEncryptionChar = (orxSTRING)_zEncryptionKey;

      /* Adds encryption tag */
      orxFile_Print(pstDstFile, "%s", orxCONFIG_KZ_ENCRYPTION_TAG);
    }

    /* For all source files */
    for(i = 0; i < _u32Number; i++)
    {
      const orxSTRING zSrcFileName;

      /* Gets its name */
      zSrcFileName = _azSrcFileName[i];

      /* Valid names */
      if(orxString_Compare(_zDstFileName, zSrcFileName) != 0)
      {
        orxFILE *pstSrcFile;

        /* Opens source file */
        pstSrcFile = orxFile_Open(zSrcFileName, orxFILE_KU32_FLAG_OPEN_READ | orxFILE_KU32_FLAG_OPEN_BINARY);

        /* Valid? */
        if(pstSrcFile != orxNULL)
        {
          orxCHAR acBuffer[orxCONFIG_KU32_BUFFER_SIZE];
          orxU32  u32Size;
          orxBOOL bFirstTime, bUseEncryption;

          /* While source file isn't empty */
          for(u32Size = (orxU32)orxFile_Read(acBuffer, sizeof(orxCHAR), orxCONFIG_KU32_BUFFER_SIZE, pstSrcFile), bFirstTime = orxTRUE, bUseEncryption = orxFALSE;
              u32Size > 0;
              u32Size = (orxU32)orxFile_Read(acBuffer, sizeof(orxCHAR), orxCONFIG_KU32_BUFFER_SIZE, pstSrcFile), bFirstTime = orxFALSE)
          {
            /* First time? */
            if(bFirstTime != orxFALSE)
            {
              /* Has encryption tag? */
              if(orxString_NCompare(acBuffer, orxCONFIG_KZ_ENCRYPTION_TAG, orxCONFIG_KU32_ENCRYPTION_TAG_LENGTH) == 0)
              {
                /* Uses encryption */
                bUseEncryption = orxTRUE;

                /* Reinits encryption char */
                sstConfig.pcEncryptionChar = sstConfig.zEncryptionKey;

                /* Decrypts remaining buffer */
                orxConfig_CryptBuffer(acBuffer + orxCONFIG_KU32_ENCRYPTION_TAG_LENGTH, u32Size - orxCONFIG_KU32_ENCRYPTION_TAG_LENGTH);
              }
            }
            else
            {
              /* Uses encryption */
              if(bUseEncryption != orxFALSE)
              {
                /* Decrypts buffer */
                orxConfig_CryptBuffer(acBuffer, u32Size);
              }
            }

            /* Use encryption for destination? */
            if(_zEncryptionKey != orxNULL)
            {
              orxCHAR *pcPreviousEncryptionChar;

              /* Stores previous encryption char */
              pcPreviousEncryptionChar = sstConfig.pcEncryptionChar;

              /* Stores new encryption key */
              sstConfig.zEncryptionKey        = (orxSTRING)_zEncryptionKey;
              sstConfig.u32EncryptionKeySize  = orxString_GetLength(_zEncryptionKey);

              /* Sets encryption char */
              sstConfig.pcEncryptionChar = pcEncryptionChar;

              /* Encrypts buffer */
              if((bFirstTime != orxFALSE) && (bUseEncryption != orxFALSE))
              {
                orxConfig_CryptBuffer(acBuffer + orxCONFIG_KU32_ENCRYPTION_TAG_LENGTH, u32Size - orxCONFIG_KU32_ENCRYPTION_TAG_LENGTH);
              }
              else
              {
                orxConfig_CryptBuffer(acBuffer, u32Size);
              }

              /* Saves encryption char */
              pcEncryptionChar = sstConfig.pcEncryptionChar;

              /* Restores encryption key */
              sstConfig.zEncryptionKey        = zPreviousEncryptionKey;
              sstConfig.u32EncryptionKeySize  = u32PreviousEncryptionKeySize;

              /* Restores encryption char */
              sstConfig.pcEncryptionChar = pcPreviousEncryptionChar;
            }

            /* Writes buffer */
            if((bFirstTime != orxFALSE) && (bUseEncryption != orxFALSE))
            {
              orxFile_Write(acBuffer + orxCONFIG_KU32_ENCRYPTION_TAG_LENGTH, sizeof(orxCHAR), (orxS64)(u32Size - orxCONFIG_KU32_ENCRYPTION_TAG_LENGTH), pstDstFile);
            }
            else
            {
              orxFile_Write(acBuffer, sizeof(orxCHAR), (orxS64)u32Size, pstDstFile);
            }
          }

          /* Closes source file */
          orxFile_Close(pstSrcFile);
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Can't merge config file <%s> -> <%s>: can't open source file.", zSrcFileName, _zDstFileName);

          /* Updates result */
          eResult = orxSTATUS_FAILURE;
          break;
        }
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Can't merge merge file <%s> -> <%s>: source and destination must be different.", zSrcFileName, _zDstFileName);

        /* Updates result */
        eResult = orxSTATUS_FAILURE;
        break;
      }
    }

    /* Closes destination file */
    orxFile_Close(pstDstFile);

    /* Restores previous encryption char */
    sstConfig.pcEncryptionChar = pcEncryptionCharBackup;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Can't merge config files -> <%s>: can't open destination file.", _zDstFileName);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Selects current working section
 * @param[in] _zSectionName     Section name to select
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_SelectSection(const orxSTRING _zSectionName)
{
  orxSTATUS eResult;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxConfig_SelectSection");

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zSectionName != orxNULL);

  /* Selects section */
  eResult = orxConfig_SelectSectionInternal(_zSectionName);

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return eResult;
}

/** Renames a section
 * @param[in] _zSectionName     Section to rename
 * @param[in] _zNewSectionName  New name for the section
 */
orxSTATUS orxFASTCALL orxConfig_RenameSection(const orxSTRING _zSectionName, const orxSTRING _zNewSectionName)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zSectionName != orxNULL);
  orxASSERT(_zNewSectionName != orxNULL);

  /* Valid? */
  if((_zSectionName != orxSTRING_EMPTY) && (_zNewSectionName != orxSTRING_EMPTY))
  {
    /* No inheritance markers? */
    if((orxString_SearchCharIndex(_zSectionName, orxCONFIG_KC_INHERITANCE_MARKER, 0) < 0)
    && (orxString_SearchCharIndex(_zNewSectionName, orxCONFIG_KC_INHERITANCE_MARKER, 0) < 0))
    {
      orxCONFIG_SECTION  *pstSection;
      orxSTRINGID         stID;

      /* Gets section name ID */
      stID = orxString_ToCRC(_zSectionName);

      /* Gets it from table */
      pstSection = (orxCONFIG_SECTION *)orxHashTable_Get(sstConfig.pstSectionTable, stID);

      /* Valid? */
      if(pstSection != orxNULL)
      {
        const orxSTRING zNewName;

        /* Get new name */
        zNewName = orxString_Store(_zNewSectionName);

        /* Stores it */
        pstSection->zName = zNewName;

        /* Removes it from table */
        orxHashTable_Remove(sstConfig.pstSectionTable, stID);

        /* Adds it again with the new ID */
        orxHashTable_Add(sstConfig.pstSectionTable, orxString_ToCRC(_zNewSectionName), pstSection);

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Can't rename section [%s] -> <%s>: section not found.", _zSectionName, _zNewSectionName);

        /* Updates result */
        eResult = orxSTATUS_FAILURE;
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Can't rename section [%s] -> <%s>: inheritance marker detected.", _zSectionName, _zNewSectionName);

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Can't rename section [%s] -> <%s>: empty name.", _zSectionName, _zNewSectionName);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets a section's parent
 * @param[in] _zSectionName     Concerned section, if the section doesn't exist, it will be created
 * @param[in] _zParentName      Parent section's name, if the section doesn't exist, it will be created, if orxNULL is provided, the former parent will be erased, if orxSTRING_EMPTY is provided, "no default parent" will be enforced
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
    /* Already has a parent? */
    if((sstConfig.pstCurrentSection->pstParent != orxNULL) && (sstConfig.pstCurrentSection->pstParent != orxHANDLE_UNDEFINED))
    {
      /* Unprotects it */
      sstConfig.pstCurrentSection->pstParent->s32ProtectionCount--;

      /* Checks */
      orxASSERT(sstConfig.pstCurrentSection->pstParent->s32ProtectionCount >= 0);

      /* Removes its reference */
      sstConfig.pstCurrentSection->pstParent = orxNULL;
    }

    /* Force no parent? */
    if(_zParentName == orxSTRING_EMPTY)
    {
      /* Updates section */
      sstConfig.pstCurrentSection->pstParent = (orxCONFIG_SECTION *)orxHANDLE_UNDEFINED;
    }
    /* Has new parent? */
    else if(_zParentName != orxNULL)
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
        pstSection->pstParent = sstConfig.pstCurrentSection;

        /* Protects it */
        sstConfig.pstCurrentSection->s32ProtectionCount++;
      }
    }
    else
    {
      /* Clears its parent */
      sstConfig.pstCurrentSection->pstParent = orxNULL;
    }
  }

  /* Restores previous section */
  sstConfig.pstCurrentSection = pstPreviousSection;

  /* Done! */
  return eResult;
}

/** Gets a section's parent
 * @param[in] _zSectionName     Concerned section
 * @return Section's parent name if set or orxSTRING_EMPTY if no parent has been forced, orxNULL otherwise
 */
const orxSTRING orxFASTCALL orxConfig_GetParent(const orxSTRING _zSectionName)
{
  const orxSTRING zResult = orxNULL;

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
      if(sstConfig.pstCurrentSection->pstParent != orxNULL)
      {
        /* Updates result */
        zResult = (sstConfig.pstCurrentSection->pstParent != orxHANDLE_UNDEFINED) ? sstConfig.pstCurrentSection->pstParent->zName : orxSTRING_EMPTY;
      }
    }

    /* Restores previous section */
    sstConfig.pstCurrentSection = pstPreviousSection;
  }

  /* Done! */
  return zResult;
}

/** Sets default parent for all sections
* @param[in] _zSectionName     Section name that will be used as an implicit default parent section for all config sections, if orxNULL is provided, default parent will be removed
* @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
*/
orxSTATUS orxFASTCALL orxConfig_SetDefaultParent(const orxSTRING _zSectionName)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));

  /* Has current default parent? */
  if(sstConfig.pstDefaultSection != orxNULL)
  {
    /* Unprotects it */
    sstConfig.pstDefaultSection->s32ProtectionCount--;

    /* Checks */
    orxASSERT(sstConfig.pstDefaultSection->s32ProtectionCount >= 0);

    /* Clears it */
    sstConfig.pstDefaultSection = orxNULL;
  }

  /* Valid? */
  if((_zSectionName != orxNULL) && (_zSectionName != orxSTRING_EMPTY))
  {
    orxCONFIG_SECTION *pstPreviousSection;

    /* Backups current section */
    pstPreviousSection = sstConfig.pstCurrentSection;

    /* Selects default parent section */
    eResult = orxConfig_SelectSection(_zSectionName);

    /* Success? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Checks */
      orxASSERT(sstConfig.pstCurrentSection != orxNULL);

      /* Protects it */
      sstConfig.pstCurrentSection->s32ProtectionCount++;

      /* Stores it */
      sstConfig.pstDefaultSection = sstConfig.pstCurrentSection;
    }

    /* Restores current section */
    sstConfig.pstCurrentSection = pstPreviousSection;
  }

  /* Done! */
  return eResult;
}

/** Gets current working section
 * @return Current selected section
 */
const orxSTRING orxFASTCALL orxConfig_GetCurrentSection()
{
  const orxSTRING zResult;

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
        /* Clears it */
        orxMemory_Zero(pstStackEntry, sizeof(orxCONFIG_STACK_ENTRY));

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
  if(orxLinkList_GetCount(&(sstConfig.stStackList)) > 0)
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

/** Has section for the given section name?
 * @param[in] _zSectionName     Section name
 * @return orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxConfig_HasSection(const orxSTRING _zSectionName)
{
  orxSTRINGID stID;
  orxBOOL     bResult = orxFALSE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zSectionName != orxNULL);

  /* Valid? */
  if(_zSectionName != orxSTRING_EMPTY)
  {
    orxCONFIG_SECTION *pstSection;

    /* Gets section name ID */
    stID = orxString_ToCRC(_zSectionName);

    /* Gets it from table */
    pstSection = (orxCONFIG_SECTION *)orxHashTable_Get(sstConfig.pstSectionTable, stID);

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
  orxSTRINGID         stID;
  orxSTATUS           eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zSectionName != orxNULL);

  /* Gets section name ID */
  stID = orxString_ToCRC(_zSectionName);

  /* Gets it from table */
  pstSection = (orxCONFIG_SECTION *)orxHashTable_Get(sstConfig.pstSectionTable, stID);

  /* Valid? */
  if(pstSection != orxNULL)
  {
    /* Updates protection count */
    pstSection->s32ProtectionCount += (_bProtect != orxFALSE) ? 1 : -1;

    /* Checks */
    orxASSERT(pstSection->s32ProtectionCount >= 0);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Gets section origin (ie. the file where it was defined for the first time or orxSTRING_EMPTY if not defined via a file)
 * @param[in] _zSectionName     Concerned section name
 * @return orxSTRING if found, orxSTRING_EMPTY otherwise
 */
const orxSTRING orxFASTCALL orxConfig_GetOrigin(const orxSTRING _zSectionName)
{
  orxSTRINGID     stID;
  const orxSTRING zResult = orxSTRING_EMPTY;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zSectionName != orxNULL);

  /* Gets origin ID */
  stID = orxConfig_GetOriginID(_zSectionName);

  /* Valid? */
  if(stID != orxSTRINGID_UNDEFINED)
  {
    /* Updates result */
    zResult = orxString_GetFromID(stID);
  }

  /* Done! */
  return zResult;
}

/** Gets section origin ID (ie. the file where it was defined for the first time or orxSTRING_EMPTY if not defined via a file)
 * @param[in] _zSectionName     Concerned section name
 * @return String ID if found, orxSTRINGID_UNDEFINED otherwise
 */
orxSTRINGID orxFASTCALL orxConfig_GetOriginID(const orxSTRING _zSectionName)
{
  orxCONFIG_SECTION  *pstSection;
  orxSTRINGID         stID, stResult = orxSTRINGID_UNDEFINED;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zSectionName != orxNULL);

  /* Gets section name ID */
  stID = orxString_ToCRC(_zSectionName);

  /* Gets it from table */
  pstSection = (orxCONFIG_SECTION *)orxHashTable_Get(sstConfig.pstSectionTable, stID);

  /* Valid? */
  if(pstSection != orxNULL)
  {
    /* Updates result */
    stResult = pstSection->stOriginID;
  }

  /* Done! */
  return stResult;
}

/** Gets section count
 * @return Section count
 */
orxU32 orxFASTCALL orxConfig_GetSectionCount()
{
  orxU32 u32Result;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));

  /* Updates result */
  u32Result = orxLinkList_GetCount(&(sstConfig.stSectionList));

  /* Done! */
  return u32Result;
}

/** Gets section at the given index
 * @param[in] _u32SectionIndex  Index of the desired section
 * @return orxSTRING if exist, orxSTRING_EMPTY otherwise
 */
const orxSTRING orxFASTCALL orxConfig_GetSection(orxU32 _u32SectionIndex)
{
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));

  /* Valid? */
  if(_u32SectionIndex < orxConfig_GetSectionCount())
  {
    orxCONFIG_SECTION  *pstSection;
    orxU32              i;

    /* Finds correct entry */
    for(i = _u32SectionIndex, pstSection = (orxCONFIG_SECTION *)orxLinkList_GetFirst(&(sstConfig.stSectionList));
        i > 0;
        i--, pstSection = (orxCONFIG_SECTION *)orxLinkList_GetNext(&(pstSection->stNode)));

    /* Updates result */
    zResult = pstSection->zName;
  }
  else
  {
    /* Updates result */
    zResult = orxSTRING_EMPTY;
  }

  /* Done! */
  return zResult;
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
    orxASSERT(pstNewSection->s32ProtectionCount >= 0);

    /* Protected? */
    if(pstNewSection->s32ProtectionCount > 0)
    {
      /* Updates last section */
      pstLastSection = pstNewSection;
    }
    else
    {
      /* Deletes section */
      orxConfig_DeleteSection(pstNewSection);

      /* Clears last section */
      pstLastSection = orxNULL;
    }
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
  orxSTRINGID         stID;
  orxSTATUS           eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zSectionName != orxNULL);
  orxASSERT(_zSectionName != orxSTRING_EMPTY);

  /* Gets section name ID */
  stID = orxString_ToCRC(_zSectionName);

  /* Gets it from table */
  pstSection = (orxCONFIG_SECTION *)orxHashTable_Get(sstConfig.pstSectionTable, stID);

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
    orxConfig_DeleteEntry(pstEntry);

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

/** Is this value locally inherited from another one (ie. with a Value = @... syntax)?
 * @param[in] _zKey             Key name
 * @return orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxConfig_IsLocallyInheritedValue(const orxSTRING _zKey)
{
  orxCONFIG_ENTRY  *pstEntry;
  orxSTRINGID       stKeyID;
  orxBOOL           bResult = orxFALSE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);

  /* Gets ID */
  stKeyID = orxString_ToCRC(_zKey);

  /* Gets corresponding entry */
  pstEntry = orxConfig_GetEntryFromKey(stKeyID);

  /* Valid? */
  if(pstEntry != orxNULL)
  {
    /* Has local inheritance? */
    if(orxFLAG_TEST(pstEntry->stValue.u16Flags, orxCONFIG_VALUE_KU16_FLAG_INHERITANCE))
    {
      /* Updates result */
      bResult = orxTRUE;
    }
  }

  /* Done! */
  return bResult;
}

/** Is this value inherited from another one?
 * @param[in] _zKey             Key name
 * @return orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxConfig_IsInheritedValue(const orxSTRING _zKey)
{
  orxCONFIG_ENTRY  *pstEntry;
  orxSTRINGID       stKeyID;
  orxBOOL           bResult = orxFALSE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);

  /* Gets ID */
  stKeyID = orxString_ToCRC(_zKey);

  /* Gets corresponding entry */
  pstEntry = orxConfig_GetEntry(stKeyID);

  /* Valid? */
  if(pstEntry != orxNULL)
  {
    /* Has local inheritance? */
    if(orxFLAG_TEST(pstEntry->stValue.u16Flags, orxCONFIG_VALUE_KU16_FLAG_INHERITANCE))
    {
      /* Updates result */
      bResult = orxTRUE;
    }
  }
  else
  {
    orxCONFIG_SECTION *pstDummy = orxNULL;

    /* Updates result */
    bResult = (orxConfig_GetValueFromKey(stKeyID, sstConfig.pstCurrentSection, &pstDummy) != orxNULL) ? orxTRUE : orxFALSE;
  }

  /* Done! */
  return bResult;
}

/** Is this value random? (ie. using '~' character, within or without a list)
 * @param[in] _zKey             Key name
 * @return orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxConfig_IsRandomValue(const orxSTRING _zKey)
{
  orxCONFIG_VALUE  *pstValue;
  orxBOOL           bResult = orxFALSE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);

  /* Gets corresponding value */
  pstValue = orxConfig_GetValue(_zKey);

  /* Valid? */
  if(pstValue != orxNULL)
  {
    /* Has random flag? */
    if(orxFLAG_TEST(pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_RANDOM))
    {
      /* Updates result */
      bResult = orxTRUE;
    }
  }

  /* Done! */
  return bResult;
}

/** Is this value dynamic? (ie. random and/or a list or command)
 * @param[in] _zKey             Key name
 * @return orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxConfig_IsDynamicValue(const orxSTRING _zKey)
{
  orxCONFIG_VALUE  *pstValue;
  orxBOOL           bResult = orxFALSE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);

  /* Gets corresponding value */
  pstValue = orxConfig_GetValue(_zKey);

  /* Valid? */
  if(pstValue != orxNULL)
  {
    /* Has random, list or command flag? */
    if(orxFLAG_TEST(pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_RANDOM | orxCONFIG_VALUE_KU16_FLAG_LIST | orxCONFIG_VALUE_KU16_FLAG_COMMAND))
    {
      /* Updates result */
      bResult = orxTRUE;
    }
  }

  /* Done! */
  return bResult;
}

/** Is this a command value? (ie. lazily evaluated command: %...)
* @param[in] _zKey             Key name
* @return orxTRUE / orxFALSE
*/
orxBOOL orxFASTCALL orxConfig_IsCommandValue(const orxSTRING _zKey)
{
  orxCONFIG_VALUE  *pstValue;
  orxBOOL           bResult = orxFALSE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);

  /* Gets corresponding value */
  pstValue = orxConfig_GetValue(_zKey);

  /* Valid? */
  if(pstValue != orxNULL)
  {
    /* Is a command? */
    if(orxFLAG_TEST(pstValue->u16Flags, orxCONFIG_VALUE_KU16_FLAG_COMMAND))
    {
      /* Updates result */
      bResult = orxTRUE;
    }
  }

  /* Done! */
  return bResult;
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

/** Gets a value's source section (ie. the section where the value is explicitly defined), only considering section inheritance, not local one
 * @param[in] _zKey             Key name
 * @return Name of the section that explicitly contains the value, orxSTRING_EMPTY if not found
 */
const orxSTRING orxFASTCALL orxConfig_GetValueSource(const orxSTRING _zKey)
{
  orxCONFIG_ENTRY  *pstEntry;
  orxSTRINGID       stKeyID;
  const orxSTRING   zResult = orxSTRING_EMPTY;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);

  /* Gets ID */
  stKeyID = orxString_ToCRC(_zKey);

  /* Gets corresponding entry */
  pstEntry = orxConfig_GetEntry(stKeyID);

  /* Valid? */
  if(pstEntry != orxNULL)
  {
    /* Updates result */
    zResult = sstConfig.pstCurrentSection->zName;
  }
  else
  {
    orxCONFIG_SECTION *pstSource = orxNULL;

    /* Gets value from key */
    if(orxConfig_GetValueFromKey(stKeyID, sstConfig.pstCurrentSection, &pstSource) != orxNULL)
    {
      /* Updates result */
      zResult = pstSource->zName;
    }
  }

  /* Done! */
  return zResult;
}

/** Reads a signed integer value from config (will take a random value if a list is provided for this key)
 * @param[in] _zKey             Key name
 * @return The value
 */
orxS32 orxFASTCALL orxConfig_GetS32(const orxSTRING _zKey)
{
  orxCONFIG_VALUE  *pstValue;
  orxS32            s32Result = 0;

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
    orxConfig_GetS32FromValue(pstValue, -1, &s32Result);
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
  orxU32            u32Result = 0;

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
    orxConfig_GetU32FromValue(pstValue, -1, &u32Result);
  }

  /* Done! */
  return u32Result;
}

/** Reads a signed integer value from config (will take a random value if a list is provided for this key)
 * @param[in] _zKey             Key name
 * @return The value
 */
orxS64 orxFASTCALL orxConfig_GetS64(const orxSTRING _zKey)
{
  orxCONFIG_VALUE  *pstValue;
  orxS64            s64Result = 0;

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
    orxConfig_GetS64FromValue(pstValue, -1, &s64Result);
  }

  /* Done! */
  return s64Result;
}

/** Reads an unsigned integer value from config (will take a random value if a list is provided for this key)
 * @param[in] _zKey             Key name
 * @return The value
 */
orxU64 orxFASTCALL orxConfig_GetU64(const orxSTRING _zKey)
{
  orxCONFIG_VALUE  *pstValue;
  orxU64            u64Result = 0;

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
    orxConfig_GetU64FromValue(pstValue, -1, &u64Result);
  }

  /* Done! */
  return u64Result;
}

/** Reads a float value from config (will take a random value if a list is provided for this key)
 * @param[in] _zKey             Key name
 * @return The value
 */
orxFLOAT orxFASTCALL orxConfig_GetFloat(const orxSTRING _zKey)
{
  orxCONFIG_VALUE  *pstValue;
  orxFLOAT          fResult = orxFLOAT_0;

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
    orxConfig_GetFloatFromValue(pstValue, -1, &fResult);
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
  const orxSTRING   zResult = orxSTRING_EMPTY;

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
    orxConfig_GetStringFromValue(pstValue, -1, &zResult);
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
  orxBOOL           bResult = orxFALSE;

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
    orxConfig_GetBoolFromValue(pstValue, -1, &bResult);
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
  orxVECTOR        *pvResult = orxNULL;

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
    if(orxConfig_GetVectorFromValue(pstValue, -1, _pvVector) != orxSTATUS_FAILURE)
    {
      pvResult = _pvVector;
    }
  }

  /* Done! */
  return pvResult;
}

/** Duplicates a raw value (string) from config
 * @param[in] _zKey             Key name
 * @return The value
 */
orxSTRING orxFASTCALL orxConfig_DuplicateRawValue(const orxSTRING _zKey)
{
  orxCONFIG_ENTRY  *pstEntry;
  orxSTRING         zResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);

  /* Gets corresponding entry */
  pstEntry = orxConfig_GetEntry(orxString_ToCRC(_zKey));

  /* Found? */
  if(pstEntry != orxNULL)
  {
    /* Not in block mode? */
    if(!orxFLAG_TEST(pstEntry->stValue.u16Flags, orxCONFIG_VALUE_KU16_FLAG_BLOCK_MODE))
    {
      /* Restores string */
      orxConfig_RestoreLiteralValue(&(pstEntry->stValue));

      /* Duplicates its content */
      zResult = orxString_Duplicate(pstEntry->stValue.zValue);

      /* Computes working value */
      orxConfig_ComputeWorkingValue(&(pstEntry->stValue), orxU32_UNDEFINED);
    }
    else
    {
      /* Duplicates its content */
      zResult = orxString_Duplicate(pstEntry->stValue.zValue);
    }
  }
  else
  {
    /* Updates result */
    zResult = orxNULL;
  }

  /* Done! */
  return zResult;
}

/** Writes an integer value to config
 * @param[in] _zKey             Key name
 * @param[in] _s32Value         Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_SetS32(const orxSTRING _zKey, orxS32 _s32Value)
{
  orxCHAR   zValue[16];
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);

  /* Clears buffer */
  orxMemory_Zero(zValue, 16 * sizeof(orxCHAR));

  /* Gets literal value */
  orxString_Print(zValue, "%d", _s32Value);

  /* Adds/replaces new entry */
  eResult = orxConfig_SetEntry(_zKey, zValue, orxFALSE, orxFALSE);

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
  orxCHAR   zValue[16];
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);

  /* Clears buffer */
  orxMemory_Zero(zValue, 16 * sizeof(orxCHAR));

  /* Gets literal value */
  orxString_Print(zValue, "%u", _u32Value);

  /* Adds/replaces new entry */
  eResult = orxConfig_SetEntry(_zKey, zValue, orxFALSE, orxFALSE);

  /* Done! */
  return eResult;
}

/** Writes an integer value to config
 * @param[in] _zKey             Key name
 * @param[in] _s64Value         Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_SetS64(const orxSTRING _zKey, orxS64 _s64Value)
{
  orxCHAR   zValue[32];
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);

  /* Clears buffer */
  orxMemory_Zero(zValue, 32 * sizeof(orxCHAR));

  /* Gets literal value */
  orxString_Print(zValue, "%lld", _s64Value);

  /* Adds/replaces new entry */
  eResult = orxConfig_SetEntry(_zKey, zValue, orxFALSE, orxFALSE);

  /* Done! */
  return eResult;
}

/** Writes an unsigned integer value to config
 * @param[in] _zKey             Key name
 * @param[in] _u64Value         Value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_SetU64(const orxSTRING _zKey, orxU64 _u64Value)
{
  orxCHAR   zValue[32];
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);

  /* Clears buffer */
  orxMemory_Zero(zValue, 32 * sizeof(orxCHAR));

  /* Gets literal value */
  orxString_Print(zValue, "%llu", _u64Value);

  /* Adds/replaces new entry */
  eResult = orxConfig_SetEntry(_zKey, zValue, orxFALSE, orxFALSE);

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
  orxCHAR   zValue[16];
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);

  /* Clears buffer */
  orxMemory_Zero(zValue, 16 * sizeof(orxCHAR));

  /* Gets literal value */
  orxString_Print(zValue, "%f", _fValue);

  /* Adds/replaces new entry */
  eResult = orxConfig_SetEntry(_zKey, zValue, orxFALSE, orxFALSE);

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
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);
  orxASSERT(_zValue != orxNULL);

  /* Adds/replaces new entry */
  eResult = orxConfig_SetEntry(_zKey, _zValue, orxFALSE, orxFALSE);

  /* Done! */
  return eResult;
}

/** Writes a string value to config, in block mode
 * @param[in] _zKey             Key name
 * @param[in] _zValue           Value to write in block mode
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_SetStringBlock(const orxSTRING _zKey, const orxSTRING _zValue)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);
  orxASSERT(_zValue != orxNULL);

  /* Adds/replaces new entry */
  eResult = orxConfig_SetEntry(_zKey, _zValue, orxTRUE, orxFALSE);

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
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);

  /* Adds/replaces new entry */
  eResult = orxConfig_SetEntry(_zKey, (_bValue == orxFALSE) ? orxSTRING_FALSE : orxSTRING_TRUE, orxFALSE, orxFALSE);

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
  orxCHAR   zValue[64];
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);
  orxASSERT(_pvValue != orxNULL);

  /* Clears buffer */
  orxMemory_Zero(zValue, 64 * sizeof(orxCHAR));

  /* Gets literal value */
  orxString_Print(zValue, "%c%g%c %g%c %g%c", orxSTRING_KC_VECTOR_START, _pvValue->fX, orxSTRING_KC_VECTOR_SEPARATOR, _pvValue->fY, orxSTRING_KC_VECTOR_SEPARATOR, _pvValue->fZ, orxSTRING_KC_VECTOR_END);

  /* Adds/replaces new entry */
  eResult = orxConfig_SetEntry(_zKey, zValue, orxFALSE, orxFALSE);

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

/** Gets list count for a given key
 * @param[in] _zKey             Key name
 * @return List count if it's a valid list, 0 otherwise
 */
orxS32 orxFASTCALL orxConfig_GetListCount(const orxSTRING _zKey)
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
    s32Result = (orxS32)pstValue->u16ListCount;
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
  orxASSERT(_s32ListIndex < 0xFFFF);

  /* Gets value */
  pstValue = orxConfig_GetValue(_zKey);

  /* Valid? */
  if(pstValue != orxNULL)
  {
    /* Is index valid? */
    if(_s32ListIndex < (orxS32)pstValue->u16ListCount)
    {
      /* Updates result */
      orxConfig_GetS32FromValue(pstValue, _s32ListIndex, &s32Result);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Failed to get S32 list item value <%s.%s>, invalid index: %d out of %d item(s).", _zKey, pstValue->zValue, _s32ListIndex, (orxS32)pstValue->u16ListCount);
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
  orxASSERT(_s32ListIndex < 0xFFFF);

  /* Gets value */
  pstValue = orxConfig_GetValue(_zKey);

  /* Valid? */
  if(pstValue != orxNULL)
  {
    /* Is index valid? */
    if(_s32ListIndex < (orxS32)pstValue->u16ListCount)
    {
      /* Updates result */
      orxConfig_GetU32FromValue(pstValue, _s32ListIndex, &u32Result);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Failed to get U32 list item config value <%s.%s>, invalid index: %d out of %d item(s).", _zKey, pstValue->zValue, _s32ListIndex, (orxS32)pstValue->u16ListCount);
    }
  }

  /* Done! */
  return u32Result;
}

/** Reads a signed integer value from config list
 * @param[in] _zKey             Key name
 * @param[in] _s32ListIndex     Index of desired item in list / -1 for random
 * @return The value
 */
orxS64 orxFASTCALL orxConfig_GetListS64(const orxSTRING _zKey, orxS32 _s32ListIndex)
{
  orxCONFIG_VALUE  *pstValue;
  orxS64            s64Result = 0;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);
  orxASSERT(_s32ListIndex < 0xFFFF);

  /* Gets value */
  pstValue = orxConfig_GetValue(_zKey);

  /* Valid? */
  if(pstValue != orxNULL)
  {
    /* Is index valid? */
    if(_s32ListIndex < (orxS64)pstValue->u16ListCount)
    {
      /* Updates result */
      orxConfig_GetS64FromValue(pstValue, _s32ListIndex, &s64Result);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Failed to get S64 list item value <%s.%s>, invalid index: %d out of %d item(s).", _zKey, pstValue->zValue, _s32ListIndex, (orxS64)pstValue->u16ListCount);
    }
  }

  /* Done! */
  return s64Result;
}

/** Reads an unsigned integer value from config list
 * @param[in] _zKey             Key name
 * @param[in] _s32ListIndex     Index of desired item in list / -1 for random
 * @return The value
 */
orxU64 orxFASTCALL orxConfig_GetListU64(const orxSTRING _zKey, orxS32 _s32ListIndex)
{
  orxCONFIG_VALUE  *pstValue;
  orxU64            u64Result = 0;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);
  orxASSERT(_s32ListIndex < 0xFFFF);

  /* Gets value */
  pstValue = orxConfig_GetValue(_zKey);

  /* Valid? */
  if(pstValue != orxNULL)
  {
    /* Is index valid? */
    if(_s32ListIndex < (orxS64)pstValue->u16ListCount)
    {
      /* Updates result */
      orxConfig_GetU64FromValue(pstValue, _s32ListIndex, &u64Result);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Failed to get U64 list item config value <%s.%s>, invalid index: %d out of %d item(s).", _zKey, pstValue->zValue, _s32ListIndex, (orxS64)pstValue->u16ListCount);
    }
  }

  /* Done! */
  return u64Result;
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
  orxASSERT(_s32ListIndex < 0xFFFF);

  /* Gets value */
  pstValue = orxConfig_GetValue(_zKey);

  /* Valid? */
  if(pstValue != orxNULL)
  {
    /* Is index valid? */
    if(_s32ListIndex < (orxS32)pstValue->u16ListCount)
    {
      /* Updates result */
      orxConfig_GetFloatFromValue(pstValue, _s32ListIndex, &fResult);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Failed to get FLOAT list item config value <%s.%s>, invalid index: %d out of %d item(s).", _zKey, pstValue->zValue, _s32ListIndex, (orxS32)pstValue->u16ListCount);
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
  const orxSTRING   zResult = orxSTRING_EMPTY;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);
  orxASSERT(_s32ListIndex < 0xFFFF);

  /* Gets value */
  pstValue = orxConfig_GetValue(_zKey);

  /* Valid? */
  if(pstValue != orxNULL)
  {
    /* Is index valid? */
    if(_s32ListIndex < (orxS32)pstValue->u16ListCount)
    {
      /* Updates result */
      orxConfig_GetStringFromValue(pstValue, _s32ListIndex, &zResult);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Failed to get STRING list item config value <%s.%s>, invalid index: %d out of %d item(s).", _zKey, pstValue->zValue, _s32ListIndex, (orxS32)pstValue->u16ListCount);
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
  orxASSERT(_s32ListIndex < 0xFFFF);

  /* Gets value */
  pstValue = orxConfig_GetValue(_zKey);

  /* Valid? */
  if(pstValue != orxNULL)
  {
    /* Is index valid? */
    if(_s32ListIndex < (orxS32)pstValue->u16ListCount)
    {
      /* Updates result */
      orxConfig_GetBoolFromValue(pstValue, _s32ListIndex, &bResult);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Failed to get BOOL list item config value <%s.%s>, invalid index: %d out of %d item(s).", _zKey, pstValue->zValue, _s32ListIndex, (orxS32)pstValue->u16ListCount);
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
  orxASSERT(_s32ListIndex < 0xFFFF);

  /* Gets value */
  pstValue = orxConfig_GetValue(_zKey);

  /* Valid? */
  if(pstValue != orxNULL)
  {
    /* Is index valid? */
    if(_s32ListIndex < (orxS32)pstValue->u16ListCount)
    {
      /* Updates result */
      if(orxConfig_GetVectorFromValue(pstValue, _s32ListIndex, _pvVector) != orxSTATUS_FAILURE)
      {
        pvResult = _pvVector;
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Failed to get U32 list item config value <%s.%s>, invalid index: %d out of %d item(s).", _zKey, pstValue->zValue, _s32ListIndex, (orxS32)pstValue->u16ListCount);
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
orxSTATUS orxFASTCALL orxConfig_SetListString(const orxSTRING _zKey, const orxSTRING _azValue[], orxU32 _u32Number)
{
  orxU32    u32Index, i;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);
  orxASSERT(_azValue != orxNULL);

  /* Valid? */
  if((_u32Number > 0) && (_u32Number < 0xFFFF))
  {
    /* For all values */
    for(i = 0, u32Index = 0; (i < _u32Number) && (u32Index < orxCONFIG_KU32_LARGE_BUFFER_SIZE - 1); i++)
    {
      const orxCHAR *pc;

      /* For all characters */
      for(pc = _azValue[i]; (*pc != orxCHAR_NULL) && (u32Index < orxCONFIG_KU32_LARGE_BUFFER_SIZE - 1); pc++)
      {
        /* Copies it */
        sstConfig.acValueBuffer[u32Index++] = *pc;
      }

      /* Adds separator */
      sstConfig.acValueBuffer[u32Index++] = orxCONFIG_KC_LIST_SEPARATOR;
    }

    /* Ran out of memory? */
    if(u32Index >= orxCONFIG_KU32_LARGE_BUFFER_SIZE - 1)
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Cannot write config string list as the list would exceed %d bytes in memory.", orxCONFIG_KU32_LARGE_BUFFER_SIZE);

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
    else
    {
      /* Removes last separator */
      if(u32Index > 0)
      {
        sstConfig.acValueBuffer[u32Index - 1] = orxCHAR_NULL;
      }

      /* Stores buffer list size */
      sstConfig.u32BufferListSize = _u32Number;

      /* Adds/replaces new entry */
      eResult = orxConfig_SetEntry(_zKey, sstConfig.acValueBuffer, orxFALSE, orxFALSE);
    }

    /* Clears value buffer */
    sstConfig.u32BufferListSize = 0;
    sstConfig.acValueBuffer[0]  = orxCHAR_NULL;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Cannot write config string list as no or too many item(s) are provided.");

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Appends string values to a config list (will create a new entry if not already present)
 * @param[in] _zKey             Key name
 * @param[in] _azValue          Values
 * @param[in] _u32Number        Number of values
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_AppendListString(const orxSTRING _zKey, const orxSTRING _azValue[], orxU32 _u32Number)
{
  orxU32    u32Index, i;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(_zKey != orxSTRING_EMPTY);
  orxASSERT(_azValue != orxNULL);

  /* Valid? */
  if((_u32Number > 0) && (_u32Number < 0xFFFF))
  {
    /* For all values */
    for(i = 0, u32Index = 0; (i < _u32Number) && (u32Index < orxCONFIG_KU32_LARGE_BUFFER_SIZE - 1); i++)
    {
      const orxCHAR *pc;

      /* For all characters */
      for(pc = _azValue[i]; (*pc != orxCHAR_NULL) && (u32Index < orxCONFIG_KU32_LARGE_BUFFER_SIZE - 1); pc++)
      {
        /* Copies it */
        sstConfig.acValueBuffer[u32Index++] = *pc;
      }

      /* Adds separator */
      sstConfig.acValueBuffer[u32Index++] = orxCONFIG_KC_LIST_SEPARATOR;
    }

    /* Ran out of memory? */
    if(u32Index >= orxCONFIG_KU32_LARGE_BUFFER_SIZE - 1)
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Cannot append config string list as the list would exceed %d bytes in memory.", orxCONFIG_KU32_LARGE_BUFFER_SIZE);

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
    else
    {
      /* Removes last separator */
      if(u32Index > 0)
      {
        sstConfig.acValueBuffer[u32Index - 1] = orxCHAR_NULL;
      }

      /* Stores buffer list size */
      sstConfig.u32BufferListSize = _u32Number;

      /* Appends to existing entry */
      eResult = orxConfig_SetEntry(_zKey, sstConfig.acValueBuffer, orxFALSE, orxTRUE);
    }

    /* Clears value buffer */
    sstConfig.u32BufferListSize = 0;
    sstConfig.acValueBuffer[0]  = orxCHAR_NULL;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_CONFIG, "Cannot write config string list as no or too many item(s) are provided.");

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets key count for the current section
 * @return Key count the current section if valid, 0 otherwise
 */
orxU32 orxFASTCALL orxConfig_GetKeyCount()
{
  orxU32 u32Result;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));

  /* Has current section? */
  if(sstConfig.pstCurrentSection != orxNULL)
  {
    /* Updates result */
    u32Result = orxLinkList_GetCount(&(sstConfig.pstCurrentSection->stEntryList));
  }
  else
  {
    /* Updates result */
    u32Result = 0;
  }

  /* Done! */
  return u32Result;
}

/** Gets key for the current section at the given index
 * @param[in] _u32KeyIndex      Index of the desired key
 * @return orxSTRING if exist, orxSTRING_EMPTY otherwise
 */
const orxSTRING orxFASTCALL orxConfig_GetKey(orxU32 _u32KeyIndex)
{
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));

  /* Valid? */
  if(_u32KeyIndex < orxConfig_GetKeyCount())
  {
    orxCONFIG_ENTRY  *pstEntry;
    orxU32            i;

    /* Finds correct entry */
    for(i = _u32KeyIndex, pstEntry = (orxCONFIG_ENTRY *)orxLinkList_GetFirst(&(sstConfig.pstCurrentSection->stEntryList));
        i > 0;
        i--, pstEntry = (orxCONFIG_ENTRY *)orxLinkList_GetNext(&(pstEntry->stNode)));

    /* Updates result */
    zResult = orxString_GetFromID(pstEntry->stID);
  }
  else
  {
    /* Updates result */
    zResult = orxSTRING_EMPTY;
  }

  /* Done! */
  return zResult;
}

#ifdef __orxGCC__

  #pragma GCC diagnostic pop

#endif /* __orxGCC__ */
