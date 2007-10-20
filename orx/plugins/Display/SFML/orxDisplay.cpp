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

#include "debug/orxDebug.h"
#include "math/orxMath.h"
#include "plugin/orxPluginUser.h"

#include "msg/msg_graph.h"

#include "display/orxDisplay.h"
}

#include <SFML/Graphics.hpp>


/** Module flags
 */
#define orxDISPLAY_KU32_STATIC_FLAG_NONE        0x00000000 /**< No flags */

#define orxDISPLAY_KU32_STATIC_FLAG_READY       0x00000001 /**< Ready flag */

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

extern "C" orxSTATUS orxDisplay_SFML_DrawText(orxCONST orxBITMAP *_pstBitmap, orxCONST orxVECTOR *_pvPos, orxRGBA _stColor, orxCONST orxSTRING _zFormat)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((_pstBitmap == spoScreen) && "Can only draw on screen with this version!");

  /* TODO :
 * Write the string onto screen, using char per char pixel writing
 */

  orxASSERT(orxFALSE && "Not implemented yet!");

  /* Done! */
  return eResult;
}

extern "C" orxVOID orxDisplay_SFML_DeleteBitmap(orxBITMAP *_pstBitmap)
{
  sf::Image *poImage;

  /* Checks */
  orxASSERT((_pstBitmap != orxNULL) && (_pstBitmap != spoScreen));

  /* Gets image */
  poImage = (sf::Image *)_pstBitmap;

  /* Deletes it */
  delete poImage;

  return;
}

extern "C" orxBITMAP *orxDisplay_SFML_CreateBitmap(orxU32 _u32Width, orxU32 _u32Height)
{
  sf::Image *poImage;

  /* Creates image */
  poImage = new sf::Image(_u32Width, _u32Height);

  /* Done! */
  return (orxBITMAP *)poImage;
}

extern "C" orxSTATUS orxDisplay_SFML_ClearBitmap(orxBITMAP *_pstBitmap, orxRGBA _stColor)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  sf::Image  *poImage;
  orxU32     *pu32Cursor, *pu32End;
  orxU32      u32Color;   

  /* Checks */
  orxASSERT(_pstBitmap != orxNULL);

  /* Is not screen? */
  if(_pstBitmap != spoScreen)
  {
    /* Gets image */
    poImage = (sf::Image *)_pstBitmap;

    /* Gets color */
    u32Color = sf::Color(orxRGBA_R(_stColor), orxRGBA_G(_stColor), orxRGBA_B(_stColor), orxRGBA_A(_stColor)).ToRGBA();
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

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxDisplay_SFML_Swap()
{
  sf::Event oEvent;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Displays render window */
  sstDisplay.poRenderWindow->Display();

  /* Polls all the event */
  while(sstDisplay.poRenderWindow->GetEvent(oEvent))
  {
    //! TEMP
    if(oEvent.Type == sf::Event::Close)
    {
      exit(EXIT_SUCCESS);
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
  poImage = (sf::Image *)_pstBitmap;

  /* Enable? */
  if(_bEnable != orxFALSE)
  {
    /* Creates transparency mask */
    poImage->CreateMaskFromColor(sf::Color(orxRGBA_R(_stColor), orxRGBA_G(_stColor), orxRGBA_B(_stColor), orxRGBA_A(_stColor)));
  }
  else
  {
    /* Clears transparency mask */
    poImage->CreateMaskFromColor(sf::Color(0), 0xFF);
  }

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxDisplay_SFML_BlitBitmap(orxBITMAP *_pstDst, orxCONST orxBITMAP *_pstSrc, orxCONST orxVECTOR *_pvDstCoord)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((_pstSrc != orxNULL) && (_pstSrc != spoScreen));
  orxASSERT((_pstDst == spoScreen) && "Can only draw on screen with this version!");

  //! TODO
  eResult = orxSTATUS_FAILURE;
  return eResult;
}

extern "C" orxSTATUS orxDisplay_SFML_TransformBitmap(orxBITMAP *_pstDst, orxCONST orxBITMAP *_pstSrc, orxCONST orxBITMAP_TRANSFORM *_pstTransform, orxU32 _u32Flags)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((_pstSrc != orxNULL) && (_pstSrc != spoScreen));
  orxASSERT((_pstDst == spoScreen) && "Can only draw on screen with this version!");

  //! TODO
  eResult = orxSTATUS_FAILURE;
  return eResult;
}

extern "C" orxSTATUS orxDisplay_SFML_SaveBitmap(orxCONST orxBITMAP *_pstBitmap, orxCONST orxSTRING _zFilename)
{
  orxSTATUS   eResult;
  sf::Image  *poImage;

  /* Checks */
  orxASSERT(_pstBitmap != orxNULL);

  /* Gets image */
  poImage = (sf::Image *)_pstBitmap;

  /* Saves it */
  eResult = (poImage->SaveToFile(_zFilename) != orxFALSE) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
  
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
    /* Updates result */
    pstResult = (orxBITMAP *)poImage;
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
  orxSTATUS eResult;

  orxASSERT(_pu32Width != orxNULL);
  orxASSERT(_pu32Height != orxNULL);
  
  /* Non null? */
  if(_pstBitmap != NULL)
  {
    sf::Image *poImage;

    /* Gets image */
    poImage = (sf::Image *)_pstBitmap;

    /* Gets size info */
    *_pu32Width  = poImage->GetWidth();
    *_pu32Height = poImage->GetHeight();

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* !!! MSG !!! */

    /* Null pointer -> cleans size values */
    *_pu32Width  = 0;
    *_pu32Height = 0;

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxDisplay_SFML_SetBitmapClipping(orxBITMAP *_pstBitmap, orxU32 _u32TLX, orxU32 _u32TLY, orxU32 _u32BRX, orxU32 _u32BRY)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((_pstBitmap == spoScreen) && "Can only draw on screen with this version!");

  //! TODO
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

    orxLOG("Video mode (%d x %d) was initialized succesfully!", su32ScreenWidth, su32ScreenHeight);

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
