/**
 * @file orxConfig.c
 */

/***************************************************************************
 orxConfig.c
 Lib C / file implementation of the Config module
 begin                : 09/12/2007
 author               : (C) Arcallians
 email                : cursor@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/


#include<stdio.h>


#include "orxInclude.h"

#include "core/orxConfig.h"
#include "debug/orxDebug.h"
#include "memory/orxBank.h"
#include "utils/orxString.h"


/** Module flags
 */
#define orxCONFIG_KU32_STATIC_FLAG_NONE   0x00000000  /**< No flags */

#define orxCONFIG_KU32_STATIC_FLAG_READY  0x00000001  /**< Ready flag */

#define orxCONFIG_KU32_STATIC_MASK_ALL    0xFFFFFFFF  /**< All mask */


/** Defines
 */
#define orxCONFIG_KU32_SECTION_BANK_SIZE  16          /**< Default section bank size */
#define orxCONFIG_KU32_ENTRY_BANK_SIZE    16          /**< Default section bank size */

#define orxCONFIG_KU32_BUFFER_SIZE        4096        /**< Buffer size */

#define orxCONFIG_KC_SECTION_START        '['         /**< Section start character */
#define orxCONFIG_KC_SECTION_END          ']'         /**< Section end character */
#define orxCONFIG_KC_ASSIGN               '='         /**< Assign character */
#define orxCONFIG_KC_COMMENT              ';'         /**< Comment character */

#define orxCONFIG_KZ_DEFAULT_FILE         "orx.ini"   /**< Default config file name */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Config node structure
 */
typedef struct __orxCONFIG_ENTRY_t
{
  orxSTRING         zKey;                   /**< Entry key: 4 */
  orxSTRING         zValue;                 /**< Entry value : 8 */
  orxU32            u32CRC;                 /**< Key CRC : 12 */

  orxPAD(12)

} orxCONFIG_ENTRY;

/** Config section structure
 */
typedef struct __orxCONFIG_SECTION_t
{
  orxBANK    *pstBank;                      /**< Bank of entries : 4 */
  orxSTRING   zName;                        /**< Section name : 8 */ 
  orxU32      u32CRC;                       /**< Section CRC : 12 */

  orxPAD(12)

} orxCONFIG_SECTION;

/** Static structure
 */
typedef struct __orxCONFIG_STATIC_t
{
  orxBANK            *pstSectionBank;                           /**< Bank of sections */
  orxCONFIG_SECTION  *pstCurrentSection;                        /**< Current working section */
  orxU32              u32Flags;                                 /**< Control flags */

} orxCONFIG_STATIC;


/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/

orxSTATIC orxCONFIG_STATIC sstConfig;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Gets an entry from the current section
 * @param[in] _zKey             Entry key
 * @return                      orxCONFIG_ENTRY / orxNULL
 */
orxSTATIC orxINLINE orxCONFIG_ENTRY *orxConfig_GetEntry(orxCONST orxSTRING _zKey)
{
  orxU32            u32CRC;
  orxCONFIG_ENTRY  *pstResult = orxNULL, *pstEntry;

  /* Checks */
  orxASSERT(sstConfig.pstCurrentSection != orxNULL);
  orxASSERT(_zKey != orxNULL);

  /* Gets key CRC */
  u32CRC = orxString_ToCRC(_zKey);

  /* For all entries */
  for(pstEntry = orxBank_GetNext(sstConfig.pstCurrentSection->pstBank, orxNULL);
      pstEntry != orxNULL;
      pstEntry = orxBank_GetNext(sstConfig.pstCurrentSection->pstBank, pstEntry))
  {
    /* Found? */
    if(u32CRC == pstEntry->u32CRC)
    {
      /* Updates result */
      pstResult = pstEntry;

      break;
    }
  }

  /* Done! */
  return pstResult;
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
        /* Sets its CRC */
        pstEntry->u32CRC = orxString_ToCRC(pstEntry->zKey);

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
 */
orxSTATIC orxINLINE orxCONFIG_SECTION *orxConfig_CreateSection(orxCONST orxSTRING _zSectionName)
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
        /* Sets its CRC */
        pstSection->u32CRC = orxString_ToCRC(pstSection->zName);
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
    /* Cleans control structure */
    orxMemory_Set(&sstConfig, 0, sizeof(orxCONFIG_STATIC));

    /* Creates section bank */
    sstConfig.pstSectionBank = orxBank_Create(orxCONFIG_KU32_SECTION_BANK_SIZE, sizeof(orxCONFIG_SECTION), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_CONFIG);

    /* Valid? */
    if(sstConfig.pstSectionBank != orxNULL)
    {
      /* Inits Flags */
      orxFLAG_SET(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY, orxCONFIG_KU32_STATIC_MASK_ALL);

      /* Loads default config file */
      orxConfig_Load(orxCONFIG_KZ_DEFAULT_FILE);

      /* Success */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* !!! MSG !!! */

      /* Section bank not created */
      eResult = orxSTATUS_FAILURE;
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
    orxCONFIG_SECTION *pstSection;

    /* While there's still a section */
    while((pstSection = orxBank_GetNext(sstConfig.pstSectionBank, orxNULL)) != orxNULL)
    {
      /* Deletes it */
      orxConfig_DeleteSection(pstSection);
    }

    /* Deletes section bank */
    orxBank_Delete(sstConfig.pstSectionBank);
    sstConfig.pstSectionBank = orxNULL;

    /* Updates flags */
    orxFLAG_SET(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_NONE, orxCONFIG_KU32_STATIC_MASK_ALL);
  }

  return;
}

/** Selects current working section
* @param[in] _zSectionName     Section name to select
*/
orxSTATUS orxConfig_SelectSection(orxCONST orxSTRING _zSectionName)
{
  orxCONFIG_SECTION  *pstSection;
  orxU32              u32CRC;
  orxSTATUS           eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zSectionName != orxNULL);
  orxASSERT(*_zSectionName != *orxSTRING_EMPTY);

  /* Gets section name CRC */
  u32CRC = orxString_ToCRC(_zSectionName);

  /* For all the sections */
  for(pstSection = orxBank_GetNext(sstConfig.pstSectionBank, orxNULL);
      pstSection != orxNULL;
      pstSection = orxBank_GetNext(sstConfig.pstSectionBank, pstSection))
  {
    /* Found? */
    if(u32CRC == pstSection->u32CRC)
    {
      /* Selects it */
      sstConfig.pstCurrentSection = pstSection;

      break;
    }
  }

  /* Not found? */
  if(pstSection == orxNULL)
  {
    /* Creates it */
    pstSection = orxConfig_CreateSection(_zSectionName);

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

  /* Done! */
  return eResult;
}

/** Read config config from source.
 * @param[in] _zFileName        File name
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_Load(orxCONST orxSTRING _zFileName)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zFileName != orxNULL);
  orxASSERT(*_zFileName != *orxSTRING_EMPTY);

  FILE *pstFile;

  /* Opens file */
  pstFile = fopen(_zFileName, "r");

  /* Valid? */
  if(pstFile != orxNULL)
  {
    orxCHAR acBuffer[orxCONFIG_KU32_BUFFER_SIZE];
    orxU32  u32Size, u32Offset;

    /* While file isn't empty */
    for(u32Size = fread(acBuffer, sizeof(orxCHAR), orxCONFIG_KU32_BUFFER_SIZE, pstFile), u32Offset = 0;
        u32Size > 0;
        u32Size = fread(acBuffer + u32Offset, sizeof(orxCHAR), orxCONFIG_KU32_BUFFER_SIZE - u32Offset, pstFile) + u32Offset)
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
            orxCONFIG_ENTRY *pstEntry;

            /* Cuts the strings */
            *pcKeyEnd = *pc = orxCHAR_NULL;

            /* Already defined? */
            if((pstEntry = orxConfig_GetEntry(pcLineStart)) != orxNULL)
            {
              orxLOG("Config entry [%s::%s] has already the value <%s>. Ignoring new value <%s>.", sstConfig.pstCurrentSection->zName, pstEntry->zKey, pstEntry->zValue, pcValueStart);
            }
            else
            {
              /* Adds entry */
              orxConfig_AddEntry(pcLineStart, pcValueStart);

              /* Updates pointers */
              pcKeyEnd = pcValueStart = orxNULL;
            }
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
          while((pc < acBuffer + u32Size) && ((*pc == orxCHAR_CR) || (*pc == orxCHAR_LF)))
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
                orxLOG("Section name <%*s> incomplete, closing character '%c' not found.", pc - (pcLineStart + 1), pcLineStart + 1, orxCONFIG_KC_SECTION_END);

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
              for(pcKeyEnd = pc;
                  (pcKeyEnd > pcLineStart) && ((*pcKeyEnd == ' ') || (*pcKeyEnd == '\t'));
                  pcKeyEnd--);

              /* Checks */
              orxASSERT(pcKeyEnd > pcLineStart);

              /* Finds start of value position */
              for(pcValueStart = pc + 1;
                  (pcValueStart < acBuffer + u32Size) && ((*pcKeyEnd == ' ') || (*pcKeyEnd == '\t') || (*pcKeyEnd == '\n'));
                  pcValueStart++);
            }
          }
        }
      }

      /* Has remaining buffer? */
      if((pcLineStart != acBuffer) && (pc > pcLineStart))
      {
        /* Updates offset */
        u32Offset = pc - pcLineStart;

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
    zFileName = orxCONFIG_KZ_DEFAULT_FILE;
  }

  /* Opens file */
  pstFile = fopen(zFileName, "wR");

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

/** Reads an integer value from config
 * @param[in] _zKey             Key name
 * @param[in] _s32DefaultValue  Default value if key is not found
 * @return The value
 */
orxS32 orxFASTCALL orxConfig_GetS32(orxCONST orxSTRING _zKey)
{
  orxCONFIG_ENTRY  *pstEntry;
  orxS32            s32Result = 0;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(*_zKey != *orxSTRING_EMPTY);

  /* Gets corresponding entry */
  pstEntry = orxConfig_GetEntry(_zKey);

  /* Found? */
  if(pstEntry != orxNULL)
  {
    orxS32 s32Value;

    /* Gets value */
    if(orxString_ToS32(pstEntry->zValue, 10, &s32Value, orxNULL) != orxSTATUS_FAILURE)
    {
      /* Updates result */
      s32Result = s32Value;
    }
  }

  /* Done! */
  return s32Result;
}

/** Reads a float value from config
 * @param[in] _zKey             Key name
 * @param[in] _fDefaultValue    Default value if key is not found
 * @return The value
 */
orxFLOAT orxFASTCALL orxConfig_GetFloat(orxCONST orxSTRING _zKey)
{
  orxCONFIG_ENTRY  *pstEntry;
  orxFLOAT          fResult = orxFLOAT_0;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(*_zKey != *orxSTRING_EMPTY);

  /* Gets corresponding entry */
  pstEntry = orxConfig_GetEntry(_zKey);

  /* Found? */
  if(pstEntry != orxNULL)
  {
    orxFLOAT fValue;

    /* Gets value */
    if(orxString_ToFloat(pstEntry->zValue, &fValue, orxNULL) != orxSTATUS_FAILURE)
    {
      /* Updates result */
      fResult = fValue;
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
  orxCONFIG_ENTRY  *pstEntry;
  orxSTRING         zResult = orxSTRING_EMPTY;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(*_zKey != *orxSTRING_EMPTY);

  /* Gets corresponding entry */
  pstEntry = orxConfig_GetEntry(_zKey);

  /* Found? */
  if(pstEntry != orxNULL)
  {
    /* Updates result */
    zResult = pstEntry->zValue;
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
  orxCONFIG_ENTRY  *pstEntry;
  orxBOOL           bResult = orxFALSE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zKey != orxNULL);
  orxASSERT(*_zKey != *orxSTRING_EMPTY);

  /* Gets corresponding entry */
  pstEntry = orxConfig_GetEntry(_zKey);

  /* Found? */
  if(pstEntry != orxNULL)
  {
    orxBOOL bValue;

    /* Gets value */
    if(orxString_ToBool(pstEntry->zValue, &bValue, orxNULL) != orxSTATUS_FAILURE)
    {
      /* Updates result */
      bResult = bValue;
    }
  }

  /* Done! */
  return bResult;
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
  orxMemory_Set(zValue, 0, 16 * sizeof(orxCHAR));

  /* Gets literal value */
  orxString_Print(zValue, "%d", _s32Value); 

  /* Gets entry */
  pstEntry = orxConfig_GetEntry(_zKey);

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
  orxMemory_Set(zValue, 0, 16 * sizeof(orxCHAR));

  /* Gets literal value */
  orxString_Print(zValue, "%g", _fValue); 

  /* Gets entry */
  pstEntry = orxConfig_GetEntry(_zKey);

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
  pstEntry = orxConfig_GetEntry(_zKey);

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
  pstEntry = orxConfig_GetEntry(_zKey);

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
