/**
 * \file graph_plug.c
 */

/***************************************************************************
 begin                : 14/11/2003
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

#include "orxInclude.h"

#include "debug/orxDebug.h"
#include "plugin/orxPluginUser.h"

#include "msg/msg_graph.h"


#include <SDL/SDL.h>
#include <SDL/sge.h>

#define KI_BPP 16
#define KI_WIDTH 800
#define KI_HEIGHT 600


/********************
 *   Core Related   *
 ********************/

static SDL_Surface *graph_spst_screen = NULL;

SDL_Surface *graph_screen_get()
{
  return SDL_GetVideoSurface();
}

void orxDisplay_DrawText(SDL_Surface *_pst_bmp, int32 _i_x, int32 _i_y, uint32 _u32_color, const char *_z_format, ...)
{
  char ac_buf[1024];
  va_list st_args;

  va_start(st_args, _z_format);
  vsprintf(ac_buf, _z_format, st_args);
  va_end(st_args);

/* TODO :
 * Write the string onto screen, using char per char pixel writing
 */
  return;
}

void graph_bitmap_delete(SDL_Surface *_pst_bmp)
{
  SDL_FreeSurface(_pst_bmp);
  return;
}

SDL_Surface *graph_bitmap_create(int32 _i_w, int32 _i_h)
{
  uint32 u32_rmask, u32_gmask, u32_bmask, u32_amask;

  /* SDL interprets each pixel as a 32-bit number, so our masks must depend
     on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  u32_rmask = 0xFF000000;
  u32_gmask = 0x00FF0000;
  u32_bmask = 0x0000FF00;
  u32_amask = 0x000000FF;
#else
  u32_rmask = 0x000000FF;
  u32_gmask = 0x0000FF00;
  u32_bmask = 0x00FF0000;
  u32_amask = 0xFF000000;
#endif

  return SDL_CreateRGBSurface(SDL_SWSURFACE, _i_w, _i_h, KI_BPP,
                              u32_rmask, u32_gmask, u32_bmask, u32_amask);
}

SDL_Surface *orxDisplay_CreateVideoBitmap(int32 _i_w, int32 _i_h)
{
  uint32 u32_rmask, u32_gmask, u32_bmask, u32_amask;

  /* SDL interprets each pixel as a 32-bit number, so our masks must depend
     on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  u32_rmask = 0xFF000000;
  u32_gmask = 0x00FF0000;
  u32_bmask = 0x0000FF00;
  u32_amask = 0x000000FF;
#else
  u32_rmask = 0x000000FF;
  u32_gmask = 0x0000FF00;
  u32_bmask = 0x00FF0000;
  u32_amask = 0xFF000000;
#endif

  return SDL_CreateRGBSurface(SDL_HWSURFACE, _i_w, _i_h, KI_BPP,
                              u32_rmask, u32_gmask, u32_bmask, u32_amask);
}

void orxDisplay_ClearBitmap(SDL_Surface *_pst_bmp)
{
  SDL_FillRect(_pst_bmp, NULL, 0x00000000);

  return;
}

void orxDisplay_Swap()
{
  SDL_Flip(graph_spst_screen);

  return;
}

void orxDisplay_SetBitmapColorKey(SDL_Surface *_pst_src, uint32 _u32_red, uint32 _u32_green, uint32 _u32_blue, bool _b_enable)
{
  if(_b_enable != FALSE)
  {
    SDL_SetColorKey(_pst_src, SDL_SRCCOLORKEY|SDL_RLEACCEL, SDL_MapRGB(_pst_src->format, _u32_red, _u32_green, _u32_blue));
  }
  else
  {
    SDL_SetColorKey(_pst_src, 0, 0);
  }
  
  return;
}

void orxDisplay_BlitBitmap(SDL_Surface *_pst_src, SDL_Surface *_pst_dst, int32 _i_src_x, int32 _i_src_y, int32 _i_dst_x, int32 _i_dst_y, int32 _i_w, int32 _i_h)
{
  SDL_Rect st_src_rect, st_dst_rect;

  st_src_rect.x = _i_src_x;
  st_src_rect.y = _i_src_y;
  st_src_rect.w = _i_w;
  st_src_rect.h = _i_h;
  st_dst_rect.x = _i_dst_x;
  st_dst_rect.y = _i_dst_y;

  SDL_BlitSurface(_pst_src, &st_src_rect, _pst_dst, &st_dst_rect);

  return;
}

void graph_draw_sprite(SDL_Surface *_pst_src, SDL_Surface *_pst_dst, int32 x, int32 y)
{
  return;
}

void orxDisplay_TransformBitmap(SDL_Surface *_pst_src, SDL_Surface *_pst_dst, float _f_rotation, float _f_scale_x, float _f_scale_y, int32 _i_src_x, int32 _i_src_y, int32 _i_dst_x, int32 _i_dst_y, bool _b_antialiased)
{
  if(_b_antialiased == FALSE)
  {
    sge_transform(_pst_src, _pst_dst, _f_rotation * (180.0 / PI), _f_scale_x, _f_scale_y, _i_src_x, _i_src_y, _i_dst_x, _i_dst_y, 0);
  }
  else
  {
    sge_transform(_pst_src, _pst_dst, _f_rotation * (180.0 / PI), _f_scale_x, _f_scale_y, _i_src_x, _i_src_y, _i_dst_x, _i_dst_y, SGE_TAA);
  }

  return;
}

void orxDisplay_SaveBitmap(const char *_z_filename, SDL_Surface *_pst_bmp)
{
  SDL_SaveBMP(_pst_bmp, _z_filename);

  return;
}

void orxDisplay_SetBitmapClipping(SDL_Surface *_pst_bmp, int32 _i_x, int32 _i_y, int32 _i_w, int32 _i_h)
{
  SDL_Rect st_clip_rect;

  st_clip_rect.x = _i_x;
  st_clip_rect.y = _i_y;
  st_clip_rect.w = _i_w;
  st_clip_rect.h = _i_h;

  SDL_SetClipRect(_pst_bmp, &st_clip_rect);

  return;
}

uint32 graph_init()
{
  if(SDL_WasInit(SDL_INIT_EVERYTHING) != 0)
  {
    SDL_InitSubSystem(SDL_INIT_VIDEO);
  }
  else
  {
    SDL_Init(SDL_INIT_VIDEO);
  }

  graph_spst_screen = SDL_SetVideoMode(KI_WIDTH,
                                       KI_HEIGHT,
                                       KI_BPP,
                                       SDL_SWSURFACE);
//                                       SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_FULLSCREEN);

  if(graph_spst_screen == NULL)
  {
    DEBUG(D_GRAPH, KZ_MSG_MODE_INIT_FAILED_III, KI_WIDTH, KI_HEIGHT, KI_BPP);

    return EXIT_FAILURE;
  }
  else
  {
    DEBUG(D_GRAPH, KZ_MSG_MODE_INIT_SUCCESS_III, KI_WIDTH, KI_HEIGHT, KI_BPP);
  }

  return EXIT_SUCCESS;  
}

void graph_exit()
{
  if(SDL_WasInit(SDL_INIT_EVERYTHING) == SDL_INIT_VIDEO)
  {
    SDL_Quit();
  }
  else
  {
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
  }

  return;
}

SDL_Surface *orxDisplay_LoadBitmap(const char *_z_filename)
{
  /* !!! TODO !!!
   * Needs to add a test on requested format.
   * Needs to work with other format than BMP. */

  return(SDL_LoadBMP(_z_filename));
}

void orxDisplay_GetBitmapSize(SDL_Surface *_pst_bitmap, int32 *_pi_width, int32 *_pi_height)
{
  /* Non null? */
  if(_pst_bitmap != NULL)
  {
    /* Gets size info */
    *_pi_height = _pst_bitmap->h;
    *_pi_width = _pst_bitmap->w;
  }
  else
  {
    /* Null pointer -> cleans size values */
    /* !!! MSG !!! */
    *_pi_height = -1;
    *_pi_width = -1;
  }

  return;
}



/********************
 *  Plugin Related  *
 ********************/

static plugin_user_st_function_info graph_plug_spst_function[PLUGIN_GRAPH_KUL_FUNCTION_NUMBER];

void plugin_init(int32 *_pi_fn_number, plugin_user_st_function_info **_ppst_fn_info)
{
  PLUGIN_USER_FUNCTION_START(graph_plug_spst_function);

  PLUGIN_USER_CORE_FUNCTION_ADD(graph_init, GRAPH, INIT);

  PLUGIN_USER_CORE_FUNCTION_ADD(graph_exit, GRAPH, EXIT);

  PLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Swap, GRAPH, SWITCH);

  PLUGIN_USER_CORE_FUNCTION_ADD(graph_bitmap_create, GRAPH, BITMAP_CREATE);

  PLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_CreateVideoBitmap, GRAPH, VIDEO_BITMAP_CREATE);

  PLUGIN_USER_CORE_FUNCTION_ADD(graph_bitmap_delete, GRAPH, BITMAP_DELETE);

  PLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SaveBitmap, GRAPH, BITMAP_SAVE);

  PLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_TransformBitmap, GRAPH, BITMAP_TRANSFORM);

  PLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_LoadBitmap, GRAPH, BITMAP_LOAD);

  PLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GetBitmapSize, GRAPH, BITMAP_SIZE_GET);

  PLUGIN_USER_CORE_FUNCTION_ADD(graph_screen_get, GRAPH, SCREEN_BITMAP_GET);

  PLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_ClearBitmap, GRAPH, CLEAR);

  PLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SetBitmapClipping, GRAPH, CLIP_SET);

  PLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_BlitBitmap, GRAPH, BLIT);

  PLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SetBitmapColorKey, GRAPH, BITMAP_COLOR_KEY_SET);

  PLUGIN_USER_CORE_FUNCTION_ADD(graph_draw_sprite, GRAPH, SPRITE_DRAW);

  PLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_DrawText, GRAPH, PRINTF);


  PLUGIN_USER_FUNCTION_END(_pi_fn_number, _ppst_fn_info);

  return;
}