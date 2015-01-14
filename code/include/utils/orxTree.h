/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2015 Orx-Project
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
 * @file orxTree.h
 * @date 06/04/2005
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxTree
 * 
 * Tree module
 * Module that handles trees
 *
 * @{
 */


#ifndef _orxTREE_H_
#define _orxTREE_H_


#include "orxInclude.h"

#include "debug/orxDebug.h"


/** Tree node structure
 */
typedef struct __orxTREE_NODE_t
{
  struct __orxTREE_NODE_t *pstParent;           /**< Parent node pointer : 4/8 */
  struct __orxTREE_NODE_t *pstChild;            /**< First child node pointer : 8/16 */
  struct __orxTREE_NODE_t *pstSibling;          /**< Next sibling node pointer : 12/24 */
  struct __orxTREE_t      *pstTree;             /**< Associated tree pointer : 16/32 */

} orxTREE_NODE;

/** Tree structure
 */
typedef struct __orxTREE_t
{
  orxTREE_NODE *pstRoot;                        /**< Root node pointer : 4/8 */
  orxU32        u32Counter;                     /**< Node counter : 8/12 */

} orxTREE;


/** Cleans a tree
 * @param[in]   _pstTree                        Concerned tree
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxTree_Clean(orxTREE *_pstTree);

/** Adds a node at the root of a tree
 * @param[in]   _pstTree                        Concerned tree
 * @param[in]   _pstNode                        Node to add
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxTree_AddRoot(orxTREE *_pstTree, orxTREE_NODE *_pstNode);

/** Adds a node as a parent of another one
 * @param[in]   _pstRefNode                     Reference node (add as a parent of this one)
 * @param[in]   _pstNode                        Node to add
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxTree_AddParent(orxTREE_NODE *_pstRefNode, orxTREE_NODE *_pstNode);

/** Adds a node as a sibling of another one
 * @param[in]   _pstRefNode                     Reference node (add as a sibling of this one)
 * @param[in]   _pstNode                        Node to add
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxTree_AddSibling(orxTREE_NODE *_pstRefNode, orxTREE_NODE *_pstNode);

/** Adds a node as a child of another one
 * @param[in]   _pstRefNode                     Reference node (add as a child of this one)
 * @param[in]   _pstNode                        Node to add
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxTree_AddChild(orxTREE_NODE *_pstRefNode, orxTREE_NODE *_pstNode);

/** Moves a node as a child of another one of the same tree
 * @param[in]   _pstRefNode                     Reference node (move as a child of this one)
 * @param[in]   _pstNode                        Node to move
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxTree_MoveAsChild(orxTREE_NODE *_pstRefNode, orxTREE_NODE *_pstNode);

/** Removes a node from its tree
 * @param[in]   _pstNode                        Concerned node
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxTree_Remove(orxTREE_NODE *_pstNode);


/* *** Tree inlined accessors *** */


/** Gets a node tree
 * @param[in]   _pstNode                        Concerned node
 * @return orxTREE / orxNULL
 */
static orxINLINE orxTREE *                      orxTree_GetTree(const orxTREE_NODE *_pstNode)
{
  /* Checks */
  orxASSERT(_pstNode != orxNULL);

  /* Returns it */
  return(_pstNode->pstTree);
}

/** Gets parent node
 * @param[in]   _pstNode                        Concerned node
 * @return orxTREE_NODE / orxNULL
 */
static orxINLINE orxTREE_NODE *                 orxTree_GetParent(const orxTREE_NODE *_pstNode)
{
  /* Checks */
  orxASSERT(_pstNode != orxNULL);

  /* Returns it */
  return((_pstNode->pstTree != orxNULL) ? _pstNode->pstParent : (orxTREE_NODE *)orxNULL);
}

/** Gets first child node
 * @param[in]   _pstNode                        Concerned node
 * @return orxTREE_NODE / orxNULL
 */
static orxINLINE orxTREE_NODE *                 orxTree_GetChild(const orxTREE_NODE *_pstNode)
{
  /* Checks */
  orxASSERT(_pstNode != orxNULL);

  /* Returns it */
  return((_pstNode->pstTree != orxNULL) ? _pstNode->pstChild : (orxTREE_NODE *)orxNULL);
}

/** Gets sibling node
 * @param[in]   _pstNode                        Concerned node
 * @return orxTREE_NODE / orxNULL
 */
static orxINLINE orxTREE_NODE *                 orxTree_GetSibling(const orxTREE_NODE *_pstNode)
{
  /* Checks */
  orxASSERT(_pstNode != orxNULL);

  /* Returns it */
  return((_pstNode->pstTree != orxNULL) ? _pstNode->pstSibling : (orxTREE_NODE *)orxNULL);
}


/** Gets a tree root
 * @param[in]   _pstTree                        Concerned tree
 * @return orxTREE_NODE / orxNULL
 */
static orxINLINE orxTREE_NODE *                 orxTree_GetRoot(const orxTREE *_pstTree)
{
  /* Checks */
  orxASSERT(_pstTree != orxNULL);

  /* Returns it */
  return(_pstTree->pstRoot);
}

/** Gets a tree counter
 * @param[in]   _pstTree                        Concerned tree
 * @return Number of nodes in tree
 */
static orxINLINE orxU32                         orxTree_GetCounter(const orxTREE *_pstTree)
{
  /* Checks */
  orxASSERT(_pstTree != orxNULL);

  /* Returns it */
  return(_pstTree->u32Counter);
}

#endif /* _orxTREE_H_ */

/** @} */
