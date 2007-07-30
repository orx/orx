///**
// * \file graph_plug.c
// */
//
///***************************************************************************
// begin                : 14/11/2003
// author               : (C) Arcallians
// email                : iarwain@arcallians.org
// ***************************************************************************/
//
///***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************/
//
//#include "orxInclude.h"
//
//#include "debug/orxDebug.h"
//#include "plugin/orxPluginUser.h"
//
//#include "msg/msg_graph.h"
//
//#include <allegro.h>
//
//
//#define KI_BPP 16
//#define KI_ALT_BPP 8
//#define KI_WIDTH 800
//#define KI_HEIGHT 600
//
//#define KI_DOUBLE_BUFFER 1
//#define KI_PAGE_FLIP 2
//
//
///********************
// *   Core Related   *
// ********************/
//
//static BITMAP *spst_page1 = NULL;
//static BITMAP *spst_page2 = NULL;
//BITMAP *gpst_current;
//
//int32 gi_graph_animation_mode = 0;
//
//BITMAP *graph_screen_get()
//{
//  return gpst_current;
//}
//
//void orxDisplay_DrawText(BITMAP *_pst_bmp, int32 _i_x, int32 _i_y, uint32 _u32_color, const char *_z_format, ...)
//{
//  char ac_buf[1024];
//  va_list st_args;
//
//  va_start(st_args, _z_format);
//  uvszprintf(ac_buf, sizeof(ac_buf), _z_format, st_args);
//  va_end(st_args);
//  
//  textout(_pst_bmp, font, ac_buf, _i_x, _i_y, _u32_color);
//
//  return;
//}
//
//void graph_bitmap_delete(BITMAP *_pst_bmp)
//{
//  destroy_bitmap(_pst_bmp);
//  return;
//}
//
//BITMAP *graph_bitmap_create(int32 _i_w, int32 _i_h)
//{
//  return create_bitmap(_i_w, _i_h);
//}
//
//BITMAP *orxDisplay_CreateVideoBitmap(int32 _i_w, int32 _i_h)
//{
//  return create_video_bitmap(_i_w, _i_h);
//}
//
//void orxDisplay_ClearBitmap(BITMAP *_pst_bmp)
//{
//  clear(_pst_bmp);
//
//  return;
//}
//
//void orxDisplay_Swap(void)
//{
//  switch(gi_graph_animation_mode)
//  {
//    case KI_DOUBLE_BUFFER:
//#ifdef WINDOWS
//      acquire_screen();
//#endif
//      vsync();
//      blit(gpst_current, screen, 0, 0, 0, 0, KI_WIDTH, KI_HEIGHT);
//      vsync();
//#ifdef WINDOWS
//      release_screen();
//#endif
//      break;
//    
//    case KI_PAGE_FLIP:
//      show_video_bitmap(gpst_current);
//
//      gpst_current = (gpst_current == spst_page1) ? spst_page2 : spst_page1;
//
//      break;
//      
//    default:
//      DEBUG(D_GRAPH, KZ_MSG_NO_MODE_FOUND);
//      break;
//    }
//  
//  return;
//}
//
//void graph_masked_blit(BITMAP *_pst_src, BITMAP *_pst_dst, int32 _i_src_x, int32 _i_src_y, int32 _i_dst_x, int32 _i_dst_y, int32 _i_w, int32 _i_h)
//{
//  masked_blit(_pst_src, _pst_dst, _i_src_x, _i_src_y, _i_dst_x, _i_dst_y, _i_w, _i_h);
//
//  return;
//}
//
//void orxDisplay_BlitBitmap(BITMAP *_pst_src, BITMAP *_pst_dst, int32 _i_src_x, int32 _i_src_y, int32 _i_dst_x, int32 _i_dst_y, int32 _i_w, int32 _i_h)
//{
//  blit(_pst_src, _pst_dst, _i_src_x, _i_src_y, _i_dst_x, _i_dst_y, _i_w, _i_h);
//
//  return;
//}
//
//void graph_draw_sprite(RLE_SPRITE *_pst_src, BITMAP *_pst_dst, int32 x, int32 y)
//{
//  draw_rle_sprite(_pst_dst, _pst_src, x, y);
//
//  return;
//}
//
//void orxDisplay_SaveBitmap(const char *filename, BITMAP *_pst_bmp)
//{
//  save_bitmap(filename, _pst_bmp, NULL);
//
//  return;
//}
//
//void orxDisplay_SetBitmapClipping(BITMAP *_pst_bmp, int32 _i_x, int32 _i_y, int32 _i_w, int32 _i_h)
//{
//  /* !!! TODO !!! */
//  return;
//}
//
//int32 graph_init(void)
//{
//  set_color_depth(KI_BPP);
//
//  set_color_conversion(COLORCONV_TOTAL);
//
//  if(set_gfx_mode(GFX_AUTODETECT_FULLSCREEN, KI_WIDTH, KI_HEIGHT, 0, 0))
//  {
//    DEBUG(D_GRAPH, KZ_MSG_MODE_INIT_FAILED_III, KI_WIDTH, KI_HEIGHT, KI_BPP);
//      
//    /* Essai avec la profondeur alternative */
//    set_color_depth(KI_ALT_BPP);
//    if(set_gfx_mode(GFX_AUTODETECT_FULLSCREEN, KI_WIDTH, KI_HEIGHT, 0, 0))
//    {
//      DEBUG(D_GRAPH, KZ_MSG_MODE_INIT_FAILED_III, KI_WIDTH, KI_HEIGHT, KI_ALT_BPP);
//
//      return EXIT_FAILURE;
//    }
//    else
//    {
//      DEBUG(D_GRAPH, KZ_MSG_MODE_INIT_SUCCESS_III, KI_WIDTH, KI_HEIGHT, KI_ALT_BPP);
//    }
//  }
//  
//  if(!(spst_page1 = orxDisplay_CreateVideoBitmap(KI_WIDTH, KI_HEIGHT)) || !(spst_page2 = orxDisplay_CreateVideoBitmap(KI_WIDTH, KI_HEIGHT)))
//  {
//    if(spst_page1)
//    {
//      graph_bitmap_delete(spst_page1);
//      DEBUG(D_GRAPH, KZ_MSG_MODE_PAGEFLIP_FAILED);
//    }
//
//    if(!(spst_page1 = graph_bitmap_create(KI_WIDTH, KI_HEIGHT)))
//    {
//      DEBUG(D_GRAPH, KZ_MSG_MODE_DOUBLEBUFFER_FAILED);
//      return EXIT_FAILURE;
//    }
//    else
//    {
//      gi_graph_animation_mode = KI_DOUBLE_BUFFER;
//    }
//  }
//  else
//  {
//    gi_graph_animation_mode = KI_PAGE_FLIP;
//  }
//  
//#ifdef WINDOWS
//  acquire_bitmap(spst_page1);
//#endif
//  orxDisplay_ClearBitmap(spst_page1);
//#ifdef WINDOWS
//  release_bitmap(spst_page1);
//#endif
//  
//  if(spst_page2)
//  {
//#ifdef WINDOWS
//    acquire_bitmap(spst_page2);
//#endif
//    orxDisplay_ClearBitmap(spst_page2);
//#ifdef WINDOWS
//    release_bitmap(spst_page2);
//#endif
//  }
//  
//  gpst_current = spst_page1;
//
//  /* Initialize Switch Mode */
//#ifdef DOS
//  set_display_switch_mode(SWITCH_NONE);
//#elif defined WINDOWS
//  set_display_switch_mode(SWITCH_AMNESIA);
//#elif defined LINUX
//  set_display_switch_mode(SWITCH_PAUSE);
//#endif
//  
//  return EXIT_SUCCESS;  
//}
//
//void graph_exit(void)
//{
//#ifdef ALLEGRO_CONSOLE_OK
//  set_gfx_mode(GFX_TEXT, 40, 25, 0, 0);
//#endif
//
//  /* ??? Crash Windows !!! */
//  /*
//    graph_bitmap_delete(spst_page1);
//  
//    if (spst_page2)
//    {
//    graph_bitmap_delete(spst_page2);
//    }
//  */
//  
//  return;
//}
//
///* !!! TODO : Write next functions !!! */
//
//graph_st_bitmap *(*orxDisplay_LoadBitmap)(const char *_z_filename)
//{
//  return NULL;
//}
//
//extern void orxDisplay_GetBitmapSize(graph_st_bitmap *_pst_bitmap, int32 *_pi_height, int32 *_pi_width)
//{
//  return;
//}
//
///********************
// *  Plugin Related  *
// ********************/
//
//static plugin_user_st_function_info graph_plug_spst_function[PLUGIN_GRAPH_KUL_FUNCTION_NUMBER];
//
//void plugin_init(int32 *_pi_fn_number, plugin_user_st_function_info **_ppst_fn_info)
//{
//  PLUGIN_USER_FUNCTION_START(graph_plug_spst_function);
//
//  PLUGIN_USER_CORE_FUNCTION_ADD(graph_init, GRAPH, INIT);
//
//  PLUGIN_USER_CORE_FUNCTION_ADD(graph_exit, GRAPH, EXIT);
//
//  PLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Swap, GRAPH, SWITCH);
//
//  PLUGIN_USER_CORE_FUNCTION_ADD(graph_bitmap_create, GRAPH, BITMAP_CREATE);
//
//  PLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_CreateVideoBitmap, GRAPH, VIDEO_BITMAP_CREATE);
//
//  PLUGIN_USER_CORE_FUNCTION_ADD(graph_bitmap_delete, GRAPH, BITMAP_DELETE);
//
//  PLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SaveBitmap, GRAPH, BITMAP_SAVE);
//
//  PLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_TransformBitmap, GRAPH, BITMAP_TRANSFORM);
//
//  PLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_LoadBitmap, GRAPH, BITMAP_LOAD);
//
//  PLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GetBitmapSize, GRAPH, BITMAP_SIZE_GET);
//
//  PLUGIN_USER_CORE_FUNCTION_ADD(graph_screen_get, GRAPH, SCREEN_BITMAP_GET);
//
//  PLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_ClearBitmap, GRAPH, CLEAR);
//
//  PLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SetBitmapClipping, GRAPH, CLIP_SET);
//
//  PLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_BlitBitmap, GRAPH, BLIT);
//
//  PLUGIN_USER_CORE_FUNCTION_ADD(graph_bitmap_color_key_set, GRAPH, BITMAP_COLOR_KEY_SET);
//
//  PLUGIN_USER_CORE_FUNCTION_ADD(graph_draw_sprite, GRAPH, SPRITE_DRAW);
//
//  PLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_DrawText, GRAPH, PRINTF);
//
//
//  PLUGIN_USER_FUNCTION_END(_pi_fn_number, _ppst_fn_info);
//
//  return;
//}
