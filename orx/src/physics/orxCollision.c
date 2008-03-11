/**
 * @file orxCollision.c
 * 
 * Collision module
 * 
 */

 /***************************************************************************
 orxCollision.c
 Collision module
 
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


#include "physics/orxCollision.h"

#include "debug/orxDebug.h"
#include "memory/orxMemory.h"


/** Module flags
 */

#define orxCOLLISION_KU32_STATIC_FLAG_NONE  0x00000000

#define orxCOLLISION_KU32_STATIC_FLAG_READY 0x10000000


#define orxCOLLISION_KU32_MASK_ALL          0xFFFFFFFF  /**< All flags */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Collision structure
 */
typedef struct __orxCOLLISION_t
{
  orxSTRUCTURE  stStructure;                                /**< Public structure, first structure member : 16 */

  orxPAD(16)
};

/** Static structure
 */
typedef struct __orxCOLLISION_STATIC_t
{
  orxU32 u32Flags;                                          /**< Control flags : 4 */

} orxCOLLISION_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
orxSTATIC orxCOLLISION_STATIC sstCollision;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Deletes all collisions
 */
orxSTATIC orxINLINE orxVOID orxCollision_DeleteAll()
{
  orxREGISTER orxCOLLISION *pstCollision;

  /* Gets first collision */
  pstCollision = (orxCOLLISION *)orxStructure_GetFirst(orxSTRUCTURE_ID_COLLISION);

  /* Non empty? */
  while(pstCollision != orxNULL)
  {
    /* Deletes Collision */
    orxCollision_Delete(pstCollision);

    /* Gets first Collision */
    pstCollision = (orxCOLLISION *)orxStructure_GetFirst(orxSTRUCTURE_ID_COLLISION);
  }

  return;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Collision module setup
 */
orxVOID orxCollision_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_COLLISION, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_COLLISION, orxMODULE_ID_STRUCTURE);

  return;
}

/** Inits the Collision module
 */
orxSTATUS orxCollision_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;
  
  /* Not already Initialized? */
  if((sstCollision.u32Flags & orxCOLLISION_KU32_STATIC_FLAG_READY) == orxCOLLISION_KU32_STATIC_FLAG_NONE)
  {
    /* Cleans static controller */
    orxMemory_Set(&sstCollision, 0, sizeof(orxCOLLISION_STATIC));

    /* Registers structure type */
    eResult = orxSTRUCTURE_REGISTER(COLLISION, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxNULL);
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
    sstCollision.u32Flags = orxCOLLISION_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return eResult;
}

/** Exits from the Collision module
 */
orxVOID orxCollision_Exit()
{
  /* Initialized? */
  if(sstCollision.u32Flags & orxCOLLISION_KU32_STATIC_FLAG_READY)
  {
    /* Deletes collision list */
    orxCollision_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_COLLISION);

    /* Updates flags */
    sstCollision.u32Flags &= ~orxCOLLISION_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}

/** Creates a collision
 * @param[in]   _u32Flags                     Collision flags (2D / shape / box / adaptive box / ...)
 * @return      Created orxCOLLISION / orxNULL
 */
orxCOLLISION *orxFASTCALL orxCollision_Create(orxU32 _u32Flags)
{
  orxCOLLISION *pstCollision;

  /* Checks */
  orxASSERT(sstCollision.u32Flags & orxCOLLISION_KU32_STATIC_FLAG_READY);
  orxASSERT((_u32Flags & orxCOLLISION_KU32_MASK_USER_ALL) == _u32Flags);

  /* Creates collision */
  pstCollision = (orxCOLLISION *)orxStructure_Create(orxSTRUCTURE_ID_COLLISION);

  /* Valid? */
  if(pstCollision != orxNULL)
  {
    /* Inits flags */
    orxStructure_SetFlags(pstCollision, orxCOLLISION_KU32_FLAG_NONE, orxCOLLISION_KU32_MASK_ALL);

    /* 2D? */
    if(orxFLAG_TEST(_u32Flags, orxCOLLISION_KU32_FLAG_2D))
    {
      /* !!! TODO !!! */

      /* Updates flags */
      orxStructure_SetFlags(pstCollision, orxCOLLISION_KU32_FLAG_2D, orxCOLLISION_KU32_FLAG_NONE);
    }
  }

  /* Done! */
  return pstCollision;
}

/** Deletes a collision
 * @param[in]   _pstCollision     Collision to delete
 */
orxSTATUS orxFASTCALL orxCollision_Delete(orxCOLLISION *_pstCollision)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstCollision.u32Flags & orxCOLLISION_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstCollision);

  /* Not referenced? */
  if(orxStructure_GetRefCounter(_pstCollision) == 0)
  {
    /* Deletes structure */
    orxStructure_Delete(_pstCollision);
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
