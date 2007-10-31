/**
 * @file orxDisplay.cpp
 * 
 * SFML display plugin
 * 
 */
 
 /***************************************************************************
 orxDisplay.cpp
 SFML display plugin
 
 begin                : 18/10/2007
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

extern "C"
{
#include "orxInclude.h"

#include "math/orxMath.h"
#include "plugin/orxPluginUser.h"

#include "display/orxDisplay.h"
}

#include <SFML/Graphics.hpp>


/** Module flags
 */
#define orxDISPLAY_KU32_STATIC_FLAG_NONE        0x00000000 /**< No flags */

#define orxDISPLAY_KU32_STATIC_FLAG_READY       0x00000001 /**< Ready flag */
#define orxDISPLAY_KU32_STATIC_FLAG_VSYNC       0x00000002 /**< Ready flag */

#define orxDISPLAY_KU32_STATIC_MASK_ALL         0xFFFFFFFF /**< All mask */

orxSTATIC orxCONST orxU32     su32ScreenWidth   = 800;
orxSTATIC orxCONST orxU32     su32ScreenHeight  = 600;
orxSTATIC orxCONST orxBITMAP *spoScreen         = (orxCONST orxBITMAP *)0xFFFFFFFF;
orxSTATIC orxCONST orxSTRING  szTitle           = "orxTestWindow";

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxDISPLAY_STATIC_t
{
  orxU32            u32Flags;
  sf::RenderWindow *poRenderWindow;
  sf::IntRect       oScreenRectangle;
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

extern "C" orxBITMAP *orxDisplay_SFML_GetScreen()
{
  return const_cast<orxBITMAP *>(spoScreen);
}

extern "C" orxSTATUS orxDisplay_SFML_DrawText(orxCONST orxBITMAP *_pstBitmap, orxCONST orxBITMAP_TRANSFORM *_pstTransform, orxRGBA _stColor, orxCONST orxSTRING _zString)
{
  sf::String  oText;
  orxSTATUS   eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((_pstBitmap == spoScreen) && "Can only draw on screen with this version!");

  /* Sets text content */
  oText.SetText(_zString);

  /* Sets its color */
  oText.SetColor(sf::Color(orxRGBA_R(_stColor), orxRGBA_G(_stColor), orxRGBA_B(_stColor), orxRGBA_A(_stColor)));

  /* Sets its center */
  oText.SetRotationCenter(_pstTransform->s32SrcX, _pstTransform->s32SrcY);

  /* Sets its rotation */
  oText.SetRotation(-orxMATH_KF_RAD_TO_DEG * _pstTransform->fRotation);

  /* Sets its scale */
  oText.SetScale(_pstTransform->fScaleX, _pstTransform->fScaleY);

  /* Sets its position */
  oText.SetLeft(_pstTransform->s32DstX - _pstTransform->s32SrcX);
  oText.SetTop(_pstTransform->s32DstY - _pstTransform->s32SrcY);

  /* Draws it */
  sstDisplay.poRenderWindow->Draw(oText);

  /* Done! */
  return eResult;
}

extern "C" orxVOID orxDisplay_SFML_DeleteBitmap(orxBITMAP *_pstBitmap)
{
  sf::Sprite         *poSprite;
  orxCONST sf::Image *poImage;

  /* Checks */
  orxASSERT((_pstBitmap != orxNULL) && (_pstBitmap != spoScreen));

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

  return;
}

extern "C" orxBITMAP *orxDisplay_SFML_CreateBitmap(orxU32 _u32Width, orxU32 _u32Height)
{
  sf::Image  *poImage;
  sf::Sprite *poSprite;

  /* Creates image */
  poImage = new sf::Image(_u32Width, _u32Height);

  /* Deactivates smoothing */
  poImage->SetSmooth(orxFALSE);

  /* Creates sprite using the new image */
  poSprite = new sf::Sprite(*poImage);

  /* Done! */
  return (orxBITMAP *)poSprite;
}

extern "C" orxSTATUS orxDisplay_SFML_ClearBitmap(orxBITMAP *_pstBitmap, orxRGBA _stColor)
{
  sf::Color oColor;   
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(_pstBitmap != orxNULL);

  /* Gets color */
  oColor = sf::Color(orxRGBA_R(_stColor), orxRGBA_G(_stColor), orxRGBA_B(_stColor), orxRGBA_A(_stColor));

  /* Is not screen? */
  if(_pstBitmap != spoScreen)
  {
    sf::Image  *poImage;
    orxU32     *pu32Cursor, *pu32End, u32Color;

    /* Gets flat color */
    u32Color = oColor.ToRGBA();

    /* Gets image */
    poImage = const_cast<sf::Image *>(((sf::Sprite *)_pstBitmap)->GetImage());

    /* For all pixels */
    for(pu32Cursor = (orxU32 *)poImage->GetPixelsPtr(), pu32End = pu32Cursor + (poImage->GetWidth() * poImage->GetHeight());
        pu32Cursor < pu32End; pu32Cursor++)
    {
      /* Updates pixel */
      *pu32Cursor = u32Color;
    }

    /* Updates whole image */
    poImage->Update();
  }
  else
  {
    /* Sets rendering background color */
    sstDisplay.poRenderWindow->SetBackgroundColor(oColor);
  }

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxDisplay_SFML_Swap()
{
  sf::Event oEvent;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Displays render window */
  sstDisplay.poRenderWindow->Display();

  //! TEMP : Until orx events are ready
  while(sstDisplay.poRenderWindow->GetEvent(oEvent))
  {
    /* Depending on type */
    switch(oEvent.Type)
    {
      case sf::Event::Close:
      {
        /* Exits */
        exit(EXIT_SUCCESS);

        break;
      }

      case sf::Event::KeyPressed:
      {
        /* Depending on key */
        switch(oEvent.Key.Code)
        {
          case sf::Key::Escape:
          {
            /* Exits */
            exit(EXIT_SUCCESS);

            break;
          }

          case sf::Key::V:
          {
            /* Updates VSync flag */
            sstDisplay.u32Flags ^= orxDISPLAY_KU32_STATIC_FLAG_VSYNC;

            /* Updates VSync use */
            sstDisplay.poRenderWindow->UseVerticalSync((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_VSYNC) ? orxTRUE : orxFALSE);

            break;
          }

          default:
          {
            break;
          }
        }

        break;
      }

      default:
      {
        break;
      }
    }
  }

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxDisplay_SFML_SetBitmapColorKey(orxBITMAP *_pstBitmap, orxRGBA _stColor, orxBOOL _bEnable)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  sf::Image *poImage;

  /* Checks */
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

  /* Updates it */
  poImage->Update();

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxDisplay_SFML_BlitBitmap(orxBITMAP *_pstDst, orxCONST orxBITMAP *_pstSrc,  orxCONST orxS32 _s32PosX, orxS32 _s32PosY)
{
  sf::Sprite *poSprite;
  sf::IntRect oClippingRectangle, oSpriteRectangle;
  orxFLOAT    fLeft, fTop, fRight, fBottom, fInvScaleX, fInvScaleY;
  orxSTATUS   eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((_pstSrc != orxNULL) && (_pstSrc != spoScreen));
  orxASSERT((_pstDst == spoScreen) && "Can only draw on screen with this version!");

  /* Gets sprite */
  poSprite = (sf::Sprite *)_pstSrc;

  /* Gets sprite rectangle */
  oSpriteRectangle = poSprite->GetSubRect();

  /* Updates it */
  oSpriteRectangle.Left    += _s32PosX;
  oSpriteRectangle.Top     += _s32PosY;
  fRight                    = orxS2F(oSpriteRectangle.Right) * orx2F(poSprite->GetScaleX());
  oSpriteRectangle.Right    = orxF2S(fRight) + _s32PosX;
  fBottom                   = orxS2F(oSpriteRectangle.Bottom) * orx2F(poSprite->GetScaleY());
  oSpriteRectangle.Bottom   = orxF2S(fBottom) + _s32PosY;

  /* Gets clipping coordinates */
  if(sstDisplay.oScreenRectangle.Intersects(oSpriteRectangle, &oClippingRectangle) != orxFALSE)
  {
    /* Gets scale inv */
    fInvScaleX = orxFLOAT_1 / orx2F(poSprite->GetScaleX());
    fInvScaleY = orxFLOAT_1 / orx2F(poSprite->GetScaleY());

    /* Updates its position */
    poSprite->SetLeft(orxS2F(oClippingRectangle.Left));
    poSprite->SetTop(orxS2F(oClippingRectangle.Top));

    /* Updates clipping rectangle */
    oClippingRectangle.Left    -= _s32PosX;
    fLeft                       = orxS2F(oClippingRectangle.Left) * fInvScaleX;
    oClippingRectangle.Left     = orxF2S(fLeft);
    oClippingRectangle.Top     -= _s32PosY;
    fTop                        = orxS2F(oClippingRectangle.Top) * fInvScaleY;
    oClippingRectangle.Top      = orxF2S(fTop);
    oClippingRectangle.Right   -= _s32PosX;
    fRight                      = orxS2F(oClippingRectangle.Right) * fInvScaleX;
    oClippingRectangle.Right    = orxF2S(fRight);
    oClippingRectangle.Bottom  -= _s32PosY;
    fBottom                     = orxS2F(oClippingRectangle.Bottom) * fInvScaleY;
    oClippingRectangle.Bottom   = orxF2S(fBottom);

    /* Updates sprite sub-rectangle */
    poSprite->SetSubRect(oClippingRectangle);

    /* Draws it */
    sstDisplay.poRenderWindow->Draw(*poSprite);

    /* Resets sprite sub-rectangle */
    poSprite->SetSubRect(sf::IntRect(0, 0, poSprite->GetImage()->GetWidth(), poSprite->GetImage()->GetHeight()));
  }

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxDisplay_SFML_TransformBitmap(orxBITMAP *_pstDst, orxCONST orxBITMAP *_pstSrc, orxCONST orxBITMAP_TRANSFORM *_pstTransform, orxU32 _u32Flags)
{
  sf::Sprite *poSprite;
  orxSTATUS   eResult;

  /* Checks */
  orxASSERT((_pstSrc != orxNULL) && (_pstSrc != spoScreen));
  orxASSERT((_u32Flags == 0) && "Not used yet!")
  orxASSERT((_pstDst == spoScreen) && "Can only draw on screen with this version!");

  /* Gets sprite */
  poSprite = (sf::Sprite *)_pstSrc;

  /* Updates its center */
  poSprite->SetRotationCenter(_pstTransform->s32SrcX, _pstTransform->s32SrcY);

  /* Updates its rotation */
  poSprite->SetRotation(-orxMATH_KF_RAD_TO_DEG * _pstTransform->fRotation);

  /* Updates its scale */
  poSprite->SetScale(_pstTransform->fScaleX, _pstTransform->fScaleY);

  /* Blits it */
  eResult = orxDisplay_SFML_BlitBitmap(_pstDst, _pstSrc, _pstTransform->s32DstX - orxF2S(_pstTransform->fScaleX * orxS2F(_pstTransform->s32SrcX)), _pstTransform->s32DstY - orxF2S(_pstTransform->fScaleY * orxS2F(_pstTransform->s32SrcY)));

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxDisplay_SFML_SaveBitmap(orxCONST orxBITMAP *_pstBitmap, orxCONST orxSTRING _zFilename)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(_pstBitmap != orxNULL);

  /* Not screen? */
  if(_pstBitmap != spoScreen)
  {
    sf::Image *poImage;

    /* Gets image */
    poImage = const_cast<sf::Image *>(((sf::Sprite *)_pstBitmap)->GetImage());

    /* Saves it */
    eResult = (poImage->SaveToFile(_zFilename) != orxFALSE) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
  }
  else
  {
    /* Gets screen capture */
    eResult = (sstDisplay.poRenderWindow->Capture().SaveToFile(_zFilename) != orxFALSE) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

extern "C" orxBITMAP *orxDisplay_SFML_LoadBitmap(orxCONST orxSTRING _zFilename)
{
  orxBITMAP *pstResult;
  sf::Image *poImage;

  /* Creates empty image */
  poImage = new sf::Image();

  /* Loads it from file */
  if(poImage->LoadFromFile(_zFilename) != orxFALSE)
  {
    sf::Sprite *poSprite;

    /* Deactivates smoothing */
    poImage->SetSmooth(orxFALSE);

    /* Creates a sprite from it */
    poSprite = new sf::Sprite(*poImage);

    /* Updates result */
    pstResult = (orxBITMAP *)poSprite;
  }
  else
  {
    /* Updates result */
    pstResult = (orxBITMAP *)orxNULL;
  }

  /* Done! */
  return pstResult;
}

extern "C" orxSTATUS orxDisplay_SFML_GetBitmapSize(orxCONST orxBITMAP *_pstBitmap, orxU32 *_pu32Width, orxU32 *_pu32Height)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT(_pu32Width != orxNULL);
  orxASSERT(_pu32Height != orxNULL);
  
  /* Not screen? */
  if(_pstBitmap != spoScreen)
  {
    sf::Image *poImage;

    /* Gets image */
    poImage = const_cast<sf::Image *>(((sf::Sprite *)_pstBitmap)->GetImage());

    /* Gets size info */
    *_pu32Width  = poImage->GetWidth();
    *_pu32Height = poImage->GetHeight();
  }
  else
  {
    /* Gets siwe info */
    *_pu32Width  = sstDisplay.poRenderWindow->GetWidth();
    *_pu32Height = sstDisplay.poRenderWindow->GetHeight();
  }

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxDisplay_SFML_SetBitmapClipping(orxBITMAP *_pstBitmap, orxU32 _u32TLX, orxU32 _u32TLY, orxU32 _u32BRX, orxU32 _u32BRY)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT((_pstBitmap == spoScreen) && "Can only draw on screen with this version!");

  /* Stores screen clipping */
  sstDisplay.oScreenRectangle.Left    = _u32TLX;
  sstDisplay.oScreenRectangle.Top     = _u32TLY;
  sstDisplay.oScreenRectangle.Right   = _u32BRX;
  sstDisplay.oScreenRectangle.Bottom  = _u32BRY;

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxDisplay_SFML_Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Was already initialized. */
  if(!(sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Set(&sstDisplay, 0, sizeof(orxDISPLAY_STATIC));

    /* Inits rendering window */
    sstDisplay.poRenderWindow = new sf::RenderWindow(sf::VideoMode(su32ScreenWidth, su32ScreenHeight), szTitle, sf::RenderWindow::Fixed);

    /* Waits for vertical sync */
    sstDisplay.poRenderWindow->UseVerticalSync(orxFALSE);

    /* Updates status */
    sstDisplay.u32Flags |= orxDISPLAY_KU32_STATIC_FLAG_READY;
  }

  /* Done! */
  return eResult;  
}

extern "C" orxVOID orxDisplay_SFML_Exit()
{
  /* Was initialized? */
  if(sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY)
  {
    /* Deletes rendering window */
    delete sstDisplay.poRenderWindow;

    /* Cleans static controller */
    orxMemory_Set(&sstDisplay, 0, sizeof(orxDISPLAY_STATIC));
  }

  return;
}


/********************
 *  Plugin Related  *
 ********************/

orxPLUGIN_USER_CORE_FUNCTION_CPP_START(DISPLAY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_Init, DISPLAY, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_Exit, DISPLAY, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_Swap, DISPLAY, SWAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_CreateBitmap, DISPLAY, CREATE_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_DeleteBitmap, DISPLAY, DELETE_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_SaveBitmap, DISPLAY, SAVE_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_TransformBitmap, DISPLAY, TRANSFORM_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_LoadBitmap, DISPLAY, LOAD_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_GetBitmapSize, DISPLAY, GET_BITMAP_SIZE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_GetScreen, DISPLAY, GET_SCREEN_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_ClearBitmap, DISPLAY, CLEAR_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_SetBitmapClipping, DISPLAY, SET_BITMAP_CLIPPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_BlitBitmap, DISPLAY, BLIT_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_SetBitmapColorKey, DISPLAY, SET_BITMAP_COLOR_KEY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_DrawText, DISPLAY, DRAW_TEXT);
orxPLUGIN_USER_CORE_FUNCTION_END();
