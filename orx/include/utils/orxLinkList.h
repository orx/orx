/* Orx - Portable Game Engine
 *
 * Orx is the legal property of its developers, whose names
 * are listed in the COPYRIGHT file distributed 
 * with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * @file orxLinkList.h
 * @date 06/04/2005
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxLinkList
 * 
 * Linklist module
 * Module that handles linklists
 *
 * @{
 *
 * @section linklist Link List - How to.
 * This module provide an easy and powerfull interface to manipulate link lists.
 *
 * @subsection linklist_datadefine Data definition.
 * You can define a data to store :
 * @code
 * typedef struct __orxFOO_t
 * {
 *   orxU32 u32Data;        Data to store.
 * } orxFOO;
 * @endcode
 *
 * @subsection linklist_dataalloc Data without link.
 * You can define an allocation place to store data:
 * @code
 * orxBANK *pstBank = orxBank_Create(10, sizeof(orxFOO), 0, orxMEMORY_TYPE_MAIN);
 * @endcode
 * And you can allocate a memory cel :
 * @code
 * orxTRUC *pstNode = (orxFOO *)orxBank_Allocate(pstBank);
 * pstNode->u32Data = 205;
 * @endcode
 * So you can allocate an manipulate data. But no link list manipulation is define.
 * @subsection linklist_realalloc Real link list item definition.
 * In order to use link list functions, you can (must) define your structure with an orxLINKLIST_NODE member at its begining :
 * @code
 * typedef struct __orxFOO_t
 * {
 *  orxLINKLIST_NODE stNode;
 *  orxU32 u32Data;
 * } orxFOO;
 * @endcode
 * @subsection linklist_realuse Use of link list.
 * Now you can use a list with your own list items :
 * @code
 * orxLINKLIST stList;
 * orxLinkList_AddEnd(&stList, (orxLINKLIST_NODE *)pstNode);
 * @endcode
 * @note As the first member of your item structure, you can cast your structure to orxLINKLIST_NODE and reciprocally.
 */


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
typedef struct __orxLINKLIST_t                  orxLINKLIST;
typedef struct __orxLINKLIST_NODE_t             orxLINKLIST_NODE;


/** LinkList module setup. */
extern orxDLLAPI orxVOID                        orxLinkList_Setup();
/** Inits the object system. */
extern orxDLLAPI orxSTATUS                      orxLinkList_Init();
/** Ends the object system. */
extern orxDLLAPI orxVOID                        orxLinkList_Exit();

/** Cleans a link list. */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxLinkList_Clean(orxLINKLIST *_pstList);

/** Adds a node at the start of the list. */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxLinkList_AddStart(orxLINKLIST *_pstList, orxLINKLIST_NODE *_pstNode);
/** Adds a node at the end of the list. */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxLinkList_AddEnd(orxLINKLIST *_pstList, orxLINKLIST_NODE *_pstNode);
/** Adds a node before another one. */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxLinkList_AddBefore(orxLINKLIST_NODE *_pstRefNode, orxLINKLIST_NODE *_pstNode);
/** Adds a node after another one. */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxLinkList_AddAfter(orxLINKLIST_NODE *_pstRefNode, orxLINKLIST_NODE *_pstNode);

/** Removes a node from its list. */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxLinkList_Remove(orxLINKLIST_NODE *_pstNode);


/* *** LinkList inlined accessors *** */


/** Gets a node list. */
orxSTATIC orxINLINE orxLINKLIST                *orxLinkList_GetList(orxCONST orxLINKLIST_NODE *_pstNode)
{
  /* Checks */
  orxASSERT(_pstNode != orxNULL);

  /* Returns it */
  return(_pstNode->pstList);
}

/** Gets a node previous. */
orxSTATIC orxINLINE orxLINKLIST_NODE           *orxLinkList_GetPrevious(orxCONST orxLINKLIST_NODE *_pstNode)
{
  /* Checks */
  orxASSERT(_pstNode != orxNULL);

  /* Returns it */
  return((_pstNode->pstList != orxNULL) ? _pstNode->pstPrevious : orxNULL);
}

/** Gets a node next. */
orxSTATIC orxINLINE orxLINKLIST_NODE           *orxLinkList_GetNext(orxCONST orxLINKLIST_NODE *_pstNode)
{
  /* Checks */
  orxASSERT(_pstNode != orxNULL);

  /* Returns it */
  return((_pstNode->pstList != orxNULL) ? _pstNode->pstNext : orxNULL);
}


/** Gets a list first node. */
orxSTATIC orxINLINE orxLINKLIST_NODE           *orxLinkList_GetFirst(orxCONST orxLINKLIST *_pstList)
{
  /* Checks */
  orxASSERT(_pstList != orxNULL);

  /* Returns it */
  return(_pstList->pstFirst);
}

/** Gets a list last node. */
orxSTATIC orxINLINE orxLINKLIST_NODE           *orxLinkList_GetLast(orxCONST orxLINKLIST *_pstList)
{
  /* Checks */
  orxASSERT(_pstList != orxNULL);

  /* Returns it */
  return(_pstList->pstLast);
}

/** Gets a list counter. */
orxSTATIC orxINLINE orxU32                      orxLinkList_GetCounter(orxCONST orxLINKLIST *_pstList)
{
  /* Checks */
  orxASSERT(_pstList != orxNULL);

  /* Returns it */
  return(_pstList->u32Counter);
}

#endif /* _orxLINKLIST_H_ */

/** @} */
