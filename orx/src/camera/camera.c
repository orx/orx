/***************************************************************************
 camera.c
 Camera module
 
 begin                : 10/12/2003
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


#include "camera/camera.h"

#include "anim/animpointer.h"
#include "core/timer.h"
#include "graph/graphic.h"


/*
 * Platform independant defines
 */

#define CAMERA_KUL_FLAG_NONE          0x00000000
#define CAMERA_KUL_FLAG_READY         0x00000001
#define CAMERA_KUL_FLAG_DATA_2D       0x00000010
#define CAMERA_KUL_FLAG_DEFAULT       0x00000010

#define CAMERA_KUL_ID_FLAG_NONE       0x00000000
#define CAMERA_KUL_ID_FLAG_MOVED      0x00010000
#define CAMERA_KUL_ID_FLAG_LINKED     0x00100000
#define CAMERA_KUL_ID_FLAG_LIMITED    0x00200000

#define CAMERA_KUL_ID_MASK_NUMBER     0x00000007

#define CAMERA_KI_VIEW_LIST_NUMBER    256
#define CAMERA_KI_CAMERA_NUMBER       8


/*
 * View list structure
 */
struct st_view_list_t
{
  /* Z sort value : 4 */
  uint32 u32_z_sort;

  /* Internal screen frame pointer : 8 */
  frame_st_frame *pst_screen_frame;

  /* External object pointer : 12 */
  object_st_object *pst_object;

  /* List handling pointer : 20 */
  struct st_view_list_t *pst_previous, *pst_next;

  /* Used / Not used : 24 */
  bool b_used;

  /* 8 extra bytes of padding : 32 */
  uint8 auc_unused[8];
};

/*
 * Internal 2D Camera Data Structure
 */
typedef struct st_data_2d_t
{
  /* Clip corners coords : 32 */
  coord_st_coord st_clip_ul, st_clip_br;

  /* Limit corners coords : 64 */
  coord_st_coord st_limit_ul, st_limit_br;

  /* Size coord : 96 */
  coord_st_coord st_size;
} camera_st_data_2d;


/*
 * Camera structure
 */
struct st_camera_t
{
  /* Public structure, first structure member : 16 */
  structure_st_struct st_struct;

  /* Internal id flags : 20 */
  uint32 u32_id_flags;

  /* Frame : 24 */
  frame_st_frame *pst_frame;

  /* Linked object : 28 */
  object_st_object *pst_link;

  /* Data : 32 */
  void *pst_data;

  /* On screen position coord : 48 */
  coord_st_coord st_on_screen_position;

   /* View list current pointer : 52 */
  camera_st_view_list *pst_view_list_current;

  /* View list first pointer : 56 */
  camera_st_view_list *pst_view_list_first;

  /* View list counter : 60 */
  int32 i_view_list_counter;

  /* 8 extra bytes of padding : 64 */
  uint8 auc_unused[8];

  /* View list : 8256 */
  camera_st_view_list ast_view_list[CAMERA_KI_VIEW_LIST_NUMBER];
};


/*
 * Static members
 */
static uint32 camera_su32_flags = CAMERA_KUL_FLAG_DEFAULT;
static bool camera_sab_camera_used[CAMERA_KI_CAMERA_NUMBER];


/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 camera_list_delete
 Deletes all cameras.

 returns: void
 ***************************************************************************/
void camera_list_delete()
{
  camera_st_camera *pst_camera = (camera_st_camera *)structure_struct_first_get(STRUCTURE_KUL_STRUCT_ID_CAMERA);

  /* Non null? */
  while(pst_camera != NULL)
  {
    /* Deletes object */
    camera_delete(pst_camera);

    /* Gets first object */
    pst_camera = (camera_st_camera *)structure_struct_first_get(STRUCTURE_KUL_STRUCT_ID_CAMERA);
  }

  return;
}

/***************************************************************************
 camera_view_list_cell_clean
 Creates a camera view list cell.

 returns: void
 ***************************************************************************/
inline void camera_view_list_cell_clean(camera_st_view_list *_pst_view_list)
{
  /* Updates pointers */
  _pst_view_list->pst_previous = NULL;
  _pst_view_list->pst_next = NULL;
  _pst_view_list->pst_object = NULL;

  /* Updates status & sort value */
  _pst_view_list->b_used = FALSE;
  _pst_view_list->u32_z_sort = 0;

  return;
}

/***************************************************************************
 camera_view_list_clean
 Cleans a camera view list.

 returns: void
 ***************************************************************************/
inline void camera_view_list_clean(camera_st_camera *_pst_camera)
{
  int32 i;

    /* Cleans view list */

    for(i = 0; i < CAMERA_KI_VIEW_LIST_NUMBER; i++)
    {
      camera_view_list_cell_clean(&(_pst_camera->ast_view_list[i]));
    }

    _pst_camera->i_view_list_counter = 0;
    _pst_camera->pst_view_list_first = NULL;
    _pst_camera->pst_view_list_current = NULL;

  return;
}

/***************************************************************************
 camera_view_list_create
 Creates a camera view list.

 returns: EXIT_SUCCESS/EXIT_FAILURE
 ***************************************************************************/
inline uint32 camera_view_list_create(camera_st_camera *_pst_camera)
{
  int32 i;

  /* For all view list structures */
  for(i = 0; i < CAMERA_KI_VIEW_LIST_NUMBER; i++)
  {
    /* Creates screen frame */
    (_pst_camera->ast_view_list[i]).pst_screen_frame = frame_create();
    if((_pst_camera->ast_view_list[i]).pst_screen_frame == NULL)
    {
      int32 j;

      /* Frees all previously created frames */
      for(j = i - 1; j >= 0; j--)
      {
        frame_delete((_pst_camera->ast_view_list[j]).pst_screen_frame);
      }

      return EXIT_FAILURE;
    }
    else
    {
      /* Cleans view list cell */
      camera_view_list_cell_clean(&(_pst_camera->ast_view_list[i]));
    }
  }

  /* Cleans view list */
  camera_view_list_clean(_pst_camera);

  return EXIT_SUCCESS;
}

/***************************************************************************
 camera_view_list_delete
 Deletes a camera view list.

 returns: void
 ***************************************************************************/
inline void camera_view_list_delete(camera_st_camera *_pst_camera)
{
  int32 i;

  /* For all view list structures */
  for(i = 0; i < CAMERA_KI_VIEW_LIST_NUMBER; i++)
  {
    /* Deletes screen frame */
    frame_delete(_pst_camera->ast_view_list[i].pst_screen_frame);
  }

  /* Cleans view list */
  camera_view_list_clean(_pst_camera);

  return;
}

/***************************************************************************
 camera_position_update
 Computes camera position using linked object & limits.

 returns: void
 ***************************************************************************/
inline void camera_position_update(camera_st_camera *_pst_camera, bool _b_force)
{
  coord_st_coord st_pos;
  coord_st_coord *pst_ul, *pst_br;

  /* Is camera linked? */
  if(_pst_camera->u32_id_flags & CAMERA_KUL_ID_FLAG_LINKED)
  {
    /* Gets linked object frame */
    frame_st_frame *pst_frame = (frame_st_frame *)object_struct_get(_pst_camera->pst_link, STRUCTURE_KUL_STRUCT_ID_FRAME);

    /* Non null? */
    if(pst_frame != NULL)
    {
      /* Is frame render dirty or update forced? */
      if((frame_render_status_ok(pst_frame) == FALSE) || (_b_force != FALSE))
      {
        /* 2D? */
        if(camera_flag_test(_pst_camera, CAMERA_KUL_ID_FLAG_2D) != FALSE)
        {
          /* Gets linked object positions */
          frame_2d_position_get(pst_frame, &st_pos, FALSE);

          /* Has camera limits? */
          if(_pst_camera->u32_id_flags & CAMERA_KUL_ID_FLAG_LIMITED)
          {
            /* Gets limit coords */
            pst_ul = &(((camera_st_data_2d *)(_pst_camera->pst_data))->st_limit_ul);
            pst_br = &(((camera_st_data_2d *)(_pst_camera->pst_data))->st_limit_br);

            /* Verify position */

            /* Z coordinate */
            if(st_pos.s32_z < pst_ul->s32_z)
            {
              st_pos.s32_z = pst_ul->s32_z;
            }
            else if(st_pos.s32_z > pst_br->s32_z)
            {
              st_pos.s32_z = pst_br->s32_z;
            }

            /* Y coordinate */
            if(st_pos.s32_y < pst_ul->s32_y)
            {
              st_pos.s32_y = pst_ul->s32_y;
            }
            else if(st_pos.s32_y > pst_br->s32_y)
            {
              st_pos.s32_y = pst_br->s32_y;
            }

            /* X coordinate */
            if(st_pos.s32_x < pst_ul->s32_x)
            {
              st_pos.s32_x = pst_ul->s32_x;
            }
            else if(st_pos.s32_x > pst_br->s32_x)
            {
              st_pos.s32_x = pst_br->s32_x;
            }
          }

          /* Sets camera position */
          frame_2d_position_set(_pst_camera->pst_frame, &st_pos);

          /* Updates camera flags */
          _pst_camera->u32_id_flags |= CAMERA_KUL_ID_FLAG_MOVED;
        }
      }
    }
    else
    {
      /* Has camera moved? */
      if(_pst_camera->u32_id_flags & CAMERA_KUL_ID_FLAG_MOVED)
      {
        /* Has camera limits? */
        if(_pst_camera->u32_id_flags & CAMERA_KUL_ID_FLAG_LIMITED)
        {
          /* Gets camera position */
          frame_2d_position_get(_pst_camera->pst_frame, &st_pos, TRUE);

          /* Gets limit coords pointers */
          pst_ul = &(((camera_st_data_2d *)(_pst_camera->pst_data))->st_limit_ul);
          pst_br = &(((camera_st_data_2d *)(_pst_camera->pst_data))->st_limit_br);

          /* Verify position */

          /* Z coordinate */
          if(st_pos.s32_z < pst_ul->s32_z)
          {
            st_pos.s32_z = pst_ul->s32_z;
          }
          else if(st_pos.s32_z > pst_br->s32_z)
          {
            st_pos.s32_z = pst_br->s32_z;
          }

          /* Y coordinate */
          if(st_pos.s32_y < pst_ul->s32_y)
          {
            st_pos.s32_y = pst_ul->s32_y;
          }
          else if(st_pos.s32_y > pst_br->s32_y)
          {
            st_pos.s32_y = pst_br->s32_y;
          }

          /* X coordinate */
          if(st_pos.s32_x < pst_ul->s32_x)
          {
            st_pos.s32_x = pst_ul->s32_x;
          }
          else if(st_pos.s32_x > pst_br->s32_x)
          {
            st_pos.s32_x = pst_br->s32_x;
          }

          /* Sets camera position */
          frame_2d_position_set(_pst_camera->pst_frame, &st_pos);
        }
      }
    }
  }

  return;
}

/***************************************************************************
 camera_clip_corners_compute
 Computes camera clip corner coordinates.

 returns: void
 ***************************************************************************/
inline void camera_clip_corners_compute(camera_st_camera *_pst_camera)
{
  coord_st_coord st_pos;
  coord_st_coord *pst_ul, *pst_br, *pst_size;
  float f_rot, f_scale;

  /* Updates camera position using limits & link options */
  camera_position_update(_pst_camera, FALSE);

  /* Has camera moved? */
  if(_pst_camera->u32_id_flags & CAMERA_KUL_ID_FLAG_MOVED)
  {
    /* 2D? */
    if(camera_flag_test(_pst_camera, CAMERA_KUL_ID_FLAG_2D) != FALSE)
    {
      /* Gets coords pointers */
      pst_ul    = &(((camera_st_data_2d *)(_pst_camera->pst_data))->st_clip_ul);
      pst_br    = &(((camera_st_data_2d *)(_pst_camera->pst_data))->st_clip_br);
      pst_size  = &(((camera_st_data_2d *)(_pst_camera->pst_data))->st_size);

      /* Gets camera info */
      frame_2d_position_get(_pst_camera->pst_frame, &st_pos, FALSE);
      f_rot = frame_2d_rotation_get(_pst_camera->pst_frame, FALSE);
      f_scale = frame_2d_scale_get(_pst_camera->pst_frame, FALSE);

      /* Computes relative camera upper left & bottom right corners */
      coord_mul(pst_br, pst_size, 0.5);
      coord_sub(pst_ul, pst_br, pst_size);

      /* Applies scale & rotation if needed */
      if(f_rot != 0.0)
      {
        int32 s32_max;

        /* We want only axis aligned box for clipping */
        /* We thus compute axis aligned smallest box that contains real rotated camera box */

        /* Rotates one corner */
        coord_rotate(pst_ul, pst_ul, f_rot);

        /* Gets corner maximum absolute value between X & Y values */
        if(abs(pst_ul->s32_x) > abs(pst_ul->s32_y))
        {
          s32_max = abs(pst_ul->s32_x) + 1;
        }
        else
        {
          s32_max = abs(pst_ul->s32_y) + 1;
        }

        /* Applies it to both corners */
        pst_ul->s32_x = -s32_max;
        pst_ul->s32_y = -s32_max;
        pst_br->s32_x = s32_max;
        pst_br->s32_y = s32_max;
      }
      if(f_scale != 1.0)
      {
        coord_mul(pst_ul, pst_ul, f_scale);
        coord_mul(pst_br, pst_br, f_scale);
      }

      /* Computes global corners */
      coord_add(pst_ul, pst_ul, &st_pos);
      coord_add(pst_br, pst_br, &st_pos);
      coord_aabox_reorder(pst_ul, pst_br);
    }
  }

  return;
}

/***************************************************************************
 camera_view_list_remove
 Removes a cell from the sorted camera view list.

 returns: void
 ***************************************************************************/
inline void camera_view_list_remove(camera_st_camera *_pst_camera, camera_st_view_list* _pst_cell)
{
  /* First cell? */
  if(_pst_cell->pst_previous == NULL)
  {
    /* Second cell will become first one */
    _pst_camera->pst_view_list_first = _pst_cell->pst_next;
  }

  /* Updates neighbours */
  if(_pst_cell->pst_previous != NULL)
  {
    (_pst_cell->pst_previous)->pst_next = _pst_cell->pst_next;
  }

  if(_pst_cell->pst_next != NULL)
  {
    (_pst_cell->pst_next)->pst_previous = _pst_cell->pst_previous;
  }

  /* Cleans pointers */
  _pst_cell->pst_next = NULL;
  _pst_cell->pst_previous = NULL;

  /* Decreases counter */
  _pst_camera->i_view_list_counter--;

  return;
}

/***************************************************************************
 camera_view_list_insert
 Inserts a cell in the sorted camera view list, using frame.z coordinate as sort value.

 returns: void
 ***************************************************************************/
inline void camera_view_list_insert(camera_st_camera *_pst_camera, camera_st_view_list* _pst_cell)
{
  camera_st_view_list *pst_current, *pst_previous = NULL;

  /* Is list empty? */
  if(_pst_camera->pst_view_list_first == NULL)
  {
    /* Becomes first cell */
    _pst_camera->pst_view_list_first = _pst_cell;

    /* Updates cell pointers */
    _pst_cell->pst_previous = NULL;
    _pst_cell->pst_next = NULL;

  }
  else
  {
    /* Finds correct index using Z sort value */
    pst_current = _pst_camera->pst_view_list_first;
    while((pst_current != NULL) && (_pst_cell->u32_z_sort < pst_current->u32_z_sort))
    {
      pst_previous = pst_current;
      pst_current = pst_current->pst_next;
    }

    /* Updates cell */
    _pst_cell->pst_next = pst_current;
    _pst_cell->pst_previous = pst_previous;

    /* Not last cell */
    if(pst_current != NULL)
    {
      pst_current->pst_previous = _pst_cell;
    }

    /* Not first cell */
    if(pst_previous != NULL)
    {
      pst_previous->pst_next = _pst_cell;
    }
    else
    {
      /* Updates list first cell */
      _pst_camera->pst_view_list_first = _pst_cell;
    }
  }

  /* Increases counter */
  _pst_camera->i_view_list_counter++;

  return;
}

/***************************************************************************
 camera_view_list_search
 Search view list for a cell corresponding to the given object.

 returns: object cell/NULL
 ***************************************************************************/
inline camera_st_view_list *camera_view_list_search(camera_st_camera *_pst_camera, object_st_object *_pst_object)
{
  camera_st_view_list *pst_cell = NULL;

  /* Find corresponding cell */
  for(pst_cell = _pst_camera->pst_view_list_first;
      (pst_cell != NULL) && (pst_cell->pst_object != _pst_object);
      pst_cell = pst_cell->pst_next)
  {
  }

  return pst_cell;
}

/***************************************************************************
 camera_view_list_free_search
 Search view list for a free cell.

 returns: object cell/NULL
 ***************************************************************************/
inline camera_st_view_list *camera_view_list_free_search(camera_st_camera *_pst_camera)
{
  camera_st_view_list *pst_cell = NULL;
  int32 i;

  /* Find free cell */
  for(i = 0;
      (i < CAMERA_KI_VIEW_LIST_NUMBER) && ((_pst_camera->ast_view_list[i]).b_used != FALSE);
      i++)
  {
  }

  /* Gets cell if found */
  if(i < CAMERA_KI_VIEW_LIST_NUMBER)
  {
    pst_cell = &(_pst_camera->ast_view_list[i]);
  }

  return pst_cell;
}

/***************************************************************************
 camera_object_compute
 Test object/camera intersection.
 If succesfull :
 - computes camera transformed screen frame,
 - stores object pointer in view list,
 - add view_list cell to sorted list.

 If not :
 - removes view_list cell from sorted list.

 returns: EXIT_SUCCESS/EXIT_FAILURE (list full)
 ***************************************************************************/
inline uint32 camera_object_compute(camera_st_camera *_pst_camera, object_st_object *_pst_object, uint32 _u32_timestamp)
{
  frame_st_frame *pst_frame;
  graphic_st_graphic *pst_graphic;
  camera_st_view_list *pst_cell = NULL;
  coord_st_coord *pst_cam_ul, *pst_cam_br, *pst_cam_size, st_cam_pos;
  coord_st_coord st_tex_ul, st_tex_br, st_tex_ref, st_tex_pos;
  float f_cam_rot, f_cam_scale, f_tex_rot, f_tex_scale;
  float f_scroll_x, f_scroll_y;

  /* Has graphic? */
  pst_graphic = (graphic_st_graphic *)object_struct_get(_pst_object, STRUCTURE_KUL_STRUCT_ID_GRAPHIC);
  if(pst_graphic != NULL)
  {
    /* 2D? */
    if(graphic_flag_test(pst_graphic, GRAPHIC_KUL_ID_FLAG_2D) != FALSE)
    {
      /* Has frame? */
      pst_frame = (frame_st_frame *)object_struct_get(_pst_object, STRUCTURE_KUL_STRUCT_ID_FRAME);
      if(pst_frame != NULL)
      {
        /* Gets camera clip corners pointers */
        pst_cam_ul = &(((camera_st_data_2d *)(_pst_camera->pst_data))->st_clip_ul);
        pst_cam_br = &(((camera_st_data_2d *)(_pst_camera->pst_data))->st_clip_br);

        /* Gets texture infos */
        frame_2d_position_get(pst_frame, &st_tex_pos, FALSE);
        f_tex_rot = frame_2d_rotation_get(pst_frame, FALSE);
        f_tex_scale = frame_2d_scale_get(pst_frame, FALSE);

        /* Computes texture global corners */
        graphic_2d_ref_coord_get(pst_graphic, &st_tex_ref);
        coord_sub(&st_tex_ul, &st_tex_pos, &st_tex_ref);

        graphic_2d_size_get(pst_graphic, &st_tex_ref);
        coord_add(&st_tex_br, &st_tex_ul, &st_tex_ref);

        /* Intersection? */
        if(coord_aabox_intersection_test(pst_cam_ul, pst_cam_br, &st_tex_ul, &st_tex_br) != FALSE)
        {
          /* Search for object cell in list*/
          pst_cell = camera_view_list_search(_pst_camera, _pst_object);

          /* Already in list */
          if(pst_cell != NULL)
          {
            /* Removes it from list */
            camera_view_list_remove(_pst_camera, pst_cell);

            /* Cleans view list cell */
            camera_view_list_cell_clean(pst_cell);
          }
          else
          {
            /* Find first free cell */
            pst_cell = camera_view_list_free_search(_pst_camera);

            /* No cell left? */
            if(pst_cell == NULL)
            {
              return EXIT_FAILURE;
            }
          }

          /* Has animation? */
          if(graphic_flag_test(pst_graphic, GRAPHIC_KUL_ID_FLAG_ANIM) != FALSE)
          {
            /* Updates animation */
            animpointer_compute((animpointer_st_animpointer *)graphic_struct_get(pst_graphic, STRUCTURE_KUL_STRUCT_ID_ANIMPOINTER), _u32_timestamp);
          }

          /* Stores the object */
          pst_cell->pst_object = _pst_object;

          /* Gets camera infos */
          pst_cam_size = &(((camera_st_data_2d *)(_pst_camera->pst_data))->st_size);
          frame_2d_position_get(_pst_camera->pst_frame, &st_cam_pos, FALSE);
          f_cam_rot = frame_2d_rotation_get(_pst_camera->pst_frame, FALSE);
          f_cam_scale = frame_2d_scale_get(_pst_camera->pst_frame, FALSE);

          /* Computes texture screen frame, using viewport coordinates */

          /* Gets into camera space */
          coord_sub(&st_tex_ul, &st_tex_ul, &st_cam_pos);

          /* Applies rotation & scale if needed */
          if(f_cam_rot != 0.0)
          {
            coord_rotate(&st_tex_ul, &st_tex_ul, -f_cam_rot);
          }
          if(f_cam_scale != 1.0)
          {
            coord_div(&st_tex_ul, &st_tex_ul, f_cam_scale);
          }

          /* Uses differential scrolling? */
          if(frame_differential_scrolling_use(pst_frame) != FALSE)
          {
            /* Gets scrolling coefficients */
            frame_differential_scrolling_get(pst_frame, &f_scroll_x, &f_scroll_y);

            /* X axis scrolling? */
            if(f_scroll_x != 0.0)
            {
              f_scroll_x *= (float)st_tex_ul.s32_x;
            }

            /* Y axis scrolling? */
            if(f_scroll_y != 0.0)
            {
              f_scroll_y *= (float)st_tex_ul.s32_y;
            }

            /* Updates texture coordinates */
            coord_set(&st_tex_ul, (int)rintf(f_scroll_x), (int)rintf(f_scroll_y), st_tex_ul.s32_z);
          }

          /* Gets into viewport coordinates */
          coord_mul(&st_cam_pos, pst_cam_size, 0.5);
          coord_add(&st_tex_ul, &st_tex_ul, &st_cam_pos);

          /* Gets into screen coordinates */
          coord_add(&st_tex_ul, &st_tex_ul, &(_pst_camera->st_on_screen_position));

          /* Stores screen coordinates */
          pst_frame = pst_cell->pst_screen_frame;
          frame_2d_position_set(pst_frame, &st_tex_ul);
          frame_2d_rotation_set(pst_frame, f_tex_rot - f_cam_rot);
          frame_2d_scale_set(pst_frame, f_tex_scale / f_cam_scale);

          /* Updates view list sort value */
          pst_cell->u32_z_sort = st_tex_ul.s32_z;

          /* Updates view list used status */
          pst_cell->b_used = TRUE;

          /* Insert it into sorted list */
          camera_view_list_insert(_pst_camera, pst_cell);

          /* Updates graphic status */
          graphic_flag_set(pst_graphic, GRAPHIC_KUL_ID_FLAG_RENDERED, GRAPHIC_KUL_ID_FLAG_NONE);
        }
        else
        {
          /* Search for object position in list*/
          pst_cell = camera_view_list_search(_pst_camera, _pst_object);

          /* Already in list */
          if(pst_cell != NULL)
          {
            /* Removes it from list */
            camera_view_list_remove(_pst_camera, pst_cell);

            /* Cleans view list cell */
            camera_view_list_cell_clean(pst_cell);
          }

          /* Updates graphic status */
          graphic_flag_set(pst_graphic, GRAPHIC_KUL_ID_FLAG_NONE, GRAPHIC_KUL_ID_FLAG_RENDERED);
        }
      }
    }
  }

  /* Everything's ok */

  return EXIT_SUCCESS;
}

/***************************************************************************
 camera_view_list_sort
 Sorts camera view list.

 returns: void
 ***************************************************************************/
inline void camera_view_list_sort(camera_st_camera *_pst_camera)
{
  int32 i;

  /* Cleans sorted list */
  _pst_camera->pst_view_list_first = NULL;
  _pst_camera->pst_view_list_current = NULL;

  /* For all view list cells */
  for(i = 0; i < CAMERA_KI_VIEW_LIST_NUMBER; i++)
  {
    if((_pst_camera->ast_view_list[i]).b_used != FALSE)
    {
      /* Insert it into the list */
      camera_view_list_insert(_pst_camera, &(_pst_camera->ast_view_list[i]));
    }
  }

  return;
}


/***************************************************************************
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/


/***************************************************************************
 camera_init
 Inits camera system.

 returns: EXIT_SUCCESS/EXIT_FAILURE
 ***************************************************************************/
uint32 camera_init()
{
  int32 i;

  /* Not already Initialized? */
  if(!(camera_su32_flags & CAMERA_KUL_FLAG_READY))
  {
    /* Inits cameras */
    for(i = 0; i < CAMERA_KI_CAMERA_NUMBER; i++)
    {
      camera_sab_camera_used[i] = FALSE;
    }

    /* Inits Flags */
    camera_su32_flags = CAMERA_KUL_FLAG_DEFAULT|CAMERA_KUL_FLAG_READY;

    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}

/***************************************************************************
 camera_exit
 Exits from the camera system.

 returns: void
 ***************************************************************************/
void camera_exit()
{
  /* Initialized? */
  if(camera_su32_flags & CAMERA_KUL_FLAG_READY)
  {
    camera_su32_flags &= ~CAMERA_KUL_FLAG_READY;

    /* Deletes camera list */
    camera_list_delete();
  }

  return;
}

/***************************************************************************
 camera_create
 Creates a new empty camera.

 returns: Created camera.
 ***************************************************************************/
camera_st_camera *camera_create()
{
  camera_st_camera *pst_camera;
  frame_st_frame *pst_frame;
  int32 i_camera = -1, i;

  /* Gets free camera slot */
  for(i = 0; i < CAMERA_KI_CAMERA_NUMBER; i++)
  {
    /* Camera slot free? */
    if(camera_sab_camera_used[i] == FALSE)
    {
      i_camera = i;
      break;
    }
  }

  /* No free slot? */
  if(i_camera == -1)
  {
    /* !!! MSG !!! */

    return NULL;
  }

  /* Creates camera */
  pst_camera = (camera_st_camera *) malloc(sizeof(camera_st_camera));

  /* Non null? */
  if(pst_camera != NULL)
  {
    /* Creates frame */
    pst_frame = frame_create();

    if(pst_frame != NULL)
    {
      /* Inits structure */
      if(structure_struct_init((structure_st_struct *)pst_camera, STRUCTURE_KUL_STRUCT_ID_CAMERA) != EXIT_SUCCESS)
      {
        /* !!! MSG !!! */
  
        /* Fress partially allocated camera */
        free(pst_camera);
        frame_delete(pst_frame);

        /* Returns nothing */
        return NULL;
      }

      /* Creates camera view list */
      if(camera_view_list_create(pst_camera) == EXIT_FAILURE)
      {
        /* !!! MSG !!! */

        /* Fress partially allocated camera */
        free(pst_camera);
        frame_delete(pst_frame);

        /* Returns nothing */
        return NULL;
      }

      /* Inits camera members */
      pst_camera->u32_id_flags = CAMERA_KUL_ID_FLAG_MOVED | (uint32)i_camera;
      pst_camera->pst_frame = pst_frame;
      pst_camera->pst_link = NULL;
      coord_set(&(pst_camera->st_on_screen_position), 0, 0, 0);

      /* 2D */
      if(camera_su32_flags & CAMERA_KUL_FLAG_DATA_2D)
      {
        camera_st_data_2d *pst_data;

        /* Updates ID flags */
        pst_camera->u32_id_flags |= CAMERA_KUL_ID_FLAG_2D;

        /* Allocates data memory */
        pst_data = (camera_st_data_2d *) malloc(sizeof(camera_st_data_2d));

        /* Inits & assigns it */
        if(pst_data != NULL)
        {
          /* Cleans values */
          coord_set(&(pst_data->st_size), 0, 0, 0);
          coord_set(&(pst_data->st_limit_ul), 0, 0, 0);
          coord_set(&(pst_data->st_limit_br), 0, 0, 0);

          /* Links data to frame */
          pst_camera->pst_data = pst_data;
        }
        else
        {
          /* !!! MSG !!! */

          /* Fress partially allocated camera */
          camera_view_list_delete(pst_camera);
          frame_delete(pst_frame);
          free(pst_camera);

          /* Returns nothing */
          return NULL;
        }
      }

      /* Computes clip infos */
      camera_clip_corners_compute(pst_camera);

      /* Updates camera slot */
      camera_sab_camera_used[i_camera] = TRUE;
    }
    else
    {
        /* !!! MSG !!! */
  
        /* Fress partially allocated camera */
        free(pst_camera);

        /* Returns nothing */
        return NULL;
    }
  }

  return(pst_camera);
}

/***************************************************************************
 camera_delete
 Deletes a camera.

 returns: void
 ***************************************************************************/
void camera_delete(camera_st_camera *_pst_camera)
{
  uint32 i_camera;

  /* Non null? */
  if(_pst_camera != NULL)
  {
    /* Gets camera id number */
    i_camera = _pst_camera->u32_id_flags & CAMERA_KUL_ID_MASK_NUMBER;

    /* Frees camera slot */
    camera_sab_camera_used[i_camera] = FALSE;

    /* Frees camera view list */
    camera_view_list_delete(_pst_camera);

    /* Remove linked object reference */
    if(_pst_camera->pst_link != NULL)
    {
      structure_struct_counter_decrease((structure_st_struct *)(_pst_camera->pst_link));
    }

    /* Deletes frame*/
    frame_delete(_pst_camera->pst_frame);

    /* Frees data */
    free(_pst_camera->pst_data);

    /* Cleans structure */
    structure_struct_clean((structure_st_struct *)_pst_camera);

    /* Frees camera memory */
    free(_pst_camera);
  }

  return;
}

/***************************************************************************
 camera_view_list_update
 Updates camera view list.

 returns: void
 ***************************************************************************/
extern void camera_view_list_update(camera_st_camera *_pst_camera)
{
  object_st_object *pst_object;
  frame_st_frame *pst_frame;
  uint32 u32_timestamp;

  /* Computes camera corners */
  camera_clip_corners_compute(_pst_camera);

  /* Gets timestamp */
  u32_timestamp = timer_game_time_get();

  /* If camera moved, process all objects */
  if(_pst_camera->u32_id_flags & CAMERA_KUL_ID_FLAG_MOVED)
  {
    /* For all objects */
    for(pst_object = (object_st_object *)structure_struct_first_get(STRUCTURE_KUL_STRUCT_ID_OBJECT);
        pst_object != NULL;
        pst_object = (object_st_object *)structure_struct_next_get((structure_st_struct *)pst_object))
    {
      /* Computes object */
      if(camera_object_compute(_pst_camera, pst_object, u32_timestamp) == EXIT_FAILURE)
      {
        /* No room left in camera view list */
        /* !!! MSG !!! */

        break;
      }
    }
  }
  /* Process only render dirty objects */
  else
  {
    /* For all objects */
    for(pst_object = (object_st_object *)structure_struct_first_get(STRUCTURE_KUL_STRUCT_ID_OBJECT);
        pst_object != NULL;
        pst_object = (object_st_object *)structure_struct_next_get((structure_st_struct *)pst_object))
    {
      /* Gets object frame? */
      pst_frame = (frame_st_frame *)object_struct_get(pst_object, STRUCTURE_KUL_STRUCT_ID_FRAME);

      /* Has to be processed? */
      if(object_render_status_ok(pst_object) == FALSE)
      {
        /* Computes object */
        if(camera_object_compute(_pst_camera, pst_object, u32_timestamp) == EXIT_FAILURE)
        {
          /* No room left in camera view list */
          /* !!! MSG !!! */

          break;
        }
      }
    }
  }

  /* Removes camera moved flag */
  _pst_camera->u32_id_flags &= ~CAMERA_KUL_ID_FLAG_MOVED;

  return;
}


/* *** Structure accessors *** */


/***************************************************************************
 camera_view_list_first_get
 Gets camera first view list cell.

 returns: Requested camera_st_view_list
 ***************************************************************************/
inline camera_st_view_list *camera_view_list_first_get(camera_st_camera *_pst_camera)
{
  /* Stores first pointer */
  _pst_camera->pst_view_list_current = _pst_camera->pst_view_list_first;

  return _pst_camera->pst_view_list_current;
}

/***************************************************************************
 camera_view_list_next_get
 Gets camera next view list cell.

 returns: Requested camera_st_view_list
 ***************************************************************************/
inline camera_st_view_list *camera_view_list_next_get(camera_st_camera *_pst_camera)
{
  /* Stores next pointer */
  if((_pst_camera->pst_view_list_current)->pst_next != NULL)
  {
    _pst_camera->pst_view_list_current = (_pst_camera->pst_view_list_current)->pst_next;
  }
  else
  {
    /* End of list */
    /* !!! MSG !!! */

    return NULL;
  }

  return _pst_camera->pst_view_list_current;
}

/***************************************************************************
 camera_view_list_screen_frame_get
 Gets view list frame.

 returns: Requested frame pointer
 ***************************************************************************/
inline frame_st_frame *camera_view_list_screen_frame_get(camera_st_view_list *_pst_view_list)
{
  return _pst_view_list->pst_screen_frame;
}

/***************************************************************************
 camera_view_list_object_get
 Gets view list object.

 returns: Requested texture pointer
 ***************************************************************************/
inline object_st_object *camera_view_list_object_get(camera_st_view_list *_pst_view_list)
{
  return(_pst_view_list->pst_object);
}

/***************************************************************************
 camera_view_list_number_get
 Gets camera view list number.

 returns: Requested camera_st_view_list
 ***************************************************************************/
inline int32 camera_view_list_number_get(camera_st_camera *_pst_camera)
{
  return(_pst_camera->i_view_list_counter);
}

/***************************************************************************
 camera_2d_size_set
 Camera 2D size set accessor.

 returns: void
 ***************************************************************************/
inline void camera_2d_size_set(camera_st_camera *_pst_camera, coord_st_coord *_pst_size)
{
  /* Updates */
  coord_copy(&(((camera_st_data_2d *)(_pst_camera->pst_data))->st_size), _pst_size);

  /* Updates camera flags */
  _pst_camera->u32_id_flags |= CAMERA_KUL_ID_FLAG_MOVED;

  return;
}

/***************************************************************************
 camera_2d_position_set
 Camera 2D position set accessor.

 returns: void
 ***************************************************************************/
inline void camera_2d_position_set(camera_st_camera *_pst_camera, coord_st_coord *_pst_position)
{
  /* Sets camera position */
  frame_2d_position_set(_pst_camera->pst_frame, _pst_position);

  /* Updates camera flags */
  _pst_camera->u32_id_flags |= CAMERA_KUL_ID_FLAG_MOVED;

  return;
}

/***************************************************************************
 camera_2d_rotation_set
 Camera 2D rotation set accessor.

 returns: void
 ***************************************************************************/
inline void camera_2d_rotation_set(camera_st_camera *_pst_camera, float _f_rotation)
{
   /* Sets camera rotation */
  frame_2d_rotation_set(_pst_camera->pst_frame, _f_rotation);

  /* Updates camera flags */
  _pst_camera->u32_id_flags |= CAMERA_KUL_ID_FLAG_MOVED;

  return;
}

/***************************************************************************
 camera_2d_zoom_set
 Camera 2D zoom set accessor.

 returns: void
 ***************************************************************************/
inline void camera_2d_zoom_set(camera_st_camera *_pst_camera, float _f_zoom)
{
   /* Sets camera zoom */
  frame_2d_scale_set(_pst_camera->pst_frame, 1.0 / _f_zoom);

  /* Updates camera flags */
  _pst_camera->u32_id_flags |= CAMERA_KUL_ID_FLAG_MOVED;

  return;
}

/***************************************************************************
 camera_link_set
 Camera zoom set accessor.

 returns: void
 ***************************************************************************/
inline void camera_link_set(camera_st_camera *_pst_camera, object_st_object *_pst_object)
{
  /* Has already a linked object */
  if(_pst_camera->pst_link != NULL)
  {
    /* Updates structure reference counter */
    structure_struct_counter_decrease((structure_st_struct *)(_pst_camera->pst_link));
  }

  /* Sets camera link object */
  _pst_camera->pst_link = _pst_object;

  /* Null object? */
  if(_pst_object == NULL)
  {
    /* Updates id flags */
    _pst_camera->u32_id_flags &= ~CAMERA_KUL_ID_FLAG_LINKED;
  }
  else
  {
    /* Updates structure reference counter */
    structure_struct_counter_increase((structure_st_struct *)_pst_object);

    /* Updates id flags */
    _pst_camera->u32_id_flags |= CAMERA_KUL_ID_FLAG_LINKED;

    /* Updates camera position */
    camera_position_update(_pst_camera, TRUE);
  }

  return;
}

/***************************************************************************
 camera_on_screen_position_set
 Camera on screen position set accessor.

 returns: void
 ***************************************************************************/
inline void camera_on_screen_position_set(camera_st_camera *_pst_camera, coord_st_coord *_pst_position)
{
  /* Copy on screen camera position coords */
  coord_copy(&(_pst_camera->st_on_screen_position), _pst_position);

  return;
}


/***************************************************************************
 camera_2d_size_get
 Camera 2D size get accessor.

 returns: void
 ***************************************************************************/
inline void camera_2d_size_get(camera_st_camera *_pst_camera, coord_st_coord *_pst_size)
{
  /* Copy coord */
  coord_copy(_pst_size, &(((camera_st_data_2d *)(_pst_camera->pst_data))->st_size));

  return;
}

/***************************************************************************
 camera_2d_position_get
 Camera 2D position get accessor.

 returns: void
 ***************************************************************************/
inline void camera_2d_position_get(camera_st_camera *_pst_camera, coord_st_coord *_pst_position)
{
  /* Gets camera position */
  frame_2d_position_get(_pst_camera->pst_frame, _pst_position, TRUE);

  return;
}

/***************************************************************************
 camera_2d_rotation_get
 Camera 2D rotation get accessor.

 returns: void
 ***************************************************************************/
inline float camera_2d_rotation_get(camera_st_camera *_pst_camera)
{
  /* Gets camera position */
  return(frame_2d_rotation_get(_pst_camera->pst_frame, TRUE));
}

/***************************************************************************
 camera_2d_zoom_get
 Camera 2D zoom get accessor.

 returns: void
 ***************************************************************************/
inline float camera_2d_zoom_get(camera_st_camera *_pst_camera)
{
  /* Gets camera position */
  return(1.0 / frame_2d_scale_get(_pst_camera->pst_frame, TRUE));
}

/***************************************************************************
 camera_link_get
 Camera zoom get accessor.

 returns: link object pointer
 ***************************************************************************/
inline object_st_object *camera_link_get(camera_st_camera *_pst_camera)
{
  /* Gets camera link object */
  return(_pst_camera->pst_link);
}

/***************************************************************************
 camera_limit_set
 Camera limit set accessor (Upper left & Bottom right corners positions).

 returns: void
 ***************************************************************************/
inline void camera_limit_set(camera_st_camera *_pst_camera, coord_st_coord *_pst_ul, coord_st_coord *_pst_br)
{
  /* 2D? */
  if(camera_flag_test(_pst_camera, CAMERA_KUL_ID_FLAG_2D) != FALSE)
  {
    /* Sets camera limits position */
    coord_copy(&(((camera_st_data_2d *)(_pst_camera->pst_data))->st_limit_ul), _pst_ul);
    coord_copy(&(((camera_st_data_2d *)(_pst_camera->pst_data))->st_limit_br), _pst_br);

    /* Updates camera flags */
    _pst_camera->u32_id_flags |= CAMERA_KUL_ID_FLAG_LIMITED;
  }

  return;
}

/***************************************************************************
 camera_limit_set
 Camera limit reset accessor (Removes all position limits).

 returns: void
 ***************************************************************************/
inline void camera_limit_reset(camera_st_camera *_pst_camera)
{
  /* Updates camera flags */
  _pst_camera->u32_id_flags &= ~CAMERA_KUL_ID_FLAG_LIMITED;

  return;
}

/***************************************************************************
 camera_limit_get
 Camera limit get accessor (Upper left & Bottom right corners positions).

 returns: void
 ***************************************************************************/
inline void camera_limit_get(camera_st_camera *_pst_camera, coord_st_coord *_pst_ul, coord_st_coord *_pst_br)
{
  /* 2D? */
  if(camera_flag_test(_pst_camera, CAMERA_KUL_ID_FLAG_2D) != FALSE)
  {
    /* Gets camera limits position */
    coord_copy(_pst_ul, &(((camera_st_data_2d *)(_pst_camera->pst_data))->st_limit_ul));
    coord_copy(_pst_br, &(((camera_st_data_2d *)(_pst_camera->pst_data))->st_limit_br));
  }

  return;
}

/***************************************************************************
 camera_on_screen_position_get
 Camera on screen position get accessor.

 returns: void
 ***************************************************************************/
inline void camera_on_screen_position_get(camera_st_camera *_pst_camera, coord_st_coord *_pst_position)
{
  /* Copy on screen camera position coords */
  coord_copy(_pst_position, &(_pst_camera->st_on_screen_position));

  return;
}

/***************************************************************************
 camera_flag_test
 Camera flag test accessor.

 returns: bool
 ***************************************************************************/
bool camera_flag_test(camera_st_camera *_pst_camera, uint32 _u32_flag)
{
  if(_pst_camera->u32_id_flags & _u32_flag)
  {
    return TRUE;
  }

  return FALSE;
}

/***************************************************************************
 camera_flag_set
 Camera flag get/set accessor.

 returns: void
 ***************************************************************************/
void camera_flag_set(camera_st_camera *_pst_camera, uint32 _u32_add_flags, uint32 _u32_remove_flags)
{
  _pst_camera->u32_id_flags &= ~_u32_remove_flags;
  _pst_camera->u32_id_flags |= _u32_add_flags;

  return;
}
