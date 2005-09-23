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
 orxTree_PrivateRemove
 Removes a node from its tree.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxFASTCALL orxTree_PrivateRemove(orxTREE_NODE *_pstNode, orxBOOL _bKeepRef)
{
  orxREGISTER orxTREE *pstTree;
  orxREGISTER orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(_pstNode != orxNULL);

  /* Gets tree */
  pstTree = _pstNode->pstTree;

  /* Keep heirs refs? */
  if(_bKeepRef != orxFALSE)
  {
    /* Isn't root? */
    if(pstTree->pstRoot != _pstNode)
    {
      /* Was firt child? */
      if(_pstNode->pstParent->pstChild == _pstNode)
      {
        /* Udpates parent */
        _pstNode->pstParent->pstChild = _pstNode->pstSibling;
      }
      else
      {
        orxREGISTER orxTREE_NODE *pstChild;

        /* Finds left sibling */
        for(pstChild = _pstNode->pstParent->pstChild;
            pstChild->pstSibling != _pstNode;
            pstChild = pstChild->pstSibling);
    
        /* Updates it */
        pstChild->pstSibling = _pstNode->pstSibling;
      }

      /* Updates node */
      _pstNode->pstParent   = orxNULL;
      _pstNode->pstSibling  = orxNULL;
    }
    else
    {
      /* !!! MSG !!! */

      /* Can't process */
      eResult = orxSTATUS_FAILED;
    }
  }
  /* Remove completely from tree */
  else
  {
    /* Is root? */
    if(pstTree->pstRoot == _pstNode)
    {
      /* Is the last node in tree? */
      if(pstTree->u32Counter == 1)
      {
        /* Removes it */
        pstTree->pstRoot = orxNULL;
        
        /* Updates node */
        orxMemory_Set(_pstNode, 0, sizeof(orxTREE_NODE));

        /* Updates counter */
        pstTree->u32Counter = 0;
      }
      else
      {
        /* !!! MSG !!! */

        /* Can't process */
        eResult = orxSTATUS_FAILED;
      }
    }
    else
    {
      orxREGISTER orxTREE_NODE *pstNewChild;

      /* Had child? */
      if(_pstNode->pstChild != orxNULL)
      {
        orxREGISTER orxTREE_NODE *pstChild;

        /* Updates all children but last */
        for(pstChild = _pstNode->pstChild;
            pstChild->pstSibling != orxNULL;
            pstChild = pstChild->pstSibling)
        {
          /* Updates it */
          pstChild->pstParent = _pstNode->pstParent;
        }

        /* Updates last child */
        pstChild->pstParent   = _pstNode->pstParent;
        pstChild->pstSibling  = _pstNode->pstSibling;

        /* New parent's child is previous first child */
        pstNewChild = _pstNode->pstChild;
      }
      /* No child */
      else
      {
        /* New parent's child is previous sibling */
        pstNewChild = _pstNode->pstSibling;
      }

      /* Was first child? */
      if(_pstNode->pstParent->pstChild == _pstNode)
      {
        /* Updates parent */
        _pstNode->pstParent->pstChild = pstNewChild;
      }
      /* Not first child */
      else
      {
        orxREGISTER orxTREE_NODE *pstChild;

        /* Find left sibling */
        for(pstChild = _pstNode->pstParent->pstChild;
            pstChild->pstSibling != _pstNode;
            pstChild = pstChild->pstSibling);

        /* Updates it */
        pstChild->pstSibling = pstNewChild;
      }

      /* Updates node */
      orxMemory_Set(_pstNode, 0, sizeof(orxTREE_NODE));

      /* Updates counter */
      pstTree->u32Counter--;      
    }
  }

  /* Done! */
  return eResult;
}  


/***************************************************************************
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 orxTree_Setup
 Tree module setup.

 returns: nothing
 ***************************************************************************/
orxVOID orxTree_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_TREE, orxMODULE_ID_MEMORY);

  return;
}

/***************************************************************************
 orxTree_Init
 Inits the tree system.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxTree_Init()
{
  orxSTATUS eResult;
  
  eResult = orxSTATUS_FAILED;
  
  /* Already Initialized? */
  if(!(sstTree.u32Flags & orxTREE_KU32_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Set(&sstTree, 0, sizeof(orxTREE_STATIC));
  
    /* Inits ID Flags */
    sstTree.u32Flags = orxTREE_KU32_FLAG_READY;
    
    /* Success */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* !!! MSG !!! */

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxTree_Exit
 Exits from the tree system.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxTree_Exit()
{
  /* Initialized? */
  if(sstTree.u32Flags & orxTREE_KU32_FLAG_READY)
  {
    /* Updates flags */
    sstTree.u32Flags &= ~orxTREE_KU32_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }
  
  return;
}

/***************************************************************************
 orxTree_Clean
 Cleans a tree.

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

  /* Isn't already in a tree? */
  if(_pstNode->pstTree == orxNULL)
  {
    /* Has a root? */
    if(_pstTree->pstRoot != orxNULL)
    {
      /* Adds as parent of the current root */
      eResult = orxTree_AddParent(_pstTree->pstRoot, _pstNode);
    }
    else
    {
      /* Checks there are no node right now */
      orxASSERT(_pstTree->u32Counter == 0);

      /* Stores it as root */
      _pstTree->pstRoot = _pstNode;

      /* Cleans it */
      orxMemory_Set(_pstNode, 0, sizeof(orxTREE_NODE));

      /* Stores tree pointer */
      _pstNode->pstTree = _pstTree;

      /* Updates counter */
      _pstTree->u32Counter++;
    }
  }
  else
  {
    /* !!! MSG !!! */

    /* Already in a tree */
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

  /* Isn't already in a tree? */
  if(_pstNode->pstTree == orxNULL)
  {
    /* Gets tree */
    pstTree = _pstRefNode->pstTree;
  
    /* Valid? */
    if(pstTree != orxNULL)
    {
      /* Adds it in the tree */
      _pstNode->pstChild    = _pstRefNode;
      _pstNode->pstParent   = _pstRefNode->pstParent;
      _pstNode->pstTree     = pstTree;
      _pstNode->pstSibling  = _pstRefNode->pstSibling;

      /* Updates parent? */
      if(_pstRefNode->pstParent != orxNULL)
      {
        /* Was first child? */
        if(_pstRefNode->pstParent->pstChild == _pstRefNode)
        {
            /* Updates parent */
            _pstRefNode->pstParent->pstChild = _pstNode;
        }
        else
        {
          orxREGISTER orxTREE_NODE *pstChild;

          /* Finds left sibling */
          for(pstChild = _pstRefNode->pstParent->pstChild;
              pstChild->pstSibling != _pstRefNode;
              pstChild = pstChild->pstSibling);

          /* Updates sibling */
          pstChild->pstSibling = _pstNode;
        }
      }
      else
      {
        /* Checks node was the root */
        orxASSERT(pstTree->pstRoot == _pstRefNode);

        /* Updates new root */
        pstTree->pstRoot = _pstNode;
      }

      /* Updates ref node */
      _pstRefNode->pstParent  = _pstNode;
      _pstRefNode->pstSibling = orxNULL;

      /* Updates counter */
      pstTree->u32Counter++;
    }
    else
    {
      /* !!! MSG !!! */
  
      /* No tree found */
      eResult = orxSTATUS_FAILED;
    }
  }
  else
  {
    /* !!! MSG !!! */
    
    /* Already in a tree */
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

  /* Isn't already in a tree? */
  if(_pstNode->pstTree == orxNULL)
  {
    /* Gets tree */
    pstTree = _pstRefNode->pstTree;

    /* Valid? */
    if(pstTree != orxNULL)
    {
      /* Adds it in the tree */
      _pstNode->pstParent   = _pstRefNode;
      _pstNode->pstSibling  = _pstRefNode->pstChild;
      _pstNode->pstTree     = pstTree;
      _pstNode->pstChild    = orxNULL;

      /* Updates ref node */
      _pstRefNode->pstChild = _pstNode;

      /* Updates counter */
      pstTree->u32Counter++;
    }
    else
    {
      /* !!! MSG !!! */
  
      /* No tree found */
      eResult = orxSTATUS_FAILED;
    }
  }
  else
  {
    /* !!! MSG !!! */
    
    /* Already in a tree */
    eResult = orxSTATUS_FAILED;
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxTree_MoveAsChild
 Moves as a child of another node of the same tree.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxTree_MoveAsChild(orxTREE_NODE *_pstRefNode, orxTREE_NODE *_pstNode)
{
  orxREGISTER orxSTATUS eResult = orxSTATUS_SUCCESS;
  orxREGISTER orxTREE *pstTree;

  /* Checks */
  orxASSERT(sstTree.u32Flags & orxTREE_KU32_FLAG_READY);
  orxASSERT(_pstRefNode != orxNULL);
  orxASSERT(_pstNode != orxNULL);

  /* Gets tree */
  pstTree = _pstRefNode->pstTree;

  /* Is already in the tree? */
  if(_pstNode->pstTree == pstTree)
  {
    orxREGISTER orxTREE_NODE *pstTest;

    /* Checks for preventing tree from turning into graph */
    for(pstTest = _pstRefNode;
        (pstTest != orxNULL) && (pstTest != _pstNode);
        pstTest = pstTest->pstParent);

    /* No graph cycle found? */
    if(pstTest == orxNULL)
    {
      /* Removes it from its place */
      eResult = orxTree_PrivateRemove(_pstNode, orxTRUE);

      /* Success? */
      if(eResult == orxSTATUS_SUCCESS)
      {
        /* Adds it at new place */
        _pstNode->pstParent   = _pstRefNode;
        _pstNode->pstSibling  = _pstRefNode->pstChild;

        /* Updates ref node */
        _pstRefNode->pstChild = _pstNode;
      }
      else
      {
        /* !!! MSG !!! */
      }
    }
    else
    {
      /* !!! MSG !!! */

      /* Can't process */
      eResult = orxSTATUS_FAILED;
    }
  }
  else
  {
    /* !!! MSG !!! */
    
    /* Not already in the tree */
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

  /* Is in a tree? */
  if(pstTree != orxNULL)
  {
    /* Checks tree is non empty */
    orxASSERT(pstTree->u32Counter != 0);

    /* Removes it */
    eResult = orxTree_PrivateRemove(_pstNode, orxFALSE);
  }
  else
  {
    /* !!! MSG !!! */

    /* Not in a tree */
    eResult = orxSTATUS_FAILED;
  }

  /* Done! */
  return eResult;
}
