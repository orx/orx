/***************************************************************************
 orxViewport.c
 Viewport module
 
 begin                : 14/12/2003
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


#include "camera/orxViewport.h"

#include "debug/orxDebug.h"
#include "math/orxMath.h"
#include "memory/orxMemory.h"
#include "object/orxStructure.h"


/*
 * Platform independant defines
 */

/* Control flags */
#define orxVIEWPORT_KU32_FLAG_NONE                0x00000000

#define orxVIEWPORT_KU32_FLAG_READY               0x00000001

#define orxVIEWPORT_KU32_MASK_ALL                 0xFFFFFFFF

/* ID flags */
#define orxVIEWPORT_KU32_ID_FLAG_NONE             0x00000000
#define orxVIEWPORT_KU32_ID_FLAG_ACTIVE           0x00000100
#define orxVIEWPORT_KU32_ID_FLAG_VIRTUAL          0x00001000
#define orxVIEWPORT_KU32_ID_FLAG_CAMERA           0x00010000
#define orxVIEWPORT_KU32_ID_FLAG_SURFACE          0x00100000

#define orxVIEWPORT_KU32_ID_MASK_ALIGN            0xF0000000
#define orxVIEWPORT_KU32_ID_MASK_ALL              0xFFFFFFFF


/*
 * Viewport structure
 */
struct __orxVIEWPORT_t
{
  /* Public structure, first structure member : 16 */
  orxSTRUCTURE stStructure;

  /* Position coord : 32 */
  orxVECTOR vPosition;

  /* Size coord : 48 */
  orxVECTOR vSize;

  /* Clip coords : 80 */
  orxVECTOR vClipPosition, vClipSize;

  /* Internal id flags : 84 */
  orxU32 u32IDFlags;

  /* Associated camera : 88 */
  orxCAMERA *pstCamera;

  /* Associated surface : 92 */
  orxTEXTURE *pstSurface;

  /* Padding */
  orxPAD(92)
};


/*
 * Static structure
 */
typedef struct __orxVIEWPORT_STATIC_t
{

  /* Control flags */
  orxU32 u32Flags;

} orxVIEWPORT_STATIC;


/*
 * Static data
 */
orxSTATIC orxVIEWPORT_STATIC sstViewport;


/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 orxViewport_ComputeClipCorners
 Updates viewport clip corners.

 returns: orxVOID
 ***************************************************************************/
orxSTATIC orxVOID orxFASTCALL orxViewport_ComputeClipCorners(orxVIEWPORT *_pstViewport)
{
  orxVECTOR *pvPos, *pvSize, *pvClipPosition, *pvClipSize;
  orxVECTOR vCamPos, vCamSize;

  /* Checks */
  orxASSERT(_pstViewport != orxNULL);

  /* Gets internal pointer */
  pvPos           = &(_pstViewport->vPosition);
  pvSize          = &(_pstViewport->vSize);
  pvClipPosition  = &(_pstViewport->vClipPosition);
  pvClipSize      = &(_pstViewport->vClipSize);

  /* 2D? */
  if(orxCamera_TestFlags(_pstViewport->pstCamera, orxCAMERA_KU32_ID_FLAG_2D) != orxFALSE)
  {
    /* Gets camera infos */
    orxCamera_GetOnScreenPosition(_pstViewport->pstCamera, &vCamPos);
    orxCamera_GetSize(_pstViewport->pstCamera, &vCamSize);

    /* Clips position */
    pvClipPosition->fX  = orxMAX(vCamPos.fX, pvPos->fX);
    pvClipPosition->fY  = orxMAX(vCamPos.fY, pvPos->fY);

    /* Clips size */
    pvClipSize->fX      = orxMIN(vCamSize.fX, pvSize->fX);
    pvClipSize->fY      = orxMIN(vCamSize.fY, pvSize->fY);
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}

/***************************************************************************
 orxViewport_UpdateCameraOnScreenPosition
 Updates camera on screen position.

 returns: orxVOID
 ***************************************************************************/
orxSTATIC orxVOID orxFASTCALL orxViewport_UpdateCameraOnScreenPosition(orxVIEWPORT *_pstViewport)
{
  orxVECTOR *pvPos, *pvSize;
  orxVECTOR vResult;
  orxU32 u32Flags;
  orxFLOAT fX = orxFLOAT_0, fY = orxFLOAT_0;

  /* Checks */
  orxASSERT(_pstViewport != orxNULL);

  /* Gets viewport flags */
  u32Flags = _pstViewport->u32IDFlags;

  /* Is virtual & has camera? */
  if((u32Flags & orxVIEWPORT_KU32_ID_FLAG_VIRTUAL) && (u32Flags & orxVIEWPORT_KU32_ID_FLAG_CAMERA))
  {
    /* 2D? */
    if(orxCamera_TestFlags(_pstViewport->pstCamera, orxCAMERA_KU32_ID_FLAG_2D) != orxFALSE)
    {
      orxVECTOR vCamSize;

      /* Gets internal pointer */
      pvPos = &(_pstViewport->vPosition);
      pvSize = &(_pstViewport->vSize);

      /* Gets camera size */
      orxCamera_GetSize(_pstViewport->pstCamera, &vCamSize);

      /* X alignment */
      if(u32Flags & orxVIEWPORT_KU32_FLAG_ALIGN_LEFT)
      {
        /* Left aligned */
        fX = pvPos->fX;
      }
      else if(u32Flags & orxVIEWPORT_KU32_FLAG_ALIGN_RIGHT)
      {
        /* Right aligned */
        fX = pvPos->fX + pvSize->fX - vCamSize.fX;
      }
      else
      {
        /* Center aligned */
        fX = pvPos->fX + (orxFLOAT)floor(0.5 * (pvSize->fX - vCamSize.fX)); /* floorf() or rintf() is not recognized by MSVC :/ */
      }
      /* Y alignment */
      if(u32Flags & orxVIEWPORT_KU32_FLAG_ALIGN_TOP)
      {
        /* Left aligned */
        fY = pvPos->fY;
      }
      else if(u32Flags & orxVIEWPORT_KU32_FLAG_ALIGN_BOTTOM)
      {
        /* Right aligned */
        fY = pvPos->fY + pvSize->fY - vCamSize.fY;
      }
      else
      {
        /* Center aligned */
        fY = pvPos->fY + (orxFLOAT)floor(0.5f * (pvSize->fY - vCamSize.fY)); /* floorf() or rintf() is not recognized by MSVC :/ */
      }

      /* Stores it in a coord structure */
      orxVector_Set3(&vResult, fX, fY, orxFLOAT_0);

      /* Updates camera screen position */
      orxCamera_SetOnScreenPosition(_pstViewport->pstCamera, &vResult);

      /* Computes clip corners */
      orxViewport_ComputeClipCorners(_pstViewport);
    }
    else
    {
      /* !!! MSG !!! */
    }
  }

  return;
}

/***************************************************************************
 orxViewport_DeleteAll
 Deletes all viewports.

 returns: orxVOID
 ***************************************************************************/
orxSTATIC orxINLINE orxVOID orxViewport_DeleteAll()
{
  orxVIEWPORT *pstViewport;

  /* Gets first viewport */
  pstViewport = (orxVIEWPORT *)orxStructure_GetFirst(orxSTRUCTURE_ID_VIEWPORT);

  /* Non empty? */
  while(pstViewport != orxNULL)
  {
    /* Deletes viewport */
    orxViewport_Delete(pstViewport);

    /* Gets first remaining viewport */
    pstViewport = (orxVIEWPORT *)orxStructure_GetFirst(orxSTRUCTURE_ID_VIEWPORT);
  }

  return;
}


/***************************************************************************
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 orxViewport_Setup
 Viewport system setup.

 returns: nothing
 ***************************************************************************/
orxVOID orxViewport_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_VIEWPORT, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_VIEWPORT, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_VIEWPORT, orxMODULE_ID_TEXTURE);
  orxModule_AddDependency(orxMODULE_ID_VIEWPORT, orxMODULE_ID_CAMERA);

  return;
}

/***************************************************************************
 orxViewport_Init
 Inits viewport system.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 ***************************************************************************/
orxSTATUS orxViewport_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstViewport.u32Flags & orxVIEWPORT_KU32_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Set(&sstViewport, 0, sizeof(orxVIEWPORT_STATIC));

    /* Registers structure type */
    eResult = orxSTRUCTURE_REGISTER(VIEWPORT, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxNULL);

    /* Initialized? */
    if(eResult == orxSTATUS_SUCCESS)
    {
      /* Inits Flags */
      sstViewport.u32Flags = orxVIEWPORT_KU32_FLAG_READY;
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
  
  /* Done! */
  return eResult;
}

/***************************************************************************
 orxViewport_Exit
 Exits from the viewport system.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxViewport_Exit()
{
  /* Initialized? */
  if(sstViewport.u32Flags & orxVIEWPORT_KU32_FLAG_READY)
  {
    /* Deletes viewport list */
    orxViewport_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_VIEWPORT);

    /* Updates flags */
    sstViewport.u32Flags &= ~orxVIEWPORT_KU32_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}

/***************************************************************************
 orxViewport_Create
 Creates a new empty viewport.

 returns: Created viewport.
 ***************************************************************************/
orxVIEWPORT *orxViewport_Create()
{
  orxVIEWPORT *pstViewport = orxNULL;

  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_FLAG_READY);

  /* Creates viewport */
  pstViewport = (orxVIEWPORT *)orxStructure_Create(orxSTRUCTURE_ID_VIEWPORT);

  /* Created? */
  if(pstViewport != orxNULL)
  {
    /* Inits viewport flags */
    pstViewport->u32IDFlags = orxVIEWPORT_KU32_ID_FLAG_VIRTUAL   |
                              orxVIEWPORT_KU32_ID_FLAG_ACTIVE    |
                              orxVIEWPORT_KU32_FLAG_ALIGN_CENTER |
                              orxVIEWPORT_KU32_FLAG_ALIGN_CENTER;
  }
  else
  {
    /* !!! MSG !!! */

    /* Not created */
    pstViewport = orxNULL;
  }

  /* Done! */
  return pstViewport;
}

/***************************************************************************
 orxViewport_Delete
 Deletes a viewport.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 ***************************************************************************/
orxSTATUS orxFASTCALL orxViewport_Delete(orxVIEWPORT *_pstViewport)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_FLAG_READY);
  orxASSERT(_pstViewport != orxNULL);

  /* Not referenced? */
  if(orxStructure_GetRefCounter((orxSTRUCTURE *)_pstViewport) == 0)
  {
    /* Was linked to a camera? */
    if(_pstViewport->pstCamera != orxNULL)
    {
      orxStructure_DecreaseCounter((orxSTRUCTURE *)(_pstViewport->pstCamera));
    }

    /* Was linked to a surface? */
    if(_pstViewport->pstSurface != orxNULL)
    {
      orxStructure_DecreaseCounter((orxSTRUCTURE *)(_pstViewport->pstSurface));
    }

    /* Deletes structure */
    orxStructure_Delete((orxSTRUCTURE *)_pstViewport);
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


/* *** Structure accessors *** */


/***************************************************************************
 orxViewport_SetCamera
 Viewport camera set accessor.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFASTCALL orxViewport_SetCamera(orxVIEWPORT *_pstViewport, orxCAMERA *_pstCamera)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_FLAG_READY);
  orxASSERT(_pstViewport != orxNULL);

  /* Has already a camera? */
  if(_pstViewport->u32IDFlags & orxVIEWPORT_KU32_ID_FLAG_CAMERA)
  {
    /* Updates reference counter */
    orxStructure_DecreaseCounter((orxSTRUCTURE *)(_pstViewport->pstCamera));

    /* Updates flags */
    _pstViewport->u32IDFlags &= ~orxVIEWPORT_KU32_ID_FLAG_CAMERA;
  }

  /* Updates pointer */
  _pstViewport->pstCamera = _pstCamera;

  /* Has a new camera? */
  if(_pstCamera != orxNULL)
  {
    /* Updates reference counter */
    orxStructure_IncreaseCounter((orxSTRUCTURE *)_pstCamera);

    /* Updates flags */
    _pstViewport->u32IDFlags |= orxVIEWPORT_KU32_ID_FLAG_CAMERA;
  }

  /* Updates camera on screen position */
  orxViewport_UpdateCameraOnScreenPosition(_pstViewport);

  return;
} 

/***************************************************************************
 orxViewport_SetPosition
 Viewport position set accessor.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFASTCALL orxViewport_SetPosition(orxVIEWPORT *_pstViewport, orxCONST orxVECTOR *_pvPosition)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_FLAG_READY);
  orxASSERT(_pstViewport != orxNULL);
  orxASSERT(_pvPosition != orxNULL);

  /* Updates position */
  orxVector_Copy(&(_pstViewport->vPosition), _pvPosition);

  /* Updates camera on screen position */
  orxViewport_UpdateCameraOnScreenPosition(_pstViewport);

  return;
}

/***************************************************************************
 orxViewport_SetSize
 Viewport size set accessor.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFASTCALL orxViewport_SetSize(orxVIEWPORT *_pstViewport, orxCONST orxVECTOR *_pvSize)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_FLAG_READY);
  orxASSERT(_pstViewport != orxNULL);
  orxASSERT(_pvSize != orxNULL);

  /* Updates position */
  orxVector_Copy(&(_pstViewport->vSize), _pvSize);

  /* Updates camera on screen position */
  orxViewport_UpdateCameraOnScreenPosition(_pstViewport);

  return;
}

/***************************************************************************
 orxViewport_GetCamera
 Viewport camera get accessor.

 returns: orxVOID
 ***************************************************************************/
orxCAMERA *orxFASTCALL orxViewport_GetCamera(orxCONST orxVIEWPORT *_pstViewport)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_FLAG_READY);
  orxASSERT(_pstViewport != orxNULL);

  /* Has a camera? */
  if(_pstViewport->u32IDFlags & orxVIEWPORT_KU32_ID_FLAG_CAMERA)
  {
    return _pstViewport->pstCamera;
  }
  else
  {
    return orxNULL;
  }
}

/***************************************************************************
 orxViewport_GetPosition
 Viewport position get accessor.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFASTCALL orxViewport_GetPosition(orxCONST orxVIEWPORT *_pstViewport, orxVECTOR *_pvPosition)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_FLAG_READY);
  orxASSERT(_pstViewport != orxNULL);
  orxASSERT(_pvPosition != orxNULL);

  /* Gets position */
  orxVector_Copy(_pvPosition, &(_pstViewport->vPosition));

  return;
}

/***************************************************************************
 orxViewport_GetSize
 Viewport size get accessor.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFASTCALL orxViewport_GetSize(orxCONST orxVIEWPORT *_pstViewport, orxVECTOR *_pvSize)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_FLAG_READY);
  orxASSERT(_pstViewport != orxNULL);
  orxASSERT(_pvSize != orxNULL);

  /* Gets size */
  orxVector_Copy(_pvSize, &(_pstViewport->vSize));

  return;
}


/***************************************************************************
 orxViewport_SetAlignment
 Viewport alignment set accessor (flags must be OR'ed).

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFASTCALL orxViewport_SetAlignment(orxVIEWPORT *_pstViewport, orxU32 _u32AlignFlags)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_FLAG_READY);
  orxASSERT(_pstViewport != orxNULL);

  /* Cleans current alignment */
  _pstViewport->u32IDFlags &= ~orxVIEWPORT_KU32_ID_MASK_ALIGN;

  /* Updates alignement */
  _pstViewport->u32IDFlags |= (_u32AlignFlags & orxVIEWPORT_KU32_ID_MASK_ALIGN);

  return;
}

/***************************************************************************
 orxViewport_SetSurface
 Viewport surface set accessor.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFASTCALL orxViewport_SetSurface(orxVIEWPORT *_pstViewport, orxTEXTURE *_pstSurface)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_FLAG_READY);
  orxASSERT(_pstViewport != orxNULL);

  /* Has already a surface? */
  if(_pstViewport->u32IDFlags & orxVIEWPORT_KU32_ID_FLAG_SURFACE)
  {
    /* Updates surface reference counter */
    orxStructure_DecreaseCounter((orxSTRUCTURE *)(_pstViewport->pstSurface));

    /* Updates flags */
    _pstViewport->u32IDFlags &= ~orxVIEWPORT_KU32_ID_FLAG_SURFACE;
  }

  /* Updates surface pointer */
  _pstViewport->pstSurface = _pstSurface;
  
  /* Has a new surface? */
  if(_pstSurface != orxNULL)
  {
    /* Updates surface reference counter */
    orxStructure_IncreaseCounter((orxSTRUCTURE *)_pstSurface);

    /* Updates flags */
    _pstViewport->u32IDFlags |= orxVIEWPORT_KU32_ID_FLAG_SURFACE;
  }
  else
  {
    /* Deactivates viewport */
    _pstViewport->u32IDFlags &= ~orxVIEWPORT_KU32_ID_FLAG_ACTIVE;
  }

  return;
}

/***************************************************************************
 orxViewport_GetSurface
 Viewport surface get accessor

 returns: orxVOID
 ***************************************************************************/
orxTEXTURE *orxFASTCALL orxViewport_GetSurface(orxCONST orxVIEWPORT *_pstViewport)
{
  orxTEXTURE *pstResult = orxNULL;

  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_FLAG_READY);
  orxASSERT(_pstViewport != orxNULL);

  /* Has surface? */
  if(_pstViewport->u32IDFlags & orxVIEWPORT_KU32_ID_FLAG_SURFACE)
  {
    /* Updates result */
    pstResult = _pstViewport->pstSurface;
  }

  /* Done! */
  return pstResult;
}

/***************************************************************************
 orxViewport_Enable
 Viewport activate accessor.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFASTCALL orxViewport_Enable(orxVIEWPORT *_pstViewport, orxBOOL _bEnable)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_FLAG_READY);
  orxASSERT(_pstViewport != orxNULL);

  /* Updates activation flag */
  if(_bEnable == orxFALSE)
  {
    _pstViewport->u32IDFlags &= ~orxVIEWPORT_KU32_ID_FLAG_ACTIVE;
  }
  else
  {
    _pstViewport->u32IDFlags |= orxVIEWPORT_KU32_ID_FLAG_ACTIVE;
  }

  return;
}

/***************************************************************************
 orxViewport_IsEnabled
 Viewport is active accessor.

 returns: orxTRUE/FALSE
 ***************************************************************************/
orxBOOL orxFASTCALL orxViewport_IsEnabled(orxCONST orxVIEWPORT *_pstViewport)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_FLAG_READY);
  orxASSERT(_pstViewport != orxNULL);

  /* Tests */
  return((_pstViewport->u32IDFlags & orxVIEWPORT_KU32_ID_FLAG_ACTIVE) ? orxTRUE : orxFALSE);
}

/***************************************************************************
 orxViewport_GetClip
 Viewport clip get accessor.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFASTCALL orxViewport_GetClip(orxCONST orxVIEWPORT * _pstViewport, orxVECTOR *_pvPosition, orxVECTOR *_pvSize)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_FLAG_READY);
  orxASSERT(_pstViewport != orxNULL);
  orxASSERT(_pvPosition != orxNULL);
  orxASSERT(_pvSize != orxNULL);

  /* Gets clip infos */
  orxVector_Copy(_pvPosition, &(_pstViewport->vClipPosition));
  orxVector_Copy(_pvSize, &(_pstViewport->vClipSize));

  return;
}
