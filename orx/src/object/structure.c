/***************************************************************************
 structure.c
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


#include "object/structure.h"


/*
 * Platform independant defines
 */

#define STRUCTURE_KUL_FLAG_NONE         0x00000000
#define STRUCTURE_KUL_FLAG_READY        0x00000001

#define STRUCTURE_KI_OFFSET_INVALID     -1

/* Tree offsets */
#define STRUCTURE_KI_OFFSET_FRAME       0

/* List offsets */
#define STRUCTURE_KI_OFFSET_OBJECT      0
#define STRUCTURE_KI_OFFSET_TEXTURE     1
#define STRUCTURE_KI_OFFSET_GRAPHIC     2
#define STRUCTURE_KI_OFFSET_CAMERA      3
#define STRUCTURE_KI_OFFSET_VIEWPORT    4
#define STRUCTURE_KI_OFFSET_ANIM        5
#define STRUCTURE_KI_OFFSET_ANIMSET     6
#define STRUCTURE_KI_OFFSET_ANIMPOINTER 7

/* Storage types */
#define STRUCTURE_KI_TYPE_NONE          0
#define STRUCTURE_KI_TYPE_LIST          1
#define STRUCTURE_KI_TYPE_TREE          2

/* Numbers of differents structures stored in each storage type */
#define STRUCTURE_KI_NUMBER_LIST        8
#define STRUCTURE_KI_NUMBER_TREE        1


/*
 * Internal List structure
 */
typedef struct structure_st_list_cell_t
{
  /* Corresponding structure : 4 */
  structure_st_struct *pst_struct;

  /* List handling pointers : 12 */
  struct structure_st_list_cell_t *pst_next;
  struct structure_st_list_cell_t *pst_previous;
  
  /* 4 extra bytes of padding : 16 */
  uint8 auc_unused[4];
} structure_st_list_cell;

/*
 * Internal Tree structure
 */
typedef struct structure_st_tree_cell_t
{
  /* Corresponding structure : 4 */
  structure_st_struct *pst_struct;

  /* Tree handling pointers : 20 */
  struct structure_st_tree_cell_t *pst_parent;
  struct structure_st_tree_cell_t *pst_child;
  struct structure_st_tree_cell_t *pst_left_sibling;
  struct structure_st_tree_cell_t *pst_right_sibling;

  /* 12 extra bytes of padding : 32 */
  uint8 auc_unused[12];
} structure_st_tree_cell;


/*
 * Static members
 */
static uint32 structure_su32_flags = STRUCTURE_KUL_FLAG_NONE;
static structure_st_list_cell *spast_list[STRUCTURE_KI_NUMBER_LIST];
static structure_st_tree_cell *spast_tree[STRUCTURE_KI_NUMBER_TREE];
static int32 sal_list_counter[STRUCTURE_KI_NUMBER_LIST];
static int32 sal_tree_counter[STRUCTURE_KI_NUMBER_TREE];


/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 structure_storage_info_get
 Gets storage type & offset according to structure ID.

 returns: void
 ***************************************************************************/
inline void structure_storage_info_get(uint32 _u32_type, int32 *_pi_type, int32 *_pi_offset)
{
  /* According to structure type */
  switch(_u32_type)
  {
    /* Object structure */
    case STRUCTURE_KUL_STRUCT_ID_OBJECT:
      *_pi_offset = STRUCTURE_KI_OFFSET_OBJECT;
      *_pi_type = STRUCTURE_KI_TYPE_LIST;
      break;

    /* Frame structure */
    case STRUCTURE_KUL_STRUCT_ID_FRAME:
      *_pi_offset = STRUCTURE_KI_OFFSET_FRAME;
      *_pi_type = STRUCTURE_KI_TYPE_TREE;
      break;

    /* Texture structure */
    case STRUCTURE_KUL_STRUCT_ID_TEXTURE:
      *_pi_offset = STRUCTURE_KI_OFFSET_TEXTURE;
      *_pi_type = STRUCTURE_KI_TYPE_LIST;
      break;

    /* Graphic2d structure */
    case STRUCTURE_KUL_STRUCT_ID_GRAPHIC:
      *_pi_offset = STRUCTURE_KI_OFFSET_GRAPHIC;
      *_pi_type = STRUCTURE_KI_TYPE_LIST;
      break;

    /* Camera2d structure */
    case STRUCTURE_KUL_STRUCT_ID_CAMERA:
      *_pi_offset = STRUCTURE_KI_OFFSET_CAMERA;
      *_pi_type = STRUCTURE_KI_TYPE_LIST;
      break;

    /* Viewport structure */
    case STRUCTURE_KUL_STRUCT_ID_VIEWPORT:
      *_pi_offset = STRUCTURE_KI_OFFSET_VIEWPORT;
      *_pi_type = STRUCTURE_KI_TYPE_LIST;
      break;

    /* Animation structure */
    case STRUCTURE_KUL_STRUCT_ID_ANIM:
      *_pi_offset = STRUCTURE_KI_OFFSET_ANIM;
      *_pi_type = STRUCTURE_KI_TYPE_LIST;
      break;

    /* Animation Set structure */
    case STRUCTURE_KUL_STRUCT_ID_ANIMSET:
      *_pi_offset = STRUCTURE_KI_OFFSET_ANIMSET;
      *_pi_type = STRUCTURE_KI_TYPE_LIST;
      break;

    /* Animation Pointer structure */
    case STRUCTURE_KUL_STRUCT_ID_ANIMPOINTER:
      *_pi_offset = STRUCTURE_KI_OFFSET_ANIMPOINTER;
      *_pi_type = STRUCTURE_KI_TYPE_LIST;
      break;

    default:
      /* Bad type */
      *_pi_offset = STRUCTURE_KI_OFFSET_INVALID;
      *_pi_type = STRUCTURE_KI_TYPE_NONE;
  }

  return;
}


/* *** List Handling *** */


/***************************************************************************
 structure_list_cell_delete
 Deletes a list cell.

 returns: void
 ***************************************************************************/
inline void structure_list_cell_delete(structure_st_list_cell *_pst_cell)
{
  /* Frees cell memory */
  free(_pst_cell);

  return;
}

/***************************************************************************
 structure_list_cell_create
 Creates an empty list cell linked to a structure.

 returns: list cell
 ***************************************************************************/
inline structure_st_list_cell *structure_list_cell_create(structure_st_struct *_pst_struct)
{
  structure_st_list_cell *pst_list_cell;

  /* Allocates it */
  pst_list_cell = (structure_st_list_cell *) malloc(sizeof(structure_st_list_cell));

  /* Non null? */
  if(pst_list_cell != NULL)
  {
    /* Assigns cell members */
    pst_list_cell->pst_struct = _pst_struct;
    pst_list_cell->pst_next = NULL;
    pst_list_cell->pst_previous = NULL;

    /* Assigns cell to structure */
    _pst_struct->pst_cell = (structure_st_cell *)pst_list_cell;
  }
  else
  {
    /* !!! MSG !!! */
    return NULL;
  }

  return pst_list_cell;
}

/***************************************************************************
 structure_list_cell_add
 Adds a new cell in the corresponding list.

 returns: void
 ***************************************************************************/
inline void structure_list_cell_add(structure_st_list_cell *_pst_cell, int32 _i_offset)
{
  structure_st_list_cell *pst_list;

  /* Gets list */
  pst_list = spast_list[_i_offset];

  /* Adds it at the start of the list */
  _pst_cell->pst_next = pst_list;

  /* Updates old cell if needed */
  if(pst_list != NULL)
  {
    pst_list->pst_previous = _pst_cell;
  }

  /* Stores cell at beginning of the list */
  spast_list[_i_offset] = _pst_cell;

  /* Updates list counter */
  sal_list_counter[_i_offset]++;

  return;
}

/***************************************************************************
 structure_list_cell_remove
 Removes a cell from the corresponding list.

 returns: void
 ***************************************************************************/
inline void structure_list_cell_remove(structure_st_list_cell *_pst_cell, int32 _i_offset)
{
  structure_st_list_cell *pst_previous, *pst_next;

  /* Gets neighbours pointers */
  pst_previous = _pst_cell->pst_previous;
  pst_next = _pst_cell->pst_next;

  /* Not at beginning of list? */
  if(pst_previous != NULL)
  {
    pst_previous->pst_next = pst_next;
  }
  else
  {
    /* Updates global list pointer */
    spast_list[_i_offset] = pst_next;
  }

  /* Updates next cell pointers */
  if(pst_next != NULL)
  {
    pst_next->pst_previous = pst_previous;
  }

  /* Updates list counter */
  sal_list_counter[_i_offset]--;

  return;
}


/* *** Tree Handling *** */


/***************************************************************************
 structure_tree_cell_create
 Creates an empty tree cell linked to a structure.

 returns: tree cell
 ***************************************************************************/
inline structure_st_tree_cell *structure_tree_cell_create(structure_st_struct *_pst_struct)
{
  structure_st_tree_cell *pst_tree_cell;

  /* Allocates it */
  pst_tree_cell = (structure_st_tree_cell *) malloc(sizeof(structure_st_tree_cell));

  /* Non null? */
  if(pst_tree_cell != NULL)
  {
    /* Assigns cell members */
    pst_tree_cell->pst_struct = _pst_struct;
    pst_tree_cell->pst_parent = NULL;
    pst_tree_cell->pst_child = NULL;
    pst_tree_cell->pst_left_sibling = NULL;
    pst_tree_cell->pst_right_sibling = NULL;

    /* Assigns cell to structure */
    _pst_struct->pst_cell = (structure_st_cell *)pst_tree_cell;
  }
  else
  {
    /* !!! MSG !!! */
    return NULL;
  }

  return pst_tree_cell;
}

/***************************************************************************
 structure_tree_cell_delete
 Deletes a list cell.

 returns: void
 ***************************************************************************/
inline void structure_tree_cell_delete(structure_st_tree_cell *_pst_cell)
{
  /* Frees cell memory */
  free(_pst_cell);

  return;
}

/***************************************************************************
 structure_tree_cell_move
 Moves a cell under another cell.
 If new parent cell is null, it removes it from the tree (used for delete/remove)

 returns: void
 ***************************************************************************/
inline void structure_tree_cell_move(structure_st_tree_cell *_pst_cell, structure_st_tree_cell *_pst_parent, int32 _i_offset)
{
  structure_st_tree_cell *pst_parent, *pst_left_sibling, *pst_right_sibling, *pst_child, *pst_root, *pst_it;

  /* Gets root */
  pst_root = spast_tree[_i_offset];

  /* Make sure request is valid (prevent tree from turning into graph) */
  for(pst_it = _pst_parent; (pst_it != NULL) && (pst_it != pst_root); pst_it = pst_it->pst_parent)
  {
    /* Bad request? */
    /* !!! MSG !!! */
    if(pst_it == _pst_cell)
    {
      return;
    }
  }

  /* Updates old position links (removes it from the tree) */

  /* Gets current parent */
  pst_parent = _pst_cell->pst_parent;

  /* Was the cell in the tree? */
  if(pst_parent != NULL)
  {
    /* Gets siblings */
    pst_left_sibling = _pst_cell->pst_left_sibling;
    pst_right_sibling = _pst_cell->pst_right_sibling;

    /* Is current child ? */
    if(pst_parent->pst_child == _pst_cell)
    {
      /* Updates parent's child */
      pst_parent->pst_child = pst_right_sibling;
    }
    else
    {
      /* Should have a left sibling */
      pst_left_sibling->pst_right_sibling = pst_right_sibling;
    }

    /* Has a right sibling? */
    if(pst_right_sibling != NULL)
    {
      pst_right_sibling->pst_left_sibling = pst_left_sibling;
    }
  }

  /* Updates new position links (adds it at new position in the tree) */

  /* Is new parent non null? */
  if(_pst_parent != NULL)
  {
    /* Gets child */
    pst_child = _pst_parent->pst_child;

    /* Updates new parent */
    _pst_parent->pst_child = _pst_cell;

    /* Updates old child */
    if(pst_child != NULL)
    {
      pst_child->pst_left_sibling = _pst_cell;
    }

  }
  /* Will become single (outside tree) */
  else
  {
    pst_child = NULL;
  }

  /* Becomes new child (or single) */
  _pst_cell->pst_left_sibling = NULL;
  _pst_cell->pst_right_sibling = pst_child;
  _pst_cell->pst_parent = _pst_parent;

  return;
}

/***************************************************************************
 structure_tree_cell_add
 Adds a new cell in the corresponding tree.

 returns: void
 ***************************************************************************/
inline void structure_tree_cell_add(structure_st_tree_cell *_pst_cell, int32 _i_offset)
{
  structure_st_tree_cell *pst_tree;

  /* Gets tree */
  pst_tree = spast_tree[_i_offset];

  /* Is tree empty? */
  if(pst_tree == NULL)
  {
    /* Becomes tree root */
    spast_tree[_i_offset] = _pst_cell;
  }
  else
  {
    /* Moves it under the tree root */
    structure_tree_cell_move(_pst_cell, pst_tree, _i_offset);
  }

  /* Updates tree counter */
  sal_tree_counter[_i_offset]++;

  return;
}

/***************************************************************************
 structure_tree_cell_remove
 Removes a cell from the corresponding tree.

 returns: void
 ***************************************************************************/
inline void structure_tree_cell_remove(structure_st_tree_cell *_pst_cell, int32 _i_offset)
{
  structure_st_tree_cell *pst_parent;
  structure_st_tree_cell *pst_tree;

  /* Gets tree */
  pst_tree = spast_tree[_i_offset];

  /* Gets parent */
  pst_parent = _pst_cell->pst_parent;

  /* Has parent? */
  if(pst_parent != NULL)
  {
    /* Moves all childs to the next higher level */
    while(_pst_cell->pst_child != NULL)
    {
      structure_tree_cell_move(_pst_cell->pst_child, pst_parent, _i_offset);
    }

    /* Moves it outside the tree */
    structure_tree_cell_move(_pst_cell, NULL, _i_offset);
  }
  else
  {
    /* Is root? */
    if(_pst_cell == pst_tree)
    {
      spast_tree[_i_offset] = NULL;
    }
  }

  /* Updates tree counter */
  sal_tree_counter[_i_offset]--;

  return;
}


/***************************************************************************
 structure_struct_add
 Adds a new cell in the corresponding storage.

 returns: void
 ***************************************************************************/
inline void structure_struct_add(structure_st_struct *_pst_struct)
{
  int32 i_type, i_offset;

  /* Gets storage type & offset */
  structure_storage_info_get(_pst_struct->u32_id, &i_type, &i_offset);

  /* According to storage type */
  switch(i_type)
  {
    case STRUCTURE_KI_TYPE_LIST:
      structure_list_cell_add((structure_st_list_cell *)(_pst_struct->pst_cell), i_offset);
      break;

    case STRUCTURE_KI_TYPE_TREE:
      structure_tree_cell_add((structure_st_tree_cell *)(_pst_struct->pst_cell), i_offset);
      break;

    default:
      /* !!! MSG !!! */
      break;
  }
}

/***************************************************************************
 structure_struct_remove
 Removes a cell from the corresponding storage.

 returns: void
 ***************************************************************************/
inline void structure_struct_remove(structure_st_struct *_pst_struct)
{
  int32 i_type, i_offset;

  /* Gets storage type & offset */
  structure_storage_info_get(_pst_struct->u32_id, &i_type, &i_offset);

  /* According to storage type */
  switch(i_type)
  {
    case STRUCTURE_KI_TYPE_LIST:
      structure_list_cell_remove((structure_st_list_cell *)(_pst_struct->pst_cell), i_offset);
      break;

    case STRUCTURE_KI_TYPE_TREE:
      structure_tree_cell_remove((structure_st_tree_cell *)(_pst_struct->pst_cell), i_offset);
      break;

    default:
      /* !!! MSG !!! */
      break;
  }
}


/***************************************************************************
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/


/***************************************************************************
 structure_init
 Inits structure system.

 returns: EXIT_SUCCESS/EXIT_FAILURE
 ***************************************************************************/
uint32 structure_init()
{
  int32 i;

  /* Not already Initialized? */
  if(!(structure_su32_flags & STRUCTURE_KUL_FLAG_READY))
  {
    /* Inits Flags */
    structure_su32_flags = STRUCTURE_KUL_FLAG_READY;

    /* Cleans all storage pointers & counters */
    for(i = 0; i < STRUCTURE_KI_NUMBER_LIST; i++)
    {
      spast_list[i] = NULL;
      sal_list_counter[i] = 0;
    }
    for(i = 0; i < STRUCTURE_KI_NUMBER_TREE; i++)
    {
      spast_tree[i] = NULL;
      sal_tree_counter[i] = 0;
    }

    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}

/***************************************************************************
 structure_exit
 Exits from the structure system.

 returns: void
 ***************************************************************************/
void structure_exit()
{
  /* Initialized? */
  if(structure_su32_flags & STRUCTURE_KUL_FLAG_READY)
  {
    /* Updates flags */
    structure_su32_flags &= ~STRUCTURE_KUL_FLAG_READY;
  }

  return;
}

/***************************************************************************
 structure_struct_number_get
 Gets given type structure number.

 returns: counter/-1
 ***************************************************************************/
inline int32 structure_struct_number_get(uint32 _u32_struct_id)
{
  int32 i_counter = -1;
  int32 i_type, i_offset;

  /* Gets type & offset */
  structure_storage_info_get(_u32_struct_id, &i_type, &i_offset);

  /* Depending on type */
  switch(i_type)
  {
    case STRUCTURE_KI_TYPE_LIST:
      /* Gets corresponding counter */
      i_counter =  sal_list_counter[i_offset];
      break;

    case STRUCTURE_KI_TYPE_TREE:
      /* Gets corresponding counter */
      i_counter =  sal_tree_counter[i_offset];
      break;

    default:
      /* !!! MSG !!! */
      break;
  }

  return i_counter;
}

/***************************************************************************
 structure_struct_init
 Inits a structure with given type.

 returns: EXIT_SUCCESS/EXIT_FAILURE
 ***************************************************************************/
inline uint32 structure_struct_init(structure_st_struct *_pst_struct, uint32 _u32_struct_id)
{
  int32 i_type, i_offset;
  structure_st_list_cell *pst_list_cell = NULL;
  structure_st_tree_cell *pst_tree_cell = NULL;
 
  /* Non null? */
  if(_pst_struct != NULL)
  {
    /* Gets type & offset */
    structure_storage_info_get(_u32_struct_id, &i_type, &i_offset);

    /* Depending on type */
    switch(i_type)
    {
      case STRUCTURE_KI_TYPE_LIST:

        /* Creates it */
        pst_list_cell = structure_list_cell_create(_pst_struct);

        /* Non null? */
        if(pst_list_cell != NULL)
        {
          /* Adds cell to list */
          structure_list_cell_add(pst_list_cell, i_offset);
        }
        else
        {
          /* !!! MSG !!! */
          return EXIT_FAILURE;
        }

        break;

      case STRUCTURE_KI_TYPE_TREE:

        /* Creates it */
        pst_tree_cell = structure_tree_cell_create(_pst_struct);

        /* Non null? */
        if(pst_tree_cell != NULL)
        {
          /* Adds cell to tree */
          structure_tree_cell_add(pst_tree_cell, i_offset);
        }
        else
        {
          /* !!! MSG !!! */
          return EXIT_FAILURE;
        }

        break;

      default:
        /* !!! MSG !!! */
	      return EXIT_FAILURE;
    }

    /* Cleans reference counter */
    _pst_struct->u32_ref_counter = 0;

    /* Stores ID */
    _pst_struct->u32_id = _u32_struct_id;
  }

  return EXIT_SUCCESS;
}

/***************************************************************************
 structure_struct_clean
 Cleans a structure.

 returns: void
 ***************************************************************************/
inline void structure_struct_clean(structure_st_struct *_pst_struct)
{
  int32 i_type, i_offset;

  /* Non null? */
  if(_pst_struct != NULL)
  {
    /* Gets type & offset */
    structure_storage_info_get(_pst_struct->u32_id, &i_type, &i_offset);

    /* Depending on type */
    switch(i_type)
    {
      case STRUCTURE_KI_TYPE_LIST:

        /* Removes cell from list */
        structure_list_cell_remove((structure_st_list_cell *)(_pst_struct->pst_cell), i_offset);

        /* Deletes it */
        structure_list_cell_delete((structure_st_list_cell *)(_pst_struct->pst_cell));

        break;

      case STRUCTURE_KI_TYPE_TREE:
  
        /* Removes cell from tree */
        structure_tree_cell_remove((structure_st_tree_cell *)(_pst_struct->pst_cell), i_offset);

        /* Deletes it */
        structure_tree_cell_delete((structure_st_tree_cell *)(_pst_struct->pst_cell));

        break;

      default:
        /* !!! MSG !!! */
        break;
    }

    /* Cleans ID */
    _pst_struct->u32_id = STRUCTURE_KUL_STRUCT_ID_NONE;
  }

  return;
}

/***************************************************************************
 structure_struct_first_get
 Gets first stored structure (first list cell or tree root depending on storage type).

 returns: first structure
 ***************************************************************************/
inline structure_st_struct *structure_struct_first_get(uint32 _u32_struct_id)
{
  structure_st_struct *pst_struct = NULL;
  int32 i_type, i_offset;

  /* Gets type & offset */
  structure_storage_info_get(_u32_struct_id, &i_type, &i_offset);

  /* Depending on type */
  switch(i_type)
  {
    case STRUCTURE_KI_TYPE_LIST:
      /* Non null? */
      if(spast_list[i_offset] != NULL)
      {
        pst_struct = (spast_list[i_offset])->pst_struct;
      }
      break;

    case STRUCTURE_KI_TYPE_TREE:
      /* Non null? */
      if(spast_tree[i_offset] != NULL)
      {
        pst_struct = (spast_tree[i_offset])->pst_struct;
      }
      break;

    default:
      /* !!! MSG !!! */
      return NULL;
  }

  return pst_struct;
}

/***************************************************************************
 structure_struct_counter_increase
 Increases a structure reference counter.

 returns: void
 ***************************************************************************/
inline void structure_struct_counter_increase(structure_st_struct *_pst_struct)
{
  /* Non null? */
  if(_pst_struct != NULL)
  {
    /* Updates reference counter */
    _pst_struct->u32_ref_counter++;
  }

  return;
}

/***************************************************************************
 structure_struct_counter_decrease
 Decreases a structure reference counter.

 returns: void
 ***************************************************************************/
inline void structure_struct_counter_decrease(structure_st_struct *_pst_struct)
{
  /* Non null? */
  if(_pst_struct != NULL)
  {
    /* Updates reference counter */
    _pst_struct->u32_ref_counter--;
  }

  return;
}

/***************************************************************************
 structure_struct_counter_get
 Gets a structure reference counter.

 returns: counter
 ***************************************************************************/
inline uint32 structure_struct_counter_get(structure_st_struct *_pst_struct)
{
  /* Non null? */
  if(_pst_struct != NULL)
  {
    return(_pst_struct->u32_ref_counter);
  }
  else
  {
    return 0;
  }
}

/***************************************************************************
 structure_struct_id_get
 Gets structure ID.

 returns: ID
 ***************************************************************************/
inline uint32 structure_struct_id_get(structure_st_struct *_pst_struct)
{
  /* Non null? */
  if(_pst_struct != NULL)
  {
    return(_pst_struct->u32_id);
  }
  else
  {
    return STRUCTURE_KUL_STRUCT_ID_NONE;
  }
}


/* *** Structure accessors *** */


/***************************************************************************
 structure_struct_parent_get
 Structure tree parent get accessor.

 returns: parent
 ***************************************************************************/
inline structure_st_struct *structure_struct_parent_get(structure_st_struct *_pst_struct)
{
  structure_st_tree_cell *pst_cell;

  /* Gets parent cell */
  pst_cell = ((structure_st_tree_cell *)(_pst_struct->pst_cell))->pst_parent;

  /* Non null? */
  if(pst_cell != NULL)
  {
    return pst_cell->pst_struct;
  }
  else
  {
    return NULL;
  }
}

/***************************************************************************
 structure_struct_child_get
 Structure tree child get accessor.

 returns: child
 ***************************************************************************/
inline structure_st_struct *structure_struct_child_get(structure_st_struct *_pst_struct)
{
  structure_st_tree_cell *pst_cell;

  /* Gets child cell */
  pst_cell = ((structure_st_tree_cell *)(_pst_struct->pst_cell))->pst_child;

  /* Non null? */
  if(pst_cell != NULL)
  {
    return pst_cell->pst_struct;
  }
  else
  {
    return NULL;
  }
}

/***************************************************************************
 structure_struct_left_sibling_get
 Structure tree left sibling get accessor.

 returns: left sibling
 ***************************************************************************/
inline structure_st_struct *structure_struct_left_sibling_get(structure_st_struct *_pst_struct)
{
  structure_st_tree_cell *pst_cell;

  /* Gets left sibling cell */
  pst_cell = ((structure_st_tree_cell *)(_pst_struct->pst_cell))->pst_left_sibling;

  /* Non null? */
  if(pst_cell != NULL)
  {
    return pst_cell->pst_struct;
  }
  else
  {
    return NULL;
  }
}

/***************************************************************************
 structure_struct_right_sibling_get
 Structure tree right sibling get accessor.

 returns: right sibling
 ***************************************************************************/
inline structure_st_struct *structure_struct_right_sibling_get(structure_st_struct *_pst_struct)
{
  structure_st_tree_cell *pst_cell;

  /* Gets right sibling cell */
  pst_cell = ((structure_st_tree_cell *)(_pst_struct->pst_cell))->pst_right_sibling;

  /* Non null? */
  if(pst_cell != NULL)
  {
    return pst_cell->pst_struct;
  }
  else
  {
    return NULL;
  }
}


/***************************************************************************
 structure_struct_previous_get
 Structure list previous get accessor.

 returns: previous
 ***************************************************************************/
inline structure_st_struct *structure_struct_previous_get(structure_st_struct *_pst_struct)
{
  structure_st_list_cell *pst_cell;

  /* Gets previous cell */
  pst_cell = ((structure_st_list_cell *)(_pst_struct->pst_cell))->pst_previous;

  /* Non null? */
  if(pst_cell != NULL)
  {
    return pst_cell->pst_struct;
  }
  else
  {
    return NULL;
  }
}

/***************************************************************************
 structure_struct_next_get
 Structure list next get accessor.

 returns: next
 ***************************************************************************/
inline structure_st_struct *structure_struct_next_get(structure_st_struct *_pst_struct)
{
  structure_st_list_cell *pst_cell;

  /* Gets next cell */
  pst_cell = ((structure_st_list_cell *)(_pst_struct->pst_cell))->pst_next;

  /* Non null? */
  if(pst_cell != NULL)
  {
    return pst_cell->pst_struct;
  }
  else
  {
    return NULL;
  }
}

/***************************************************************************
 structure_struct_parent_set
 Structure tree parent set accessor.

 returns: void
 ***************************************************************************/
inline void structure_struct_parent_set(structure_st_struct *_pst_struct, structure_st_struct *_pst_parent)
{
  int32 i_type, i_offset;

  /* Gets storage type & offset */
  structure_storage_info_get(_pst_struct->u32_id, &i_type, &i_offset);

  /* Moves cell */
  structure_tree_cell_move((structure_st_tree_cell *)(_pst_struct->pst_cell), (structure_st_tree_cell *)(_pst_parent->pst_cell), i_offset);

  return;
}
