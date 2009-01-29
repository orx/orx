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
 * @file orxStructure.c
 * @date 08/12/2003
 * @author iarwain@orx-project.org
 *
 * @todo
 */


#include "object/orxStructure.h"

#include "memory/orxBank.h"
#include "utils/orxLinkList.h"
#include "utils/orxTree.h"


/** Module flags
 */

#define orxSTRUCTURE_KU32_STATIC_FLAG_NONE    0x00000000

#define orxSTRUCTURE_KU32_STATIC_FLAG_READY   0x00000001

#define orxSTRUCTURE_KU32_STATIC_MASK_ALL     0xFFFFFFFF


/** Defines
 */
#define orxSTRUCTURE_KU32_STORAGE_BANK_SIZE   256

#define orxSTRUCTURE_KU32_STRUCTURE_BANK_SIZE 32


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Internal storage structure
 */
typedef struct __orxSTRUCTURE_STORAGE_t
{
  orxSTRUCTURE_STORAGE_TYPE eType;            /**< Storage type : 4 */
  orxBANK                  *pstNodeBank;      /**< Associated node bank : 8 */
  orxBANK                  *pstStructureBank; /**< Associated structure bank : 12 */

  union
  {
    orxLINKLIST             stLinkList;       /**< Linklist : 24 */
    orxTREE                 stTree;           /**< Tree : 20 */
  };                                          /**< Storage union : 24 */

} orxSTRUCTURE_STORAGE;

/** Internal registration info
 */
typedef struct __orxSTRUCTURE_REGISTER_INFO_t
{
  orxSTRUCTURE_STORAGE_TYPE     eStorageType; /**< Structure storage type : 4 */
  orxU32                        u32Size;      /**< Structure storage size : 8 */
  orxMEMORY_TYPE                eMemoryType;  /**< Structure storage memory type : 12 */
  orxSTRUCTURE_UPDATE_FUNCTION  pfnUpdate;    /**< Structure update callbacks : 16 */

} orxSTRUCTURE_REGISTER_INFO;

/** Internal storage node
 */
typedef struct __orxSTRUCTURE_STORAGE_NODE_t
{
  union
  {
    orxLINKLIST_NODE stLinkListNode;          /**< Linklist node : 12 */
    orxTREE_NODE stTreeNode;                  /**< Tree node : 16 */
  };                                          /**< Storage node union : 16 */
  orxSTRUCTURE *pstStructure;                 /**< Pointer to structure : 20 */
  orxSTRUCTURE_STORAGE_TYPE eType;            /**< Storage type : 24 */

} orxSTRUCTURE_STORAGE_NODE;

/** Static structure
 */
typedef struct __orxSTRUCTURE_STATIC_t
{
  orxSTRUCTURE_STORAGE        astStorage[orxSTRUCTURE_ID_NUMBER]; /**< Structure banks */
  orxSTRUCTURE_REGISTER_INFO  astInfo[orxSTRUCTURE_ID_NUMBER];    /**< Structure info */
  orxU32                      u32Flags;                           /**< Control flags */

} orxSTRUCTURE_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** static data
 */
orxSTATIC orxSTRUCTURE_STATIC sstStructure;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Structure module setup
 */
orxVOID orxStructure_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_STRUCTURE, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_STRUCTURE, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_STRUCTURE, orxMODULE_ID_LINKLIST);
  orxModule_AddDependency(orxMODULE_ID_STRUCTURE, orxMODULE_ID_TREE);

  return;
}

/** Initializess the structure module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxStructure_Init()
{
  orxU32    i;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstStructure.u32Flags & orxSTRUCTURE_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstStructure, sizeof(orxSTRUCTURE_STATIC));

    /* For all IDs */
    for(i = 0; i < orxSTRUCTURE_ID_NUMBER; i++)
    {
      /* Creates a bank */
      sstStructure.astStorage[i].pstNodeBank = orxBank_Create(orxSTRUCTURE_KU32_STORAGE_BANK_SIZE, sizeof(orxSTRUCTURE_STORAGE_NODE), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

      /* Cleans storage type */
      sstStructure.astStorage[i].eType = orxSTRUCTURE_STORAGE_TYPE_NONE;
    }

    /* All banks created? */
    if(i == orxSTRUCTURE_ID_NUMBER)
    {
      /* Inits Flags */
      sstStructure.u32Flags = orxSTRUCTURE_KU32_STATIC_FLAG_READY;

      /* Everything's ok */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      orxU32 j;

      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to create bank with ID (%ld).", i);

      /* For all created banks */
      for(j = 0; j < i; j++)
      {
        /* Deletes it */
        orxBank_Delete(sstStructure.astStorage[j].pstNodeBank);
      }
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Tried to initialize structure module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Exits from the structure module
 */
orxVOID orxStructure_Exit()
{
  /* Initialized? */
  if(sstStructure.u32Flags & orxSTRUCTURE_KU32_STATIC_FLAG_READY)
  {
    orxU32 i;

    /* For all banks */
    for(i = 0; i < orxSTRUCTURE_ID_NUMBER; i++)
    {
      /* Depending on storage type */
      switch(sstStructure.astStorage[i].eType)
      {
        case orxSTRUCTURE_STORAGE_TYPE_LINKLIST:
        {
          /* Empties list */
          orxLinkList_Clean(&(sstStructure.astStorage[i].stLinkList));

          break;
        }

        case orxSTRUCTURE_STORAGE_TYPE_TREE:
        {
            /* Empties tree */
            orxTree_Clean(&(sstStructure.astStorage[i].stTree));

            break;
        }

        default:
        {
          break;
        }
      }

      /* Deletes node */
      orxBank_Delete(sstStructure.astStorage[i].pstNodeBank);

      /* Is bank empty? */
      if(sstStructure.astStorage[i].pstStructureBank != orxNULL)
      {
        /* Deletes it */
        orxBank_Delete(sstStructure.astStorage[i].pstStructureBank);
      }
    }

    /* Updates flags */
    sstStructure.u32Flags &= ~orxSTRUCTURE_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Tried to exit structure module when it wasn't initialized.");
  }

  return;
}

/** Registers a given ID
 * @param[in]   _eStructureID   Concerned structure ID
 * @param[in]   _eStorageType   Storage type to use for this structure type
 * @param[in]   _eMemoryTyp     Memory type to store this structure type
 * @param[in]   _u32Size        Structure size
 * @param[in]   _pfnUpdate      Structure update function
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxStructure_Register(orxSTRUCTURE_ID _eStructureID, orxSTRUCTURE_STORAGE_TYPE _eStorageType, orxMEMORY_TYPE _eMemoryType, orxU32 _u32Size, orxCONST orxSTRUCTURE_UPDATE_FUNCTION _pfnUpdate)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstStructure.u32Flags & orxSTRUCTURE_KU32_STATIC_FLAG_READY);
  orxASSERT(_eStructureID < orxSTRUCTURE_ID_NUMBER);
  orxASSERT(_u32Size != 0);
  orxASSERT(_eStorageType < orxSTRUCTURE_STORAGE_TYPE_NUMBER);
  orxASSERT(_eMemoryType < orxMEMORY_TYPE_NUMBER);

  /* Not already registered? */
  if(sstStructure.astInfo[_eStructureID].u32Size == 0)
  {
    /* Creates bank for structure storage */
    sstStructure.astStorage[_eStructureID].pstStructureBank = orxBank_Create(orxSTRUCTURE_KU32_STRUCTURE_BANK_SIZE, _u32Size, orxBANK_KU32_FLAG_NONE, _eMemoryType);

    /* Valid? */
    if(sstStructure.astStorage[_eStructureID].pstStructureBank != orxNULL)
    {
      /* Registers it */
      sstStructure.astInfo[_eStructureID].eStorageType  = _eStorageType;
      sstStructure.astInfo[_eStructureID].eMemoryType   = _eMemoryType;
      sstStructure.astInfo[_eStructureID].u32Size       = _u32Size;
      sstStructure.astInfo[_eStructureID].pfnUpdate     = _pfnUpdate;

      sstStructure.astStorage[_eStructureID].eType      = _eStorageType;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Invalid storage bank created.");
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Structure is already registered.");

    /* Already registered */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Unregisters a given ID
 * @param[in]   _eStructureID   Concerned structure ID
 */
orxVOID orxFASTCALL orxStructure_Unregister(orxSTRUCTURE_ID _eStructureID)
{
  /* Checks */
  orxASSERT(sstStructure.u32Flags & orxSTRUCTURE_KU32_STATIC_FLAG_READY);
  orxASSERT(_eStructureID < orxSTRUCTURE_ID_NUMBER);

  /* Registered? */
  if(sstStructure.astInfo[_eStructureID].u32Size != 0)
  {
    /* Deletes structure storage bank */
    orxBank_Delete(sstStructure.astStorage[_eStructureID].pstStructureBank);

    /* Unregisters it */
    orxMemory_Zero(&(sstStructure.astInfo[_eStructureID]), sizeof(orxSTRUCTURE_REGISTER_INFO));
    sstStructure.astStorage[_eStructureID].pstStructureBank = orxNULL;
    sstStructure.astStorage[_eStructureID].eType            = orxSTRUCTURE_STORAGE_TYPE_NONE;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Structure is already registered.");
  }

  return;
}

/** Creates a clean structure for given type
 * @param[in]   _eStructureID   Concerned structure ID
 * @return      orxSTRUCTURE / orxNULL
 */
orxSTRUCTURE *orxFASTCALL orxStructure_Create(orxSTRUCTURE_ID _eStructureID)
{
  orxREGISTER orxSTRUCTURE_STORAGE_NODE *pstNode;
  orxREGISTER orxSTRUCTURE *pstStructure = orxNULL;

  /* Checks */
  orxASSERT(sstStructure.u32Flags & orxSTRUCTURE_KU32_STATIC_FLAG_READY);
  orxASSERT(_eStructureID < orxSTRUCTURE_ID_NUMBER);

  /* Is structure type registered? */
  if(sstStructure.astInfo[_eStructureID].u32Size != 0)
  {
    /* Creates structure */
    pstStructure = (orxSTRUCTURE *)orxBank_Allocate(sstStructure.astStorage[_eStructureID].pstStructureBank);

    /* Valid? */
    if(pstStructure != orxNULL)
    {
      /* Creates node */
      pstNode = (orxSTRUCTURE_STORAGE_NODE *)orxBank_Allocate(sstStructure.astStorage[_eStructureID].pstNodeBank);

      /* Valid? */
      if(pstNode != orxNULL)
      {
        orxSTATUS eResult;

        /* Cleans it */
        orxMemory_Zero(pstNode, sizeof(orxSTRUCTURE_STORAGE_NODE));

        /* Stores its type */
        pstNode->eType = sstStructure.astStorage[_eStructureID].eType;

        /* Dependig on type */
        switch(pstNode->eType)
        {
        case orxSTRUCTURE_STORAGE_TYPE_LINKLIST:

          /* Adds node to list */
          eResult = orxLinkList_AddEnd(&(sstStructure.astStorage[_eStructureID].stLinkList), &(pstNode->stLinkListNode));

          break;

        case orxSTRUCTURE_STORAGE_TYPE_TREE:

          /* No root yet? */
          if(orxTree_GetRoot(&(sstStructure.astStorage[_eStructureID].stTree)) == orxNULL)
          {
            /* Adds root to tree */
            eResult = orxTree_AddRoot(&(sstStructure.astStorage[_eStructureID].stTree), &(pstNode->stTreeNode));
          }
          else
          {
            /* Adds node to tree */
            eResult = orxTree_AddChild(orxTree_GetRoot(&(sstStructure.astStorage[_eStructureID].stTree)), &(pstNode->stTreeNode));
          }

          break;

        default:

          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Invalid structure storage type.");

          /* Wrong type */
          eResult = orxSTATUS_FAILURE;
        }

        /* Succesful? */
        if(eResult == orxSTATUS_SUCCESS)
        {
          /* Cleans whole structure */
          orxMemory_Zero(pstStructure, sstStructure.astInfo[_eStructureID].u32Size);

          /* Stores ID with magic number */
          pstStructure->eID           = _eStructureID ^ orxSTRUCTURE_MAGIC_TAG_ACTIVE;

          /* Stores storage handle */
          pstStructure->hStorageNode  = (orxHANDLE)pstNode;

          /* Stores structure pointer */
          pstNode->pstStructure       = pstStructure;
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed adding node to list.");

          /* Frees allocated node & structure */
          orxBank_Free(sstStructure.astStorage[_eStructureID].pstNodeBank, pstNode);
          orxBank_Free(sstStructure.astStorage[_eStructureID].pstStructureBank, pstStructure);

          /* Not created */
          pstStructure = orxNULL;
        }
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to allocate node bank.");

        /* Frees allocated structure */
        orxBank_Free(sstStructure.astStorage[_eStructureID].pstStructureBank, pstStructure);

        /* Not allocated */
        pstStructure = orxNULL;
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to allocate structure bank.");
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Structure type is not registered.");
  }

  /* Done! */
  return pstStructure;
}

/** Deletes a structure (needs to be cleaned beforehand)
 * @param[in]   _pStructure    Concerned structure
 */
orxVOID orxFASTCALL orxStructure_Delete(orxVOID *_pStructure)
{
  orxREGISTER orxSTRUCTURE_STORAGE_NODE *pstNode;

  /* Checks */
  orxASSERT(sstStructure.u32Flags & orxSTRUCTURE_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pStructure);

  /* Gets storage node */
  pstNode = (orxSTRUCTURE_STORAGE_NODE *)((orxSTRUCTURE *)_pStructure)->hStorageNode;

  /* Valid? */
  if(pstNode != orxNULL)
  {
    /* Dependig on type */
    switch(sstStructure.astStorage[orxStructure_GetID(_pStructure)].eType)
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
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Invalid structure storage type.");
        break;
    }

    /* Deletes it */
    orxBank_Free(sstStructure.astStorage[orxStructure_GetID(_pStructure)].pstNodeBank, pstNode);

    /* Deletes structure */
    orxBank_Free(sstStructure.astStorage[orxStructure_GetID(_pStructure)].pstStructureBank, _pStructure);

    /* Tags structure as deleted */
    orxSTRUCTURE(_pStructure)->eID = orxSTRUCTURE_MAGIC_TAG_DELETED;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Invalid storage node.");
  }

  return;
}

/** Gets structure storage type
 * @param[in]   _eStructureID   Concerned structure ID
 * @return      orxSTRUCTURE_STORAGE_TYPE
 */
orxSTRUCTURE_STORAGE_TYPE orxFASTCALL orxStructure_GetStorageType(orxSTRUCTURE_ID _eStructureID)
{
  /* Checks */
  orxASSERT(sstStructure.u32Flags & orxSTRUCTURE_KU32_STATIC_FLAG_READY);
  orxASSERT(_eStructureID < orxSTRUCTURE_ID_NUMBER);

  /* Returns it */
  return(sstStructure.astStorage[_eStructureID].eType);
}

/** Gets given type structure number
 * @param[in]   _eStructureID   Concerned structure ID
 * @return      orxU32 / orxU32_UNDEFINED
 */
orxU32 orxFASTCALL orxStructure_GetNumber(orxSTRUCTURE_ID _eStructureID)
{
  orxREGISTER orxU32 u32Result = orxU32_UNDEFINED;

  /* Checks */
  orxASSERT(sstStructure.u32Flags & orxSTRUCTURE_KU32_STATIC_FLAG_READY);
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

    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Invalid structure storage type.");

    break;
  }

  /* Done ! */
  return u32Result;
}

/** Updates structure if update function was registered for the structure type
 * @param[in]   _pStructure    Concerned structure
 * @param[in]   _pCaller       Caller structure
 * @param[in]   _pstClockInfo   Update associated clock info
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxStructure_Update(orxVOID *_pStructure, orxCONST orxVOID *_pCaller, orxCONST orxCLOCK_INFO *_pstClockInfo)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstStructure.u32Flags & orxSTRUCTURE_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pStructure);
  orxASSERT(_pstClockInfo != orxNULL);

  /* Is structure registered? */
  if(sstStructure.astInfo[orxStructure_GetID(_pStructure)].u32Size != 0)
  {
    /* Is an update function registered? */
    if(sstStructure.astInfo[orxStructure_GetID(_pStructure)].pfnUpdate != orxNULL)
    {
      /* Calls it */
      eResult = sstStructure.astInfo[orxStructure_GetID(_pStructure)].pfnUpdate(((orxSTRUCTURE *)_pStructure), ((orxSTRUCTURE *)_pCaller), _pstClockInfo);
    }
    else
    {
        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Structure is not registered.");
  }

  /* Done! */
  return eResult;
}

/** Gets first stored structure (first list cell or tree root depending on storage type)
 * @param[in]   _eStructureID   Concerned structure ID
 * @return      orxSTRUCTURE
 */
orxSTRUCTURE *orxFASTCALL orxStructure_GetFirst(orxSTRUCTURE_ID _eStructureID)
{
  orxREGISTER orxSTRUCTURE_STORAGE_NODE *pstNode;
  orxREGISTER orxSTRUCTURE *pstStructure = orxNULL;

  /* Checks */
  orxASSERT(sstStructure.u32Flags & orxSTRUCTURE_KU32_STATIC_FLAG_READY);
  orxASSERT(_eStructureID < orxSTRUCTURE_ID_NUMBER);

  /* Depending on type */
  switch(sstStructure.astStorage[_eStructureID].eType)
  {
  case orxSTRUCTURE_STORAGE_TYPE_LINKLIST:

    /* Gets node from list */
    pstNode = (orxSTRUCTURE_STORAGE_NODE *)orxLinkList_GetFirst(&(sstStructure.astStorage[_eStructureID].stLinkList));

    break;

  case orxSTRUCTURE_STORAGE_TYPE_TREE:

    /* Gets node from tree */
    pstNode = (orxSTRUCTURE_STORAGE_NODE *)orxTree_GetRoot(&(sstStructure.astStorage[_eStructureID].stTree));

    break;

  default:

    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Invalid structure storage type.");

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

/** Gets last stored structure (last list cell or tree root depending on storage type)
 * @param[in] _eStructureID      Concerned structure ID
 * return orxSTRUCTURE / orxNULL
 */
orxSTRUCTURE *orxFASTCALL orxStructure_GetLast(orxSTRUCTURE_ID _eStructureID)
{
  orxREGISTER orxSTRUCTURE_STORAGE_NODE *pstNode;
  orxREGISTER orxSTRUCTURE *pstStructure = orxNULL;

  /* Checks */
  orxASSERT(sstStructure.u32Flags & orxSTRUCTURE_KU32_STATIC_FLAG_READY);
  orxASSERT(_eStructureID < orxSTRUCTURE_ID_NUMBER);

  /* Depending on type */
  switch(sstStructure.astStorage[_eStructureID].eType)
  {
  case orxSTRUCTURE_STORAGE_TYPE_LINKLIST:

    /* Gets node from list */
    pstNode = (orxSTRUCTURE_STORAGE_NODE *)orxLinkList_GetLast(&(sstStructure.astStorage[_eStructureID].stLinkList));

    break;

  case orxSTRUCTURE_STORAGE_TYPE_TREE:

    /* Gets node from tree */
    pstNode = (orxSTRUCTURE_STORAGE_NODE *)orxTree_GetRoot(&(sstStructure.astStorage[_eStructureID].stTree));

    break;

  default:

    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Invalid structure storage type.");

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

/** Gets structure tree parent
 * @param[in]   _pStructure    Concerned structure
 * @return      orxSTRUCTURE
 */
orxSTRUCTURE *orxFASTCALL orxStructure_GetParent(orxCONST orxVOID *_pStructure)
{
  orxREGISTER orxSTRUCTURE_STORAGE_NODE *pstNode;
  orxREGISTER orxSTRUCTURE *pstStructure = orxNULL;

  /* Checks */
  orxASSERT(sstStructure.u32Flags & orxSTRUCTURE_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pStructure);

  /* Gets node */
  pstNode = (orxSTRUCTURE_STORAGE_NODE *)((orxSTRUCTURE *)_pStructure)->hStorageNode;

  /* Valid? */
  if(pstNode != orxNULL)
  {
    /* Is storate type correct? */
    if(pstNode->eType == orxSTRUCTURE_STORAGE_TYPE_TREE)
    {
      /* Gets parent node */
      pstNode = (orxSTRUCTURE_STORAGE_NODE *)orxTree_GetParent(&(pstNode->stTreeNode));

      /* Valid? */
      if(pstNode != orxNULL)
      {
        /* Gets structure */
        pstStructure = pstNode->pstStructure;
      }
    }
    else
    {
      /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Storage node is not tree type.");
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Invalid storage node.");
  }

  /* Done! */
  return pstStructure;
}

/** Gets structure tree child
 * @param[in]   _pStructure    Concerned structure
 * @return      orxSTRUCTURE
 */
orxSTRUCTURE *orxFASTCALL orxStructure_GetChild(orxCONST orxVOID *_pStructure)
{
  orxREGISTER orxSTRUCTURE_STORAGE_NODE *pstNode;
  orxREGISTER orxSTRUCTURE *pstStructure = orxNULL;

  /* Checks */
  orxASSERT(sstStructure.u32Flags & orxSTRUCTURE_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pStructure);

  /* Gets node */
  pstNode = (orxSTRUCTURE_STORAGE_NODE *)((orxSTRUCTURE *)_pStructure)->hStorageNode;

  /* Valid? */
  if(pstNode != orxNULL)
  {
    /* Is storate type correct? */
    if(pstNode->eType == orxSTRUCTURE_STORAGE_TYPE_TREE)
    {
      /* Gets child node */
      pstNode = (orxSTRUCTURE_STORAGE_NODE *)orxTree_GetChild(&(pstNode->stTreeNode));

      /* Valid? */
      if(pstNode != orxNULL)
      {
        /* Gets structure */
        pstStructure = pstNode->pstStructure;
      }
    }
    else
    {
      /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Structure is not tree type.");
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Invalid storage node.");
  }

  /* Done! */
  return pstStructure;
}

/** Gets structure tree sibling
 * @param[in]   _pStructure    Concerned structure
 * @return      orxSTRUCTURE
 */
orxSTRUCTURE *orxFASTCALL orxStructure_GetSibling(orxCONST orxVOID *_pStructure)
{
  orxREGISTER orxSTRUCTURE_STORAGE_NODE *pstNode;
  orxREGISTER orxSTRUCTURE *pstStructure = orxNULL;

  /* Checks */
  orxASSERT(sstStructure.u32Flags & orxSTRUCTURE_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pStructure);

  /* Gets node */
  pstNode = (orxSTRUCTURE_STORAGE_NODE *)((orxSTRUCTURE *)_pStructure)->hStorageNode;

  /* Valid? */
  if(pstNode != orxNULL)
  {
    /* Is storate type correct? */
    if(pstNode->eType == orxSTRUCTURE_STORAGE_TYPE_TREE)
    {
      /* Gets sibling node */
      pstNode = (orxSTRUCTURE_STORAGE_NODE *)orxTree_GetSibling(&(pstNode->stTreeNode));

      /* Valid? */
      if(pstNode != orxNULL)
      {
        /* Gets structure */
        pstStructure = pstNode->pstStructure;
      }
    }
    else
    {
      /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Storage type isn't tree.");
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Invalid storage node.");
  }

  /* Done! */
  return pstStructure;
}

/** Gets structure list previous
 * @param[in]   _pStructure    Concerned structure
 * @return      orxSTRUCTURE
 */
orxSTRUCTURE *orxFASTCALL orxStructure_GetPrevious(orxCONST orxVOID *_pStructure)
{
  orxREGISTER orxSTRUCTURE_STORAGE_NODE *pstNode;
  orxREGISTER orxSTRUCTURE *pstStructure = orxNULL;

  /* Checks */
  orxASSERT(sstStructure.u32Flags & orxSTRUCTURE_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pStructure);

  /* Gets node */
  pstNode = (orxSTRUCTURE_STORAGE_NODE *)((orxSTRUCTURE *)_pStructure)->hStorageNode;

  /* Valid? */
  if(pstNode != orxNULL)
  {
    /* Is storate type correct? */
    if(pstNode->eType == orxSTRUCTURE_STORAGE_TYPE_LINKLIST)
    {
      /* Gets previous node */
      pstNode = (orxSTRUCTURE_STORAGE_NODE *)orxLinkList_GetPrevious(&(pstNode->stLinkListNode));

      /* Valid? */
      if(pstNode != orxNULL)
      {
        /* Gets structure */
        pstStructure = pstNode->pstStructure;
      }
    }
    else
    {
      /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Storage type is not a linked list.");
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Not a valid storage node.");
  }

  /* Done! */
  return pstStructure;
}

/** Gets structure list next
 * @param[in]   _pStructure    Concerned structure
 * @return      orxSTRUCTURE
 */
orxSTRUCTURE *orxFASTCALL orxStructure_GetNext(orxCONST orxVOID *_pStructure)
{
  orxREGISTER orxSTRUCTURE_STORAGE_NODE *pstNode;
  orxREGISTER orxSTRUCTURE *pstStructure = orxNULL;

  /* Checks */
  orxASSERT(sstStructure.u32Flags & orxSTRUCTURE_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pStructure);

  /* Gets node */
  pstNode = (orxSTRUCTURE_STORAGE_NODE *)((orxSTRUCTURE *)_pStructure)->hStorageNode;

  /* Valid? */
  if(pstNode != orxNULL)
  {
    /* Is storate type correct? */
    if(pstNode->eType == orxSTRUCTURE_STORAGE_TYPE_LINKLIST)
    {
      /* Gets next node */
      pstNode = (orxSTRUCTURE_STORAGE_NODE *)orxLinkList_GetNext(&(pstNode->stLinkListNode));

      /* Valid? */
      if(pstNode != orxNULL)
      {
        /* Gets structure */
        pstStructure = pstNode->pstStructure;
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Storage type is not a linked list.");
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Invalid storage node.");
  }

  /* Done! */
  return pstStructure;
}

/** Sets structure tree parent
 * @param[in]   _pStructure    Concerned structure
 * @param[in]   _pParent       Structure to set as parent
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxStructure_SetParent(orxVOID *_pStructure, orxVOID *_pParent)
{
  orxREGISTER orxSTRUCTURE_STORAGE_NODE *pstNode, *pstParentNode;
  orxREGISTER orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstStructure.u32Flags & orxSTRUCTURE_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pStructure);
  orxSTRUCTURE_ASSERT(_pParent);

  /* Gets nodes */
  pstNode       = (orxSTRUCTURE_STORAGE_NODE *)((orxSTRUCTURE *)_pStructure)->hStorageNode;
  pstParentNode = (orxSTRUCTURE_STORAGE_NODE *)((orxSTRUCTURE *)_pParent)->hStorageNode;

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
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Storage type is not tree type.");

      /* Not done */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Invalid parameter storage nodes.");

    /* Not done */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}
