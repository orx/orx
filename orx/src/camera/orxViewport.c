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

/* ID flags */
#define orxVIEWPORT_KU32_ID_FLAG_NONE             0x00000000
#define orxVIEWPORT_KU32_ID_FLAG_ACTIVE           0x00000100
#define orxVIEWPORT_KU32_ID_FLAG_VIRTUAL          0x00001000
#define orxVIEWPORT_KU32_ID_FLAG_CAMERA           0x00010000
#define orxVIEWPORT_KU32_ID_FLAG_CAMERA           0x00010000
#define orxVIEWPORT_KU32_ID_FLAG_SURFACE          0x00100000

#define orxVIEWPORT_KU32_ID_MASK_ALIGN            0xF0000000

#define orxVIEWPORT_KU32_ID_MASK_NUMBER           0x0000000F

#define orxVIEWPORT_KU32_VIEWPORT_NUMBER          16


/*
 * Viewport structure
 */
struct __orxVIEWPORT_t
{
  /* Public structure, first structure member : 16 */
  orxSTRUCTURE stStructure;

  /* Position coord : 32 */
  orxVEC vPosition;

  /* Size coord : 48 */
  orxVEC vSize;

  /* Clip coords : 80 */
  orxVEC vClipPosition, vClipSize;

  /* Internal id flags : 84 */
  orxU32 u32IDFlags;

  /* Associated camera : 88 */
  orxCAMERA *pstCamera;

  /* Associated surface : 92 */
  orxTEXTURE *pstSurface;

  /* 4 extra bytes of padding : 96 */
  orxU8 au8Unused[4];
};


/*
 * Static structure
 */
typedef struct __orxVIEWPORT_STATIC_t
{

  /* Control flags */
  orxU32 u32Flags;

  /* Used viewports */
  orxBOOL abViewportUsed[orxVIEWPORT_KU32_VIEWPORT_NUMBER];

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
orxSTATIC orxVOID orxViewport_ComputeClipCorners(orxVIEWPORT *_pstViewport)
{
  orxVEC *pvPos, *pvSize, *pvClipPosition, *pvClipSize;
  orxVEC vCamPos, vCamSize;

  /* Checks */
  orxASSERT(_pstViewport != orxNULL);

  /* Gets internal pointer */
  pvPos           = &(_pstViewport->vPosition);
  pvSize          = &(_pstViewport->vSize);
  pvClipPosition  = &(_pstViewport->vClipPosition);
  pvClipSize      = &(_pstViewport->vClipSize);

  /* 2D? */
  if(orxCamera_TestFlag(_pstViewport->pstCamera, orxCAMERA_KU32_ID_FLAG_2D) != orxFALSE)
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
orxSTATIC orxVOID orxViewport_UpdateCameraOnScreenPosition(orxVIEWPORT *_pstViewport)
{
  orxVEC *pvPos, *pvSize;
  orxVEC vResult;
  orxU32 u32Flags;
  orxFLOAT fX = 0.0f, fY = 0.0f;

  /* Checks */
  orxASSERT(_pstViewport != orxNULL);

  /* Gets viewport flags */
  u32Flags = _pstViewport->u32IDFlags;

  /* Is virtual & has camera? */
  if((u32Flags & orxVIEWPORT_KU32_ID_FLAG_VIRTUAL) && (u32Flags & orxVIEWPORT_KU32_ID_FLAG_CAMERA))
  {
    /* 2D? */
    if(orxCamera_TestFlag(_pstViewport->pstCamera, orxCAMERA_KU32_ID_FLAG_2D) != orxFALSE)
    {
      orxVEC vCamSize;

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
        fX = pvPos->fX + rintf(0.5 * (pvSize->fX - vCamSize.fX));
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
        fY = pvPos->fY + rintf(0.5 * (orxFLOAT)(pvSize->fY - vCamSize.fY));
      }

      /* Stores it in a coord structure */
      coord_set(&vResult, fX, fY, 0.0f);

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
orxVOID orxViewport_DeleteAll()
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
 orxViewport_Init
 Inits viewport system.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxViewport_Init()
{
  /* Already Initialized? */
  if(sstViewport.u32Flags & orxVIEWPORT_KU32_FLAG_READY)
  {
    /* !!! MSG !!! */

    return orxSTATUS_FAILED;
  }

  /* Cleans static controller */
  orxMemory_Set(&sstViewport, 0, sizeof(orxVIEWPORT_STATIC));

  /* Inits Flags */
  sstViewport.u32Flags = orxVIEWPORT_KU32_FLAG_READY;

  return orxSTATUS_SUCCESS;
}

/***************************************************************************
 orxViewport_Exit
 Exits from the viewport system.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxViewport_Exit()
{
  /* Not initialized? */
  if((sstViewport.u32Flags & orxVIEWPORT_KU32_FLAG_READY) == orxVIEWPORT_KU32_FLAG_NONE)
  {
    /* !!! MSG !!! */
    
    return;
  }

  /* Deletes viewport list */
  orxViewport_DeleteAll();

  /* Updates flags */
  sstViewport.u32Flags &= ~orxVIEWPORT_KU32_FLAG_READY;

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
  orxU32 u32Viewport = orxU32_Undefined, i;

  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_FLAG_READY);

  /* Gets free viewport slot */
  for(i = 0; i < orxVIEWPORT_KU32_VIEWPORT_NUMBER; i++)
  {
    /* Veiwport slot free? */
    if(sstViewport.abViewportUsed[i] == orxFALSE)
    {
      u32Viewport = i;
      break;
    }
  }

  /* Free slot found? */
  if(u32Viewport != orxU32_Undefined)
  {
    /* Creates viewport */
    pstViewport = (orxVIEWPORT *) orxMemory_Allocate(sizeof(orxVIEWPORT), orxMEMORY_TYPE_MAIN);
  
    /* Created? */
    if(pstViewport != orxNULL)
    {
      /* Cleans it */
      orxMemory_Set(pstViewport, 0, sizeof(orxVIEWPORT));

      /* Inits structure */
      if(orxStructure_Setup((orxSTRUCTURE *)pstViewport, orxSTRUCTURE_ID_VIEWPORT) == orxSTATUS_SUCCESS)
      {
        /* Inits viewport flags */
        pstViewport->u32IDFlags = orxVIEWPORT_KU32_ID_FLAG_VIRTUAL   |
                                  orxVIEWPORT_KU32_ID_FLAG_ACTIVE    |
                                  orxVIEWPORT_KU32_FLAG_ALIGN_CENTER |
                                  orxVIEWPORT_KU32_FLAG_ALIGN_CENTER |
                                  (orxU32)u32Viewport;
    
        /* Updates viewport slot */
        sstViewport.abViewportUsed[u32Viewport] = orxTRUE;
      }
      else
      {
        /* !!! MSG !!! */

        /* Fress partially allocated viewport */
        orxMemory_Free(pstViewport);

        /* Not created */
        pstViewport = orxNULL;
      }
    }
    else
    {
      /* !!! MSG !!! */
  
      /* Not created */
      pstViewport = orxNULL;
    }
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

 returns: orxVOID
 ***************************************************************************/
orxVOID orxViewport_Delete(orxVIEWPORT *_pstViewport)
{
  orxU32 u32Viewport;

  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_FLAG_READY);
  orxASSERT(_pstViewport != orxNULL);

  /* Non null? */
  if(_pstViewport != orxNULL)
  {
    /* Gets viewport id number */
    u32Viewport = _pstViewport->u32IDFlags & orxVIEWPORT_KU32_ID_MASK_NUMBER;

    /* Frees viewport slot */
    sstViewport.abViewportUsed[u32Viewport] = orxFALSE;

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

    /* Cleans structure */
    orxStructure_Clean((orxSTRUCTURE *)_pstViewport);

    /* Frees viewport memory */
    orxMemory_Free(_pstViewport);
  }

  return;
}


/* *** Structure accessors *** */


/***************************************************************************
 orxViewport_SetCamera
 Viewport camera set accessor.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxViewport_SetCamera(orxVIEWPORT *_pstViewport, orxCAMERA *_pstCamera)
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
orxVOID orxViewport_SetPosition(orxVIEWPORT *_pstViewport, orxVEC *_pvPosition)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_FLAG_READY);
  orxASSERT(_pstViewport != orxNULL);
  orxASSERT(_pvPosition != orxNULL);

  /* Updates position */
  coord_copy(&(_pstViewport->vPosition), _pvPosition);

  /* Updates camera on screen position */
  orxViewport_UpdateCameraOnScreenPosition(_pstViewport);

  return;
}

/***************************************************************************
 orxViewport_SetSize
 Viewport size set accessor.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxViewport_SetSize(orxVIEWPORT *_pstViewport, orxVEC *_pvSize)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_FLAG_READY);
  orxASSERT(_pstViewport != orxNULL);
  orxASSERT(_pvSize != orxNULL);

  /* Updates position */
  coord_copy(&(_pstViewport->vSize), _pvSize);

  /* Updates camera on screen position */
  orxViewport_UpdateCameraOnScreenPosition(_pstViewport);

  return;
}

/***************************************************************************
 orxViewport_GetCamera
 Viewport camera get accessor.

 returns: orxVOID
 ***************************************************************************/
orxCAMERA *orxViewport_GetCamera(orxVIEWPORT *_pstViewport)
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
orxVOID orxViewport_GetPosition(orxVIEWPORT *_pstViewport, orxVEC *_pvPosition)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_FLAG_READY);
  orxASSERT(_pstViewport != orxNULL);
  orxASSERT(_pvPosition != orxNULL);

  /* Gets position */
  coord_copy(_pvPosition, &(_pstViewport->vPosition));

  return;
}

/***************************************************************************
 orxViewport_GetSize
 Viewport size get accessor.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxViewport_GetSize(orxVIEWPORT *_pstViewport, orxVEC *_pvSize)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_FLAG_READY);
  orxASSERT(_pstViewport != orxNULL);
  orxASSERT(_pvSize != orxNULL);

  /* Gets size */
  coord_copy(_pvSize, &(_pstViewport->vSize));

  return;
}


/***************************************************************************
 orxViewport_SetAlignment
 Viewport alignment set accessor (flags must be OR'ed).

 returns: orxVOID
 ***************************************************************************/
orxVOID orxViewport_SetAlignment(orxVIEWPORT *_pstViewport, orxU32 _u32AlignFlags)
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
orxVOID orxViewport_SetSurface(orxVIEWPORT *_pstViewport, orxTEXTURE *_pstSurface)
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
orxTEXTURE *orxViewport_GetSurface(orxVIEWPORT *_pstViewport)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_FLAG_READY);
  orxASSERT(_pstViewport != orxNULL);

  /* Has surface? */
  if(_pstViewport->u32IDFlags & orxVIEWPORT_KU32_ID_FLAG_SURFACE)
  {
    return _pstViewport->pstSurface;
  }
  else
  {
    return orxNULL;
  }
}

/***************************************************************************
 orxViewport_Enable
 Viewport activate accessor.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxViewport_Enable(orxVIEWPORT *_pstViewport, orxBOOL _bEnable)
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
orxBOOL orxViewport_IsEnabled(orxVIEWPORT *_pstViewport)
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
orxVOID orxViewport_GetClip(orxVIEWPORT * _pstViewport, orxVEC *_pvPosition, orxVEC *_pvSize)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_FLAG_READY);
  orxASSERT(_pstViewport != orxNULL);
  orxASSERT(_pvPosition != orxNULL);
  orxASSERT(_pvSize != orxNULL);

  /* Gets clip infos */
  coord_copy(_pvPosition, &(_pstViewport->vClipPosition));
  coord_copy(_pvSize, &(_pstViewport->vClipSize));

  return;
}
