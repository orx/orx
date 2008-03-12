/** 
 * \file orxTree.h
 * 
 * Tree Module.
 * Allows to handle trees.
 * 
 * \todo
 * Add new features/optims at need.
 */


/***************************************************************************
 orxTree.h
 Tree module
 
 begin                : 06/04/2005
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/


#ifndef _orxTREE_H_
#define _orxTREE_H_


#include "orxInclude.h"

#include "debug/orxDebug.h"


/*
 * Tree Node structure
 */
struct __orxTREE_NODE_t
{
  /* Tree pointers : 12 */
  struct __orxTREE_NODE_t *pstParent;
  struct __orxTREE_NODE_t *pstChild;
  struct __orxTREE_NODE_t *pstSibling;
  
  /* Associated tree : 16 */
  struct __orxTREE_t *pstTree;
};

/*
 * Tree structure
 */
struct __orxTREE_t
{
  /* Tree cell pointer : 4 */
  struct __orxTREE_NODE_t *pstRoot;

  /* Counter : 8 */
  orxU32 u32Counter;
};

/* Tree types */
typedef struct __orxTREE_t                  orxTREE;
typedef struct __orxTREE_NODE_t             orxTREE_NODE;


/** Tree module setup. */
extern orxDLLAPI orxVOID                    orxTree_Setup();
/** Inits the object system. */
extern orxDLLAPI orxSTATUS                  orxTree_Init();
/** Ends the object system. */
extern orxDLLAPI orxVOID                    orxTree_Exit();

/** Cleans a tree. */
extern orxDLLAPI orxSTATUS                  orxTree_Clean(orxTREE *_pstTree);

/** Removes a node from its tree. */
extern orxDLLAPI orxSTATUS                  orxTree_Remove(orxTREE_NODE *_pstNode);

/** Adds a node at the root of the tree. */
extern orxDLLAPI orxSTATUS                  orxTree_AddRoot(orxTREE *_pstTree, orxTREE_NODE *_pstNode);
/** Adds a node as a parent of another one. */
extern orxDLLAPI orxSTATUS                  orxTree_AddParent(orxTREE_NODE *_pstRefNode, orxTREE_NODE *_pstNode);
/** Adds a node as a child of another one. */
extern orxDLLAPI orxSTATUS                  orxTree_AddChild(orxTREE_NODE *_pstRefNode, orxTREE_NODE *_pstNode);

/** Removes a node from its tree. */
extern orxDLLAPI orxSTATUS                  orxTree_Remove(orxTREE_NODE *_pstNode);

/** Moves as a child of another node of the same tree. */
extern orxDLLAPI orxSTATUS                  orxTree_MoveAsChild(orxTREE_NODE *_pstRefNode, orxTREE_NODE *_pstNode);

/* *** Tree inlined accessors *** */


/** Gets a node tree. */
orxSTATIC orxINLINE orxTREE                *orxTree_GetTree(orxCONST orxTREE_NODE *_pstNode)
{
  /* Checks */
  orxASSERT(_pstNode != orxNULL);

  /* Returns it */
  return(_pstNode->pstTree);
}

/** Gets a node parent. */
orxSTATIC orxINLINE orxTREE_NODE           *orxTree_GetParent(orxCONST orxTREE_NODE *_pstNode)
{
  /* Checks */
  orxASSERT(_pstNode != orxNULL);

  /* Returns it */
  return((_pstNode->pstTree != orxNULL) ? _pstNode->pstParent : orxNULL);
}

/** Gets a node first child. */
orxSTATIC orxINLINE orxTREE_NODE           *orxTree_GetChild(orxCONST orxTREE_NODE *_pstNode)
{
  /* Checks */
  orxASSERT(_pstNode != orxNULL);

  /* Returns it */
  return((_pstNode->pstTree != orxNULL) ? _pstNode->pstChild : orxNULL);
}

/** Gets a node sibling. */
orxSTATIC orxINLINE orxTREE_NODE           *orxTree_GetSibling(orxCONST orxTREE_NODE *_pstNode)
{
  /* Checks */
  orxASSERT(_pstNode != orxNULL);

  /* Returns it */
  return((_pstNode->pstTree != orxNULL) ? _pstNode->pstSibling : orxNULL);
}


/** Gets a tree root. */
orxSTATIC orxINLINE orxTREE_NODE           *orxTree_GetRoot(orxCONST orxTREE *_pstTree)
{
  /* Checks */
  orxASSERT(_pstTree != orxNULL);

  /* Returns it */
  return(_pstTree->pstRoot);
}

/** Gets a tree counter. */
orxSTATIC orxINLINE orxU32                  orxTree_GetCounter(orxCONST orxTREE *_pstTree)
{
  /* Checks */
  orxASSERT(_pstTree != orxNULL);

  /* Returns it */
  return(_pstTree->u32Counter);
}


#endif /* _orxTREE_H_ */
