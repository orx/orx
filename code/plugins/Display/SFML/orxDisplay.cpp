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
 */


#include "orxPluginAPI.h"

#include <SFML/Graphics.hpp>
#include "render/orxShader.h"


/** Module flags
 */
#define orxDISPLAY_KU32_STATIC_FLAG_NONE              0x00000000 /**< No flags */

#define orxDISPLAY_KU32_STATIC_FLAG_READY             0x00000001 /**< Ready flag */
#define orxDISPLAY_KU32_STATIC_FLAG_VSYNC             0x00000002 /**< VSync flag */
#define orxDISPLAY_KU32_STATIC_FLAG_FULLSCREEN        0x00000004 /**< FullScreen flag */
#define orxDISPLAY_KU32_STATIC_FLAG_SHOW_CURSOR       0x00000008 /**< Show cursor flag */

#define orxDISPLAY_KU32_STATIC_MASK_ALL               0xFFFFFFFF /**< All mask */

namespace orxDisplay
{
  static const orxU32     su32ScreenWidth         = 1024;
  static const orxU32     su32ScreenHeight        = 768;
  static const orxU32     su32ScreenDepth         = 32;
  static const orxBITMAP *spoScreen               = (const orxBITMAP *)-1;
  static const orxU32     su32TextBankSize        = 32;
  static const orxU32     su32InstantTextBankSize = 4;
  static const orxU32     su32FontTableSize       = 4;
  static const orxSTRING  szPlaceHolderString     = " "; /* Prevents a weird bug when creating a sf::String with SFML 1.4 on mac OS X */
}

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Text structure
 */
typedef struct __orxDISPLAY_TEXT_t
{
  sf::String *poString;
  orxSTRING   zFont;
  orxSTRING   zString;

} orxDISPLAY_TEXT;

/** Static structure
 */
typedef struct __orxDISPLAY_STATIC_t
{
  orxU32            u32Flags;
  orxU32            u32ScreenWidth, u32ScreenHeight, u32ScreenDepth;
  unsigned long     ulWindowStyle;
  orxBOOL           bDefaultSmooth;
  sf::RenderWindow *poRenderWindow;
  sf::Font         *poDefaultFont;
  orxSTRING         zDefaultFont;

  orxBANK          *pstTextBank;
  orxBANK          *pstInstantTextBank;
  orxHASHTABLE     *pstFontTable;

} orxDISPLAY_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxDISPLAY_STATIC sstDisplay;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Loads a font
 */
static orxINLINE sf::Font *orxDisplay_SFML_LoadFont(const orxSTRING _zFontName)
{
  orxU32    u32Key;
  sf::Font *poResult = (sf::Font *)orxNULL;

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
        poResult = (sf::Font *)orxNULL;
      }
    }
  }

  /* Done! */
  return poResult;
}

/** Get SFML blend mode
 */
static orxINLINE sf::Blend::Mode orxDisplay_SFML_GetBlendMode(orxDISPLAY_BLEND_MODE _eBlendMode)
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
static orxSTATUS orxFASTCALL orxDisplay_SFML_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Is a cursor set position? */
  if((_pstEvent->eType == orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseMoved)
  && (_pstEvent->eID == orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseMoved))
  {
    orxVECTOR *pvPosition;

    /* Gets position */
    pvPosition = (orxVECTOR *)(_pstEvent->pstPayload);

    /* Updates cursor position */
    sstDisplay.poRenderWindow->SetCursorPosition(orxF2S(pvPosition->fX), orxF2S(pvPosition->fY));
  }
  /* Is a cursor show/hide? */
  else if((_pstEvent->eType == orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseButtonPressed)
  && (_pstEvent->eID == orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseButtonPressed))
  {
    orxBOOL *pbShowCursor;

    /* Gets cursor status */
    pbShowCursor = (orxBOOL *)(_pstEvent->pstPayload);

    /* Show? */
    if(*pbShowCursor != orxFALSE)
    {
      /* Shows cursor */
      sstDisplay.poRenderWindow->ShowMouseCursor(true);

      /* Updates status */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHOW_CURSOR, orxDISPLAY_KU32_STATIC_FLAG_NONE);
    }
    else
    {
      /* Hides cursor */
      sstDisplay.poRenderWindow->ShowMouseCursor(false);

      /* Updates status */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NONE, orxDISPLAY_KU32_STATIC_FLAG_SHOW_CURSOR);
    }
  }

  /* Done! */
  return eResult;
}

static void orxFASTCALL orxDisplay_SFML_EventUpdate(const orxCLOCK_INFO *_pstClockInfo, void *_pContext)
{
  sf::Event oEvent;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Clears event */
  orxMemory_Zero(&oEvent, sizeof(sf::Event));

  /* Clears wheel event */
  orxEVENT_SEND(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseWheelMoved, sf::Event::MouseWheelMoved, orxNULL, orxNULL, &oEvent);

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
        orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_CLOSE);

        break;
      }

      /* Gained focus? */
      case sf::Event::GainedFocus:
      {
        /* Sends system focus gained event */
        orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_FOCUS_GAINED);

        break;
      }

      /* Lost focus? */
      case sf::Event::LostFocus:
      {
        /* Sends system focus lost event */
        orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_FOCUS_LOST);

        break;
      }

      /* Mouse in? */
      case sf::Event::MouseEntered:
      {
        /* Sends system mouse in event */
        orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_MOUSE_IN);
        
        break;
      }

      /* Mouse out? */
      case sf::Event::MouseLeft:
      {
        /* Sends system mouse out event */
        orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_MOUSE_OUT);
        
        break;
      }

      case sf::Event::MouseMoved:
      case sf::Event::MouseWheelMoved:
      case sf::Event::MouseButtonPressed:
      case sf::Event::MouseButtonReleased:
      case sf::Event::KeyPressed:
      case sf::Event::KeyReleased:
      case sf::Event::TextEntered:
      case sf::Event::JoyButtonPressed:
      case sf::Event::JoyButtonReleased:
      {
        /* Sends reserved event */
        orxEVENT_SEND(orxEVENT_TYPE_FIRST_RESERVED + oEvent.Type, oEvent.Type, orxNULL, orxNULL, &oEvent);

        break;
      }

      default:
      {
        break;
      }
    }
  }
}

extern "C" orxBITMAP *orxFASTCALL orxDisplay_SFML_GetScreen()
{
  return const_cast<orxBITMAP *>(orxDisplay::spoScreen);
}

extern "C" orxDISPLAY_TEXT *orxFASTCALL orxDisplay_SFML_CreateText()
{
  orxDISPLAY_TEXT *pstResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Allocates it */
  pstResult = (orxDISPLAY_TEXT *)orxBank_Allocate(sstDisplay.pstTextBank);

  /* Valid? */
  if(pstResult != orxNULL)
  {
    /* Has default font? */
    if(sstDisplay.poDefaultFont != orxNULL)
    {
      /* Allocates text */
      pstResult->poString = new sf::String(orxDisplay::szPlaceHolderString, *sstDisplay.poDefaultFont);
    }
    else
    {
      /* Allocates text */
      pstResult->poString = new sf::String(orxDisplay::szPlaceHolderString);
    }

    /* No string nor font */
    pstResult->zFont    = (orxSTRING)orxNULL;
    pstResult->zString  = (orxSTRING)orxNULL;
  }

  /* Done! */
  return pstResult;
}

extern "C" void orxFASTCALL orxDisplay_SFML_DeleteText(orxDISPLAY_TEXT *_pstText)
{
  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstText != orxNULL);

  /* Deletes its string */
  delete _pstText->poString;

  /* Frees it */
  orxBank_Free(sstDisplay.pstTextBank, _pstText);
}

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_TransformText(orxBITMAP *_pstDst, const orxDISPLAY_TEXT *_pstText, const orxDISPLAY_TRANSFORM *_pstTransform, orxRGBA _stColor, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstDst == orxDisplay::spoScreen) && "Can only draw on screen with this version!");
  orxASSERT(_pstText != orxNULL);
  orxASSERT(_pstTransform != orxNULL);

  /* Valid? */
  if(_pstText->zString != orxNULL)
  {
    /* Empty string? */
    if(_pstText->zString == orxSTRING_EMPTY)
    {
      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      sf::Vector2f vPosition;

      /* Sets its color */
      _pstText->poString->SetColor(sf::Color(orxRGBA_R(_stColor), orxRGBA_G(_stColor), orxRGBA_B(_stColor), orxRGBA_A(_stColor)));

      /* Sets its center */
      _pstText->poString->SetCenter(_pstTransform->fSrcX, _pstTransform->fSrcY);

      /* Sets its rotation */
      _pstText->poString->SetRotation(-orxMATH_KF_RAD_TO_DEG * _pstTransform->fRotation);

      /* Updates its scale */
      _pstText->poString->SetScale(orxMath_Abs(_pstTransform->fScaleX), orxMath_Abs(_pstTransform->fScaleY));

      /* Sets its blend mode */
      _pstText->poString->SetBlendMode(orxDisplay_SFML_GetBlendMode(_eBlendMode));

      /* Sets its position */
      vPosition.x = _pstTransform->fDstX;
      vPosition.y = _pstTransform->fDstY;
      _pstText->poString->SetPosition(vPosition);

      /* Draws it */
      sstDisplay.poRenderWindow->Draw(*(_pstText->poString));

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
  }

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_SetTextString(orxDISPLAY_TEXT *_pstText, const orxSTRING _zString)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstText != orxNULL);

  /* Valid? */
  if((_zString != orxNULL) && (_zString != orxSTRING_EMPTY))
  {
    /* Updates string */
    _pstText->poString->SetText(_zString);
  }

  /* Stores it */
  _pstText->zString = _zString;

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_SetTextFont(orxDISPLAY_TEXT *_pstText, const orxSTRING _zFont)
{
  sf::Font *poFont;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstText != orxNULL);

  /* Loads font */
  poFont = orxDisplay_SFML_LoadFont(_zFont);

  /* Valid? */
  if(poFont != orxNULL)
  {
    /* Updates string */
    _pstText->poString->SetFont(*poFont);

    /* Stores its name */
    _pstText->zFont = _zFont;
  }

  /* Done! */
  return eResult;
}

extern "C" orxSTRING orxFASTCALL orxDisplay_SFML_GetTextString(const orxDISPLAY_TEXT *_pstText)
{
  orxSTRING zResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstText != orxNULL);

  /* Updates result */
  zResult = _pstText->zString;

  /* Done! */
  return zResult;
}

extern "C" orxSTRING orxFASTCALL orxDisplay_SFML_GetTextFont(const orxDISPLAY_TEXT *_pstText)
{
  orxSTRING zResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstText != orxNULL);

  /* Updates result */
  zResult = _pstText->zFont;

  /* Done! */
  return zResult;
}

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_GetTextSize(const orxDISPLAY_TEXT *_pstText, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
{
  orxSTATUS     eResult = orxSTATUS_SUCCESS;
  sf::FloatRect stRect;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstText != orxNULL);
  orxASSERT(_pfWidth != orxNULL);
  orxASSERT(_pfHeight != orxNULL);

  /* Gets string rectangle */
  stRect = _pstText->poString->GetRect();

  /* Stores values */
  *_pfWidth   = stRect.Right - stRect.Left;
  *_pfHeight  = stRect.Bottom - stRect.Top;

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_PrintString(const orxBITMAP *_pstBitmap, const orxSTRING _zString, const orxDISPLAY_TRANSFORM *_pstTransform, orxRGBA _stColor)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstBitmap == orxDisplay::spoScreen) && "Can only draw on screen with this version!");
  orxASSERT(_pstTransform != orxNULL);

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
      pstText = (orxDISPLAY_TEXT *)orxBank_Allocate(sstDisplay.pstInstantTextBank);

      /* Valid? */
      if(pstText != orxNULL)
      {
        sf::Vector2f vPosition;

        /* Has default font? */
        if(sstDisplay.poDefaultFont != orxNULL)
        {
          /* Allocates text */
          pstText->poString = new sf::String(_zString, *sstDisplay.poDefaultFont);
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

        /* Updates its scale */
        pstText->poString->SetScale(orxMath_Abs(_pstTransform->fScaleX), orxMath_Abs(_pstTransform->fScaleY));

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

extern "C" void orxFASTCALL orxDisplay_SFML_DeleteBitmap(orxBITMAP *_pstBitmap)
{
  sf::Sprite       *poSprite;
  const sf::Image  *poImage;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Not screen? */
  if(_pstBitmap != orxDisplay::spoScreen)
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

extern "C" orxBITMAP *orxFASTCALL orxDisplay_SFML_CreateBitmap(orxU32 _u32Width, orxU32 _u32Height)
{
  sf::Image  *poImage;
  sf::Sprite *poSprite;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Pushes display section */
  orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

  /* Creates image */
  poImage = new sf::Image(_u32Width, _u32Height);

  /* Activates smoothing */
  poImage->SetSmooth(orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_SMOOTH) ? true : false);

  /* Creates sprite using the new image */
  poSprite = new sf::Sprite(*poImage);

  /* Pops config section */
  orxConfig_PopSection();

  /* Done! */
  return (orxBITMAP *)poSprite;
}

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_ClearBitmap(orxBITMAP *_pstBitmap, orxRGBA _stColor)
{
  sf::Color oColor;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Gets color */
  oColor = sf::Color(orxRGBA_R(_stColor), orxRGBA_G(_stColor), orxRGBA_B(_stColor), orxRGBA_A(_stColor));

  /* Is not screen? */
  if(_pstBitmap != orxDisplay::spoScreen)
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

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_Swap()
{
  orxDISPLAY_TEXT  *pstText;
  orxSTATUS         eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* For all texts */
  for(pstText = (orxDISPLAY_TEXT *)orxBank_GetNext(sstDisplay.pstInstantTextBank, orxNULL);
      pstText != orxNULL;
      pstText = (orxDISPLAY_TEXT *)orxBank_GetNext(sstDisplay.pstInstantTextBank, pstText))
  {
    /* Disables clipping */
    glDisable(GL_SCISSOR_TEST);

    /* Draws it */
    sstDisplay.poRenderWindow->Draw(*(pstText->poString));

    /* Deletes it */
    delete(pstText->poString);
  }

  /* Clears text bank */
  orxBank_Clear(sstDisplay.pstInstantTextBank);

  /* Displays render window */
  sstDisplay.poRenderWindow->Display();

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_SetBitmapColorKey(orxBITMAP *_pstBitmap, orxRGBA _stColor, orxBOOL _bEnable)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  sf::Image *poImage;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstBitmap != orxNULL) && (_pstBitmap != orxDisplay::spoScreen));

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

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_SetBitmapColor(orxBITMAP *_pstBitmap, orxRGBA _stColor)
{
  sf::Sprite *poSprite;
  orxSTATUS   eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstBitmap != orxNULL) && (_pstBitmap != orxDisplay::spoScreen));

  /* Gets sprite */
  poSprite = (sf::Sprite *)_pstBitmap;

  /* Sets sprite color */
  poSprite->SetColor(sf::Color(orxRGBA_R(_stColor), orxRGBA_G(_stColor), orxRGBA_B(_stColor), orxRGBA_A(_stColor)));

  /* Done! */
  return eResult;
}

extern "C" orxRGBA orxFASTCALL orxDisplay_SFML_GetBitmapColor(const orxBITMAP *_pstBitmap)
{
  sf::Sprite *poSprite;
  sf::Color   oColor;
  orxRGBA     stResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstBitmap != orxNULL) && (_pstBitmap != orxDisplay::spoScreen));

  /* Gets image */
  poSprite = (sf::Sprite *)_pstBitmap;

  /* Gets its color */
  oColor = poSprite->GetColor();

  /* Updates result */
  stResult = orx2RGBA(oColor.r, oColor.g, oColor.b, oColor.a);

  /* Done! */
  return stResult;
}

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_BlitBitmap(orxBITMAP *_pstDst, const orxBITMAP *_pstSrc, const orxFLOAT _fPosX, orxFLOAT _fPosY, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  sf::Sprite   *poSprite;
  sf::Vector2f  vPosition;
  orxSTATUS   eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstSrc != orxNULL) && (_pstSrc != orxDisplay::spoScreen));
  orxASSERT((_pstDst == orxDisplay::spoScreen) && "Can only draw on screen with this version!");

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

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_TransformBitmap(orxBITMAP *_pstDst, const orxBITMAP *_pstSrc, const orxDISPLAY_TRANSFORM *_pstTransform, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  sf::Sprite *poSprite;
  bool        bSmooth;
  orxSTATUS   eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstSrc != orxNULL) && (_pstSrc != orxDisplay::spoScreen));
  orxASSERT((_pstDst == orxDisplay::spoScreen) && "Can only draw on screen with this version!");
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
      /* Applies default smoothing */
      bSmooth = (sstDisplay.bDefaultSmooth != orxFALSE) ? true : false;

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

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_SaveBitmap(const orxBITMAP *_pstBitmap, const orxSTRING _zFilename)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT(_zFilename != orxNULL);

  /* Not screen? */
  if(_pstBitmap != orxDisplay::spoScreen)
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

extern "C" orxBITMAP *orxFASTCALL orxDisplay_SFML_LoadBitmap(const orxSTRING _zFilename)
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

    /* Pushes display section */
    orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

    /* Activates smoothing */
    poImage->SetSmooth(orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_SMOOTH) ? true : false);

    /* Creates a sprite from it */
    poSprite = new sf::Sprite(*poImage);

    /* Pops config section */
    orxConfig_PopSection();

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

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_GetBitmapSize(const orxBITMAP *_pstBitmap, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT(_pfWidth != orxNULL);
  orxASSERT(_pfHeight != orxNULL);

  /* Not screen? */
  if(_pstBitmap != orxDisplay::spoScreen)
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

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_GetScreenSize(orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
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

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_SetBitmapClipping(orxBITMAP *_pstBitmap, orxU32 _u32TLX, orxU32 _u32TLY, orxU32 _u32BRX, orxU32 _u32BRY)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Screen? */
  if(_pstBitmap == orxDisplay::spoScreen)
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

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_EnableVSync(orxBOOL _bEnable)
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

extern "C" orxBOOL orxFASTCALL orxDisplay_SFML_IsVSyncEnabled()
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Updates result */
  bResult = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_VSYNC) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_SetFullScreen(orxBOOL _bFullScreen)
{
  orxBOOL   bUpdate = orxFALSE;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Enables? */
  if(_bFullScreen != orxFALSE)
  {
    /* Wasn't already full screen? */
    if(!orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_FULLSCREEN))
    {
      /* Updates window style */
      sstDisplay.ulWindowStyle |= sf::Style::Fullscreen;

      /* Updates status */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_FULLSCREEN, orxDISPLAY_KU32_STATIC_FLAG_NONE);

      /* Asks for update */
      bUpdate = orxTRUE;
    }
  }
  else
  {
    /* Was full screen? */
    if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_FULLSCREEN))
    {
      /* Updates window style */
      sstDisplay.ulWindowStyle &= ~sf::Style::Fullscreen;

      /* Updates status */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NONE, orxDISPLAY_KU32_STATIC_FLAG_FULLSCREEN);

      /* Asks for update */
      bUpdate = orxTRUE;
    }
  }

  /* Should update? */
  if(bUpdate != orxFALSE)
  {
    /* Pushes display section */
    orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

    /* Creates new window */
    sstDisplay.poRenderWindow->Create(sf::VideoMode(sstDisplay.u32ScreenWidth, sstDisplay.u32ScreenHeight, sstDisplay.u32ScreenDepth), orxConfig_GetString(orxDISPLAY_KZ_CONFIG_TITLE), sstDisplay.ulWindowStyle);

    /* Clears rendering window */
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    /* Enforces mouse cursor status */
    sstDisplay.poRenderWindow->ShowMouseCursor(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHOW_CURSOR) ? true : false);

    /* Enforces VSync status */
    sstDisplay.poRenderWindow->UseVerticalSync(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_VSYNC) ? true : false);

    /* Pops config section */
    orxConfig_PopSection();
  }

  /* Done! */
  return eResult;
}

extern "C" orxBOOL orxFASTCALL orxDisplay_SFML_IsFullScreen()
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Updates result */
  bResult = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_FULLSCREEN) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

extern "C" orxU32 orxFASTCALL orxDisplay_SFML_GetVideoModeCounter()
{
  orxU32 u32Result;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Updates result */
  u32Result = (orxU32)sf::VideoMode::GetModesCount();

  /* Done! */
  return u32Result;
}

extern "C" orxDISPLAY_VIDEO_MODE *orxFASTCALL orxDisplay_SFML_GetVideoMode(orxU32 _u32Index, orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  orxDISPLAY_VIDEO_MODE *pstResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstVideoMode != orxNULL);

  /* Is index valid? */
  if(_u32Index < orxDisplay_SFML_GetVideoModeCounter())
  {
    /* Gets video mode */
    sf::VideoMode roVideoMode = sf::VideoMode::GetMode(_u32Index);

    /* Stores info */
    _pstVideoMode->u32Width   = roVideoMode.Width;
    _pstVideoMode->u32Height  = roVideoMode.Height;
    _pstVideoMode->u32Depth   = roVideoMode.BitsPerPixel;

    /* Updates result */
    pstResult = _pstVideoMode;
  }
  else
  {
    /* Updates result */
    pstResult = orxNULL;
  }

  /* Done! */
  return pstResult;
}

extern "C" orxBOOL orxFASTCALL orxDisplay_SFML_IsVideoModeAvailable(const orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  sf::VideoMode oVideoMode;
  orxBOOL       bResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstVideoMode != orxNULL);

  /* Stores info */
  oVideoMode.Width        = _pstVideoMode->u32Width;
  oVideoMode.Height       = _pstVideoMode->u32Height;
  oVideoMode.BitsPerPixel = _pstVideoMode->u32Depth;

  /* Updates result */
  bResult = oVideoMode.IsValid() ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_SetVideoMode(const orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Is video mode available? */
  if((_pstVideoMode == orxNULL) || (orxDisplay_SFML_IsVideoModeAvailable(_pstVideoMode) != orxFALSE))
  {
    /* Updates local info */
    if(_pstVideoMode != orxNULL)
    {
      sstDisplay.u32ScreenWidth   = _pstVideoMode->u32Width;
      sstDisplay.u32ScreenHeight  = _pstVideoMode->u32Height;
      sstDisplay.u32ScreenDepth   = _pstVideoMode->u32Depth;
    }

    /* Pushes display section */
    orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

    /* Full screen? */
    if(orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_FULLSCREEN) != orxFALSE)
    {
      /* Updates flags */
      sstDisplay.ulWindowStyle = sf::Style::Fullscreen;
    }
    /* Decoration? */
    else if((orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_DECORATION) == orxFALSE)
    || (orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_DECORATION) != orxFALSE))
    {
      /* Updates flags */
      sstDisplay.ulWindowStyle = sf::Style::Close | sf::Style::Titlebar;
    }
    else
    {
      /* Updates flags */
      sstDisplay.ulWindowStyle = sf::Style::None;
    }

    /* Creates new window */
    sstDisplay.poRenderWindow->Create(sf::VideoMode(sstDisplay.u32ScreenWidth, sstDisplay.u32ScreenHeight, sstDisplay.u32ScreenDepth), orxConfig_GetString(orxDISPLAY_KZ_CONFIG_TITLE), sstDisplay.ulWindowStyle);

    /* Enforces mouse cursor status */
    sstDisplay.poRenderWindow->ShowMouseCursor(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHOW_CURSOR) ? true : false);

    /* Enforces VSync status */
    sstDisplay.poRenderWindow->UseVerticalSync(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_VSYNC) ? true : false);

    /* Pops config section */
    orxConfig_PopSection();

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

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Was not already initialized? */
  if(!(sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstDisplay, sizeof(orxDISPLAY_STATIC));

    /* Registers our mouse event handler */
    if(orxEvent_AddHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseMoved), orxDisplay_SFML_EventHandler) != orxSTATUS_FAILURE)
    {
      /* Registers our mouse wheell event handler */
      if(orxEvent_AddHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseButtonPressed), orxDisplay_SFML_EventHandler) != orxSTATUS_FAILURE)
      {
        /* Creates font table */
        sstDisplay.pstFontTable = orxHashTable_Create(orxDisplay::su32FontTableSize, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

        /* Valid? */
        if(sstDisplay.pstFontTable != orxNULL)
        {
          /* Creates text banks */
          sstDisplay.pstTextBank        = orxBank_Create(orxDisplay::su32TextBankSize, sizeof(orxDISPLAY_TEXT), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
          sstDisplay.pstInstantTextBank = orxBank_Create(orxDisplay::su32InstantTextBankSize, sizeof(orxDISPLAY_TEXT), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

          /* Valid? */
          if((sstDisplay.pstTextBank != orxNULL) && (sstDisplay.pstInstantTextBank != orxNULL))
          {
            orxU32        u32ConfigWidth, u32ConfigHeight, u32ConfigDepth, u32Flags = orxDISPLAY_KU32_STATIC_FLAG_READY;
            orxCLOCK     *pstClock;

            /* Gets resolution from config */
            orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);
            u32ConfigWidth  = orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_WIDTH) ? orxConfig_GetU32(orxDISPLAY_KZ_CONFIG_WIDTH) : orxDisplay::su32ScreenWidth;
            u32ConfigHeight = orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_HEIGHT) ? orxConfig_GetU32(orxDISPLAY_KZ_CONFIG_HEIGHT) : orxDisplay::su32ScreenHeight;
            u32ConfigDepth  = orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_DEPTH) ? orxConfig_GetU32(orxDISPLAY_KZ_CONFIG_DEPTH) : orxDisplay::su32ScreenDepth;

            /* Gets default smoothing */
            sstDisplay.bDefaultSmooth = orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_SMOOTH);

            /* Full screen? */
            if(orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_FULLSCREEN) != orxFALSE)
            {
              /* Updates flags */
              sstDisplay.ulWindowStyle  = sf::Style::Fullscreen;
              u32Flags                 |= orxDISPLAY_KU32_STATIC_FLAG_FULLSCREEN;
            }
            /* Decoration? */
            else if((orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_DECORATION) == orxFALSE)
            || (orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_DECORATION) != orxFALSE))
            {
              /* Updates flags */
              sstDisplay.ulWindowStyle = sf::Style::Close | sf::Style::Titlebar;
            }
            else
            {
              /* Updates flags */
              sstDisplay.ulWindowStyle = sf::Style::None;
            }

            /* Not valid? */
            if((sstDisplay.poRenderWindow = new sf::RenderWindow(sf::VideoMode(u32ConfigWidth, u32ConfigHeight, u32ConfigDepth), orxConfig_GetString(orxDISPLAY_KZ_CONFIG_TITLE), sstDisplay.ulWindowStyle)) == orxNULL)
            {
              /* Inits default rendering window */
              sstDisplay.poRenderWindow = new sf::RenderWindow(sf::VideoMode(orxDisplay::su32ScreenWidth, orxDisplay::su32ScreenHeight, orxDisplay::su32ScreenDepth), orxConfig_GetString(orxDISPLAY_KZ_CONFIG_TITLE), sstDisplay.ulWindowStyle);

              /* Stores depth */
              sstDisplay.u32ScreenDepth = orxDisplay::su32ScreenDepth;
            }
            else
            {
              /* Stores depth */
              sstDisplay.u32ScreenDepth = u32ConfigDepth;
            }

            /* Clears rendering window */
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            /* Shows cursor */
            sstDisplay.poRenderWindow->ShowMouseCursor(true);

            /* Updates status */
            orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHOW_CURSOR, orxDISPLAY_KU32_STATIC_FLAG_NONE);

            /* Stores values */
            sstDisplay.u32ScreenWidth   = sstDisplay.poRenderWindow->GetWidth();
            sstDisplay.u32ScreenHeight  = sstDisplay.poRenderWindow->GetHeight();

            /* Has config font? */
            if(orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_FONT) != orxFALSE)
            {
              /* Loads it */
              sstDisplay.zDefaultFont   = orxConfig_GetString(orxDISPLAY_KZ_CONFIG_FONT);
              sstDisplay.poDefaultFont  = orxDisplay_SFML_LoadFont(sstDisplay.zDefaultFont);
            }
#ifdef __orxWINDOWS__
            else
            {
              //! TEMP: Prevents a crash when exiting SFML 1.4 on windows with no default font
              sstDisplay.zDefaultFont   = "c:/windows/fonts/arial.ttf";
              sstDisplay.poDefaultFont  = orxDisplay_SFML_LoadFont(sstDisplay.zDefaultFont);
            }
#endif /* __orxWINDOWS__ */

            /* Updates status */
            orxFLAG_SET(sstDisplay.u32Flags, u32Flags, orxDISPLAY_KU32_STATIC_MASK_ALL);

            /* Gets clock */
            pstClock = orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE);

            /* Valid? */
            if(pstClock != orxNULL)
            {
              /* Registers event update function */
              eResult = orxClock_Register(pstClock, orxDisplay_SFML_EventUpdate, orxNULL, orxMODULE_ID_DISPLAY, orxCLOCK_PRIORITY_HIGHEST);
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

            /* Pops config section */
            orxConfig_PopSection();
          }
          else
          {
            /* Removes event handler */
            orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseMoved), orxDisplay_SFML_EventHandler);
          }
        }
        else
        {
          /* Removes event handler */
          orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseMoved), orxDisplay_SFML_EventHandler);
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

extern "C" void orxFASTCALL orxDisplay_SFML_Exit()
{
  /* Was initialized? */
  if(sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY)
  {
    sf::Font *poFont;
    orxU32    u32Key;

    /* Unregisters event handlers */
    orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseMoved), orxDisplay_SFML_EventHandler);
    orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseButtonPressed), orxDisplay_SFML_EventHandler);

    /* For all fonts */
    while(orxHashTable_FindFirst(sstDisplay.pstFontTable, &u32Key, (void **)&poFont) != orxHANDLE_UNDEFINED)
    {
      /* Removes it */
      orxHashTable_Remove(sstDisplay.pstFontTable, u32Key);

      /* Deletes it */
      delete poFont;
    }

    /* Deletes text banks */
    orxBank_Delete(sstDisplay.pstTextBank);
    orxBank_Delete(sstDisplay.pstInstantTextBank);

    /* Deletes rendering window */
    delete sstDisplay.poRenderWindow;

    /* Cleans static controller */
    orxMemory_Zero(&sstDisplay, sizeof(orxDISPLAY_STATIC));
  }

  return;
}

extern "C" orxHANDLE orxFASTCALL orxDisplay_SFML_CreateShader(const orxSTRING _zCode, const orxLINKLIST *_pstParamList)
{
  orxHANDLE hResult = orxHANDLE_UNDEFINED;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Supports post FX? */
  if(sf::PostFX::CanUsePostFX() != false)
  {
    /* Valid? */
    if((_zCode != orxNULL) && (_zCode != orxSTRING_EMPTY))
    {
      sf::PostFX *poFX;

      /* Creates new post FX */
      poFX = new sf::PostFX();

      /* Valid? */
      if(poFX)
      {
        orxSHADER_PARAM  *pstParam;
        orxCHAR           acBuffer[32768], *pc;
        orxS32            s32Free;

        /* Inits buffer */
        acBuffer[0] = acBuffer[32767] = orxCHAR_NULL;
        pc          = acBuffer;
        s32Free     = 32767;

        /* For all parameters */
        for(pstParam = (orxSHADER_PARAM *)orxLinkList_GetFirst(_pstParamList);
            pstParam != orxNULL;
            pstParam = (orxSHADER_PARAM *)orxLinkList_GetNext(&(pstParam->stNode)))
        {
          /* Depending on type */
          switch(pstParam->eType)
          {
            case orxSHADER_PARAM_TYPE_FLOAT:
            {
              orxS32 s32Offset;

              /* Adds its literal value */
              s32Offset = orxString_NPrint(pc, s32Free, "float %s\n", pstParam->zName);
              pc       += s32Offset;
              s32Free  -= s32Offset;

              break;
            }

            case orxSHADER_PARAM_TYPE_TEXTURE:
            {
              orxS32 s32Offset;

              /* Adds its literal value */
              s32Offset = orxString_NPrint(pc, s32Free, "texture %s\n", pstParam->zName);
              pc       += s32Offset;
              s32Free  -= s32Offset;

              break;
            }

            case orxSHADER_PARAM_TYPE_VECTOR:
            {
              orxS32 s32Offset;

              /* Adds its literal value */
              s32Offset = orxString_NPrint(pc, s32Free, "vec3 %s\n", pstParam->zName);
              pc       += s32Offset;
              s32Free  -= s32Offset;

              break;
            }

            default:
            {
              break;
            }
          }
        }

        /* Adds code */
        orxString_NPrint(pc, s32Free, "effect\n%s\n", _zCode);

        /* Compiles code */
        if(poFX->LoadFromMemory(acBuffer) != false)
        {
          /* Updates result */
          hResult = (orxHANDLE)poFX;
        }
        else
        {
          orxLOG("Failed to compile shader code: %s", acBuffer);

          /* Deletes post FX */
          delete poFX;
        }
      }
    }
  }

  /* Done! */
  return hResult;
}

extern "C" void orxFASTCALL orxDisplay_SFML_DeleteShader(orxHANDLE _hShader)
{
  sf::PostFX *poFX;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Gets post FX */
  poFX = (sf::PostFX *)_hShader;

  /* Deletes it */
  delete poFX;
}

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_RenderShader(orxHANDLE _hShader)
{
  const sf::PostFX *poFX;
  orxSTATUS         eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Gets post FX */
  poFX = (const sf::PostFX *)_hShader;

  /* Renders it */
  sstDisplay.poRenderWindow->Draw(*poFX);

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_SetShaderBitmap(orxHANDLE _hShader, const orxSTRING _zParam, orxBITMAP *_pstValue)
{
  sf::PostFX *poFX;
  orxSTATUS   eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxNULL) && (_hShader != orxHANDLE_UNDEFINED));

  /* Gets post FX */
  poFX = (sf::PostFX *)_hShader;

  /* Screen? */
  if((_pstValue == orxNULL) || (_pstValue == orxDisplay::spoScreen))
  {
    /* Sets texture */
    poFX->SetTexture(_zParam, NULL);
  }
  else
  {
    sf::Sprite *poSprite;
    sf::Image  *poImage;

    /* Gets sprite */
    poSprite = (sf::Sprite *)_pstValue;

    /* Has image? */
    if((poImage = const_cast<sf::Image *>(poSprite->GetImage())) != orxNULL)
    {
      /* Sets texture */
      poFX->SetTexture(_zParam, poImage);
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

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_SetShaderFloat(orxHANDLE _hShader, const orxSTRING _zParam, orxFLOAT _fValue)
{
  sf::PostFX *poFX;
  orxSTATUS   eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxNULL) && (_hShader != orxHANDLE_UNDEFINED));

  /* Gets post FX */
  poFX = (sf::PostFX *)_hShader;

  /* Sets parameter */
  poFX->SetParameter(_zParam, _fValue);

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_SetShaderVector(orxHANDLE _hShader, const orxSTRING _zParam, const orxVECTOR *_pvValue)
{
  sf::PostFX *poFX;
  orxSTATUS   eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxNULL) && (_hShader != orxHANDLE_UNDEFINED));

  /* Gets post FX */
  poFX = (sf::PostFX *)_hShader;

  /* Sets parameter */
  poFX->SetParameter(_zParam, _pvValue->fX, _pvValue->fY, _pvValue->fZ);

  /* Done! */
  return eResult;
}

extern "C" orxHANDLE orxFASTCALL orxDisplay_SFML_GetApplicationInput()
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
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_CreateText, DISPLAY, CREATE_TEXT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_DeleteText, DISPLAY, DELETE_TEXT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_TransformText, DISPLAY, TRANSFORM_TEXT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_SetTextString, DISPLAY, SET_TEXT_STRING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_SetTextFont, DISPLAY, SET_TEXT_FONT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_GetTextString, DISPLAY, GET_TEXT_STRING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_GetTextFont, DISPLAY, GET_TEXT_FONT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_GetTextSize, DISPLAY, GET_TEXT_SIZE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_PrintString, DISPLAY, PRINT_STRING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_CreateShader, DISPLAY, CREATE_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_DeleteShader, DISPLAY, DELETE_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_RenderShader, DISPLAY, RENDER_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_SetShaderBitmap, DISPLAY, SET_SHADER_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_SetShaderFloat, DISPLAY, SET_SHADER_FLOAT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_SetShaderVector, DISPLAY, SET_SHADER_VECTOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_GetApplicationInput, DISPLAY, GET_APPLICATION_INPUT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_EnableVSync, DISPLAY, ENABLE_VSYNC);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_IsVSyncEnabled, DISPLAY, IS_VSYNC_ENABLED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_SetFullScreen, DISPLAY, SET_FULL_SCREEN);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_IsFullScreen, DISPLAY, IS_FULL_SCREEN);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_GetVideoModeCounter, DISPLAY, GET_VIDEO_MODE_COUNTER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_GetVideoMode, DISPLAY, GET_VIDEO_MODE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_SetVideoMode, DISPLAY, SET_VIDEO_MODE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_IsVideoModeAvailable, DISPLAY, IS_VIDEO_MODE_AVAILABLE);
orxPLUGIN_USER_CORE_FUNCTION_END();
