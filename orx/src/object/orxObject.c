/***************************************************************************
 orxObject.c
 Object module
 
 begin                : 01/12/2003
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


#include "object/orxObject.h"

#include "debug/orxDebug.h"
#include "display/orxGraphic.h"
#include "object/orxFrame.h"
#include "memory/orxMemory.h"


/*
 * Platform independant defines
 */

#define orxOBJECT_KU32_FLAG_NONE                0x00000000
#define orxOBJECT_KU32_FLAG_READY               0x00000001

#define orxOBJECT_KU32_PROPERTY_FLAG_NONE       0x00000000


/*
 * Object structure
 */
struct __orxOBJECT_t
{
  /* Public structure, first structure member : 16 */
  orxSTRUCTURE stStructure;

  /* Used structures : 20 */
  orxSTRUCTURE *pastStructure[orxSTRUCTURE_ID_LINKABLE_NUMBER];

  /* Property flags : 24*/
  orxU32 u32Flags;

  /* Padding */
  orxPAD(24)
};

/*
 * Static structure
 */
typedef struct __orxOBJECT_STATIC_t
{
  /* Control flags */
  orxU32 u32Flags;

} orxOBJECT_STATIC;

/*
 * Static data
 */
orxSTATIC orxOBJECT_STATIC sstObject;


/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 orxObject_DeleteAll
 Deletes all objects.

 returns: orxVOID
 ***************************************************************************/
orxSTATIC orxINLINE orxVOID orxObject_DeleteAll()
{
  orxOBJECT *pstObject;
  
  /* Gets first object */
  pstObject = (orxOBJECT *)orxStructure_GetFirst(orxSTRUCTURE_ID_OBJECT);

  /* Non empty? */
  while(pstObject != orxNULL)
  {
    /* Deletes object */
    orxObject_Delete(pstObject);

    /* Gets first object */
    pstObject = (orxOBJECT *)orxStructure_GetFirst(orxSTRUCTURE_ID_OBJECT);
  }

  return;
}

/***************************************************************************
 orxObject_UpdateAll
 Updates all objects.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFASTCALL orxObject_UpdateAll(orxCONST orxCLOCK_INFO *_pstClockInfo, orxVOID *_pstContext)
{
  orxOBJECT *pstObject;

  /* For all objects */
  for(pstObject = (orxOBJECT *)orxStructure_GetFirst(orxSTRUCTURE_ID_OBJECT);
      pstObject != orxNULL;
      pstObject = (orxOBJECT *)orxStructure_GetNext((orxSTRUCTURE *)pstObject))
  {
    orxU32 i;

    /* !!! TODO !!! */
    /* Updates culling infos before calling update subfunctions */

    /* For all linked structures */
    for(i = 0; i < orxSTRUCTURE_ID_LINKABLE_NUMBER; i++)
    {
      /* Is structure linked? */
      if(pstObject->pastStructure[i] != orxNULL)
      {
        /* Updates it */
        if(orxStructure_Update(pstObject->pastStructure[i], (orxSTRUCTURE *)pstObject, _pstClockInfo) == orxSTATUS_FAILURE)
        {
          /* !!! MSG !!! */
        }
      }
    }
  }

  return;
}


/***************************************************************************
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 orxObject_Setup
 Object module setup.

 returns: nothing
 ***************************************************************************/
orxVOID orxObject_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_OBJECT, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_OBJECT, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_OBJECT, orxMODULE_ID_FRAME);

  return;
}

/***************************************************************************
 orxObject_Init
 Inits object system.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 ***************************************************************************/
orxSTATUS orxObject_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstObject.u32Flags & orxOBJECT_KU32_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Set(&sstObject, 0, sizeof(orxOBJECT_STATIC));

    /* Registers structure type */
    eResult = orxSTRUCTURE_REGISTER(orxSTRUCTURE_ID_OBJECT, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxNULL);

    /* Initialized? */
    if(eResult == orxSTATUS_SUCCESS)
    {
      /* Inits Flags */
      sstObject.u32Flags = orxOBJECT_KU32_FLAG_READY;
    }
    else
    {
      /* !!! MSG !!! */
    }
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
 orxObject_Exit
 Exits from the object system.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxObject_Exit()
{
  /* Initialized? */
  if(sstObject.u32Flags & orxOBJECT_KU32_FLAG_READY)
  {
    /* Deletes object list */
    orxObject_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_OBJECT);

    /* Updates flags */
    sstObject.u32Flags &= ~orxOBJECT_KU32_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}

/***************************************************************************
 orxObject_Create
 Creates a new empty object.

 returns: Created object.
 ***************************************************************************/
orxOBJECT *orxObject_Create()
{
  orxOBJECT *pstObject;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_FLAG_READY);

  /* Creates object */
  pstObject = (orxOBJECT *)orxStructure_Create(orxSTRUCTURE_ID_OBJECT);

  /* Created? */
  if(pstObject != orxNULL)
  {
    /* Inits property flags */
    pstObject->u32Flags = orxOBJECT_KU32_PROPERTY_FLAG_NONE;
  }
  else
  {
    /* !!! MSG !!! */
  }

  return pstObject;
}

/***************************************************************************
 orxObject_Delete
 Deletes an object.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 ***************************************************************************/
orxSTATUS orxFASTCALL orxObject_Delete(orxOBJECT *_pstObject)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_FLAG_READY);
  orxASSERT(_pstObject != orxNULL);

  /* Not referenced? */
  if(orxStructure_GetRefCounter((orxSTRUCTURE *)_pstObject) == 0)
  {
    orxU32 i;

    /* Unlink all structures */
    for(i = 0; i < orxSTRUCTURE_ID_LINKABLE_NUMBER; i++)
    {
      orxObject_UnlinkStructure(_pstObject, (orxSTRUCTURE_ID)i);
    }

    /* Deletes structure */
    orxStructure_Delete((orxSTRUCTURE *)_pstObject);

    /* Frees object memory */
    orxMemory_Free(_pstObject);
  }
  else
  {
    /* !!! MSG !!! */

    /* Referenced by others */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxObject_LinkStructure
 Links a structure to an object given.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 ***************************************************************************/
orxSTATUS orxFASTCALL orxObject_LinkStructure(orxOBJECT *_pstObject, orxSTRUCTURE *_pstStructure)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;
  orxSTRUCTURE_ID eStructureID;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_FLAG_READY);
  orxASSERT(_pstObject != orxNULL);
  orxASSERT(_pstStructure != orxNULL);

  /* Gets structure id & offset */
  eStructureID = orxStructure_GetID(_pstStructure);

  /* Valid? */
  if(eStructureID < orxSTRUCTURE_ID_LINKABLE_NUMBER)
  {
    /* Unlink previous structure if needed */
    orxObject_UnlinkStructure(_pstObject, eStructureID);

    /* Updates structure reference counter */
    orxStructure_IncreaseCounter(_pstStructure);

    /* Links new structure to object */
    _pstObject->pastStructure[eStructureID] = _pstStructure;
  }
  else
  {
    /* !!! MSG !!! */

    /* Wrong structure ID */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxObject_UnlinkStructure
 Unlinks structure from an object given its ID.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFASTCALL orxObject_UnlinkStructure(orxOBJECT *_pstObject, orxSTRUCTURE_ID _eStructureID)
{
  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_FLAG_READY);
  orxASSERT(_pstObject != orxNULL);
  orxASSERT(_eStructureID < orxSTRUCTURE_ID_LINKABLE_NUMBER);

  /* Needs to be processed? */
  if(_pstObject->pastStructure[_eStructureID] != orxNULL)
  {
    orxSTRUCTURE *pstStructure;

    /* Gets referenced structure */
    pstStructure = _pstObject->pastStructure[_eStructureID];

    /* Decreases structure reference counter */
    orxStructure_DecreaseCounter(pstStructure);

    /* Unlinks structure */
    _pstObject->pastStructure[_eStructureID] = orxNULL;
  }

  return;
}


/* *** Structure accessors *** */


/***************************************************************************
 orxObject_GetStructure
 Gets a structure used by an object, given its structure ID.

 returns: pointer to the requested structure (must be cast correctly)
 ***************************************************************************/
orxSTRUCTURE *orxFASTCALL orxObject_GetStructure(orxCONST orxOBJECT *_pstObject, orxSTRUCTURE_ID _eStructureID)
{
  orxSTRUCTURE *pstStructure = orxNULL;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_FLAG_READY);
  orxASSERT(_pstObject != orxNULL);

  /* Offset is valid? */
  if(_eStructureID < orxSTRUCTURE_ID_LINKABLE_NUMBER)
  {
    /* Gets requested structure */
    pstStructure = _pstObject->pastStructure[_eStructureID];
  }

  /* Done ! */
  return pstStructure;
}    


/* *** render handling *** */


/***************************************************************************
 orxObject_IsRenderStatusClean
 Test object render status (TRUE : clean / orxFALSE : dirty)

 returns: orxTRUE (clean) / orxFALSE (dirty)
 ***************************************************************************/
orxBOOL orxFASTCALL orxObject_IsRenderStatusClean(orxCONST orxOBJECT *_pstObject)
{
  orxFRAME *pstFrame;
//  orxGRAPHIC *pstGraphic;
  orxBOOL bResult = orxTRUE;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_FLAG_READY);
  orxASSERT(_pstObject != orxNULL);

  /* Gets frame */
  pstFrame = (orxFRAME *)orxObject_GetStructure(_pstObject, orxSTRUCTURE_ID_FRAME);

  /* Valid? */
  if(pstFrame != orxNULL)
  {
    /* Is frame not clean? */
    if(orxFrame_IsRenderStatusClean(pstFrame) == orxFALSE)
    {
      /* Not clean */
      bResult = orxFALSE;
    }
//    else
//    {
//      /* Gets graphic */
//      pstGraphic = (orxGRAPHIC *)orxObject_GetStructure(_pstObject, orxSTRUCTURE_ID_GRAPHIC);
//
//      /* Valid? */
//      if(pstGraphic != orxNULL)
//      {
//        /* !!! TODO : polls the anim status */
//        if(graphic_render_status_ok(pstGraphic) == orxFALSE)
//        {
//          /* Not clean */
//          bResult = orxFALSE;
//        }
//      }
//    }
  }

  /* Done! */
  return bResult;
}
