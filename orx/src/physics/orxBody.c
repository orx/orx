/**
 * @file orxBody.c
 * 
 * Body module
 * 
 */

 /***************************************************************************
 orxBody.c
 Body module
 
 begin                : 05/02/2008
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


#include "physics/orxBody.h"

#include "debug/orxDebug.h"
#include "memory/orxMemory.h"


/** Module flags
 */

#define orxBODY_KU32_STATIC_FLAG_NONE       0x00000000

#define orxBODY_KU32_STATIC_FLAG_READY      0x00000001


#define orxBODY_KU32_MASK_ALL               0xFFFFFFFF  /**< All flags */


#define orxBODY_KC_MARKER_START             '$'
#define orxBODY_KC_MARKER_WIDTH             'w'
#define orxBODY_KC_MARKER_HEIGHT            'h'


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Body part structure
 */
typedef struct __orxBODY_PART_t
{
  orxSTRUCTURE *pstData;                                    /**< Data structure : 4 */
  orxU16        u16SelfFlags;                               /**< Self defining flags : 6 */
  orxU16        u16CheckMask;                               /**< Check mask : 8 */

  orxPAD(8);

} orxBODY_PART;

/** Body structure
 */
struct __orxBODY_t
{
  orxSTRUCTURE  stStructure;                                /**< Public structure, first structure member : 16 */
  orxBODY_PART  astDataList[orxBODY_KU32_PART_MAX_NUMBER];  /**< Body part structure list : 48 */

  orxPAD(48)
};

/** Static structure
 */
typedef struct __orxBODY_STATIC_t
{
  orxU32 u32Flags;                                          /**< Control flags : 4 */

} orxBODY_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
orxSTATIC orxBODY_STATIC sstBody;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Deletes all bodys
 */
orxSTATIC orxINLINE orxVOID orxBody_DeleteAll()
{
  orxREGISTER orxBODY *pstBody;

  /* Gets first body */
  pstBody = (orxBODY *)orxStructure_GetFirst(orxSTRUCTURE_ID_BODY);

  /* Non empty? */
  while(pstBody != orxNULL)
  {
    /* Deletes Body */
    orxBody_Delete(pstBody);

    /* Gets first Body */
    pstBody = (orxBODY *)orxStructure_GetFirst(orxSTRUCTURE_ID_BODY);
  }

  return;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Body module setup
 */
orxVOID orxBody_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_BODY, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_BODY, orxMODULE_ID_STRUCTURE);

  return;
}

/** Inits the Body module
 */
orxSTATUS orxBody_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;
  
  /* Not already Initialized? */
  if((sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY) == orxBODY_KU32_STATIC_FLAG_NONE)
  {
    /* Cleans static controller */
    orxMemory_Set(&sstBody, 0, sizeof(orxBODY_STATIC));

    /* Registers structure type */
    eResult = orxSTRUCTURE_REGISTER(BODY, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxNULL);
  }
  else
  {
    /* !!! MSG !!! */

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }
  
  /* Initialized? */
  if(eResult == orxSTATUS_SUCCESS)
  {
    /* Inits Flags */
    sstBody.u32Flags = orxBODY_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return eResult;
}

/** Exits from the Body module
 */
orxVOID orxBody_Exit()
{
  /* Initialized? */
  if(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY)
  {
    /* Deletes body list */
    orxBody_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_BODY);

    /* Updates flags */
    sstBody.u32Flags &= ~orxBODY_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}

/** Creates an empty body
 * @param[in]   _u32Flags                     Body flags (2D / ...)
 * @return      Created orxBODY / orxNULL
 */
orxBODY *orxFASTCALL orxBody_Create(orxU32 _u32Flags)
{
  orxBODY *pstBody;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxASSERT((_u32Flags & orxBODY_KU32_MASK_USER_ALL) == _u32Flags);

  /* Creates body */
  pstBody = (orxBODY *)orxStructure_Create(orxSTRUCTURE_ID_BODY);

  /* Valid? */
  if(pstBody != orxNULL)
  {
    /* Inits flags */
    orxStructure_SetFlags(pstBody, orxBODY_KU32_FLAG_NONE, orxBODY_KU32_MASK_ALL);

    /* 2D? */
    if(orxStructure_TestFlags(pstBody, orxBODY_KU32_FLAG_2D) != orxFALSE)
    {
      /* Updates flags */
      orxStructure_SetFlags(pstBody, orxBODY_KU32_FLAG_2D, orxBODY_KU32_FLAG_NONE);
    }
  }

  /* Done! */
  return pstBody;
}

/** Deletes a body
 * @param[in]   _pstBody     Body to delete
 */
orxSTATUS orxFASTCALL orxBody_Delete(orxBODY *_pstBody)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstBody);

  /* Not referenced? */
  if(orxStructure_GetRefCounter(_pstBody) == 0)
  {
    orxU32 i;

    /* For all data structure */
    for(i = 0; i < orxBODY_KU32_PART_MAX_NUMBER; i++)
    {
      /* Cleans it */
      orxBody_SetPartData(_pstBody, i, orxNULL, 0, 0);
    }

    /* Deletes structure */
    orxStructure_Delete(_pstBody);
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

/** Sets body part data
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _u32Index       Data index (should be less than orxBODY_KU32_DATA_MAX_NUMBER)
 * @param[in]   _pstData        Data structure to set / orxNULL
 * @param[in]   _u16SelfFlags   Self defining flags
 * @param[in]   _u16CheckMask   Mask to check against other body parts
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxBody_SetPartData(orxBODY *_pstBody, orxU32 _u32Index, orxSTRUCTURE *_pstData, orxU16 _u16SelfFlags, orxU16 _u16CheckMask)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstBody);
  orxASSERT(_u32Index < orxBODY_KU32_PART_MAX_NUMBER);

  /* Had previously data? */
  if(_pstBody->astDataList[_u32Index].pstData != orxNULL)
  {
    /* Updates structure reference counter */
    orxStructure_DecreaseCounter(_pstBody->astDataList[_u32Index].pstData);

    /* Stores flags & mask */
    _pstBody->astDataList[_u32Index].u16SelfFlags = _u16SelfFlags;
    _pstBody->astDataList[_u32Index].u16CheckMask = _u16CheckMask;

    /* 2D data ? */
    if(orxStructure_TestFlags(_pstBody, orxBODY_KU32_FLAG_2D))
    {
      /* !!! TODO !!! */
    }
    else
    {
      /* !!! MSG !!! */

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }

    /* Cleans reference */
    _pstBody->astDataList[_u32Index].pstData = orxNULL;
  }

  /* Valid & sets new data? */
  if((eResult != orxSTATUS_FAILURE) && (_pstData != orxNULL))
  {
    /* Stores it */
    _pstBody->astDataList[_u32Index].pstData = _pstData;

    /* Updates structure reference counter */
    orxStructure_IncreaseCounter(_pstData);

    /* !!! TODO : Update internal flags given data type */
  }

  /* Done! */
  return eResult;
}

/** Gets body part data
 * @param[in]   _pstBody      Concerned body
 * @param[in]   _u32Index     Data index (should be less than orxBODY_KU32_DATA_MAX_NUMBER)
 * @return      OrxSTRUCTURE / orxNULL
 */
orxSTRUCTURE *orxFASTCALL orxBody_GetPartData(orxCONST orxBODY *_pstBody, orxU32 _u32Index)
{
  orxSTRUCTURE *pstStructure;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstBody);
  orxASSERT(_u32Index < orxBODY_KU32_PART_MAX_NUMBER);

  /* Updates result */
  pstStructure = _pstBody->astDataList[_u32Index].pstData;

  /* Done! */
  return pstStructure;
}

/** Gets body part self flags
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _u32Index       Data index (should be less than orxBODY_KU32_DATA_MAX_NUMBER)
 * @return      Body part self flags / orxU16_UNDEFINED
 */
orxU16 orxFASTCALL orxBody_GetPartSelfFlags(orxCONST orxBODY *_pstBody, orxU32 _u32Index)
{
  orxU16 u16Result = orxU16_UNDEFINED;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstBody);
  orxASSERT(_u32Index < orxBODY_KU32_PART_MAX_NUMBER);

  /* Is a valid part? */
  if(_pstBody->astDataList[_u32Index].pstData != orxNULL)
  {
    /* Updates result */
    u16Result = _pstBody->astDataList[_u32Index].u16SelfFlags;
  }

  /* Done! */
  return u16Result;
}

/** Gets body part self flags
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _u32Index       Data index (should be less than orxBODY_KU32_DATA_MAX_NUMBER)
 * @return      Body part check mask / orxU16_UNDEFINED
 */
orxU16 orxFASTCALL orxBody_GetPartCheckMask(orxCONST orxBODY *_pstBody, orxU32 _u32Index)
{
  orxU16 u16Result = orxU16_UNDEFINED;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstBody);
  orxASSERT(_u32Index < orxBODY_KU32_PART_MAX_NUMBER);

  /* Is a valid part? */
  if(_pstBody->astDataList[_u32Index].pstData != orxNULL)
  {
    /* Updates result */
    u16Result = _pstBody->astDataList[_u32Index].u16CheckMask;
  }

  /* Done! */
  return u16Result;
}
