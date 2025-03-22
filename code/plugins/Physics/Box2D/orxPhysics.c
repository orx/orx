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


#if defined(__orxWINDOWS__) && !defined(__orxMSVC__) && !defined(alloca)

  #define alloca(x) __builtin_alloca((x))

#endif /* __orxWINDOWS__ && !__orxMSVC__ && !alloca */


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
#define orxPHYSICS_KF_DEFAULT_FREQUENCY         orx2F(60.0f) /* Default frequency */
#define orxPHYSICS_KF_DEFAULT_DIMENSION_RATIO   orx2F(0.01f) /* Default dimension ratio */
#define orxPHYSICS_KF_MIN_STEP_DURATION         orx2F(0.001f) /* Min step duration */

#ifdef orxPHYSICS_ENABLE_DEBUG_DRAW

#define orxPHYSICS_KU32_RAY_BANK_SIZE           128
#define orxPHYSICS_KU32_RAY_MISS_COLOR          b2_colorLime
#define orxPHYSICS_KU32_RAY_BEFORE_HIT_COLOR    b2_colorYellow
#define orxPHYSICS_KU32_RAY_AFTER_HIT_COLOR     b2_colorRed

#endif /* orxPHYSICS_ENABLE_DEBUG_DRAW */


// /***************************************************************************
//  * Structure declaration                                                   *
//  ***************************************************************************/

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
};

/** Body part
 */
struct __orxPHYSICS_BODY_PART_t
{
  b2ShapeId           stShape;                /**< Box2D shape */
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
  orxFLOAT                    fDimensionRatio;        /**< Dimension ratio */
  orxFLOAT                    fRecDimensionRatio;     /**< Reciprocal dimension ratio */
  orxFLOAT                    fLastDT;                /**< Last DT */
  b2WorldId                   stWorld;                /**< Box2D world */
  orxFLOAT                    fFixedDT;               /**< Fixed DT */
  orxFLOAT                    fDTAccumulator;         /**< DT accumulator */
  orxLINKLIST                 stBodyList;             /**< Body link list */
  orxBANK                    *pstBodyBank;            /**< Body bank */
  orxBANK                    *pstBodyPartBank;        /**< Body part bank */

#ifdef orxPHYSICS_ENABLE_DEBUG_DRAW

  orxBANK                    *pstRayBank;             /**< Debug ray bank */
  b2DebugDraw                 stDebugDraw;            /**< Debug draw interface */

#endif /* orxPHYSICS_ENABLE_DEBUG_DRAW */

} orxPHYSICS_STATIC;


// /***************************************************************************
//  * Static variables                                                        *
//  ***************************************************************************/

/** Static data
 */
static orxPHYSICS_STATIC sstPhysics;


// /***************************************************************************
//  * Private functions                                                       *
//  ***************************************************************************/

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

// static void orxFASTCALL orxPhysics_Box2D_SendContactEvent(b2Contact *_poContact, orxPHYSICS_EVENT _eEventID)
// {
//   orxBODY_PART             *pstSourceBodyPart, *pstDestinationBodyPart;

//   /* Gets body parts */
//   pstSourceBodyPart       = (orxBODY_PART *)_poContact->GetFixtureA()->GetUserData();
//   pstDestinationBodyPart  = (orxBODY_PART *)_poContact->GetFixtureB()->GetUserData();

//   /* Valid? */
//   if((pstSourceBodyPart != orxNULL) && (pstDestinationBodyPart != orxNULL))
//   {
//     orxPHYSICS_EVENT_STORAGE *pstEventStorage;

//     /* Adds a contact event */
//     pstEventStorage = (orxPHYSICS_EVENT_STORAGE *)orxBank_Allocate(sstPhysics.pstEventBank);

//     /* Valid? */
//     if(pstEventStorage != orxNULL)
//     {
//       b2Body *poSource, *poDestination;

//       /* Clears it */
//       orxMemory_Zero(pstEventStorage, sizeof(orxPHYSICS_EVENT_STORAGE));

//       /* Adds it to list */
//       orxLinkList_AddEnd(&(sstPhysics.stEventList), &(pstEventStorage->stNode));

//       /* Gets both bodies */
//       poSource      = _poContact->GetFixtureA()->GetBody();
//       poDestination = _poContact->GetFixtureB()->GetBody();

//       /* Inits it */
//       pstEventStorage->eID            = _eEventID;
//       pstEventStorage->poSource       = poSource;
//       pstEventStorage->poDestination  = poDestination;

//       /* Contact add? */
//       if(_eEventID == orxPHYSICS_EVENT_CONTACT_ADD)
//       {
//         const b2Manifold *poManifold;

//         /* Gets local manifold */
//         poManifold = _poContact->GetManifold();

//         /* 2 contacts? */
//         if(poManifold->pointCount > 1)
//         {
//           b2WorldManifold oManifold;

//           /* Gets global manifold */
//           _poContact->GetWorldManifold(&oManifold);

//           /* Updates values */
//           orxVector_Set(&(pstEventStorage->stPayload.vPosition), orx2F(0.5f) * sstPhysics.fRecDimensionRatio * (oManifold.points[0].x + oManifold.points[1].x), orx2F(0.5f) * sstPhysics.fRecDimensionRatio * (oManifold.points[0].y + oManifold.points[1].y), orxFLOAT_0);
//           orxVector_Set(&(pstEventStorage->stPayload.vNormal), oManifold.normal.x, oManifold.normal.y, orxFLOAT_0);
//         }
//         /* 1 contact? */
//         else if(poManifold->pointCount == 1)
//         {
//           b2WorldManifold oManifold;

//           /* Gets global manifold */
//           _poContact->GetWorldManifold(&oManifold);

//           /* Updates values */
//           orxVector_Set(&(pstEventStorage->stPayload.vPosition), sstPhysics.fRecDimensionRatio * oManifold.points[0].x, sstPhysics.fRecDimensionRatio * oManifold.points[0].y, orxFLOAT_0);
//           orxVector_Set(&(pstEventStorage->stPayload.vNormal), oManifold.normal.x, oManifold.normal.y, orxFLOAT_0);
//         }
//         /* 0 contact */
//         else
//         {
//           orxVector_Copy(&(pstEventStorage->stPayload.vPosition), &orxVECTOR_0);
//           orxVector_Copy(&(pstEventStorage->stPayload.vNormal), &orxVECTOR_0);
//         }
//       }
//       else
//       {
//         orxVector_Copy(&(pstEventStorage->stPayload.vPosition), &orxVECTOR_0);
//         orxVector_Copy(&(pstEventStorage->stPayload.vNormal), &orxVECTOR_0);
//       }

//       /* Updates part names */
//       pstEventStorage->stPayload.pstSenderPart    = pstSourceBodyPart;
//       pstEventStorage->stPayload.pstRecipientPart = pstDestinationBodyPart;
//     }
//   }

//   /* Done! */
//   return;
// }

// void orxPhysicsContactListener::BeginContact(b2Contact *_poContact)
// {
//   /* Sends contact event */
//   orxPhysics_Box2D_SendContactEvent(_poContact, orxPHYSICS_EVENT_CONTACT_ADD);

//   /* Done! */
//   return;
// }

// void orxPhysicsContactListener::EndContact(b2Contact *_poContact)
// {
//   /* Sends contact event */
//   orxPhysics_Box2D_SendContactEvent(_poContact, orxPHYSICS_EVENT_CONTACT_REMOVE);

//   /* Done! */
//   return;
// }


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
      /* Updates rotation */
      orxFrame_SetRotation(pstFrame, eFrameSpace, orxPhysics_GetRotation(_pstBody));

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
    b2BodyEvents  stEvents;
    orxS32        s32Steps, i;

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
      b2World_Step(sstPhysics.stWorld, sstPhysics.fFixedDT, 4);
    }

    /* Not absolute fixed DT? */
    if(!orxFLAG_TEST(sstPhysics.u32Flags, orxPHYSICS_KU32_STATIC_FLAG_FIXED_DT))
    {
      /* Should run a last simulation step? */
      if(sstPhysics.fDTAccumulator >= orxPHYSICS_KF_MIN_STEP_DURATION)
      {
        /* Updates last step of world simulation */
        b2World_Step(sstPhysics.stWorld, sstPhysics.fDTAccumulator, 4);

        /* Clears accumulator */
        sstPhysics.fDTAccumulator = orxFLOAT_0;
      }
    }

    /* For all body events */
    stEvents = b2World_GetBodyEvents(sstPhysics.stWorld);
    for(i = 0; i < stEvents.moveCount; i++)
    {
      b2BodyMoveEvent *pstEvent;

      /* Gets it */
      pstEvent = &(stEvents.moveEvents[i]);


      /* Non-static and awake? */
      if((b2Body_GetType(pstEvent->bodyId) != b2_staticBody)
      && (b2Body_IsAwake(pstEvent->bodyId) != false))
      {
        /* Applies simulation result */
        orxPhysics_ApplySimulationResult((orxPHYSICS_BODY *)b2Body_GetUserData(pstEvent->bodyId));
      }
    }

    //! TODO
    ///* For all stored events */
    //for(pstEventStorage = (orxPHYSICS_EVENT_STORAGE *)orxLinkList_GetFirst(&(sstPhysics.stEventList));
    //    pstEventStorage != orxNULL;
    //    pstEventStorage = (orxPHYSICS_EVENT_STORAGE *)orxLinkList_GetNext(&(pstEventStorage->stNode)))
    //{
    //  /* Depending on type */
    //  switch(pstEventStorage->eID)
    //  {
    //    case orxPHYSICS_EVENT_CONTACT_ADD:
    //    case orxPHYSICS_EVENT_CONTACT_REMOVE:
    //    {
    //      /* Sends event */
    //      orxEVENT_SEND(orxEVENT_TYPE_PHYSICS, pstEventStorage->eID, orxStructure_GetOwner(orxBODY(pstEventStorage->poSource->GetUserData())), orxStructure_GetOwner(orxBODY(pstEventStorage->poDestination->GetUserData())), &(pstEventStorage->stPayload));

    //      break;
    //    }

    //    default:
    //    {
    //      break;
    //    }
    //  }
    //}
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
        if(pstResult->stBody.index1 > 0)
        {
          /* Valid and has mass data? */
          if((bHasMass != orxFALSE) && (pstResult->stBody.index1 != 0))
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
      if(pstResult->stBody.index1 > 0)
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
    b2BodyId    stBody;
    b2ShapeDef  stShapeDef;

    /* Gets body */
    stBody = _pstBody->stBody;

    /* Inits shape definition */
    stShapeDef = b2DefaultShapeDef();
    stShapeDef.userData             = _hUserData;
    stShapeDef.friction             = _pstBodyPartDef->fFriction;
    stShapeDef.restitution          = _pstBodyPartDef->fRestitution;
    stShapeDef.density              = (b2Body_GetType(stBody) != b2_dynamicBody) ? 0.0f : _pstBodyPartDef->fDensity;
    stShapeDef.filter.categoryBits  = _pstBodyPartDef->u16SelfFlags;
    stShapeDef.filter.maskBits      = _pstBodyPartDef->u16CheckMask;
    stShapeDef.filter.groupIndex    = 0;
    stShapeDef.isSensor             = orxFLAG_TEST(_pstBodyPartDef->u32Flags, orxBODY_PART_DEF_KU32_FLAG_SOLID) == orxFALSE;

    /* Circle? */
    if(orxFLAG_TEST(_pstBodyPartDef->u32Flags, orxBODY_PART_DEF_KU32_FLAG_SPHERE))
    {
      b2Circle stCircle;

      /* Inits circle */
      stCircle.center.x = sstPhysics.fDimensionRatio * _pstBodyPartDef->stSphere.vCenter.fX * _pstBodyPartDef->vScale.fX;
      stCircle.center.y = sstPhysics.fDimensionRatio * _pstBodyPartDef->stSphere.vCenter.fY * _pstBodyPartDef->vScale.fY;
      stCircle.radius   = sstPhysics.fDimensionRatio * _pstBodyPartDef->stSphere.fRadius * orx2F(0.5f) * (orxMath_Abs(_pstBodyPartDef->vScale.fX) + orxMath_Abs(_pstBodyPartDef->vScale.fY));

      /* Creates its shape */
      pstResult->stShape = b2CreateCircleShape(stBody, &stShapeDef, &stCircle);
    }
    /* Polygon? */
    else if(orxFLAG_TEST(_pstBodyPartDef->u32Flags, orxBODY_PART_DEF_KU32_FLAG_BOX | orxBODY_PART_DEF_KU32_FLAG_MESH))
    {
      b2Polygon stPolygon;

      /* Box? */
      if(orxFLAG_TEST(_pstBodyPartDef->u32Flags, orxBODY_PART_DEF_KU32_FLAG_BOX))
      {
        stPolygon = b2MakeBox(orxMath_Abs(orx2F(0.5f) * sstPhysics.fDimensionRatio * _pstBodyPartDef->vScale.fX * (_pstBodyPartDef->stAABox.stBox.vBR.fX - _pstBodyPartDef->stAABox.stBox.vTL.fX)),
                              orxMath_Abs(orx2F(0.5f) * sstPhysics.fDimensionRatio * _pstBodyPartDef->vScale.fY * (_pstBodyPartDef->stAABox.stBox.vBR.fY - _pstBodyPartDef->stAABox.stBox.vTL.fY)));
      }
      else
      {
        //! TODO
        //b2Vec2 avVertexList[b2_maxPolygonVertices];
        //orxU32 i;

        ///* Checks */
        //orxASSERT(_pstBodyPartDef->stMesh.u32VertexCount > 0);
        //orxASSERT(orxBODY_PART_DEF_KU32_MESH_VERTEX_NUMBER <= b2_maxPolygonVertices);

        ///* No mirroring? */
        //if(_pstBodyPartDef->vScale.fX * _pstBodyPartDef->vScale.fY > orxFLOAT_0)
        //{
        //  /* For all the vertices */
        //  for(i = 0; i < _pstBodyPartDef->stMesh.u32VertexCount; i++)
        //  {
        //    /* Sets its vector */
        //    avVertexList[i].Set(sstPhysics.fDimensionRatio * _pstBodyPartDef->stMesh.avVertices[i].fX * _pstBodyPartDef->vScale.fX, sstPhysics.fDimensionRatio * _pstBodyPartDef->stMesh.avVertices[i].fY * _pstBodyPartDef->vScale.fY);
        //  }
        //}
        //else
        //{
        //  orxS32 iDst;

        //  /* For all the vertices */
        //  for(iDst = _pstBodyPartDef->stMesh.u32VertexCount - 1, i = 0; iDst >= 0; iDst--, i++)
        //  {
        //    /* Sets its vector */
        //    avVertexList[iDst].Set(sstPhysics.fDimensionRatio * _pstBodyPartDef->stMesh.avVertices[i].fX * _pstBodyPartDef->vScale.fX, sstPhysics.fDimensionRatio * _pstBodyPartDef->stMesh.avVertices[i].fY * _pstBodyPartDef->vScale.fY);
        //  }
        //}
      }

      /* Creates its shape */
      pstResult->stShape = b2CreatePolygonShape(stBody, &stShapeDef, &stPolygon);
     }
    //else if(orxFLAG_TEST(_pstBodyPartDef->u32Flags, orxBODY_PART_DEF_KU32_FLAG_EDGE))
    //{
    // b2Vec2  av[2];
    // orxU32  i;

    // /* Stores shape reference */
    // stFixtureDef.shape = &stEdgeShape;

    // /* Sets vertices */
    // for(i = 0; i < 2; i++)
    // {
    //   av[i].Set(sstPhysics.fDimensionRatio * _pstBodyPartDef->stEdge.avVertices[i].fX * _pstBodyPartDef->vScale.fX, sstPhysics.fDimensionRatio * _pstBodyPartDef->stEdge.avVertices[i].fY * _pstBodyPartDef->vScale.fY);
    // }

    // /* Updates shape */
    // stEdgeShape.Set(av[0], av[1]);

    // /* Has previous (ghost)? */
    // if(_pstBodyPartDef->stEdge.bHasPrevious)
    // {
    //   stEdgeShape.m_vertex0.Set(sstPhysics.fDimensionRatio * _pstBodyPartDef->stEdge.vPrevious.fX * _pstBodyPartDef->vScale.fX, sstPhysics.fDimensionRatio * _pstBodyPartDef->stEdge.vPrevious.fY * _pstBodyPartDef->vScale.fY);
    //   stEdgeShape.m_hasVertex0 = true;
    // }

    // /* Has next (ghost)? */
    // if(_pstBodyPartDef->stEdge.bHasNext)
    // {
    //   stEdgeShape.m_vertex3.Set(sstPhysics.fDimensionRatio * _pstBodyPartDef->stEdge.vNext.fX * _pstBodyPartDef->vScale.fX, sstPhysics.fDimensionRatio * _pstBodyPartDef->stEdge.vNext.fY * _pstBodyPartDef->vScale.fY);
    //   stEdgeShape.m_hasVertex3 = true;
    // }
    //}
    //else if(orxFLAG_TEST(_pstBodyPartDef->u32Flags, orxBODY_PART_DEF_KU32_FLAG_CHAIN))
    //{
    // b2Vec2 *avVertexList = (b2Vec2 *)alloca(_pstBodyPartDef->stChain.u32VertexCount * sizeof(b2Vec2));
    // orxU32  i;

    // /* Checks */
    // orxASSERT(_pstBodyPartDef->stChain.u32VertexCount > 0);
    // orxASSERT(_pstBodyPartDef->stChain.avVertices != orxNULL);

    // /* Stores shape reference */
    // stFixtureDef.shape = &stChainShape;

    // /* For all the vertices */
    // for(i = 0; i < _pstBodyPartDef->stChain.u32VertexCount; i++)
    // {
    //   /* Sets its vector */
    //   avVertexList[i].Set(sstPhysics.fDimensionRatio * _pstBodyPartDef->stChain.avVertices[i].fX * _pstBodyPartDef->vScale.fX, sstPhysics.fDimensionRatio * _pstBodyPartDef->stChain.avVertices[i].fY * _pstBodyPartDef->vScale.fY);
    // }

    // /* Is loop? */
    // if(_pstBodyPartDef->stChain.bIsLoop != orxFALSE)
    // {
    //   /* Creates loop chain */
    //   stChainShape.CreateLoop(avVertexList, _pstBodyPartDef->stChain.u32VertexCount);
    // }
    // else
    // {
    //   /* Creates chain */
    //   stChainShape.CreateChain(avVertexList, _pstBodyPartDef->stChain.u32VertexCount);

    //   /* Has Previous? */
    //   if(_pstBodyPartDef->stChain.bHasPrevious != orxFALSE)
    //   {
    //     b2Vec2 vPrevious;

    //     /* Sets previous vertex */
    //     vPrevious.Set(sstPhysics.fDimensionRatio * _pstBodyPartDef->stChain.vPrevious.fX * _pstBodyPartDef->vScale.fX, sstPhysics.fDimensionRatio * _pstBodyPartDef->stChain.vPrevious.fY * _pstBodyPartDef->vScale.fY);
    //     stChainShape.SetPrevVertex(vPrevious);
    //   }

    //   /* Has Next? */
    //   if(_pstBodyPartDef->stChain.bHasNext != orxFALSE)
    //   {
    //     b2Vec2 vNext;

    //     /* Sets next vertex */
    //     vNext.Set(sstPhysics.fDimensionRatio * _pstBodyPartDef->stChain.vNext.fX * _pstBodyPartDef->vScale.fX, sstPhysics.fDimensionRatio * _pstBodyPartDef->stChain.vNext.fY * _pstBodyPartDef->vScale.fY);
    //     stChainShape.SetNextVertex(vNext);
    //   }
    // }
    //}
    
    /* Failure? */
    if(pstResult->stShape.index1 == 0)
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

  /* Deletes shape */
  b2DestroyShape(_pstBodyPart->stShape, true);  

  /* Frees part */
  orxBank_Free(sstPhysics.pstBodyPartBank, _pstBodyPart);

  /* Done! */
  return;
}

// orxPHYSICS_BODY_JOINT *orxFASTCALL orxPhysics_Box2D_CreateJoint(orxPHYSICS_BODY *_pstSrcBody, orxPHYSICS_BODY *_pstDstBody, const orxHANDLE _hUserData, const orxBODY_JOINT_DEF *_pstBodyJointDef)
// {
//   b2Joint            *poResult = 0;
//   orxBOOL             bSuccess = orxTRUE;
//   b2JointDef         *pstJointDef;
//   b2RevoluteJointDef  stRevoluteJointDef;
//   b2PrismaticJointDef stPrismaticJointDef;
//   b2DistanceJointDef  stSpringJointDef;
//   b2RopeJointDef      stRopeJointDef;
//   b2PulleyJointDef    stPulleyJointDef;
//   b2WheelJointDef     stWheelJointDef;
//   b2WeldJointDef      stWeldJointDef;
//   b2FrictionJointDef  stFrictionJointDef;
//   b2GearJointDef      stGearJointDef;

//   /* Checks */
//   orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
//   orxASSERT(_pstSrcBody != orxNULL);
//   orxASSERT(_pstDstBody != orxNULL);
//   orxASSERT(_hUserData != orxHANDLE_UNDEFINED);
//   orxASSERT(_pstBodyJointDef != orxNULL);
//   orxASSERT(orxFLAG_TEST(_pstBodyJointDef->u32Flags, orxBODY_JOINT_DEF_KU32_MASK_TYPE));

//   /* Depending on joint type */
//   switch(orxFLAG_GET(_pstBodyJointDef->u32Flags, orxBODY_JOINT_DEF_KU32_MASK_TYPE))
//   {
//     /* Revolute? */
//     case orxBODY_JOINT_DEF_KU32_FLAG_REVOLUTE:
//     {
//       /* Stores joint reference */
//       pstJointDef = &stRevoluteJointDef;

//       /* Stores anchors */
//       stRevoluteJointDef.localAnchorA.Set(sstPhysics.fDimensionRatio * _pstBodyJointDef->vSrcScale.fX * _pstBodyJointDef->vSrcAnchor.fX, sstPhysics.fDimensionRatio * _pstBodyJointDef->vSrcScale.fY * _pstBodyJointDef->vSrcAnchor.fY);
//       stRevoluteJointDef.localAnchorB.Set(sstPhysics.fDimensionRatio * _pstBodyJointDef->vDstScale.fX * _pstBodyJointDef->vDstAnchor.fX, sstPhysics.fDimensionRatio * _pstBodyJointDef->vDstScale.fY * _pstBodyJointDef->vDstAnchor.fY);

//       /* Stores reference angle */
//       stRevoluteJointDef.referenceAngle = _pstBodyJointDef->stRevolute.fDefaultRotation;

//       /* Has rotation limits? */
//       if(orxFLAG_TEST(_pstBodyJointDef->u32Flags, orxBODY_JOINT_DEF_KU32_FLAG_ROTATION_LIMIT))
//       {
//         /* Stores them */
//         stRevoluteJointDef.lowerAngle   = _pstBodyJointDef->stRevolute.fMinRotation;
//         stRevoluteJointDef.upperAngle   = _pstBodyJointDef->stRevolute.fMaxRotation;

//         /* Updates status */
//         stRevoluteJointDef.enableLimit  = true;
//       }

//       /* Is motor? */
//       if(orxFLAG_TEST(_pstBodyJointDef->u32Flags, orxBODY_JOINT_DEF_KU32_FLAG_MOTOR))
//       {
//         /* Stores them */
//         stRevoluteJointDef.motorSpeed     = _pstBodyJointDef->stRevolute.fMotorSpeed;
//         stRevoluteJointDef.maxMotorTorque = _pstBodyJointDef->stRevolute.fMaxMotorTorque;

//         /* Updates status */
//         stRevoluteJointDef.enableMotor    = true;
//       }

//       break;
//     }

//     /* Prismatic? */
//     case orxBODY_JOINT_DEF_KU32_FLAG_PRISMATIC:
//     {
//       /* Stores joint reference */
//       pstJointDef = &stPrismaticJointDef;

//       /* Stores anchors */
//       stPrismaticJointDef.localAnchorA.Set(sstPhysics.fDimensionRatio * _pstBodyJointDef->vSrcScale.fX * _pstBodyJointDef->vSrcAnchor.fX, sstPhysics.fDimensionRatio * _pstBodyJointDef->vSrcScale.fY * _pstBodyJointDef->vSrcAnchor.fY);
//       stPrismaticJointDef.localAnchorB.Set(sstPhysics.fDimensionRatio * _pstBodyJointDef->vDstScale.fX * _pstBodyJointDef->vDstAnchor.fX, sstPhysics.fDimensionRatio * _pstBodyJointDef->vDstScale.fY * _pstBodyJointDef->vDstAnchor.fY);

//       /* Stores reference angle */
//       stPrismaticJointDef.referenceAngle = _pstBodyJointDef->stPrismatic.fDefaultRotation;

//       /* Stores translation axis */
//       stPrismaticJointDef.localAxisA.Set(_pstBodyJointDef->stPrismatic.vTranslationAxis.fX, _pstBodyJointDef->stPrismatic.vTranslationAxis.fY);

//       /* Has translation limits? */
//       if(orxFLAG_TEST(_pstBodyJointDef->u32Flags, orxBODY_JOINT_DEF_KU32_FLAG_TRANSLATION_LIMIT))
//       {
//         /* Stores them */
//         stPrismaticJointDef.lowerTranslation  = sstPhysics.fDimensionRatio * _pstBodyJointDef->stPrismatic.fMinTranslation;
//         stPrismaticJointDef.upperTranslation  = sstPhysics.fDimensionRatio * _pstBodyJointDef->stPrismatic.fMaxTranslation;

//         /* Updates status */
//         stPrismaticJointDef.enableLimit       = true;
//       }

//       /* Is motor? */
//       if(orxFLAG_TEST(_pstBodyJointDef->u32Flags, orxBODY_JOINT_DEF_KU32_FLAG_MOTOR))
//       {
//         /* Stores them */
//         stPrismaticJointDef.motorSpeed    = sstPhysics.fDimensionRatio * _pstBodyJointDef->stPrismatic.fMotorSpeed;
//         stPrismaticJointDef.maxMotorForce = _pstBodyJointDef->stPrismatic.fMaxMotorForce;

//         /* Updates status */
//         stPrismaticJointDef.enableMotor   = true;
//       }

//       break;
//     }

//     /* Spring? */
//     case orxBODY_JOINT_DEF_KU32_FLAG_SPRING:
//     {
//       /* Stores joint reference */
//       pstJointDef = &stSpringJointDef;

//       /* Stores anchors */
//       stSpringJointDef.localAnchorA.Set(sstPhysics.fDimensionRatio * _pstBodyJointDef->vSrcScale.fX * _pstBodyJointDef->vSrcAnchor.fX, sstPhysics.fDimensionRatio * _pstBodyJointDef->vSrcScale.fY * _pstBodyJointDef->vSrcAnchor.fY);
//       stSpringJointDef.localAnchorB.Set(sstPhysics.fDimensionRatio * _pstBodyJointDef->vDstScale.fX * _pstBodyJointDef->vDstAnchor.fX, sstPhysics.fDimensionRatio * _pstBodyJointDef->vDstScale.fY * _pstBodyJointDef->vDstAnchor.fY);

//       /* Stores length */
//       stSpringJointDef.length       = sstPhysics.fDimensionRatio * _pstBodyJointDef->stSpring.fLength;

//       /* Stores frequency */
//       stSpringJointDef.frequencyHz  = _pstBodyJointDef->stSpring.fFrequency;

//       /* Stores damping ratio */
//       stSpringJointDef.dampingRatio = _pstBodyJointDef->stSpring.fDamping;

//       break;
//     }
//     /* Rope? */
//     case orxBODY_JOINT_DEF_KU32_FLAG_ROPE:
//     {
//       /* Stores joint reference */
//       pstJointDef = &stRopeJointDef;

//       /* Stores anchors */
//       stRopeJointDef.localAnchorA.Set(sstPhysics.fDimensionRatio * _pstBodyJointDef->vSrcScale.fX * _pstBodyJointDef->vSrcAnchor.fX, sstPhysics.fDimensionRatio * _pstBodyJointDef->vSrcScale.fY * _pstBodyJointDef->vSrcAnchor.fY);
//       stRopeJointDef.localAnchorB.Set(sstPhysics.fDimensionRatio * _pstBodyJointDef->vDstScale.fX * _pstBodyJointDef->vDstAnchor.fX, sstPhysics.fDimensionRatio * _pstBodyJointDef->vDstScale.fY * _pstBodyJointDef->vDstAnchor.fY);

//       /* Stores length */
//       stRopeJointDef.maxLength = sstPhysics.fDimensionRatio * _pstBodyJointDef->stRope.fLength;

//       break;
//     }
//     /* Pulley? */
//     case orxBODY_JOINT_DEF_KU32_FLAG_PULLEY:
//     {
//       /* Stores joint reference */
//       pstJointDef = &stPulleyJointDef;

//       /* Stores anchors */
//       stPulleyJointDef.localAnchorA.Set(sstPhysics.fDimensionRatio * _pstBodyJointDef->vSrcScale.fX * _pstBodyJointDef->vSrcAnchor.fX, sstPhysics.fDimensionRatio * _pstBodyJointDef->vSrcScale.fY * _pstBodyJointDef->vSrcAnchor.fY);
//       stPulleyJointDef.localAnchorB.Set(sstPhysics.fDimensionRatio * _pstBodyJointDef->vDstScale.fX * _pstBodyJointDef->vDstAnchor.fX, sstPhysics.fDimensionRatio * _pstBodyJointDef->vDstScale.fY * _pstBodyJointDef->vDstAnchor.fY);
//       stPulleyJointDef.groundAnchorA.Set(sstPhysics.fDimensionRatio * _pstBodyJointDef->stPulley.vSrcGroundAnchor.fX, sstPhysics.fDimensionRatio * _pstBodyJointDef->stPulley.vSrcGroundAnchor.fY);
//       stPulleyJointDef.groundAnchorB.Set(sstPhysics.fDimensionRatio * _pstBodyJointDef->stPulley.vDstGroundAnchor.fX, sstPhysics.fDimensionRatio * _pstBodyJointDef->stPulley.vDstGroundAnchor.fY);

//       /* Stores lengths */
//       stPulleyJointDef.lengthA    = sstPhysics.fDimensionRatio * _pstBodyJointDef->stPulley.fSrcLength;
//       stPulleyJointDef.lengthB    = sstPhysics.fDimensionRatio * _pstBodyJointDef->stPulley.fDstLength;

//       /* Stores ratio */
//       stPulleyJointDef.ratio      = _pstBodyJointDef->stPulley.fLengthRatio;

//       break;
//     }

//     /* Suspension? */
//     case orxBODY_JOINT_DEF_KU32_FLAG_SUSPENSION:
//     {
//       /* Stores joint reference */
//       pstJointDef = &stWheelJointDef;

//       /* Stores anchors */
//       stWheelJointDef.localAnchorA.Set(sstPhysics.fDimensionRatio * _pstBodyJointDef->vSrcScale.fX * _pstBodyJointDef->vSrcAnchor.fX, sstPhysics.fDimensionRatio * _pstBodyJointDef->vSrcScale.fY * _pstBodyJointDef->vSrcAnchor.fY);
//       stWheelJointDef.localAnchorB.Set(sstPhysics.fDimensionRatio * _pstBodyJointDef->vDstScale.fX * _pstBodyJointDef->vDstAnchor.fX, sstPhysics.fDimensionRatio * _pstBodyJointDef->vDstScale.fY * _pstBodyJointDef->vDstAnchor.fY);

//       /* Stores translation axis */
//       stWheelJointDef.localAxisA.Set(_pstBodyJointDef->stSuspension.vTranslationAxis.fX, _pstBodyJointDef->stSuspension.vTranslationAxis.fY);

//       /* Stores frequency */
//       stWheelJointDef.frequencyHz  = _pstBodyJointDef->stSuspension.fFrequency;

//       /* Stores damping ratio */
//       stWheelJointDef.dampingRatio = _pstBodyJointDef->stSuspension.fDamping;

//       /* Is motor? */
//       if(orxFLAG_TEST(_pstBodyJointDef->u32Flags, orxBODY_JOINT_DEF_KU32_FLAG_MOTOR))
//       {
//         /* Stores them */
//         stWheelJointDef.motorSpeed     = sstPhysics.fDimensionRatio * _pstBodyJointDef->stSuspension.fMotorSpeed;
//         stWheelJointDef.maxMotorTorque = _pstBodyJointDef->stSuspension.fMaxMotorForce;

//         /* Updates status */
//         stWheelJointDef.enableMotor    = true;
//       }

//       break;
//     }

//     /* Weld? */
//     case orxBODY_JOINT_DEF_KU32_FLAG_WELD:
//     {
//       /* Stores joint reference */
//       pstJointDef = &stWeldJointDef;

//       /* Stores anchors */
//       stWeldJointDef.localAnchorA.Set(sstPhysics.fDimensionRatio * _pstBodyJointDef->vSrcScale.fX * _pstBodyJointDef->vSrcAnchor.fX, sstPhysics.fDimensionRatio * _pstBodyJointDef->vSrcScale.fY * _pstBodyJointDef->vSrcAnchor.fY);
//       stWeldJointDef.localAnchorB.Set(sstPhysics.fDimensionRatio * _pstBodyJointDef->vDstScale.fX * _pstBodyJointDef->vDstAnchor.fX, sstPhysics.fDimensionRatio * _pstBodyJointDef->vDstScale.fY * _pstBodyJointDef->vDstAnchor.fY);

//       /* Stores reference angle */
//       stWeldJointDef.referenceAngle = _pstBodyJointDef->stWeld.fDefaultRotation;

//       break;
//     }

//     /* Friction? */
//     case orxBODY_JOINT_DEF_KU32_FLAG_FRICTION:
//     {
//       /* Stores joint reference */
//       pstJointDef = &stFrictionJointDef;

//       /* Stores anchors */
//       stFrictionJointDef.localAnchorA.Set(sstPhysics.fDimensionRatio * _pstBodyJointDef->vSrcScale.fX * _pstBodyJointDef->vSrcAnchor.fX, sstPhysics.fDimensionRatio * _pstBodyJointDef->vSrcScale.fY * _pstBodyJointDef->vSrcAnchor.fY);
//       stFrictionJointDef.localAnchorB.Set(sstPhysics.fDimensionRatio * _pstBodyJointDef->vDstScale.fX * _pstBodyJointDef->vDstAnchor.fX, sstPhysics.fDimensionRatio * _pstBodyJointDef->vDstScale.fY * _pstBodyJointDef->vDstAnchor.fY);

//       /* Stores max force & torque values */
//       stFrictionJointDef.maxForce   = _pstBodyJointDef->stFriction.fMaxForce;
//       stFrictionJointDef.maxTorque  = _pstBodyJointDef->stFriction.fMaxTorque;

//       break;
//     }

//     /* Gear? */
//     case orxBODY_JOINT_DEF_KU32_FLAG_GEAR:
//     {
//       b2Body *poBody;

//       /* Stores joint reference */
//       pstJointDef = &stGearJointDef;

//       /* Stores ratio */
//       stGearJointDef.ratio = _pstBodyJointDef->stGear.fJointRatio;

//       /* Gets source body */
//       poBody = (b2Body *)_pstSrcBody->poBody;

//       /* For all its joints */
//       for(b2JointEdge *poEdge = poBody->GetJointList();
//           poEdge != 0;
//           poEdge = poEdge->next)
//       {
//         b2Joint        *poJoint;
//         orxBODY_JOINT  *pstJoint;

//         /* Gets it */
//         poJoint = poEdge->joint;

//         /* Gets its body joint */
//         pstJoint = (orxBODY_JOINT *)poJoint->GetUserData();

//         /* Does name match? */
//         if(orxString_Compare(orxBody_GetJointName(pstJoint), _pstBodyJointDef->stGear.zSrcJointName) == 0)
//         {
//           /* Stores it */
//           stGearJointDef.joint1 = poJoint;
//           break;
//         }
//       }

//       /* Found source joint? */
//       if(stGearJointDef.joint1 != 0)
//       {
//         /* Gets destination body */
//         poBody = (b2Body *)_pstDstBody->poBody;

//         /* For all its joints */
//         for(b2JointEdge *poEdge = poBody->GetJointList();
//             poEdge != 0;
//             poEdge = poEdge->next)
//         {
//           b2Joint        *poJoint;
//           orxBODY_JOINT  *pstJoint;

//           /* Gets it */
//           poJoint = poEdge->joint;

//           /* Gets its body joint */
//           pstJoint = (orxBODY_JOINT *)poJoint->GetUserData();

//           /* Does name match? */
//           if(orxString_Compare(orxBody_GetJointName(pstJoint), _pstBodyJointDef->stGear.zDstJointName) == 0)
//           {
//             /* Stores it */
//             stGearJointDef.joint2 = poJoint;
//             break;
//           }
//         }

//         /* No destination joint found? */
//         if(stGearJointDef.joint2 == 0)
//         {
//           /* Logs message */
//           orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "Can't create gear body joint, couldn't find joint <%s> on destination body.", _pstBodyJointDef->stGear.zDstJointName);

//           /* Updates status */
//           bSuccess = orxFALSE;
//         }
//       }
//       else
//       {
//         /* Logs message */
//         orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "Can't create gear body joint, couldn't find joint <%s> on source body.", _pstBodyJointDef->stGear.zSrcJointName);

//         /* Updates status */
//         bSuccess = orxFALSE;
//       }

//       break;
//     }

//     default:
//     {
//       /* Logs message */
//       orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "Can't create body joint, invalid type <0x%X>.", orxFLAG_GET(_pstBodyJointDef->u32Flags, orxBODY_JOINT_DEF_KU32_MASK_TYPE));

//       /* Updates status */
//       bSuccess = orxFALSE;

//       break;
//     }
//   }

//   /* Valid? */
//   if(bSuccess != orxFALSE)
//   {
//     /* Inits joint definition */
//     pstJointDef->userData             = _hUserData;
//     pstJointDef->bodyA                = (b2Body *)_pstSrcBody->poBody;
//     pstJointDef->bodyB                = (b2Body *)_pstDstBody->poBody;
//     pstJointDef->collideConnected     = orxFLAG_TEST(_pstBodyJointDef->u32Flags, orxBODY_JOINT_DEF_KU32_FLAG_COLLIDE) ? true : false;

//     /* Creates it */
//     poResult = sstPhysics.poWorld->CreateJoint(pstJointDef);
//   }

//   /* Done! */
//   return (orxPHYSICS_BODY_JOINT *)poResult;
// }

// void orxFASTCALL orxPhysics_Box2D_DeleteJoint(orxPHYSICS_BODY_JOINT *_pstBodyJoint)
// {
//   /* Checks */
//   orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
//   orxASSERT(_pstBodyJoint != orxNULL);

//   /* Deletes it */
//   sstPhysics.poWorld->DestroyJoint((b2Joint *)_pstBodyJoint);

//   return;
// }

// void orxFASTCALL orxPhysics_Box2D_EnableMotor(orxPHYSICS_BODY_JOINT *_pstBodyJoint, orxBOOL _bEnable)
// {
//   b2Joint *poJoint;

//   /* Checks */
//   orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
//   orxASSERT(_pstBodyJoint != orxNULL);

//   /* Gets joint */
//   poJoint = (b2Joint *)_pstBodyJoint;

//   /* Is a revolute joint? */
//   if(poJoint->GetType() == e_revoluteJoint)
//   {
//     /* Enables / disables it */
//     static_cast<b2RevoluteJoint *>(poJoint)->EnableMotor((_bEnable != orxFALSE) ? true : false);
//   }
//   else
//   {
//     /* Logs message */
//     orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "Can't enable motor on non-revolute joint.");
//   }

//   /* Done! */
//   return;
// }

// void orxFASTCALL orxPhysics_Box2D_SetJointMotorSpeed(orxPHYSICS_BODY_JOINT *_pstBodyJoint, orxFLOAT _fSpeed)
// {
//   b2Joint *poJoint;

//   /* Checks */
//   orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
//   orxASSERT(_pstBodyJoint != orxNULL);

//   /* Gets joint */
//   poJoint = (b2Joint *)_pstBodyJoint;

//   /* Is a revolute joint? */
//   if(poJoint->GetType() == e_revoluteJoint)
//   {
//     /* Sets its motor speed */
//     static_cast<b2RevoluteJoint *>(poJoint)->SetMotorSpeed(_fSpeed);
//   }
//   else
//   {
//     /* Logs message */
//     orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "Can't set motor speed on non-revolute joint.");
//   }

//   /* Done! */
//   return;
// }

// void orxFASTCALL orxPhysics_Box2D_SetJointMaxMotorTorque(orxPHYSICS_BODY_JOINT *_pstBodyJoint, orxFLOAT _fMaxTorque)
// {
//   b2Joint *poJoint;

//   /* Checks */
//   orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
//   orxASSERT(_pstBodyJoint != orxNULL);

//   /* Gets joint */
//   poJoint = (b2Joint *)_pstBodyJoint;

//   /* Is a revolute joint? */
//   if(poJoint->GetType() == e_revoluteJoint)
//   {
//     /* Sets its max torque */
//     static_cast<b2RevoluteJoint *>(poJoint)->SetMaxMotorTorque(_fMaxTorque);
//   }
//   else
//   {
//     /* Logs message */
//     orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "Can't set max motor torque on non-revolute joint.");
//   }

//   /* Done! */
//   return;
// }

// orxVECTOR *orxFASTCALL orxPhysics_Box2D_GetJointReactionForce(const orxPHYSICS_BODY_JOINT *_pstBodyJoint, orxVECTOR *_pvForce)
// {
//   const b2Joint  *poJoint;
//   b2Vec2          vForce;

//   /* Checks */
//   orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
//   orxASSERT(_pstBodyJoint != orxNULL);
//   orxASSERT(_pvForce != orxNULL);

//   /* Gets joint */
//   poJoint = (const b2Joint *)_pstBodyJoint;

//   /* Gets reaction force */
//   vForce = poJoint->GetReactionForce((sstPhysics.fLastDT != orxFLOAT_0) ? orxFLOAT_1 / sstPhysics.fLastDT : orxFLOAT_1 / sstPhysics.fFixedDT);

//   /* Updates result */
//   orxVector_Set(_pvForce, orx2F(vForce.x), orx2F(vForce.y), orxFLOAT_0);

//   /* Done! */
//   return _pvForce;
// }

// orxFLOAT orxFASTCALL orxPhysics_Box2D_GetJointReactionTorque(const orxPHYSICS_BODY_JOINT *_pstBodyJoint)
// {
//   const b2Joint  *poJoint;
//   orxFLOAT        fResult;

//   /* Checks */
//   orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
//   orxASSERT(_pstBodyJoint != orxNULL);

//   /* Gets joint */
//   poJoint = (const b2Joint *)_pstBodyJoint;

//   /* Updates result */
//   fResult = orx2F(poJoint->GetReactionTorque((sstPhysics.fLastDT != orxFLOAT_0) ? orxFLOAT_1 / sstPhysics.fLastDT : orxFLOAT_1 / sstPhysics.fFixedDT));

//   /* Done! */
//   return fResult;
// }

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
  b2BodyId  stBody;
  b2Rot     stRotation, stCurrentRotation;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);

  /* Gets body */
  stBody = _pstBody->stBody;

  /* Gets new rotation */
  stRotation = b2MakeRot(_fRotation);

  /* Gets current rotation */
  stCurrentRotation = orxFLAG_TEST(sstPhysics.u32Flags, orxPHYSICS_KU32_STATIC_FLAG_INTERPOLATE) ? b2MakeRot(_pstBody->fInterpolatedRotation) : b2Body_GetRotation(stBody);

  /* Should apply? */
  if((stRotation.c != stCurrentRotation.c) || (stRotation.s != stCurrentRotation.s))
  {
    /* Wakes up */
    b2Body_SetAwake(stBody, true);

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
  // const b2Vec2  vCustomGravity;
  orxSTATUS     eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);

  /* Gets body */
  stBody = _pstBody->stBody;

  //! TODO
  // /* Gets its custom gravity */
  // vCustomGravity = poBody->GetCustomGravity();

  // /* Has new custom gravity? */
  // if(_pvCustomGravity != orxNULL)
  // {
  //   b2Vec2 vGravity;

  //   /* Sets gravity vector */
  //   vGravity.Set(sstPhysics.fDimensionRatio * _pvCustomGravity->fX, sstPhysics.fDimensionRatio * _pvCustomGravity->fY);

  //   /* Should apply? */
  //   if((pvCustomGravity == NULL) || (pvCustomGravity->x != vGravity.x) || (pvCustomGravity->y != vGravity.y))
  //   {
  //     /* Wakes up */
  //     poBody->SetAwake(true);

  //     /* Updates it */
  //     poBody->SetCustomGravity(&vGravity);
  //   }
  // }
  // else
  // {
  //   /* Should apply */
  //   if(pvCustomGravity != NULL)
  //   {
  //     /* Wakes up */
  //     poBody->SetAwake(true);

  //     /* Removes it */
  //     poBody->SetCustomGravity(orxNULL);
  //   }
  // }

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
  b2BodyId  stBody;
  orxFLOAT  fResult;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);

  /* Gets body */
  stBody = _pstBody->stBody;

  /* Gets its rotation */
  fResult = b2Rot_GetAngle(b2Body_GetRotation(stBody));

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
  // const b2Vec2  vGravity;
  orxVECTOR    *pvResult = orxNULL;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);
  orxASSERT(_pvCustomGravity != orxNULL);

  /* Gets body */
  stBody = _pstBody->stBody;

  //! TODO
  // /* Gets its custom gravity */
  // pvGravity = poBody->GetCustomGravity();

  // /* Found? */
  // if(pvGravity != orxNULL)
  // {
  //   /* Updates result */
  //   orxVector_Set(_pvCustomGravity, sstPhysics.fRecDimensionRatio * pvGravity->x, sstPhysics.fRecDimensionRatio * pvGravity->y, orxFLOAT_0);
  //   pvResult = _pvCustomGravity;
  // }
  // else
  // {
  //   /* Clears result */
  //   pvResult = orxNULL;
  // }

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

orxSTATUS orxFASTCALL orxPhysics_Box2D_SetPartSelfFlags(orxPHYSICS_BODY_PART *_pstBodyPart, orxU16 _u16SelfFlags)
{
  b2ShapeId stShape;
  b2Filter  stFilter;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyPart != orxNULL);

  /* Gets shape */
  stShape = _pstBodyPart->stShape;

  /* Gets its current filter */
  stFilter = b2Shape_GetFilter(stShape);

  /* Updates it */
  stFilter.categoryBits = _u16SelfFlags;

  /* Sets new filter */
  b2Shape_SetFilter(stShape, stFilter);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxPhysics_Box2D_SetPartCheckMask(orxPHYSICS_BODY_PART *_pstBodyPart, orxU16 _u16CheckMask)
{
  b2ShapeId stShape;
  b2Filter  stFilter;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyPart != orxNULL);

  /* Gets shape */
  stShape = _pstBodyPart->stShape;

  /* Gets its current filter */
  stFilter = b2Shape_GetFilter(stShape);

  /* Updates it */
  stFilter.maskBits = _u16CheckMask;

  /* Sets new filter */
  b2Shape_SetFilter(stShape, stFilter);

  /* Done! */
  return eResult;
}

orxU16 orxFASTCALL orxPhysics_Box2D_GetPartSelfFlags(const orxPHYSICS_BODY_PART *_pstBodyPart)
{
  b2ShapeId stShape;
  orxU16    u16Result;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyPart != orxNULL);

  /* Gets shape */
  stShape = _pstBodyPart->stShape;

  /* Updates result */
  u16Result = (orxU16)(b2Shape_GetFilter(stShape).categoryBits);

  /* Done! */
  return u16Result;
}

orxU16 orxFASTCALL orxPhysics_Box2D_GetPartCheckMask(const orxPHYSICS_BODY_PART *_pstBodyPart)
{
  b2ShapeId stShape;
  orxU16    u16Result;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyPart != orxNULL);

  /* Gets shape */
  stShape = _pstBodyPart->stShape;

  /* Updates result */
  u16Result = (orxU16)(b2Shape_GetFilter(stShape).maskBits);

  /* Done! */
  return u16Result;
}

orxSTATUS orxFASTCALL orxPhysics_Box2D_SetPartSolid(orxPHYSICS_BODY_PART *_pstBodyPart, orxBOOL _bSolid)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyPart != orxNULL);

  /* Logs message */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "Can't change the solidity of a part: this feature isn't supported by this plugin.");

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
  bResult = b2Shape_IsSensor(stShape) ? orxFALSE : orxTRUE;

  /* Done! */
  return bResult;
}

orxSTATUS orxFASTCALL orxPhysics_Box2D_SetPartFriction(orxPHYSICS_BODY_PART *_pstBodyPart, orxFLOAT _fFriction)
{
  b2ShapeId stShape;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyPart != orxNULL);

  /* Gets shape */
  stShape = _pstBodyPart->stShape;

  /* Updates it */
  b2Shape_SetFriction(stShape, _fFriction);

  /* Done! */
  return eResult;
}

orxFLOAT orxFASTCALL orxPhysics_Box2D_GetPartFriction(const orxPHYSICS_BODY_PART *_pstBodyPart)
{
  b2ShapeId stShape;
  orxFLOAT  fResult;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyPart != orxNULL);

  /* Gets shape */
  stShape = _pstBodyPart->stShape;

  /* Updates result */
  fResult = orx2F(b2Shape_GetFriction(stShape));

  /* Done! */
  return fResult;
}

orxSTATUS orxFASTCALL orxPhysics_Box2D_SetPartRestitution(orxPHYSICS_BODY_PART *_pstBodyPart, orxFLOAT _fRestitution)
{
  b2ShapeId stShape;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyPart != orxNULL);

  /* Gets shape */
  stShape = _pstBodyPart->stShape;

  /* Updates it */
  b2Shape_SetRestitution(stShape, _fRestitution);

  /* Done! */
  return eResult;
}

orxFLOAT orxFASTCALL orxPhysics_Box2D_GetPartRestitution(const orxPHYSICS_BODY_PART *_pstBodyPart)
{
  b2ShapeId stShape;
  orxFLOAT  fResult;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyPart != orxNULL);

  /* Gets shape */
  stShape = _pstBodyPart->stShape;

  /* Updates result */
  fResult = orx2F(b2Shape_GetRestitution(stShape));

  /* Done! */
  return fResult;
}

orxSTATUS orxFASTCALL orxPhysics_Box2D_SetPartDensity(orxPHYSICS_BODY_PART *_pstBodyPart, orxFLOAT _fDensity)
{
  b2ShapeId stShape;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyPart != orxNULL);

  /* Gets shape */
  stShape = _pstBodyPart->stShape;

  /* Updates it */
  b2Shape_SetDensity(stShape, _fDensity, true);

  /* Done! */
  return eResult;
}

orxFLOAT orxFASTCALL orxPhysics_Box2D_GetPartDensity(const orxPHYSICS_BODY_PART *_pstBodyPart)
{
  b2ShapeId stShape;
  orxFLOAT  fResult;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyPart != orxNULL);

  /* Gets shape */
  stShape = _pstBodyPart->stShape;

  /* Updates result */
  fResult = orx2F(b2Shape_GetDensity(stShape));

  /* Done! */
  return fResult;
}

orxBOOL orxFASTCALL orxPhysics_Box2D_IsInsidePart(const orxPHYSICS_BODY_PART *_pstBodyPart, const orxVECTOR *_pvPosition)
{
  b2ShapeId stShape;
  b2Vec2    vPosition;
  orxBOOL     bResult;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyPart != orxNULL);

  /* Gets shape */
  stShape = _pstBodyPart->stShape;

  /* Sets position */
  vPosition.x = sstPhysics.fDimensionRatio * _pvPosition->fX;
  vPosition.y = sstPhysics.fDimensionRatio * _pvPosition->fY;

  /* Updates result */
  bResult = (b2Shape_TestPoint(stShape, vPosition) != false) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

orxHANDLE orxFASTCALL orxPhysics_Box2D_Raycast(const orxVECTOR *_pvBegin, const orxVECTOR *_pvEnd, orxU16 _u16SelfFlags, orxU16 _u16CheckMask, orxBOOL _bEarlyExit, orxVECTOR *_pvContact, orxVECTOR *_pvNormal)
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
  stFilter.categoryBits = _u16SelfFlags;
  stFilter.maskBits     = _u16CheckMask;

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

orxU32 orxFASTCALL orxPhysics_Box2D_BoxPick(const orxAABOX *_pstBox, orxU16 _u16SelfFlags, orxU16 _u16CheckMask, orxHANDLE _ahUserDataList[], orxU32 _u32Number)
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
  stFilter.categoryBits = _u16SelfFlags;
  stFilter.maskBits     = _u16CheckMask;

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
    if(sstPhysics.stWorld.index1 != 0)
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

        /* Success? */
        if((sstPhysics.pstBodyBank != orxNULL) && (sstPhysics.pstBodyPartBank != orxNULL))
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
// orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_CreateJoint, PHYSICS, CREATE_JOINT);
// orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_DeleteJoint, PHYSICS, DELETE_JOINT);
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
// orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_EnableMotor, PHYSICS, ENABLE_MOTOR);
// orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_SetJointMotorSpeed, PHYSICS, SET_JOINT_MOTOR_SPEED);
// orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_SetJointMaxMotorTorque, PHYSICS, SET_JOINT_MAX_MOTOR_TORQUE);
// orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_GetJointReactionForce, PHYSICS, GET_JOINT_REACTION_FORCE);
// orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_GetJointReactionTorque, PHYSICS, GET_JOINT_REACTION_TORQUE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_Raycast, PHYSICS, RAYCAST);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_BoxPick, PHYSICS, BOX_PICK);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_EnableSimulation, PHYSICS, ENABLE_SIMULATION);
orxPLUGIN_USER_CORE_FUNCTION_END();


#if defined(__orxWINDOWS__) && !defined(__orxMSVC__)

  #undef alloca

#endif /* __orxWINDOWS__ && !__orxMSVC__ */
