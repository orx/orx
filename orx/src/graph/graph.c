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

static plugin_core_st_function plugin_graph_spst_function[PLUGIN_GRAPH_KUL_FUNCTION_NUMBER] =
{
  {(plugin_function *) &graph_init,                     PLUGIN_GRAPH_KUL_ID_INIT},
  {(plugin_function *) &graph_exit,                     PLUGIN_GRAPH_KUL_ID_EXIT},
  {(plugin_function *) &graph_switch,                   PLUGIN_GRAPH_KUL_ID_SWITCH},
  {(plugin_function *) &graph_bitmap_create,            PLUGIN_GRAPH_KUL_ID_BITMAP_CREATE},
  {(plugin_function *) &graph_video_bitmap_create,      PLUGIN_GRAPH_KUL_ID_VIDEO_BITMAP_CREATE},
  {(plugin_function *) &graph_bitmap_delete,            PLUGIN_GRAPH_KUL_ID_BITMAP_DELETE},
  {(plugin_function *) &graph_bitmap_save,              PLUGIN_GRAPH_KUL_ID_BITMAP_SAVE},
  {(plugin_function *) &graph_bitmap_load,              PLUGIN_GRAPH_KUL_ID_BITMAP_LOAD},
  {(plugin_function *) &graph_bitmap_transform,         PLUGIN_GRAPH_KUL_ID_BITMAP_TRANSFORM},
  {(plugin_function *) &graph_bitmap_color_key_set,     PLUGIN_GRAPH_KUL_ID_BITMAP_COLOR_KEY_SET},
  {(plugin_function *) &graph_bitmap_size_get,          PLUGIN_GRAPH_KUL_ID_BITMAP_SIZE_GET},
  {(plugin_function *) &graph_screen_bitmap_get,        PLUGIN_GRAPH_KUL_ID_SCREEN_BITMAP_GET},
  {(plugin_function *) &graph_clip_set,                 PLUGIN_GRAPH_KUL_ID_CLIP_SET},
  {(plugin_function *) &graph_clear,                    PLUGIN_GRAPH_KUL_ID_CLEAR},
  {(plugin_function *) &graph_blit,                     PLUGIN_GRAPH_KUL_ID_BLIT},
  {(plugin_function *) &graph_sprite_draw,              PLUGIN_GRAPH_KUL_ID_SPRITE_DRAW},
  {(plugin_function *) &graph_printf,                   PLUGIN_GRAPH_KUL_ID_PRINTF}
};

void graph_plugin_init()
{
  /* Plugin init */
  plugin_core_info_add(PLUGIN_GRAPH_KUL_PLUGIN_ID, plugin_graph_spst_function, PLUGIN_GRAPH_KUL_FUNCTION_NUMBER);

  return;
}



/********************
 *   Core Related   *
 ********************/

PLUGIN_CORE_FUNCTION_DEFINE(graph_init, uint32);
PLUGIN_CORE_FUNCTION_DEFINE(graph_exit, void);

PLUGIN_CORE_FUNCTION_DEFINE(graph_printf, void, graph_st_bitmap *, int32, int32, uint32, const char *, ...);

PLUGIN_CORE_FUNCTION_DEFINE(graph_video_bitmap_create, graph_st_bitmap *, int32, int32);

PLUGIN_CORE_FUNCTION_DEFINE(graph_screen_bitmap_get, graph_st_bitmap *);

PLUGIN_CORE_FUNCTION_DEFINE(graph_clear, void, graph_st_bitmap *);
PLUGIN_CORE_FUNCTION_DEFINE(graph_switch, void);
PLUGIN_CORE_FUNCTION_DEFINE(graph_blit, void, graph_st_bitmap *, graph_st_bitmap *, int32, int32, int32, int32, int32, int32);
PLUGIN_CORE_FUNCTION_DEFINE(graph_clip_set, void, graph_st_bitmap *, int32, int32, int32, int32);


PLUGIN_CORE_FUNCTION_DEFINE(graph_sprite_draw, void, graph_st_sprite *, graph_st_bitmap *, int32, int32);

PLUGIN_CORE_FUNCTION_DEFINE(graph_bitmap_create, graph_st_bitmap *, int32, int32);
PLUGIN_CORE_FUNCTION_DEFINE(graph_bitmap_delete, void, graph_st_bitmap *);
PLUGIN_CORE_FUNCTION_DEFINE(graph_bitmap_transform, void, graph_st_bitmap *, graph_st_bitmap *, float, float, float, int32, int32, int32, int32, bool);
PLUGIN_CORE_FUNCTION_DEFINE(graph_bitmap_color_key_set, void, graph_st_bitmap *, uint32, uint32, uint32, bool);
PLUGIN_CORE_FUNCTION_DEFINE(graph_bitmap_save, void, const char *, graph_st_bitmap *);
PLUGIN_CORE_FUNCTION_DEFINE(graph_bitmap_load, graph_st_bitmap *, const char *);
PLUGIN_CORE_FUNCTION_DEFINE(graph_bitmap_size_get, void, graph_st_bitmap *, int32*, int32 *);
