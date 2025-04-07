/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008- Orx-Project
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
 * @file orxPhysics.c
 * @date 24/03/2008
 * @author iarwain@orx-project.org
 *
 * Box2D physics plugin implementation
 *
 */


#include "orxPluginAPI.h"

#include "box2d/box2d.h"


#if defined(__orxDEBUG__) || defined(__orxPROFILER__)

  #define orxPHYSICS_ENABLE_DEBUG_DRAW

#endif /* __orxDEBUG__ || __orxPROFILER__ */


/** Module flags
 */
#define orxPHYSICS_KU32_STATIC_FLAG_NONE        0x00000000 /**< No flags */

#define orxPHYSICS_KU32_STATIC_FLAG_READY       0x00000001 /**< Ready flag */
#define orxPHYSICS_KU32_STATIC_FLAG_ENABLED     0x00000002 /**< Enabled flag */
#define orxPHYSICS_KU32_STATIC_FLAG_FIXED_DT    0x00000004 /**< Fixed DT flag */
#define orxPHYSICS_KU32_STATIC_FLAG_INTERPOLATE 0x00000008 /**< Interpolate flag */

#define orxPHYSICS_KU32_STATIC_MASK_ALL         0xFFFFFFFF /**< All mask */

#define orxPHYSICS_KU32_BODY_BANK_SIZE          512
#define orxPHYSICS_KU32_BODY_PART_BANK_SIZE     1024
#define orxPHYSICS_KU32_BODY_JOINT_BANK_SIZE    256
#define orxPHYSICS_KS32_DEFAULT_SUB_STEP        4
#define orxPHYSICS_KF_DEFAULT_FREQUENCY         orx2F(60.0f) /* Default frequency */
#define orxPHYSICS_KF_DEFAULT_DIMENSION_RATIO   orx2F(0.01f) /* Default dimension ratio */
#define orxPHYSICS_KF_MIN_STEP_DURATION         orx2F(0.001f) /* Min step duration */
#define orxPHYSICS_KF_CACHED_ROTATION_EPSILON   orx2F(0.002f) /* Cached rotation epsilon */

#ifdef orxPHYSICS_ENABLE_DEBUG_DRAW

#define orxPHYSICS_KU32_RAY_BANK_SIZE           128
#define orxPHYSICS_KU32_RAY_MISS_COLOR          b2_colorLime
#define orxPHYSICS_KU32_RAY_BEFORE_HIT_COLOR    b2_colorYellow
#define orxPHYSICS_KU32_RAY_AFTER_HIT_COLOR     b2_colorRed

#endif /* orxPHYSICS_ENABLE_DEBUG_DRAW */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Body
 */
struct __orxPHYSICS_BODY_t
{
  orxLINKLIST_NODE    stNode;                 /**< Link list node */
  orxVECTOR           vPreviousPosition;      /**< Previous position */
  orxVECTOR           vInterpolatedPosition;  /**< Interpolated position */
  const orxSTRUCTURE *pstOwner;               /**< Owner */
  b2BodyId            stBody;                 /**< Box2D body */
  orxFLOAT            fPreviousRotation;      /**< Previous rotation */
  orxFLOAT            fInterpolatedRotation;  /**< Interpolated rotation */
  orxFLOAT            fCachedRotation;        /**< Cached rotation */
};

/** Body part
 */
struct __orxPHYSICS_BODY_PART_t
{
  union
  {
    b2ShapeId         stShape;                /**< Box2D shape */
    b2ChainId         stChain;                /**< Box2D chain */
  };
  orxBOOL             bIsChain;               /**< Is Chain? */
};

/** Body joint
 */
struct __orxPHYSICS_BODY_JOINT_t
{
  b2JointId           stJoint;                /**< Box2D joint */
};

/** RayCast
 */
typedef struct __orxPHYSICS_RAYCAST_t
{
  orxVECTOR           vContact;
  orxVECTOR           vNormal;
  orxHANDLE           hResult;
  orxBOOL             bEarlyExit;

} orxPHYSICS_RAYCAST;

/** BoxPick
 */
typedef struct __orxPHYSICS_BOXPICK_t
{
  orxHANDLE          *ahUserDataList;
  orxU32              u32Size;
  orxU32              u32Count;

} orxPHYSICS_BOXPICK;

#ifdef orxPHYSICS_ENABLE_DEBUG_DRAW

/** Debug ray
 */
typedef struct __orxPHYSICS_DEBUG_RAY_t
{
  b2Vec2              vBegin;
  b2Vec2              vEnd;
  b2HexColor          stColor;

} orxPHYSICS_DEBUG_RAY;

#endif /* orxPHYSICS_ENABLE_DEBUG_DRAW */


/** Static structure
 */
typedef struct __orxPHYSICS_STATIC_t
{
  orxU32                      u32Flags;               /**< Control flags */
  orxS32                      s32IterationPerSteps;   /**< Iteration per steps */
  orxFLOAT                    fDimensionRatio;        /**< Dimension ratio */
  orxFLOAT                    fRecDimensionRatio;     /**< Reciprocal dimension ratio */
  orxFLOAT                    fLastDT;                /**< Last DT */
  b2WorldId                   stWorld;                /**< Box2D world */
  orxFLOAT                    fFixedDT;               /**< Fixed DT */
  orxFLOAT                    fDTAccumulator;         /**< DT accumulator */
  orxLINKLIST                 stBodyList;             /**< Body link list */
  orxBANK                    *pstBodyBank;            /**< Body bank */
  orxBANK                    *pstBodyPartBank;        /**< Body part bank */
  orxBANK                    *pstBodyJointBank;       /**< Body joint bank */

#ifdef orxPHYSICS_ENABLE_DEBUG_DRAW

  orxBANK                    *pstRayBank;             /**< Debug ray bank */
  b2DebugDraw                 stDebugDraw;            /**< Debug draw interface */

#endif /* orxPHYSICS_ENABLE_DEBUG_DRAW */

} orxPHYSICS_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxPHYSICS_STATIC sstPhysics;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

void *orxPhysics_Box2D_Allocate(unsigned int _uiSize, int _iAlignment)
{
  orxU32  u32RealSize;
  void   *pAlloc, *pResult;

  /* Gets real allocation size */
  u32RealSize = _uiSize + sizeof(orxUPTR) + _iAlignment;
  
  /* Allocates memory */
  pAlloc = orxMemory_Allocate(u32RealSize, orxMEMORY_TYPE_PHYSICS);
  
  /* Success? */
  if(pAlloc != orxNULL)
  {
    /* Updates result */
    pResult = (void *)orxALIGN((orxUPTR)pAlloc + sizeof(orxUPTR), _iAlignment);
    
    /* Stores pointer */
    ((void **)pResult)[-1] = pAlloc;
  }
  
  /* Done */
  return pResult;
}

void orxPhysics_Box2D_Free(void *_pMem)
{
  /* Frees real allocation */
  orxMemory_Free(((void **)_pMem)[-1]);
}

float orxPhysics_Box2D_RayCastCallback(b2ShapeId _stShape, b2Vec2 _vContact, b2Vec2 _vNormal, float _fFraction, void *_pContext)
{
  orxPHYSICS_RAYCAST *pstRayCast;
  float               fResult;

  /* Gets ray cast */
  pstRayCast = (orxPHYSICS_RAYCAST *)_pContext;

  /* Stores contact and normal */
  orxVector_Set(&(pstRayCast->vContact), sstPhysics.fRecDimensionRatio * _vContact.x, sstPhysics.fRecDimensionRatio * _vContact.y, orxFLOAT_0);
  orxVector_Set(&(pstRayCast->vNormal), _vNormal.x, _vNormal.y, orxFLOAT_0);

  /* Stores associated object's handle */
  pstRayCast->hResult = (orxHANDLE)(((orxPHYSICS_BODY *)b2Body_GetUserData(b2Shape_GetBody(_stShape)))->pstOwner);

  /* Early exit? */
  if(pstRayCast->bEarlyExit != orxFALSE)
  {
    /* Stops now */
    fResult = 0.0f;
  }
  else
  {
    /* Checks for closer fixture */
    fResult = _fFraction;
  }

  /* Done! */
  return fResult;
}

static bool orxPhysics_Box2D_BoxPickCallback(b2ShapeId _stShape, void *_pContext)
{
  orxPHYSICS_BOXPICK *pstBoxPick;
  bool                bResult = true;

  /* Gets box pick */
  pstBoxPick = (orxPHYSICS_BOXPICK *)_pContext;

  /* Has available storage? */
  if(pstBoxPick->u32Count < pstBoxPick->u32Size)
  {
    /* Stores associated object's handle */
    pstBoxPick->ahUserDataList[pstBoxPick->u32Count] = (orxHANDLE)(((orxPHYSICS_BODY *)b2Body_GetUserData(b2Shape_GetBody(_stShape)))->pstOwner);
  }

  /* Updates count */
  pstBoxPick->u32Count++;

  /* Done! */
  return bResult;
}

#ifdef orxPHYSICS_ENABLE_DEBUG_DRAW

static void orxPhysics_Box2D_DrawPolygon(const b2Vec2 *_avVertexList, int _s32VertexNumber, b2HexColor _stColor, void *_pContext)
{
  orxVIEWPORT *pstViewport;

  /* For all viewports */
  for(pstViewport = orxVIEWPORT(orxStructure_GetFirst(orxSTRUCTURE_ID_VIEWPORT));
      pstViewport != orxNULL;
      pstViewport = orxVIEWPORT(orxStructure_GetNext(pstViewport)))
  {
    /* Is enabled and supports debug? */
    if((orxViewport_IsEnabled(pstViewport) != orxFALSE)
    && (!orxStructure_GetFlags(pstViewport, orxVIEWPORT_KU32_FLAG_NO_DEBUG)))
    {
      orxCAMERA *pstCamera;

      /* Gets viewport camera */
      pstCamera = orxViewport_GetCamera(pstViewport);

      /* Valid? */
      if(pstCamera != orxNULL)
      {
        orxAABOX    stFrustum;
        orxVECTOR   vCameraPosition;
        orxVECTOR  *avVertexList = (orxVECTOR *)alloca(_s32VertexNumber * sizeof(orxVECTOR));
        orxFLOAT    fZ;
        orxS32      i;

        /* Gets camera position */
        orxFrame_GetPosition(orxCamera_GetFrame(pstCamera), orxFRAME_SPACE_GLOBAL, &vCameraPosition);

        /* Gets its frustum */
        orxCamera_GetFrustum(pstCamera, &stFrustum);

        /* Stores its Z */
        fZ = stFrustum.vTL.fZ + vCameraPosition.fZ;

        /* For all vertices */
        for(i = 0; i < _s32VertexNumber; i++)
        {
          orxVECTOR vTemp;

          /* Sets it */
          orxVector_Set(&vTemp, sstPhysics.fRecDimensionRatio * orx2F(_avVertexList[i].x), sstPhysics.fRecDimensionRatio * orx2F(_avVertexList[i].y), fZ);

          /* Stores its screen position */
          orxRender_GetScreenPosition(&vTemp, pstViewport, &(avVertexList[i]));
        }

        /* Draws polygon */
        orxDisplay_DrawPolygon(avVertexList, (orxS32)_s32VertexNumber, orx2RGBA((_stColor & 0xFF0000) >> 16, (_stColor & 0x00FF00) >> 8, _stColor & 0x0000FF, 0xFF), orxFALSE);
      }
    }
  }

  /* Done! */
  return;
}

static void orxPhysics_Box2D_DrawSolidPolygon(b2Transform _stTransform, const b2Vec2 *_avVertexList, int _s32VertexNumber, float _fRadius, b2HexColor _stColor, void *_pContext)
{
  orxVIEWPORT *pstViewport;

  /* For all viewports */
  for(pstViewport = orxVIEWPORT(orxStructure_GetFirst(orxSTRUCTURE_ID_VIEWPORT));
      pstViewport != orxNULL;
      pstViewport = orxVIEWPORT(orxStructure_GetNext(pstViewport)))
  {
    /* Is enabled and supports debug? */
    if((orxViewport_IsEnabled(pstViewport) != orxFALSE)
    && (!orxStructure_GetFlags(pstViewport, orxVIEWPORT_KU32_FLAG_NO_DEBUG)))
    {
      orxCAMERA *pstCamera;

      /* Gets viewport camera */
      pstCamera = orxViewport_GetCamera(pstViewport);

      /* Valid? */
      if(pstCamera != orxNULL)
      {
        orxAABOX    stFrustum;
        orxVECTOR   vCameraPosition;
        orxVECTOR  *avVertexList = (orxVECTOR *)alloca(_s32VertexNumber * sizeof(orxVECTOR));
        orxFLOAT    fZ;
        orxS32      i;

        /* Gets camera position */
        orxFrame_GetPosition(orxCamera_GetFrame(pstCamera), orxFRAME_SPACE_GLOBAL, &vCameraPosition);

        /* Gets its frustum */
        orxCamera_GetFrustum(pstCamera, &stFrustum);

        /* Stores its Z */
        fZ = stFrustum.vTL.fZ + vCameraPosition.fZ;

        /* For all vertices */
        for(i = 0; i < _s32VertexNumber; i++)
        {
          orxVECTOR vTemp;
          b2Vec2    vPoint;

          /* Sets it */
          vPoint = b2TransformPoint(_stTransform, _avVertexList[i]);
          orxVector_Set(&vTemp, sstPhysics.fRecDimensionRatio * orx2F(vPoint.x), sstPhysics.fRecDimensionRatio * orx2F(vPoint.y), fZ);

          /* Stores its screen position */
          orxRender_GetScreenPosition(&vTemp, pstViewport, &(avVertexList[i]));
        }

        /* Draws polygon inside */
        orxDisplay_DrawPolygon(avVertexList, (orxS32)_s32VertexNumber, orx2RGBA((_stColor & 0xFF0000) >> 16, (_stColor & 0x00FF00) >> 8, _stColor & 0x0000FF, 0x7F), orxTRUE);

        /* Draws polygon outside */
        orxDisplay_DrawPolygon(avVertexList, (orxS32)_s32VertexNumber, orx2RGBA((_stColor & 0xFF0000) >> 16, (_stColor & 0x00FF00) >> 8, _stColor & 0x0000FF, 0xFF), orxFALSE);
      }
    }
  }

  /* Done! */
  return;
}

static void orxPhysics_Box2D_DrawCircle(b2Vec2 _vCenter, float _fRadius, b2HexColor _stColor, void *_pContext)
{
  orxVIEWPORT *pstViewport;

  /* For all viewports */
  for(pstViewport = orxVIEWPORT(orxStructure_GetFirst(orxSTRUCTURE_ID_VIEWPORT));
      pstViewport != orxNULL;
      pstViewport = orxVIEWPORT(orxStructure_GetNext(pstViewport)))
  {
    /* Is enabled and supports debug? */
    if((orxViewport_IsEnabled(pstViewport) != orxFALSE)
    && (!orxStructure_GetFlags(pstViewport, orxVIEWPORT_KU32_FLAG_NO_DEBUG)))
    {
      orxCAMERA *pstCamera;

      /* Gets viewport camera */
      pstCamera = orxViewport_GetCamera(pstViewport);

      /* Valid? */
      if(pstCamera != orxNULL)
      {
        orxAABOX  stFrustum;
        orxVECTOR vCameraPosition, vCenter, vTemp;
        orxFLOAT  fRadius, fZ;

        /* Gets camera position */
        orxFrame_GetPosition(orxCamera_GetFrame(pstCamera), orxFRAME_SPACE_GLOBAL, &vCameraPosition);

        /* Gets its frustum */
        orxCamera_GetFrustum(pstCamera, &stFrustum);

        /* Stores its Z */
        fZ = stFrustum.vTL.fZ + vCameraPosition.fZ;

        /* Inits center & temp vectors */
        orxVector_Set(&vCenter, sstPhysics.fRecDimensionRatio * orx2F(_vCenter.x), sstPhysics.fRecDimensionRatio * orx2F(_vCenter.y), fZ);
        orxVector_Copy(&vTemp, &vCenter);
        vTemp.fX += sstPhysics.fRecDimensionRatio * orx2F(_fRadius);

        /* Gets their screen positions */
        orxRender_GetScreenPosition(&vCenter, pstViewport, &vCenter);
        orxRender_GetScreenPosition(&vTemp, pstViewport, &vTemp);

        /* Retrieves transformed radius */
        fRadius = orxVector_GetDistance(&vCenter, &vTemp);

        /* Draws circle */
        orxDisplay_DrawCircle(&vCenter, fRadius, orx2RGBA((_stColor & 0xFF0000) >> 16, (_stColor & 0x00FF00) >> 8, _stColor & 0x0000FF, 0xFF), orxFALSE);
      }
    }
  }

  /* Done! */
  return;
}

static void orxPhysics_Box2D_DrawSolidCircle(b2Transform _stTransform, float _fRadius, b2HexColor _stColor, void *_pContext)
{
  orxVIEWPORT *pstViewport;

  /* For all viewports */
  for(pstViewport = orxVIEWPORT(orxStructure_GetFirst(orxSTRUCTURE_ID_VIEWPORT));
      pstViewport != orxNULL;
      pstViewport = orxVIEWPORT(orxStructure_GetNext(pstViewport)))
  {
    /* Is enabled and supports debug? */
    if((orxViewport_IsEnabled(pstViewport) != orxFALSE)
    && (!orxStructure_GetFlags(pstViewport, orxVIEWPORT_KU32_FLAG_NO_DEBUG)))
    {
      orxCAMERA *pstCamera;

      /* Gets viewport camera */
      pstCamera = orxViewport_GetCamera(pstViewport);

      /* Valid? */
      if(pstCamera != orxNULL)
      {
        orxAABOX  stFrustum;
        orxVECTOR vCameraPosition, vCenter, vTemp;
        orxFLOAT  fRadius, fZ;

        /* Gets camera position */
        orxFrame_GetPosition(orxCamera_GetFrame(pstCamera), orxFRAME_SPACE_GLOBAL, &vCameraPosition);

        /* Gets its frustum */
        orxCamera_GetFrustum(pstCamera, &stFrustum);

        /* Stores its Z */
        fZ = stFrustum.vTL.fZ + vCameraPosition.fZ;

        /* Inits center vectors */
        orxVector_Set(&vCenter, sstPhysics.fRecDimensionRatio * orx2F(_stTransform.p.x), sstPhysics.fRecDimensionRatio * orx2F(_stTransform.p.y), fZ);
        orxVector_Copy(&vTemp, &vCenter);
        vTemp.fX += sstPhysics.fRecDimensionRatio * orx2F(_fRadius);

        /* Gets their screen positions */
        orxRender_GetScreenPosition(&vCenter, pstViewport, &vCenter);
        orxRender_GetScreenPosition(&vTemp, pstViewport, &vTemp);

        /* Retrieves transformed radius */
        fRadius = orxVector_GetDistance(&vCenter, &vTemp);

        /* Draws circle inside */
        orxDisplay_DrawCircle(&vCenter, fRadius, orx2RGBA((_stColor & 0xFF0000) >> 16, (_stColor & 0x00FF00) >> 8, _stColor & 0x0000FF, 0x7F), orxTRUE);

        /* Draws circle outside */
        orxDisplay_DrawCircle(&vCenter, fRadius, orx2RGBA((_stColor & 0xFF0000) >> 16, (_stColor & 0x00FF00) >> 8, _stColor & 0x0000FF, 0xFF), orxFALSE);
      }
    }
  }

  /* Done! */
  return;
}

static void orxPhysics_Box2D_DrawSegment(b2Vec2 _vP1, b2Vec2 _vP2, b2HexColor _stColor, void *_pContext)
{
  orxVIEWPORT *pstViewport;

  /* For all viewports */
  for(pstViewport = orxVIEWPORT(orxStructure_GetFirst(orxSTRUCTURE_ID_VIEWPORT));
      pstViewport != orxNULL;
      pstViewport = orxVIEWPORT(orxStructure_GetNext(pstViewport)))
  {
    /* Is enabled and supports debug? */
    if((orxViewport_IsEnabled(pstViewport) != orxFALSE)
    && (!orxStructure_GetFlags(pstViewport, orxVIEWPORT_KU32_FLAG_NO_DEBUG)))
    {
      orxCAMERA *pstCamera;

      /* Gets viewport camera */
      pstCamera = orxViewport_GetCamera(pstViewport);

      /* Valid? */
      if(pstCamera != orxNULL)
      {
        orxAABOX  stFrustum;
        orxVECTOR vCameraPosition, vBegin, vEnd;
        orxFLOAT  fZ;

        /* Gets camera position */
        orxFrame_GetPosition(orxCamera_GetFrame(pstCamera), orxFRAME_SPACE_GLOBAL, &vCameraPosition);

        /* Gets its frustum */
        orxCamera_GetFrustum(pstCamera, &stFrustum);

        /* Stores its Z */
        fZ = stFrustum.vTL.fZ + vCameraPosition.fZ;

        /* Inits points */
        orxVector_Set(&vBegin, sstPhysics.fRecDimensionRatio * orx2F(_vP1.x), sstPhysics.fRecDimensionRatio * orx2F(_vP1.y), fZ);
        orxVector_Set(&vEnd, sstPhysics.fRecDimensionRatio * orx2F(_vP2.x), sstPhysics.fRecDimensionRatio * orx2F(_vP2.y), fZ);

        /* Gets their screen positions */
        orxRender_GetScreenPosition(&vBegin, pstViewport, &vBegin);
        orxRender_GetScreenPosition(&vEnd, pstViewport, &vEnd);

        /* Draws segment */
        orxDisplay_DrawLine(&vBegin, &vEnd, orx2RGBA((_stColor & 0xFF0000) >> 16, (_stColor & 0x00FF00) >> 8, _stColor & 0x0000FF, 0xFF));
      }
    }
  }

  /* Done! */
  return;
}

static void orxPhysics_Box2D_DrawTransform(b2Transform _stTransform, void *_pContext)
{
  orxVIEWPORT *pstViewport;

  /* For all viewports */
  for(pstViewport = orxVIEWPORT(orxStructure_GetFirst(orxSTRUCTURE_ID_VIEWPORT));
      pstViewport != orxNULL;
      pstViewport = orxVIEWPORT(orxStructure_GetNext(pstViewport)))
  {
    /* Is enabled and supports debug? */
    if((orxViewport_IsEnabled(pstViewport) != orxFALSE)
    && (!orxStructure_GetFlags(pstViewport, orxVIEWPORT_KU32_FLAG_NO_DEBUG)))
    {
      orxCAMERA *pstCamera;

      /* Gets viewport camera */
      pstCamera = orxViewport_GetCamera(pstViewport);

      /* Valid? */
      if(pstCamera != orxNULL)
      {
        orxAABOX  stFrustum;
        orxVECTOR vCameraPosition, vBegin, vEndX, vEndY;
        orxFLOAT  fZ;

        /* Gets camera position */
        orxFrame_GetPosition(orxCamera_GetFrame(pstCamera), orxFRAME_SPACE_GLOBAL, &vCameraPosition);

        /* Gets its frustum */
        orxCamera_GetFrustum(pstCamera, &stFrustum);

        /* Stores its Z */
        fZ = stFrustum.vTL.fZ + vCameraPosition.fZ;

        /* Inits points */
#define orxPHYSICS_KF_SCALE orx2F(0.5f)
        orxVector_Set(&vBegin, sstPhysics.fRecDimensionRatio * orx2F(_stTransform.p.x), sstPhysics.fRecDimensionRatio * orx2F(_stTransform.p.y), fZ);
        orxVector_Set(&vEndX, vBegin.fX + sstPhysics.fRecDimensionRatio * orxPHYSICS_KF_SCALE * orx2F(_stTransform.q.c), vBegin.fY + sstPhysics.fRecDimensionRatio * orxPHYSICS_KF_SCALE * orx2F(_stTransform.q.s), fZ);
        orxVector_Set(&vEndY, vBegin.fX + sstPhysics.fRecDimensionRatio * orxPHYSICS_KF_SCALE * orx2F(-_stTransform.q.s), vBegin.fY + sstPhysics.fRecDimensionRatio * orxPHYSICS_KF_SCALE * orx2F(_stTransform.q.c), fZ);
#undef orxPHYSICS_KF_SCALE

        /* Gets their screen positions */
        orxRender_GetScreenPosition(&vBegin, pstViewport, &vBegin);
        orxRender_GetScreenPosition(&vEndX, pstViewport, &vEndX);
        orxRender_GetScreenPosition(&vEndY, pstViewport, &vEndY);

        /* Draws segments */
        orxDisplay_DrawLine(&vBegin, &vEndX, orx2RGBA(0xFF, 0x00, 0x00, 0xFF));
        orxDisplay_DrawLine(&vBegin, &vEndY, orx2RGBA(0x00, 0xFF, 0x00, 0xFF));
      }
    }
  }

  /* Done! */
  return;
}

static void orxPhysics_Box2D_DrawPoint(b2Vec2 _vPoint, float _fSize, b2HexColor _stColor, void *_pContext)
{
  orxVIEWPORT *pstViewport;

  /* For all viewports */
  for(pstViewport = orxVIEWPORT(orxStructure_GetFirst(orxSTRUCTURE_ID_VIEWPORT));
      pstViewport != orxNULL;
      pstViewport = orxVIEWPORT(orxStructure_GetNext(pstViewport)))
  {
    /* Is enabled and supports debug? */
    if((orxViewport_IsEnabled(pstViewport) != orxFALSE)
    && (!orxStructure_GetFlags(pstViewport, orxVIEWPORT_KU32_FLAG_NO_DEBUG)))
    {
      orxCAMERA *pstCamera;

      /* Gets viewport camera */
      pstCamera = orxViewport_GetCamera(pstViewport);

      /* Valid? */
      if(pstCamera != orxNULL)
      {
        orxAABOX  stFrustum;
        orxVECTOR vCameraPosition, vCenter, vTemp;
        orxFLOAT  fRadius, fZ;

        /* Gets camera position */
        orxFrame_GetPosition(orxCamera_GetFrame(pstCamera), orxFRAME_SPACE_GLOBAL, &vCameraPosition);

        /* Gets its frustum */
        orxCamera_GetFrustum(pstCamera, &stFrustum);

        /* Stores its Z */
        fZ = stFrustum.vTL.fZ + vCameraPosition.fZ;

        /* Inits center & temp vectors */
        orxVector_Set(&vCenter, sstPhysics.fRecDimensionRatio * orx2F(_vPoint.x), sstPhysics.fRecDimensionRatio * orx2F(_vPoint.y), fZ);
        orxVector_Copy(&vTemp, &vCenter);
        vTemp.fX += sstPhysics.fRecDimensionRatio * orx2F(_fSize);

        /* Gets their screen positions */
        orxRender_GetScreenPosition(&vCenter, pstViewport, &vCenter);
        orxRender_GetScreenPosition(&vTemp, pstViewport, &vTemp);

        /* Retrieves transformed radius */
        fRadius = orxVector_GetDistance(&vCenter, &vTemp);

        /* Draws circle */
        orxDisplay_DrawCircle(&vCenter, fRadius, orx2RGBA((_stColor & 0xFF0000) >> 16, (_stColor & 0x00FF00) >> 8, _stColor & 0x0000FF, 0xFF), orxFALSE);
      }
    }
  }

  /* Done! */
  return;
}

static void orxPhysics_Box2D_DrawString(b2Vec2 _vPosition, const char *_zString, b2HexColor _stColor, void *_pContext)
{
  orxVIEWPORT *pstViewport;

  /* For all viewports */
  for(pstViewport = orxVIEWPORT(orxStructure_GetFirst(orxSTRUCTURE_ID_VIEWPORT));
      pstViewport != orxNULL;
      pstViewport = orxVIEWPORT(orxStructure_GetNext(pstViewport)))
  {
    /* Is enabled and supports debug? */
    if((orxViewport_IsEnabled(pstViewport) != orxFALSE)
    && (!orxStructure_GetFlags(pstViewport, orxVIEWPORT_KU32_FLAG_NO_DEBUG)))
    {
      orxCAMERA *pstCamera;

      /* Gets viewport camera */
      pstCamera = orxViewport_GetCamera(pstViewport);

      /* Valid? */
      if(pstCamera != orxNULL)
      {
        orxAABOX              stFrustum;
        orxVECTOR             vCameraPosition, vTemp;
        orxDISPLAY_TRANSFORM  stTransform;
        orxFONT              *pstFont;
        orxFLOAT              fZ;

        /* Gets camera position */
        orxFrame_GetPosition(orxCamera_GetFrame(pstCamera), orxFRAME_SPACE_GLOBAL, &vCameraPosition);

        /* Gets its frustum */
        orxCamera_GetFrustum(pstCamera, &stFrustum);

        /* Stores its Z */
        fZ = stFrustum.vTL.fZ + vCameraPosition.fZ;

        /* Inits vectors */
        orxVector_Set(&vTemp, sstPhysics.fRecDimensionRatio * orx2F(_vPosition.x), sstPhysics.fRecDimensionRatio * orx2F(_vPosition.y), fZ);

        /* Gets its screen position */
        orxRender_GetScreenPosition(&vTemp, pstViewport, &vTemp);

        /* Draws circle outside */
        pstFont = orxFont_Get(orxFONT_KZ_DEFAULT_FONT_NAME);
        orxMemory_Zero(&stTransform, sizeof(orxDISPLAY_TRANSFORM));
        stTransform.fDstX     = vTemp.fX;
        stTransform.fDstY     = vTemp.fY;
        stTransform.fRepeatX  =
        stTransform.fRepeatY  =
        stTransform.fScaleX   =
        stTransform.fScaleY   = orxFLOAT_1;
        orxDisplay_TransformText(_zString, orxTexture_GetBitmap(orxFont_GetTexture(pstFont)), orxFont_GetMap(pstFont), &stTransform, orx2RGBA((_stColor & 0xFF0000) >> 16, (_stColor & 0x00FF00) >> 8, _stColor & 0x0000FF, 0xFF), orxDISPLAY_SMOOTHING_OFF, orxDISPLAY_BLEND_MODE_ALPHA);
      }
    }
  }

  /* Done! */
  return;
}

static orxSTATUS orxFASTCALL orxPhysics_Box2D_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(_pstEvent->eType == orxEVENT_TYPE_RENDER);

  /* End of rendering? */
  if(_pstEvent->eID == orxRENDER_EVENT_STOP)
  {
    /* Pushes config section */
    orxConfig_PushSection(orxPHYSICS_KZ_CONFIG_SECTION);

    /* Show debug? */
    if(orxConfig_GetBool(orxPHYSICS_KZ_CONFIG_SHOW_DEBUG) != orxFALSE)
    {
      orxPHYSICS_DEBUG_RAY *pstRay;

      /* Draws debug */
      b2World_Draw(sstPhysics.stWorld, &(sstPhysics.stDebugDraw));

      /* For all rays */
      for(pstRay = (orxPHYSICS_DEBUG_RAY *)orxBank_GetNext(sstPhysics.pstRayBank, orxNULL);
          pstRay != orxNULL;
          pstRay = (orxPHYSICS_DEBUG_RAY *)orxBank_GetNext(sstPhysics.pstRayBank, pstRay))
      {
        /* Draws it */
        orxPhysics_Box2D_DrawSegment(pstRay->vBegin, pstRay->vEnd, pstRay->stColor, orxNULL);
      }
    }

    /* Clears ray bank */
    orxBank_Clear(sstPhysics.pstRayBank);

    /* Pops config section */
    orxConfig_PopSection();
  }

  /* Done! */
  return eResult;
}

#endif /* orxPHYSICS_ENABLE_DEBUG_DRAW */

/** Applies physics simulation result to the body
 * @param[in]   _pstBody                      Concerned body
 */
static void orxFASTCALL orxPhysics_ApplySimulationResult(orxPHYSICS_BODY *_pstBody)
{
  orxOBJECT      *pstObject;
  orxBODY        *pstBody;
  orxFRAME       *pstFrame;
  b2BodyId        stBody;
  orxFRAME_SPACE  eFrameSpace;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxPhysics_ApplySimResult");

  /* Gets Box2D body */
  stBody = _pstBody->stBody;

  /* Gets owner body */
  pstBody = orxBODY(_pstBody->pstOwner);

  /* Gets owner object */
  pstObject = orxOBJECT(orxStructure_GetOwner(pstBody));

  /* Gets its frame */
  pstFrame = orxOBJECT_GET_STRUCTURE(pstObject, FRAME);

  /* Gets its frame space */
  eFrameSpace = (orxFrame_IsRootChild(pstFrame) != orxFALSE) ? orxFRAME_SPACE_LOCAL : orxFRAME_SPACE_GLOBAL;

  /* Is enabled? */
  if(orxObject_IsEnabled(pstObject) != orxFALSE)
  {
    orxVECTOR   vSpeed, vOldPos, vNewPos;
    b2BodyId    stBody;
    orxCLOCK   *pstClock;
    orxFLOAT    fCoef = orxFLOAT_1, fRotation;

    /* Gets its clock */
    pstClock = orxObject_GetClock(pstObject);

    /* Valid */
    if(pstClock != orxNULL)
    {
      /* Paused? */
      if(orxClock_IsPaused(pstClock) != orxFALSE)
      {
        /* Updates coef */
        fCoef = orxFLOAT_0;
      }
      else
      {
        orxFLOAT fModifier;

        /* Gets multiply modifier */
        fModifier = orxClock_GetModifier(pstClock, orxCLOCK_MODIFIER_MULTIPLY);

        /* Valid? */
        if(fModifier != orxFLOAT_0)
        {
          /* Updates coef */
          fCoef = orxFLOAT_1 / fModifier;
        }
      }
    }

    /* Gets resulting speed */
    orxPhysics_GetSpeed(_pstBody, &vSpeed);

    /* Global space? */
    if(eFrameSpace == orxFRAME_SPACE_GLOBAL)
    {
      orxVECTOR vScale;
      orxFRAME *pstParentFrame;

      /* Gets parent frame */
      pstParentFrame = orxFRAME(orxStructure_GetParent(pstFrame));

      /* Updates speed according to parent scale & rotation */
      orxVector_2DRotate(&vSpeed, &vSpeed, -orxFrame_GetRotation(pstParentFrame, orxFRAME_SPACE_GLOBAL));
      orxVector_Div(&vSpeed, &vSpeed, orxFrame_GetScale(pstParentFrame, orxFRAME_SPACE_GLOBAL, &vScale));
    }

    /* Updates its speed & angular velocity */
    orxBody_SetSpeed(pstBody, orxVector_Mulf(&vSpeed, &vSpeed, fCoef));
    orxBody_SetAngularVelocity(pstBody, fCoef * orxPhysics_GetAngularVelocity(_pstBody));

    /* Gets physics body */
    stBody = _pstBody->stBody;

    /* Gets its rotation */
    fRotation = b2Rot_GetAngle(b2Body_GetRotation(stBody)) * (orxFLOAT_1 / orxMATH_KF_2_PI);
    fRotation -= orxS2F(orxF2S(fRotation) - (orxS32)(fRotation < orxFLOAT_0));
    fRotation *= orxMATH_KF_2_PI;

    /* Should update? */
    if(orxMath_Abs(fRotation - _pstBody->fCachedRotation) > orxPHYSICS_KF_CACHED_ROTATION_EPSILON)
    {      
      /* Updates rotation */
      orxFrame_SetRotation(pstFrame, eFrameSpace, fRotation);
      _pstBody->fCachedRotation = fRotation;
    }

    /* Should interpolate? */
    if(orxFLAG_TEST(sstPhysics.u32Flags, orxPHYSICS_KU32_STATIC_FLAG_INTERPOLATE))
    {
      orxFLOAT fInterpolationCoef;

      /* Gets interpolation coef */
      fInterpolationCoef = sstPhysics.fDTAccumulator / sstPhysics.fFixedDT;

      /* Updates rotation */
      _pstBody->fInterpolatedRotation = orxLERP(_pstBody->fPreviousRotation, orxPhysics_GetRotation(_pstBody), fInterpolationCoef);
      orxFrame_SetRotation(pstFrame, eFrameSpace, _pstBody->fInterpolatedRotation);

      /* Updates position */
      orxFrame_GetPosition(pstFrame, eFrameSpace, &vOldPos);
      orxPhysics_GetPosition(_pstBody, &vNewPos);
      _pstBody->vInterpolatedPosition.fX = orxLERP(_pstBody->vPreviousPosition.fX, vNewPos.fX, fInterpolationCoef);
      _pstBody->vInterpolatedPosition.fY = orxLERP(_pstBody->vPreviousPosition.fY, vNewPos.fY, fInterpolationCoef);
      _pstBody->vInterpolatedPosition.fZ = vOldPos.fZ;
      orxFrame_SetPosition(pstFrame, eFrameSpace, &_pstBody->vInterpolatedPosition);
    }
    else
    {
      /* Updates position */
      orxFrame_GetPosition(pstFrame, eFrameSpace, &vOldPos);
      orxPhysics_GetPosition(_pstBody, &vNewPos);
      vNewPos.fZ = vOldPos.fZ;
      orxFrame_SetPosition(pstFrame, eFrameSpace, &vNewPos);
    }
  }
  else
  {
    orxVECTOR vPosition;

    /* Enforces its body properties */
    orxPhysics_SetRotation(_pstBody, orxFrame_GetRotation(pstFrame, eFrameSpace));
    orxPhysics_SetAngularVelocity(_pstBody, orxFLOAT_0);
    orxPhysics_SetPosition(_pstBody, orxFrame_GetPosition(pstFrame, eFrameSpace, &vPosition));
    orxPhysics_SetSpeed(_pstBody, &orxVECTOR_0);
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return;
}

/** Resets interpolation values
 */
static void orxFASTCALL orxPhysics_Box2D_ResetInterpolation()
{
  orxPHYSICS_BODY *pstPhysicBody;

  /* For all physical bodies */
  for(pstPhysicBody = (orxPHYSICS_BODY*)orxLinkList_GetFirst(&(sstPhysics.stBodyList));
      pstPhysicBody != orxNULL;
      pstPhysicBody = (orxPHYSICS_BODY*)orxLinkList_GetNext(&(pstPhysicBody->stNode)))
  {
    b2BodyId stBody;

    /* Gets body */
    stBody = pstPhysicBody->stBody;

    /* Non-static and awake? */
    if((b2Body_GetType(stBody) != b2_staticBody)
    && (b2Body_IsAwake(stBody) != false))
    {
      /* Resets its values */
      orxPhysics_GetPosition(pstPhysicBody, &pstPhysicBody->vPreviousPosition);
      orxVector_Copy(&pstPhysicBody->vInterpolatedPosition, &pstPhysicBody->vPreviousPosition);
      pstPhysicBody->fPreviousRotation      =
      pstPhysicBody->fInterpolatedRotation  = orxPhysics_GetRotation(pstPhysicBody);
    }
  }

  /* Done! */
  return;
}

/** Update (callback to register on a clock)
 * @param[in]   _pstClockInfo   Clock info of the clock used upon registration
 * @param[in]   _pContext       Context sent when registering callback to the clock
 */
static void orxFASTCALL orxPhysics_Box2D_Update(const orxCLOCK_INFO *_pstClockInfo, void *_pContext)
{
  orxPHYSICS_BODY *pstPhysicBody;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxPhysics_Update");

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstClockInfo != orxNULL);

  /* For all physical bodies */
  for(pstPhysicBody = (orxPHYSICS_BODY*)orxLinkList_GetFirst(&(sstPhysics.stBodyList));
      pstPhysicBody != orxNULL;
      pstPhysicBody = (orxPHYSICS_BODY*)orxLinkList_GetNext(&(pstPhysicBody->stNode)))
  {
    orxOBJECT      *pstObject;
    const orxBODY  *pstBody;
    b2BodyId        stBody;

    /* Gets Box2D body */
    stBody = pstPhysicBody->stBody;

    /* Gets associated body */
    pstBody = orxBODY(pstPhysicBody->pstOwner);

    /* Gets owner object */
    pstObject = orxOBJECT(orxStructure_GetOwner(pstBody));

    /* Is enabled? */
    if(orxObject_IsEnabled(pstObject) != orxFALSE)
    {
      orxFRAME   *pstFrame;
      orxVECTOR   vSpeed, vGravity;
      orxCLOCK   *pstClock;
      orxFLOAT    fCoef = orxFLOAT_1;

      /* Gets its clock */
      pstClock = orxObject_GetClock(pstObject);

      /* Valid */
      if(pstClock != orxNULL)
      {
        /* Paused? */
        if(orxClock_IsPaused(pstClock) != orxFALSE)
        {
          /* Updates coef */
          fCoef = orxFLOAT_0;
        }
        else
        {
          orxFLOAT fModifier;

          /* Gets multiply modifier */
          fModifier = orxClock_GetModifier(pstClock, orxCLOCK_MODIFIER_MULTIPLY);

          /* Valid? */
          if(fModifier != orxFLOAT_0)
          {
            /* Uses it */
            fCoef = fModifier;
          }
        }
      }

      /* Enforces its activation state */
      b2Body_SetAwake(stBody, true);

      /* Gets owner's frame */
      pstFrame = orxOBJECT_GET_STRUCTURE(pstObject, FRAME);

      /* Gets its body speed */
      orxBody_GetSpeed(pstBody, &vSpeed);

      /* Is not a root child? */
      if(orxFrame_IsRootChild(pstFrame) == orxFALSE)
      {
        orxVECTOR vPos, vScale;
        orxFRAME *pstParentFrame;

        /* Updates body position & rotation */
        orxPhysics_SetPosition(pstPhysicBody, orxFrame_GetPosition(pstFrame, orxFRAME_SPACE_GLOBAL, &vPos));
        orxPhysics_SetRotation(pstPhysicBody, orxFrame_GetRotation(pstFrame, orxFRAME_SPACE_GLOBAL));

        /* Gets parent frame */
        pstParentFrame = orxFRAME(orxStructure_GetParent(pstFrame));

        /* Updates speed according to parent scale & rotation */
        orxVector_2DRotate(&vSpeed, &vSpeed, orxFrame_GetRotation(pstParentFrame, orxFRAME_SPACE_GLOBAL));
        orxVector_Mul(&vSpeed, &vSpeed, orxFrame_GetScale(pstParentFrame, orxFRAME_SPACE_GLOBAL, &vScale));
      }

      /* Applies speed & angular velocity */
      orxPhysics_SetSpeed(pstPhysicBody, orxVector_Mulf(&vSpeed, &vSpeed, fCoef));
      orxPhysics_SetAngularVelocity(pstPhysicBody, fCoef * orxBody_GetAngularVelocity(pstBody));

      /* No custom gravity */
      if(orxBody_GetCustomGravity(pstBody, &vGravity) == orxNULL)
      {
        /* Uses world gravity */
        orxPhysics_GetGravity(&vGravity);
      }

      /* Applies modified gravity */
      orxPhysics_SetCustomGravity(pstPhysicBody, orxVector_Mulf(&vGravity, &vGravity, fCoef * fCoef));
    }
    else
    {
      /* Is still active? */
      if(b2Body_IsAwake(stBody) != false)
      {
        /* Deactivates it */
        b2Body_SetAwake(stBody, false);
      }
    }
  }

  /* Is simulation enabled? */
  if(orxFLAG_TEST(sstPhysics.u32Flags, orxPHYSICS_KU32_STATIC_FLAG_ENABLED))
  {
    b2BodyEvents    stBodyEvents;
    b2SensorEvents  stSensorEvents;
    b2ContactEvents stContactEvents;
    orxS32          s32Steps, i;

    /* Stores DT */
    sstPhysics.fLastDT = _pstClockInfo->fDT;

    /* Updates DT accumulator */
    sstPhysics.fDTAccumulator += _pstClockInfo->fDT;

    /* Computes the number of steps */
    s32Steps = (orxS32)orxMath_Floor((sstPhysics.fDTAccumulator + orxPHYSICS_KF_MIN_STEP_DURATION) / sstPhysics.fFixedDT);

    /* Updates accumulator */
    sstPhysics.fDTAccumulator = orxMAX(orxFLOAT_0, sstPhysics.fDTAccumulator - (orxS2F(s32Steps) * sstPhysics.fFixedDT));

    /* For all steps */
    for(i = 0; i < s32Steps; i++)
    {
      /* Last step and should interpolate? */
      if(orxFLAG_TEST(sstPhysics.u32Flags, orxPHYSICS_KU32_STATIC_FLAG_INTERPOLATE)
      && (i == s32Steps - 1))
      {
        /* Resets interpolation */
        orxPhysics_Box2D_ResetInterpolation();
      }

      /* Updates world simulation */
      b2World_Step(sstPhysics.stWorld, sstPhysics.fFixedDT, sstPhysics.s32IterationPerSteps);
    }

    /* Not absolute fixed DT? */
    if(!orxFLAG_TEST(sstPhysics.u32Flags, orxPHYSICS_KU32_STATIC_FLAG_FIXED_DT))
    {
      /* Should run a last simulation step? */
      if(sstPhysics.fDTAccumulator >= orxPHYSICS_KF_MIN_STEP_DURATION)
      {
        /* Updates last step of world simulation */
        b2World_Step(sstPhysics.stWorld, sstPhysics.fDTAccumulator, sstPhysics.s32IterationPerSteps);

        /* Clears accumulator */
        sstPhysics.fDTAccumulator = orxFLOAT_0;
      }
    }

    /* For all body events */
    stBodyEvents = b2World_GetBodyEvents(sstPhysics.stWorld);
    for(i = 0; i < stBodyEvents.moveCount; i++)
    {
      b2BodyMoveEvent *pstEvent;

      /* Gets it */
      pstEvent = &(stBodyEvents.moveEvents[i]);

      /* Valid? */
      if(b2Body_IsValid(pstEvent->bodyId) != false)
      {
        /* Non-static and awake? */
        if((b2Body_GetType(pstEvent->bodyId) != b2_staticBody)
        && (b2Body_IsAwake(pstEvent->bodyId) != false))
        {
          /* Applies simulation result */
          orxPhysics_ApplySimulationResult((orxPHYSICS_BODY *)b2Body_GetUserData(pstEvent->bodyId));
        }
      }
    }

    /* Gets sensor & contact events */
    stSensorEvents  = b2World_GetSensorEvents(sstPhysics.stWorld);
    stContactEvents = b2World_GetContactEvents(sstPhysics.stWorld);

    /* For all sensor begin events */
    for(i = 0; i < stSensorEvents.beginCount; i++)
    {
      /* Valid? */
      if((b2Shape_IsValid(stSensorEvents.beginEvents[i].sensorShapeId) != false)
      && (b2Shape_IsValid(stSensorEvents.beginEvents[i].visitorShapeId) != false))
      {
        orxS32  j;
        orxBOOL bSkip = orxFALSE;
        
        /* For all previous events */
        for(j = 0; j < i; j++)
        {
          /* Already handled? */
          if((orxMemory_Compare(&(stSensorEvents.beginEvents[i].sensorShapeId), &(stSensorEvents.beginEvents[j].visitorShapeId), sizeof(b2ShapeId)) == 0)
          && (orxMemory_Compare(&(stSensorEvents.beginEvents[i].visitorShapeId), &(stSensorEvents.beginEvents[j].sensorShapeId), sizeof(b2ShapeId)) == 0))
          {
            /* Updates status */
            bSkip = orxTRUE;
            break;
          }
        }

        /* Continue? */
        if(bSkip == orxFALSE)
        {        
          orxPHYSICS_EVENT_PAYLOAD  stPayload;
          b2BodyId                  stSender, stRecipient;

          /* Gets both bodies */
          stSender    = b2Shape_GetBody(stSensorEvents.beginEvents[i].sensorShapeId);
          stRecipient = b2Shape_GetBody(stSensorEvents.beginEvents[i].visitorShapeId);

          /* Inits event payload */
          stPayload.pstSenderPart     = (orxBODY_PART *)b2Shape_GetUserData(stSensorEvents.beginEvents[i].sensorShapeId);
          stPayload.pstRecipientPart  = (orxBODY_PART *)b2Shape_GetUserData(stSensorEvents.beginEvents[i].visitorShapeId);
          orxVector_Copy(&(stPayload.vPosition), &orxVECTOR_0);
          orxVector_Copy(&(stPayload.vNormal), &orxVECTOR_0);

          /* Sends event */
          orxEVENT_SEND(orxEVENT_TYPE_PHYSICS, orxPHYSICS_EVENT_CONTACT_ADD, orxStructure_GetOwner(orxBODY(((orxPHYSICS_BODY *)b2Body_GetUserData(stSender))->pstOwner)), orxStructure_GetOwner(orxBODY(((orxPHYSICS_BODY *)b2Body_GetUserData(stRecipient))->pstOwner)), &stPayload);
        }
      }
    }

    /* For all contact begin events */
    for(i = 0; i < stContactEvents.beginCount; i++)
    {
      /* Valid? */
      if((b2Shape_IsValid(stContactEvents.beginEvents[i].shapeIdA) != false)
      && (b2Shape_IsValid(stContactEvents.beginEvents[i].shapeIdB) != false))
      {
        orxPHYSICS_EVENT_PAYLOAD  stPayload;
        b2BodyId                  stSender, stRecipient;

        /* Gets both bodies */
        stSender    = b2Shape_GetBody(stContactEvents.beginEvents[i].shapeIdA);
        stRecipient = b2Shape_GetBody(stContactEvents.beginEvents[i].shapeIdB);

        /* Inits event payload */
        stPayload.pstSenderPart     = (orxBODY_PART *)b2Shape_GetUserData(stContactEvents.beginEvents[i].shapeIdA);
        stPayload.pstRecipientPart  = (orxBODY_PART *)b2Shape_GetUserData(stContactEvents.beginEvents[i].shapeIdB);

        /* 2 contacts? */
        if(stContactEvents.beginEvents[i].manifold.pointCount > 1)
        {
          /* Updates payload */
          orxVector_Set(&(stPayload.vPosition),
                        orx2F(0.5f) * sstPhysics.fRecDimensionRatio * (stContactEvents.beginEvents[i].manifold.points[0].point.x + stContactEvents.beginEvents[i].manifold.points[1].point.x),
                        orx2F(0.5f) * sstPhysics.fRecDimensionRatio * (stContactEvents.beginEvents[i].manifold.points[0].point.y + stContactEvents.beginEvents[i].manifold.points[1].point.y),
                        orxFLOAT_0);
          orxVector_Set(&(stPayload.vNormal), stContactEvents.beginEvents[i].manifold.normal.x, stContactEvents.beginEvents[i].manifold.normal.y, orxFLOAT_0);
        }
        /* 1 contact? */
        else if(stContactEvents.beginEvents[i].manifold.pointCount == 1)
        {
          /* Updates payload */
          orxVector_Set(&(stPayload.vPosition),
                        sstPhysics.fRecDimensionRatio * stContactEvents.beginEvents[i].manifold.points[0].point.x,
                        sstPhysics.fRecDimensionRatio * stContactEvents.beginEvents[i].manifold.points[0].point.y,
                        orxFLOAT_0);
          orxVector_Set(&(stPayload.vNormal), stContactEvents.beginEvents[i].manifold.normal.x, stContactEvents.beginEvents[i].manifold.normal.y, orxFLOAT_0);
        }
        /* 0 contact */
        else
        {
          orxVector_Copy(&(stPayload.vPosition), &orxVECTOR_0);
          orxVector_Copy(&(stPayload.vNormal), &orxVECTOR_0);
        }

        /* Sends event */
        orxEVENT_SEND(orxEVENT_TYPE_PHYSICS, orxPHYSICS_EVENT_CONTACT_ADD, orxStructure_GetOwner(orxBODY(((orxPHYSICS_BODY *)b2Body_GetUserData(stSender))->pstOwner)), orxStructure_GetOwner(orxBODY(((orxPHYSICS_BODY *)b2Body_GetUserData(stRecipient))->pstOwner)), &stPayload);
      }
    }
    
    /* For all sensor end events */
    for(i = 0; i < stSensorEvents.endCount; i++)
    {
      /* Valid? */
      if((b2Shape_IsValid(stSensorEvents.endEvents[i].sensorShapeId) != false)
      && (b2Shape_IsValid(stSensorEvents.endEvents[i].visitorShapeId) != false))
      {
        orxS32  j;
        orxBOOL bSkip = orxFALSE;
        
        /* For all previous events */
        for(j = 0; j < i; j++)
        {
          /* Already handled? */
          if((orxMemory_Compare(&(stSensorEvents.beginEvents[i].sensorShapeId), &(stSensorEvents.beginEvents[j].visitorShapeId), sizeof(b2ShapeId)) == 0)
          && (orxMemory_Compare(&(stSensorEvents.beginEvents[i].visitorShapeId), &(stSensorEvents.beginEvents[j].sensorShapeId), sizeof(b2ShapeId)) == 0))
          {
            /* Updates status */
            bSkip = orxTRUE;
            break;
          }
        }

        /* Continue? */
        if(bSkip == orxFALSE)
        {        
          orxPHYSICS_EVENT_PAYLOAD  stPayload;
          b2BodyId                  stSender, stRecipient;

          /* Gets both bodies */
          stSender    = b2Shape_GetBody(stSensorEvents.endEvents[i].sensorShapeId);
          stRecipient = b2Shape_GetBody(stSensorEvents.endEvents[i].visitorShapeId);

          /* Inits event payload */
          stPayload.pstSenderPart     = (orxBODY_PART *)b2Shape_GetUserData(stSensorEvents.endEvents[i].sensorShapeId);
          stPayload.pstRecipientPart  = (orxBODY_PART *)b2Shape_GetUserData(stSensorEvents.endEvents[i].visitorShapeId);
          orxVector_Copy(&(stPayload.vPosition), &orxVECTOR_0);
          orxVector_Copy(&(stPayload.vNormal), &orxVECTOR_0);

          /* Sends event */
          orxEVENT_SEND(orxEVENT_TYPE_PHYSICS, orxPHYSICS_EVENT_CONTACT_REMOVE, orxStructure_GetOwner(orxBODY(((orxPHYSICS_BODY *)b2Body_GetUserData(stSender))->pstOwner)), orxStructure_GetOwner(orxBODY(((orxPHYSICS_BODY *)b2Body_GetUserData(stRecipient))->pstOwner)), &stPayload);
        }
      }
    }

    /* For all contact end events */
    for(i = 0; i < stContactEvents.endCount; i++)
    {
      /* Valid? */
      if((b2Shape_IsValid(stContactEvents.endEvents[i].shapeIdA) != false)
      && (b2Shape_IsValid(stContactEvents.endEvents[i].shapeIdB) != false))
      {
        orxPHYSICS_EVENT_PAYLOAD  stPayload;
        b2BodyId                  stSender, stRecipient;

        /* Gets both bodies */
        stSender    = b2Shape_GetBody(stContactEvents.endEvents[i].shapeIdA);
        stRecipient = b2Shape_GetBody(stContactEvents.endEvents[i].shapeIdB);

        /* Inits event payload */
        stPayload.pstSenderPart     = (orxBODY_PART *)b2Shape_GetUserData(stContactEvents.endEvents[i].shapeIdA);
        stPayload.pstRecipientPart  = (orxBODY_PART *)b2Shape_GetUserData(stContactEvents.endEvents[i].shapeIdB);
        orxVector_Copy(&(stPayload.vPosition), &orxVECTOR_0);
        orxVector_Copy(&(stPayload.vNormal), &orxVECTOR_0);

        /* Sends event */
        orxEVENT_SEND(orxEVENT_TYPE_PHYSICS, orxPHYSICS_EVENT_CONTACT_REMOVE, orxStructure_GetOwner(orxBODY(((orxPHYSICS_BODY *)b2Body_GetUserData(stSender))->pstOwner)), orxStructure_GetOwner(orxBODY(((orxPHYSICS_BODY *)b2Body_GetUserData(stRecipient))->pstOwner)), &stPayload);
      }
    }
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return;
}

orxPHYSICS_BODY *orxFASTCALL orxPhysics_Box2D_CreateBody(const orxSTRUCTURE *_pstOwner, const orxBODY_DEF *_pstBodyDef)
{
  orxPHYSICS_BODY *pstResult = orxNULL;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstOwner != orxNULL);
  orxASSERT(_pstBodyDef != orxNULL);

  /* 2D? */
  if(orxFLAG_TEST(_pstBodyDef->u32Flags, orxBODY_DEF_KU32_FLAG_2D))
  {
    /* Creates physics body */
    pstResult = (orxPHYSICS_BODY*) orxBank_Allocate(sstPhysics.pstBodyBank);

    /* Valid? */
    if(pstResult != orxNULL)
    {
      b2BodyDef stBodyDef;

      /* clears it */
      orxMemory_Zero(pstResult, sizeof(orxPHYSICS_BODY));

      /* Inits body definition */
      stBodyDef                   = b2DefaultBodyDef();
      stBodyDef.userData          = (void *)pstResult;
      stBodyDef.rotation          = b2MakeRot(_pstBodyDef->fRotation);
      stBodyDef.linearDamping     = _pstBodyDef->fLinearDamping;
      stBodyDef.angularDamping    = _pstBodyDef->fAngularDamping;
      stBodyDef.isBullet          = orxFLAG_TEST(_pstBodyDef->u32Flags, orxBODY_DEF_KU32_FLAG_HIGH_SPEED);
      stBodyDef.enableSleep       = orxFLAG_TEST(_pstBodyDef->u32Flags, orxBODY_DEF_KU32_FLAG_ALLOW_SLEEP);
      stBodyDef.fixedRotation     = orxFLAG_TEST(_pstBodyDef->u32Flags, orxBODY_DEF_KU32_FLAG_FIXED_ROTATION);
      stBodyDef.position.x        = sstPhysics.fDimensionRatio * _pstBodyDef->vPosition.fX;
      stBodyDef.position.y        = sstPhysics.fDimensionRatio * _pstBodyDef->vPosition.fY;

      /* Is dynamic? */
      if(orxFLAG_TEST(_pstBodyDef->u32Flags, orxBODY_DEF_KU32_FLAG_DYNAMIC))
      {
        b2MassData  stMassData;
        orxBOOL     bHasMass;

        /* Sets its type */
        stBodyDef.type = b2_dynamicBody;

        /* Has mass data? */
        if((_pstBodyDef->fInertia > 0.0f) && (_pstBodyDef->fMass > 0.0f))
        {
          /* Stores mass properties */
          stMassData.rotationalInertia  = _pstBodyDef->fInertia;
          stMassData.mass               = _pstBodyDef->fMass;

          /* Updates status */
          bHasMass = orxTRUE;
        }
        else
        {
          /* Updates status */
          bHasMass = orxFALSE;
        }

        /* Creates dynamic body */
        pstResult->stBody = b2CreateBody(sstPhysics.stWorld, &stBodyDef);

        /* Success? */
        if(b2Body_IsValid(pstResult->stBody))
        {
          /* Has mass data? */
          if(bHasMass != orxFALSE)
          {
            /* Updates its mass data */
            b2Body_SetMassData(pstResult->stBody, stMassData);
          }
        }
      }
      else
      {
        /* Sets its type */
        stBodyDef.type = orxFLAG_TEST(_pstBodyDef->u32Flags, orxBODY_DEF_KU32_FLAG_CAN_MOVE) ? b2_kinematicBody : b2_staticBody;

        /* Creates dynamic body */
        pstResult->stBody = b2CreateBody(sstPhysics.stWorld, &stBodyDef);
      }

      /* Success? */
      if(b2Body_IsValid(pstResult->stBody))
      {
        /* Stores owner */
        pstResult->pstOwner = _pstOwner;

        /* Adds it to list */
        orxLinkList_AddEnd(&(sstPhysics.stBodyList), &(pstResult->stNode));
      }
      else
      {
        /* Deletes physics body */
        orxBank_Free(sstPhysics.pstBodyBank, pstResult);
        pstResult = orxNULL;
      }
    }
  }

  /* Done! */
  return pstResult;
}

void orxFASTCALL orxPhysics_Box2D_DeleteBody(orxPHYSICS_BODY *_pstBody)
{
  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);

  /* Deletes it */
  b2DestroyBody(_pstBody->stBody);

  /* Removes it */
  orxLinkList_Remove(&(_pstBody->stNode));
  orxBank_Free(sstPhysics.pstBodyBank, _pstBody);

  /* Done! */
  return;
}

orxPHYSICS_BODY_PART *orxFASTCALL orxPhysics_Box2D_CreatePart(orxPHYSICS_BODY *_pstBody, const orxHANDLE _hUserData, const orxBODY_PART_DEF *_pstBodyPartDef)
{
  orxPHYSICS_BODY_PART *pstResult;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);
  orxASSERT(_pstBodyPartDef != orxNULL);
  orxASSERT(orxFLAG_TEST(_pstBodyPartDef->u32Flags, orxBODY_PART_DEF_KU32_MASK_TYPE));

  /* Creates physics body */
  pstResult = (orxPHYSICS_BODY_PART *) orxBank_Allocate(sstPhysics.pstBodyPartBank);

  /* Valid? */
  if(pstResult != orxNULL)
  {
    b2BodyId stBody;

    /* Clears it */
    orxMemory_Zero(pstResult, sizeof(orxPHYSICS_BODY_PART));

    /* Gets body */
    stBody = _pstBody->stBody;

    /* Chain? */
    if(orxFLAG_TEST(_pstBodyPartDef->u32Flags, orxBODY_PART_DEF_KU32_FLAG_CHAIN))
    {
      b2ChainDef        stChainDef;
      b2SurfaceMaterial stMaterial;
      b2Vec2           *avVertexList = (b2Vec2 *)alloca((_pstBodyPartDef->stChain.u32VertexCount + 2) * sizeof(b2Vec2));
      orxU32            i;
      int               iCount = 0;

      /* Not looping and has previous (ghost) vertex? */
      if((_pstBodyPartDef->stChain.bIsLoop == orxFALSE) && (_pstBodyPartDef->stChain.bHasPrevious != orxFALSE))
      {
        /* Sets its vector */
        avVertexList[iCount].x = sstPhysics.fDimensionRatio * _pstBodyPartDef->vScale.fX * _pstBodyPartDef->stChain.vPrevious.fX;
        avVertexList[iCount].y = sstPhysics.fDimensionRatio * _pstBodyPartDef->vScale.fY * _pstBodyPartDef->stChain.vPrevious.fY;
        iCount++;
      }
      /* For all the vertices */
      for(i = 0; i < _pstBodyPartDef->stChain.u32VertexCount; i++, iCount++)
      {
        /* Sets its vector */
        avVertexList[iCount].x = sstPhysics.fDimensionRatio * _pstBodyPartDef->vScale.fX * _pstBodyPartDef->stChain.avVertices[i].fX;
        avVertexList[iCount].y = sstPhysics.fDimensionRatio * _pstBodyPartDef->vScale.fY * _pstBodyPartDef->stChain.avVertices[i].fY;
      }
      /* Not looping and has next (ghost) vertex? */
      if((_pstBodyPartDef->stChain.bIsLoop == orxFALSE) && (_pstBodyPartDef->stChain.bHasNext != orxFALSE))
      {
        /* Sets its vector */
        avVertexList[iCount].x = sstPhysics.fDimensionRatio * _pstBodyPartDef->vScale.fX * _pstBodyPartDef->stChain.vNext.fX;
        avVertexList[iCount].y = sstPhysics.fDimensionRatio * _pstBodyPartDef->vScale.fY * _pstBodyPartDef->stChain.vNext.fY;
        iCount++;
      }

      /* Inits material definition */
      stMaterial                      = b2DefaultSurfaceMaterial();
      stMaterial.friction             = _pstBodyPartDef->fFriction;
      stMaterial.restitution          = _pstBodyPartDef->fRestitution;

      /* Inits chain definition */
      stChainDef                      = b2DefaultChainDef();
      stChainDef.userData             = _hUserData;
      stChainDef.points               = avVertexList;
      stChainDef.count                = iCount;
      stChainDef.materials            = &stMaterial;
      stChainDef.materialCount        = 1;
      stChainDef.isLoop               = (_pstBodyPartDef->stChain.bIsLoop != orxFALSE) ? true : false;
      stChainDef.filter.categoryBits  = _pstBodyPartDef->u64SelfFlags;
      stChainDef.filter.maskBits      = _pstBodyPartDef->u64CheckMask;
      stChainDef.filter.groupIndex    = 0;
      stChainDef.enableSensorEvents   = true;

      /* Creates the chain */
      pstResult->stChain = b2CreateChain(stBody, &stChainDef);

      /* Updates status */
      pstResult->bIsChain = orxTRUE;
    }
    else
    {
      b2ShapeDef stShapeDef;

      /* Inits shape definition */
      stShapeDef = b2DefaultShapeDef();
      stShapeDef.userData             = _hUserData;
      stShapeDef.friction             = _pstBodyPartDef->fFriction;
      stShapeDef.restitution          = _pstBodyPartDef->fRestitution;
      stShapeDef.density              = (b2Body_GetType(stBody) != b2_dynamicBody) ? 0.0f : _pstBodyPartDef->fDensity;
      stShapeDef.filter.categoryBits  = _pstBodyPartDef->u64SelfFlags;
      stShapeDef.filter.maskBits      = _pstBodyPartDef->u64CheckMask;
      stShapeDef.filter.groupIndex    = 0;
      stShapeDef.isSensor             = orxFLAG_TEST(_pstBodyPartDef->u32Flags, orxBODY_PART_DEF_KU32_FLAG_SOLID) == orxFALSE;
      stShapeDef.enableSensorEvents   = true;
      stShapeDef.enableContactEvents  = true;
      stShapeDef.enableHitEvents      = false;

      /* Circle? */
      if(orxFLAG_TEST(_pstBodyPartDef->u32Flags, orxBODY_PART_DEF_KU32_FLAG_SPHERE))
      {
        b2Circle stCircle;

        /* Inits circle */
        stCircle.center.x = sstPhysics.fDimensionRatio * _pstBodyPartDef->stSphere.vCenter.fX * _pstBodyPartDef->vScale.fX;
        stCircle.center.y = sstPhysics.fDimensionRatio * _pstBodyPartDef->stSphere.vCenter.fY * _pstBodyPartDef->vScale.fY;
        stCircle.radius   = sstPhysics.fDimensionRatio * _pstBodyPartDef->stSphere.fRadius * orx2F(0.5f) * (orxMath_Abs(_pstBodyPartDef->vScale.fX) + orxMath_Abs(_pstBodyPartDef->vScale.fY));

        /* Creates the shape */
        pstResult->stShape = b2CreateCircleShape(stBody, &stShapeDef, &stCircle);
      }
      /* Polygon? */
      else if(orxFLAG_TEST(_pstBodyPartDef->u32Flags, orxBODY_PART_DEF_KU32_FLAG_BOX | orxBODY_PART_DEF_KU32_FLAG_MESH))
      {
        b2Polygon stPolygon;

        /* Box? */
        if(orxFLAG_TEST(_pstBodyPartDef->u32Flags, orxBODY_PART_DEF_KU32_FLAG_BOX))
        {
          /* Makes the polygon */
          stPolygon = b2MakeBox(orxMath_Abs(orx2F(0.5f) * sstPhysics.fDimensionRatio * _pstBodyPartDef->vScale.fX * (_pstBodyPartDef->stAABox.stBox.vBR.fX - _pstBodyPartDef->stAABox.stBox.vTL.fX)),
                                orxMath_Abs(orx2F(0.5f) * sstPhysics.fDimensionRatio * _pstBodyPartDef->vScale.fY * (_pstBodyPartDef->stAABox.stBox.vBR.fY - _pstBodyPartDef->stAABox.stBox.vTL.fY)));
        }
        else
        {
          b2Hull stHull;
          b2Vec2 avVertexList[B2_MAX_POLYGON_VERTICES];
          orxU32 i;

          /* Checks */
          orxASSERT(_pstBodyPartDef->stMesh.u32VertexCount > 3);
          orxASSERT(orxBODY_PART_DEF_KU32_MESH_VERTEX_NUMBER <= B2_MAX_POLYGON_VERTICES);

          /* For all the vertices */
          for(i = 0; i < _pstBodyPartDef->stMesh.u32VertexCount; i++)
          {
            /* Sets its vector */
            avVertexList[i].x = sstPhysics.fDimensionRatio * _pstBodyPartDef->stMesh.avVertices[i].fX * _pstBodyPartDef->vScale.fX;
            avVertexList[i].y = sstPhysics.fDimensionRatio * _pstBodyPartDef->stMesh.avVertices[i].fY * _pstBodyPartDef->vScale.fY;
          }

          /* Computes the hull */
          stHull = b2ComputeHull((b2Vec2 *)&avVertexList, (int)i);

          /* Makes the polygon */
          stPolygon = b2MakePolygon(&stHull, 0.0f);
        }

        /* Creates the shape */
        pstResult->stShape = b2CreatePolygonShape(stBody, &stShapeDef, &stPolygon);
      }
      /* Edge? */
      else if(orxFLAG_TEST(_pstBodyPartDef->u32Flags, orxBODY_PART_DEF_KU32_FLAG_EDGE))
      {
        b2Segment stSegment;

        /* Sets vertices */
        stSegment.point1.x  = sstPhysics.fDimensionRatio * _pstBodyPartDef->vScale.fX * _pstBodyPartDef->stEdge.avVertices[0].fX;
        stSegment.point1.y  = sstPhysics.fDimensionRatio * _pstBodyPartDef->vScale.fY * _pstBodyPartDef->stEdge.avVertices[0].fY;
        stSegment.point2.x  = sstPhysics.fDimensionRatio * _pstBodyPartDef->vScale.fX * _pstBodyPartDef->stEdge.avVertices[1].fX;
        stSegment.point2.y  = sstPhysics.fDimensionRatio * _pstBodyPartDef->vScale.fY * _pstBodyPartDef->stEdge.avVertices[1].fY;

        /* Creates the shape */
        pstResult->stShape = b2CreateSegmentShape(stBody, &stShapeDef, &stSegment);
      }
    }
    
    /* Failure? */
    if(((pstResult->bIsChain != orxFALSE) && !b2Chain_IsValid(pstResult->stChain))
    || !b2Shape_IsValid(pstResult->stShape))
    {
      /* Deletes part */
      orxBank_Free(sstPhysics.pstBodyPartBank, pstResult);
      pstResult = orxNULL;
    }
  }

  /* Done! */
  return pstResult;
}

void orxFASTCALL orxPhysics_Box2D_DeletePart(orxPHYSICS_BODY_PART *_pstBodyPart)
{
  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyPart != orxNULL);

  /* Deletes chain / shape */
  (_pstBodyPart->bIsChain != orxFALSE) ? b2DestroyChain(_pstBodyPart->stChain) : b2DestroyShape(_pstBodyPart->stShape, true);  

  /* Frees part */
  orxBank_Free(sstPhysics.pstBodyPartBank, _pstBodyPart);

  /* Done! */
  return;
}

orxPHYSICS_BODY_JOINT *orxFASTCALL orxPhysics_Box2D_CreateJoint(orxPHYSICS_BODY *_pstSrcBody, orxPHYSICS_BODY *_pstDstBody, const orxHANDLE _hUserData, const orxBODY_JOINT_DEF *_pstBodyJointDef)
{
  orxPHYSICS_BODY_JOINT *pstResult;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSrcBody != orxNULL);
  orxASSERT(_pstDstBody != orxNULL);
  orxASSERT(_hUserData != orxHANDLE_UNDEFINED);
  orxASSERT(_pstBodyJointDef != orxNULL);
  orxASSERT(orxFLAG_TEST(_pstBodyJointDef->u32Flags, orxBODY_JOINT_DEF_KU32_MASK_TYPE));

  /* Creates physics joint */
  pstResult = (orxPHYSICS_BODY_JOINT *) orxBank_Allocate(sstPhysics.pstBodyJointBank);

  /* Valid? */
  if(pstResult != orxNULL)
  {
    b2BodyId stSrcBody, stDstBody;

    /* Clears it */
    orxMemory_Zero(pstResult, sizeof(orxPHYSICS_BODY_JOINT));

    /* Gets bodies */
    stSrcBody = _pstSrcBody->stBody;
    stDstBody = _pstDstBody->stBody;

    /* Depending on joint type */
    switch(orxFLAG_GET(_pstBodyJointDef->u32Flags, orxBODY_JOINT_DEF_KU32_MASK_TYPE))
    {
      /* Revolute? */
      case orxBODY_JOINT_DEF_KU32_FLAG_REVOLUTE:
      {
        b2RevoluteJointDef stRevoluteJointDef;

        /* Inits its definition */
        stRevoluteJointDef                  = b2DefaultRevoluteJointDef();
        stRevoluteJointDef.userData         = (void *)_hUserData;
        stRevoluteJointDef.bodyIdA          = stSrcBody;
        stRevoluteJointDef.bodyIdB          = stDstBody;
        stRevoluteJointDef.localAnchorA.x   = sstPhysics.fDimensionRatio * _pstBodyJointDef->vSrcScale.fX * _pstBodyJointDef->vSrcAnchor.fX;
        stRevoluteJointDef.localAnchorA.y   = sstPhysics.fDimensionRatio * _pstBodyJointDef->vSrcScale.fY * _pstBodyJointDef->vSrcAnchor.fY;
        stRevoluteJointDef.localAnchorB.x   = sstPhysics.fDimensionRatio * _pstBodyJointDef->vDstScale.fX * _pstBodyJointDef->vDstAnchor.fX;
        stRevoluteJointDef.localAnchorB.y   = sstPhysics.fDimensionRatio * _pstBodyJointDef->vDstScale.fY * _pstBodyJointDef->vDstAnchor.fY;
        stRevoluteJointDef.collideConnected = orxFLAG_TEST(_pstBodyJointDef->u32Flags, orxBODY_JOINT_DEF_KU32_FLAG_COLLIDE) ? true : false;
        stRevoluteJointDef.referenceAngle   = _pstBodyJointDef->stRevolute.fDefaultRotation;

        /* Has rotation limits? */
        if(orxFLAG_TEST(_pstBodyJointDef->u32Flags, orxBODY_JOINT_DEF_KU32_FLAG_ROTATION_LIMIT))
        {
          /* Stores them */
          stRevoluteJointDef.lowerAngle     = _pstBodyJointDef->stRevolute.fMinRotation;
          stRevoluteJointDef.upperAngle     = _pstBodyJointDef->stRevolute.fMaxRotation;

          /* Updates status */
          stRevoluteJointDef.enableLimit    = true;
        }

        /* Is motor? */
        if(orxFLAG_TEST(_pstBodyJointDef->u32Flags, orxBODY_JOINT_DEF_KU32_FLAG_IS_MOTOR))
        {
          /* Stores them */
          stRevoluteJointDef.motorSpeed     = _pstBodyJointDef->stRevolute.fMotorSpeed;
          stRevoluteJointDef.maxMotorTorque = _pstBodyJointDef->stRevolute.fMaxMotorTorque;

          /* Updates status */
          stRevoluteJointDef.enableMotor    = true;
        }

        /* Is spring? */
        if(orxFLAG_TEST(_pstBodyJointDef->u32Flags, orxBODY_JOINT_DEF_KU32_FLAG_IS_SPRING))
        {
          /* Stores frequency */
          stRevoluteJointDef.hertz          = _pstBodyJointDef->stRevolute.fFrequency;

          /* Stores damping ratio */
          stRevoluteJointDef.dampingRatio   = _pstBodyJointDef->stRevolute.fDamping;

          /* Updates status */
          stRevoluteJointDef.enableSpring   = true;
        }

        /* Creates joint */
        pstResult->stJoint = b2CreateRevoluteJoint(sstPhysics.stWorld, &stRevoluteJointDef);

        break;
      }
      
      /* Prismatic? */
      case orxBODY_JOINT_DEF_KU32_FLAG_PRISMATIC:
      {
        b2PrismaticJointDef stPrismaticJointDef;

        /* Inits its definition */
        stPrismaticJointDef                     = b2DefaultPrismaticJointDef();
        stPrismaticJointDef.userData            = (void *)_hUserData;
        stPrismaticJointDef.bodyIdA             = stSrcBody;
        stPrismaticJointDef.bodyIdB             = stDstBody;
        stPrismaticJointDef.localAnchorA.x      = sstPhysics.fDimensionRatio * _pstBodyJointDef->vSrcScale.fX * _pstBodyJointDef->vSrcAnchor.fX;
        stPrismaticJointDef.localAnchorA.y      = sstPhysics.fDimensionRatio * _pstBodyJointDef->vSrcScale.fY * _pstBodyJointDef->vSrcAnchor.fY;
        stPrismaticJointDef.localAnchorB.x      = sstPhysics.fDimensionRatio * _pstBodyJointDef->vDstScale.fX * _pstBodyJointDef->vDstAnchor.fX;
        stPrismaticJointDef.localAnchorB.y      = sstPhysics.fDimensionRatio * _pstBodyJointDef->vDstScale.fY * _pstBodyJointDef->vDstAnchor.fY;
        stPrismaticJointDef.collideConnected    = orxFLAG_TEST(_pstBodyJointDef->u32Flags, orxBODY_JOINT_DEF_KU32_FLAG_COLLIDE) ? true : false;
        stPrismaticJointDef.referenceAngle      = _pstBodyJointDef->stPrismatic.fDefaultRotation;
        stPrismaticJointDef.localAxisA.x        = _pstBodyJointDef->stPrismatic.vTranslationAxis.fX;
        stPrismaticJointDef.localAxisA.y        = _pstBodyJointDef->stPrismatic.vTranslationAxis.fY;

        /* Has translation limits? */
        if(orxFLAG_TEST(_pstBodyJointDef->u32Flags, orxBODY_JOINT_DEF_KU32_FLAG_TRANSLATION_LIMIT))
        {
          /* Stores them */
          stPrismaticJointDef.lowerTranslation  = sstPhysics.fDimensionRatio * _pstBodyJointDef->stPrismatic.fMinTranslation;
          stPrismaticJointDef.upperTranslation  = sstPhysics.fDimensionRatio * _pstBodyJointDef->stPrismatic.fMaxTranslation;

          /* Updates status */
          stPrismaticJointDef.enableLimit       = true;
        }

        /* Is motor? */
        if(orxFLAG_TEST(_pstBodyJointDef->u32Flags, orxBODY_JOINT_DEF_KU32_FLAG_IS_MOTOR))
        {
          /* Stores them */
          stPrismaticJointDef.motorSpeed        = sstPhysics.fDimensionRatio * _pstBodyJointDef->stPrismatic.fMotorSpeed;
          stPrismaticJointDef.maxMotorForce     = _pstBodyJointDef->stPrismatic.fMaxMotorForce;

          /* Updates status */
          stPrismaticJointDef.enableMotor       = true;
        }

        /* Is spring? */
        if(orxFLAG_TEST(_pstBodyJointDef->u32Flags, orxBODY_JOINT_DEF_KU32_FLAG_IS_SPRING))
        {
          /* Stores frequency */
          stPrismaticJointDef.hertz             = _pstBodyJointDef->stPrismatic.fFrequency;

          /* Stores damping ratio */
          stPrismaticJointDef.dampingRatio      = _pstBodyJointDef->stPrismatic.fDamping;
          
          /* Updates status */
          stPrismaticJointDef.enableSpring      = true;
        }

        /* Creates joint */
        pstResult->stJoint = b2CreatePrismaticJoint(sstPhysics.stWorld, &stPrismaticJointDef);

        break;
      }

      /* Weld? */
      case orxBODY_JOINT_DEF_KU32_FLAG_WELD:
      {
        b2WeldJointDef stWeldJointDef;

        /* Inits its definition */
        stWeldJointDef                    = b2DefaultWeldJointDef();
        stWeldJointDef.userData           = (void *)_hUserData;
        stWeldJointDef.bodyIdA            = stSrcBody;
        stWeldJointDef.bodyIdB            = stDstBody;
        stWeldJointDef.localAnchorA.x     = sstPhysics.fDimensionRatio * _pstBodyJointDef->vSrcScale.fX * _pstBodyJointDef->vSrcAnchor.fX;
        stWeldJointDef.localAnchorA.y     = sstPhysics.fDimensionRatio * _pstBodyJointDef->vSrcScale.fY * _pstBodyJointDef->vSrcAnchor.fY;
        stWeldJointDef.localAnchorB.x     = sstPhysics.fDimensionRatio * _pstBodyJointDef->vDstScale.fX * _pstBodyJointDef->vDstAnchor.fX;
        stWeldJointDef.localAnchorB.y     = sstPhysics.fDimensionRatio * _pstBodyJointDef->vDstScale.fY * _pstBodyJointDef->vDstAnchor.fY;
        stWeldJointDef.collideConnected   = orxFLAG_TEST(_pstBodyJointDef->u32Flags, orxBODY_JOINT_DEF_KU32_FLAG_COLLIDE) ? true : false;
        stWeldJointDef.referenceAngle     = _pstBodyJointDef->stWeld.fDefaultRotation;
        stWeldJointDef.linearHertz        = _pstBodyJointDef->stWeld.fFrequency;
        stWeldJointDef.linearDampingRatio = _pstBodyJointDef->stWeld.fDamping;
        stWeldJointDef.angularHertz       = _pstBodyJointDef->stWeld.fAngularFrequency;
        stWeldJointDef.angularDampingRatio= _pstBodyJointDef->stWeld.fAngularDamping;

        /* Creates joint */
        pstResult->stJoint = b2CreateWeldJoint(sstPhysics.stWorld, &stWeldJointDef);

        break;
      }

      /* Distance? */
      case orxBODY_JOINT_DEF_KU32_FLAG_DISTANCE:
      {
        b2DistanceJointDef stDistanceJointDef;

        /* Inits its definition */
        stDistanceJointDef                     = b2DefaultDistanceJointDef();
        stDistanceJointDef.userData            = (void *)_hUserData;
        stDistanceJointDef.bodyIdA             = stSrcBody;
        stDistanceJointDef.bodyIdB             = stDstBody;
        stDistanceJointDef.localAnchorA.x      = sstPhysics.fDimensionRatio * _pstBodyJointDef->vSrcScale.fX * _pstBodyJointDef->vSrcAnchor.fX;
        stDistanceJointDef.localAnchorA.y      = sstPhysics.fDimensionRatio * _pstBodyJointDef->vSrcScale.fY * _pstBodyJointDef->vSrcAnchor.fY;
        stDistanceJointDef.localAnchorB.x      = sstPhysics.fDimensionRatio * _pstBodyJointDef->vDstScale.fX * _pstBodyJointDef->vDstAnchor.fX;
        stDistanceJointDef.localAnchorB.y      = sstPhysics.fDimensionRatio * _pstBodyJointDef->vDstScale.fY * _pstBodyJointDef->vDstAnchor.fY;
        stDistanceJointDef.collideConnected    = orxFLAG_TEST(_pstBodyJointDef->u32Flags, orxBODY_JOINT_DEF_KU32_FLAG_COLLIDE) ? true : false;
        stDistanceJointDef.length              = _pstBodyJointDef->stDistance.fLength;

        /* Has length limits? */
        if(orxFLAG_TEST(_pstBodyJointDef->u32Flags, orxBODY_JOINT_DEF_KU32_FLAG_LENGTH_LIMIT))
        {
          /* Stores them */
          stDistanceJointDef.minLength        = sstPhysics.fDimensionRatio * _pstBodyJointDef->stDistance.fMinLength;
          stDistanceJointDef.maxLength        = sstPhysics.fDimensionRatio * _pstBodyJointDef->stDistance.fMaxLength;

          /* Updates status */
          stDistanceJointDef.enableLimit       = true;
        }

        /* Is motor? */
        if(orxFLAG_TEST(_pstBodyJointDef->u32Flags, orxBODY_JOINT_DEF_KU32_FLAG_IS_MOTOR))
        {
          /* Stores them */
          stDistanceJointDef.motorSpeed        = sstPhysics.fDimensionRatio * _pstBodyJointDef->stDistance.fMotorSpeed;
          stDistanceJointDef.maxMotorForce     = _pstBodyJointDef->stDistance.fMaxMotorForce;

          /* Updates status */
          stDistanceJointDef.enableMotor       = true;
        }

        /* Is spring? */
        if(orxFLAG_TEST(_pstBodyJointDef->u32Flags, orxBODY_JOINT_DEF_KU32_FLAG_IS_SPRING))
        {
          /* Stores frequency */
          stDistanceJointDef.hertz             = _pstBodyJointDef->stDistance.fFrequency;

          /* Stores damping ratio */
          stDistanceJointDef.dampingRatio      = _pstBodyJointDef->stDistance.fDamping;
          
          /* Updates status */
          stDistanceJointDef.enableSpring      = true;
        }

        /* Creates joint */
        pstResult->stJoint = b2CreateDistanceJoint(sstPhysics.stWorld, &stDistanceJointDef);

        break;
      }

      /* Motor? */
      case orxBODY_JOINT_DEF_KU32_FLAG_MOTOR:
      {
        b2MotorJointDef stMotorJointDef;

        /* Inits its definition */
        stMotorJointDef                   = b2DefaultMotorJointDef();
        stMotorJointDef.userData          = (void *)_hUserData;
        stMotorJointDef.bodyIdA           = stSrcBody;
        stMotorJointDef.bodyIdB           = stDstBody;
        stMotorJointDef.collideConnected  = orxFLAG_TEST(_pstBodyJointDef->u32Flags, orxBODY_JOINT_DEF_KU32_FLAG_COLLIDE) ? true : false;
        stMotorJointDef.linearOffset.x    = sstPhysics.fDimensionRatio * _pstBodyJointDef->vSrcScale.fX * _pstBodyJointDef->vSrcAnchor.fX;
        stMotorJointDef.linearOffset.y    = sstPhysics.fDimensionRatio * _pstBodyJointDef->vSrcScale.fY * _pstBodyJointDef->vSrcAnchor.fY;
        stMotorJointDef.angularOffset     = _pstBodyJointDef->stMotor.fDefaultRotation;
        stMotorJointDef.correctionFactor  = _pstBodyJointDef->stMotor.fCorrectionFactor;
        stMotorJointDef.maxTorque         = _pstBodyJointDef->stMotor.fMaxMotorTorque;
        stMotorJointDef.maxForce          = _pstBodyJointDef->stMotor.fMaxMotorForce;

        /* Creates joint */
        pstResult->stJoint = b2CreateMotorJoint(sstPhysics.stWorld, &stMotorJointDef);

        break;
      }

      /* Wheel? */
      case orxBODY_JOINT_DEF_KU32_FLAG_WHEEL:
      {
        b2WheelJointDef stWheelJointDef;

        /* Inits its definition */
        stWheelJointDef                     = b2DefaultWheelJointDef();
        stWheelJointDef.userData            = (void *)_hUserData;
        stWheelJointDef.bodyIdA             = stSrcBody;
        stWheelJointDef.bodyIdB             = stDstBody;
        stWheelJointDef.localAnchorA.x      = sstPhysics.fDimensionRatio * _pstBodyJointDef->vSrcScale.fX * _pstBodyJointDef->vSrcAnchor.fX;
        stWheelJointDef.localAnchorA.y      = sstPhysics.fDimensionRatio * _pstBodyJointDef->vSrcScale.fY * _pstBodyJointDef->vSrcAnchor.fY;
        stWheelJointDef.localAnchorB.x      = sstPhysics.fDimensionRatio * _pstBodyJointDef->vDstScale.fX * _pstBodyJointDef->vDstAnchor.fX;
        stWheelJointDef.localAnchorB.y      = sstPhysics.fDimensionRatio * _pstBodyJointDef->vDstScale.fY * _pstBodyJointDef->vDstAnchor.fY;
        stWheelJointDef.collideConnected    = orxFLAG_TEST(_pstBodyJointDef->u32Flags, orxBODY_JOINT_DEF_KU32_FLAG_COLLIDE) ? true : false;
        stWheelJointDef.localAxisA.x        = _pstBodyJointDef->stWheel.vTranslationAxis.fX;
        stWheelJointDef.localAxisA.y        = _pstBodyJointDef->stWheel.vTranslationAxis.fY;

        /* Has translation limits? */
        if(orxFLAG_TEST(_pstBodyJointDef->u32Flags, orxBODY_JOINT_DEF_KU32_FLAG_TRANSLATION_LIMIT))
        {
          /* Stores them */
          stWheelJointDef.lowerTranslation  = sstPhysics.fDimensionRatio * _pstBodyJointDef->stWheel.fMinTranslation;
          stWheelJointDef.upperTranslation  = sstPhysics.fDimensionRatio * _pstBodyJointDef->stWheel.fMaxTranslation;

          /* Updates status */
          stWheelJointDef.enableLimit       = true;
        }

        /* Is motor? */
        if(orxFLAG_TEST(_pstBodyJointDef->u32Flags, orxBODY_JOINT_DEF_KU32_FLAG_IS_MOTOR))
        {
          /* Stores them */
          stWheelJointDef.motorSpeed        = sstPhysics.fDimensionRatio * _pstBodyJointDef->stWheel.fMotorSpeed;
          stWheelJointDef.maxMotorTorque    = _pstBodyJointDef->stWheel.fMaxMotorTorque;

          /* Updates status */
          stWheelJointDef.enableMotor       = true;
        }

        /* Is spring? */
        if(orxFLAG_TEST(_pstBodyJointDef->u32Flags, orxBODY_JOINT_DEF_KU32_FLAG_IS_SPRING))
        {
          /* Stores frequency */
          stWheelJointDef.hertz             = _pstBodyJointDef->stWheel.fFrequency;

          /* Stores damping ratio */
          stWheelJointDef.dampingRatio      = _pstBodyJointDef->stWheel.fDamping;
          
          /* Updates status */
          stWheelJointDef.enableSpring      = true;
        }

        /* Creates joint */
        pstResult->stJoint = b2CreateWheelJoint(sstPhysics.stWorld, &stWheelJointDef);

        break;
      }

      default:
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "Can't create body joint, unknown type <0x%X>.", orxFLAG_GET(_pstBodyJointDef->u32Flags, orxBODY_JOINT_DEF_KU32_MASK_TYPE));

        break;
      }
    }

    /* Failure? */
    if(!b2Joint_IsValid(pstResult->stJoint))
    {
      /* Deletes joint */
      orxBank_Free(sstPhysics.pstBodyJointBank, pstResult);
      pstResult = orxNULL;
    }
  }

  /* Done! */
  return pstResult;
}

void orxFASTCALL orxPhysics_Box2D_DeleteJoint(orxPHYSICS_BODY_JOINT *_pstBodyJoint)
{
  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyJoint != orxNULL);

  /* Deletes joint */
  b2DestroyJoint(_pstBodyJoint->stJoint);  

  /* Frees joint */
  orxBank_Free(sstPhysics.pstBodyJointBank, _pstBodyJoint);

  /* Done! */
  return;
}

void orxFASTCALL orxPhysics_Box2D_EnableMotor(orxPHYSICS_BODY_JOINT *_pstBodyJoint, orxBOOL _bEnable)
{
  b2JointId stJoint;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyJoint != orxNULL);

  /* Gets joint */
  stJoint = _pstBodyJoint->stJoint;

  /* Depending on its type */
  switch(b2Joint_GetType(stJoint))
  {
    case b2_revoluteJoint:
    {
      /* Updates joint */
      b2RevoluteJoint_EnableMotor(stJoint, (_bEnable != orxFALSE) ? true : false);
      
      break;
    }

    case b2_prismaticJoint:
    {
      /* Updates joint */
      b2PrismaticJoint_EnableMotor(stJoint, (_bEnable != orxFALSE) ? true : false);
      
      break;
    }

    case b2_distanceJoint:
    {
      /* Updates joint */
      b2DistanceJoint_EnableMotor(stJoint, (_bEnable != orxFALSE) ? true : false);
      
      break;
    }

    case b2_wheelJoint:
    {
      /* Updates joint */
      b2WheelJoint_EnableMotor(stJoint, (_bEnable != orxFALSE) ? true : false);
      
      break;
    }
    
    default:
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "Can't enable motor on this type of joint.");
      break;
    }
  }

  /* Done! */
  return;
}

void orxFASTCALL orxPhysics_Box2D_SetJointMotorSpeed(orxPHYSICS_BODY_JOINT *_pstBodyJoint, orxFLOAT _fSpeed)
{
  b2JointId stJoint;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyJoint != orxNULL);

  /* Gets joint */
  stJoint = _pstBodyJoint->stJoint;

  /* Depending on its type */
  switch(b2Joint_GetType(stJoint))
  {
    case b2_revoluteJoint:
    {
      /* Updates joint */
      b2RevoluteJoint_SetMotorSpeed(stJoint, _fSpeed);
      
      break;
    }

    case b2_prismaticJoint:
    {
      /* Updates joint */
      b2PrismaticJoint_SetMotorSpeed(stJoint, _fSpeed);
      
      break;
    }

    case b2_distanceJoint:
    {
      /* Updates joint */
      b2DistanceJoint_SetMotorSpeed(stJoint, _fSpeed);
      
      break;
    }

    case b2_wheelJoint:
    {
      /* Updates joint */
      b2WheelJoint_SetMotorSpeed(stJoint, _fSpeed);
      
      break;
    }
    
    default:
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "Can't set motor speed on this type of joint.");
      break;
    }
  }

  /* Done! */
  return;
}

void orxFASTCALL orxPhysics_Box2D_SetJointMaxMotorTorque(orxPHYSICS_BODY_JOINT *_pstBodyJoint, orxFLOAT _fMaxTorque)
{
  b2JointId stJoint;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyJoint != orxNULL);

  /* Gets joint */
  stJoint = _pstBodyJoint->stJoint;

  /* Depending on its type */
  switch(b2Joint_GetType(stJoint))
  {
    case b2_revoluteJoint:
    {
      /* Updates joint */
      b2RevoluteJoint_SetMaxMotorTorque(stJoint, _fMaxTorque);
      
      break;
    }

    case b2_wheelJoint:
    {
      /* Updates joint */
      b2WheelJoint_SetMaxMotorTorque(stJoint, _fMaxTorque);
      
      break;
    }
    
    case b2_motorJoint:
    {
      /* Updates joint */
      b2MotorJoint_SetMaxTorque(stJoint, _fMaxTorque);
      
      break;
    }

    default:
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "Can't set max motor torque on this type of joint.");
      break;
    }
  }

  /* Done! */
  return;
}

void orxFASTCALL orxPhysics_Box2D_SetJointMaxMotorForce(orxPHYSICS_BODY_JOINT *_pstBodyJoint, orxFLOAT _fMaxForce)
{
  b2JointId stJoint;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyJoint != orxNULL);

  /* Gets joint */
  stJoint = _pstBodyJoint->stJoint;

  /* Depending on its type */
  switch(b2Joint_GetType(stJoint))
  {
    case b2_prismaticJoint:
    {
      /* Updates joint */
      b2PrismaticJoint_SetMaxMotorForce(stJoint, _fMaxForce);
      
      break;
    }

    case b2_distanceJoint:
    {
      /* Updates joint */
      b2DistanceJoint_SetMaxMotorForce(stJoint, _fMaxForce);
      
      break;
    }
    
    case b2_motorJoint:
    {
      /* Updates joint */
      b2MotorJoint_SetMaxForce(stJoint, _fMaxForce);
      
      break;
    }

    default:
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "Can't set max motor force on this type of joint.");
      break;
    }
  }

  /* Done! */
  return;
}

orxVECTOR *orxFASTCALL orxPhysics_Box2D_GetJointReactionForce(const orxPHYSICS_BODY_JOINT *_pstBodyJoint, orxVECTOR *_pvForce)
{
  b2JointId stJoint;
  b2Vec2    vForce;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyJoint != orxNULL);
  orxASSERT(_pvForce != orxNULL);

  /* Gets joint */
  stJoint = _pstBodyJoint->stJoint;

  /* Gets reaction force */
  vForce = b2Joint_GetConstraintForce(stJoint);

  /* Updates result */
  orxVector_Set(_pvForce, sstPhysics.fRecDimensionRatio * orx2F(vForce.x), sstPhysics.fRecDimensionRatio * orx2F(vForce.y), orxFLOAT_0);

  /* Done! */
  return _pvForce;
}

orxFLOAT orxFASTCALL orxPhysics_Box2D_GetJointReactionTorque(const orxPHYSICS_BODY_JOINT *_pstBodyJoint)
{
  b2JointId stJoint;
  orxFLOAT  fResult;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyJoint != orxNULL);

  /* Gets joint */
  stJoint = _pstBodyJoint->stJoint;

  /* Updates result */
  fResult = b2Joint_GetConstraintTorque(stJoint);

  /* Done! */
  return fResult;
}

orxSTATUS orxFASTCALL orxPhysics_Box2D_SetPosition(orxPHYSICS_BODY *_pstBody, const orxVECTOR *_pvPosition)
{
  b2BodyId  stBody;
  b2Rot     stRotation;
  orxFLOAT  fPosX, fPosY;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);
  orxASSERT(_pvPosition != orxNULL);

  /* Gets body */
  stBody = _pstBody->stBody;

  /* Should interpolate? */
  if(orxFLAG_TEST(sstPhysics.u32Flags, orxPHYSICS_KU32_STATIC_FLAG_INTERPOLATE))
  {
    /* Updates position and rotation */
    fPosX       = _pstBody->vInterpolatedPosition.fX;
    fPosY       = _pstBody->vInterpolatedPosition.fY;
    stRotation  = b2MakeRot(_pstBody->fInterpolatedRotation);
  }
  else
  {
    b2Vec2 vPosition;

    /* Gets position */
    vPosition = b2Body_GetPosition(stBody);

    /* Updates position and rotation */
    fPosX       = vPosition.x * sstPhysics.fRecDimensionRatio;
    fPosY       = vPosition.y * sstPhysics.fRecDimensionRatio;
    stRotation  = b2Body_GetRotation(stBody);
  }

  /* Should apply? */
  if((fPosX != _pvPosition->fX) || (fPosY != _pvPosition->fY))
  {
    b2Vec2 vPosition;

    /* Sets position vector */
    vPosition.x = sstPhysics.fDimensionRatio * _pvPosition->fX;
    vPosition.y = sstPhysics.fDimensionRatio * _pvPosition->fY;

    /* Wakes up */
    b2Body_SetAwake(stBody, true);

    /* Updates its position & rotation */
    b2Body_SetTransform(stBody, vPosition, stRotation);

    /* Should interpolate? */
    if(orxFLAG_TEST(sstPhysics.u32Flags, orxPHYSICS_KU32_STATIC_FLAG_INTERPOLATE))
    {
      /* Updates position */
      orxVector_Copy(&_pstBody->vPreviousPosition, _pvPosition);
      orxVector_Copy(&_pstBody->vInterpolatedPosition, _pvPosition);
    }
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxPhysics_Box2D_SetRotation(orxPHYSICS_BODY *_pstBody, orxFLOAT _fRotation)
{
  orxFLOAT  fRotation;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);

  /* Applies circular clamp to rotation */
  _fRotation *= orxFLOAT_1 / orxMATH_KF_2_PI;
  _fRotation -= orxS2F(orxF2S(_fRotation) - (orxS32)(_fRotation < orxFLOAT_0));
  _fRotation *= orxMATH_KF_2_PI;

  /* Gets current rotation */
  fRotation = orxFLAG_TEST(sstPhysics.u32Flags, orxPHYSICS_KU32_STATIC_FLAG_INTERPOLATE) ? _pstBody->fInterpolatedRotation : _pstBody->fCachedRotation;

  /* Should apply? */
  if(_fRotation != fRotation)
  {
    b2BodyId  stBody;
    b2Rot     stRotation;

    /* Gets body */
    stBody = _pstBody->stBody;

    /* Wakes up */
    b2Body_SetAwake(stBody, true);

    /* Gets new rotation */
    stRotation = b2MakeRot(_fRotation);

    /* Should interpolate? */
    if(orxFLAG_TEST(sstPhysics.u32Flags, orxPHYSICS_KU32_STATIC_FLAG_INTERPOLATE))
    {
      b2Vec2 vPosition;

      /* Sets current position */
      vPosition.x = sstPhysics.fDimensionRatio * _pstBody->vInterpolatedPosition.fX;
      vPosition.y = sstPhysics.fDimensionRatio * _pstBody->vInterpolatedPosition.fY;

      /* Updates its rotation */
      b2Body_SetTransform(stBody, vPosition, stRotation);

      /* Updates interpolated rotation */
      _pstBody->fPreviousRotation     = _fRotation;
      _pstBody->fInterpolatedRotation = _fRotation;
    }
    else
    {
      /* Updates its rotation */
      b2Body_SetTransform(stBody, b2Body_GetPosition(stBody), stRotation);
    }
    
    /* Updates cached value */
    _pstBody->fCachedRotation = _fRotation;
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxPhysics_Box2D_SetSpeed(orxPHYSICS_BODY *_pstBody, const orxVECTOR *_pvSpeed)
{
  b2BodyId  stBody;
  b2Vec2    vSpeed;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);
  orxASSERT(_pvSpeed != orxNULL);

  /* Gets body */
  stBody = _pstBody->stBody;

  /* Gets its speed */
  const b2Vec2 vCurrentSpeed = b2Body_GetLinearVelocity(stBody);

  /* Sets speed vector */
  vSpeed.x = sstPhysics.fDimensionRatio * _pvSpeed->fX;
  vSpeed.y = sstPhysics.fDimensionRatio * _pvSpeed->fY;

  /* Should apply? */
  if((vCurrentSpeed.x != vSpeed.x) || (vCurrentSpeed.y != vSpeed.y))
  {
    /* Wakes up */
    b2Body_SetAwake(stBody, true);

    /* Updates its speed */
    b2Body_SetLinearVelocity(stBody, vSpeed);
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxPhysics_Box2D_SetAngularVelocity(orxPHYSICS_BODY *_pstBody, orxFLOAT _fVelocity)
{
  b2BodyId  stBody;
  float     fAngularVelocity;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);

  /* Gets body */
  stBody = _pstBody->stBody;

  /* Gets its angular velocity */
  fAngularVelocity = b2Body_GetAngularVelocity(stBody);

  /* Should apply? */
  if(fAngularVelocity != _fVelocity)
  {
    /* Wakes up */
    b2Body_SetAwake(stBody, true);

    /* Updates its angular velocity */
    b2Body_SetAngularVelocity(stBody, _fVelocity);
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxPhysics_Box2D_SetCustomGravity(orxPHYSICS_BODY *_pstBody, const orxVECTOR *_pvCustomGravity)
{
  b2BodyId      stBody;
  const b2Vec2 *pvCustomGravity;
  orxSTATUS     eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);

  /* Gets body */
  stBody = _pstBody->stBody;

  /* Gets its custom gravity */
  pvCustomGravity = b2Body_GetCustomGravity(stBody);;

  /* Has new custom gravity? */
  if(_pvCustomGravity != orxNULL)
  {
    b2Vec2 vGravity;

    /* Sets gravity vector */
    vGravity.x = sstPhysics.fDimensionRatio * _pvCustomGravity->fX;
    vGravity.y = sstPhysics.fDimensionRatio * _pvCustomGravity->fY;

    /* Should apply? */
    if((pvCustomGravity == NULL) || (pvCustomGravity->x != vGravity.x) || (pvCustomGravity->y != vGravity.y))
    {
      /* Wakes up */
      b2Body_SetAwake(stBody, true);

      /* Updates it */
      b2Body_SetCustomGravity(stBody, &vGravity);
    }
  }
  else
  {
    /* Should apply */
    if(pvCustomGravity != NULL)
    {
      /* Wakes up */
      b2Body_SetAwake(stBody, true);

      /* Removes it */
      b2Body_SetCustomGravity(stBody, NULL);
    }
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxPhysics_Box2D_SetFixedRotation(orxPHYSICS_BODY * _pstBody, orxBOOL _bFixed)
{
  b2BodyId  stBody;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);

  /* Gets body */
  stBody = _pstBody->stBody;

  /* Updates its fixed rotation property */
  b2Body_SetFixedRotation(stBody, (_bFixed != orxFALSE) ? true : false);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxPhysics_Box2D_SetDynamic(orxPHYSICS_BODY * _pstBody, orxBOOL _bDynamic)
{
  b2BodyId  stBody;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);

  /* Gets body */
  stBody = _pstBody->stBody;

  /* Dynamic? */
  if(_bDynamic != orxFALSE)
  {
    /* Updates its type */
    b2Body_SetType(stBody, b2_dynamicBody);
  }
  else
  {
    const orxBODY *pstBody;

    /* Gets associated body */
    pstBody = orxBODY(_pstBody->pstOwner);

    /* Updates its type */
    b2Body_SetType(stBody, orxBody_GetAllowMoving(pstBody) != orxFALSE ? b2_kinematicBody : b2_staticBody);
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxPhysics_Box2D_SetAllowMoving(orxPHYSICS_BODY * _pstBody, orxBOOL _bAllowMoving)
{
  b2BodyId  stBody;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);

  /* Gets body */
  stBody = _pstBody->stBody;

  /* Not dynamic? */
  if(b2Body_GetType(stBody) != b2_dynamicBody)
  {
    /* Updates its type */
    b2Body_SetType(stBody, _bAllowMoving != orxFALSE ? b2_kinematicBody : b2_staticBody);
  }

  /* Done! */
  return eResult;
}

orxVECTOR *orxFASTCALL orxPhysics_Box2D_GetPosition(const orxPHYSICS_BODY *_pstBody, orxVECTOR *_pvPosition)
{
  b2BodyId    stBody;
  b2Vec2      vPosition;
  orxVECTOR  *pvResult;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);
  orxASSERT(_pvPosition != orxNULL);

  /* Gets body */
  stBody = _pstBody->stBody;

  /* Gets its position */
  vPosition = b2Body_GetPosition(stBody);

  /* Updates result */
  pvResult      = _pvPosition;
  pvResult->fX  = sstPhysics.fRecDimensionRatio * vPosition.x;
  pvResult->fY  = sstPhysics.fRecDimensionRatio * vPosition.y;

  /* Done! */
  return pvResult;
}

orxFLOAT orxFASTCALL orxPhysics_Box2D_GetRotation(const orxPHYSICS_BODY *_pstBody)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);

  /* Gets its rotation */
  fResult = _pstBody->fCachedRotation;

  /* Done! */
  return fResult;
}

orxVECTOR *orxFASTCALL orxPhysics_Box2D_GetSpeed(const orxPHYSICS_BODY *_pstBody, orxVECTOR *_pvSpeed)
{
  b2BodyId    stBody;
  b2Vec2      vSpeed;
  orxVECTOR  *pvResult;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);
  orxASSERT(_pvSpeed != orxNULL);

  /* Gets body */
  stBody = _pstBody->stBody;

  /* Gets its speed */
  vSpeed = b2Body_GetLinearVelocity(stBody);

  /* Updates result */
  pvResult      = _pvSpeed;
  pvResult->fX  = sstPhysics.fRecDimensionRatio * vSpeed.x;
  pvResult->fY  = sstPhysics.fRecDimensionRatio * vSpeed.y;
  pvResult->fZ  = orxFLOAT_0;

  /* Done! */
  return pvResult;
}

orxVECTOR *orxFASTCALL orxPhysics_Box2D_GetSpeedAtWorldPosition(const orxPHYSICS_BODY *_pstBody, const orxVECTOR *_pvPosition, orxVECTOR *_pvSpeed)
{
  b2BodyId  stBody;
  b2Vec2    vSpeed, vPosition;
  orxVECTOR *pvResult;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);
  orxASSERT(_pvPosition != orxNULL);
  orxASSERT(_pvSpeed != orxNULL);

  /* Gets body */
  stBody = _pstBody->stBody;

  /* Gets its speed at given position */
  vPosition.x = sstPhysics.fDimensionRatio * _pvPosition->fX;
  vPosition.y = sstPhysics.fDimensionRatio * _pvPosition->fY;
  vSpeed = b2Body_GetLocalPointVelocity(stBody, b2Body_GetLocalPoint(stBody, vPosition));

  /* Updates result */
  pvResult      = _pvSpeed;
  pvResult->fX  = sstPhysics.fRecDimensionRatio * vSpeed.x;
  pvResult->fY  = sstPhysics.fRecDimensionRatio * vSpeed.y;
  pvResult->fZ  = orxFLOAT_0;

  /* Done! */
  return pvResult;
}

orxFLOAT orxFASTCALL orxPhysics_Box2D_GetAngularVelocity(const orxPHYSICS_BODY *_pstBody)
{
  b2BodyId  stBody;
  orxFLOAT  fResult;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);

  /* Gets body */
  stBody = _pstBody->stBody;

  /* Gets its angular velocity */
  fResult = b2Body_GetAngularVelocity(stBody);

  /* Done! */
  return fResult;
}

orxVECTOR *orxFASTCALL orxPhysics_Box2D_GetCustomGravity(const orxPHYSICS_BODY *_pstBody, orxVECTOR *_pvCustomGravity)
{
  b2BodyId      stBody;
  const b2Vec2 *pvGravity;
  orxVECTOR    *pvResult;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);
  orxASSERT(_pvCustomGravity != orxNULL);

  /* Gets body */
  stBody = _pstBody->stBody;

  /* Gets its custom gravity */
  pvGravity = b2Body_GetCustomGravity(stBody);

  /* Found? */
  if(pvGravity != orxNULL)
  {
    /* Updates result */
    orxVector_Set(_pvCustomGravity, sstPhysics.fRecDimensionRatio * pvGravity->x, sstPhysics.fRecDimensionRatio * pvGravity->y, orxFLOAT_0);
    pvResult = _pvCustomGravity;
  }
  else
  {
    /* Clears result */
    pvResult = orxNULL;
  }

  /* Done! */
  return pvResult;
}

orxBOOL orxFASTCALL orxPhysics_Box2D_IsFixedRotation(const orxPHYSICS_BODY * _pstBody)
{
  b2BodyId  stBody;
  orxBOOL   bResult;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);

  /* Gets body */
  stBody = _pstBody->stBody;

  /* Updates result */
  bResult = (b2Body_IsFixedRotation(stBody) != false) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

orxFLOAT orxFASTCALL orxPhysics_Box2D_GetMass(const orxPHYSICS_BODY *_pstBody)
{
  b2BodyId  stBody;
  orxFLOAT  fResult;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);

  /* Gets body */
  stBody = _pstBody->stBody;

  /* Updates result */
  fResult = orx2F(b2Body_GetMass(stBody));

  /* Done! */
  return fResult;
}

orxVECTOR *orxFASTCALL orxPhysics_Box2D_GetMassCenter(const orxPHYSICS_BODY *_pstBody, orxVECTOR *_pvMassCenter)
{
  b2BodyId    stBody;
  b2Vec2      vMassCenter;
  orxVECTOR  *pvResult;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);
  orxASSERT(_pvMassCenter != orxNULL);

  /* Gets body */
  stBody = _pstBody->stBody;

  /* Gets its mass center */
  vMassCenter = b2Body_GetLocalCenterOfMass(stBody);

  /* Transfer values */
  _pvMassCenter->fX = sstPhysics.fRecDimensionRatio * vMassCenter.x;
  _pvMassCenter->fY = sstPhysics.fRecDimensionRatio * vMassCenter.y;
  _pvMassCenter->fZ = orxFLOAT_0;

  /* Updates result */
  pvResult = _pvMassCenter;

  /* Done! */
  return pvResult;
}

orxSTATUS orxFASTCALL orxPhysics_Box2D_SetLinearDamping(orxPHYSICS_BODY *_pstBody, orxFLOAT _fDamping)
{
  b2BodyId  stBody;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);

  /* Gets body */
  stBody = _pstBody->stBody;

  /* Sets its linear damping */
  b2Body_SetLinearDamping(stBody, _fDamping);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxPhysics_Box2D_SetAngularDamping(orxPHYSICS_BODY *_pstBody, orxFLOAT _fDamping)
{
  b2BodyId  stBody;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);

  /* Gets body */
  stBody = _pstBody->stBody;

  /* Sets its angular damping */
  b2Body_SetAngularDamping(stBody, _fDamping);

  /* Done! */
  return eResult;
}

orxFLOAT orxFASTCALL orxPhysics_Box2D_GetLinearDamping(const orxPHYSICS_BODY *_pstBody)
{
  b2BodyId  stBody;
  orxFLOAT  fResult;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);

  /* Gets body */
  stBody = _pstBody->stBody;

  /* Gets its linear damping */
  fResult = orx2F(b2Body_GetLinearDamping(stBody));

  /* Done! */
  return fResult;
}

orxFLOAT orxFASTCALL orxPhysics_Box2D_GetAngularDamping(const orxPHYSICS_BODY *_pstBody)
{
  b2BodyId  stBody;
  orxFLOAT  fResult;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);

  /* Gets body */
  stBody = _pstBody->stBody;

  /* Gets its angular damping */
  fResult = orx2F(b2Body_GetAngularDamping(stBody));

  /* Done! */
  return fResult;
}

orxSTATUS orxFASTCALL orxPhysics_Box2D_ApplyTorque(orxPHYSICS_BODY *_pstBody, orxFLOAT _fTorque)
{
  b2BodyId    stBody;
  orxOBJECT  *pstObject;
  float       fTorque = (float)_fTorque;
  orxSTATUS   eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);

  /* Gets body */
  stBody = _pstBody->stBody;

  /* Gets owner object */
  pstObject = orxOBJECT(orxStructure_GetOwner(_pstBody->pstOwner));

  /* Is enabled? */
  if((pstObject != orxNULL) && (orxObject_IsEnabled(pstObject) != orxFALSE))
  {
    orxCLOCK *pstClock;

    /* Gets its clock */
    pstClock = orxObject_GetClock(pstObject);

    /* Valid */
    if(pstClock != orxNULL)
    {
      /* Paused? */
      if(orxClock_IsPaused(pstClock) != orxFALSE)
      {
        /* Updates torque */
        fTorque = orxFLOAT_0;
      }
      else
      {
        orxFLOAT fModifier;

        /* Gets multiply modifier */
        fModifier = orxClock_GetModifier(pstClock, orxCLOCK_MODIFIER_MULTIPLY);

        /* Valid? */
        if(fModifier != orxFLOAT_0)
        {
          /* Updates torque */
          fTorque *= (float)fModifier;
        }
      }
    }
  }

  /* Applies torque */
  b2Body_ApplyTorque(stBody, fTorque, true);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxPhysics_Box2D_ApplyForce(orxPHYSICS_BODY *_pstBody, const orxVECTOR *_pvForce, const orxVECTOR *_pvPoint)
{
  b2BodyId    stBody;
  orxOBJECT  *pstObject;
  b2Vec2      vForce, vPoint;
  orxSTATUS   eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);
  orxASSERT(_pvForce != orxNULL);

  /* Sets force */
  vForce.x = sstPhysics.fDimensionRatio * _pvForce->fX;
  vForce.y = sstPhysics.fDimensionRatio * _pvForce->fY;

  /* Gets body */
  stBody = _pstBody->stBody;

  /* Gets owner object */
  pstObject = orxOBJECT(orxStructure_GetOwner(_pstBody->pstOwner));

  /* Is enabled? */
  if((pstObject != orxNULL) && (orxObject_IsEnabled(pstObject) != orxFALSE))
  {
    orxCLOCK *pstClock;

    /* Gets its clock */
    pstClock = orxObject_GetClock(pstObject);

    /* Valid */
    if(pstClock != orxNULL)
    {
      /* Paused? */
      if(orxClock_IsPaused(pstClock) != orxFALSE)
      {
        /* Updates force */
        vForce.x = vForce.y = (float)orxFLOAT_0;
      }
      else
      {
        orxFLOAT fModifier;

        /* Gets multiply modifier */
        fModifier = orxClock_GetModifier(pstClock, orxCLOCK_MODIFIER_MULTIPLY);

        /* Valid? */
        if(fModifier != orxFLOAT_0)
        {
          /* Updates force */
          vForce.x *= (float)(fModifier * fModifier);
          vForce.y *= (float)(fModifier * fModifier);
        }
      }
    }
  }

  /* Has point? */
  if(_pvPoint != orxNULL)
  {
    /* Sets point */
    vPoint.x = sstPhysics.fDimensionRatio * _pvPoint->fX;
    vPoint.y = sstPhysics.fDimensionRatio * _pvPoint->fY;
  }
  else
  {
    /* Gets world mass center */
    vPoint = b2Body_GetWorldCenterOfMass(stBody);
  }

  /* Applies force */
  b2Body_ApplyForce(stBody, vForce, vPoint, true);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxPhysics_Box2D_ApplyImpulse(orxPHYSICS_BODY *_pstBody, const orxVECTOR *_pvImpulse, const orxVECTOR *_pvPoint)
{
  b2BodyId    stBody;
  orxOBJECT  *pstObject;
  b2Vec2      vImpulse, vPoint;
  orxSTATUS   eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);
  orxASSERT(_pvImpulse != orxNULL);

  /* Sets impulse */
  vImpulse.x = sstPhysics.fDimensionRatio * _pvImpulse->fX;
  vImpulse.y = sstPhysics.fDimensionRatio * _pvImpulse->fY;

  /* Gets body */
  stBody = _pstBody->stBody;

  /* Gets owner object */
  pstObject = orxOBJECT(orxStructure_GetOwner(_pstBody->pstOwner));

  /* Is enabled? */
  if((pstObject != orxNULL) && (orxObject_IsEnabled(pstObject) != orxFALSE))
  {
    orxCLOCK *pstClock;

    /* Gets its clock */
    pstClock = orxObject_GetClock(pstObject);

    /* Valid */
    if(pstClock != orxNULL)
    {
      /* Paused? */
      if(orxClock_IsPaused(pstClock) != orxFALSE)
      {
        /* Updates impulse */
        vImpulse.x = vImpulse.y = (float)orxFLOAT_0;
      }
      else
      {
        orxFLOAT fModifier;

        /* Gets multiply modifier */
        fModifier = orxClock_GetModifier(pstClock, orxCLOCK_MODIFIER_MULTIPLY);

        /* Valid? */
        if(fModifier != orxFLOAT_0)
        {
          /* Updates impulse */
          vImpulse.x *= (float)fModifier;
          vImpulse.y *= (float)fModifier;
        }
      }
    }
  }

  /* Has point? */
  if(_pvPoint != orxNULL)
  {
    /* Sets point */
    vPoint.x = sstPhysics.fDimensionRatio * _pvPoint->fX;
    vPoint.y = sstPhysics.fDimensionRatio * _pvPoint->fY;
  }
  else
  {
    /* Gets world mass center */
    vPoint = b2Body_GetWorldCenterOfMass(stBody);
  }

  /* Applies force */
  b2Body_ApplyLinearImpulse(stBody, vImpulse, vPoint, true);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxPhysics_Box2D_SetPartSelfFlags(orxPHYSICS_BODY_PART *_pstBodyPart, orxU64 _u64SelfFlags)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyPart != orxNULL);

  /* Chain? */
  if(_pstBodyPart->bIsChain != orxFALSE)
  {
    b2ChainId stChain;
    b2Filter  stFilter;

    /* Gets chain */
    stChain = _pstBodyPart->stChain;

    /* Gets its current filter */
    stFilter = b2Chain_GetFilter(stChain);

    /* Updates it */
    stFilter.categoryBits = _u64SelfFlags;

    /* Sets new filter */
    b2Chain_SetFilter(stChain, stFilter);
  }
  else
  {
    b2ShapeId stShape;
    b2Filter  stFilter;

    /* Gets shape */
    stShape = _pstBodyPart->stShape;

    /* Gets its current filter */
    stFilter = b2Shape_GetFilter(stShape);

    /* Updates it */
    stFilter.categoryBits = _u64SelfFlags;

    /* Sets new filter */
    b2Shape_SetFilter(stShape, stFilter);
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxPhysics_Box2D_SetPartCheckMask(orxPHYSICS_BODY_PART *_pstBodyPart, orxU64 _u64CheckMask)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyPart != orxNULL);

  /* Chain? */
  if(_pstBodyPart->bIsChain != orxFALSE)
  {
    b2ChainId stChain;
    b2Filter  stFilter;

    /* Gets chain */
    stChain = _pstBodyPart->stChain;

    /* Gets its current filter */
    stFilter = b2Chain_GetFilter(stChain);

    /* Updates it */
    stFilter.maskBits = _u64CheckMask;

    /* Sets new filter */
    b2Chain_SetFilter(stChain, stFilter);
  }
  else
  {
    b2ShapeId stShape;
    b2Filter  stFilter;

    /* Gets shape */
    stShape = _pstBodyPart->stShape;

    /* Gets its current filter */
    stFilter = b2Shape_GetFilter(stShape);

    /* Updates it */
    stFilter.maskBits = _u64CheckMask;

    /* Sets new filter */
    b2Shape_SetFilter(stShape, stFilter);
  }

  /* Done! */
  return eResult;
}

orxU64 orxFASTCALL orxPhysics_Box2D_GetPartSelfFlags(const orxPHYSICS_BODY_PART *_pstBodyPart)
{
  orxU64 u64Result = 0;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyPart != orxNULL);

  /* Chain? */
  if(_pstBodyPart->bIsChain != orxFALSE)
  {
    b2ChainId stChain;

    /* Gets chain */
    stChain = _pstBodyPart->stChain;

    /* Updates result */
    u64Result = b2Chain_GetFilter(stChain).categoryBits;
  }
  else
  {
    b2ShapeId stShape;

    /* Gets shape */
    stShape = _pstBodyPart->stShape;

    /* Updates result */
    u64Result = b2Shape_GetFilter(stShape).categoryBits;
  }

  /* Done! */
  return u64Result;
}

orxU64 orxFASTCALL orxPhysics_Box2D_GetPartCheckMask(const orxPHYSICS_BODY_PART *_pstBodyPart)
{
  orxU64 u64Result = 0;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyPart != orxNULL);

  /* Chain? */
  if(_pstBodyPart->bIsChain != orxFALSE)
  {
    b2ChainId stChain;

    /* Gets chain */
    stChain = _pstBodyPart->stChain;

    /* Updates result */
    u64Result = b2Chain_GetFilter(stChain).maskBits;
  }
  else
  {
    b2ShapeId stShape;

    /* Gets shape */
    stShape = _pstBodyPart->stShape;

    /* Updates result */
    u64Result = b2Shape_GetFilter(stShape).maskBits;
  }

  /* Done! */
  return u64Result;
}

orxSTATUS orxFASTCALL orxPhysics_Box2D_SetPartSolid(orxPHYSICS_BODY_PART *_pstBodyPart, orxBOOL _bSolid)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyPart != orxNULL);

  /* Chain? */
  if(_pstBodyPart->bIsChain != orxFALSE)
  {
    b2ChainId stChain;

    /* Gets chain */
    stChain = _pstBodyPart->stChain;

    //! TODO
  }
  else
  {
    b2ShapeId stShape;

    /* Gets shape */
    stShape = _pstBodyPart->stShape;

    //! TODO
  }

  /* Done! */
  return eResult;
}

orxBOOL orxFASTCALL orxPhysics_Box2D_IsPartSolid(const orxPHYSICS_BODY_PART *_pstBodyPart)
{
  b2ShapeId stShape;
  orxBOOL   bResult;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyPart != orxNULL);

  /* Gets shape */
  stShape = _pstBodyPart->stShape;

  /* Updates result */
  bResult = ((_pstBodyPart->bIsChain == orxFALSE) && b2Shape_IsSensor(stShape)) ? orxFALSE : orxTRUE;

  /* Done! */
  return bResult;
}

orxSTATUS orxFASTCALL orxPhysics_Box2D_SetPartFriction(orxPHYSICS_BODY_PART *_pstBodyPart, orxFLOAT _fFriction)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyPart != orxNULL);

  /* Chain? */
  if(_pstBodyPart->bIsChain != orxFALSE)
  {
    b2ChainId stChain;

    /* Gets chain */
    stChain = _pstBodyPart->stChain;

    /* Updates it */
    b2Chain_SetFriction(stChain, _fFriction);
  }
  else
  {
    b2ShapeId stShape;

    /* Gets shape */
    stShape = _pstBodyPart->stShape;

    /* Updates it */
    b2Shape_SetFriction(stShape, _fFriction);
  }

  /* Done! */
  return eResult;
}

orxFLOAT orxFASTCALL orxPhysics_Box2D_GetPartFriction(const orxPHYSICS_BODY_PART *_pstBodyPart)
{
  orxFLOAT fResult = orxFLOAT_0;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyPart != orxNULL);

  /* Chain? */
  if(_pstBodyPart->bIsChain != orxFALSE)
  {
    b2ChainId stChain;

    /* Gets chain */
    stChain = _pstBodyPart->stChain;

    /* Updates result */
    fResult = orx2F(b2Chain_GetFriction(stChain));
  }
  else
  {
    b2ShapeId stShape;

    /* Gets shape */
    stShape = _pstBodyPart->stShape;

    /* Updates result */
    fResult = orx2F(b2Shape_GetFriction(stShape));
  }

  /* Done! */
  return fResult;
}

orxSTATUS orxFASTCALL orxPhysics_Box2D_SetPartRestitution(orxPHYSICS_BODY_PART *_pstBodyPart, orxFLOAT _fRestitution)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyPart != orxNULL);

  /* Chain? */
  if(_pstBodyPart->bIsChain != orxFALSE)
  {
    b2ChainId stChain;

    /* Gets chain */
    stChain = _pstBodyPart->stChain;

    /* Updates it */
    b2Chain_SetRestitution(stChain, _fRestitution);
  }
  else
  {
    b2ShapeId stShape;

    /* Gets shape */
    stShape = _pstBodyPart->stShape;

    /* Updates it */
    b2Shape_SetRestitution(stShape, _fRestitution);
  }

  /* Done! */
  return eResult;
}

orxFLOAT orxFASTCALL orxPhysics_Box2D_GetPartRestitution(const orxPHYSICS_BODY_PART *_pstBodyPart)
{
  orxFLOAT fResult = orxFLOAT_0;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyPart != orxNULL);

  /* Chain? */
  if(_pstBodyPart->bIsChain != orxFALSE)
  {
    b2ChainId stChain;

    /* Gets chain */
    stChain = _pstBodyPart->stChain;

    /* Updates result */
    fResult = orx2F(b2Chain_GetRestitution(stChain));
  }
  else
  {
    b2ShapeId stShape;

    /* Gets shape */
    stShape = _pstBodyPart->stShape;

    /* Updates result */
    fResult = orx2F(b2Shape_GetRestitution(stShape));
  }

  /* Done! */
  return fResult;
}

orxSTATUS orxFASTCALL orxPhysics_Box2D_SetPartDensity(orxPHYSICS_BODY_PART *_pstBodyPart, orxFLOAT _fDensity)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyPart != orxNULL);

  /* Chain? */
  if(_pstBodyPart->bIsChain != orxFALSE)
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }
  else
  {
    b2ShapeId stShape;

    /* Gets shape */
    stShape = _pstBodyPart->stShape;

    /* Updates it */
    b2Shape_SetDensity(stShape, _fDensity, true);
  }

  /* Done! */
  return eResult;
}

orxFLOAT orxFASTCALL orxPhysics_Box2D_GetPartDensity(const orxPHYSICS_BODY_PART *_pstBodyPart)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyPart != orxNULL);

  /* Chain? */
  if(_pstBodyPart->bIsChain != orxFALSE)
  {
    /* Updates result */
    fResult = orxFLOAT_0;
  }
  else
  {
    b2ShapeId stShape;

    /* Gets shape */
    stShape = _pstBodyPart->stShape;

    /* Updates result */
    fResult = orx2F(b2Shape_GetDensity(stShape));
  }

  /* Done! */
  return fResult;
}

orxBOOL orxFASTCALL orxPhysics_Box2D_IsInsidePart(const orxPHYSICS_BODY_PART *_pstBodyPart, const orxVECTOR *_pvPosition)
{
  orxBOOL bResult = orxFALSE;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyPart != orxNULL);

  /* Not a chain? */
  if(_pstBodyPart->bIsChain == orxFALSE)
  {
    b2ShapeId stShape;
    b2Vec2    vPosition;

    /* Gets shape */
    stShape = _pstBodyPart->stShape;

    /* Sets position */
    vPosition.x = sstPhysics.fDimensionRatio * _pvPosition->fX;
    vPosition.y = sstPhysics.fDimensionRatio * _pvPosition->fY;

    /* Updates result */
    bResult = (b2Shape_TestPoint(stShape, vPosition) != false) ? orxTRUE : orxFALSE;
  }

  /* Done! */
  return bResult;
}

orxHANDLE orxFASTCALL orxPhysics_Box2D_Raycast(const orxVECTOR *_pvBegin, const orxVECTOR *_pvEnd, orxU64 _u64SelfFlags, orxU64 _u64CheckMask, orxBOOL _bEarlyExit, orxVECTOR *_pvContact, orxVECTOR *_pvNormal)
{
  orxPHYSICS_RAYCAST  stRayCast;
  b2QueryFilter       stFilter;
  b2Vec2              vOrigin, vRay;
  orxHANDLE           hResult = orxHANDLE_UNDEFINED;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvBegin != orxNULL);
  orxASSERT(_pvEnd != orxNULL);

  /* Gets extremities */
  vOrigin.x = sstPhysics.fDimensionRatio * _pvBegin->fX;
  vOrigin.y = sstPhysics.fDimensionRatio * _pvBegin->fY;
  vRay.x    = sstPhysics.fDimensionRatio * _pvEnd->fX - vOrigin.x;
  vRay.y    = sstPhysics.fDimensionRatio * _pvEnd->fY - vOrigin.y;

  /* Inits ray cast */
  orxMemory_Zero(&stRayCast, sizeof(orxPHYSICS_RAYCAST));
  stRayCast.hResult     = orxHANDLE_UNDEFINED;
  stRayCast.bEarlyExit  = _bEarlyExit;

  /* Inits filter */
  stFilter              = b2DefaultQueryFilter();
  stFilter.categoryBits = _u64SelfFlags;
  stFilter.maskBits     = _u64CheckMask;

  /* Issues Raycast */
  b2World_CastRay(sstPhysics.stWorld, vOrigin, vRay, stFilter, &orxPhysics_Box2D_RayCastCallback, &stRayCast);

  /* Found? */
  if(stRayCast.hResult != orxHANDLE_UNDEFINED)
  {
    /* Updates result */
    hResult = stRayCast.hResult;

    /* Asked for contact? */
    if(_pvContact != orxNULL)
    {
      /* Updates it */
      orxVector_Set(_pvContact, stRayCast.vContact.fX, stRayCast.vContact.fY, _pvBegin->fZ);
    }

    /* Asked for normal? */
    if(_pvNormal != orxNULL)
    {
      /* Updates it */
      orxVector_Copy(_pvNormal, &(stRayCast.vNormal));
    }
  }

#ifdef orxPHYSICS_ENABLE_DEBUG_DRAW

  /* Hit? */
  if(stRayCast.hResult != orxHANDLE_UNDEFINED)
  {
    orxPHYSICS_DEBUG_RAY *pstBeforeHitRay, *pstAfterHitRay;

    /* Allocates debug rays */
    pstBeforeHitRay  = (orxPHYSICS_DEBUG_RAY *)orxBank_Allocate(sstPhysics.pstRayBank);
    pstAfterHitRay   = (orxPHYSICS_DEBUG_RAY *)orxBank_Allocate(sstPhysics.pstRayBank);
    orxASSERT(pstBeforeHitRay != orxNULL);
    orxASSERT(pstAfterHitRay != orxNULL);

    /* Inits them */
    pstBeforeHitRay->vBegin   = vOrigin;
    pstBeforeHitRay->vEnd.x   = sstPhysics.fDimensionRatio * stRayCast.vContact.fX;
    pstBeforeHitRay->vEnd.y   = sstPhysics.fDimensionRatio * stRayCast.vContact.fY;
    pstBeforeHitRay->stColor  = orxPHYSICS_KU32_RAY_BEFORE_HIT_COLOR;
    pstAfterHitRay->vBegin.x  = sstPhysics.fDimensionRatio * stRayCast.vContact.fX;
    pstAfterHitRay->vBegin.y  = sstPhysics.fDimensionRatio * stRayCast.vContact.fY;
    pstAfterHitRay->vEnd.x    = vOrigin.x + vRay.x;
    pstAfterHitRay->vEnd.y    = vOrigin.y + vRay.y;
    pstAfterHitRay->stColor   = orxPHYSICS_KU32_RAY_AFTER_HIT_COLOR;
  }
  else
  {
    orxPHYSICS_DEBUG_RAY *pstMissRay;

    /* Allocates debug ray */
    pstMissRay = (orxPHYSICS_DEBUG_RAY *)orxBank_Allocate(sstPhysics.pstRayBank);
    orxASSERT(pstMissRay != orxNULL);

    /* Inits it */
    pstMissRay->vBegin  = vOrigin;
    pstMissRay->vEnd.x  = vOrigin.x + vRay.x;
    pstMissRay->vEnd.y  = vOrigin.y + vRay.y;
    pstMissRay->stColor = orxPHYSICS_KU32_RAY_MISS_COLOR;
  }

#endif /* orxPHYSICS_ENABLE_DEBUG_DRAW */

  /* Done! */
  return hResult;
}

orxU32 orxFASTCALL orxPhysics_Box2D_BoxPick(const orxAABOX *_pstBox, orxU64 _u64SelfFlags, orxU64 _u64CheckMask, orxHANDLE _ahUserDataList[], orxU32 _u32Number)
{
  b2AABB              stBox;
  b2QueryFilter       stFilter;
  orxPHYSICS_BOXPICK  stBoxPick;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBox != orxNULL);

  /* Gets extremities */
  stBox.lowerBound.x = sstPhysics.fDimensionRatio * _pstBox->vTL.fX;
  stBox.lowerBound.y = sstPhysics.fDimensionRatio * _pstBox->vTL.fY;
  stBox.upperBound.x = sstPhysics.fDimensionRatio * _pstBox->vBR.fX;
  stBox.upperBound.y = sstPhysics.fDimensionRatio * _pstBox->vBR.fY;

  /* Inits ray cast */
  orxMemory_Zero(&stBoxPick, sizeof(orxPHYSICS_BOXPICK));

  /* Inits filter */
  stFilter              = b2DefaultQueryFilter();
  stFilter.categoryBits = _u64SelfFlags;
  stFilter.maskBits     = _u64CheckMask;

  /* Inits storage */
  stBoxPick.ahUserDataList  = _ahUserDataList;
  stBoxPick.u32Size         = (_ahUserDataList != orxNULL) ? _u32Number : 0;

  /* Issues query */
  b2World_OverlapAABB(sstPhysics.stWorld, stBox, stFilter, &orxPhysics_Box2D_BoxPickCallback, &stBoxPick);

  /* Done! */
  return stBoxPick.u32Count;
}

void orxFASTCALL orxPhysics_Box2D_EnableSimulation(orxBOOL _bEnable)
{
  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);

  /* Enabled? */
  if(_bEnable != orxFALSE)
  {
    /* Updates status */
    orxFLAG_SET(sstPhysics.u32Flags, orxPHYSICS_KU32_STATIC_FLAG_ENABLED, orxPHYSICS_KU32_STATIC_FLAG_NONE);
  }
  else
  {
    /* Updates status */
    orxFLAG_SET(sstPhysics.u32Flags, orxPHYSICS_KU32_STATIC_FLAG_NONE, orxPHYSICS_KU32_STATIC_FLAG_ENABLED);
  }

  /* Done! */
  return;
}

orxSTATUS orxFASTCALL orxPhysics_Box2D_SetGravity(const orxVECTOR *_pvGravity)
{
  b2Vec2    vGravity;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvGravity != orxNULL);

  /* Sets gravity vector */
  vGravity.x = sstPhysics.fDimensionRatio * _pvGravity->fX;
  vGravity.y = sstPhysics.fDimensionRatio * _pvGravity->fY;

  /* Updates gravity */
  b2World_SetGravity(sstPhysics.stWorld, vGravity);

  /* Done! */
  return eResult;
}

orxVECTOR *orxFASTCALL orxPhysics_Box2D_GetGravity(orxVECTOR *_pvGravity)
{
  b2Vec2      vGravity;
  orxVECTOR  *pvResult = _pvGravity;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvGravity != orxNULL);

  /* Gets gravity vector */
  vGravity = b2World_GetGravity(sstPhysics.stWorld);
  orxVector_Set(_pvGravity, sstPhysics.fRecDimensionRatio * vGravity.x, sstPhysics.fRecDimensionRatio * vGravity.y, orxFLOAT_0);

  /* Done! */
  return pvResult;
}

orxSTATUS orxFASTCALL orxPhysics_Box2D_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Was not already initialized? */
  if(!(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY))
  {
    orxFLOAT    fRatio, fStepFrequency;
    orxVECTOR   vGravity;
    b2WorldDef  stWorldDef;
    
    /* Cleans static controller */
    orxMemory_Zero(&sstPhysics, sizeof(orxPHYSICS_STATIC));

    /* Pushes config section */
    orxConfig_PushSection(orxPHYSICS_KZ_CONFIG_SECTION);

    /* Sets custom memory alloc/free */
    b2SetAllocator(orxPhysics_Box2D_Allocate, orxPhysics_Box2D_Free);

    /* Gets gravity */
    if(orxConfig_GetVector(orxPHYSICS_KZ_CONFIG_GRAVITY, &vGravity) == orxNULL)
    {
      orxVector_Copy(&vGravity, &orxVECTOR_0);
    }

    /* Gets step frequency */
    fStepFrequency = orxConfig_GetFloat(orxPHYSICS_KZ_CONFIG_STEP_FREQUENCY);

    /* Deactivated? */
    if(fStepFrequency <= orxFLOAT_0)
    {
      /* Uses default frequency */
      sstPhysics.fFixedDT = orxFLOAT_1 / orxPHYSICS_KF_DEFAULT_FREQUENCY;
    }
    else
    {
      /* Stores fixed DT */
      sstPhysics.fFixedDT = orxFLOAT_1 / fStepFrequency;

      /* Updates status */
      orxFLAG_SET(sstPhysics.u32Flags, orxPHYSICS_KU32_STATIC_FLAG_FIXED_DT, orxPHYSICS_KU32_STATIC_FLAG_NONE);

      /* Should interpolate? */
      if((orxConfig_HasValue(orxPHYSICS_KZ_CONFIG_INTERPOLATE) == orxFALSE) || (orxConfig_GetBool(orxPHYSICS_KZ_CONFIG_INTERPOLATE) != orxFALSE))
      {
        /* Updates status */
        orxFLAG_SET(sstPhysics.u32Flags, orxPHYSICS_KU32_STATIC_FLAG_INTERPOLATE, orxPHYSICS_KU32_STATIC_FLAG_NONE);
      }
    }

    /* Gets iteration per steps */
    sstPhysics.s32IterationPerSteps = (orxConfig_HasValue(orxPHYSICS_KZ_CONFIG_ITERATIONS) != orxFALSE) ? orxConfig_GetS32(orxPHYSICS_KZ_CONFIG_ITERATIONS) : orxPHYSICS_KS32_DEFAULT_SUB_STEP;

    /* Gets dimension ratio */
    fRatio = orxConfig_GetFloat(orxPHYSICS_KZ_CONFIG_RATIO);

    /* Valid? */
    if(fRatio > orxFLOAT_0)
    {
      /* Stores it */
      sstPhysics.fDimensionRatio = fRatio;
    }
    else
    {
      /* Stores default one */
      sstPhysics.fDimensionRatio = orxPHYSICS_KF_DEFAULT_DIMENSION_RATIO;
    }

    /* Stores it */
    orxConfig_SetFloat(orxPHYSICS_KZ_CONFIG_RATIO, sstPhysics.fDimensionRatio);

    /* Setups world */
    stWorldDef              = b2DefaultWorldDef();
    stWorldDef.enableSleep  = (orxConfig_HasValue(orxPHYSICS_KZ_CONFIG_ALLOW_SLEEP) == orxFALSE) || (orxConfig_GetBool(orxPHYSICS_KZ_CONFIG_ALLOW_SLEEP) != orxFALSE);
    stWorldDef.gravity.x    = sstPhysics.fDimensionRatio * vGravity.fX;
    stWorldDef.gravity.y    = sstPhysics.fDimensionRatio * vGravity.fY;

    /* Creates world */
    sstPhysics.stWorld      = b2CreateWorld(&stWorldDef);

    /* Success? */
    if(b2World_IsValid(sstPhysics.stWorld))
    {
      orxCLOCK *pstClock;

      /* Stores inverse dimension ratio */
      sstPhysics.fRecDimensionRatio = orxFLOAT_1 / sstPhysics.fDimensionRatio;

      /* Gets core clock */
      pstClock = orxClock_Get(orxCLOCK_KZ_CORE);

      /* Resyncs clocks */
      orxClock_ResyncAll();

      /* Valid? */
      if(pstClock != orxNULL)
      {
        /* Creates banks */
        sstPhysics.pstBodyBank      = orxBank_Create(orxPHYSICS_KU32_BODY_BANK_SIZE, sizeof(orxPHYSICS_BODY), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
        sstPhysics.pstBodyPartBank  = orxBank_Create(orxPHYSICS_KU32_BODY_PART_BANK_SIZE, sizeof(orxPHYSICS_BODY_PART), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
        sstPhysics.pstBodyJointBank  = orxBank_Create(orxPHYSICS_KU32_BODY_JOINT_BANK_SIZE, sizeof(orxPHYSICS_BODY_JOINT), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

        /* Success? */
        if((sstPhysics.pstBodyBank != orxNULL) && (sstPhysics.pstBodyPartBank != orxNULL) && (sstPhysics.pstBodyJointBank != orxNULL))
        {
          /* Registers update function */
          eResult = orxClock_Register(pstClock, orxPhysics_Box2D_Update, orxNULL, orxMODULE_ID_PHYSICS, orxCLOCK_PRIORITY_LOWER);

          /* Valid? */
          if(eResult != orxSTATUS_FAILURE)
          {
#ifdef orxPHYSICS_ENABLE_DEBUG_DRAW

            /* Creates ray bank */
            sstPhysics.pstRayBank = orxBank_Create(orxPHYSICS_KU32_RAY_BANK_SIZE, sizeof(orxPHYSICS_DEBUG_RAY), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_DEBUG);

            /* Inits draw interface */
            sstPhysics.stDebugDraw                      = b2DefaultDebugDraw();
            sstPhysics.stDebugDraw.drawShapes           = true;
            sstPhysics.stDebugDraw.drawMass             = true;
            sstPhysics.stDebugDraw.drawJoints           = true;
            sstPhysics.stDebugDraw.drawJointExtras      = true;
            sstPhysics.stDebugDraw.drawGraphColors      = true;
            sstPhysics.stDebugDraw.DrawCircleFcn        = &orxPhysics_Box2D_DrawCircle;
            sstPhysics.stDebugDraw.DrawSolidCircleFcn   = &orxPhysics_Box2D_DrawSolidCircle;
            sstPhysics.stDebugDraw.DrawPolygonFcn       = &orxPhysics_Box2D_DrawPolygon;
            sstPhysics.stDebugDraw.DrawSolidPolygonFcn  = &orxPhysics_Box2D_DrawSolidPolygon;
            sstPhysics.stDebugDraw.DrawSegmentFcn       = &orxPhysics_Box2D_DrawSegment;
            sstPhysics.stDebugDraw.DrawTransformFcn     = &orxPhysics_Box2D_DrawTransform;
            sstPhysics.stDebugDraw.DrawPointFcn         = &orxPhysics_Box2D_DrawPoint;
            sstPhysics.stDebugDraw.DrawStringFcn        = &orxPhysics_Box2D_DrawString;

            /* Adds event handler */
            orxEvent_AddHandler(orxEVENT_TYPE_RENDER, orxPhysics_Box2D_EventHandler);
            orxEvent_SetHandlerIDFlags(orxPhysics_Box2D_EventHandler, orxEVENT_TYPE_RENDER, orxNULL, orxEVENT_GET_FLAG(orxRENDER_EVENT_STOP), orxEVENT_KU32_MASK_ID_ALL);

#endif /* orxPHYSICS_ENABLE_DEBUG_DRAW */

            /* Updates status */
            sstPhysics.u32Flags |= orxPHYSICS_KU32_STATIC_FLAG_READY | orxPHYSICS_KU32_STATIC_FLAG_ENABLED;

            /* Updates result */
            eResult = orxSTATUS_SUCCESS;
          }
          else
          {
            /* Deletes banks */
            orxBank_Delete(sstPhysics.pstBodyBank);
            orxBank_Delete(sstPhysics.pstBodyPartBank);
            orxBank_Delete(sstPhysics.pstBodyJointBank);

            /* Deletes world */
            b2DestroyWorld(sstPhysics.stWorld);
          }
        }
        else
        {
          /* Deletes banks */
          if(sstPhysics.pstBodyBank != orxNULL)
          {
            orxBank_Delete(sstPhysics.pstBodyBank);
          }
          if(sstPhysics.pstBodyPartBank != orxNULL)
          {
            orxBank_Delete(sstPhysics.pstBodyPartBank);
          }
          if(sstPhysics.pstBodyJointBank != orxNULL)
          {
            orxBank_Delete(sstPhysics.pstBodyJointBank);
          }
          
          /* Deletes world */
          b2DestroyWorld(sstPhysics.stWorld);
        }
      }
      else
      {
        /* Deletes world */
        b2DestroyWorld(sstPhysics.stWorld);
      }
    }

    /* Pops config section */
    orxConfig_PopSection();
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

void orxFASTCALL orxPhysics_Box2D_Exit()
{
  /* Was initialized? */
  if(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY)
  {
    /* Unregisters update */
    orxClock_Unregister(orxClock_Get(orxCLOCK_KZ_CORE), orxPhysics_Box2D_Update);

    /* Deletes banks */
    orxBank_Delete(sstPhysics.pstBodyBank);
    orxBank_Delete(sstPhysics.pstBodyPartBank);
    orxBank_Delete(sstPhysics.pstBodyJointBank);

#ifdef orxPHYSICS_ENABLE_DEBUG_DRAW

    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_RENDER, orxPhysics_Box2D_EventHandler);

    /* Deletes ray bank */
    orxBank_Delete(sstPhysics.pstRayBank);

#endif /* orxPHYSICS_ENABLE_DEBUG_DRAW */

    /* Deletes world */
    b2DestroyWorld(sstPhysics.stWorld);

    /* Cleans static controller */
    orxMemory_Zero(&sstPhysics, sizeof(orxPHYSICS_STATIC));
  }

  /* Done! */
  return;
}


// /***************************************************************************
//  * Plugin related                                                          *
//  ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(PHYSICS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_Init, PHYSICS, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_Exit, PHYSICS, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_SetGravity, PHYSICS, SET_GRAVITY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_GetGravity, PHYSICS, GET_GRAVITY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_CreateBody, PHYSICS, CREATE_BODY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_DeleteBody, PHYSICS, DELETE_BODY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_CreatePart, PHYSICS, CREATE_PART);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_DeletePart, PHYSICS, DELETE_PART);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_CreateJoint, PHYSICS, CREATE_JOINT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_DeleteJoint, PHYSICS, DELETE_JOINT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_SetPosition, PHYSICS, SET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_SetRotation, PHYSICS, SET_ROTATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_SetSpeed, PHYSICS, SET_SPEED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_SetAngularVelocity, PHYSICS, SET_ANGULAR_VELOCITY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_SetCustomGravity, PHYSICS, SET_CUSTOM_GRAVITY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_SetFixedRotation, PHYSICS, SET_FIXED_ROTATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_SetDynamic, PHYSICS, SET_DYNAMIC);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_SetAllowMoving, PHYSICS, SET_ALLOW_MOVING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_GetPosition, PHYSICS, GET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_GetRotation, PHYSICS, GET_ROTATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_GetSpeed, PHYSICS, GET_SPEED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_GetSpeedAtWorldPosition, PHYSICS, GET_SPEED_AT_WORLD_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_GetAngularVelocity, PHYSICS, GET_ANGULAR_VELOCITY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_GetCustomGravity, PHYSICS, GET_CUSTOM_GRAVITY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_IsFixedRotation, PHYSICS, IS_FIXED_ROTATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_GetMass, PHYSICS, GET_MASS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_GetMassCenter, PHYSICS, GET_MASS_CENTER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_SetLinearDamping, PHYSICS, SET_LINEAR_DAMPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_SetAngularDamping, PHYSICS, SET_ANGULAR_DAMPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_GetLinearDamping, PHYSICS, GET_LINEAR_DAMPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_GetAngularDamping, PHYSICS, GET_ANGULAR_DAMPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_ApplyTorque, PHYSICS, APPLY_TORQUE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_ApplyForce, PHYSICS, APPLY_FORCE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_ApplyImpulse, PHYSICS, APPLY_IMPULSE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_SetPartSelfFlags, PHYSICS, SET_PART_SELF_FLAGS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_SetPartCheckMask, PHYSICS, SET_PART_CHECK_MASK);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_GetPartSelfFlags, PHYSICS, GET_PART_SELF_FLAGS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_GetPartCheckMask, PHYSICS, GET_PART_CHECK_MASK);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_SetPartSolid, PHYSICS, SET_PART_SOLID);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_IsPartSolid, PHYSICS, IS_PART_SOLID);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_SetPartFriction, PHYSICS, SET_PART_FRICTION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_GetPartFriction, PHYSICS, GET_PART_FRICTION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_SetPartRestitution, PHYSICS, SET_PART_RESTITUTION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_GetPartRestitution, PHYSICS, GET_PART_RESTITUTION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_SetPartDensity, PHYSICS, SET_PART_DENSITY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_GetPartDensity, PHYSICS, GET_PART_DENSITY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_IsInsidePart, PHYSICS, IS_INSIDE_PART);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_EnableMotor, PHYSICS, ENABLE_MOTOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_SetJointMotorSpeed, PHYSICS, SET_JOINT_MOTOR_SPEED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_SetJointMaxMotorTorque, PHYSICS, SET_JOINT_MAX_MOTOR_TORQUE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_SetJointMaxMotorForce, PHYSICS, SET_JOINT_MAX_MOTOR_FORCE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_GetJointReactionForce, PHYSICS, GET_JOINT_REACTION_FORCE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_GetJointReactionTorque, PHYSICS, GET_JOINT_REACTION_TORQUE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_Raycast, PHYSICS, RAYCAST);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_BoxPick, PHYSICS, BOX_PICK);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_EnableSimulation, PHYSICS, ENABLE_SIMULATION);
orxPLUGIN_USER_CORE_FUNCTION_END();


#if defined(__orxWINDOWS__) && !defined(__orxMSVC__)

  #undef alloca

#endif /* __orxWINDOWS__ && !__orxMSVC__ */
