/**
 * \file plugin_graph.h
 * This header is used to define ID for graph plugin registration.
 */

/*
 begin                : 23/04/2003
 author               : (C) Gdp
 email                : iarwain@ifrance.com
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _PLUGIN_GRAPH_H_
#define _PLUGIN_GRAPH_H_

/*********************************************
 Constants
 *********************************************/

#define PLUGIN_GRAPH_KUL_PLUGIN_ID              0x10000000

#define PLUGIN_GRAPH_KUL_FUNCTION_NUMBER        0x00000011

#define PLUGIN_GRAPH_KUL_ID_INIT                    0x10000000
#define PLUGIN_GRAPH_KUL_ID_EXIT                    0x10000001
#define PLUGIN_GRAPH_KUL_ID_SWITCH                  0x10000002
#define PLUGIN_GRAPH_KUL_ID_BITMAP_CREATE           0x10000003
#define PLUGIN_GRAPH_KUL_ID_VIDEO_BITMAP_CREATE     0x10000004
#define PLUGIN_GRAPH_KUL_ID_BITMAP_DELETE           0x10000005
#define PLUGIN_GRAPH_KUL_ID_BITMAP_SAVE             0x10000006
#define PLUGIN_GRAPH_KUL_ID_BITMAP_LOAD             0x10000007
#define PLUGIN_GRAPH_KUL_ID_BITMAP_TRANSFORM        0x10000008
#define PLUGIN_GRAPH_KUL_ID_BITMAP_COLOR_KEY_SET    0x10000009
#define PLUGIN_GRAPH_KUL_ID_BITMAP_SIZE_GET         0x1000000A
#define PLUGIN_GRAPH_KUL_ID_SCREEN_BITMAP_GET       0x1000000B
#define PLUGIN_GRAPH_KUL_ID_CLIP_SET                0x1000000C
#define PLUGIN_GRAPH_KUL_ID_CLEAR                   0x1000000D
#define PLUGIN_GRAPH_KUL_ID_BLIT                    0x1000000E
#define PLUGIN_GRAPH_KUL_ID_SPRITE_DRAW             0x1000000F
#define PLUGIN_GRAPH_KUL_ID_PRINTF                  0x10000010

#endif /* _PLUGIN_GRAPH_H_ */
