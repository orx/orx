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
 * @file orxLinkList.c
 * @date 06/04/2005
 * @author iarwain@orx-project.org
 *
 * @todo
 */


#include "utils/orxLinkList.h"
#include "memory/orxMemory.h"


/** Module flags
 */
#define orxLINKLIST_KU32_STATIC_FLAG_NONE         0x00000000  /**< No flags */

#define orxLINKLIST_KU32_STATIC_FLAG_READY        0x00000001  /**< Ready flags */

#define orxLINKLIST_KU32_STATIC_MASK_ALL          0xFFFFFFFF  /**< All mask */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxLINKLIST_STATIC_t
{
  orxU32 u32Flags;                                            /**< Control flags */

} orxLINKLIST_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
orxSTATIC orxLINKLIST_STATIC sstLinkList;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Linklist module setup
 */
orxVOID orxLinkList_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_LINKLIST, orxMODULE_ID_MEMORY);

  return;
}

/** Inits the linklist module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxLinkList_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Already Initialized? */
  if(!(sstLinkList.u32Flags & orxLINKLIST_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstLinkList, sizeof(orxLINKLIST_STATIC));

    /* Inits flags */
    sstLinkList.u32Flags = orxLINKLIST_KU32_STATIC_FLAG_READY;

    /* Success */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Tried to initialize LinkList module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Exits from the linklist module
 */
orxVOID orxLinkList_Exit()
{
  /* Initialized? */
  if(sstLinkList.u32Flags & orxLINKLIST_KU32_STATIC_FLAG_READY)
  {
    /* Updates flags */
    sstLinkList.u32Flags &= ~orxLINKLIST_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Tried to exit LinkList module when it wasn't initialized.");
  }

  return;
}

/** Cleans a link list
 * @param[in]   _pstList                        Concerned list
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxLinkList_Clean(orxLINKLIST *_pstList)
{
  /* Checks */
  orxASSERT(sstLinkList.u32Flags & orxLINKLIST_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstList != orxNULL);

  /* Non empty? */
  if(_pstList->u32Counter != 0)
  {
    orxREGISTER orxLINKLIST_NODE *pstNode, *pstNext;

    /* Gets first node */
    pstNode = _pstList->pstFirst;

    /* Clean all nodes */
    while(pstNode != orxNULL)
    {
      /* Backups next node */
      pstNext = pstNode->pstNext;

      /* Cleans current node */
      orxMemory_Zero(pstNode, sizeof(orxLINKLIST_NODE));

      /* Go to next node */
      pstNode = pstNext;
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
  orxREGISTER orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstLinkList.u32Flags & orxLINKLIST_KU32_STATIC_FLAG_READY);
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

    /* Updates counter */
    _pstList->u32Counter++;
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
  orxREGISTER orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstLinkList.u32Flags & orxLINKLIST_KU32_STATIC_FLAG_READY);
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

    /* Updates counter */
    _pstList->u32Counter++;
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
  orxREGISTER orxSTATUS eResult = orxSTATUS_SUCCESS;
  orxREGISTER orxLINKLIST *pstList;

  /* Checks */
  orxASSERT(sstLinkList.u32Flags & orxLINKLIST_KU32_STATIC_FLAG_READY);
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

      /* Updates counter */
      pstList->u32Counter++;
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
  orxREGISTER orxSTATUS eResult = orxSTATUS_SUCCESS;
  orxREGISTER orxLINKLIST *pstList;

  /* Checks */
  orxASSERT(sstLinkList.u32Flags & orxLINKLIST_KU32_STATIC_FLAG_READY);
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

      /* Updates counter */
      pstList->u32Counter++;
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
  orxREGISTER orxLINKLIST *pstList;
  orxREGISTER orxLINKLIST_NODE *pstPrevious, *pstNext;
  orxREGISTER orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstLinkList.u32Flags & orxLINKLIST_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstNode != orxNULL);

  /* Gets list */
  pstList = _pstNode->pstList;

  /* Valid? */
  if(pstList != orxNULL)
  {
    /* Checks list is non empty */
    orxASSERT(pstList->u32Counter != 0);

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

    /* Udpates counter */
    pstList->u32Counter--;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Target's list is not valid.");

    /* Failed */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}
