/** 
 * \file orxLinkList.h
 * 
 * Link List Module.
 * Allows to handle link lists.
 * 
 * \todo
 * Add cell accessors.
 * Add GetList / GetCounter accessors.
 * Add "AddAfter & AddBefore" functions.
 * Add new features at need.
 */


/***************************************************************************
 orxLinkList.h
 Link List module
 
 begin                : 06/04/2005
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


#ifndef _orxLINKLIST_H_
#define _orxLINKLIST_H_


#include "orxInclude.h"


/*
 * Internal List Cell structure
 */
struct __orxLINKLIST_CELL_t
{
  /* List handling pointers : 8 */
  struct __orxLINKLIST_CELL_t *pstNext;
  struct __orxLINKLIST_CELL_t *pstPrevious;

  /* Associated list : 12 */
  struct __orxLINKLIST_t *pstList;
};

/*
 * Internal List structure
 */
struct __orxLINKLIST_t
{
  /* List cell pointers : 8 */
  struct __orxLINKLIST_CELL_t *pstFirst;
  struct __orxLINKLIST_CELL_t *pstLast;

  /* Counter : 12 */
  orxU32 u32Counter;
};

/* Link list types */
typedef struct __orxLINKLIST_t        orxLINKLIST;
typedef struct __orxLINKLIST_CELL_t   orxLINKLIST_CELL;


/** Inits the object system. */
extern orxSTATUS                      orxLinkList_Init();
/** Ends the object system. */
extern orxVOID                        orxLinkList_Exit();

/** Setups a link list. */
extern orxSTATUS                      orxLinkList_Setup(orxLINKLIST *_pstList);

/** Adds a cell at the start of the list. */
extern orxSTATUS                      orxLinkList_AddStart(orxLINKLIST *_pstList, orxLINKLIST_CELL *_pstCell);
/** Adds a cell at the end of the list. */
extern orxSTATUS                      orxLinkList_AddEnd(orxLINKLIST *_pstList, orxLINKLIST_CELL *_pstCell);

/** Removes a cell from its list. */
extern orxSTATUS                      orxLinkList_Remove(orxLINKLIST_CELL *_pstCell);

#endif /* _orxLINKLIST_H_ */
