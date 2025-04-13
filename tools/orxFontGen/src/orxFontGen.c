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
 * @file orxFontGen.c
 * @date 14/07/2010
 * @author iarwain@orx-project.org
 *
 */


#include "orx.h"


/** Module flags
 */
#define orxFONTGEN_KU32_STATIC_FLAG_NONE            0x00000000  /**< No flags */

#define orxFONTGEN_KU32_STATIC_FLAG_FONT            0x00000001  /**< Font flag */
#define orxFONTGEN_KU32_STATIC_FLAG_SIZE            0x00000002  /**< Size flag */
#define orxFONTGEN_KU32_STATIC_FLAG_MONOSPACE       0x00000004  /**< Monospace flag */
#define orxFONTGEN_KU32_STATIC_FLAG_SDF             0x00000008  /**< SDF flag */

#define orxFONTGEN_KU32_STATIC_MASK_READY           0x00000001  /**< Ready mask */

#define orxFONTGEN_KU32_STATIC_MASK_ALL             0xFFFFFFFF  /**< All mask */


/** Misc defines
 */
#define orxFONTGEN_KZ_DEFAULT_NAME                  "Font"

#define orxFONTGEN_KU32_DEFAULT_SIZE                32

#define orxFONTGEN_KU32_BUFFER_SIZE                 8192

#define orxFONTGEN_KU32_CHARACTER_TABLE_SIZE        512

#define orxFONTGEN_KZ_UTF8_BOM                      "\xEF\xBB\xBF"
#define orxFONTGEN_KU32_UTF8_BOM_LENGTH             3

#define orxFONTGEN_KZ_LOG_TAG_LENGTH                "10"

#if defined(__orxGCC__) || defined(__orxLLVM__)

  #define orxFONTGEN_LOG(TAG, FORMAT, ...) orxLOG(orxANSI_KZ_COLOR_FG_YELLOW "%-" orxFONTGEN_KZ_LOG_TAG_LENGTH "s" orxANSI_KZ_COLOR_FG_DEFAULT FORMAT, "[" #TAG "]", ##__VA_ARGS__)

#else // __orxGCC__ || __orxLLVM__

  #define orxFONTGEN_LOG(TAG, FORMAT, ...) orxLOG(orxANSI_KZ_COLOR_FG_YELLOW "%-" orxFONTGEN_KZ_LOG_TAG_LENGTH "s" orxANSI_KZ_COLOR_FG_DEFAULT FORMAT, "[" #TAG "]", __VA_ARGS__)

#endif //__orxGCC__ || __orxLLVM__


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxFONTGEN_STATIC_t
{
  orxSTRING       zFontName;
  orxSTRING       zFileName;
  orxSTRING       zCharacterList;
  orxVECTOR       vCharacterSize;
  orxVECTOR       vCharacterPadding;
  orxU32          u32MaxWidth;
  orxU32          u32Flags;

} orxFONTGEN_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxFONTGEN_STATIC sstFontGen;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

static orxBOOL orxFASTCALL SaveFilter(const orxSTRING _zSectionName, const orxSTRING _zKeyName, const orxSTRING _zFileName, orxBOOL _bUseEncryption)
{
  orxBOOL bResult = orxTRUE;

  // Udpates result
  bResult = !orxString_Compare(_zSectionName, sstFontGen.zFontName) ? orxTRUE : orxFALSE;

  // Done!
  return bResult;
}

static orxSTATUS orxFASTCALL ParseTextFile(const orxSTRING _zFileName, orxHASHTABLE *_pstCharacterTable)
{
  orxFILE  *pstFile;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  // Opens file
  pstFile = orxFile_Open(_zFileName, orxFILE_KU32_FLAG_OPEN_READ | orxFILE_KU32_FLAG_OPEN_BINARY);

  // Success?
  if(pstFile)
  {
    orxCHAR acBuffer[orxFONTGEN_KU32_BUFFER_SIZE];
    orxU32  u32Size, u32Offset, u32Counter;
    orxBOOL bFirst;

    // While file isn't empty
    for(u32Size = (orxU32)orxFile_Read(acBuffer, sizeof(orxCHAR), orxFONTGEN_KU32_BUFFER_SIZE, pstFile), u32Offset = 0, u32Counter = 0, bFirst = orxTRUE;
        u32Size > 0;
        u32Size = (orxU32)orxFile_Read(acBuffer + u32Offset, sizeof(orxCHAR), orxFONTGEN_KU32_BUFFER_SIZE - u32Offset, pstFile) + u32Offset, bFirst = orxFALSE)
    {
      orxCHAR *pc, *pcNext;

      // Has UTF-8 BOM?
      if((bFirst != orxFALSE) && (orxString_NCompare(acBuffer, orxFONTGEN_KZ_UTF8_BOM, orxFONTGEN_KU32_UTF8_BOM_LENGTH) == 0))
      {
        // Skips it
        pc = acBuffer + orxFONTGEN_KU32_UTF8_BOM_LENGTH;
      }
      else
      {
        // Starts at the beginning of the buffer
        pc = acBuffer;
      }

      // For all characters
      for(pcNext = orxNULL; pc < acBuffer + u32Size; pc = pcNext)
      {
        orxU32 u32CharacterCodePoint;

        // Reads it
        u32CharacterCodePoint = orxString_GetFirstCharacterCodePoint(pc, (const orxSTRING *)&pcNext);

        // Non EOL?
        if((u32CharacterCodePoint != orxCHAR_CR)
        && (u32CharacterCodePoint != orxCHAR_LF))
        {
          // Valid?
          if(u32CharacterCodePoint != orxU32_UNDEFINED)
          {
            void **ppBucket;

            // Retrieves bucket from table
            ppBucket = orxHashTable_Retrieve(_pstCharacterTable, (orxU64)u32CharacterCodePoint);

            // Not already set?
            if(*ppBucket == orxNULL)
            {
              // Updates it
              *ppBucket = (void *)(orxUPTR)u32CharacterCodePoint;

              // Updates counter
              u32Counter++;
            }
          }
          else
          {
            // End of buffer?
            if(pcNext >= acBuffer + u32Size)
            {
              // Stops
              break;
            }
            else
            {
              // Logs message
              orxFONTGEN_LOG(LOAD, "Character [U+%X] '%.*s': invalid, skipping.", u32CharacterCodePoint, (orxU32)(pcNext - pc), pc);
            }
          }
        }
      }

      // Has remaining buffer?
      if((pc != acBuffer) && (pcNext > pc))
      {
        // Updates offset
        u32Offset = (orxU32)(orxMIN(pcNext, acBuffer + u32Size) - pc);

        // Copies it at the beginning of the buffer
        orxMemory_Copy(acBuffer, pc, u32Offset);
      }
      else
      {
        // Clears offset
        u32Offset = 0;
      }
    }

    // Logs message
    orxFONTGEN_LOG(LOAD, "'%s': added %u characters.", _zFileName, u32Counter);

    // Updates result
    eResult = orxSTATUS_SUCCESS;
  }

  // Done!
  return eResult;
}

static orxSTATUS orxFASTCALL ProcessInputParams(orxU32 _u32ParamCount, const orxSTRING _azParams[])
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // Has a valid key parameter?
  if(_u32ParamCount > 1)
  {
    // ANSI?
    if(orxString_ICompare(_azParams[1], orxFONT_KZ_ANSI) == 0)
    {
      // Updates character list
      sstFontGen.zCharacterList = orxString_Duplicate(orxFONT_KZ_ANSI);

      // Logs message
      orxFONTGEN_LOG(INPUT, "Character list set to: ANSI");
    }
    // ASCII?
    else if(orxString_ICompare(_azParams[1], orxFONT_KZ_ASCII) == 0)
    {
      // Updates character list
      sstFontGen.zCharacterList = orxString_Duplicate(orxFONT_KZ_ASCII);

      // Logs message
      orxFONTGEN_LOG(INPUT, "Character list set to: ASCII");
    }
    else
    {
      orxHASHTABLE *pstCharacterTable;

      // Creates character table
      pstCharacterTable = orxHashTable_Create(orxFONTGEN_KU32_CHARACTER_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_TEMP);

      // Success?
      if(pstCharacterTable != orxNULL)
      {
        orxU32 i;

        // Updates result
        eResult = orxSTATUS_FAILURE;

        // For all input files
        for(i = 1; i < _u32ParamCount; i++)
        {
          // Parses it
          if(ParseTextFile(_azParams[i], pstCharacterTable) != orxSTATUS_FAILURE)
          {
            // Logs message
            orxFONTGEN_LOG(LOAD, "'%s': SUCCESS.", _azParams[i]);

            // Updates result
            eResult = orxSTATUS_SUCCESS;
          }
          else
          {
            // Logs message
            orxFONTGEN_LOG(LOAD, "'%s': FAILURE, skipping.", _azParams[i]);
          }
        }

        // Success?
        if(eResult != orxSTATUS_FAILURE)
        {
          orxHANDLE hIterator;
          orxU64    u64Key;
          orxU32    u32Length;

          // For all characters
          for(u32Length = 0, hIterator = orxHashTable_GetNext(pstCharacterTable, orxHANDLE_UNDEFINED, &u64Key, orxNULL);
              hIterator != orxHANDLE_UNDEFINED;
              hIterator = orxHashTable_GetNext(pstCharacterTable, hIterator, &u64Key, orxNULL))
          {
            // Updates length
            u32Length += orxString_GetUTF8CharacterLength((orxU32)u64Key);
          }

          // Valid?
          if(u32Length > 0)
          {
            orxCHAR *pc;

            // Allocates it
            sstFontGen.zCharacterList = orxMemory_Allocate(u32Length + 1, orxMEMORY_TYPE_TEXT);
            orxASSERT(sstFontGen.zCharacterList != orxNULL);

            // For all characters
            for(pc = sstFontGen.zCharacterList, hIterator = orxHashTable_GetNext(pstCharacterTable, orxHANDLE_UNDEFINED, &u64Key, orxNULL);
                hIterator != orxHANDLE_UNDEFINED;
                hIterator = orxHashTable_GetNext(pstCharacterTable, hIterator, &u64Key, orxNULL))
            {
              // Prints character
              pc += orxString_PrintUTF8Character(pc, u32Length - (orxU32)(pc - sstFontGen.zCharacterList), (orxU32)u64Key);
            }
          }
          else
          {
            // Updates result
            eResult = orxSTATUS_FAILURE;

            // Logs message
            orxFONTGEN_LOG(INPUT, "No valid text input provided, aborting.");
          }
        }
        else
        {
          // Logs message
          orxFONTGEN_LOG(INPUT, "No valid text input provided, aborting.");
        }

        // Deletes character table
        orxHashTable_Delete(pstCharacterTable);
      }
    }
  }
  else
  {
    // Logs message
    orxFONTGEN_LOG(INPUT, "No text input provided, using default ASCII character list.");
  }

  // Done!
  return eResult;
}

static orxSTATUS orxFASTCALL ProcessOutputParams(orxU32 _u32ParamCount, const orxSTRING _azParams[])
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // Has a valid output parameter?
  if(_u32ParamCount > 1)
  {
    // Stores it
    sstFontGen.zFontName = orxString_Duplicate(_azParams[1]);

    // Logs message
    orxFONTGEN_LOG(OUTPUT, "Using output font name '%s'.", sstFontGen.zFontName);
  }

  // Done!
  return eResult;
}

static orxSTATUS orxFASTCALL ProcessSizeParams(orxU32 _u32ParamCount, const orxSTRING _azParams[])
{
  orxU32    u32Size;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // Has a valid size parameter?
  if((_u32ParamCount > 1)
  && (orxString_ToU32(_azParams[1], &u32Size, orxNULL) != orxSTATUS_FAILURE))
  {
    // Stores it
    sstFontGen.vCharacterSize.fY = orxU2F(u32Size);

    // Updates status
    orxFLAG_SET(sstFontGen.u32Flags, orxFONTGEN_KU32_STATIC_FLAG_SIZE, orxFONTGEN_KU32_STATIC_FLAG_NONE);

    // Logs message
    orxFONTGEN_LOG(SIZE, "Character height (size) set to <%u>.", u32Size);
  }
  else
  {
    // Has value?
    if(_u32ParamCount > 1)
    {
      // Logs message
      orxFONTGEN_LOG(SIZE, "Invalid character size found in '%s', defaulting to <%u>.", _azParams[1], orxFONTGEN_KU32_DEFAULT_SIZE);
    }
    else
    {
      // Logs message
      orxFONTGEN_LOG(SIZE, "No character size found, defaulting to <%u>.", orxFONTGEN_KU32_DEFAULT_SIZE);
    }
  }

  // Done!
  return eResult;
}

static orxSTATUS orxFASTCALL ProcessMaxWidthParams(orxU32 _u32ParamCount, const orxSTRING _azParams[])
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // Has a valid max width parameter?
  if(_u32ParamCount > 1)
  {
    orxU32 u32MaxWidth;

    // Gets it
    if(((eResult = orxString_ToU32(_azParams[1], &u32MaxWidth, orxNULL)) != orxSTATUS_FAILURE)
    && (u32MaxWidth != 0))
    {
      // Stores it
      sstFontGen.u32MaxWidth = u32MaxWidth;

      // Logs message
      orxFONTGEN_LOG(WIDTH, "Maximum texture width set to <%u>.", u32MaxWidth);
    }
    else
    {
      // Logs message
      orxFONTGEN_LOG(WIDTH, "Invalid maximum texture width found in '%s', ignoring.", _azParams[1]);
    }
  }
  else
  {
    // Logs message
    orxFONTGEN_LOG(WIDTH, "No maximum texture width provided, ignoring.");
  }

  // Done!
  return eResult;
}

static orxSTATUS orxFASTCALL ProcessSDFParams(orxU32 _u32ParamCount, const orxSTRING _azParams[])
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // Updates status
  orxFLAG_SET(sstFontGen.u32Flags, orxFONTGEN_KU32_STATIC_FLAG_SDF, orxFONTGEN_KU32_STATIC_FLAG_NONE);

  // Logs message
  orxFONTGEN_LOG(TYPE, "Font type set to SDF.");

  // Done!
  return eResult;
}

static orxSTATUS orxFASTCALL ProcessPaddingParams(orxU32 _u32ParamCount, const orxSTRING _azParams[])
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // Has a valid size parameter?
  if(_u32ParamCount > 1)
  {
    orxFLOAT fPadding;

    // Gets it
    if((orxString_ToVector(_azParams[1], &(sstFontGen.vCharacterPadding), orxNULL) != orxNULL)
    && (sstFontGen.vCharacterPadding.fX >= orxFLOAT_0)
    && (sstFontGen.vCharacterPadding.fY >= orxFLOAT_0))
    {
      // Logs message
      orxFONTGEN_LOG(PADDING, "Character padding set to <(%g, %g)>.", sstFontGen.vCharacterPadding.fX, sstFontGen.vCharacterPadding.fY);
    }
    else if((orxString_ToFloat(_azParams[1], &fPadding, orxNULL) != orxSTATUS_FAILURE)
         && (fPadding >= orxFLOAT_0))
    {
      // Updates padding
      orxVector_Set(&(sstFontGen.vCharacterPadding), fPadding, fPadding, orxFLOAT_0);

      // Logs message
      orxFONTGEN_LOG(PADDING, "Character padding set to <(%g, %g)>.", sstFontGen.vCharacterPadding.fX, sstFontGen.vCharacterPadding.fY);
    }
    else
    {
      // Logs message
      orxFONTGEN_LOG(PADDING, "Invalid character padding found in '%s', ignoring.", _azParams[1]);
    }
  }

  // Done!
  return eResult;
}

static orxSTATUS orxFASTCALL ProcessFontParams(orxU32 _u32ParamCount, const orxSTRING _azParams[])
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  // Has a valid font parameter?
  if(_u32ParamCount > 1)
  {
    // Found?
    if(orxResource_Locate(orxFONT_KZ_RESOURCE_GROUP, _azParams[1]) != orxNULL)
    {
      // Updates status
      orxFLAG_SET(sstFontGen.u32Flags, orxFONTGEN_KU32_STATIC_FLAG_FONT, orxFONTGEN_KU32_STATIC_FLAG_NONE);

      // Stores it
      sstFontGen.zFileName = orxString_Duplicate(_azParams[1]);

      // Updates result
      eResult = orxSTATUS_SUCCESS;

      // Logs message
      orxFONTGEN_LOG(FONT, "Using font '%s'.", sstFontGen.zFileName);
    }
    else
    {
      // Logs message
      orxFONTGEN_LOG(FONT, "Couldn't find font '%s', aborting.", _azParams[1]);
    }
  }
  else
  {
    // Logs message
    orxFONTGEN_LOG(FONT, "No font specified, aborting.");
  }

  // Done!
  return eResult;
}

static orxSTATUS orxFASTCALL ProcessMonospaceParams(orxU32 _u32ParamCount, const orxSTRING _azParams[])
{
  orxU32    u32Width;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // Updates status flags
  orxFLAG_SET(sstFontGen.u32Flags, orxFONTGEN_KU32_STATIC_FLAG_MONOSPACE, orxFONTGEN_KU32_STATIC_FLAG_NONE);

  // Has valid width?
  if((_u32ParamCount > 1)
  && (orxString_ToU32(_azParams[1], &u32Width, orxNULL) != orxSTATUS_FAILURE))
  {
    // Updates character size
    sstFontGen.vCharacterSize.fX = orxU2F(u32Width);

    // Logs message
    orxFONTGEN_LOG(MODE, "Output mode set to monospace, using width <%u>.", u32Width);
  }
  else
  {
    // Updates character size
    sstFontGen.vCharacterSize.fX = -orxFLOAT_1;

    // Logs message
    orxFONTGEN_LOG(MODE, "Output mode set to monospace (proportional).");
  }

  // Done!
  return eResult;
}

static void orxFASTCALL Setup()
{
  // Adds module dependencies
  orxModule_AddDependency(orxMODULE_ID_MAIN, orxMODULE_ID_PARAM);
  orxModule_AddDependency(orxMODULE_ID_MAIN, orxMODULE_ID_FILE);
  orxModule_AddDependency(orxMODULE_ID_MAIN, orxMODULE_ID_FONT);
}

static orxSTATUS orxFASTCALL Init()
{
#define orxFONTGEN_DECLARE_PARAM(SN, LN, SD, LD, FN) {orxPARAM_KU32_FLAG_STOP_ON_ERROR, SN, LN, SD, LD, &FN},

  orxU32    i;
  orxSTATUS eResult = orxSTATUS_SUCCESS;
  orxPARAM  astParamList[] =
  {
    orxFONTGEN_DECLARE_PARAM("f", "font", "Input font file", "TrueType/OpenType font (usually .ttf or .otf) used to generate all the required glyphs", ProcessFontParams)
    orxFONTGEN_DECLARE_PARAM("p", "padding", "Character padding", "Extra padding added to all characters, either as a float (will affect both axes) or a vector", ProcessPaddingParams)
    orxFONTGEN_DECLARE_PARAM("mw", "max-width", "Maximum texture width", "The maximum width for the output texture, in pixels", ProcessMaxWidthParams)
    orxFONTGEN_DECLARE_PARAM("o", "output", "Font output name", "Font base output name: .png will be added to the image and .ini will be added to the config file", ProcessOutputParams)
    orxFONTGEN_DECLARE_PARAM("s", "size", "Size (height) of characters", "Height to use for characters defined with this font", ProcessSizeParams)
    orxFONTGEN_DECLARE_PARAM("m", "monospace", "Monospaced font", "Will output a monospace (ie. fixed-width) font, the width will be the provided value, if any, or proportional to the height otherwise", ProcessMonospaceParams)
    orxFONTGEN_DECLARE_PARAM("sdf", "sdf", "SDF font", "The font will contain mSDF values in its RGB channels and true SDF in its alpha channel", ProcessSDFParams)
    orxFONTGEN_DECLARE_PARAM("t", "textlist", "Characters list", "A list of files containing all the characters that will be displayed using this font, or either ANSI or ASCII to use those character sets", ProcessInputParams)
  };

  // Clears static controller
  orxMemory_Zero(&sstFontGen, sizeof(orxFONTGEN_STATIC));

  // Sets default character size
  orxVector_Set(&(sstFontGen.vCharacterSize), orxFLOAT_0, orxU2F(orxFONTGEN_KU32_DEFAULT_SIZE), orxFLOAT_0);

  // For all params
  for(i = 0; (i < orxARRAY_GET_ITEM_COUNT(astParamList)) && (eResult != orxSTATUS_FAILURE); i++)
  {
    // Registers param
    eResult = orxParam_Register(&astParamList[i]);
  }

  // No font name?
  if(sstFontGen.zFontName == orxNULL)
  {
    // Logs message
    orxFONTGEN_LOG(OUTPUT, "No output provided, defaulting to '%s'.", orxFONTGEN_KZ_DEFAULT_NAME);

    // Uses default one
    sstFontGen.zFontName = orxString_Duplicate(orxFONTGEN_KZ_DEFAULT_NAME);
  }

  // No character size?
  if(sstFontGen.vCharacterSize.fY == orxFLOAT_0)
  {
    // Updates it
    sstFontGen.vCharacterSize.fY = orxU2F(orxFONTGEN_KU32_DEFAULT_SIZE);

    // Logs message
    orxFONTGEN_LOG(SIZE, "Character size defaulting to <%u>.", orxFONTGEN_KU32_DEFAULT_SIZE);
  }

  // Not monospace?
  if(sstFontGen.vCharacterSize.fX == orxFLOAT_0)
  {
    // Logs message
    orxFONTGEN_LOG(MODE, "Output mode set to non-monospace.");
  }

  // Not SDF?
  if(!orxFLAG_TEST(sstFontGen.u32Flags, orxFONTGEN_KU32_STATIC_FLAG_SDF))
  {
    // Logs message
    orxFONTGEN_LOG(TYPE, "Font type set to bitmap (regular).");
  }

  // No character list?
  if(sstFontGen.zCharacterList == orxNULL)
  {
    // Logs message
    orxFONTGEN_LOG(INPUT, "Character list set to: ASCII");

    // Defaults to ASCII
    sstFontGen.zCharacterList = orxString_Duplicate(orxFONT_KZ_ASCII);
  }

  // Done!
  return eResult;
}

static void orxFASTCALL Exit()
{
  // Has font name?
  if(sstFontGen.zFontName)
  {
    // Frees its string
    orxString_Delete(sstFontGen.zFontName);
    sstFontGen.zFontName = orxNULL;
  }

  // Has file name?
  if(sstFontGen.zFileName)
  {
    // Frees its string
    orxString_Delete(sstFontGen.zFileName);
    sstFontGen.zFileName = orxNULL;
  }

  // Has character list?
  if(sstFontGen.zCharacterList)
  {
    // Frees its string
    orxString_Delete(sstFontGen.zCharacterList);
    sstFontGen.zCharacterList = orxNULL;
  }
}

static void Run()
{
  // Ready?
  if(orxFLAG_TEST_ALL(sstFontGen.u32Flags, orxFONTGEN_KU32_STATIC_MASK_READY))
  {
    orxFONT *pstFont;

    // Pushes font section
    orxConfig_PushSection(sstFontGen.zFontName);

    // Sets font properties
    orxConfig_SetString(orxFONT_KZ_CONFIG_TYPEFACE, sstFontGen.zFileName);
    orxConfig_SetVector(orxFONT_KZ_CONFIG_CHARACTER_SIZE, &(sstFontGen.vCharacterSize));
    orxConfig_SetVector(orxFONT_KZ_CONFIG_CHARACTER_PADDING, &(sstFontGen.vCharacterPadding));
    orxConfig_SetStringBlock(orxFONT_KZ_CONFIG_CHARACTER_LIST, sstFontGen.zCharacterList);
    orxConfig_SetBool(orxFONT_KZ_CONFIG_SDF, orxFLAG_TEST(sstFontGen.u32Flags, orxFONTGEN_KU32_STATIC_FLAG_SDF));

    // Pops config section
    orxConfig_PopSection();

    // Has maximum texture width?
    if(sstFontGen.u32MaxWidth > 0)
    {
      // Overrides display value
      orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);
      orxConfig_SetU32(orxDISPLAY_KZ_CONFIG_MAX_TEXTURE_SIZE, sstFontGen.u32MaxWidth);
      orxConfig_PopSection();
    }

    // Creates font
    pstFont = orxFont_CreateFromConfig(sstFontGen.zFontName);

    // Success?
    if(pstFont != orxNULL)
    {
      orxTEXTURE       *pstTexture;
      const orxCHAR    *pcSrc;
      orxCHAR          *pcDst;
      const orxSTRING   zCharacterList;
      const orxSTRING  *azCharacterWidthList;
      orxVECTOR         vCharacterSpacing;
      orxFLOAT          fWidth, fHeight, fCharacterHeight;
      orxU32            u32CharacterCodePoint, u32CharacterCount, i;
      orxCHAR           acBuffer[256], *acWidthBuffer;

      // Clears config
      orxConfig_ClearSection(sstFontGen.zFontName);

      // Pushes font section
      orxConfig_PushSection(sstFontGen.zFontName);

      // Gets font properties
      zCharacterList    = orxFont_GetCharacterList(pstFont);
      fCharacterHeight  = orxFont_GetCharacterHeight(pstFont);
      u32CharacterCount = orxString_GetCharacterCount(sstFontGen.zCharacterList);

      // Allocates width list
      acWidthBuffer         = (orxCHAR *)orxMemory_Allocate(u32CharacterCount * 8, orxMEMORY_TYPE_TEMP);
      azCharacterWidthList  = (const orxSTRING *)orxMemory_Allocate(u32CharacterCount * sizeof(orxSTRING), orxMEMORY_TYPE_TEMP);
      orxASSERT(acWidthBuffer != orxNULL);
      orxASSERT(azCharacterWidthList != orxNULL);
      orxMemory_Zero(acWidthBuffer, u32CharacterCount * 8);

      // For all characters
      for(u32CharacterCodePoint = orxString_GetFirstCharacterCodePoint(zCharacterList, &pcSrc), pcDst = acWidthBuffer, i = 0;
          pcSrc != orxNULL;
          u32CharacterCodePoint = orxString_GetFirstCharacterCodePoint(pcSrc, &pcSrc), pcDst += 8, i++)
      {
        orxFLOAT fCharacterWidth;

        // Gets its width
        fCharacterWidth = orxFont_GetCharacterWidth(pstFont, u32CharacterCodePoint);

        // Prints it
        orxString_NPrint(pcDst, 8, "%g", fCharacterWidth);

        // References it
        azCharacterWidthList[i] = pcDst;
      }

      // Stores them
      orxConfig_SetStringBlock(orxFONT_KZ_CONFIG_CHARACTER_LIST, zCharacterList);
      orxConfig_SetVector(orxFONT_KZ_CONFIG_CHARACTER_SPACING, orxFont_GetCharacterSpacing(pstFont, &vCharacterSpacing));
      orxConfig_SetU32(orxFONT_KZ_CONFIG_CHARACTER_HEIGHT, orxF2U(fCharacterHeight));
      orxConfig_SetBool(orxFONT_KZ_CONFIG_SDF, orxFont_IsSDF(pstFont));
      orxString_NPrint(acBuffer, sizeof(acBuffer), "%s.png", sstFontGen.zFontName);
      orxConfig_SetString(orxFONT_KZ_CONFIG_TEXTURE_NAME, acBuffer);
      orxConfig_SetListString(orxFONT_KZ_CONFIG_CHARACTER_WIDTH_LIST, azCharacterWidthList, u32CharacterCount);

      // Pops config section
      orxConfig_PopSection();

      // Waits until all tasks are complete
      while(orxThread_GetTaskCount() != 0)
        ;

      // Gets font texture
      pstTexture = orxFont_GetTexture(pstFont);

      // Saves texture
      orxDisplay_SaveBitmap(orxTexture_GetBitmap(pstTexture), acBuffer);

      // Gets texture size
      orxTexture_GetSize(pstTexture, &fWidth, &fHeight);

      // Logs messages
      orxFONTGEN_LOG(PROCESS, "Calculated character size:    %4g x %g.", orxFont_GetCharacterWidth(pstFont, (orxString_SearchChar(zCharacterList, ' ') != orxNULL) ? ' ' : orxString_GetFirstCharacterCodePoint(zCharacterList, orxNULL)), fCharacterHeight);
      orxFONTGEN_LOG(PROCESS, "Calculated character spacing: %4g x %g.", vCharacterSpacing.fX, vCharacterSpacing.fY);
      orxFONTGEN_LOG(PROCESS, "Calculated texture size:      %4g x %g.", fWidth, fHeight);
      orxFONTGEN_LOG(PROCESS, "%u glyphs generated in '%s'.", u32CharacterCount, acBuffer);

      // Waits until all tasks are complete
      while(orxThread_GetTaskCount() != 0)
        ;

      // Logs message
      orxFONTGEN_LOG(SAVE, "'%s': SUCCESS.", acBuffer);

      // Gets config file name
      orxString_NPrint(acBuffer, sizeof(acBuffer), "%s.ini", sstFontGen.zFontName);

      // Saves it
      if(orxConfig_Save(acBuffer, orxFALSE, SaveFilter) != orxSTATUS_FAILURE)
      {
        // Logs message
        orxFONTGEN_LOG(SAVE, "'%s': SUCCESS.", acBuffer);
      }
      else
      {
        // Logs message
        orxFONTGEN_LOG(SAVE, "'%s': FAILURE.", acBuffer);
      }

      // Frees buffers
      orxMemory_Free(azCharacterWidthList);
      orxMemory_Free(acWidthBuffer);
    }
    else
    {
      // Logs message
      orxFONTGEN_LOG(LOAD, "Failed to create font '%s'.", sstFontGen.zFontName);
    }
  }
}

int main(int argc, char **argv)
{
  // Inits the Debug System
  orxDEBUG_INIT();

  // Registers main module
  orxModule_Register(orxMODULE_ID_MAIN, "MAIN", Setup, Init, Exit);

  // Sends the command line arguments to orxParam module
  if(orxParam_SetArgs(argc, argv) != orxSTATUS_FAILURE)
  {
    // Inits the engine
    if(orxModule_Init(orxMODULE_ID_MAIN) != orxSTATUS_FAILURE)
    {
      // Runs
      Run();

      // Exits from engine
      orxModule_Exit(orxMODULE_ID_MAIN);
    }

    // Clears params
    orxParam_SetArgs(0, orxNULL);
  }

  // Exits from the Debug system
  orxDEBUG_EXIT();

  // Done!
  return EXIT_SUCCESS;
}
