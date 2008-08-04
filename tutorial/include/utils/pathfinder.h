///** 
// * \file orxPathfinder.h
// * 
// * orxPathfinder Module.
// * Allows to find the shortest way between two points if exists under constraints.
// * 
// * 2D implementation : 
// * It can strip the resulting way and return only the pivots so as to avoid
// * walls and obstacles. This resulting way is computed given a sampling rate
// * and a path width.
// * It uses a mask structure that describes moving capabilities and costs.
// * As for now there are 3 masks implemented: 4-connex, 8-connex
// * and "chess knight"-style.
// * 
// * Algorithm used is based on A*.
// * 
// * \todo
// */
//
//
///***************************************************************************
// orxPathfinder.h
// Pathfinder module
// 
// begin                : 06/09/2005
// author               : (C) Arcallians
// email                : iarwain@arcallians.org
// ***************************************************************************/
//
///***************************************************************************
// *                                                                         *
// *   This library is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU Lesser General Public License           *
// *   as published by the Free Software Foundation; either version 2.1      *
// *   of the License, or (at your option) any later version.                *
// *                                                                         *
// ***************************************************************************/
//
//#ifndef _orxPATHFINDER_H_
//#define _orxPATHFINDER_H_
//
//
//#include "orxInclude.h"
//
//#include "math/orxMath.h"
//
//#define orxPathfinder_KS32_EMPTY     0x00000000 /**< Empty box type */
//#define orxPathfinder_KS32_WALL      0x00000001 /**< Wall box type */
//
//#define orxPathfinder_KF_POSX      0.5          /**< Default X coord in a map box */
//#define orxPathfinder_KF_POSY      0.5          /**< Default Y coord in a map box */
//#define orxPathfinder_KF_SAMPLING  0.1          /**< Default sampling value for the path smoothing/stripping */
//
//
///** Internal tile structure */
//typedef struct st_tile_t orxPathfinder_st_tile;
///** Internal mask structure */
//typedef struct st_mask_t orxPathfinder_st_mask;
//
//
///** Inits orxPathfinder module. */
//extern orxSTATUS                                orxPathfinder_Init();
///** Exits from orxPathfinder module. */
//extern orxVOID                                  orxPathfinder_Exit();
//
///** Current map width */
//extern orxS32  orxPathfinder_gi_map_h_size;
///** Current map height */
//extern orxS32  orxPathfinder_gi_map_v_size;
///** Last found path distance */
//extern orxFLOAT  orxPathfinder_gf_goal_distance;
//
//
///** Creates a 4-connex moving mask. */
//extern orxVOID   orxPathfinder_mask_create_4();
///** Creates a 8-connex moving mask. */
//extern orxVOID   orxPathfinder_mask_create_8();
///** Creates a "chess knight"-style moving mask. */
//extern orxVOID   orxPathfinder_mask_create_knight();
///** Clears last orxPathfinder structures. */
//extern orxVOID   orxPathfinder_delete();
///** Loads a map and initializes orxPathfinder structures.
// * \param _i_horizontal_size Map width.
// * \param _i_vertical_size Map height.
// * \param _pi_map Environment map : Int Array of (_i_horizontal_size * _i_vertical_size) size.
// */
//extern orxVOID   orxPathfinder_map_load(orxS32 _i_horizontal_size, orxS32 _i_vertical_size, orxS32 *_pi_map);
///** Gets a path between 2 given locations.
// * \param _pst_src Source coordinates (pointer on coord structure).
// * \param _pst_dest Destination coordinates (pointer on coord structure).
// * \param _ppst_result Coordinates of all found path locations.
// * \param _b_smooth Tells if found path must be smoothed/stripped.
// * \return Returns the size of the found path (number of locations). 0 is no path found.
// */
//extern orxS32  orxPathfinder_path_get(orxVECTOR *pv_result, orxBOOL _b_smooth);
//
//
//#endif /* _orxPATHFINDER_H_ */
