/***************************************************************************
 orxCamera.c
 Camera module
 
 begin                : 10/12/2003
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


#include "camera/orxCamera.h"

#include "anim/orxAnimPointer.h"
#include "core/orxTime.h"
#include "debug/orxDebug.h"
#include "display/orxGraphic.h"
#include "math/orxMath.h"
#include "memory/orxMemory.h"


/*
 * Platform independant defines
 */

#define orxCAMERA_KU32_FLAG_NONE          0x00000000
#define orxCAMERA_KU32_FLAG_READY         0x00000001
#define orxCAMERA_KU32_FLAG_DATA_2D       0x00000010
#define orxCAMERA_KU32_FLAG_DEFAULT       0x00000010

#define orxCAMERA_KU32_ID_FLAG_NONE       0x00000000
#define orxCAMERA_KU32_ID_FLAG_MOVED      0x00010000
#define orxCAMERA_KU32_ID_FLAG_LINKED     0x00100000
#define orxCAMERA_KU32_ID_FLAG_LIMITED    0x00200000

#define orxCAMERA_KU32_VIEW_LIST_NUMBER   128


/*
 * View list structure
 */
struct __orxCAMERA_VIEW_LIST_t
{
  /* Z sort value : 4 */
  orxFLOAT fZSort;

  /* Internal screen frame pointer : 8 */
  orxFRAME *pstScreenFrame;

  /* External object pointer : 12 */
  orxOBJECT *pstObject;

  /* List handling pointer : 20 */
  struct __orxCAMERA_VIEW_LIST_t *pstPrevious, *pstNext;

  /* Used / Not used : 24 */
  orxBOOL bUsed;
};

/*
 * Internal 2D Camera Data Structure
 */
typedef struct __orxCAMERA_DATA_2D_t
{
  /* Clip corners coords : 32 */
  orxVECTOR vClipUL, vClipBR;

  /* Limit corners coords : 64 */
  orxVECTOR vLimitUL, vLimitBR;

  /* Size coord : 80 */
  orxVECTOR vSize;

} orxCAMERA_DATA_2D;


/*
 * Camera structure
 */
struct __orxCAMERA_t
{
  /* Public structure, first structure member : 16 */
  orxSTRUCTURE stStructure;

  /* Internal id flags : 20 */
  orxU32 u32IDFlags;

  /* Frame : 24 */
  orxFRAME *pstFrame;

  /* Linked object : 28 */
  orxOBJECT *pstLink;

  /* Data : 32 */
  orxVOID *pstData;

  /* On screen position coord : 48 */
  orxVECTOR vOnScreenPosition;

   /* View list current pointer : 52 */
  orxCAMERA_VIEW_LIST *pstViewListCurrent;

  /* View list first pointer : 56 */
  orxCAMERA_VIEW_LIST *pstViewListFirst;

  /* View list counter : 60 */
  orxS32 s32ViewListCounter;

  /* View list : 3132 */
  orxCAMERA_VIEW_LIST astViewList[orxCAMERA_KU32_VIEW_LIST_NUMBER];

  orxPAD(3132)
};


/*
 * Static structure
 */
typedef struct __orxCAMERA_STATIC_t
{
  /* Control flags */
  orxU32 u32Flags;

} orxCAMERA_STATIC;


/*
 * Static data
 */
orxSTATIC orxCAMERA_STATIC sstCamera;


/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 orxCamera_DeleteAll
 Deletes all cameras.

 returns: orxVOID
 ***************************************************************************/
orxSTATIC orxVOID orxCamera_DeleteAll()
{
  orxCAMERA *pstCamera;
  
  /* Gets first camera */
  pstCamera = (orxCAMERA *)orxStructure_GetFirst(orxSTRUCTURE_ID_CAMERA);

  /* Non null? */
  while(pstCamera != orxNULL)
  {
    /* Deletes object */
    orxCamera_Delete(pstCamera);

    /* Gets first object */
    pstCamera = (orxCAMERA *)orxStructure_GetFirst(orxSTRUCTURE_ID_CAMERA);
  }

  return;
}

/***************************************************************************
 orxCamera_CleanViewListCell
 Creates a camera view list cell.

 returns: orxVOID
 ***************************************************************************/
orxSTATIC orxINLINE orxVOID orxCamera_CleanViewListCell(orxCAMERA_VIEW_LIST *_pstViewList)
{
  /* Checks */
  orxASSERT(_pstViewList != orxNULL);
  
  /* Cleans cell */
  _pstViewList->fZSort      = 0.0f;
  _pstViewList->pstObject   = orxNULL;
  _pstViewList->pstPrevious = orxNULL;
  _pstViewList->pstNext     = orxNULL;
  _pstViewList->bUsed       = orxFALSE;

  return;
}

/***************************************************************************
 orxCamera_CleanViewList
 Cleans a camera view list.

 returns: orxVOID
 ***************************************************************************/
orxSTATIC orxINLINE orxVOID orxCamera_CleanViewList(orxCAMERA *_pstCamera)
{
  orxU32 i;

  /* Checks */
  orxASSERT(_pstCamera != orxNULL);

  /* Cleans view list */
  for(i = 0; i < orxCAMERA_KU32_VIEW_LIST_NUMBER; i++)
  {
    orxCamera_CleanViewListCell(&(_pstCamera->astViewList[i]));
  }

  /* Cleans data */
  _pstCamera->s32ViewListCounter  = 0;
  _pstCamera->pstViewListFirst    = orxNULL;
  _pstCamera->pstViewListCurrent  = orxNULL;

  return;
}

/***************************************************************************
 orxCamera_CreateViewList
 Creates a camera view list.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 ***************************************************************************/
orxSTATIC orxSTATUS orxCamera_CreateViewList(orxCAMERA *_pstCamera)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;
  orxU32 i;

  /* Checks */
  orxASSERT(_pstCamera != orxNULL);

  /* For all view list structures */
  for(i = 0; i < orxCAMERA_KU32_VIEW_LIST_NUMBER; i++)
  {
    /* Creates screen frame */
    (_pstCamera->astViewList[i]).pstScreenFrame = orxFrame_Create();

    /* Failed? */
    if((_pstCamera->astViewList[i]).pstScreenFrame == orxNULL)
    {
      orxU32 j;

      /* Frees all previously created frames */
      for(j = 0; j < i; j++)
      {
        orxFrame_Delete((_pstCamera->astViewList[j]).pstScreenFrame);
      }
 
      /* Updates result */
      eResult = orxSTATUS_FAILURE;
      break;
    }
    else
    {
      /* Cleans view list cell */
      orxCamera_CleanViewListCell(&(_pstCamera->astViewList[i]));
    }
  }

  /* Cleans view list */
  orxCamera_CleanViewList(_pstCamera);

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxCamera_DeleteViewList
 Deletes a camera view list.

 returns: orxVOID
 ***************************************************************************/
orxSTATIC orxVOID orxCamera_DeleteViewList(orxCAMERA *_pstCamera)
{
  orxU32 i;

  /* Checks */
  orxASSERT(_pstCamera != orxNULL);

  /* For all view list structures */
  for(i = 0; i < orxCAMERA_KU32_VIEW_LIST_NUMBER; i++)
  {
    /* Deletes screen frame */
    orxFrame_Delete(_pstCamera->astViewList[i].pstScreenFrame);
  }

  /* Cleans view list */
  orxCamera_CleanViewList(_pstCamera);

  return;
}

/***************************************************************************
 orxCamera_UpdatePosition
 Computes camera position using linked object & limits.

 returns: orxVOID
 ***************************************************************************/
orxSTATIC orxVOID orxCamera_UpdatePosition(orxCAMERA *_pstCamera, orxBOOL _bForce)
{
  orxVECTOR vPos;
  orxVECTOR *pvUL, *pvBR;

  /* Checks */
  orxASSERT(_pstCamera != orxNULL);

  /* Is camera linked? */
  if(_pstCamera->u32IDFlags & orxCAMERA_KU32_ID_FLAG_LINKED)
  {
    /* Gets linked object frame */
    orxFRAME *pstFrame = (orxFRAME *)orxObject_GetStructure(_pstCamera->pstLink, orxSTRUCTURE_ID_FRAME);

    /* Non null? */
    if(pstFrame != orxNULL)
    {
      /* Is frame render dirty or update forced? */
      if((orxFrame_IsRenderStatusClean(pstFrame) == orxFALSE) || (_bForce != orxFALSE))
      {
        /* 2D? */
        if(orxCamera_TestFlags(_pstCamera, orxCAMERA_KU32_ID_FLAG_2D) != orxFALSE)
        {
          /* Gets linked object positions */
          orxFrame_GetPosition(pstFrame, &vPos, orxFALSE);

          /* Has camera limits? */
          if(_pstCamera->u32IDFlags & orxCAMERA_KU32_ID_FLAG_LIMITED)
          {
            /* Gets limit coords */
            pvUL = &(((orxCAMERA_DATA_2D *)(_pstCamera->pstData))->vLimitUL);
            pvBR = &(((orxCAMERA_DATA_2D *)(_pstCamera->pstData))->vLimitBR);

            /* Clamps camera position according to limits */
            vPos.fX = orxCLAMP(vPos.fX, pvUL->fX, pvBR->fX);
            vPos.fY = orxCLAMP(vPos.fY, pvUL->fY, pvBR->fY);
            vPos.fZ = orxCLAMP(vPos.fZ, pvUL->fZ, pvBR->fZ);
          }

          /* Sets camera position */
          orxFrame_SetPosition(_pstCamera->pstFrame, &vPos);

          /* Updates camera flags */
          _pstCamera->u32IDFlags |= orxCAMERA_KU32_ID_FLAG_MOVED;
        }
      }
    }
    else
    {
      /* Has camera moved? */
      if(_pstCamera->u32IDFlags & orxCAMERA_KU32_ID_FLAG_MOVED)
      {
        /* Has camera limits? */
        if(_pstCamera->u32IDFlags & orxCAMERA_KU32_ID_FLAG_LIMITED)
        {
          /* Gets camera position */
          orxFrame_GetPosition(_pstCamera->pstFrame, &vPos, orxTRUE);

          /* Gets limit coords pointers */
          pvUL = &(((orxCAMERA_DATA_2D *)(_pstCamera->pstData))->vLimitUL);
          pvBR = &(((orxCAMERA_DATA_2D *)(_pstCamera->pstData))->vLimitBR);

          /* Clamps camera position according to limits */
          vPos.fX = orxCLAMP(vPos.fX, pvUL->fX, pvBR->fX);
          vPos.fY = orxCLAMP(vPos.fY, pvUL->fY, pvBR->fY);
          vPos.fZ = orxCLAMP(vPos.fZ, pvUL->fZ, pvBR->fZ);

          /* Sets camera position */
          orxFrame_SetPosition(_pstCamera->pstFrame, &vPos);
        }
      }
    }
  }

  return;
}

/***************************************************************************
 orxCamera_ComputeClipCorners
 Computes camera clip corner coordinates.

 returns: orxVOID
 ***************************************************************************/
orxSTATIC orxVOID orxCamera_ComputeClipCorners(orxCAMERA *_pstCamera)
{
  orxVECTOR vPos;
  orxVECTOR *pvUL, *pvBR, *pvSize;
  orxFLOAT fRot, fScale;

  /* Checks */
  orxASSERT(_pstCamera != orxNULL);

  /* Updates camera position using limits & link options */
  orxCamera_UpdatePosition(_pstCamera, orxFALSE);

  /* Has camera moved? */
  if(_pstCamera->u32IDFlags & orxCAMERA_KU32_ID_FLAG_MOVED)
  {
    /* 2D? */
    if(orxCamera_TestFlags(_pstCamera, orxCAMERA_KU32_ID_FLAG_2D) != orxFALSE)
    {
      /* Gets coords pointers */
      pvUL    = &(((orxCAMERA_DATA_2D *)(_pstCamera->pstData))->vClipUL);
      pvBR    = &(((orxCAMERA_DATA_2D *)(_pstCamera->pstData))->vClipBR);
      pvSize  = &(((orxCAMERA_DATA_2D *)(_pstCamera->pstData))->vSize);

      /* Gets camera info */
      orxFrame_GetPosition(_pstCamera->pstFrame, &vPos, orxFALSE);
      fRot    = orxFrame_GetRotation(_pstCamera->pstFrame, orxFALSE);
      fScale  = orxFrame_GetScale(_pstCamera->pstFrame, orxFALSE);

      /* Computes relative camera upper left & bottom right corners */
      orxVector_Mul(pvBR, pvSize, 0.5f);
      orxVector_Add(pvUL, pvBR, orxVector_Neg(pvSize, pvSize));

      /* Applies scale & rotation if needed */
      if(fRot != orxFLOAT_0)
      {
        orxFLOAT fMax;

        /* We want only axis aligned box for clipping */
        /* We thus compute axis aligned smallest box that contains real rotated camera box */

        /* Rotates one corner */
        orxVector_Rot(pvUL, pvUL, &orxVECTOR_Z, fRot);

        /* Gets corner maximum absolute value between X & Y values */
        if(orxFABS(pvUL->fX) > orxFABS(pvUL->fY))
        {
          fMax = orxFABS(pvUL->fX) + orxFLOAT_1;
        }
        else
        {
          fMax = orxFABS(pvUL->fY) + orxFLOAT_1;
        }

        /* Applies it to both corners */
        pvUL->fX = -fMax;
        pvUL->fY = -fMax;
        pvBR->fX = fMax;
        pvBR->fY = fMax;
      }
      
      /* Non neutral scale? */
      if(fScale != orxFLOAT_1)
      {
        /* Applies it */
        orxVector_Mul(pvUL, pvUL, fScale);
        orxVector_Mul(pvBR, pvBR, fScale);
      }

      /* Computes global corners */
      orxVector_Add(pvUL, pvUL, &vPos);
      orxVector_Add(pvBR, pvBR, &vPos);
      orxVector_ReorderAABox(pvUL, pvBR);
    }
  }

  return;
}

/***************************************************************************
 orxCamera_RemoveViewListCell
 Removes a cell from the sorted camera view list.

 returns: orxVOID
 ***************************************************************************/
orxSTATIC orxVOID orxCamera_RemoveViewListCell(orxCAMERA *_pstCamera, orxCAMERA_VIEW_LIST *_pstCell)
{
  /* Checks */
  orxASSERT(_pstCamera != orxNULL);
  orxASSERT(_pstCell != orxNULL);

  /* First cell? */
  if(_pstCell->pstPrevious == orxNULL)
  {
    /* Second cell will become first one */
    _pstCamera->pstViewListFirst = _pstCell->pstNext;
  }

  /* Updates neighbours */
  if(_pstCell->pstPrevious != orxNULL)
  {
    (_pstCell->pstPrevious)->pstNext = _pstCell->pstNext;
  }
  if(_pstCell->pstNext != orxNULL)
  {
    (_pstCell->pstNext)->pstPrevious = _pstCell->pstPrevious;
  }

  /* Cleans pointers */
  _pstCell->pstNext     = orxNULL;
  _pstCell->pstPrevious = orxNULL;

  /* Decreases counter */
  _pstCamera->s32ViewListCounter--;

  return;
}

/***************************************************************************
 orxCamera_InsertViewListCell
 Inserts a cell in the sorted camera view list, using frame.z coordinate as sort value.

 returns: orxVOID
 ***************************************************************************/
orxSTATIC orxVOID orxCamera_InsertViewListCell(orxCAMERA *_pstCamera, orxCAMERA_VIEW_LIST* _pstCell)
{
  orxCAMERA_VIEW_LIST *pst_current, *pstPrevious = orxNULL;

  /* Checks */
  orxASSERT(_pstCamera != orxNULL);
  orxASSERT(_pstCell != orxNULL);

  /* Is list empty? */
  if(_pstCamera->pstViewListFirst == orxNULL)
  {
    /* Becomes first cell */
    _pstCamera->pstViewListFirst = _pstCell;

    /* Updates cell pointers */
    _pstCell->pstPrevious = orxNULL;
    _pstCell->pstNext     = orxNULL;
  }
  /* Not empty */
  else
  {
    /* Finds correct index using Z sort value */
    pst_current = _pstCamera->pstViewListFirst;
    while((pst_current != orxNULL) && (_pstCell->fZSort < pst_current->fZSort))
    {
      pstPrevious = pst_current;
      pst_current = pst_current->pstNext;
    }

    /* Updates cell */
    _pstCell->pstNext     = pst_current;
    _pstCell->pstPrevious = pstPrevious;

    /* Not last cell */
    if(pst_current != orxNULL)
    {
      pst_current->pstPrevious = _pstCell;
    }

    /* Not first cell */
    if(pstPrevious != orxNULL)
    {
      pstPrevious->pstNext = _pstCell;
    }
    else
    {
      /* Updates list first cell */
      _pstCamera->pstViewListFirst = _pstCell;
    }
  }

  /* Increases counter */
  _pstCamera->s32ViewListCounter++;

  return;
}

/***************************************************************************
 orxCamera_SearchViewList
 Search view list for a cell corresponding to the given object.

 returns: object cell/orxNULL
 ***************************************************************************/
orxSTATIC orxINLINE orxCAMERA_VIEW_LIST *orxCamera_SearchViewList(orxCAMERA *_pstCamera, orxOBJECT *_pstObject)
{
  orxREGISTER orxCAMERA_VIEW_LIST *pstCell = orxNULL;

  /* Checks */
  orxASSERT(_pstCamera != orxNULL);
  orxASSERT(_pstObject != orxNULL);

  /* Find corresponding cell */
  for(pstCell = _pstCamera->pstViewListFirst;
      (pstCell != orxNULL) && (pstCell->pstObject != _pstObject);
      pstCell = pstCell->pstNext);

  /* Done! */
  return pstCell;
}

/***************************************************************************
 orxCamera_FindFreeViewListCell
 Search view list for a free cell.

 returns: object cell/orxNULL
 ***************************************************************************/
orxSTATIC orxINLINE orxCAMERA_VIEW_LIST *orxCamera_FindFreeViewListCell(orxCAMERA *_pstCamera)
{
  orxREGISTER orxCAMERA_VIEW_LIST *pstCell = orxNULL;
  orxU32 i;

  /* Checks */
  orxASSERT(_pstCamera != orxNULL);

  /* Find free cell */
  for(i = 0;
      (i < orxCAMERA_KU32_VIEW_LIST_NUMBER) && ((_pstCamera->astViewList[i]).bUsed != orxFALSE);
      i++);

  /* Gets cell if found */
  if(i < orxCAMERA_KU32_VIEW_LIST_NUMBER)
  {
    pstCell = &(_pstCamera->astViewList[i]);
  }

  /* Done! */
  return pstCell;
}

/***************************************************************************
 orxCamera_ComputeObject
 Test object/camera intersection.
 If succesfull :
 - computes camera transformed screen frame,
 - stores object pointer in view list,
 - add view_list cell to sorted list.

 If not :
 - removes view_list cell from sorted list.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILURE (list full)
 ***************************************************************************/
orxSTATIC orxSTATUS orxCamera_ComputeObject(orxCAMERA *_pstCamera, orxOBJECT *_pstObject)
{
  orxFRAME             *pstFrame;
  orxGRAPHIC           *pstGraphic;
  orxCAMERA_VIEW_LIST  *pstCell = orxNULL;
  orxVECTOR            *pvCamUL, *pvCamBR, *pvCamSize, vCamPos;
  orxVECTOR             vTextureUL, vTextureBR, vGraphicPivot, vTexturePos, vTemp;
  orxFLOAT              fCamRot, fCamScale, fTextureRot, fTextureScale;
  orxVECTOR             vScroll;
  orxSTATUS             eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(_pstCamera != orxNULL);
  orxASSERT(_pstObject != orxNULL);

  /* Has graphic? */
  pstGraphic = (orxGRAPHIC *)orxObject_GetStructure(_pstObject, orxSTRUCTURE_ID_GRAPHIC);
  
  /* Valid? */
  if(pstGraphic != orxNULL)
  {
    /* 2D? */
    if(orxGraphic_TestFlags(pstGraphic, orxGRAPHIC_KU32_ID_FLAG_2D) != orxFALSE)
    {
      orxTEXTURE *pstTexture;
      
      /* Gets texture */
      pstTexture = (orxTEXTURE *)orxGraphic_GetData(pstGraphic);
      
      /* Valid? */
      if(pstTexture != orxNULL)
      {
        /* Has frame? */
        pstFrame = (orxFRAME *)orxObject_GetStructure(_pstObject, orxSTRUCTURE_ID_FRAME);
        
        /* Valid? */
        if(pstFrame != orxNULL)
        {
          /* Gets camera clip corners pointers */
          pvCamUL = &(((orxCAMERA_DATA_2D *)(_pstCamera->pstData))->vClipUL);
          pvCamBR = &(((orxCAMERA_DATA_2D *)(_pstCamera->pstData))->vClipBR);
  
          /* Gets texture infos */
          orxFrame_GetPosition(pstFrame, &vTexturePos, orxFALSE);
          fTextureRot   = orxFrame_GetRotation(pstFrame, orxFALSE);
          fTextureScale = orxFrame_GetScale(pstFrame, orxFALSE);
  
          /* Computes texture global corners */
          orxGraphic_GetPivot(pstGraphic, &vGraphicPivot);
          orxVector_Add(&vTextureUL, &vTexturePos, orxVector_Neg(&vTemp, &vGraphicPivot));
  
          orxTexture_GetSize(pstTexture, &(vGraphicPivot.fX), &(vGraphicPivot.fY));
          orxVector_Add(&vTextureBR, &vTextureUL, &vGraphicPivot);
  
          /* Intersection? */
          if(orxVector_TestAABoxIntersection(pvCamUL, pvCamBR, &vTextureUL, &vTextureBR) != orxFALSE)
          {
            /* Search for object cell in list*/
            pstCell = orxCamera_SearchViewList(_pstCamera, _pstObject);
  
            /* Already in list */
            if(pstCell != orxNULL)
            {
              /* Removes it from list */
              orxCamera_RemoveViewListCell(_pstCamera, pstCell);
  
              /* Cleans view list cell */
              orxCamera_CleanViewListCell(pstCell);
            }
            else
            {
              /* Find first free cell */
              pstCell = orxCamera_FindFreeViewListCell(_pstCamera);
  
              /* No cell left? */
              if(pstCell == orxNULL)
              {
                /* Updates result */
                eResult = orxSTATUS_FAILURE;
              }
            }
  
            /* Success? */
            if(eResult == orxSTATUS_SUCCESS)
            {
              /* Stores the object */
              pstCell->pstObject = _pstObject;
  
              /* Gets camera infos */
              orxFrame_GetPosition(_pstCamera->pstFrame, &vCamPos, orxFALSE);
              pvCamSize = &(((orxCAMERA_DATA_2D *)(_pstCamera->pstData))->vSize);
              fCamRot   = orxFrame_GetRotation(_pstCamera->pstFrame, orxFALSE);
              fCamScale = orxFrame_GetScale(_pstCamera->pstFrame, orxFALSE);
  
              /* Computes texture screen frame, using viewport coordinates */
  
              /* Gets into camera space */
              orxVector_Add(&vTextureUL, &vTextureUL, orxVector_Neg(&vTemp, &vCamPos));
  
              /* Applies rotation & scale if needed */
              if(fCamRot != orxFLOAT_0)
              {
                orxVector_Rot(&vTextureUL, &vTextureUL, &orxVECTOR_Z, -fCamRot);
              }
              if(fCamScale != orxFLOAT_1)
              {
                orxVector_Mul(&vTextureUL, &vTextureUL, orxFLOAT_1 / fCamScale);
              }
  
              /* Uses differential scrolling? */
              if(orxFrame_HasDifferentialScrolling(pstFrame) != orxFALSE)
              {
                /* Gets scrolling coefficients */
                orxFrame_GetDifferentialScrolling(pstFrame, &vScroll);
  
                /* X axis scrolling? */
                if(vScroll.fX != orxFLOAT_0)
                {
                  vScroll.fX *= vTextureUL.fX;
                }
  
                /* Y axis scrolling? */
                if(vScroll.fY != orxFLOAT_0)
                {
                  vScroll.fY *= vTextureUL.fY;
                }
  
                /* Updates texture coordinates */
                orxVector_Set3(&vTextureUL, (orxFLOAT)floor(vScroll.fX), (orxFLOAT)floor(vScroll.fY), vTextureUL.fZ); /* MSVC doesn't recognize floorf for x86) */
              }
  
              /* Gets into viewport coordinates */
              orxVector_Mul(&vCamPos, pvCamSize, 0.5);
              orxVector_Add(&vTextureUL, &vTextureUL, &vCamPos);
  
              /* Gets into screen coordinates */
              orxVector_Add(&vTextureUL, &vTextureUL, &(_pstCamera->vOnScreenPosition));
  
              /* Stores screen coordinates */
              pstFrame = pstCell->pstScreenFrame;
              orxFrame_SetPosition(pstFrame, &vTextureUL);
              orxFrame_SetRotation(pstFrame, fTextureRot - fCamRot);
              orxFrame_SetScale(pstFrame, fTextureScale / fCamScale);
  
              /* Updates view list sort value */
              pstCell->fZSort = vTextureUL.fZ;
  
              /* Updates view list used status */
              pstCell->bUsed = orxTRUE;
  
              /* Insert it into sorted list */
              orxCamera_InsertViewListCell(_pstCamera, pstCell);
  
              /* !!! TODO : Updates graphic status !!! */
    //          graphic_flag_set(pstGraphic, orxGRAPHIC_KU32_ID_FLAG_RENDERED, orxGRAPHIC_KU32_ID_FLAG_NONE);
            }
            else
            {
              /* Search for object position in list*/
              pstCell = orxCamera_SearchViewList(_pstCamera, _pstObject);
  
              /* Already in list */
              if(pstCell != orxNULL)
              {
                /* Removes it from list */
                orxCamera_RemoveViewListCell(_pstCamera, pstCell);
  
                /* Cleans view list cell */
                orxCamera_CleanViewListCell(pstCell);
              }
  
              /* !!! TODO : Updates graphic status !!! */
    //          graphic_flag_set(pstGraphic, orxGRAPHIC_KU32_ID_FLAG_NONE, orxGRAPHIC_KU32_ID_FLAG_RENDERED);
            }
          }
        }
      }
    }
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxCamera_SortViewList
 Sorts camera view list.

 returns: orxVOID
 ***************************************************************************/
orxSTATIC orxINLINE orxVOID orxCamera_SortViewList(orxCAMERA *_pstCamera)
{
  orxU32 i;

  /* Checks */
  orxASSERT(_pstCamera != orxNULL);

  /* Cleans sorted list */
  _pstCamera->pstViewListFirst    = orxNULL;
  _pstCamera->pstViewListCurrent  = orxNULL;

  /* For all view list cells */
  for(i = 0; i < orxCAMERA_KU32_VIEW_LIST_NUMBER; i++)
  {
    /* Used? */
    if((_pstCamera->astViewList[i]).bUsed != orxFALSE)
    {
      /* Insert it into the list */
      orxCamera_InsertViewListCell(_pstCamera, &(_pstCamera->astViewList[i]));
    }
  }

  return;
}


/***************************************************************************
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 orxCamera_Setup
 Camera system setup.

 returns: nothing
 ***************************************************************************/
orxVOID orxCamera_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_CAMERA, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_CAMERA, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_CAMERA, orxMODULE_ID_FRAME);
  orxModule_AddDependency(orxMODULE_ID_CAMERA, orxMODULE_ID_OBJECT);

  return;
}

/***************************************************************************
 orxCamera_Init
 Inits camera system.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 ***************************************************************************/
orxSTATUS orxCamera_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;
  
  /* Not already Initialized? */
  if(!(sstCamera.u32Flags & orxCAMERA_KU32_FLAG_READY))
  {
    /* Cleans control structure */
    orxMemory_Set(&sstCamera, 0, sizeof(orxCAMERA_STATIC));

    /* Registers structure type */
    eResult = orxSTRUCTURE_REGISTER(CAMERA, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxNULL);

    /* Initialized? */
    if(eResult == orxSTATUS_SUCCESS)
    {
      /* Inits Flags */
      sstCamera.u32Flags = orxCAMERA_KU32_FLAG_READY|orxCAMERA_KU32_FLAG_DEFAULT;
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
 orxCamera_Exit
 Exits from the camera system.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxCamera_Exit()
{
  /* Initialized? */
  if(sstCamera.u32Flags & orxCAMERA_KU32_FLAG_READY)
  {
    /* Deletes camera list */
    orxCamera_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_CAMERA);

    /* Updates flags */
    sstCamera.u32Flags &= ~orxCAMERA_KU32_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}

/***************************************************************************
 orxCamera_Create
 Creates a new empty camera.

 returns: Created camera.
 ***************************************************************************/
orxCAMERA *orxCamera_Create()
{
  orxCAMERA *pstCamera = orxNULL;
  orxFRAME *pstFrame;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_FLAG_READY);

  /* Creates camera */
  pstCamera = (orxCAMERA *)orxStructure_Create(orxSTRUCTURE_ID_CAMERA);

  /* Created? */
  if(pstCamera != orxNULL)
  {
    /* Creates frame */
    pstFrame = orxFrame_Create();

    /* Created? */  
    if(pstFrame != orxNULL)
    {
      /* Creates camera view list */
      if(orxCamera_CreateViewList(pstCamera) == orxSTATUS_SUCCESS)
      {
        /* Inits camera members */
        pstCamera->u32IDFlags = orxCAMERA_KU32_ID_FLAG_MOVED;
        pstCamera->pstFrame   = pstFrame;
        pstCamera->pstLink    = orxNULL;
        orxVector_Set3(&(pstCamera->vOnScreenPosition), orxFLOAT_0, orxFLOAT_0, orxFLOAT_0);

        /* 2D? */
        if(sstCamera.u32Flags & orxCAMERA_KU32_FLAG_DATA_2D)
        {
          orxCAMERA_DATA_2D *pstData;

          /* Updates ID flags */
          pstCamera->u32IDFlags |= orxCAMERA_KU32_ID_FLAG_2D;

          /* Allocates data memory */
          pstData = (orxCAMERA_DATA_2D *) orxMemory_Allocate(sizeof(orxCAMERA_DATA_2D), orxMEMORY_TYPE_MAIN);
  
          /* Created? */
          if(pstData != orxNULL)
          {
            /* Cleans it */
            orxMemory_Set(pstData, 0, sizeof(orxCAMERA_DATA_2D));
            
            /* Links data to frame */
            pstCamera->pstData = pstData;

            /* Computes clip infos */
            orxCamera_ComputeClipCorners(pstCamera);
          }
          else
          {
            /* !!! MSG !!! */
  
            /* Fress partially allocated camera */
            orxCamera_DeleteViewList(pstCamera);
            orxFrame_Delete(pstFrame);
            orxStructure_Delete((orxSTRUCTURE *)pstCamera);

            /* Not created */
            pstCamera = orxNULL;
          }
        }
      }
      else
      {
        /* !!! MSG !!! */

        /* Fress partially allocated camera */
        orxFrame_Delete(pstFrame);
        orxStructure_Delete((orxSTRUCTURE *)pstCamera);

        /* Not created */
        pstCamera = orxNULL;
      }
    }
    else
    {
      /* !!! MSG !!! */

      /* Fress partially allocated camera */
      orxStructure_Delete((orxSTRUCTURE *)pstCamera);

      /* Not created */
      pstCamera = orxNULL;
    }
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return pstCamera;
}

/***************************************************************************
 orxCamera_Delete
 Deletes a camera.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 ***************************************************************************/
orxSTATUS orxCamera_Delete(orxCAMERA *_pstCamera)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_FLAG_READY);
  orxASSERT(_pstCamera != orxNULL);

  /* Not referenced? */
  if(orxStructure_GetRefCounter((orxSTRUCTURE *)_pstCamera) == 0)
  {
    /* Frees camera view list */
    orxCamera_DeleteViewList(_pstCamera);

    /* Remove linked object reference */
    if(_pstCamera->pstLink != orxNULL)
    {
      orxStructure_DecreaseCounter((orxSTRUCTURE *)(_pstCamera->pstLink));
    }

    /* Deletes frame*/
    orxFrame_Delete(_pstCamera->pstFrame);

    /* Frees data */
    orxMemory_Free(_pstCamera->pstData);

    /* Deletes structure */
    orxStructure_Delete((orxSTRUCTURE *)_pstCamera);
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
 orxCamera_UpdateViewList
 Updates camera view list.

 returns: orxVOID
 ***************************************************************************/
extern orxVOID orxCamera_UpdateViewList(orxCAMERA *_pstCamera)
{
  orxOBJECT *pstObject;
  orxFRAME *pstFrame;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_FLAG_READY);
  orxASSERT(_pstCamera != orxNULL);

  /* Computes camera corners */
  orxCamera_ComputeClipCorners(_pstCamera);

  /* If camera moved, process all objects */
  if(_pstCamera->u32IDFlags & orxCAMERA_KU32_ID_FLAG_MOVED)
  {
    /* For all objects */
    for(pstObject = (orxOBJECT *)orxStructure_GetFirst(orxSTRUCTURE_ID_OBJECT);
        pstObject != orxNULL;
        pstObject = (orxOBJECT *)orxStructure_GetNext((orxSTRUCTURE *)pstObject))
    {
      /* Computes object */
      if(orxCamera_ComputeObject(_pstCamera, pstObject) == orxSTATUS_FAILURE)
      {
        /* No room left in camera view list */
        /* !!! MSG !!! */

        break;
      }
    }
  }
  /* Process only render dirty objects */
  else
  {
    /* For all objects */
    for(pstObject = (orxOBJECT *)orxStructure_GetFirst(orxSTRUCTURE_ID_OBJECT);
        pstObject != orxNULL;
        pstObject = (orxOBJECT *)orxStructure_GetNext((orxSTRUCTURE *)pstObject))
    {
      /* Gets object frame? */
      pstFrame = (orxFRAME *)orxObject_GetStructure(pstObject, orxSTRUCTURE_ID_FRAME);

      /* Has to be processed? */
      if(orxObject_IsRenderStatusClean(pstObject) == orxFALSE)
      {
        /* Computes object */
        if(orxCamera_ComputeObject(_pstCamera, pstObject) == orxSTATUS_FAILURE)
        {
          /* No room left in camera view list */
          /* !!! MSG !!! */

          break;
        }
      }
    }
  }

  /* Removes camera moved flag */
  _pstCamera->u32IDFlags &= ~orxCAMERA_KU32_ID_FLAG_MOVED;

  return;
}


/* *** Structure accessors *** */


/***************************************************************************
 orxCamera_GetViewListFirstCell
 Gets camera first view list cell.

 returns: Requested orxCAMERA_VIEW_LIST
 ***************************************************************************/
orxCAMERA_VIEW_LIST *orxCamera_GetViewListFirstCell(orxCAMERA *_pstCamera)
{
  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_FLAG_READY);
  orxASSERT(_pstCamera != orxNULL);

  /* Stores first pointer */
  _pstCamera->pstViewListCurrent = _pstCamera->pstViewListFirst;

  /* Done */
  return _pstCamera->pstViewListCurrent;
}

/***************************************************************************
 orxCamera_GetViewListNextCell
 Gets camera next view list cell.

 returns: Requested orxCAMERA_VIEW_LIST
 ***************************************************************************/
orxCAMERA_VIEW_LIST *orxCamera_GetViewListNextCell(orxCAMERA *_pstCamera)
{
  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_FLAG_READY);
  orxASSERT(_pstCamera != orxNULL);

  /* Stores next pointer */
  if((_pstCamera->pstViewListCurrent)->pstNext != orxNULL)
  {
    _pstCamera->pstViewListCurrent = (_pstCamera->pstViewListCurrent)->pstNext;
  }
  else
  {
    /* End of list */
    /* !!! MSG !!! */

    return orxNULL;
  }

  /* Done */
  return _pstCamera->pstViewListCurrent;
}

/***************************************************************************
 orxCamera_GetViewListFrame
 Gets view list frame.

 returns: Requested frame pointer
 ***************************************************************************/
orxFRAME *orxCamera_GetViewListFrame(orxCAMERA_VIEW_LIST *_pstViewList)
{
  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_FLAG_READY);
  orxASSERT(_pstViewList != orxNULL);

  return _pstViewList->pstScreenFrame;
}

/***************************************************************************
 orxCamera_GetViewListObject
 Gets view list object.

 returns: Requested texture pointer
 ***************************************************************************/
orxOBJECT *orxCamera_GetViewListObject(orxCAMERA_VIEW_LIST *_pstViewList)
{
  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_FLAG_READY);
  orxASSERT(_pstViewList != orxNULL);

  return(_pstViewList->pstObject);
}

/***************************************************************************
 orxCamera_GetViewListSize
 Gets camera view list number.

 returns: Requested orxCAMERA_VIEW_LIST
 ***************************************************************************/
orxS32 orxCamera_GetViewListSize(orxCAMERA *_pstCamera)
{
  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_FLAG_READY);
  orxASSERT(_pstCamera != orxNULL);

  return(_pstCamera->s32ViewListCounter);
}

/***************************************************************************
 orxCamera_SetSize
 Camera 2D size set accessor.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxCamera_SetSize(orxCAMERA *_pstCamera, orxVECTOR *_pvSize)
{
  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_FLAG_READY);
  orxASSERT(_pstCamera != orxNULL);
  orxASSERT(_pvSize != orxNULL);

  /* Is camera 2D? */
  if(orxCamera_TestFlags(_pstCamera, orxCAMERA_KU32_ID_FLAG_2D) != orxFALSE)
  {
    /* Updates */
    orxVector_Copy(&(((orxCAMERA_DATA_2D *)(_pstCamera->pstData))->vSize), _pvSize);
  
    /* Updates camera flags */
    _pstCamera->u32IDFlags |= orxCAMERA_KU32_ID_FLAG_MOVED;
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}

/***************************************************************************
 orxCamera_SetPosition
 Camera 2D position set accessor.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxCamera_SetPosition(orxCAMERA *_pstCamera, orxVECTOR *_pvPosition)
{
  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_FLAG_READY);
  orxASSERT(_pstCamera != orxNULL);
  orxASSERT(_pvPosition != orxNULL);

  /* Sets camera position */
  orxFrame_SetPosition(_pstCamera->pstFrame, _pvPosition);

  /* Updates camera flags */
  _pstCamera->u32IDFlags |= orxCAMERA_KU32_ID_FLAG_MOVED;

  return;
}

/***************************************************************************
 orxCamera_SetRotation
 Camera 2D rotation set accessor.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxCamera_SetRotation(orxCAMERA *_pstCamera, orxFLOAT _fRotation)
{
  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_FLAG_READY);
  orxASSERT(_pstCamera != orxNULL);

   /* Sets camera rotation */
  orxFrame_SetRotation(_pstCamera->pstFrame, _fRotation);

  /* Updates camera flags */
  _pstCamera->u32IDFlags |= orxCAMERA_KU32_ID_FLAG_MOVED;

  return;
}

/***************************************************************************
 orxCamera_SetZoom
 Camera 2D zoom set accessor.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxCamera_SetZoom(orxCAMERA *_pstCamera, orxFLOAT _fZoom)
{
  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_FLAG_READY);
  orxASSERT(_pstCamera != orxNULL);

   /* Sets camera zoom */
  orxFrame_SetScale(_pstCamera->pstFrame, orxFLOAT_1 / _fZoom);

  /* Updates camera flags */
  _pstCamera->u32IDFlags |= orxCAMERA_KU32_ID_FLAG_MOVED;

  return;
}

/***************************************************************************
 orxCamera_SetTarget
 Camera zoom set accessor.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxCamera_SetTarget(orxCAMERA *_pstCamera, orxOBJECT *_pstObject)
{
  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_FLAG_READY);
  orxASSERT(_pstCamera != orxNULL);

  /* Has already a linked object */
  if(_pstCamera->pstLink != orxNULL)
  {
    /* Updates structure reference counter */
    orxStructure_DecreaseCounter((orxSTRUCTURE *)(_pstCamera->pstLink));
  }

  /* Sets camera link object */
  _pstCamera->pstLink = _pstObject;

  /* Null object? */
  if(_pstObject == orxNULL)
  {
    /* Updates id flags */
    _pstCamera->u32IDFlags &= ~orxCAMERA_KU32_ID_FLAG_LINKED;
  }
  else
  {
    /* Updates structure reference counter */
    orxStructure_IncreaseCounter((orxSTRUCTURE *)_pstObject);

    /* Updates id flags */
    _pstCamera->u32IDFlags |= orxCAMERA_KU32_ID_FLAG_LINKED;

    /* Updates camera position */
    orxCamera_UpdatePosition(_pstCamera, orxTRUE);
  }

  return;
}

/***************************************************************************
 orxCamera_SetOnScreenPosition
 Camera on screen position set accessor.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxCamera_SetOnScreenPosition(orxCAMERA *_pstCamera, orxVECTOR *_pvPosition)
{
  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_FLAG_READY);
  orxASSERT(_pstCamera != orxNULL);
  orxASSERT(_pvPosition != orxNULL);

  /* Copy on screen camera position coords */
  orxVector_Copy(&(_pstCamera->vOnScreenPosition), _pvPosition);

  return;
}


/***************************************************************************
 orxCamera_GetSize
 Camera 2D size get accessor.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxCamera_GetSize(orxCAMERA *_pstCamera, orxVECTOR *_pvSize)
{
  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_FLAG_READY);
  orxASSERT(_pstCamera != orxNULL);
  orxASSERT(_pvSize != orxNULL);

  /* Copy coord */
  orxVector_Copy(_pvSize, &(((orxCAMERA_DATA_2D *)(_pstCamera->pstData))->vSize));

  return;
}

/***************************************************************************
 orxCamera_GetPosition
 Camera 2D position get accessor.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxCamera_GetPosition(orxCAMERA *_pstCamera, orxVECTOR *_pvPosition)
{
  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_FLAG_READY);
  orxASSERT(_pstCamera != orxNULL);
  orxASSERT(_pvPosition != orxNULL);

  /* Gets camera position */
  orxFrame_GetPosition(_pstCamera->pstFrame, _pvPosition, orxTRUE);

  return;
}

/***************************************************************************
 orxCamera_GetRotation
 Camera 2D rotation get accessor.

 returns: orxVOID
 ***************************************************************************/
orxFLOAT orxCamera_GetRotation(orxCAMERA *_pstCamera)
{
  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_FLAG_READY);
  orxASSERT(_pstCamera != orxNULL);

  /* Gets camera position */
  return(orxFrame_GetRotation(_pstCamera->pstFrame, orxTRUE));
}

/***************************************************************************
 orxCamera_GetZoom
 Camera 2D zoom get accessor.

 returns: orxVOID
 ***************************************************************************/
orxFLOAT orxCamera_GetZoom(orxCAMERA *_pstCamera)
{
  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_FLAG_READY);
  orxASSERT(_pstCamera != orxNULL);

  /* Gets camera position */
  return(orxFLOAT_1 / orxFrame_GetScale(_pstCamera->pstFrame, orxTRUE));
}

/***************************************************************************
 orxCamera_GetTarget
 Camera zoom get accessor.

 returns: link object pointer
 ***************************************************************************/
orxOBJECT *orxCamera_GetTarget(orxCAMERA *_pstCamera)
{
  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_FLAG_READY);
  orxASSERT(_pstCamera != orxNULL);

  /* Gets camera link object */
  return(_pstCamera->pstLink);
}

/***************************************************************************
 orxCamera_SetLimits
 Camera limit set accessor (Upper left & Bottom right corners positions).

 returns: orxVOID
 ***************************************************************************/
orxVOID orxCamera_SetLimits(orxCAMERA *_pstCamera, orxVECTOR *_pvUL, orxVECTOR *_pvBR)
{
  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_FLAG_READY);
  orxASSERT(_pstCamera != orxNULL);
  orxASSERT(_pvUL != orxNULL);
  orxASSERT(_pvBR != orxNULL);

  /* 2D camera? */
  if(orxCamera_TestFlags(_pstCamera, orxCAMERA_KU32_ID_FLAG_2D) != orxFALSE)
  {
    /* Sets camera limits position */
    orxVector_Copy(&(((orxCAMERA_DATA_2D *)(_pstCamera->pstData))->vLimitUL), _pvUL);
    orxVector_Copy(&(((orxCAMERA_DATA_2D *)(_pstCamera->pstData))->vLimitBR), _pvBR);

    /* Updates camera flags */
    _pstCamera->u32IDFlags |= orxCAMERA_KU32_ID_FLAG_LIMITED;
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}

/***************************************************************************
 orxCamera_SetLimits
 Camera limit reset accessor (Removes all position limits).

 returns: orxVOID
 ***************************************************************************/
orxVOID orxCamera_RemoveLimits(orxCAMERA *_pstCamera)
{
  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_FLAG_READY);
  orxASSERT(_pstCamera != orxNULL);

  /* Updates camera flags */
  _pstCamera->u32IDFlags &= ~orxCAMERA_KU32_ID_FLAG_LIMITED;

  return;
}

/***************************************************************************
 orxCamera_GetLimits
 Camera limit get accessor (Upper left & Bottom right corners positions).

 returns: orxVOID
 ***************************************************************************/
orxVOID orxCamera_GetLimits(orxCAMERA *_pstCamera, orxVECTOR *_pvUL, orxVECTOR *_pvBR)
{
  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_FLAG_READY);
  orxASSERT(_pstCamera != orxNULL);
  orxASSERT(_pvUL != orxNULL);
  orxASSERT(_pvBR != orxNULL);

  /* 2D camera? */
  if(orxCamera_TestFlags(_pstCamera, orxCAMERA_KU32_ID_FLAG_2D) != orxFALSE)
  {
    /* Gets camera limits position */
    orxVector_Copy(_pvUL, &(((orxCAMERA_DATA_2D *)(_pstCamera->pstData))->vLimitUL));
    orxVector_Copy(_pvBR, &(((orxCAMERA_DATA_2D *)(_pstCamera->pstData))->vLimitBR));
  }

  return;
}

/***************************************************************************
 orxCamera_GetOnScreenPosition
 Camera on screen position get accessor.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxCamera_GetOnScreenPosition(orxCAMERA *_pstCamera, orxVECTOR *_pvPosition)
{
  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_FLAG_READY);
  orxASSERT(_pstCamera != orxNULL);
  orxASSERT(_pvPosition != orxNULL);

  /* Copy on screen camera position coords */
  orxVector_Copy(_pvPosition, &(_pstCamera->vOnScreenPosition));

  return;
}

/***************************************************************************
 orxCamera_TestFlag
 Camera flag test accessor.

 returns: orxBOOL
 ***************************************************************************/
orxBOOL orxCamera_TestFlags(orxCAMERA *_pstCamera, orxU32 _u32Flags)
{
  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_FLAG_READY);
  orxASSERT(_pstCamera != orxNULL);

  /* Tests */
  return((_pstCamera->u32IDFlags & _u32Flags) == _u32Flags);
}

/***************************************************************************
 orxCamera_SetFlag
 Camera flag get/set accessor.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxCamera_SetFlags(orxCAMERA *_pstCamera, orxU32 _u32AddFlags, orxU32 _u32RemoveFlags)
{
  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_FLAG_READY);
  orxASSERT(_pstCamera != orxNULL);

  /* Updates flags */
  _pstCamera->u32IDFlags &= ~_u32RemoveFlags;
  _pstCamera->u32IDFlags |= _u32AddFlags;

  return;
}
