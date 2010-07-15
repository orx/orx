/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2010 Orx-Project
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 */

/**
 * @file orxFrame.c
 * @date 02/12/2003
 * @author iarwain@orx-project.org
 *
 */


#include "object/orxFrame.h"

#include "debug/orxDebug.h"
#include "memory/orxMemory.h"
#include "memory/orxBank.h"
#include "object/orxStructure.h"
#include "math/orxMath.h"


/** Module flags
 */
#define orxFRAME_KU32_STATIC_FLAG_NONE      0x00000000  /**< No flags */

#define orxFRAME_KU32_STATIC_FLAG_READY     0x00000001  /**< Ready flag */
#define orxFRAME_KU32_STATIC_FLAG_DATA_2D   0x00000010  /**< 2D flag */

#define orxFRAME_KU32_STATIC_MASK_DEFAULT   0x00000010  /**< Default flag */

#define orxFRAME_KU32_STATIC_MASK_ALL       0xFFFFFFFF  /**< All mask */


/** orxFRAME flags
 */
#define orxFRAME_KU32_FLAG_NONE             0x00000000  /**< No flags */

#define orxFRAME_KU32_FLAG_DATA_2D          0x10000000  /**< 2D ID flag */
#define orxFRAME_KU32_FLAG_DIRTY            0x01000000  /**< Dirty ID flag */

#define orxFRAME_KU32_MASK_ALL              0xFFFFFFFF  /**< Dirty ID flag */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Internal 2D Frame Data structure
 */
typedef struct __orxFRAME_DATA_2D_t
{
  orxVECTOR vGlobalPos;                     /**< Global 2D coordinates : 12 */
  orxVECTOR vLocalPos;                      /**< Local 2D coordinates : 24 */
  orxFLOAT  fGlobalAngle;                   /**< Global 2D rotation angle : 28 */
  orxFLOAT  fGlobalScaleX;                  /**< Global 2D isometric X scale : 32 */
  orxFLOAT  fGlobalScaleY;                  /**< Global 2D isometric Y scale : 36 */
  orxFLOAT  fLocalAngle;                    /**< Local 2D rotation angle : 40 */
  orxFLOAT  fLocalScaleX;                   /**< Local 2D isometric X scale : 44 */
  orxFLOAT  fLocalScaleY;                   /**< Local 2D isometric Y scale : 48 */

} orxFRAME_DATA_2D;


/** Frame structure
 */
struct __orxFRAME_t
{
  orxSTRUCTURE      stStructure;            /**< Public structure, first structure member : 16 */
  orxFRAME_DATA_2D  stData;                 /**< Frame data : 64 */
};

/** Static structure
 */
typedef struct __orxFRAME_STATIC_t
{
  orxU32    u32Flags;                       /**< Control flags : 4 */
  orxFRAME *pstRoot;                        /**< Frame root : 8 */
  orxBANK  *pst2DDataBank;                  /**< 2D Data bank : 12 */

} orxFRAME_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxFRAME_STATIC sstFrame;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Sets a frame position
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _pvPos          Position to set
 * @param[in]   _eSpace         Coordinate space system to use
 */
static orxINLINE void _orxFrame_SetPosition(orxFRAME *_pstFrame, const orxVECTOR *_pvPos, orxFRAME_SPACE _eSpace)
{
  /* Checks */
  orxSTRUCTURE_ASSERT(_pstFrame);
  orxASSERT(_pvPos != orxNULL);

  /* According to space */
  switch(_eSpace)
  {
    case orxFRAME_SPACE_GLOBAL:
    {
      orxVector_Copy(&(_pstFrame->stData.vGlobalPos), _pvPos);

      break;
    }

    case orxFRAME_SPACE_LOCAL:
    {
      orxVector_Copy(&(_pstFrame->stData.vLocalPos), _pvPos);

      break;
    }

    default:
    {
      /* Wrong space */
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Incorrect space (%ld).", _eSpace);

      break;
    }
  }

  return;
}

/** Sets a frame rotation
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _fAngle         Rotation angle to set (radians)
 * @param[in]   _eSpace         Coordinate space system to use
 */
static orxINLINE void _orxFrame_SetRotation(orxFRAME *_pstFrame, orxFLOAT _fAngle, orxFRAME_SPACE _eSpace)
{
  /* Checks */
  orxASSERT((_pstFrame != orxNULL));

  /* According to coord type */
  switch(_eSpace)
  {
    case orxFRAME_SPACE_GLOBAL:
    {
      _pstFrame->stData.fGlobalAngle  = _fAngle;

      break;
    }

    case orxFRAME_SPACE_LOCAL:
    {
      _pstFrame->stData.fLocalAngle   = _fAngle;

      break;
    }

    default:
    {
      /* Wrong coord type */
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Incorrect space (%ld).", _eSpace);

      break;
    }
  }

  return;
}

/** Sets a frame scale
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _pvScale        Scale to set
 * @param[in]   _eSpace         Coordinate space system to use
 */
static orxINLINE void _orxFrame_SetScale(orxFRAME *_pstFrame, const orxVECTOR *_pvScale, orxFRAME_SPACE _eSpace)
{
  /* Checks */
  orxASSERT(_pstFrame != orxNULL);
  orxASSERT(_pvScale != orxNULL);

  /* According to coord type */
  switch(_eSpace)
  {
    case orxFRAME_SPACE_GLOBAL:

      _pstFrame->stData.fGlobalScaleX = _pvScale->fX;
      _pstFrame->stData.fGlobalScaleY = _pvScale->fY;

      break;

    case orxFRAME_SPACE_LOCAL:

      _pstFrame->stData.fLocalScaleX = _pvScale->fX;
      _pstFrame->stData.fLocalScaleY = _pvScale->fY;

      break;

    default:

      /* Wrong coord type */
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Incorrect space (%ld).", _eSpace);

      break;
  }

  return;
}

/** Gets a frame position
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _eSpace         Coordinate space system to use
 * @return orxVECTOR / orxNULL
 */
static orxINLINE const orxVECTOR *_orxFrame_GetPosition(const orxFRAME *_pstFrame, orxFRAME_SPACE _eSpace)
{
  const orxVECTOR *pvResult = orxNULL;

  /* Checks */
  orxASSERT((_pstFrame != orxNULL));

  /* According to coord type */
  switch(_eSpace)
  {
    case orxFRAME_SPACE_GLOBAL:
    {
      /* Updates result */
      pvResult = &(_pstFrame->stData.vGlobalPos);

      break;
    }

    case orxFRAME_SPACE_LOCAL:
    {
      /* Updates result */
      pvResult = &(_pstFrame->stData.vLocalPos);

      break;
    }

    default:
    {
      /* Wrong coord type */
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Incorrect space (%ld).", _eSpace);

      break;
    }
  }

  /* Done */
  return pvResult;
}

/** Gets a frame rotation
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _eSpace         Coordinate space system to use
 * @return orxFLOAT (radians) / orxNULL
 */
static orxINLINE orxFLOAT _orxFrame_GetRotation(const orxFRAME *_pstFrame, orxFRAME_SPACE _eSpace)
{
  orxFLOAT fAngle = orxFLOAT_0;

  /* Checks */
  orxASSERT((_pstFrame != orxNULL));

  /* According to coord type */
  switch(_eSpace)
  {
    case orxFRAME_SPACE_GLOBAL:
    {
      fAngle = _pstFrame->stData.fGlobalAngle;

      break;
    }

    case orxFRAME_SPACE_LOCAL:
    {
      fAngle = _pstFrame->stData.fLocalAngle;

      break;
    }

    default:
    {
      /* Wrong coord type */
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Incorrect space (%ld).", _eSpace);

      break;
    }
  }

  /* Done */
  return fAngle;
}

/** Gets a frame scale
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _eSpace         Coordinate space system to use
 * @param[out]  _pvScale        Scale
 * @return      orxVECTOR / orxNULL
 */
static orxINLINE orxVECTOR *_orxFrame_GetScale(const orxFRAME *_pstFrame, orxFRAME_SPACE _eSpace, orxVECTOR *_pvScale)
{
  orxVECTOR *pvResult;

  /* Checks */
  orxSTRUCTURE_ASSERT(_pstFrame);
  orxASSERT(_pvScale != orxNULL);

  /* No z scale */
  _pvScale->fZ = orxFLOAT_1;

  /* According to coord type */
  switch(_eSpace)
  {
    case orxFRAME_SPACE_GLOBAL:
    {
      _pvScale->fX = _pstFrame->stData.fGlobalScaleX;
      _pvScale->fY = _pstFrame->stData.fGlobalScaleY;

      /* Updates result */
      pvResult = _pvScale;

      break;
    }

    case orxFRAME_SPACE_LOCAL:
    {
      _pvScale->fX = _pstFrame->stData.fLocalScaleX;
      _pvScale->fY = _pstFrame->stData.fLocalScaleY;

      /* Updates result */
      pvResult = _pvScale;

      break;
    }

    default:
    {
      /* Wrong coord type */
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Incorrect space (%ld).", _eSpace);

      /* Clears scale */
      orxVector_Copy(_pvScale, &orxVECTOR_0);

      /* Updates result */
      pvResult = orxNULL;

      break;
    }
  }

  /* Done */
  return pvResult;
}

/** Updates frame global data using parent's global and frame local ones
 * @param[out]  _pstDstFrame    Destination frame, will contain up-to-date frame
 * @param[in]   _pstSrcFrame    Source frame, which needs update
 */
static void orxFASTCALL orxFrame_UpdateData(orxFRAME *_pstDstFrame, const orxFRAME *_pstSrcFrame)
{
  /* Checks */
  orxASSERT((_pstDstFrame != orxNULL));
  orxASSERT((_pstSrcFrame != orxNULL));

  /* 2D data? */
  if(orxStructure_TestFlags(_pstSrcFrame, orxFRAME_KU32_FLAG_DATA_2D) != orxFALSE)
  {
    orxVECTOR           vTempPos, vScale, vParentScale, vLocalScale;
    const orxVECTOR *pvParentPos, *pvPos;
    orxFLOAT            fParentAngle, fAngle;
    orxFLOAT            fX, fY, fLocalX, fLocalY, fCos, fSin;
    orxFRAME            *pstParentFrame;

    /* gets parent frame */
    pstParentFrame = orxFRAME(orxStructure_GetParent(_pstSrcFrame));

    /* Gets parent's global data */
    pvParentPos   = _orxFrame_GetPosition(pstParentFrame, orxFRAME_SPACE_GLOBAL);
    fParentAngle  = _orxFrame_GetRotation(pstParentFrame, orxFRAME_SPACE_GLOBAL);
    _orxFrame_GetScale(pstParentFrame, orxFRAME_SPACE_GLOBAL, &vParentScale);

    /* Gets frame's local coord */
    pvPos         = _orxFrame_GetPosition(_pstSrcFrame, orxFRAME_SPACE_LOCAL);

    /* Gets frame's local scales */
    _orxFrame_GetScale(_pstSrcFrame, orxFRAME_SPACE_LOCAL, &vLocalScale);

    /* Updates angle */
    fAngle        = _orxFrame_GetRotation(_pstSrcFrame, orxFRAME_SPACE_LOCAL) + fParentAngle;
    fCos          = orxMath_Cos(fParentAngle);
    fSin          = orxMath_Sin(fParentAngle);

    /* Updates scales */
    vScale.fX     = vLocalScale.fX * vParentScale.fX;
    vScale.fY     = vLocalScale.fY * vParentScale.fY;

    /* Updates coord */
    /* Gets scaled X&Y coordinates */
    fLocalX       = vParentScale.fX * pvPos->fX;
    fLocalY       = vParentScale.fY * pvPos->fY;

    /* Applies rotation on X&Y coordinates */
    fX            = (fLocalX * fCos) - (fLocalY * fSin);
    fY            = (fLocalX * fSin) + (fLocalY * fCos);

    /* Computes final global coordinates */
    vTempPos.fX   = fX + pvParentPos->fX;
    vTempPos.fY   = fY + pvParentPos->fY;

    /* Z coordinate is not affected by rotation nor scale in 2D */
    vTempPos.fZ   = pvParentPos->fZ + pvPos->fZ;

    /* Stores them */
    _orxFrame_SetRotation(_pstDstFrame, fAngle, orxFRAME_SPACE_GLOBAL);
    _orxFrame_SetScale(_pstDstFrame, &vScale, orxFRAME_SPACE_GLOBAL);
    _orxFrame_SetPosition(_pstDstFrame, &vTempPos, orxFRAME_SPACE_GLOBAL);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Frame does not support non-2d yet.");
  }

  return;
}

/** Processes frame dirty state
 * @param[in]   _pstFrame       Concerned frame
 */
static orxINLINE void orxFrame_ProcessDirty(orxFRAME *_pstFrame)
{
  orxFRAME *pstParentFrame;

  /* Checks */
  orxSTRUCTURE_ASSERT(_pstFrame);

  /* gets parent frame */
  pstParentFrame = orxFRAME(orxStructure_GetParent(_pstFrame));

  /* Is cell dirty & has parent? */
  if((orxStructure_TestFlags(_pstFrame, orxFRAME_KU32_FLAG_DIRTY) != orxFALSE)
  && (pstParentFrame != orxNULL))
  {
    /* Updates parent status */
    orxFrame_ProcessDirty(pstParentFrame);

    /* Updates frame global data */
    orxFrame_UpdateData(_pstFrame, _pstFrame);
  }

  /* Updates dirty status */
  orxStructure_SetFlags(_pstFrame, orxFRAME_KU32_FLAG_NONE, orxFRAME_KU32_FLAG_DIRTY);

  return;
}

/** Sets a frame flag recursively
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _u32AddFlags    Flags to add
 * @param[in]   _u32RemoveFlags Flags to remove
 * @param[in]   _bRecursed      Recursive?
 */
static void orxFASTCALL orxFrame_SetFlagRecursively(orxFRAME *_pstFrame, orxU32 _u32AddFlags, orxU32 _u32RemoveFlags, orxBOOL _bRecursed)
{
  /* Non null? */
  if(_pstFrame != orxNULL)
  {
    /* Updates child status */
    orxFrame_SetFlagRecursively(orxFRAME(orxStructure_GetChild(_pstFrame)), _u32AddFlags, _u32RemoveFlags, orxTRUE);

    /* Recursed? */
    if(_bRecursed)
    {
      /* Updates siblings status */
      orxFrame_SetFlagRecursively(orxFRAME(orxStructure_GetSibling(_pstFrame)), _u32AddFlags, _u32RemoveFlags, orxTRUE);
    }

    /* Updates cell flags */
    orxStructure_SetFlags(_pstFrame, _u32AddFlags, _u32RemoveFlags);
  }

  return;
}

/** Tags a frame as dirty
 * @param[in]   _pstFrame       Concerned frame
 */
static orxINLINE void orxFrame_SetDirty(orxFRAME *_pstFrame)
{
  /* Checks */
  orxSTRUCTURE_ASSERT(_pstFrame);

  /* Adds dirty flags to all frame's heirs */
  orxFrame_SetFlagRecursively(_pstFrame, orxFRAME_KU32_FLAG_DIRTY, orxFRAME_KU32_FLAG_NONE, orxFALSE);

  return;
}

/** Deletes all frames
 */
static orxINLINE void orxFrame_DeleteAll()
{
  register orxFRAME *pstFrame;

  /* Gets first frame */
  pstFrame = orxFRAME(orxStructure_GetChild(sstFrame.pstRoot));

  /* Untill only root remains */
  while(pstFrame != orxNULL)
  {
    /* Deletes firt child cell */
    orxFrame_Delete(pstFrame);

    /* Gets root new child */
    pstFrame = (orxFRAME *)orxStructure_GetChild(sstFrame.pstRoot);
  }

  /* Removes root */
  orxFrame_Delete(sstFrame.pstRoot);
  sstFrame.pstRoot = orxNULL;

  return;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Animation module setup
 */
void orxFASTCALL orxFrame_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_FRAME, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_FRAME, orxMODULE_ID_STRUCTURE);

  return;
}

/** Inits the Frame module
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFrame_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_READY))
  {
    /* Cleans control structure */
    orxMemory_Zero(&sstFrame, sizeof(orxFRAME_STATIC));

    /* Inits flags */
    sstFrame.u32Flags = orxFRAME_KU32_STATIC_MASK_DEFAULT|orxFRAME_KU32_STATIC_FLAG_READY;

    /* Registers structure type */
    eResult = orxSTRUCTURE_REGISTER(FRAME, orxSTRUCTURE_STORAGE_TYPE_TREE, orxMEMORY_TYPE_MAIN, orxNULL);

    /* Successful? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Inits frame tree */
      sstFrame.pstRoot = orxFrame_Create(orxFRAME_KU32_FLAG_NONE);

      /* Not created? */
      if(sstFrame.pstRoot == orxNULL)
      {
        /* Unregister structure type */
        orxStructure_Unregister(orxSTRUCTURE_ID_FRAME);

        /* Cleans flags */
        sstFrame.u32Flags = orxFRAME_KU32_STATIC_FLAG_NONE;

        /* Can't process */
        eResult = orxSTATUS_FAILURE;
      }
      else
      {
        /* Continue */
        eResult = orxSTATUS_SUCCESS;
      }
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Tried to initialize frame mdoule when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Exits from the Frame module
 */
void orxFASTCALL orxFrame_Exit()
{
  /* Initialized? */
  if(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_READY)
  {
    /* Deletes frame tree */
    orxFrame_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_FRAME);

    /* Updates flags */
    sstFrame.u32Flags &= ~orxFRAME_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Tried to exit frame module when it wasn't initialized.");
  }

  return;
}

/** Creates a frame
 * @param[in]   _u32Flags     flags for created animation
 * @return      Created orxFRAME / orxNULL
 */
orxFRAME *orxFASTCALL orxFrame_Create(orxU32 _u32Flags)
{
  orxFRAME *pstFrame;

  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_READY);
  orxASSERT((_u32Flags & orxFRAME_KU32_MASK_USER_ALL) == _u32Flags);

  /* Creates frame */
  pstFrame = orxFRAME(orxStructure_Create(orxSTRUCTURE_ID_FRAME));

  /* Valid? */
  if(pstFrame != orxNULL)
  {
    /* Inits flags */
    orxStructure_SetFlags(pstFrame, _u32Flags & orxFRAME_KU32_MASK_USER_ALL, orxFRAME_KU32_MASK_ALL);

    /* Inits members */
    if(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_DATA_2D)
    {
      /* Updates flags */
      orxStructure_SetFlags(pstFrame, orxFRAME_KU32_FLAG_DATA_2D, orxFRAME_KU32_FLAG_NONE);

      /* Inits values */
      pstFrame->stData.fGlobalScaleX  = orxFLOAT_1;
      pstFrame->stData.fGlobalScaleY  = orxFLOAT_1;
      pstFrame->stData.fLocalScaleX   = orxFLOAT_1;
      pstFrame->stData.fLocalScaleY   = orxFLOAT_1;

      /* Has already a root? */
      if(sstFrame.pstRoot != orxNULL)
      {
        /* Sets frame to root */
        orxFrame_SetParent(pstFrame, sstFrame.pstRoot);
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Frame does not support non-2d data yet.");
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to create structure for frame.");
  }

  /* Done! */
  return pstFrame;
}

/** Deletes a frame
 * @param[in]   _pstFrame       Frame to delete
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFrame_Delete(orxFRAME *_pstFrame)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFrame);

  /* Not referenced? */
  if(orxStructure_GetRefCounter(_pstFrame) == 0)
  {
    /* Deletes structure */
    orxStructure_Delete(_pstFrame);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Tried to delete frame when it was still referenced.");

    /* Referenced by others */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets a frame parent
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _pstParent      Parent frame to set
 */
void orxFASTCALL orxFrame_SetParent(orxFRAME *_pstFrame, orxFRAME *_pstParent)
{
  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFrame);

  /* Has no parent? */
  if(_pstParent == orxNULL)
  {
    /* Root is parent */
    orxStructure_SetParent(_pstFrame, sstFrame.pstRoot);
  }
  else
  {
    /* Sets parent */
    orxStructure_SetParent(_pstFrame, _pstParent);
  }

  /* Tags as dirty */
  orxFrame_SetDirty(_pstFrame);

  return;
}

/** Is a root child?
 * @param[in]   _pstFrame       Concerned frame
 * @return orxTRUE if its parent is root, orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxFrame_IsRootChild(const orxFRAME *_pstFrame)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFrame);

  /* Updates result*/
  bResult = orxFRAME(orxStructure_GetParent(_pstFrame)) == sstFrame.pstRoot;

  /* Done! */
  return bResult;
}

/** Is dirty?
 * @param[in]   _pstFrame       Concerned frame
 * @return orxTRUE if dirty, orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxFrame_IsDirty(const orxFRAME *_pstFrame)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFrame);

  /* Updates result*/
  bResult = orxStructure_TestFlags(_pstFrame, orxFRAME_KU32_FLAG_DIRTY);

  /* Done! */
  return bResult;
}

/** Sets a frame position
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _eSpace         Coordinate space system to use
 * @param[in]   _pvPos          Position to set
 */
void orxFASTCALL orxFrame_SetPosition(orxFRAME *_pstFrame, orxFRAME_SPACE _eSpace, const orxVECTOR *_pvPos)
{
  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFrame);
  orxASSERT(_pvPos != orxNULL);

  /* Local? */
  if(_eSpace == orxFRAME_SPACE_LOCAL)
  {
    /* Updates coord values */
    _orxFrame_SetPosition(_pstFrame, _pvPos, orxFRAME_SPACE_LOCAL);

    /* Tags as dirty */
    orxFrame_SetDirty(_pstFrame);
  }
  else
  {
    /* 2D data? */
    if(orxStructure_TestFlags(_pstFrame, orxFRAME_KU32_FLAG_DATA_2D) != orxFALSE)
    {
      const orxVECTOR *pvGlobalPos;

      /* Gets global position */
      pvGlobalPos = _orxFrame_GetPosition(_pstFrame, orxFRAME_SPACE_GLOBAL);

      /* Needs update? */
      if((pvGlobalPos->fX != _pvPos->fX)
      || (pvGlobalPos->fY != _pvPos->fY)
      || (pvGlobalPos->fZ != _pvPos->fZ))
      {
        orxVECTOR         vLocalPos, vParentScale;
        orxFLOAT          fParentAngle, fCos, fSin, fX, fY, fLocalX, fLocalY;
        const orxVECTOR  *pvParentPos;
        orxFRAME         *pstParentFrame;

        /* gets parent frame */
        pstParentFrame = orxFRAME(orxStructure_GetParent(_pstFrame));

        /* Gets parent's global data */
        pvParentPos   = _orxFrame_GetPosition(pstParentFrame, orxFRAME_SPACE_GLOBAL);
        fParentAngle  = _orxFrame_GetRotation(pstParentFrame, orxFRAME_SPACE_GLOBAL);
        _orxFrame_GetScale(pstParentFrame, orxFRAME_SPACE_GLOBAL, &vParentScale);

        /* Gets cosine & sine */
        fCos  = orxMath_Cos(fParentAngle);
        fSin  = orxMath_Sin(fParentAngle);

        /* Computes intermediate coordinates */
        fX = _pvPos->fX - pvParentPos->fX;
        fY = _pvPos->fY - pvParentPos->fY;

        /* Removes rotation from X&Y coordinates */
        fLocalX = (fX * fCos) + (fY * fSin);
        fLocalY = (fY * fCos) - (fX * fSin);

        /* Removes scale from X&Y coordinates */
        if(vParentScale.fX != orxFLOAT_0)
        {
          vLocalPos.fX = fLocalX / vParentScale.fX;
        }
        if(vParentScale.fY != orxFLOAT_0)
        {
          vLocalPos.fY = fLocalY / vParentScale.fX;
        }

        /* Z coordinate is not affected by rotation nor scale in 2D */
        vLocalPos.fZ = _pvPos->fZ - pvParentPos->fZ;

        /* Stores it */
        _orxFrame_SetPosition(_pstFrame, &vLocalPos, orxFRAME_SPACE_LOCAL);
        _orxFrame_SetPosition(_pstFrame, _pvPos, orxFRAME_SPACE_GLOBAL);

        /* Updates child status */
        orxFrame_SetFlagRecursively(orxFRAME(orxStructure_GetChild(_pstFrame)), orxFRAME_KU32_FLAG_DIRTY, orxFRAME_KU32_FLAG_NONE, orxTRUE);
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Frame does not support non-2d yet.");
    }
  }

  return;
}

/** Sets a frame rotation
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _eSpace         Coordinate space system to use
 * @param[in]   _fAngle         Angle to set (radians)
 */
void orxFASTCALL orxFrame_SetRotation(orxFRAME *_pstFrame, orxFRAME_SPACE _eSpace, orxFLOAT _fAngle)
{
  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFrame);

  /* Local or root child? */
  if((_eSpace == orxFRAME_SPACE_LOCAL) || (orxFrame_IsRootChild(_pstFrame) != orxFALSE))
  {
    /* Updates angle value */
    _orxFrame_SetRotation(_pstFrame, _fAngle, orxFRAME_SPACE_LOCAL);

    /* Tags as dirty */
    orxFrame_SetDirty(_pstFrame);
  }
  else
  {
    /* 2D data? */
    if(orxStructure_TestFlags(_pstFrame, orxFRAME_KU32_FLAG_DATA_2D) != orxFALSE)
    {
      orxFLOAT fGlobalAngle;

      /* Gets global angle */
      fGlobalAngle = _orxFrame_GetRotation(_pstFrame, orxFRAME_SPACE_GLOBAL);

      /* Needs update? */
      if(fGlobalAngle != _fAngle)
      {
        orxFLOAT fParentAngle, fLocalAngle;
        orxFRAME *pstParentFrame;

        /* gets parent frame */
        pstParentFrame = orxFRAME(orxStructure_GetParent(_pstFrame));

        /* Gets parent's global angle */
        fParentAngle = _orxFrame_GetRotation(pstParentFrame, orxFRAME_SPACE_GLOBAL);

        /* Gets local angle */
        fLocalAngle = _fAngle - fParentAngle;

        /* Stores it */
        _orxFrame_SetRotation(_pstFrame, fLocalAngle, orxFRAME_SPACE_LOCAL);
        _orxFrame_SetRotation(_pstFrame, _fAngle, orxFRAME_SPACE_GLOBAL);

        /* Updates child status */
        orxFrame_SetFlagRecursively(orxFRAME(orxStructure_GetChild(_pstFrame)), orxFRAME_KU32_FLAG_DIRTY, orxFRAME_KU32_FLAG_NONE, orxTRUE);
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Frame does not support non-2d yet.");
    }
  }

  return;
}

/** Sets a frame scale
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _eSpace         Coordinate space system to use
 * @param[in]   _pvScale        Scale to set
 */
void orxFASTCALL orxFrame_SetScale(orxFRAME *_pstFrame, orxFRAME_SPACE _eSpace, const orxVECTOR *_pvScale)
{
  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFrame);
  orxASSERT(_pvScale != orxNULL);

  /* Local or root child? */
  if((_eSpace == orxFRAME_SPACE_LOCAL) || (orxFrame_IsRootChild(_pstFrame) != orxFALSE))
  {
    /* Updates scale value */
    _orxFrame_SetScale(_pstFrame, _pvScale, orxFRAME_SPACE_LOCAL);

    /* Tags as dirty */
    orxFrame_SetDirty(_pstFrame);
  }
  else
  {
    /* 2D data? */
    if(orxStructure_TestFlags(_pstFrame, orxFRAME_KU32_FLAG_DATA_2D) != orxFALSE)
    {
      orxVECTOR vGlobalScale;

      /* Gets global scale */
      _orxFrame_GetScale(_pstFrame, orxFRAME_SPACE_GLOBAL, &vGlobalScale);

      /* Needs update? */
      if((vGlobalScale.fX != _pvScale->fX)
      || (vGlobalScale.fY != _pvScale->fY)
      || (vGlobalScale.fZ != _pvScale->fZ))
      {
        orxVECTOR vParentScale, vLocalScale;
        orxFRAME *pstParentFrame;

        /* gets parent frame */
        pstParentFrame = orxFRAME(orxStructure_GetParent(_pstFrame));

        /* Gets parent's global scale */
        _orxFrame_GetScale(pstParentFrame, orxFRAME_SPACE_GLOBAL, &vParentScale);

        /* Gets local scale */
        if(vParentScale.fX != orxFLOAT_0)
        {
          vLocalScale.fX = _pvScale->fX / vParentScale.fX;
        }
        if(vParentScale.fY != orxFLOAT_0)
        {
          vLocalScale.fY = _pvScale->fY / vParentScale.fY;
        }

        /* Stores it */
        _orxFrame_SetScale(_pstFrame, &vLocalScale, orxFRAME_SPACE_LOCAL);
        _orxFrame_SetScale(_pstFrame, _pvScale, orxFRAME_SPACE_GLOBAL);

        /* Updates child status */
        orxFrame_SetFlagRecursively(orxFRAME(orxStructure_GetChild(_pstFrame)), orxFRAME_KU32_FLAG_DIRTY, orxFRAME_KU32_FLAG_NONE, orxTRUE);
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Frame does not support non-2d yet.");
    }
  }

  return;
}

/** Gets a frame position
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _eSpace         Coordinate space system to use
 * @param[out]  _pvPos          Position of the given frame
 * @return orxVECTOR / orxNULL
 */
orxVECTOR *orxFASTCALL orxFrame_GetPosition(orxFRAME *_pstFrame, orxFRAME_SPACE _eSpace, orxVECTOR *_pvPos)
{
  orxVECTOR *pvResult;

  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFrame);
  orxASSERT(_pvPos != orxNULL);
  orxASSERT(_eSpace < orxFRAME_SPACE_NUMBER);

  /* Updates result */
  pvResult = _pvPos;

  /* Is a 2D Frame? */
  if(orxStructure_TestFlags(_pstFrame, orxFRAME_KU32_FLAG_DATA_2D) != orxFALSE)
  {
    const orxVECTOR *pvIntern = orxNULL;

    /* Depending on space */
    switch(_eSpace)
    {
      case orxFRAME_SPACE_GLOBAL:
      {
        /* Process dirty cell */
        orxFrame_ProcessDirty(_pstFrame);

        /* Gets requested position */
        pvIntern = _orxFrame_GetPosition(_pstFrame, orxFRAME_SPACE_GLOBAL);

        break;
      }

      case orxFRAME_SPACE_LOCAL:
      default:
      {
        /* Gets local position */
        pvIntern = _orxFrame_GetPosition(_pstFrame, orxFRAME_SPACE_LOCAL);

        break;
      }
    }

    /* Makes a copy */
    orxVector_Copy(_pvPos, (orxVECTOR *)pvIntern);
  }
  else
  {
    /* Resets coord structure */
    orxVector_SetAll(_pvPos, orxFLOAT_0);
  }

  /* Done! */
  return pvResult;
}

/** Gets a frame rotation
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _eSpace         Coordinate space system to use
 * @return Rotation of the given frame (radians)
 */
orxFLOAT orxFASTCALL orxFrame_GetRotation(orxFRAME *_pstFrame, orxFRAME_SPACE _eSpace)
{
  orxFLOAT fAngle = orxFLOAT_0;

  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFrame);
  orxASSERT(_eSpace < orxFRAME_SPACE_NUMBER);

  /* Is Frame 2D? */
  if(orxStructure_TestFlags(_pstFrame, orxFRAME_KU32_FLAG_DATA_2D) != orxFALSE)
  {
    /* Depending on space */
    switch(_eSpace)
    {
      case orxFRAME_SPACE_LOCAL:
      {
        /* Get rotation */
        fAngle = _orxFrame_GetRotation(_pstFrame, orxFRAME_SPACE_LOCAL);

        break;
      }

      case orxFRAME_SPACE_GLOBAL:
      default:
      {
        /* Process dirty cell */
        orxFrame_ProcessDirty(_pstFrame);

        /* Gets requested rotation */
        fAngle = _orxFrame_GetRotation(_pstFrame, orxFRAME_SPACE_GLOBAL);

        break;
      }
    }
  }

  /* Done! */
  return fAngle;
}

/** Gets a frame scale
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _eSpace         Coordinate space system to use
 * @param[out]  _pvScale        Scale
 * @return      orxVECTOR / orxNULL
 */
orxVECTOR *orxFASTCALL orxFrame_GetScale(orxFRAME *_pstFrame, orxFRAME_SPACE _eSpace, orxVECTOR *_pvScale)
{
  orxVECTOR *pvResult;

  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFrame);
  orxASSERT(_eSpace < orxFRAME_SPACE_NUMBER);
  orxASSERT(_pvScale != orxNULL);

  /* Is Frame 2D? */
  if(orxStructure_TestFlags(_pstFrame, orxFRAME_KU32_FLAG_DATA_2D) != orxFALSE)
  {
    /* Depending on space */
    switch(_eSpace)
    {
      case orxFRAME_SPACE_LOCAL:
      {
        /* Gets scale */
        pvResult = _orxFrame_GetScale(_pstFrame, orxFRAME_SPACE_LOCAL, _pvScale);

        break;
      }

      case orxFRAME_SPACE_GLOBAL:
      {
        /* Process dirty cell */
        orxFrame_ProcessDirty(_pstFrame);

        /* Gets requested scale */
        pvResult = _orxFrame_GetScale(_pstFrame, orxFRAME_SPACE_GLOBAL, _pvScale);

        break;
      }

      default:
      {
        /* Clears scale */
        orxVector_Copy(_pvScale, &orxVECTOR_0);

        /* Updates result */
        pvResult = orxNULL;

        break;
      }
    }
  }
  else
  {
    /* Clears scale */
    orxVector_Copy(_pvScale, &orxVECTOR_0);

    /* Updates result */
    pvResult = orxNULL;
  }

  /* Done! */
  return pvResult;
}
