/***************************************************************************
 orxFrame.c
 Frame module

 begin                : 02/12/2003
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


#include "object/orxFrame.h"

#include "debug/orxDebug.h"
#include "memory/orxMemory.h"
#include "object/orxStructure.h"


/*
 * Platform independant defines
 */

#define orxFRAME_KU32_FLAG_NONE             0x00000000
#define orxFRAME_KU32_FLAG_READY            0x00000001
#define orxFRAME_KU32_FLAG_DATA_2D          0x00000010

#define orxFRAME_KU32_FLAG_DEFAULT          0x00000010


#define orxFRAME_KU32_ID_FLAG_NONE          0x00000000
#define orxFRAME_KU32_ID_FLAG_DATA_2D       0x00000010
#define orxFRAME_KU32_ID_FLAG_VALUE_DIRTY   0x10000000
#define orxFRAME_KU32_ID_FLAG_RENDER_DIRTY  0x20000000
#define orxFRAME_KU32_ID_FLAG_DIRTY         0x30000000
#define orxFRAME_KU32_ID_FLAG_SCROLL_X      0x01000000
#define orxFRAME_KU32_ID_FLAG_SCROLL_Y      0x02000000
#define orxFRAME_KU32_ID_FLAG_SCROLL_BOTH   0x03000000

/*
 * Frame space
 */
typedef enum __orxFRAME_SPACE_t
{
  orxFRAME_SPACE_GLOBAL = 0,
  orxFRAME_SPACE_LOCAL,
  
  orxFRAME_SPACE_NUMBER,
  
  orxFRAME_SPACE_NONE = orxENUM_NONE
  
} orxFRAME_SPACE;

/*
 * Internal 2D Frame Data structure
 */
typedef struct __orxFRAME_DATA_2D_t
{
  /* Global 2D coordinates : 16 */
  orxVECTOR vGlobalPos;
  /* Local 2D coordinates : 32 */
  orxVECTOR vLocalCoord;

  /* Global 2D rotation angle : 36 */
  orxFLOAT fGlobalAngle;
  /* Global 2D isometric scale : 40 */
  orxFLOAT fGlobalScale;
  /* Local 2D rotation angle : 44 */
  orxFLOAT fLocalAngle;
  /* Local 2D isometric scale : 48 */
  orxFLOAT fLocalScale;

  /* Scroll coefficients used for differential scrolling : 64 */
  orxVECTOR vScroll;

} orxFRAME_DATA_2D;


/*
 * Frame structure
 */
struct __orxFRAME_t
{
  /* Public structure, first structure member : 16 */
  orxSTRUCTURE stStructure;

  /* Internal id flags : 20 */
  orxU32 u32IDFlags;

  /* Data : 24 */
  orxVOID *pstData;

  /* Padding */
  orxPAD(24)
};

/*
 * Static structure
 */
typedef struct __orxFRAME_STATIC_t
{
  /* Control flags */
  orxU32 u32Flags;

  /* Frames root */
  orxFRAME *pstRoot;

} orxFRAME_STATIC;


/*
 * Static data
 */
orxSTATIC orxFRAME_STATIC sstFrame;


/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 _orxFrame_SetPosition
 Sets a 2D frame local coord

 returns: orxVOID
 ***************************************************************************/
orxSTATIC orxINLINE orxVOID _orxFrame_SetPosition(orxFRAME *_pstFrame, orxCONST orxVECTOR *_pvPos, orxFRAME_SPACE eSpace)
{
  /* Checks */
  orxASSERT(_pstFrame != orxNULL);
  orxASSERT(_pvPos != orxNULL);

  /* According to space */
  switch(eSpace)
  {
    case orxFRAME_SPACE_GLOBAL:

      orxVector_Copy(&(((orxFRAME_DATA_2D *)(_pstFrame->pstData))->vGlobalPos), _pvPos);

      break;

    case orxFRAME_SPACE_LOCAL:

      orxVector_Copy(&(((orxFRAME_DATA_2D *)(_pstFrame->pstData))->vLocalCoord), _pvPos);

      break;

    default:

      /* Wrong space */
      /* !!! MSG !!! */

      break;
  }

  return;
}

/***************************************************************************
 _orxFrame_SetAngle
 Sets a 2D frame local angle

 returns: orxVOID
 ***************************************************************************/
orxSTATIC orxINLINE orxVOID _orxFrame_SetAngle(orxFRAME *_pstFrame, orxFLOAT _fAngle, orxFRAME_SPACE eSpace)
{
  /* Checks */
  orxASSERT((_pstFrame != orxNULL));

  /* According to coord type */
  switch(eSpace)
  {
    case orxFRAME_SPACE_GLOBAL:

      ((orxFRAME_DATA_2D *)(_pstFrame->pstData))->fGlobalAngle  = _fAngle;

      break;

    case orxFRAME_SPACE_LOCAL:

      ((orxFRAME_DATA_2D *)(_pstFrame->pstData))->fLocalAngle   = _fAngle;

      break;

    default:

      /* Wrong coord type */
      /* !!! MSG !!! */

      break;
  }

  return;
}

/***************************************************************************
 _orxFrame_SetScale
 Sets a 2D frame local scale

 returns: orxVOID
 ***************************************************************************/
orxSTATIC orxINLINE orxVOID _orxFrame_SetScale(orxFRAME *_pstFrame, orxFLOAT _fScale, orxFRAME_SPACE eSpace)
{
  /* Checks */
  orxASSERT((_pstFrame != orxNULL));

  /* According to coord type */
  switch(eSpace)
  {
    case orxFRAME_SPACE_GLOBAL:

      ((orxFRAME_DATA_2D *)(_pstFrame->pstData))->fGlobalScale = _fScale;

      break;

    case orxFRAME_SPACE_LOCAL:

      ((orxFRAME_DATA_2D *)(_pstFrame->pstData))->fLocalScale = _fScale;

      break;

    default:

      /* Wrong coord type */
      /* !!! MSG !!! */

      break;
  }

  return;
}

/***************************************************************************
 _orxFrame_GetPosition
 Gets a 2D frame local/global coord

 returns: Internal coord data pointer
 ***************************************************************************/
orxSTATIC orxINLINE orxCONST orxVECTOR *_orxFrame_GetPosition(orxCONST orxFRAME *_pstFrame, orxFRAME_SPACE eSpace)
{
  orxVECTOR *pvPos = orxNULL;

  /* Checks */
  orxASSERT((_pstFrame != orxNULL));

  /* According to coord type */
  switch(eSpace)
  {
    case orxFRAME_SPACE_GLOBAL:

      pvPos = &(((orxFRAME_DATA_2D *)(_pstFrame->pstData))->vGlobalPos);

      break;

    case orxFRAME_SPACE_LOCAL:

      pvPos = &(((orxFRAME_DATA_2D *)(_pstFrame->pstData))->vLocalCoord);

      break;

    default:

      /* Wrong coord type */
      /* !!! MSG !!! */

      break;
  }

  /* Done */
  return pvPos;
}

/***************************************************************************
 _orxFrame_GetAngle
 Gets a 2D frame local/global angle

 returns: Requested angle value
 ***************************************************************************/
orxSTATIC orxINLINE orxFLOAT _orxFrame_GetAngle(orxCONST orxFRAME *_pstFrame, orxFRAME_SPACE eSpace)
{
  orxFLOAT fAngle = orxFLOAT_0;

  /* Checks */
  orxASSERT((_pstFrame != orxNULL));

  /* According to coord type */
  switch(eSpace)
  {
    case orxFRAME_SPACE_GLOBAL:

      fAngle = ((orxFRAME_DATA_2D *)(_pstFrame->pstData))->fGlobalAngle;

      break;

    case orxFRAME_SPACE_LOCAL:

      fAngle = ((orxFRAME_DATA_2D *)(_pstFrame->pstData))->fLocalAngle;

      break;

    default:

      /* Wrong coord type */
      /* !!! MSG !!! */

      break;
  }

  /* Done */
  return fAngle;
}

/***************************************************************************
 _orxFrame_GetScale
 Gets a 2D frame local/global scale

 returns: Requested scale value
 ***************************************************************************/
orxSTATIC orxINLINE orxFLOAT _orxFrame_GetScale(orxCONST orxFRAME *_pstFrame, orxFRAME_SPACE eSpace)
{
  orxFLOAT fScale = orxFLOAT_1;
 
  /* Checks */
  orxASSERT((_pstFrame != orxNULL));

  /* According to coord type */
  switch(eSpace)
  {
    case orxFRAME_SPACE_GLOBAL:
      fScale = ((orxFRAME_DATA_2D *)(_pstFrame->pstData))->fGlobalScale;
      break;

    case orxFRAME_SPACE_LOCAL:
      fScale = ((orxFRAME_DATA_2D *)(_pstFrame->pstData))->fLocalScale;
      break;

    default:
      /* Wrong coord type */
      /* !!! MSG !!! */
      break;
  }

  /* Done */
  return fScale;
}

/***************************************************************************
 orxFrame_UpdateData
 Updates frame global data using parent's global and frame local ones.
 Result can be stored in a third party frame.

 returns: orxVOID
 ***************************************************************************/
orxSTATIC orxVOID orxFrame_UpdateData(orxFRAME *_pstDstFrame, orxCONST orxFRAME *_pstSrcFrame, orxCONST orxFRAME *_pstParentFrame)
{
  /* Checks */
  orxASSERT((_pstSrcFrame != orxNULL));
  orxASSERT((_pstParentFrame != orxNULL));

  /* 2D data? */
  if(_pstSrcFrame->u32IDFlags & orxFRAME_KU32_ID_FLAG_DATA_2D)
  {
    orxVECTOR vTempPos;
    orxCONST orxVECTOR *pvParentPos, *pvPos;
    orxFLOAT fParentAngle, fParentScale, fAngle, fScale;
    orxFLOAT fX, fY, fLocalX, fLocalY, fCos, fSin;

    /* Gets parent's global data */
    pvParentPos   = _orxFrame_GetPosition(_pstParentFrame, orxFRAME_SPACE_GLOBAL);
    fParentAngle  = _orxFrame_GetAngle(_pstParentFrame, orxFRAME_SPACE_GLOBAL);
    fParentScale  = _orxFrame_GetScale(_pstParentFrame, orxFRAME_SPACE_GLOBAL);

    /* Gets frame's local coord */
    pvPos         = _orxFrame_GetPosition(_pstSrcFrame, orxFRAME_SPACE_LOCAL);

    /* Updates angle */
    fAngle        = _orxFrame_GetAngle(_pstSrcFrame, orxFRAME_SPACE_LOCAL) + fParentAngle;

    /* Updates scale */
    fScale        = _orxFrame_GetScale(_pstSrcFrame, orxFRAME_SPACE_LOCAL) * fParentScale;

    /* Updates coord */
    /* Gets needed orxFLOAT values for rotation & scale applying */
    fLocalX       = pvPos->fX;
    fLocalY       = pvPos->fY;
    fCos          = cosf(fParentAngle);
    fSin          = sinf(fParentAngle);

    /* Applies rotation & scale on X & Y coordinates*/
    fX            = fParentScale * ((fLocalX * fCos) - (fLocalY * fSin));
    fY            = fParentScale * ((fLocalX * fSin) + (fLocalY * fCos));

    /* Computes final global coordinates */
    vTempPos.fX   = floorf(fX) + pvParentPos->fX;
    vTempPos.fY   = floorf(fY) + pvParentPos->fY;

    /* Z coordinate is not affected by rotation nor scale in 2D */
    vTempPos.fZ   = pvParentPos->fZ + pvPos->fZ;

    /* Stores them */
    _orxFrame_SetAngle(_pstDstFrame, fAngle, orxFRAME_SPACE_GLOBAL);
    _orxFrame_SetScale(_pstDstFrame, fScale, orxFRAME_SPACE_GLOBAL);
    _orxFrame_SetPosition(_pstDstFrame, &vTempPos, orxFRAME_SPACE_GLOBAL);
  }
  else
  {
    /* !!! MSG !!! */
  }

 return;
}

/***************************************************************************
 orxFrame_ProcessDirty
 Process a dirty frame and all its dirty ancestors.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFrame_ProcessDirty(orxFRAME *_pstFrame)
{
  orxFRAME *pstParentFrame;

  /* Checks */
  orxASSERT(_pstFrame != orxNULL);

  /* gets parent frame */
  pstParentFrame = (orxFRAME *)orxStructure_GetParent((orxSTRUCTURE *)_pstFrame);

  /* Is cell dirty & has parent? */
  if((_pstFrame->u32IDFlags & orxFRAME_KU32_ID_FLAG_VALUE_DIRTY)
  && (pstParentFrame != orxNULL))
  {
    /* Updates parent status */
    orxFrame_ProcessDirty(pstParentFrame);

    /* Updates frame global data */
    orxFrame_UpdateData(_pstFrame, _pstFrame, pstParentFrame);
  }

  /* Updates cell dirty status */
  _pstFrame->u32IDFlags &= ~orxFRAME_KU32_ID_FLAG_VALUE_DIRTY;

 return;
}

/***************************************************************************
 orxFrame_SetFlagRecursively
 Sets a frame and all its heirs as requested.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFrame_SetFlagRecursively(orxFRAME *_pstFrame, orxU32 _u32AddFlags, orxU32 _u32RemoveFlags, orxBOOL _bRecursed)
{
  /* Non null? */
  if(_pstFrame != orxNULL)
  {
    /* Updates child status */
    orxFrame_SetFlagRecursively((orxFRAME *)orxStructure_GetChild((orxSTRUCTURE *)_pstFrame), _u32AddFlags, _u32RemoveFlags, orxTRUE);

    /* Recursed? */
    if(_bRecursed)
    {
      /* Updates siblings status */
      orxFrame_SetFlagRecursively((orxFRAME *)orxStructure_GetSibling((orxSTRUCTURE *)_pstFrame), _u32AddFlags, _u32RemoveFlags, orxTRUE);
    }

    /* Updates cell flags */
    _pstFrame->u32IDFlags &= ~(_u32RemoveFlags);
    _pstFrame->u32IDFlags |= _u32AddFlags;
  }

 return;
}


/***************************************************************************
 orxFrame_SetDirty
 Sets a frame and all its heirs as dirty.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFrame_SetDirty(orxFRAME *_pstFrame)
{
  /* Checks */
  orxASSERT(_pstFrame != orxNULL);

  /* Adds dirty flags (render + value) to all frame's heirs */
  orxFrame_SetFlagRecursively(_pstFrame, orxFRAME_KU32_ID_FLAG_DIRTY, orxFRAME_KU32_ID_FLAG_NONE, orxFALSE);

 return;
}

/***************************************************************************
 orxFrame_DeleteAll
 Deletes all the frames stored in the tree and cleans it.

 returns: orxVOID
 ***************************************************************************/
orxSTATIC orxVOID orxFrame_DeleteAll()
{
  orxFRAME *pstFrame;
  
  /* Gets first frame */
  pstFrame = (orxFRAME *)orxStructure_GetChild((orxSTRUCTURE *)sstFrame.pstRoot);

  /* Untill only root remains */
  while(pstFrame != orxNULL)
  {
    /* Deletes firt child cell */
    orxFrame_Delete(pstFrame);

    /* Gets root new child */
    pstFrame = (orxFRAME *)orxStructure_GetChild((orxSTRUCTURE *)sstFrame.pstRoot);
  }

  /* Removes root */
  orxFrame_Delete(sstFrame.pstRoot);
  sstFrame.pstRoot = orxNULL;

  return;
}


/***************************************************************************
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 orxFrame_Setup
 Frame module setup.

 returns: nothing
 ***************************************************************************/
orxVOID orxFrame_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_FRAME, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_FRAME, orxMODULE_ID_STRUCTURE);

  return;
}

/***************************************************************************
 orxFrame_Init
 Inits frame system.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 ***************************************************************************/
orxSTATUS orxFrame_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstFrame.u32Flags & orxFRAME_KU32_FLAG_READY))
  {
    /* Cleans control structure */
    orxMemory_Set(&sstFrame, 0, sizeof(orxFRAME_STATIC));

    /* Inits ID Flags */
    sstFrame.u32Flags = orxFRAME_KU32_FLAG_DEFAULT|orxFRAME_KU32_FLAG_READY;

    /* Registers structure type */
    eResult = orxSTRUCTURE_REGISTER(FRAME, orxSTRUCTURE_STORAGE_TYPE_TREE, orxMEMORY_TYPE_MAIN, orxNULL);

    /* Successful? */
    if(eResult == orxSTATUS_SUCCESS)
    {
      /* Inits frame tree */
      sstFrame.pstRoot = orxFrame_Create();

      /* Not created? */
      if(sstFrame.pstRoot == orxNULL)
      {
        /* Unregister structure type */
        orxStructure_Unregister(orxSTRUCTURE_ID_FRAME);

        /* Cleans flags */
        sstFrame.u32Flags = orxFRAME_KU32_FLAG_NONE;
  
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
    /* !!! MSG !!! */

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }
  
  /* Done! */
  return eResult;
}

/***************************************************************************
 orxFrame_Exit
 Exits from frame system.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFrame_Exit()
{
  /* Initialized? */
  if(sstFrame.u32Flags & orxFRAME_KU32_FLAG_READY)
  {
    /* Deletes frame tree */
    orxFrame_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_FRAME);

    /* Updates flags */
    sstFrame.u32Flags &= ~orxFRAME_KU32_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}

/***************************************************************************
 orxFrame_Create
 Creates a new frame.

 returns: Created frame.
 ***************************************************************************/
orxFRAME *orxFrame_Create()
{
  orxFRAME *pstFrame;

  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_FLAG_READY);

  /* Creates frame */
  pstFrame = (orxFRAME *)orxStructure_Create(orxSTRUCTURE_ID_FRAME);

  /* Non null? */
  if(pstFrame != orxNULL)
  {
    /* Inits members */
    if(sstFrame.u32Flags & orxFRAME_KU32_FLAG_DATA_2D)
    {
      orxFRAME_DATA_2D *pstData;

      /* Updates flags */
      pstFrame->u32IDFlags = orxFRAME_KU32_ID_FLAG_DATA_2D;

      /* Allocates data memory */
      pstData = (orxFRAME_DATA_2D *) orxMemory_Allocate(sizeof(orxFRAME_DATA_2D), orxMEMORY_TYPE_MAIN);

      /* Inits & assigns it */
      if(pstData != orxNULL)
      {
        /* Cleans it */
        orxMemory_Set(pstData, 0, sizeof(orxFRAME_DATA_2D));

        /* Inits values */
        pstData->fGlobalScale = orxFLOAT_1;
        pstData->fLocalScale = orxFLOAT_1;

        /* Links data to frame */
        pstFrame->pstData = pstData;
      }
      else
      {
        /* Deletes partially created frame */
        orxStructure_Delete((orxSTRUCTURE *)pstFrame);

        /* Not created */
        pstFrame = orxNULL;
      }
    }
    else
    {
      /* !!! MSG !!! */
    }
  }
  else
  {
    /* !!! MSG !!! */
  }

  return pstFrame;
}

/***************************************************************************
 orxFrame_Delete
 Deletes a frame.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 ***************************************************************************/
orxSTATUS orxFASTCALL orxFrame_Delete(orxFRAME *_pstFrame)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_FLAG_READY);
  orxASSERT(_pstFrame != orxNULL);

  /* Not referenced? */
  if(orxStructure_GetRefCounter((orxSTRUCTURE *)_pstFrame) == 0)
  {
    /* Cleans data */
    if(_pstFrame->u32IDFlags & orxFRAME_KU32_ID_FLAG_DATA_2D)
    {
      /* Frees frame data memory */
      orxMemory_Free(_pstFrame->pstData);
    }

    /* Deletes structure */
    orxStructure_Delete((orxSTRUCTURE *)_pstFrame);
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
 orxFrame_IsRenderStatusClean
 Test frame render status (TRUE : clean / orxFALSE : dirty)

 returns: orxTRUE (clean) / orxFALSE (dirty)
 ***************************************************************************/
orxBOOL orxFASTCALL orxFrame_IsRenderStatusClean(orxCONST orxFRAME *_pstFrame)
{
  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_FLAG_READY);
  orxASSERT(_pstFrame != orxNULL);

  /* Test render dirty flag */
  return((_pstFrame->u32IDFlags & orxFRAME_KU32_ID_FLAG_RENDER_DIRTY) ? orxTRUE : orxFALSE);
}

/***************************************************************************
 orxFrame_CleanAllRenderStatus
 Cleans all frames render status

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFrame_CleanAllRenderStatus()
{
  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_FLAG_READY);

  /* Removes render dirty flag from all frames */
  orxFrame_SetFlagRecursively(sstFrame.pstRoot, orxFRAME_KU32_ID_FLAG_NONE, orxFRAME_KU32_ID_FLAG_RENDER_DIRTY, orxFALSE);

  return;
}

/***************************************************************************
 orxFrame_HasDifferentialScrolling
 Does frame use differential scrolling?

 returns: orxTRUE/FALSE
 ***************************************************************************/
orxBOOL orxFASTCALL orxFrame_HasDifferentialScrolling(orxCONST orxFRAME *_pstFrame)
{
  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_FLAG_READY);
  orxASSERT(_pstFrame != orxNULL);

  return((_pstFrame->u32IDFlags & orxFRAME_KU32_ID_FLAG_SCROLL_BOTH) ? orxTRUE : orxFALSE);
}

/***************************************************************************
 orxFrame_GetDifferentialScrolling
 Gets frame differential scrolling (X & Y axis)

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFASTCALL orxFrame_GetDifferentialScrolling(orxCONST orxFRAME * _pstFrame, orxVECTOR *_pvScroll)
{
  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_FLAG_READY);
  orxASSERT(_pstFrame != orxNULL);
  orxASSERT(_pvScroll != orxNULL);

  /* Use 2D data? */
  if(_pstFrame->u32IDFlags & orxFRAME_KU32_ID_FLAG_DATA_2D)
  {
    /* Uses X scroll? */
    if(_pstFrame->u32IDFlags & orxFRAME_KU32_ID_FLAG_SCROLL_X)
    {
      /* Stores value */
      _pvScroll->fX = ((orxFRAME_DATA_2D *)(_pstFrame->pstData))->vScroll.fX;
    }
    else
    {
      /* Stores value */
      _pvScroll->fX = orxFLOAT_0;
    }

    /* Uses Y scroll? */
    if(_pstFrame->u32IDFlags & orxFRAME_KU32_ID_FLAG_SCROLL_Y)
    {
      /* Stores value */
      _pvScroll->fY = ((orxFRAME_DATA_2D *)(_pstFrame->pstData))->vScroll.fY;
    }
    else
    {
      /* Stores value */
      _pvScroll->fY = orxFLOAT_0;
    }
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}

/***************************************************************************
 orxFrame_SetDifferentialScrolling
 Sets frame differential scrolling (X & Y axis)

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFASTCALL orxFrame_SetDifferentialScrolling(orxFRAME * _pstFrame, orxCONST orxVECTOR *_pvScroll)
{
  orxU32 u32AddFlags = orxFRAME_KU32_ID_FLAG_NONE, u32RemoveFlags = orxFRAME_KU32_ID_FLAG_NONE;

  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_FLAG_READY);
  orxASSERT(_pstFrame != orxNULL);
  orxASSERT(_pvScroll != orxNULL);

  /* Use 2D data? */
  if(_pstFrame->u32IDFlags & orxFRAME_KU32_ID_FLAG_DATA_2D)
  {
    /* Enables X axis differential scrolling? */
    if(_pvScroll->fX != orxFLOAT_0)
    {
      u32AddFlags     |= orxFRAME_KU32_ID_FLAG_SCROLL_X;
    }
    else
    {
      u32RemoveFlags  |= orxFRAME_KU32_ID_FLAG_SCROLL_X;
    }

    /* Enables Y axis differential scrolling? */
    if(_pvScroll->fY != 0.0)
    {
      u32AddFlags     |= orxFRAME_KU32_ID_FLAG_SCROLL_Y;
    }
    else
    {
      u32RemoveFlags  |= orxFRAME_KU32_ID_FLAG_SCROLL_Y;
    }

    /* Updates scroll values */
    orxVector_Copy(&(((orxFRAME_DATA_2D *)(_pstFrame->pstData))->vScroll), _pvScroll);

    /* Updates flags on frame and its heirs */
    orxFrame_SetFlagRecursively(_pstFrame, u32AddFlags, u32RemoveFlags, orxFALSE);
  }

  return;
}

/***************************************************************************
 orxFrame_SetParent
 Sets a frame parent & updates links.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFASTCALL orxFrame_SetParent(orxFRAME *_pstFrame, orxFRAME *_pstParent)
{
  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_FLAG_READY);
  orxASSERT(_pstFrame != orxNULL);

  /* Has no parent? */
  if(_pstParent == orxNULL)
  {
    /* Root is parent */
    orxStructure_SetParent((orxSTRUCTURE *)_pstFrame, (orxSTRUCTURE *)sstFrame.pstRoot);
  }
  else
  {
    /* Sets parent */
    orxStructure_SetParent((orxSTRUCTURE *)_pstFrame, (orxSTRUCTURE *)_pstParent);
  }

  /* Tags as dirty */
  orxFrame_SetDirty(_pstFrame);

  return;
}

/***************************************************************************
 orxFrame_SetPosition
 Sets a 2D frame local position

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFASTCALL orxFrame_SetPosition(orxFRAME *_pstFrame, orxCONST orxVECTOR *_pvPos)
{
  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_FLAG_READY);
  orxASSERT(_pstFrame != orxNULL);
  orxASSERT(_pvPos != orxNULL);

  /* Updates coord values */
  _orxFrame_SetPosition(_pstFrame, _pvPos, orxFRAME_SPACE_LOCAL);

  /* Tags as dirty */
  orxFrame_SetDirty(_pstFrame);

  return;
}

/***************************************************************************
 orxFrame_SetRotation
 Sets a 2D frame local rotation

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFASTCALL orxFrame_SetRotation(orxFRAME *_pstFrame, orxFLOAT _fAngle)
{
  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_FLAG_READY);
  orxASSERT(_pstFrame != orxNULL);

  /* Updates angle value */
  _orxFrame_SetAngle(_pstFrame, _fAngle, orxFRAME_SPACE_LOCAL);

  /* Tags as dirty */
  orxFrame_SetDirty(_pstFrame);

  return;
}

/***************************************************************************
 orxFrame_SetScale
 Sets a 2D frame local scale

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFASTCALL orxFrame_SetScale(orxFRAME *_pstFrame, orxFLOAT _fScale)
{
  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_FLAG_READY);
  orxASSERT(_pstFrame != orxNULL);

  /* Updates scale value */
  _orxFrame_SetScale(_pstFrame, _fScale, orxFRAME_SPACE_LOCAL);

  /* Tags as dirty */
  orxFrame_SetDirty(_pstFrame);

  return;
}

/***************************************************************************
 orxFrame_GetPosition
 Gets a 2D frame local/global position

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFASTCALL orxFrame_GetPosition(orxFRAME *_pstFrame, orxVECTOR *_pvPos, orxBOOL _bLocal)
{
  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_FLAG_READY);
  orxASSERT(_pstFrame != orxNULL);
  orxASSERT(_pvPos != orxNULL);

  /* Is Frame 2D? */
  if(_pstFrame->u32IDFlags & orxFRAME_KU32_ID_FLAG_DATA_2D)
  {
    orxCONST orxVECTOR *pvIntern = orxNULL;

    /* Local coordinates? */
    if(_bLocal != orxFALSE)
    {
      pvIntern = _orxFrame_GetPosition(_pstFrame, orxFRAME_SPACE_LOCAL);
    }
    else
    {
      /* Process dirty cell */
      orxFrame_ProcessDirty(_pstFrame);

      /* Gets requested position */
      pvIntern = _orxFrame_GetPosition(_pstFrame, orxFRAME_SPACE_GLOBAL);
    }

    /* Makes a copy */
    orxVector_Copy(_pvPos, (orxVECTOR *)pvIntern);
  }
  else
  {
    /* Resets coord structure */
    orxVector_SetAll(_pvPos, orxFLOAT_0);
  }

  return;
}

/***************************************************************************
 orxFrame_GetRotation
 Gets a 2D frame local/global rotation

 returns: Requested rotation value
 ***************************************************************************/
orxFLOAT orxFASTCALL orxFrame_GetRotation(orxFRAME *_pstFrame, orxBOOL _bLocal)
{
  orxFLOAT fAngle = orxFLOAT_0;

  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_FLAG_READY);
  orxASSERT(_pstFrame != orxNULL);
 
  /* Is Frame 2D? */
  if(_pstFrame->u32IDFlags & orxFRAME_KU32_ID_FLAG_DATA_2D)
  {
    /* Local coordinates? */
    if(_bLocal != orxFALSE)
    {
      fAngle = _orxFrame_GetAngle(_pstFrame, orxFRAME_SPACE_LOCAL);
    }
    else
    {
      /* Process dirty cell */
      orxFrame_ProcessDirty(_pstFrame);

      /* Gets requested rotation */
      fAngle = _orxFrame_GetAngle(_pstFrame, orxFRAME_SPACE_GLOBAL);
    }
  }

  return fAngle;
}

/***************************************************************************
 orxFrame_GetScale
 Gets a 2D frame local/global scale

 returns: Requested scale value
 ***************************************************************************/
orxFLOAT orxFASTCALL orxFrame_GetScale(orxFRAME *_pstFrame, orxBOOL _bLocal)
{
  orxFLOAT fScale = orxFLOAT_1;

  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_FLAG_READY);
  orxASSERT(_pstFrame != orxNULL);
 
  /* Is Frame 2D? */
  if(_pstFrame->u32IDFlags & orxFRAME_KU32_ID_FLAG_DATA_2D)
  {
    /* Local coordinates? */
    if(_bLocal != orxFALSE)
    {
      fScale = _orxFrame_GetScale(_pstFrame, orxFRAME_SPACE_LOCAL);
    }
    else
    {
      /* Process dirty cell */
      orxFrame_ProcessDirty(_pstFrame);

      /* Gets requested scale */
      fScale = _orxFrame_GetScale(_pstFrame, orxFRAME_SPACE_GLOBAL);
    }
  }

  return fScale;
}

/***************************************************************************
 orxFrame_ComputeGlobalData
 Computes frame global data using parent's global and frame local ones.
 Result is stored in a third party frame.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFASTCALL orxFrame_ComputeGlobalData(orxFRAME *_pstDstFrame, orxCONST orxFRAME *_pstSrcFrame, orxCONST orxFRAME *_pstParentFrame)
{
  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_FLAG_READY);
  orxASSERT(_pstSrcFrame != orxNULL);
  orxASSERT(_pstParentFrame != orxNULL);

  /* Not self updating? */
  if(_pstDstFrame != _pstSrcFrame)
  {
    /* Computes frame global data */
    orxFrame_UpdateData(_pstDstFrame, _pstSrcFrame, _pstParentFrame);
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}
