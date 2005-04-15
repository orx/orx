/**
 * \file graph.c
 */

/***************************************************************************
 begin                : 23/04/2003
 author               : (C) Gdp
 email                : iarwain@ifrance.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "graph/graph.h"

#include "plugin/plugin_core.h"


/********************
 *  Plugin Related  *
 ********************/

orxSTATIC plugin_core_st_function plugin_graph_spst_function[PLUGIN_GRAPH_KU32_FUNCTION_NUMBER] =
{
  {(plugin_function *) &graph_init,                     PLUGIN_GRAPH_KU32_ID_INIT},
  {(plugin_function *) &graph_exit,                     PLUGIN_GRAPH_KU32_ID_EXIT},
  {(plugin_function *) &graph_switch,                   PLUGIN_GRAPH_KU32_ID_SWITCH},
  {(plugin_function *) &graph_create       ,            PLUGIN_GRAPH_KU32_ID_BITMAP_CREATE},
  {(plugin_function *) &graph_video_bitmap_create,      PLUGIN_GRAPH_KU32_ID_VIDEO_BITMAP_CREATE},
  {(plugin_function *) &graph_delete,                   PLUGIN_GRAPH_KU32_ID_BITMAP_DELETE},
  {(plugin_function *) &graph_bitmap_save,              PLUGIN_GRAPH_KU32_ID_BITMAP_SAVE},
  {(plugin_function *) &graph_bitmap_load,              PLUGIN_GRAPH_KU32_ID_BITMAP_LOAD},
  {(plugin_function *) &graph_bitmap_transform,         PLUGIN_GRAPH_KU32_ID_BITMAP_TRANSFORM},
  {(plugin_function *) &graph_bitmap_color_key_set,     PLUGIN_GRAPH_KU32_ID_BITMAP_COLOR_KEY_SET},
  {(plugin_function *) &graph_bitmap_size_get,          PLUGIN_GRAPH_KU32_ID_BITMAP_SIZE_GET},
  {(plugin_function *) &graph_screen_bitmap_get,        PLUGIN_GRAPH_KU32_ID_SCREEN_BITMAP_GET},
  {(plugin_function *) &graph_clip_set,                 PLUGIN_GRAPH_KU32_ID_CLIP_SET},
  {(plugin_function *) &graph_clear,                    PLUGIN_GRAPH_KU32_ID_CLEAR},
  {(plugin_function *) &graph_blit,                     PLUGIN_GRAPH_KU32_ID_BLIT},
  {(plugin_function *) &graph_sprite_draw,              PLUGIN_GRAPH_KU32_ID_SPRITE_DRAW},
  {(plugin_function *) &graph_printf,                   PLUGIN_GRAPH_KU32_ID_PRINTF}
};

orxVOID graph_plugin_init()
{
  /* Plugin init */
  plugin_core_info_add(PLUGIN_GRAPH_KU32_PLUGIN_ID, plugin_graph_spst_function, PLUGIN_GRAPH_KU32_FUNCTION_NUMBER);

  return;
}



/********************
 *   Core Related   *
 ********************/

PLUGIN_CORE_FUNCTION_DEFINE(graph_init, orxU32);
PLUGIN_CORE_FUNCTION_DEFINE(graph_exit, orxVOID);

PLUGIN_CORE_FUNCTION_DEFINE(graph_printf, orxVOID, orxBITMAP *, orxS32, orxS32, orxU32, orxCONST orxU8 *, ...);

PLUGIN_CORE_FUNCTION_DEFINE(graph_delete, orxVOID, orxBITMAP *);
PLUGIN_CORE_FUNCTION_DEFINE(graph_create, orxBITMAP *, orxS32, orxS32);

PLUGIN_CORE_FUNCTION_DEFINE(graph_video_bitmap_create, orxBITMAP *, orxS32, orxS32);

PLUGIN_CORE_FUNCTION_DEFINE(graph_screen_bitmap_get, orxBITMAP *);

PLUGIN_CORE_FUNCTION_DEFINE(graph_clear, orxVOID, orxBITMAP *);
PLUGIN_CORE_FUNCTION_DEFINE(graph_switch, orxVOID);
PLUGIN_CORE_FUNCTION_DEFINE(graph_blit, orxVOID, orxBITMAP *, orxBITMAP *, orxS32, orxS32, orxS32, orxS32, orxS32, orxS32);
PLUGIN_CORE_FUNCTION_DEFINE(graph_clip_set, orxVOID, orxBITMAP *, orxS32, orxS32, orxS32, orxS32);


PLUGIN_CORE_FUNCTION_DEFINE(graph_sprite_draw, orxVOID, graph_st_sprite *, orxBITMAP *, orxS32, orxS32);

PLUGIN_CORE_FUNCTION_DEFINE(graph_bitmap_transform, orxVOID, orxBITMAP *, orxBITMAP *, orxFLOAT, orxFLOAT, orxFLOAT, orxS32, orxS32, orxS32, orxS32, orxBOOL);
PLUGIN_CORE_FUNCTION_DEFINE(graph_bitmap_color_key_set, orxVOID, orxBITMAP *, orxU32, orxU32, orxU32, orxBOOL);
PLUGIN_CORE_FUNCTION_DEFINE(graph_bitmap_save, orxVOID, orxCONST orxU8 *, orxBITMAP *);
PLUGIN_CORE_FUNCTION_DEFINE(graph_bitmap_load, orxBITMAP *, orxCONST orxU8 *);
PLUGIN_CORE_FUNCTION_DEFINE(graph_bitmap_size_get, orxVOID, orxBITMAP *, orxS32*, orxS32 *);
