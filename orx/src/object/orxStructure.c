/***************************************************************************
 orxStructure.c
 Structure module
 
 begin                : 08/12/2003
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


#include "object/orxStructure.h"

#include "memory/orxBank.h"
#include "utils/orxLinkList.h"
#include "utils/orxTree.h"


/*
 * Platform independant defines
 */

#define orxSTRUCTURE_KU32_FLAG_NONE         0x00000000
#define orxSTRUCTURE_KU32_FLAG_READY        0x00000001

/* *** Misc *** */
#define orxSTRUCTURE_KU32_BANK_SIZE         256

/* Storage types */
#define STRUCTURE_KS32_TYPE_NONE          0
#define STRUCTURE_KS32_TYPE_LIST          1
#define STRUCTURE_KS32_TYPE_TREE          2



/*
 * Internal storage structure
 */
typedef struct __orxSTORAGE_t
{
  /* Associated bank */
  orxBANK *pstBank;

  /* Storage type */
  orxSTRUCTURE_STORAGE_TYPE eType;

  /* Storage union */
  union
  {
    /* Link List */
    orxLINKLIST stLinkList;

    /* Tree */
    orxTREE stTree;
  };

} orxSTORAGE;

/*
 * Internal storage node
 */
typedef struct __orxSTORAGE_NODE_t
{
  /* Storage node union */
  union
  {    
    /* Link list node */
    orxLINKLIST_NODE stLinkListNode;

    /* Tree node */
    orxTREE_NODE stTreeNode;
  };

  /* Pointer to structure */
  orxSTRUCTURE *pstStructure;

  /* Storage type */
  orxSTRUCTURE_STORAGE_TYPE eType;

} orxSTORAGE_NODE;

/*
 * Static structure
 */
typedef struct __orxSTRUCTURE_STATIC_t
{
  /* Structure banks */
  orxSTORAGE astStorage[orxSTRUCTURE_ID_NUMBER];

  /* Control flags */
  orxU32 u32Flags;

} orxSTRUCTURE_STATIC;

/*
 * Static data
 */
orxSTATIC orxSTRUCTURE_STATIC sstStructure;


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
 orxStructure_Init
 Inits structure system.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxStructure_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILED;
  orxU32 i;

  /* Makes sure the structure IDs are coherent */
  orxASSERT(orxSTRUCTURE_ID_NUMBER <= orxSTRUCTURE_ID_MAX_NUMBER);

  /* Not already Initialized? */
  if(!(sstStructure.u32Flags & orxSTRUCTURE_KU32_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Set(&sstStructure, 0, sizeof(orxSTRUCTURE_STATIC));

    /* For all IDs */
    for(i = 0; i < orxSTRUCTURE_ID_NUMBER; i++)
    {
      /* Creates a bank */
      sstStructure.astStorage[i].pstBank  = orxBank_Create(orxSTRUCTURE_KU32_BANK_SIZE, sizeof(orxSTORAGE_NODE), orxBANK_KU32_FLAGS_NONE, orxMEMORY_TYPE_MAIN);

      /* Cleans storage type */
      sstStructure.astStorage[i].eType    = orxSTRUCTURE_STORAGE_TYPE_NONE;
    }

    /* All banks created? */
    if(i == orxSTRUCTURE_ID_NUMBER)
    {
      /* Inits Flags */
      sstStructure.u32Flags = orxSTRUCTURE_KU32_FLAG_READY;

      /* Everything's ok */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      orxU32 j;

      /* !!! MSG !!! */

      /* For all created banks */
      for(j = 0; j < i; j++)
      {
        /* Deletes it */
        orxBank_Delete(sstStructure.astStorage[j].pstBank);
      }
    }
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxStructure_Exit
 Exits from the structure system.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxStructure_Exit()
{
  /* Initialized? */
  if(sstStructure.u32Flags & orxSTRUCTURE_KU32_FLAG_READY)
  {
    orxU32 i;

    /* For all banks */
    for(i = 0; i < orxSTRUCTURE_ID_NUMBER; i++)
    {
      /* Depending on storage type */
      switch(sstStructure.astStorage[i].eType)
      {
        case orxSTRUCTURE_STORAGE_TYPE_LINKLIST:

        /* Empties list */
        orxLinkList_Clean(&(sstStructure.astStorage[i].stLinkList));

        break;
      
      case orxSTRUCTURE_STORAGE_TYPE_TREE:

        /* Empties tree */
        orxTree_Clean(&(sstStructure.astStorage[i].stTree));

        break;

      default:

        break;
      }

      /* Deletes it */
      orxBank_Delete(sstStructure.astStorage[i].pstBank);
    }

    /* Updates flags */
    sstStructure.u32Flags &= ~orxSTRUCTURE_KU32_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}

/***************************************************************************
 orxStructure_RegisterStorageType
 Registers a storage type for a given ID.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxStructure_RegisterStorageType(orxSTRUCTURE_ID _eStructureID, orxSTRUCTURE_STORAGE_TYPE _eType)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstStructure.u32Flags & orxSTRUCTURE_KU32_FLAG_READY);
  orxASSERT(_eStructureID < orxSTRUCTURE_ID_NUMBER);
  orxASSERT(_eType < orxSTRUCTURE_STORAGE_TYPE_NUMBER);

  /* Not already registered? */
  if(sstStructure.astStorage[_eStructureID].eType == orxSTRUCTURE_STORAGE_TYPE_NONE)
  {
    /* Register it */
    sstStructure.astStorage[_eStructureID].eType = _eType;
  }
  else
  {
    /* !!! MSG !!! */

    /* Already registered */
    eResult = orxSTATUS_FAILED;
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxStructure_GetStorageType
 Gets structure storage type.

 returns: Structure storage ID
 ***************************************************************************/
orxSTRUCTURE_STORAGE_TYPE orxFASTCALL orxStructure_GetStorageType(orxSTRUCTURE_ID _eStructureID)
{
  /* Checks */
  orxASSERT(sstStructure.u32Flags & orxSTRUCTURE_KU32_FLAG_READY);
  orxASSERT(_eStructureID < orxSTRUCTURE_ID_NUMBER);

  /* Returns it */
  return(sstStructure.astStorage[_eStructureID].eType);
}

/***************************************************************************
 orxStructure_GetNumber
 Gets given type structure number.

 returns: number / orxU32_Undefined
 ***************************************************************************/
orxU32 orxFASTCALL orxStructure_GetNumber(orxSTRUCTURE_ID _eStructureID)
{
  orxREGISTER orxU32 u32Result = orxU32_Undefined;

  /* Checks */
  orxASSERT(sstStructure.u32Flags & orxSTRUCTURE_KU32_FLAG_READY);
  orxASSERT(_eStructureID < orxSTRUCTURE_ID_NUMBER);

  /* Dependig on type */
  switch(sstStructure.astStorage[_eStructureID].eType)
  {
  case orxSTRUCTURE_STORAGE_TYPE_LINKLIST:

    /* Gets counter */
    u32Result = orxLinkList_GetCounter(&(sstStructure.astStorage[_eStructureID].stLinkList));

    break;

  case orxSTRUCTURE_STORAGE_TYPE_TREE:

    /* Gets counter */
    u32Result = orxTree_GetCounter(&(sstStructure.astStorage[_eStructureID].stTree));

    break;

  default:

    /* !!! MSG !!! */

    break;
  }

  /* Done ! */
  return u32Result;
}

/***************************************************************************
 orxStructure_Setup
 Inits a structure with given type.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxFASTCALL orxStructure_Setup(orxSTRUCTURE *_pstStructure, orxSTRUCTURE_ID _eStructureID)
{
  orxREGISTER orxSTORAGE_NODE *pstNode;
  orxREGISTER orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstStructure.u32Flags & orxSTRUCTURE_KU32_FLAG_READY);
  orxASSERT(_pstStructure != orxNULL);
  orxASSERT(_eStructureID < orxSTRUCTURE_ID_NUMBER);

  /* Creates node */
  pstNode = (orxSTORAGE_NODE *)orxBank_Allocate(sstStructure.astStorage[_eStructureID].pstBank);

  /* Valid? */
  if(pstNode != orxNULL)
  {
    /* Cleans it */
    orxMemory_Set(pstNode, 0, sizeof(orxSTORAGE_NODE));

    /* Dependig on type */
    switch(sstStructure.astStorage[_eStructureID].eType)
    {
    case orxSTRUCTURE_STORAGE_TYPE_LINKLIST:

      /* Adds node to list */
      eResult = orxLinkList_AddStart(&(sstStructure.astStorage[_eStructureID].stLinkList), &(pstNode->stLinkListNode));

      break;

    case orxSTRUCTURE_STORAGE_TYPE_TREE:

      /* Adds node to tree */
      eResult = orxTree_AddChild(orxTree_GetRoot(&(sstStructure.astStorage[_eStructureID].stTree)), &(pstNode->stTreeNode));

      break;

    default:

      /* !!! MSG !!! */

      /* Wrong type */
      eResult = orxSTATUS_FAILED;
    }
    
    /* Succesful? */
    if(eResult == orxSTATUS_SUCCESS)
    {
      /* Cleans reference counter */
      _pstStructure->u32RefCounter = 0;

      /* Stores ID */
      _pstStructure->eID = _eStructureID;
      
      /* Stores storage handle */
      _pstStructure->hStorageNode = (orxHANDLE)pstNode;
      
      /* Stores structure pointer */
      pstNode->pstStructure = _pstStructure;
    }
    else
    {
      /* !!! MSG !!! */

      /* Frees allocated node */
      orxBank_Free(sstStructure.astStorage[_eStructureID].pstBank, pstNode);
    }        
  }
  else
  {
    /* !!! MSG !!! */
    
    /* Not allocated */
    eResult = orxSTATUS_FAILED;
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxStructure_Clean
 Cleans a structure.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFASTCALL orxStructure_Clean(orxSTRUCTURE *_pstStructure)
{
  orxREGISTER orxSTORAGE_NODE *pstNode;

  /* Checks */
  orxASSERT(sstStructure.u32Flags & orxSTRUCTURE_KU32_FLAG_READY);
  orxASSERT(_pstStructure != orxNULL);

  /* Gets storage node */
  pstNode = (orxSTORAGE_NODE *)_pstStructure->hStorageNode;

  /* Valid? */
  if(pstNode != orxNULL)
  {
    /* Dependig on type */
    switch(sstStructure.astStorage[_pstStructure->eID].eType)
    {
    case orxSTRUCTURE_STORAGE_TYPE_LINKLIST:

      /* Removes node from list */
      orxLinkList_Remove(&(pstNode->stLinkListNode));

    break;

    case orxSTRUCTURE_STORAGE_TYPE_TREE:

      /* Removes node from list */
      orxTree_Remove(&(pstNode->stTreeNode));

    break;

      default:
        /* !!! MSG !!! */
        break;
    }

    /* Deletes it */
    orxBank_Free(sstStructure.astStorage[_pstStructure->eID].pstBank, pstNode);

    /* Cleans structure ID */
    _pstStructure->eID = orxSTRUCTURE_ID_NONE;
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}


/* *** Structure accessors *** */


/***************************************************************************
 orxStructure_GetFirst
 Gets first stored structure (first list cell or tree root depending on storage type).

 returns: first structure
 ***************************************************************************/
orxSTRUCTURE *orxFASTCALL orxStructure_GetFirst(orxSTRUCTURE_ID _eStructureID)
{
  orxREGISTER orxSTORAGE_NODE *pstNode;
  orxREGISTER orxSTRUCTURE *pstStructure = orxNULL;

  /* Checks */
  orxASSERT(sstStructure.u32Flags & orxSTRUCTURE_KU32_FLAG_READY);
  orxASSERT(_eStructureID < orxSTRUCTURE_ID_NUMBER);

  /* Depending on type */
  switch(sstStructure.astStorage[_eStructureID].eType)
  {
  case orxSTRUCTURE_STORAGE_TYPE_LINKLIST:

    /* Gets node from list */
    pstNode = (orxSTORAGE_NODE *)orxLinkList_GetFirst(&(sstStructure.astStorage[_eStructureID].stLinkList));

    break;

  case orxSTRUCTURE_STORAGE_TYPE_TREE:

    /* Gets node from tree */
    pstNode = (orxSTORAGE_NODE *)orxTree_GetRoot(&(sstStructure.astStorage[_eStructureID].stTree));

    break;

  default:

    /* !!! MSG !!! */

    /* No node found */
    pstNode = orxNULL;

    break;
  }

  /* Node found? */
  if(pstNode != orxNULL)
  {
    /* Gets associated structure */
    pstStructure = pstNode->pstStructure;
  }

  /* Done! */
  return pstStructure;
}

/***************************************************************************
 orxStructure_GetParent
 Structure tree parent get accessor.

 returns: parent
 ***************************************************************************/
orxSTRUCTURE *orxFASTCALL orxStructure_GetParent(orxSTRUCTURE *_pstStructure)
{
  orxREGISTER orxSTORAGE_NODE *pstNode;
  orxREGISTER orxSTRUCTURE *pstStructure = orxNULL;

  /* Checks */
  orxASSERT(sstStructure.u32Flags & orxSTRUCTURE_KU32_FLAG_READY);
  orxASSERT(_pstStructure != orxNULL);

  /* Gets node */
  pstNode = (orxSTORAGE_NODE *)_pstStructure->hStorageNode;

  /* Valid? */
  if(pstNode != orxNULL)
  {
    /* Is storate type correct? */
    if(pstNode->eType == orxSTRUCTURE_STORAGE_TYPE_TREE)
    {
      /* Gets parent node */
      pstNode = (orxSTORAGE_NODE *)orxTree_GetParent(&(pstNode->stTreeNode));

      /* Valid? */
      if(pstNode != orxNULL)
      {
        /* Gets structure */
        pstStructure = pstNode->pstStructure;
      }
    }
    else
    {
      /* !!! MSG !!! */
    }
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return pstStructure;
}

/***************************************************************************
 orxStructure_GetChild
 Structure tree child get accessor.

 returns: child
 ***************************************************************************/
orxSTRUCTURE *orxFASTCALL orxStructure_GetChild(orxSTRUCTURE *_pstStructure)
{
  orxREGISTER orxSTORAGE_NODE *pstNode;
  orxREGISTER orxSTRUCTURE *pstStructure = orxNULL;

  /* Checks */
  orxASSERT(sstStructure.u32Flags & orxSTRUCTURE_KU32_FLAG_READY);
  orxASSERT(_pstStructure != orxNULL);

  /* Gets node */
  pstNode = (orxSTORAGE_NODE *)_pstStructure->hStorageNode;

  /* Valid? */
  if(pstNode != orxNULL)
  {
    /* Is storate type correct? */
    if(pstNode->eType == orxSTRUCTURE_STORAGE_TYPE_TREE)
    {
      /* Gets child node */
      pstNode = (orxSTORAGE_NODE *)orxTree_GetChild(&(pstNode->stTreeNode));

      /* Valid? */
      if(pstNode != orxNULL)
      {
        /* Gets structure */
        pstStructure = pstNode->pstStructure;
      }
    }
    else
    {
      /* !!! MSG !!! */
    }
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return pstStructure;
}

/***************************************************************************
 orxStructure_GetLeftSibling
 Structure tree left sibling get accessor.

 returns: left sibling
 ***************************************************************************/
orxSTRUCTURE *orxFASTCALL orxStructure_GetSibling(orxSTRUCTURE *_pstStructure)
{
  orxREGISTER orxSTORAGE_NODE *pstNode;
  orxREGISTER orxSTRUCTURE *pstStructure = orxNULL;

  /* Checks */
  orxASSERT(sstStructure.u32Flags & orxSTRUCTURE_KU32_FLAG_READY);
  orxASSERT(_pstStructure != orxNULL);

  /* Gets node */
  pstNode = (orxSTORAGE_NODE *)_pstStructure->hStorageNode;

  /* Valid? */
  if(pstNode != orxNULL)
  {
    /* Is storate type correct? */
    if(pstNode->eType == orxSTRUCTURE_STORAGE_TYPE_TREE)
    {
      /* Gets sibling node */
      pstNode = (orxSTORAGE_NODE *)orxTree_GetSibling(&(pstNode->stTreeNode));

      /* Valid? */
      if(pstNode != orxNULL)
      {
        /* Gets structure */
        pstStructure = pstNode->pstStructure;
      }
    }
    else
    {
      /* !!! MSG !!! */
    }
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return pstStructure;
}

/***************************************************************************
 orxStructure_GetPrevious
 Structure list previous get accessor.

 returns: previous
 ***************************************************************************/
orxSTRUCTURE *orxFASTCALL orxStructure_GetPrevious(orxSTRUCTURE *_pstStructure)
{
  orxREGISTER orxSTORAGE_NODE *pstNode;
  orxREGISTER orxSTRUCTURE *pstStructure = orxNULL;

  /* Checks */
  orxASSERT(sstStructure.u32Flags & orxSTRUCTURE_KU32_FLAG_READY);
  orxASSERT(_pstStructure != orxNULL);

  /* Gets node */
  pstNode = (orxSTORAGE_NODE *)_pstStructure->hStorageNode;

  /* Valid? */
  if(pstNode != orxNULL)
  {
    /* Is storate type correct? */
    if(pstNode->eType == orxSTRUCTURE_STORAGE_TYPE_LINKLIST)
    {
      /* Gets previous node */
      pstNode = (orxSTORAGE_NODE *)orxLinkList_GetPrevious(&(pstNode->stLinkListNode));

      /* Valid? */
      if(pstNode != orxNULL)
      {
        /* Gets structure */
        pstStructure = pstNode->pstStructure;
      }
    }
    else
    {
      /* !!! MSG !!! */
    }
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return pstStructure;
}

/***************************************************************************
 orxStructure_GetNext
 Structure list next get accessor.

 returns: next
 ***************************************************************************/
orxSTRUCTURE *orxFASTCALL orxStructure_GetNext(orxSTRUCTURE *_pstStructure)
{
  orxREGISTER orxSTORAGE_NODE *pstNode;
  orxREGISTER orxSTRUCTURE *pstStructure = orxNULL;

  /* Checks */
  orxASSERT(sstStructure.u32Flags & orxSTRUCTURE_KU32_FLAG_READY);
  orxASSERT(_pstStructure != orxNULL);

  /* Gets node */
  pstNode = (orxSTORAGE_NODE *)_pstStructure->hStorageNode;

  /* Valid? */
  if(pstNode != orxNULL)
  {
    /* Is storate type correct? */
    if(pstNode->eType == orxSTRUCTURE_STORAGE_TYPE_LINKLIST)
    {
      /* Gets next node */
      pstNode = (orxSTORAGE_NODE *)orxLinkList_GetNext(&(pstNode->stLinkListNode));

      /* Valid? */
      if(pstNode != orxNULL)
      {
        /* Gets structure */
        pstStructure = pstNode->pstStructure;
      }
    }
    else
    {
      /* !!! MSG !!! */
    }
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return pstStructure;
}

/***************************************************************************
 orxStructure_SetParent
 Structure tree parent set accessor.

 returns: orxVOID
 ***************************************************************************/
orxSTATUS orxFASTCALL orxStructure_SetParent(orxSTRUCTURE *_pstStructure, orxSTRUCTURE *_pstParent)
{
  orxREGISTER orxSTORAGE_NODE *pstNode, *pstParentNode;
  orxREGISTER orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstStructure.u32Flags & orxSTRUCTURE_KU32_FLAG_READY);
  orxASSERT(_pstStructure != orxNULL);
  orxASSERT(_pstParent != orxNULL);

  /* Gets nodes */
  pstNode       = (orxSTORAGE_NODE *)_pstStructure->hStorageNode;
  pstParentNode = (orxSTORAGE_NODE *)_pstParent->hStorageNode;

  /* Valid? */
  if((pstNode != orxNULL) && (pstParentNode != orxNULL))
  {
    /* Is storate type correct? */
    if(pstNode->eType == orxSTRUCTURE_STORAGE_TYPE_TREE)
    {
      /* Moves it */
      eResult = orxTree_MoveAsChild(&(pstParentNode->stTreeNode), &(pstNode->stTreeNode));
    }
    else
    {
      /* !!! MSG !!! */

      /* Not done */
      eResult = orxSTATUS_FAILED;
    }
  }
  else
  {
    /* !!! MSG !!! */
    
    /* Not done */
    eResult = orxSTATUS_FAILED;
  }

  /* Done! */
  return eResult;
}
