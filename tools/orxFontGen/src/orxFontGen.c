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

#include "SOIL.h"

#include "ft2build.h"
#include FT_FREETYPE_H


/** Module flags
 */
#define orxFONTGEN_KU32_STATIC_FLAG_NONE            0x00000000  /**< No flags */

#define orxFONTGEN_KU32_STATIC_FLAG_FONT            0x00000001  /**< Font flag */
#define orxFONTGEN_KU32_STATIC_FLAG_SIZE            0x00000002  /**< Size flag */

#define orxFONTGEN_KU32_STATIC_MASK_READY           0x00000003  /**< Ready mask */

#define orxFONTGEN_KU32_STATIC_MASK_ALL             0xFFFFFFFF  /**< All mask */


/** Misc defines
 */
#define orxFONTGEN_KZ_DEFAULT_NAME                  "orxFont"

#define orxFONTGEN_KU32_BUFFER_SIZE                 8192

#define orxFONTGEN_KU32_CHARACTER_TABLE_SIZE        256

#define orxFONTGEN_KZ_UTF8_BOM                      "\xEF\xBB\xBF"
#define orxFONTGEN_KU32_UTF8_BOM_LENGTH             3


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
  orxFLOAT        fFontScale;
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

/** static data
 */
static orxFONTGEN_STATIC sstFontGen;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

static orxBOOL orxFASTCALL SaveFilter(const orxSTRING _zSectionName, const orxSTRING _zKeyName, orxBOOL _bUseEncryption)
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
    for(u32Size = orxFile_Read(acBuffer, sizeof(orxCHAR), orxFONTGEN_KU32_BUFFER_SIZE, pstFile), u32Offset = 0, u32Counter = 0, bFirst = orxTRUE;
        u32Size > 0;
        u32Size = orxFile_Read(acBuffer + u32Offset, sizeof(orxCHAR), orxFONTGEN_KU32_BUFFER_SIZE - u32Offset, pstFile) + u32Offset, bFirst = orxFALSE)
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
                  orxLOG("[LOAD]    Character '0x%X': couldn't add to table, skipping.", u32CharacterCodePoint);
                }
              }
              else
              {
                // Adds it
                orxHashTable_Add(sstFontGen.pstCharacterTable, u32CharacterCodePoint, (void *)u32CharacterCodePoint);

                // Logs message
                orxLOG("[LOAD]    Character '0x%X': glyph not found in font, skipping.", u32CharacterCodePoint);
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
              orxLOG("[LOAD]    Invalid character code point '0x%X', skipping.", u32CharacterCodePoint);
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
    orxLOG("[LOAD]    '%s': added %ld characters.", _zFileName, u32Counter);

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
        orxLOG("[LOAD]    '%s': SUCCESS.", _azParams[i]);

        // Updates result
        eResult = orxSTATUS_SUCCESS;
      }
      else
      {
        // Logs message
        orxLOG("[LOAD]    '%s': FAILURE, skipping.", _azParams[i]);
      }
    }
  }
  else
  {
    // Logs message
    orxLOG("[INPUT]   No valid file list found, aborting");
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
    orxLOG("[OUTPUT]  Using output font name '%s'.", sstFontGen.zFontName);
  }
  else
  {
    // Uses default one
    sstFontGen.zFontName = orxString_Duplicate(orxFONTGEN_KZ_DEFAULT_NAME);

    // Logs message
    orxLOG("[OUTPUT]  No valid output found, using default '%s'.", orxFONTGEN_KZ_DEFAULT_NAME);
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
      orxLOG("[SIZE]    Character size set to '%g'.", fSize);
    }
    else
    {
      // Logs message
      orxLOG("[SIZE]    Invalid character size found '%s', aborting.", _azParams[1]);
    }
  }
  else
  {
    // Logs message
    orxLOG("[SIZE]    No character size found, aborting.");

    // Updates result
    eResult = orxSTATUS_FAILURE;
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
          orxFLOAT fRatio;

          // Gets its ratio
          fRatio = orxS2F(sstFontGen.pstFontFace->max_advance_width) / orxS2F(sstFontGen.pstFontFace->max_advance_height);

          // Updates character size
          sstFontGen.vCharacterSize.fX = orxMath_Ceil(fRatio * sstFontGen.vCharacterSize.fY);

          // Stores scale
          sstFontGen.fFontScale = sstFontGen.vCharacterSize.fY / orxS2F(sstFontGen.pstFontFace->max_advance_height);

          // Sets pixel's size (with an extra pixel of spacing)
          eResult = FT_Set_Pixel_Sizes(sstFontGen.pstFontFace, (FT_UInt)orxF2U(sstFontGen.vCharacterSize.fX) - 1, (FT_UInt)orxF2U(sstFontGen.vCharacterSize.fY) - 1) ? orxSTATUS_FAILURE : orxSTATUS_SUCCESS;
        }
      }
    }

    // Success?
    if(eResult != orxSTATUS_FAILURE)
    {
      // Updates status
      orxFLAG_SET(sstFontGen.u32Flags, orxFONTGEN_KU32_STATIC_FLAG_FONT, orxFONTGEN_KU32_STATIC_FLAG_NONE);

      // Logs message
      orxLOG("[FONT]    Using font '%s'.", _azParams[1]);
    }
    else
    {
      // Logs message
      orxLOG("[FONT]    Couldn't load font '%s'.", _azParams[1]);
    }
  }
  else
  {
    // Logs message
    orxLOG("[FONT]    No font specified, aborting.");
  }

  // Done!
  return eResult;
}

static void orxFASTCALL Setup()
{
  // Adds module dependencies
  orxModule_AddDependency(orxMODULE_ID_MAIN, orxMODULE_ID_PARAM);
  orxModule_AddDependency(orxMODULE_ID_MAIN, orxMODULE_ID_FILESYSTEM);
  orxModule_AddDependency(orxMODULE_ID_MAIN, orxMODULE_ID_FILE);
}

static orxSTATUS orxFASTCALL Init()
{
  orxPARAM  stParams;
  orxSTATUS eResult;

  // Clears static controller
  orxMemory_Zero(&sstFontGen, sizeof(orxFONTGEN_STATIC));

  // Creates character table
  sstFontGen.pstCharacterTable = orxHashTable_Create(orxFONTGEN_KU32_CHARACTER_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

  // Creates glyph bank
  sstFontGen.pstGlyphBank = orxBank_Create(orxFONTGEN_KU32_CHARACTER_TABLE_SIZE, sizeof(orxFONTGEN_GLYPH), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

  // Asks for command line output file parameter
  stParams.u32Flags   = orxPARAM_KU32_FLAG_STOP_ON_ERROR;
  stParams.zShortName = "o";
  stParams.zLongName  = "output";
  stParams.zShortDesc = "Output name";
  stParams.zLongDesc  = "Name to use for output font: .tga will be added for the texture and .ini will be added to the config file";
  stParams.pfnParser  = ProcessOutputParams;

  // Registers params
  eResult = orxParam_Register(&stParams);

  // Success?
  if(eResult != orxSTATUS_FAILURE)
  {
    // Asks for command line size parameter
    stParams.u32Flags   = orxPARAM_KU32_FLAG_STOP_ON_ERROR;
    stParams.zShortName = "s";
    stParams.zLongName  = "size";
    stParams.zShortDesc = "Size (height) of characters";
    stParams.zLongDesc  = "Height to use for characters defined with this font, as only monospaced font are supported the width will depend directly on it";
    stParams.pfnParser  = ProcessSizeParams;

    // Registers params
    eResult = orxParam_Register(&stParams);

    // Success?
    if(eResult != orxSTATUS_FAILURE)
    {
      // Valid size?
      if(orxFLAG_TEST(sstFontGen.u32Flags, orxFONTGEN_KU32_STATIC_FLAG_SIZE))
      {
        // Asks for command line decrypt parameter
        stParams.u32Flags   = orxPARAM_KU32_FLAG_STOP_ON_ERROR;
        stParams.zShortName = "f";
        stParams.zLongName  = "font";
        stParams.zShortDesc = "Input font file";
        stParams.zLongDesc  = "Truetype font (usually .ttf) used to generate all the required glyphs";
        stParams.pfnParser  = ProcessFontParams;

        // Registers params
        eResult = orxParam_Register(&stParams);

        // Success?
        if(eResult != orxSTATUS_FAILURE)
        {
          // Valid font?
          if(orxFLAG_TEST(sstFontGen.u32Flags, orxFONTGEN_KU32_STATIC_FLAG_FONT))
          {
            // Asks for command line input file parameter
            stParams.u32Flags   = orxPARAM_KU32_FLAG_STOP_ON_ERROR;
            stParams.zShortName = "t";
            stParams.zLongName  = "textlist";
            stParams.zShortDesc = "List of input text files";
            stParams.zLongDesc  = "List of text files containing all the texts that will be displayed using this font";
            stParams.pfnParser  = ProcessInputParams;

            // Registers params
            eResult = orxParam_Register(&stParams);
          }
        }
      }
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
      orxS32    s32Width, s32Height, s32BaseLine;
      orxFLOAT  fWidth, fHeight;
      orxU8    *pu8ImageBuffer;

      // No font name?
      if(!sstFontGen.zFontName)
      {
        // Uses default one
        sstFontGen.zFontName = orxString_Duplicate(orxFONTGEN_KZ_DEFAULT_NAME);

        // Logs message
        orxLOG("[OUTPUT]  No output name specified, defaulting to '%s'.", orxFONTGEN_KZ_DEFAULT_NAME);
      }
      
      // Gets width & height
      fWidth  = orxMath_Floor(orxMath_Sqrt(orxU2F(u32Counter)));
      fHeight = orxMath_Ceil(orxU2F(u32Counter) / fWidth);
      s32Width  = orxF2S(fWidth * sstFontGen.vCharacterSize.fX);
      s32Height = orxF2S(fHeight * sstFontGen.vCharacterSize.fY);

      // Logs messages
      orxLOG("[PROCESS] Calculated character size: %6g x %g.", sstFontGen.vCharacterSize.fX, sstFontGen.vCharacterSize.fY);
      orxLOG("[PROCESS] Calculated texture size:   %6ld x %ld.", s32Width, s32Height);

      // Gets baseline (using scaled ascender)
      s32BaseLine = orxF2S(orxMath_Ceil(sstFontGen.fFontScale * orxS2F(sstFontGen.pstFontFace->ascender)));

      // Allocates image buffer
      pu8ImageBuffer = (orxU8 *)orxMemory_Allocate(s32Width * s32Height * sizeof(orxRGBA), orxMEMORY_TYPE_MAIN);

      // Valid?
      if(pu8ImageBuffer)
      {
        orxU32            u32Size;
        orxS32            s32X, s32Y;
        orxFONTGEN_GLYPH *pstGlyph;
        orxCHAR           acBuffer[orxFONTGEN_KU32_BUFFER_SIZE], *pc;

        // Clears bitmap
        orxMemory_Zero(pu8ImageBuffer, s32Width * s32Height * sizeof(orxRGBA));

        // For all defined glyphs
        for(pstGlyph = (orxFONTGEN_GLYPH *)orxLinkList_GetFirst(&sstFontGen.stGlyphList), pc = acBuffer, u32Size = orxFONTGEN_KU32_BUFFER_SIZE - 1, s32X = 0, s32Y = s32BaseLine;
            pstGlyph;
            pstGlyph = (orxFONTGEN_GLYPH *)orxLinkList_GetNext(&pstGlyph->stNode))
        {
          orxU32 u32Offset;

          // Adds it to character list
          u32Offset = orxString_PrintUTF8Character(pc, u32Size, pstGlyph->u32CodePoint);

          // Success?
          if(u32Offset != orxU32_UNDEFINED)
          {
            FT_Error stError;

            // Loads rendered glyph
            stError = FT_Load_Glyph(sstFontGen.pstFontFace, (FT_UInt)pstGlyph->u32Index, FT_LOAD_RENDER);
            orxASSERT(!stError);

            // Has data?
            if(sstFontGen.pstFontFace->glyph->bitmap.buffer)
            {
              orxS32  s32AdjustedX, s32AdjustedY, i;
              orxU8  *pu8Src, *pu8Dst;

              // Gets adjusted position
              s32AdjustedX  = s32X + (orxS32)sstFontGen.pstFontFace->glyph->bitmap_left;
              s32AdjustedY  = orxMAX(0, s32Y - (orxS32)sstFontGen.pstFontFace->glyph->bitmap_top);

              // For all rows
              for(i = 0, pu8Src = (orxU8 *)sstFontGen.pstFontFace->glyph->bitmap.buffer, pu8Dst = pu8ImageBuffer + sizeof(orxRGBA) * ((s32AdjustedY * s32Width) + s32AdjustedX);
                  i < (orxS32)sstFontGen.pstFontFace->glyph->bitmap.rows;
                  i++, pu8Src += sstFontGen.pstFontFace->glyph->bitmap.pitch - sstFontGen.pstFontFace->glyph->bitmap.width, pu8Dst += sizeof(orxRGBA) * (s32Width - (orxS32)sstFontGen.pstFontFace->glyph->bitmap.width))
              {
                orxS32 j;

                // For all columns
                for(j = 0;
                    j < (orxS32)sstFontGen.pstFontFace->glyph->bitmap.width;
                    j++, pu8Src++, pu8Dst += sizeof(orxRGBA))
                {
                  // Sets texture's pixel
                  pu8Dst[0] = pu8Dst[1] = pu8Dst[2] = 0xFF;
                  pu8Dst[3] = pu8Src[0];
                }
              }
            }

            // Updates position
            s32X += orxF2S(sstFontGen.vCharacterSize.fX);

            // Next line?
            if(s32X >= s32Width)
            {
              s32X  = 0;
              s32Y += orxF2S(sstFontGen.vCharacterSize.fY);
            }

            // Updates character pointer & size
            pc      += u32Offset;
            u32Size -= u32Offset;
          }
          else
          {
            // Logs message
            orxLOG("[PROCESS] Too many characters defined for a single font, stopping.");

            break;
          }
        }

        // Ends character list
        *pc = orxCHAR_NULL;

        // Pushes font section
        orxConfig_PushSection(sstFontGen.zFontName);

        // Stores font info
        orxConfig_SetStringBlock("CharacterList", acBuffer);
        orxConfig_SetVector("CharacterSize", &sstFontGen.vCharacterSize);
        orxString_NPrint(acBuffer, orxFONTGEN_KU32_BUFFER_SIZE - 1, "%s.tga", sstFontGen.zFontName);
        orxConfig_SetString("Texture", acBuffer);

        // Pops config
        orxConfig_PopSection();

        // Saves texture
        SOIL_save_image(acBuffer, SOIL_SAVE_TYPE_TGA, s32Width, s32Height, sizeof(orxRGBA), pu8ImageBuffer);

        // Logs message
        orxLOG("[PROCESS] %ld glyphs generated in '%s'.", u32Counter, acBuffer);

        // Gets config file name
        orxString_NPrint(acBuffer, orxFONTGEN_KU32_BUFFER_SIZE - 1, "%s.ini", sstFontGen.zFontName);

        // Saves it
        if(orxConfig_Save(acBuffer, orxFALSE, SaveFilter) != orxSTATUS_FAILURE)
        {
          // Logs message
          orxLOG("[SAVE]    '%s': SUCCESS.", acBuffer);
        }
        else
        {
          // Logs message
          orxLOG("[SAVE]    '%s': FAILURE.", acBuffer);
        }

        // Frees image buffer
        orxMemory_Free(pu8ImageBuffer);
      }
      else
      {
        // Logs message
        orxLOG("[PROCESS] Couldn't allocate memory for bitmap (%ld x %ld), aborting.", s32Width, s32Height);
      }
    }
    else
    {
      // Logs message
      orxLOG("[PROCESS] No characters to output, aborting.");
    }
  }
  else
  {
    // No size?
    if(!orxFLAG_TEST(sstFontGen.u32Flags, orxFONTGEN_KU32_STATIC_FLAG_SIZE))
    {
      // Logs message
      orxLOG("[SIZE]    No character size found, aborting.");
    }
    // No font?
    else if(!orxFLAG_TEST(sstFontGen.u32Flags, orxFONTGEN_KU32_STATIC_FLAG_FONT))
    {
      // Logs message
      orxLOG("[FONT]    No font specified, aborting.");
    }
  }
}

int main(int argc, char **argv)
{
  // Inits the Debug System
  orxDEBUG_INIT();

  // Registers main module
  orxModule_Register(orxMODULE_ID_MAIN, Setup, Init, Exit);

  // Registers all other modules
  orxModule_RegisterAll();

  // Calls all modules setup
  orxModule_SetupAll();

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

    // Exits from all other modules
    orxModule_ExitAll();
  }

  // Exits from the Debug system
  orxDEBUG_EXIT();

  // Done!
  return EXIT_SUCCESS;
}
