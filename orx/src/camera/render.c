
/***************************************************************************
 render.c
 Render module
 
 begin                : 15/12/2003
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


#include "camera/render.h"

#include "camera/camera.h"
#include "graph/graph.h"
#include "graph/graphic.h"


/*
 * Platform independant defines
 */

#define RENDER_KUL_FLAG_NONE                0x00000000
#define RENDER_KUL_FLAG_READY               0x00000001
#define RENDER_KUL_FLAG_DATA_2D             0x00000010


/*
 * Internal render viewport structure
 */
typedef struct render_st_viewport_list_t
{
  /* Corresponding viewport : 4 */
  viewport_st_viewport *pst_viewport;

  /* Z sort value : 8 */
  int32 s32_z_sort;

  /* 8 extra bytes of padding : 16 */
  uint8 auc_unused[8];
} render_st_viewport_list;


/*
 * Static members
 */
static uint32 render_su32_flags = RENDER_KUL_FLAG_NONE;


/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 render_object
 Renders given object

 returns: EXIT_SUCCESS/EXIT_FAILURE
 ***************************************************************************/
inline void render_object(graph_st_bitmap *_pst_surface, object_st_object *_pst_object, frame_st_frame *_pst_frame)
{
  graphic_st_graphic *pst_graphic;
  texture_st_texture *pst_texture;
  graph_st_bitmap *pst_bitmap;
  coord_st_coord st_coord;
  float f_rotation, f_scale;
  bool b_antialias = FALSE;
  int32 w, h;

  /* Gets object's graphic */
  pst_graphic = (graphic_st_graphic *)object_struct_get(_pst_object, STRUCTURE_KUL_STRUCT_ID_GRAPHIC);

  /* 2D? */
  if(graphic_flag_test(pst_graphic, GRAPHIC_KUL_ID_FLAG_2D) != FALSE)
  {
    /* Gets graphic's texture */
    pst_texture = graphic_2d_data_get(pst_graphic);

    /* Gets texture's bitmap */
    pst_bitmap = texture_bitmap_get(pst_texture);

    /* Gets antialiasing info */
    b_antialias = graphic_flag_test(pst_graphic, GRAPHIC_KUL_ID_FLAG_ANTIALIAS);

    /* Gets frame's position, rotation & zoom */
    frame_2d_position_get(_pst_frame, &st_coord, FALSE);
    f_rotation = frame_2d_rotation_get(_pst_frame, FALSE);
    f_scale = frame_2d_scale_get(_pst_frame, FALSE);

    /* Blit bitmap onto surface */
    if((f_rotation == 0.0) && (f_scale == 1.0))
    {
      /* Gets bitmap's size */
      graph_bitmap_size_get(pst_bitmap, &w, &h);

      graph_blit(pst_bitmap, _pst_surface, 0, 0, st_coord.s32_x, st_coord.s32_y, w, h);
    }
    /* Blit transformed bitmap onto surface */
    else
    {
      /* Gets bitmap's size */
      graph_bitmap_size_get(pst_bitmap, &w, &h);

      graph_bitmap_transform(pst_bitmap, _pst_surface, f_rotation, f_scale, f_scale, 0, 0, st_coord.s32_x, st_coord.s32_y, b_antialias);
    }
  }

  return;
}

/***************************************************************************
 render_viewport_list_sort
 Sorts a viewport list using their Z value.
 Uses a "shaker sort".

 returns: void
 ***************************************************************************/
void render_viewport_list_sort(render_st_viewport_list *_pst_viewport_list, int32 _i_number)
{
  uint32 u32_low, u32_high, u32_min, u32_max, i;
  int32 l_min, l_max, l_test;

  /* Untill all viewports are sorted */
  for(u32_low = 0, u32_high = _i_number - 1; u32_low < u32_high; u32_low++, u32_high --)
  {
    /* Inits values */
    u32_min = u32_low;
    u32_max = u32_low;
    l_min = _pst_viewport_list[u32_min].s32_z_sort;
    l_max = _pst_viewport_list[u32_max].s32_z_sort;

    /* Finds min & max */
    for(i = u32_high; i > u32_low; i--)
    {
      l_test = _pst_viewport_list[i].s32_z_sort;

      if(l_test < l_min)
      {
        u32_min = i;
        l_min = l_test;
      }
      else if(l_test > l_max)
      {
        u32_max = i;
        l_max = l_test;
      }
    }

    /* Min swap? */
    if(u32_min != u32_low)
    {
      /* Swap Z sort values */
      _pst_viewport_list[u32_low].s32_z_sort = _pst_viewport_list[u32_high].s32_z_sort ^ _pst_viewport_list[u32_max].s32_z_sort;
      _pst_viewport_list[u32_min].s32_z_sort = _pst_viewport_list[u32_high].s32_z_sort ^ _pst_viewport_list[u32_max].s32_z_sort;
      _pst_viewport_list[u32_low].s32_z_sort = _pst_viewport_list[u32_high].s32_z_sort ^ _pst_viewport_list[u32_max].s32_z_sort;

      /* Swap viewport pointers */
      (_pst_viewport_list[u32_low].pst_viewport) = (viewport_st_viewport *)((uint32)(_pst_viewport_list[u32_high].pst_viewport) ^ (uint32)(_pst_viewport_list[u32_max].pst_viewport));
      (_pst_viewport_list[u32_min].pst_viewport) = (viewport_st_viewport *)((uint32)(_pst_viewport_list[u32_high].pst_viewport) ^ (uint32)(_pst_viewport_list[u32_max].pst_viewport));
      (_pst_viewport_list[u32_low].pst_viewport) = (viewport_st_viewport *)((uint32)(_pst_viewport_list[u32_high].pst_viewport) ^ (uint32)(_pst_viewport_list[u32_max].pst_viewport));
    }

    /* Max swap? */
    if(u32_max != u32_high)
    {
      /* Swap Z sort values */
      _pst_viewport_list[u32_high].s32_z_sort = _pst_viewport_list[u32_high].s32_z_sort ^ _pst_viewport_list[u32_max].s32_z_sort;
      _pst_viewport_list[u32_max].s32_z_sort  = _pst_viewport_list[u32_high].s32_z_sort ^ _pst_viewport_list[u32_max].s32_z_sort;
      _pst_viewport_list[u32_high].s32_z_sort = _pst_viewport_list[u32_high].s32_z_sort ^ _pst_viewport_list[u32_max].s32_z_sort;

      /* Swap viewport pointers */
      (_pst_viewport_list[u32_high].pst_viewport) = (viewport_st_viewport *)((uint32)(_pst_viewport_list[u32_high].pst_viewport) ^ (uint32)(_pst_viewport_list[u32_max].pst_viewport));
      (_pst_viewport_list[u32_max].pst_viewport)  = (viewport_st_viewport *)((uint32)(_pst_viewport_list[u32_high].pst_viewport) ^ (uint32)(_pst_viewport_list[u32_max].pst_viewport));
      (_pst_viewport_list[u32_high].pst_viewport) = (viewport_st_viewport *)((uint32)(_pst_viewport_list[u32_high].pst_viewport) ^ (uint32)(_pst_viewport_list[u32_max].pst_viewport));
    }
  }

  /* Done */
  return;
}


/***************************************************************************
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 render_init
 Inits render system.

 returns: EXIT_SUCCESS/EXIT_FAILURE
 ***************************************************************************/
uint32 render_init()
{
  /* Not already Initialized? */
  if(!(render_su32_flags & RENDER_KUL_FLAG_READY))
  {
    /* Inits Flags */
    render_su32_flags = RENDER_KUL_FLAG_READY | RENDER_KUL_FLAG_DATA_2D;

    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}

/***************************************************************************
 render_exit
 Exits from the render system.

 returns: void
 ***************************************************************************/
void render_exit()
{
  /* Initialized? */
  if(render_su32_flags & RENDER_KUL_FLAG_READY)
  {
    render_su32_flags &= ~RENDER_KUL_FLAG_READY;
  }

  return;
}

/***************************************************************************
 render_viewport
 Renders given viewport.

 returns: void
 ***************************************************************************/
inline void render_viewport(viewport_st_viewport *_pst_viewport)
{
  /* 2D rendering? */
  if(render_su32_flags & RENDER_KUL_FLAG_DATA_2D)
  {
    /* Is viewport active? */
    if(viewport_active(_pst_viewport) != FALSE)
    {
      camera_st_camera *pst_camera;
      texture_st_texture *pst_surface;
      graph_st_bitmap *pst_surface_bitmap;
      coord_st_coord st_pos, st_size;

      /* Gets viewport surface */
      pst_surface = viewport_surface_get(_pst_viewport);

      /* Has surface? */
      if(pst_surface != NULL)
      {
        pst_surface_bitmap = texture_bitmap_get(pst_surface);
      }
      /* Gets screen surface */
      else
      {
        pst_surface_bitmap = graph_screen_bitmap_get();
      }

      /* Gets viewport info */
      viewport_clip_get(_pst_viewport, &st_pos, &st_size);

      /* Sets surface clipping */
      graph_clip_set(pst_surface_bitmap, st_pos.s32_x, st_pos.s32_y, st_size.s32_x, st_size.s32_y);

      /* Clears surface bitmap */
      graph_clear(pst_surface_bitmap);

      /* Gets camera */
      pst_camera = viewport_camera_get(_pst_viewport);

      /* Non null? */
      if(pst_camera != NULL)
      {
        camera_st_view_list *pst_view;

        /* Updates camera view list */
        camera_view_list_update(pst_camera);

        /* Gets first view list element */
        pst_view = camera_view_list_first_get(pst_camera);

        /* Untill all objects have been processed */
        while(pst_view != NULL)
        {
          frame_st_frame *pst_screen_frame;
          object_st_object *pst_object;

          /* Gets view list element's screen frame */
          pst_screen_frame = camera_view_list_screen_frame_get(pst_view);

          /* Gets view list element's object */
          pst_object = camera_view_list_object_get(pst_view);

          render_object(pst_surface_bitmap, pst_object, pst_screen_frame);

          /* Gets next view list element */
          pst_view = camera_view_list_next_get(pst_camera);
        }
      }
    }
  }

  return;
}

/***************************************************************************
 render_all
 Renders all viewports.

 returns: void
 ***************************************************************************/
inline void render_all()
{
  render_st_viewport_list *pst_viewport_list;
  viewport_st_viewport *pst_viewport;
  coord_st_coord st_position;
  int32 i, i_viewport_number;

  /* Gets viewports number */
  i_viewport_number = structure_struct_number_get(STRUCTURE_KUL_STRUCT_ID_VIEWPORT);

  /* Creates local viewport sorted list */
  pst_viewport_list = (render_st_viewport_list *)malloc(i_viewport_number * sizeof(render_st_viewport_list));

  /* Gets first viewport */
  pst_viewport = (viewport_st_viewport *)structure_struct_first_get(STRUCTURE_KUL_STRUCT_ID_VIEWPORT);

  /* Non null? */
  for(i = 0; i < i_viewport_number; i++)
  {
    /* Gets viewport position */
    viewport_position_get(pst_viewport, &st_position);

    /* Adds it to list */
    pst_viewport_list[i].pst_viewport = pst_viewport;
    pst_viewport_list[i].s32_z_sort = st_position.s32_z;

    /* Gets next viewport */
    pst_viewport = (viewport_st_viewport *)structure_struct_next_get((structure_st_struct *)pst_viewport);
  }

  /* Sorts viewport list */
  render_viewport_list_sort(pst_viewport_list, i_viewport_number);

  /* Renders all viewports */
  for(i = 0; i < i_viewport_number; i++)
  {
    render_viewport(pst_viewport_list[i].pst_viewport);
  }

  /* Deletes local viewport list */
  free(pst_viewport_list);

  return;
}
