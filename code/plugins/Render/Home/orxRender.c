/* Orx - Portable Game Engine
 *
 * Orx is the legal property of its developers, whose names
 * are listed in the COPYRIGHT file distributed 
 * with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * @file orxRender.c
 * @date 25/09/2007
 * @author iarwain@orx-project.org
 *
 * Home render plugin implementation
 *
 * @todo
 */


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
 * @param[in]   _pstObject        Object to render
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
    orxRGBA         stBackupColor = 0;
    orxBITMAP      *pstBitmap;
    orxTEXTURE     *pstTexture;
    orxANIMPOINTER *pstAnimPointer;
    orxDISPLAY_BLEND_MODE eBlendMode;
    orxVECTOR       vPivot, vPosition, vSize, vScale;
    orxFLOAT        fRotation, fClipTop, fClipLeft, fClipBottom, fClipRight, fRepeatX, fRepeatY;

    /* Gets animation pointer */
    pstAnimPointer = orxOBJECT_GET_STRUCTURE(_pstObject, ANIMPOINTER);

    /* Valid? */
    if(pstAnimPointer != orxNULL)
    {
      orxGRAPHIC *pstTemp;

      /* Gets current anim data */
      pstTemp = orxGRAPHIC(orxAnimPointer_GetCurrentAnimData(pstAnimPointer));

      /* Valid? */
      if(pstTemp != orxNULL)
      {
        /* Uses it */
        pstGraphic = pstTemp;
      }
    }

    /* Gets its pivot */
    orxGraphic_GetPivot(pstGraphic, &vPivot);

    /* Gets its texture */
    pstTexture = orxTEXTURE(orxGraphic_GetData(pstGraphic));

    /* Gets its bitmap */
    pstBitmap = orxTexture_GetBitmap(pstTexture);

    /* Gets rendering frame's position, rotation & scale */
    fRotation = orxFrame_GetRotation(_pstRenderFrame, orxFALSE);
    orxFrame_GetScale(_pstRenderFrame, orxFRAME_SPACE_GLOBAL, &vScale);
    orxFrame_GetPosition(_pstRenderFrame, orxFRAME_SPACE_GLOBAL, &vPosition);

    /* Gets its clipping corners */
    orxGraphic_GetSize(pstGraphic, &vSize);
    fClipTop      = orxGraphic_GetTop(pstGraphic);
    fClipLeft     = orxGraphic_GetLeft(pstGraphic);
    fClipBottom   = fClipTop + vSize.fY;
    fClipRight    = fClipLeft + vSize.fX;

    /* Updates its clipping */
    orxDisplay_SetBitmapClipping(pstBitmap, orxF2U(fClipLeft), orxF2U(fClipTop), orxF2U(fClipRight), orxF2U(fClipBottom));

    /* Uses flipping? */
    if(orxStructure_TestFlags(pstGraphic, orxGRAPHIC_KU32_MASK_FLIP_BOTH) != orxFALSE)
    {
      /* X-axis flip? */
      if(orxStructure_TestFlags(pstGraphic, orxGRAPHIC_KU32_FLAG_FLIP_X) != orxFALSE)
      {
        /* Updates render scale */
        vScale.fX = -vScale.fX;
      }

      /* Y-axis flip? */
      if(orxStructure_TestFlags(pstGraphic, orxGRAPHIC_KU32_FLAG_FLIP_Y) != orxFALSE)
      {
        /* Updates render scale */
        vScale.fY = -vScale.fY;
      }
    }

    /* Has object color? */
    if(orxObject_HasColor(_pstObject) != orxFALSE)
    {
      orxCOLOR stColor;

      /* Backups previous color */
      stBackupColor = orxDisplay_GetBitmapColor(pstBitmap);

      /* Updates display color */
      orxDisplay_SetBitmapColor(pstBitmap, orxColor_ToRGBA(orxObject_GetColor(_pstObject, &stColor)));
    }
    /* Has graphic color? */
    else if(orxGraphic_HasColor(pstGraphic) != orxFALSE)
    {
      orxCOLOR stColor;

      /* Backups previous color */
      stBackupColor = orxDisplay_GetBitmapColor(pstBitmap);

      /* Updates display color */
      orxDisplay_SetBitmapColor(pstBitmap, orxColor_ToRGBA(orxGraphic_GetColor(pstGraphic, &stColor)));
    }

    /* Gets repeat values */
    orxGraphic_GetRepeat(pstGraphic, &fRepeatX, &fRepeatY);

    /* Gets graphic blend mode */
    eBlendMode = orxGraphic_GetBlendMode(pstGraphic);

    /* None? */
    if(eBlendMode == orxDISPLAY_BLEND_MODE_NONE)
    {
      /* Gets object blend mode */
      eBlendMode = orxObject_GetBlendMode(_pstObject);
    }

    /* No scale nor rotation nor repeat? */
    if((fRotation == orxFLOAT_0) && (vScale.fX == orxFLOAT_1) && (vScale.fY == orxFLOAT_1) && (fRepeatX == orxFLOAT_1) && (fRepeatY == orxFLOAT_1))
    {
      /* Updates position with pivot */
      orxVector_Sub(&vPosition, &vPosition, &vPivot);

      /* Blits bitmap */
      eResult = orxDisplay_BlitBitmap(_pstRenderBitmap, pstBitmap, vPosition.fX, vPosition.fY, eBlendMode);
    }
    else
    {
      /* Valid scale? */
      if((vScale.fX != orxFLOAT_0) && (vScale.fY != orxFLOAT_0))
      {
        orxBITMAP_TRANSFORM   stTransform;
        orxDISPLAY_SMOOTHING  eSmoothing;

        /* Gets graphic smoothing */
        eSmoothing = orxGraphic_GetSmoothing(pstGraphic);

        /* Default? */
        if(eSmoothing == orxDISPLAY_SMOOTHING_DEFAULT)
        {
          /* Gets object smoothing */
          eSmoothing = orxObject_GetSmoothing(_pstObject);
        }

        /* No repeat? */
        if((fRepeatX == orxFLOAT_1)  && (fRepeatY == orxFLOAT_1))
        {
          /* Sets transformation values */
          stTransform.fSrcX     = vPivot.fX;
          stTransform.fSrcY     = vPivot.fY;
          stTransform.fDstX     = vPosition.fX;
          stTransform.fDstY     = vPosition.fY;
          stTransform.fScaleX   = vScale.fX;
          stTransform.fScaleY   = vScale.fY;
          stTransform.fRotation = fRotation;

          /* Blits bitmap */
          eResult = orxDisplay_TransformBitmap(_pstRenderBitmap, pstBitmap, &stTransform, eSmoothing, eBlendMode);
        }
        else
        {
          orxFLOAT fIncX, fIncY, fCos, fSin, fX, fY, fEndX, fEndY, fRemainderX, fRemainderY, fRelativePivotX, fRelativePivotY;

          /* Gets cosine and sine of the object angle */
          fCos = orxMath_Cos(-fRotation);
          fSin = orxMath_Sin(-fRotation);

          /* Updates scales */
          vScale.fX /= fRepeatX;
          vScale.fY /= fRepeatY;

          /* Updates increments */
          fIncX = vSize.fX * vScale.fX;
          fIncY = vSize.fY * vScale.fY;

          fRelativePivotX = vPivot.fX / vSize.fX;
          fRelativePivotY = vPivot.fY / vSize.fY;

          /* For all lines */
          for(fY = -fRelativePivotY * fIncY * (fRepeatY - orxFLOAT_1), fEndY = (orxFLOAT_1 - fRelativePivotY) * fIncY * fRepeatY, fRemainderY = fRepeatY;
              fY <= fEndY;
              fY += fIncY, fRemainderY -= orxFLOAT_1)
          {
            /* For all columns */
            for(fX = -fRelativePivotX * fIncX * (fRepeatX - orxFLOAT_1), fEndX = (orxFLOAT_1 - fRelativePivotX) * fIncX * fRepeatX, fRemainderX = fRepeatX;
                fX <= fEndX;
                fX += fIncX, fRemainderX -= orxFLOAT_1)
            {
              /* Valid? */
              if(fRemainderX > orxFLOAT_0)
              {
                orxFLOAT fOffsetX, fOffsetY;

                /* Updates clipping */
                orxDisplay_SetBitmapClipping(pstBitmap, orxF2U(fClipLeft), orxF2U(fClipTop), orxF2U(fClipLeft + orxMIN(orxFLOAT_1, fRemainderX) * vSize.fX), orxF2U(fClipTop + orxMIN(orxFLOAT_1, fRemainderY) * vSize.fY));

                /* Computes offsets */
                fOffsetX = fCos * fX + fSin * fY;
                fOffsetY = -fSin * fX + fCos * fY;

                /* Sets transformation values */
                stTransform.fSrcX     = vPivot.fX;
                stTransform.fSrcY     = vPivot.fY;
                stTransform.fDstX     = vPosition.fX + fOffsetX;
                stTransform.fDstY     = vPosition.fY + fOffsetY;
                stTransform.fScaleX   = vScale.fX;
                stTransform.fScaleY   = vScale.fY;
                stTransform.fRotation = fRotation;

                /* Blits bitmap */
                eResult = orxDisplay_TransformBitmap(_pstRenderBitmap, pstBitmap, &stTransform, eSmoothing, eBlendMode);
              }
            }
          }
        }
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Scaling factor should not equal 0. Got (%g, %g).", vScale.fX, vScale.fY);

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
    }

    /* Has object or graphic color? */
    if((orxObject_HasColor(_pstObject) != orxFALSE)
    || (orxGraphic_HasColor(pstGraphic) != orxFALSE))
    {
      /* Restores its original color */
      orxDisplay_SetBitmapColor(pstBitmap, stBackupColor);
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Invalid graphic or non-2d graphic detected.");
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
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Failed to get texture for viewport.");
      pstBitmap = orxNULL;
    }

    /* Valid? */
    if(pstBitmap != orxNULL)
    {
      orxCAMERA  *pstCamera;

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
          orxAABOX  stFrustum, stViewportBox, stTextureBox;
          orxFLOAT  fCameraWidth, fCameraHeight, fTextureWidth, fTextureHeight;
          orxVECTOR vViewportCenter;

          /* Gets camera frustum */
          orxCamera_GetFrustum(pstCamera, &stFrustum);

          /* Gets camera size */
          fCameraWidth  = stFrustum.vBR.fX - stFrustum.vTL.fX;
          fCameraHeight = stFrustum.vBR.fY - stFrustum.vTL.fY;

          /* Gets texture size */
          fTextureWidth   = orxTexture_GetWidth(pstTexture);
          fTextureHeight  = orxTexture_GetHeight(pstTexture);

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
            orxFLOAT fCorrectionRatio;

            /* Gets current correction ratio */
            fCorrectionRatio = orxViewport_GetCorrectionRatio(_pstViewport);

            /* Has correction ratio? */
            if(fCorrectionRatio != orxFLOAT_1)
            {
              orxFLOAT fDelta;

              /* Gets rendering limit delta using correction ratio */
              fDelta = orx2F(0.5f) * (orxFLOAT_1 - fCorrectionRatio) * (stViewportBox.vBR.fX - stViewportBox.vTL.fX);

              /* Updates viewport */
              stViewportBox.vTL.fX += fDelta;
              stViewportBox.vBR.fX -= fDelta;
            }

            /* Sets bitmap clipping */
            orxDisplay_SetBitmapClipping(pstBitmap, orxF2U(stViewportBox.vTL.fX), orxF2U(stViewportBox.vTL.fY), orxF2U(stViewportBox.vBR.fX), orxF2U(stViewportBox.vBR.fY));
            
            /* Should clear bitmap? */
            if(orxViewport_IsBackgroundClearingEnabled(_pstViewport) != orxFALSE)
            {
              /* Clears it */
              orxDisplay_ClearBitmap(pstBitmap, orxViewport_GetBackgroundColor(_pstViewport));
            }

            /* Valid? */
            if((fCameraWidth > orxFLOAT_0)
            && (fCameraHeight > orxFLOAT_0))
            {
              orxOBJECT      *pstObject;
              orxRENDER_NODE *pstRenderNode;
              orxVECTOR       vCameraCenter, vCameraPosition;
              orxFLOAT        fRenderScaleX, fRenderScaleY, fZoom, fRenderRotation, fCameraSqrBoundingRadius;

              /* Gets camera zoom */
              fZoom = orxCamera_GetZoom(pstCamera);

              /* Gets camera center */
              orxVector_Add(&vCameraCenter, &(stFrustum.vTL), &(stFrustum.vBR));
              orxVector_Mulf(&vCameraCenter, &vCameraCenter, orx2F(0.5f));

              /* Gets camera position */
              orxFrame_GetPosition(orxCamera_GetFrame(pstCamera), orxFRAME_SPACE_GLOBAL, &vCameraPosition);

              /* Gets camera square bounding radius */
              fCameraSqrBoundingRadius = orx2F(0.5f) * ((fCameraWidth * fCameraWidth) + (fCameraHeight * fCameraHeight));

              /* Gets rendering scales */
              fRenderScaleX = fZoom * (stViewportBox.vBR.fX - stViewportBox.vTL.fX) / fCameraWidth;
              fRenderScaleY = fZoom * (stViewportBox.vBR.fY - stViewportBox.vTL.fY) / fCameraHeight;

              /* Gets camera rotation */
              fRenderRotation = orxCamera_GetRotation(pstCamera);

              /* For all objects */
              for(pstObject = orxOBJECT(orxStructure_GetFirst(orxSTRUCTURE_ID_OBJECT));
                  pstObject != orxNULL;
                  pstObject = orxOBJECT(orxStructure_GetNext(pstObject)))
              {
                /* Updates its render status */
                orxObject_SetRendered(pstObject, orxFALSE);

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
                    pstTexture = orxTEXTURE(orxGraphic_GetData(pstGraphic));

                    /* Valid? */
                    if((pstFrame != orxNULL) && (pstTexture != orxNULL))
                    {
                      orxVECTOR vObjectPos;

                      /* Gets its position */
                      orxFrame_GetPosition(pstFrame, orxFRAME_SPACE_GLOBAL, &vObjectPos);

                      /* Is object in Z frustum? */
                      if((vObjectPos.fZ > vCameraPosition.fZ) && (vObjectPos.fZ >= stFrustum.vTL.fZ) && (vObjectPos.fZ <= stFrustum.vBR.fZ))
                      {
                        orxFLOAT  fObjectSqrBoundingRadius, fSqrDist;
                        orxVECTOR vSize, vObjectScale, vDist;

                        /* Gets its size */
                        orxGraphic_GetSize(pstGraphic, &vSize);

                        /* Gets object's scales */
                        orxFrame_GetScale(pstFrame, orxFRAME_SPACE_GLOBAL, &vObjectScale);

                        /* Updates it with object scale */
                        vSize.fX *= vObjectScale.fX;
                        vSize.fY *= vObjectScale.fY;

                        /* Gets object square bounding radius */
                        fObjectSqrBoundingRadius = orx2F(1.5f) * ((vSize.fX * vSize.fX) + (vSize.fY * vSize.fY));

                        /* Gets 2D distance vector */
                        orxVector_Sub(&vDist, &vObjectPos, &vCameraCenter);
                        vDist.fZ = orxFLOAT_0;

                        /* Gets 2D square distance to camera */
                        fSqrDist = orxVector_GetSquareSize(&vDist);

                        /* Circle test between object & camera */
                        if(fSqrDist * (fZoom * fZoom) <= (fCameraSqrBoundingRadius + fObjectSqrBoundingRadius))
                        {
                          orxLINKLIST_NODE *pstNode;

                          /* Creates a render node */
                          pstRenderNode = orxBank_Allocate(sstRender.pstRenderBank);

                          /* Cleans its internal node */
                          orxMemory_Zero(pstRenderNode, sizeof(orxLINKLIST_NODE));

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
                orxVECTOR vObjectPos, vRenderPos, vObjectScale;
                orxFLOAT  fObjectRotation;

                /* Gets object */
                pstObject = pstRenderNode->pstObject;

                /* Gets object's position */
                orxVector_Copy(&vObjectPos, &(pstRenderNode->vPosition));

                /* Gets object's frame */
                pstFrame = orxOBJECT_GET_STRUCTURE(pstObject, FRAME);

                /* Gets object's scales */
                orxFrame_GetScale(pstFrame, orxFRAME_SPACE_GLOBAL, &vObjectScale);

                /* Gets object's rotation */
                fObjectRotation = orxFrame_GetRotation(pstFrame, orxFRAME_SPACE_GLOBAL);

                /* Gets position in camera space */
                orxVector_Sub(&vRenderPos, &vObjectPos, &(vCameraCenter));
                vRenderPos.fX  *= fRenderScaleX;
                vRenderPos.fY  *= fRenderScaleY;

                /* Uses differential scrolling? */
                if(orxStructure_TestFlags(pstFrame, orxFRAME_KU32_MASK_SCROLL_BOTH) != orxFALSE)
                {
                  orxREGISTER orxFLOAT fScroll;

                  /* Gets scroll coefficient */
                  fScroll = (stFrustum.vBR.fZ - stFrustum.vTL.fZ) / (vObjectPos.fZ - stFrustum.vTL.fZ);

                  /* X-axis scroll? */
                  if(orxStructure_TestFlags(pstFrame, orxFRAME_KU32_FLAG_SCROLL_X) != orxFALSE)
                  {
                    /* Updates render position */
                    vRenderPos.fX *= fScroll;
                  }

                  /* Y-axis scroll? */
                  if(orxStructure_TestFlags(pstFrame, orxFRAME_KU32_FLAG_SCROLL_Y) != orxFALSE)
                  {
                    /* Updates render position */
                    vRenderPos.fY *= fScroll;
                  }
                }

                /* Uses flipping? */
                if(orxStructure_TestFlags(pstFrame, orxFRAME_KU32_MASK_FLIP_BOTH) != orxFALSE)
                {
                  /* X-axis flip? */
                  if(orxStructure_TestFlags(pstFrame, orxFRAME_KU32_FLAG_FLIP_X) != orxFALSE)
                  {
                    /* Updates render scale */
                    fRenderScaleX = -fRenderScaleX;
                  }

                  /* Y-axis flip? */
                  if(orxStructure_TestFlags(pstFrame, orxFRAME_KU32_FLAG_FLIP_Y) != orxFALSE)
                  {
                    /* Updates render scale */
                    fRenderScaleY = -fRenderScaleY;
                  }
                }

                /* Uses depth scaling? */
                if(orxStructure_TestFlags(pstFrame, orxFRAME_KU32_FLAG_DEPTH_SCALE) != orxFALSE)
                {
                  orxFLOAT fObjectRelativeDepth, fCameraDepth, fDepthScale;

                  /* Gets objects relative depth */
                  fObjectRelativeDepth = vObjectPos.fZ - vCameraPosition.fZ;

                  /* Gets camera depth */
                  fCameraDepth = stFrustum.vBR.fZ - vCameraPosition.fZ;

                  /* Near space? */
                  if(fObjectRelativeDepth < (orx2F(0.5f) * fCameraDepth))
                  {
                    /* Gets depth scale */
                    fDepthScale = (orx2F(0.5f) * fCameraDepth) / fObjectRelativeDepth;
                  }
                  /* Far space */
                  else
                  {
                    /* Gets depth scale */
                    fDepthScale = (fCameraDepth - fObjectRelativeDepth) / (orx2F(0.5f) * fCameraDepth);
                  }

                  /* Updates object scales */
                  vObjectScale.fX *= fDepthScale;
                  vObjectScale.fY *= fDepthScale;
                }

                /* Has camera rotation? */
                if(fRenderRotation != orxFLOAT_0)
                {
                  /* Rotates it */
                  orxVector_2DRotate(&vRenderPos, &vRenderPos, -fRenderRotation);
                }

                /* Gets position in screen space */
                orxVector_Add(&vRenderPos, &vRenderPos, &vViewportCenter);
                orxVector_Add(&vRenderPos, &vRenderPos, &(stTextureBox.vTL));

                /* Updates render frame */
                orxFrame_SetPosition(pstRenderFrame, &vRenderPos);
                orxFrame_SetRotation(pstRenderFrame, fObjectRotation - fRenderRotation);
                vObjectScale.fX *= fRenderScaleX;
                vObjectScale.fY *= fRenderScaleY;
                orxFrame_SetScale(pstRenderFrame, &vObjectScale);

                /* Renders it */
                if(orxRender_RenderObject(pstObject, pstBitmap, pstRenderFrame) != orxSTATUS_FAILURE)
                {
                  /* Updates its render status */
                  orxObject_SetRendered(pstObject, orxTRUE);
                }
                else
                {
                  /* Prints error message */
                  orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "[orxOBJECT %p -> orxBITMAP %p] couldn't be rendered.", pstObject, pstBitmap);
                }
              }

              /* Cleans rendering bank */
              orxBank_Clear(sstRender.pstRenderBank);

              /* Cleans rendering list */
              orxMemory_Zero(&(sstRender.stRenderList), sizeof(orxLINKLIST));

              /* Deletes rendering frame */
              orxFrame_Delete(pstRenderFrame);
            }
            else
            {
              /* Logs message */
              orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Camera size out of bounds: %g, %g.", fCameraWidth, fCameraHeight);
            }
          }
          else
          {
            /* Logs message */
            orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Viewport intersects with texture.");
          }
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Could not create rendering frame.");
        }
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Not a valid camera.");
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Not a valid bitmap.");
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Viewport is not enabled.");
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
  for(pstViewport = orxVIEWPORT(orxStructure_GetLast(orxSTRUCTURE_ID_VIEWPORT));
      pstViewport != orxNULL;
      pstViewport = orxVIEWPORT(orxStructure_GetPrevious(pstViewport)))
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
    orxMemory_Zero(&stTextTransform, sizeof(orxBITMAP_TRANSFORM));

    /* Inits it */
    stTextTransform.fScaleX = stTextTransform.fScaleY = orx2F(0.8f);
    stTextTransform.fDstX = stTextTransform.fDstY = orx2F(10.0f);

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
 * @return orxVECTOR / orxNULL
 */
orxVECTOR *orxRender_Home_GetWorldPosition(orxCONST orxVECTOR *_pvScreenPosition, orxVECTOR *_pvWorldPosition)
{
  orxVIEWPORT  *pstViewport;
  orxVECTOR    *pvResult = orxNULL;

  /* Checks */
  orxASSERT(sstRender.u32Flags & orxRENDER_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvScreenPosition != orxNULL);
  orxASSERT(_pvWorldPosition != orxNULL);

  /* For all viewports */
  for(pstViewport = orxVIEWPORT(orxStructure_GetFirst(orxSTRUCTURE_ID_VIEWPORT));
      pstViewport != orxNULL;
      pstViewport = orxVIEWPORT(orxStructure_GetNext(pstViewport)))
  {
    orxCAMERA *pstCamera;

    /* Is active and has camera? */
    if((orxViewport_IsEnabled(pstViewport) != orxFALSE)
    && ((pstCamera = orxViewport_GetCamera(pstViewport)) != orxNULL))
    {
      orxAABOX  stViewportBox;
      orxFLOAT  fCorrectionRatio;

      /* Gets viewport box */
      orxViewport_GetBox(pstViewport, &stViewportBox);

      /* Gets viewport correction ratio */
      fCorrectionRatio = orxViewport_GetCorrectionRatio(pstViewport);
      
      /* Has one? */
      if(fCorrectionRatio != orxFLOAT_1)
      {
        orxFLOAT fDelta;

        /* Gets rendering limit delta using correction ratio */
        fDelta = orx2F(0.5f) * (orxFLOAT_1 - fCorrectionRatio) * (stViewportBox.vBR.fX - stViewportBox.vTL.fX);

        /* Updates viewport */
        stViewportBox.vTL.fX += fDelta;
        stViewportBox.vBR.fX -= fDelta;
      }

      /* Is position in box? */
      if((_pvScreenPosition->fX >= stViewportBox.vTL.fX)
      && (_pvScreenPosition->fX <= stViewportBox.vBR.fX)
      && (_pvScreenPosition->fY >= stViewportBox.vTL.fY)
      && (_pvScreenPosition->fY <= stViewportBox.vBR.fY))
      {
        orxVECTOR vLocalPosition, vCenter, vCameraCenter;
        orxAABOX  stCameraFrustum;
        orxFLOAT  fZoom, fRotation;

        /* Gets viewport center */
        orxVector_Mulf(&vCenter, orxVector_Add(&vCenter, &(stViewportBox.vBR), &(stViewportBox.vTL)), orx2F(0.5f));

        /* Gets camera frustum */
        orxCamera_GetFrustum(pstCamera, &stCameraFrustum);

        /* Gets camera position */
        orxVector_Mulf(&vCameraCenter, orxVector_Add(&vCameraCenter, &(stCameraFrustum.vBR), &(stCameraFrustum.vTL)), orx2F(0.5f));

        /* Gets viewport space normalized position */
        orxVector_Set(&vLocalPosition, (_pvScreenPosition->fX - vCenter.fX) / (stViewportBox.vBR.fX - stViewportBox.vTL.fX), (_pvScreenPosition->fY - vCenter.fY) / (stViewportBox.vBR.fY - stViewportBox.vTL.fY), orxFLOAT_0);

        /* Gets camera zoom */
        fZoom = orxCamera_GetZoom(pstCamera);

        /* Has rotation */
        if((fRotation = orxCamera_GetRotation(pstCamera)) != orxFLOAT_0)
        {
          orxFLOAT fCos, fSin;

          /* Gets values in camera space */
          vLocalPosition.fX *= (stCameraFrustum.vBR.fX - stCameraFrustum.vTL.fX);
          vLocalPosition.fY *= (stCameraFrustum.vBR.fY - stCameraFrustum.vTL.fY);

          /* Gets cosine and sine of the camera angle */
          fCos = orxMath_Cos(-fRotation);
          fSin = orxMath_Sin(-fRotation);

          /* Gets its world coordinates */
          orxVector_Set(_pvWorldPosition, (vCameraCenter.fX * fZoom) + (fCos * vLocalPosition.fX) + (fSin * vLocalPosition.fY), (vCameraCenter.fY * fZoom) + (-fSin * vLocalPosition.fX) + (fCos * vLocalPosition.fY), stCameraFrustum.vTL.fZ);
        }
        else
        {
          /* Gets its world coordinates */
          orxVector_Set(_pvWorldPosition, vCameraCenter.fX * fZoom + vLocalPosition.fX * (stCameraFrustum.vBR.fX - stCameraFrustum.vTL.fX), vCameraCenter.fY * fZoom + vLocalPosition.fY * (stCameraFrustum.vBR.fY - stCameraFrustum.vTL.fY), stCameraFrustum.vTL.fZ);
        }

        /* Has zoom? */
        if((fZoom = orxCamera_GetZoom(pstCamera)) != orxFLOAT_1)
        {
          orxFLOAT fRecZoom;

          /* Gets reciprocal zoom */
          fRecZoom = orxFLOAT_1 / fZoom;

          /* Updates result */
          _pvWorldPosition->fX *= fRecZoom;
          _pvWorldPosition->fY *= fRecZoom;
        }

        /* Updates result */
        pvResult = _pvWorldPosition;

        break;
      }
    }
  }

  /* Done! */
  return pvResult;
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
    orxMemory_Zero(&sstRender, sizeof(orxRENDER_STATIC));

    /* Creates rendering bank */
    sstRender.pstRenderBank = orxBank_Create(orxRENDER_KU32_ORDER_BANK_SIZE, sizeof(orxRENDER_NODE), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if(sstRender.pstRenderBank != orxNULL)
    {
      /* Creates rendering clock */
      sstRender.pstClock = orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE);
      orxClock_SetModifier(sstRender.pstClock, orxCLOCK_MOD_TYPE_MAXED, orxConfig_HasValue(orxRENDER_KZ_CONFIG_MAX_TICK_SIZE) ? orxConfig_GetFloat(orxRENDER_KZ_CONFIG_MAX_TICK_SIZE) : orxRENDER_KF_TICK_SIZE);

      /* Valid? */
      if(sstRender.pstClock != orxNULL)
      {
        /* Registers rendering function */
        eResult = orxClock_Register(sstRender.pstClock, orxRender_RenderAll, orxNULL, orxMODULE_ID_RENDER, orxCLOCK_FUNCTION_PRIORITY_HIGH);
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
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Render module already loaded.");

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
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Failed to register Render module.");
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

    /* Deletes rendering bank */
    orxBank_Delete(sstRender.pstRenderBank);

    /* Updates flags */
    sstRender.u32Flags &= ~orxRENDER_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Attempting to exit from Render module without initializing it.");
  }

  return;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(RENDER);

orxPLUGIN_USER_CORE_FUNCTION_ADD(orxRender_Home_Init, RENDER, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxRender_Home_Exit, RENDER, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxRender_Home_GetWorldPosition, RENDER, GET_WORLD_POSITION);

orxPLUGIN_USER_CORE_FUNCTION_END();
