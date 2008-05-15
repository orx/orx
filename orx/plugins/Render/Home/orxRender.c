/**
 * @file orxRender.c
 *
 * Render module
 *
 */

 /***************************************************************************
 orxRender.c
 Render module

 begin                : 25/09/2007
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


#include "orxInclude.h"

#include "plugin/orxPluginUser.h"

#include "render/orxViewport.h"
#include "render/orxRender.h"
#include "core/orxConfig.h"
#include "debug/orxFPS.h"
#include "memory/orxBank.h"
#include "utils/orxLinkList.h"
#include "anim/orxAnimPointer.h"
#include "display/orxDisplay.h"
#include "display/orxGraphic.h"
#include "object/orxFrame.h"
#include "object/orxObject.h"
#include "math/orxVector.h"

/** Module flags
 */
#define orxRENDER_KU32_STATIC_FLAG_NONE       0x00000000 /**< No flags */

#define orxRENDER_KU32_STATIC_FLAG_READY      0x00000001 /**< Ready flag */

#define orxRENDER_KU32_STATIC_MASK_ALL        0xFFFFFFFF /**< All mask */


/** Defines
 */
#define orxRENDER_KF_TICK_SIZE                orx2F(1.0f / 60.0f)
#define orxRENDER_KU32_ORDER_BANK_SIZE        128
#define orxRENDER_KST_DEFAULT_COLOR           orx2RGBA(255, 0, 0, 255)
#define orxRENDER_KZ_FPS_FORMAT               "FPS: %d"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

typedef struct __orxRENDER_RENDER_NODE_t
{
  orxLINKLIST_NODE  stNode;                       /**< Linklist node : 12 */
  orxOBJECT        *pstObject;                    /**< Object pointer : 16 */
  orxVECTOR         vPosition;                    /**< Object position : 32 */

} orxRENDER_NODE;

/** Static structure
 */
typedef struct __orxRENDER_STATIC_t
{
  orxU32        u32Flags;                         /**< Control flags : 4 */
  orxCLOCK     *pstClock;                         /**< Rendering clock pointer : 8 */
  orxBANK      *pstRenderBank;                    /**< Rendering bank : 12 */
  orxLINKLIST   stRenderList;                     /**< Rendering list : 16 */

} orxRENDER_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
orxSTATIC orxRENDER_STATIC sstRender;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Renders a viewport
 * @param[in]   _pstObject        Obbject to render
 * @param[in]   _pstRenderBitmap  Bitmap surface where to render
 * @param[in]   _pstFrame         Rendering frame
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATIC orxSTATUS orxFASTCALL orxRender_RenderObject(orxCONST orxOBJECT *_pstObject, orxBITMAP *_pstRenderBitmap, orxFRAME *_pstRenderFrame)
{
  orxGRAPHIC *pstGraphic;
  orxSTATUS   eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstRender.u32Flags & orxRENDER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pstRenderBitmap != orxNULL);
  orxASSERT(_pstRenderFrame != orxNULL);

  /* Gets object's graphic */
  pstGraphic = orxOBJECT_GET_STRUCTURE(_pstObject, GRAPHIC);

  /* Valid? */
  if((pstGraphic != orxNULL)
  && (orxStructure_TestFlags(pstGraphic, orxGRAPHIC_KU32_FLAG_2D)))
  {
    orxBITMAP      *pstBitmap;
    orxANIMPOINTER *pstAnimPointer;
    orxVECTOR       vPivot, vPosition;
    orxFLOAT        fRotation, fScaleX, fScaleY;

    /* Gets animation pointer */
    pstAnimPointer = orxOBJECT_GET_STRUCTURE(_pstObject, ANIMPOINTER);

    /* Valid? */
    if(pstAnimPointer != orxNULL)
    {
      orxGRAPHIC *pstTemp;

      /* Gets current anim data */
      pstTemp = orxSTRUCTURE_GET_POINTER(orxAnimPointer_GetCurrentAnimData(pstAnimPointer), GRAPHIC);

      /* Valid? */
      if(pstTemp != orxNULL)
      {
        /* Uses it */
        pstGraphic = pstTemp;
      }
    }

    /* Gets its pivot */
    orxGraphic_GetPivot(pstGraphic, &vPivot);

    /* Gets graphic's bitmap */
    pstBitmap = orxTexture_GetBitmap((orxTEXTURE *)orxGraphic_GetData(pstGraphic));

    /* Gets rendering frame's position, rotation & scale */
    fRotation = orxFrame_GetRotation(_pstRenderFrame, orxFALSE);
    orxFrame_GetScale(_pstRenderFrame, orxFRAME_SPACE_GLOBAL, &fScaleX, &fScaleY);
    orxFrame_GetPosition(_pstRenderFrame, orxFRAME_SPACE_GLOBAL, &vPosition);

    /* No scale nor rotation? */
    if((fRotation == orxFLOAT_0) && (fScaleX == orxFLOAT_1) && (fScaleY == orxFLOAT_1))
    {
      /* Updates position with pivot */
      orxVector_Sub(&vPosition, &vPosition, &vPivot);

      /* Blits bitmap */
      eResult = orxDisplay_BlitBitmap(_pstRenderBitmap, pstBitmap, orxF2U(vPosition.fX), orxF2U(vPosition.fY));
    }
    else
    {
      orxBITMAP_TRANSFORM stTransform;

      /* Sets transformation values */
      stTransform.s32SrcX   = orxF2S(vPivot.fX);
      stTransform.s32SrcY   = orxF2S(vPivot.fY);
      stTransform.s32DstX   = orxF2S(vPosition.fX);
      stTransform.s32DstY   = orxF2S(vPosition.fY);
      stTransform.fScaleX   = fScaleX;
      stTransform.fScaleY   = fScaleY;
      stTransform.fRotation = fRotation;

      /* Blits bitmap */
      eResult = orxDisplay_TransformBitmap(_pstRenderBitmap, pstBitmap, &stTransform, 0);
    }
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return eResult;
}

/** Renders a viewport
 * @param[in]   _pstViewport    Viewport to render
 */
orxSTATIC orxINLINE orxVOID orxRender_RenderViewport(orxCONST orxVIEWPORT *_pstViewport)
{
  /* Checks */
  orxASSERT(sstRender.u32Flags & orxRENDER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);

  /* Is viewport enabled? */
  if(orxViewport_IsEnabled(_pstViewport) != orxFALSE)
  {
    orxTEXTURE *pstTexture;
    orxBITMAP  *pstBitmap;

    /* Gets viewport texture */
    pstTexture = orxViewport_GetTexture(_pstViewport);

    /* Has texture? */
    if(pstTexture != orxNULL)
    {
      /* Gets its bitmap */
      pstBitmap = orxTexture_GetBitmap(pstTexture);
    }
    else
    {
      /* !!! MSG !!! */
      pstBitmap = orxNULL;
    }

    /* Valid? */
    if(pstBitmap != orxNULL)
    {
      orxCAMERA  *pstCamera;
      orxFLOAT    fTextureWidth, fTextureHeight;
      orxAABOX    stViewportBox, stTextureBox;
      orxVECTOR   vViewportCenter;

      /* Gets texture size */
      orxTexture_GetSize(pstTexture, &fTextureWidth, &fTextureHeight);

      /* Inits texture box */
      orxVector_SetAll(&(stTextureBox.vTL), orxFLOAT_0);
      orxVector_Set(&(stTextureBox.vBR), fTextureWidth, fTextureHeight, orxFLOAT_0);

      /* Gets viewport box */
      orxViewport_GetBox(_pstViewport, &stViewportBox);

      /* Gets its center */
      orxAABox_GetCenter(&stViewportBox, &vViewportCenter);

      /* Does it intersect with texture */
      if(orxAABox_Test2DIntersection(&stTextureBox, &stViewportBox) != orxFALSE)
      {
        /* Sets bitmap clipping */
        orxDisplay_SetBitmapClipping(pstBitmap, orxF2U(stViewportBox.vTL.fX), orxF2U(stViewportBox.vTL.fY), orxF2U(stViewportBox.vBR.fX), orxF2U(stViewportBox.vBR.fY));

        /* Should clear bitmap? */
        if(orxViewport_IsBackgroundClearingEnabled(_pstViewport) != orxFALSE)
        {
          /* Clears it */
          orxDisplay_ClearBitmap(pstBitmap, orxViewport_GetBackgroundColor(_pstViewport));
        }

        /* Gets camera */
        pstCamera = orxViewport_GetCamera(_pstViewport);

        /* Valid 2D camera? */
        if((pstCamera != orxNULL)
        && (orxStructure_TestFlags(pstCamera, orxCAMERA_KU32_FLAG_2D) != orxFALSE))
        {
          orxFRAME *pstRenderFrame;

          /* Creates rendering frame */
          pstRenderFrame = orxFrame_Create(orxFRAME_KU32_FLAG_NONE);

          /* Valid? */
          if(pstRenderFrame != orxNULL)
          {
            orxOBJECT      *pstObject;
            orxRENDER_NODE *pstRenderNode;
            orxVECTOR       vCameraPosition;
            orxAABOX        stFrustum;
            orxFLOAT        fRenderScaleX, fRenderScaleY, fZoom, fRenderRotation, fCameraWidth, fCameraHeight, fCameraSqrBoundingRadius;

            /* Gets camera frustum */
            orxCamera_GetFrustum(pstCamera, &stFrustum);

            /* Gets camera zoom */
            fZoom = orxCamera_GetZoom(pstCamera);

            /* Gets camera size */
            fCameraWidth  = stFrustum.vBR.fX - stFrustum.vTL.fX;
            fCameraHeight = stFrustum.vBR.fY - stFrustum.vTL.fY;

            /* Gets camera center */
            orxVector_Add(&vCameraPosition, &(stFrustum.vTL), &(stFrustum.vBR));
            orxVector_Mulf(&vCameraPosition, &vCameraPosition, orx2F(0.5f));

            /* Gets camera square bounding radius */
            fCameraSqrBoundingRadius = orx2F(0.5f) * ((fCameraWidth * fCameraWidth) + (fCameraHeight * fCameraHeight));

            /* Gets rendering scales */
            fRenderScaleX = fZoom * (stViewportBox.vBR.fX - stViewportBox.vTL.fX) / fCameraWidth;
            fRenderScaleY = fZoom * (stViewportBox.vBR.fY - stViewportBox.vTL.fY) / fCameraHeight;

            /* Gets camera rotation */
            fRenderRotation = orxCamera_GetRotation(pstCamera);

            /* For all objects */
            for(pstObject = (orxOBJECT *)orxStructure_GetFirst(orxSTRUCTURE_ID_OBJECT);
                pstObject != orxNULL;
                pstObject = (orxOBJECT *)orxStructure_GetNext(pstObject))
            {
              /* Is object enabled? */
              if(orxObject_IsEnabled(pstObject) != orxFALSE)
              {
                orxGRAPHIC *pstGraphic;

                /* Gets object's graphic */
                pstGraphic = orxOBJECT_GET_STRUCTURE(pstObject, GRAPHIC);

                /* Valid 2D graphic? */
                if((pstGraphic != orxNULL)
                && (orxStructure_TestFlags(pstGraphic, orxGRAPHIC_KU32_FLAG_2D) != orxFALSE))
                {
                  orxFRAME   *pstFrame;
                  orxTEXTURE *pstTexture;

                  /* Gets object's frame */
                  pstFrame = orxOBJECT_GET_STRUCTURE(pstObject, FRAME);

                  /* Gets graphic's texture */
                  pstTexture = orxSTRUCTURE_GET_POINTER(orxGraphic_GetData(pstGraphic), TEXTURE);

                  /* Valid? */
                  if((pstFrame != orxNULL) && (pstTexture != orxNULL))
                  {
                    orxVECTOR vObjectPos;

                    /* Gets its position */
                    orxFrame_GetPosition(pstFrame, orxFRAME_SPACE_GLOBAL, &vObjectPos);

                    /* Is object in Z frustum? */
                    if((vObjectPos.fZ >= stFrustum.vTL.fZ) && (vObjectPos.fZ <= stFrustum.vBR.fZ))
                    {
                      orxFLOAT  fWidth, fHeight, fObjectScaleX, fObjectScaleY, fObjectSqrBoundingRadius, fSqrDist;

                      /* Gets its texture size */
                      orxTexture_GetSize(pstTexture, &fWidth, &fHeight);

                      /* Gets object's scales */
                      orxFrame_GetScale(pstFrame, orxFRAME_SPACE_GLOBAL, &fObjectScaleX, &fObjectScaleY);

                      /* Updates it with object scale */
                      fWidth  *= fObjectScaleX;
                      fHeight *= fObjectScaleY;

                      /* Gets object square bounding radius */
                      fObjectSqrBoundingRadius = orx2F(1.5f) * ((fWidth * fWidth) + (fHeight * fHeight));

                      /* Gets 2D square distance to camera */
                      fSqrDist = orxVector_GetSquareDistance(&vObjectPos, &vCameraPosition);

                      /* Circle test between object & camera */
                      if(fSqrDist * (fZoom * fZoom) <= (fCameraSqrBoundingRadius + fObjectSqrBoundingRadius))
                      {
                        orxLINKLIST_NODE *pstNode;

                        /* Creates a render node */
                        pstRenderNode = orxBank_Allocate(sstRender.pstRenderBank);

                        /* Cleans its internal node */
                        orxMemory_Set(pstRenderNode, 0, sizeof(orxLINKLIST_NODE));

                        /* Stores object */
                        pstRenderNode->pstObject = pstObject;

                        /* Stores its position */
                        orxVector_Copy(&(pstRenderNode->vPosition), &vObjectPos);

                        /* Empty list? */
                        if(orxLinkList_GetCounter(&(sstRender.stRenderList)) == 0)
                        {
                          /* Adds node at beginning */
                          orxLinkList_AddStart(&(sstRender.stRenderList), (orxLINKLIST_NODE *)pstRenderNode);
                        }
                        else
                        {
                          /* Finds correct node */
                          for(pstNode = orxLinkList_GetFirst(&(sstRender.stRenderList));
                              (pstNode != orxNULL);
                              pstNode = orxLinkList_GetNext(pstNode))
                          {
                            /* Is current object further? */
                            if(vObjectPos.fZ > ((orxRENDER_NODE *)pstNode)->vPosition.fZ)
                            {
                              break;
                            }
                          }

                          /* End of list reached? */
                          if(pstNode == orxNULL)
                          {
                            /* Adds it at end */
                            orxLinkList_AddEnd(&(sstRender.stRenderList), (orxLINKLIST_NODE *)pstRenderNode);
                          }
                          else
                          {
                            /* Adds it before found node */
                            orxLinkList_AddBefore(pstNode, (orxLINKLIST_NODE *)pstRenderNode);
                          }
                        }
                      }
                    }
                  }
                }
              }
            }

            /* For all render nodes */
            for(pstRenderNode = (orxRENDER_NODE *)orxLinkList_GetFirst(&(sstRender.stRenderList));
                pstRenderNode != orxNULL;
                pstRenderNode = (orxRENDER_NODE *)orxLinkList_GetNext((orxLINKLIST_NODE *)pstRenderNode))
            {
              orxFRAME *pstFrame;
              orxVECTOR vObjectPos, vRenderPos;
              orxFLOAT  fObjectScaleX, fObjectScaleY, fObjectRotation, fScrollX, fScrollY;

              /* Gets object */
              pstObject = pstRenderNode->pstObject;

              /* Gets object's position */
              orxVector_Copy(&vObjectPos, &(pstRenderNode->vPosition));

              /* Gets object's frame */
              pstFrame = orxOBJECT_GET_STRUCTURE(pstObject, FRAME);

              /* Gets object's scales */
              orxFrame_GetScale(pstFrame, orxFRAME_SPACE_GLOBAL, &fObjectScaleX, &fObjectScaleY);

              /* Gets object's rotation */
              fObjectRotation = orxFrame_GetRotation(pstFrame, orxFRAME_SPACE_GLOBAL);

              /* Uses differential scrolling? */
              if((orxStructure_TestFlags(pstFrame, orxFRAME_KU32_MASK_SCROLL_BOTH) != orxFALSE)
              && (vObjectPos.fZ > stFrustum.vTL.fZ))
              {
                orxREGISTER orxFLOAT fScroll;

                /* Gets scroll coefficient */
                fScroll = (stFrustum.vBR.fZ - stFrustum.vTL.fZ) / (vObjectPos.fZ - stFrustum.vTL.fZ);

                /* Gets differential scrolling values */
                fScrollX = (orxStructure_TestFlags(pstFrame, orxFRAME_KU32_FLAG_SCROLL_X) != orxFALSE) ? fScroll : orxFLOAT_1;
                fScrollY = (orxStructure_TestFlags(pstFrame, orxFRAME_KU32_FLAG_SCROLL_Y) != orxFALSE) ? fScroll : orxFLOAT_1;
              }
              else
              {
                /* No differential scrolling */
                fScrollX = fScrollY = orxFLOAT_1;
              }

              /* Gets position in camera space */
              orxVector_Sub(&vRenderPos, &vObjectPos, &(vCameraPosition));
              vRenderPos.fX  *= fRenderScaleX * fScrollX;
              vRenderPos.fY  *= fRenderScaleY * fScrollY;

              /* Has camera rotation? */
              if(fRenderRotation != orxFLOAT_0)
              {
                /* Rotates it */
                orxVector_2DRotate(&vRenderPos, &vRenderPos, -fRenderRotation);
              }

              /* Gets position in screen space */
              orxVector_Add(&vRenderPos, &vRenderPos, &vViewportCenter);

              /* Updates render frame */
              orxFrame_SetPosition(pstRenderFrame, &vRenderPos);
              orxFrame_SetRotation(pstRenderFrame, fObjectRotation - fRenderRotation);
              orxFrame_SetScale(pstRenderFrame, fRenderScaleX * fObjectScaleX, fRenderScaleY * fObjectScaleY);

              /* Renders it */
              if(orxRender_RenderObject(pstObject, pstBitmap, pstRenderFrame) != orxSTATUS_SUCCESS)
              {
                /* Prints error message */
                orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "[orxOBJECT %p / orxBITMAP %p] couldn't be rendered.", pstObject, pstBitmap);
              }
            }

            /* Cleans rendering bank */
            orxBank_Clear(sstRender.pstRenderBank);

            /* Cleans rendering list */
            orxMemory_Set(&(sstRender.stRenderList), 0, sizeof(orxLINKLIST));

            /* Deletes rendering frame */
            orxFrame_Delete(pstRenderFrame);
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
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}

/** Renders all (callback to register on a clock)
 * @param[in]   _pstClockInfo   Clock info of the clock used upon registration
 * @param[in]   _pstContext     Context sent when registering callback to the clock
 */
orxVOID orxFASTCALL orxRender_RenderAll(orxCONST orxCLOCK_INFO *_pstClockInfo, orxVOID *_pstContext)
{
  orxVIEWPORT *pstViewport;

  /* Checks */
  orxASSERT(sstRender.u32Flags & orxRENDER_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstClockInfo != orxNULL);

  /* For all viewports */
  for(pstViewport = (orxVIEWPORT *)orxStructure_GetLast(orxSTRUCTURE_ID_VIEWPORT);
      pstViewport != orxNULL;
      pstViewport = (orxVIEWPORT *)orxStructure_GetPrevious(pstViewport))
  {
    /* Renders it */
    orxRender_RenderViewport(pstViewport);
  }

  /* Increases FPS counter */
  orxFPS_IncreaseFrameCounter();

  /* Selects render config section */
  orxConfig_SelectSection(orxRENDER_KZ_CONFIG_SECTION);

  /* Should display FPS? */
  if(orxConfig_GetBool(orxRENDER_KZ_CONFIG_SHOW_FPS) != orxFALSE)
  {
    orxBITMAP_TRANSFORM stTextTransform;
    orxCHAR             acText[16];

    /* Clears text transform */
    orxMemory_Set(&stTextTransform, 0, sizeof(orxBITMAP_TRANSFORM));

    /* Inits it */
    stTextTransform.fScaleX = stTextTransform.fScaleY = orx2F(0.8f);
    stTextTransform.s32DstX = stTextTransform.s32DstY = 10;

    /* Writes text */
    orxString_Print(acText, orxRENDER_KZ_FPS_FORMAT, orxFPS_GetFPS());

    /* Display FPS */
    orxDisplay_DrawText(orxDisplay_GetScreenBitmap(), &stTextTransform, orxRENDER_KST_DEFAULT_COLOR, acText);
  }

  /* Swap buffers */
  orxDisplay_Swap();

  return;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Gets a world position from a screen one
 * @param[in]  _pvScreenPosition        Screen space position
 * @param[out] _pvWorldPosition         Corresponding world position
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxRender_Home_GetWorldPosition(orxCONST orxVECTOR *_pvScreenPosition, orxVECTOR *_pvWorldPosition)
{
  orxVIEWPORT  *pstViewport;
  orxSTATUS     eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstRender.u32Flags & orxRENDER_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvScreenPosition != orxNULL);
  orxASSERT(_pvWorldPosition != orxNULL);

  /* For all viewports */
  for(pstViewport = (orxVIEWPORT *)orxStructure_GetFirst(orxSTRUCTURE_ID_VIEWPORT);
      pstViewport != orxNULL;
      pstViewport = (orxVIEWPORT *)orxStructure_GetNext(pstViewport))
  {
    orxCAMERA *pstCamera;

    /* Is active and has camera? */
    if((orxViewport_IsEnabled(pstViewport) != orxFALSE)
    && ((pstCamera = orxViewport_GetCamera(pstViewport)) != orxNULL))
    {
      orxAABOX stViewportBox;

      /* Gets viewport box */
      orxViewport_GetBox(pstViewport, &stViewportBox);

      /* Is position in box? */
      if((_pvScreenPosition->fX >= stViewportBox.vTL.fX)
      && (_pvScreenPosition->fX <= stViewportBox.vBR.fX)
      && (_pvScreenPosition->fY >= stViewportBox.vTL.fY)
      && (_pvScreenPosition->fY <= stViewportBox.vBR.fY))
      {
        orxVECTOR vLocalPosition;
        orxAABOX  stCameraFrustum;

        /* Gets viewport space normalized position */
        orxVector_Set(&vLocalPosition, (_pvScreenPosition->fX - stViewportBox.vTL.fX) / (stViewportBox.vBR.fX - stViewportBox.vTL.fX), (_pvScreenPosition->fY - stViewportBox.vTL.fY) / (stViewportBox.vBR.fY - stViewportBox.vTL.fY), orxFLOAT_0);

        /* Gets camera frustum */
        orxCamera_GetFrustum(pstCamera, &stCameraFrustum);

        /* Gets its world coordinates */
        orxVector_Set(_pvWorldPosition, stCameraFrustum.vTL.fX + (vLocalPosition.fX * (stCameraFrustum.vBR.fX - stCameraFrustum.vTL.fX)), stCameraFrustum.vTL.fY + (vLocalPosition.fY * (stCameraFrustum.vBR.fY - stCameraFrustum.vTL.fY)), orx2F(0.5f) * (stCameraFrustum.vBR.fZ - stCameraFrustum.vTL.fZ));

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;

        break;
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Inits the Render module
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxRender_Home_Init()
{
  orxSTATUS eResult;

  /* Not already Initialized? */
  if(!(sstRender.u32Flags & orxRENDER_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Set(&sstRender, 0, sizeof(orxRENDER_STATIC));

    /* Creates rendering bank */
    sstRender.pstRenderBank = orxBank_Create(orxRENDER_KU32_ORDER_BANK_SIZE, sizeof(orxRENDER_NODE), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if(sstRender.pstRenderBank != orxNULL)
    {
      /* Creates rendering clock */
      sstRender.pstClock = orxClock_Create(orxFLOAT_0, orxCLOCK_TYPE_RENDER);
      orxClock_SetModifier(sstRender.pstClock, orxCLOCK_MOD_TYPE_FIXED, orxRENDER_KF_TICK_SIZE);

      /* Valid? */
      if(sstRender.pstClock != orxNULL)
      {
        /* Registers rendering function */
        eResult = orxClock_Register(sstRender.pstClock, orxRender_RenderAll, orxNULL, orxMODULE_ID_RENDER);
      }
      else
      {
        /* Deletes bank */
        orxBank_Delete(sstRender.pstRenderBank);

        /* Updates result */
        eResult = orxSTATUS_FAILURE;
      }
    }
    else
    {
      /* Updates result */
      eResult = orxSTATUS_FAILURE;
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
    sstRender.u32Flags = orxRENDER_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return eResult;
}

/** Exits from the Render module
 */
orxVOID orxRender_Home_Exit()
{
  /* Initialized? */
  if(sstRender.u32Flags & orxRENDER_KU32_STATIC_FLAG_READY)
  {
    /* Unregisters rendering function */
    orxClock_Unregister(sstRender.pstClock, orxRender_RenderAll);

    /* Deletes rendering clock */
    orxClock_Delete(sstRender.pstClock);

    /* Deletes rendering bank */
    orxBank_Delete(sstRender.pstRenderBank);

    /* Updates flags */
    sstRender.u32Flags &= ~orxRENDER_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}


/********************
 *  Plugin Related  *
 ********************/

orxPLUGIN_USER_CORE_FUNCTION_START(RENDER);

orxPLUGIN_USER_CORE_FUNCTION_ADD(orxRender_Home_Init, RENDER, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxRender_Home_Exit, RENDER, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxRender_Home_GetWorldPosition, RENDER, GET_WORLD_POSITION);

orxPLUGIN_USER_CORE_FUNCTION_END();
