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
 * @file orxDisplay.cpp
 * @date 18/10/2007
 * @author iarwain@orx-project.org
 *
 * SFML display plugin implementation
 *
 * @todo
 */


extern "C"
{
  #include "orxInclude.h"

  #include "core/orxClock.h"
  #include "core/orxConfig.h"
  #include "core/orxEvent.h"
  #include "core/orxSystem.h"
  #include "math/orxMath.h"
  #include "plugin/orxPluginUser.h"
  #include "memory/orxBank.h"
  #include "utils/orxHashTable.h"

  #include "display/orxDisplay.h"
}

#include <SFML/Graphics.hpp>


/** Module flags
 */
#define orxDISPLAY_KU32_STATIC_FLAG_NONE        0x00000000 /**< No flags */

#define orxDISPLAY_KU32_STATIC_FLAG_READY       0x00000001 /**< Ready flag */
#define orxDISPLAY_KU32_STATIC_FLAG_VSYNC       0x00000002 /**< Ready flag */

#define orxDISPLAY_KU32_STATIC_MASK_ALL         0xFFFFFFFF /**< All mask */

orxSTATIC orxCONST orxU32     su32ScreenWidth   = 1024;
orxSTATIC orxCONST orxU32     su32ScreenHeight  = 768;
orxSTATIC orxCONST orxU32     su32ScreenDepth   = 32;
orxSTATIC orxCONST orxBITMAP *spoScreen         = (orxCONST orxBITMAP *)-1;
orxSTATIC orxCONST orxU32     su32TextBankSize  = 16;
orxSTATIC orxCONST orxU32     su32FontTableSize = 4;


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Text structure
 */
typedef struct __orxDISPLAY_TEXT_t
{
  sf::String *poString;

} orxDISPLAY_TEXT;

/** Static structure
 */
typedef struct __orxDISPLAY_STATIC_t
{
  orxU32            u32Flags;
  orxU32            u32ScreenWidth, u32ScreenHeight;
  sf::RenderWindow *poRenderWindow;
  sf::Font         *poDefaultFont;
  sf::String       *poTestString;

  orxBANK          *pstTextBank;
  orxHASHTABLE     *pstFontTable;

} orxDISPLAY_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
orxSTATIC orxDISPLAY_STATIC sstDisplay;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Loads a font
 */
orxSTATIC orxINLINE sf::Font *orxDisplay_SFML_LoadFont(orxCONST orxSTRING _zFontName)
{
  orxU32    u32Key;
  sf::Font *poResult = orxNULL;

  /* Valid? */
  if((_zFontName != orxNULL) && (_zFontName != orxSTRING_EMPTY))
  {
    /* Gets font key */
    u32Key = orxString_ToCRC(_zFontName);

    /* Not already loaded? */
    if((poResult = (sf::Font *)orxHashTable_Get(sstDisplay.pstFontTable, u32Key)) == orxNULL)
    {
      /* Allocates it */
      poResult = new sf::Font;

      /* Tries to load it */
      if(poResult->LoadFromFile(_zFontName) != false)
      {
        /* Stores it */
        orxHashTable_Add(sstDisplay.pstFontTable, u32Key, poResult);
      }
      else
      {
        /* Deletes it */
        delete poResult;
        poResult = orxNULL;
      }
    }
  }

  /* Done! */
  return poResult;
}

/** Get SFML blend mode
 */
orxSTATIC orxINLINE sf::Blend::Mode orxDisplay_SFML_GetBlendMode(orxDISPLAY_BLEND_MODE _eBlendMode)
{
  sf::Blend::Mode eResult;

  /* Depending on blend mode */
  switch(_eBlendMode)
  {
    case orxDISPLAY_BLEND_MODE_ALPHA:
    {
      /* Updates result */
      eResult = sf::Blend::Alpha;

      break;
    }

    case orxDISPLAY_BLEND_MODE_MULTIPLY:
    {
      /* Updates result */
      eResult = sf::Blend::Multiply;

      break;
    }

    case orxDISPLAY_BLEND_MODE_ADD:
    {
      /* Updates result */
      eResult = sf::Blend::Add;

      break;
    }

    default:
    {
      /* Updates result */
      eResult = sf::Blend::None;

      break;
    }
  }

  /* Done! */
  return eResult;
}

/** Event handler
 */
orxFASTCALL orxSTATUS EventHandler(orxCONST orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Is a cursor set position? */
  if((_pstEvent->eType == orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseMoved)
  && (_pstEvent->eID == orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseMoved))
  {
    orxVECTOR *pvPosition;

    /* Gets position */
    pvPosition = (orxVECTOR *)(_pstEvent->pstPayload);

    /* Updates cursor position */
    sstDisplay.poRenderWindow->SetCursorPosition(orxF2S(pvPosition->fX), orxF2S(pvPosition->fY));

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  /* Is a cursor show/hide? */
  else if((_pstEvent->eType == orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseButtonPressed)
  && (_pstEvent->eID == orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseButtonPressed))
  {
    orxBOOL *pbShowCursor;

    /* Gets cursor status */
    pbShowCursor = (orxBOOL *)(_pstEvent->pstPayload);

    /* Updates cursor status */
    sstDisplay.poRenderWindow->ShowMouseCursor((*pbShowCursor != orxFALSE) ? true : false);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

orxFASTCALL orxVOID orxDisplay_SFML_EventUpdate(orxCONST orxCLOCK_INFO *_pstClockInfo, orxVOID *_pContext)
{
  sf::Event oEvent;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Handles all pending events */
  while(sstDisplay.poRenderWindow->GetEvent(oEvent))
  {
    /* Depending on type */
    switch(oEvent.Type)
    {
      /* Closing? */
      case sf::Event::Closed:
      {
        /* Sends system close event */
        orxEvent_SendSimple(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_CLOSE);

        break;
      }

      /* Gained focus? */
      case sf::Event::GainedFocus:
      {
        /* Sends system focus gained event */
        orxEvent_SendSimple(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_FOCUS_GAINED);

        break;
      }

      /* Lost focus? */
      case sf::Event::LostFocus:
      {
        /* Sends system focus lost event */
        orxEvent_SendSimple(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_FOCUS_LOST);

        break;
      }

      case sf::Event::MouseMoved:
      case sf::Event::MouseWheelMoved:
      {
        orxEVENT stEvent;

        /* Inits event */
        orxMemory_Zero(&stEvent, sizeof(orxEVENT));
        stEvent.eType       = (orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + oEvent.Type);
        stEvent.eID         = oEvent.Type;
        stEvent.pstPayload  = &oEvent;

        /* Sends reserved event */
        orxEvent_Send(&stEvent);
      }

      default:
      {
        break;
      }
    }
  }
}

extern "C" orxBITMAP *orxDisplay_SFML_GetScreen()
{
  return const_cast<orxBITMAP *>(spoScreen);
}

extern "C" orxSTATUS orxDisplay_SFML_GetTextSize(orxCONST orxSTRING _zString, orxCONST orxSTRING _zFont, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pfWidth != orxNULL);
  orxASSERT(_pfHeight != orxNULL);

  /* Valid? */
  if(_zString != orxNULL)
  {
    /* Empty string? */
    if(_zString == orxSTRING_EMPTY)
    {
      /* No size */
      *_pfWidth = *_pfHeight = orxFLOAT_0;
    }
    else
    {
      sf::Font     *poFont;
      sf::FloatRect stRect;

      /* Has specific font? */
      if((_zFont != orxNULL) && (_zFont != orxSTRING_EMPTY))
      {
        poFont = orxDisplay_SFML_LoadFont(_zFont);
      }
      else
      {
        poFont = sstDisplay.poDefaultFont;
      }

      /* Has font? */
      if(poFont != orxNULL)
      {
        /* Updates test string font */
        sstDisplay.poTestString->SetFont(*poFont);
      }

      /* Updates test string text */
      sstDisplay.poTestString->SetText(_zString);

      /* Gets its size */
      stRect = sstDisplay.poTestString->GetRect();

      /* Stores values */
      *_pfWidth   = stRect.Right - stRect.Left;
      *_pfHeight  = stRect.Bottom - stRect.Top;
    }

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxDisplay_SFML_DrawText(orxCONST orxBITMAP *_pstBitmap, orxCONST orxSTRING _zString, orxCONST orxSTRING _zFont, orxCONST orxBITMAP_TRANSFORM *_pstTransform, orxRGBA _stColor, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstBitmap == spoScreen) && "Can only draw on screen with this version!");

  /* Valid? */
  if(_zString != orxNULL)
  {
    /* Empty string? */
    if(_zString == orxSTRING_EMPTY)
    {
      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      orxDISPLAY_TEXT *pstText;

      /* Gets a new text from bank */
      pstText = (orxDISPLAY_TEXT *)orxBank_Allocate(sstDisplay.pstTextBank);

      /* Valid? */
      if(pstText != orxNULL)
      {
        sf::Font     *poFont;
        sf::Vector2f  vPosition;

        /* Has specific font? */
        if((_zFont != orxNULL) && (_zFont != orxSTRING_EMPTY))
        {
          poFont = orxDisplay_SFML_LoadFont(_zFont);
        }
        else
        {
          poFont = sstDisplay.poDefaultFont;
        }

        /* Has font? */
        if(poFont != orxNULL)
        {
          /* Allocates text */
          pstText->poString = new sf::String(_zString, *poFont);
        }
        else
        {
          /* Allocates text */
          pstText->poString = new sf::String(_zString);
        }

        /* Sets its color */
        pstText->poString->SetColor(sf::Color(orxRGBA_R(_stColor), orxRGBA_G(_stColor), orxRGBA_B(_stColor), orxRGBA_A(_stColor)));

        /* Sets its center */
        pstText->poString->SetCenter(_pstTransform->fSrcX, _pstTransform->fSrcY);

        /* Sets its rotation */
        pstText->poString->SetRotation(-orxMATH_KF_RAD_TO_DEG * _pstTransform->fRotation);

        /* Sets its scale */
        pstText->poString->SetScale(_pstTransform->fScaleX, _pstTransform->fScaleY);

        /* Sets its blend mode */
        pstText->poString->SetBlendMode(orxDisplay_SFML_GetBlendMode(_eBlendMode));

        /* Sets its position */
        vPosition.x = _pstTransform->fDstX;
        vPosition.y = _pstTransform->fDstY;
        pstText->poString->SetPosition(vPosition);

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
    }
  }

  /* Done! */
  return eResult;
}

extern "C" orxVOID orxDisplay_SFML_DeleteBitmap(orxBITMAP *_pstBitmap)
{
  sf::Sprite         *poSprite;
  orxCONST sf::Image *poImage;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Not screen? */
  if(_pstBitmap != spoScreen)
  {
    /* Gets sprite */
    poSprite = (sf::Sprite *)_pstBitmap;

    /* Has image? */
    if((poImage = poSprite->GetImage()) != orxNULL)
    {
      /* Deletes it */
      delete poImage;
    }

    /* Deletes sprite */
    delete poSprite;
  }

  return;
}

extern "C" orxBITMAP *orxDisplay_SFML_CreateBitmap(orxU32 _u32Width, orxU32 _u32Height)
{
  sf::Image  *poImage;
  sf::Sprite *poSprite;
  orxSTRING   zBackupSection;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Backups config section */
  zBackupSection = orxConfig_GetCurrentSection();

  /* Selects display section */
  orxConfig_SelectSection(orxDISPLAY_KZ_CONFIG_SECTION);

  /* Creates image */
  poImage = new sf::Image(_u32Width, _u32Height);

  /* Activates smoothing */
  poImage->SetSmooth(orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_SMOOTH) ? true : false);

  /* Creates sprite using the new image */
  poSprite = new sf::Sprite(*poImage);

  /* Restores config section */
  orxConfig_SelectSection(zBackupSection);

  /* Done! */
  return (orxBITMAP *)poSprite;
}

extern "C" orxSTATUS orxDisplay_SFML_ClearBitmap(orxBITMAP *_pstBitmap, orxRGBA _stColor)
{
  sf::Color oColor;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Gets color */
  oColor = sf::Color(orxRGBA_R(_stColor), orxRGBA_G(_stColor), orxRGBA_B(_stColor), orxRGBA_A(_stColor));

  /* Is not screen? */
  if(_pstBitmap != spoScreen)
  {
    sf::Image  *poImage;
    orxU32     *pu32Cursor, *pu32End, u32Color;

    /* Gets flat color */
    u32Color = orx2RGBA(oColor.r, oColor.g, oColor.b, oColor.a);

    /* Gets image */
    poImage = const_cast<sf::Image *>(((sf::Sprite *)_pstBitmap)->GetImage());

    /* For all pixels */
    for(pu32Cursor = (orxU32 *)poImage->GetPixelsPtr(), pu32End = pu32Cursor + (poImage->GetWidth() * poImage->GetHeight());
        pu32Cursor < pu32End; pu32Cursor++)
    {
      /* Updates pixel */
      *pu32Cursor = u32Color;
    }
  }
  else
  {
    /* Clear the color buffer with given color */
    glClearColor((1.0f / 255.f) * oColor.r, (1.0f / 255.f) * oColor.g, (1.0f / 255.f) * oColor.b, (1.0f / 255.f) * oColor.a);
    glClear(GL_COLOR_BUFFER_BIT);
  }

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxDisplay_SFML_Swap()
{
  orxDISPLAY_TEXT  *pstText;
  orxSTATUS         eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* For all texts */
  for(pstText = (orxDISPLAY_TEXT *)orxBank_GetNext(sstDisplay.pstTextBank, orxNULL);
      pstText != orxNULL;
      pstText = (orxDISPLAY_TEXT *)orxBank_GetNext(sstDisplay.pstTextBank, pstText))
  {
    /* Disables clipping */
    glDisable(GL_SCISSOR_TEST);

    /* Draws it */
    sstDisplay.poRenderWindow->Draw(*(pstText->poString));

    /* Deletes it */
    delete(pstText->poString);
  }

  /* Clears text bank */
  orxBank_Clear(sstDisplay.pstTextBank);

  /* Displays render window */
  sstDisplay.poRenderWindow->Display();

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxDisplay_SFML_SetBitmapColorKey(orxBITMAP *_pstBitmap, orxRGBA _stColor, orxBOOL _bEnable)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  sf::Image *poImage;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstBitmap != orxNULL) && (_pstBitmap != spoScreen));

  /* Gets image */
  poImage = const_cast<sf::Image *>(((sf::Sprite *)_pstBitmap)->GetImage());

  /* Enable? */
  if(_bEnable != orxFALSE)
  {
    /* Creates transparency mask */
    poImage->CreateMaskFromColor(sf::Color(orxRGBA_R(_stColor), orxRGBA_G(_stColor), orxRGBA_B(_stColor), 0xFF));
  }
  else
  {
    /* Clears transparency mask */
    poImage->CreateMaskFromColor(sf::Color(orxRGBA_R(_stColor), orxRGBA_G(_stColor), orxRGBA_B(_stColor), 0xFF), 0xFF);
  }

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxDisplay_SFML_SetBitmapColor(orxBITMAP *_pstBitmap, orxRGBA _stColor)
{
  sf::Sprite *poSprite;
  orxSTATUS   eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstBitmap != orxNULL) && (_pstBitmap != spoScreen));

  /* Gets sprite */
  poSprite = (sf::Sprite *)_pstBitmap;

  /* Sets sprite color */
  poSprite->SetColor(sf::Color(orxRGBA_R(_stColor), orxRGBA_G(_stColor), orxRGBA_B(_stColor), orxRGBA_A(_stColor)));

  /* Done! */
  return eResult;
}

extern "C" orxRGBA orxDisplay_SFML_GetBitmapColor(orxCONST orxBITMAP *_pstBitmap)
{
  sf::Sprite *poSprite;
  sf::Color   oColor;
  orxRGBA     stResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstBitmap != orxNULL) && (_pstBitmap != spoScreen));

  /* Gets image */
  poSprite = (sf::Sprite *)_pstBitmap;

  /* Gets its color */
  oColor = poSprite->GetColor();

  /* Updates result */
  stResult = orx2RGBA(oColor.r, oColor.g, oColor.b, oColor.a);

  /* Done! */
  return stResult;
}

extern "C" orxSTATUS orxDisplay_SFML_BlitBitmap(orxBITMAP *_pstDst, orxCONST orxBITMAP *_pstSrc, orxCONST orxFLOAT _fPosX, orxFLOAT _fPosY, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  sf::Sprite   *poSprite;
  sf::Vector2f  vPosition;
  orxSTATUS   eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstSrc != orxNULL) && (_pstSrc != spoScreen));
  orxASSERT((_pstDst == spoScreen) && "Can only draw on screen with this version!");

  /* Gets sprite */
  poSprite = (sf::Sprite *)_pstSrc;

  /* Updates its position */
  vPosition.x = _fPosX;
  vPosition.y = _fPosY;
  poSprite->SetPosition(vPosition);

  /* Updates sprite blend mode */
  poSprite->SetBlendMode(orxDisplay_SFML_GetBlendMode(_eBlendMode));

  /* Draws it */
  sstDisplay.poRenderWindow->Draw(*poSprite);

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxDisplay_SFML_TransformBitmap(orxBITMAP *_pstDst, orxCONST orxBITMAP *_pstSrc, orxCONST orxBITMAP_TRANSFORM *_pstTransform, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  sf::Sprite *poSprite;
  bool        bSmooth;
  orxSTATUS   eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstSrc != orxNULL) && (_pstSrc != spoScreen));
  orxASSERT((_pstDst == spoScreen) && "Can only draw on screen with this version!");
  orxASSERT(_pstTransform != orxNULL);

  /* Gets sprite */
  poSprite = (sf::Sprite *)_pstSrc;

  /* Updates its center */
  poSprite->SetCenter(_pstTransform->fSrcX, _pstTransform->fSrcY);

  /* Updates its rotation */
  poSprite->SetRotation(-orxMATH_KF_RAD_TO_DEG * _pstTransform->fRotation);

  /* Updates its flipping */
  if(_pstTransform->fScaleX < 0.0f)
  {
    poSprite->FlipX(true);
  }
  if(_pstTransform->fScaleY < 0.0f)
  {
    poSprite->FlipY(true);
  }

  /* Updates its scale */
  poSprite->SetScale(orxMath_Abs(_pstTransform->fScaleX), orxMath_Abs(_pstTransform->fScaleY));

  /* Depending on smoothing type */
  switch(_eSmoothing)
  {
    case orxDISPLAY_SMOOTHING_ON:
    {
      /* Applies smoothing */
      bSmooth = true;

      break;
    }

    case orxDISPLAY_SMOOTHING_OFF:
    {
      /* Applies no smoothing */
      bSmooth = false;

      break;
    }

    default:
    case orxDISPLAY_SMOOTHING_DEFAULT:
    {
      /* Selects display section */
      orxConfig_SelectSection(orxDISPLAY_KZ_CONFIG_SECTION);

      /* Applies default smoothing */
      bSmooth = orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_SMOOTH) ? true : false;

      break;
    }
  }

  /* Should update smoothing? */
  if(bSmooth != poSprite->GetImage()->IsSmooth())
  {
    /* Updates it */
    const_cast<sf::Image *>(poSprite->GetImage())->SetSmooth(bSmooth);
  }

  /* Blits it */
  eResult = orxDisplay_SFML_BlitBitmap(_pstDst, _pstSrc, _pstTransform->fDstX, _pstTransform->fDstY, _eBlendMode);

  /* Resets its center */
  poSprite->SetCenter(0.0f, 0.0f);

  /* Resets its rotation */
  poSprite->SetRotation(0.0f);

  /* Resets its flipping */
  poSprite->FlipX(false);
  poSprite->FlipY(false);

  /* Resets its scale */
  poSprite->SetScale(1.0f, 1.0f);

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxDisplay_SFML_SaveBitmap(orxCONST orxBITMAP *_pstBitmap, orxCONST orxSTRING _zFilename)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT(_zFilename != orxNULL);

  /* Not screen? */
  if(_pstBitmap != spoScreen)
  {
    sf::Image *poImage;

    /* Gets image */
    poImage = const_cast<sf::Image *>(((sf::Sprite *)_pstBitmap)->GetImage());

    /* Saves it */
    eResult = (poImage->SaveToFile(_zFilename) != false) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
  }
  else
  {
    /* Gets screen capture */
    eResult = (sstDisplay.poRenderWindow->Capture().SaveToFile(_zFilename) != false) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

extern "C" orxBITMAP *orxDisplay_SFML_LoadBitmap(orxCONST orxSTRING _zFilename)
{
  orxBITMAP *pstResult;
  sf::Image *poImage;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_zFilename != orxNULL);

  /* Creates empty image */
  poImage = new sf::Image();

  /* Loads it from file */
  if(poImage->LoadFromFile(_zFilename) != false)
  {
    sf::Sprite *poSprite;
    orxSTRING   zBackupSection;

    /* Backups config section */
    zBackupSection = orxConfig_GetCurrentSection();

    /* Selects display section */
    orxConfig_SelectSection(orxDISPLAY_KZ_CONFIG_SECTION);

    /* Activates smoothing */
    poImage->SetSmooth(orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_SMOOTH) ? true : false);

    /* Creates a sprite from it */
    poSprite = new sf::Sprite(*poImage);

    /* Restores config section */
    orxConfig_SelectSection(zBackupSection);

    /* Updates result */
    pstResult = (orxBITMAP *)poSprite;
  }
  else
  {
    /* Deletes image */
    delete poImage;

    /* Updates result */
    pstResult = (orxBITMAP *)orxNULL;
  }

  /* Done! */
  return pstResult;
}

extern "C" orxSTATUS orxDisplay_SFML_GetBitmapSize(orxCONST orxBITMAP *_pstBitmap, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT(_pfWidth != orxNULL);
  orxASSERT(_pfHeight != orxNULL);

  /* Not screen? */
  if(_pstBitmap != spoScreen)
  {
    sf::Image *poImage;

    /* Gets image */
    poImage = const_cast<sf::Image *>(((sf::Sprite *)_pstBitmap)->GetImage());

    /* Gets size info */
    *_pfWidth  = orxS2F(poImage->GetWidth());
    *_pfHeight = orxS2F(poImage->GetHeight());
  }
  else
  {
    /* Gets size info */
    *_pfWidth  = orxS2F(sstDisplay.poRenderWindow->GetWidth());
    *_pfHeight = orxS2F(sstDisplay.poRenderWindow->GetHeight());
  }

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxDisplay_SFML_GetScreenSize(orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
{
  orxU32    u32Width, u32Height;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pfWidth != orxNULL);
  orxASSERT(_pfHeight != orxNULL);

  /* Gets size info */
  u32Width  = sstDisplay.poRenderWindow->GetWidth();
  u32Height = sstDisplay.poRenderWindow->GetHeight();

  /* Updates results */
  *_pfWidth   = orxU2F(u32Width);
  *_pfHeight  = orxU2F(u32Height);

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxDisplay_SFML_SetBitmapClipping(orxBITMAP *_pstBitmap, orxU32 _u32TLX, orxU32 _u32TLY, orxU32 _u32BRX, orxU32 _u32BRY)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Screen? */
  if(_pstBitmap == spoScreen)
  {
    /* Stores screen clipping */
    glScissor(_u32TLX, sstDisplay.u32ScreenHeight - _u32BRY, _u32BRX - _u32TLX, _u32BRY - _u32TLY);

    /* Enables clipping */
    glEnable(GL_SCISSOR_TEST);
  }
  else
  {
    /* Sets sub rectangle for sprite */
    ((sf::Sprite *)_pstBitmap)->SetSubRect(sf::IntRect(_u32TLX, _u32TLY, _u32BRX, _u32BRY));
  }

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxDisplay_SFML_EnableVSync(orxBOOL _bEnable)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Enables? */
  if(_bEnable != orxFALSE)
  {
    /* Enables vertical sync */
    sstDisplay.poRenderWindow->UseVerticalSync(true);

    /* Updates status */
    orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_VSYNC, orxDISPLAY_KU32_STATIC_FLAG_NONE);
  }
  else
  {
    /* Disables vertical Sync */
    sstDisplay.poRenderWindow->UseVerticalSync(false);

    /* Updates status */
    orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NONE, orxDISPLAY_KU32_STATIC_FLAG_VSYNC);
  }

  /* Done! */
  return eResult;
}

extern "C" orxBOOL orxDisplay_SFML_IsVSyncEnabled()
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Updates result */
  bResult = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_VSYNC) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

extern "C" orxSTATUS orxDisplay_SFML_Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Was not already initialized? */
  if(!(sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstDisplay, sizeof(orxDISPLAY_STATIC));

    /* Registers our mouse event handler */
    if(orxEvent_AddHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseMoved), EventHandler) != orxSTATUS_FAILURE)
    {
      /* Registers our mouse wheell event handler */
      if(orxEvent_AddHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseButtonPressed), EventHandler) != orxSTATUS_FAILURE)
      {
        /* Creates font table */
        sstDisplay.pstFontTable = orxHashTable_Create(su32FontTableSize, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

        /* Valid? */
        if(sstDisplay.pstFontTable != orxNULL)
        {
          /* Creates text bank */
          sstDisplay.pstTextBank = orxBank_Create(su32TextBankSize, sizeof(orxDISPLAY_TEXT), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

          /* Valid? */
          if(sstDisplay.pstTextBank != orxNULL)
          {
            orxU32        u32ConfigWidth, u32ConfigHeight, u32ConfigDepth;
            orxCLOCK     *pstClock;
            unsigned long ulStyle;

            /* Creates test text */
            sstDisplay.poTestString = new sf::String(orxSTRING_EMPTY);

            /* Gets resolution from config */
            orxConfig_SelectSection(orxDISPLAY_KZ_CONFIG_SECTION);
            u32ConfigWidth  = orxConfig_GetU32(orxDISPLAY_KZ_CONFIG_WIDTH);
            u32ConfigHeight = orxConfig_GetU32(orxDISPLAY_KZ_CONFIG_HEIGHT);
            u32ConfigDepth  = orxConfig_GetU32(orxDISPLAY_KZ_CONFIG_DEPTH);

            /* Full screen? */
            if(orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_FULLSCREEN) != orxFALSE)
            {
              /* Updates flags */
              ulStyle = sf::Style::Fullscreen;
            }
            /* Decoration? */
            else if((orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_DECORATION) == orxFALSE)
            || (orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_DECORATION) != orxFALSE))
            {
              /* Updates flags */
              ulStyle = sf::Style::Close | sf::Style::Titlebar;
            }
            else
            {
              /* Updates flags */
              ulStyle = sf::Style::None;
            }

            /* Not valid? */
            if((sstDisplay.poRenderWindow = new sf::RenderWindow(sf::VideoMode(u32ConfigWidth, u32ConfigHeight, u32ConfigDepth), orxConfig_GetString(orxDISPLAY_KZ_CONFIG_TITLE), ulStyle)) == orxNULL)
            {
              /* Inits default rendering window */
              sstDisplay.poRenderWindow   = new sf::RenderWindow(sf::VideoMode(su32ScreenWidth, su32ScreenHeight, su32ScreenDepth), orxConfig_GetString(orxDISPLAY_KZ_CONFIG_TITLE), ulStyle);
            }

            /* Stores values */
            sstDisplay.u32ScreenWidth   = sstDisplay.poRenderWindow->GetWidth();
            sstDisplay.u32ScreenHeight  = sstDisplay.poRenderWindow->GetHeight();

            /* Has config font? */
            if(orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_FONT) != orxFALSE)
            {
              /* Loads it */
              sstDisplay.poDefaultFont = orxDisplay_SFML_LoadFont(orxConfig_GetString(orxDISPLAY_KZ_CONFIG_FONT));
            }

            /* Updates status */
            orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_READY, orxDISPLAY_KU32_STATIC_MASK_ALL);

            /* Gets clock */
            pstClock = orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE);

            /* Valid? */
            if(pstClock != orxNULL)
            {
              /* Registers event update function */
              eResult = orxClock_Register(pstClock, orxDisplay_SFML_EventUpdate, orxNULL, orxMODULE_ID_DISPLAY, orxCLOCK_PRIORITY_HIGH);
            }

            /* Has VSync value? */
            if(orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_VSYNC) != orxFALSE)
            {
              /* Updates vertical sync */
              orxDisplay_SFML_EnableVSync(orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_VSYNC));
            }
            else
            {
              /* Enables vertical sync */
              orxDisplay_SFML_EnableVSync(orxTRUE);
            }
          }
          else
          {
            /* Removes event handler */
            orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseMoved), EventHandler);
          }
        }
        else
        {
          /* Removes event handler */
          orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseMoved), EventHandler);
        }
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

extern "C" orxVOID orxDisplay_SFML_Exit()
{
  /* Was initialized? */
  if(sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY)
  {
    sf::Font *poFont;
    orxU32    u32Key;

    /* Unregisters event handlers */
    orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseMoved), EventHandler);
    orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseButtonPressed), EventHandler);

    /* For all fonts */
    while(orxHashTable_FindFirst(sstDisplay.pstFontTable, &u32Key, (orxVOID **)&poFont) != orxHANDLE_UNDEFINED)
    {
      /* Removes it */
      orxHashTable_Remove(sstDisplay.pstFontTable, u32Key);

      /* Deletes it */
      delete poFont;
    }

    /* Deletes text bank */
    orxBank_Delete(sstDisplay.pstTextBank);

    /* Deletes test string */
    delete sstDisplay.poTestString;

    /* Deletes rendering window */
    delete sstDisplay.poRenderWindow;

    /* Cleans static controller */
    orxMemory_Zero(&sstDisplay, sizeof(orxDISPLAY_STATIC));
  }

  return;
}

extern "C" orxHANDLE orxDisplay_SFML_GetApplicationInput()
{
  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  return((orxHANDLE)&(sstDisplay.poRenderWindow->GetInput()));
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(DISPLAY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_Init, DISPLAY, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_Exit, DISPLAY, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_Swap, DISPLAY, SWAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_CreateBitmap, DISPLAY, CREATE_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_DeleteBitmap, DISPLAY, DELETE_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_SaveBitmap, DISPLAY, SAVE_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_TransformBitmap, DISPLAY, TRANSFORM_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_LoadBitmap, DISPLAY, LOAD_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_GetBitmapSize, DISPLAY, GET_BITMAP_SIZE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_GetScreenSize, DISPLAY, GET_SCREEN_SIZE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_GetScreen, DISPLAY, GET_SCREEN_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_ClearBitmap, DISPLAY, CLEAR_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_SetBitmapClipping, DISPLAY, SET_BITMAP_CLIPPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_BlitBitmap, DISPLAY, BLIT_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_SetBitmapColorKey, DISPLAY, SET_BITMAP_COLOR_KEY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_SetBitmapColor, DISPLAY, SET_BITMAP_COLOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_GetBitmapColor, DISPLAY, GET_BITMAP_COLOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_DrawText, DISPLAY, DRAW_TEXT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_GetTextSize, DISPLAY, GET_TEXT_SIZE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_GetApplicationInput, DISPLAY, GET_APPLICATION_INPUT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_EnableVSync, DISPLAY, ENABLE_VSYNC);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_IsVSyncEnabled, DISPLAY, IS_VSYNC_ENABLED);
orxPLUGIN_USER_CORE_FUNCTION_END();
