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

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "SOIL.h"


/** Module flags
 */
#define orxFONTGEN_KU32_STATIC_FLAG_NONE            0x00000000  /**< No flags */

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

/** Static structure
 */
typedef struct __orxFONTGEN_STATIC_t
{
  orxSTRING       zFontName;
  orxVECTOR       vCharacterSize;
  orxHASHTABLE   *pstCharacterTable;
  orxHASHTABLE   *pstIgnoreTable;
  orxU8          *pu8FontBuffer;
  orxFLOAT        fFontScale;
  orxU32          u32Flags;
  stbtt_fontinfo  stFontInfo;

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
        u32CharacterCodePoint = orxString_GetFirstCharacterCodePoint(pc, &pcNext);

        // Non EOL?
        if((u32CharacterCodePoint != orxCHAR_CR)
        && (u32CharacterCodePoint != orxCHAR_LF))
        {
          // Valid?
          if(u32CharacterCodePoint != orxU32_UNDEFINED)
          {
            orxS32 s32GlyphIndex;

            // Gets character's glyph index
            s32GlyphIndex = stbtt_FindGlyphIndex(&sstFontGen.stFontInfo, u32CharacterCodePoint);

            // Valid?
            if(s32GlyphIndex)
            {
              // Not already in table?
              if(orxHashTable_Get(sstFontGen.pstCharacterTable, u32CharacterCodePoint) == orxNULL)
              {
                // Adds it
                if(orxHashTable_Add(sstFontGen.pstCharacterTable, u32CharacterCodePoint, (void *)s32GlyphIndex) != orxSTATUS_FAILURE)
                {
                  orxS32    s32Width, s32LSB;
                  orxFLOAT  fWidth;

                  // Updates counter
                  u32Counter++;

                  // Gets character's width
                  stbtt_GetCodepointHMetrics(&sstFontGen.stFontInfo, u32CharacterCodePoint, &s32Width, &s32LSB);

                  // Scales it
                  fWidth = sstFontGen.fFontScale * orxS2F(s32Width);

                  // Widest?
                  if(fWidth > sstFontGen.vCharacterSize.fX)
                  {
                    // Stores it
                    sstFontGen.vCharacterSize.fX = fWidth;
                  }
                }
                else
                {
                  // Logs message
                  orxLOG("[PROCESS] Character '0x%X': couldn't add to table, skipping.", u32CharacterCodePoint);
                }
              }
            }
            else
            {
              // Not already encountered?
              if(orxHashTable_Get(sstFontGen.pstIgnoreTable, u32CharacterCodePoint) == orxNULL)
              {
                // Adds it
                orxHashTable_Add(sstFontGen.pstIgnoreTable, u32CharacterCodePoint, (void *)u32CharacterCodePoint);

                // Logs message
                orxLOG("[PROCESS] Character '0x%X': glyph not found in font, skipping.", u32CharacterCodePoint);
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
              orxLOG("[PROCESS] Invalid character code point '0x%X', skipping.", u32CharacterCodePoint);
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

    // Maximizes character's width
    sstFontGen.vCharacterSize.fX = orxMath_Ceil(sstFontGen.vCharacterSize.fX);

    // Logs message
    orxLOG("[PROCESS] '%s': added %ld characters.", _zFileName, u32Counter);

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
        orxLOG("[LOAD] %s: SUCCESS.", _azParams[i]);

        // Updates result
        eResult = orxSTATUS_SUCCESS;
      }
      else
      {
        // Logs message
        orxLOG("[LOAD] %s: FAILURE, skipping.", _azParams[i]);
      }
    }
  }
  else
  {
    // Logs message
    orxLOG("[INPUT] No valid file list found, aborting");
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
    orxLOG("[OUTPUT] Using output font name '%s'.", sstFontGen.zFontName);
  }
  else
  {
    // Logs message
    orxLOG("[OUTPUT] No valid output found, using default '%s'.", orxFONTGEN_KZ_DEFAULT_NAME);
  }

  // Done!
  return eResult;
}

static orxSTATUS orxFASTCALL ProcessHeightParams(orxU32 _u32ParamCount, const orxSTRING _azParams[])
{
  orxSTATUS eResult;

  // Has a valid height parameter?
  if(_u32ParamCount > 1)
  {
    orxFLOAT fHeight;

    // Gets it
    if((eResult = orxString_ToFloat(_azParams[1], &fHeight, orxNULL)) != orxSTATUS_FAILURE)
    {
      // Stores it
      sstFontGen.vCharacterSize.fY = fHeight;

      // Logs message
      orxLOG("[HEIGHT] Character height set to '%g'.", fHeight);
    }
    else
    {
      // Logs message
      orxLOG("[HEIGHT] Invalid height found '%s', aborting.", _azParams[1]);
    }
  }
  else
  {
    // Logs message
    orxLOG("[HEIGHT] No height found, aborting.");

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
    orxFILESYSTEM_INFO stFileInfo;

    // Gets its info
    orxFileSystem_Info(_azParams[1], &stFileInfo);

    // Valid?
    if(stFileInfo.u32Size)
    {
      // Allocates buffer
      sstFontGen.pu8FontBuffer = orxMemory_Allocate(stFileInfo.u32Size, orxMEMORY_TYPE_MAIN);

      // Success?
      if(sstFontGen.pu8FontBuffer)
      {
        orxFILE *pstFile;

        // Opens it
        pstFile = orxFile_Open(_azParams[1], orxFILE_KU32_FLAG_OPEN_READ | orxFILE_KU32_FLAG_OPEN_BINARY);

        // Success?
        if(pstFile)
        {
          // Reads its content
          if(orxFile_Read(sstFontGen.pu8FontBuffer, 1, stFileInfo.u32Size, pstFile))
          {
            eResult = stbtt_InitFont(&sstFontGen.stFontInfo, sstFontGen.pu8FontBuffer, 0) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
          }

          // Closes file
          orxFile_Close(pstFile);
        }
      }
    }

    // Success?
    if(eResult != orxSTATUS_FAILURE)
    {
      // Stores font scale
      sstFontGen.fFontScale = stbtt_ScaleForPixelHeight(&sstFontGen.stFontInfo, sstFontGen.vCharacterSize.fY);

      // Logs message
      orxLOG("[FONT] Using font '%s'.", _azParams[1]);
    }
    else
    {
      // Frees buffer
      if(sstFontGen.pu8FontBuffer)
      {
        orxMemory_Free(sstFontGen.pu8FontBuffer);
        sstFontGen.pu8FontBuffer = orxNULL;
      }

      // Logs message
      orxLOG("[FONT] Couldn't load font '%s'.", _azParams[1]);
    }
  }
  else
  {
    // Logs message
    orxLOG("[FONT] No font specified, aborting.");
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

  // Creates character & ignore tables
  sstFontGen.pstCharacterTable  = orxHashTable_Create(orxFONTGEN_KU32_CHARACTER_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
  sstFontGen.pstIgnoreTable     = orxHashTable_Create(orxFONTGEN_KU32_CHARACTER_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

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
    stParams.pfnParser  = ProcessHeightParams;

    // Registers params
    eResult = orxParam_Register(&stParams);

    // Success?
    if(eResult != orxSTATUS_FAILURE)
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

  // Has font buffer?
  if(sstFontGen.pu8FontBuffer)
  {
    // Frees it
    orxMemory_Free(sstFontGen.pu8FontBuffer);
    sstFontGen.pu8FontBuffer = orxNULL;
  }

  // Deletes character & ignore tables
  orxHashTable_Delete(sstFontGen.pstCharacterTable);
  orxHashTable_Delete(sstFontGen.pstIgnoreTable);
}

static void Run()
{
  orxU32 u32Counter;

  // Gets character table's counter
  u32Counter = orxHashTable_GetCounter(sstFontGen.pstCharacterTable);

  // Valid?
  if(u32Counter)
  {
    orxS32    s32Width, s32Height, s32Ascent, s32Descent, s32LineGap;
    orxFLOAT  fWidth, fHeight;
    orxU8    *pu8ImageBuffer;

    // Gets width & height
    fWidth  = orxMath_Floor(orxMath_Sqrt(orxU2F(u32Counter)));
    fHeight = orxMath_Ceil(orxU2F(u32Counter) / fWidth);
    s32Width  = orxF2S(fWidth * sstFontGen.vCharacterSize.fX);
    s32Height = orxF2S(fHeight * sstFontGen.vCharacterSize.fY);

    // Gets font's metrics
    stbtt_GetFontVMetrics(&sstFontGen.stFontInfo, &s32Ascent, &s32Descent, &s32LineGap);
    s32Ascent = orxF2S(sstFontGen.fFontScale * orxS2F(s32Ascent));

    // Allocates image
    pu8ImageBuffer = (orxU8 *)orxMemory_Allocate(s32Width * s32Height * sizeof(orxU8), orxMEMORY_TYPE_MAIN);
    orxMemory_Zero(pu8ImageBuffer, s32Width * s32Height * sizeof(orxU8));

    // Valid?
    if(pu8ImageBuffer)
    {
      orxHANDLE hHandle;
      orxU32    u32CharacterCodePoint, u32Size;
      orxS32    s32GlyphIndex, s32X, s32Y;
      orxCHAR   acBuffer[orxFONTGEN_KU32_BUFFER_SIZE], *pc;

      // For all defined characters
      for(hHandle = orxHashTable_FindFirst(sstFontGen.pstCharacterTable, &u32CharacterCodePoint, (void *)&s32GlyphIndex), pc = acBuffer, u32Size = orxFONTGEN_KU32_BUFFER_SIZE - 1, s32X = 0, s32Y = s32Ascent;
          hHandle != orxHANDLE_UNDEFINED;
          hHandle = orxHashTable_FindNext(sstFontGen.pstCharacterTable, hHandle, &u32CharacterCodePoint, (void *)&s32GlyphIndex))
      {
        orxU32 u32Offset;

        // Adds it to list
        u32Offset = orxString_PrintUTF8Character(pc, u32Size, u32CharacterCodePoint);

        // Success?
        if(u32Offset != orxU32_UNDEFINED)
        {
          orxS32 s32Advance, s32LSB, s32XStart, s32XEnd, s32YStart, s32YEnd, s32AdjustedX, s32AdjustedY;

          // Gets glyph's metrics
          stbtt_GetGlyphHMetrics(&sstFontGen.stFontInfo, s32GlyphIndex, &s32Advance, &s32LSB);

          // Gets glyph's bitmap box
          stbtt_GetGlyphBitmapBox(&sstFontGen.stFontInfo, s32GlyphIndex, sstFontGen.fFontScale, sstFontGen.fFontScale, &s32XStart, &s32YStart, &s32XEnd, &s32YEnd);

          // Gets adjusted position
          s32AdjustedX  = s32X + orxF2S(orx2F(0.5f) * (sstFontGen.vCharacterSize.fX - orxS2F(s32XEnd - s32XStart)));
          s32AdjustedY  = s32Y + s32YStart;

          // Prints glyph on bitmap
          stbtt_MakeGlyphBitmap(&sstFontGen.stFontInfo, pu8ImageBuffer + (s32AdjustedY * s32Width) + s32AdjustedX, s32XEnd - s32XStart, s32YEnd - s32YStart, s32Width, sstFontGen.fFontScale, sstFontGen.fFontScale, s32GlyphIndex);

          // Updates position
          s32X += orxF2S(sstFontGen.vCharacterSize.fX);

          // Next line?
          if(s32X >= s32Width)
          {
            s32X  = 0;
            s32Y += orxF2S(sstFontGen.vCharacterSize.fY);
          }

          // Updates character list & size
          pc      += u32Offset;
          u32Size -= u32Offset;
        }
        else
        {
          // Logs message
          orxLOG("[PROCESS] Too many characters defined for a single font, aborting.");

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
      SOIL_save_image(acBuffer, SOIL_SAVE_TYPE_TGA, s32Width, s32Height, 1, pu8ImageBuffer);

        // Gets config file name
      orxString_NPrint(acBuffer, orxFONTGEN_KU32_BUFFER_SIZE - 1, "%s.ini", sstFontGen.zFontName);

      // Saves it
      if(orxConfig_Save(acBuffer, orxFALSE, SaveFilter) != orxSTATUS_FAILURE)
      {
        // Logs message
        orxLOG("[SAVE] '%s': SUCCESS.", acBuffer);
      }
      else
      {
        // Logs message
        orxLOG("[SAVE] '%s': FAILURE.", acBuffer);
      }

      // Frees image buffer
      orxMemory_Free(pu8ImageBuffer);
    }
    else
    {
      // Logs message
      orxLOG("[PROCESS] Couldn't allocate texture memory (%ld x %ld), aborting.", s32Width, s32Height);
    }
  }
  else
  {
    // Logs message
    orxLOG("[PROCESS] No characters to output, aborting.");
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
