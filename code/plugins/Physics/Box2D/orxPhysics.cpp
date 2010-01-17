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
 * @file orxPhysics.cpp
 * @date 24/03/2008
 * @author iarwain@orx-project.org
 *
 * Box2D physics plugin implementation
 *
 */


#include "orxPluginAPI.h"

#include <Box2D/Box2D.h>


#ifdef __orxMSVC__

  #pragma warning(disable : 4311 4312)

#endif /* __orxMSVC__ */


/** Module flags
 */
#define orxPHYSICS_KU32_STATIC_FLAG_NONE        0x00000000 /**< No flags */

#define orxPHYSICS_KU32_STATIC_FLAG_READY       0x00000001 /**< Ready flag */

#define orxPHYSICS_KU32_STATIC_FLAG_ENABLED     0x00000002 /**< Enabled flag */

#define orxPHYSICS_KU32_STATIC_MASK_ALL         0xFFFFFFFF /**< All mask */

namespace orxPhysics
{
  static const orxU32   su32DefaultIterations   = 10;
  static const orxFLOAT sfDefaultDimensionRatio = orx2F(0.01f);
  static const orxU32   su32MessageBankSize     = 64;
}


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Event storage
 */
typedef struct __orxPHYSICS_EVENT_STORAGE_t
{
  orxLINKLIST_NODE                  stNode;           /**< Link list node */
  orxPHYSICS_EVENT_PAYLOAD          stPayload;        /**< Event payload */
  orxPHYSICS_EVENT                  eID;              /**< Event ID */
  b2Body                           *poSource;         /**< Event source */
  b2Body                           *poDestination;    /**< Event destination */

} orxPHYSICS_EVENT_STORAGE;

/** Contact listener
 */
class orxPhysicsContactListener : public b2ContactListener
{
public:
  void BeginContact(b2Contact *_poContact);
  void EndContact(b2Contact *_poContact);
};

/** Boundary listener
 */
class orxPhysicsBoundaryListener : public b2BoundaryListener
{
public:
  void Violation(b2Body *_poBody);
};

/** Static structure
 */
typedef struct __orxPHYSICS_STATIC_t
{
  orxU32                      u32Flags;           /**< Control flags */
  orxU32                      u32Iterations;      /**< Simulation iterations per step */
  orxFLOAT                    fDimensionRatio;    /**< Dimension ratio */
  orxFLOAT                    fRecDimensionRatio; /**< Reciprocal dimension ratio */
  orxLINKLIST                 stEventList;        /**< Event link list */
  orxBANK                    *pstEventBank;       /**< Event bank */
  b2World                    *poWorld;            /**< World */
  b2Fixture                  *poRaycastFixture;   /**< Raycast fixture */
  orxPhysicsContactListener  *poContactListener;  /**< Contact listener */
  orxPhysicsBoundaryListener *poBoundaryListener; /**< Boundary listener */

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

static orxINLINE orxU32 orxPhysics_Box2D_GetFixtureIndex(const b2Body *_poBody, const b2Fixture *_poFixture)
{
  orxBODY  *pstBody;
  orxU32    u32Result = orxU32_UNDEFINED;

  /* Checks */
  orxASSERT(_poBody != orxNULL);
  orxASSERT(_poFixture != orxNULL);

  /* Gets corresponding body */
  pstBody = orxBODY(const_cast<b2Body *>(_poBody)->GetUserData());

  /* Valid? */
  if(pstBody != orxNULL)
  {
    orxPHYSICS_BODY_PART *pstBodyPart;
    orxU32                i;

    /* For all parts */
    for(i = 0; i < orxBODY_KU32_PART_MAX_NUMBER; i++)
    {
      /* Gets part */
      pstBodyPart = orxBody_GetPart(pstBody, i);

      /* Valid? */
      if(pstBodyPart != orxNULL)
      {
        /* Found? */
        if((orxHANDLE)pstBodyPart == (orxHANDLE)_poFixture)
        {
          /* Updates result */
          u32Result = i;

          break;
        }
      }
      else
      {
        /* Stops */
        break;
      }
    }
  }

  /* Done! */
  return u32Result;
}

static void orxFASTCALL orxPhysics_Box2D_SendContactEvent(b2Contact *_poContact, orxPHYSICS_EVENT _eEventID)
{
  b2Body *poSource, *poDestination;

  /* Gets both bodies */
  poSource      = _poContact->GetFixtureA()->GetBody();
  poDestination = _poContact->GetFixtureB()->GetBody();

  /* Doesn't involve ground? */
  if((poSource != sstPhysics.poWorld->GetGroundBody())
  && (poDestination != sstPhysics.poWorld->GetGroundBody()))
  {
    orxPHYSICS_EVENT_STORAGE *pstEventStorage;
    orxBOOL                   bSendEvent = orxTRUE;

    /* For all pending events */
    for(pstEventStorage = (orxPHYSICS_EVENT_STORAGE *)orxLinkList_GetFirst(&(sstPhysics.stEventList));
        pstEventStorage != orxNULL;
        pstEventStorage = (orxPHYSICS_EVENT_STORAGE *)orxLinkList_GetNext(&(pstEventStorage->stNode)))
    {
      /* Same pair? */
      if((pstEventStorage->poSource == poSource) && (pstEventStorage->poDestination == poDestination))
      {
        /* Depending on old event */
        switch(pstEventStorage->eID)
        {
          case orxPHYSICS_EVENT_CONTACT_ADD:
          {
            /* Removes it */
            orxLinkList_Remove(&(pstEventStorage->stNode));
            orxBank_Free(sstPhysics.pstEventBank, pstEventStorage);

            /* Removing it? */
            if(_eEventID == orxPHYSICS_EVENT_CONTACT_REMOVE)
            {
              /* Don't send event */
              bSendEvent = orxFALSE;
            }

            break;
          }

          case orxPHYSICS_EVENT_CONTACT_REMOVE:
          {
            /* Removes it */
            orxLinkList_Remove(&(pstEventStorage->stNode));
            orxBank_Free(sstPhysics.pstEventBank, pstEventStorage);

            /* Is new one a add? */
            if(_eEventID == orxPHYSICS_EVENT_CONTACT_ADD)
            {
              /* Don't send event */
              bSendEvent = orxFALSE;
            }

            break;
          }

          default:
          {
            break;
          }
        }

        break;
      }
    }

    /* Should send the event? */
    if(bSendEvent != orxFALSE)
    {
      orxU32 u32SourcePartIndex, u32DestinationPartIndex;

      /* Gets part indexes */
      u32SourcePartIndex      = orxPhysics_Box2D_GetFixtureIndex(poSource, _poContact->GetFixtureA());
      u32DestinationPartIndex = orxPhysics_Box2D_GetFixtureIndex(poDestination, _poContact->GetFixtureB());

      /* Valid? */
      if((u32SourcePartIndex != orxU32_UNDEFINED) && (u32DestinationPartIndex != orxU32_UNDEFINED))
      {
        orxPHYSICS_EVENT_STORAGE *pstEventStorage;

        /* Adds a contact event */
        pstEventStorage = (orxPHYSICS_EVENT_STORAGE *)orxBank_Allocate(sstPhysics.pstEventBank);

        /* Valid? */
        if(pstEventStorage != orxNULL)
        {
          b2WorldManifold oManifold;

          /* Gets manifold */
          _poContact->GetWorldManifold(&oManifold);

          /* Adds it to list */
          orxLinkList_AddEnd(&(sstPhysics.stEventList), &(pstEventStorage->stNode));

          /* Inits it */
          pstEventStorage->eID            = _eEventID;
          pstEventStorage->poSource       = poSource;
          pstEventStorage->poDestination  = poDestination;

          /* Contact add? */
          if(_eEventID == orxPHYSICS_EVENT_CONTACT_ADD)
          {
            orxVector_Set(&(pstEventStorage->stPayload.vPosition), sstPhysics.fRecDimensionRatio * oManifold.m_points[0].x, sstPhysics.fRecDimensionRatio * oManifold.m_points[0].y, orxFLOAT_0);
            orxVector_Set(&(pstEventStorage->stPayload.vNormal), oManifold.m_normal.x, oManifold.m_normal.y, orxFLOAT_0);
          }
          else
          {
            orxVector_Copy(&(pstEventStorage->stPayload.vPosition), &orxVECTOR_0);
            orxVector_Copy(&(pstEventStorage->stPayload.vNormal), &orxVECTOR_0);
          }

          /* Updates part names */
          pstEventStorage->stPayload.zSenderPartName    = orxBody_GetPartName(orxBODY(poSource->GetUserData()), u32SourcePartIndex);
          pstEventStorage->stPayload.zRecipientPartName = orxBody_GetPartName(orxBODY(poDestination->GetUserData()), u32DestinationPartIndex);
        }
      }
    }
  }

  return;
}

void orxPhysicsContactListener::BeginContact(b2Contact *_poContact)
{
  /* Sends contact event */
  orxPhysics_Box2D_SendContactEvent(_poContact, orxPHYSICS_EVENT_CONTACT_ADD);

  return;
}

void orxPhysicsContactListener::EndContact(b2Contact *_poContact)
{
  /* Sends contact event */
  orxPhysics_Box2D_SendContactEvent(_poContact, orxPHYSICS_EVENT_CONTACT_REMOVE);

  return;
}

void orxPhysicsBoundaryListener::Violation(b2Body *_poBody)
{
  orxPHYSICS_EVENT_STORAGE *pstEventStorage;

  /* Adds an out of world event */
  pstEventStorage = (orxPHYSICS_EVENT_STORAGE *)orxBank_Allocate(sstPhysics.pstEventBank);

  /* Valid? */
  if(pstEventStorage != orxNULL)
  {
    /* Adds it to list */
    orxLinkList_AddEnd(&(sstPhysics.stEventList), &(pstEventStorage->stNode));

    /* Inits it */
    pstEventStorage->eID      = orxPHYSICS_EVENT_OUT_OF_WORLD;
    pstEventStorage->poSource = _poBody;
  }

  return;
}


/** Update (callback to register on a clock)
 * @param[in]   _pstClockInfo   Clock info of the clock used upon registration
 * @param[in]   _pContext     Context sent when registering callback to the clock
 */
static void orxFASTCALL orxPhysics_Update(const orxCLOCK_INFO *_pstClockInfo, void *_pContext)
{
  orxPHYSICS_EVENT_STORAGE *pstEventStorage;
  orxBODY                  *pstBody;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstClockInfo != orxNULL);

  /* For all bodies */
  for(pstBody = orxBODY(orxStructure_GetFirst(orxSTRUCTURE_ID_BODY));
      pstBody != orxNULL;
      pstBody = orxBODY(orxStructure_GetNext(pstBody)))
  {
    orxFRAME *pstFrame;

    /* Gets owner's frame */
    pstFrame = orxOBJECT_GET_STRUCTURE(orxOBJECT(orxBody_GetOwner(pstBody)), FRAME);

    /* Is dirty? */
    if(orxFrame_IsDirty(pstFrame) != orxFALSE)
    {
      orxVECTOR vPos;

      /* Updates body's position */
      orxBody_SetPosition(pstBody, orxFrame_GetPosition(pstFrame, orxFRAME_SPACE_GLOBAL, &vPos));
    }
  }

  /* Is simulation enabled? */
  if(orxFLAG_TEST(sstPhysics.u32Flags, orxPHYSICS_KU32_STATIC_FLAG_ENABLED))
  {
    /* Updates world simulation */
    sstPhysics.poWorld->Step(_pstClockInfo->fDT, (orxU32)_pContext, (orxU32)_pContext);

    /* For all bodies */
    for(pstBody = orxBODY(orxStructure_GetFirst(orxSTRUCTURE_ID_BODY));
        pstBody != orxNULL;
        pstBody = orxBODY(orxStructure_GetNext(pstBody)))
    {
      /* Applies simulation result */
      orxBody_ApplySimulationResult(pstBody);
    }

    /* For all stored events */
    for(pstEventStorage = (orxPHYSICS_EVENT_STORAGE *)orxLinkList_GetFirst(&(sstPhysics.stEventList));
        pstEventStorage != orxNULL;
        pstEventStorage = (orxPHYSICS_EVENT_STORAGE *)orxLinkList_GetNext(&(pstEventStorage->stNode)))
    {
      /* Depending on type */
      switch(pstEventStorage->eID)
      {
        case orxPHYSICS_EVENT_OUT_OF_WORLD:
        {
          orxSTRUCTURE *pstOwner;

          /* Gets owner */
          pstOwner = orxBody_GetOwner(orxBODY(pstEventStorage->poSource->GetUserData()));

          /* Sends event */
          orxEVENT_SEND(orxEVENT_TYPE_PHYSICS, orxPHYSICS_EVENT_OUT_OF_WORLD, pstOwner, pstOwner, orxNULL);

          break;
        }

        case orxPHYSICS_EVENT_CONTACT_ADD:
        case orxPHYSICS_EVENT_CONTACT_REMOVE:
        {
          /* New contact? */
          if(pstEventStorage->eID == orxPHYSICS_EVENT_CONTACT_ADD)
          {
            b2Vec2 vPos;

            /* Source can't slide and destination is static? */
            if(!pstEventStorage->poSource->CanSlide() && pstEventStorage->poDestination->IsStatic())
            {
              /* Gets current position */
              vPos = pstEventStorage->poSource->GetPosition();

              /* Grounds it*/
              vPos.y += 0.01f;

              /* Updates it */
              pstEventStorage->poSource->SetXForm(vPos, pstEventStorage->poSource->GetAngle());
            }
            /* Destination can't slide and source is static? */
            else if(!pstEventStorage->poDestination->CanSlide() && pstEventStorage->poSource->IsStatic())
            {
              /* Gets current position */
              vPos = pstEventStorage->poDestination->GetPosition();

              /* Grounds it*/
              vPos.y += 0.01f;

              /* Updates it */
              pstEventStorage->poDestination->SetXForm(vPos, pstEventStorage->poDestination->GetAngle());
            }
          }

          /* Sends event */
          orxEVENT_SEND(orxEVENT_TYPE_PHYSICS, pstEventStorage->eID, orxBody_GetOwner(orxBODY(pstEventStorage->poSource->GetUserData())), orxBody_GetOwner(orxBODY(pstEventStorage->poDestination->GetUserData())), &(pstEventStorage->stPayload));

          break;
        }

        default:
        {
          break;
        }
      }
    }

    /* Clears stored events */
    orxLinkList_Clean(&(sstPhysics.stEventList));
    orxBank_Clear(sstPhysics.pstEventBank);
  }

  return;
}

extern "C" orxPHYSICS_BODY *orxFASTCALL orxPhysics_Box2D_CreateBody(const orxHANDLE _hUserData, const orxBODY_DEF *_pstBodyDef)
{
  b2Body     *poResult = 0;
  b2BodyDef   stBodyDef;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_hUserData != orxHANDLE_UNDEFINED);
  orxASSERT(_pstBodyDef != orxNULL);

  /* 2D? */
  if(orxFLAG_TEST(_pstBodyDef->u32Flags, orxBODY_DEF_KU32_FLAG_2D))
  {
    /* Inits body definition */
    stBodyDef.userData          = _hUserData;
    stBodyDef.angle             = _pstBodyDef->fRotation;
    stBodyDef.linearDamping     = _pstBodyDef->fLinearDamping;
    stBodyDef.angularDamping    = _pstBodyDef->fAngularDamping;
    stBodyDef.isBullet          = orxFLAG_TEST(_pstBodyDef->u32Flags, orxBODY_DEF_KU32_FLAG_HIGH_SPEED);
    stBodyDef.fixedRotation     = orxFLAG_TEST(_pstBodyDef->u32Flags, orxBODY_DEF_KU32_FLAG_FIXED_ROTATION);
    stBodyDef.canSlide          = orxFLAG_TEST(_pstBodyDef->u32Flags, orxBODY_DEF_KU32_FLAG_CAN_SLIDE);
    stBodyDef.position.Set(sstPhysics.fDimensionRatio * _pstBodyDef->vPosition.fX, sstPhysics.fDimensionRatio * _pstBodyDef->vPosition.fY);

    /* Is dynamic? */
    if(orxFLAG_TEST(_pstBodyDef->u32Flags, orxBODY_DEF_KU32_FLAG_DYNAMIC))
    {
      /* Stores mass properties */
      stBodyDef.massData.I      = (_pstBodyDef->fInertia != 0.0f) ? _pstBodyDef->fInertia : 1.0f;
      stBodyDef.massData.mass   = (_pstBodyDef->fMass != 0.0f) ? _pstBodyDef->fMass : 1.0f;;

      /* Creates dynamic body */
      poResult = sstPhysics.poWorld->CreateBody(&stBodyDef);
    }
    else
    {
      /* Cleans mass properties */
      stBodyDef.massData.I      = 0.0f;
      stBodyDef.massData.mass   = 0.0f;

      /* Creates static body */
      poResult = sstPhysics.poWorld->CreateBody(&stBodyDef);
    }
  }

  /* Done! */
  return (orxPHYSICS_BODY *)poResult;
}

extern "C" void orxFASTCALL orxPhysics_Box2D_DeleteBody(orxPHYSICS_BODY *_pstBody)
{
  orxPHYSICS_EVENT_STORAGE *pstEventStorage;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);

  /* For all stored events */
  for(pstEventStorage = (orxPHYSICS_EVENT_STORAGE *)orxLinkList_GetFirst(&(sstPhysics.stEventList));
      pstEventStorage != orxNULL;
      pstEventStorage = (pstEventStorage == orxNULL) ? (orxPHYSICS_EVENT_STORAGE *)orxLinkList_GetFirst(&(sstPhysics.stEventList)) : (orxPHYSICS_EVENT_STORAGE *)orxLinkList_GetNext(&(pstEventStorage->stNode)))
  {
    /* Is part of the event? */
    if(((b2Body *)_pstBody == pstEventStorage->poDestination) || ((b2Body *)_pstBody == pstEventStorage->poSource))
    {
      orxPHYSICS_EVENT_STORAGE *pstCurrentEventStorage;

      /* Gets current event */
      pstCurrentEventStorage = pstEventStorage;

      /* Goes back to previous */
      pstEventStorage = (orxPHYSICS_EVENT_STORAGE *)orxLinkList_GetPrevious(&(pstEventStorage->stNode));

      /* Removes event */
      orxLinkList_Remove(&(pstCurrentEventStorage->stNode));
      orxBank_Free(sstPhysics.pstEventBank, pstCurrentEventStorage);
    }
  }

  /* Deletes it */
  sstPhysics.poWorld->DestroyBody((b2Body *)_pstBody);

  return;
}

extern "C" orxPHYSICS_BODY_PART *orxFASTCALL orxPhysics_Box2D_CreateBodyPart(orxPHYSICS_BODY *_pstBody, const orxBODY_PART_DEF *_pstBodyPartDef)
{
  b2Body       *poBody;
  b2Fixture    *poResult = 0;
  b2FixtureDef *pstFixtureDef;
  b2CircleDef   stCircleDef;
  b2PolygonDef  stPolygonDef;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);
  orxASSERT(_pstBodyPartDef != orxNULL);
  orxASSERT(orxFLAG_TEST(_pstBodyPartDef->u32Flags, orxBODY_PART_DEF_KU32_FLAG_BOX | orxBODY_PART_DEF_KU32_FLAG_SPHERE | orxBODY_PART_DEF_KU32_FLAG_MESH));

  /* Gets body */
  poBody = (b2Body *)_pstBody;

  /* Circle? */
  if(orxFLAG_TEST(_pstBodyPartDef->u32Flags, orxBODY_PART_DEF_KU32_FLAG_SPHERE))
  {
    /* Gets def reference */
    pstFixtureDef = &stCircleDef;

    /* Updates Fixture type */
    stCircleDef.type = b2_circleShape;

    /* Stores its coordinates */
    stCircleDef.localPosition.Set(sstPhysics.fDimensionRatio * _pstBodyPartDef->stSphere.vCenter.fX * _pstBodyPartDef->vScale.fX, sstPhysics.fDimensionRatio * _pstBodyPartDef->stSphere.vCenter.fY * _pstBodyPartDef->vScale.fY);
    stCircleDef.radius = sstPhysics.fDimensionRatio * _pstBodyPartDef->stSphere.fRadius * orx2F(0.5f) * (orxMath_Abs(_pstBodyPartDef->vScale.fX) + orxMath_Abs(_pstBodyPartDef->vScale.fY));
  }
  /* Polygon */
  else
  {
    /* Gets def reference */
    pstFixtureDef = &stPolygonDef;

    /* Updates Fixture type */
    stPolygonDef.type = b2_polygonShape;

    /* Box? */
    if(orxFLAG_TEST(_pstBodyPartDef->u32Flags, orxBODY_PART_DEF_KU32_FLAG_BOX))
    {
      /* No mirroring? */
      if(_pstBodyPartDef->vScale.fX * _pstBodyPartDef->vScale.fY > orxFLOAT_0)
      {
        /* Stores its coordinates */
        stPolygonDef.vertexCount = 4;
        stPolygonDef.vertices[0].Set(sstPhysics.fDimensionRatio * _pstBodyPartDef->stAABox.stBox.vBR.fX * _pstBodyPartDef->vScale.fX, sstPhysics.fDimensionRatio * _pstBodyPartDef->stAABox.stBox.vTL.fY * _pstBodyPartDef->vScale.fY);
        stPolygonDef.vertices[1].Set(sstPhysics.fDimensionRatio * _pstBodyPartDef->stAABox.stBox.vBR.fX * _pstBodyPartDef->vScale.fX, sstPhysics.fDimensionRatio * _pstBodyPartDef->stAABox.stBox.vBR.fY * _pstBodyPartDef->vScale.fY);
        stPolygonDef.vertices[2].Set(sstPhysics.fDimensionRatio * _pstBodyPartDef->stAABox.stBox.vTL.fX * _pstBodyPartDef->vScale.fX, sstPhysics.fDimensionRatio * _pstBodyPartDef->stAABox.stBox.vBR.fY * _pstBodyPartDef->vScale.fY);
        stPolygonDef.vertices[3].Set(sstPhysics.fDimensionRatio * _pstBodyPartDef->stAABox.stBox.vTL.fX * _pstBodyPartDef->vScale.fX, sstPhysics.fDimensionRatio * _pstBodyPartDef->stAABox.stBox.vTL.fY * _pstBodyPartDef->vScale.fY);
      }
      else
      {
        /* Stores its coordinates */
        stPolygonDef.vertexCount = 4;
        stPolygonDef.vertices[0].Set(sstPhysics.fDimensionRatio * _pstBodyPartDef->stAABox.stBox.vTL.fX * _pstBodyPartDef->vScale.fX, sstPhysics.fDimensionRatio * _pstBodyPartDef->stAABox.stBox.vTL.fY * _pstBodyPartDef->vScale.fY);
        stPolygonDef.vertices[1].Set(sstPhysics.fDimensionRatio * _pstBodyPartDef->stAABox.stBox.vTL.fX * _pstBodyPartDef->vScale.fX, sstPhysics.fDimensionRatio * _pstBodyPartDef->stAABox.stBox.vBR.fY * _pstBodyPartDef->vScale.fY);
        stPolygonDef.vertices[2].Set(sstPhysics.fDimensionRatio * _pstBodyPartDef->stAABox.stBox.vBR.fX * _pstBodyPartDef->vScale.fX, sstPhysics.fDimensionRatio * _pstBodyPartDef->stAABox.stBox.vBR.fY * _pstBodyPartDef->vScale.fY);
        stPolygonDef.vertices[3].Set(sstPhysics.fDimensionRatio * _pstBodyPartDef->stAABox.stBox.vBR.fX * _pstBodyPartDef->vScale.fX, sstPhysics.fDimensionRatio * _pstBodyPartDef->stAABox.stBox.vTL.fY * _pstBodyPartDef->vScale.fY);
      }
    }
    else
    {
      orxU32 i;

      /* Checks */
      orxASSERT(_pstBodyPartDef->stMesh.u32VertexCounter > 0);
      orxASSERT(orxBODY_PART_DEF_KU32_MESH_VERTEX_NUMBER <= b2_maxPolygonVertices);

      /* Updates the vertex counter */
      stPolygonDef.vertexCount = _pstBodyPartDef->stMesh.u32VertexCounter;

      /* No mirroring? */
      if(_pstBodyPartDef->vScale.fX * _pstBodyPartDef->vScale.fY > orxFLOAT_0)
      {
        /* For all the vertices */
        for(i = 0; i < _pstBodyPartDef->stMesh.u32VertexCounter; i++)
        {
          /* Sets its vector */
          stPolygonDef.vertices[i].Set(sstPhysics.fDimensionRatio * _pstBodyPartDef->stMesh.avVertices[i].fX * _pstBodyPartDef->vScale.fX, sstPhysics.fDimensionRatio * _pstBodyPartDef->stMesh.avVertices[i].fY * _pstBodyPartDef->vScale.fY);
        }
      }
      else
      {
        orxS32 iDst;

        /* For all the vertices */
        for(iDst = _pstBodyPartDef->stMesh.u32VertexCounter - 1, i = 0; iDst >= 0; iDst--, i++)
        {
          /* Sets its vector */
          stPolygonDef.vertices[iDst].Set(sstPhysics.fDimensionRatio * _pstBodyPartDef->stMesh.avVertices[i].fX * _pstBodyPartDef->vScale.fX, sstPhysics.fDimensionRatio * _pstBodyPartDef->stMesh.avVertices[i].fY * _pstBodyPartDef->vScale.fY);
        }
      }
    }
  }

  /* Inits Fixture definition */
  pstFixtureDef->friction             = _pstBodyPartDef->fFriction;
  pstFixtureDef->restitution          = _pstBodyPartDef->fRestitution;
  pstFixtureDef->density              = poBody->IsStatic() ? 0.0f : _pstBodyPartDef->fDensity;
  pstFixtureDef->filter.categoryBits  = _pstBodyPartDef->u16SelfFlags;
  pstFixtureDef->filter.maskBits      = _pstBodyPartDef->u16CheckMask;
  pstFixtureDef->filter.groupIndex    = 0;
  pstFixtureDef->isSensor             = orxFLAG_TEST(_pstBodyPartDef->u32Flags, orxBODY_PART_DEF_KU32_FLAG_SOLID) == orxFALSE;

  /* Creates it */
  poResult = poBody->CreateFixture(pstFixtureDef);

  /* Valid? */
  if(poResult != 0)
  {
    /* Computes body's mass */
    poBody->SetMassFromShapes();
  }

  /* Done! */
  return (orxPHYSICS_BODY_PART *)poResult;
}

extern "C" void orxFASTCALL orxPhysics_Box2D_DeleteBodyPart(orxPHYSICS_BODY_PART *_pstBodyPart)
{
  b2Fixture  *poFixture;
  b2Body     *poBody;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyPart != orxNULL);

  /* Gets Fixture */
  poFixture = (b2Fixture *)_pstBodyPart;

  /* Gets its body */
  poBody = poFixture->GetBody();

  /* Deletes its part */
  poBody->DestroyFixture(poFixture);

  return;
}

extern "C" orxSTATUS orxFASTCALL orxPhysics_Box2D_SetPosition(orxPHYSICS_BODY *_pstBody, const orxVECTOR *_pvPosition)
{
  b2Body   *poBody;
  b2Vec2    vPosition;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);
  orxASSERT(_pvPosition != orxNULL);

  /* Gets body */
  poBody = (b2Body *)_pstBody;

  /* Wakes up */
  poBody->WakeUp();

  /* Sets position vector */
  vPosition.Set(sstPhysics.fDimensionRatio * _pvPosition->fX, sstPhysics.fDimensionRatio * _pvPosition->fY);

  /* Updates its position */
  eResult = (poBody->SetXForm(vPosition, poBody->GetAngle()) != false) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxPhysics_Box2D_SetRotation(orxPHYSICS_BODY *_pstBody, orxFLOAT _fRotation)
{
  b2Body   *poBody;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);

  /* Gets body */
  poBody = (b2Body *)_pstBody;

  /* Wakes up */
  poBody->WakeUp();

  /* Updates its rotation */
  eResult = (poBody->SetXForm(poBody->GetPosition(), _fRotation) != false) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxPhysics_Box2D_SetSpeed(orxPHYSICS_BODY *_pstBody, const orxVECTOR *_pvSpeed)
{
  b2Body   *poBody;
  b2Vec2    vSpeed;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);
  orxASSERT(_pvSpeed != orxNULL);

  /* Gets body */
  poBody = (b2Body *)_pstBody;

  /* Sets speed vector */
  vSpeed.Set(sstPhysics.fDimensionRatio * _pvSpeed->fX, sstPhysics.fDimensionRatio * _pvSpeed->fY);

  /* Wakes up */
  poBody->WakeUp();

  /* Updates its speed */
  poBody->SetLinearVelocity(vSpeed);

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxPhysics_Box2D_SetAngularVelocity(orxPHYSICS_BODY *_pstBody, orxFLOAT _fVelocity)
{
  b2Body   *poBody;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);

  /* Gets body */
  poBody = (b2Body *)_pstBody;

  /* Wakes up */
  poBody->WakeUp();

  /* Updates its angular velocity */
  poBody->SetAngularVelocity(_fVelocity);

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxPhysics_Box2D_SetCustomGravity(orxPHYSICS_BODY *_pstBody, const orxVECTOR *_pvCustomGravity)
{
  b2Body   *poBody;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);

  /* Gets body */
  poBody = (b2Body *)_pstBody;

  /* Wakes up */
  poBody->WakeUp();

  /* Has custom gravity */
  if(_pvCustomGravity != orxNULL)
  {
    b2Vec2 vGravity;

    /* Sets gravity vector */
    vGravity.Set(sstPhysics.fDimensionRatio * _pvCustomGravity->fX, sstPhysics.fDimensionRatio * _pvCustomGravity->fY);

    /* Updates it */
    poBody->SetCustomGravity(&vGravity);
  }
  else
  {
    /* Removes it */
    poBody->SetCustomGravity(orxNULL);
  }

  /* Done! */
  return eResult;
}

extern "C" orxVECTOR *orxFASTCALL orxPhysics_Box2D_GetPosition(const orxPHYSICS_BODY *_pstBody, orxVECTOR *_pvPosition)
{
  b2Body     *poBody;
  b2Vec2      vPosition;
  orxVECTOR  *pvResult;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);
  orxASSERT(_pvPosition != orxNULL);

  /* Gets body */
  poBody = (b2Body *)_pstBody;

  /* Gets its position */
  vPosition = poBody->GetPosition();

  /* Updates result */
  pvResult      = _pvPosition;
  pvResult->fX  = sstPhysics.fRecDimensionRatio * vPosition.x;
  pvResult->fY  = sstPhysics.fRecDimensionRatio * vPosition.y;

  /* Done! */
  return pvResult;
}

extern "C" orxFLOAT orxFASTCALL orxPhysics_Box2D_GetRotation(const orxPHYSICS_BODY *_pstBody)
{
  b2Body   *poBody;
  orxFLOAT  fResult;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);

  /* Gets body */
  poBody = (b2Body *)_pstBody;

  /* Gets its rotation */
  fResult = poBody->GetAngle();

  /* Done! */
  return fResult;
}

extern "C" orxVECTOR *orxFASTCALL orxPhysics_Box2D_GetSpeed(const orxPHYSICS_BODY *_pstBody, orxVECTOR *_pvSpeed)
{
  b2Body   *poBody;
  b2Vec2    vSpeed;
  orxVECTOR *pvResult;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);
  orxASSERT(_pvSpeed != orxNULL);

  /* Gets body */
  poBody = (b2Body *)_pstBody;

  /* Gets its speed */
  vSpeed = poBody->GetLinearVelocity();

  /* Updates result */
  pvResult      = _pvSpeed;
  pvResult->fX  = sstPhysics.fRecDimensionRatio * vSpeed.x;
  pvResult->fY  = sstPhysics.fRecDimensionRatio * vSpeed.y;
  pvResult->fZ  = orxFLOAT_0;

  /* Done! */
  return pvResult;
}

extern "C" orxFLOAT orxFASTCALL orxPhysics_Box2D_GetAngularVelocity(const orxPHYSICS_BODY *_pstBody)
{
  b2Body   *poBody;
  orxFLOAT  fResult;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);

  /* Gets body */
  poBody = (b2Body *)_pstBody;

  /* Gets its angular velocity */
  fResult = poBody->GetAngularVelocity();

  /* Done! */
  return fResult;
}

extern "C" orxVECTOR *orxFASTCALL orxPhysics_Box2D_GetCustomGravity(const orxPHYSICS_BODY *_pstBody, orxVECTOR *_pvCustomGravity)
{
  b2Body       *poBody;
  const b2Vec2 *pvGravity;
  orxVECTOR    *pvResult;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);
  orxASSERT(_pvCustomGravity != orxNULL);

  /* Gets body */
  poBody = (b2Body *)_pstBody;

  /* Gets its custom gravity */
  pvGravity = poBody->GetCustomGravity();

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

extern "C" orxVECTOR *orxFASTCALL orxPhysics_Box2D_GetMassCenter(const orxPHYSICS_BODY *_pstBody, orxVECTOR *_pvMassCenter)
{
  b2Body     *poBody;
  b2Vec2      vMassCenter;
  orxVECTOR  *pvResult;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);
  orxASSERT(_pvMassCenter != orxNULL);

  /* Gets body */
  poBody = (b2Body *)_pstBody;

  /* Gets its mass center */
  vMassCenter = poBody->GetWorldCenter();

  /* Transfer values */
  _pvMassCenter->fX = sstPhysics.fRecDimensionRatio * vMassCenter.x;
  _pvMassCenter->fY = sstPhysics.fRecDimensionRatio * vMassCenter.y;
  _pvMassCenter->fZ = orxFLOAT_0;

  /* Updates result */
  pvResult = _pvMassCenter;

  /* Done! */
  return pvResult;
}

extern "C" orxSTATUS orxFASTCALL orxPhysics_Box2D_ApplyTorque(orxPHYSICS_BODY *_pstBody, orxFLOAT _fTorque)
{
  b2Body   *poBody;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);

  /* Gets body */
  poBody = (b2Body *)_pstBody;

  /* Wakes up */
  poBody->WakeUp();

  /* Applies torque */
  poBody->ApplyTorque(_fTorque);

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxPhysics_Box2D_ApplyForce(orxPHYSICS_BODY *_pstBody, const orxVECTOR *_pvForce, const orxVECTOR *_pvPoint)
{
  b2Body   *poBody;
  b2Vec2    vForce, vPoint;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);
  orxASSERT(_pvForce != orxNULL);
  orxASSERT(_pvPoint != orxNULL);

  /* Gets body */
  poBody = (b2Body *)_pstBody;

  /* Wakes up */
  poBody->WakeUp();

  /* Sets force */
  vForce.Set(_pvForce->fX, _pvForce->fY);

  /* Sets point */
  vPoint.Set(sstPhysics.fDimensionRatio * _pvPoint->fX, sstPhysics.fDimensionRatio * _pvPoint->fY);

  /* Applies force */
  poBody->ApplyForce(vForce, vPoint);

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxPhysics_Box2D_ApplyImpulse(orxPHYSICS_BODY *_pstBody, const orxVECTOR *_pvImpulse, const orxVECTOR *_pvPoint)
{
  b2Body   *poBody;
  b2Vec2    vImpulse, vPoint;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);
  orxASSERT(_pvImpulse != orxNULL);
  orxASSERT(_pvPoint != orxNULL);

  /* Gets body */
  poBody = (b2Body *)_pstBody;

  /* Wakes up */
  poBody->WakeUp();

  /* Sets impulse */
  vImpulse.Set(_pvImpulse->fX, _pvImpulse->fY);

  /* Sets point */
  vPoint.Set(sstPhysics.fDimensionRatio * _pvPoint->fX, sstPhysics.fDimensionRatio * _pvPoint->fY);

  /* Applies force */
  poBody->ApplyImpulse(vImpulse, vPoint);

  /* Done! */
  return eResult;
}

extern "C" orxHANDLE orxFASTCALL orxPhysics_Box2D_Raycast(const orxVECTOR *_pvStart, const orxVECTOR *_pvEnd, orxU16 _u16SelfFlags, orxU16 _u16CheckMask, orxVECTOR *_pvContact, orxVECTOR *_pvNormal)
{
  b2Segment     stSegment;
  b2Vec2        vNormal;
  b2Fixture    *poRaycastResult;
  b2FilterData  stFilterData;
  float         fLambda;
  orxHANDLE     hResult = orxHANDLE_UNDEFINED;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvStart != orxNULL);
  orxASSERT(_pvEnd != orxNULL);

  /* Gets segment */
  stSegment.p1.Set(sstPhysics.fDimensionRatio * _pvStart->fX, sstPhysics.fDimensionRatio * _pvStart->fY);
  stSegment.p2.Set(sstPhysics.fDimensionRatio * _pvEnd->fX, sstPhysics.fDimensionRatio * _pvEnd->fY);

  /* Inits filter data */
  stFilterData.categoryBits = _u16SelfFlags;
  stFilterData.maskBits     = _u16CheckMask;
  stFilterData.groupIndex   = 0;

  /* Inits raycast fixture */
  sstPhysics.poRaycastFixture->SetFilterData(stFilterData);

  /* Issues Raycast */
  poRaycastResult = sstPhysics.poWorld->RaycastOne(stSegment, &fLambda, &vNormal, false, sstPhysics.poRaycastFixture);

  /* Found? */
  if(poRaycastResult != orxNULL)
  {
    /* Updates result */
    hResult = (orxHANDLE)poRaycastResult->GetBody()->GetUserData();

    /* Asked for contact? */
    if(_pvContact != orxNULL)
    {
      /* Updates it */
      _pvContact->fX = ((orxFLOAT_1 - orx2F(fLambda)) * _pvStart->fX) + (orx2F(fLambda) * _pvEnd->fX);
      _pvContact->fY = ((orxFLOAT_1 - orx2F(fLambda)) * _pvStart->fY) + (orx2F(fLambda) * _pvEnd->fY);
      _pvContact->fZ = _pvStart->fZ;
    }

    /* Asked for normal? */
    if(_pvNormal != orxNULL)
    {
      /* Updates it */
      orxVector_Set(_pvNormal, vNormal.x, vNormal.y, orxFLOAT_0);
    }
  }

  /* Done! */
  return hResult;
}

extern "C" void orxFASTCALL orxPhysics_Box2D_EnableSimulation(orxBOOL _bEnable)
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

extern "C" orxSTATUS orxFASTCALL orxPhysics_Box2D_SetGravity(const orxVECTOR *_pvGravity)
{
  b2Vec2    vGravity;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvGravity != orxNULL);

  /* Sets gravity vector */
  vGravity.Set(sstPhysics.fDimensionRatio * _pvGravity->fX, sstPhysics.fDimensionRatio * _pvGravity->fY);

  /* Updates gravity */
  sstPhysics.poWorld->SetGravity(vGravity);

  /* Done! */
  return eResult;
}

extern "C" orxVECTOR *orxFASTCALL orxPhysics_Box2D_GetGravity(orxVECTOR *_pvGravity)
{
  b2Vec2      vGravity;
  orxVECTOR  *pvResult = _pvGravity;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvGravity != orxNULL);

  /* Gets gravity vector */
  vGravity = sstPhysics.poWorld->GetGravity();
  orxVector_Set(_pvGravity, sstPhysics.fRecDimensionRatio * vGravity.x, sstPhysics.fRecDimensionRatio * vGravity.y, orxFLOAT_0);

  /* Done! */
  return pvResult;
}

extern "C" orxSTATUS orxFASTCALL orxPhysics_Box2D_Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Was not already initialized? */
  if(!(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY))
  {
    orxBOOL   bAllowSleep;
    orxFLOAT  fRatio;
    orxVECTOR vGravity, vLower, vUpper;
    b2AABB    stWorldAABB;
    b2Vec2    vWorldGravity;

    /* Cleans static controller */
    orxMemory_Zero(&sstPhysics, sizeof(orxPHYSICS_STATIC));

    /* Gets gravity & allow sleep from config */
    orxConfig_PushSection(orxPHYSICS_KZ_CONFIG_SECTION);
    if(orxConfig_GetVector(orxPHYSICS_KZ_CONFIG_GRAVITY, &vGravity) == orxNULL)
    {
        orxVector_Copy(&vGravity, &orxVECTOR_0);
    }
    bAllowSleep = (orxConfig_HasValue(orxPHYSICS_KZ_CONFIG_ALLOW_SLEEP) != orxFALSE) ? orxConfig_GetBool(orxPHYSICS_KZ_CONFIG_ALLOW_SLEEP) : orxTRUE;

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
      sstPhysics.fDimensionRatio = orxPhysics::sfDefaultDimensionRatio;
    }

    /* Gets world corners from config */
    orxConfig_GetVector(orxPHYSICS_KZ_CONFIG_WORLD_LOWER, &vLower);
    orxConfig_GetVector(orxPHYSICS_KZ_CONFIG_WORLD_UPPER, &vUpper);

    /* Inits world AABB */
    stWorldAABB.lowerBound.Set(sstPhysics.fDimensionRatio * vLower.fX, sstPhysics.fDimensionRatio * vLower.fY);
    stWorldAABB.upperBound.Set(sstPhysics.fDimensionRatio * vUpper.fX, sstPhysics.fDimensionRatio * vUpper.fY);

    /* Inits world gravity */
    vWorldGravity.Set(sstPhysics.fDimensionRatio * vGravity.fX, sstPhysics.fDimensionRatio * vGravity.fY);

    /* Creates world */
    sstPhysics.poWorld = new b2World(stWorldAABB, vWorldGravity, bAllowSleep ? true : false);

    /* Success? */
    if(sstPhysics.poWorld != orxNULL)
    {
      orxCLOCK *pstClock;
      orxU32    u32IterationsPerStep;

      /* Creates listeners */
      sstPhysics.poContactListener  = new orxPhysicsContactListener();
      sstPhysics.poBoundaryListener = new orxPhysicsBoundaryListener();

      /* Registers them */
      sstPhysics.poWorld->SetContactListener(sstPhysics.poContactListener);
      sstPhysics.poWorld->SetBoundaryListener(sstPhysics.poBoundaryListener);

      /* Stores inverse dimension ratio */
      sstPhysics.fRecDimensionRatio = orxFLOAT_1 / sstPhysics.fDimensionRatio;

      /* Gets iteration per step number from config */
      u32IterationsPerStep = orxConfig_GetU32(orxPHYSICS_KZ_CONFIG_ITERATIONS);

      /* Valid? */
      if(u32IterationsPerStep > 0)
      {
        /* Stores it */
        sstPhysics.u32Iterations = u32IterationsPerStep;
      }
      else
      {
        /* Uses default value */
        sstPhysics.u32Iterations = orxPhysics::su32DefaultIterations;
      }

      /* Gets core clock */
      pstClock = orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE);

      /* Resyncs clocks */
      orxClock_ResyncAll();

      /* Valid? */
      if(pstClock != orxNULL)
      {
        /* Registers rendering function */
        eResult = orxClock_Register(pstClock, orxPhysics_Update, (void *)sstPhysics.u32Iterations, orxMODULE_ID_PHYSICS, orxCLOCK_PRIORITY_LOWER);

        /* Valid? */
        if(eResult != orxSTATUS_FAILURE)
        {
          b2CircleDef stCircleDef;

          /* Updates Fixture type */
          stCircleDef.type = b2_circleShape;

          /* Stores its coordinates */
          stCircleDef.localPosition.Set(0.0f, 0.0f);
          stCircleDef.radius    = 0.0f;
          stCircleDef.isSensor  = true;

          /* Creates raycast fixture */
          sstPhysics.poRaycastFixture = sstPhysics.poWorld->GetGroundBody()->CreateFixture(&stCircleDef);

          /* Creates event bank */
          sstPhysics.pstEventBank = orxBank_Create(orxPhysics::su32MessageBankSize, sizeof(orxPHYSICS_EVENT_STORAGE), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

          /* Updates status */
          sstPhysics.u32Flags |= orxPHYSICS_KU32_STATIC_FLAG_READY | orxPHYSICS_KU32_STATIC_FLAG_ENABLED;
        }
        else
        {
          /* Deletes listeners */
          delete sstPhysics.poContactListener;
          delete sstPhysics.poBoundaryListener;

          /* Deletes world */
          delete sstPhysics.poWorld;

          /* Updates result */
          eResult = orxSTATUS_FAILURE;
        }
      }
      else
      {
        /* Deletes listeners */
        delete sstPhysics.poContactListener;
        delete sstPhysics.poBoundaryListener;

        /* Deletes world */
        delete sstPhysics.poWorld;

        /* Updates result */
        eResult = orxSTATUS_FAILURE;
      }
    }
    else
    {
      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }

    /* Pops config section */
    orxConfig_PopSection();
  }

  /* Done! */
  return eResult;
}

extern "C" void orxFASTCALL orxPhysics_Box2D_Exit()
{
  /* Was initialized? */
  if(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY)
  {
    /* Deletes the listeners */
    delete sstPhysics.poContactListener;
    delete sstPhysics.poBoundaryListener;

    /* Deletes world */
    delete sstPhysics.poWorld;

    /* Cleans static controller */
    orxMemory_Zero(&sstPhysics, sizeof(orxPHYSICS_STATIC));
  }

  return;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(PHYSICS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_Init, PHYSICS, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_Exit, PHYSICS, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_SetGravity, PHYSICS, SET_GRAVITY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_GetGravity, PHYSICS, GET_GRAVITY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_CreateBody, PHYSICS, CREATE_BODY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_DeleteBody, PHYSICS, DELETE_BODY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_CreateBodyPart, PHYSICS, CREATE_BODY_PART);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_DeleteBodyPart, PHYSICS, DELETE_BODY_PART);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_SetPosition, PHYSICS, SET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_SetRotation, PHYSICS, SET_ROTATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_SetSpeed, PHYSICS, SET_SPEED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_SetAngularVelocity, PHYSICS, SET_ANGULAR_VELOCITY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_SetCustomGravity, PHYSICS, SET_CUSTOM_GRAVITY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_GetPosition, PHYSICS, GET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_GetRotation, PHYSICS, GET_ROTATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_GetSpeed, PHYSICS, GET_SPEED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_GetAngularVelocity, PHYSICS, GET_ANGULAR_VELOCITY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_GetCustomGravity, PHYSICS, GET_CUSTOM_GRAVITY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_GetMassCenter, PHYSICS, GET_MASS_CENTER)
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_ApplyTorque, PHYSICS, APPLY_TORQUE)
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_ApplyForce, PHYSICS, APPLY_FORCE)
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_ApplyImpulse, PHYSICS, APPLY_IMPULSE)
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_Raycast, PHYSICS, RAYCAST)
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_EnableSimulation, PHYSICS, ENABLE_SIMULATION)
orxPLUGIN_USER_CORE_FUNCTION_END();


#ifdef __orxMSVC__

  #pragma warning(default : 4311 4312)

#endif /* __orxMSVC__ */
