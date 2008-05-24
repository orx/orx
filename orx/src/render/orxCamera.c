/**
 * @file orxCamera.c
 *
 * Camera module
 *
 */

 /***************************************************************************
 orxCamera.c
 Camera module

 begin                : 10/12/2003
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


#include "render/orxCamera.h"

#include "debug/orxDebug.h"
#include "core/orxConfig.h"
#include "memory/orxMemory.h"
#include "object/orxStructure.h"


/** Module flags
 */
#define orxCAMERA_KU32_STATIC_FLAG_NONE       0x00000000  /**< No flags */

#define orxCAMERA_KU32_STATIC_FLAG_READY      0x00000001  /**< Ready flag */

#define orxCAMERA_KU32_STATIC_MASK_ALL        0xFFFFFFFF  /**< All mask */


/** orxCAMERA flags / masks
 */
#define orxCAMERA_KU32_MASK_ALL               0xFFFFFFFF  /**< All mask */


/** Misc defines
 */
#define orxCAMERA_KZ_CONFIG_ZOOM              "Zoom"
#define orxCAMERA_KZ_CONFIG_POSITION          "Position"
#define orxCAMERA_KZ_CONFIG_ROTATION          "Rotation"
#define orxCAMERA_KZ_CONFIG_FRUSTUM_NEAR      "FrustumNear"
#define orxCAMERA_KZ_CONFIG_FRUSTUM_FAR       "FrustumFar"
#define orxCAMERA_KZ_CONFIG_FRUSTUM_WIDTH     "FrustumWidth"
#define orxCAMERA_KZ_CONFIG_FRUSTUM_HEIGHT    "FrustumHeight"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Camera structure
 */
struct __orxCAMERA_t
{
  orxSTRUCTURE        stStructure;            /**< Public structure, first structure member : 16 */
  orxFRAME           *pstFrame;               /**< Frame : 20 */
  orxAABOX            stFrustum;             /**< Frustum : 44 */

  orxPAD(44)
};


/** Static structure
 */
typedef struct __orxCAMERA_STATIC_t
{
  orxU32 u32Flags;                            /**< Control flags : 4 */

} orxCAMERA_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
orxSTATIC orxCAMERA_STATIC sstCamera;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Deletes all cameras
 */
orxSTATIC orxVOID orxCamera_DeleteAll()
{
  orxCAMERA *pstCamera;

  /* Gets first camera */
  pstCamera = (orxCAMERA *)orxStructure_GetFirst(orxSTRUCTURE_ID_CAMERA);

  /* Non empty? */
  while(pstCamera != orxNULL)
  {
    /* Deletes camera */
    orxCamera_Delete(pstCamera);

    /* Gets first remaining camera */
    pstCamera = (orxCAMERA *)orxStructure_GetFirst(orxSTRUCTURE_ID_CAMERA);
  }

  return;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Camera module setup
 */
orxVOID orxCamera_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_CAMERA, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_CAMERA, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_CAMERA, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_CAMERA, orxMODULE_ID_FRAME);

  return;
}

/** Inits Camera module
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxCamera_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY))
  {
    /* Cleans control structure */
    orxMemory_Zero(&sstCamera, sizeof(orxCAMERA_STATIC));

    /* Registers structure type */
    eResult = orxSTRUCTURE_REGISTER(CAMERA, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxNULL);
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
    sstCamera.u32Flags = orxCAMERA_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return eResult;
}

/** Exits from Camera module
 */
orxVOID orxCamera_Exit()
{
  /* Initialized? */
  if(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY)
  {
    /* Deletes camera list */
    orxCamera_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_CAMERA);

    /* Updates flags */
    sstCamera.u32Flags &= ~orxCAMERA_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}

/** Creates a camera
 * @param[in]   _u32Flags               Camera flags (2D / ...)
 * @return      Created orxCAMERA / orxNULL
 */
orxCAMERA *orxFASTCALL orxCamera_Create(orxU32 _u32Flags)
{
  orxCAMERA *pstCamera = orxNULL;
  orxFRAME  *pstFrame;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxASSERT((_u32Flags & orxCAMERA_KU32_MASK_USER_ALL) == _u32Flags);

  /* Creates camera */
  pstCamera = (orxCAMERA *)orxStructure_Create(orxSTRUCTURE_ID_CAMERA);

  /* Valid? */
  if(pstCamera != orxNULL)
  {
    /* Creates frame */
    pstFrame = orxFrame_Create(orxFRAME_KU32_FLAG_NONE);

    /* Valid? */
    if(pstFrame != orxNULL)
    {
      /* 2D? */
      if(orxFLAG_TEST(_u32Flags, orxCAMERA_KU32_FLAG_2D))
      {
        /* Stores frame */
        pstCamera->pstFrame = pstFrame;

        /* Increases its reference counter */
        orxStructure_IncreaseCounter(pstFrame);

        /* Updates flags */
        orxStructure_SetFlags(pstCamera, orxCAMERA_KU32_FLAG_2D, orxCAMERA_KU32_FLAG_NONE);
      }
      else
      {
        /* !!! MSG !!! */

        /* Fress partially allocated camera */
        orxFrame_Delete(pstFrame);
        orxStructure_Delete(pstCamera);

        /* Updates result */
        pstCamera = orxNULL;
      }
    }
    else
    {
      /* !!! MSG !!! */

      /* Fress partially allocated camera */
      orxStructure_Delete(pstCamera);

      /* Updates result */
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

/** Creates a camera from config
 * @param[in]   _zConfigID    Config ID
 * @ return orxCAMERA / orxNULL
 */
orxCAMERA *orxFASTCALL orxCamera_CreateFromConfig(orxCONST orxSTRING _zConfigID)
{
  orxCAMERA  *pstResult;
  orxSTRING   zPreviousSection;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);

  /* Gets previous config section */
  zPreviousSection = orxConfig_GetCurrentSection();

  /* Selects section */
  if(orxConfig_SelectSection(_zConfigID) != orxSTATUS_FAILURE)
  {
    /* Creates 2D default camera */
    pstResult = orxCamera_Create(orxCAMERA_KU32_FLAG_2D);

    /* Valid? */
    if(pstResult != orxNULL)
    {
      orxVECTOR vPosition;
      orxFLOAT  fNear, fFar, fWidth, fHeight;

      /* Gets frustum info */
      fNear   = orxConfig_GetFloat(orxCAMERA_KZ_CONFIG_FRUSTUM_NEAR);
      fFar    = orxConfig_GetFloat(orxCAMERA_KZ_CONFIG_FRUSTUM_FAR);
      fWidth  = orxConfig_GetFloat(orxCAMERA_KZ_CONFIG_FRUSTUM_WIDTH);
      fHeight = orxConfig_GetFloat(orxCAMERA_KZ_CONFIG_FRUSTUM_HEIGHT);

      /* Applies it */
      orxCamera_SetFrustum(pstResult, fWidth, fHeight, fNear, fFar);

      /* Has zoom? */
      if(orxConfig_HasValue(orxCAMERA_KZ_CONFIG_ZOOM) != orxFALSE)
      {
        orxFLOAT fZoom;

        /* Gets config zoom */
        fZoom = orxConfig_GetFloat(orxCAMERA_KZ_CONFIG_ZOOM);

        /* Valid? */
        if(fZoom > orxFLOAT_0)
        {
          /* Applies it */
          orxCamera_SetZoom(pstResult, fZoom);
        }
      }

      /* Has a position? */
      if(orxConfig_GetVector(orxCAMERA_KZ_CONFIG_POSITION, &vPosition) != orxNULL)
      {
        /* Updates camera position */
        orxCamera_SetPosition(pstResult, &vPosition);
      }

      /* Updates object rotation */
      orxCamera_SetRotation(pstResult, orxMATH_KF_DEG_TO_RAD * orxConfig_GetFloat(orxCAMERA_KZ_CONFIG_ROTATION));
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

/** Deletes a camera
 * @param[in]   _pstCamera      Camera to delete
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxCamera_Delete(orxCAMERA *_pstCamera)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstCamera);

  /* Not referenced? */
  if(orxStructure_GetRefCounter(_pstCamera) == 0)
  {
    /* Removes frame reference */
    orxStructure_DecreaseCounter(_pstCamera->pstFrame);

    /* Deletes frame*/
    orxFrame_Delete(_pstCamera->pstFrame);

    /* Deletes structure */
    orxStructure_Delete(_pstCamera);
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

/** Sets camera frustum (3D rectangle for 2D camera)
 * @param[in]   _pstCamera      Concerned camera
 * @param[in]   _fWidth         Width of frustum
 * @param[in]   _fHeight        Height of frustum
 * @param[in]   _fNear          Near distance of frustum
 * @param[in]   _fFar           Far distance of frustum
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxCamera_SetFrustum(orxCAMERA *_pstCamera, orxFLOAT _fWidth, orxFLOAT _fHeight, orxFLOAT _fNear, orxFLOAT _fFar)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstCamera);
  orxASSERT(_fNear <= _fFar);

  /* Updates internal frustum */
  orxVector_Set(&(_pstCamera->stFrustum.vTL), orx2F(-0.5f) * _fWidth, orx2F(-0.5f) * _fHeight, _fNear);
  orxVector_Set(&(_pstCamera->stFrustum.vBR), orx2F(0.5f) * _fWidth, orx2F(0.5f) * _fHeight, _fFar);

  /* Done! */
  return eResult;
}

/** Sets camera position
 * @param[in]   _pstCamera      Concerned camera
 * @param[in]   _pvPosition     Camera position
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxCamera_SetPosition(orxCAMERA *_pstCamera, orxCONST orxVECTOR *_pvPosition)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstCamera);
  orxASSERT(_pvPosition != orxNULL);

  /* Sets camera position */
  orxFrame_SetPosition(_pstCamera->pstFrame, _pvPosition);

  /* Done! */
  return eResult;
}

/** Sets camera rotation
 * @param[in]   _pstCamera      Concerned camera
 * @param[in]   _fRotation      Camera rotation
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxCamera_SetRotation(orxCAMERA *_pstCamera, orxFLOAT _fRotation)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstCamera);

   /* Sets camera rotation */
  orxFrame_SetRotation(_pstCamera->pstFrame, _fRotation);

  /* Done! */
  return eResult;
}

/** Sets camera zoom
 * @param[in]   _pstCamera      Concerned camera
 * @param[in]   _fZoom          Camera zoom
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxCamera_SetZoom(orxCAMERA *_pstCamera, orxFLOAT _fZoom)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxASSERT(_fZoom > orxFLOAT_0);
  orxSTRUCTURE_ASSERT(_pstCamera);

   /* Sets camera zoom */
  orxFrame_SetScale(_pstCamera->pstFrame, orxFLOAT_1 / _fZoom, orxFLOAT_1 / _fZoom);

  /* Done! */
  return eResult;
}

/** Gets camera frustum (3D box for 2D camera)
 * @param[in]   _pstCamera      Concerned camera
 * @param[out]  _pstFrustum    Frustum box
 */
orxVOID orxFASTCALL orxCamera_GetFrustum(orxCONST orxCAMERA *_pstCamera, orxAABOX *_pstFrustum)
{
  orxVECTOR vPosition;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstCamera);
  orxASSERT(_pstFrustum != orxNULL);

  /* Gets camera position */
  orxFrame_GetPosition(_pstCamera->pstFrame, orxFRAME_SPACE_GLOBAL, &vPosition);

  /* Stores frustum */
  orxVector_Add(&(_pstFrustum->vTL), &(_pstCamera->stFrustum.vTL), &vPosition);
  orxVector_Add(&(_pstFrustum->vBR), &(_pstCamera->stFrustum.vBR), &vPosition);

  return;
}

/** Get camera position
 * @param[in]   _pstCamera      Concerned camera
 * @param[out]  _pvPosition     Camera position
 * @return      orxVECTOR
 */
orxVECTOR *orxFASTCALL orxCamera_GetPosition(orxCONST orxCAMERA *_pstCamera, orxVECTOR *_pvPosition)
{
  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstCamera);
  orxASSERT(_pvPosition != orxNULL);

  /* Gets camera position */
  return(orxFrame_GetPosition(_pstCamera->pstFrame, orxFRAME_SPACE_LOCAL, _pvPosition));
}

/** Get camera rotation
 * @param[in]   _pstCamera      Concerned camera
 * @return      Rotation value
 */
orxFLOAT orxFASTCALL orxCamera_GetRotation(orxCONST orxCAMERA *_pstCamera)
{
  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstCamera);

  /* Gets camera rotation */
  return(orxFrame_GetRotation(_pstCamera->pstFrame, orxFRAME_SPACE_LOCAL));
}

/** Gets camera zoom
 * @param[in]   _pstCamera      Concerned camera
 * @return      Zoom value
 */
orxFLOAT orxFASTCALL orxCamera_GetZoom(orxCONST orxCAMERA *_pstCamera)
{
  orxFLOAT fScale, fDummy;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstCamera);

  /* Gets camera scale */
  orxFrame_GetScale(_pstCamera->pstFrame, orxFRAME_SPACE_LOCAL, &fScale, &fDummy);

  /* Done! */
  return(orxFLOAT_1 / fScale);
}

/** Gets camera frame
 * @param[in]   _pstCamera      Concerned camera
 * @return      orxFRAME
 */
orxFRAME *orxFASTCALL orxCamera_GetFrame(orxCONST orxCAMERA *_pstCamera)
{
  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstCamera);

  /* Gets camera frame */
  return(_pstCamera->pstFrame);
}
