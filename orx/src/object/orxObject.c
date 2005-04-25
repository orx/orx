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
#include "graph/graphic.h"
#include "object/orxFrame.h"
#include "memory/orxMemory.h"


/*
 * Platform independant defines
 */

#define orxOBJECT_KU32_FLAG_NONE                0x00000000
#define orxOBJECT_KU32_FLAG_READY               0x00000001

#define orxOBJECT_KU32_STRUCT_NUMBER            2

#define orxOBJECT_KU32_STRUCT_OFFSET_FRAME      0
#define orxOBJECT_KU32_STRUCT_OFFSET_GRAPHIC    1


/*
 * Object structure
 */
struct __orxOBJECT_t
{
  /* Public structure, first structure member : 16 */
  orxSTRUCTURE stStructure;

  /* Used structures ids : 20 */
  orxU32 u32LinkedStructures;

  /* Used structures : 28 */
  orxSTRUCTURE *pastStructure[orxOBJECT_KU32_STRUCT_NUMBER];
  
  /* 4 extra bytes of padding : 32 */
  orxU8 au8Unused[4];
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
 orxObject_GetStructureIndex
 Gets a structure index given its id

 returns: requested structure offset
 ***************************************************************************/
orxINLINE orxU32 orxObject_GetStructureIndex(orxSTRUCTURE_ID _eStructureID)
{
  /* Gets structure offset according to id */
  switch(_eStructureID)
  {
    /* Frame structure */
    case orxSTRUCTURE_ID_FRAME:
    
      return orxOBJECT_KU32_STRUCT_OFFSET_FRAME;

    /* Graphic structure */
    case orxSTRUCTURE_ID_GRAPHIC:
    
      return orxOBJECT_KU32_STRUCT_OFFSET_GRAPHIC;

    default:
    
      return orxU32_Undefined;
  }
}

/***************************************************************************
 orxObject_DeleteAll
 Deletes all objects.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxObject_DeleteAll()
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
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/


/***************************************************************************
 orxObject_Init
 Inits object system.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxObject_Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Not already Initialized? */
  if(!(sstObject.u32Flags & orxOBJECT_KU32_FLAG_READY))
  {
    /* Cleans control structure */
    orxMemory_Set(&sstObject, 0, sizeof(orxOBJECT_STATIC));

    /* Registers structure type */
    eResult = orxStructure_RegisterStorageType(orxSTRUCTURE_ID_OBJECT, orxSTRUCTURE_STORAGE_TYPE_LINKLIST);
  }
  else
  {
    /* !!! MSG !!! */

    /* Already initialized */
    eResult = orxSTATUS_FAILED;
  }

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
  pstObject = (orxOBJECT *)orxMemory_Allocate(sizeof(orxOBJECT), orxMEMORY_TYPE_MAIN);

  /* Created? */
  if(pstObject != orxNULL)
  {
    /* Cleans it */
    orxMemory_Set(pstObject, 0, sizeof(orxOBJECT));

    /* Inits structure */
    if(orxStructure_Setup((orxSTRUCTURE *)pstObject, orxSTRUCTURE_ID_OBJECT) == orxSTATUS_SUCCESS)
    {
      /* Inits structure flags */
      pstObject->u32LinkedStructures = 0;
    }
    else
    {
      /* !!! MSG !!! */

      /* Fress partially allocated object */
      orxMemory_Free(pstObject);

      /* Not created */
      pstObject = orxNULL;
    }
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

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxObject_Delete(orxOBJECT *_pstObject)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;
  orxU32    i;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_FLAG_READY);
  orxASSERT(_pstObject != orxNULL);

  /* Not referenced? */
  if(orxStructure_GetRefCounter((orxSTRUCTURE *)_pstObject) == 0)
  {
    /* Unlink all structures */
    for(i = 0; i < orxSTRUCTURE_ID_NUMBER; i++)
    {
      orxObject_UnlinkStructure(_pstObject, (orxSTRUCTURE_ID)i);
    }

    /* Cleans structure */
    orxStructure_Clean((orxSTRUCTURE *)_pstObject);

    /* Frees object memory */
    orxMemory_Free(_pstObject);
  }
  else
  {
    /* !!! MSG !!! */

    /* Referenced by others */
    eResult = orxSTATUS_FAILED;
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxObject_LinkStructure
 Links a structure to an object given.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxObject_LinkStructure(orxOBJECT *_pstObject, orxSTRUCTURE *_pstStructure)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;
  orxSTRUCTURE_ID eStructureID;
  orxU32 u32StructureIndex;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_FLAG_READY);
  orxASSERT(_pstObject != orxNULL);
  orxASSERT(_pstStructure != orxNULL);

  /* Gets structure id & offset */
  eStructureID      = orxStructure_GetID(_pstStructure);
  u32StructureIndex = orxObject_GetStructureIndex(eStructureID);

  /* Valid? */
  if(u32StructureIndex != orxU32_Undefined)
  {
    /* Unlink previous structure if needed */
    orxObject_UnlinkStructure(_pstObject, eStructureID);

    /* Updates structure reference counter */
    orxStructure_IncreaseCounter(_pstStructure);

    /* Links new structure to object */
    _pstObject->pastStructure[u32StructureIndex] = _pstStructure;
    _pstObject->u32LinkedStructures |= (1 << eStructureID);
  }
  else
  {
    /* !!! MSG !!! */

    /* Wrong structure ID */
    eResult = orxSTATUS_FAILED;
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxObject_UnlinkStructure
 Unlinks structure from an object given its ID.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxObject_UnlinkStructure(orxOBJECT *_pstObject, orxSTRUCTURE_ID _eStructureID)
{
  orxSTRUCTURE *pstStructure;
  orxU32 u32ID, u32StructureIndex;

   /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_FLAG_READY);
  orxASSERT(_pstObject != orxNULL);

  /* Gets used struct ids */
  u32ID = (1 << _eStructureID);

  /* Needs to be processed? */
  if(u32ID & _pstObject->u32LinkedStructures)
  {
    /* Gets structure index */
    u32StructureIndex = orxObject_GetStructureIndex(_eStructureID);

    /* Decreases structure reference counter */
    pstStructure      = _pstObject->pastStructure[u32StructureIndex];
    orxStructure_DecreaseCounter(pstStructure);

    /* Unlinks structure */
    _pstObject->pastStructure[u32StructureIndex] = orxNULL;

    /* Updates structures ids */
    _pstObject->u32LinkedStructures &= ~u32ID;
  }

  return;
}


/* *** Structure accessors *** */


/***************************************************************************
 orxObject_GetStructure
 Gets a structure used by an object, given its structure ID.

 returns: pointer to the requested structure (must be cast correctly)
 ***************************************************************************/
orxSTRUCTURE *orxObject_GetStructure(orxOBJECT *_pstObject, orxSTRUCTURE_ID _eStructureID)
{
  orxSTRUCTURE *pstStructure = orxNULL;
  orxU32 u32StructureIndex;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_FLAG_READY);
  orxASSERT(_pstObject != orxNULL);

  /* Gets offset */
  u32StructureIndex = orxObject_GetStructureIndex(_eStructureID);

  /* Offset is valid? */
  if(u32StructureIndex != orxU32_Undefined)
  {
    /* Gets requested structure */
    pstStructure = _pstObject->pastStructure[u32StructureIndex];
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
orxBOOL orxObject_IsRenderStatusClean(orxOBJECT *_pstObject)
{
  orxFRAME *pstFrame;
  graphic_st_graphic *pstGraphic;
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
    else
    {
      /* Gets graphic */
      pstGraphic = (graphic_st_graphic *)orxObject_GetStructure(_pstObject, orxSTRUCTURE_ID_GRAPHIC);

      /* Valid? */
      if(pstGraphic != orxNULL)
      {
        /* Is graphic not clean? */
        if(graphic_render_status_ok(pstGraphic) == orxFALSE)
        {
          /* Not clean */
          bResult = orxFALSE;
        }
      }
    }
  }

  /* Done! */
  return bResult;
}
