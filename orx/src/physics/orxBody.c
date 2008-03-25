/***************************************************************************
 orxBody.c
 Body module
 
 begin                : 10/03/2008
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
#include "physics/orxPhysics.h"

#include "debug/orxDebug.h"
#include "memory/orxMemory.h"


/** Body flags
 */
#define orxBODY_KU32_FLAG_NONE                0x00000000  /**< No flags */

#define orxBODY_KU32_FLAG_HAS_DATA            0x00000001  /**< Has data flag */

#define orxBODY_KU32_MASK_USER_ALL            0xFFFFFFFF  /**< User all ID mask */


/** Module flags
 */
#define orxBODY_KU32_STATIC_FLAG_NONE         0x00000000  /**< No flags */

#define orxBODY_KU32_STATIC_FLAG_READY        0x10000000  /**< Ready flag */

#define orxBODY_KU32_MASK_ALL                 0xFFFFFFFF  /**< All mask */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Body part structure
 */
struct __orxBODY_PART_t
{
  orxPHYSICS_BODY_PART *pstData;                                /**< Data structure : 4 */

  orxPAD(4)
};

/** Body structure
 */
struct __orxBODY_t
{
  orxSTRUCTURE      stStructure;                                /**< Public structure, first structure member : 16 */
  orxBODY_PART      astPartList[orxBODY_KU32_PART_MAX_NUMBER];  /**< Body part structure list : 32 */
  orxPHYSICS_BODY  *pstData;                                    /**< Physics body data : 36 */

  orxPAD(36)
};

/** Static structure
 */
typedef struct __orxBODY_STATIC_t
{
  orxU32 u32Flags;                                              /**< Control flags : 4 */

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

/** Deletes all bodies
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

/** Updates the Body (Callback for generic structure update calling)
 * @param[in]   _pstStructure                 Generic Structure or the concerned Body
 * @param[in]   _pstCaller                    Structure of the caller
 * @param[in]   _pstClockInfo                 Clock info used for time updates
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATIC orxSTATUS orxFASTCALL orxBody_Update(orxSTRUCTURE *_pstStructure, orxCONST orxSTRUCTURE *_pstCaller, orxCONST orxCLOCK_INFO *_pstClockInfo)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  //! TODO : Process & forwards events sent by physics sensor zones

  /* Done! */
  return eResult;
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
  orxModule_AddDependency(orxMODULE_ID_BODY, orxMODULE_ID_PHYSICS);

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
    eResult = orxSTRUCTURE_REGISTER(BODY, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, &orxBody_Update);
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
 * @param[in]   _pstBodyDef                   Body definition
 * @return      Created orxBODY / orxNULL
 */
orxBODY *orxFASTCALL orxBody_Create(orxCONST orxBODY_DEF *_pstBodyDef)
{
  orxBODY *pstBody;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyDef != orxNULL);

  /* Creates body */
  pstBody = (orxBODY *)orxStructure_Create(orxSTRUCTURE_ID_BODY);

  /* Valid? */
  if(pstBody != orxNULL)
  {
    /* Inits flags */
    orxStructure_SetFlags(pstBody, orxBODY_KU32_FLAG_NONE, orxBODY_KU32_MASK_ALL);

    /* Creates physics body */
    pstBody->pstData = orxPhysics_CreateBody(_pstBodyDef);

    /* Valid? */
    if(pstBody->pstData != orxNULL)
    {
      /* Updates flags */
      orxStructure_SetFlags(pstBody, orxBODY_KU32_FLAG_HAS_DATA, orxBODY_KU32_FLAG_NONE);
    }
    else
    {
      /* !!! MSG !!! */

      /* Deletes allocated structure */
      orxStructure_Delete(pstBody);
      pstBody = orxNULL;
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
      orxBody_RemovePart(_pstBody, i);
    }

    /* Has data? */
    if(orxStructure_TestFlags(_pstBody, orxBODY_KU32_FLAG_HAS_DATA))
    {
      /* Deletes physics body */
      orxPhysics_DeleteBody(_pstBody->pstData);
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

/** Adds a body part
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _u32Index       Part index (should be less than orxBODY_KU32_PART_MAX_NUMBER)
 * @param[in]   _pstPartDef     Body part definition
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxBody_AddPart(orxBODY *_pstBody, orxU32 _u32Index, orxBODY_PART_DEF *_pstPartDef)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstBody);
  orxASSERT(_pstPartDef != orxNULL);
  orxASSERT(_u32Index < orxBODY_KU32_PART_MAX_NUMBER);

  /* Had previous part? */
  if(_pstBody->astPartList[_u32Index].pstData != orxNULL)
  {
    /* Removes it */
    eResult = orxBody_RemovePart(_pstBody, _u32Index);
  }

  /* Valid? */
  if(eResult != orxSTATUS_FAILURE)
  {
    //! TODO
  }

  /* Done! */
  return eResult;
}

/** Gets a body part
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _u32Index       Body part index (should be less than orxBODY_KU32_DATA_MAX_NUMBER)
 * @return      orxBODY_PART / orxNULL
 */
orxBODY_PART *orxFASTCALL orxBody_GetPart(orxCONST orxBODY *_pstBody, orxU32 _u32Index)
{
  orxBODY_PART *pstResult = orxNULL;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstBody);
  orxASSERT(_u32Index < orxBODY_KU32_PART_MAX_NUMBER);

  /* Updates result */
  if(_pstBody->astPartList[_u32Index].pstData != orxNULL)
  {
    pstResult = (orxBODY_PART *)&(_pstBody->astPartList[_u32Index]);
  }

  /* Done! */
  return pstResult;
}

/** Removes a body part
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _u32Index       Part index (should be less than orxBODY_KU32_DATA_MAX_NUMBER)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxBody_RemovePart(orxBODY *_pstBody, orxU32 _u32Index)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Has a part? */
  if(_pstBody->astPartList[_u32Index].pstData != orxNULL)
  {
    //! TODO

    /* Deletes reference */
    _pstBody->astPartList[_u32Index].pstData = orxNULL;
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets body part self flags
 * @param[in]   _pstBodyPart    Concerned body part
 * @return      Body part self flags / orxU16_UNDEFINED
 */
orxU16 orxFASTCALL orxBody_GetPartSelfFlags(orxCONST orxBODY_PART *_pstBodyPart)
{
  orxU16 u16Result = orxU16_UNDEFINED;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyPart != orxNULL);

  /* Updates result */
  //! TODO

  /* Done! */
  return u16Result;
}

/** Gets body part check mask
 * @param[in]   _pstBodyPart    Concerned body part
 * @return      Body part check mask / orxU16_UNDEFINED
 */
orxU16 orxFASTCALL orxBody_GetPartCheckMask(orxCONST orxBODY_PART *_pstBodyPart)
{
  orxU16 u16Result = orxU16_UNDEFINED;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyPart != orxNULL);

  /* Updates result */
  //! TODO

  /* Done! */
  return u16Result;
}

/** Sets a body position
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _pvPosition     Position to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxBody_SetPosition(orxBODY *_pstBody, orxCONST orxVECTOR *_pvPosition)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstBody);
  orxASSERT(_pvPosition != orxNULL);

  /* Has data? */
  if(orxStructure_TestFlags(_pstBody, orxBODY_KU32_FLAG_HAS_DATA))
  {
    //! TODO
  }
  else
  {
    /* !!! MSG !!! */

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets a body rotation
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _fRotation      Rotation to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxBody_SetRotation(orxBODY *_pstBody, orxFLOAT _fRotation)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstBody);

  /* Has data? */
  if(orxStructure_TestFlags(_pstBody, orxBODY_KU32_FLAG_HAS_DATA))
  {
    //! TODO
  }
  else
  {
    /* !!! MSG !!! */

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}
