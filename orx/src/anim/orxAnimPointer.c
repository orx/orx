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

#include "core/orxConfig.h"
#include "debug/orxDebug.h"
#include "math/orxMath.h"
#include "memory/orxMemory.h"
#include "core/orxClock.h"


#ifdef __orxMSVC__

  #pragma warning(disable : 4311 4312)

#endif /* __orxMSVC__ */


/** Module flags
 */
#define orxANIMPOINTER_KU32_STATIC_FLAG_NONE          0x00000000  /**< No flags */

#define orxANIMPOINTER_KU32_STATIC_FLAG_READY         0x00000001  /**< Ready flag */


/** orxANIMPOINTER flags
 */
#define orxANIMPOINTER_KU32_FLAG_NONE                 0x00000000  /**< No flags */

#define orxANIMPOINTER_KU32_FLAG_HAS_CURRENT_ANIM     0x00000001  /**< Has current animation flag */
#define orxANIMPOINTER_KU32_FLAG_HAS_NEXT_ANIM        0x00000002  /**< Has next animation flag */
#define orxANIMPOINTER_KU32_FLAG_ANIMSET              0x00000010  /**< Has animset flag */
#define orxANIMPOINTER_KU32_FLAG_LINK_TABLE           0x00000020  /**< Has link table flag */
#define orxANIMPOINTER_KU32_FLAG_PAUSED               0x00000040  /**< Pause flag */
#define orxANIMPOINTER_KU32_FLAG_INTERNAL             0x10000000  /**< Internal structure handling flag  */

#define orxANIMPOINTER_KU32_MASK_FLAGS                0xFFFFFFFF  /**< Flags ID mask */


/** Misc defines
 */
#define orxANIMPOINTER_KZ_CONFIG_ANIMSET              "AnimationSet"
#define orxANIMPOINTER_KZ_CONFIG_FREQUENCY            "Frequency"

#define orxANIMPOINTER_KF_FREQUENCY_DEFAULT           1.0         /**< Default animation frequency */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** AnimPointer structure
 */
struct __orxANIMPOINTER_t
{
  orxSTRUCTURE            stStructure;                /**< Public structure, first structure member : 16 */
  orxANIMSET             *pstAnimSet;                 /**< Referenced AnimationSet : 20 */
  orxANIMSET_LINK_TABLE  *pstLinkTable;               /**< Link table pointer : 24 */
  orxHANDLE               hCurrentAnim;               /**< Current animation ID : 28 */
  orxHANDLE               hTargetAnim;                /**< Target animation ID : 32 */
  orxFLOAT                fCurrentAnimTime;           /**< Current Time (Relative to current animation) : 26 */
  orxFLOAT                fTime;                      /**< Current Time (Absolute) : 40 */
  orxFLOAT                fFrequency;                 /**< Current animation frequency : 44 */
  orxU32                  u32CurrentKey;              /**< Current animation key : 48 */

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
 * @param[in]   _fDT                          Delta time
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATIC orxINLINE orxSTATUS orxAnimPointer_Compute(orxANIMPOINTER *_pstAnimPointer, orxFLOAT _fDT)
{
  orxHANDLE hNewAnim;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxSTRUCTURE_ASSERT(_pstAnimPointer);
  orxASSERT(orxStructure_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_ANIMSET) != orxFALSE);

  /* Not Paused? */
  if(orxStructure_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_PAUSED) == orxFALSE)
  {
    /* Has current animation */
    if(orxStructure_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_HAS_CURRENT_ANIM) != orxFALSE)
    {
      /* Updates Times */
      _pstAnimPointer->fTime += _fDT * _pstAnimPointer->fFrequency;
      _pstAnimPointer->fCurrentAnimTime += _fDT * _pstAnimPointer->fFrequency;

      /* Computes & updates anim*/
      hNewAnim = orxAnimSet_ComputeAnim(_pstAnimPointer->pstAnimSet, _pstAnimPointer->hCurrentAnim, _pstAnimPointer->hTargetAnim, &(_pstAnimPointer->fCurrentAnimTime), _pstAnimPointer->pstLinkTable);

      /* Change happened? */
      if(hNewAnim != _pstAnimPointer->hCurrentAnim)
      {
        /* Updates anim handle */
        _pstAnimPointer->hCurrentAnim = hNewAnim;

        /* No next anim? */
        if(hNewAnim == orxHANDLE_UNDEFINED)
        {
          /* Cleans target anim */
          _pstAnimPointer->hTargetAnim = orxHANDLE_UNDEFINED;

          /* Updates flags */
          orxStructure_SetFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_NONE, orxANIMPOINTER_KU32_FLAG_HAS_CURRENT_ANIM);
        }
      }

      /* Has current anim? */
      if(orxStructure_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_HAS_CURRENT_ANIM) != orxFALSE)
      {
        /* Updates current anim */
        eResult = orxAnim_Update(orxAnimSet_GetAnim(_pstAnimPointer->pstAnimSet, _pstAnimPointer->hCurrentAnim), _pstAnimPointer->fCurrentAnimTime, &(_pstAnimPointer->u32CurrentKey));
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
  orxSTRUCTURE_ASSERT(pstAnimPointer);

  /* Computes animation pointer */
  return(orxAnimPointer_Compute(pstAnimPointer, _pstClockInfo->fDT));
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
  orxModule_AddDependency(orxMODULE_ID_ANIMPOINTER, orxMODULE_ID_CLOCK);
  orxModule_AddDependency(orxMODULE_ID_ANIMPOINTER, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_ANIMPOINTER, orxMODULE_ID_ANIMSET);
  orxModule_AddDependency(orxMODULE_ID_ANIMPOINTER, orxMODULE_ID_ANIM);

  return;
}

/** Inits the AnimPointer module
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxAnimPointer_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstAnimPointer, sizeof(orxANIMPOINTER_STATIC));

    /* Registers structure type */
    eResult = orxSTRUCTURE_REGISTER(ANIMPOINTER, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, &orxAnimPointer_Update);
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
    sstAnimPointer.u32Flags = orxANIMPOINTER_KU32_STATIC_FLAG_READY;
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
  if(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY)
  {
    /* Deletes animpointer list */
    orxAnimPointer_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_ANIMPOINTER);

    /* Updates flags */
    sstAnimPointer.u32Flags &= ~orxANIMPOINTER_KU32_STATIC_FLAG_READY;

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
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimSet);

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
    orxStructure_SetFlags(pstAnimPointer, orxANIMPOINTER_KU32_FLAG_ANIMSET | orxANIMPOINTER_KU32_FLAG_HAS_CURRENT_ANIM, orxANIMPOINTER_KU32_MASK_FLAGS);

    /* Inits value */
    pstAnimPointer->hCurrentAnim      = (orxHANDLE)0;
    pstAnimPointer->fCurrentAnimTime  = orxFLOAT_0;
    pstAnimPointer->fFrequency        = orxANIMPOINTER_KF_FREQUENCY_DEFAULT;
    pstAnimPointer->fTime             = orxFLOAT_0;
    pstAnimPointer->hTargetAnim       = orxHANDLE_UNDEFINED;

    /* Is animset link table non-static? */
    if(orxStructure_TestFlags(_pstAnimSet, orxANIMSET_KU32_FLAG_LINK_STATIC) == orxFALSE)
    {
      /* Stores link table */
      pstAnimPointer->pstLinkTable = orxAnimSet_CloneLinkTable(_pstAnimSet);

      /* Updates flags */
      orxStructure_SetFlags(pstAnimPointer, orxANIMPOINTER_KU32_FLAG_LINK_TABLE, orxANIMPOINTER_KU32_FLAG_NONE);
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

/** Creates an animation pointer from config
 * @param[in]   _zConfigID                    Config ID
 * @return      orxANIMPOINTER / orxNULL
 */
orxANIMPOINTER *orxFASTCALL orxAnimPointer_CreateFromConfig(orxCONST orxSTRING _zConfigID)
{
  orxSTRING       zPreviousSection;
  orxANIMPOINTER *pstResult = orxNULL;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);

  /* Gets previous config section */
  zPreviousSection = orxConfig_GetCurrentSection();

  /* Selects section */
  if(orxConfig_SelectSection(_zConfigID) != orxSTATUS_FAILURE)
  {
    orxSTRING zAnimSetName;

    /* Gets its name */
    zAnimSetName = orxConfig_GetString(orxANIMPOINTER_KZ_CONFIG_ANIMSET);

    /* Valid? */
    if((zAnimSetName != orxNULL) && (*zAnimSetName != *orxSTRING_EMPTY))
    {
      orxANIMSET *pstAnimSet;

      /* Creates animset from config */
      pstAnimSet = orxAnimSet_CreateFromConfig(zAnimSetName);

      /* Valid? */
      if(pstAnimSet != orxNULL)
      {
        /* Creates animation pointer from it */
        pstResult = orxAnimPointer_Create(pstAnimSet);

        /* Valid? */
        if(pstResult != orxNULL)
        {
          /* Has frequency? */
          if(orxConfig_HasValue(orxANIMPOINTER_KZ_CONFIG_FREQUENCY) != orxFALSE)
          {
            /* Updates animation pointer frequency */
            orxAnimPointer_SetFrequency(pstResult, orxConfig_GetFloat(orxANIMPOINTER_KZ_CONFIG_FREQUENCY));
          }

          /* Updates status flags */
          orxStructure_SetFlags(pstResult, orxANIMPOINTER_KU32_FLAG_INTERNAL, orxANIMPOINTER_KU32_FLAG_NONE);
        }
        else
        {
          /* !!! MSG !!! */

          /* Deletes created anim set */
          orxAnimSet_Delete(pstAnimSet);
        }
      }
    }

    /* Restores previous section */
    orxConfig_SelectSection(zPreviousSection);
  }
  else
  {
    /* !!! MSG !!! */

    /* Updates result */
    pstResult = orxNULL;
  }

  /* Done! */
  return pstResult;
}

/** Deletes an AnimPointer
 * @param[in]   _pstAnimPointer               AnimPointer to delete
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnimPointer_Delete(orxANIMPOINTER *_pstAnimPointer)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimPointer);

  /* Not referenced? */
  if(orxStructure_GetRefCounter(_pstAnimPointer) == 0)
  {
    /* Has an animset? */
    if(orxStructure_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_ANIMSET) != orxFALSE)
    {
      /* Removes the reference from the animset */
      orxAnimSet_RemoveReference(_pstAnimPointer->pstAnimSet);

      /* Was internally allocated? */
      if(orxStructure_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_INTERNAL) != orxFALSE)
      {
        /* Deletes animset */
        orxAnimSet_Delete(_pstAnimPointer->pstAnimSet);
      }
    }

    /* Has a link table? */
    if(orxStructure_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_LINK_TABLE) != orxFALSE)
    {
      /* Deletes it */
      orxAnimSet_DeleteLinkTable(_pstAnimPointer->pstLinkTable);
    }

    /* Deletes structure */
    orxStructure_Delete(_pstAnimPointer);
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
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimPointer);

  /* Has an animset? */
  if(orxStructure_TestFlags((orxANIMPOINTER *)_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_ANIMSET) != orxFALSE)
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
orxHANDLE orxFASTCALL orxAnimPointer_GetCurrentAnim(orxCONST orxANIMPOINTER *_pstAnimPointer)
{
  orxHANDLE hAnimHandle = orxHANDLE_UNDEFINED;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimPointer);

  /* Has anim? */
  if((orxStructure_TestFlags((orxANIMPOINTER *)_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_ANIMSET) != orxFALSE)
  && (orxStructure_TestFlags((orxANIMPOINTER *)_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_HAS_CURRENT_ANIM) != orxFALSE))
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


/** AnimPointer target Animation get accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      Target Animation handle
 */
orxHANDLE orxFASTCALL orxAnimPointer_GetTargetAnim(orxCONST orxANIMPOINTER *_pstAnimPointer)
{
  orxHANDLE hAnimHandle = orxHANDLE_UNDEFINED;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimPointer);

  /* Has anim? */
  if((orxStructure_TestFlags((orxANIMPOINTER *)_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_ANIMSET) != orxFALSE)
  && (orxStructure_TestFlags((orxANIMPOINTER *)_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_HAS_CURRENT_ANIM) != orxFALSE))
  {
    hAnimHandle = _pstAnimPointer->hTargetAnim;
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return hAnimHandle;
}

/** AnimPointer current anim data get accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      Current anim data / orxNULL
 */
orxSTRUCTURE *orxFASTCALL orxAnimPointer_GetCurrentAnimData(orxCONST orxANIMPOINTER *_pstAnimPointer)
{
  orxHANDLE     hAnimHandle;
  orxSTRUCTURE *pstResult = orxNULL;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimPointer);

  /* Gets current anim handle */
  hAnimHandle = orxAnimPointer_GetCurrentAnim(_pstAnimPointer);

  /* Valid? */
  if(hAnimHandle != orxHANDLE_UNDEFINED)
  {
    orxANIM *pstAnim;

    /* Gets anim */
    pstAnim = orxAnimSet_GetAnim(orxAnimPointer_GetAnimSet(_pstAnimPointer), hAnimHandle);

    /* Valid? */
    if(pstAnim != orxNULL)
    {
      /* Gets data */
      pstResult = orxAnim_GetKeyData(pstAnim, _pstAnimPointer->u32CurrentKey);
    }
  }

  /* Done! */
  return pstResult;
}

/** AnimPointer current Time get accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      Current time
 */
orxFLOAT orxFASTCALL orxAnimPointer_GetCurrentTime(orxCONST orxANIMPOINTER *_pstAnimPointer)
{
  orxREGISTER orxFLOAT fResult = orxFLOAT_0;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimPointer);

  /* Has anim? */
  if(orxStructure_TestFlags((orxANIMPOINTER *)_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_HAS_CURRENT_ANIM) != orxFALSE)
  {
    /* Gets time */
    fResult = _pstAnimPointer->fCurrentAnimTime;
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return fResult;
}

/** AnimPointer Frequency get accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      AnimPointer frequency
 */
orxFLOAT orxFASTCALL orxAnimPointer_GetFrequency(orxCONST orxANIMPOINTER *_pstAnimPointer)
{
  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimPointer);

  /* Gets frequency */
  return _pstAnimPointer->fFrequency;
}

/** AnimPointer current Animation set accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @param[in]   _hAnimHandle                  Animation handle to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnimPointer_SetCurrentAnimHandle(orxANIMPOINTER *_pstAnimPointer, orxHANDLE _hAnimHandle)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimPointer);

  /* Has Animset? */
  if(orxStructure_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_ANIMSET) != orxFALSE)
  {
    /* In range? */
    if((orxU32)_hAnimHandle < orxAnimSet_GetAnimCounter(_pstAnimPointer->pstAnimSet))
    {
      /* Stores ID */
      _pstAnimPointer->hCurrentAnim = _hAnimHandle;

      /* Clears target anim */
      _pstAnimPointer->hTargetAnim  = orxHANDLE_UNDEFINED;

      /* Updates flags */
      orxStructure_SetFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_HAS_CURRENT_ANIM, orxANIMPOINTER_KU32_FLAG_NONE);

      /* Computes animpointer */
      eResult = orxAnimPointer_Compute(_pstAnimPointer, orxFLOAT_0);
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

/** AnimPointer target Animation set accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @param[in]   _hAnimHandle                  Animation handle to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnimPointer_SetTargetAnimHandle(orxANIMPOINTER *_pstAnimPointer, orxHANDLE _hAnimHandle)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimPointer);

  /* Has Animset? */
  if(orxStructure_TestAllFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_ANIMSET | orxANIMPOINTER_KU32_FLAG_HAS_CURRENT_ANIM) != orxFALSE)
  {
    /* In range? */
    if((orxU32)_hAnimHandle < orxAnimSet_GetAnimCounter(_pstAnimPointer->pstAnimSet))
    {
      /* Stores ID */
      _pstAnimPointer->hTargetAnim = _hAnimHandle;

      /* Computes animpointer */
      eResult = orxAnimPointer_Compute(_pstAnimPointer, orxFLOAT_0);
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
    /* Clears it */
    _pstAnimPointer->hTargetAnim = orxHANDLE_UNDEFINED;

    /* !!! MSG !!! */

    /* Can't process */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** AnimPointer current Animation set accessor using ID
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @param[in]   _u32AnimID                    Animation ID (config's name CRC) to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnimPointer_SetCurrentAnim(orxANIMPOINTER *_pstAnimPointer, orxU32 _u32AnimID)
{
  orxHANDLE hAnimHandle;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimPointer);

  /* Gets corresponding anim handle */
  hAnimHandle = orxAnimSet_GetAnimHandleFromID(_pstAnimPointer->pstAnimSet, _u32AnimID);

  /* Valid? */
  if(hAnimHandle != orxHANDLE_UNDEFINED)
  {
    /* Sets current anim */
    eResult = orxAnimPointer_SetCurrentAnimHandle(_pstAnimPointer, hAnimHandle);
  }

  /* Done! */
  return eResult;
}

/** AnimPointer target Animation set accessor using ID
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @param[in]   _u32AnimID                    Animation ID (config's name CRC) to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnimPointer_SetTargetAnim(orxANIMPOINTER *_pstAnimPointer, orxU32 _u32AnimID)
{
  orxHANDLE hAnimHandle;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimPointer);

  /* Gets corresponding anim handle */
  hAnimHandle = orxAnimSet_GetAnimHandleFromID(_pstAnimPointer->pstAnimSet, _u32AnimID);

  /* Valid? */
  if(hAnimHandle != orxHANDLE_UNDEFINED)
  {
    /* Sets target anim */
    eResult = orxAnimPointer_SetTargetAnimHandle(_pstAnimPointer, hAnimHandle);
  }

  /* Done! */
  return eResult;
}

/** AnimPointer current Time accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @param[in]   _fTime                        Time to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnimPointer_SetTime(orxANIMPOINTER *_pstAnimPointer, orxFLOAT _fTime)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimPointer);

  /* Stores relative timestamp */
  _pstAnimPointer->fCurrentAnimTime = _fTime;

  /* Computes animpointer */
  eResult = orxAnimPointer_Compute(_pstAnimPointer, orxFLOAT_0);

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
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimPointer);
  orxASSERT(_fFrequency >= 0.0);

  /* Stores frequency */
  _pstAnimPointer->fFrequency = _fFrequency;

  /* Computes animpointer */
  eResult = orxAnimPointer_Compute(_pstAnimPointer, orxFLOAT_0);

  /* Done! */
  return eResult;
}

/** AnimPointer pause accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @param[in]   _bPause                       Pause / Unpause
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnimPointer_Pause(orxANIMPOINTER *_pstAnimPointer, orxBOOL _bPause)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimPointer);

  /* Pause? */
  if(_bPause != orxFALSE)
  {
    /* Updates status flags */
    orxStructure_SetFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_PAUSED, orxANIMPOINTER_KU32_FLAG_NONE);
  }
  else
  {
    /* Updates status flags */
    orxStructure_SetFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_NONE, orxANIMPOINTER_KU32_FLAG_PAUSED);
  }

  /* Done! */
  return eResult;
}


#ifdef __orxMSVC__

  #pragma warning(default : 4311 4312)

#endif /* __orxMSVC__ */
