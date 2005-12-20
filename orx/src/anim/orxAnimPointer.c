/**
 * @file orxAnimPointer.c
 * 
 * Animation (Pointer) module
 * 
 */

 /***************************************************************************
 orxAnimPointer.c
 Animation (Pointer) module
 
 begin                : 03/03/2004
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


#include "anim/orxAnimPointer.h"

#include "debug/orxDebug.h"
#include "memory/orxMemory.h"


/** Module flags
 */
#define orxANIMPOINTER_KU32_FLAG_NONE                 0x00000000  /**< No flags */

#define orxANIMPOINTER_KU32_FLAG_READY                0x00000001  /**< Ready flag */


/** orxANIMPOINTER ID flags
 */
#define orxANIMPOINTER_KU32_ID_FLAG_NONE              0x00000000  /**< No ID flags */

#define orxANIMPOINTER_KU32_ID_FLAG_HAS_CURRENT_ANIM  0x01000000  /**< Has current animation flag */
#define orxANIMPOINTER_KU32_ID_FLAG_HAS_NEXT_ANIM     0x02000000  /**< Has next animation flag */
#define orxANIMPOINTER_KU32_ID_FLAG_ANIMSET           0x10000000  /**< Has animset flag */
#define orxANIMPOINTER_KU32_ID_FLAG_LINK_TABLE        0x20000000  /**< Has link table flag */
#define orxANIMPOINTER_KU32_ID_FLAG_PAUSED            0x40000000  /**< Pause flag */

#define orxANIMPOINTER_KU32_ID_MASK_FLAGS             0xFFFF0000  /**< Flags ID mask */


/** Module constants
 */
#define orxANIMPOINTER_KF_FREQUENCY_DEFAULT           1.0         /**< Default animation frequency */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** AnimPointer structure
 */
struct __orxANIMPOINTER_t
{
  orxSTRUCTURE            stStructure;                /**< Public structure, first structure member : 16 */
  orxU32                  u32IDFlags;                 /**< ID flags : 20 */
  orxANIMSET             *pstAnimSet;                 /**< Referenced AnimationSet : 24 */
  orxANIMSET_LINK_TABLE  *pstLinkTable;               /**< Link table pointer : 28 */
  orxHANDLE               hCurrentAnim;               /**< Current animation ID : 32 */
  orxHANDLE               hDstAnim;                   /**< Destination animation ID : 36 */
  orxU32                  u32CurrentAnimTime;         /**< Current Time (Relative to current animation) : 40 */
  orxU32                  u32Time;                    /**< Current Time (Absolute) : 44 */
  orxFLOAT                fFrequency;                 /**< Current animation frequency : 48 */

  orxPAD(48)
};


/** Static structure
 */
typedef struct __orxANIMPOINTER_STATIC_t
{
  orxU32 u32Flags;                                    /**< Control flags : 4 */

} orxANIMPOINTER_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
orxSTATIC orxANIMPOINTER_STATIC sstAnimPointer;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Deletes all AnimPointers
 */
orxSTATIC orxVOID orxAnimPointer_DeleteAll()
{
  orxREGISTER orxANIMPOINTER *pstAnimPointer;
  
  /* Gets first anim pointer */
  pstAnimPointer = (orxANIMPOINTER *)orxStructure_GetFirst(orxSTRUCTURE_ID_ANIMPOINTER);

  /* Non empty? */
  while(pstAnimPointer != orxNULL)
  {
    /* Deletes AnimPointer */
    orxAnimPointer_Delete(pstAnimPointer);

    /* Gets first Animation Set */
    pstAnimPointer = (orxANIMPOINTER *)orxStructure_GetFirst(orxSTRUCTURE_ID_ANIMPOINTER);
  }

  return;
}

/** Computes current Anim for the given time
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @param[in]   _u32Time                      Time to use for current Anim computation
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATIC orxINLINE orxSTATUS orxAnimPointer_Compute(orxANIMPOINTER *_pstAnimPointer, orxU32 _u32Time)
{
  orxHANDLE hNewAnim;
  orxU32 u32DT;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(_pstAnimPointer != orxNULL);
  orxASSERT(orxAnimPointer_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_ID_FLAG_ANIMSET) != orxFALSE);

  /* Not Paused? */
  if(orxAnimPointer_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_ID_FLAG_PAUSED) == orxFALSE)
  {
    /* Has current animation */
    if(orxAnimPointer_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_ID_FLAG_HAS_CURRENT_ANIM) != orxFALSE)
    {
      /* Computes TimeDT */
      u32DT = orxF2U(orxU2F(_u32Time - _pstAnimPointer->u32Time) * _pstAnimPointer->fFrequency);

      /* Updates Times */
      _pstAnimPointer->u32Time = _u32Time;
      _pstAnimPointer->u32CurrentAnimTime += u32DT;

      /* Computes & updates anim*/
      hNewAnim = orxAnimSet_ComputeAnim(_pstAnimPointer->pstAnimSet, _pstAnimPointer->hCurrentAnim, _pstAnimPointer->hDstAnim, &(_pstAnimPointer->u32CurrentAnimTime), _pstAnimPointer->pstLinkTable);
    
      /* Change happened? */
      if(hNewAnim != _pstAnimPointer->hCurrentAnim)
      {
        /* Updates anim handle */
        _pstAnimPointer->hCurrentAnim = hNewAnim;

        /* No next anim? */
        if(hNewAnim == orxHANDLE_Undefined)
        {
          /* Updates flags */
          orxAnimPointer_SetFlags(_pstAnimPointer, orxANIMPOINTER_KU32_ID_FLAG_NONE, orxANIMPOINTER_KU32_ID_FLAG_HAS_CURRENT_ANIM);
        }
      }
    }
    else
    {
      /* !!! MSG !!! */

      /* Can't process */
      eResult = orxSTATUS_FAILURE;
    }
  }

  /* Done! */
  return eResult;
}

/** Updates the AnimPointer (Callback for generic structure update calling)
 * @param[in]   _pstStructure                 Generic Structure or the concerned AnimPointer
 * @param[in]   _pstCaller                    Structure of the caller
 * @param[in]   _pstClockInfo                 Clock info used for time updates
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATIC orxSTATUS orxFASTCALL orxAnimPointer_Update(orxSTRUCTURE *_pstStructure, orxCONST orxSTRUCTURE *_pstCaller, orxCONST orxCLOCK_INFO *_pstClockInfo)
{
  orxREGISTER orxANIMPOINTER *pstAnimPointer;

  /* Gets pointer */
  pstAnimPointer = orxSTRUCTURE_GET_POINTER(_pstStructure, ANIMPOINTER);

  /* Checks */
  orxASSERT(pstAnimPointer != orxNULL);

  /* Computes animation pointer */
  return(orxAnimPointer_Compute(pstAnimPointer, _pstClockInfo->u32Time));
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** AnimPointer module setup
 */
orxVOID orxAnimPointer_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_ANIMPOINTER, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_ANIMPOINTER, orxMODULE_ID_ANIMSET);
  orxModule_AddDependency(orxMODULE_ID_ANIMPOINTER, orxMODULE_ID_ANIM);

  return;
}

/** Inits the AnimPointer module
 */
orxSTATUS orxAnimPointer_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;
  
  /* Not already Initialized? */
  if(!(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_FLAG_READY))
  {
    orxSTRUCTURE_REGISTER_INFO stRegisterInfo;
  
    /* Cleans control structure */
    orxMemory_Set(&sstAnimPointer, 0, sizeof(orxANIMPOINTER_STATIC));

    /* Registers structure type */
    stRegisterInfo.eStorageType = orxSTRUCTURE_STORAGE_TYPE_LINKLIST;
    stRegisterInfo.u32Size      = sizeof(orxANIMPOINTER);
    stRegisterInfo.eMemoryType  = orxMEMORY_TYPE_MAIN;
    stRegisterInfo.pfnUpdate    = &orxAnimPointer_Update;

    eResult = orxStructure_Register(orxSTRUCTURE_ID_ANIMPOINTER, &stRegisterInfo);
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
    sstAnimPointer.u32Flags = orxANIMPOINTER_KU32_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return eResult;
}

/** Exits from the AnimPointer module
 */
orxVOID orxAnimPointer_Exit()
{
  /* Initialized? */
  if(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_FLAG_READY)
  {
    /* Deletes animpointer list */
    orxAnimPointer_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_ANIMPOINTER);

    /* Updates flags */
    sstAnimPointer.u32Flags &= ~orxANIMPOINTER_KU32_FLAG_READY;

  }
  
  return;
}

/** Creates an empty AnimPointer
 * @param[in]   _pstAnimSet                   AnimationSet reference
 * @return      Created orxANIMPOINTER / orxNULL
 */
orxANIMPOINTER *orxFASTCALL orxAnimPointer_Create(orxANIMSET *_pstAnimSet)
{
  orxANIMPOINTER *pstAnimPointer = orxNULL;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_FLAG_READY);
  orxASSERT(_pstAnimSet != orxNULL);

  /* Creates animpointer */
  pstAnimPointer = (orxANIMPOINTER *)orxStructure_Create(orxSTRUCTURE_ID_ANIMPOINTER);

  /* Was allocated? */
  if(pstAnimPointer != orxNULL)
  {
    /* Stores animset */
    pstAnimPointer->pstAnimSet = _pstAnimSet;

    /* Adds a reference on the animset */
    orxAnimSet_AddReference(_pstAnimSet);

    /* Inits flags */
    orxAnimPointer_SetFlags(pstAnimPointer, orxANIMPOINTER_KU32_ID_FLAG_ANIMSET | orxANIMPOINTER_KU32_ID_FLAG_HAS_CURRENT_ANIM, orxANIMPOINTER_KU32_ID_MASK_FLAGS);

    /* Inits value */
    pstAnimPointer->hCurrentAnim        = (orxHANDLE)0;
    pstAnimPointer->u32CurrentAnimTime  = 0;
    pstAnimPointer->fFrequency          = orxANIMPOINTER_KF_FREQUENCY_DEFAULT;
    pstAnimPointer->u32Time             = orxTime_GetTime();
    pstAnimPointer->hDstAnim            = orxHANDLE_Undefined;

    /* Is animset link table non-static? */
    if(orxAnimSet_TestFlags(_pstAnimSet, orxANIMSET_KU32_ID_FLAG_LINK_STATIC) == orxFALSE)
    {
      /* Stores link table */
      pstAnimPointer->pstLinkTable = orxAnimSet_CloneLinkTable(_pstAnimSet);

      /* Updates flags */
      orxAnimPointer_SetFlags(pstAnimPointer, orxANIMPOINTER_KU32_ID_FLAG_LINK_TABLE, orxANIMPOINTER_KU32_ID_FLAG_NONE);
    }
  }
  else
  {
    /* !!! MSG !!! */
    
    /* Not created */
    pstAnimPointer = orxNULL;
  }

  /* Done! */
  return pstAnimPointer;
}

/** Deletes an AnimPointer
 * @param[in]   _pstAnimPointer               AnimPointer to delete
 */
orxSTATUS orxFASTCALL orxAnimPointer_Delete(orxANIMPOINTER *_pstAnimPointer)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_FLAG_READY);
  orxASSERT(_pstAnimPointer != orxNULL);

  /* Not referenced? */
  if(orxStructure_GetRefCounter((orxSTRUCTURE *)_pstAnimPointer) == 0)
  {
    /* Has an animset? */
    if(orxAnimPointer_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_ID_FLAG_ANIMSET) != orxFALSE)
    {
      /* Removes the reference from the animset */
      orxAnimSet_RemoveReference(_pstAnimPointer->pstAnimSet);
    }

    /* Has a link table? */
    if(orxAnimPointer_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_ID_FLAG_LINK_TABLE) != orxFALSE)
    {
      /* Deletes it */
      orxAnimSet_DeleteLinkTable(_pstAnimPointer->pstLinkTable);
    }

    /* Deletes structure */
    orxStructure_Delete((orxSTRUCTURE *)_pstAnimPointer);
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

/** Gets the referenced AnimationSet
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      Referenced orxANIMSET
 */
orxANIMSET *orxFASTCALL orxAnimPointer_GetAnimSet(orxCONST orxANIMPOINTER *_pstAnimPointer)
{
  orxANIMSET *pstAnimSet = orxNULL;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_FLAG_READY);
  orxASSERT(_pstAnimPointer != orxNULL);

  /* Has an animset? */
  if(orxAnimPointer_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_ID_FLAG_ANIMSET) != orxFALSE)
  {
    pstAnimSet = _pstAnimPointer->pstAnimSet;
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return pstAnimSet;
}

/** AnimPointer current Animation get accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      Current Animation handle
 */
orxHANDLE orxFASTCALL orxAnimPointer_GetAnim(orxCONST orxANIMPOINTER *_pstAnimPointer)
{
  orxHANDLE hAnimHandle = orxHANDLE_Undefined;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_FLAG_READY);
  orxASSERT(_pstAnimPointer != orxNULL);

  /* Has anim? */
  if((orxAnimPointer_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_ID_FLAG_ANIMSET) != orxFALSE)
  && (orxAnimPointer_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_ID_FLAG_HAS_CURRENT_ANIM) != orxFALSE))
  {
    hAnimHandle = _pstAnimPointer->hCurrentAnim;
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return hAnimHandle;    
}

/** AnimPointer current Time get accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      Current time
 */
orxU32 orxFASTCALL orxAnimPointer_GetTime(orxCONST orxANIMPOINTER *_pstAnimPointer)
{
  orxU32 u32Result = 0;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_FLAG_READY);
  orxASSERT(_pstAnimPointer != orxNULL);

  /* Has anim? */
  if(orxAnimPointer_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_ID_FLAG_HAS_CURRENT_ANIM) != orxFALSE)
  {
    /* Gets time */
    u32Result = _pstAnimPointer->u32CurrentAnimTime;
  }
  else
  {
    /* !!! MSG !!! */
  }
  
  /* Done! */
  return u32Result;
}

/** AnimPointer Frequency get accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      AnimPointer frequency
 */
orxFLOAT orxFASTCALL orxAnimPointer_GetFrequency(orxCONST orxANIMPOINTER *_pstAnimPointer)
{
  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_FLAG_READY);
  orxASSERT(_pstAnimPointer != orxNULL);

  /* Gets frequency */
  return _pstAnimPointer->fFrequency;
}

/** AnimPointer current Animation set accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @param[in]   _hAnimHandle                  Animation handle to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnimPointer_SetAnim(orxANIMPOINTER *_pstAnimPointer, orxHANDLE _hAnimHandle)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;
  
  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_FLAG_READY);
  orxASSERT(_pstAnimPointer != orxNULL);

  /* Has Animset? */
  if(orxAnimPointer_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_ID_FLAG_ANIMSET) != orxFALSE)
  {
    /* In range? */
    if((orxU32)_hAnimHandle < orxAnimSet_GetAnimCounter(_pstAnimPointer->pstAnimSet))
    {
      /* Stores ID */
      _pstAnimPointer->hCurrentAnim = _hAnimHandle;

      /* Updates absolute timestamp */
      _pstAnimPointer->u32Time       = orxTime_GetTime();
    
      /* Updates flags */
      orxAnimPointer_SetFlags(_pstAnimPointer, orxANIMPOINTER_KU32_ID_FLAG_HAS_CURRENT_ANIM, orxANIMPOINTER_KU32_ID_FLAG_NONE);
    
      /* Computes animpointer */
      eResult = orxAnimPointer_Compute(_pstAnimPointer, _pstAnimPointer->u32Time);
    }
    else
    {
      /* !!! MSG !!! */

      /* Can't process */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* !!! MSG !!! */

    /* Can't process */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;    
}

/** AnimPointer current Time accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @param[in]   _u32Time                      Time to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnimPointer_SetTime(orxANIMPOINTER *_pstAnimPointer, orxU32 _u32Time)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_FLAG_READY);
  orxASSERT(_pstAnimPointer != orxNULL);

  /* Stores relative timestamp */
  _pstAnimPointer->u32CurrentAnimTime = _u32Time;
  
  /* Updates absolute timestamp */
  _pstAnimPointer->u32Time = orxTime_GetTime();

  /* Computes animpointer */
  eResult = orxAnimPointer_Compute(_pstAnimPointer, _pstAnimPointer->u32Time);

  /* Done! */
  return eResult;
}

/** AnimPointer Frequency set accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @param[in]   _fFrequency                   Frequency to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnimPointer_SetFrequency(orxANIMPOINTER *_pstAnimPointer, orxFLOAT _fFrequency)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_FLAG_READY);
  orxASSERT(_pstAnimPointer != orxNULL);
  orxASSERT(_fFrequency >= 0.0);

  /* Computes animpointer */
  eResult = orxAnimPointer_Compute(_pstAnimPointer, orxTime_GetTime());

  /* Succeeded? */
  if(eResult == orxSTATUS_SUCCESS)
  {
    /* Stores frequency */
    _pstAnimPointer->fFrequency = _fFrequency; 
  }

  /* Done! */
  return eResult;
}

/** AnimPointer flag test accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @param[in]   _u32Flags                     Flags to test
 * @return      orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxAnimPointer_TestFlags(orxCONST orxANIMPOINTER *_pstAnimPointer, orxU32 _u32Flags)
{
  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_FLAG_READY);
  orxASSERT(_pstAnimPointer != orxNULL);

  /* Tests flag */
  return((_pstAnimPointer->u32IDFlags & _u32Flags) == _u32Flags);
}

/** AnimPointer flag set accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @param[in]   _u32AddFlags                  Flags to add
 * @param[in]   _u32RemoveFlags               Flags to remove
 */
orxVOID orxFASTCALL orxAnimPointer_SetFlags(orxANIMPOINTER *_pstAnimPointer, orxU32 _u32AddFlags, orxU32 _u32RemoveFlags)
{
  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_FLAG_READY);
  orxASSERT(_pstAnimPointer != orxNULL);

  /* Updates flags */
  _pstAnimPointer->u32IDFlags &= ~_u32RemoveFlags;
  _pstAnimPointer->u32IDFlags |= _u32AddFlags;

  return;
}
