/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2013 Orx-Project
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source
 *    distribution.
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
 * @section linklist Link List - How to
 * This module provides an easy and powerful interface for manipulating linked lists.
 *
 * @subsection linklist_datadefine Data definition
 * Using this data structure as an example:
 * @code
 * typedef struct __orxFOO_t
 * {
 *   orxU32 u32Data;        Data
 * } orxFOO;
 * @endcode
 *
 * @subsection linklist_dataalloc Data without link
 * Creating a bank to allocate memory storage:
 * @code
 * orxBANK *pstBank = orxBank_Create(10, sizeof(orxFOO), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
 * @endcode
 * You can then instantiate it this way:
 * @code
 * orxFOO *pstNode = (orxFOO *)orxBank_Allocate(pstBank);
 * pstNode->u32Data = 205;
 * @endcode
 * Having this basic behavior, you can add list linking to it.
 * @subsection linklist_realalloc Linked list item definition
 * To do so, you need to include in your structure an orxLINKLIST_NODE member as *FIRST MEMBER*:
 * @code
 * typedef struct __orxFOO_t
 * {
 *  orxLINKLIST_NODE stNode;
 *  orxU32 u32Data;
 * } orxFOO;
 * @endcode
 * @subsection linklist_realuse Use of link list
 * Your data structure can now be linked in lists:
 * @code
 * orxLINKLIST stList;
 * orxLinkList_AddEnd(&stList, (orxLINKLIST_NODE *)pstNode);
 * @endcode
 * @note As the first member of your data structure is a linked list node, you can cast your structure to orxLINKLIST_NODE and reciprocally.
 */


#ifndef _orxLINKLIST_H_
#define _orxLINKLIST_H_


#include "orxInclude.h"

#include "debug/orxDebug.h"


/** Node list structure
 */
typedef struct __orxLINKLIST_NODE_t
{
  struct __orxLINKLIST_NODE_t *pstNext;         /**< Next node pointer : 4 */
  struct __orxLINKLIST_NODE_t *pstPrevious;     /**< Previous node pointer : 8 */
  struct __orxLINKLIST_t      *pstList;         /**< Associated list pointer : 12 */

} orxLINKLIST_NODE;

/** List structure
 */
typedef struct __orxLINKLIST_t
{
  orxLINKLIST_NODE *pstFirst;                   /**< First node pointer : 4 */
  orxLINKLIST_NODE *pstLast;                    /**< Last node pointer : 8 */
  orxU32            u32Counter;                 /**< Node counter : 12 */

} orxLINKLIST;


/** Cleans a linklist
 * @param[in]   _pstList                        Concerned list
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxLinkList_Clean(orxLINKLIST *_pstList);


/** Adds a node at the start of a list
 * @param[in]   _pstList                        Concerned list
 * @param[in]   _pstNode                        Node to add
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxLinkList_AddStart(orxLINKLIST *_pstList, orxLINKLIST_NODE *_pstNode);

/** Adds a node at the end of a list
 * @param[in]   _pstList                        Concerned list
 * @param[in]   _pstNode                        Node to add
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxLinkList_AddEnd(orxLINKLIST *_pstList, orxLINKLIST_NODE *_pstNode);

/** Adds a node before another one
 * @param[in]   _pstRefNode                     Reference node (add before this one)
 * @param[in]   _pstNode                        Node to add
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxLinkList_AddBefore(orxLINKLIST_NODE *_pstRefNode, orxLINKLIST_NODE *_pstNode);

/** Adds a node after another one
 * @param[in]   _pstRefNode                     Reference node (add after this one)
 * @param[in]   _pstNode                        Node to add
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxLinkList_AddAfter(orxLINKLIST_NODE *_pstRefNode, orxLINKLIST_NODE *_pstNode);

/** Removes a node from its list
 * @param[in]   _pstNode                        Concerned node
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxLinkList_Remove(orxLINKLIST_NODE *_pstNode);


/* *** LinkList inlined accessors *** */


/** Gets a node list
 * @param[in]   _pstNode                        Concerned node
 * @return orxLINKLIST / orxNULL
 */
static orxINLINE orxLINKLIST *                  orxLinkList_GetList(const orxLINKLIST_NODE *_pstNode)
{
  /* Checks */
  orxASSERT(_pstNode != orxNULL);

  /* Returns it */
  return(_pstNode->pstList);
}

/** Gets previous node in list
 * @param[in]   _pstNode                        Concerned node
 * @return orxLINKLIST_NODE / orxNULL
 */
static orxINLINE orxLINKLIST_NODE *             orxLinkList_GetPrevious(const orxLINKLIST_NODE *_pstNode)
{
  /* Checks */
  orxASSERT(_pstNode != orxNULL);

  /* Returns it */
  return((_pstNode->pstList != orxNULL) ? _pstNode->pstPrevious : (orxLINKLIST_NODE *)orxNULL);
}

/** Gets next node in list
 * @param[in]   _pstNode                        Concerned node
 * @return orxLINKLIST_NODE / orxNULL
 */
static orxINLINE orxLINKLIST_NODE *             orxLinkList_GetNext(const orxLINKLIST_NODE *_pstNode)
{
  /* Checks */
  orxASSERT(_pstNode != orxNULL);

  /* Returns it */
  return((_pstNode->pstList != orxNULL) ? _pstNode->pstNext : (orxLINKLIST_NODE *)orxNULL);
}


/** Gets a list first node
 * @param[in]   _pstList                        Concerned list
 * @return orxLINKLIST_NODE / orxNULL
 */
static orxINLINE orxLINKLIST_NODE *             orxLinkList_GetFirst(const orxLINKLIST *_pstList)
{
  /* Checks */
  orxASSERT(_pstList != orxNULL);

  /* Returns it */
  return(_pstList->pstFirst);
}

/** Gets a list last node
 * @param[in]   _pstList                        Concerned list
 * @return orxLINKLIST_NODE / orxNULL
 */
static orxINLINE orxLINKLIST_NODE *             orxLinkList_GetLast(const orxLINKLIST *_pstList)
{
  /* Checks */
  orxASSERT(_pstList != orxNULL);

  /* Returns it */
  return(_pstList->pstLast);
}

/** Gets a list counter
 * @param[in]   _pstList                        Concerned list
 * @return Number of nodes in list
 */
static orxINLINE orxU32                         orxLinkList_GetCounter(const orxLINKLIST *_pstList)
{
  /* Checks */
  orxASSERT(_pstList != orxNULL);

  /* Returns it */
  return(_pstList->u32Counter);
}

#endif /* _orxLINKLIST_H_ */

/** @} */
