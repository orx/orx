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
 * @file orxBody.c
 * @date 10/03/2008
 * @author iarwain@orx-project.org
 *
 */


#include "physics/orxBody.h"
#include "physics/orxPhysics.h"
#include "core/orxConfig.h"
#include "object/orxObject.h"
#include "object/orxFrame.h"
#include "utils/orxString.h"

#include "debug/orxDebug.h"
#include "memory/orxMemory.h"


/** Body flags
 */
#define orxBODY_KU32_FLAG_NONE                0x00000000  /**< No flags */

#define orxBODY_KU32_FLAG_HAS_DATA            0x00000001  /**< Has data flag */
#define orxBODY_KU32_FLAG_USE_TEMPLATE        0x00000002  /**< Use body template flag */
#define orxBODY_KU32_FLAG_USE_PART_TEMPLATE   0x00000004  /**< Use body part template flag */

#define orxBODY_KU32_MASK_ALL                 0xFFFFFFFF  /**< User all ID mask */


/** Module flags
 */
#define orxBODY_KU32_STATIC_FLAG_NONE         0x00000000  /**< No flags */

#define orxBODY_KU32_STATIC_FLAG_READY        0x10000000  /**< Ready flag */

#define orxBODY_KU32_MASK_ALL                 0xFFFFFFFF  /**< All mask */


/** Misc defines
 */
#define orxBODY_KZ_CONFIG_INERTIA             "Inertia"
#define orxBODY_KZ_CONFIG_MASS                "Mass"
#define orxBODY_KZ_CONFIG_LINEAR_DAMPING      "LinearDamping"
#define orxBODY_KZ_CONFIG_ANGULAR_DAMPING     "AngularDamping"
#define orxBODY_KZ_CONFIG_CUSTOM_GRAVITY      "CustomGravity"
#define orxBODY_KZ_CONFIG_FIXED_ROTATION      "FixedRotation"
#define orxBODY_KZ_CONFIG_ALLOW_GROUND_SLIDING "AllowGroundSliding"
#define orxBODY_KZ_CONFIG_HIGH_SPEED          "HighSpeed"
#define orxBODY_KZ_CONFIG_DYNAMIC             "Dynamic"
#define orxBODY_KZ_CONFIG_PART_LIST           "PartList"
#define orxBODY_KZ_CONFIG_FRICTION            "Friction"
#define orxBODY_KZ_CONFIG_RESTITUTION         "Restitution"
#define orxBODY_KZ_CONFIG_DENSITY             "Density"
#define orxBODY_KZ_CONFIG_SELF_FLAGS          "SelfFlags"
#define orxBODY_KZ_CONFIG_CHECK_MASK          "CheckMask"
#define orxBODY_KZ_CONFIG_TYPE                "Type"
#define orxBODY_KZ_CONFIG_SOLID               "Solid"
#define orxBODY_KZ_CONFIG_TOP_LEFT            "TopLeft"
#define orxBODY_KZ_CONFIG_BOTTOM_RIGHT        "BottomRight"
#define orxBODY_KZ_CONFIG_CENTER              "Center"
#define orxBODY_KZ_CONFIG_RADIUS              "Radius"
#define orxBODY_KZ_CONFIG_VERTEX_LIST         "VertexList"

#define orxBODY_KZ_FULL                       "full"
#define orxBODY_KZ_TYPE_SPHERE                "sphere"
#define orxBODY_KZ_TYPE_BOX                   "box"
#define orxBODY_KZ_TYPE_MESH                  "mesh"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Body part structure
 */
struct __orxBODY_PART_t
{
  orxPHYSICS_BODY_PART *pstData;                                      /**< Data structure : 4 */
  orxSTRING             zReference;                                   /**< Part reference name : 8 */

  orxPAD(8)
};

/** Body structure
 */
struct __orxBODY_t
{
  orxSTRUCTURE            stStructure;                                /**< Public structure, first structure member : 16 */
  orxVECTOR               vScale;                                     /**< Scale : 28 */
  orxPHYSICS_BODY        *pstData;                                    /**< Physics body data : 32 */
  const orxSTRUCTURE     *pstOwner;                                   /**< Owner structure : 36 */
  orxU32                  u32DefFlags;                                /**< Definition flags : 40 */
  orxFLOAT                fTimeMultiplier;                            /**< Current time multiplier : 44 */
  orxVECTOR               vPreviousPosition;                          /**< Previous position : 56 */
  orxFLOAT                fPreviousRotation;                          /**< Previous rotation : 60 */
  orxBODY_PART            astPartList[orxBODY_KU32_PART_MAX_NUMBER];  /**< Body part structure list : 124 */
};

/** Static structure
 */
typedef struct __orxBODY_STATIC_t
{
  orxU32            u32Flags;                                         /**< Control flags */
  orxBODY_DEF       stBodyTemplate;                                   /**< Body template */
  orxBODY_PART_DEF  stBodyPartTemplate;                               /**< Body part template */

} orxBODY_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxBODY_STATIC sstBody;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Deletes all bodies
 */
static orxINLINE void orxBody_DeleteAll()
{
  register orxBODY *pstBody;

  /* Gets first body */
  pstBody = orxBODY(orxStructure_GetFirst(orxSTRUCTURE_ID_BODY));

  /* Non empty? */
  while(pstBody != orxNULL)
  {
    /* Deletes Body */
    orxBody_Delete(pstBody);

    /* Gets first Body */
    pstBody = orxBODY(orxStructure_GetFirst(orxSTRUCTURE_ID_BODY));
  }

  return;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Body module setup
 */
void orxFASTCALL orxBody_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_BODY, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_BODY, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_BODY, orxMODULE_ID_PHYSICS);
  orxModule_AddDependency(orxMODULE_ID_BODY, orxMODULE_ID_FRAME);
  orxModule_AddDependency(orxMODULE_ID_BODY, orxMODULE_ID_CONFIG);

  return;
}

/** Inits the Body module
 */
orxSTATUS orxFASTCALL orxBody_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if((sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY) == orxBODY_KU32_STATIC_FLAG_NONE)
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstBody, sizeof(orxBODY_STATIC));

    /* Registers structure type */
    eResult = orxSTRUCTURE_REGISTER(BODY, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxNULL);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "Tried to initialize body module when it is already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Initialized? */
  if(eResult == orxSTATUS_SUCCESS)
  {
    /* Inits Flags */
    sstBody.u32Flags = orxBODY_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "Failed to register storage link list.");
  }

  /* Done! */
  return eResult;
}

/** Exits from the Body module
 */
void orxFASTCALL orxBody_Exit()
{
  /* Initialized? */
  if(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY)
  {
    /* Deletes body list */
    orxBody_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_BODY);

    /* Updates flags */
    sstBody.u32Flags &= ~orxBODY_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "Tried to exit body module when it wasn't initialized.");
  }

  return;
}

/** Creates an empty body
 * @param[in]   _pstOwner                     Body's owner used for collision callbacks (usually an orxOBJECT)
 * @param[in]   _pstBodyDef                   Body definition
 * @return      Created orxBODY / orxNULL
 */
orxBODY *orxFASTCALL orxBody_Create(const orxSTRUCTURE *_pstOwner, const orxBODY_DEF *_pstBodyDef)
{
  orxBODY    *pstBody;
  orxOBJECT  *pstObject;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxASSERT(orxOBJECT(_pstOwner));
  orxASSERT((_pstBodyDef != orxNULL) || (orxFLAG_TEST(sstBody.u32Flags, orxBODY_KU32_FLAG_USE_TEMPLATE)));

  /* Gets owner object */
  pstObject = orxOBJECT(_pstOwner);

  /* Creates body */
  pstBody = orxBODY(orxStructure_Create(orxSTRUCTURE_ID_BODY));

  /* Valid? */
  if(pstBody != orxNULL)
  {
    orxBODY_DEF           stMergedDef;
    const orxBODY_DEF *pstSelectedDef;

    /* Inits flags */
    orxStructure_SetFlags(pstBody, orxBODY_KU32_FLAG_NONE, orxBODY_KU32_MASK_ALL);

    /* Uses template? */
    if(orxFLAG_TEST(sstBody.u32Flags, orxBODY_KU32_FLAG_USE_TEMPLATE))
    {
      /* Has specific definition? */
      if(_pstBodyDef != orxNULL)
      {
        /* Cleans merged def */
        orxMemory_Zero(&stMergedDef, sizeof(orxBODY_DEF));

        /* Merges template with specialized definition */
        orxObject_GetWorldPosition(pstObject, &(stMergedDef.vPosition));
        stMergedDef.fRotation           = orxObject_GetWorldRotation(pstObject);
        stMergedDef.fInertia            = (_pstBodyDef->fInertia > 0.0f) ? _pstBodyDef->fInertia : sstBody.stBodyTemplate.fInertia;
        stMergedDef.fMass               = (_pstBodyDef->fMass > 0.0f) ? _pstBodyDef->fMass : sstBody.stBodyTemplate.fMass;
        stMergedDef.fLinearDamping      = (_pstBodyDef->fLinearDamping > 0.0f) ? _pstBodyDef->fLinearDamping : sstBody.stBodyTemplate.fLinearDamping;
        stMergedDef.fAngularDamping     = (_pstBodyDef->fAngularDamping > 0.0f) ? _pstBodyDef->fAngularDamping : sstBody.stBodyTemplate.fAngularDamping;
        stMergedDef.u32Flags            = (_pstBodyDef->u32Flags != orxBODY_DEF_KU32_FLAG_NONE) ? _pstBodyDef->u32Flags : sstBody.stBodyTemplate.u32Flags;

        /* Selects it */
        pstSelectedDef = &stMergedDef;
      }
      else
      {
        /* Selects template */
        pstSelectedDef = &(sstBody.stBodyTemplate);
      }
    }
    else
    {
      /* Selects specialized definition */
      pstSelectedDef = _pstBodyDef;
    }

    /* Creates physics body */
    pstBody->pstData = orxPhysics_CreateBody(pstBody, pstSelectedDef);

    /* Valid? */
    if(pstBody->pstData != orxNULL)
    {
      /* Stores owner */
      pstBody->pstOwner = _pstOwner;

      /* Stores its scale */
      orxObject_GetScale(pstObject, &(pstBody->vScale));

      /* Stores its definition flags */
      pstBody->u32DefFlags = pstSelectedDef->u32Flags;

      /* Clears its time multiplier */
      pstBody->fTimeMultiplier = orxFLOAT_1;

      /* Updates flags */
      orxStructure_SetFlags(pstBody, orxBODY_KU32_FLAG_HAS_DATA, orxBODY_KU32_FLAG_NONE);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "Failed to create body.");

      /* Deletes allocated structure */
      orxStructure_Delete(pstBody);
      pstBody = orxNULL;
    }
  }

  /* Done! */
  return pstBody;
}

/** Creates a body from config
 * @param[in]   _pstOwner                     Body's owner used for collision callbacks (usually an orxOBJECT)
 * @param[in]   _zConfigID                    Body config ID
 * @return      Created orxGRAPHIC / orxNULL
 */
orxBODY *orxFASTCALL orxBody_CreateFromConfig(const orxSTRUCTURE *_pstOwner, const orxSTRING _zConfigID)
{
  orxBODY *pstResult;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstOwner);
  orxASSERT((_zConfigID != orxNULL) && (_zConfigID != orxSTRING_EMPTY));

  /* Pushes section */
  if((orxConfig_HasSection(_zConfigID) != orxFALSE)
  && (orxConfig_PushSection(_zConfigID) != orxSTATUS_FAILURE))
  {
    orxBODY_DEF stBodyDef;

    /* Clears body definition */
    orxMemory_Zero(&stBodyDef, sizeof(orxBODY_DEF));

    /* Inits it */
    stBodyDef.fInertia            = orxConfig_GetFloat(orxBODY_KZ_CONFIG_INERTIA);
    stBodyDef.fMass               = orxConfig_GetFloat(orxBODY_KZ_CONFIG_MASS);
    stBodyDef.fLinearDamping      = orxConfig_GetFloat(orxBODY_KZ_CONFIG_LINEAR_DAMPING);
    stBodyDef.fAngularDamping     = orxConfig_GetFloat(orxBODY_KZ_CONFIG_ANGULAR_DAMPING);
    stBodyDef.u32Flags            = orxBODY_DEF_KU32_FLAG_2D;
    if(orxConfig_GetBool(orxBODY_KZ_CONFIG_FIXED_ROTATION) != orxFALSE)
    {
      stBodyDef.u32Flags |= orxBODY_DEF_KU32_FLAG_FIXED_ROTATION;
    }
    if((orxConfig_HasValue(orxBODY_KZ_CONFIG_ALLOW_GROUND_SLIDING) == orxFALSE) || (orxConfig_GetBool(orxBODY_KZ_CONFIG_ALLOW_GROUND_SLIDING) != orxFALSE))
    {
      stBodyDef.u32Flags |= orxBODY_DEF_KU32_FLAG_CAN_SLIDE;
    }
    if(orxConfig_GetBool(orxBODY_KZ_CONFIG_HIGH_SPEED) != orxFALSE)
    {
      stBodyDef.u32Flags |= orxBODY_DEF_KU32_FLAG_HIGH_SPEED;
    }
    if(orxConfig_GetBool(orxBODY_KZ_CONFIG_DYNAMIC) != orxFALSE)
    {
      stBodyDef.u32Flags |= orxBODY_DEF_KU32_FLAG_DYNAMIC;
    }

    /* Creates body */
    pstResult = orxBody_Create(_pstOwner, &stBodyDef);

    /* Valid? */
    if(pstResult != orxNULL)
    {
      orxU32 i, u32SlotCounter;

      /* Gets number of declared slots */
      u32SlotCounter = orxConfig_GetListCounter(orxBODY_KZ_CONFIG_PART_LIST);

      /* Too many slots? */
      if(u32SlotCounter > orxBODY_KU32_PART_MAX_NUMBER)
      {
        /* For all exceeding slots */
        for(i = orxBODY_KU32_PART_MAX_NUMBER; i < u32SlotCounter; i++)
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "[%s]: Too many parts for this body, can't add part <%s>.", _zConfigID, orxConfig_GetListString(orxBODY_KZ_CONFIG_PART_LIST, i));
        }

        /* Updates slot counter */
        u32SlotCounter = orxBODY_KU32_PART_MAX_NUMBER;
      }

      /* For all parts */
      for(i = 0; i < u32SlotCounter; i++)
      {
        orxSTRING zPartName;

        /* Gets its name */
        zPartName = orxConfig_GetListString(orxBODY_KZ_CONFIG_PART_LIST, i);

        /* Valid? */
        if((zPartName != orxNULL) && (zPartName != orxSTRING_EMPTY))
        {
          /* Adds part */
          orxBody_AddPartFromConfig(pstResult, i, zPartName);
        }
        else
        {
          break;
        }
      }

      /* Has custom gravity? */
      if(orxConfig_HasValue(orxBODY_KZ_CONFIG_CUSTOM_GRAVITY) != orxFALSE)
      {
        orxVECTOR vGravity;

        /* Sets it */
        orxBody_SetCustomGravity(pstResult, orxConfig_GetVector(orxBODY_KZ_CONFIG_CUSTOM_GRAVITY, &vGravity));
      }
    }

    /* Pops previous section */
    orxConfig_PopSection();
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "Cannot find config section named (%s).", _zConfigID);

    /* Updates result */
    pstResult = orxNULL;
  }

  /* Done! */
  return pstResult;
}

/** Deletes a body
 * @param[in]   _pstBody     Body to delete
 */
orxSTATUS orxFASTCALL orxBody_Delete(orxBODY *_pstBody)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstBody);

  /* Not referenced? */
  if(orxStructure_GetRefCounter(_pstBody) == 0)
  {
    orxU32 i;

    /* For all data structure */
    for(i = 0; i < orxBODY_KU32_PART_MAX_NUMBER; i++)
    {
      /* Cleans it */
      orxBody_RemovePart(_pstBody, i);
    }

    /* Has data? */
    if(orxStructure_TestFlags(_pstBody, orxBODY_KU32_FLAG_HAS_DATA))
    {
      /* Deletes physics body */
      orxPhysics_DeleteBody(_pstBody->pstData);
    }

    /* Deletes structure */
    orxStructure_Delete(_pstBody);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "Cannot delete body object while it is still referenced.");

    /* Referenced by others */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets a body owner
 * @param[in]   _pstBody        Concerned body
 * @return      orxSTRUCTURE / orxNULL
 */
orxSTRUCTURE *orxFASTCALL orxBody_GetOwner(const orxBODY *_pstBody)
{
  orxSTRUCTURE *pstResult;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstBody);

  /* Updates result */
  pstResult = orxSTRUCTURE(_pstBody->pstOwner);

  /* Done! */
  return pstResult;
}

/** Adds a body part
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _u32Index       Part index (should be less than orxBODY_KU32_PART_MAX_NUMBER)
 * @param[in]   _pstPartDef     Body part definition
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxBody_AddPart(orxBODY *_pstBody, orxU32 _u32Index, const orxBODY_PART_DEF *_pstBodyPartDef)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstBody);
  orxASSERT((_pstBodyPartDef != orxNULL) || (orxFLAG_TEST(sstBody.u32Flags, orxBODY_KU32_FLAG_USE_PART_TEMPLATE)));
  orxASSERT(_u32Index < orxBODY_KU32_PART_MAX_NUMBER);

  /* Had previous part? */
  if(_pstBody->astPartList[_u32Index].pstData != orxNULL)
  {
    /* Removes it */
    eResult = orxBody_RemovePart(_pstBody, _u32Index);
  }

  /* Valid? */
  if(eResult != orxSTATUS_FAILURE)
  {
    orxBODY_PART_DEF            stMergedPartDef;
    const orxBODY_PART_DEF     *pstSelectedPartDef;
    orxPHYSICS_BODY_PART       *pstBodyPart;

    /* Uses part template? */
    if(orxFLAG_TEST(sstBody.u32Flags, orxBODY_KU32_FLAG_USE_PART_TEMPLATE))
    {
      /* Has specific part definition? */
      if(_pstBodyPartDef != orxNULL)
      {
        /* Cleans merged part def */
        orxMemory_Zero(&stMergedPartDef, sizeof(orxBODY_PART_DEF));

        /* Merges template with specialized definition */
        stMergedPartDef.fFriction     = (_pstBodyPartDef->fFriction > 0.0f) ? _pstBodyPartDef->fFriction : sstBody.stBodyPartTemplate.fFriction;
        stMergedPartDef.fRestitution  = (_pstBodyPartDef->fRestitution > 0.0f) ? _pstBodyPartDef->fRestitution : sstBody.stBodyPartTemplate.fRestitution;
        stMergedPartDef.fDensity      = (_pstBodyPartDef->fDensity > 0.0f) ? _pstBodyPartDef->fDensity : sstBody.stBodyPartTemplate.fDensity;
        stMergedPartDef.u16SelfFlags  = (_pstBodyPartDef->u16SelfFlags != 0) ? _pstBodyPartDef->u16SelfFlags : sstBody.stBodyPartTemplate.u16SelfFlags;
        stMergedPartDef.u16CheckMask  = (_pstBodyPartDef->u16CheckMask != 0) ? _pstBodyPartDef->u16CheckMask : sstBody.stBodyPartTemplate.u16CheckMask;
        stMergedPartDef.u32Flags      = (_pstBodyPartDef->u32Flags != orxBODY_PART_DEF_KU32_FLAG_NONE) ? _pstBodyPartDef->u32Flags : sstBody.stBodyPartTemplate.u32Flags;

        /* Has scale? */
        if(orxVector_IsNull(&(_pstBodyPartDef->vScale)) == orxFALSE)
        {
          orxVector_Copy(&(stMergedPartDef.vScale), &(_pstBodyPartDef->vScale));
        }
        else
        {
          orxVector_Copy(&(stMergedPartDef.vScale), &(sstBody.stBodyPartTemplate.vScale));
        }

        /* Sphere? */
        if(orxFLAG_TEST(_pstBodyPartDef->u32Flags, orxBODY_PART_DEF_KU32_FLAG_SPHERE))
        {
          orxVector_Copy(&(stMergedPartDef.stSphere.vCenter), (orxVector_IsNull(&(_pstBodyPartDef->stSphere.vCenter)) == orxFALSE) ? &(_pstBodyPartDef->stSphere.vCenter) : &(sstBody.stBodyPartTemplate.stSphere.vCenter));
          stMergedPartDef.stSphere.fRadius = (_pstBodyPartDef->stSphere.fRadius > 0.0f) ? _pstBodyPartDef->stSphere.fRadius : sstBody.stBodyPartTemplate.stSphere.fRadius;
        }
        /* Box? */
        else if(orxFLAG_TEST(_pstBodyPartDef->u32Flags, orxBODY_PART_DEF_KU32_FLAG_BOX))
        {
          orxVector_Copy(&(stMergedPartDef.stAABox.stBox.vTL), (orxVector_IsNull(&(_pstBodyPartDef->stAABox.stBox.vTL)) == orxFALSE) ? &(_pstBodyPartDef->stAABox.stBox.vTL) : &(sstBody.stBodyPartTemplate.stAABox.stBox.vTL));
          orxVector_Copy(&(stMergedPartDef.stAABox.stBox.vBR), (orxVector_IsNull(&(_pstBodyPartDef->stAABox.stBox.vBR)) == orxFALSE) ? &(_pstBodyPartDef->stAABox.stBox.vBR) : &(sstBody.stBodyPartTemplate.stAABox.stBox.vBR));
        }
        /* Mesh? */
        else if(orxFLAG_TEST(_pstBodyPartDef->u32Flags, orxBODY_PART_DEF_KU32_FLAG_MESH))
        {
          orxU32 i;

          /* For all vertices */
          for(i = 0; i < _pstBodyPartDef->stMesh.u32VertexCounter; i++)
          {
            /* Copies it */
            orxVector_Copy(&(stMergedPartDef.stMesh.avVertices[i]), (orxVector_IsNull(&(_pstBodyPartDef->stAABox.stBox.vTL)) == orxFALSE) ? &(_pstBodyPartDef->stAABox.stBox.vTL) : &(sstBody.stBodyPartTemplate.stAABox.stBox.vTL));
          }
        }

        /* Selects it */
        pstSelectedPartDef = &stMergedPartDef;
      }
      else
      {
        /* Selects template */
        pstSelectedPartDef = &(sstBody.stBodyPartTemplate);
      }
    }
    else
    {
      /* Selects specialized definition */
      pstSelectedPartDef = _pstBodyPartDef;
    }

    /* Creates part */
    pstBodyPart = orxPhysics_CreateBodyPart(_pstBody->pstData, pstSelectedPartDef);

    /* Valid? */
    if(pstBodyPart != orxNULL)
    {
      /* Stores it */
      _pstBody->astPartList[_u32Index].pstData = pstBodyPart;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "Failed to create body part.");

      /* Cleans reference */
      _pstBody->astPartList[_u32Index].pstData = orxNULL;

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }

  /* Done! */
  return eResult;
}

/** Adds a part to body from config
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _u32Index       Part index (should be less than orxBODY_KU32_PART_MAX_NUMBER)
 * @param[in]   _zConfigID      Body part config ID
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxBody_AddPartFromConfig(orxBODY *_pstBody, orxU32 _u32Index, const orxSTRING _zConfigID)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstBody);
  orxASSERT(_u32Index < orxBODY_KU32_PART_MAX_NUMBER);
  orxASSERT((_zConfigID != orxNULL) && (_zConfigID != orxSTRING_EMPTY));

  /* Pushes section */
  if((orxConfig_HasSection(_zConfigID) != orxFALSE)
  && (orxConfig_PushSection(_zConfigID) != orxSTATUS_FAILURE))
  {
    orxSTRING         zBodyPartType;
    orxBODY_PART_DEF  stBodyPartDef;

    /* Clears body part definition */
    orxMemory_Zero(&stBodyPartDef, sizeof(orxBODY_PART_DEF));

    /* Gets body part type */
    zBodyPartType = orxString_LowerCase(orxConfig_GetString(orxBODY_KZ_CONFIG_TYPE));

    /* Inits it */
    stBodyPartDef.fFriction     = orxConfig_GetFloat(orxBODY_KZ_CONFIG_FRICTION);
    stBodyPartDef.fRestitution  = orxConfig_GetFloat(orxBODY_KZ_CONFIG_RESTITUTION);
    stBodyPartDef.fDensity      = orxConfig_GetFloat(orxBODY_KZ_CONFIG_DENSITY);
    stBodyPartDef.u16SelfFlags  = (orxU16)orxConfig_GetU32(orxBODY_KZ_CONFIG_SELF_FLAGS);
    stBodyPartDef.u16CheckMask  = (orxU16)orxConfig_GetU32(orxBODY_KZ_CONFIG_CHECK_MASK);
    orxVector_Copy(&(stBodyPartDef.vScale), &(_pstBody->vScale));
    if(orxConfig_GetBool(orxBODY_KZ_CONFIG_SOLID) != orxFALSE)
    {
      stBodyPartDef.u32Flags |= orxBODY_PART_DEF_KU32_FLAG_SOLID;
    }
    /* Sphere? */
    if(orxString_Compare(zBodyPartType, orxBODY_KZ_TYPE_SPHERE) == 0)
    {
      /* Updates sphere specific info */
      stBodyPartDef.u32Flags |= orxBODY_PART_DEF_KU32_FLAG_SPHERE;
      if(((orxConfig_HasValue(orxBODY_KZ_CONFIG_CENTER) == orxFALSE)
       && (orxConfig_HasValue(orxBODY_KZ_CONFIG_RADIUS) == orxFALSE))
      || (orxString_Compare(orxString_LowerCase(orxConfig_GetString(orxBODY_KZ_CONFIG_RADIUS)), orxBODY_KZ_FULL) == 0)
      || (orxString_Compare(orxString_LowerCase(orxConfig_GetString(orxBODY_KZ_CONFIG_CENTER)), orxBODY_KZ_FULL) == 0))
      {
        orxVECTOR vPivot, vSize;

        /* Gets object size & pivot */
        orxObject_GetSize(orxOBJECT(_pstBody->pstOwner), &vSize);
        orxObject_GetPivot(orxOBJECT(_pstBody->pstOwner), &vPivot);

        /* Gets radius size */
        orxVector_Mulf(&vSize, &vSize, orx2F(0.5f));

        /* Inits body part def */
        orxVector_Set(&(stBodyPartDef.stSphere.vCenter), vSize.fX - vPivot.fX, vSize.fY - vPivot.fY, vSize.fZ - vPivot.fZ);
        stBodyPartDef.stSphere.fRadius = orxMAX(vSize.fX, orxMAX(vSize.fY, vSize.fZ));
      }
      else
      {
        orxConfig_GetVector(orxBODY_KZ_CONFIG_CENTER, &(stBodyPartDef.stSphere.vCenter));
        stBodyPartDef.stSphere.fRadius = orxConfig_GetFloat(orxBODY_KZ_CONFIG_RADIUS);
      }
    }
    /* Box? */
    else if(orxString_Compare(zBodyPartType, orxBODY_KZ_TYPE_BOX) == 0)
    {
      /* Updates box specific info */
      stBodyPartDef.u32Flags |= orxBODY_PART_DEF_KU32_FLAG_BOX;
      if(((orxConfig_HasValue(orxBODY_KZ_CONFIG_TOP_LEFT) == orxFALSE)
       && (orxConfig_HasValue(orxBODY_KZ_CONFIG_BOTTOM_RIGHT) == orxFALSE))
      || (orxString_Compare(orxString_LowerCase(orxConfig_GetString(orxBODY_KZ_CONFIG_TOP_LEFT)), orxBODY_KZ_FULL) == 0)
      || (orxString_Compare(orxString_LowerCase(orxConfig_GetString(orxBODY_KZ_CONFIG_BOTTOM_RIGHT)), orxBODY_KZ_FULL) == 0))
      {
        orxVECTOR vPivot, vSize;

        /* Gets object size & pivot */
        orxObject_GetSize(orxOBJECT(_pstBody->pstOwner), &vSize);
        orxObject_GetPivot(orxOBJECT(_pstBody->pstOwner), &vPivot);

        /* Inits body part def */
        orxVector_Set(&(stBodyPartDef.stAABox.stBox.vTL), -vPivot.fX, -vPivot.fY, -vPivot.fZ);
        orxVector_Set(&(stBodyPartDef.stAABox.stBox.vBR), vSize.fX - vPivot.fX, vSize.fY - vPivot.fY, vSize.fZ - vPivot.fZ);
      }
      else
      {
        orxConfig_GetVector(orxBODY_KZ_CONFIG_TOP_LEFT, &(stBodyPartDef.stAABox.stBox.vTL));
        orxConfig_GetVector(orxBODY_KZ_CONFIG_BOTTOM_RIGHT, &(stBodyPartDef.stAABox.stBox.vBR));
      }
    }
    /* Mesh */
    else if(orxString_Compare(zBodyPartType, orxBODY_KZ_TYPE_MESH) == 0)
    {
      /* Updates mesh specific info */
      stBodyPartDef.u32Flags |= orxBODY_PART_DEF_KU32_FLAG_MESH;
      if((orxConfig_HasValue(orxBODY_KZ_CONFIG_VERTEX_LIST) != orxFALSE)
      && ((stBodyPartDef.stMesh.u32VertexCounter = orxConfig_GetListCounter(orxBODY_KZ_CONFIG_VERTEX_LIST)) >= 3))
      {
        orxU32 i;

        /* Too many defined vertices? */
        if(stBodyPartDef.stMesh.u32VertexCounter > orxBODY_PART_DEF_KU32_MESH_VERTEX_NUMBER)
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "Too many vertices in the list: %ld. The maximum allowed is: %ld. Using the first %ld ones for the shape <%s>", stBodyPartDef.stMesh.u32VertexCounter, orxBODY_PART_DEF_KU32_MESH_VERTEX_NUMBER, orxBODY_PART_DEF_KU32_MESH_VERTEX_NUMBER, _zConfigID);

          /* Updates vertices number */
          stBodyPartDef.stMesh.u32VertexCounter = orxBODY_PART_DEF_KU32_MESH_VERTEX_NUMBER;
        }

        /* For all defined vertices */
        for(i = 0; i < stBodyPartDef.stMesh.u32VertexCounter; i++)
        {
          /* Gets its vector */
          orxConfig_GetListVector(orxBODY_KZ_CONFIG_VERTEX_LIST, i, &(stBodyPartDef.stMesh.avVertices[i]));
        }
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "Vertex list for creating mesh body <%s> is invalid (missing or less than 3 vertices).", _zConfigID);

        /* Updates result */
        eResult = orxSTATUS_FAILURE;
      }
    }
    /* Unknown */
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "<%s> isn't a valid type for a body part.", zBodyPartType);

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }

    /* Valid? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Adds body part */
      eResult = orxBody_AddPart(_pstBody, _u32Index, &stBodyPartDef);

      /* Valid? */
      if(eResult != orxSTATUS_FAILURE)
      {
        /* Stores its reference */
        _pstBody->astPartList[_u32Index].zReference = orxConfig_GetCurrentSection();

        /* Protects it */
        orxConfig_ProtectSection(_pstBody->astPartList[_u32Index].zReference, orxTRUE);
      }
    }

    /* Pops previous section */
    orxConfig_PopSection();
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "Couldn't find config section named (%s)", _zConfigID);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets a body part
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _u32Index       Body part index (should be less than orxBODY_KU32_DATA_MAX_NUMBER)
 * @return      orxPHYSICS_BODY_PART / orxNULL
 */
orxPHYSICS_BODY_PART *orxFASTCALL orxBody_GetPart(const orxBODY *_pstBody, orxU32 _u32Index)
{
  orxPHYSICS_BODY_PART *pstResult = orxNULL;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstBody);
  orxASSERT(_u32Index < orxBODY_KU32_PART_MAX_NUMBER);

  /* Updates result */
  pstResult = _pstBody->astPartList[_u32Index].pstData;

  /* Done! */
  return pstResult;
}

/** Gets a body part name
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _u32Index       Part index (should be less than orxBODY_KU32_DATA_MAX_NUMBER)
 * @return      orxSTRING / orxNULL
 */
const orxSTRING orxFASTCALL orxBody_GetPartName(const orxBODY *_pstBody, orxU32 _u32Index)
{
  orxSTRING zResult;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstBody);
  orxASSERT(_u32Index < orxBODY_KU32_PART_MAX_NUMBER);

  /* Updates result */
  zResult = _pstBody->astPartList[_u32Index].zReference;

  /* Done! */
  return zResult;
}

/** Removes a body part
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _u32Index       Part index (should be less than orxBODY_KU32_DATA_MAX_NUMBER)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxBody_RemovePart(orxBODY *_pstBody, orxU32 _u32Index)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Has a part? */
  if(_pstBody->astPartList[_u32Index].pstData != orxNULL)
  {
    /* Deletes it */
    orxPhysics_DeleteBodyPart(_pstBody->astPartList[_u32Index].pstData);
    _pstBody->astPartList[_u32Index].pstData = orxNULL;

    /* Has reference? */
    if(_pstBody->astPartList[_u32Index].zReference != orxNULL)
    {
      /* Unprotects it */
      orxConfig_ProtectSection(_pstBody->astPartList[_u32Index].zReference, orxFALSE);

      /* Clears it */
      _pstBody->astPartList[_u32Index].zReference = orxNULL;
    }
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets a body template
 * @param[in]   _pstBodyTemplate  Body template to set / orxNULL to remove it
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxBody_SetTemplate(const orxBODY_DEF *_pstBodyTemplate)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);

  /* Has template? */
  if(_pstBodyTemplate != orxNULL)
  {
    /* Copies template */
    orxMemory_Copy(&(sstBody.stBodyTemplate), _pstBodyTemplate, sizeof(orxBODY_DEF));

    /* Updates flags */
    orxFLAG_SET(sstBody.u32Flags, orxBODY_KU32_FLAG_USE_TEMPLATE, orxBODY_KU32_FLAG_NONE);
  }
  else
  {
    /* Clears template */
    orxMemory_Zero(&(sstBody.stBodyTemplate), sizeof(orxBODY_DEF));

    /* Updates flags */
    orxFLAG_SET(sstBody.u32Flags, orxBODY_KU32_FLAG_NONE, orxBODY_KU32_FLAG_USE_TEMPLATE);
  }

  /* Done! */
  return eResult;
}

/** Sets a body part template
 * @param[in]   _pstBodyPartTemplate  Body part template to set / orxNULL to remove it
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxBody_SetPartTemplate(const orxBODY_PART_DEF *_pstBodyPartTemplate)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);

  /* Has template? */
  if(_pstBodyPartTemplate != orxNULL)
  {
    /* Copies template */
    orxMemory_Copy(&(sstBody.stBodyPartTemplate), _pstBodyPartTemplate, sizeof(orxBODY_PART_DEF));

    /* Updates flags */
    orxFLAG_SET(sstBody.u32Flags, orxBODY_KU32_FLAG_USE_PART_TEMPLATE, orxBODY_KU32_FLAG_NONE);
  }
  else
  {
    /* Clears template */
    orxMemory_Zero(&(sstBody.stBodyPartTemplate), sizeof(orxBODY_PART_DEF));

    /* Updates flags */
    orxFLAG_SET(sstBody.u32Flags, orxBODY_KU32_FLAG_NONE, orxBODY_KU32_FLAG_USE_PART_TEMPLATE);
  }

  /* Done! */
  return eResult;
}

/** Gets the body template
 * @param[out]  _pstBodyTemplate  Body template to get
 * @return      orxBODY_DEF / orxNULL
 */
orxBODY_DEF *orxFASTCALL orxBody_GetTemplate(orxBODY_DEF *_pstBodyTemplate)
{
  orxBODY_DEF *pstResult;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyTemplate != orxNULL);

  /* Has template? */
  if(orxFLAG_TEST(sstBody.u32Flags, orxBODY_KU32_FLAG_USE_TEMPLATE))
  {
    /* Copies template */
    orxMemory_Copy(_pstBodyTemplate, &(sstBody.stBodyTemplate),sizeof(orxBODY_DEF));

    /* Updates result */
    pstResult = _pstBodyTemplate;
  }
  else
  {
    /* Clears template */
    orxMemory_Zero(_pstBodyTemplate, sizeof(orxBODY_DEF));

    /* Updates result */
    pstResult = orxNULL;
  }

  /* Done! */
  return pstResult;
}

/** Gets the body part template
 * @param[out]  _pstBodyPartTemplate  Body part template to get
 * @return      orxBODY_PART_DEF / orxNULL
 */
orxBODY_PART_DEF *orxFASTCALL orxBody_GetPartTemplate(orxBODY_PART_DEF *_pstBodyPartTemplate)
{
  orxBODY_PART_DEF *pstResult;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBodyPartTemplate != orxNULL);

  /* Has template? */
  if(orxFLAG_TEST(sstBody.u32Flags, orxBODY_KU32_FLAG_USE_PART_TEMPLATE))
  {
    /* Copies template */
    orxMemory_Copy(_pstBodyPartTemplate, &(sstBody.stBodyPartTemplate),sizeof(orxBODY_PART_DEF));

    /* Updates result */
    pstResult = _pstBodyPartTemplate;
  }
  else
  {
    /* Clears template */
    orxMemory_Zero(_pstBodyPartTemplate, sizeof(orxBODY_PART_DEF));

    /* Updates result */
    pstResult = orxNULL;
  }

  /* Done! */
  return pstResult;
}

/** Sets a body position
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _pvPosition     Position to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxBody_SetPosition(orxBODY *_pstBody, const orxVECTOR *_pvPosition)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstBody);
  orxASSERT(_pvPosition != orxNULL);

  /* Has data? */
  if(orxStructure_TestFlags(_pstBody, orxBODY_KU32_FLAG_HAS_DATA))
  {
    /* Updates its position */
    eResult = orxPhysics_SetPosition(_pstBody->pstData, _pvPosition);
    orxVector_Copy(&(_pstBody->vPreviousPosition), _pvPosition);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "Structure does not have data.");

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets a body rotation
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _fRotation      Rotation to set (radians)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxBody_SetRotation(orxBODY *_pstBody, orxFLOAT _fRotation)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstBody);

  /* Has data? */
  if(orxStructure_TestFlags(_pstBody, orxBODY_KU32_FLAG_HAS_DATA))
  {
    /* Updates its position */
    eResult = orxPhysics_SetRotation(_pstBody->pstData, _fRotation);
    _pstBody->fPreviousRotation = _fRotation;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "Structure does not have data.");

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets a body scale
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _pvScale        Scale to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxBody_SetScale(orxBODY *_pstBody, const orxVECTOR *_pvScale)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstBody);
  orxASSERT(_pvScale != orxNULL);

  /* Has data? */
  if(orxStructure_TestFlags(_pstBody, orxBODY_KU32_FLAG_HAS_DATA))
  {
    orxU32 i;

    /* Is new scale different? */
    if(orxVector_AreEqual(_pvScale, &(_pstBody->vScale)) == orxFALSE)
    {
      /* Stores it */
      orxVector_Copy(&(_pstBody->vScale), _pvScale);

      /* For all parts */
      for(i = 0; i < orxBODY_KU32_PART_MAX_NUMBER; i++)
      {
        /* Has part? */
        if(_pstBody->astPartList[i].pstData != orxNULL)
        {
          /* Has reference? */
          if(_pstBody->astPartList[i].zReference != orxNULL)
          {
            orxSTRING zReference;

            /* Stores it locally */
            zReference = _pstBody->astPartList[i].zReference;

            /* Removes part */
            orxBody_RemovePart(_pstBody, i);

            /* Creates new part */
            orxBody_AddPartFromConfig(_pstBody, i, zReference);
          }
          else
          {
            /* Logs message */
            orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "[%s]: Scaling of body part with no config reference is unsupported. Please scale only bodies that contain parts created from config.", (_pstBody->pstOwner != orxNULL) ? orxObject_GetName(orxOBJECT(_pstBody->pstOwner)) : "UNDEFINED");
          }
        }
        else
        {
          break;
        }
      }
    }

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "Structure does not have data.");

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets a body speed
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _pvSpeed        Speed to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxBody_SetSpeed(orxBODY *_pstBody, const orxVECTOR *_pvSpeed)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstBody);
  orxASSERT(_pvSpeed != orxNULL);

  /* Has data? */
  if(orxStructure_TestFlags(_pstBody, orxBODY_KU32_FLAG_HAS_DATA))
  {
    /* Updates its speed */
    eResult = orxPhysics_SetSpeed(_pstBody->pstData, _pvSpeed);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "Strcuture does not have data.");

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets a body angular velocity
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _fVelocity      Angular velocity to set (radians/seconds)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxBody_SetAngularVelocity(orxBODY *_pstBody, orxFLOAT _fVelocity)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstBody);

  /* Has data? */
  if(orxStructure_TestFlags(_pstBody, orxBODY_KU32_FLAG_HAS_DATA))
  {
    /* Updates its position */
    eResult = orxPhysics_SetAngularVelocity(_pstBody->pstData, _fVelocity);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "Structure does not have data.");

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets a body custom gravity
 * @param[in]   _pstBody          Concerned body
 * @param[in]   _pvCustomGravity  Custom gravity to set / orxNULL to remove it
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxBody_SetCustomGravity(orxBODY *_pstBody, const orxVECTOR *_pvCustomGravity)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstBody);

  /* Has data? */
  if(orxStructure_TestFlags(_pstBody, orxBODY_KU32_FLAG_HAS_DATA))
  {
    /* Updates its position */
    eResult = orxPhysics_SetCustomGravity(_pstBody->pstData, _pvCustomGravity);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "Structure does not have data.");

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets a body position
 * @param[in]   _pstBody        Concerned body
 * @param[out]  _pvPosition     Position to get
 * @return      Body position / orxNULL
 */
orxVECTOR *orxFASTCALL orxBody_GetPosition(const orxBODY *_pstBody, orxVECTOR *_pvPosition)
{
  orxVECTOR *pvResult;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstBody);
  orxASSERT(_pvPosition != orxNULL);

  /* Has data? */
  if(orxStructure_TestFlags(_pstBody, orxBODY_KU32_FLAG_HAS_DATA))
  {
    /* Updates result */
    pvResult = orxPhysics_GetPosition(_pstBody->pstData, _pvPosition);
  }
  else
  {
    /* Updates result */
    pvResult = orxNULL;
  }

  /* Done! */
  return pvResult;
}

/** Gets a body rotation
 * @param[in]   _pstBody        Concerned body
 * @return      Body rotation (radians)
 */
orxFLOAT orxFASTCALL orxBody_GetRotation(const orxBODY *_pstBody)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstBody);

  /* Has data? */
  if(orxStructure_TestFlags(_pstBody, orxBODY_KU32_FLAG_HAS_DATA))
  {
    /* Updates result */
    fResult = orxPhysics_GetRotation(_pstBody->pstData);
  }
  else
  {
    /* Updates result */
    fResult = orxFLOAT_0;
  }

  /* Done! */
  return fResult;
}

/** Gets a body speed
 * @param[in]   _pstBody        Concerned body
 * @param[out]   _pvSpeed       Speed to get
 * @return      Body speed / orxNULL
 */
orxVECTOR *orxFASTCALL orxBody_GetSpeed(const orxBODY *_pstBody, orxVECTOR *_pvSpeed)
{
  orxVECTOR *pvResult;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstBody);
  orxASSERT(_pvSpeed != orxNULL);

  /* Has data? */
  if(orxStructure_TestFlags(_pstBody, orxBODY_KU32_FLAG_HAS_DATA))
  {
    /* Updates result */
    pvResult = orxPhysics_GetSpeed(_pstBody->pstData, _pvSpeed);
  }
  else
  {
    /* Updates result */
    pvResult = orxNULL;
  }

  /* Done! */
  return pvResult;
}

/** Gets a body angular velocity
 * @param[in]   _pstBody        Concerned body
 * @return      Body angular velocity (radians/seconds)
 */
orxFLOAT orxFASTCALL orxBody_GetAngularVelocity(const orxBODY *_pstBody)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstBody);

  /* Has data? */
  if(orxStructure_TestFlags(_pstBody, orxBODY_KU32_FLAG_HAS_DATA))
  {
    /* Updates result */
    fResult = orxPhysics_GetAngularVelocity(_pstBody->pstData);
  }
  else
  {
    /* Updates result */
    fResult = orxFLOAT_0;
  }

  /* Done! */
  return fResult;
}

/** Gets a body custom gravity
 * @param[in]   _pstBody          Concerned body
 * @param[out]  _pvCustomGravity  Custom gravity to get
 * @return      Body custom gravity / orxNULL is object doesn't have any
 */
orxVECTOR *orxFASTCALL orxBody_GetCustomGravity(const orxBODY *_pstBody, orxVECTOR *_pvCustomGravity)
{
  orxVECTOR *pvResult;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstBody);
  orxASSERT(_pvCustomGravity != orxNULL);

  /* Has data? */
  if(orxStructure_TestFlags(_pstBody, orxBODY_KU32_FLAG_HAS_DATA))
  {
    /* Updates result */
    pvResult = orxPhysics_GetCustomGravity(_pstBody->pstData, _pvCustomGravity);
  }
  else
  {
    /* Updates result */
    pvResult = orxNULL;
  }

  /* Done! */
  return pvResult;
}

/** Gets a body center of mass
 * @param[in]   _pstBody        Concerned body
 * @param[out]  _pvMassCenter   Mass center to get
 * @return      Mass center / orxNULL
 */
orxVECTOR *orxFASTCALL orxBody_GetMassCenter(const orxBODY *_pstBody, orxVECTOR *_pvMassCenter)
{
  orxVECTOR *pvResult;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstBody);
  orxASSERT(_pvMassCenter != orxNULL);

  /* Has data? */
  if(orxStructure_TestFlags(_pstBody, orxBODY_KU32_FLAG_HAS_DATA))
  {
    /* Gets mass center */
    pvResult = orxPhysics_GetMassCenter(_pstBody->pstData, _pvMassCenter);
  }
  else
  {
    /* Updates result */
    pvResult = orxNULL;
  }

  /* Done! */
  return pvResult;
}

/** Applies a torque
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _fTorque        Torque to apply
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxBody_ApplyTorque(orxBODY *_pstBody, orxFLOAT _fTorque)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstBody);

  /* Has data? */
  if(orxStructure_TestFlags(_pstBody, orxBODY_KU32_FLAG_HAS_DATA))
  {
    /* Applies torque */
    eResult = orxPhysics_ApplyTorque(_pstBody->pstData, _fTorque);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Applies a force
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _pvForce        Force to apply
 * @param[in]   _pvPoint        Point (world coordinates) where the force will be applied, if orxNULL, center of mass will be used
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxBody_ApplyForce(orxBODY *_pstBody, const orxVECTOR *_pvForce, const orxVECTOR *_pvPoint)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstBody);
  orxASSERT(_pvForce != orxNULL);

  /* Has data? */
  if(orxStructure_TestFlags(_pstBody, orxBODY_KU32_FLAG_HAS_DATA))
  {
    /* Has given point? */
    if(_pvPoint != orxNULL)
    {
      /* Applies force */
      eResult = orxPhysics_ApplyForce(_pstBody->pstData, _pvForce, _pvPoint);
    }
    else
    {
      orxVECTOR vMassCenter;

      /* Applies force on mass center */
      eResult = orxPhysics_ApplyForce(_pstBody->pstData, _pvForce, orxPhysics_GetMassCenter(_pstBody->pstData, &vMassCenter));
    }
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Applies an impulse
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _pvImpulse      Impulse to apply
 * @param[in]   _pvPoint        Point (world coordinates) where the impulse will be applied, if orxNULL, center of mass will be used
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxBody_ApplyImpulse(orxBODY *_pstBody, const orxVECTOR *_pvImpulse, const orxVECTOR *_pvPoint)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstBody);
  orxASSERT(_pvImpulse != orxNULL);

  /* Has data? */
  if(orxStructure_TestFlags(_pstBody, orxBODY_KU32_FLAG_HAS_DATA))
  {
    /* Has given point? */
    if(_pvPoint != orxNULL)
    {
      /* Applies impulse */
      eResult = orxPhysics_ApplyImpulse(_pstBody->pstData, _pvImpulse, _pvPoint);
    }
    else
    {
      orxVECTOR vMassCenter;

      /* Applies impusle on mass center */
      eResult = orxPhysics_ApplyForce(_pstBody->pstData, _pvImpulse, orxPhysics_GetMassCenter(_pstBody->pstData, &vMassCenter));
    }
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Issues a raycast to test for potential bodies in the way
 * @param[in]   _pvStart        Start of raycast
 * @param[in]   _pvEnd          End of raycast
 * @param[in]   _u16SelfFlags   Selfs flags used for filtering (0xFFFF for no filtering)
 * @param[in]   _u16CheckMask   Check mask used for filtering (0xFFFF for no filtering)
 * @param[in]   _pvContact      If non-null and a contact is found it will be stored here
 * @param[in]   _pvNormal       If non-null and a contact is found, its normal will be stored here
 * @return Colliding orxBODY / orxNULL
 */
orxBODY *orxFASTCALL orxBody_Raycast(const orxVECTOR *_pvStart, const orxVECTOR *_pvEnd, orxU16 _u16SelfFlags, orxU16 _u16CheckMask, orxVECTOR *_pvContact, orxVECTOR *_pvNormal)
{
  orxHANDLE hRaycastResult;
  orxBODY  *pstResult = orxNULL;

  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvStart != orxNULL);
  orxASSERT(_pvEnd != orxNULL);

  /* Issues raycast */
  hRaycastResult = orxPhysics_Raycast(_pvStart, _pvEnd, _u16SelfFlags, _u16CheckMask, _pvContact, _pvNormal);

  /* Found? */
  if(hRaycastResult != orxHANDLE_UNDEFINED)
  {
    /* Updates result */
    pstResult = orxBODY(hRaycastResult);
  }

  /* Done! */
  return pstResult;
}

/** Applies physics simulation result to the Body
 * @param[in]   _pstBody                      Concerned body
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
void orxFASTCALL orxBody_ApplySimulationResult(orxBODY *_pstBody)
{
  /* Checks */
  orxASSERT(sstBody.u32Flags & orxBODY_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstBody);

  /* Has data? */
  if(orxStructure_TestFlags(_pstBody, orxBODY_KU32_FLAG_HAS_DATA))
  {
    /* Dynamic? */
    if(orxFLAG_TEST(_pstBody->u32DefFlags, orxBODY_DEF_KU32_FLAG_DYNAMIC))
    {
      orxFRAME_SPACE  eFrameSpace;
      orxFRAME       *pstFrame;
      orxOBJECT      *pstOwner;

      /* Gets ower */
      pstOwner = orxOBJECT(_pstBody->pstOwner);

      /* Gets its frame */
      pstFrame = orxOBJECT_GET_STRUCTURE(pstOwner, FRAME);

      /* Gets its frame space */
      eFrameSpace = (orxFrame_IsRootChild(pstFrame) != orxFALSE) ? orxFRAME_SPACE_LOCAL : orxFRAME_SPACE_GLOBAL;

      /* Owner enabled? */
      if(orxObject_IsEnabled(pstOwner) != orxFALSE)
      {
        orxVECTOR             vPosition, vSpeed, vDiff;
        orxFLOAT              fZBackup, fRotation, fDiff;
        orxFLOAT              fSpeedCoef;
        const orxCLOCK_INFO  *pstClockInfo;
        orxCLOCK             *pstClock;

        /* Gets owner clock */
        pstClock = orxObject_GetClock(pstOwner);

        /* Gets corresponding clock info */
        pstClockInfo = (pstClock != orxNULL) ? orxClock_GetInfo(pstClock) : orxNULL;

        /* Has a multiply modifier? */
        if((pstClockInfo != orxNULL) && (pstClockInfo->eModType == orxCLOCK_MOD_TYPE_MULTIPLY))
        {
          /* Gets speed coef */
          fSpeedCoef = (pstClockInfo->fModValue != _pstBody->fTimeMultiplier) ? pstClockInfo->fModValue / _pstBody->fTimeMultiplier : orxFLOAT_1;

          /* Stores multiplier */
          _pstBody->fTimeMultiplier = pstClockInfo->fModValue;
        }
        else
        {
          /* Reverts speed coef */
          fSpeedCoef = (_pstBody->fTimeMultiplier != orxFLOAT_1) ? orxFLOAT_1 / _pstBody->fTimeMultiplier : orxFLOAT_1;

          /* Stores multiplier */
          _pstBody->fTimeMultiplier = orxFLOAT_1;
        }

        /* Gets current position */
        orxFrame_GetPosition(pstFrame, eFrameSpace, &vPosition);

        /* Backups its Z */
        fZBackup = vPosition.fZ;

        /* Global space? */
        if(eFrameSpace == orxFRAME_SPACE_GLOBAL)
        {
          /* Computes diff vector & rotation */
          orxVector_Set(&vDiff, vPosition.fX - _pstBody->vPreviousPosition.fX, vPosition.fY - _pstBody->vPreviousPosition.fY, orxFLOAT_0);
          fDiff = orxFrame_GetRotation(pstFrame, eFrameSpace) - _pstBody->fPreviousRotation;
        }

        /* Gets body up-to-date position */
        orxPhysics_GetPosition(_pstBody->pstData, &vPosition);

        /* Restores Z */
        vPosition.fZ = fZBackup;

        /* Gets body up-to-date rotation */
        fRotation = orxPhysics_GetRotation(_pstBody->pstData);

        /* Global space? */
        if(eFrameSpace == orxFRAME_SPACE_GLOBAL)
        {
          /* Updates position & rotation with diffs */
          orxVector_Add(&vPosition, &vPosition, &vDiff);
          fRotation += fDiff;

          /* Stores them */
          orxPhysics_SetPosition(_pstBody->pstData, &vPosition);
          orxPhysics_SetRotation(_pstBody->pstData, fRotation);
          orxVector_Copy(&(_pstBody->vPreviousPosition), &vPosition);
          _pstBody->fPreviousRotation = fRotation;
        }

        /* Updates position */
        orxFrame_SetPosition(pstFrame, eFrameSpace, &vPosition);

        /* Updates rotation */
        orxFrame_SetRotation(pstFrame, eFrameSpace, fRotation);

        /* Updates its angular velocity */
        orxPhysics_SetAngularVelocity(_pstBody->pstData, orxPhysics_GetAngularVelocity(_pstBody->pstData) * fSpeedCoef);

        /* Updates its speed */
        orxPhysics_SetSpeed(_pstBody->pstData, orxVector_Mulf(&vSpeed, orxPhysics_GetSpeed(_pstBody->pstData, &vSpeed), fSpeedCoef));

        /* Has speed coef */
        if(fSpeedCoef != orxFLOAT_1)
        {
          orxVECTOR vGravity;

          /* No custom gravity */
          if(orxBody_GetCustomGravity(_pstBody, &vGravity) == orxNULL)
          {
            /* Uses world gravity */
            orxPhysics_GetGravity(&vGravity);
          }

          /* Applies modified gravity */
          orxBody_SetCustomGravity(_pstBody, orxVector_Mulf(&vGravity, &vGravity, fSpeedCoef));
        }
      }
      else
      {
        orxVECTOR vPosition;

        /* Enforces its body properties */
        orxPhysics_SetRotation(_pstBody->pstData, orxFrame_GetRotation(pstFrame, eFrameSpace));
        orxPhysics_SetAngularVelocity(_pstBody->pstData, orxFLOAT_0);
        orxPhysics_SetPosition(_pstBody->pstData, orxFrame_GetPosition(pstFrame, eFrameSpace, &vPosition));
        orxPhysics_SetSpeed(_pstBody->pstData, &orxVECTOR_0);
      }
    }
  }

  /* Done! */
  return;
}
