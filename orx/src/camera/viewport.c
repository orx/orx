/***************************************************************************
 viewport.c
 Viewport module
 
 begin                : 14/12/2003
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


#include "camera/viewport.h"

#include "object/structure.h"


/*
 * Platform independant defines
 */

#define VIEWPORT_KUL_FLAG_NONE                0x00000000
#define VIEWPORT_KUL_FLAG_READY               0x00000001

#define VIEWPORT_KUL_ID_FLAG_NONE             0x00000000
#define VIEWPORT_KUL_ID_FLAG_ACTIVE           0x00000100
#define VIEWPORT_KUL_ID_FLAG_VIRTUAL          0x00001000
#define VIEWPORT_KUL_ID_FLAG_CAMERA         0x00010000
#define VIEWPORT_KUL_ID_FLAG_CAMERA           0x00010000
#define VIEWPORT_KUL_ID_FLAG_SURFACE          0x00100000

#define VIEWPORT_KUL_ID_MASK_ALIGN            0xF0000000

#define VIEWPORT_KUL_ID_MASK_NUMBER           0x00000007

#define VIEWPORT_KI_VIEWPORT_NUMBER           8


/*
 * Viewport structure
 */
struct st_viewport_t
{
  /* Public structure, first structure member : 16 */
  structure_st_struct st_struct;

  /* Internal id flags : 20 */
  uint32 u32_id_flags;

  /* Associated camera : 24 */
  structure_st_struct *pst_camera;

  /* Associated surface : 28 */
  texture_st_texture *pst_surface;

  /* Position coord : 44 */
  coord_st_coord st_position;

  /* Size coord : 60 */
  coord_st_coord st_size;

  /* Clip coords : 92 */
  coord_st_coord st_clip_position, st_clip_size;

  /* 4 extra bytes of padding : 96 */
  uint8 auc_unused[4];
};


/*
 * Static members
 */
static uint32 viewport_su32_flags = VIEWPORT_KUL_FLAG_NONE;
static bool viewport_sab_viewport_used[VIEWPORT_KI_VIEWPORT_NUMBER];


/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 viewport_clip_corners_compute
 Updates viewport clip corners.

 returns: void
 ***************************************************************************/
inline void viewport_clip_corners_compute(viewport_st_viewport *_pst_viewport)
{
  coord_st_coord *pst_pos, *pst_size, *pst_clip_position, *pst_clip_size;
  coord_st_coord st_cam_pos, st_cam_size;

  /* Gets internal pointer */
  pst_pos = &(_pst_viewport->st_position);
  pst_size = &(_pst_viewport->st_size);
  pst_clip_position = &(_pst_viewport->st_clip_position);
  pst_clip_size = &(_pst_viewport->st_clip_size);

  /* 2D? */
  if(camera_flag_test((camera_st_camera *)(_pst_viewport->pst_camera), CAMERA_KUL_ID_FLAG_2D) != FALSE)
  {
    /* Gets camera infos */
    camera_on_screen_position_get((camera_st_camera *)(_pst_viewport->pst_camera), &st_cam_pos);
    camera_2d_size_get((camera_st_camera *)(_pst_viewport->pst_camera), &st_cam_size);

    /* Clip position */

    /* X coordinate */
    if(pst_pos->s32_x > st_cam_pos.s32_x)
    {
      pst_clip_position->s32_x = pst_pos->s32_x;
    }
    else
    {
      pst_clip_position->s32_x = st_cam_pos.s32_x;
    }

    /* Y coordinate */
    if(pst_pos->s32_y > st_cam_pos.s32_y)
    {
      pst_clip_position->s32_y = pst_pos->s32_y;
    }
    else
    {
      pst_clip_position->s32_y = st_cam_pos.s32_y;
    }

    /* Clip size */

    /* X coordinate */
    if(pst_size->s32_x < st_cam_size.s32_x)
    {
      pst_clip_size->s32_x = pst_size->s32_x;
    }
    else
    {
      pst_clip_size->s32_x = st_cam_size.s32_x;
    }

    /* Y coordinate */
    if(pst_size->s32_y < st_cam_size.s32_y)
    {
      pst_clip_size->s32_y = pst_size->s32_y;
    }
    else
    {
      pst_clip_size->s32_y = st_cam_size.s32_y;
    }
  }

  return;
}

/***************************************************************************
 viewport_on_screen_camera_position_update
 Updates camera on screen position.

 returns: void
 ***************************************************************************/
inline void viewport_on_screen_camera_position_update(viewport_st_viewport *_pst_viewport)
{
  coord_st_coord *pst_pos, *pst_size;
  coord_st_coord st_result;
  uint32 u32_flags;
  int32 l_x = 0, l_y = 0;

  /* Gets viewport flags */
  u32_flags = _pst_viewport->u32_id_flags;

  /* Is virtual & has camera? */
  if((u32_flags & VIEWPORT_KUL_ID_FLAG_VIRTUAL) && (u32_flags & VIEWPORT_KUL_ID_FLAG_CAMERA))
  {
    /* 2D? */
    if(camera_flag_test((camera_st_camera *)(_pst_viewport->pst_camera), CAMERA_KUL_ID_FLAG_2D) != FALSE)
    {
      coord_st_coord st_cam_size;

      /* Gets internal pointer */
      pst_pos = &(_pst_viewport->st_position);
      pst_size = &(_pst_viewport->st_size);

      /* Gets camera size */
      camera_2d_size_get((camera_st_camera *)(_pst_viewport->pst_camera), &st_cam_size);

      /* X alignment */
      if(u32_flags & VIEWPORT_KUL_FLAG_ALIGN_LEFT)
      {
        /* Left aligned */
        l_x = pst_pos->s32_x;
      }
      else if(u32_flags & VIEWPORT_KUL_FLAG_ALIGN_RIGHT)
      {
        /* Right aligned */
        l_x = pst_pos->s32_x + pst_size->s32_x - st_cam_size.s32_x;
      }
      else
      {
        /* Center aligned */
        l_x = pst_pos->s32_x + (int)rintf(0.5 * (float)(pst_size->s32_x - st_cam_size.s32_x));
      }

      /* Y alignment */
      if(u32_flags & VIEWPORT_KUL_FLAG_ALIGN_TOP)
      {
        /* Left aligned */
        l_y = pst_pos->s32_y;
      }
      else if(u32_flags & VIEWPORT_KUL_FLAG_ALIGN_BOTTOM)
      {
        /* Right aligned */
        l_y = pst_pos->s32_y + pst_size->s32_y - st_cam_size.s32_y;
      }
      else
      {
        /* Center aligned */
        l_y = pst_pos->s32_y + (int)rintf(0.5 * (float)(pst_size->s32_y - st_cam_size.s32_y));
      }

      /* Stores it in a coord structure */
      coord_set(&st_result, l_x, l_y, 0);

      /* Updates camera screen position */
      camera_on_screen_position_set((camera_st_camera *)(_pst_viewport->pst_camera), &st_result);

      /* Computes clip corners */
      viewport_clip_corners_compute(_pst_viewport);
    }
  }

  return;
}

/***************************************************************************
 viewport_list_delete
 Deletes all viewports.

 returns: void
 ***************************************************************************/
void viewport_list_delete()
{
  viewport_st_viewport *pst_viewport = (viewport_st_viewport *)structure_struct_first_get(STRUCTURE_KUL_STRUCT_ID_VIEWPORT);

  /* Non null? */
  while(pst_viewport != NULL)
  {
    /* Deletes viewport */
    viewport_delete(pst_viewport);

    /* Gets first remaining viewport */
    pst_viewport = (viewport_st_viewport *)structure_struct_first_get(STRUCTURE_KUL_STRUCT_ID_VIEWPORT);
  }

  return;
}


/***************************************************************************
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/


/***************************************************************************
 viewport_init
 Inits viewport system.

 returns: EXIT_SUCCESS/EXIT_FAILURE
 ***************************************************************************/
uint32 viewport_init()
{
  int32 i;

  /* Not already Initialized? */
  if(!(viewport_su32_flags & VIEWPORT_KUL_FLAG_READY))
  {
    /* Inits viewports */
    for(i = 0; i < VIEWPORT_KI_VIEWPORT_NUMBER; i++)
    {
      viewport_sab_viewport_used[i] = FALSE;
    }

    /* Inits Flags */
    viewport_su32_flags = VIEWPORT_KUL_FLAG_READY;

    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}

/***************************************************************************
 viewport_exit
 Exits from the viewport system.

 returns: void
 ***************************************************************************/
void viewport_exit()
{
  /* Initialized? */
  if(viewport_su32_flags & VIEWPORT_KUL_FLAG_READY)
  {
    viewport_su32_flags &= ~VIEWPORT_KUL_FLAG_READY;

    /* Deletes viewport list */
    viewport_list_delete();
  }

  return;
}

/***************************************************************************
 viewport_create
 Creates a new empty viewport.

 returns: Created viewport.
 ***************************************************************************/
viewport_st_viewport *viewport_create()
{
  viewport_st_viewport *pst_viewport;
  int32 i_viewport = -1, i;

  /* Gets free viewport slot */
  for(i = 0; i < VIEWPORT_KI_VIEWPORT_NUMBER; i++)
  {
    /* Camera slot free? */
    if(viewport_sab_viewport_used[i] == FALSE)
    {
      i_viewport = i;
      break;
    }
  }

  /* No free slot? */
  if(i_viewport == -1)
  {
    /* !!! MSG !!! */

    return NULL;
  }

  /* Creates viewport */
  pst_viewport = (viewport_st_viewport *) malloc(sizeof(viewport_st_viewport));

  /* Non null? */
  if(pst_viewport != NULL)
  {
    /* Inits structure */
    if(structure_struct_init((structure_st_struct *)pst_viewport, STRUCTURE_KUL_STRUCT_ID_VIEWPORT) != EXIT_SUCCESS)
    {
      /* !!! MSG !!! */

      /* Fress partially allocated viewport */
      free(pst_viewport);

      /* Returns nothing */
      return NULL;
    }

    /* Inits viewport members */
    pst_viewport->u32_id_flags = VIEWPORT_KUL_ID_FLAG_VIRTUAL   |
                                VIEWPORT_KUL_ID_FLAG_ACTIVE    |
                                VIEWPORT_KUL_FLAG_ALIGN_CENTER |
                                VIEWPORT_KUL_FLAG_ALIGN_CENTER |
                                (uint32)i_viewport;
    pst_viewport->pst_camera = NULL;
    pst_viewport->pst_surface = NULL;
    coord_set(&(pst_viewport->st_position), 0, 0, 0);
    coord_set(&(pst_viewport->st_size), 0, 0, 0);
    coord_set(&(pst_viewport->st_clip_position), 0, 0, 0);
    coord_set(&(pst_viewport->st_clip_size), 0, 0, 0);

    /* Updates viewport slot */
    viewport_sab_viewport_used[i_viewport] = TRUE;
  }

  return(pst_viewport);
}

/***************************************************************************
 viewport_delete
 Deletes a viewport.

 returns: void
 ***************************************************************************/
void viewport_delete(viewport_st_viewport *_pst_viewport)
{
  int32 i_camera;

  /* Non null? */
  if(_pst_viewport != NULL)
  {
    /* Gets camera id number */
    i_camera = (int) (_pst_viewport->u32_id_flags & VIEWPORT_KUL_ID_MASK_NUMBER);

    /* Frees viewport slot */
    viewport_sab_viewport_used[i_camera] = FALSE;

    /* Was linked to a camera? */
    if(_pst_viewport->pst_camera != NULL)
    {
      structure_struct_counter_decrease((structure_st_struct *)(_pst_viewport->pst_camera));
    }

    /* Was linked to a surface? */
    if(_pst_viewport->pst_surface != NULL)
    {
      structure_struct_counter_decrease((structure_st_struct *)(_pst_viewport->pst_surface));
    }

    /* Cleans structure */
    structure_struct_clean((structure_st_struct *)_pst_viewport);

    /* Frees viewport memory */
    free(_pst_viewport);
  }

  return;
}


/* *** Structure accessors *** */


/***************************************************************************
 viewport_camera_set
 Viewport camera set accessor.

 returns: void
 ***************************************************************************/
inline void viewport_camera_set(viewport_st_viewport *_pst_viewport, camera_st_camera *_pst_camera)
{
  /* Had already a camera? */
  if(_pst_viewport->u32_id_flags & VIEWPORT_KUL_ID_FLAG_CAMERA)
  {
    /* Updates reference counter */
    structure_struct_counter_decrease(_pst_viewport->pst_camera);

    /* Updates flags */
    _pst_viewport->u32_id_flags &= ~VIEWPORT_KUL_ID_FLAG_CAMERA;
  }

  /* Adds a new camera? */
  if(_pst_camera != NULL)
  {
    /* Updates pointer */
    _pst_viewport->pst_camera = (structure_st_struct *)_pst_camera;

    /* Updates reference counter */
    structure_struct_counter_increase((structure_st_struct *)_pst_camera);

    /* Updates flags */
    _pst_viewport->u32_id_flags |= VIEWPORT_KUL_ID_FLAG_CAMERA;
  }

  /* Updates camera on screen position */
  viewport_on_screen_camera_position_update(_pst_viewport);

  return;
} 

/***************************************************************************
 viewport_position_set
 Viewport position set accessor.

 returns: void
 ***************************************************************************/
inline void viewport_position_set(viewport_st_viewport *_pst_viewport, coord_st_coord *_pst_position)
{
  /* Updates position */
  coord_copy(&(_pst_viewport->st_position), _pst_position);

  /* Updates camera on screen position */
  viewport_on_screen_camera_position_update(_pst_viewport);

  return;
}

/***************************************************************************
 viewport_size_set
 Viewport size set accessor.

 returns: void
 ***************************************************************************/
inline void viewport_size_set(viewport_st_viewport *_pst_viewport, coord_st_coord *_pst_size)
{
  /* Updates position */
  coord_copy(&(_pst_viewport->st_size), _pst_size);

  /* Updates camera on screen position */
  viewport_on_screen_camera_position_update(_pst_viewport);

  return;
}


/***************************************************************************
 viewport_camera_get
 Viewport camera get accessor.

 returns: void
 ***************************************************************************/
inline camera_st_camera *viewport_camera_get(viewport_st_viewport *_pst_viewport)
{
  /* Had a camera? */
  if(_pst_viewport->u32_id_flags & VIEWPORT_KUL_ID_FLAG_CAMERA)
  {
    return (camera_st_camera *)(_pst_viewport->pst_camera);
  }
  else
  {
    return NULL;
  }
}

/***************************************************************************
 viewport_position_get
 Viewport position get accessor.

 returns: void
 ***************************************************************************/
inline void viewport_position_get(viewport_st_viewport *_pst_viewport, coord_st_coord *_pst_position)
{
  /* gets position */
  coord_copy(_pst_position, &(_pst_viewport->st_position));

  return;
}

/***************************************************************************
 viewport_size_get
 Viewport size get accessor.

 returns: void
 ***************************************************************************/
inline void viewport_size_get(viewport_st_viewport *_pst_viewport, coord_st_coord *_pst_size)
{
  /* gets position */
  coord_copy(_pst_size, &(_pst_viewport->st_size));

  return;
}


/***************************************************************************
 viewport_align_set
 Viewport alignment set accessor (flags must be OR'ed).

 returns: void
 ***************************************************************************/
inline void viewport_align_set(viewport_st_viewport *_pst_viewport, uint32 _u32_align)
{
  /* Cleans current alignment */
  _pst_viewport->u32_id_flags &= ~VIEWPORT_KUL_ID_MASK_ALIGN;

  /* Updates alignement */
  _pst_viewport->u32_id_flags |= (_u32_align & VIEWPORT_KUL_ID_MASK_ALIGN);

  return;
}

/***************************************************************************
 viewport_surface_set
 Viewport surface set accessor.

 returns: void
 ***************************************************************************/
inline void viewport_surface_set(viewport_st_viewport *_pst_viewport, texture_st_texture *_pst_surface)
{
  /* Has already a surface? */
  if(_pst_viewport->u32_id_flags & VIEWPORT_KUL_ID_FLAG_SURFACE)
  {
    /* Updates surface reference counter */
    structure_struct_counter_decrease((structure_st_struct *)(_pst_viewport->pst_surface));

    /* Updates flags */
    _pst_viewport->u32_id_flags &= ~VIEWPORT_KUL_ID_FLAG_SURFACE;
  }

  /* Updates surface pointer */
  _pst_viewport->pst_surface = _pst_surface;
  
  /* Is new surface non null? */
  if(_pst_surface != NULL)
  {
    /* Updates surface reference counter */
    structure_struct_counter_increase((structure_st_struct *)_pst_surface);

    /* Updates flags */
    _pst_viewport->u32_id_flags |= VIEWPORT_KUL_ID_FLAG_SURFACE;
  }
  else
  {
    /* Deactivates viewport */
    _pst_viewport->u32_id_flags &= ~VIEWPORT_KUL_ID_FLAG_ACTIVE;
  }

  return;
}

/***************************************************************************
 viewport_surface_get
 Viewport surface get accessor

 returns: void
 ***************************************************************************/
inline texture_st_texture *viewport_surface_get(viewport_st_viewport *_pst_viewport)
{
  /* Has surface? */
  if(_pst_viewport->u32_id_flags & VIEWPORT_KUL_ID_FLAG_SURFACE)
  {
    return _pst_viewport->pst_surface;
  }
  else
  {
    return NULL;
  }
}

/***************************************************************************
 viewport_activate
 Viewport activate accessor.

 returns: void
 ***************************************************************************/
inline void viewport_activate(viewport_st_viewport *_pst_viewport, bool _b_activation)
{
  /* Updates activation flag */
  if(_b_activation == FALSE)
  {
    _pst_viewport->u32_id_flags &= ~VIEWPORT_KUL_ID_FLAG_ACTIVE;
  }
  else
  {
    _pst_viewport->u32_id_flags |= VIEWPORT_KUL_ID_FLAG_ACTIVE;
  }

  return;
}

/***************************************************************************
 viewport_active
 Viewport is active accessor.

 returns: TRUE/FALSE
 ***************************************************************************/
inline bool viewport_active(viewport_st_viewport *_pst_viewport)
{
  /* Active? */
  if(_pst_viewport->u32_id_flags & VIEWPORT_KUL_ID_FLAG_ACTIVE)
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

/***************************************************************************
 viewport_clip_get
 Viewport clip get accessor.

 returns: void
 ***************************************************************************/
inline void viewport_clip_get(viewport_st_viewport * _pst_viewport, coord_st_coord *_pst_position, coord_st_coord *_pst_size)
{
  /* Gets clip infos */
  coord_copy(_pst_position, &(_pst_viewport->st_clip_position));
  coord_copy(_pst_size, &(_pst_viewport->st_clip_size));

  return;
}
