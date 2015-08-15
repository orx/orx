/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2010 Orx-Project
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

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#undef STB_IMAGE_WRITE_IMPLEMENTATION

#include "ft2build.h"
#include FT_FREETYPE_H


/** Module flags
 */
#define orxFONTGEN_KU32_STATIC_FLAG_NONE            0x00000000  /**< No flags */

#define orxFONTGEN_KU32_STATIC_FLAG_FONT            0x00000001  /**< Font flag */
#define orxFONTGEN_KU32_STATIC_FLAG_SIZE            0x00000002  /**< Size flag */
#define orxFONTGEN_KU32_STATIC_FLAG_MONOSPACE       0x00000004  /**< Monospace flag */
#define orxFONTGEN_KU32_STATIC_FLAG_ADVANCE         0x00000008  /**< Advance flag */

#define orxFONTGEN_KU32_STATIC_MASK_READY           0x00000003  /**< Ready mask */

#define orxFONTGEN_KU32_STATIC_MASK_ALL             0xFFFFFFFF  /**< All mask */


/** Misc defines
 */
#define orxFONTGEN_KZ_DEFAULT_NAME                  "orxFont"

#define orxFONTGEN_KU32_BUFFER_SIZE                 8192

#define orxFONTGEN_KU32_CHARACTER_TABLE_SIZE        256

#define orxFONTGEN_KZ_UTF8_BOM                      "\xEF\xBB\xBF"
#define orxFONTGEN_KU32_UTF8_BOM_LENGTH             3

#define orxFONTGEN_KZ_LOG_TAG_LENGTH                "10"

#if defined(__orxGCC__) || defined(__orxLLVM__)

  #define orxFONTGEN_LOG(TAG, FORMAT, ...) orxLOG("%-" orxFONTGEN_KZ_LOG_TAG_LENGTH "s" FORMAT, "[" #TAG "]", ##__VA_ARGS__)

#else // __orxGCC__ || __orxLLVM__

  #define orxFONTGEN_LOG(TAG, FORMAT, ...) orxLOG("%-" orxFONTGEN_KZ_LOG_TAG_LENGTH "s" FORMAT, "[" #TAG "]", __VA_ARGS__)

#endif //__orxGCC__ || __orxLLVM__


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Glyph structure
 */
typedef struct __orxFONTGEN_GLYPH_t
{
  orxLINKLIST_NODE  stNode;
  orxU32            u32Index;
  orxU32            u32CodePoint;

} orxFONTGEN_GLYPH;

/** Static structure
 */
typedef struct __orxFONTGEN_STATIC_t
{
  orxSTRING       zFontName;
  orxVECTOR       vCharacterSize;
  orxVECTOR       vCharacterSpacing;
  orxFLOAT        fFontScale;
  orxFLOAT        fPadding;
  orxHASHTABLE   *pstCharacterTable;
  orxBANK        *pstGlyphBank;
  orxU32          u32Flags;
  orxLINKLIST     stGlyphList;
  FT_Library      pstFontLibrary;
  FT_Face         pstFontFace;

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

static orxSTATUS orxFASTCALL ParseTextFile(const orxSTRING _zFileName)
{
  orxFILE  *pstFile;
  orxSTATUS eResult;

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
            // Not already in table?
            if(orxHashTable_Get(sstFontGen.pstCharacterTable, u32CharacterCodePoint) == orxNULL)
            {
              orxU32 u32GlyphIndex;

              // Gets character's glyph index
              u32GlyphIndex = (orxU32)FT_Get_Char_Index(sstFontGen.pstFontFace, (FT_ULong)u32CharacterCodePoint);

              // Valid?
              if(u32GlyphIndex)
              {
                orxFONTGEN_GLYPH *pstGlyph;

                // Allocates glyph
                pstGlyph = (orxFONTGEN_GLYPH *)orxBank_Allocate(sstFontGen.pstGlyphBank);

                // Checks
                orxASSERT(pstGlyph);

                // Inits it
                pstGlyph->u32Index      = u32GlyphIndex;
                pstGlyph->u32CodePoint  = u32CharacterCodePoint;

                // Adds it
                if(orxHashTable_Add(sstFontGen.pstCharacterTable, u32CharacterCodePoint, (void *)pstGlyph) != orxSTATUS_FAILURE)
                {
                  orxFONTGEN_GLYPH *pstSearchGlyph;

                  // Finds position
                  for(pstSearchGlyph = (orxFONTGEN_GLYPH *)orxLinkList_GetFirst(&sstFontGen.stGlyphList);
                      pstSearchGlyph && (u32CharacterCodePoint > pstSearchGlyph->u32CodePoint);
                      pstSearchGlyph = (orxFONTGEN_GLYPH *)orxLinkList_GetNext(&pstSearchGlyph->stNode));

                  // Valid?
                  if(pstSearchGlyph)
                  {
                    // Adds it before
                    orxLinkList_AddBefore(&pstSearchGlyph->stNode, &pstGlyph->stNode);
                  }
                  else
                  {
                    // Adds it at the end
                    orxLinkList_AddEnd(&sstFontGen.stGlyphList, &pstGlyph->stNode);
                  }

                  // Updates counter
                  u32Counter++;
                }
                else
                {
                  // Logs message
                  orxFONTGEN_LOG(LOAD, "Character '0x%X': couldn't add to table, skipping.", u32CharacterCodePoint);
                }
              }
              else
              {
                // Adds it
                orxHashTable_Add(sstFontGen.pstCharacterTable, u32CharacterCodePoint, (void *)sstFontGen.pstCharacterTable);

                // Logs message
                orxFONTGEN_LOG(LOAD, "Character '0x%X': glyph not found in font, skipping.", u32CharacterCodePoint);
              }
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
              orxFONTGEN_LOG(LOAD, "Invalid character code point '0x%X', skipping.", u32CharacterCodePoint);
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
    orxFONTGEN_LOG(LOAD, "'%s': added %d characters.", _zFileName, u32Counter);

    // Updates result
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    // Updates result
    eResult = orxSTATUS_FAILURE;
  }

  // Done!
  return eResult;
}

static orxSTATUS orxFASTCALL ProcessInputParams(orxU32 _u32ParamCount, const orxSTRING _azParams[])
{
  orxU32    i;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  // Has a valid key parameter?
  if(_u32ParamCount > 1)
  {
    // For all input files
    for(i = 1; i < _u32ParamCount; i++)
    {
      // Parses it
      if(ParseTextFile(_azParams[i]) != orxSTATUS_FAILURE)
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
  }
  else
  {
    // Logs message
    orxFONTGEN_LOG(INPUT, "No valid file list found, aborting");
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
  else
  {
    // Uses default one
    sstFontGen.zFontName = orxString_Duplicate(orxFONTGEN_KZ_DEFAULT_NAME);

    // Logs message
    orxFONTGEN_LOG(OUTPUT, "No valid output found, using default '%s'.", orxFONTGEN_KZ_DEFAULT_NAME);
  }

  // Done!
  return eResult;
}

static orxSTATUS orxFASTCALL ProcessSizeParams(orxU32 _u32ParamCount, const orxSTRING _azParams[])
{
  orxSTATUS eResult;

  // Has a valid size parameter?
  if(_u32ParamCount > 1)
  {
    orxFLOAT fSize;

    // Gets it
    if((eResult = orxString_ToFloat(_azParams[1], &fSize, orxNULL)) != orxSTATUS_FAILURE)
    {
      // Stores it
      sstFontGen.vCharacterSize.fY = fSize;

      // Updates status
      orxFLAG_SET(sstFontGen.u32Flags, orxFONTGEN_KU32_STATIC_FLAG_SIZE, orxFONTGEN_KU32_STATIC_FLAG_NONE);

      // Logs message
      orxFONTGEN_LOG(SIZE, "Character size set to '%g'.", fSize);
    }
    else
    {
      // Logs message
      orxFONTGEN_LOG(SIZE, "Invalid character size found in '%s', aborting.", _azParams[1]);
    }
  }
  else
  {
    // Logs message
    orxFONTGEN_LOG(SIZE, "No character size found, aborting.");

    // Updates result
    eResult = orxSTATUS_FAILURE;
  }

  // Done!
  return eResult;
}

static orxSTATUS orxFASTCALL ProcessPaddingParams(orxU32 _u32ParamCount, const orxSTRING _azParams[])
{
  orxSTATUS eResult;

  // Has a valid size parameter?
  if(_u32ParamCount > 1)
  {
    orxFLOAT fPadding;

    // Gets it
    if((eResult = orxString_ToFloat(_azParams[1], &fPadding, orxNULL)) != orxSTATUS_FAILURE)
    {
      // Stores it
      sstFontGen.fPadding = orx2F(2.0f) * fPadding;

      // Logs message
      orxFONTGEN_LOG(PADDING, "Character padding set to '%g'.", fPadding);
    }
    else
    {
      // Logs message
      orxFONTGEN_LOG(PADDING, "Invalid character padding found in '%s', aborting.", _azParams[1]);
    }
  }
  else
  {
    // Updates result
    eResult = orxSTATUS_SUCCESS;
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
    // Inits FreeType
    if(!FT_Init_FreeType(&sstFontGen.pstFontLibrary))
    {
      // Loads font's default face
      if(!FT_New_Face(sstFontGen.pstFontLibrary, _azParams[1], 0, &sstFontGen.pstFontFace))
      {
        // Sets unicode map
        if(!FT_Select_Charmap(sstFontGen.pstFontFace, ft_encoding_unicode))
        {
          // Updates character size
          sstFontGen.vCharacterSize.fX = sstFontGen.vCharacterSize.fY;

          // Updates character spacing
          sstFontGen.vCharacterSpacing.fX = orx2F(2.0f);
          sstFontGen.vCharacterSpacing.fY = orx2F(2.0f);

          // Stores scale
          sstFontGen.fFontScale = sstFontGen.vCharacterSize.fY / orxS2F(sstFontGen.pstFontFace->bbox.yMax - sstFontGen.pstFontFace->bbox.yMin);

          // Sets pixel's size
          eResult = FT_Set_Pixel_Sizes(sstFontGen.pstFontFace, (FT_UInt)orxF2U(sstFontGen.vCharacterSize.fX) - 2, (FT_UInt)orxF2U(sstFontGen.vCharacterSize.fY) - 2) ? orxSTATUS_FAILURE : orxSTATUS_SUCCESS;
        }
      }
    }

    // Success?
    if(eResult != orxSTATUS_FAILURE)
    {
      // Updates status
      orxFLAG_SET(sstFontGen.u32Flags, orxFONTGEN_KU32_STATIC_FLAG_FONT, orxFONTGEN_KU32_STATIC_FLAG_NONE);

      // Logs message
      orxFONTGEN_LOG(FONT, "Using font '%s'.", _azParams[1]);
    }
    else
    {
      // Logs message
      orxFONTGEN_LOG(FONT, "Couldn't load font '%s'.", _azParams[1]);
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
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // Defined?
  if(_u32ParamCount > 0)
  {
    // Updates status flags
    orxFLAG_SET(sstFontGen.u32Flags, orxFONTGEN_KU32_STATIC_FLAG_MONOSPACE, orxFONTGEN_KU32_STATIC_FLAG_NONE);

    // Logs message
    orxFONTGEN_LOG(MODE, "Output mode set to monospace.");
  }

  // Done!
  return eResult;
}

static orxSTATUS orxFASTCALL ProcessAdvanceParams(orxU32 _u32ParamCount, const orxSTRING _azParams[])
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // Defined?
  if(_u32ParamCount > 0)
  {
    // Updates status flags
    orxFLAG_SET(sstFontGen.u32Flags, orxFONTGEN_KU32_STATIC_FLAG_ADVANCE, orxFONTGEN_KU32_STATIC_FLAG_NONE);

    // Logs message
    orxFONTGEN_LOG(PACKING, "Using original glyph advance values.");
  }

  // Done!
  return eResult;
}

static void orxFASTCALL Setup()
{
  // Adds module dependencies
  orxModule_AddDependency(orxMODULE_ID_MAIN, orxMODULE_ID_PARAM);
  orxModule_AddDependency(orxMODULE_ID_MAIN, orxMODULE_ID_FILE);
}

static orxSTATUS orxFASTCALL Init()
{
#define orxFONTGEN_DECLARE_PARAM(SN, LN, SD, LD, FN) {orxPARAM_KU32_FLAG_STOP_ON_ERROR, SN, LN, SD, LD, &FN},

  orxU32    i;
  orxSTATUS eResult = orxSTATUS_SUCCESS;
  orxPARAM  astParamList[] =
  {
    orxFONTGEN_DECLARE_PARAM("o", "output", "Font output name", "Font base output name: .png will be added to the image and .ini will be added to the config file", ProcessOutputParams)
    orxFONTGEN_DECLARE_PARAM("s", "size", "Size (height) of characters", "Height to use for characters defined with this font", ProcessSizeParams)
    orxFONTGEN_DECLARE_PARAM("p", "padding", "Character padding", "Extra padding added to all characters on both dimensions (width and height)", ProcessPaddingParams)
    orxFONTGEN_DECLARE_PARAM("f", "font", "Input font file", "TrueType font (usually .ttf) used to generate all the required glyphs", ProcessFontParams)
    orxFONTGEN_DECLARE_PARAM("t", "textlist", "List of input text files", "List of text files containing all the characters that will be displayed using this font", ProcessInputParams)
    orxFONTGEN_DECLARE_PARAM("m", "monospace", "Monospaced font", "Will output a monospace (ie. fixed-width) font", ProcessMonospaceParams)
    orxFONTGEN_DECLARE_PARAM("a", "advance", "Use glyph advance values for non-monospace fonts", "In non-monospace mode only: the font's original glyph advance values will be used instead of packing glyphs as efficiently as possible", ProcessAdvanceParams)
  };

  // Clears static controller
  orxMemory_Zero(&sstFontGen, sizeof(orxFONTGEN_STATIC));

  // Creates character table
  sstFontGen.pstCharacterTable = orxHashTable_Create(orxFONTGEN_KU32_CHARACTER_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

  // Creates glyph bank
  sstFontGen.pstGlyphBank = orxBank_Create(orxFONTGEN_KU32_CHARACTER_TABLE_SIZE, sizeof(orxFONTGEN_GLYPH), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

  // For all params but last
  for(i = 0; (i < (sizeof(astParamList) / sizeof(astParamList[0])) - 1) && (eResult != orxSTATUS_FAILURE); i++)
  {
    // Registers param
    eResult = orxParam_Register(&astParamList[i]);
  }

  // Not in monospace mode?
  if(!orxFLAG_TEST(sstFontGen.u32Flags, orxFONTGEN_KU32_STATIC_FLAG_MONOSPACE))
  {
    // Logs message
    orxFONTGEN_LOG(MODE, "Output mode set to non-monospace.");

    // Registers last param
    eResult = orxParam_Register(&astParamList[(sizeof(astParamList) / sizeof(astParamList[0])) - 1]);

    // Not using original advance values?
    if(!orxFLAG_TEST(sstFontGen.u32Flags, orxFONTGEN_KU32_STATIC_FLAG_ADVANCE))
    {
      // Logs message
      orxFONTGEN_LOG(PACKING, "Characters will be packed.");
    }
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

  // Deletes character table
  orxHashTable_Delete(sstFontGen.pstCharacterTable);

  // Deletes glyph bank
  orxBank_Delete(sstFontGen.pstGlyphBank);

  // Has face?
  if(sstFontGen.pstFontFace)
  {
    // Deletes it
    FT_Done_Face(sstFontGen.pstFontFace);
  }

  // Has library?
  if(sstFontGen.pstFontLibrary)
  {
    // Exits from it
    FT_Done_FreeType(sstFontGen.pstFontLibrary);
  }
}

static void Run()
{
  // Ready?
  if(orxFLAG_TEST_ALL(sstFontGen.u32Flags, orxFONTGEN_KU32_STATIC_MASK_READY))
  {
    orxU32 u32Counter;

    // Gets glyph list's counter
    u32Counter = orxLinkList_GetCounter(&sstFontGen.stGlyphList);

    // Valid?
    if(u32Counter)
    {
      orxS32      s32Width, s32Height, s32BaseLine, s32MaxAscend, s32MaxDescend;
      orxSTRING  *azWidthList = orxNULL;
      orxFLOAT    fWidth, fHeight;
      orxU8      *pu8ImageBuffer;

      // Not monospaced?
      if(!orxFLAG_TEST(sstFontGen.u32Flags, orxFONTGEN_KU32_STATIC_FLAG_MONOSPACE))
      {
        orxU32 i;

        // Allocates width list
        azWidthList = (orxSTRING *)orxMemory_Allocate(u32Counter * sizeof(orxSTRING), orxMEMORY_TYPE_MAIN);

        // Checks
        orxASSERT(azWidthList);

        // For all strings
        for(i = 0; i < u32Counter; i++)
        {
          azWidthList[i] = (orxSTRING)orxMemory_Allocate(8 * sizeof(orxCHAR), orxMEMORY_TYPE_MAIN);
        }
      }

      // No font name?
      if(!sstFontGen.zFontName)
      {
        // Uses default one
        sstFontGen.zFontName = orxString_Duplicate(orxFONTGEN_KZ_DEFAULT_NAME);

        // Logs message
        orxFONTGEN_LOG(OUTPUT, "No output name specified, defaulting to '%s'.", orxFONTGEN_KZ_DEFAULT_NAME);
      }

      // Is not monospaced?
      if(!orxFLAG_TEST(sstFontGen.u32Flags, orxFONTGEN_KU32_STATIC_FLAG_MONOSPACE))
      {
        orxFONTGEN_GLYPH *pstGlyph;
        orxS32            s32LargestWidth = 0;

        // For all defined glyphs
        for(pstGlyph = (orxFONTGEN_GLYPH *)orxLinkList_GetFirst(&sstFontGen.stGlyphList);
            pstGlyph;
            pstGlyph = (orxFONTGEN_GLYPH *)orxLinkList_GetNext(&pstGlyph->stNode))
        {
          orxS32    s32CharacterWidth;
          FT_Error  eError;

          // Loads rendered glyph
          eError = FT_Load_Glyph(sstFontGen.pstFontFace, (FT_UInt)pstGlyph->u32Index, FT_LOAD_RENDER);
          orxASSERT(!eError);

          // Use original advance value?
          if(orxFLAG_TEST(sstFontGen.u32Flags, orxFONTGEN_KU32_STATIC_FLAG_ADVANCE))
          {
            // Gets character width
            s32CharacterWidth = sstFontGen.pstFontFace->glyph->advance.x >> 6;
          }
          else
          {
            // Gets character width
            s32CharacterWidth = orxMAX((orxS32)sstFontGen.pstFontFace->glyph->bitmap_left, 0) + (orxS32)sstFontGen.pstFontFace->glyph->bitmap.width;
          }

          // Updates largest character width
          s32LargestWidth = orxMAX(s32LargestWidth, s32CharacterWidth);
        }

        // Updates character width
        sstFontGen.vCharacterSize.fX = orxS2F(s32LargestWidth);
      }

      // Gets width & height
      fWidth  = orxMath_Floor(orxMath_Sqrt(orxU2F(u32Counter)));
      fHeight = orxMath_Ceil(orxU2F(u32Counter) / fWidth);
      s32Width  = orxF2S((fWidth * (sstFontGen.vCharacterSize.fX + sstFontGen.fPadding)) + (sstFontGen.vCharacterSpacing.fX * orxMAX(fWidth - orxFLOAT_1, orxFLOAT_0)));
      s32Height = orxF2S((fHeight * (sstFontGen.vCharacterSize.fY + sstFontGen.fPadding)) + (sstFontGen.vCharacterSpacing.fY * orxMAX(fHeight - orxFLOAT_1, orxFLOAT_0)));

      // Is not monospaced?
      if(!orxFLAG_TEST(sstFontGen.u32Flags, orxFONTGEN_KU32_STATIC_FLAG_MONOSPACE))
      {
        orxS32            s32X, s32Y;
        orxFONTGEN_GLYPH *pstGlyph;

        // For all defined glyphs
        for(pstGlyph = (orxFONTGEN_GLYPH *)orxLinkList_GetFirst(&sstFontGen.stGlyphList), s32X = 0, s32Y = 0, s32MaxAscend = 0, s32MaxDescend = 0;
            pstGlyph;
            pstGlyph = (orxFONTGEN_GLYPH *)orxLinkList_GetNext(&pstGlyph->stNode))
        {
          orxS32    s32CharacterWidth;
          FT_Error  eError;

          // Loads rendered glyph
          eError = FT_Load_Glyph(sstFontGen.pstFontFace, (FT_UInt)pstGlyph->u32Index, FT_LOAD_RENDER);
          orxASSERT(!eError);

          // Is ascend bigger than any previous?
          if((orxS32)sstFontGen.pstFontFace->glyph->bitmap_top > s32MaxAscend)
          {
            // Stores it
            s32MaxAscend = (orxS32)sstFontGen.pstFontFace->glyph->bitmap_top;
          }

          // Is descend bigger than any previous?
          if((orxS32)sstFontGen.pstFontFace->glyph->bitmap.rows - (orxS32)sstFontGen.pstFontFace->glyph->bitmap_top > s32MaxDescend)
          {
            // Stores it
            s32MaxDescend = (orxS32)sstFontGen.pstFontFace->glyph->bitmap.rows - (orxS32)sstFontGen.pstFontFace->glyph->bitmap_top;
          }

          // Use original advance value?
          if(orxFLAG_TEST(sstFontGen.u32Flags, orxFONTGEN_KU32_STATIC_FLAG_ADVANCE))
          {
            // Gets character width
            s32CharacterWidth = sstFontGen.pstFontFace->glyph->advance.x >> 6;
          }
          else
          {
            // Gets character width
            s32CharacterWidth = orxMAX((orxS32)sstFontGen.pstFontFace->glyph->bitmap_left, 0) + (orxS32)sstFontGen.pstFontFace->glyph->bitmap.width;

            // No width?
            if(s32CharacterWidth == 0)
            {
              // Uses its advance value
              s32CharacterWidth = sstFontGen.pstFontFace->glyph->advance.x >> 6;
            }
          }

          // Adds padding
          s32CharacterWidth = orxF2S(sstFontGen.fPadding) + ((s32CharacterWidth > 0) ? s32CharacterWidth : orxF2S(sstFontGen.vCharacterSize.fX));

          // Next line?
          if(s32X + s32CharacterWidth > s32Width)
          {
            s32X  = 0;
            s32Y += orxF2S(sstFontGen.vCharacterSize.fY + sstFontGen.fPadding + sstFontGen.vCharacterSpacing.fY);
          }

          // Updates position
          s32X += s32CharacterWidth + orxF2S(sstFontGen.vCharacterSpacing.fX);
        }

        // Updates height
        s32Height = s32Y + orxF2S(sstFontGen.vCharacterSize.fY + sstFontGen.fPadding);
      }
      else
      {
        orxFONTGEN_GLYPH *pstGlyph;

        // For all defined glyphs
        for(pstGlyph = (orxFONTGEN_GLYPH *)orxLinkList_GetFirst(&sstFontGen.stGlyphList), s32MaxAscend = 0, s32MaxDescend = 0;
            pstGlyph;
            pstGlyph = (orxFONTGEN_GLYPH *)orxLinkList_GetNext(&pstGlyph->stNode))
        {
          FT_Error eError;

          // Loads rendered glyph
          eError = FT_Load_Glyph(sstFontGen.pstFontFace, (FT_UInt)pstGlyph->u32Index, FT_LOAD_RENDER);
          orxASSERT(!eError);

          // Is ascend bigger than any previous?
          if(sstFontGen.pstFontFace->glyph->bitmap_top > s32MaxAscend)
          {
            // Stores it
            s32MaxAscend = sstFontGen.pstFontFace->glyph->bitmap_top;
          }

          // Is descend bigger than any previous?
          if(sstFontGen.pstFontFace->glyph->bitmap.rows - sstFontGen.pstFontFace->glyph->bitmap_top - 1 > s32MaxDescend)
          {
            // Stores it
            s32MaxDescend = sstFontGen.pstFontFace->glyph->bitmap.rows - sstFontGen.pstFontFace->glyph->bitmap_top - 1;
          }
        }
      }

      // Can't fit all characters?
      if(s32MaxAscend + s32MaxDescend + 1 > orxF2S(sstFontGen.vCharacterSize.fY))
      {
        // Logs message
        orxFONTGEN_LOG(PROCESS, "Some characters will be cropped (height=%d/%d).", s32MaxAscend + s32MaxDescend + 1, orxF2S(sstFontGen.vCharacterSize.fY));
      }

      // Gets baseline (using ascender)
      s32BaseLine = sstFontGen.pstFontFace->ascender >> 6;

      // Is baseline incorrect?
      if((s32BaseLine < s32MaxAscend)
      || (s32BaseLine + s32MaxDescend + 1) > orxF2S(sstFontGen.vCharacterSize.fY))
      {
        // Logs messages
        orxFONTGEN_LOG(PROCESS, "Adjusting baseline to %d (%+d).", s32MaxAscend, s32MaxAscend - s32BaseLine);

        // Adjusts value
        s32BaseLine = s32MaxAscend;
      }

      // Logs messages
      orxFONTGEN_LOG(PROCESS, "Calculated character size:    %4g x %g.", sstFontGen.vCharacterSize.fX + sstFontGen.fPadding, sstFontGen.vCharacterSize.fY + sstFontGen.fPadding);
      orxFONTGEN_LOG(PROCESS, "Calculated character spacing: %4g x %g.", sstFontGen.vCharacterSpacing.fX, sstFontGen.vCharacterSpacing.fY);
      orxFONTGEN_LOG(PROCESS, "Calculated texture size:      %4d x %d.", s32Width, s32Height);

      // Allocates image buffer
      pu8ImageBuffer = (orxU8 *)orxMemory_Allocate(s32Width * s32Height * sizeof(orxRGBA), orxMEMORY_TYPE_MAIN);

      // Valid?
      if(pu8ImageBuffer)
      {
        orxU32            u32Size, s32Index, i;
        orxS32            s32X, s32Y;
        orxFONTGEN_GLYPH *pstGlyph;
        orxCHAR           acBuffer[orxFONTGEN_KU32_BUFFER_SIZE], *pc;

        // Clears bitmap
        for(i = 0; i < s32Width * s32Height * sizeof(orxRGBA); i+= 4)
        {
          pu8ImageBuffer[i]     = 0xFF;
          pu8ImageBuffer[i + 1] = 0xFF;
          pu8ImageBuffer[i + 2] = 0xFF;
          pu8ImageBuffer[i + 3] = 0x00;
        }

        // For all defined glyphs
        for(s32Index = 0, pstGlyph = (orxFONTGEN_GLYPH *)orxLinkList_GetFirst(&sstFontGen.stGlyphList), pc = acBuffer, u32Size = orxFONTGEN_KU32_BUFFER_SIZE - 1, s32X = 0, s32Y = s32BaseLine;
            pstGlyph;
            s32Index++, pstGlyph = (orxFONTGEN_GLYPH *)orxLinkList_GetNext(&pstGlyph->stNode))
        {
          orxU32 u32Offset;

          // Adds it to character list
          u32Offset = orxString_PrintUTF8Character(pc, u32Size, pstGlyph->u32CodePoint);

          // Success?
          if(u32Offset != orxU32_UNDEFINED)
          {
            FT_Error  eError;
            orxS32    s32CharacterWidth;

            // Loads rendered glyph
            eError = FT_Load_Glyph(sstFontGen.pstFontFace, (FT_UInt)pstGlyph->u32Index, FT_LOAD_RENDER);
            orxASSERT(!eError);

            // Is monospaced?
            if(orxFLAG_TEST(sstFontGen.u32Flags, orxFONTGEN_KU32_STATIC_FLAG_MONOSPACE))
            {
              // Updates position
              s32CharacterWidth = orxF2S(sstFontGen.vCharacterSize.fX + sstFontGen.fPadding);
            }
            else
            {
              // Use original advance value?
              if(orxFLAG_TEST(sstFontGen.u32Flags, orxFONTGEN_KU32_STATIC_FLAG_ADVANCE))
              {
                // Gets character width
                s32CharacterWidth = sstFontGen.pstFontFace->glyph->advance.x >> 6;
              }
              else
              {
                // Gets character width
                s32CharacterWidth = orxMAX((orxS32)sstFontGen.pstFontFace->glyph->bitmap_left, 0) + (orxS32)sstFontGen.pstFontFace->glyph->bitmap.width;

                // No width?
                if(s32CharacterWidth == 0)
                {
                  // Uses its advance value
                  s32CharacterWidth = sstFontGen.pstFontFace->glyph->advance.x >> 6;
                }
              }

              // Adds padding
              s32CharacterWidth = orxF2S(sstFontGen.fPadding) + ((s32CharacterWidth > 0) ? s32CharacterWidth : orxF2S(sstFontGen.vCharacterSize.fX));

              // Stores its value
              orxString_NPrint(azWidthList[s32Index], 8, "%d", s32CharacterWidth);
            }

            // Next line?
            if(s32X + s32CharacterWidth > s32Width)
            {
              s32X  = 0;
              s32Y += orxF2S(sstFontGen.vCharacterSize.fY + sstFontGen.fPadding + sstFontGen.vCharacterSpacing.fY);
            }

            // Has data?
            if(sstFontGen.pstFontFace->glyph->bitmap.buffer)
            {
              orxS32  s32AdjustedX, s32AdjustedY, s32AdjustedHeight, s32DeltaHeight, i;
              orxU8  *pu8Src, *pu8Dst;

              // Is monospaced?
              if(orxFLAG_TEST(sstFontGen.u32Flags, orxFONTGEN_KU32_STATIC_FLAG_MONOSPACE))
              {
                // Gets adjusted X position
                s32AdjustedX = s32X + (s32CharacterWidth - (orxS32)sstFontGen.pstFontFace->glyph->bitmap.width) / 2;
              }
              else
              {
                // Gets adjusted X position
                s32AdjustedX = s32X + orxF2S(orx2F(0.5f) * sstFontGen.fPadding) + orxMAX((orxS32)sstFontGen.pstFontFace->glyph->bitmap_left, 0);
              }

              // Gets adjusted Y position
              s32AdjustedY = s32Y + orxF2S(orx2F(0.5f) * sstFontGen.fPadding) - (orxS32)sstFontGen.pstFontFace->glyph->bitmap_top;

              // Gets default height
              s32AdjustedHeight = (orxS32)sstFontGen.pstFontFace->glyph->bitmap.rows;

              // Gets delta height
              s32DeltaHeight = s32AdjustedHeight - (orxF2S(sstFontGen.vCharacterSize.fY) - s32BaseLine + (orxS32)sstFontGen.pstFontFace->glyph->bitmap_top);

              // Needs cropping?
              if(s32DeltaHeight > 0)
              {
                // Adjusts height
                s32AdjustedHeight -= s32DeltaHeight;

                // Terminates string
                *(pc + u32Offset) = orxCHAR_NULL;

                // Logs message
                orxFONTGEN_LOG(PROCESS, "Cropping %d rows from character '%s'.", s32DeltaHeight, pc);
              }

              // For all rows
              for(i = 0, pu8Src = (orxU8 *)sstFontGen.pstFontFace->glyph->bitmap.buffer, pu8Dst = pu8ImageBuffer + sizeof(orxRGBA) * ((s32AdjustedY * s32Width) + s32AdjustedX);
                  i < s32AdjustedHeight;
                  i++, pu8Src += orxMAX(sstFontGen.pstFontFace->glyph->bitmap.pitch, -sstFontGen.pstFontFace->glyph->bitmap.pitch) - sstFontGen.pstFontFace->glyph->bitmap.width, pu8Dst += sizeof(orxRGBA) * (s32Width - (orxS32)sstFontGen.pstFontFace->glyph->bitmap.width))
              {
                orxS32 j;

                // For all columns
                for(j = 0;
                    j < (orxS32)sstFontGen.pstFontFace->glyph->bitmap.width;
                    j++, pu8Src++, pu8Dst += sizeof(orxRGBA))
                {
                  // Sets texture's pixel
                  pu8Dst[3] = pu8Src[0];
                }
              }
            }

            // Updates position
            s32X += s32CharacterWidth + orxF2S(sstFontGen.vCharacterSpacing.fX);

            // Updates character pointer & size
            pc      += u32Offset;
            u32Size -= u32Offset;
          }
          else
          {
            // Logs message
            orxFONTGEN_LOG(PROCESS, "Too many characters defined for a single font, stopping.");

            break;
          }
        }

        // Ends character list
        *pc = orxCHAR_NULL;

        // Pushes font section
        orxConfig_PushSection(sstFontGen.zFontName);

        // Updates character size with padding
        sstFontGen.vCharacterSize.fX += sstFontGen.fPadding;
        sstFontGen.vCharacterSize.fY += sstFontGen.fPadding;

        // Is monospaced?
        if(orxFLAG_TEST(sstFontGen.u32Flags, orxFONTGEN_KU32_STATIC_FLAG_MONOSPACE))
        {
          // Stores font info
          orxConfig_SetStringBlock("CharacterList", acBuffer);
          orxConfig_SetVector("CharacterSize", &sstFontGen.vCharacterSize);
          orxConfig_SetVector("CharacterSpacing", &sstFontGen.vCharacterSpacing);
          orxString_NPrint(acBuffer, orxFONTGEN_KU32_BUFFER_SIZE, "%s.png", sstFontGen.zFontName);
          orxConfig_SetString("Texture", acBuffer);
        }
        else
        {
          orxU32 i;

          // Stores font info
          orxConfig_SetStringBlock("CharacterList", acBuffer);
          orxConfig_SetListString("CharacterWidthList", (const orxSTRING *)azWidthList, u32Counter);
          orxConfig_SetFloat("CharacterHeight", sstFontGen.vCharacterSize.fY);
          orxConfig_SetVector("CharacterSpacing", &sstFontGen.vCharacterSpacing);
          orxString_NPrint(acBuffer, orxFONTGEN_KU32_BUFFER_SIZE, "%s.png", sstFontGen.zFontName);
          orxConfig_SetString("Texture", acBuffer);

          // For all width strings
          for(i = 0; i < u32Counter; i++)
          {
            // Deletes it
            orxMemory_Free(azWidthList[i]);
          }

          // Deletes width list
          orxMemory_Free(azWidthList);
        }

        // Pops config
        orxConfig_PopSection();

        // Saves texture
        stbi_write_png(acBuffer, s32Width, s32Height, sizeof(orxRGBA), pu8ImageBuffer, 0);

        // Logs message
        orxFONTGEN_LOG(PROCESS, "%d glyphs generated in '%s'.", u32Counter, acBuffer);

        // Gets config file name
        orxString_NPrint(acBuffer, orxFONTGEN_KU32_BUFFER_SIZE, "%s.ini", sstFontGen.zFontName);

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

        // Frees image buffer
        orxMemory_Free(pu8ImageBuffer);
      }
      else
      {
        // Logs message
        orxFONTGEN_LOG(PROCESS, "Couldn't allocate memory for bitmap (%d x %d), aborting.", s32Width, s32Height);
      }
    }
    else
    {
      // Logs message
      orxFONTGEN_LOG(PROCESS, "No characters to output, aborting.");
    }
  }
  else
  {
    // No size?
    if(!orxFLAG_TEST(sstFontGen.u32Flags, orxFONTGEN_KU32_STATIC_FLAG_SIZE))
    {
      // Logs message
      orxFONTGEN_LOG(SIZE, "No character size found, aborting.");
    }
    // No font?
    else if(!orxFLAG_TEST(sstFontGen.u32Flags, orxFONTGEN_KU32_STATIC_FLAG_FONT))
    {
      // Logs message
      orxFONTGEN_LOG(FONT, "No font specified, aborting.");
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
      // Displays help
      if(orxParam_DisplayHelp() != orxSTATUS_FAILURE)
      {
        // Runs
        Run();
      }

      // Exits from engine
      orxModule_Exit(orxMODULE_ID_MAIN);
    }
  }

  // Exits from the Debug system
  orxDEBUG_EXIT();

  // Done!
  return EXIT_SUCCESS;
}
