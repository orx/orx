/***************************************************************************
 pathfinder.c
 pathfinder module
 
 begin                : 04/09/2002
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


#include "utils/pathfinder.h"

#include "debug/debug.h"


int32 pathfinder_gi_map_h_size = 0;
int32 pathfinder_gi_map_v_size = 0;
float pathfinder_gf_goal_distance = 0.0;

static pathfinder_st_tile **sppst_matrix = NULL;
static pathfinder_st_mask *spst_mask = NULL;
static int32 si_mask_size = 0;
static coord_st_coord sst_source;
static coord_st_coord sst_destination;
static coord_st_coord sst_null;
static bool *spb_touched = NULL;
static int32 si_touched_size = 0;

struct st_tile_t
{
  float f_gcost, f_fcost;
  int32 i_num;
  
  coord_st_coord st_parent, st_previous, st_next;

  bool b_open;
  bool b_blocked;

  /* 12 extra bytes of padding : 80 */
  uint8 auc_unused[12];
};

struct st_mask_t
{
  int32 s32_x, s32_y;
  float f_cost;

  /* 4 extra bytes of padding : 16 */
  uint8 auc_unused[4];
};

inline void pathfinder_tile_copy(pathfinder_st_tile *_pst_dest, pathfinder_st_tile *_pst_src)
{
  if(_pst_dest != NULL)
  {
    if(_pst_src != NULL)
    {
      _pst_dest->f_gcost = _pst_src->f_gcost;
      _pst_dest->f_fcost = _pst_src->f_fcost;
      _pst_dest->i_num = _pst_src->i_num;
      _pst_dest->st_parent = _pst_src->st_parent;
      _pst_dest->st_previous = _pst_src->st_previous;
      _pst_dest->st_next = _pst_src->st_next;
      _pst_dest->b_open = _pst_src->b_open;
      _pst_dest->b_blocked = _pst_src->b_blocked;
    }
  }

  return;
}

inline void pathfinder_mask_delete()
{
  if(spst_mask != NULL)
  {
    free(spst_mask);
    spst_mask = NULL;
  }

  return;
}

void pathfinder_mask_create_4()
{
  pathfinder_mask_delete();

  si_mask_size = 4;

  spst_mask = (pathfinder_st_mask *) malloc(sizeof(pathfinder_st_mask));

  spst_mask[0].s32_x = 0;
  spst_mask[0].s32_y = -1;
  spst_mask[0].f_cost = 1.0;

  spst_mask[1].s32_x = 1;
  spst_mask[1].s32_y = 0;
  spst_mask[1].f_cost = 1.0;
                
  spst_mask[2].s32_x = 0;
  spst_mask[2].s32_y = 1;
  spst_mask[2].f_cost = 1.0;

  spst_mask[3].s32_x = -1;
  spst_mask[3].s32_y = 0;
  spst_mask[3].f_cost = 1.0;

  return;
}

void pathfinder_mask_create_knight()
{
  pathfinder_mask_delete();

  si_mask_size = 8;

  spst_mask = (pathfinder_st_mask *) malloc(sizeof(pathfinder_st_mask));

  spst_mask[0].s32_x = 1;
  spst_mask[0].s32_y = -2;
  spst_mask[0].f_cost = 1.0;

  spst_mask[1].s32_x = 2;
  spst_mask[1].s32_y = -1;
  spst_mask[1].f_cost = 1.0;

  spst_mask[2].s32_x = 2;
  spst_mask[2].s32_y = 1;
  spst_mask[2].f_cost = 1.0;

  spst_mask[3].s32_x = 1;
  spst_mask[3].s32_y = 2;
  spst_mask[3].f_cost = 1.0;

  spst_mask[4].s32_x = -1;
  spst_mask[4].s32_y = 2;
  spst_mask[4].f_cost = 1.0;

  spst_mask[5].s32_x = -2;
  spst_mask[5].s32_y = 1;
  spst_mask[5].f_cost = 1.0;

  spst_mask[6].s32_x = -2;
  spst_mask[6].s32_y = -1;
  spst_mask[6].f_cost = 1.0;

  spst_mask[7].s32_x = -1;
  spst_mask[7].s32_y = -2;
  spst_mask[7].f_cost = 1.0;

  return;
}

void pathfinder_mask_create_8()
{
  pathfinder_mask_delete();

  si_mask_size = 8;

  spst_mask = (pathfinder_st_mask *) malloc(sizeof(pathfinder_st_mask));

  spst_mask[0].s32_x = 0;
  spst_mask[0].s32_y = -1;
  spst_mask[0].f_cost = 1.0;

  spst_mask[1].s32_x = 1;
  spst_mask[1].s32_y = 0;
  spst_mask[1].f_cost = 1.0;

  spst_mask[2].s32_x = 0;
  spst_mask[2].s32_y = 1;
  spst_mask[2].f_cost = 1.0;

  spst_mask[3].s32_x = -1;
  spst_mask[3].s32_y = 0;
  spst_mask[3].f_cost = 1.0;

  spst_mask[4].s32_x = -1;
  spst_mask[4].s32_y = -1;
  spst_mask[4].f_cost = KF_MATH_SQRT2;

  spst_mask[5].s32_x = 1;
  spst_mask[5].s32_y = -1;
  spst_mask[5].f_cost = KF_MATH_SQRT2;

  spst_mask[6].s32_x = 1;
  spst_mask[6].s32_y = 1;
  spst_mask[6].f_cost = KF_MATH_SQRT2;

  spst_mask[7].s32_x = -1;
  spst_mask[7].s32_y = 1;
  spst_mask[7].f_cost = KF_MATH_SQRT2;

  return;
}

inline void pathfinder_touched_delete()
{
  if(spb_touched != NULL)
  {
    free(spb_touched);
    si_touched_size = 0;
    spb_touched = NULL;
  }

  return;
}

inline void pathfinder_touched_create()
{
  pathfinder_touched_delete();

  si_touched_size = pathfinder_gi_map_v_size * pathfinder_gi_map_h_size;
  spb_touched = (bool *) malloc(si_touched_size * sizeof(bool));

  return;
}

void pathfinder_touched_clean()
{
  int32 i;
  for(i = 0; i < pathfinder_gi_map_h_size * pathfinder_gi_map_v_size; i++)
  {
    spb_touched[i] = FALSE;
  }

  return;
}

inline void pathfinder_matrix_clean()
{
  int32 i, j;
  pathfinder_st_tile *pst_tile;

  if(sppst_matrix != NULL)
  { 
    for(i = 0; i < pathfinder_gi_map_h_size; i++)
    {
      for(j = 0; j < pathfinder_gi_map_v_size; j++)
      {
        pst_tile = &sppst_matrix[i][j];

        pst_tile->f_gcost = pst_tile->f_fcost = 0.0;
        pst_tile->b_open = pst_tile->b_blocked = FALSE;
      }
    }
  }

  return;
}

inline void pathfinder_matrix_delete()
{
  int32 i;
  pathfinder_st_tile *pst_tile;

  if(sppst_matrix != NULL)
  {
    for(i = 0; i < pathfinder_gi_map_h_size; i++)
    {
      pst_tile = sppst_matrix[i];
      if(pst_tile != NULL)
        {
          free(pst_tile);
        }
    }

    free(sppst_matrix);
    sppst_matrix = NULL;
  }

  return;
}

inline void pathfinder_matrix_create()
{
  int32 i;

  pathfinder_matrix_delete();

  sppst_matrix = (pathfinder_st_tile **) malloc(pathfinder_gi_map_h_size * sizeof(pathfinder_st_tile *));
  for(i = 0; i < pathfinder_gi_map_h_size; i++)
  {
    sppst_matrix[i] = (pathfinder_st_tile *) malloc(pathfinder_gi_map_v_size * sizeof(pathfinder_st_tile));
  }

  return;
}

inline void pathfinder_map_init(int32 _i_horizontal_size, int32 _i_vertical_size)
{
  coord_reset(&sst_null);

  pathfinder_gi_map_h_size = _i_horizontal_size;
  pathfinder_gi_map_v_size = _i_vertical_size;

  pathfinder_touched_create();
  pathfinder_touched_clean();

  pathfinder_matrix_create();
  pathfinder_matrix_clean();

  return;
}


void pathfinder_delete()
{
  pathfinder_touched_delete();
  pathfinder_matrix_delete();
  pathfinder_mask_delete();

  return;
}

inline void pathfinder_source_set(coord_st_coord *_pst_coord)
{
  coord_copy(&sst_source, _pst_coord);

  return;
}

inline void pathfinder_destination_set(coord_st_coord *_pst_coord)
{
  coord_copy(&sst_destination, _pst_coord);

  return;
}

inline float pathfinder_cost(coord_st_coord *_pst_coord1, coord_st_coord *_pst_coord2)
{
  int32 dx, dy;

  dx = abs(_pst_coord1->s32_x - _pst_coord2->s32_x);
  dy = abs(_pst_coord1->s32_y - _pst_coord2->s32_y);
  
  return(sqrtf((float)(dx*dx + dy*dy)));
}

inline float pathfinder_goal_distance_estimate(coord_st_coord *_pst_coord)
{
  return(pathfinder_cost(_pst_coord, &sst_destination));
}

inline void pathfinder_tile_set(coord_st_coord *_pst_coord, float _f_gcost, float _f_fcost, int32 _i_num, coord_st_coord *_pst_parent, coord_st_coord *_pst_previous, coord_st_coord *_pst_next, bool _b_status)
{
  pathfinder_st_tile *tile = &sppst_matrix[_pst_coord->s32_x][_pst_coord->s32_y];

  tile->f_gcost = _f_gcost;
  tile->f_fcost = _f_fcost;
  
  tile->i_num = _i_num;

  coord_copy(&tile->st_parent, _pst_parent);
  coord_copy(&tile->st_previous, _pst_previous);
  coord_copy(&tile->st_next, _pst_next);

  tile->b_open = _b_status;

  spb_touched[_pst_coord->s32_x + (_pst_coord->s32_y * pathfinder_gi_map_h_size)] = TRUE;

  return;
}

inline pathfinder_st_tile *pathfinder_tile_get(coord_st_coord *_pst_coord)
{
  if((_pst_coord->s32_x < 0)
     || (_pst_coord->s32_x >= pathfinder_gi_map_h_size)
     || (_pst_coord->s32_y < 0)
     || (_pst_coord->s32_y >= pathfinder_gi_map_v_size))
  {
    return NULL;
  }
  else
  {
    return(&(sppst_matrix[_pst_coord->s32_x][_pst_coord->s32_y]));
  }
}

inline bool pathfinder_is_walkable(coord_st_coord *_pst_coord1, coord_st_coord *_pst_coord2)
{
  coord_st_coord st_start, st_end;
  float f_a, f_sampling, f_dx, f_dy, f_x, f_y;
  bool b_res;

  if(_pst_coord2->s32_x > _pst_coord1->s32_x)
  {
    coord_copy(&st_start, _pst_coord1);
    coord_copy(&st_end, _pst_coord2);
  }
  else if(_pst_coord2->s32_x < _pst_coord1->s32_x)
  {
    coord_copy(&st_start, _pst_coord2);
    coord_copy(&st_end, _pst_coord1);
  }
  else if(_pst_coord2->s32_y > _pst_coord1->s32_y)
  {
    coord_copy(&st_start, _pst_coord1);
    coord_copy(&st_end, _pst_coord2);
  }
  else
  {
    coord_copy(&st_start, _pst_coord2);
    coord_copy(&st_end, _pst_coord1);
  }

  b_res = TRUE;

  if(st_end.s32_x != st_start.s32_x)
  {
    f_a = ((float)(st_end.s32_y - st_start.s32_y) / (float)(st_end.s32_x - st_start.s32_x));
    f_sampling = ((st_end.s32_x - st_start.s32_x) > 0) ? PATHFINDER_KF_SAMPLING : -PATHFINDER_KF_SAMPLING;
    f_dx = sqrtf((f_sampling * f_sampling) / (1.0 + (f_a * f_a)));
    f_dy = f_a * f_dx;

    for(f_x = (float)st_start.s32_x + PATHFINDER_KF_POSX, f_y = (float)st_start.s32_y + PATHFINDER_KF_POSY; f_x < (float)st_end.s32_x + PATHFINDER_KF_POSX; f_x += f_dx, f_y += f_dy)
    {
      if(sppst_matrix[(int)f_x][(int)f_y].b_blocked != FALSE)
      {
        b_res = FALSE;
        break;
      }
    }
  }
  else
  {
    f_dy = PATHFINDER_KF_SAMPLING;

    for(f_x = (float)st_start.s32_x + PATHFINDER_KF_POSX, f_y = (float)st_start.s32_y + PATHFINDER_KF_POSY; f_y < (float)st_end.s32_y + PATHFINDER_KF_POSY; f_y += f_dy)
    {
      if(sppst_matrix[(int)f_x][(int)f_y].b_blocked != FALSE)
      {
        b_res = FALSE;
        break;
      }
    }
  }

  return b_res;
}

inline bool pathfinder_is_goal(coord_st_coord *_pst_coord)
{
  return((_pst_coord->s32_x == sst_destination.s32_x) && (_pst_coord->s32_y == sst_destination.s32_y));
}

inline coord_st_coord* pathfinder_coord_previous(coord_st_coord *_pst_coord)
{
  return(&sppst_matrix[_pst_coord->s32_x][_pst_coord->s32_y].st_previous);
}

inline coord_st_coord* pathfinder_coord_next(coord_st_coord *_pst_coord)
{
  return(&sppst_matrix[_pst_coord->s32_x][_pst_coord->s32_y].st_next);
}

inline coord_st_coord* pathfinder_coord_parent(coord_st_coord *_pst_coord)
{
  return(&sppst_matrix[_pst_coord->s32_x][_pst_coord->s32_y].st_parent);
}

inline bool pathfinder_is_touched(coord_st_coord *_pst_coord)
{
  return(spb_touched[_pst_coord->s32_x+(_pst_coord->s32_y * pathfinder_gi_map_h_size)]);
}

inline float pathfinder_gcost(coord_st_coord *_pst_coord)
{
  return(sppst_matrix[_pst_coord->s32_x][_pst_coord->s32_y].f_gcost);
}

inline float pathfinder_fcost(coord_st_coord *_pst_coord)
{
  return(sppst_matrix[_pst_coord->s32_x][_pst_coord->s32_y].f_fcost);
}

void pathfinder_map_load(int32 _i_horizontal_size, int32 _i_vertical_size, int32 *_pi_map)
{
  int32 i, j;

  pathfinder_map_init(_i_horizontal_size, _i_vertical_size);

  for(j = 0; j < pathfinder_gi_map_v_size; j ++)
  {
    for(i = 0; i < pathfinder_gi_map_h_size; i++)
    {
      switch(_pi_map[i + (j * pathfinder_gi_map_h_size)])
      {
        case PATHFINDER_KI_EMPTY:
          sppst_matrix[i][j].b_blocked = FALSE;
          break;

        default:
          sppst_matrix[i][j].b_blocked = TRUE;
          break;
      }
    }
  }

  DEBUG(D_PATHFINDER, "Largeur : %d. Hauteur : %d.\n", _i_horizontal_size, _i_vertical_size);

  return;
}

int32 pathfinder_path_get(coord_st_coord *_pst_src, coord_st_coord *_pst_dest, coord_st_coord **_ppst_result, bool _b_smooth)
{
  int32 i_current_point, i_check_point, i_newnum;
  int32 i, j;
  float f_newg, f_newf;
  coord_st_coord st_act, st_parent, st_prev, st_next, st_temp;
  coord_st_coord *pst_result, *pst_temp, *pst_parent;
  pathfinder_st_tile *pst_tile;

  pathfinder_touched_clean();

  pathfinder_source_set(_pst_src);
  pathfinder_destination_set(_pst_dest);

  pathfinder_tile_set(_pst_src, 0.0, pathfinder_goal_distance_estimate(_pst_src), 1, NULL, NULL, NULL, TRUE);


  coord_copy(&st_parent, _pst_src);
  coord_copy(&st_prev, &st_parent);
  coord_copy(&st_next, &sst_null);

  pst_tile = pathfinder_tile_get(&st_parent);

  while(pst_tile->b_open != FALSE)
  {
    if(pathfinder_is_goal(&st_parent) != FALSE)
    {
      pst_result = (coord_st_coord *) malloc(pst_tile->i_num * sizeof(coord_st_coord));

      for(i = pst_tile->i_num - 1, coord_copy(&st_temp, &st_parent); i >= 0; i--)
      {
        coord_copy(&pst_result[i], &st_temp);
        pst_parent = pathfinder_coord_parent(&st_temp);
        coord_copy(&st_temp, pst_parent);
      }

      pathfinder_gf_goal_distance = pst_tile->f_gcost;

      /*
       * Return the raw path?
       */

      if(_b_smooth == FALSE)
      {
        *_ppst_result = pst_result;
        return(pst_tile->i_num - 1);
      }

      /*
       * Or smooth it before returning it
       */

      pst_temp = (coord_st_coord *) malloc(pst_tile->i_num * sizeof(coord_st_coord));

      for(i = 0, i_check_point = 0, i_current_point = 1; i_current_point + 1 < pst_tile->i_num; i_current_point++)
      {
        if(pathfinder_is_walkable(&pst_result[i_check_point], &pst_result[i_current_point + 1]) == FALSE)
        {
          coord_copy(&pst_temp[i++], &pst_result[i_current_point]);
          i_check_point = i_current_point;
        }
      }

      if((pst_temp[i-1].s32_x != pst_result[i_current_point].s32_x) || (pst_temp[i-1].s32_y != pst_result[i_current_point].s32_y))
      {
        coord_copy(&pst_temp[i++], &pst_result[i_current_point]);
      }

      free(pst_result);

      /*
       * Return Smoothed Path
       */

      *_ppst_result = pst_temp;
      return(i);
    }
    else
    {
      for(j = 0; j < si_mask_size; j++)
      {
        st_act.s32_x = st_parent.s32_x + spst_mask[j].s32_x;
        st_act.s32_y = st_parent.s32_y + spst_mask[j].s32_y;
        f_newg = pst_tile->f_gcost + spst_mask[j].f_cost;

        if((st_act.s32_x >= 0) && (st_act.s32_x < pathfinder_gi_map_h_size) && (st_act.s32_y >= 0) && (st_act.s32_y < pathfinder_gi_map_v_size) && (pathfinder_tile_get(&st_act)->b_blocked == FALSE))
        {
          if((pathfinder_is_touched(&st_act) != FALSE))
          {
            if((pathfinder_gcost(&st_act) <= f_newg))
            {
              continue;
            }
            else
            {
              /*
               * Already here?
               * Then we delete it!
               */

              coord_copy(&st_next, pathfinder_coord_next(&st_act));
              coord_copy(&st_prev, pathfinder_coord_previous(&st_act));

              if(coord_is_null(&st_next) == FALSE)
              {
                coord_copy(&sppst_matrix[st_next.s32_x][st_next.s32_y].st_previous, &st_prev);
              }

              if(coord_is_null(&st_prev) == FALSE)
              {
                coord_copy(&sppst_matrix[st_prev.s32_x][st_prev.s32_y].st_next, &st_next);
              }
            }
          }

          f_newf = f_newg + pathfinder_goal_distance_estimate(&st_act);
          i_newnum = pst_tile->i_num + 1;

          /*
           * Looking for the place to insert the new cell
           */

          pst_temp = pathfinder_coord_next(&st_parent);
          coord_copy(&st_next, pst_temp);
          coord_copy(&st_prev, &st_parent);
          while((coord_is_null(&st_next) == FALSE) && (f_newf >= pathfinder_fcost(&st_next)))
          {
            coord_copy(&st_prev, &st_next);
            pst_temp = pathfinder_coord_next(&st_next);
            coord_copy(&st_next, pst_temp);
          }
          pathfinder_tile_set(&st_act, f_newg, f_newf, i_newnum, &st_parent, &st_prev, &st_next, TRUE);

          coord_copy(&sppst_matrix[st_prev.s32_x][st_prev.s32_y].st_next, &st_act);
          if(coord_is_null(&st_next) == FALSE)
          {
            coord_copy(&sppst_matrix[st_next.s32_x][st_next.s32_y].st_previous, &st_act);
          }
        }
      }
    }

    pst_tile->b_open = FALSE;

    pst_temp = pathfinder_coord_previous(&st_parent);
    coord_copy(&st_prev, pst_temp);

    pst_temp = pathfinder_coord_next(&st_parent);
    coord_copy(&st_next, pst_temp);

    if(coord_is_null(&st_next) == FALSE)
    {
      coord_copy(&sppst_matrix[st_next.s32_x][st_next.s32_y].st_previous, &st_prev);
    }

    if(coord_is_null(&st_prev) == FALSE)
    {
      coord_copy(&sppst_matrix[st_prev.s32_x][st_prev.s32_y].st_next, &st_next);
    }

    coord_copy(&st_parent, &pst_tile->st_next);
    if(coord_is_null(&st_parent) != FALSE)
    {
      break;
    }
    else
    {
      pst_tile = pathfinder_tile_get(&st_parent);
    }
  }

  return 0;
}

uint32 pathfinder_init()
{
  return EXIT_SUCCESS;
}

void pathfinder_exit()
{
  return;
}
