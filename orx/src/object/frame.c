/***************************************************************************
 frame.c
 frame module

 begin                : 02/12/2003
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


#include "object/frame.h"

#include "object/structure.h"


/*
 * Platform independant defines
 */

#define FRAME_KUL_FLAG_NONE             0x00000000
#define FRAME_KUL_FLAG_READY            0x00000001
#define FRAME_KUL_FLAG_DATA_2D          0x00000010
#define FRAME_KUL_FLAG_DEFAULT          0x00000010


#define FRAME_KUL_ID_FLAG_NONE          0x00000000
#define FRAME_KUL_ID_FLAG_DATA_2D       0x00000010
#define FRAME_KUL_ID_FLAG_VALUE_DIRTY   0x10000000
#define FRAME_KUL_ID_FLAG_RENDER_DIRTY  0x20000000
#define FRAME_KUL_ID_FLAG_DIRTY         0x30000000
#define FRAME_KUL_ID_FLAG_SCROLL_X      0x01000000
#define FRAME_KUL_ID_FLAG_SCROLL_Y      0x02000000
#define FRAME_KUL_ID_FLAG_SCROLL_BOTH   0x03000000


#define FRAME_KI_COORD_GLOBAL           1
#define FRAME_KI_COORD_LOCAL            2

/*
 * Internal 2D Frame Data structure
 */
typedef struct st_data_2d_t
{
  /* Global 2D coordinates : 16 */
  coord_st_coord st_global_coord;
  /* Global 2D rotation angle : 20 */
  float f_global_angle;
  /* Global 2D isometric scale : 24 */
  float f_global_scale;

  /* Local 2D coordinates : 40 */
  coord_st_coord st_local_coord;
  /* Local 2D rotation angle : 44 */
  float f_local_angle;
  /* Local 2D isometric scale : 48 */
  float f_local_scale;

  /* X axis scroll coefficient used for differential scrolling : 52 */
  float f_scroll_x;
  /* Y axis scroll coefficient used for differential scrolling : 56 */
  float f_scroll_y;

  /* 8 extra bytes of padding : 64 */
  uint8 auc_unused[8];
} frame_st_data_2d;


/*
 * Frame structure
 */
struct st_frame_t
{
  /* Public structure, first structure member : 16 */
  structure_st_struct st_struct;

  /* Internal id flags : 20 */
  uint32 u32_id_flags;

  /* Data : 24 */
  void *pst_data;

  /* 8 extra bytes of padding : 32 */
  uint8 auc_unused[8];
};



/*
 * Static members
 */
static uint32 frame_su32_flags = FRAME_KUL_FLAG_DEFAULT;
static frame_st_frame *frame_spst_root = NULL;


/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 frame_coord_set
 Sets a 2D frame local coord

 returns: void
 ***************************************************************************/
inline void frame_coord_set(frame_st_frame *_pst_frame, coord_st_coord *_pst_coord, int32 _i_coord_type)
{
  /* According to coord type */
  switch(_i_coord_type)
  {
    case FRAME_KI_COORD_GLOBAL:
      coord_copy(&(((frame_st_data_2d *)(_pst_frame->pst_data))->st_global_coord), _pst_coord);
      break;

    case FRAME_KI_COORD_LOCAL:
      coord_copy(&(((frame_st_data_2d *)(_pst_frame->pst_data))->st_local_coord), _pst_coord);
      break;

    default:
      /* Wrong coord type */
      /* !!! MSG !!! */
      break;
  }

  return;
}

/***************************************************************************
 frame_angle_set
 Sets a 2D frame local angle

 returns: void
 ***************************************************************************/
inline void frame_angle_set(frame_st_frame *_pst_frame, float _f_angle, int32 _i_coord_type)
{
  /* According to coord type */
  switch(_i_coord_type)
  {
    case FRAME_KI_COORD_GLOBAL:
      ((frame_st_data_2d *)(_pst_frame->pst_data))->f_global_angle = _f_angle;
      break;

    case FRAME_KI_COORD_LOCAL:
      ((frame_st_data_2d *)(_pst_frame->pst_data))->f_local_angle = _f_angle;
      break;

    default:
      /* Wrong coord type */
      /* !!! MSG !!! */
      break;
  }

  return;
}

/***************************************************************************
 frame_scale_set
 Sets a 2D frame local scale

 returns: void
 ***************************************************************************/
inline void frame_scale_set(frame_st_frame *_pst_frame, float _f_scale, int32 _i_coord_type)
{
  /* According to coord type */
  switch(_i_coord_type)
  {
    case FRAME_KI_COORD_GLOBAL:
      ((frame_st_data_2d *)(_pst_frame->pst_data))->f_global_scale = _f_scale;
      break;

    case FRAME_KI_COORD_LOCAL:
      ((frame_st_data_2d *)(_pst_frame->pst_data))->f_local_scale = _f_scale;
      break;

    default:
      /* Wrong coord type */
      /* !!! MSG !!! */
      break;
  }

  return;
}

/***************************************************************************
 frame_coord_get
 Gets a 2D frame local/global coord

 returns: Internal coord data pointer
 ***************************************************************************/
inline coord_st_coord *frame_coord_get(frame_st_frame *_pst_frame, int32 _i_coord_type)
{
  coord_st_coord *pst_coord = NULL;

  /* According to coord type */
  switch(_i_coord_type)
  {
    case FRAME_KI_COORD_GLOBAL:
      pst_coord = &(((frame_st_data_2d *)(_pst_frame->pst_data))->st_global_coord);
      break;

    case FRAME_KI_COORD_LOCAL:
      pst_coord = &(((frame_st_data_2d *)(_pst_frame->pst_data))->st_local_coord);
      break;

    default:
      /* Wrong coord type */
      /* !!! MSG !!! */
      break;
  }

  return pst_coord;
}

/***************************************************************************
 frame_angle_get
 Gets a 2D frame local/global angle

 returns: Requested angle value
 ***************************************************************************/
inline float frame_angle_get(frame_st_frame *_pst_frame, int32 _i_coord_type)
{
  float f_angle = 0.0;

  /* According to coord type */
  switch(_i_coord_type)
  {
    case FRAME_KI_COORD_GLOBAL:
      f_angle = ((frame_st_data_2d *)(_pst_frame->pst_data))->f_global_angle;
      break;

    case FRAME_KI_COORD_LOCAL:
      f_angle = ((frame_st_data_2d *)(_pst_frame->pst_data))->f_local_angle;
      break;

    default:
      /* Wrong coord type */
      /* !!! MSG !!! */
      break;
  }

  return f_angle;
}

/***************************************************************************
 frame_scale_get
 Gets a 2D frame local/global scale

 returns: Requested scale value
 ***************************************************************************/
inline float frame_scale_get(frame_st_frame *_pst_frame, int32 _i_coord_type)
{
  float f_scale = 1.0;
 
  /* According to coord type */
  switch(_i_coord_type)
  {
    case FRAME_KI_COORD_GLOBAL:
      f_scale = ((frame_st_data_2d *)(_pst_frame->pst_data))->f_global_scale;
      break;

    case FRAME_KI_COORD_LOCAL:
      f_scale = ((frame_st_data_2d *)(_pst_frame->pst_data))->f_local_scale;
      break;

    default:
      /* Wrong coord type */
      /* !!! MSG !!! */
      break;
  }

  return f_scale;
}

/***************************************************************************
 frame_data_update
 Updates frame global data using parent's global and frame local ones.
 Result can be stored in a third party frame.

 returns: void
 ***************************************************************************/
inline void frame_data_update(frame_st_frame *_pst_dest_frame, frame_st_frame *_pst_src_frame, frame_st_frame *_pst_parent_frame)
{
  /* 2D data? */
  if(_pst_src_frame->u32_id_flags & FRAME_KUL_ID_FLAG_DATA_2D)
  {
    coord_st_coord st_temp_coord, *pst_parent_coord, *pst_coord;
    float f_parent_angle, f_parent_scale, f_angle, f_scale;
    float f_x, f_y, f_local_x, f_local_y, f_cos, f_sin;

    /* Gets parent's global data */
    pst_parent_coord = frame_coord_get(_pst_parent_frame, FRAME_KI_COORD_GLOBAL);
    f_parent_angle = frame_angle_get(_pst_parent_frame, FRAME_KI_COORD_GLOBAL);
    f_parent_scale = frame_scale_get(_pst_parent_frame, FRAME_KI_COORD_GLOBAL);

    /* Gets frame's local coord */
    pst_coord = frame_coord_get(_pst_src_frame, FRAME_KI_COORD_LOCAL);

    /* Updates angle */
    f_angle = frame_angle_get(_pst_src_frame, FRAME_KI_COORD_LOCAL) + f_parent_angle;

    /* Updates scale */
    f_scale = frame_scale_get(_pst_src_frame, FRAME_KI_COORD_LOCAL) * f_parent_scale;

    /* Updates coord */
    /* Gets needed float values for rotation & scale applying */
    f_local_x = (float)(pst_coord->s32_x);
    f_local_y = (float)(pst_coord->s32_y);
    f_cos = cosf(f_parent_angle);
    f_sin = sinf(f_parent_angle);

    /* Applies rotation & scale on X & Y coordinates*/
    f_x = f_parent_scale * ((f_local_x * f_cos) - (f_local_y * f_sin));
    f_y = f_parent_scale * ((f_local_x * f_sin) + (f_local_y * f_cos));

    /* Computes final global coordinates */
    st_temp_coord.s32_x = (int)rintf(f_x) + pst_parent_coord->s32_x;
    st_temp_coord.s32_y = (int)rintf(f_y) + pst_parent_coord->s32_y;

    /* Z coordinate is not affected by rotation nor scale in 2D */
    st_temp_coord.s32_z = pst_parent_coord->s32_z + pst_coord->s32_z;

    /* Stores them */
    /* Is destination frame non null? */
    if(_pst_dest_frame != NULL)
    {
      frame_angle_set(_pst_dest_frame, f_angle, FRAME_KI_COORD_GLOBAL);
      frame_scale_set(_pst_dest_frame, f_scale, FRAME_KI_COORD_GLOBAL);
      frame_coord_set(_pst_dest_frame, &st_temp_coord, FRAME_KI_COORD_GLOBAL);
    }
    else
    {
      frame_angle_set(_pst_src_frame, f_angle, FRAME_KI_COORD_GLOBAL);
      frame_scale_set(_pst_src_frame, f_scale, FRAME_KI_COORD_GLOBAL);
      frame_coord_set(_pst_src_frame, &st_temp_coord, FRAME_KI_COORD_GLOBAL);
    }
  }

 return;
}

/***************************************************************************
 frame_dirty_process
 Process a dirty frame and all its dirty ancestors.

 returns: void
 ***************************************************************************/
void frame_dirty_process(frame_st_frame *_pst_frame)
{
  frame_st_frame *pst_parent_frame = (frame_st_frame *)structure_struct_parent_get((structure_st_struct *)_pst_frame);

  /* Is cell dirty & has parent? */
  if(((_pst_frame->u32_id_flags) & FRAME_KUL_ID_FLAG_VALUE_DIRTY)
  && (pst_parent_frame != NULL))
  {
    /* Updates parent status */
    frame_dirty_process(pst_parent_frame);

    /* Updates frame global data */
    frame_data_update(NULL, _pst_frame, pst_parent_frame);
  }

  /* Updates cell dirty status */
  _pst_frame->u32_id_flags &= ~FRAME_KUL_ID_FLAG_VALUE_DIRTY;

 return;
}

/***************************************************************************
 frame_recursive_flag_set
 Sets a frame and all its heirs as requested.

 returns: void
 ***************************************************************************/
void frame_recursive_flag_set(frame_st_frame *_pst_frame, uint32 _u32_add_flags, uint32 _u32_remove_flags, bool _b_recursed)
{
  /* Non null? */
  if(_pst_frame != NULL)
  {
    /* Updates child status */
    frame_recursive_flag_set((frame_st_frame *)structure_struct_child_get((structure_st_struct *)_pst_frame), _u32_add_flags, _u32_remove_flags, TRUE);

    /* Recursed? */
    if(_b_recursed)
    {
      /* Updates siblings status */
      frame_recursive_flag_set((frame_st_frame *)structure_struct_right_sibling_get((structure_st_struct *)_pst_frame), _u32_add_flags, _u32_remove_flags, TRUE);
    }

    /* Updates cell flags */
    _pst_frame->u32_id_flags &= ~(_u32_remove_flags);
    _pst_frame->u32_id_flags |= _u32_add_flags;
  }

 return;
}


/***************************************************************************
 frame_dirty_set
 Sets a frame and all its heirs as dirty.

 returns: void
 ***************************************************************************/
void frame_dirty_set(frame_st_frame *_pst_frame)
{
  /* Adds dirty flags (render + value) to all frame's heirs */
  frame_recursive_flag_set(_pst_frame, FRAME_KUL_ID_FLAG_DIRTY, FRAME_KUL_ID_FLAG_NONE, FALSE);

 return;
}

/***************************************************************************
 frame_tree_delete
 Deletes all the frames stored in the tree and cleans it.

 returns: void
 ***************************************************************************/
inline void frame_tree_delete()
{
  frame_st_frame *pst_frame = (frame_st_frame *)structure_struct_child_get((structure_st_struct *)frame_spst_root);

  /* Untill only root remains */
  while(pst_frame != NULL)
  {
    /* Deletes firt child cell */
    frame_delete(pst_frame);

    /* Gets root new child */
    pst_frame = (frame_st_frame *)structure_struct_child_get((structure_st_struct *)frame_spst_root);
  }

  /* Removes root */
  frame_delete(frame_spst_root);

  frame_spst_root = NULL;

  return;
}


/***************************************************************************
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/


/***************************************************************************
 frame_init
 Inits frame system.

 returns: EXIT_SUCCESS/EXIT_FAILURE
 ***************************************************************************/
uint32 frame_init()
{
  /* Not already Initialized? */
  if(!(frame_su32_flags & FRAME_KUL_FLAG_READY))
  {
    /* Inits ID Flags */
    frame_su32_flags = FRAME_KUL_FLAG_DEFAULT|FRAME_KUL_FLAG_READY;

    /* Inits frame tree */
    frame_spst_root = frame_create();

    /* Non null? */
    if(frame_spst_root != NULL)
    {
      return EXIT_SUCCESS;
    }
  }

  return EXIT_FAILURE;
}

/***************************************************************************
 frame_exit
 Exits from frame system.

 returns: void
 ***************************************************************************/
void frame_exit()
{
  /* Initialized? */
  if(frame_su32_flags & FRAME_KUL_FLAG_READY)
  {
    frame_su32_flags &= ~FRAME_KUL_FLAG_READY;

    /* Deletes frame tree */
    frame_tree_delete();
  }

  return;
}

/***************************************************************************
 frame_create
 Creates a new frame.

 returns: Created frame.
 ***************************************************************************/
frame_st_frame *frame_create()
{
  frame_st_frame *pst_frame;

  /* Creates frame */
  pst_frame = (frame_st_frame *) malloc(sizeof(frame_st_frame));

  /* Non null? */
  if(pst_frame != NULL)
  {
    /* Inits members */
    if(frame_su32_flags & FRAME_KUL_FLAG_DATA_2D)
    {
      frame_st_data_2d *pst_data;

      /* Updates flags */
      pst_frame->u32_id_flags = FRAME_KUL_ID_FLAG_DATA_2D;

      /* Allocates data memory */
      pst_data = (frame_st_data_2d *) malloc(sizeof(frame_st_data_2d));

      /* Inits & assigns it */
      if(pst_data != NULL)
      {
        /* Cleans values */
        coord_set(&(pst_data->st_global_coord), 0, 0, 0);
        coord_set(&(pst_data->st_local_coord), 0, 0, 0);
        pst_data->f_global_angle = 0.0;
        pst_data->f_local_angle = 0.0;
        pst_data->f_global_scale = 1.0;
        pst_data->f_local_scale = 1.0;
        pst_data->f_scroll_x = 0.0;
        pst_data->f_scroll_y = 0.0;

        /* Links data to frame */
        pst_frame->pst_data = pst_data;
      }
      else
      {
        /* Deletes partially created frame */
        free(pst_frame);

        return NULL;
      }
    }

    /* Inits structure */
    if(structure_struct_init((structure_st_struct *)pst_frame, STRUCTURE_KUL_STRUCT_ID_FRAME) != EXIT_SUCCESS)
    {
      /* !!! MSG !!! */

      /* Fress partially allocated texture */
      free(pst_frame);

      /* Returns nothing */
      return NULL;
    }
  }

  return pst_frame;
}

/***************************************************************************
 frame_delete
 Deletes a frame.

 returns: void
 ***************************************************************************/
void frame_delete(frame_st_frame *_pst_frame)
{
  /* Non null? */
  if(_pst_frame != NULL)
  {
    /* Cleans structure */
    structure_struct_clean((structure_st_struct *)_pst_frame);

    /* Cleans data */
    if(_pst_frame->u32_id_flags & FRAME_KUL_ID_FLAG_DATA_2D)
    {
      /* Frees frame data memory */
      free(_pst_frame->pst_data);
    }

    /* Frees frame memory */
    free(_pst_frame);
  }

  return;
}

/***************************************************************************
 frame_render_status_ok
 Test frame render status (TRUE : clean / FALSE : dirty)

 returns: TRUE (clean) / FALSE (dirty)
 ***************************************************************************/
inline bool frame_render_status_ok(frame_st_frame *_pst_frame)
{
  /* Non null? */
  if(_pst_frame != NULL)
  {
    /* Test render dirty flag */
    if(_pst_frame->u32_id_flags & FRAME_KUL_ID_FLAG_RENDER_DIRTY)
    {
      return FALSE;
    }
    else
    {
      return TRUE;
    }
  }

  return TRUE;
}

/***************************************************************************
 frame_render_status_clean
 Cleans all frames render status

 returns: void
 ***************************************************************************/
void frame_render_status_clean()
{
  /* Removes render dirty flag from all frames */
  frame_recursive_flag_set(frame_spst_root, FRAME_KUL_ID_FLAG_NONE, FRAME_KUL_ID_FLAG_RENDER_DIRTY, FALSE);

  return;
}

/***************************************************************************
 frame_differential_scrolling_use
 Does frame use differential scrolling?

 returns: TRUE/FALSE
 ***************************************************************************/
inline bool frame_differential_scrolling_use(frame_st_frame *_pst_frame)
{
  /* Non null? */
  if(_pst_frame != NULL)
  {
    if(_pst_frame->u32_id_flags & FRAME_KUL_ID_FLAG_SCROLL_BOTH)
    {
      return TRUE;
    }
  }

  return FALSE;
}

/***************************************************************************
 frame_differential_scrolling_get
 Gets frame differential scrolling (X & Y axis)

 returns: void
 ***************************************************************************/
inline void frame_differential_scrolling_get(frame_st_frame * _pst_frame, float *_pf_x_axis, float *_pf_y_axis)
{
  /* Non null & use 2D data? */
  if((_pst_frame != NULL) && (_pst_frame->u32_id_flags & FRAME_KUL_ID_FLAG_DATA_2D))
  {
    /* Uses X scroll? */
    if(_pst_frame->u32_id_flags & FRAME_KUL_ID_FLAG_SCROLL_X)
    {
      /* Stores value */
      *_pf_x_axis = ((frame_st_data_2d *)(_pst_frame->pst_data))->f_scroll_x;
    }
    else
    {
      /* Stores value */
      *_pf_x_axis = 0.0;
    }

    /* Uses Y scroll? */
    if(_pst_frame->u32_id_flags & FRAME_KUL_ID_FLAG_SCROLL_Y)
    {
      /* Stores value */
      *_pf_y_axis = ((frame_st_data_2d *)(_pst_frame->pst_data))->f_scroll_y;
    }
    else
    {
      /* Stores value */
      *_pf_y_axis = 0.0;
    }
  }

  return;
}

/***************************************************************************
 frame_differential_scrolling_set
 Sets frame differential scrolling (X & Y axis)

 returns: void
 ***************************************************************************/
inline void frame_differential_scrolling_set(frame_st_frame * _pst_frame, float _f_x_axis, float _f_y_axis)
{
  uint32 u32_add_flags = FRAME_KUL_ID_FLAG_NONE, u32_remove_flags = FRAME_KUL_ID_FLAG_NONE;

  /* Non null & use 2D data? */
  if((_pst_frame != NULL) && (_pst_frame->u32_id_flags & FRAME_KUL_ID_FLAG_DATA_2D))
  {
    /* Enables X axis differential scrolling? */
    if(_f_x_axis != 0.0)
    {
      u32_add_flags |= FRAME_KUL_ID_FLAG_SCROLL_X;

      /* Updates coef */
      ((frame_st_data_2d *)(_pst_frame->pst_data))->f_scroll_x = _f_x_axis;
    }
    else
    {
      u32_remove_flags |= FRAME_KUL_ID_FLAG_SCROLL_X;
    }

    /* Enables Y axis differential scrolling? */
    if(_f_y_axis != 0.0)
    {
      u32_add_flags |= FRAME_KUL_ID_FLAG_SCROLL_Y;

      /* Updates coef */
      ((frame_st_data_2d *)(_pst_frame->pst_data))->f_scroll_y = _f_y_axis;
    }
    else
    {
      u32_remove_flags |= FRAME_KUL_ID_FLAG_SCROLL_Y;
    }

    /* Updates flags on frame and its heirs */
    frame_recursive_flag_set(_pst_frame, u32_add_flags, u32_remove_flags, FALSE);
  }

  return;
}

/***************************************************************************
 frame_parent_set
 Sets a frame parent & updates links.

 returns: void
 ***************************************************************************/
inline void frame_parent_set(frame_st_frame *_pst_frame, frame_st_frame *_pst_parent)
{
  /* Non null? */
  if(_pst_frame != NULL)
  {
    /* If parent is null, root will become parent */
    if(_pst_parent == NULL)
    {
      structure_struct_parent_set((structure_st_struct *)_pst_frame, (structure_st_struct *)frame_spst_root);
    }
    else
    {
      structure_struct_parent_set((structure_st_struct *)_pst_frame, (structure_st_struct *)_pst_parent);
    }

    /* Tags as dirty */
    frame_dirty_set(_pst_frame);
  }

  return;
}

/***************************************************************************
 frame_2d_position_set
 Sets a 2D frame local position

 returns: void
 ***************************************************************************/
inline void frame_2d_position_set(frame_st_frame *_pst_frame, coord_st_coord *_pst_coord)
{
  /* Non null? */
  if(_pst_frame != NULL)
  {
    /* Updates coord values */
    frame_coord_set(_pst_frame, _pst_coord, FRAME_KI_COORD_LOCAL);

    /* Tags as dirty */
    frame_dirty_set(_pst_frame);
  }

  return;
}

/***************************************************************************
 frame_2d_rotation_set
 Sets a 2D frame local rotation

 returns: void
 ***************************************************************************/
inline void frame_2d_rotation_set(frame_st_frame *_pst_frame, float _f_angle)
{
  /* Non null? */
  if(_pst_frame != NULL)
  {
    /* Updates angle value */
    frame_angle_set(_pst_frame, _f_angle, FRAME_KI_COORD_LOCAL);

    /* Tags as dirty */
    frame_dirty_set(_pst_frame);
  }

  return;
}

/***************************************************************************
 frame_2d_scale_set
 Sets a 2D frame local scale

 returns: void
 ***************************************************************************/
inline void frame_2d_scale_set(frame_st_frame *_pst_frame, float _f_scale)
{
  /* Non null? */
  if(_pst_frame != NULL)
  {
    /* Updates scale value */
    frame_scale_set(_pst_frame, _f_scale, FRAME_KI_COORD_LOCAL);

    /* Tags as dirty */
    frame_dirty_set(_pst_frame);
  }

  return;
}

/***************************************************************************
 frame_2d_position_get
 Gets a 2D frame local/global position

 returns: void
 ***************************************************************************/
inline void frame_2d_position_get(frame_st_frame *_pst_frame, coord_st_coord *_pst_coord, bool _b_local)
{
  coord_st_coord *pst_intern = NULL;
 
  /* Is Frame 2D? */
  if((_pst_frame != NULL) && (_pst_frame->u32_id_flags & FRAME_KUL_ID_FLAG_DATA_2D))
  {
    /* Local coordinates? */
    if(_b_local != FALSE)
    {
      pst_intern = frame_coord_get(_pst_frame, FRAME_KI_COORD_LOCAL);
    }
    else
    {
      /* Process dirty cell */
      frame_dirty_process(_pst_frame);

      /* Gets requested position */
      pst_intern = frame_coord_get(_pst_frame, FRAME_KI_COORD_GLOBAL);
    }

    /* Makes a copy */
    coord_copy(_pst_coord, pst_intern);
  }
  else
  {
    /* Resets coord structure */
    coord_reset(_pst_coord);
  }

  return;
}

/***************************************************************************
 frame_2d_rotation_get
 Gets a 2D frame local/global rotation

 returns: Requested rotation value
 ***************************************************************************/
inline float frame_2d_rotation_get(frame_st_frame *_pst_frame, bool _b_local)
{
  float f_angle = 0.0;
 
  /* Is Frame 2D? */
  if((_pst_frame != NULL) && (_pst_frame->u32_id_flags & FRAME_KUL_ID_FLAG_DATA_2D))
  {
    /* Local coordinates? */
    if(_b_local != FALSE)
    {
      f_angle = frame_angle_get(_pst_frame, FRAME_KI_COORD_LOCAL);
    }
    else
    {
      /* Process dirty cell */
      frame_dirty_process(_pst_frame);

      /* Gets requested rotation */
      f_angle = frame_angle_get(_pst_frame, FRAME_KI_COORD_GLOBAL);
    }
  }

  return f_angle;
}

/***************************************************************************
 frame_2d_scale_get
 Gets a 2D frame local/global scale

 returns: Requested scale value
 ***************************************************************************/
inline float frame_2d_scale_get(frame_st_frame *_pst_frame, bool _b_local)
{
  float f_scale = 1.0;
 
  /* Is Frame 2D? */
  if((_pst_frame != NULL) && (_pst_frame->u32_id_flags & FRAME_KUL_ID_FLAG_DATA_2D))
  {
    /* Local coordinates? */
    if(_b_local != FALSE)
    {
      f_scale = frame_scale_get(_pst_frame, FRAME_KI_COORD_LOCAL);
    }
    else
    {
      /* Process dirty cell */
      frame_dirty_process(_pst_frame);

      /* Gets requested scale */
      f_scale = frame_scale_get(_pst_frame, FRAME_KI_COORD_GLOBAL);
    }
  }

  return f_scale;
}

/***************************************************************************
 frame_global_data_compute
 Computes frame global data using parent's global and frame local ones.
 Result is stored in a third party frame.

 returns: void
 ***************************************************************************/
inline void frame_global_data_compute(frame_st_frame *_pst_src_frame, frame_st_frame *_pst_parent_frame, frame_st_frame *_pst_dest_frame)
{
  /* Non null & not self? */
  if((_pst_dest_frame != NULL) && (_pst_dest_frame != _pst_src_frame))
  {
    /* Computes frame global data */
    frame_data_update(_pst_dest_frame, _pst_src_frame, _pst_parent_frame);
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}
