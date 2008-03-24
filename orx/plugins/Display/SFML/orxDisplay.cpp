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

  #include "core/orxConfig.h"
  #include "math/orxMath.h"
  #include "plugin/orxPluginUser.h"
  #include "memory/orxBank.h"

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
orxSTATIC orxCONST orxBITMAP *spoScreen         = (orxCONST orxBITMAP *)0xFFFFFFFF;
orxSTATIC orxCONST orxSTRING  szTitle           = "orxTestWindow";
orxSTATIC orxCONST orxU32     su32TextBankSize  = 8;


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

  orxBANK          *pstTextBank;
  
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
  orxDISPLAY_TEXT  *pstText;
  orxSTATUS         eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstBitmap == spoScreen) && "Can only draw on screen with this version!");

  /* Gets a new text from bank */
  pstText = (orxDISPLAY_TEXT *)orxBank_Allocate(sstDisplay.pstTextBank);

  /* Valid? */
  if(pstText != orxNULL)
  {
    /* Sets config section */
    orxConfig_SelectSection(orxDISPLAY_KZ_CONFIG_SECTION);

    /* Allocates text */
    pstText->poString = new sf::String(_zString, orxConfig_GetString(orxDISPLAY_KZ_CONFIG_FONT));

    /* Sets its color */
    pstText->poString->SetColor(sf::Color(orxRGBA_R(_stColor), orxRGBA_G(_stColor), orxRGBA_B(_stColor), orxRGBA_A(_stColor)));

    /* Sets its center */
    pstText->poString->SetRotationCenter(_pstTransform->s32SrcX, _pstTransform->s32SrcY);

    /* Sets its rotation */
    pstText->poString->SetRotation(-orxMATH_KF_RAD_TO_DEG * _pstTransform->fRotation);

    /* Sets its scale */
    pstText->poString->SetScale(_pstTransform->fScaleX, _pstTransform->fScaleY);

    /* Sets its position */
    pstText->poString->SetLeft(_pstTransform->s32DstX - _pstTransform->s32SrcX);
    pstText->poString->SetTop(_pstTransform->s32DstY - _pstTransform->s32SrcY);
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

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Creates image */
  poImage = new sf::Image(_u32Width, _u32Height);

  /* Activates smoothing */
  poImage->SetSmooth(orxTRUE);

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

    /* Updates whole image */
//    poImage->Update();
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
  sf::Event         oEvent;
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

  //! TEMP : Until orx events are ready
  while(sstDisplay.poRenderWindow->GetEvent(oEvent))
  {
    /* Depending on type */
    switch(oEvent.Type)
    {
      case sf::Event::Closed:
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

  /* Updates it */
//  poImage->Update();

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxDisplay_SFML_BlitBitmap(orxBITMAP *_pstDst, orxCONST orxBITMAP *_pstSrc,  orxCONST orxS32 _s32PosX, orxS32 _s32PosY)
{
  sf::Sprite *poSprite;
  orxSTATUS   eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstSrc != orxNULL) && (_pstSrc != spoScreen));
  orxASSERT((_pstDst == spoScreen) && "Can only draw on screen with this version!");

  /* Gets sprite */
  poSprite = (sf::Sprite *)_pstSrc;

  /* Updates its position */
  poSprite->SetLeft(_s32PosX);
  poSprite->SetTop(_s32PosY);

  /* Draws it */
  sstDisplay.poRenderWindow->Draw(*poSprite);

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxDisplay_SFML_TransformBitmap(orxBITMAP *_pstDst, orxCONST orxBITMAP *_pstSrc, orxCONST orxBITMAP_TRANSFORM *_pstTransform, orxU32 _u32Flags)
{
  sf::Sprite *poSprite;
  orxSTATUS   eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstSrc != orxNULL) && (_pstSrc != spoScreen));
  orxASSERT((_u32Flags == 0) && "Not used yet!")
  orxASSERT((_pstDst == spoScreen) && "Can only draw on screen with this version!");

  /* Gets sprite */
  poSprite = (sf::Sprite *)_pstSrc;

  /* Updates its center */
  poSprite->SetRotationCenter(_pstTransform->fScaleX * _pstTransform->s32SrcX, _pstTransform->fScaleY * _pstTransform->s32SrcY);

  /* Updates its rotation */
  poSprite->SetRotation(-orxMATH_KF_RAD_TO_DEG * _pstTransform->fRotation);

  /* Updates its scale */
  poSprite->SetScale(_pstTransform->fScaleX, _pstTransform->fScaleY);

  /* Blits it */
  eResult = orxDisplay_SFML_BlitBitmap(_pstDst, _pstSrc, _pstTransform->s32DstX - orxF2S(_pstTransform->fScaleX * orxS2F(_pstTransform->s32SrcX)), _pstTransform->s32DstY - orxF2S(_pstTransform->fScaleY * orxS2F(_pstTransform->s32SrcY)));

  /* Updates its center */
  poSprite->SetRotationCenter(_pstTransform->fScaleX * _pstTransform->s32SrcX, _pstTransform->fScaleY * _pstTransform->s32SrcY);

  /* Resets its rotation */
  poSprite->SetRotation(0.0f);

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

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Creates empty image */
  poImage = new sf::Image();

  /* Loads it from file */
  if(poImage->LoadFromFile(_zFilename) != orxFALSE)
  {
    sf::Sprite *poSprite;

    /* Activates smoothing */
    poImage->SetSmooth(orxTRUE);

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
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
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
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT((_pstBitmap == spoScreen) && "Can only draw on screen with this version!");

  /* Stores screen clipping */
  glScissor(_u32TLX, sstDisplay.u32ScreenHeight - _u32BRY, _u32BRX - _u32TLX, _u32BRY - _u32TLY);

  /* Enables clipping */
  glEnable(GL_SCISSOR_TEST);

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxDisplay_SFML_Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Was not already initialized? */
  if(!(sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Set(&sstDisplay, 0, sizeof(orxDISPLAY_STATIC));

    /* Creates text bank */
    sstDisplay.pstTextBank = orxBank_Create(su32TextBankSize, sizeof(orxDISPLAY_TEXT), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN); 

    /* Valid? */
    if(sstDisplay.pstTextBank != orxNULL)
    {
      orxS32 s32ConfigWidth, s32ConfigHeight;

      /* Gets resolution from config */
      orxConfig_SelectSection(orxDISPLAY_KZ_CONFIG_SECTION);
      s32ConfigWidth  = orxConfig_GetS32(orxDISPLAY_KZ_CONFIG_WIDTH);
      s32ConfigHeight = orxConfig_GetS32(orxDISPLAY_KZ_CONFIG_HEIGHT);

      /* Not valid? */
      if((s32ConfigWidth <= 0) || (s32ConfigHeight <= 0) || ((sstDisplay.poRenderWindow = new sf::RenderWindow(sf::VideoMode(s32ConfigWidth, s32ConfigHeight), szTitle, sf::Style::Close)) == orxNULL))
      {
        /* Inits default rendering window */
        sstDisplay.poRenderWindow   = new sf::RenderWindow(sf::VideoMode(su32ScreenWidth, su32ScreenHeight), szTitle, sf::Style::Close);

        /* Stores values */
        sstDisplay.u32ScreenWidth   = su32ScreenWidth;
        sstDisplay.u32ScreenHeight  = su32ScreenHeight;
      }
      else
      {
        /* Stores values */
        sstDisplay.u32ScreenWidth   = s32ConfigWidth;
        sstDisplay.u32ScreenHeight  = s32ConfigHeight;
      }

      /* Waits for vertical sync */
      sstDisplay.poRenderWindow->UseVerticalSync(orxTRUE);

      /* Updates status */
      sstDisplay.u32Flags |= orxDISPLAY_KU32_STATIC_FLAG_READY | orxDISPLAY_KU32_STATIC_FLAG_VSYNC;
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
    /* Deletes rendering window */
    delete sstDisplay.poRenderWindow;

    /* Cleans static controller */
    orxMemory_Set(&sstDisplay, 0, sizeof(orxDISPLAY_STATIC));
  }

  return;
}

extern "C" orxHANDLE orxDisplay_SFML_GetApplicationInput()
{
  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  return((orxHANDLE)&(sstDisplay.poRenderWindow->GetInput()));
}


/********************
 *  Plugin Related  *
 ********************/

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
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_GetScreen, DISPLAY, GET_SCREEN_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_ClearBitmap, DISPLAY, CLEAR_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_SetBitmapClipping, DISPLAY, SET_BITMAP_CLIPPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_BlitBitmap, DISPLAY, BLIT_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_SetBitmapColorKey, DISPLAY, SET_BITMAP_COLOR_KEY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_DrawText, DISPLAY, DRAW_TEXT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SFML_GetApplicationInput, DISPLAY, GET_APPLICATION_INPUT);
orxPLUGIN_USER_CORE_FUNCTION_END();
