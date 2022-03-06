/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2022 Orx-Project
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
 * @file orxLinkList.c
 * @date 06/04/2005
 * @author iarwain@orx-project.org
 *
 */


#include "utils/orxLinkList.h"
#include "memory/orxMemory.h"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Cleans a link list
 * @param[in]   _pstList                        Concerned list
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxLinkList_Clean(orxLINKLIST *_pstList)
{
  /* Checks */
  orxASSERT(_pstList != orxNULL);

  /* Non empty? */
  if(_pstList->u32Count != 0)
  {
    orxLINKLIST_NODE *pstNode;

    /* Gets first node */
    pstNode = _pstList->pstFirst;

    /* Clean all nodes */
    while(pstNode != orxNULL)
    {
      orxLINKLIST_NODE *pstCurrent;

      /* Backups current node */
      pstCurrent = pstNode;

      /* Go to next node */
      pstNode = pstNode->pstNext;

      /* Cleans current node */
      orxMemory_Zero(pstCurrent, sizeof(orxLINKLIST_NODE));
    }
  }

  /* Cleans list */
  orxMemory_Zero(_pstList, sizeof(orxLINKLIST));

  /* Done! */
  return orxSTATUS_SUCCESS;
}

/** Adds a node at the start of a list
 * @param[in]   _pstList                        Concerned list
 * @param[in]   _pstNode                        Node to add
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxLinkList_AddStart(orxLINKLIST *_pstList, orxLINKLIST_NODE *_pstNode)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(_pstList != orxNULL);
  orxASSERT(_pstNode != orxNULL);

  /* Not already used in a list? */
  if(_pstNode->pstList == orxNULL)
  {
    /* Adds it at the start of the list */
    _pstNode->pstNext     = _pstList->pstFirst;
    _pstNode->pstPrevious = orxNULL;
    _pstNode->pstList     = _pstList;

    /* Updates old node if needed */
    if(_pstList->pstFirst != orxNULL)
    {
      _pstList->pstFirst->pstPrevious = _pstNode;
    }
    else
    {
      /* Updates last node */
      _pstList->pstLast   = _pstNode;
    }

    /* Stores node at the start of the list */
    _pstList->pstFirst    = _pstNode;

    /* Updates count */
    _pstList->u32Count++;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Node is already linked. Remove it before trying to add it to another.");

    /* Not linked */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Adds a node at the end of a list
 * @param[in]   _pstList                        Concerned list
 * @param[in]   _pstNode                        Node to add
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxLinkList_AddEnd(orxLINKLIST *_pstList, orxLINKLIST_NODE *_pstNode)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(_pstList != orxNULL);
  orxASSERT(_pstNode != orxNULL);

  /* Not already used in a list? */
  if(_pstNode->pstList == orxNULL)
  {
    /* Adds it at the end of the list */
    _pstNode->pstPrevious = _pstList->pstLast;
    _pstNode->pstNext     = orxNULL;
    _pstNode->pstList     = _pstList;

    /* Updates old node if needed */
    if(_pstList->pstLast != orxNULL)
    {
      _pstList->pstLast->pstNext = _pstNode;
    }
    else
    {
      /* Updates first node */
      _pstList->pstFirst  = _pstNode;
    }

    /* Stores node at the end of the list */
    _pstList->pstLast     = _pstNode;

    /* Updates count */
    _pstList->u32Count++;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Node is already linked. Remove it before trying to add it to another.");

    /* Not linked */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Adds a node before another one
 * @param[in]   _pstList                        Concerned list
 * @param[in]   _pstRefNode                     Reference node (add before this one)
 * @param[in]   _pstNode                        Node to add
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxLinkList_AddBefore(orxLINKLIST_NODE *_pstRefNode, orxLINKLIST_NODE *_pstNode)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;
  orxLINKLIST *pstList;

  /* Checks */
  orxASSERT(_pstRefNode != orxNULL);
  orxASSERT(_pstNode != orxNULL);

  /* Isn't already linked? */
  if(_pstNode->pstList == orxNULL)
  {
    /* Gets list */
    pstList = _pstRefNode->pstList;

    /* Valid? */
    if(pstList != orxNULL)
    {
      /* Adds it in the list */
      _pstNode->pstNext         = _pstRefNode;
      _pstNode->pstPrevious     = _pstRefNode->pstPrevious;
      _pstNode->pstList         = pstList;

      /* Updates previous? */
      if(_pstRefNode->pstPrevious != orxNULL)
      {
        /* Updates it */
        _pstRefNode->pstPrevious->pstNext = _pstNode;
      }
      else
      {
        /* Checks node was the first one */
        orxASSERT(pstList->pstFirst == _pstRefNode);

        /* Updates new first node */
        pstList->pstFirst = _pstNode;
      }

      /* Updates ref node */
      _pstRefNode->pstPrevious  = _pstNode;

      /* Updates count */
      pstList->u32Count++;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Target's list is not valid.");

      /* No list found */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Node is already linked. Remove it before trying to add it to another.");

    /* Already linked */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Adds a node after another one
 * @param[in]   _pstList                        Concerned list
 * @param[in]   _pstRefNode                     Reference node (add after this one)
 * @param[in]   _pstNode                        Node to add
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxLinkList_AddAfter(orxLINKLIST_NODE *_pstRefNode, orxLINKLIST_NODE *_pstNode)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;
  orxLINKLIST *pstList;

  /* Checks */
  orxASSERT(_pstRefNode != orxNULL);
  orxASSERT(_pstNode != orxNULL);

  /* Isn't already linked? */
  if(_pstNode->pstList == orxNULL)
  {
    /* Gets list */
    pstList = _pstRefNode->pstList;

    /* Valid? */
    if(pstList != orxNULL)
    {
      /* Adds it in the list */
      _pstNode->pstNext         = _pstRefNode->pstNext;
      _pstNode->pstPrevious     = _pstRefNode;
      _pstNode->pstList         = pstList;

      /* Updates next? */
      if(_pstRefNode->pstNext != orxNULL)
      {
        /* Updates it */
        _pstRefNode->pstNext->pstPrevious = _pstNode;
      }
      else
      {
        /* Checks node was the last one */
        orxASSERT(pstList->pstLast == _pstRefNode);

        /* Updates new last node */
        pstList->pstLast        = _pstNode;
      }

      /* Updates ref node */
      _pstRefNode->pstNext      = _pstNode;

      /* Updates count */
      pstList->u32Count++;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Target node's list is not valid.");

      /* No list found */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Node is already linked. Remove it before trying to add it to another.");

    /* Already linked */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Removes a node from its list
 * @param[in]   _pstNode                        Concerned node
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxLinkList_Remove(orxLINKLIST_NODE *_pstNode)
{
  orxLINKLIST *pstList;
  orxLINKLIST_NODE *pstPrevious, *pstNext;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(_pstNode != orxNULL);

  /* Gets list */
  pstList = _pstNode->pstList;

  /* Valid? */
  if(pstList != orxNULL)
  {
    /* Checks list is non empty */
    orxASSERT(pstList->u32Count != 0);

    /* Gets neighbours pointers */
    pstPrevious = _pstNode->pstPrevious;
    pstNext     = _pstNode->pstNext;

    /* Not at the start of the list? */
    if(pstPrevious != orxNULL)
    {
      /* Updates previous node */
      pstPrevious->pstNext  = pstNext;
    }
    else
    {
      /* Checks node was at the start of the list */
      orxASSERT(pstList->pstFirst == _pstNode);

      /* Updates list first pointer */
      pstList->pstFirst     = pstNext;
    }

    /* Not at the end of the list? */
    if(pstNext != orxNULL)
    {
      /* Updates previous node */
      pstNext->pstPrevious  = pstPrevious;
    }
    else
    {
      /* Checks node was at the end of the list */
      orxASSERT(pstList->pstLast == _pstNode);

      /* Updates list last pointer */
      pstList->pstLast      = pstPrevious;
    }

    /* Cleans node */
    orxMemory_Zero(_pstNode, sizeof(orxLINKLIST_NODE));

    /* Udpates count */
    pstList->u32Count--;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Can't remove node from list: it isn't part of any list.");

    /* Failed */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}
