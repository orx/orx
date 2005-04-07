/***************************************************************************
 orxTree.c
 Tree module
 
 begin                : 07/04/2005
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


#include "utils/orxTree.h"

#include "memory/orxMemory.h"


/*
 * Platform independant defines
 */

#define orxTREE_KU32_FLAG_NONE                    0x00000000
#define orxTREE_KU32_FLAG_READY                   0x00000001


/*
 * Static structure
 */
typedef struct __orxTREE_STATIC_t
{
  /* Control flags */
  orxU32 u32Flags;

} orxTREE_STATIC;

/*
 * Static data
 */
orxSTATIC orxTREE_STATIC sstTree;


/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/



/***************************************************************************
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 orxTree_Init
 Inits the link list system.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxTree_Init()
{
  /* Already Initialized? */
  if(sstTree.u32Flags & orxTREE_KU32_FLAG_READY)
  {
    /* !!! MSG !!! */

    return orxSTATUS_FAILED;
  }

  /* Cleans static controller */
  orxMemory_Set(&sstTree, 0, sizeof(orxTREE_STATIC));

  /* Inits ID Flags */
  sstTree.u32Flags = orxTREE_KU32_FLAG_READY;

  /* Done! */
  return orxSTATUS_SUCCESS;
}

/***************************************************************************
 orxTree_Exit
 Exits from the link list system.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxTree_Exit()
{
  /* Not initialized? */
  if((sstTree.u32Flags & orxTREE_KU32_FLAG_READY) == orxTREE_KU32_FLAG_NONE)
  {
    /* !!! MSG !!! */

    return;
  }

  /* Updates flags */
  sstTree.u32Flags &= ~orxTREE_KU32_FLAG_READY;

  return;
}

/***************************************************************************
 orxTree_Clean
 Cleans a link list.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxTree_Clean(orxTREE *_pstTree)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;
    
  /* Checks */
  orxASSERT(sstTree.u32Flags & orxTREE_KU32_FLAG_READY);
  orxASSERT(_pstTree != orxNULL);

  /* Non empty? */
  while((_pstTree->u32Counter != 0) && (eResult == orxSTATUS_SUCCESS))
  {
    /* Removes root node */
    eResult = orxTree_Remove(_pstTree->pstRoot);
  }

  /* Successful? */
  if(eResult == orxSTATUS_SUCCESS)
  {
    /* Cleans tree */
    orxMemory_Set(_pstTree, 0, sizeof(orxTREE));
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxTree_AddRoot
 Adds a new node at the root of the corresponding tree.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxTree_AddRoot(orxTREE *_pstTree, orxTREE_NODE *_pstNode)
{
  orxREGISTER orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstTree.u32Flags & orxTREE_KU32_FLAG_READY);
  orxASSERT(_pstTree != orxNULL);
  orxASSERT(_pstNode != orxNULL);

  /* Not already used in a list? */
  if(_pstNode->pstTree == orxNULL)
  {
  }
  else
  {
    /* !!! MSG !!! */

    /* Not linked */
    eResult = orxSTATUS_FAILED;
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxTree_AddParent
 Adds a new node as parent of another one.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxTree_AddParent(orxTREE_NODE *_pstRefNode, orxTREE_NODE *_pstNode)
{
  orxREGISTER orxSTATUS eResult = orxSTATUS_SUCCESS;
  orxREGISTER orxTREE *pstTree;

  /* Checks */
  orxASSERT(sstTree.u32Flags & orxTREE_KU32_FLAG_READY);
  orxASSERT(_pstRefNode != orxNULL);
  orxASSERT(_pstNode != orxNULL);

  /* Isn't already linked? */
  if(_pstNode->pstTree == orxNULL)
  {
    /* Gets tree */
    pstTree = _pstRefNode->pstTree;
  
    /* Valid? */
    if(pstTree != orxNULL)
    {
        /* !!! TODO !!! */
    }
    else
    {
      /* !!! MSG !!! */
  
      /* No list found */
      eResult = orxSTATUS_FAILED;
    }
  }
  else
  {
    /* !!! MSG !!! */
    
    /* Already linked */
    eResult = orxSTATUS_FAILED;
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxTree_AddChild
 Adds a new node as a child of another one.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxTree_AddChild(orxTREE_NODE *_pstRefNode, orxTREE_NODE *_pstNode)
{
  orxREGISTER orxSTATUS eResult = orxSTATUS_SUCCESS;
  orxREGISTER orxTREE *pstTree;

  /* Checks */
  orxASSERT(sstTree.u32Flags & orxTREE_KU32_FLAG_READY);
  orxASSERT(_pstRefNode != orxNULL);
  orxASSERT(_pstNode != orxNULL);

  /* Isn't already linked? */
  if(_pstNode->pstTree == orxNULL)
  {
    /* Gets tree */
    pstTree = _pstRefNode->pstTree;
  
    /* Valid? */
    if(pstTree != orxNULL)
    {
        /* !!! TODO !!! */
    }
    else
    {
      /* !!! MSG !!! */
  
      /* No list found */
      eResult = orxSTATUS_FAILED;
    }
  }
  else
  {
    /* !!! MSG !!! */
    
    /* Already linked */
    eResult = orxSTATUS_FAILED;
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxTree_Remove
 Removes a node from its tree.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxTree_Remove(orxTREE_NODE *_pstNode)
{
  orxREGISTER orxTREE *pstTree;
  orxREGISTER orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstTree.u32Flags & orxTREE_KU32_FLAG_READY);
  orxASSERT(_pstNode != orxNULL);

  /* Gets tree */
  pstTree = _pstNode->pstTree;

  /* Valid? */
  if(pstTree != orxNULL)
  {
    /* Checks tree is non empty */
    orxASSERT(pstTree->u32Counter != 0);

    /* !!! TODO !!! */
  }
  else
  {
    /* !!! MSG !!! */
    
    /* Failed */
    eResult = orxSTATUS_FAILED;
  }

  /* Done! */
  return eResult;
}
