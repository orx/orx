/** 
 * \file structure.h
 * 
 * Structure Module.
 * Allows to creates and handle structures.
 * Structures can be referenced by other structures (or objects).
 * 
 * \todo
 * Add the required structures when needed.
 * Do a generic system for structure registering (id given at realtime)
 */


/***************************************************************************
 structure.h
 structure module
 
 begin                : 08/12/2003
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


#ifndef _STRUCTURE_H_
#define _STRUCTURE_H_

#include "include.h"


/** Structure max number. */
#define STRUCTURE_KI_STRUCT_MAX_NUMBER      32

/** Structure IDs. */
#define STRUCTURE_KUL_STRUCT_ID_NONE        0x00000000

#define STRUCTURE_KUL_STRUCT_ID_OBJECT      0x00000001
#define STRUCTURE_KUL_STRUCT_ID_FRAME       0x00000002
#define STRUCTURE_KUL_STRUCT_ID_TEXTURE     0x00000004
#define STRUCTURE_KUL_STRUCT_ID_GRAPHIC     0x00000008
#define STRUCTURE_KUL_STRUCT_ID_CAMERA      0x00000010
#define STRUCTURE_KUL_STRUCT_ID_VIEWPORT    0x00000020
#define STRUCTURE_KUL_STRUCT_ID_ANIM        0x00000040
#define STRUCTURE_KUL_STRUCT_ID_ANIMSET     0x00000080
#define STRUCTURE_KUL_STRUCT_ID_ANIMPOINTER 0x00000100

#define STRUCTURE_KUL_STRUCT_ID_ALL         0x000001FF


/** Internal storage cell structure. */
typedef struct structure_st_cell_t structure_st_cell;

/** Public struct structure (Must be first structure member!).*/
typedef struct
{
  /* Structure ID. : 4 */
  uint32 u32_id;

  /* Reference counter. : 8 */
  uint32 u32_ref_counter;

  /* Pointer to storage cell. : 12 */
  structure_st_cell *pst_cell;

  /* 4 extra bytes of padding : 16 */
  uint8 auc_unused[4];
} structure_st_struct;

/** Inits the structure system. */
extern uint32               structure_init();
/** Exits from the structure system. */
extern void                 structure_exit();

/** Inits a structure with given type. */
extern uint32               structure_struct_init(structure_st_struct *_pst_struct, uint32 _u32_struct_id);
/** Cleans a structure. */
extern void                 structure_struct_clean(structure_st_struct *_pst_struct);

/** Increase structure reference counter. */
extern void                 structure_struct_counter_increase(structure_st_struct *_pst_struct);
/** Decrease structure reference counter. */
extern void                 structure_struct_counter_decrease(structure_st_struct *_pst_struct);
/** Gets structure reference counter. */
extern uint32               structure_struct_counter_get(structure_st_struct *_pst_struct);
/** Gets structure ID. */
extern uint32               structure_struct_id_get(structure_st_struct *_pst_struct);

/** Gets given type structure number. */
extern int32                structure_struct_number_get(uint32 _u32_struct_id);
/** Gets first stored structure (first list cell or tree root depending on storage type). */
extern structure_st_struct *structure_struct_first_get(uint32 _u32_struct_id);

/** !!! Warning : Structure accessors don't test storage type nor parameter validity !!! */

/** Structure tree parent get accessor. */
extern structure_st_struct *structure_struct_parent_get(structure_st_struct *_pst_struct);
/** Structure tree child get accessor. */
extern structure_st_struct *structure_struct_child_get(structure_st_struct *_pst_struct);
/** Structure tree left sibling get accessor. */
extern structure_st_struct *structure_struct_left_sibling_get(structure_st_struct *_pst_struct);
/** Structure tree right sibling get accessor. */
extern structure_st_struct *structure_struct_right_sibling_get(structure_st_struct *_pst_struct);
/** Structure list previous get accessor. */
extern structure_st_struct *structure_struct_previous_get(structure_st_struct *_pst_struct);
/** Structure list next get accessor. */
extern structure_st_struct *structure_struct_next_get(structure_st_struct *_pst_struct);

/** Structure tree parent set accessor. */
extern void structure_struct_parent_set(structure_st_struct *_pst_struct, structure_st_struct *_pst_parent);

#endif /* _STRUCTURE_H_ */
