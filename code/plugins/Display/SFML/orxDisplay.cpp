/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2013 Orx-Project
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
  static const orxBITMAP *spoScreen               = (const orxBITMAP *)-1;
}

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Display shader
 */
struct orxDISPLAY_SHADER
{
  sf::PostFX             *poFX;
  std::vector<orxSTRING>  azParamList;

  orxDISPLAY_SHADER()
  {
    poFX = new sf::PostFX();
  }

  ~orxDISPLAY_SHADER()
  {
    delete poFX;

    for(orxU32 i = 0; i < azParamList.size(); i++)
    {
      orxString_Delete(azParamList[i]);
    }
    azParamList.clear();
  }

};

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
  sf::Sprite *poSprite;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Not screen? */
  if(_pstBitmap != orxDisplay::spoScreen)
  {
    const sf::Image *poImage;

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
    au8Buffer = (orxU8 *)orxMemory_Allocate(poImage->GetWidth() * poImage->GetHeight() * 4 * sizeof(orxU8), orxMEMORY_TYPE_MAIN);

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

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_GetBitmapData(orxBITMAP *_pstBitmap, orxU8 *_au8Data, orxU32 _u32ByteNumber)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Not screen? */
  if(_pstBitmap != orxDisplay::spoScreen)
  {
    const sf::Uint8 *pu8Data;
    const sf::Image *poImage;

    /* Gets image */
    poImage = ((sf::Sprite *)_pstBitmap)->GetImage();

    /* Gets its content */
    pu8Data = poImage->GetPixelsPtr();

    /* Copies it */
    orxMemory_Copy(_au8Data, pu8Data, orxMIN(_u32ByteNumber, poImage->GetWidth() * poImage->GetHeight() * 4));

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

extern "C" orxU32 orxFASTCALL orxDisplay_SFML_GetBitmapID(const orxBITMAP *_pstBitmap)
{
    /* Checks */
    orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
    orxASSERT((_pstBitmap != orxNULL) && (_pstBitmap != orxDisplay::spoScreen));

    /* Not available */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

    /* Done! */
    return orxU32_UNDEFINED;
}

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_BlitBitmap(const orxBITMAP *_pstSrc, orxFLOAT _fPosX, orxFLOAT _fPosY, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
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

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_SetDestinationBitmaps(orxBITMAP **_apstDst, orxU32 _u32Number)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_apstDst[0] == orxDisplay::spoScreen) && "Can only draw on screen with this version!");

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
    orxDISPLAY_EVENT_PAYLOAD stPayload;

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

    /* Inits event payload */
    orxMemory_Zero(&stPayload, sizeof(orxDISPLAY_EVENT_PAYLOAD));
    stPayload.u32Width    = sstDisplay.u32ScreenWidth;
    stPayload.u32Height   = sstDisplay.u32ScreenHeight;
    stPayload.u32Depth    = sstDisplay.u32ScreenDepth;
    stPayload.bFullScreen = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_FULLSCREEN) ? orxTRUE : orxFALSE;

    /* Sends it */
    orxEVENT_SEND(orxEVENT_TYPE_DISPLAY, orxDISPLAY_EVENT_SET_VIDEO_MODE, orxNULL, orxNULL, &stPayload);
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

  /* Desktop mode? */
  if(_u32Index == orxU32_UNDEFINED)
  {
    /* Gets desktop video mode */
    sf::VideoMode roVideoMode = sf::VideoMode::GetDesktopMode();

    /* Stores info */
    _pstVideoMode->u32Width       = roVideoMode.Width;
    _pstVideoMode->u32Height      = roVideoMode.Height;
    _pstVideoMode->u32Depth       = roVideoMode.BitsPerPixel;
    _pstVideoMode->u32RefreshRate = 0;
    _pstVideoMode->bFullScreen    = orxDisplay_SFML_IsFullScreen();
  }
  /* Is index valid? */
  else if(_u32Index < orxDisplay_SFML_GetVideoModeCounter())
  {
    /* Gets video mode */
    sf::VideoMode roVideoMode = sf::VideoMode::GetMode(_u32Index);

    /* Stores info */
    _pstVideoMode->u32Width       = roVideoMode.Width;
    _pstVideoMode->u32Height      = roVideoMode.Height;
    _pstVideoMode->u32Depth       = roVideoMode.BitsPerPixel;
    _pstVideoMode->u32RefreshRate = 0;
    _pstVideoMode->bFullScreen    = orxDisplay_SFML_IsFullScreen();
  }
  else
  {
    /* Stores info */
    _pstVideoMode->u32Width       = sstDisplay.u32ScreenWidth;
    _pstVideoMode->u32Height      = sstDisplay.u32ScreenHeight;
    _pstVideoMode->u32Depth       = sstDisplay.u32ScreenDepth;
    _pstVideoMode->u32RefreshRate = 0;
    _pstVideoMode->bFullScreen    = orxDisplay_SFML_IsFullScreen();
  }

  /* Updates result */
  pstResult = _pstVideoMode;

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
    orxDISPLAY_EVENT_PAYLOAD stPayload;

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
    if(_pstVideoMode->bFullScreen != orxFALSE)
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

    /* Is fullscreen? */
    if(_pstVideoMode->bFullScreen != orxFALSE)
    {
      /* Updates status */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_FULLSCREEN, orxDISPLAY_KU32_STATIC_FLAG_NONE);
    }
    else
    {
      /* Updates status */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NONE, orxDISPLAY_KU32_STATIC_FLAG_FULLSCREEN);
    }

    /* Pops config section */
    orxConfig_PopSection();

    /* Inits event payload */
    orxMemory_Zero(&stPayload, sizeof(orxDISPLAY_EVENT_PAYLOAD));
    stPayload.u32Width    = sstDisplay.u32ScreenWidth;
    stPayload.u32Height   = sstDisplay.u32ScreenHeight;
    stPayload.u32Depth    = sstDisplay.u32ScreenDepth;
    stPayload.bFullScreen = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_FULLSCREEN) ? orxTRUE : orxFALSE;

    /* Sends it */
    orxEVENT_SEND(orxEVENT_TYPE_DISPLAY, orxDISPLAY_EVENT_SET_VIDEO_MODE, orxNULL, orxNULL, &stPayload);

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

        /* Gets desktop video mode */
        sf::VideoMode roDesktopMode = sf::VideoMode::GetDesktopMode();

        /* Adds event handler */
        orxEvent_AddHandler((orxEVENT_TYPE)orxEVENT_TYPE_FIRST_RESERVED, orxDisplay_SFML_EventHandler);

        /* Gets resolution from config */
        orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);
        u32ConfigWidth  = orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_WIDTH) ? orxConfig_GetU32(orxDISPLAY_KZ_CONFIG_WIDTH) : roDesktopMode.Width;
        u32ConfigHeight = orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_HEIGHT) ? orxConfig_GetU32(orxDISPLAY_KZ_CONFIG_HEIGHT) : roDesktopMode.Height;
        u32ConfigDepth  = orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_DEPTH) ? orxConfig_GetU32(orxDISPLAY_KZ_CONFIG_DEPTH) : roDesktopMode.BitsPerPixel;

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
          sstDisplay.poRenderWindow = new sf::RenderWindow(roDesktopMode, orxConfig_GetString(orxDISPLAY_KZ_CONFIG_TITLE), sstDisplay.ulWindowStyle);

          /* Stores depth */
          sstDisplay.u32ScreenDepth = roDesktopMode.BitsPerPixel;
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
          eResult = orxClock_Register(pstClock, orxDisplay_SFML_EventUpdate, orxNULL, orxMODULE_ID_DISPLAY, orxCLOCK_PRIORITY_HIGHER);
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

    /* Has no rotation? */
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
    /* 270°? */
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

  /* Gets vertical spacing */
  vSpacing.fY = -_pstMap->fCharacterHeight;
  
  /* Updates its flipping */
  if(_pstTransform->fScaleX < 0.0f)
  {
    poSprite->FlipX(true);
    fStartX     = -_pstTransform->fSrcX - vSpacing.fY;
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
          /* Gets horizontal spacing */
          vSpacing.fX = -pstGlyph->fWidth;

          /* Sets sub rectangle for sprite */
          poSprite->SetSubRect(sf::IntRect(orxF2S(pstGlyph->fX), orxF2S(pstGlyph->fY), orxF2S(pstGlyph->fX + pstGlyph->fWidth), orxF2S(pstGlyph->fY + _pstMap->fCharacterHeight)));

          /* Updates its center */
          poSprite->SetCenter(fX, fY);

          /* Blits it */
          eResult = orxDisplay_SFML_BlitBitmap(_pstFont, _pstTransform->fDstX, _pstTransform->fDstY, _eSmoothing, _eBlendMode);
        }
        else
        {
          /* Gets default horizontal spacing */
          vSpacing.fX = -_pstMap->fCharacterHeight;
        }

        /* Updates its flipping */
        if(_pstTransform->fScaleX < 0.0f)
        {
          vSpacing.fX = -vSpacing.fX;
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

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_DrawLine(const orxVECTOR *_pvStart, const orxVECTOR *_pvEnd, orxRGBA _stColor)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvStart != orxNULL);
  orxASSERT(_pvEnd != orxNULL);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_DrawPolyline(const orxVECTOR *_avVertexList, orxU32 _u32VertexNumber, orxRGBA _stColor)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_avVertexList != orxNULL);
  orxASSERT(_u32VertexNumber > 0);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_DrawPolygon(const orxVECTOR *_avVertexList, orxU32 _u32VertexNumber, orxRGBA _stColor, orxBOOL _bFill)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_avVertexList != orxNULL);
  orxASSERT(_u32VertexNumber > 0);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_DrawCircle(const orxVECTOR *_pvCenter, orxFLOAT _fRadius, orxRGBA _stColor, orxBOOL _bFill)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvCenter != orxNULL);
  orxASSERT(_fRadius >= orxFLOAT_0);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_DrawOBox(const orxOBOX *_pstBox, orxRGBA _stColor, orxBOOL _bFill)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBox != orxNULL);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_DrawMesh(const orxBITMAP *_pstBitmap, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode, orxU32 _u32VertexNumber, const orxDISPLAY_VERTEX *_astVertexList)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_u32VertexNumber > 2);
  orxASSERT(_astVertexList != orxNULL);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return eResult;
}

extern "C" orxBOOL orxFASTCALL orxDisplay_SFML_HasShaderSupport()
{
  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  
  /* Done! */
  return (sf::PostFX::CanUsePostFX() != false) ? orxTRUE : orxFALSE;
}

extern "C" orxHANDLE orxFASTCALL orxDisplay_SFML_CreateShader(const orxSTRING _zCode, const orxLINKLIST *_pstParamList, orxBOOL _bUseCustomParam)
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
      orxDISPLAY_SHADER *pstShader;

      /* Creates new shader */
      pstShader = new orxDISPLAY_SHADER();

      /* Valid? */
      if(pstShader)
      {
        orxSHADER_PARAM  *pstParam;
        orxCHAR           acBuffer[32768], *pc;
        orxS32            s32Free;

        /* Inits buffer */
        acBuffer[0] = acBuffer[sizeof(acBuffer) - 1] = orxCHAR_NULL;
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
            case orxSHADER_PARAM_TYPE_TIME:
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
        if(pstShader->poFX->LoadFromMemory(acBuffer) != false)
        {
          /* Updates result */
          hResult = (orxHANDLE)pstShader;
        }
        else
        {
          orxLOG("Failed to compile shader code: %s", acBuffer);

          /* Deletes shader */
          delete pstShader;
        }
      }
    }
  }

  /* Done! */
  return hResult;
}

extern "C" void orxFASTCALL orxDisplay_SFML_DeleteShader(orxHANDLE _hShader)
{
  orxDISPLAY_SHADER *pstShader;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* Deletes it */
  delete pstShader;
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
  orxDISPLAY_SHADER *pstShader;
  orxSTATUS         eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* Renders it */
  sstDisplay.poRenderWindow->Draw(*pstShader->poFX);

  /* Done! */
  return eResult;
}

extern "C" orxS32 orxFASTCALL orxDisplay_SFML_GetParameterID(const orxHANDLE _hShader, const orxSTRING _zParam, orxS32 _s32Index, orxBOOL _bIsTexture)
{
  orxDISPLAY_SHADER  *pstShader;
  orxSTRING           zString;
  orxS32              s32Result;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxNULL) && (_hShader != orxHANDLE_UNDEFINED));

  /* Duplicates name */
  zString = orxString_Duplicate(_zParam);

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* Adds parameter */
  pstShader->azParamList.push_back(zString);

  /* Updates result */
  s32Result = pstShader->azParamList.size() - 1;

  /* Done! */
  return s32Result;
}

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_SetShaderBitmap(orxHANDLE _hShader, orxS32 _s32ID, orxBITMAP *_pstValue)
{
  orxDISPLAY_SHADER  *pstShader;
  orxCHAR             acBuffer[256];
  float               fRecWidth, fRecHeight;
  orxSTATUS           eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxNULL) && (_hShader != orxHANDLE_UNDEFINED));

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* Inits buffer */
  acBuffer[sizeof(acBuffer) - 1] = orxCHAR_NULL;

  /* Screen? */
  if((_pstValue == orxNULL) || (_pstValue == orxDisplay::spoScreen))
  {
    orxSTRING zText = pstShader->azParamList[_s32ID];

    /* Sets texture */
    pstShader->poFX->SetTexture(zText, NULL);

    /* Get's image rec width & rec height */
    fRecWidth = 1.0f / (float)sstDisplay.u32ScreenWidth;
    fRecHeight = 1.0f / (float)sstDisplay.u32ScreenHeight;

    /* Gets top parameter location */
    orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_TOP, zText);
    pstShader->poFX->SetParameter(acBuffer, 1.0f - (fRecHeight * sstDisplay.stScreenClip.vTL.fY));

    /* Gets left parameter location */
    orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_LEFT, zText);
    pstShader->poFX->SetParameter(acBuffer, fRecWidth * sstDisplay.stScreenClip.vTL.fX);

    /* Gets bottom parameter location */
    orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_BOTTOM, zText);
    pstShader->poFX->SetParameter(acBuffer, 1.0f - (fRecHeight * sstDisplay.stScreenClip.vBR.fY));

    /* Gets right parameter location */
    orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_RIGHT, zText);
    pstShader->poFX->SetParameter(acBuffer, fRecWidth * sstDisplay.stScreenClip.vBR.fX);
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
      orxSTRING zText = pstShader->azParamList[_s32ID];

      /* Sets texture */
      pstShader->poFX->SetTexture(zText, poImage);

      /* Gets sprite's clipping */
      stClip = poSprite->GetSubRect();

      /* Get's image rec width & rec height */
      fRecWidth = 1.0f / (float)poImage->GetWidth();
      fRecHeight = 1.0f / (float)poImage->GetHeight();

      /* Gets top parameter location */
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_TOP, zText);
      pstShader->poFX->SetParameter(acBuffer, 1.0f - (fRecHeight * stClip.Top));

      /* Gets left parameter location */
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_LEFT, zText);
      pstShader->poFX->SetParameter(acBuffer, fRecWidth * stClip.Left);

      /* Gets bottom parameter location */
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_BOTTOM, zText);
      pstShader->poFX->SetParameter(acBuffer, 1.0f - (fRecHeight * stClip.Bottom));

      /* Gets right parameter location */
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_RIGHT, zText);
      pstShader->poFX->SetParameter(acBuffer, fRecWidth * stClip.Right);
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

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_SetShaderFloat(orxHANDLE _hShader, orxS32 _s32ID, orxFLOAT _fValue)
{
  orxDISPLAY_SHADER *pstShader;
  orxSTATUS          eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxNULL) && (_hShader != orxHANDLE_UNDEFINED));

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* Sets parameter */
  pstShader->poFX->SetParameter(pstShader->azParamList[_s32ID], _fValue);

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxDisplay_SFML_SetShaderVector(orxHANDLE _hShader, orxS32 _s32ID, const orxVECTOR *_pvValue)
{
  orxDISPLAY_SHADER *pstShader;
  orxSTATUS          eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxNULL) && (_hShader != orxHANDLE_UNDEFINED));

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* Sets parameter */
  pstShader->poFX->SetParameter(pstShader->azParamList[_s32ID], _pvValue->fX, _pvValue->fY, _pvValue->fZ);

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
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_SetDestinationBitmaps, DISPLAY, SET_DESTINATION_BITMAPS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_LoadBitmap, DISPLAY, LOAD_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_GetBitmapData, DISPLAY, GET_BITMAP_DATA);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_GetBitmapSize, DISPLAY, GET_BITMAP_SIZE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_GetScreenSize, DISPLAY, GET_SCREEN_SIZE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_GetScreen, DISPLAY, GET_SCREEN_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_ClearBitmap, DISPLAY, CLEAR_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_SetBitmapClipping, DISPLAY, SET_BITMAP_CLIPPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_GetBitmapID, DISPLAY, GET_BITMAP_ID);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_SetBitmapData, DISPLAY, SET_BITMAP_DATA);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_SetBitmapColorKey, DISPLAY, SET_BITMAP_COLOR_KEY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_SetBitmapColor, DISPLAY, SET_BITMAP_COLOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_GetBitmapColor, DISPLAY, GET_BITMAP_COLOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_TransformBitmap, DISPLAY, TRANSFORM_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_TransformText, DISPLAY, TRANSFORM_TEXT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_DrawLine, DISPLAY, DRAW_LINE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_DrawPolyline, DISPLAY, DRAW_POLYLINE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_DrawPolygon, DISPLAY, DRAW_POLYGON);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_DrawCircle, DISPLAY, DRAW_CIRCLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_DrawOBox, DISPLAY, DRAW_OBOX);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_DrawMesh, DISPLAY, DRAW_MESH);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_HasShaderSupport, DISPLAY, HAS_SHADER_SUPPORT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_CreateShader, DISPLAY, CREATE_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_DeleteShader, DISPLAY, DELETE_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_StartShader, DISPLAY, START_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_StopShader, DISPLAY, STOP_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_GetParameterID, DISPLAY, GET_PARAMETER_ID);
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
