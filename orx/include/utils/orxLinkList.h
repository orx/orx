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
 * List Node structure
 */
struct __orxLINKLIST_NODE_t
{
  /* List handling pointers : 8 */
  struct __orxLINKLIST_NODE_t *pstNext;
  struct __orxLINKLIST_NODE_t *pstPrevious;

  /* Associated list : 12 */
  struct __orxLINKLIST_t *pstList;
};

/*
 * List structure
 */
struct __orxLINKLIST_t
{
  /* List node pointers : 8 */
  struct __orxLINKLIST_NODE_t *pstFirst;
  struct __orxLINKLIST_NODE_t *pstLast;

  /* Counter : 12 */
  orxU32 u32Counter;
};

/* Link list types */
typedef struct __orxLINKLIST_t        orxLINKLIST;
typedef struct __orxLINKLIST_NODE_t   orxLINKLIST_NODE;


/** Inits the object system. */
extern orxSTATUS                      orxLinkList_Init();
/** Ends the object system. */
extern orxVOID                        orxLinkList_Exit();

/** Cleans a link list. */
extern orxSTATUS                      orxLinkList_Clean(orxLINKLIST *_pstList);

/** Adds a node at the start of the list. */
extern orxSTATUS                      orxLinkList_AddStart(orxLINKLIST *_pstList, orxLINKLIST_NODE *_pstNode);
/** Adds a node at the end of the list. */
extern orxSTATUS                      orxLinkList_AddEnd(orxLINKLIST *_pstList, orxLINKLIST_NODE *_pstNode);
/** Adds a node before another one. */
extern orxSTATUS                      orxLinkList_AddBefore(orxLINKLIST_NODE *_pstRefNode, orxLINKLIST_NODE *_pstNode);
/** Adds a node after another one. */
extern orxSTATUS                      orxLinkList_AddAfter(orxLINKLIST_NODE *_pstRefNode, orxLINKLIST_NODE *_pstNode);

/** Removes a node from its list. */
extern orxSTATUS                      orxLinkList_Remove(orxLINKLIST_NODE *_pstNode);


/* *** LinkList inlined accessors *** */


/** Gets a node list. */
extern orxINLINE  orxLINKLIST        *orxLinkList_GetList(orxLINKLIST_NODE *_pstNode)
{
  /* Checks */
  orxASSERT(_pstNode != orxNULL);

  return(_pstNode->pstList);
}

/** Gets a node previous. */
extern orxINLINE  orxLINKLIST_NODE   *orxLinkList_GetPrevious(orxLINKLIST_NODE *_pstNode)
{
  /* Checks */
  orxASSERT(_pstNode != orxNULL);

  return(_pstNode->pstPrevious);
}

/** Gets a node next. */
extern orxINLINE  orxLINKLIST_NODE   *orxLinkList_GetNext(orxLINKLIST_NODE *_pstNode)
{
  /* Checks */
  orxASSERT(_pstNode != orxNULL);

  return(_pstNode->pstNext);
}


/** Gets a list first node. */
extern orxINLINE  orxLINKLIST_NODE   *orxLinkList_GetFirst(orxLINKLIST *_pstList)
{
  /* Checks */
  orxASSERT(_pstList != orxNULL);

  return(_pstList->pstFirst);
}

/** Gets a list last node. */
extern orxINLINE  orxLINKLIST_NODE   *orxLinkList_GetLast(orxLINKLIST *_pstList)
{
  /* Checks */
  orxASSERT(_pstList != orxNULL);

  return(_pstList->pstLast);
}

/** Gets a list counter. */
extern orxINLINE  orxU32              orxLinkList_GetCounter(orxLINKLIST *_pstList)
{
  /* Checks */
  orxASSERT(_pstList != orxNULL);

  return(_pstList->u32Counter);
}


#endif /* _orxLINKLIST_H_ */
