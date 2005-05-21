/***************************************************************************
 pathfinder.c
 pathfinder module
 
 begin                : 04/09/2002
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


#include "utils/pathfinder.h"

#include "debug/orxDebug.h"
#include "math/orxMath.h"
#include "memory/orxMemory.h"


orxS32 pathfinder_gi_map_h_size = 0;
orxS32 pathfinder_gi_map_v_size = 0;
orxFLOAT pathfinder_gf_goal_distance = 0.0;

orxSTATIC pathfinder_st_tile **sppst_matrix = orxNULL;
orxSTATIC pathfinder_st_mask *spst_mask = orxNULL;
orxSTATIC orxS32 si_mask_size = 0;
orxSTATIC orxVEC sst_source;
orxSTATIC orxVEC sst_destination;
orxSTATIC orxVEC sst_null;
orxSTATIC orxBOOL *spb_touched = orxNULL;
orxSTATIC orxS32 si_touched_size = 0;

struct st_tile_t
{
  orxFLOAT f_gcost, f_fcost;
  orxS32 i_num;
  
  orxVEC st_parent, st_previous, st_next;

  orxBOOL b_open;
  orxBOOL b_blocked;

  /* 12 extra bytes of padding : 80 */
  orxU8 au8Unused[12];
};

struct st_mask_t
{
  orxS32 fX, fY;
  orxFLOAT fCost;

  /* 4 extra bytes of padding : 16 */
  orxU8 au8Unused[4];
};

orxVOID pathfinder_tile_copy(pathfinder_st_tile *_pst_dest, pathfinder_st_tile *_pst_src)
{
  if(_pst_dest != orxNULL)
  {
    if(_pst_src != orxNULL)
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

orxVOID pathfinder_mask_delete()
{
  if(spst_mask != orxNULL)
  {
    orxMemory_Free(spst_mask);
    spst_mask = orxNULL;
  }

  return;
}

orxVOID pathfinder_mask_create_4()
{
  pathfinder_mask_delete();

  si_mask_size = 4;

  spst_mask = (pathfinder_st_mask *) orxMemory_Allocate(sizeof(pathfinder_st_mask), orxMEMORY_TYPE_MAIN);

  spst_mask[0].fX = 0;
  spst_mask[0].fY = -1;
  spst_mask[0].fCost = 1.0;

  spst_mask[1].fX = 1;
  spst_mask[1].fY = 0;
  spst_mask[1].fCost = 1.0;
                
  spst_mask[2].fX = 0;
  spst_mask[2].fY = 1;
  spst_mask[2].fCost = 1.0;

  spst_mask[3].fX = -1;
  spst_mask[3].fY = 0;
  spst_mask[3].fCost = 1.0;

  return;
}

orxVOID pathfinder_mask_create_knight()
{
  pathfinder_mask_delete();

  si_mask_size = 8;

  spst_mask = (pathfinder_st_mask *) orxMemory_Allocate(sizeof(pathfinder_st_mask), orxMEMORY_TYPE_MAIN);

  spst_mask[0].fX = 1;
  spst_mask[0].fY = -2;
  spst_mask[0].fCost = 1.0;

  spst_mask[1].fX = 2;
  spst_mask[1].fY = -1;
  spst_mask[1].fCost = 1.0;

  spst_mask[2].fX = 2;
  spst_mask[2].fY = 1;
  spst_mask[2].fCost = 1.0;

  spst_mask[3].fX = 1;
  spst_mask[3].fY = 2;
  spst_mask[3].fCost = 1.0;

  spst_mask[4].fX = -1;
  spst_mask[4].fY = 2;
  spst_mask[4].fCost = 1.0;

  spst_mask[5].fX = -2;
  spst_mask[5].fY = 1;
  spst_mask[5].fCost = 1.0;

  spst_mask[6].fX = -2;
  spst_mask[6].fY = -1;
  spst_mask[6].fCost = 1.0;

  spst_mask[7].fX = -1;
  spst_mask[7].fY = -2;
  spst_mask[7].fCost = 1.0;

  return;
}

orxVOID pathfinder_mask_create_8()
{
  pathfinder_mask_delete();

  si_mask_size = 8;

  spst_mask = (pathfinder_st_mask *) orxMemory_Allocate(sizeof(pathfinder_st_mask), orxMEMORY_TYPE_MAIN);

  spst_mask[0].fX = 0;
  spst_mask[0].fY = -1;
  spst_mask[0].fCost = 1.0;

  spst_mask[1].fX = 1;
  spst_mask[1].fY = 0;
  spst_mask[1].fCost = 1.0;

  spst_mask[2].fX = 0;
  spst_mask[2].fY = 1;
  spst_mask[2].fCost = 1.0;

  spst_mask[3].fX = -1;
  spst_mask[3].fY = 0;
  spst_mask[3].fCost = 1.0;

  spst_mask[4].fX = -1;
  spst_mask[4].fY = -1;
  spst_mask[4].fCost = KF_MATH_SQRT2;

  spst_mask[5].fX = 1;
  spst_mask[5].fY = -1;
  spst_mask[5].fCost = KF_MATH_SQRT2;

  spst_mask[6].fX = 1;
  spst_mask[6].fY = 1;
  spst_mask[6].fCost = KF_MATH_SQRT2;

  spst_mask[7].fX = -1;
  spst_mask[7].fY = 1;
  spst_mask[7].fCost = KF_MATH_SQRT2;

  return;
}

orxVOID pathfinder_touched_delete()
{
  if(spb_touched != orxNULL)
  {
    orxMemory_Free(spb_touched);
    si_touched_size = 0;
    spb_touched = orxNULL;
  }

  return;
}

orxVOID pathfinder_touched_create()
{
  pathfinder_touched_delete();

  si_touched_size = pathfinder_gi_map_v_size * pathfinder_gi_map_h_size;
  spb_touched = (orxBOOL *) orxMemory_Allocate(si_touched_size * sizeof(orxBOOL), orxMEMORY_TYPE_MAIN);

  return;
}

orxVOID pathfinder_touched_clean()
{
  orxS32 i;
  for(i = 0; i < pathfinder_gi_map_h_size * pathfinder_gi_map_v_size; i++)
  {
    spb_touched[i] = orxFALSE;
  }

  return;
}

orxVOID pathfinder_matrix_clean()
{
  orxS32 i, j;
  pathfinder_st_tile *pst_tile;

  if(sppst_matrix != orxNULL)
  { 
    for(i = 0; i < pathfinder_gi_map_h_size; i++)
    {
      for(j = 0; j < pathfinder_gi_map_v_size; j++)
      {
        pst_tile = &sppst_matrix[i][j];

        pst_tile->f_gcost = pst_tile->f_fcost = 0.0;
        pst_tile->b_open = pst_tile->b_blocked = orxFALSE;
      }
    }
  }

  return;
}

orxVOID pathfinder_matrix_delete()
{
  orxS32 i;
  pathfinder_st_tile *pst_tile;

  if(sppst_matrix != orxNULL)
  {
    for(i = 0; i < pathfinder_gi_map_h_size; i++)
    {
      pst_tile = sppst_matrix[i];
      if(pst_tile != orxNULL)
        {
          orxMemory_Free(pst_tile);
        }
    }

    orxMemory_Free(sppst_matrix);
    sppst_matrix = orxNULL;
  }

  return;
}

orxVOID pathfinder_matrix_create()
{
  orxS32 i;

  pathfinder_matrix_delete();

  sppst_matrix = (pathfinder_st_tile **) orxMemory_Allocate(pathfinder_gi_map_h_size * sizeof(pathfinder_st_tile *), orxMEMORY_TYPE_MAIN);
  for(i = 0; i < pathfinder_gi_map_h_size; i++)
  {
    sppst_matrix[i] = (pathfinder_st_tile *) orxMemory_Allocate(pathfinder_gi_map_v_size * sizeof(pathfinder_st_tile), orxMEMORY_TYPE_MAIN);
  }

  return;
}

orxVOID pathfinder_map_init(orxS32 _i_horizontal_size, orxS32 _i_vertical_size)
{
//  orxVec_Load(&sst_null);

  pathfinder_gi_map_h_size = _i_horizontal_size;
  pathfinder_gi_map_v_size = _i_vertical_size;

  pathfinder_touched_create();
  pathfinder_touched_clean();

  pathfinder_matrix_create();
  pathfinder_matrix_clean();

  return;
}


orxVOID pathfinder_delete()
{
  pathfinder_touched_delete();
  pathfinder_matrix_delete();
  pathfinder_mask_delete();

  return;
}

orxVOID pathfinder_source_set(orxVEC *_pst_coord)
{
  orxVec_Copy(&sst_source, _pst_coord);

  return;
}

orxVOID pathfinder_destination_set(orxVEC *_pst_coord)
{
  orxVec_Copy(&sst_destination, _pst_coord);

  return;
}

orxFLOAT pathfinder_cost(orxVEC *_pst_coord1, orxVEC *_pst_coord2)
{
  orxFLOAT fX, fY;

  fX = orxFABS(_pst_coord1->fX - _pst_coord2->fX);
  fY = orxFABS(_pst_coord1->fY - _pst_coord2->fY);

  return(sqrtf((fX * fX) + (fY * fY)));
}

orxFLOAT pathfinder_goal_distance_estimate(orxVEC *_pst_coord)
{
  return(pathfinder_cost(_pst_coord, &sst_destination));
}

orxVOID pathfinder_tile_set(orxVEC *_pst_coord, orxFLOAT _f_gcost, orxFLOAT _f_fcost, orxS32 _i_num, orxVEC *_pstParent, orxVEC *_pstPrevious, orxVEC *_pstNext, orxBOOL _b_status)
{
  pathfinder_st_tile *tile = &sppst_matrix[(orxU32)_pst_coord->fX][(orxU32)_pst_coord->fY];

  tile->f_gcost = _f_gcost;
  tile->f_fcost = _f_fcost;
  
  tile->i_num = _i_num;

  orxVec_Copy(&tile->st_parent, _pstParent);
  orxVec_Copy(&tile->st_previous, _pstPrevious);
  orxVec_Copy(&tile->st_next, _pstNext);

  tile->b_open = _b_status;

  spb_touched[(orxU32)(_pst_coord->fX + (_pst_coord->fY * pathfinder_gi_map_h_size))] = orxTRUE;

  return;
}

pathfinder_st_tile *pathfinder_tile_get(orxVEC *_pst_coord)
{
  if((_pst_coord->fX < 0)
     || (_pst_coord->fX >= pathfinder_gi_map_h_size)
     || (_pst_coord->fY < 0)
     || (_pst_coord->fY >= pathfinder_gi_map_v_size))
  {
    return orxNULL;
  }
  else
  {
    return(&(sppst_matrix[(orxU32)_pst_coord->fX][(orxU32)_pst_coord->fY]));
  }
}

orxBOOL pathfinder_is_walkable(orxVEC *_pst_coord1, orxVEC *_pst_coord2)
{
  orxVEC st_start, st_end;
  orxFLOAT f_a, f_sampling, f_dx, f_dy, fX, fY;
  orxBOOL b_res;

  if(_pst_coord2->fX > _pst_coord1->fX)
  {
    orxVec_Copy(&st_start, _pst_coord1);
    orxVec_Copy(&st_end, _pst_coord2);
  }
  else if(_pst_coord2->fX < _pst_coord1->fX)
  {
    orxVec_Copy(&st_start, _pst_coord2);
    orxVec_Copy(&st_end, _pst_coord1);
  }
  else if(_pst_coord2->fY > _pst_coord1->fY)
  {
    orxVec_Copy(&st_start, _pst_coord1);
    orxVec_Copy(&st_end, _pst_coord2);
  }
  else
  {
    orxVec_Copy(&st_start, _pst_coord2);
    orxVec_Copy(&st_end, _pst_coord1);
  }

  b_res = orxTRUE;

  if(st_end.fX != st_start.fX)
  {
    f_a = ((st_end.fY - st_start.fY) / (st_end.fX - st_start.fX));
    f_sampling = ((st_end.fX - st_start.fX) > 0) ? PATHFINDER_KF_SAMPLING : -PATHFINDER_KF_SAMPLING;
    f_dx = sqrtf((f_sampling * f_sampling) / (1.0 + (f_a * f_a)));
    f_dy = f_a * f_dx;

    for(fX = st_start.fX + PATHFINDER_KF_POSX, fY = st_start.fY + PATHFINDER_KF_POSY; fX < (orxFLOAT)st_end.fX + PATHFINDER_KF_POSX; fX += f_dx, fY += f_dy)
    {
      if(sppst_matrix[(orxU32)fX][(orxU32)fY].b_blocked != orxFALSE)
      {
        b_res = orxFALSE;
        break;
      }
    }
  }
  else
  {
    f_dy = PATHFINDER_KF_SAMPLING;

    for(fX = st_start.fX + PATHFINDER_KF_POSX, fY = st_start.fY + PATHFINDER_KF_POSY; fY < (orxFLOAT)st_end.fY + PATHFINDER_KF_POSY; fY += f_dy)
    {
      if(sppst_matrix[(orxU32)fX][(orxU32)fY].b_blocked != orxFALSE)
      {
        b_res = orxFALSE;
        break;
      }
    }
  }

  return b_res;
}

orxBOOL pathfinder_is_goal(orxVEC *_pst_coord)
{
  return((_pst_coord->fX == sst_destination.fX) && (_pst_coord->fY == sst_destination.fY));
}

orxVEC* pathfinder_coord_previous(orxVEC *_pst_coord)
{
  return(&sppst_matrix[(orxU32)_pst_coord->fX][(orxU32)_pst_coord->fY].st_previous);
}

orxVEC* pathfinder_coord_next(orxVEC *_pst_coord)
{
  return(&sppst_matrix[(orxU32)_pst_coord->fX][(orxU32)_pst_coord->fY].st_next);
}

orxVEC* pathfinder_coord_parent(orxVEC *_pst_coord)
{
  return(&sppst_matrix[(orxU32)_pst_coord->fX][(orxU32)_pst_coord->fY].st_parent);
}

orxBOOL pathfinder_is_touched(orxVEC *_pst_coord)
{
  return(spb_touched[(orxU32)(_pst_coord->fX+(_pst_coord->fY * pathfinder_gi_map_h_size))]);
}

orxFLOAT pathfinder_gcost(orxVEC *_pst_coord)
{
  return(sppst_matrix[(orxU32)_pst_coord->fX][(orxU32)_pst_coord->fY].f_gcost);
}

orxFLOAT pathfinder_fcost(orxVEC *_pst_coord)
{
  return(sppst_matrix[(orxU32)_pst_coord->fX][(orxU32)_pst_coord->fY].f_fcost);
}

orxVOID pathfinder_map_load(orxS32 _i_horizontal_size, orxS32 _i_vertical_size, orxS32 *_pi_map)
{
  orxS32 i, j;

  pathfinder_map_init(_i_horizontal_size, _i_vertical_size);

  for(j = 0; j < pathfinder_gi_map_v_size; j ++)
  {
    for(i = 0; i < pathfinder_gi_map_h_size; i++)
    {
      switch(_pi_map[i + (j * pathfinder_gi_map_h_size)])
      {
        case PATHFINDER_KS32_EMPTY:
          sppst_matrix[i][j].b_blocked = orxFALSE;
          break;

        default:
          sppst_matrix[i][j].b_blocked = orxTRUE;
          break;
      }
    }
  }

  orxDEBUG_LOG(orxDEBUG_LEVEL_PATHFINDER, "Largeur : %d. Hauteur : %d.\n", _i_horizontal_size, _i_vertical_size);

  return;
}

orxS32 pathfinder_path_get(orxVEC *_pst_src, orxVEC *_pst_dest, orxVEC **_ppst_result, orxBOOL _b_smooth)
{
  orxS32 i_current_point, i_check_point, i_newnum;
  orxS32 i, j;
  orxFLOAT f_newg, f_newf;
  orxVEC st_act, st_parent, st_prev, st_next, st_temp;
  orxVEC *pst_result, *pst_temp, *pstParent;
  pathfinder_st_tile *pst_tile;

  pathfinder_touched_clean();

  pathfinder_source_set(_pst_src);
  pathfinder_destination_set(_pst_dest);

  pathfinder_tile_set(_pst_src, 0.0, pathfinder_goal_distance_estimate(_pst_src), 1, orxNULL, orxNULL, orxNULL, orxTRUE);


  orxVec_Copy(&st_parent, _pst_src);
  orxVec_Copy(&st_prev, &st_parent);
  orxVec_Copy(&st_next, &sst_null);

  pst_tile = pathfinder_tile_get(&st_parent);

  while(pst_tile->b_open != orxFALSE)
  {
    if(pathfinder_is_goal(&st_parent) != orxFALSE)
    {
      pst_result = (orxVEC *) orxMemory_Allocate(pst_tile->i_num * sizeof(orxVEC), orxMEMORY_TYPE_MAIN);

      for(i = pst_tile->i_num - 1, orxVec_Copy(&st_temp, &st_parent); i >= 0; i--)
      {
        orxVec_Copy(&pst_result[i], &st_temp);
        pstParent = pathfinder_coord_parent(&st_temp);
        orxVec_Copy(&st_temp, pstParent);
      }

      pathfinder_gf_goal_distance = pst_tile->f_gcost;

      /*
       * Return the raw path?
       */

      if(_b_smooth == orxFALSE)
      {
        *_ppst_result = pst_result;
        return(pst_tile->i_num - 1);
      }

      /*
       * Or smooth it before returning it
       */

      pst_temp = (orxVEC *) orxMemory_Allocate(pst_tile->i_num * sizeof(orxVEC), orxMEMORY_TYPE_MAIN);

      for(i = 0, i_check_point = 0, i_current_point = 1; i_current_point + 1 < pst_tile->i_num; i_current_point++)
      {
        if(pathfinder_is_walkable(&pst_result[i_check_point], &pst_result[i_current_point + 1]) == orxFALSE)
        {
          orxVec_Copy(&pst_temp[i++], &pst_result[i_current_point]);
          i_check_point = i_current_point;
        }
      }

      if((pst_temp[i-1].fX != pst_result[i_current_point].fX) || (pst_temp[i-1].fY != pst_result[i_current_point].fY))
      {
        orxVec_Copy(&pst_temp[i++], &pst_result[i_current_point]);
      }

      orxMemory_Free(pst_result);

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
        st_act.fX = st_parent.fX + spst_mask[j].fX;
        st_act.fY = st_parent.fY + spst_mask[j].fY;
        f_newg = pst_tile->f_gcost + spst_mask[j].fCost;

        if((st_act.fX >= 0) && (st_act.fX < pathfinder_gi_map_h_size) && (st_act.fY >= 0) && (st_act.fY < pathfinder_gi_map_v_size) && (pathfinder_tile_get(&st_act)->b_blocked == orxFALSE))
        {
          if((pathfinder_is_touched(&st_act) != orxFALSE))
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

              orxVec_Copy(&st_next, pathfinder_coord_next(&st_act));
              orxVec_Copy(&st_prev, pathfinder_coord_previous(&st_act));

//              if(coord_is_null(&st_next) == orxFALSE)
//              {
//                orxVec_Copy(&sppst_matrix[(orxU32)st_next.fX][(orxU32)st_next.fY].st_previous, &st_prev);
//              }
//
//              if(coord_is_null(&st_prev) == orxFALSE)
//              {
//                orxVec_Copy(&sppst_matrix[(orxU32)st_prev.fX][(orxU32)st_prev.fY].st_next, &st_next);
//              }
            }
          }

          f_newf = f_newg + pathfinder_goal_distance_estimate(&st_act);
          i_newnum = pst_tile->i_num + 1;

          /*
           * Looking for the place to insert the new cell
           */

          pst_temp = pathfinder_coord_next(&st_parent);
          orxVec_Copy(&st_next, pst_temp);
          orxVec_Copy(&st_prev, &st_parent);
//          while((coord_is_null(&st_next) == orxFALSE) && (f_newf >= pathfinder_fcost(&st_next)))
//          {
//            orxVec_Copy(&st_prev, &st_next);
//            pst_temp = pathfinder_coord_next(&st_next);
//            orxVec_Copy(&st_next, pst_temp);
//          }
          pathfinder_tile_set(&st_act, f_newg, f_newf, i_newnum, &st_parent, &st_prev, &st_next, orxTRUE);

          orxVec_Copy(&sppst_matrix[(orxU32)st_prev.fX][(orxU32)st_prev.fY].st_next, &st_act);
//          if(coord_is_null(&st_next) == orxFALSE)
//          {
//            orxVec_Copy(&sppst_matrix[(orxU32)st_next.fX][(orxU32)st_next.fY].st_previous, &st_act);
//          }
        }
      }
    }

    pst_tile->b_open = orxFALSE;

    pst_temp = pathfinder_coord_previous(&st_parent);
    orxVec_Copy(&st_prev, pst_temp);

    pst_temp = pathfinder_coord_next(&st_parent);
    orxVec_Copy(&st_next, pst_temp);

//    if(coord_is_null(&st_next) == orxFALSE)
//    {
//      orxVec_Copy(&sppst_matrix[(orxU32)st_next.fX][(orxU32)st_next.fY].st_previous, &st_prev);
//    }
//
//    if(coord_is_null(&st_prev) == orxFALSE)
//    {
//      orxVec_Copy(&sppst_matrix[(orxU32)st_prev.fX][(orxU32)st_prev.fY].st_next, &st_next);
//    }

    orxVec_Copy(&st_parent, &pst_tile->st_next);
//    if(coord_is_null(&st_parent) != orxFALSE)
//    {
//      break;
//    }
//    else
//    {
//      pst_tile = pathfinder_tile_get(&st_parent);
//    }
  }

  return 0;
}

orxU32 pathfinder_init()
{
  return orxSTATUS_SUCCESS;
}

orxVOID pathfinder_exit()
{
  return;
}
