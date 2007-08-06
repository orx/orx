
/***************************************************************************
 orxRender.c
 Render module
 
 begin                : 15/12/2003
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


#include "camera/orxRender.h"

#include "camera/orxCamera.h"
#include "debug/orxDebug.h"
#include "display/orxDisplay.h"
#include "display/orxGraphic.h"
#include "memory/orxMemory.h"



/*
 * Platform independant defines
 */

#define orxRENDER_KU32_FLAG_NONE                0x00000000

#define orxRENDER_KU32_FLAG_READY               0x00000001
#define orxRENDER_KU32_FLAG_DATA_2D             0x00000010


/*
 * Internal render viewport structure
 */
typedef struct __orxRENDER_VIEWPORT_LIST_t
{
  /* Corresponding viewport : 4 */
  orxVIEWPORT *pstViewport;

  /* Z sort value : 8 */
  orxFLOAT fZSort;

  /* Padding */
  orxPAD(8)

} orxRENDER_VIEWPORT_LIST;


/*
 * Static structure
 */
typedef struct __orxRENDER_STATIC_t
{
  /* Control flags */
  orxU32 u32Flags;

} orxRENDER_STATIC;


/*
 * Static data
 */
orxSTATIC orxRENDER_STATIC sstRender;


/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 orxRender_RenderObject
 Renders given object

 returns: orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 ***************************************************************************/
orxSTATIC orxVOID orxFASTCALL orxRender_RenderObject(orxBITMAP *_pstSurface, orxOBJECT *_pstObject, orxFRAME *_pstFrame)
{
  orxGRAPHIC *pstGraphic;
  orxTEXTURE *pstTexture;
  orxBITMAP *pstBitmap;
  orxVECTOR vPos;
  orxFLOAT fRotation, fScale;

  /* Checks */
  orxASSERT(_pstSurface != orxNULL);
  orxASSERT(_pstObject != orxNULL);
  orxASSERT(_pstFrame != orxNULL);

  /* Gets object's graphic */
  pstGraphic = (orxGRAPHIC *)orxObject_GetStructure(_pstObject, orxSTRUCTURE_ID_GRAPHIC);

  /* 2D? */
  if(orxGraphic_TestFlags(pstGraphic, orxGRAPHIC_KU32_ID_FLAG_2D) != orxFALSE)
  {
    /* Gets graphic's texture */
    pstTexture  = (orxTEXTURE *)orxGraphic_GetData(pstGraphic);

    /* Gets texture's bitmap */
    pstBitmap   = (orxBITMAP *)orxTexture_GetBitmap(pstTexture);

    /* Gets frame's position, rotation & zoom */
    fRotation   = orxFrame_GetRotation(_pstFrame, orxFALSE);
    fScale      = orxFrame_GetScale(_pstFrame, orxFALSE);
    orxFrame_GetPosition(_pstFrame, &vPos, orxFALSE);

    /* Blit bitmap onto surface */
    if((fRotation == orxFLOAT_0) && (fScale == orxFLOAT_1))
    {
      orxVECTOR vSize;

      /* Gets bitmap's size */
      orxDisplay_GetBitmapSize(pstBitmap, &vSize);

      /* Blit it */
      orxDisplay_BlitBitmap(_pstSurface, pstBitmap, &vPos, &orxVector_0, &vSize);
    }
    /* Blit transformed bitmap onto surface */
    else
    {
      orxBITMAP_TRANSFORM stTransform;

      /* Sets transformation values */
      stTransform.vDstCoord = vPos;
      stTransform.vScale.fX = fScale;
      stTransform.vScale.fY = fScale;
      stTransform.fRotation = fRotation;

      /* Blit it */
      orxDisplay_TransformBitmap(_pstSurface, pstBitmap, &stTransform, 0);
    }
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}

/***************************************************************************
 orxRender_SortViewportList
 Sorts a viewport list using their Z value.
 Uses a "shaker sort".

 returns: orxVOID
 ***************************************************************************/
orxSTATIC orxVOID orxFASTCALL orxRender_SortViewportList(orxRENDER_VIEWPORT_LIST *_pstViewportList, orxU32 _u32Number)
{
  orxU32 u32Low, u32High, u32Min, u32Max;

  /* Checks */
  orxASSERT(_pstViewportList != orxNULL);
  orxASSERT(_u32Number > 0);

  /* Untill all viewports are sorted */
  for(u32Low = 0, u32High = _u32Number - 1; u32Low < u32High; u32Low++, u32High --)
  {
    orxFLOAT fMin, fMax, fTest;
    orxU32 i;
    
    /* Inits values */
    u32Min  = u32Low;
    u32Max  = u32Low;
    fMin    = _pstViewportList[u32Min].fZSort;
    fMax    = _pstViewportList[u32Max].fZSort;

    /* Finds min & max */
    for(i = u32High; i > u32Low; i--)
    {
      /* Gets value to test */
      fTest = _pstViewportList[i].fZSort;

      /* Inferior */
      if(fTest < fMin)
      {
        /* Updates values */
        u32Min  = i;
        fMin    = fTest;
      }
      /* Superior */
      else if(fTest > fMax)
      {
        /* Updates values */
        u32Max  = i;
        fMax    = fTest;
      }
    }

    /* Min swap? */
    if(u32Min != u32Low)
    {
      /* Swap Z sort values */
      orxSWAP32(_pstViewportList[u32Low].fZSort, _pstViewportList[u32Min].fZSort);

      /* Swap viewport pointers */
      orxSWAP32(_pstViewportList[u32Low].pstViewport, _pstViewportList[u32Min].pstViewport);
    }

    /* Max swap? */
    if(u32Max != u32High)
    {
      /* Swap Z sort values */
      orxSWAP32(_pstViewportList[u32High].fZSort, _pstViewportList[u32Max].fZSort);

      /* Swap viewport pointers */
      orxSWAP32(_pstViewportList[u32High].pstViewport, _pstViewportList[u32Max].pstViewport);
    }
  }

  /* Done */
  return;
}


/***************************************************************************
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 orxRender_Setup
 Render system setup.

 returns: nothing
 ***************************************************************************/
orxVOID orxRender_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_TEXTURE);
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_DISPLAY);
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_VIEWPORT);
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_CAMERA);

  return;
}

/***************************************************************************
 orxRender_Init
 Inits render system.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 ***************************************************************************/
orxSTATUS orxRender_Init()
{
	orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Already Initialized? */
  if(!(sstRender.u32Flags & orxRENDER_KU32_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Set(&sstRender, 0, sizeof(orxRENDER_STATIC));

    /* Inits Flags */
    sstRender.u32Flags = orxRENDER_KU32_FLAG_READY | orxRENDER_KU32_FLAG_DATA_2D;

    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* !!! MSG !!! */

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }
  
  /* Done */
  return eResult;
}

/***************************************************************************
 orxRender_Exit
 Exits from the render system.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxRender_Exit()
{
  /* Initialized? */
  if((sstRender.u32Flags & orxRENDER_KU32_FLAG_READY) == orxRENDER_KU32_FLAG_NONE)
  {
    /* Updates flags */
    sstRender.u32Flags &= ~orxRENDER_KU32_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}

/***************************************************************************
 orxRender_RenderViewport
 Renders given viewport.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFASTCALL orxRender_RenderViewport(orxCONST orxVIEWPORT *_pstViewport)
{
  /* Checks */
  orxASSERT(sstRender.u32Flags & orxRENDER_KU32_FLAG_READY);
  orxASSERT(_pstViewport != orxNULL);

  /* 2D rendering? */
  if(sstRender.u32Flags & orxRENDER_KU32_FLAG_DATA_2D)
  {
    /* Is viewport active? */
    if(orxViewport_IsEnabled(_pstViewport) != orxFALSE)
    {
      orxCAMERA  *pstCamera;
      orxTEXTURE *pstSurface;
      orxBITMAP  *pstSurfaceBitmap;
      orxVECTOR   vTL, vBR;

      /* Gets viewport surface */
      pstSurface = orxViewport_GetSurface(_pstViewport);

      /* Has surface? */
      if(pstSurface != orxNULL)
      {
        pstSurfaceBitmap = (orxBITMAP *)orxTexture_GetBitmap(pstSurface);
      }
      /* Gets screen surface */
      else
      {
        pstSurfaceBitmap = orxDisplay_GetScreenBitmap();
      }

      /* Gets viewport info */
      orxViewport_GetClip(_pstViewport, &vTL, &vBR);

      /* Sets surface clipping */
      orxDisplay_SetBitmapClipping(pstSurfaceBitmap, &vTL, &vBR);

      /* Clears surface bitmap */
      orxDisplay_ClearBitmap(pstSurfaceBitmap, orx2ARGB(0, 0, 0, 0));

      /* Gets camera */
      pstCamera = orxViewport_GetCamera(_pstViewport);

      /* Non null? */
      if(pstCamera != orxNULL)
      {
        orxCAMERA_VIEW_LIST *pstViewCell;

        /* Updates camera view list */
        orxCamera_UpdateViewList(pstCamera);

        /* Gets first view list element */
        pstViewCell = orxCamera_GetViewListFirstCell(pstCamera);

        /* Untill all objects have been processed */
        while(pstViewCell != orxNULL)
        {
          orxFRAME *pstScreenFrame;
          orxOBJECT *pstObject;

          /* Gets view list element's screen frame */
          pstScreenFrame = orxCamera_GetViewListFrame(pstViewCell);

          /* Gets view list element's object */
          pstObject = orxCamera_GetViewListObject(pstViewCell);

          /* Renders object */
          orxRender_RenderObject(pstSurfaceBitmap, pstObject, pstScreenFrame);

          /* Gets next view list element */
          pstViewCell = orxCamera_GetViewListNextCell(pstCamera);
        }
      }
      else
      {
        /* !!! MSG !!! */
      }
    }
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}

/***************************************************************************
 orxRender_RenderAllViewports
 Renders all viewports.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFASTCALL orxRender_RenderAllViewports(orxCONST orxCLOCK_INFO *_pstClockInfo, orxVOID *_pstContext)
{
  orxRENDER_VIEWPORT_LIST *pstViewportList;
  orxVIEWPORT *pstViewport;
  orxVECTOR vPosition;
  orxU32 u32ViewportNumber;

  /* Checks */
  orxASSERT(sstRender.u32Flags & orxRENDER_KU32_FLAG_READY);

  /* Gets viewports number */
  u32ViewportNumber = orxStructure_GetNumber(orxSTRUCTURE_ID_VIEWPORT);

  /* Creates local viewport sorted list */
  pstViewportList = (orxRENDER_VIEWPORT_LIST *)orxMemory_Allocate(u32ViewportNumber * sizeof(orxRENDER_VIEWPORT_LIST), orxMEMORY_TYPE_MAIN);

  /* Created? */
  if(pstViewportList != orxNULL)
  {
    orxU32 i;

    /* Cleans it */
    orxMemory_Set(pstViewportList, 0, u32ViewportNumber * sizeof(orxRENDER_VIEWPORT_LIST));
  
    /* Gets first viewport */
    pstViewport = (orxVIEWPORT *)orxStructure_GetFirst(orxSTRUCTURE_ID_VIEWPORT);

    /* Non null? */
    for(i = 0; i < u32ViewportNumber; i++)
    {
      /* Gets viewport position */
      orxViewport_GetPosition(pstViewport, &vPosition);
  
      /* Adds it to list */
      pstViewportList[i].pstViewport = pstViewport;
      pstViewportList[i].fZSort = vPosition.fZ;
  
      /* Gets next viewport */
      pstViewport = (orxVIEWPORT *)orxStructure_GetNext((orxSTRUCTURE *)pstViewport);
    }

    /* Sorts viewport list */
    orxRender_SortViewportList(pstViewportList, u32ViewportNumber);
  
    /* Renders all viewports */
    for(i = 0; i < u32ViewportNumber; i++)
    {
      orxRender_RenderViewport(pstViewportList[i].pstViewport);
    }

    /* Swap buffers */
    orxDisplay_Swap();

    /* Deletes local viewport list */
    orxMemory_Free(pstViewportList);
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}
