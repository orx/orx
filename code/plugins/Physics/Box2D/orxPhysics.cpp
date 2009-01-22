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
 * @todo
 */


#include "orx.h"

#include <Box2D/Box2D.h>


#ifdef __orxMSVC__

  #pragma warning(disable : 4311 4312)

#endif /* __orxMSVC__ */


/** Module flags
 */
#define orxPHYSICS_KU32_STATIC_FLAG_NONE        0x00000000 /**< No flags */

#define orxPHYSICS_KU32_STATIC_FLAG_READY       0x00000001 /**< Ready flag */

#define orxPHYSICS_KU32_STATIC_MASK_ALL         0xFFFFFFFF /**< All mask */


orxSTATIC orxCONST orxU32   su32DefaultIterations   = 10;
orxSTATIC orxCONST orxFLOAT sfDefaultDimensionRatio = orx2F(0.01f);
orxSTATIC orxCONST orxU32   su32MessageBankSize     = 64;


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Event storage
 */
typedef struct __orxPHYSICS_EVENT_STORAGE_t
{
  orxPHYSICS_CONTACT_EVENT_PAYLOAD  stPayload;        /**< Event payload */
  orxPHYSICS_EVENT                  eID;              /**< Event ID */
  b2Body                           *poSource;         /**< Event source */
  b2Body                           *poDestination;    /**< Event destination */
  orxU32                            u32SrcShapeIndex; /**< Source shape index */
  orxU32                            u32DstShapeIndex; /**< Destination shape index */
  orxU32                            u32Key;           /**< Event key */

} orxPHYSICS_EVENT_STORAGE;

/** Contact listener
 */
class orxPhysicsContactListener : public b2ContactListener
{
public:
  void Remove(const b2ContactPoint *_poPoint);
  void Add(const b2ContactPoint *_poPoint);
  void Persist(const b2ContactPoint *_poPoint);
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
  orxCLOCK                   *pstClock;           /**< Simulation clock */
  orxBANK                    *pstEventBank;       /**< Event bank */
  b2World                    *poWorld;            /**< World */
  orxPhysicsContactListener  *poContactListener;  /**< Contact listener */
  orxPhysicsBoundaryListener *poBoundaryListener; /**< Boundary listener */

} orxPHYSICS_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
orxSTATIC orxPHYSICS_STATIC sstPhysics;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

orxU32 orxPhysics_Box2D_GetShapeIndex(orxCONST b2Body *_poBody, orxCONST b2Shape *_poShape)
{
  orxBODY  *pstBody;
  orxU32    u32Result = orxU32_UNDEFINED;

  /* Checks */
  orxASSERT(_poBody != orxNULL);
  orxASSERT(_poShape != orxNULL);

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
        if((orxHANDLE)pstBodyPart == (orxHANDLE)_poShape)
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

void orxPhysics_Box2D_SendContactEvent(const b2ContactPoint *_poPoint, orxPHYSICS_EVENT _eEventID)
{
  orxPHYSICS_EVENT_STORAGE *pstEventStorage;
  orxBOOL                   bSendEvent = orxTRUE;

  /* For all pending events */
  for(pstEventStorage = (orxPHYSICS_EVENT_STORAGE *)orxBank_GetNext(sstPhysics.pstEventBank, orxNULL);
      pstEventStorage != orxNULL;
      pstEventStorage = (orxPHYSICS_EVENT_STORAGE *)orxBank_GetNext(sstPhysics.pstEventBank, pstEventStorage))
  {
    /* Same pair? */
    if((pstEventStorage->poSource == _poPoint->shape1->GetBody()) && (pstEventStorage->poDestination == _poPoint->shape2->GetBody()))
    {
      /* Depending on old event */
      switch(pstEventStorage->eID)
      {
        case orxPHYSICS_EVENT_CONTACT_ADD:
        {
          /* Is new one a persist? */
          if(_eEventID == orxPHYSICS_EVENT_CONTACT_PERSIST)
          {
            /* Don't send event */
            bSendEvent = orxFALSE;
          }
          else
          {
            /* Removes it */
            orxBank_Free(sstPhysics.pstEventBank, pstEventStorage);

            /* Removing it? */
            if(_eEventID == orxPHYSICS_EVENT_CONTACT_REMOVE)
            {
              /* Don't send event */
              bSendEvent = orxFALSE;
            }
          }

          break;
        }

        case orxPHYSICS_EVENT_CONTACT_PERSIST:
        {
          /* Is new one a add? */
          if(_eEventID == orxPHYSICS_EVENT_CONTACT_ADD)
          {
            /* Don't send event */
            bSendEvent = orxFALSE;
          }
          else
          {
            /* Removes it */
            orxBank_Free(sstPhysics.pstEventBank, pstEventStorage);
          }

          break;
        }

        case orxPHYSICS_EVENT_CONTACT_REMOVE:
        {
          /* Removes it */
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
    orxPHYSICS_EVENT_STORAGE *pstEventStorage;

    /* Adds a contact event */
    pstEventStorage = (orxPHYSICS_EVENT_STORAGE *)orxBank_Allocate(sstPhysics.pstEventBank);

    /* Valid? */
    if(pstEventStorage != orxNULL)
    {
      /* Inits it */
      pstEventStorage->eID                                = _eEventID;
      pstEventStorage->u32Key                             = _poPoint->id.key;
      pstEventStorage->poSource                           = _poPoint->shape1->GetBody();
      pstEventStorage->poDestination                      = _poPoint->shape2->GetBody();
      pstEventStorage->stPayload.fPenetration             = -sstPhysics.fRecDimensionRatio * _poPoint->separation;
      pstEventStorage->stPayload.u32SourcePartIndex       = orxPhysics_Box2D_GetShapeIndex(pstEventStorage->poSource, _poPoint->shape1);
      pstEventStorage->stPayload.u32DestinationPartIndex  = orxPhysics_Box2D_GetShapeIndex(pstEventStorage->poDestination, _poPoint->shape2);
      orxVector_Set(&(pstEventStorage->stPayload.vPosition), sstPhysics.fRecDimensionRatio * _poPoint->position.x, sstPhysics.fRecDimensionRatio * _poPoint->position.y, orxFLOAT_0);
      orxVector_Set(&(pstEventStorage->stPayload.vNormal), _poPoint->normal.x, _poPoint->normal.y, orxFLOAT_0);
    }
  }

  return;
}

void orxPhysicsContactListener::Add(const b2ContactPoint *_poPoint)
{
  /* Sends contact event */
  orxPhysics_Box2D_SendContactEvent(_poPoint, orxPHYSICS_EVENT_CONTACT_ADD);

  return;
}

void orxPhysicsContactListener::Remove(const b2ContactPoint *_poPoint)
{
  /* Sends contact event */
  orxPhysics_Box2D_SendContactEvent(_poPoint, orxPHYSICS_EVENT_CONTACT_REMOVE);

  return;
}

void orxPhysicsContactListener::Persist(const b2ContactPoint *_poPoint)
{
  /* Sends contact event */
  orxPhysics_Box2D_SendContactEvent(_poPoint, orxPHYSICS_EVENT_CONTACT_PERSIST);

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
    /* Inits it */
    pstEventStorage->eID      = orxPHYSICS_EVENT_OUT_OF_WORLD;
    pstEventStorage->poSource = _poBody;
  }

  return;
}


/** Update (callback to register on a clock)
 * @param[in]   _pstClockInfo   Clock info of the clock used upon registration
 * @param[in]   _pstContext     Context sent when registering callback to the clock
 */
orxVOID orxFASTCALL orxPhysics_Update(orxCONST orxCLOCK_INFO *_pstClockInfo, orxVOID *_pstContext)
{
  orxPHYSICS_EVENT_STORAGE *pstEventStorage;
  orxEVENT                  stEvent;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstClockInfo != orxNULL);

  /* Updates world simulation */
  sstPhysics.poWorld->Step(_pstClockInfo->fDT, (orxU32)_pstContext);

  /* Clears and inits event */
  orxMemory_Zero(&stEvent, sizeof(orxEVENT));
  stEvent.eType = orxEVENT_TYPE_PHYSICS;

  /* For all stored events */
  for(pstEventStorage = (orxPHYSICS_EVENT_STORAGE *)orxBank_GetNext(sstPhysics.pstEventBank, orxNULL);
      pstEventStorage != orxNULL;
      pstEventStorage = (orxPHYSICS_EVENT_STORAGE *)orxBank_GetNext(sstPhysics.pstEventBank, pstEventStorage))
  {
    /* Depending on type */
    switch(pstEventStorage->eID)
    {
      case orxPHYSICS_EVENT_OUT_OF_WORLD:
      {
        /* Inits event */
        stEvent.eID         = orxPHYSICS_EVENT_OUT_OF_WORLD;
        stEvent.hRecipient  = stEvent.hSender = (orxHANDLE)orxBody_GetOwner(orxBODY(pstEventStorage->poSource->GetUserData()));
        stEvent.pstPayload  = orxNULL;

        /* Sends it */
        orxEvent_Send(&stEvent);

        break;
      }

      case orxPHYSICS_EVENT_CONTACT_ADD:
      case orxPHYSICS_EVENT_CONTACT_PERSIST:
      case orxPHYSICS_EVENT_CONTACT_REMOVE:
      {
        /* Inits event */
        stEvent.eID         = pstEventStorage->eID;
        stEvent.hSender     = (orxHANDLE)orxBody_GetOwner(orxBODY(pstEventStorage->poSource->GetUserData()));
        stEvent.hRecipient  = (orxHANDLE)orxBody_GetOwner(orxBODY(pstEventStorage->poDestination->GetUserData()));
        stEvent.pstPayload  = &(pstEventStorage->stPayload);

        /* Sends it */
        orxEvent_Send(&stEvent);

        break;
      }

      default:
      {
        break;
      }
    }
  }

  /* Clears stored events */
  orxBank_Clear(sstPhysics.pstEventBank);

  return;
}

extern "C" orxPHYSICS_BODY *orxPhysics_Box2D_CreateBody(orxCONST orxHANDLE _hUserData, orxCONST orxBODY_DEF *_pstBodyDef)
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
    stBodyDef.userData        = _hUserData;
    stBodyDef.angle           = _pstBodyDef->fRotation;
    stBodyDef.linearDamping   = _pstBodyDef->fLinearDamping;
    stBodyDef.angularDamping  = _pstBodyDef->fAngularDamping;
    stBodyDef.isBullet        = orxFLAG_TEST(_pstBodyDef->u32Flags, orxBODY_DEF_KU32_FLAG_HIGH_SPEED);
    stBodyDef.fixedRotation   = orxFLAG_TEST(_pstBodyDef->u32Flags, orxBODY_DEF_KU32_FLAG_FIXED_ROTATION);
    stBodyDef.position.Set(sstPhysics.fDimensionRatio * _pstBodyDef->vPosition.fX, sstPhysics.fDimensionRatio * _pstBodyDef->vPosition.fY);

    /* Is dynamic? */
    if(orxFLAG_TEST(_pstBodyDef->u32Flags, orxBODY_DEF_KU32_FLAG_DYNAMIC))
    {
      /* Stores mass properties */
      stBodyDef.massData.I      = (_pstBodyDef->fInertia != 0.0f) ? _pstBodyDef->fInertia : 1.0f;
      stBodyDef.massData.mass   = _pstBodyDef->fMass;

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

extern "C" orxVOID orxPhysics_Box2D_DeleteBody(orxPHYSICS_BODY *_pstBody)
{
  orxPHYSICS_EVENT_STORAGE *pstEventStorage, *pstPreviousEventStorage;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);

  /* For all stored events */
  for(pstEventStorage = (orxPHYSICS_EVENT_STORAGE *)orxBank_GetNext(sstPhysics.pstEventBank, orxNULL), pstPreviousEventStorage = (orxPHYSICS_EVENT_STORAGE *)orxNULL;
      pstEventStorage != orxNULL;
      pstPreviousEventStorage = pstEventStorage, pstEventStorage = (orxPHYSICS_EVENT_STORAGE *)orxBank_GetNext(sstPhysics.pstEventBank, pstEventStorage))
  {
    /* Is part of the event? */
    if(((b2Body *)_pstBody == pstEventStorage->poDestination) || ((b2Body *)_pstBody == pstEventStorage->poSource))
    {
      /* Removes event */
      orxBank_Free(sstPhysics.pstEventBank, pstEventStorage);

      /* Reverts to previous */
      pstEventStorage = pstPreviousEventStorage;
    }
  }

  /* Deletes it */
  sstPhysics.poWorld->DestroyBody((b2Body *)_pstBody);

  return;
}

extern "C" orxPHYSICS_BODY_PART *orxPhysics_Box2D_CreateBodyPart(orxPHYSICS_BODY *_pstBody, orxCONST orxBODY_PART_DEF *_pstBodyPartDef)
{
  b2Body       *poBody;
  b2Shape      *poResult = 0;
  b2ShapeDef   *pstShapeDef;
  b2CircleDef   stCircleDef;
  b2PolygonDef  stPolygonDef;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);
  orxASSERT(_pstBodyPartDef != orxNULL);
  orxASSERT(orxFLAG_TEST(_pstBodyPartDef->u32Flags, orxBODY_PART_DEF_KU32_FLAG_BOX | orxBODY_PART_DEF_KU32_FLAG_SPHERE));

  /* Gets body */
  poBody = (b2Body *)_pstBody;

  /* Circle? */
  if(orxFLAG_TEST(_pstBodyPartDef->u32Flags, orxBODY_PART_DEF_KU32_FLAG_SPHERE))
  {
    /* Gets def reference */
    pstShapeDef = &stCircleDef;

    /* Updates shape type */
    stCircleDef.type = e_circleShape;

    /* Stores its coordinates */
    stCircleDef.localPosition.Set(sstPhysics.fDimensionRatio * _pstBodyPartDef->stSphere.vCenter.fX * _pstBodyPartDef->vScale.fX, sstPhysics.fDimensionRatio * _pstBodyPartDef->stSphere.vCenter.fY * _pstBodyPartDef->vScale.fY);
    stCircleDef.radius = sstPhysics.fDimensionRatio * _pstBodyPartDef->stSphere.fRadius * orx2F(0.5f) * (_pstBodyPartDef->vScale.fX + _pstBodyPartDef->vScale.fY);
  }
  /* Polygon */
  else
  {
    /* Gets def reference */
    pstShapeDef = &stPolygonDef;

    /* Updates shape type */
    stPolygonDef.type = e_polygonShape;

    /* Stores its coordinates */
    stPolygonDef.vertexCount = 4;
    stPolygonDef.vertices[0].Set(sstPhysics.fDimensionRatio * _pstBodyPartDef->stAABox.stBox.vBR.fX * _pstBodyPartDef->vScale.fX, sstPhysics.fDimensionRatio * _pstBodyPartDef->stAABox.stBox.vTL.fY * _pstBodyPartDef->vScale.fY);
    stPolygonDef.vertices[1].Set(sstPhysics.fDimensionRatio * _pstBodyPartDef->stAABox.stBox.vBR.fX * _pstBodyPartDef->vScale.fX, sstPhysics.fDimensionRatio * _pstBodyPartDef->stAABox.stBox.vBR.fY * _pstBodyPartDef->vScale.fY);
    stPolygonDef.vertices[2].Set(sstPhysics.fDimensionRatio * _pstBodyPartDef->stAABox.stBox.vTL.fX * _pstBodyPartDef->vScale.fX, sstPhysics.fDimensionRatio * _pstBodyPartDef->stAABox.stBox.vBR.fY * _pstBodyPartDef->vScale.fY);
    stPolygonDef.vertices[3].Set(sstPhysics.fDimensionRatio * _pstBodyPartDef->stAABox.stBox.vTL.fX * _pstBodyPartDef->vScale.fX, sstPhysics.fDimensionRatio * _pstBodyPartDef->stAABox.stBox.vTL.fY * _pstBodyPartDef->vScale.fY);
  }

  /* Inits shape definition */
  pstShapeDef->friction             = _pstBodyPartDef->fFriction;
  pstShapeDef->restitution          = _pstBodyPartDef->fRestitution;
  pstShapeDef->density              = (poBody->GetInertia() != 0.0f) ? _pstBodyPartDef->fDensity : 0.0f;
  pstShapeDef->filter.categoryBits  = _pstBodyPartDef->u16SelfFlags;
  pstShapeDef->filter.maskBits      = _pstBodyPartDef->u16CheckMask;
  pstShapeDef->filter.groupIndex    = (orxU16)(sstPhysics.fDimensionRatio * _pstBodyPartDef->stAABox.stBox.vTL.fZ);
  pstShapeDef->isSensor             = orxFLAG_TEST(_pstBodyPartDef->u32Flags, orxBODY_PART_DEF_KU32_FLAG_SOLID) == orxFALSE;

  /* Creates it */
  poResult = poBody->CreateShape(pstShapeDef);

  /* Valid? */
  if(poResult != 0)
  {
    /* Computes body's mass */
    poBody->SetMassFromShapes();
  }

  /* Done! */
  return (orxPHYSICS_BODY_PART *)poResult;
}

extern "C" orxVOID orxPhysics_Box2D_DeleteBodyPart(orxPHYSICS_BODY_PART *_pstBodyPart)
{
  b2Shape  *poShape;
  b2Body   *poBody;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyPart != orxNULL);

  /* Gets shape */
  poShape = (b2Shape *)_pstBodyPart;

  /* Gets its body */
  poBody = poShape->GetBody();

  /* Deletes its part */
  poBody->DestroyShape(poShape);

  return;
}

extern "C" orxSTATUS orxPhysics_Box2D_SetPosition(orxPHYSICS_BODY *_pstBody, orxCONST orxVECTOR *_pvPosition)
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

  /* Sets position vector */
  vPosition.Set(sstPhysics.fDimensionRatio * _pvPosition->fX, sstPhysics.fDimensionRatio * _pvPosition->fY);

  /* Updates its position */
  eResult = (poBody->SetXForm(vPosition, poBody->GetAngle()) != false) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxPhysics_Box2D_SetRotation(orxPHYSICS_BODY *_pstBody, orxFLOAT _fRotation)
{
  b2Body   *poBody;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);

  /* Gets body */
  poBody = (b2Body *)_pstBody;

  /* Updates its rotation */
  eResult = (poBody->SetXForm(poBody->GetPosition(), _fRotation) != false) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxPhysics_Box2D_SetSpeed(orxPHYSICS_BODY *_pstBody, orxCONST orxVECTOR *_pvSpeed)
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

  /* Updates its speed */
  poBody->SetLinearVelocity(vSpeed);

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxPhysics_Box2D_SetAngularVelocity(orxPHYSICS_BODY *_pstBody, orxFLOAT _fVelocity)
{
  b2Body   *poBody;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);

  /* Gets body */
  poBody = (b2Body *)_pstBody;

  /* Updates its angular velocity */
  poBody->SetAngularVelocity(_fVelocity);

  /* Done! */
  return eResult;
}

extern "C" orxVECTOR *orxPhysics_Box2D_GetPosition(orxPHYSICS_BODY *_pstBody, orxVECTOR *_pvPosition)
{
  b2Body   *poBody;
  b2Vec2    vPosition;
  orxVECTOR *pvResult;

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

extern "C" orxFLOAT orxPhysics_Box2D_GetRotation(orxPHYSICS_BODY *_pstBody)
{
  b2Body   *poBody;
  orxFLOAT fResult;

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

extern "C" orxVECTOR *orxPhysics_Box2D_GetSpeed(orxPHYSICS_BODY *_pstBody, orxVECTOR *_pvSpeed)
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

  /* Done! */
  return pvResult;
}

extern "C" orxFLOAT orxPhysics_Box2D_GetAngularVelocity(orxPHYSICS_BODY *_pstBody)
{
  b2Body   *poBody;
  orxFLOAT fResult;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);

  /* Gets body */
  poBody = (b2Body *)_pstBody;

  /* Gets its rotation */
  fResult = poBody->GetAngularVelocity();

  /* Done! */
  return fResult;
}

extern "C" orxVECTOR *orxPhysics_Box2D_GetMassCenter(orxPHYSICS_BODY *_pstBody, orxVECTOR *_pvMassCenter)
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

extern "C" orxSTATUS orxPhysics_Box2D_ApplyTorque(orxPHYSICS_BODY *_pstBody, orxFLOAT _fTorque)
{
  b2Body   *poBody;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBody != orxNULL);

  /* Gets body */
  poBody = (b2Body *)_pstBody;

  /* Applies torque */
  poBody->ApplyTorque(_fTorque);

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxPhysics_Box2D_ApplyForce(orxPHYSICS_BODY *_pstBody, orxCONST orxVECTOR *_pvForce, orxCONST orxVECTOR *_pvPoint)
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

  /* Sets force */
  vForce.Set(_pvForce->fX, _pvForce->fY);

  /* Sets point */
  vPoint.Set(sstPhysics.fDimensionRatio * _pvPoint->fX, sstPhysics.fDimensionRatio * _pvPoint->fY);

  /* Applies force */
  poBody->ApplyForce(vForce, vPoint);

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxPhysics_Box2D_ApplyImpulse(orxPHYSICS_BODY *_pstBody, orxCONST orxVECTOR *_pvImpulse, orxCONST orxVECTOR *_pvPoint)
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

  /* Sets impulse */
  vImpulse.Set(_pvImpulse->fX, _pvImpulse->fY);

  /* Sets point */
  vPoint.Set(sstPhysics.fDimensionRatio * _pvPoint->fX, sstPhysics.fDimensionRatio * _pvPoint->fY);

  /* Applies force */
  poBody->ApplyImpulse(vImpulse, vPoint);

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxPhysics_Box2D_SetGravity(orxCONST orxVECTOR *_pvGravity)
{
  b2Vec2    vGravity;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvGravity != orxNULL);

  /* Sets gravity vector */
  vGravity.Set(_pvGravity->fX, _pvGravity->fY);

  /* Updates gravity */
  sstPhysics.poWorld->SetGravity(vGravity);

  /* Done! */
  return eResult;
}

extern "C" orxVECTOR *orxPhysics_Box2D_GetGravity(orxVECTOR *_pvGravity)
{
  b2Vec2    vGravity;
  orxVECTOR *pvResult = _pvGravity;

  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvGravity != orxNULL);

  /* Gets gravity vector */
  orxVector_Set(_pvGravity, sstPhysics.poWorld->m_gravity.x, sstPhysics.poWorld->m_gravity.y, orxFLOAT_0);

  /* Done! */
  return pvResult;
}

extern "C" orxSTATUS orxPhysics_Box2D_Init()
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
    orxConfig_SelectSection(orxPHYSICS_KZ_CONFIG_SECTION);
    orxConfig_GetVector(orxPHYSICS_KZ_CONFIG_GRAVITY, &vGravity);
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
      sstPhysics.fDimensionRatio = sfDefaultDimensionRatio;
    }

    /* Gets world corners from config */
    orxConfig_GetVector(orxPHYSICS_KZ_CONFIG_WORLD_LOWER, &vLower);
    orxConfig_GetVector(orxPHYSICS_KZ_CONFIG_WORLD_UPPER, &vUpper);

    /* Inits world AABB */
    stWorldAABB.lowerBound.Set(sstPhysics.fDimensionRatio * vLower.fX, sstPhysics.fDimensionRatio * vLower.fY);
    stWorldAABB.upperBound.Set(sstPhysics.fDimensionRatio * vUpper.fX, sstPhysics.fDimensionRatio * vUpper.fY);

    /* Inits world gravity */
    vWorldGravity.Set(vGravity.fX, vGravity.fY);

    /* Creates world */
    sstPhysics.poWorld = new b2World(stWorldAABB, vWorldGravity, bAllowSleep ? true : false);

    /* Success? */
    if(sstPhysics.poWorld != orxNULL)
    {
      orxU32 u32IterationsPerStep;

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
        sstPhysics.u32Iterations = su32DefaultIterations;
      }

      /* Creates physics clock */
      sstPhysics.pstClock = orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE);

      /* Resyncs clocks */
      orxClock_ResyncAll();

      /* Valid? */
      if(sstPhysics.pstClock != orxNULL)
      {
        /* Registers rendering function */
        eResult = orxClock_Register(sstPhysics.pstClock, orxPhysics_Update, (orxVOID *)sstPhysics.u32Iterations, orxMODULE_ID_PHYSICS, orxCLOCK_PRIORITY_LOWER);

        /* Valid? */
        if(eResult != orxSTATUS_FAILURE)
        {
          /* Creates contact bank */
          sstPhysics.pstEventBank = orxBank_Create(su32MessageBankSize, sizeof(orxPHYSICS_EVENT_STORAGE), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

          /* Updates status */
          sstPhysics.u32Flags |= orxPHYSICS_KU32_STATIC_FLAG_READY;
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
  }

  /* Done! */
  return eResult;
}

extern "C" orxVOID orxPhysics_Box2D_Exit()
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
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_GetPosition, PHYSICS, GET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_GetRotation, PHYSICS, GET_ROTATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_GetSpeed, PHYSICS, GET_SPEED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_GetAngularVelocity, PHYSICS, GET_ANGULAR_VELOCITY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_GetMassCenter, PHYSICS, GET_MASS_CENTER)
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_ApplyTorque, PHYSICS, APPLY_TORQUE)
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_ApplyForce, PHYSICS, APPLY_FORCE)
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_ApplyImpulse, PHYSICS, APPLY_IMPULSE)
orxPLUGIN_USER_CORE_FUNCTION_END();


#ifdef __orxMSVC__

  #pragma warning(default : 4311 4312)

#endif /* __orxMSVC__ */
