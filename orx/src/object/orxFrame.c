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
  
  orxFRAME_SPACE_NONE = 0xFFFFFFFF
  
} orxFRAME_SPACE;

/*
 * Internal 2D Frame Data structure
 */
typedef struct __orxFRAME_DATA_2D_t
{
  /* Global 2D coordinates : 16 */
  orxVEC vGlobalPos;
  /* Local 2D coordinates : 32 */
  orxVEC vLocalCoord;

  /* Global 2D rotation angle : 36 */
  orxFLOAT fGlobalAngle;
  /* Global 2D isometric scale : 40 */
  orxFLOAT fGlobalScale;
  /* Local 2D rotation angle : 44 */
  orxFLOAT fLocalAngle;
  /* Local 2D isometric scale : 48 */
  orxFLOAT fLocalScale;

  /* Scroll coefficients used for differential scrolling : 64 */
  orxVEC vScroll;

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

  /* 8 extra bytes of padding : 32 */
  orxU8 au8Unused[8];
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
static orxFRAME_STATIC sstFrame;


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
inline orxVOID _orxFrame_SetPosition(orxFRAME *_pstFrame, orxVEC *_pvPos, orxFRAME_SPACE eSpace)
{
  /* Checks */
  orxASSERT(_pstFrame != orxNULL);
  orxASSERT(_pvPos != orxNULL);

  /* According to space */
  switch(eSpace)
  {
    case orxFRAME_SPACE_GLOBAL:

      coord_copy(&(((orxFRAME_DATA_2D *)(_pstFrame->pstData))->vGlobalPos), _pvPos);

      break;

    case orxFRAME_SPACE_LOCAL:

      coord_copy(&(((orxFRAME_DATA_2D *)(_pstFrame->pstData))->vLocalCoord), _pvPos);

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
inline orxVOID _orxFrame_SetAngle(orxFRAME *_pstFrame, orxFLOAT _fAngle, orxFRAME_SPACE eSpace)
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
inline orxVOID _orxFrame_SetScale(orxFRAME *_pstFrame, orxFLOAT _fScale, orxFRAME_SPACE eSpace)
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
inline orxCONST orxVEC *_orxFrame_GetPosition(orxFRAME *_pstFrame, orxFRAME_SPACE eSpace)
{
  orxVEC *pvPos = orxNULL;

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
inline orxFLOAT _orxFrame_GetAngle(orxFRAME *_pstFrame, orxFRAME_SPACE eSpace)
{
  orxFLOAT fAngle = 0.0f;

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
inline orxFLOAT _orxFrame_GetScale(orxFRAME *_pstFrame, orxFRAME_SPACE eSpace)
{
  orxFLOAT fScale = 1.0f;
 
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
inline orxVOID orxFrame_UpdateData(orxFRAME *_pstDstFrame, orxFRAME *_pstSrcFrame, orxFRAME *_pstParentFrame)
{
  /* Checks */
  orxASSERT((_pstSrcFrame != orxNULL));
  orxASSERT((_pstParentFrame != orxNULL));

  /* 2D data? */
  if(_pstSrcFrame->u32IDFlags & orxFRAME_KU32_ID_FLAG_DATA_2D)
  {
    orxVEC vTempPos;
    orxCONST orxVEC *pvParentPos, *pvPos;
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
    vTempPos.fX   = rintf(fX) + pvParentPos->fX;
    vTempPos.fY   = rintf(fY) + pvParentPos->fY;

    /* Z coordinate is not affected by rotation nor scale in 2D */
    vTempPos.fZ   = pvParentPos->fZ + pvPos->fZ;

    /* Stores them */
    /* Is destination frame non null? */
    if(_pstDstFrame != orxNULL)
    {
      _orxFrame_SetAngle(_pstDstFrame, fAngle, orxFRAME_SPACE_GLOBAL);
      _orxFrame_SetScale(_pstDstFrame, fScale, orxFRAME_SPACE_GLOBAL);
      _orxFrame_SetPosition(_pstDstFrame, &vTempPos, orxFRAME_SPACE_GLOBAL);
    }
    else
    {
      _orxFrame_SetAngle(_pstSrcFrame, fAngle, orxFRAME_SPACE_GLOBAL);
      _orxFrame_SetScale(_pstSrcFrame, fScale, orxFRAME_SPACE_GLOBAL);
      _orxFrame_SetPosition(_pstSrcFrame, &vTempPos, orxFRAME_SPACE_GLOBAL);
    }
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
    orxFrame_UpdateData(orxNULL, _pstFrame, pstParentFrame);
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
      orxFrame_SetFlagRecursively((orxFRAME *)orxStructure_GetRightSibling((orxSTRUCTURE *)_pstFrame), _u32AddFlags, _u32RemoveFlags, orxTRUE);
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
inline orxVOID orxFrame_DeleteAll()
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
 orxFrame_Init
 Inits frame system.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxFrame_Init()
{
  /* Already Initialized? */
  if(sstFrame.u32Flags & orxFRAME_KU32_FLAG_READY)
  {
    /* !!! MSG !!! */

    return orxSTATUS_FAILED;
  }

  /* Cleans static controller */
  orxMemory_Set(&sstFrame, 0, sizeof(orxFRAME_STATIC));

  /* Inits ID Flags */
  sstFrame.u32Flags = orxFRAME_KU32_FLAG_DEFAULT|orxFRAME_KU32_FLAG_READY;

  /* Inits frame tree */
  sstFrame.pstRoot = orxFrame_Create();

  /* Not created? */
  if(sstFrame.pstRoot == orxNULL)
  {
    /* Cleans flags */
    sstFrame.u32Flags = orxFRAME_KU32_FLAG_NONE;

    return orxSTATUS_FAILED;
  }

  /* Done! */
  return orxSTATUS_FAILED;
}

/***************************************************************************
 orxFrame_Exit
 Exits from frame system.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFrame_Exit()
{
  /* Not initialized? */
  if((sstFrame.u32Flags & orxFRAME_KU32_FLAG_READY) == orxFRAME_KU32_FLAG_NONE)
  {
    /* !!! MSG !!! */

    return;
  }

  /* Deletes frame tree */
  orxFrame_DeleteAll();

  /* Updates flags */
  sstFrame.u32Flags &= ~orxFRAME_KU32_FLAG_READY;

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
  pstFrame = (orxFRAME *) orxMemory_Allocate(sizeof(orxFRAME), orxMEMORY_TYPE_MAIN);

  /* Non null? */
  if(pstFrame != orxNULL)
  {
    /* Cleans it */
    orxMemory_Set(pstFrame, 0, sizeof(orxFRAME));
    
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

        /* Inits structure */
        if(orxStructure_Setup((orxSTRUCTURE *)pstFrame, orxSTRUCTURE_ID_FRAME) == orxSTATUS_SUCCESS)
        {
          /* Inits values */
          pstData->fGlobalScale = 1.0f;
          pstData->fLocalScale = 1.0f;

          /* Links data to frame */
          pstFrame->pstData = pstData;
        }
        else
        {
          /* !!! MSG !!! */
    
          /* Fress partially allocated texture */
          orxMemory_Free(pstFrame);
    
          /* Not created */
          pstFrame = orxNULL;
        }
      }
      else
      {
        /* Deletes partially created frame */
        orxMemory_Free(pstFrame);

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

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFrame_Delete(orxFRAME *_pstFrame)
{
  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_FLAG_READY);
  orxASSERT(_pstFrame != orxNULL);

  /* Cleans data */
  if(_pstFrame->u32IDFlags & orxFRAME_KU32_ID_FLAG_DATA_2D)
  {
    /* Frees frame data memory */
    orxMemory_Free(_pstFrame->pstData);
  }

  /* Cleans structure */
  orxStructure_Clean((orxSTRUCTURE *)_pstFrame);

  /* Frees frame memory */
  orxMemory_Free(_pstFrame);

  return;
}

/***************************************************************************
 orxFrame_IsRenderStatusClean
 Test frame render status (TRUE : clean / orxFALSE : dirty)

 returns: orxTRUE (clean) / orxFALSE (dirty)
 ***************************************************************************/
inline orxBOOL orxFrame_IsRenderStatusClean(orxFRAME *_pstFrame)
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
inline orxBOOL orxFrame_HasDifferentialScrolling(orxFRAME *_pstFrame)
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
inline orxVOID orxFrame_GetDifferentialScrolling(orxFRAME * _pstFrame, orxVEC *_pvScroll)
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
      _pvScroll->fX = 0.0f;
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
      _pvScroll->fY = 0.0f;
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
inline orxVOID orxFrame_SetDifferentialScrolling(orxFRAME * _pstFrame, orxVEC *_pvScroll)
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
    if(_pvScroll->fX != 0.0f)
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
    coord_copy(&(((orxFRAME_DATA_2D *)(_pstFrame->pstData))->vScroll), _pvScroll);

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
inline orxVOID orxFrame_SetParent(orxFRAME *_pstFrame, orxFRAME *_pstParent)
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
inline orxVOID orxFrame_SetPosition(orxFRAME *_pstFrame, orxVEC *_pvPos)
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
inline orxVOID orxFrame_SetRotation(orxFRAME *_pstFrame, orxFLOAT _fAngle)
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
inline orxVOID orxFrame_SetScale(orxFRAME *_pstFrame, orxFLOAT _fScale)
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
inline orxVOID orxFrame_GetPosition(orxFRAME *_pstFrame, orxVEC *_pvPos, orxBOOL _bLocal)
{
  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_FLAG_READY);
  orxASSERT(_pstFrame != orxNULL);
  orxASSERT(_pvPos != orxNULL);

  /* Is Frame 2D? */
  if(_pstFrame->u32IDFlags & orxFRAME_KU32_ID_FLAG_DATA_2D)
  {
    orxCONST orxVEC *pvIntern = orxNULL;

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
    coord_copy(_pvPos, (orxVEC *)pvIntern);
  }
  else
  {
    /* Resets coord structure */
    coord_reset(_pvPos);
  }

  return;
}

/***************************************************************************
 orxFrame_GetRotation
 Gets a 2D frame local/global rotation

 returns: Requested rotation value
 ***************************************************************************/
inline orxFLOAT orxFrame_GetRotation(orxFRAME *_pstFrame, orxBOOL _bLocal)
{
  orxFLOAT fAngle = 0.0f;

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
inline orxFLOAT orxFrame_GetScale(orxFRAME *_pstFrame, orxBOOL _bLocal)
{
  orxFLOAT fScale = 1.0f;

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
inline orxVOID orxFrame_ComputeGlobalData(orxFRAME *_pstSrcFrame, orxFRAME *_pstParentFrame, orxFRAME *_pstDstFrame)
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
