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

#include "orxInclude.h"

#include "msg/msg_graph.h"


extern orxVOID               graph_plugin_init();

extern orxU32               (*graph_init)();
extern orxVOID             (*graph_exit)();
extern orxVOID             (*graph_switch)();

extern orxVOID             (*graph_printf)(graph_st_bitmap *_pst_bmp, orxS32 _i_x, orxS32 _i_y, orxU32 _u32_color, orxCONST orxU8 *_zFormat, ...);

extern graph_st_bitmap *(*graph_create)(orxS32 _i_w, orxS32 _i_h);
extern graph_st_bitmap *(*graph_video_bitmap_create)(orxS32 _i_w, orxS32 _i_h);
extern orxVOID             (*graph_delete)(graph_st_bitmap *_pst_bmp);
extern graph_st_bitmap *(*graph_screen_bitmap_get)();
extern orxVOID             (*graph_clear)(graph_st_bitmap *_pst_bmp);
extern orxVOID             (*graph_bitmap_transform)(graph_st_bitmap *_pst_src, graph_st_bitmap *_pst_dst, orxFLOAT _fRotation, orxFLOAT _fScale_x, orxFLOAT _fScale_y, orxS32 _i_src_x, orxS32 _i_src_y, orxS32 _i_dst_x, orxS32 _i_dst_y, orxBOOL _bAntialiased);
extern orxVOID             (*graph_bitmap_color_key_set)(graph_st_bitmap *_pst_src, orxU32 _u32_red, orxU32 _u32_green, orxU32 _u32_blue, orxBOOL _b_enable);

extern orxVOID             (*graph_clip_set)(graph_st_bitmap *_pst_bmp, orxS32 _i_x, orxS32 _i_y, orxS32 _i_w, orxS32 _i_h);

extern orxVOID             (*graph_blit)(graph_st_bitmap *_pst_src, graph_st_bitmap *_pst_dst, orxS32 _i_src_x, orxS32 _i_src_y, orxS32 _i_dst_x, orxS32 _i_dst_y, orxS32 _i_w, orxS32 _i_h);

extern orxVOID             (*graph_sprite_draw)(graph_st_sprite *_pst_src, graph_st_bitmap *_pst_dst, orxS32 _i_x, orxS32 _i_y);

extern orxVOID             (*graph_bitmap_save)(orxCONST orxU8 *_zFilename, graph_st_bitmap *_pst_bmp);
extern graph_st_bitmap *(*graph_bitmap_load)(orxCONST orxU8 *_zFilename);

extern orxVOID             (*graph_bitmap_size_get)(graph_st_bitmap *_pstBitmap, orxS32 *_pi_width, orxS32 *_pi_height);

#endif /* _GRAPH_H_ */
