/** 
 * \file orxLinkList.h
 * 
 * Link List Module.
 * Allows to handle link lists.
 * 
 * \todo
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

#include "debug/orxDebug.h"


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
/** Adds a cell before another one. */
extern orxSTATUS                      orxLinkList_AddBefore(orxLINKLIST_CELL *_pstRefCell, orxLINKLIST_CELL *_pstCell);
/** Adds a cell after another one. */
extern orxSTATUS                      orxLinkList_AddAfter(orxLINKLIST_CELL *_pstRefCell, orxLINKLIST_CELL *_pstCell);

/** Removes a cell from its list. */
extern orxSTATUS                      orxLinkList_Remove(orxLINKLIST_CELL *_pstCell);


/* *** LinkList inlined accessors *** */


/** Gets a cell list. */
extern orxINLINE  orxLINKLIST        *orxLinkList_GetList(orxLINKLIST_CELL *_pstCell)
{
  /* Checks */
  orxASSERT(_pstCell != orxNULL);

  return(_pstCell->pstList);
}

/** Gets a cell previous. */
extern orxINLINE  orxLINKLIST_CELL   *orxLinkList_GetPrevious(orxLINKLIST_CELL *_pstCell)
{
  /* Checks */
  orxASSERT(_pstCell != orxNULL);

  return(_pstCell->pstPrevious);
}

/** Gets a cell next. */
extern orxINLINE  orxLINKLIST_CELL   *orxLinkList_GetNext(orxLINKLIST_CELL *_pstCell)
{
  /* Checks */
  orxASSERT(_pstCell != orxNULL);

  return(_pstCell->pstNext);
}


/** Gets a list first cell. */
extern orxINLINE  orxLINKLIST_CELL   *orxLinkList_GetFirst(orxLINKLIST *_pstList)
{
  /* Checks */
  orxASSERT(_pstList != orxNULL);

  return(_pstList->pstFirst);
}

/** Gets a list last cell. */
extern orxINLINE  orxLINKLIST_CELL   *orxLinkList_GetLast(orxLINKLIST *_pstList)
{
  /* Checks */
  orxASSERT(_pstList != orxNULL);

  return(_pstList->pstLast);
}

/** Gets a list counter. */
extern orxINLINE  orxLINKLIST_CELL   *orxLinkList_GetCounter(orxLINKLIST *_pstList)
{
  /* Checks */
  orxASSERT(_pstList != orxNULL);

  return(_pstList->u32Counter);
}


#endif /* _orxLINKLIST_H_ */
