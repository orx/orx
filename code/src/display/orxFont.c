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
 * @file orxFont.c
 * @date 08/03/2010
 * @author iarwain@orx-project.org
 *
 */


#include "display/orxFont.h"

#include "memory/orxBank.h"
#include "core/orxConfig.h"
#include "core/orxEvent.h"
#include "core/orxResource.h"
#include "display/orxDisplay.h"
#include "object/orxStructure.h"
#include "utils/orxHashTable.h"


/** Module flags
 */
#define orxFONT_KU32_STATIC_FLAG_NONE           0x00000000  /**< No flags */

#define orxFONT_KU32_STATIC_FLAG_READY          0x00000001  /**< Ready flag */

#define orxFONT_KU32_STATIC_MASK_ALL            0xFFFFFFFF  /**< All mask */

/** orxFONT flags / masks
 */
#define orxFONT_KU32_FLAG_NONE                  0x00000000  /**< No flags */

#define orxFONT_KU32_FLAG_INTERNAL              0x10000000  /**< Internal structure handling flag  */
#define orxFONT_KU32_FLAG_REFERENCED            0x20000000  /**< Referenced flag */
#define orxFONT_KU32_FLAG_CACHED                0x40000000  /**< Cached flag */
#define orxFONT_KU32_FLAG_CAN_UPDATE_MAP        0x80000000  /**< Can update map flag */
#define orxFONT_KU32_FLAG_SDF                   0x01000000  /**< SDF flag */

#define orxFONT_KU32_MASK_ALL                   0xFFFFFFFF  /**< All mask */


/** Misc defines
 */
#define orxFONT_KU32_MAP_BANK_SIZE              4           /**< Map bank size */
#define orxFONT_KU32_REFERENCE_TABLE_SIZE       4           /**< Reference table size */
#define orxFONT_KU32_CHARACTER_BANK_SIZE        256         /**< Character bank size */
#define orxFONT_KU32_CHARACTER_TABLE_SIZE       256         /**< Character table size */

#define orxFONT_KU32_BANK_SIZE                  16          /**< Bank size */

#define orxFONT_KV_DEFAULT_CHARACTER_SPACING    orx2F(2.0f), orx2F(2.0f), orxFLOAT_0
#define orxFONT_KV_DEFAULT_CHARACTER_SIZE       orxFLOAT_0, orx2F(32.0f), orxFLOAT_0

#define orxFONT_KZ_CONFIG_TEXTURE_NAME          "Texture"
#define orxFONT_KZ_CONFIG_CHARACTER_LIST        "CharacterList"
#define orxFONT_KZ_CONFIG_CHARACTER_SIZE        "CharacterSize"
#define orxFONT_KZ_CONFIG_CHARACTER_HEIGHT      "CharacterHeight"
#define orxFONT_KZ_CONFIG_CHARACTER_WIDTH_LIST  "CharacterWidthList"
#define orxFONT_KZ_CONFIG_CHARACTER_SPACING     "CharacterSpacing"
#define orxFONT_KZ_CONFIG_CHARACTER_PADDING     "CharacterPadding"
#define orxFONT_KZ_CONFIG_TEXTURE_ORIGIN        "TextureOrigin"
#define orxFONT_KZ_CONFIG_TEXTURE_SIZE          "TextureSize"
#define orxFONT_KZ_CONFIG_TEXTURE_CORNER        "TextureCorner" /**< Kept for retro-compatibility reason */
#define orxFONT_KZ_CONFIG_KEEP_IN_CACHE         "KeepInCache"
#define orxFONT_KZ_CONFIG_TYPEFACE              "Typeface"
#define orxFONT_KZ_CONFIG_SDF                   "SDF"
#define orxFONT_KZ_CONFIG_SHADER                "Shader"

#define orxFONT_KZ_ASCII                        "ascii"
#define orxFONT_KZ_ANSI                         "ansi"
#define orxFONT_KZ_TEXTURE_FONT_PREFIX          "orx:texture:font"
#define orxFONT_KZ_CONFIG_SDF_NAME              "orx:config:font:sdf"

#define orxFONT_KC_INHERITANCE_MARKER           '@'


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Font structure
 */
struct __orxFONT_t
{
  orxSTRUCTURE      stStructure;                /**< Public structure, first structure member : 32 */
  orxCHARACTER_MAP *pstMap;                     /**< Font's map : 20 */
  orxFLOAT         *afCharacterWidthList;       /**< Character width list : 24 */
  orxFLOAT          fCharacterHeight;           /**< Character height : 28 */
  orxVECTOR         vCharacterSpacing;          /**< Character spacing : 40 */
  orxTEXTURE       *pstTexture;                 /**< Texture : 44 */
  orxFLOAT          fTop;                       /**< Top coordinate : 48 */
  orxFLOAT          fLeft;                      /**< Left coordinate : 52 */
  orxFLOAT          fWidth;                     /**< Width : 56 */
  orxFLOAT          fHeight;                    /**< Height : 60 */
  const orxSTRING   zCharacterList;             /**< Character list : 64 */
  const orxSTRING   zReference;                 /**< Config reference : 68 */
  const orxSTRING   zTypeface;                  /**< Typeface : 72 */
  orxSHADER        *pstShader;                  /**< Shader : 76 */
};

/** Static structure
 */
typedef struct __orxFONT_STATIC_t
{
  orxBANK          *pstMapBank;                 /**< Map bank : 4 */
  orxHASHTABLE     *pstReferenceTable;          /**< Reference table : 8 */
  orxFONT          *pstDefaultFont;             /**< Default font : 12 */
  orxU32            u32Flags;                   /**< Control flags : 16 */
  const orxSTRING   zANSICharacterList;         /**< ANSI character list : 20 */
  const orxSTRING   zASCIICharacterList;        /**< ASCII character list : 24 */

} orxFONT_STATIC;


/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/

static orxFONT_STATIC sstFont;

#include "../src/display/orxDefaultFont.inc"


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Updates font's map
 * @param[in]   _pstFont       Concerned font
 */
static void orxFASTCALL orxFont_UpdateMap(orxFONT *_pstFont)
{
  /* Check */
  orxSTRUCTURE_ASSERT(_pstFont);

  /* Can update? */
  if(orxStructure_TestFlags(_pstFont, orxFONT_KU32_FLAG_CAN_UPDATE_MAP))
  {
    /* Clears UTF-8 table */
    orxHashTable_Clear(_pstFont->pstMap->pstCharacterTable);

    /* Clears UTF-8 bank */
    orxBank_Clear(_pstFont->pstMap->pstCharacterBank);

    /* Has texture, texture size, character size and character list? */
    if((_pstFont->pstTexture != orxNULL)
    && (_pstFont->fWidth > orxFLOAT_0)
    && (_pstFont->fHeight > orxFLOAT_0)
    && (_pstFont->fCharacterHeight > orxFLOAT_0)
    && (_pstFont->afCharacterWidthList != orxNULL)
    && (_pstFont->zCharacterList != orxSTRING_EMPTY))
    {
      const orxCHAR  *pc;
      orxU32          u32CharacterCodePoint;
      orxS32          s32Index;
      orxVECTOR       vOrigin;

      /* For all defined characters */
      for(s32Index = 0, u32CharacterCodePoint = orxString_GetFirstCharacterCodePoint(_pstFont->zCharacterList, &pc), orxVector_Set(&vOrigin, _pstFont->fLeft, _pstFont->fTop, orxFLOAT_0);
          (u32CharacterCodePoint != orxCHAR_NULL) && (vOrigin.fY < _pstFont->fTop + _pstFont->fHeight);
          s32Index++, u32CharacterCodePoint = orxString_GetFirstCharacterCodePoint(pc, &pc))
      {
        orxCHARACTER_GLYPH  **ppstBucket;
        orxCHARACTER_GLYPH   *pstGlyph;

        /* Gets glyph's bucket */
        ppstBucket = (orxCHARACTER_GLYPH **)orxHashTable_Retrieve(_pstFont->pstMap->pstCharacterTable, u32CharacterCodePoint);

        /* Checks */
        orxASSERT(ppstBucket != orxNULL);

        /* Not already defined? */
        if(*ppstBucket == orxNULL)
        {
          /* Allocates it */
          pstGlyph = (orxCHARACTER_GLYPH *)orxBank_Allocate(_pstFont->pstMap->pstCharacterBank);

          /* Checks */
          orxASSERT(pstGlyph != orxNULL);

          /* Adds it to table */
          *ppstBucket = pstGlyph;
        }
        else
        {
          /* Gets it */
          pstGlyph = *ppstBucket;
        }

        /* Stores its width */
        pstGlyph->fWidth = _pstFont->afCharacterWidthList[s32Index];

        /* Out of bound? */
        if(vOrigin.fX + pstGlyph->fWidth > _pstFont->fLeft + _pstFont->fWidth)
        {
          /* Reinits its X value */
          vOrigin.fX = _pstFont->fLeft;

          /* Updates its Y value */
          vOrigin.fY += _pstFont->fCharacterHeight + _pstFont->vCharacterSpacing.fY;
        }

        /* Stores its origin */
        pstGlyph->fX = vOrigin.fX;
        pstGlyph->fY = vOrigin.fY;

        /* Updates current origin X value */
        vOrigin.fX += pstGlyph->fWidth + _pstFont->vCharacterSpacing.fX;
      }

      /* Optimizes character table */
      orxHashTable_Optimize(_pstFont->pstMap->pstCharacterTable);

      /* Stores character height */
      _pstFont->pstMap->fCharacterHeight = _pstFont->fCharacterHeight;

      /* Had more defined characters? */
      if(u32CharacterCodePoint != orxCHAR_NULL)
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Too many characters defined for font <%s>: couldn't map characters [%s].", _pstFont->zReference, pc);
      }
    }
  }

  /* Done! */
  return;
}

/* Creates default font
 */
static orxINLINE void orxFont_CreateDefaultFont()
{
  /* Sets its texture as memory resource */
  if(orxResource_SetMemoryResource(orxTEXTURE_KZ_RESOURCE_GROUP, orxNULL, orxFONT_KZ_DEFAULT_TEXTURE_NAME, sstDefaultFont.s64Size, sstDefaultFont.pu8Data) != orxSTATUS_FAILURE)
  {
    orxTEXTURE *pstTexture;

    /* Loads it */
    pstTexture = orxTexture_Load(orxFONT_KZ_DEFAULT_TEXTURE_NAME, orxFALSE);

    /* Success? */
    if(pstTexture != orxNULL)
    {
      /* Creates default font */
      sstFont.pstDefaultFont = orxFont_Create();

      /* Success? */
      if(sstFont.pstDefaultFont != orxNULL)
      {
        /* Sets it as its own owner */
        orxStructure_SetOwner(sstFont.pstDefaultFont, sstFont.pstDefaultFont);

        /* Sets its texture */
        if(orxFont_SetTexture(sstFont.pstDefaultFont, pstTexture) != orxSTATUS_FAILURE)
        {
          orxVECTOR vSpacing;
          orxFLOAT *afCharacterWidthList;
          orxU32    u32CharacterCount, i;
          orxBOOL   bDebugLevelBackup;

          /* Gets character count */
          u32CharacterCount = orxString_GetCharacterCount(sstDefaultFont.zCharacterList);

          /* Allocates array for character widths */
          afCharacterWidthList = (orxFLOAT *)orxMemory_Allocate(u32CharacterCount * sizeof(orxFLOAT), orxMEMORY_TYPE_MAIN);

          /* For all characters */
          for(i = 0; i < u32CharacterCount; i++)
          {
            /* Stores its width */
            afCharacterWidthList[i] = sstDefaultFont.fCharacterWidth;
          }

          /* Disables display logs */
          bDebugLevelBackup = orxDEBUG_IS_LEVEL_ENABLED(orxDEBUG_LEVEL_DISPLAY);
          orxDEBUG_ENABLE_LEVEL(orxDEBUG_LEVEL_DISPLAY, orxFALSE);

          /* Inits font */
          orxFont_SetCharacterList(sstFont.pstDefaultFont, sstDefaultFont.zCharacterList);
          orxFont_SetCharacterHeight(sstFont.pstDefaultFont, sstDefaultFont.fCharacterHeight);
          orxFont_SetCharacterWidthList(sstFont.pstDefaultFont, u32CharacterCount, afCharacterWidthList);
          orxFont_SetCharacterSpacing(sstFont.pstDefaultFont, orxVector_Set(&vSpacing, sstDefaultFont.fCharacterSpacingX, sstDefaultFont.fCharacterSpacingY, orxFLOAT_0));

          /* Reenables display logs */
          orxDEBUG_ENABLE_LEVEL(orxDEBUG_LEVEL_DISPLAY, bDebugLevelBackup);

          /* Stores its reference key */
          sstFont.pstDefaultFont->zReference = orxFONT_KZ_DEFAULT_FONT_NAME;

          /* Adds it to reference table */
          orxHashTable_Add(sstFont.pstReferenceTable, orxString_Hash(sstFont.pstDefaultFont->zReference), sstFont.pstDefaultFont);

          /* Updates its flags */
          orxStructure_SetFlags(sstFont.pstDefaultFont, orxFONT_KU32_FLAG_REFERENCED, orxFONT_KU32_FLAG_NONE);

          /* Frees character widths array */
          orxMemory_Free(afCharacterWidthList);
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't set default font's texture.");

          /* Deletes font */
          orxFont_Delete(sstFont.pstDefaultFont);
          sstFont.pstDefaultFont = orxNULL;

          /* Deletes texture */
          orxTexture_Delete(pstTexture);
        }
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't create default font.");

        /* Deletes texture */
        orxTexture_Delete(pstTexture);
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't load default font's texture.");
    }
  }

  /* Sets SDF shader config as memory resource */
  if(orxResource_SetMemoryResource(orxCONFIG_KZ_RESOURCE_GROUP, orxNULL, orxFONT_KZ_CONFIG_SDF, orxString_GetLength(szDefaultFontSDFShader), szDefaultFontSDFShader) != orxSTATUS_FAILURE)
  {
    /* Loads it */
    orxConfig_Load(orxFONT_KZ_CONFIG_SDF);
  }

  /* Done! */
  return;
}

static orxBOOL orxFASTCALL orxFont_InheritProperty(const orxSTRING _zKeyName, const orxSTRING _zSectionName, void *_pContext)
{
  const orxSTRING *azSectionList;

  /* Gets sections */
  azSectionList = (const orxSTRING *)_pContext;

  /* Pushes source section (without inheritance marker) */
  orxConfig_PushSection(azSectionList[0] + 1);

  /* Should inherit? */
  if(orxConfig_HasValue(_zKeyName) != orxFALSE)
  {
    /* Selects destination section */
    orxConfig_SelectSection(azSectionList[1]);

    /* Inherits its value */
    orxConfig_SetString(_zKeyName, azSectionList[0]);
  }

  /* Pops it */
  orxConfig_PopSection();

  /* Done! */
  return orxTRUE;
}

static orxSTATUS orxFASTCALL orxFont_ProcessConfigData(orxFONT *_pstFont)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Has reference? */
  if((_pstFont->zReference != orxNULL)
  && (*(_pstFont->zReference) != orxCHAR_NULL)
  && (orxConfig_HasSection(_pstFont->zReference) != orxFALSE))
  {
    const orxSTRING zName;

    /* Pushes its config section */
    orxConfig_PushSection(_pstFont->zReference);

    /* Gets typeface name */
    zName = orxConfig_GetString(orxFONT_KZ_CONFIG_TYPEFACE);

    /* Valid? */
    if((zName != orxNULL) && (zName != orxSTRING_EMPTY))
    {
      orxVECTOR       vCharacterSize, vCharacterSpacing, vCharacterPadding;
      const orxSTRING zCharacterList;
      orxBITMAP      *pstBitmap;
      orxFLOAT       *afCharacterWidthList;
      orxU32          u32CharacterCount;
      orxBOOL         bSDF;

      /* Retrieves character spacing */
      if(orxConfig_GetVector(orxFONT_KZ_CONFIG_CHARACTER_SPACING, &vCharacterSpacing) == orxNULL)
      {
        /* Uses default spacing */
        orxVector_Set(&vCharacterSpacing, orxFONT_KV_DEFAULT_CHARACTER_SPACING);
      }

      /* Gets character padding */
      if(orxConfig_GetVector(orxFONT_KZ_CONFIG_CHARACTER_PADDING, &vCharacterPadding) == orxNULL)
      {
        orxVector_SetAll(&vCharacterPadding, orxConfig_GetFloat(orxFONT_KZ_CONFIG_CHARACTER_PADDING));
      }

      /* Has character size? */
      if(orxConfig_HasValue(orxFONT_KZ_CONFIG_CHARACTER_SIZE) != orxFALSE)
      {
        /* Isn't a vector? */
        if(orxConfig_GetVector(orxFONT_KZ_CONFIG_CHARACTER_SIZE, &vCharacterSize) == orxNULL)
        {
          /* Gets float value */
          orxVector_Set(&vCharacterSize, orxFLOAT_0, orxConfig_GetFloat(orxFONT_KZ_CONFIG_CHARACTER_SIZE), orxFLOAT_0);
        }
      }
      else
      {
        /* Uses default size */
        orxVector_Set(&vCharacterSize, orxFONT_KV_DEFAULT_CHARACTER_SIZE);
      }

      /* Gets character list */
      zCharacterList = orxConfig_GetString(orxFONT_KZ_CONFIG_CHARACTER_LIST);

      /* Default/ASCII? */
      if((*zCharacterList == orxCHAR_NULL) || (orxString_ICompare(zCharacterList, orxFONT_KZ_ASCII) == 0))
      {
        /* Updates it */
        zCharacterList = sstFont.zASCIICharacterList;
      }
      /* ANSI? */
      else if(orxString_ICompare(zCharacterList, orxFONT_KZ_ANSI) == 0)
      {
        /* Updates it */
        zCharacterList = sstFont.zANSICharacterList;
      }

      /* Gets SDF status */
      bSDF = orxConfig_GetBool(orxFONT_KZ_CONFIG_SDF);

      /* Gets character count */
      u32CharacterCount = orxString_GetCharacterCount(zCharacterList);

      /* Allocates character width list */
      afCharacterWidthList = (orxFLOAT *)alloca(u32CharacterCount * sizeof(orxFLOAT));
      orxASSERT(afCharacterWidthList != orxNULL);

      /* Loads font bitmap */
      pstBitmap = orxDisplay_LoadFont(zName, zCharacterList, &vCharacterSize, &vCharacterSpacing, &vCharacterPadding, bSDF, afCharacterWidthList);

      /* Success? */
      if(pstBitmap != orxNULL)
      {
        orxTEXTURE *pstTexture;

        /* Gets texture */
        pstTexture = (_pstFont->pstTexture != orxNULL) ? _pstFont->pstTexture : orxTexture_Create();

        /* Valid? */
        if(pstTexture != orxNULL)
        {
          orxCHAR acBuffer[256];

          /* Creates texture name */
          orxString_NPrint(acBuffer, sizeof(acBuffer), "%s:%s", orxFONT_KZ_TEXTURE_FONT_PREFIX, _pstFont->zReference);

          /* Links them */
          if(orxTexture_LinkBitmap(pstTexture, pstBitmap, acBuffer, orxTRUE) != orxSTATUS_FAILURE)
          {
            /* Deactivates map update */
            orxStructure_SetFlags(_pstFont, orxFONT_KU32_FLAG_NONE, orxFONT_KU32_FLAG_CAN_UPDATE_MAP);

            /* Already set or sets it */
            if(((_pstFont->pstTexture == pstTexture) && (_pstFont->fTop = _pstFont->fLeft, orxTexture_GetSize(pstTexture, &(_pstFont->fWidth), &(_pstFont->fHeight)) != orxSTATUS_FAILURE))
            || (orxFont_SetTexture(_pstFont, pstTexture) != orxSTATUS_FAILURE))
            {
              const orxSTRING zShader;
              orxCHAR         acShaderBuffer[256];

              /* Sets its owner */
              orxStructure_SetOwner(pstTexture, _pstFont);

              /* Sets character spacing */
              orxFont_SetCharacterSpacing(_pstFont, &vCharacterSpacing);

              /* Sets character list */
              orxFont_SetCharacterList(_pstFont, zCharacterList);

              /* Sets character height & width list */
              orxFont_SetCharacterHeight(_pstFont, vCharacterSize.fY);
              orxFont_SetCharacterWidthList(_pstFont, u32CharacterCount, afCharacterWidthList);

              /* Updates flags */
              orxStructure_SetFlags(_pstFont, orxFONT_KU32_FLAG_INTERNAL | orxFONT_KU32_FLAG_REFERENCED | orxFONT_KU32_FLAG_CAN_UPDATE_MAP, orxFONT_KU32_MASK_ALL);

              /* Stores its typeface */
              _pstFont->zTypeface = orxString_Store(zName);

              /* Gets shader */
              zShader = orxConfig_GetString(orxFONT_KZ_CONFIG_SHADER);

              /* SDF? */
              if(bSDF != orxFALSE)
              {
                /* Updates flags */
                orxStructure_SetFlags(_pstFont, orxFONT_KU32_FLAG_SDF, orxFONT_KU32_FLAG_NONE);

                /* No custom shader? */
                if(*zShader == orxCHAR_NULL)
                {
                  orxVECTOR       vSize;
                  orxCHAR         acBuffer[256];
                  const orxSTRING azSectionList[2] = {acBuffer, acShaderBuffer};

                  /* Gets shader config section */
                  orxString_NPrint(acShaderBuffer, sizeof(acShaderBuffer), "%s:%s", orxFONT_KZ_SDF_SHADER_NAME, _pstFont->zReference);

                  /* Uses SDF shader as its parent */
                  orxConfig_SetParent(acShaderBuffer, orxFONT_KZ_SDF_SHADER_NAME);

                  /* Copies font's origin (for hot-reloading purposes) */
                  orxConfig_SetOrigin(acShaderBuffer, orxConfig_GetOrigin(_pstFont->zReference));

                  /* Stores it */
                  zShader = acShaderBuffer;

                  /* Gets current section with inheritance marker  */
                  orxString_NPrint(acBuffer, sizeof(acBuffer), "%c%s", orxFONT_KC_INHERITANCE_MARKER, _pstFont->zReference);

                  /* For all SDF shader properties, inherits the local ones */
                  orxConfig_PushSection(orxFONT_KZ_SDF_SHADER_NAME);
                  orxConfig_ForAllKeys(orxFont_InheritProperty, orxTRUE, azSectionList);
                  orxConfig_PopSection();

                  /* Sets its texture size */
                  orxVector_Set(&vSize, _pstFont->fWidth, _pstFont->fHeight, orxFLOAT_0);
                  orxConfig_PushSection(acShaderBuffer);
                  orxConfig_SetVector(orxFONT_KZ_CONFIG_TEXTURE_SIZE, &vSize);
                  orxConfig_PopSection();
                }
              }

              /* Sets shader */
              orxFont_SetShader(_pstFont, zShader);

              /* Updates its map */
              orxFont_UpdateMap(_pstFont);

              /* Updates result */
              eResult = orxSTATUS_SUCCESS;
            }
            else
            {
              /* Logs message */
              orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't link texture (%s) to font (%s).", zName, _pstFont->zReference);

              /* Deletes texture */
              orxTexture_Delete(pstTexture);
            }
          }
        }
        else
        {
          /* Deletes texture */
          orxTexture_Delete(pstTexture);

          /* Deletes bitmap */
          orxDisplay_DeleteBitmap(pstBitmap);
        }
      }
    }
    else
    {
      /* Gets texture name */
      zName = orxConfig_GetString(orxFONT_KZ_CONFIG_TEXTURE_NAME);

      /* Valid? */
      if((zName != orxNULL) && (zName != orxSTRING_EMPTY))
      {
        orxTEXTURE *pstTexture;

        /* Loads texture */
        pstTexture = orxTexture_Load(zName, orxFALSE);

        /* Valid? */
        if(pstTexture != orxNULL)
        {
          /* Deactivates map update */
          orxStructure_SetFlags(_pstFont, orxFONT_KU32_FLAG_NONE, orxFONT_KU32_FLAG_CAN_UPDATE_MAP);

          /* Links it */
          if(orxFont_SetTexture(_pstFont, pstTexture) != orxSTATUS_FAILURE)
          {
            orxVECTOR       vCharacterSize, vCharacterSpacing;
            const orxSTRING zCharacterList;

            /* Sets its owner */
            orxStructure_SetOwner(pstTexture, _pstFont);

            /* Updates flags */
            orxStructure_SetFlags(_pstFont, orxFONT_KU32_FLAG_INTERNAL, orxFONT_KU32_MASK_ALL);

            /* Gets character list */
            zCharacterList = orxConfig_GetString(orxFONT_KZ_CONFIG_CHARACTER_LIST);

            /* Sets it */
            if(orxFont_SetCharacterList(_pstFont, zCharacterList) != orxSTATUS_FAILURE)
            {
              orxVECTOR vValue;
              orxFLOAT *afCharacterWidthList = orxNULL, fCharacterHeight;
              orxU32    u32CharacterCount;

              /* Updates result */
              eResult = orxSTATUS_SUCCESS;

              /* Gets character count */
              u32CharacterCount = orxString_GetCharacterCount(zCharacterList);

              /* Has origin / corner? */
              if((orxConfig_GetVector(orxFONT_KZ_CONFIG_TEXTURE_ORIGIN, &vValue) != orxNULL)
              || (orxConfig_GetVector(orxFONT_KZ_CONFIG_TEXTURE_CORNER, &vValue) != orxNULL))
              {
                /* Applies it */
                orxFont_SetOrigin(_pstFont, &vValue);

                /* Updates size */
                vValue.fX = orxMAX(orxFLOAT_0, _pstFont->fWidth - vValue.fX);
                vValue.fY = orxMAX(orxFLOAT_0, _pstFont->fHeight - vValue.fY);
                orxFont_SetSize(_pstFont, &vValue);
              }

              /* Has size? */
              if(orxConfig_GetVector(orxFONT_KZ_CONFIG_TEXTURE_SIZE, &vValue) != orxNULL)
              {
                /* Applies it */
                orxFont_SetSize(_pstFont, &vValue);
              }

              /* Gets character spacing */
              if(orxConfig_GetVector(orxFONT_KZ_CONFIG_CHARACTER_SPACING, &vCharacterSpacing) != orxNULL)
              {
                /* Sets it */
                orxFont_SetCharacterSpacing(_pstFont, &vCharacterSpacing);
              }

              /* Gets character size */
              if(orxConfig_GetVector(orxFONT_KZ_CONFIG_CHARACTER_SIZE, &vCharacterSize) != orxNULL)
              {
                orxU32 i;

                /* Allocates character width list */
                afCharacterWidthList = (orxFLOAT *)orxMemory_Allocate(u32CharacterCount * sizeof(orxFLOAT), orxMEMORY_TYPE_MAIN);
                orxASSERT(afCharacterWidthList != orxNULL);

                /* For all characters */
                for(i = 0; i < u32CharacterCount; i++)
                {
                  /* Stores its width */
                  afCharacterWidthList[i] = vCharacterSize.fX;
                }

                /* Stores character height */
                fCharacterHeight = vCharacterSize.fY;
              }
              else
              {
                /* Has valid character height and character width list */
                if(((fCharacterHeight = orxConfig_GetFloat(orxFONT_KZ_CONFIG_CHARACTER_HEIGHT)) > orxFLOAT_0)
                && (orxConfig_GetListCount(orxFONT_KZ_CONFIG_CHARACTER_WIDTH_LIST) == (orxS32)u32CharacterCount))
                {
                  orxU32 i;

                  /* Allocates character width list */
                  afCharacterWidthList = (orxFLOAT *)orxMemory_Allocate(u32CharacterCount * sizeof(orxFLOAT), orxMEMORY_TYPE_MAIN);
                  orxASSERT(afCharacterWidthList != orxNULL);

                  /* For all characters */
                  for(i = 0; i < u32CharacterCount; i++)
                  {
                    /* Stores its width */
                    afCharacterWidthList[i] = orxConfig_GetListFloat(orxFONT_KZ_CONFIG_CHARACTER_WIDTH_LIST, i);
                  }
                }
                else
                {
                  /* Logs message */
                  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't find character size / height & width list properties for font (%s).", _pstFont->zReference);

                  /* Updates result */
                  eResult = orxSTATUS_FAILURE;
                }
              }

              /* Valid? */
              if(eResult != orxSTATUS_FAILURE)
              {
                /* Sets character height & width list */
                if((orxFont_SetCharacterHeight(_pstFont, fCharacterHeight) != orxSTATUS_FAILURE)
                && (orxFont_SetCharacterWidthList(_pstFont, u32CharacterCount, afCharacterWidthList) != orxSTATUS_FAILURE))
                {
                  /* Updates status flags */
                  orxStructure_SetFlags(_pstFont, orxFONT_KU32_FLAG_REFERENCED, orxFONT_KU32_FLAG_NONE);
                }
                else
                {
                  /* Logs message */
                  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Invalid character size (%f, %f) for font (%s).", vCharacterSize.fX, vCharacterSize.fY, _pstFont->zReference);

                  /* Unlinks texture */
                  orxFont_SetTexture(_pstFont, orxNULL);

                  /* Updates result */
                  eResult = orxSTATUS_FAILURE;
                }
              }
              else
              {
                /* Unlinks texture */
                orxFont_SetTexture(_pstFont, orxNULL);
              }

              /* Has character width list? */
              if(afCharacterWidthList != orxNULL)
              {
                /* Frees it */
                orxMemory_Free(afCharacterWidthList);
              }
            }
            else
            {
              /* Logs message */
              orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Invalid character list (%s) for font (%s).", zCharacterList, _pstFont->zReference);

              /* Unlinks texture */
              orxFont_SetTexture(_pstFont, orxNULL);
            }

            /* Reactivates map update */
            orxStructure_SetFlags(_pstFont, orxFONT_KU32_FLAG_CAN_UPDATE_MAP, orxFONT_KU32_FLAG_NONE);

            /* Updates its map */
            orxFont_UpdateMap(_pstFont);
          }
          else
          {
            /* Logs message */
            orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't link texture (%s) to font (%s).", zName, _pstFont->zReference);

            /* Deletes texture */
            orxTexture_Delete(pstTexture);
          }
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't create texture (%s) for font (%s).", zName, _pstFont->zReference);

          /* Removes texture */
          orxFont_SetTexture(_pstFont, orxNULL);
        }
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't find texture property for font (%s).", _pstFont->zReference);
      }
    }

    /* Pops config section */
    orxConfig_PopSection();
  }

  /* Done! */
  return eResult;
}

/** Event handler
 */
static orxSTATUS orxFASTCALL orxFont_EventHandler(const orxEVENT *_pstEvent)
{
  orxRESOURCE_EVENT_PAYLOAD  *pstPayload;
  orxSTATUS                   eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(_pstEvent->eType == orxEVENT_TYPE_RESOURCE);

  /* Gets payload */
  pstPayload = (orxRESOURCE_EVENT_PAYLOAD *)_pstEvent->pstPayload;

  /* Is config or font group? */
  if((pstPayload->stGroupID == orxString_Hash(orxCONFIG_KZ_RESOURCE_GROUP))
  || (pstPayload->stGroupID == orxString_Hash(orxFONT_KZ_RESOURCE_GROUP)))
  {
    orxFONT *pstFont;

    /* For all fonts */
    for(pstFont = orxFONT(orxStructure_GetFirst(orxSTRUCTURE_ID_FONT));
        pstFont != orxNULL;
        pstFont = orxFONT(orxStructure_GetNext(pstFont)))
    {
      /* Not default one and has reference? */
      if((pstFont != sstFont.pstDefaultFont) && (pstFont->zReference != orxNULL) && (pstFont->zReference != orxSTRING_EMPTY))
      {
        /* Match origin? */
        if(orxConfig_GetOriginID(pstFont->zReference) == pstPayload->stNameID)
        {
          /* Re-processes its config data */
          orxFont_ProcessConfigData(pstFont);
        }
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Deletes all fonts
 */
static orxINLINE void orxFont_DeleteAll()
{
  orxFONT *pstFont;

  /* Gets first font */
  pstFont = orxFONT(orxStructure_GetFirst(orxSTRUCTURE_ID_FONT));

  /* Non empty? */
  while(pstFont != orxNULL)
  {
    /* Deletes font */
    orxFont_Delete(pstFont);

    /* Gets first font */
    pstFont = orxFONT(orxStructure_GetFirst(orxSTRUCTURE_ID_FONT));
  }

  /* Done! */
  return;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Setups the font module
 */
void orxFASTCALL orxFont_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_FONT, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_FONT, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_FONT, orxMODULE_ID_EVENT);
  orxModule_AddDependency(orxMODULE_ID_FONT, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_FONT, orxMODULE_ID_RESOURCE);
  orxModule_AddDependency(orxMODULE_ID_FONT, orxMODULE_ID_SHADER);
  orxModule_AddDependency(orxMODULE_ID_FONT, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_FONT, orxMODULE_ID_TEXTURE);

  /* Done! */
  return;
}

/** Inits the font module
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFont_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstFont.u32Flags & orxFONT_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstFont, sizeof(orxFONT_STATIC));

    /* Creates reference table */
    sstFont.pstReferenceTable = orxHashTable_Create(orxFONT_KU32_REFERENCE_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if(sstFont.pstReferenceTable != orxNULL)
    {
      /* Creates font map bank */
      sstFont.pstMapBank = orxBank_Create(orxFONT_KU32_MAP_BANK_SIZE, sizeof(orxCHARACTER_MAP), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

      /* Valid? */
      if(sstFont.pstMapBank != orxNULL)
      {
        /* Registers structure type */
        eResult = orxSTRUCTURE_REGISTER(FONT, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxFONT_KU32_BANK_SIZE, orxNULL);
      }
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Tried to initialize font module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Initialized? */
  if(eResult != orxSTATUS_FAILURE)
  {
    orxCHAR acBuffer[96 + 1];

    /* Inits Flags */
    sstFont.u32Flags = orxFONT_KU32_STATIC_FLAG_READY;

    /* Creates default font */
    orxFont_CreateDefaultFont();

    /* Stores ASCII & ANSI characters lists */
    orxString_NCopy(acBuffer, sstDefaultFont.zCharacterList, sizeof(acBuffer) - 1);
    acBuffer[sizeof(acBuffer) - 1] = orxCHAR_NULL;
    sstFont.zASCIICharacterList = orxString_Store(acBuffer);
    sstFont.zANSICharacterList  = orxString_Store(sstDefaultFont.zCharacterList);

    /* Adds event handler */
    orxEvent_AddHandler(orxEVENT_TYPE_RESOURCE, orxFont_EventHandler);
    orxEvent_SetHandlerIDFlags(orxFont_EventHandler, orxEVENT_TYPE_RESOURCE, orxNULL, orxEVENT_GET_FLAG(orxRESOURCE_EVENT_ADD) | orxEVENT_GET_FLAG(orxRESOURCE_EVENT_UPDATE), orxEVENT_KU32_MASK_ID_ALL);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Initializing font module failed.");

    /* Has reference table? */
    if(sstFont.pstReferenceTable != orxNULL)
    {
      /* Deletes it */
      orxHashTable_Delete(sstFont.pstReferenceTable);
      sstFont.pstReferenceTable = orxNULL;
    }

    /* Has map bank? */
    if(sstFont.pstMapBank != orxNULL)
    {
      /* Deletes it */
      orxBank_Delete(sstFont.pstMapBank);
      sstFont.pstMapBank = orxNULL;
    }

    /* Updates Flags */
    sstFont.u32Flags &= ~orxFONT_KU32_STATIC_FLAG_READY;
  }

  /* Done! */
  return eResult;
}

/** Exits from the font module
 */
void orxFASTCALL orxFont_Exit()
{
  /* Initialized? */
  if(sstFont.u32Flags & orxFONT_KU32_STATIC_FLAG_READY)
  {
    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_RESOURCE, orxFont_EventHandler);

    /* Deletes default font */
    if(sstFont.pstDefaultFont != orxNULL)
    {
      orxTEXTURE *pstTexture;
      pstTexture = orxFont_GetTexture(sstFont.pstDefaultFont);
      orxFont_Delete(sstFont.pstDefaultFont);
      orxTexture_Delete(pstTexture);
      orxResource_SetMemoryResource(orxTEXTURE_KZ_RESOURCE_GROUP, orxNULL, orxFONT_KZ_DEFAULT_TEXTURE_NAME, 0, orxNULL);
      orxResource_SetMemoryResource(orxCONFIG_KZ_RESOURCE_GROUP, orxNULL, orxFONT_KZ_CONFIG_SDF, 0, orxNULL);
    }

    /* Deletes font list */
    orxFont_DeleteAll();

    /* Deletes reference table */
    orxHashTable_Delete(sstFont.pstReferenceTable);
    sstFont.pstReferenceTable = orxNULL;

    /* Deletes map bank */
    orxBank_Delete(sstFont.pstMapBank);
    sstFont.pstMapBank = orxNULL;

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_FONT);

    /* Updates flags */
    sstFont.u32Flags &= ~orxFONT_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Tried to exit font module when it wasn't initialized.");
  }

  /* Done! */
  return;
}

/** Creates an empty font
 * @return      orxFONT / orxNULL
 */
orxFONT *orxFASTCALL orxFont_Create()
{
  orxFONT *pstResult;

  /* Checks */
  orxASSERT(sstFont.u32Flags & orxFONT_KU32_STATIC_FLAG_READY);

  /* Creates font */
  pstResult = orxFONT(orxStructure_Create(orxSTRUCTURE_ID_FONT));

  /* Created? */
  if(pstResult != orxNULL)
  {
    /* Allocates its map */
    pstResult->pstMap = (orxCHARACTER_MAP *)orxBank_Allocate(sstFont.pstMapBank);

    /* Valid? */
    if(pstResult->pstMap != orxNULL)
    {
      /* Creates its character bank */
      pstResult->pstMap->pstCharacterBank = orxBank_Create(orxFONT_KU32_CHARACTER_BANK_SIZE, sizeof(orxCHARACTER_GLYPH), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

      /* Valid? */
      if(pstResult->pstMap->pstCharacterBank != orxNULL)
      {
        /* Creates its character table */
        pstResult->pstMap->pstCharacterTable = orxHashTable_Create(orxFONT_KU32_CHARACTER_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

        /* Valid? */
        if(pstResult->pstMap->pstCharacterTable != orxNULL)
        {
          /* Clears its character list */
          pstResult->zCharacterList = orxSTRING_EMPTY;

          /* Increases count */
          orxStructure_IncreaseCount(pstResult);

          /* Updates status flags */
          orxStructure_SetFlags(pstResult, orxFONT_KU32_FLAG_CAN_UPDATE_MAP, orxFONT_KU32_FLAG_NONE);
        }
        else
        {
          /* Deletes character bank */
          orxBank_Delete(pstResult->pstMap->pstCharacterBank);

          /* Deletes maps */
          orxBank_Free(sstFont.pstMapBank, pstResult->pstMap);

          /* Deletes structure */
          orxStructure_Delete(pstResult);

          /* Updates result */
          pstResult = orxNULL;

          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't allocate font's character table.");
        }
      }
      else
      {
        /* Deletes map */
        orxBank_Free(sstFont.pstMapBank, pstResult->pstMap);

        /* Deletes structure */
        orxStructure_Delete(pstResult);

        /* Updates result */
        pstResult = orxNULL;

        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't allocate font's character bank.");
      }
    }
    else
    {
      /* Deletes structure */
      orxStructure_Delete(pstResult);

      /* Updates result */
      pstResult = orxNULL;

      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't allocate font's map.");
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Failed to create structure for font.");
  }

  return pstResult;
}

/** Creates a font from config
 * @param[in]   _zConfigID    Config ID
 * @return      orxFONT / orxNULL
 */
orxFONT *orxFASTCALL orxFont_CreateFromConfig(const orxSTRING _zConfigID)
{
  orxSTRINGID stID;
  orxFONT    *pstResult;

  /* Checks */
  orxASSERT(sstFont.u32Flags & orxFONT_KU32_STATIC_FLAG_READY);
  orxASSERT(_zConfigID != orxNULL);

  /* Gets font ID */
  stID = orxString_Hash(_zConfigID);

  /* Searches for font */
  pstResult = (orxFONT *)orxHashTable_Get(sstFont.pstReferenceTable, stID);

  /* Found? */
  if(pstResult != orxNULL)
  {
    /* Increases count */
    orxStructure_IncreaseCount(pstResult);
  }
  else
  {
    /* Pushes section */
    if((orxConfig_HasSection(_zConfigID) != orxFALSE)
    && (orxConfig_PushSection(_zConfigID) != orxSTATUS_FAILURE))
    {
      /* Creates font */
      pstResult = orxFont_Create();

      /* Valid? */
      if(pstResult != orxNULL)
      {
        /* Stores its reference key */
        pstResult->zReference = orxConfig_GetCurrentSection();

        /* Processes its config data */
        if(orxFont_ProcessConfigData(pstResult) != orxSTATUS_FAILURE)
        {
          /* Adds it to reference table */
          orxHashTable_Add(sstFont.pstReferenceTable, stID, pstResult);

          /* Should keep it in cache? */
          if(orxConfig_GetBool(orxFONT_KZ_CONFIG_KEEP_IN_CACHE) != orxFALSE)
          {
            /* Increases its reference count to keep it in cache table */
            orxStructure_IncreaseCount(pstResult);

            /* Updates its flags */
            orxStructure_SetFlags(pstResult, orxFONT_KU32_FLAG_CACHED, orxFONT_KU32_FLAG_NONE);
          }
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't process config data for font <%s>.", _zConfigID);

          /* Deletes font */
          orxFont_Delete(pstResult);

          /* Updates result */
          pstResult = orxNULL;
        }
      }

      /* Pops previous section */
      orxConfig_PopSection();
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't find config section named (%s).", _zConfigID);

      /* Updates result */
      pstResult = orxNULL;
    }
  }

  /* Done! */
  return pstResult;
}

/** Deletes a font
 * @param[in]   _pstFont      Concerned font
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFont_Delete(orxFONT *_pstFont)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstFont.u32Flags & orxFONT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFont);

  /* Decreases count */
  orxStructure_DecreaseCount(_pstFont);

  /* Not referenced? */
  if(orxStructure_GetRefCount(_pstFont) == 0)
  {
    /* Removes texture */
    orxFont_SetTexture(_pstFont, orxNULL);

    /* Removes shader */
    orxFont_SetShader(_pstFont, orxNULL);

    /* Deletes character table */
    orxHashTable_Delete(_pstFont->pstMap->pstCharacterTable);

    /* Deletes character bank */
    orxBank_Delete(_pstFont->pstMap->pstCharacterBank);

    /* Had a character width list? */
    if(_pstFont->afCharacterWidthList != orxNULL)
    {
      /* Frees it */
      orxMemory_Free(_pstFont->afCharacterWidthList);
    }

    /* Deletes map */
    orxBank_Free(sstFont.pstMapBank, _pstFont->pstMap);

    /* Is referenced? */
    if(orxStructure_TestFlags(_pstFont, orxFONT_KU32_FLAG_REFERENCED) != orxFALSE)
    {
      /* Removes it from reference table */
      orxHashTable_Remove(sstFont.pstReferenceTable, orxString_Hash(_pstFont->zReference));
    }

    /* Deletes structure */
    orxStructure_Delete(_pstFont);
  }
  else
  {
    /* Referenced by others */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Clears cache (if any Font is still in active use, it'll remain in memory until not referenced anymore)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFont_ClearCache()
{
  orxFONT  *pstFont, *pstNextFont;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstFont.u32Flags & orxFONT_KU32_STATIC_FLAG_READY);

  /* For all fonts */
  for(pstFont = orxFONT(orxStructure_GetFirst(orxSTRUCTURE_ID_FONT));
      pstFont != orxNULL;
      pstFont = pstNextFont)
  {
    /* Gets next font */
    pstNextFont = orxFONT(orxStructure_GetNext(pstFont));

    /* Is cached? */
    if(orxStructure_TestFlags(pstFont, orxFONT_KU32_FLAG_CACHED))
    {
      /* Updates its flags */
      orxStructure_SetFlags(pstFont, orxFONT_KU32_FLAG_NONE, orxFONT_KU32_FLAG_CACHED);

      /* Deletes its extra reference */
      orxFont_Delete(pstFont);
    }
  }

  /* Done! */
  return eResult;
}

/** Sets font's texture
 * @param[in]   _pstFont      Concerned font
 * @param[in]   _pstTexture   Texture to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFont_SetTexture(orxFONT *_pstFont, orxTEXTURE *_pstTexture)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstFont.u32Flags & orxFONT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFont);

  /* Had previous texture? */
  if(_pstFont->pstTexture != orxNULL)
  {
    /* Updates structure reference count */
    orxStructure_DecreaseCount(_pstFont->pstTexture);

    /* Internally handled? */
    if(orxStructure_TestFlags(_pstFont, orxFONT_KU32_FLAG_INTERNAL))
    {
      /* Removes its owner */
      orxStructure_SetOwner(_pstFont->pstTexture, orxNULL);

      /* Deletes it */
      orxTexture_Delete(_pstFont->pstTexture);

      /* Updates flags */
      orxStructure_SetFlags(_pstFont, orxFONT_KU32_FLAG_NONE, orxFONT_KU32_FLAG_INTERNAL);
    }

    /* Cleans reference */
    _pstFont->pstTexture = orxNULL;

    /* Cleans typeface */
    _pstFont->zTypeface = orxNULL;

    /* Clears origin & size */
    _pstFont->fTop = _pstFont->fLeft = _pstFont->fWidth = _pstFont->fHeight = orxFLOAT_0;
  }

  /* New texture? */
  if(_pstTexture != orxNULL)
  {
    /* Stores it */
    _pstFont->pstTexture = _pstTexture;

    /* Updates its reference count */
    orxStructure_IncreaseCount(_pstTexture);

    /* Updates font's size */
    orxTexture_GetSize(_pstTexture, &(_pstFont->fWidth), &(_pstFont->fHeight));
  }

  /* Updates font's map */
  orxFont_UpdateMap(_pstFont);

  /* Done! */
  return eResult;
}

/** Sets font's character list
 * @param[in]   _pstFont      Concerned font
 * @param[in]   _zList        Character list
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFont_SetCharacterList(orxFONT *_pstFont, const orxSTRING _zList)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstFont.u32Flags & orxFONT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFont);

  /* Had a character list? */
  if(_pstFont->zCharacterList != orxSTRING_EMPTY)
  {
    /* Cleans its reference */
    _pstFont->zCharacterList = orxSTRING_EMPTY;
  }

  /* Valid? */
  if((_zList != orxNULL) && (_zList != orxSTRING_EMPTY))
  {
    /* Stores it */
    _pstFont->zCharacterList = orxString_Store(_zList);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Updates font's map */
  orxFont_UpdateMap(_pstFont);

  /* Done! */
  return eResult;
}

/** Sets font's character height
 * @param[in]   _pstFont              Concerned font
 * @param[in]   _fCharacterHeight     Character's height
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFont_SetCharacterHeight(orxFONT *_pstFont, orxFLOAT _fCharacterHeight)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstFont.u32Flags & orxFONT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFont);

  /* Stores it */
  _pstFont->fCharacterHeight = _fCharacterHeight;

  /* Done! */
  return eResult;
}

/** Sets font's character width list
 * @param[in]   _pstFont              Concerned font
 * @param[in]   _u32CharacterNumber   Character's number
= * @param[in]   _afCharacterWidthList List of widths for all the characters
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFont_SetCharacterWidthList(orxFONT *_pstFont, orxU32 _u32CharacterNumber, const orxFLOAT *_afCharacterWidthList)
{
  orxU32    u32CharacterCount;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstFont.u32Flags & orxFONT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFont);
  orxASSERT(_afCharacterWidthList != orxNULL);

  /* Gets character count */
  u32CharacterCount = orxString_GetCharacterCount(_pstFont->zCharacterList);

  /* Valid? */
  if(_u32CharacterNumber == u32CharacterCount)
  {
    /* Had a character width list? */
    if(_pstFont->afCharacterWidthList != orxNULL)
    {
      /* Frees it */
      orxMemory_Free(_pstFont->afCharacterWidthList);
    }

    /* Allocates character width list */
    _pstFont->afCharacterWidthList = (orxFLOAT *)orxMemory_Allocate(u32CharacterCount * sizeof(orxFLOAT), orxMEMORY_TYPE_MAIN);
    orxASSERT(_pstFont->afCharacterWidthList != orxNULL);

    /* Stores values */
    orxMemory_Copy(_pstFont->afCharacterWidthList, _afCharacterWidthList, u32CharacterCount * sizeof(orxFLOAT));

    /* Updates font's map */
    orxFont_UpdateMap(_pstFont);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets font's character spacing
 * @param[in]   _pstFont      Concerned font
 * @param[in]   _pvSpacing    Character's spacing
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFont_SetCharacterSpacing(orxFONT *_pstFont, const orxVECTOR *_pvSpacing)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstFont.u32Flags & orxFONT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFont);
  orxASSERT(_pvSpacing != orxNULL);

  /* Valid? */
  if((_pvSpacing->fX >= orxFLOAT_0) && (_pvSpacing->fY >= orxFLOAT_0))
  {
    /* Stores it */
    orxVector_Set(&(_pstFont->vCharacterSpacing), _pvSpacing->fX, _pvSpacing->fY, orxFLOAT_0);

    /* Updates font's map */
    orxFont_UpdateMap(_pstFont);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets font's origin
 * @param[in]   _pstFont      Concerned font
 * @param[in]   _pvOrigin     Font's origin
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFont_SetOrigin(orxFONT *_pstFont, const orxVECTOR *_pvOrigin)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstFont.u32Flags & orxFONT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFont);
  orxASSERT(_pvOrigin != orxNULL);

  /* Has texture? */
  if(_pstFont->pstTexture)
  {
    orxFLOAT fWidth, fHeight;

    /* Gets its size */
    orxTexture_GetSize(_pstFont->pstTexture, &fWidth, &fHeight);

    /* Valid? */
    if((_pvOrigin->fX >= orxFLOAT_0)
    && (_pvOrigin->fX < fWidth)
    && (_pvOrigin->fY >= orxFLOAT_0)
    && (_pvOrigin->fY < fHeight))
    {
      /* Stores it */
      _pstFont->fLeft = _pvOrigin->fX;
      _pstFont->fTop  = _pvOrigin->fY;

      /* Updates font's map */
      orxFont_UpdateMap(_pstFont);

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
  }

  /* Done! */
  return eResult;
}

/** Sets font's size
 * @param[in]   _pstFont      Concerned font
 * @param[in]   _pvSize       Font's size
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFont_SetSize(orxFONT *_pstFont, const orxVECTOR *_pvSize)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstFont.u32Flags & orxFONT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFont);
  orxASSERT(_pvSize != orxNULL);

  /* Has texture? */
  if(_pstFont->pstTexture)
  {
    orxFLOAT fWidth, fHeight;

    /* Gets its size */
    orxTexture_GetSize(_pstFont->pstTexture, &fWidth, &fHeight);

    /* Valid? */
    if((_pvSize->fX > orxFLOAT_0)
    && (_pvSize->fX <= fWidth)
    && (_pvSize->fY > orxFLOAT_0)
    && (_pvSize->fY <= fHeight))
    {
      /* Stores it */
      _pstFont->fWidth  = _pvSize->fX;
      _pstFont->fHeight = _pvSize->fY;

      /* Updates font's map */
      orxFont_UpdateMap(_pstFont);

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
  }

  /* Done! */
  return eResult;
}

/** Sets font's shader
 * @param[in]   _pstFont      Concerned font
 * @param[in]   _zShaderID    Config ID of the shader to set, orxNULL to remove the current one
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFont_SetShader(orxFONT *_pstFont, const orxSTRING _zShaderID)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstFont.u32Flags & orxFONT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFont);

  /* Had previous shader? */
  if(_pstFont->pstShader != orxNULL)
  {
    /* Decreases its reference count */
    orxStructure_DecreaseCount(_pstFont->pstShader);

    /* Removes its owner */
    orxStructure_SetOwner(_pstFont->pstShader, orxNULL);

    /* Deletes it */
    orxShader_Delete(_pstFont->pstShader);

    /* Cleans reference */
    _pstFont->pstShader = orxNULL;
  }

  /* New shader? */
  if((_zShaderID != orxNULL) && (*_zShaderID != orxCHAR_NULL))
  {
    /* Creates it */
    _pstFont->pstShader = orxShader_CreateFromConfig(_zShaderID);

    /* Success? */
    if(_pstFont->pstShader != orxNULL)
    {
      /* Increases its reference count */
      orxStructure_IncreaseCount(_pstFont->pstShader);

      /* Sets its owner */
      orxStructure_SetOwner(_pstFont->pstShader, _pstFont);

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
  }

  /* Done! */
  return eResult;
}

/** Gets font's texture
 * @param[in]   _pstFont      Concerned font
 * @return      Font texture / orxNULL
 */
orxTEXTURE *orxFASTCALL orxFont_GetTexture(const orxFONT *_pstFont)
{
  orxTEXTURE *pstResult;

  /* Checks */
  orxASSERT(sstFont.u32Flags & orxFONT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFont);

  /* Updates result */
  pstResult = _pstFont->pstTexture;

  /* Done! */
  return pstResult;
}

/** Gets font's character list
 * @param[in]   _pstFont      Concerned font
 * @return      Font's character list / orxNULL
 */
const orxSTRING orxFASTCALL orxFont_GetCharacterList(const orxFONT *_pstFont)
{
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(sstFont.u32Flags & orxFONT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFont);

  /* Updates result */
  zResult = _pstFont->zCharacterList;

  /* Done! */
  return zResult;
}

/** Gets font's character height
 * @param[in]   _pstFont                Concerned font
 * @return      orxFLOAT
 */
orxFLOAT orxFASTCALL orxFont_GetCharacterHeight(const orxFONT *_pstFont)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT(sstFont.u32Flags & orxFONT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFont);

  /* Updates result */
  fResult = _pstFont->fCharacterHeight;

  /* Done! */
  return fResult;
}

/** Gets font's character width
 * @param[in]   _pstFont                Concerned font
 * @param[in]   _u32CharacterCodePoint  Character code point
 * @return      orxFLOAT
 */
orxFLOAT orxFASTCALL orxFont_GetCharacterWidth(const orxFONT *_pstFont, orxU32 _u32CharacterCodePoint)
{
  orxCHARACTER_GLYPH *pstGlyph;
  orxFLOAT fResult = orxFLOAT_0;

  /* Checks */
  orxASSERT(sstFont.u32Flags & orxFONT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFont);

  /* Gets glyph */
  pstGlyph = (orxCHARACTER_GLYPH *)orxHashTable_Get(_pstFont->pstMap->pstCharacterTable, _u32CharacterCodePoint);

  /* Valid? */
  if(pstGlyph != orxNULL)
  {
    /* Updates result */
    fResult = pstGlyph->fWidth;
  }

  /* Done! */
  return fResult;
}

/** Gets font's character spacing
 * @param[in]   _pstFont      Concerned font
 * @param[out]  _pvSpacing    Character's spacing
 * @return      orxVECTOR / orxNULL
 */
orxVECTOR *orxFASTCALL orxFont_GetCharacterSpacing(const orxFONT *_pstFont, orxVECTOR *_pvSpacing)
{
  orxVECTOR *pvResult = _pvSpacing;

  /* Checks */
  orxASSERT(sstFont.u32Flags & orxFONT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFont);
  orxASSERT(_pvSpacing != orxNULL);

  /* Updates result */
  orxVector_Copy(pvResult, &(_pstFont->vCharacterSpacing));

  /* Done! */
  return pvResult;
}

/** Gets font's origin
 * @param[in]   _pstFont      Concerned font
 * @param[out]  _pvOrigin     Font's origin
 * @return      orxVECTOR / orxNULL
 */
orxVECTOR *orxFASTCALL orxFont_GetOrigin(const orxFONT *_pstFont, orxVECTOR *_pvOrigin)
{
  orxVECTOR *pvResult = _pvOrigin;

  /* Checks */
  orxASSERT(sstFont.u32Flags & orxFONT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFont);
  orxASSERT(_pvOrigin != orxNULL);

  /* Updates result */
  orxVector_Set(pvResult, _pstFont->fLeft, _pstFont->fTop, orxFLOAT_0);

  /* Done! */
  return pvResult;
}

/** Gets font's size
 * @param[in]   _pstFont      Concerned font
 * @param[out]  _pvSize       Font's size
 * @return      orxVECTOR / orxNULL
 */
orxVECTOR *orxFASTCALL orxFont_GetSize(const orxFONT *_pstFont, orxVECTOR *_pvSize)
{
  orxVECTOR *pvResult = _pvSize;

  /* Checks */
  orxASSERT(sstFont.u32Flags & orxFONT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFont);
  orxASSERT(_pvSize != orxNULL);

  /* Updates result */
  orxVector_Set(pvResult, _pstFont->fWidth, _pstFont->fHeight, orxFLOAT_0);

  /* Done! */
  return pvResult;
}

/** Gets font's shader
 * @param[in]   _pstFont      Concerned font
 * @return     orxSHADER / orxNULL
 */
const orxSHADER *orxFASTCALL orxFont_GetShader(const orxFONT *_pstFont)
{
  const orxSHADER *pstResult;

  /* Checks */
  orxASSERT(sstFont.u32Flags & orxFONT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFont);

  /* Updates result */
  pstResult = _pstFont->pstShader;

  /* Done! */
  return pstResult;
}

/** Is Font SDF?
 * @param[in]   _pstFont      Concerned font
 * @return      orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxFont_IsSDF(const orxFONT *_pstFont)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT(sstFont.u32Flags & orxFONT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFont);

  /* Updates result */
  bResult = orxStructure_TestFlags(_pstFont, orxFONT_KU32_FLAG_SDF);

  /* Done! */
  return bResult;
}

/** Gets font's map
 * @param[in]   _pstFont      Concerned font
 * @return      orxCHARACTER_MAP / orxNULL
 */
const orxCHARACTER_MAP *orxFASTCALL orxFont_GetMap(const orxFONT *_pstFont)
{
  const orxCHARACTER_MAP *pstResult;

  /* Checks */
  orxASSERT(sstFont.u32Flags & orxFONT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFont);

  /* Updates result */
  pstResult = _pstFont->pstMap;

  /* Done! */
  return pstResult;
}

/** Gets font given its name
 * @param[in]   _zName        Font name
 * @return      orxFONT / orxNULL
 */
orxFONT *orxFASTCALL orxFont_Get(const orxSTRING _zName)
{
  orxFONT *pstResult;

  /* Checks */
  orxASSERT(sstFont.u32Flags & orxFONT_KU32_STATIC_FLAG_READY);
  orxASSERT(_zName != orxNULL);

  /* Updates result */
  pstResult = (orxFONT *)orxHashTable_Get(sstFont.pstReferenceTable, orxString_Hash(_zName));

  /* Done! */
  return pstResult;
}

/** Gets font name
 * @param[in]   _pstFont      Concerned font
 * @return      Font name / orxSTRING_EMPTY
 */
const orxSTRING orxFASTCALL orxFont_GetName(const orxFONT *_pstFont)
{
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(sstFont.u32Flags & orxFONT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFont);

  /* Updates result */
  zResult = (_pstFont->zReference != orxNULL) ? _pstFont->zReference : orxSTRING_EMPTY;

  /* Done! */
  return zResult;
}
