/**
 * \file graph.h
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

#ifndef _GRAPH_H_
#define _GRAPH_H_

#include "include.h"

#include "msg/msg_graph.h"


extern void               graph_plugin_init();

extern uint32           (*graph_init)();
extern void             (*graph_exit)();
extern void             (*graph_switch)();

extern void             (*graph_printf)(graph_st_bitmap *_pst_bmp, int32 _i_x, int32 _i_y, uint32 _u32_color, const char *_z_format, ...);

extern graph_st_bitmap *(*graph_bitmap_create)(int32 _i_w, int32 _i_h);
extern graph_st_bitmap *(*graph_video_bitmap_create)(int32 _i_w, int32 _i_h);
extern void             (*graph_bitmap_delete)(graph_st_bitmap *_pst_bmp);
extern graph_st_bitmap *(*graph_screen_bitmap_get)();
extern void             (*graph_clear)(graph_st_bitmap *_pst_bmp);
extern void             (*graph_bitmap_transform)(graph_st_bitmap *_pst_src, graph_st_bitmap *_pst_dst, float _f_rotation, float _f_scale_x, float _f_scale_y, int32 _i_src_x, int32 _i_src_y, int32 _i_dst_x, int32 _i_dst_y, bool _b_antialiased);
extern void             (*graph_bitmap_color_key_set)(graph_st_bitmap *_pst_src, uint32 _u32_red, uint32 _u32_green, uint32 _u32_blue, bool _b_enable);

extern void             (*graph_clip_set)(graph_st_bitmap *_pst_bmp, int32 _i_x, int32 _i_y, int32 _i_w, int32 _i_h);

extern void             (*graph_blit)(graph_st_bitmap *_pst_src, graph_st_bitmap *_pst_dst, int32 _i_src_x, int32 _i_src_y, int32 _i_dst_x, int32 _i_dst_y, int32 _i_w, int32 _i_h);

extern void             (*graph_sprite_draw)(graph_st_sprite *_pst_src, graph_st_bitmap *_pst_dst, int32 _i_x, int32 _i_y);

extern void             (*graph_bitmap_save)(const char *_z_filename, graph_st_bitmap *_pst_bmp);
extern graph_st_bitmap *(*graph_bitmap_load)(const char *_z_filename);

extern void             (*graph_bitmap_size_get)(graph_st_bitmap *_pst_bitmap, int32 *_pi_width, int32 *_pi_height);

#endif /* _GRAPH_H_ */
