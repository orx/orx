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
 * @file orxText.c
 * @date 02/12/2008
 * @author iarwain@orx-project.org
 *
 */


#include "display/orxText.h"

#include "memory/orxMemory.h"
#include "core/orxConfig.h"
#include "core/orxEvent.h"
#include "core/orxLocale.h"
#include "core/orxResource.h"
#include "math/orxVector.h"
#include "object/orxStructure.h"
#include "utils/orxHashTable.h"


/** Module flags
 */
#define orxTEXT_KU32_STATIC_FLAG_NONE         0x00000000  /**< No flags */

#define orxTEXT_KU32_STATIC_FLAG_READY        0x00000001  /**< Ready flag */

#define orxTEXT_KU32_STATIC_MASK_ALL          0xFFFFFFFF  /**< All mask */

/** orxTEXT flags / masks
 */
#define orxTEXT_KU32_FLAG_NONE                0x00000000  /**< No flags */

#define orxTEXT_KU32_FLAG_INTERNAL            0x10000000  /**< Internal structure handling flag */
#define orxTEXT_KU32_FLAG_FIXED_WIDTH         0x00000001  /**< Fixed width flag */
#define orxTEXT_KU32_FLAG_FIXED_HEIGHT        0x00000002  /**< Fixed height flag */

#define orxTEXT_KU32_MASK_ALL                 0xFFFFFFFF  /**< All mask */


/** Misc defines
 */
#define orxTEXT_KZ_CONFIG_STRING              "String"
#define orxTEXT_KZ_CONFIG_FONT                "Font"

#define orxTEXT_KC_LOCALE_MARKER              '$'

#define orxTEXT_KU32_BANK_SIZE                256         /**< Bank size */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Text structure
 */
struct __orxTEXT_t
{
  orxSTRUCTURE      stStructure;                /**< Public structure, first structure member : 40 / 64 */
  orxSTRING         zString;                    /**< String : 44 / 72 */
  orxFONT          *pstFont;                    /**< Font : 48 / 80 */
  orxSTRINGID       stLocaleStringID;           /**< Locale string ID : 56 / 88 */
  orxSTRINGID       stLocaleFontID;             /**< Locale font ID : 64 / 96 */
  orxFLOAT          fWidth;                     /**< Width : 68 / 100 */
  orxFLOAT          fHeight;                    /**< Height : 72 / 104 */
  const orxSTRING   zReference;                 /**< Config reference : 76 / 112 */
  orxSTRING         zOriginalString;            /**< Original string : 80 / 120 */
};

/** Static structure
 */
typedef struct __orxTEXT_STATIC_t
{
  orxU32            u32Flags;                   /**< Control flags */

} orxTEXT_STATIC;


/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/

static orxTEXT_STATIC sstText;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

static orxSTATUS orxFASTCALL orxText_ProcessConfigData(orxTEXT *_pstText)
{
  const orxSTRING zString;
  const orxSTRING zName;
  orxSTATUS       eResult = orxSTATUS_FAILURE;

  /* Pushes its config section */
  orxConfig_PushSection(_pstText->zReference);

  /* Gets font name */
  zName = orxConfig_GetString(orxTEXT_KZ_CONFIG_FONT);

  /* Begins with locale marker? */
  if(*zName == orxTEXT_KC_LOCALE_MARKER)
  {
    /* Updates name */
    zName = zName + 1;

    /* Using locale? */
    if(*zName != orxTEXT_KC_LOCALE_MARKER)
    {
      /* Stores its locale ID */
      _pstText->stLocaleFontID = orxString_GetID(zName);

      /* Gets its locale value */
      zName = orxLocale_GetString(zName, orxTEXT_KZ_LOCALE_GROUP);
    }
  }

  /* Valid? */
  if((zName != orxNULL) && (zName != orxSTRING_EMPTY))
  {
    orxFONT *pstFont;

    /* Creates font */
    pstFont = orxFont_CreateFromConfig(zName);

    /* Valid? */
    if(pstFont != orxNULL)
    {
      /* Stores it */
      if(orxText_SetFont(_pstText, pstFont) != orxSTATUS_FAILURE)
      {
        /* Sets its owner */
        orxStructure_SetOwner(pstFont, _pstText);

        /* Updates flags */
        orxStructure_SetFlags(_pstText, orxTEXT_KU32_FLAG_INTERNAL, orxTEXT_KU32_FLAG_NONE);
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't set font (%s) for text (%s).", zName, _pstText->zReference);

        /* Sets default font */
        orxText_SetFont(_pstText, orxFONT(orxFont_GetDefaultFont()));
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't create font (%s) for text (%s).", zName, _pstText->zReference);

      /* Sets default font */
      orxText_SetFont(_pstText, orxFONT(orxFont_GetDefaultFont()));
    }
  }
  else
  {
    /* Sets default font */
    orxText_SetFont(_pstText, orxFONT(orxFont_GetDefaultFont()));
  }

  /* Gets its string */
  zString = orxConfig_GetString(orxTEXT_KZ_CONFIG_STRING);

  /* Begins with locale marker? */
  if(*zString == orxTEXT_KC_LOCALE_MARKER)
  {
    /* Updates string */
    zString = zString + 1;

    /* Using locale? */
    if(*zString != orxTEXT_KC_LOCALE_MARKER)
    {
      /* Stores its locale ID */
      _pstText->stLocaleStringID = orxString_GetID(zString);

      /* Gets its locale value */
      zString = orxLocale_GetString(zString, orxTEXT_KZ_LOCALE_GROUP);
    }
  }

  /* Stores its value */
  eResult = orxText_SetString(_pstText, zString);

  /* Pops config section */
  orxConfig_PopSection();

  /* Done! */
  return eResult;
}

/** Event handler
 * @param[in]   _pstEvent                     Sent event
 * @return      orxSTATUS_SUCCESS if handled / orxSTATUS_FAILURE otherwise
 */
static orxSTATUS orxFASTCALL orxText_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Locale? */
  if(_pstEvent->eType == orxEVENT_TYPE_LOCALE)
  {
    /* Select language event? */
    if(_pstEvent->eID == orxLOCALE_EVENT_SELECT_LANGUAGE)
    {
      orxLOCALE_EVENT_PAYLOAD *pstPayload;

      /* Gets its payload */
      pstPayload = (orxLOCALE_EVENT_PAYLOAD *)_pstEvent->pstPayload;

      /* Text group? */
      if((pstPayload->zGroup == orxNULL) || (orxString_Compare(pstPayload->zGroup, orxTEXT_KZ_LOCALE_GROUP) == 0))
      {
        orxTEXT *pstText;

        /* For all texts */
        for(pstText = orxTEXT(orxStructure_GetFirst(orxSTRUCTURE_ID_TEXT));
            pstText != orxNULL;
            pstText = orxTEXT(orxStructure_GetNext(pstText)))
        {
          /* Has locale string ID? */
          if(pstText->stLocaleStringID != 0)
          {
            const orxSTRING zText;

            /* Gets its localized value */
            zText = orxLocale_GetString(orxString_GetFromID(pstText->stLocaleStringID), orxTEXT_KZ_LOCALE_GROUP);

            /* Valid? */
            if(*zText != orxCHAR_NULL)
            {
              /* Updates text */
              orxText_SetString(pstText, zText);
            }
          }

          /* Has locale font ID? */
          if(pstText->stLocaleFontID != 0)
          {
            orxFONT *pstFont;

            /* Creates font */
            pstFont = orxFont_CreateFromConfig(orxLocale_GetString(orxString_GetFromID(pstText->stLocaleFontID), orxTEXT_KZ_LOCALE_GROUP));

            /* Valid? */
            if(pstFont != orxNULL)
            {
              /* Updates text */
              if(orxText_SetFont(pstText, pstFont) != orxSTATUS_FAILURE)
              {
                /* Sets its owner */
                orxStructure_SetOwner(pstFont, pstText);

                /* Updates flags */
                orxStructure_SetFlags(pstText, orxTEXT_KU32_FLAG_INTERNAL, orxTEXT_KU32_FLAG_NONE);
              }
              else
              {
                /* Sets default font */
                orxText_SetFont(pstText, orxFONT(orxFont_GetDefaultFont()));
              }
            }
          }
        }
      }
    }
  }
  /* Resource */
  else
  {
    /* Checks */
    orxASSERT(_pstEvent->eType == orxEVENT_TYPE_RESOURCE);

    /* Add or update? */
    if((_pstEvent->eID == orxRESOURCE_EVENT_ADD) || (_pstEvent->eID == orxRESOURCE_EVENT_UPDATE))
    {
      orxRESOURCE_EVENT_PAYLOAD *pstPayload;

      /* Gets payload */
      pstPayload = (orxRESOURCE_EVENT_PAYLOAD *)_pstEvent->pstPayload;

      /* Is config group? */
      if(pstPayload->stGroupID == orxString_Hash(orxCONFIG_KZ_RESOURCE_GROUP))
      {
        orxTEXT *pstText;

        /* For all texts */
        for(pstText = orxTEXT(orxStructure_GetFirst(orxSTRUCTURE_ID_TEXT));
            pstText != orxNULL;
            pstText = orxTEXT(orxStructure_GetNext(pstText)))
        {
          /* Has reference? */
          if((pstText->zReference != orxNULL) && (pstText->zReference != orxSTRING_EMPTY))
          {
            /* Match origin? */
            if(orxConfig_GetOriginID(pstText->zReference) == pstPayload->stNameID)
            {
              /* Re-processes its config data */
              orxText_ProcessConfigData(pstText);
            }
          }
        }
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Updates text size
 * @param[in]   _pstText      Concerned text
 */
static void orxFASTCALL orxText_UpdateSize(orxTEXT *_pstText)
{
  /* Checks */
  orxSTRUCTURE_ASSERT(_pstText);

  /* Has original string? */
  if(_pstText->zOriginalString != orxNULL)
  {
    /* Has current string? */
    if(_pstText->zString != orxNULL)
    {
      /* Deletes it */
      orxString_Delete(_pstText->zString);
    }

    /* Restores string from original */
    _pstText->zString = _pstText->zOriginalString;
    _pstText->zOriginalString = orxNULL;
  }

  /* Has string and font? */
  if((_pstText->zString != orxNULL) && (_pstText->zString != orxSTRING_EMPTY) && (_pstText->pstFont != orxNULL))
  {
    orxFLOAT        fWidth, fHeight, fCharacterHeight;
    orxU32          u32CharacterCodePoint;

    /* Gets character height */
    fCharacterHeight = orxFont_GetCharacterHeight(_pstText->pstFont);

    /* No fixed size? */
    if(orxStructure_TestFlags(_pstText, orxTEXT_KU32_FLAG_FIXED_WIDTH | orxTEXT_KU32_FLAG_FIXED_HEIGHT) == orxFALSE)
    {
      const orxCHAR  *pc;
      orxFLOAT        fMaxWidth;

      /* For all characters */
      for(u32CharacterCodePoint = orxString_GetFirstCharacterCodePoint(_pstText->zString, &pc), fHeight = fCharacterHeight, fWidth = fMaxWidth = orxFLOAT_0;
          u32CharacterCodePoint != orxCHAR_NULL;
          u32CharacterCodePoint = orxString_GetFirstCharacterCodePoint(pc, &pc))
      {
        /* Depending on character */
        switch(u32CharacterCodePoint)
        {
          case orxCHAR_CR:
          {
            /* Half EOL? */
            if(*pc == orxCHAR_LF)
            {
              /* Updates pointer */
              pc++;
            }

            /* Fall through */
          }

          case orxCHAR_LF:
          {
            /* Updates height */
            fHeight += fCharacterHeight;

            /* Updates max width */
            fMaxWidth = orxMAX(fMaxWidth, fWidth);

            /* Resets width */
            fWidth = orxFLOAT_0;

            break;
          }

          default:
          {
            /* Updates width */
            fWidth += orxFont_GetCharacterWidth(_pstText->pstFont, u32CharacterCodePoint);

            break;
          }
        }
      }

      /* Stores values */
      _pstText->fWidth  = orxMAX(fWidth, fMaxWidth);
      _pstText->fHeight = fHeight;
    }
    else
    {
      orxCHAR  *pc;
      orxSTRING zLastSpace;

      /* For all characters */
      for(u32CharacterCodePoint = orxString_GetFirstCharacterCodePoint(_pstText->zString, (const orxCHAR **)&pc), fHeight = fCharacterHeight, fWidth = orxFLOAT_0, zLastSpace = orxNULL;
          u32CharacterCodePoint != orxCHAR_NULL;
          u32CharacterCodePoint = orxString_GetFirstCharacterCodePoint(pc, (const orxCHAR **)&pc))
      {
        /* Depending on the character */
        switch(u32CharacterCodePoint)
        {
          case orxCHAR_CR:
          {
            /* Half EOL? */
            if(*pc == orxCHAR_LF)
            {
              /* Updates pointer */
              pc++;
            }

            /* Fall through */
          }

          case orxCHAR_LF:
          {
            /* Updates height */
            fHeight += fCharacterHeight;

            /* Should truncate? */
            if((orxStructure_TestFlags(_pstText, orxTEXT_KU32_FLAG_FIXED_HEIGHT) != orxFALSE) && (fHeight > _pstText->fHeight))
            {
              /* Truncates the string */
              *pc = orxCHAR_NULL;
            }
            else
            {
              /* Resets width */
              fWidth = orxFLOAT_0;
            }

            break;
          }

          case ' ':
          case '\t':
          {
            /* Updates width */
            fWidth += orxFont_GetCharacterWidth(_pstText->pstFont, u32CharacterCodePoint);

            /* Updates last space */
            zLastSpace = pc - 1;

            break;
          }

          default:
          {
            /* Finds end of word */
            for(; (u32CharacterCodePoint != ' ') && (u32CharacterCodePoint != '\t') && (u32CharacterCodePoint != orxCHAR_CR) && (u32CharacterCodePoint != orxCHAR_LF) && (u32CharacterCodePoint != orxCHAR_NULL); u32CharacterCodePoint = orxString_GetFirstCharacterCodePoint(pc, (const orxCHAR **)&pc))
            {
              fWidth += orxFont_GetCharacterWidth(_pstText->pstFont, u32CharacterCodePoint);
            }

            /* Gets back to previous character */
            pc--;

            break;
          }
        }

        /* Doesn't fit inside the line? */
        if(fWidth > _pstText->fWidth)
        {
          /* No original string copy yet? */
          if(_pstText->zOriginalString == orxNULL)
          {
            /* Stores a copy of the original */
            _pstText->zOriginalString = orxString_Duplicate(_pstText->zString);
          }

          /* Has last space? */
          if(zLastSpace != orxNULL)
          {
            /* Inserts end of line */
            *zLastSpace = orxCHAR_LF;

            /* Updates cursor */
            pc = zLastSpace;

            /* Clears last space */
            zLastSpace = orxNULL;

            /* Updates width */
            fWidth = orxFLOAT_0;
          }
          else
          {
            orxCHAR cBackup, *pcDebug;

            /* Logs message */
            cBackup = *pc;
            *pc = orxCHAR_NULL;
            for(pcDebug = pc - 1; (pcDebug >= _pstText->zString) && (*pcDebug != ' ') && (*pcDebug != '\t') && (*pcDebug != orxCHAR_LF) && (*pcDebug != orxCHAR_CR); pcDebug--)
              ;
            orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "[%s] Word <%s> is too long to fit inside the requested <%g> pixels!", (_pstText->zReference != orxNULL) ? _pstText->zReference : orxSTRING_EMPTY, pcDebug + 1, _pstText->fWidth);
            *pc = cBackup;
          }
        }
      }

      /* Isn't height fixed? */
      if(orxStructure_TestFlags(_pstText, orxTEXT_KU32_FLAG_FIXED_HEIGHT) == orxFALSE)
      {
        /* Stores it */
        _pstText->fHeight = fHeight;
      }
    }
  }
  else
  {
    /* Isn't width fixed? */
    if(orxStructure_TestFlags(_pstText, orxTEXT_KU32_FLAG_FIXED_WIDTH) == orxFALSE)
    {
      /* Clears it */
      _pstText->fWidth = orxFLOAT_0;
    }

    /* Isn't height fixed? */
    if(orxStructure_TestFlags(_pstText, orxTEXT_KU32_FLAG_FIXED_HEIGHT) == orxFALSE)
    {
      /* Clears it */
      _pstText->fHeight = orxFLOAT_0;
    }
  }

  /* Done! */
  return;
}

/** Deletes all texts
 */
static orxINLINE void orxText_DeleteAll()
{
  orxTEXT *pstText;

  /* Gets first text */
  pstText = orxTEXT(orxStructure_GetFirst(orxSTRUCTURE_ID_TEXT));

  /* Non empty? */
  while(pstText != orxNULL)
  {
    /* Deletes text */
    orxText_Delete(pstText);

    /* Gets first text */
    pstText = orxTEXT(orxStructure_GetFirst(orxSTRUCTURE_ID_TEXT));
  }

  return;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Setups the text module
 */
void orxFASTCALL orxText_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_TEXT, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_TEXT, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_TEXT, orxMODULE_ID_EVENT);
  orxModule_AddDependency(orxMODULE_ID_TEXT, orxMODULE_ID_FONT);
  orxModule_AddDependency(orxMODULE_ID_TEXT, orxMODULE_ID_LOCALE);
  orxModule_AddDependency(orxMODULE_ID_TEXT, orxMODULE_ID_STRUCTURE);

  return;
}

/** Inits the text module
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxText_Init()
{
  orxSTATUS eResult;

  /* Not already Initialized? */
  if(!(sstText.u32Flags & orxTEXT_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstText, sizeof(orxTEXT_STATIC));

    /* Adds event handler */
    eResult = orxEvent_AddHandler(orxEVENT_TYPE_LOCALE, orxText_EventHandler);

    /* Valid? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Filters relevant event IDs */
      orxEvent_SetHandlerIDFlags(orxText_EventHandler, orxEVENT_TYPE_LOCALE, orxNULL, orxEVENT_GET_FLAG(orxLOCALE_EVENT_SELECT_LANGUAGE), orxEVENT_KU32_MASK_ID_ALL);

      /* Registers structure type */
      eResult = orxSTRUCTURE_REGISTER(TEXT, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxTEXT_KU32_BANK_SIZE, orxNULL);

      /* Success? */
      if(eResult != orxSTATUS_FAILURE)
      {
        /* Updates flags for screen text creation */
        sstText.u32Flags = orxTEXT_KU32_STATIC_FLAG_READY;

        /* Adds event handler for resources */
        orxEvent_AddHandler(orxEVENT_TYPE_RESOURCE, orxText_EventHandler);
        orxEvent_SetHandlerIDFlags(orxText_EventHandler, orxEVENT_TYPE_RESOURCE, orxNULL, orxEVENT_GET_FLAG(orxRESOURCE_EVENT_ADD) | orxEVENT_GET_FLAG(orxRESOURCE_EVENT_UPDATE), orxEVENT_KU32_MASK_ID_ALL);
      }
      else
      {
        /* Removes event handler */
        orxEvent_RemoveHandler(orxEVENT_TYPE_LOCALE, orxText_EventHandler);
      }
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Tried to initialize text module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Not initialized? */
  if(eResult == orxSTATUS_FAILURE)
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Initializing text module failed.");

    /* Updates Flags */
    sstText.u32Flags &= ~orxTEXT_KU32_STATIC_FLAG_READY;
  }

  /* Done! */
  return eResult;
}

/** Exits from the text module
 */
void orxFASTCALL orxText_Exit()
{
  /* Initialized? */
  if(sstText.u32Flags & orxTEXT_KU32_STATIC_FLAG_READY)
  {
    /* Deletes text list */
    orxText_DeleteAll();

    /* Removes event handlers */
    orxEvent_RemoveHandler(orxEVENT_TYPE_RESOURCE, orxText_EventHandler);
    orxEvent_RemoveHandler(orxEVENT_TYPE_LOCALE, orxText_EventHandler);

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_TEXT);

    /* Updates flags */
    sstText.u32Flags &= ~orxTEXT_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Tried to exit text module when it wasn't initialized.");
  }

  return;
}

/** Creates an empty text
 * @return      orxTEXT / orxNULL
 */
orxTEXT *orxFASTCALL orxText_Create()
{
  orxTEXT *pstResult;

  /* Checks */
  orxASSERT(sstText.u32Flags & orxTEXT_KU32_STATIC_FLAG_READY);

  /* Creates text */
  pstResult = orxTEXT(orxStructure_Create(orxSTRUCTURE_ID_TEXT));

  /* Created? */
  if(pstResult != orxNULL)
  {
    /* Inits it */
    pstResult->zString          = orxNULL;
    pstResult->pstFont          = orxNULL;
    pstResult->zOriginalString  = orxNULL;

    /* Inits flags */
    orxStructure_SetFlags(pstResult, orxTEXT_KU32_FLAG_NONE, orxTEXT_KU32_MASK_ALL);

    /* Increases count */
    orxStructure_IncreaseCount(pstResult);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Failed to create structure for text.");
  }

  /* Done! */
  return pstResult;
}

/** Creates a text from config
 * @param[in]   _zConfigID    Config ID
 * @return      orxTEXT / orxNULL
 */
orxTEXT *orxFASTCALL orxText_CreateFromConfig(const orxSTRING _zConfigID)
{
  orxTEXT *pstResult;

  /* Checks */
  orxASSERT(sstText.u32Flags & orxTEXT_KU32_STATIC_FLAG_READY);
  orxASSERT((_zConfigID != orxNULL) && (_zConfigID != orxSTRING_EMPTY));

  /* Pushes section */
  if((orxConfig_HasSection(_zConfigID) != orxFALSE)
  && (orxConfig_PushSection(_zConfigID) != orxSTATUS_FAILURE))
  {
    /* Creates text */
    pstResult = orxText_Create();

    /* Valid? */
    if(pstResult != orxNULL)
    {
      /* Stores its reference key */
      pstResult->zReference = orxConfig_GetCurrentSection();

      /* Processes its config data */
      if(orxText_ProcessConfigData(pstResult) == orxSTATUS_FAILURE)
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't process config data for text <%s>.", _zConfigID);

        /* Deletes it */
        orxText_Delete(pstResult);

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

  /* Done! */
  return pstResult;
}

/** Deletes a text
 * @param[in]   _pstText      Concerned text
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxText_Delete(orxTEXT *_pstText)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstText.u32Flags & orxTEXT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstText);

  /* Decreases count */
  orxStructure_DecreaseCount(_pstText);

  /* Not referenced? */
  if(orxStructure_GetRefCount(_pstText) == 0)
  {
    /* Removes string */
    orxText_SetString(_pstText, orxNULL);

    /* Removes font */
    orxText_SetFont(_pstText, orxNULL);

    /* Deletes structure */
    orxStructure_Delete(_pstText);
  }
  else
  {
    /* Referenced by others */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets text name
 * @param[in]   _pstText      Concerned text
 * @return      Text name / orxNULL
 */
const orxSTRING orxFASTCALL orxText_GetName(const orxTEXT *_pstText)
{
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(sstText.u32Flags & orxTEXT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstText);

  /* Updates result */
  zResult = (_pstText->zReference != orxNULL) ? _pstText->zReference : orxSTRING_EMPTY;

  /* Done! */
  return zResult;
}

/** Gets text's line count
 * @param[in]   _pstText      Concerned text
 * @return      orxU32
 */
orxU32 orxFASTCALL orxText_GetLineCount(const orxTEXT *_pstText)
{
  orxU32 u32Result;

  /* Checks */
  orxASSERT(sstText.u32Flags & orxTEXT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstText);

  /* Is not empty? */
  if((_pstText->zString != orxNULL) && (*(_pstText->zString) != orxCHAR_NULL))
  {
    const orxCHAR *pc;

    /* For all characters */
    for(pc = _pstText->zString, u32Result = 1; *pc != orxCHAR_NULL; pc++)
    {
      /* Depending on character */
      switch(*pc)
      {
        case orxCHAR_CR:
        {
          /* Half EOL? */
          if(*(pc + 1) == orxCHAR_LF)
          {
            /* Updates pointer */
            pc++;
          }

          /* Fall through */
        }

        case orxCHAR_LF:
        {
          /* Updates result */
          u32Result++;
        }

        default:
        {
          break;
        }
      }
    }
  }
  else
  {
    /* Updates result */
    u32Result = 0;
  }

  /* Done! */
  return u32Result;
}

/** Gets text's line size
 * @param[in]   _pstText      Concerned text
 * @param[out]  _u32Line      Line index
 * @param[out]  _pfWidth      Line's width
 * @param[out]  _pfHeight     Line's height
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxText_GetLineSize(const orxTEXT *_pstText, orxU32 _u32Line, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstText.u32Flags & orxTEXT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstText);
  orxASSERT(_pfWidth != orxNULL);
  orxASSERT(_pfHeight != orxNULL);

  /* Has font? */
  if(_pstText->pstFont != orxNULL)
  {
    /* Has text? */
    if(_pstText->zString != orxNULL)
    {
      const orxCHAR  *pc;
      orxU32          u32Line;

      /* Skips to requested line */
      for(pc = _pstText->zString, u32Line = 0; (u32Line < _u32Line) && (*pc != orxCHAR_NULL); pc++)
      {
        /* Depending on character */
        switch(*pc)
        {
          case orxCHAR_CR:
          {
            /* Half EOL? */
            if(*(pc + 1) == orxCHAR_LF)
            {
              /* Updates pointer */
              pc++;
            }

            /* Fall through */
          }

          case orxCHAR_LF:
          {
            /* Updates line count */
            u32Line++;
          }

          default:
          {
            break;
          }
        }
      }

      /* Valid? */
      if(*pc != orxCHAR_NULL)
      {
        orxU32    u32CharacterCodePoint;
        orxFLOAT  fWidth;

        /* For all characters in the line */
        for(u32CharacterCodePoint = orxString_GetFirstCharacterCodePoint(pc, &pc), fWidth = orxFLOAT_0;
            (u32CharacterCodePoint != orxCHAR_CR) && (u32CharacterCodePoint != orxCHAR_LF) && (u32CharacterCodePoint != orxCHAR_NULL);
            u32CharacterCodePoint = orxString_GetFirstCharacterCodePoint(pc, &pc))
        {
          /* Updates width */
          fWidth += orxFont_GetCharacterWidth(_pstText->pstFont, u32CharacterCodePoint);
        }

        /* Stores dimensions */
        *_pfWidth   = fWidth;
        *_pfHeight  = orxFont_GetCharacterHeight(_pstText->pstFont);

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "[%s:%u]: Couldn't get text line size, invalid line number.", (_pstText->zReference != orxNULL) ? _pstText->zReference : orxSTRING_EMPTY, _u32Line);
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "[%s:%u]: Couldn't get text line size as no string is set.", (_pstText->zReference != orxNULL) ? _pstText->zReference : orxSTRING_EMPTY, _u32Line);
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "[%s:%u]: Couldn't get text line size as no font is set.", (_pstText->zReference != orxNULL) ? _pstText->zReference : orxSTRING_EMPTY, _u32Line);
  }

  /* Done! */
  return eResult;
}

/** Is text's size fixed? (ie. manually constrained with orxText_SetSize())
 * @param[in]   _pstText      Concerned text
 * @return      orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxText_IsFixedSize(const orxTEXT *_pstText)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT(sstText.u32Flags & orxTEXT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstText);

  /* Updates result */
  bResult = orxStructure_TestFlags(_pstText, orxTEXT_KU32_FLAG_FIXED_WIDTH | orxTEXT_KU32_FLAG_FIXED_HEIGHT);

  /* Done! */
  return bResult;
}

/** Gets text size
 * @param[in]   _pstText      Concerned text
 * @param[out]  _pfWidth      Text's width
 * @param[out]  _pfHeight     Text's height
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxText_GetSize(const orxTEXT *_pstText, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstText.u32Flags & orxTEXT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstText);
  orxASSERT(_pfWidth != orxNULL);
  orxASSERT(_pfHeight != orxNULL);

  /* Updates result */
  *_pfWidth   = _pstText->fWidth;
  *_pfHeight  = _pstText->fHeight;

  /* Done! */
  return eResult;
}

/** Gets text string
 * @param[in]   _pstText      Concerned text
 * @return      Text string / orxSTRING_EMPTY
 */
const orxSTRING orxFASTCALL orxText_GetString(const orxTEXT *_pstText)
{
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(sstText.u32Flags & orxTEXT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstText);

  /* Has string? */
  if(_pstText->zString != orxNULL)
  {
    /* Updates result */
    zResult = _pstText->zString;
  }
  else
  {
    /* Updates result */
    zResult = orxSTRING_EMPTY;
  }

  /* Done! */
  return zResult;
}

/** Gets text font
 * @param[in]   _pstText      Concerned text
 * @return      Text font / orxNULL
 */
orxFONT *orxFASTCALL orxText_GetFont(const orxTEXT *_pstText)
{
  orxFONT *pstResult;

  /* Checks */
  orxASSERT(sstText.u32Flags & orxTEXT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstText);

  /* Updates result */
  pstResult = _pstText->pstFont;

  /* Done! */
  return pstResult;
}

/** Sets text's size, will lead to reformatting if text doesn't fit (pass width = -1.0f to restore text's original size, ie. unconstrained)
 * @param[in]   _pstText      Concerned text
 * @param[in]   _fWidth       Max width for the text, remove any size constraint if negative
 * @param[in]   _fHeight      Max height for the text, ignored if negative (ie. unconstrained height)
 * @param[in]   _pzExtra      Text that wouldn't fit inside the box if height is provided, orxSTRING_EMPTY if no extra, orxNULL to ignore
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxText_SetSize(orxTEXT *_pstText, orxFLOAT _fWidth, orxFLOAT _fHeight, const orxSTRING *_pzExtra)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstText.u32Flags & orxTEXT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstText);
  orxASSERT (_fWidth > orxFLOAT_0);

  /* Unconstrained? */
  if(_fWidth <= orxFLOAT_0)
  {
    /* Was constrained? */
    if(orxStructure_TestFlags(_pstText, orxTEXT_KU32_FLAG_FIXED_WIDTH) != orxFALSE)
    {
      /* Clears dimensions */
      _pstText->fWidth = _pstText->fHeight = orxFLOAT_0;

      /* Updates status */
      orxStructure_SetFlags(_pstText, orxTEXT_KU32_FLAG_NONE, orxTEXT_KU32_FLAG_FIXED_WIDTH | orxTEXT_KU32_FLAG_FIXED_HEIGHT);

      /* Updates size */
      orxText_UpdateSize(_pstText);
    }

    /* Asked for extra? */
    if(_pzExtra != orxNULL)
    {
      /* Updates it */
      *_pzExtra = orxSTRING_EMPTY;
    }
  }
  else
  {
    /* Stores dimensions */
    _pstText->fWidth  = _fWidth;
    _pstText->fHeight = (_fHeight > orxFLOAT_0) ? _fHeight : orxFLOAT_0;

    /* Updates status */
    orxStructure_SetFlags(_pstText, (_fHeight > orxFLOAT_0) ? orxTEXT_KU32_FLAG_FIXED_WIDTH | orxTEXT_KU32_FLAG_FIXED_HEIGHT : orxTEXT_KU32_FLAG_FIXED_WIDTH, orxTEXT_KU32_FLAG_FIXED_WIDTH | orxTEXT_KU32_FLAG_FIXED_HEIGHT);

    /* Updates size */
    orxText_UpdateSize(_pstText);

    /* Asked for extra? */
    if(_pzExtra != orxNULL)
    {
      /* Has original string? */
      if(_pstText->zOriginalString != orxNULL)
      {
        const orxCHAR *pcSrc, *pcDst;

        /* Finds end of new string */
        for(pcSrc = _pstText->zString, pcDst = _pstText->zOriginalString; *pcSrc != orxCHAR_NULL; pcSrc++, pcDst++)
          ;

        /* Updates extra */
        *_pzExtra = (*pcDst != orxCHAR_NULL) ? pcDst : orxSTRING_EMPTY;
      }
      else
      {
        /* Updates extra */
        *_pzExtra = orxSTRING_EMPTY;
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Sets text string
 * @param[in]   _pstText      Concerned text
 * @param[in]   _zString      String to contain
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxText_SetString(orxTEXT *_pstText, const orxSTRING _zString)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstText.u32Flags & orxTEXT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstText);

  /* Has current string? */
  if(_pstText->zString != orxNULL)
  {
    /* Deletes it */
    orxString_Delete(_pstText->zString);
    _pstText->zString = orxNULL;

    /* Has original? */
    if(_pstText->zOriginalString != orxNULL)
    {
      /* Deletes it */
      orxString_Delete(_pstText->zOriginalString);
      _pstText->zOriginalString = orxNULL;
    }
  }

  /* Has new string? */
  if((_zString != orxNULL) && (_zString != orxSTRING_EMPTY))
  {
    /* Stores a duplicate */
    _pstText->zString = orxString_Duplicate(_zString);
  }

  /* Updates text size */
  orxText_UpdateSize(_pstText);

  /* Done! */
  return eResult;
}

/** Sets text font
 * @param[in]   _pstText      Concerned text
 * @param[in]   _pstFont      Font / orxNULL to use default
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxText_SetFont(orxTEXT *_pstText, orxFONT *_pstFont)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstText.u32Flags & orxTEXT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstText);

  /* Different? */
  if(_pstText->pstFont != _pstFont)
  {
    /* Has current font? */
    if(_pstText->pstFont != orxNULL)
    {
      /* Updates structure reference count *indirectly*, as deletion needs to be handled for non-internal fonts */
      orxFont_Delete(_pstText->pstFont);

      /* Internally handled? */
      if(orxStructure_TestFlags(_pstText, orxTEXT_KU32_FLAG_INTERNAL) != orxFALSE)
      {
        /* Removes its owner */
        orxStructure_SetOwner(_pstText->pstFont, orxNULL);

        /* Deletes it */
        orxFont_Delete(_pstText->pstFont);

        /* Updates flags */
        orxStructure_SetFlags(_pstText, orxTEXT_KU32_FLAG_NONE, orxTEXT_KU32_FLAG_INTERNAL);
      }

      /* Cleans it */
      _pstText->pstFont = orxNULL;
    }

    /* Has new font? */
    if(_pstFont != orxNULL)
    {
      /* Stores it */
      _pstText->pstFont = _pstFont;

      /* Updates its reference count */
      orxStructure_IncreaseCount(_pstFont);
    }

    /* Updates text's size */
    orxText_UpdateSize(_pstText);
  }

  /* Done! */
  return eResult;
}
