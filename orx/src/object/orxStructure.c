/***************************************************************************
 orxStructure.c
 Structure module
 
 begin                : 08/12/2003
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


#include "object/orxStructure.h"

#include "memory/orxMemory.h"


/*
 * Platform independant defines
 */

#define orxSTRUCTURE_KU32_FLAG_NONE         0x00000000
#define orxSTRUCTURE_KU32_FLAG_READY        0x00000001

/* Tree offsets */
#define STRUCTURE_KS32_OFFSET_FRAME       0

/* List offsets */
#define STRUCTURE_KS32_OFFSET_OBJECT      0
#define STRUCTURE_KS32_OFFSET_TEXTURE     1
#define STRUCTURE_KS32_OFFSET_GRAPHIC     2
#define STRUCTURE_KS32_OFFSET_CAMERA      3
#define STRUCTURE_KS32_OFFSET_VIEWPORT    4
#define STRUCTURE_KS32_OFFSET_ANIM        5
#define STRUCTURE_KS32_OFFSET_ANIMSET     6
#define STRUCTURE_KS32_OFFSET_ANIMPOINTER 7

/* Storage types */
#define STRUCTURE_KS32_TYPE_NONE          0
#define STRUCTURE_KS32_TYPE_LIST          1
#define STRUCTURE_KS32_TYPE_TREE          2

/* Numbers of differents structures stored in each storage type */
#define STRUCTURE_KS32_NUMBER_LIST        8
#define STRUCTURE_KS32_NUMBER_TREE        1


/*
 * Internal Tree structure
 */
typedef struct structure_st_tree_cell_t
{
  /* Corresponding structure : 4 */
  orxSTRUCTURE *pstStructure;

  /* Tree handling pointers : 20 */
  struct structure_st_tree_cell_t *pstParent;
  struct structure_st_tree_cell_t *pst_child;
  struct structure_st_tree_cell_t *pst_left_sibling;
  struct structure_st_tree_cell_t *pst_right_sibling;

  /* 12 extra bytes of padding : 32 */
  orxU8 au8Unused[12];
} structure_st_tree_cell;


/*
 * Static members
 */
static orxU32 structure_su32Flags = orxSTRUCTURE_KU32_FLAG_NONE;
static structure_st_list_cell *spast_list[STRUCTURE_KS32_NUMBER_LIST];
static structure_st_tree_cell *spast_tree[STRUCTURE_KS32_NUMBER_TREE];
static orxS32 sal_list_counter[STRUCTURE_KS32_NUMBER_LIST];
static orxS32 sal_tree_counter[STRUCTURE_KS32_NUMBER_TREE];


/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 structure_storage_info_get
 Gets storage type & offset according to structure ID.

 returns: orxVOID
 ***************************************************************************/
inline orxVOID structure_storage_info_get(orxU32 _u32_type, orxS32 *_pi_type, orxS32 *_pi_offset)
{
  /* According to structure type */
  switch(_u32_type)
  {
    /* Object structure */
    case orxSTRUCTURE_ID_OBJECT:
      *_pi_offset = STRUCTURE_KS32_OFFSET_OBJECT;
      *_pi_type = STRUCTURE_KS32_TYPE_LIST;
      break;

    /* Frame structure */
    case orxSTRUCTURE_ID_FRAME:
      *_pi_offset = STRUCTURE_KS32_OFFSET_FRAME;
      *_pi_type = STRUCTURE_KS32_TYPE_TREE;
      break;

    /* Texture structure */
    case orxSTRUCTURE_ID_TEXTURE:
      *_pi_offset = STRUCTURE_KS32_OFFSET_TEXTURE;
      *_pi_type = STRUCTURE_KS32_TYPE_LIST;
      break;

    /* Graphic2d structure */
    case orxSTRUCTURE_ID_GRAPHIC:
      *_pi_offset = STRUCTURE_KS32_OFFSET_GRAPHIC;
      *_pi_type = STRUCTURE_KS32_TYPE_LIST;
      break;

    /* Camera2d structure */
    case orxSTRUCTURE_ID_CAMERA:
      *_pi_offset = STRUCTURE_KS32_OFFSET_CAMERA;
      *_pi_type = STRUCTURE_KS32_TYPE_LIST;
      break;

    /* Viewport structure */
    case orxSTRUCTURE_ID_VIEWPORT:
      *_pi_offset = STRUCTURE_KS32_OFFSET_VIEWPORT;
      *_pi_type = STRUCTURE_KS32_TYPE_LIST;
      break;

    /* Animation structure */
    case orxSTRUCTURE_ID_ANIM:
      *_pi_offset = STRUCTURE_KS32_OFFSET_ANIM;
      *_pi_type = STRUCTURE_KS32_TYPE_LIST;
      break;

    /* Animation Set structure */
    case orxSTRUCTURE_ID_ANIMSET:
      *_pi_offset = STRUCTURE_KS32_OFFSET_ANIMSET;
      *_pi_type = STRUCTURE_KS32_TYPE_LIST;
      break;

    /* Animation Pointer structure */
    case orxSTRUCTURE_ID_ANIMPOINTER:
      *_pi_offset = STRUCTURE_KS32_OFFSET_ANIMPOINTER;
      *_pi_type = STRUCTURE_KS32_TYPE_LIST;
      break;

    default:
      /* Bad type */
      *_pi_offset = orxU32_Undefined;
      *_pi_type = STRUCTURE_KS32_TYPE_NONE;
  }

  return;
}


/* *** List Handling *** */


/***************************************************************************
 structure_list_cell_delete
 Deletes a list cell.

 returns: orxVOID
 ***************************************************************************/
inline orxVOID structure_list_cell_delete(structure_st_list_cell *_pstCell)
{
  /* Frees cell memory */
  orxMemory_Free(_pstCell);

  return;
}

/***************************************************************************
 structure_list_cell_create
 Creates an empty list cell linked to a structure.

 returns: list cell
 ***************************************************************************/
inline structure_st_list_cell *structure_list_cell_create(orxSTRUCTURE *_pstStructure)
{
  structure_st_list_cell *pst_list_cell;

  /* Allocates it */
  pst_list_cell = (structure_st_list_cell *) orxMemory_Allocate(sizeof(structure_st_list_cell), orxMEMORY_TYPE_MAIN);

  /* Non null? */
  if(pst_list_cell != orxNULL)
  {
    /* Assigns cell members */
    pst_list_cell->pstStructure = _pstStructure;
    pst_list_cell->pstNext = orxNULL;
    pst_list_cell->pstPrevious = orxNULL;

    /* Assigns cell to structure */
    _pstStructure->pstCell = (orxSTRUCTURE_CELL *)pst_list_cell;
  }
  else
  {
    /* !!! MSG !!! */
    return orxNULL;
  }

  return pst_list_cell;
}

/***************************************************************************
 structure_list_cell_add
 Adds a new cell in the corresponding list.

 returns: orxVOID
 ***************************************************************************/
inline orxVOID structure_list_cell_add(structure_st_list_cell *_pstCell, orxS32 _i_offset)
{
  structure_st_list_cell *pst_list;

  /* Gets list */
  pst_list = spast_list[_i_offset];

  /* Adds it at the start of the list */
  _pstCell->pstNext = pst_list;

  /* Updates old cell if needed */
  if(pst_list != orxNULL)
  {
    pst_list->pstPrevious = _pstCell;
  }

  /* Stores cell at beginning of the list */
  spast_list[_i_offset] = _pstCell;

  /* Updates list counter */
  sal_list_counter[_i_offset]++;

  return;
}

/***************************************************************************
 structure_list_cell_remove
 Removes a cell from the corresponding list.

 returns: orxVOID
 ***************************************************************************/
inline orxVOID structure_list_cell_remove(structure_st_list_cell *_pstCell, orxS32 _i_offset)
{
  structure_st_list_cell *pstPrevious, *pstNext;

  /* Gets neighbours pointers */
  pstPrevious = _pstCell->pstPrevious;
  pstNext = _pstCell->pstNext;

  /* Not at beginning of list? */
  if(pstPrevious != orxNULL)
  {
    pstPrevious->pstNext = pstNext;
  }
  else
  {
    /* Updates global list pointer */
    spast_list[_i_offset] = pstNext;
  }

  /* Updates next cell pointers */
  if(pstNext != orxNULL)
  {
    pstNext->pstPrevious = pstPrevious;
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
inline structure_st_tree_cell *structure_tree_cell_create(orxSTRUCTURE *_pstStructure)
{
  structure_st_tree_cell *pst_tree_cell;

  /* Allocates it */
  pst_tree_cell = (structure_st_tree_cell *) orxMemory_Allocate(sizeof(structure_st_tree_cell), orxMEMORY_TYPE_MAIN);

  /* Non null? */
  if(pst_tree_cell != orxNULL)
  {
    /* Assigns cell members */
    pst_tree_cell->pstStructure = _pstStructure;
    pst_tree_cell->pstParent = orxNULL;
    pst_tree_cell->pst_child = orxNULL;
    pst_tree_cell->pst_left_sibling = orxNULL;
    pst_tree_cell->pst_right_sibling = orxNULL;

    /* Assigns cell to structure */
    _pstStructure->pstCell = (orxSTRUCTURE_CELL *)pst_tree_cell;
  }
  else
  {
    /* !!! MSG !!! */
    return orxNULL;
  }

  return pst_tree_cell;
}

/***************************************************************************
 structure_tree_cell_delete
 Deletes a list cell.

 returns: orxVOID
 ***************************************************************************/
inline orxVOID structure_tree_cell_delete(structure_st_tree_cell *_pstCell)
{
  /* Frees cell memory */
  orxMemory_Free(_pstCell);

  return;
}

/***************************************************************************
 structure_tree_cell_move
 Moves a cell under another cell.
 If new parent cell is null, it removes it from the tree (used for delete/remove)

 returns: orxVOID
 ***************************************************************************/
inline orxVOID structure_tree_cell_move(structure_st_tree_cell *_pstCell, structure_st_tree_cell *_pstParent, orxS32 _i_offset)
{
  structure_st_tree_cell *pstParent, *pst_left_sibling, *pst_right_sibling, *pst_child, *pst_root, *pst_it;

  /* Gets root */
  pst_root = spast_tree[_i_offset];

  /* Make sure request is valid (prevent tree from turning into graph) */
  for(pst_it = _pstParent; (pst_it != orxNULL) && (pst_it != pst_root); pst_it = pst_it->pstParent)
  {
    /* Bad request? */
    /* !!! MSG !!! */
    if(pst_it == _pstCell)
    {
      return;
    }
  }

  /* Updates old position links (removes it from the tree) */

  /* Gets current parent */
  pstParent = _pstCell->pstParent;

  /* Was the cell in the tree? */
  if(pstParent != orxNULL)
  {
    /* Gets siblings */
    pst_left_sibling = _pstCell->pst_left_sibling;
    pst_right_sibling = _pstCell->pst_right_sibling;

    /* Is current child ? */
    if(pstParent->pst_child == _pstCell)
    {
      /* Updates parent's child */
      pstParent->pst_child = pst_right_sibling;
    }
    else
    {
      /* Should have a left sibling */
      pst_left_sibling->pst_right_sibling = pst_right_sibling;
    }

    /* Has a right sibling? */
    if(pst_right_sibling != orxNULL)
    {
      pst_right_sibling->pst_left_sibling = pst_left_sibling;
    }
  }

  /* Updates new position links (adds it at new position in the tree) */

  /* Is new parent non null? */
  if(_pstParent != orxNULL)
  {
    /* Gets child */
    pst_child = _pstParent->pst_child;

    /* Updates new parent */
    _pstParent->pst_child = _pstCell;

    /* Updates old child */
    if(pst_child != orxNULL)
    {
      pst_child->pst_left_sibling = _pstCell;
    }

  }
  /* Will become single (outside tree) */
  else
  {
    pst_child = orxNULL;
  }

  /* Becomes new child (or single) */
  _pstCell->pst_left_sibling = orxNULL;
  _pstCell->pst_right_sibling = pst_child;
  _pstCell->pstParent = _pstParent;

  return;
}

/***************************************************************************
 structure_tree_cell_add
 Adds a new cell in the corresponding tree.

 returns: orxVOID
 ***************************************************************************/
inline orxVOID structure_tree_cell_add(structure_st_tree_cell *_pstCell, orxS32 _i_offset)
{
  structure_st_tree_cell *pst_tree;

  /* Gets tree */
  pst_tree = spast_tree[_i_offset];

  /* Is tree empty? */
  if(pst_tree == orxNULL)
  {
    /* Becomes tree root */
    spast_tree[_i_offset] = _pstCell;
  }
  else
  {
    /* Moves it under the tree root */
    structure_tree_cell_move(_pstCell, pst_tree, _i_offset);
  }

  /* Updates tree counter */
  sal_tree_counter[_i_offset]++;

  return;
}

/***************************************************************************
 structure_tree_cell_remove
 Removes a cell from the corresponding tree.

 returns: orxVOID
 ***************************************************************************/
inline orxVOID structure_tree_cell_remove(structure_st_tree_cell *_pstCell, orxS32 _i_offset)
{
  structure_st_tree_cell *pstParent;
  structure_st_tree_cell *pst_tree;

  /* Gets tree */
  pst_tree = spast_tree[_i_offset];

  /* Gets parent */
  pstParent = _pstCell->pstParent;

  /* Has parent? */
  if(pstParent != orxNULL)
  {
    /* Moves all childs to the next higher level */
    while(_pstCell->pst_child != orxNULL)
    {
      structure_tree_cell_move(_pstCell->pst_child, pstParent, _i_offset);
    }

    /* Moves it outside the tree */
    structure_tree_cell_move(_pstCell, orxNULL, _i_offset);
  }
  else
  {
    /* Is root? */
    if(_pstCell == pst_tree)
    {
      spast_tree[_i_offset] = orxNULL;
    }
  }

  /* Updates tree counter */
  sal_tree_counter[_i_offset]--;

  return;
}


/***************************************************************************
 structure_struct_add
 Adds a new cell in the corresponding storage.

 returns: orxVOID
 ***************************************************************************/
inline orxVOID structure_struct_add(orxSTRUCTURE *_pstStructure)
{
  orxS32 i_type, i_offset;

  /* Gets storage type & offset */
  structure_storage_info_get(_pstStructure->eID, &i_type, &i_offset);

  /* According to storage type */
  switch(i_type)
  {
    case STRUCTURE_KS32_TYPE_LIST:
      structure_list_cell_add((structure_st_list_cell *)(_pstStructure->pstCell), i_offset);
      break;

    case STRUCTURE_KS32_TYPE_TREE:
      structure_tree_cell_add((structure_st_tree_cell *)(_pstStructure->pstCell), i_offset);
      break;

    default:
      /* !!! MSG !!! */
      break;
  }
}

/***************************************************************************
 structure_struct_remove
 Removes a cell from the corresponding storage.

 returns: orxVOID
 ***************************************************************************/
inline orxVOID structure_struct_remove(orxSTRUCTURE *_pstStructure)
{
  orxS32 i_type, i_offset;

  /* Gets storage type & offset */
  structure_storage_info_get(_pstStructure->eID, &i_type, &i_offset);

  /* According to storage type */
  switch(i_type)
  {
    case STRUCTURE_KS32_TYPE_LIST:
      structure_list_cell_remove((structure_st_list_cell *)(_pstStructure->pstCell), i_offset);
      break;

    case STRUCTURE_KS32_TYPE_TREE:
      structure_tree_cell_remove((structure_st_tree_cell *)(_pstStructure->pstCell), i_offset);
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
 orxStructure_Init
 Inits structure system.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxStructure_Init()
{
  orxS32 i;

  /* Not already Initialized? */
  if(!(structure_su32Flags & orxSTRUCTURE_KU32_FLAG_READY))
  {
    /* Inits Flags */
    structure_su32Flags = orxSTRUCTURE_KU32_FLAG_READY;

    /* Cleans all storage pointers & counters */
    for(i = 0; i < STRUCTURE_KS32_NUMBER_LIST; i++)
    {
      spast_list[i] = orxNULL;
      sal_list_counter[i] = 0;
    }
    for(i = 0; i < STRUCTURE_KS32_NUMBER_TREE; i++)
    {
      spast_tree[i] = orxNULL;
      sal_tree_counter[i] = 0;
    }

    return orxSTATUS_SUCCESS;
  }

  return orxSTATUS_FAILED;
}

/***************************************************************************
 orxStructure_Exit
 Exits from the structure system.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxStructure_Exit()
{
  /* Initialized? */
  if(structure_su32Flags & orxSTRUCTURE_KU32_FLAG_READY)
  {
    /* Updates flags */
    structure_su32Flags &= ~orxSTRUCTURE_KU32_FLAG_READY;
  }

  return;
}

/***************************************************************************
 orxStructure_GetNumber
 Gets given type structure number.

 returns: counter/-1
 ***************************************************************************/
inline orxU32 orxStructure_GetNumber(orxSTRUCTURE_ID _eStructureID)
{
  orxS32 i_counter = -1;
  orxS32 i_type, i_offset;

  /* Gets type & offset */
  structure_storage_info_get(_eStructureID, &i_type, &i_offset);

  /* Depending on type */
  switch(i_type)
  {
    case STRUCTURE_KS32_TYPE_LIST:
      /* Gets corresponding counter */
      i_counter =  sal_list_counter[i_offset];
      break;

    case STRUCTURE_KS32_TYPE_TREE:
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
 orxStructure_Setup
 Inits a structure with given type.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
inline orxSTATUS orxStructure_Setup(orxSTRUCTURE *_pstStructure, orxSTRUCTURE_ID _eStructureID)
{
  orxS32 i_type, i_offset;
  structure_st_list_cell *pst_list_cell = orxNULL;
  structure_st_tree_cell *pst_tree_cell = orxNULL;
 
  /* Non null? */
  if(_pstStructure != orxNULL)
  {
    /* Gets type & offset */
    structure_storage_info_get(_eStructureID, &i_type, &i_offset);

    /* Depending on type */
    switch(i_type)
    {
      case STRUCTURE_KS32_TYPE_LIST:

        /* Creates it */
        pst_list_cell = structure_list_cell_create(_pstStructure);

        /* Non null? */
        if(pst_list_cell != orxNULL)
        {
          /* Adds cell to list */
          structure_list_cell_add(pst_list_cell, i_offset);
        }
        else
        {
          /* !!! MSG !!! */
          return orxSTATUS_FAILED;
        }

        break;

      case STRUCTURE_KS32_TYPE_TREE:

        /* Creates it */
        pst_tree_cell = structure_tree_cell_create(_pstStructure);

        /* Non null? */
        if(pst_tree_cell != orxNULL)
        {
          /* Adds cell to tree */
          structure_tree_cell_add(pst_tree_cell, i_offset);
        }
        else
        {
          /* !!! MSG !!! */
          return orxSTATUS_FAILED;
        }

        break;

      default:
        /* !!! MSG !!! */
	      return orxSTATUS_FAILED;
    }

    /* Cleans reference counter */
    _pstStructure->u32RefCounter = 0;

    /* Stores ID */
    _pstStructure->eID = _eStructureID;
  }

  return orxSTATUS_SUCCESS;
}

/***************************************************************************
 orxStructure_Clean
 Cleans a structure.

 returns: orxVOID
 ***************************************************************************/
inline orxVOID orxStructure_Clean(orxSTRUCTURE *_pstStructure)
{
  orxS32 i_type, i_offset;

  /* Non null? */
  if(_pstStructure != orxNULL)
  {
    /* Gets type & offset */
    structure_storage_info_get(_pstStructure->eID, &i_type, &i_offset);

    /* Depending on type */
    switch(i_type)
    {
      case STRUCTURE_KS32_TYPE_LIST:

        /* Removes cell from list */
        structure_list_cell_remove((structure_st_list_cell *)(_pstStructure->pstCell), i_offset);

        /* Deletes it */
        structure_list_cell_delete((structure_st_list_cell *)(_pstStructure->pstCell));

        break;

      case STRUCTURE_KS32_TYPE_TREE:
  
        /* Removes cell from tree */
        structure_tree_cell_remove((structure_st_tree_cell *)(_pstStructure->pstCell), i_offset);

        /* Deletes it */
        structure_tree_cell_delete((structure_st_tree_cell *)(_pstStructure->pstCell));

        break;

      default:
        /* !!! MSG !!! */
        break;
    }

    /* Cleans ID */
    _pstStructure->eID = orxSTRUCTURE_ID_NONE;
  }

  return;
}

/***************************************************************************
 orxStructure_GetFirst
 Gets first stored structure (first list cell or tree root depending on storage type).

 returns: first structure
 ***************************************************************************/
inline orxSTRUCTURE *orxStructure_GetFirst(orxSTRUCTURE_ID _eStructureID)
{
  orxSTRUCTURE *pstStructure = orxNULL;
  orxS32 i_type, i_offset;

  /* Gets type & offset */
  structure_storage_info_get(_eStructureID, &i_type, &i_offset);

  /* Depending on type */
  switch(i_type)
  {
    case STRUCTURE_KS32_TYPE_LIST:
      /* Non null? */
      if(spast_list[i_offset] != orxNULL)
      {
        pstStructure = (spast_list[i_offset])->pstStructure;
      }
      break;

    case STRUCTURE_KS32_TYPE_TREE:
      /* Non null? */
      if(spast_tree[i_offset] != orxNULL)
      {
        pstStructure = (spast_tree[i_offset])->pstStructure;
      }
      break;

    default:
      /* !!! MSG !!! */
      return orxNULL;
  }

  return pstStructure;
}

/***************************************************************************
 orxStructure_IncreaseCounter
 Increases a structure reference counter.

 returns: orxVOID
 ***************************************************************************/
inline orxVOID orxStructure_IncreaseCounter(orxSTRUCTURE *_pstStructure)
{
  /* Non null? */
  if(_pstStructure != orxNULL)
  {
    /* Updates reference counter */
    _pstStructure->u32RefCounter++;
  }

  return;
}

/***************************************************************************
 orxStructure_DecreaseCounter
 Decreases a structure reference counter.

 returns: orxVOID
 ***************************************************************************/
inline orxVOID orxStructure_DecreaseCounter(orxSTRUCTURE *_pstStructure)
{
  /* Non null? */
  if(_pstStructure != orxNULL)
  {
    /* Updates reference counter */
    _pstStructure->u32RefCounter--;
  }

  return;
}

/***************************************************************************
 orxStructure_GetCounter
 Gets a structure reference counter.

 returns: counter
 ***************************************************************************/
inline orxU32 orxStructure_GetCounter(orxSTRUCTURE *_pstStructure)
{
  /* Non null? */
  if(_pstStructure != orxNULL)
  {
    return(_pstStructure->u32RefCounter);
  }
  else
  {
    return 0;
  }
}

/***************************************************************************
 orxStructure_GetID
 Gets structure ID.

 returns: ID
 ***************************************************************************/
inline orxSTRUCTURE_ID orxStructure_GetID(orxSTRUCTURE *_pstStructure)
{
  /* Non null? */
  if(_pstStructure != orxNULL)
  {
    return(_pstStructure->eID);
  }
  else
  {
    return orxSTRUCTURE_ID_NONE;
  }
}


/* *** Structure accessors *** */


/***************************************************************************
 orxStructure_GetParent
 Structure tree parent get accessor.

 returns: parent
 ***************************************************************************/
inline orxSTRUCTURE *orxStructure_GetParent(orxSTRUCTURE *_pstStructure)
{
  structure_st_tree_cell *pstCell;

  /* Gets parent cell */
  pstCell = ((structure_st_tree_cell *)(_pstStructure->pstCell))->pstParent;

  /* Non null? */
  if(pstCell != orxNULL)
  {
    return pstCell->pstStructure;
  }
  else
  {
    return orxNULL;
  }
}

/***************************************************************************
 orxStructure_GetChild
 Structure tree child get accessor.

 returns: child
 ***************************************************************************/
inline orxSTRUCTURE *orxStructure_GetChild(orxSTRUCTURE *_pstStructure)
{
  structure_st_tree_cell *pstCell;

  /* Gets child cell */
  pstCell = ((structure_st_tree_cell *)(_pstStructure->pstCell))->pst_child;

  /* Non null? */
  if(pstCell != orxNULL)
  {
    return pstCell->pstStructure;
  }
  else
  {
    return orxNULL;
  }
}

/***************************************************************************
 orxStructure_GetLeftSibling
 Structure tree left sibling get accessor.

 returns: left sibling
 ***************************************************************************/
inline orxSTRUCTURE *orxStructure_GetLeftSibling(orxSTRUCTURE *_pstStructure)
{
  structure_st_tree_cell *pstCell;

  /* Gets left sibling cell */
  pstCell = ((structure_st_tree_cell *)(_pstStructure->pstCell))->pst_left_sibling;

  /* Non null? */
  if(pstCell != orxNULL)
  {
    return pstCell->pstStructure;
  }
  else
  {
    return orxNULL;
  }
}

/***************************************************************************
 orxStructure_GetRightSibling
 Structure tree right sibling get accessor.

 returns: right sibling
 ***************************************************************************/
inline orxSTRUCTURE *orxStructure_GetRightSibling(orxSTRUCTURE *_pstStructure)
{
  structure_st_tree_cell *pstCell;

  /* Gets right sibling cell */
  pstCell = ((structure_st_tree_cell *)(_pstStructure->pstCell))->pst_right_sibling;

  /* Non null? */
  if(pstCell != orxNULL)
  {
    return pstCell->pstStructure;
  }
  else
  {
    return orxNULL;
  }
}


/***************************************************************************
 orxStructure_GetPrevious
 Structure list previous get accessor.

 returns: previous
 ***************************************************************************/
inline orxSTRUCTURE *orxStructure_GetPrevious(orxSTRUCTURE *_pstStructure)
{
  structure_st_list_cell *pstCell;

  /* Gets previous cell */
  pstCell = ((structure_st_list_cell *)(_pstStructure->pstCell))->pstPrevious;

  /* Non null? */
  if(pstCell != orxNULL)
  {
    return pstCell->pstStructure;
  }
  else
  {
    return orxNULL;
  }
}

/***************************************************************************
 orxStructure_GetNext
 Structure list next get accessor.

 returns: next
 ***************************************************************************/
inline orxSTRUCTURE *orxStructure_GetNext(orxSTRUCTURE *_pstStructure)
{
  structure_st_list_cell *pstCell;

  /* Gets next cell */
  pstCell = ((structure_st_list_cell *)(_pstStructure->pstCell))->pstNext;

  /* Non null? */
  if(pstCell != orxNULL)
  {
    return pstCell->pstStructure;
  }
  else
  {
    return orxNULL;
  }
}

/***************************************************************************
 orxStructure_SetParent
 Structure tree parent set accessor.

 returns: orxVOID
 ***************************************************************************/
inline orxVOID orxStructure_SetParent(orxSTRUCTURE *_pstStructure, orxSTRUCTURE *_pstParent)
{
  orxS32 i_type, i_offset;

  /* Gets storage type & offset */
  structure_storage_info_get(_pstStructure->eID, &i_type, &i_offset);

  /* Moves cell */
  structure_tree_cell_move((structure_st_tree_cell *)(_pstStructure->pstCell), (structure_st_tree_cell *)(_pstParent->pstCell), i_offset);

  return;
}
