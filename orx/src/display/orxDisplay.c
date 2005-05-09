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


/********************
 *  Plugin Related  *
 ********************/

orxSTATIC orxCONST orxPLUGIN_CORE_FUNCTION sastDisplayPluginFunctionInfo[orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_NUMBER] =
{
  {(orxPLUGIN_FUNCTION *) &graph_init,                     orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_INIT},
  {(orxPLUGIN_FUNCTION *) &graph_exit,                     orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_EXIT},
  {(orxPLUGIN_FUNCTION *) &graph_switch,                   orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_SWITCH},
  {(orxPLUGIN_FUNCTION *) &orxDisplay_CreateBitmap,        orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_CREATE_BITMAP},
  {(orxPLUGIN_FUNCTION *) &graph_video_bitmap_create,      orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_CREATE_VIDEO_BITMAP},
  {(orxPLUGIN_FUNCTION *) &graph_delete,                   orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_DELETE_BITMAP},
  {(orxPLUGIN_FUNCTION *) &graph_bitmap_save,              orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_SAVE_BITMAP},
  {(orxPLUGIN_FUNCTION *) &graph_bitmap_load,              orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_LOAD_BITMAP},
  {(orxPLUGIN_FUNCTION *) &graph_bitmap_transform,         orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_TRANSFORM_BITMAP},
  {(orxPLUGIN_FUNCTION *) &graph_bitmap_color_key_set,     orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_SET_BITMAP_COLOR_KEY},
  {(orxPLUGIN_FUNCTION *) &graph_bitmap_size_get,          orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_GET_BITMAP_SIZE},
  {(orxPLUGIN_FUNCTION *) &graph_screen_bitmap_get,        orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_GET_SCREEN_BITMAP},
  {(orxPLUGIN_FUNCTION *) &graph_clip_set,                 orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_SET_CLIPPING},
  {(orxPLUGIN_FUNCTION *) &graph_clear,                    orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_CLEAR},
  {(orxPLUGIN_FUNCTION *) &graph_blit,                     orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_BLIT},
  {(orxPLUGIN_FUNCTION *) &graph_sprite_draw,              orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_DRAW_SPRITE},
  {(orxPLUGIN_FUNCTION *) &graph_printf,                   orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_PRINTF}
};

orxVOID graph_plugin_init()
{
  /* Plugin init */
  orxPlugin_AddCoreInfo(orxPLUGIN_CORE_ID_DISPLAY, sastDisplayPluginFunctionInfo, sizeof(sastDisplayPluginFunctionInfo) / sizeof(orxPLUGIN_CORE_FUNCTION));

  return;
}



/********************
 *   Core Related   *
 ********************/

orxPLUGIN_DEFINE_CORE_FUNCTION(graph_init, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(graph_exit, orxVOID);

orxPLUGIN_DEFINE_CORE_FUNCTION(graph_printf, orxVOID, orxBITMAP *, orxS32, orxS32, orxU32, orxCONST orxU8 *, ...);

orxPLUGIN_DEFINE_CORE_FUNCTION(graph_delete, orxVOID, orxBITMAP *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_CreateBitmap, orxBITMAP *, orxS32, orxS32);

orxPLUGIN_DEFINE_CORE_FUNCTION(graph_video_bitmap_create, orxBITMAP *, orxS32, orxS32);

orxPLUGIN_DEFINE_CORE_FUNCTION(graph_screen_bitmap_get, orxBITMAP *);

orxPLUGIN_DEFINE_CORE_FUNCTION(graph_clear, orxVOID, orxBITMAP *);
orxPLUGIN_DEFINE_CORE_FUNCTION(graph_switch, orxVOID);
orxPLUGIN_DEFINE_CORE_FUNCTION(graph_blit, orxVOID, orxBITMAP *, orxBITMAP *, orxS32, orxS32, orxS32, orxS32, orxS32, orxS32);
orxPLUGIN_DEFINE_CORE_FUNCTION(graph_clip_set, orxVOID, orxBITMAP *, orxS32, orxS32, orxS32, orxS32);


orxPLUGIN_DEFINE_CORE_FUNCTION(graph_sprite_draw, orxVOID, graph_st_sprite *, orxBITMAP *, orxS32, orxS32);

orxPLUGIN_DEFINE_CORE_FUNCTION(graph_bitmap_transform, orxVOID, orxBITMAP *, orxBITMAP *, orxFLOAT, orxFLOAT, orxFLOAT, orxS32, orxS32, orxS32, orxS32, orxBOOL);
orxPLUGIN_DEFINE_CORE_FUNCTION(graph_bitmap_color_key_set, orxVOID, orxBITMAP *, orxU32, orxU32, orxU32, orxBOOL);
orxPLUGIN_DEFINE_CORE_FUNCTION(graph_bitmap_save, orxVOID, orxCONST orxU8 *, orxBITMAP *);
orxPLUGIN_DEFINE_CORE_FUNCTION(graph_bitmap_load, orxBITMAP *, orxCONST orxU8 *);
orxPLUGIN_DEFINE_CORE_FUNCTION(graph_bitmap_size_get, orxVOID, orxBITMAP *, orxS32*, orxS32 *);
