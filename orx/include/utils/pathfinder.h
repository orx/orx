/** 
 * \file pathfinder.h
 * 
 * Pathfinder Module.
 * Allows to find the shortest way between two points if exists,
 * given a 2D collision map.
 * It can strip the resulting way and return only the pivots so as to avoid
 * walls and obstacles. This resulting way is computed given a sampling rate
 * and a path width.
 * It uses a mask structure that describes moving capabilities and costs.
 * As for now there are 3 masks implemented: 4-connex, 8-connex
 * and "chess knight"-style.
 * Algorithm used is A*.
 * 
 * \todo
 * Add a hierarchical structure and a recursive pathfind to combine several
 * maps so as to be able to change from floors or depths.
 * Add possibility to define freely moving masks.
 */


/***************************************************************************
 pathfinder.h
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

#ifndef _PATHFINDER_H_
#define _PATHFINDER_H_

#include "include.h"

#include "math/coord.h"

#define PATHFINDER_KI_EMPTY     0x00000000    /**< Empty box type */
#define PATHFINDER_KI_WALL      0x00000001    /**< Wall box type */

#define PATHFINDER_KF_POSX      0.5           /**< Default X coord in a map box */
#define PATHFINDER_KF_POSY      0.5           /**< Default Y coord in a map box */
#define PATHFINDER_KF_SAMPLING  0.1           /**< Default sampling value for the path smoothing/stripping */


/** Internal tile structure */
typedef struct st_tile_t pathfinder_st_tile;
/** Internal mask structure */
typedef struct st_mask_t pathfinder_st_mask;


/** Inits pathfinder module. */
extern uint32 pathfinder_init();
/** Exits from pathfinder module. */
extern void   pathfinder_exit();

/** Current map width */
extern int32  pathfinder_gi_map_h_size;
/** Current map height */
extern int32  pathfinder_gi_map_v_size;
/** Last found path distance */
extern float  pathfinder_gf_goal_distance;


/** Creates a 4-connex moving mask. */
extern void   pathfinder_mask_create_4();
/** Creates a 8-connex moving mask. */
extern void   pathfinder_mask_create_8();
/** Creates a "chess knight"-style moving mask. */
extern void   pathfinder_mask_create_knight();
/** Clears last pathfinder structures. */
extern void   pathfinder_delete();
/** Loads a map and initializes pathfinder structures.
 * \param _i_horizontal_size Map width.
 * \param _i_vertical_size Map height.
 * \param _pi_map Environment map : Int Array of (_i_horizontal_size * _i_vertical_size) size.
 */
extern void   pathfinder_map_load(int32 _i_horizontal_size, int32 _i_vertical_size, int32 *_pi_map);
/** Gets a path between 2 given locations.
 * \param _pst_src Source coordinates (pointer on coord structure).
 * \param _pst_dest Destination coordinates (pointer on coord structure).
 * \param _ppst_result Coordinates of all found path locations.
 * \param _b_smooth Tells if found path must be smoothed/stripped.
 * \return Returns the size of the found path (number of locations). 0 is no path found.
 */
extern int32  pathfinder_path_get(coord_st_coord *_pst_src, coord_st_coord *_pst_dest, coord_st_coord **_ppst_result, bool _b_smooth);


#endif /* _PATHFINDER_H_ */
