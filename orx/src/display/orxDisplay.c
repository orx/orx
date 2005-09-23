/**
 * \file orxDisplay.c
 */

/***************************************************************************
 begin                : 23/04/2003
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

#include "display/orxDisplay.h"
#include "plugin/orxPluginCore.h"


/***************************************************************************
 orxDisplay_Setup
 Display module setup.

 returns: nothing
 ***************************************************************************/
orxVOID orxDisplay_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_DISPLAY, orxMODULE_ID_PLUGIN);
  orxModule_AddDependency(orxMODULE_ID_DISPLAY, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_DISPLAY, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_DISPLAY, orxMODULE_ID_LINKLIST);
  orxModule_AddDependency(orxMODULE_ID_DISPLAY, orxMODULE_ID_TREE);

  return;
}


/********************
 *  Plugin Related  *
 ********************/

/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(DISPLAY)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, INIT, orxDisplay_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, EXIT, orxDisplay_Exit)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SWAP, orxDisplay_Swap)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, CREATE_BITMAP, orxDisplay_CreateBitmap)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, DELETE_BITMAP, orxDisplay_DeleteBitmap)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SAVE_BITMAP, orxDisplay_SaveBitmap)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, LOAD_BITMAP, orxDisplay_LoadBitmap)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, TRANSFORM_BITMAP, orxDisplay_TransformBitmap)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, CLEAR_BITMAP, orxDisplay_ClearBitmap)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, BLIT_BITMAP, orxDisplay_BlitBitmap)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SET_BITMAP_COLOR_KEY, orxDisplay_SetBitmapColorKey)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SET_BITMAP_CLIPPING, orxDisplay_SetBitmapClipping)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, GET_BITMAP_SIZE, orxDisplay_GetBitmapSize)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, GET_SCREEN_BITMAP, orxDisplay_GetScreenBitmap)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, DRAW_TEXT, orxDisplay_DrawText)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(DISPLAY)


/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION_0(orxDisplay_Init, orxSTATUS);
orxPLUGIN_DEFINE_CORE_FUNCTION_0(orxDisplay_Exit, orxVOID);

orxPLUGIN_DEFINE_CORE_FUNCTION_0(orxDisplay_Swap, orxSTATUS);

orxPLUGIN_DEFINE_CORE_FUNCTION_4(orxDisplay_DrawText, orxSTATUS, orxCONST orxBITMAP *, orxCONST orxVEC *, orxARGB, orxCONST orxSTRING);

orxPLUGIN_DEFINE_CORE_FUNCTION_2(orxDisplay_CreateBitmap, orxBITMAP *, orxCONST orxVEC *, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION_1(orxDisplay_DeleteBitmap, orxVOID, orxBITMAP *);

orxPLUGIN_DEFINE_CORE_FUNCTION_0(orxDisplay_GetScreenBitmap, orxBITMAP *);

orxPLUGIN_DEFINE_CORE_FUNCTION_2(orxDisplay_ClearBitmap, orxSTATUS, orxBITMAP *, orxARGB);
orxPLUGIN_DEFINE_CORE_FUNCTION_4(orxDisplay_TransformBitmap, orxSTATUS, orxBITMAP *, orxCONST orxBITMAP *, orxCONST orxBITMAP_TRANSFORM *, orxU32);

orxPLUGIN_DEFINE_CORE_FUNCTION_3(orxDisplay_SetBitmapColorKey, orxSTATUS, orxBITMAP *, orxARGB, orxBOOL);
orxPLUGIN_DEFINE_CORE_FUNCTION_3(orxDisplay_SetBitmapClipping, orxSTATUS, orxBITMAP *, orxCONST orxVEC *, orxCONST orxVEC *);

orxPLUGIN_DEFINE_CORE_FUNCTION_5(orxDisplay_BlitBitmap, orxSTATUS, orxBITMAP *, orxCONST orxBITMAP *, orxCONST orxVEC *, orxCONST orxVEC *, orxCONST orxVEC *);

orxPLUGIN_DEFINE_CORE_FUNCTION_2(orxDisplay_SaveBitmap, orxSTATUS, orxCONST orxBITMAP *, orxCONST orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION_1(orxDisplay_LoadBitmap, orxBITMAP *, orxCONST orxSTRING);

orxPLUGIN_DEFINE_CORE_FUNCTION_2(orxDisplay_GetBitmapSize, orxSTATUS, orxCONST orxBITMAP *, orxVEC *);
