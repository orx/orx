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
}

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxDISPLAY_STATIC_t
{
  orxU32            u32Flags;
  orxU32            u32ScreenWidth, u32ScreenHeight, u32ScreenDepth;
  unsigned long     ulWindowStyle;
  orxBOOL           bDefaultSmooth;
  sf::RenderWindow *poRenderWindow;
  orxAABOX          stScreenClip;

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

  /* Is an input request? */
  if((_pstEvent->eType == orxEVENT_TYPE_FIRST_RESERVED)
  && (_pstEvent->eID == orxEVENT_TYPE_FIRST_RESERVED))
  {
    /* Sends input back */
    *((const sf::Input **)_pstEvent->pstPayload) = &sstDisplay.poRenderWindow->GetInput();
  }
  /* Is a cursor set position? */
  else if((_pstEvent->eType == orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseMoved)
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
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Is not screen? */
  if(_pstBitmap != orxDisplay::spoScreen)
  {
    sf::Image  *poImage;
    orxU8      *au8Buffer;

    /* Gets image */
    poImage = const_cast<sf::Image *>(((sf::Sprite *)_pstBitmap)->GetImage());

    /* Allocates buffer */
    au8Buffer = (orxU8 *)orxMemory_Allocate(poImage->GetWidth() * poImage->GetHeight() * 4, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if(au8Buffer != orxNULL)
    {
      orxRGBA *pstPixel;
      orxU32  i;

      /* Fills it */
      for(pstPixel = (orxRGBA *)au8Buffer, i = poImage->GetWidth() * poImage->GetHeight(); i > 0 ; i--, pstPixel++)
      {
        /* Copies pixel */
        *pstPixel = _stColor;
      }

      /* Sets image's data */
      eResult = poImage->LoadFromPixels(poImage->GetWidth(), poImage->GetHeight(), au8Buffer) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;

      /* Frees buffer */
      orxMemory_Free(au8Buffer);
    }
  }
  else
  {
    /* Clear the color buffer with given color */
    glClearColor(orxCOLOR_NORMALIZER * orxRGBA_R(_stColor), orxCOLOR_NORMALIZER * orxRGBA_G(_stColor), orxCOLOR_NORMALIZER * orxRGBA_B(_stColor), orxCOLOR_NORMALIZER * orxRGBA_A(_stColor));
    glClear(GL_COLOR_BUFFER_BIT);
  }

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_Swap()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Displays render window */
  sstDisplay.poRenderWindow->Display();

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_SetBitmapData(orxBITMAP *_pstBitmap, const orxU8 *_au8Data, orxU32 _u32ByteNumber)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT(_au8Data != orxNULL);

  /* Not screen? */
  if(_pstBitmap != orxDisplay::spoScreen)
  {
    sf::Image *poImage;

    /* Gets image */
    poImage = const_cast<sf::Image *>(((sf::Sprite *)_pstBitmap)->GetImage());

    /* Valid number of bytes? */
    if(_u32ByteNumber == poImage->GetWidth() * poImage->GetHeight() * 4)
    {
      /* Updates pixels */
      eResult = poImage->LoadFromPixels(poImage->GetWidth(), poImage->GetHeight(), _au8Data) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
    }
  }
  
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

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_BlitBitmap(const orxBITMAP *_pstSrc, const orxFLOAT _fPosX, orxFLOAT _fPosY, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  sf::Sprite   *poSprite;
  sf::Vector2f  vPosition;
  bool          bSmooth;
  orxSTATUS     eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstSrc != orxNULL) && (_pstSrc != orxDisplay::spoScreen));

  /* Gets sprite */
  poSprite = (sf::Sprite *)_pstSrc;

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

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_SetDestinationBitmap(orxBITMAP *_pstDst)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstDst == orxDisplay::spoScreen) && "Can only draw on screen with this version!");

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_TransformText(const orxSTRING _zString, const orxBITMAP *_pstFont, const orxCHARACTER_MAP *_pstMap, const orxDISPLAY_TRANSFORM *_pstTransform, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  sf::Sprite     *poSprite;
  orxU32          u32CharacterCodePoint;
  const orxCHAR  *pc;
  orxFLOAT        fX, fY, fStartX, fStartY;
  orxVECTOR       vSpacing;
  orxSTATUS       eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_zString != orxNULL);
  orxASSERT(_pstFont != orxNULL);
  orxASSERT(_pstMap != orxNULL);
  orxASSERT(_pstTransform != orxNULL);

  /* Gets sprite */
  poSprite = (sf::Sprite *)_pstFont;

  /* Updates its rotation */
  poSprite->SetRotation(-orxMATH_KF_RAD_TO_DEG * _pstTransform->fRotation);

  /* Gets spacing */
  orxVector_Neg(&vSpacing, &(_pstMap->vCharacterSize));
  
  /* Updates its flipping */
  if(_pstTransform->fScaleX < 0.0f)
  {
    poSprite->FlipX(true);
    vSpacing.fX = -vSpacing.fX;
    fStartX     = -_pstTransform->fSrcX + vSpacing.fX;
  }
  else
  {
    fStartX     = _pstTransform->fSrcX;
  }

  if(_pstTransform->fScaleY < 0.0f)
  {
    poSprite->FlipY(true);
    vSpacing.fY = -vSpacing.fY;
    fStartY     = -_pstTransform->fSrcY + vSpacing.fY;
  }
  else
  {
    fStartY     = _pstTransform->fSrcY;
  }
  
  /* Updates its scale */
  poSprite->SetScale(orxMath_Abs(_pstTransform->fScaleX), orxMath_Abs(_pstTransform->fScaleY));

  /* For all characters */
  for(u32CharacterCodePoint = orxString_GetFirstCharacterCodePoint(_zString, &pc), fX = fStartX, fY = fStartY;
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
        /* Updates Y position */
        fY += vSpacing.fY;

        /* Resets X position */
        fX = fStartX;

        break;
      }

      default:
      {
        const orxCHARACTER_GLYPH *pstGlyph;

        /* Gets glyph from table */
        pstGlyph = (orxCHARACTER_GLYPH *)orxHashTable_Get(_pstMap->pstCharacterTable, u32CharacterCodePoint);

        /* Valid? */
        if(pstGlyph != orxNULL)
        {
          /* Sets sub rectangle for sprite */
          poSprite->SetSubRect(sf::IntRect(orxF2S(pstGlyph->fX), orxF2S(pstGlyph->fY), orxF2S(pstGlyph->fX + _pstMap->vCharacterSize.fX), orxF2S(pstGlyph->fY + _pstMap->vCharacterSize.fY)));

          /* Updates its center */
          poSprite->SetCenter(fX, fY);

          /* Blits it */
          eResult = orxDisplay_SFML_BlitBitmap(_pstFont, _pstTransform->fDstX, _pstTransform->fDstY, _eSmoothing, _eBlendMode);
        }

        /* Updates X position */
        fX += vSpacing.fX;

        break;
      }
    }
  }

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

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_TransformBitmap(const orxBITMAP *_pstSrc, const orxDISPLAY_TRANSFORM *_pstTransform, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  sf::Sprite *poSprite;
  orxFLOAT    fCenterX, fCenterY;
  orxBOOL     bFlipX, bFlipY;
  orxSTATUS   eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstSrc != orxNULL) && (_pstSrc != orxDisplay::spoScreen));
  orxASSERT(_pstTransform != orxNULL);

  /* Gets sprite */
  poSprite = (sf::Sprite *)_pstSrc;

  /* Updates its rotation */
  poSprite->SetRotation(-orxMATH_KF_RAD_TO_DEG * _pstTransform->fRotation);

  /* Updates its flipping */
  if(_pstTransform->fScaleX < 0.0f)
  {
    poSprite->FlipX(true);
    fCenterX = poSprite->GetSize().x - _pstTransform->fSrcX;
    bFlipX = true;
  }
  else
  {
    fCenterX = _pstTransform->fSrcX;
    bFlipX = false;
  }

  if(_pstTransform->fScaleY < 0.0f)
  {
    poSprite->FlipY(true);
    fCenterY = poSprite->GetSize().y - _pstTransform->fSrcY;
    bFlipY = true;
  }
  else
  {
    fCenterY = _pstTransform->fSrcY;
    bFlipY = false;
  }

  /* Updates its center */
  poSprite->SetCenter(fCenterX, fCenterY);

  /* Has repeat? */
  if((_pstTransform->fRepeatX != orxFLOAT_1) || (_pstTransform->fRepeatY != orxFLOAT_1))
  {
    orxFLOAT    fIncX, fIncY, fCos, fSin, fX, fY, fSizeX, fSizeY, fScaleX, fScaleY, fRemainderX, fRemainderY, fInitRemainderX, fInitRemainderY, fRelativePivotX, fRelativePivotY, fAbsScaleX, fAbsScaleY;
    sf::IntRect stClip;

    /* Gets sprite's size */
    fSizeX = (orxFLOAT)poSprite->GetImage()->GetWidth();
    fSizeY = (orxFLOAT)poSprite->GetImage()->GetHeight();

    /* Gets sprite's clipping */
    stClip = poSprite->GetSubRect();

    /* Has no rotation */
    if(_pstTransform->fRotation == orxFLOAT_0)
    {
      /* Gets cosine and sine of the object angle */
      fCos = orxFLOAT_1;
      fSin = orxFLOAT_0;
    }
    /* 90°? */
    else if(_pstTransform->fRotation == orxMATH_KF_PI_BY_2)
    {
      /* Gets cosine and sine of the object angle */
      fCos = orxFLOAT_0;
      fSin = -orxFLOAT_1;
    }
    /* 180°? */
    else if(_pstTransform->fRotation == orxMATH_KF_PI)
    {
      /* Gets cosine and sine of the object angle */
      fCos = -orxFLOAT_1;
      fSin = orxFLOAT_0;
    }
    /* 180°? */
    else if(_pstTransform->fRotation == -orxMATH_KF_PI_BY_2)
    {
      /* Gets cosine and sine of the object angle */
      fCos = orxFLOAT_0;
      fSin = orxFLOAT_1;
    }
    else
    {
      /* Gets cosine and sine of the object angle */
      fCos = orxMath_Cos(-_pstTransform->fRotation);
      fSin = orxMath_Sin(-_pstTransform->fRotation);
    }

    /* Tiling on X? */
    if(_pstTransform->fRepeatX == _pstTransform->fScaleX)
    {
      /* Updates scale */
      fScaleX = orxFLOAT_1;

      /* Updates increment */
      fIncX = fSizeX;
    }
    else
    {
      /* Updates scale */
      fScaleX = _pstTransform->fScaleX / _pstTransform->fRepeatX;

      /* Updates increment */
      fIncX = fSizeX * fScaleX;
    }

    /* Tiling on Y? */
    if(_pstTransform->fRepeatY == _pstTransform->fScaleY)
    {
      /* Updates scale */
      fScaleY = orxFLOAT_1;

      /* Updates increment */
      fIncY = fSizeY;
    }
    else
    {
      /* Updates scale */
      fScaleY = _pstTransform->fScaleY / _pstTransform->fRepeatY;

      /* Updates increment */
      fIncY = fSizeY * fScaleY;
    }

    /* Gets relative pivot */
    fRelativePivotX = _pstTransform->fSrcX / fSizeX;
    fRelativePivotY = _pstTransform->fSrcY / fSizeY;

    /* For all lines */
    for(fY = -fRelativePivotY * fIncY * (_pstTransform->fRepeatY - orxFLOAT_1), fInitRemainderY = fRemainderY = _pstTransform->fRepeatY * fSizeY, fAbsScaleY = orxMath_Abs(fScaleY);
      fRemainderY > orxFLOAT_0;
      fY += fIncY, fRemainderY -= fSizeY)
    {
      orxFLOAT fPosY = fY;

      /* Positive scale on Y? */
      if(fScaleY > orxFLOAT_0)
      {
        /* Flipped? */
        if(bFlipY != orxFALSE)
        {
          /* Gets adjusted position */
          fPosY -= fInitRemainderY;
        }
      }
      else
      {
        /* Not flipped? */
        if(bFlipY == orxFALSE)
        {
          /* Last line? */
          if(fRemainderY < fSizeY)
          {
            /* Gets adjusted position */
            fPosY += fAbsScaleY * (fSizeY - fRemainderY);
          }
        }
        else
        {
          /* Not last line? */
          if(fRemainderY >= fSizeY)
          {
            /* Gets adjusted position */
            fPosY += fInitRemainderY;
          }
          else
          {
            /* Gets adjusted position */
            fPosY += fInitRemainderY + fAbsScaleY * (fSizeY - fRemainderY);
          }
        }
      }

      /* For all columns */
      for(fX = -fRelativePivotX * fIncX * (_pstTransform->fRepeatX - orxFLOAT_1), fInitRemainderX = fRemainderX = _pstTransform->fRepeatX * fSizeX, fAbsScaleX = orxMath_Abs(fScaleX);
        fRemainderX > orxFLOAT_0;
        fX += fIncX, fRemainderX -= fSizeX)
      {
        orxFLOAT fOffsetX, fOffsetY, fPosX = fX;

        /* Updates clip info */
        stClip.Right  = stClip.Left + orxF2U(orxMIN(fSizeX, fRemainderX));
        stClip.Bottom = stClip.Top + orxF2U(orxMIN(fSizeY, fRemainderY));

        /* Sets sub rectangle for sprite */
        poSprite->SetSubRect(stClip);

        /* Positive scale on X? */
        if(fScaleX > orxFLOAT_0)
        {
          /* Flipped? */
          if(bFlipX != orxFALSE)
          {
            /* Gets adjusted position */
            fPosX -= fInitRemainderX;
          }
        }
        else
        {
          /* Not flipped? */
          if(bFlipX == orxFALSE)
          {
            /* Last line? */
            if(fRemainderX < fSizeX)
            {
              /* Gets adjusted position */
              fPosX += fAbsScaleX * (fSizeX - fRemainderX);
            }
          }
          else
          {
            /* Not last line? */
            if(fRemainderX >= fSizeX)
            {
              /* Gets adjusted position */
              fPosX += fInitRemainderX;
            }
            else
            {
              /* Gets adjusted position */
              fPosX += fInitRemainderX + fAbsScaleX * (fSizeX - fRemainderX);
            }
          }
        }

        /* Computes offsets */
        fOffsetX = (fCos * fPosX) + (fSin * fPosY);
        fOffsetY = (-fSin * fPosX) + (fCos * fPosY);

        /* Updates its scale */
        poSprite->SetScale(orxMath_Abs(fScaleX), orxMath_Abs(fScaleY));

        /* Blits bitmap */
        eResult = orxDisplay_SFML_BlitBitmap(_pstSrc, _pstTransform->fDstX + fOffsetX, _pstTransform->fDstY + fOffsetY, _eSmoothing, _eBlendMode);
      }
    }
  }
  else
  {
    /* Updates its scale */
    poSprite->SetScale(orxMath_Abs(_pstTransform->fScaleX), orxMath_Abs(_pstTransform->fScaleY));

    /* Blits it */
    eResult = orxDisplay_SFML_BlitBitmap(_pstSrc, _pstTransform->fDstX, _pstTransform->fDstY, _eSmoothing, _eBlendMode);
  }

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

    /* Stores clip coords */
    orxVector_Set(&(sstDisplay.stScreenClip.vTL), orxU2F(_u32TLX), orxU2F(_u32TLY), orxFLOAT_0);
    orxVector_Set(&(sstDisplay.stScreenClip.vBR), orxU2F(_u32BRX), orxU2F(_u32BRY), orxFLOAT_0);
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

  /* Enable? */
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

  /* Enable? */
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
      /* Registers our mouse wheel event handler */
      if(orxEvent_AddHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseButtonPressed), orxDisplay_SFML_EventHandler) != orxSTATUS_FAILURE)
      {
        orxU32        u32ConfigWidth, u32ConfigHeight, u32ConfigDepth, u32Flags = orxDISPLAY_KU32_STATIC_FLAG_READY;
        orxCLOCK     *pstClock;

        /* Adds event handler */
        orxEvent_AddHandler((orxEVENT_TYPE)orxEVENT_TYPE_FIRST_RESERVED, orxDisplay_SFML_EventHandler);

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
        /* Updates result */
        eResult = orxSTATUS_FAILURE;
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
    /* Unregisters event handlers */
    orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseMoved), orxDisplay_SFML_EventHandler);
    orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + sf::Event::MouseButtonPressed), orxDisplay_SFML_EventHandler);
    orxEvent_RemoveHandler((orxEVENT_TYPE)orxEVENT_TYPE_FIRST_RESERVED, orxDisplay_SFML_EventHandler);

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
        acBuffer[0] = orxCHAR_NULL;
        pc          = acBuffer;
        s32Free     = 32768;

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

              /* Adds its literal value and automated coordinates */
              s32Offset = orxString_NPrint(pc, s32Free, "texture %s\nfloat %s"orxDISPLAY_KZ_SHADER_SUFFIX_TOP"\nfloat %s"orxDISPLAY_KZ_SHADER_SUFFIX_LEFT"\nfloat %s"orxDISPLAY_KZ_SHADER_SUFFIX_BOTTOM"\nfloat %s"orxDISPLAY_KZ_SHADER_SUFFIX_RIGHT"\n", pstParam->zName, pstParam->zName, pstParam->zName, pstParam->zName, pstParam->zName);
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

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_StartShader(orxHANDLE _hShader)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_StopShader(orxHANDLE _hShader)
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
  orxCHAR     acBuffer[256];
  float       fRecWidth, fRecHeight;
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

    /* Get's image rec width & rec height */
    fRecWidth = 1.0f / (float)sstDisplay.u32ScreenWidth;
    fRecHeight = 1.0f / (float)sstDisplay.u32ScreenHeight;

    /* Gets top parameter location */
    orxString_NPrint(acBuffer, 256, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_TOP, _zParam);
    poFX->SetParameter(acBuffer, 1.0f - (fRecHeight * sstDisplay.stScreenClip.vTL.fY));

    /* Gets left parameter location */
    orxString_NPrint(acBuffer, 256, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_LEFT, _zParam);
    poFX->SetParameter(acBuffer, fRecWidth * sstDisplay.stScreenClip.vTL.fX);

    /* Gets bottom parameter location */
    orxString_NPrint(acBuffer, 256, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_BOTTOM, _zParam);
    poFX->SetParameter(acBuffer, 1.0f - (fRecHeight * sstDisplay.stScreenClip.vBR.fY));

    /* Gets right parameter location */
    orxString_NPrint(acBuffer, 256, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_RIGHT, _zParam);
    poFX->SetParameter(acBuffer, fRecWidth * sstDisplay.stScreenClip.vBR.fX);
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
      sf::IntRect stClip;

      /* Sets texture */
      poFX->SetTexture(_zParam, poImage);

      /* Gets sprite's clipping */
      stClip = poSprite->GetSubRect();

      /* Get's image rec width & rec height */
      fRecWidth = 1.0f / (float)poImage->GetWidth();
      fRecHeight = 1.0f / (float)poImage->GetHeight();

      /* Gets top parameter location */
      orxString_NPrint(acBuffer, 256, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_TOP, _zParam);
      poFX->SetParameter(acBuffer, 1.0f - (fRecHeight * stClip.Top));

      /* Gets left parameter location */
      orxString_NPrint(acBuffer, 256, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_LEFT, _zParam);
      poFX->SetParameter(acBuffer, fRecWidth * stClip.Left);

      /* Gets bottom parameter location */
      orxString_NPrint(acBuffer, 256, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_BOTTOM, _zParam);
      poFX->SetParameter(acBuffer, 1.0f - (fRecHeight * stClip.Bottom));

      /* Gets right parameter location */
      orxString_NPrint(acBuffer, 256, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_RIGHT, _zParam);
      poFX->SetParameter(acBuffer, fRecWidth * stClip.Right);
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
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_SetDestinationBitmap, DISPLAY, SET_DESTINATION_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_TransformBitmap, DISPLAY, TRANSFORM_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_LoadBitmap, DISPLAY, LOAD_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_GetBitmapSize, DISPLAY, GET_BITMAP_SIZE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_GetScreenSize, DISPLAY, GET_SCREEN_SIZE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_GetScreen, DISPLAY, GET_SCREEN_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_ClearBitmap, DISPLAY, CLEAR_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_SetBitmapClipping, DISPLAY, SET_BITMAP_CLIPPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_BlitBitmap, DISPLAY, BLIT_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_SetBitmapData, DISPLAY, SET_BITMAP_DATA);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_SetBitmapColorKey, DISPLAY, SET_BITMAP_COLOR_KEY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_SetBitmapColor, DISPLAY, SET_BITMAP_COLOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_GetBitmapColor, DISPLAY, GET_BITMAP_COLOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_TransformText, DISPLAY, TRANSFORM_TEXT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_CreateShader, DISPLAY, CREATE_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_DeleteShader, DISPLAY, DELETE_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_StartShader, DISPLAY, START_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_StopShader, DISPLAY, STOP_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_SetShaderBitmap, DISPLAY, SET_SHADER_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_SetShaderFloat, DISPLAY, SET_SHADER_FLOAT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_SetShaderVector, DISPLAY, SET_SHADER_VECTOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_EnableVSync, DISPLAY, ENABLE_VSYNC);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_IsVSyncEnabled, DISPLAY, IS_VSYNC_ENABLED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_SetFullScreen, DISPLAY, SET_FULL_SCREEN);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_IsFullScreen, DISPLAY, IS_FULL_SCREEN);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_GetVideoModeCounter, DISPLAY, GET_VIDEO_MODE_COUNTER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_GetVideoMode, DISPLAY, GET_VIDEO_MODE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_SetVideoMode, DISPLAY, SET_VIDEO_MODE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_IsVideoModeAvailable, DISPLAY, IS_VIDEO_MODE_AVAILABLE);
orxPLUGIN_USER_CORE_FUNCTION_END();
