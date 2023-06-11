/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2022 Orx-Project
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

#define orxFONT_KU32_MASK_ALL                   0xFFFFFFFF  /**< All mask */


/** Misc defines
 */
#define orxFONT_KU32_MAP_BANK_SIZE              4           /**< Map bank size */
#define orxFONT_KU32_REFERENCE_TABLE_SIZE       4           /**< Reference table size */
#define orxFONT_KU32_CHARACTER_BANK_SIZE        256         /**< Character bank size */
#define orxFONT_KU32_CHARACTER_TABLE_SIZE       256         /**< Character table size */

#define orxFONT_KU32_BANK_SIZE                  16          /**< Bank size */

#define orxFONT_KZ_CONFIG_TEXTURE_NAME          "Texture"
#define orxFONT_KZ_CONFIG_CHARACTER_LIST        "CharacterList"
#define orxFONT_KZ_CONFIG_CHARACTER_SIZE        "CharacterSize"
#define orxFONT_KZ_CONFIG_CHARACTER_HEIGHT      "CharacterHeight"
#define orxFONT_KZ_CONFIG_CHARACTER_WIDTH_LIST  "CharacterWidthList"
#define orxFONT_KZ_CONFIG_CHARACTER_SPACING     "CharacterSpacing"
#define orxFONT_KZ_CONFIG_TEXTURE_ORIGIN        "TextureOrigin"
#define orxFONT_KZ_CONFIG_TEXTURE_SIZE          "TextureSize"
#define orxFONT_KZ_CONFIG_TEXTURE_CORNER        "TextureCorner" /**< Kept for retro-compatibility reason */
#define orxFONT_KZ_CONFIG_KEEP_IN_CACHE         "KeepInCache"


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
};

/** Static structure
 */
typedef struct __orxFONT_STATIC_t
{
  orxBANK          *pstMapBank;                 /**< Map bank : 4 */
  orxHASHTABLE     *pstReferenceTable;          /**< Reference table : 8 */
  orxFONT          *pstDefaultFont;             /**< Default font : 12 */
  orxU32            u32Flags;                   /**< Control flags : 16 */

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

  /* Done! */
  return;
}

/* Creates default font
 */
static orxINLINE void orxFont_CreateDefaultFont()
{
  orxTEXTURE *pstTexture;

  /* Creates texture */
  pstTexture = orxTexture_Create();

  /* Success? */
  if(pstTexture != orxNULL)
  {
    orxBITMAP *pstBitmap;

    /* Creates bitmap */
    pstBitmap = orxDisplay_CreateBitmap(sstDefaultFont.u32Width, sstDefaultFont.u32Height);

    /* Success? */
    if(pstBitmap != orxNULL)
    {
      /* Sets it data */
      if(orxDisplay_SetBitmapData(pstBitmap, sstDefaultFont.au8Data, sstDefaultFont.u32Width * sstDefaultFont.u32Height * 4) != orxSTATUS_FAILURE)
      {
        /* Links it to texture and transfers its ownership */
        if(orxTexture_LinkBitmap(pstTexture, pstBitmap, orxFONT_KZ_DEFAULT_FONT_NAME, orxTRUE) != orxSTATUS_FAILURE)
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

              /* Sets font as texture's owner */
              orxStructure_SetOwner(pstTexture, sstFont.pstDefaultFont);

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

              /* Inits font */
              orxFont_SetCharacterList(sstFont.pstDefaultFont, sstDefaultFont.zCharacterList);
              orxFont_SetCharacterHeight(sstFont.pstDefaultFont, sstDefaultFont.fCharacterHeight);
              orxFont_SetCharacterWidthList(sstFont.pstDefaultFont, u32CharacterCount, afCharacterWidthList);
              orxFont_SetCharacterSpacing(sstFont.pstDefaultFont, orxVector_Set(&vSpacing, sstDefaultFont.fCharacterSpacingX, sstDefaultFont.fCharacterSpacingY, orxFLOAT_0));

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
          orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't link default font's bitmap to texture.");

          /* Deletes bitmap */
          orxDisplay_DeleteBitmap(pstBitmap);

          /* Deletes texture */
          orxTexture_Delete(pstTexture);
        }
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't set default font's bitmap's data.");

        /* Deletes bitmap */
        orxDisplay_DeleteBitmap(pstBitmap);

        /* Deletes texture */
        orxTexture_Delete(pstTexture);
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't create default font's bitmap.");

      /* Deletes texture */
      orxTexture_Delete(pstTexture);
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't create default font's texture.");
  }
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

    /* Gets texture name */
    zName = orxConfig_GetString(orxFONT_KZ_CONFIG_TEXTURE_NAME);

    /* Valid? */
    if((zName != orxNULL) && (zName != orxSTRING_EMPTY))
    {
      orxTEXTURE *pstTexture;

      /* Loads texture */
      pstTexture = orxTexture_Load(zName, orxConfig_GetBool(orxFONT_KZ_CONFIG_KEEP_IN_CACHE));

      /* Valid? */
      if(pstTexture != orxNULL)
      {
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

                /* Deletes texture */
                orxTexture_Delete(pstTexture);

                /* Updates result */
                eResult = orxSTATUS_FAILURE;
              }
            }
            else
            {
              /* Deletes texture */
              orxTexture_Delete(pstTexture);
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

            /* Deletes texture */
            orxTexture_Delete(pstTexture);
          }
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
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Add or update? */
  if((_pstEvent->eID == orxRESOURCE_EVENT_ADD) || (_pstEvent->eID == orxRESOURCE_EVENT_UPDATE))
  {
    orxRESOURCE_EVENT_PAYLOAD *pstPayload;

    /* Gets payload */
    pstPayload = (orxRESOURCE_EVENT_PAYLOAD *)_pstEvent->pstPayload;

    /* Is config group? */
    if(pstPayload->stGroupID == orxString_Hash(orxCONFIG_KZ_RESOURCE_GROUP))
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
  orxModule_AddDependency(orxMODULE_ID_FONT, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_FONT, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_FONT, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_FONT, orxMODULE_ID_EVENT);
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
    /* Inits Flags */
    sstFont.u32Flags = orxFONT_KU32_STATIC_FLAG_READY;

    /* Creates default font */
    orxFont_CreateDefaultFont();

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
    orxTEXTURE *pstTexture;

    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_RESOURCE, orxFont_EventHandler);

    /* Gets default font texture */
    pstTexture = orxFont_GetTexture(sstFont.pstDefaultFont);

    /* Deletes font list */
    orxFont_DeleteAll();

    /* Removes texture's owner */
    orxStructure_SetOwner(pstTexture, orxNULL);

    /* Deletes default font texture */
    orxTexture_Delete(pstTexture);

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
  orxFONT *pstResult;

  /* Checks */
  orxASSERT(sstFont.u32Flags & orxFONT_KU32_STATIC_FLAG_READY);
  orxASSERT(_zConfigID != orxNULL);

  /* Search for font */
  pstResult = (orxFONT *)orxHashTable_Get(sstFont.pstReferenceTable, orxString_Hash(_zConfigID));

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
          orxHashTable_Add(sstFont.pstReferenceTable, orxString_Hash(pstResult->zReference), pstResult);
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

    /* Deletes character table */
    orxHashTable_Delete(_pstFont->pstMap->pstCharacterTable);

    /* Deletes character bank */
    orxBank_Delete(_pstFont->pstMap->pstCharacterBank);

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

/** Gets default font
 * @return      Default font / orxNULL
 */
const orxFONT *orxFASTCALL orxFont_GetDefaultFont()
{
  orxFONT *pstResult;

  /* Checks */
  orxASSERT(sstFont.u32Flags & orxFONT_KU32_STATIC_FLAG_READY);

  /* Updates result */
  pstResult = sstFont.pstDefaultFont;

  /* Done ! */
  return pstResult;
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

  /* Has character map? */
  if(_pstFont->pstMap != orxNULL)
  {
    /* Gets glyph */
    pstGlyph = (orxCHARACTER_GLYPH *)orxHashTable_Get(_pstFont->pstMap->pstCharacterTable, _u32CharacterCodePoint);

    /* Valid? */
    if(pstGlyph != orxNULL)
    {
      /* Updates result */
      fResult = pstGlyph->fWidth;
    }
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
