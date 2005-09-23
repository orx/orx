/***************************************************************************
 orxAnimPointer.c
 Animation Pointer module
 
 begin                : 03/03/2004
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


#include "anim/orxAnimPointer.h"

#include "debug/orxDebug.h"
#include "memory/orxMemory.h"


/*
 * Platform independant defines
 */

#define orxANIMPOINTER_KU32_FLAG_NONE                 0x00000000
#define orxANIMPOINTER_KU32_FLAG_READY                0x00000001
#define orxANIMPOINTER_KU32_FLAG_DEFAULT              0x00000000

#define orxANIMPOINTER_KU32_ID_FLAG_NONE              0x00000000

#define orxANIMPOINTER_KU32_ID_FLAG_HAS_CURRENT_ANIM  0x01000000
#define orxANIMPOINTER_KU32_ID_FLAG_HAS_NEXT_ANIM     0x02000000
#define orxANIMPOINTER_KU32_ID_FLAG_ANIMSET           0x10000000
#define orxANIMPOINTER_KU32_ID_FLAG_LINK_TABLE        0x20000000
#define orxANIMPOINTER_KU32_ID_FLAG_PAUSED            0x40000000

#define orxANIMPOINTER_KU32_ID_FLAG_ALL               0xFFFF0000

#define orxANIMPOINTER_KF_FREQUENCY_DEFAULT           1.0


/*
 * Animation Pointer structure
 */
struct __orxANIMPOINTER_t
{
  /* Public structure, first structure member : 16 */
  orxSTRUCTURE stStructure;

  /* Id flags : 20 */
  orxU32 u32IDFlags;

  /* Referenced AnimationSet : 24 */
  orxANIMSET *pstAnimset;

  /* Link table pointer : 28 */
  orxANIMSET_LINK_TABLE *pstLinkTable;

  /* Current animation ID : 32 */
  orxHANDLE hCurrentAnim;

  /* Destination animation ID : 36 */
  orxHANDLE hDstAnim;

  /* Current Time (Relative to current animation) : 40 */
  orxU32 u32CurrentAnimTime;

  /* Current Time (Absolute) : 44 */
  orxU32 u32Time;

  /* Current animation frequency : 48 */
  orxFLOAT fFrequency;
};


/*
 * Static structure
 */
typedef struct __orxANIMPOINTER_STATIC_t
{
  /* Control flags */
  orxU32 u32Flags;

} orxANIMPOINTER_STATIC;


/*
 * Static data
 */
orxSTATIC orxANIMPOINTER_STATIC sstAnimPointer;


/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 orxAnimPointer_DeleteAll
 Deletes all Animation Pointers.

 returns: orxVOID
 ***************************************************************************/
orxSTATIC orxVOID orxAnimPointer_DeleteAll()
{
  orxREGISTER orxANIMPOINTER *pstAnimpointer;
  
  /* Gets first anim pointer */
  pstAnimpointer = (orxANIMPOINTER *)orxStructure_GetFirst(orxSTRUCTURE_ID_ANIMPOINTER);

  /* Non empty? */
  while(pstAnimpointer != orxNULL)
  {
    /* Deletes Animation Pointer */
    orxAnimPointer_Delete(pstAnimpointer);

    /* Gets first Animation Set */
    pstAnimpointer = (orxANIMPOINTER *)orxStructure_GetFirst(orxSTRUCTURE_ID_ANIMPOINTER);
  }

  return;
}

/***************************************************************************
 orxAnimPointer_Compute
 Computes animation for the given Time.

 returns: orxSTATUS_SUCCESS / orxSTATUS_FAILED
 ***************************************************************************/
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
      hNewAnim = orxAnimSet_ComputeAnim(_pstAnimPointer->pstAnimset, _pstAnimPointer->hCurrentAnim, _pstAnimPointer->hDstAnim, &(_pstAnimPointer->u32CurrentAnimTime), _pstAnimPointer->pstLinkTable);
    
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
      eResult = orxSTATUS_FAILED;
    }
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxAnimPointer_Update
 Updates animation pointer.

 returns: orxSTATUS_SUCCESS / orxSTATUS_FAILED
 ***************************************************************************/
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
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 orxAnimPointer_Setup
 AnimPointer module setup.

 returns: nothing
 ***************************************************************************/
orxVOID orxAnimPointer_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_ANIMPOINTER, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_ANIMPOINTER, orxMODULE_ID_ANIMSET);
  orxModule_AddDependency(orxMODULE_ID_ANIMPOINTER, orxMODULE_ID_ANIM);

  return;
}

/***************************************************************************
 orxAnimPointer_Init
 Inits Animation Pointer system.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxAnimPointer_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILED;
  
  /* Not already Initialized? */
  if(!(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_FLAG_READY))
  {
    orxSTRUCTURE_REGISTER_INFO stRegisterInfo;
  
    /* Cleans control structure */
    orxMemory_Set(&sstAnimPointer, 0, sizeof(orxANIMPOINTER_STATIC));

    /* Initialized? */
    if(eResult == orxSTATUS_SUCCESS)
    {
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
    }
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

/***************************************************************************
 orxAnimPointer_Exit
 Exits from the Animation Set system.

 returns: orxVOID
 ***************************************************************************/
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

/***************************************************************************
 orxAnimPointer_Create
 Creates an empty AnimationPointer, given its AnimationSet reference.

 returns: Created animpointer.
 ***************************************************************************/
orxANIMPOINTER *orxAnimPointer_Create(orxANIMSET *_pstAnimset)
{
  orxANIMPOINTER *pstAnimpointer = orxNULL;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_FLAG_READY);
  orxASSERT(_pstAnimset != orxNULL);

  /* Creates animpointer */
  pstAnimpointer = (orxANIMPOINTER *)orxStructure_Create(orxSTRUCTURE_ID_ANIMPOINTER);

  /* Was allocated? */
  if(pstAnimpointer != orxNULL)
  {
    /* Stores animset */
    pstAnimpointer->pstAnimset = _pstAnimset;

    /* Adds a reference on the animset */
    orxAnimSet_AddReference(_pstAnimset);

    /* Inits flags */
    orxAnimPointer_SetFlags(pstAnimpointer, orxANIMPOINTER_KU32_ID_FLAG_ANIMSET | orxANIMPOINTER_KU32_ID_FLAG_HAS_CURRENT_ANIM, orxANIMPOINTER_KU32_ID_FLAG_ALL);

    /* Inits value */
    pstAnimpointer->hCurrentAnim        = (orxHANDLE)0;
    pstAnimpointer->u32CurrentAnimTime   = 0;
    pstAnimpointer->fFrequency          = orxANIMPOINTER_KF_FREQUENCY_DEFAULT;
    pstAnimpointer->u32Time              = orxTime_GetTime();
    pstAnimpointer->hDstAnim            = orxHANDLE_Undefined;

    /* Is animset link table non-static? */
    if(orxAnimSet_TestFlags(_pstAnimset, orxANIMSET_KU32_ID_FLAG_LINK_STATIC) == orxFALSE)
    {
      /* Stores link table */
      pstAnimpointer->pstLinkTable = orxAnimSet_DuplicateLinkTable(_pstAnimset);

      /* Updates flags */
      orxAnimPointer_SetFlags(pstAnimpointer, orxANIMPOINTER_KU32_ID_FLAG_LINK_TABLE, orxANIMPOINTER_KU32_ID_FLAG_NONE);
    }
  }
  else
  {
    /* !!! MSG !!! */
    
    /* Not created */
    pstAnimpointer = orxNULL;
  }

  /* Done! */
  return pstAnimpointer;
}

/***************************************************************************
 orxAnimPointer_Delete
 Deletes an Animation Pointer.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxAnimPointer_Delete(orxANIMPOINTER *_pstAnimpointer)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_FLAG_READY);
  orxASSERT(_pstAnimpointer != orxNULL);

  /* Not referenced? */
  if(orxStructure_GetRefCounter((orxSTRUCTURE *)_pstAnimpointer) == 0)
  {
    /* Has an animset? */
    if(orxAnimPointer_TestFlags(_pstAnimpointer, orxANIMPOINTER_KU32_ID_FLAG_ANIMSET) != orxFALSE)
    {
      /* Removes the reference from the animset */
      orxAnimSet_RemoveReference(_pstAnimpointer->pstAnimset);
    }

    /* Has a link table? */
    if(orxAnimPointer_TestFlags(_pstAnimpointer, orxANIMPOINTER_KU32_ID_FLAG_LINK_TABLE) != orxFALSE)
    {
      /* Deletes it */
      orxAnimSet_DeleteLinkTable(_pstAnimpointer->pstLinkTable);
    }

    /* Deletes structure */
    orxStructure_Delete((orxSTRUCTURE *)_pstAnimpointer);
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
 orxAnimPointer_GetAnimSet
 Gets the referenced Animation Set.

 returns: orxVOID
 ***************************************************************************/
orxANIMSET *orxAnimPointer_GetAnimSet(orxCONST orxANIMPOINTER *_pstAnimpointer)
{
  orxANIMSET *pstAnimset = orxNULL;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_FLAG_READY);
  orxASSERT(_pstAnimpointer != orxNULL);

  /* Has animset? */
  if(orxAnimPointer_TestFlags(_pstAnimpointer, orxANIMPOINTER_KU32_ID_FLAG_ANIMSET) != orxFALSE)
  {
    pstAnimset = _pstAnimpointer->pstAnimset;
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return pstAnimset;
}

/***************************************************************************
 orxAnimPointer_GetAnim
 AnimationPointer current Animation get accessor.

 returns: orxAnim
 ***************************************************************************/
orxANIM *orxAnimPointer_GetAnim(orxCONST orxANIMPOINTER *_pstAnimpointer)
{
  orxANIM *pstAnim = orxNULL;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_FLAG_READY);
  orxASSERT(_pstAnimpointer != orxNULL);

  /* Has anim? */
  if((orxAnimPointer_TestFlags(_pstAnimpointer, orxANIMPOINTER_KU32_ID_FLAG_ANIMSET) != orxFALSE)
  && (orxAnimPointer_TestFlags(_pstAnimpointer, orxANIMPOINTER_KU32_ID_FLAG_HAS_CURRENT_ANIM) != orxFALSE))
  {
    pstAnim = orxAnimSet_GetAnim(_pstAnimpointer->pstAnimset, _pstAnimpointer->hCurrentAnim);
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return pstAnim;    
}

/***************************************************************************
 orxAnimPointer_GetTime
 AnimationPointer current Animation Time get accessor.

 returns: orxU32 timestamp
 ***************************************************************************/
orxU32 orxAnimPointer_GetTime(orxCONST orxANIMPOINTER *_pstAnimpointer)
{
  orxU32 u32Result = 0;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_FLAG_READY);
  orxASSERT(_pstAnimpointer != orxNULL);

  /* Has anim? */
  if(orxAnimPointer_TestFlags(_pstAnimpointer, orxANIMPOINTER_KU32_ID_FLAG_HAS_CURRENT_ANIM) != orxFALSE)
  {
    /* Gets time */
    u32Result = _pstAnimpointer->u32CurrentAnimTime;
  }
  else
  {
    /* !!! MSG !!! */
  }
  
  /* Done! */
  return u32Result;
}

/***************************************************************************
 orxAnimPointer_GetFrequency
 AnimationPointer Frequency get accessor.

 returns: orxFLOAT frequency
 ***************************************************************************/
orxFLOAT orxAnimPointer_GetFrequency(orxCONST orxANIMPOINTER *_pstAnimpointer)
{
  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_FLAG_READY);
  orxASSERT(_pstAnimpointer != orxNULL);

  /* Gets frequency */
  return _pstAnimpointer->fFrequency;
}

/***************************************************************************
 orxAnimPointer_SetAnim
 AnimationPointer current Animation set accessor.

 returns: orxSTATUS_SUCCESS / orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxAnimPointer_SetAnim(orxANIMPOINTER *_pstAnimpointer, orxHANDLE _hAnimHandle)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;
  
  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_FLAG_READY);
  orxASSERT(_pstAnimpointer != orxNULL);

  /* Has Animset? */
  if(orxAnimPointer_TestFlags(_pstAnimpointer, orxANIMPOINTER_KU32_ID_FLAG_ANIMSET) != orxFALSE)
  {
    /* In range? */
    if((orxU32)_hAnimHandle < orxAnimSet_GetAnimCounter(_pstAnimpointer->pstAnimset))
    {
      /* Stores ID */
      _pstAnimpointer->hCurrentAnim = _hAnimHandle;

      /* Updates absolute timestamp */
      _pstAnimpointer->u32Time       = orxTime_GetTime();
    
      /* Updates flags */
      orxAnimPointer_SetFlags(_pstAnimpointer, orxANIMPOINTER_KU32_ID_FLAG_HAS_CURRENT_ANIM, orxANIMPOINTER_KU32_ID_FLAG_NONE);
    
      /* Computes animpointer */
      eResult = orxAnimPointer_Compute(_pstAnimpointer, _pstAnimpointer->u32Time);
    }
    else
    {
      /* !!! MSG !!! */

      /* Can't process */
      eResult = orxSTATUS_FAILED;
    }
  }
  else
  {
    /* !!! MSG !!! */

    /* Can't process */
    eResult = orxSTATUS_FAILED;
  }

  /* Done! */
  return eResult;    
}

/***************************************************************************
 orxAnimPointer_SetTime
 AnimationPointer current Animation Time set accessor.

 returns: orxSTATUS_SUCCESS / orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxAnimPointer_SetTime(orxANIMPOINTER *_pstAnimpointer, orxU32 _u32Time)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_FLAG_READY);
  orxASSERT(_pstAnimpointer != orxNULL);

  /* Stores relative timestamp */
  _pstAnimpointer->u32CurrentAnimTime = _u32Time;
  
  /* Updates absolute timestamp */
  _pstAnimpointer->u32Time = orxTime_GetTime();

  /* Computes animpointer */
  eResult = orxAnimPointer_Compute(_pstAnimpointer, _pstAnimpointer->u32Time);

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxAnimPointer_SetFrequency
 AnimationPointer Frequency set accessor.

 returns: orxFLOAT frequency
 ***************************************************************************/
orxSTATUS orxAnimPointer_SetFrequency(orxANIMPOINTER *_pstAnimpointer, orxFLOAT _fFrequency)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_FLAG_READY);
  orxASSERT(_pstAnimpointer != orxNULL);
  orxASSERT(_fFrequency >= 0.0);

  /* Computes animpointer */
  eResult = orxAnimPointer_Compute(_pstAnimpointer, orxTime_GetTime());

  /* Succeeded? */
  if(eResult == orxSTATUS_SUCCESS)
  {
    /* Stores frequency */
    _pstAnimpointer->fFrequency = _fFrequency; 
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxAnimPointer_TestFlag
 Animation pointer flag test accessor.

 returns: orxBOOL
 ***************************************************************************/
orxBOOL orxAnimPointer_TestFlags(orxCONST orxANIMPOINTER *_pstAnimpointer, orxU32 _u32Flag)
{
  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_FLAG_READY);
  orxASSERT(_pstAnimpointer != orxNULL);

  /* Tests flag */
  return((_pstAnimpointer->u32IDFlags & _u32Flag) == _u32Flag);
}

/***************************************************************************
 orxAnimPointer_SetFlag
 Animation pointer flag get/set accessor.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxAnimPointer_SetFlags(orxANIMPOINTER *_pstAnimpointer, orxU32 _u32AddFlags, orxU32 _u32RemoveFlags)
{
  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_FLAG_READY);
  orxASSERT(_pstAnimpointer != orxNULL);

  /* Updates flags */
  _pstAnimpointer->u32IDFlags &= ~_u32RemoveFlags;
  _pstAnimpointer->u32IDFlags |= _u32AddFlags;

  return;
}
