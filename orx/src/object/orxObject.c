/***************************************************************************
 orxObject.c
 Object module
 
 begin                : 01/12/2003
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


#include "object/orxObject.h"

#include "debug/orxDebug.h"
#include "core/orxConfig.h"
#include "anim/orxAnimPointer.h"
#include "display/orxGraphic.h"
#include "physics/orxBody.h"
#include "object/orxFrame.h"
#include "core/orxClock.h"
#include "memory/orxMemory.h"


/*
 * Platform independent defines
 */

#define orxOBJECT_KU32_STATIC_FLAG_NONE         0x00000000

#define orxOBJECT_KU32_STATIC_FLAG_READY        0x00000001
#define orxOBJECT_KU32_STATIC_FLAG_CLOCK        0x00000002

#define orxOBJECT_KU32_STATIC_MASK_ALL          0xFFFFFFFF


#define orxOBJECT_KU32_FLAG_ENABLED             0x10000000

#define orxOBJECT_KU32_MASK_ALL                 0xFFFFFFFF


#define orxOBJECT_KU32_STORAGE_FLAG_NONE        0x00000000

#define orxOBJECT_KU32_STORAGE_FLAG_INTERNAL    0x00000001

#define orxOBJECT_KU32_STORAGE_MASK_ALL         0xFFFFFFFF


#define orxOBJECT_KU32_NEIGHBOR_LIST_SIZE       128


#define orxOBJECT_KZ_CONFIG_GRAPHIC_FILENAME    "Graphic"

/*
 * Object storage structure
 */
typedef struct __orxOBJECT_STORAGE_t
{
  orxSTRUCTURE *pstStructure;                   /**< Structure pointer : 4 */
  orxU32        u32Flags;                       /**< Flags : 8 */

  /* Padding */
  orxPAD(8)

} orxOBJECT_STORAGE;

/*
 * Object structure
 */
struct __orxOBJECT_t
{
  /* Public structure, first structure member : 16 */
  orxSTRUCTURE      stStructure;


  /* Used structures : 40 */
  orxOBJECT_STORAGE astStructure[orxSTRUCTURE_ID_LINKABLE_NUMBER];

  /* Padding */
  orxPAD(40)
};

/*
 * Static structure
 */
typedef struct __orxOBJECT_STATIC_t
{
  /* Clock */
  orxCLOCK *pstClock;

  /* Control flags */
  orxU32 u32Flags;

} orxOBJECT_STATIC;

/*
 * Static data
 */
orxSTATIC orxOBJECT_STATIC sstObject;


/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 orxObject_DeleteAll
 Deletes all objects.

 returns: orxVOID
 ***************************************************************************/
orxSTATIC orxINLINE orxVOID orxObject_DeleteAll()
{
  orxOBJECT *pstObject;
  
  /* Gets first object */
  pstObject = (orxOBJECT *)orxStructure_GetFirst(orxSTRUCTURE_ID_OBJECT);

  /* Non empty? */
  while(pstObject != orxNULL)
  {
    /* Deletes object */
    orxObject_Delete(pstObject);

    /* Gets first object */
    pstObject = (orxOBJECT *)orxStructure_GetFirst(orxSTRUCTURE_ID_OBJECT);
  }

  return;
}

/***************************************************************************
 orxObject_UpdateAll
 Updates all objects.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFASTCALL orxObject_UpdateAll(orxCONST orxCLOCK_INFO *_pstClockInfo, orxVOID *_pstContext)
{
  orxOBJECT *pstObject;

  /* For all objects */
  for(pstObject = (orxOBJECT *)orxStructure_GetFirst(orxSTRUCTURE_ID_OBJECT);
      pstObject != orxNULL;
      pstObject = (orxOBJECT *)orxStructure_GetNext(pstObject))
  {
    /* Is object enabled? */
    if(orxObject_IsEnabled(pstObject) != orxFALSE)
    {
      orxU32 i;

      /* !!! TODO !!! */
      /* Updates culling infos before calling update subfunctions */

      /* For all linked structures */
      for(i = 0; i < orxSTRUCTURE_ID_LINKABLE_NUMBER; i++)
      {
        /* Is structure linked? */
        if(pstObject->astStructure[i].pstStructure != orxNULL)
        {
          /* Updates it */
          if(orxStructure_Update(pstObject->astStructure[i].pstStructure, pstObject, _pstClockInfo) == orxSTATUS_FAILURE)
          {
            /* !!! MSG !!! */
          }
        }
      }
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
 orxObject_Setup
 Object module setup.

 returns: nothing
 ***************************************************************************/
orxVOID orxObject_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_OBJECT, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_OBJECT, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_OBJECT, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_OBJECT, orxMODULE_ID_FRAME);
  orxModule_AddDependency(orxMODULE_ID_OBJECT, orxMODULE_ID_GRAPHIC);
  orxModule_AddDependency(orxMODULE_ID_OBJECT, orxMODULE_ID_BODY);
  orxModule_AddDependency(orxMODULE_ID_OBJECT, orxMODULE_ID_ANIMPOINTER);
  orxModule_AddDependency(orxMODULE_ID_OBJECT, orxMODULE_ID_CLOCK);

  return;
}

/***************************************************************************
 orxObject_Init
 Inits object system.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 ***************************************************************************/
orxSTATUS orxObject_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Set(&sstObject, 0, sizeof(orxOBJECT_STATIC));

    /* Registers structure type */
    eResult = orxSTRUCTURE_REGISTER(OBJECT, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxNULL);

    /* Initialized? */
    if(eResult == orxSTATUS_SUCCESS)
    {
      /* Creates objects clock */
      sstObject.pstClock = orxClock_Create(orxFLOAT_0, orxCLOCK_TYPE_CORE);

      /* Valid? */
      if(sstObject.pstClock != orxNULL)
      {
        /* Registers object update function to clock */
        eResult = orxClock_Register(sstObject.pstClock, orxObject_UpdateAll, orxNULL, orxMODULE_ID_OBJECT);

        /* Success? */
        if(eResult == orxSTATUS_SUCCESS)
        {
          /* Inits Flags */
          sstObject.u32Flags = orxOBJECT_KU32_STATIC_FLAG_READY | orxOBJECT_KU32_STATIC_FLAG_CLOCK;
        }
        else
        {
          /* Deletes clock */
          orxClock_Delete(sstObject.pstClock);
        }
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

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxObject_Exit
 Exits from the object system.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxObject_Exit()
{
  /* Initialized? */
  if(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY)
  {
    /* Deletes object list */
    orxObject_DeleteAll();

    /* Has clock? */
    if(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_CLOCK)
    {
      /* Unregisters object update all function */
      orxClock_Unregister(sstObject.pstClock, orxObject_UpdateAll);

      /* Deletes clock */
      orxClock_Delete(sstObject.pstClock);

      /* Removes reference */
      sstObject.pstClock = orxNULL;

      /* Updates flags */
      sstObject.u32Flags &= ~orxOBJECT_KU32_STATIC_FLAG_CLOCK;
    }

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_OBJECT);

    /* Updates flags */
    sstObject.u32Flags &= ~orxOBJECT_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}

/***************************************************************************
 orxObject_Create
 Creates a new empty object.

 returns: Created object.
 ***************************************************************************/
orxOBJECT *orxObject_Create()
{
  orxOBJECT *pstObject;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);

  /* Creates object */
  pstObject = (orxOBJECT *)orxStructure_Create(orxSTRUCTURE_ID_OBJECT);

  /* Created? */
  if(pstObject != orxNULL)
  {
    /* Inits flags */
    orxStructure_SetFlags(pstObject, orxOBJECT_KU32_FLAG_ENABLED, orxOBJECT_KU32_MASK_ALL);
  }
  else
  {
    /* !!! MSG !!! */
  }

  return pstObject;
}

/***************************************************************************
 orxObject_Delete
 Deletes an object.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 ***************************************************************************/
orxSTATUS orxFASTCALL orxObject_Delete(orxOBJECT *_pstObject)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Not referenced? */
  if(orxStructure_GetRefCounter(_pstObject) == 0)
  {
    orxU32 i;

    /* Unlink all structures */
    for(i = 0; i < orxSTRUCTURE_ID_LINKABLE_NUMBER; i++)
    {
      orxObject_UnlinkStructure(_pstObject, (orxSTRUCTURE_ID)i);
    }

    /* Deletes structure */
    orxStructure_Delete(_pstObject);
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

/** Creates an object from file
 * @param[in]   _zFileName            Resource file name to associate with the 2D object
 * @param[in]   _u32Flags             Object flags (2D / body / ...)
 * @ return orxOBJECT / orxNULL
 */
orxOBJECT *orxFASTCALL orxObject_CreateFromFile(orxCONST orxSTRING _zFileName, orxU32 _u32Flags)
{
  orxOBJECT *pstObject = orxNULL;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxASSERT(_zFileName != orxNULL);
  orxASSERT((_u32Flags & orxOBJECT_KU32_MASK_USER_ALL) == _u32Flags);

  /* Creates object */
  pstObject = orxObject_Create();

  /* Valid? */
  if(pstObject != orxNULL)
  {
    orxFRAME *pstFrame;
    orxU32    u32FrameFlags;

    /* Inits flags */
    u32FrameFlags   = orxFRAME_KU32_FLAG_NONE;
    u32FrameFlags  |= orxFLAG_TEST(_u32Flags, orxOBJECT_KU32_FLAG_SCROLL_X) ? orxFRAME_KU32_FLAG_SCROLL_X : orxFRAME_KU32_FLAG_NONE; 
    u32FrameFlags  |= orxFLAG_TEST(_u32Flags, orxOBJECT_KU32_FLAG_SCROLL_Y) ? orxFRAME_KU32_FLAG_SCROLL_Y : orxFRAME_KU32_FLAG_NONE;

    /* Creates its frame */
    pstFrame = orxFrame_Create(u32FrameFlags);

    /* Valid? */
    if(pstFrame != orxNULL)
    {
      /* Links it */
      if(orxObject_LinkStructure(pstObject, (orxSTRUCTURE *)pstFrame) != orxSTATUS_FAILURE)
      {
        /* Updates flags */
        orxFLAG_SET(pstObject->astStructure[orxSTRUCTURE_ID_FRAME].u32Flags, orxOBJECT_KU32_STORAGE_FLAG_INTERNAL, orxOBJECT_KU32_STORAGE_MASK_ALL);
      }
      else
      {
        /* Deletes all structures */
        orxFrame_Delete(pstFrame);
        orxObject_Delete(pstObject);
        pstObject = orxNULL;
      }
    }
    else
    {
      /* Deletes all structures */
      orxObject_Delete(pstObject);
      pstObject = orxNULL;
    }
  }

  /* Still valid? */
  if(pstObject != orxNULL)
  {
    /* 2D? */
    if(orxFLAG_TEST(_u32Flags, orxOBJECT_KU32_FLAG_2D))
    {
      /* Should create graphic? */
      if(orxFLAG_TEST(_u32Flags, orxOBJECT_KU32_FLAG_GRAPHIC))
      {
        orxGRAPHIC *pstGraphic;

        /* Creates & inits 2D graphic object from texture */
        pstGraphic = orxGraphic_CreateFromFile(_zFileName, orxGRAPHIC_KU32_FLAG_2D);

        /* Valid? */
        if(pstGraphic != orxNULL)
        {
          /* Links it structures */
          if(orxObject_LinkStructure(pstObject, (orxSTRUCTURE *)pstGraphic) != orxSTATUS_FAILURE)
          {
            /* Should center pivot */
            if(orxFLAG_TEST(_u32Flags, orxOBJECT_KU32_FLAG_CENTERED_PIVOT))
            {
              orxFLOAT fWidth, fHeight;

              /* Gets object size */
              if(orxObject_GetSize(pstObject, &fWidth, &fHeight) != orxSTATUS_FAILURE)
              {
                orxVECTOR vPivot;

                /* Inits pivot */
                orxVector_Set(&vPivot, orx2F(0.5f) * fWidth, orx2F(0.5f) * fHeight, orxFLOAT_0);

                /* Updates object pivot */
                if(orxObject_SetPivot(pstObject, &vPivot) != orxSTATUS_FAILURE)
                {
                  /* Updates object flags */
                  orxStructure_SetFlags(pstObject, orxOBJECT_KU32_FLAG_CENTERED_PIVOT, orxOBJECT_KU32_FLAG_NONE);
                }
              }
            }

            /* Updates flags */
            orxFLAG_SET(pstObject->astStructure[orxSTRUCTURE_ID_GRAPHIC].u32Flags, orxOBJECT_KU32_STORAGE_FLAG_INTERNAL, orxOBJECT_KU32_STORAGE_MASK_ALL);
            orxStructure_SetFlags(pstObject, orxOBJECT_KU32_FLAG_GRAPHIC, orxOBJECT_KU32_FLAG_NONE);
          }
          else
          {
            /* !!! MSG !!! */
  
            /* Deletes all structures */
            orxGraphic_Delete(pstGraphic);
            orxObject_Delete(pstObject);
            pstObject = orxNULL;
          }
        }
        else
        {
          /* !!! MSG !!! */
  
          /* Deletes all structures */
          orxObject_Delete(pstObject);
          pstObject = orxNULL;
        }
      }

      /* Valid? */
      if(pstObject != orxNULL)
      {
        /* With body? */
        if(orxFLAG_TEST(_u32Flags, orxOBJECT_KU32_FLAG_BODY))
        {
          orxBODY      *pstBody;
          orxBODY_DEF   stBodyDef;

          /* Gets body template */
          orxBody_GetTemplate(&stBodyDef);

          /* Defaults it */
          if(stBodyDef.fInertia <= orxFLOAT_0)
          {
            stBodyDef.fInertia  = orxFLOAT_1;
          }
          orxFLAG_SET(stBodyDef.u32Flags, orxFLAG_TEST(_u32Flags, orxOBJECT_KU32_FLAG_BODY_DYNAMIC) ? (orxBODY_DEF_KU32_FLAG_2D | orxBODY_DEF_KU32_FLAG_DYNAMIC) : orxBODY_DEF_KU32_FLAG_2D, orxBODY_DEF_KU32_FLAG_NONE);

          /* Creates body */
          pstBody = orxBody_Create((orxSTRUCTURE *)pstObject, &stBodyDef);

          /* Valid? */
          if(pstBody != orxNULL)
          {
            /* Links it structures */
            if(orxObject_LinkStructure(pstObject, (orxSTRUCTURE *)pstBody) != orxSTATUS_FAILURE)
            {
              /* Updates flags */
              orxFLAG_SET(pstObject->astStructure[orxSTRUCTURE_ID_BODY].u32Flags, orxOBJECT_KU32_STORAGE_FLAG_INTERNAL, orxOBJECT_KU32_STORAGE_MASK_ALL);
              orxStructure_SetFlags(pstObject, orxFLAG_TEST(_u32Flags, orxOBJECT_KU32_FLAG_BODY_DYNAMIC) ? (orxOBJECT_KU32_FLAG_BODY | orxOBJECT_KU32_FLAG_BODY_DYNAMIC) : orxOBJECT_KU32_FLAG_BODY, orxOBJECT_KU32_FLAG_NONE);

              /* Asks for body part? */
              if(orxFLAG_TEST(_u32Flags, orxOBJECT_KU32_FLAG_BODY_SPHERE | orxOBJECT_KU32_FLAG_BODY_BOX))
              {
                orxBODY_PART_DEF  stBodyPartDef;
                orxBOOL           bUsePartTemplate;
                orxVECTOR         vPivot;
                orxFLOAT          fWidth, fHeight;

                /* Gets object size & pivot */
                orxObject_GetSize(pstObject, &fWidth, &fHeight);
                orxObject_GetPivot(pstObject, &vPivot);

                /* Gets body part template */
                bUsePartTemplate = (orxBody_GetPartTemplate(&stBodyPartDef) != orxNULL);

                /* Defaults it */
                if(stBodyPartDef.fDensity <= orxFLOAT_0)
                {
                  stBodyPartDef.fDensity = orxFLOAT_1;
                }
                if((bUsePartTemplate == orxFALSE) || (stBodyPartDef.fFriction < orxFLOAT_0))
                {
                  stBodyPartDef.fFriction = orxFLOAT_1;
                }
                if(stBodyPartDef.u32Flags == orxBODY_PART_DEF_KU32_FLAG_NONE)
                {
                  stBodyPartDef.u32Flags = orxBODY_PART_DEF_KU32_FLAG_RIGID;
                }
                if(stBodyPartDef.u16SelfFlags == 0)
                {
                  stBodyPartDef.u16SelfFlags = 1;
                }
                if(stBodyPartDef.u16CheckMask == 0)
                {
                  stBodyPartDef.u16CheckMask = 1;
                }

                /* Sphere? */
                if(orxFLAG_TEST(_u32Flags, orxOBJECT_KU32_FLAG_BODY_SPHERE))
                {
                  orxFLOAT fRadius;

                  /* Gets minimal radius */
                  fRadius = orx2F(0.5f) * orxMIN(fWidth, fHeight);

                  /* Inits body part def */
                  orxVector_Set(&(stBodyPartDef.stSphere.vCenter), fRadius - vPivot.fX, fRadius - vPivot.fY, -vPivot.fZ);
                  stBodyPartDef.stSphere.fRadius = fRadius;
                  orxFLAG_SET(stBodyPartDef.u32Flags, orxBODY_PART_DEF_KU32_FLAG_SPHERE, orxBODY_PART_DEF_KU32_FLAG_NONE); 
                }
                /* Box */
                else
                {
                  /* Inits body part def */
                  orxVector_Set(&(stBodyPartDef.stAABox.stBox.vTL), -vPivot.fX, -vPivot.fY, -vPivot.fZ);
                  orxVector_Set(&(stBodyPartDef.stAABox.stBox.vBR), fWidth - vPivot.fX, fHeight - vPivot.fY, -vPivot.fZ);
                  orxFLAG_SET(stBodyPartDef.u32Flags, orxBODY_PART_DEF_KU32_FLAG_BOX, orxBODY_PART_DEF_KU32_FLAG_NONE); 
                }

                /* Creates body part */
                if(orxBody_AddPart(pstBody, 0, &stBodyPartDef) != orxSTATUS_FAILURE)
                {
                  /* Updates flags */
                  orxStructure_SetFlags(pstObject, orxFLAG_TEST(_u32Flags, orxOBJECT_KU32_FLAG_BODY_SPHERE) ? orxOBJECT_KU32_FLAG_BODY_SPHERE : orxOBJECT_KU32_FLAG_BODY_BOX, orxOBJECT_KU32_FLAG_NONE);
                }
              }
            }
            else
            {
              /* !!! MSG !!! */

              /* Deletes all structures */
              orxBody_Delete(pstBody);
              orxObject_Delete(pstObject);
              pstObject = orxNULL;
            }
          }
          else
          {
            /* !!! MSG !!! */

            /* Deletes all structures */
            orxObject_Delete(pstObject);
            pstObject = orxNULL;
          }
        }
      }
      
      /* Valid? */
      if(pstObject != orxNULL)
      {
        /* Updates its flags */
        orxStructure_SetFlags(pstObject, orxOBJECT_KU32_FLAG_2D, orxOBJECT_KU32_FLAG_NONE);
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

  /* Done! */
  return pstObject;
}

/** Creates an object from config
 * @param[in]   _zConfigID            Config ID
 * @ return orxOBJECT / orxNULL
 */
orxOBJECT *orxFASTCALL orxObject_CreateFromConfig(orxCONST orxSTRING _zConfigID)
{
  orxOBJECT *pstResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxASSERT(_zConfigID != orxNULL);

  /* Selects config section */
  if(orxConfig_SelectSection(_zConfigID) != orxSTATUS_FAILURE)
  {
    orxSTRING zGraphicFileName;
    orxU32    u32Flags;

    /* Defaults to 2D flags */
    u32Flags = orxOBJECT_KU32_FLAG_2D;

    /* Gets graphic file name */
    zGraphicFileName = orxConfig_GetString(orxOBJECT_KZ_CONFIG_GRAPHIC_FILENAME);

    /* Valid? */
    if(*zGraphicFileName != *orxSTRING_EMPTY)
    {
      /* Updates flags */
      u32Flags |= orxOBJECT_KU32_FLAG_GRAPHIC;
    }

    //! TODO
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

/** Links a structure to an object
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _pstStructure   Structure to link
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_LinkStructure(orxOBJECT *_pstObject, orxSTRUCTURE *_pstStructure)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;
  orxSTRUCTURE_ID eStructureID;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxSTRUCTURE_ASSERT(_pstStructure);

  /* Gets structure id & offset */
  eStructureID = orxStructure_GetID(_pstStructure);

  /* Valid? */
  if(eStructureID < orxSTRUCTURE_ID_LINKABLE_NUMBER)
  {
    /* Unlink previous structure if needed */
    orxObject_UnlinkStructure(_pstObject, eStructureID);

    /* Updates structure reference counter */
    orxStructure_IncreaseCounter(_pstStructure);

    /* Links new structure to object */
    _pstObject->astStructure[eStructureID].pstStructure = _pstStructure;
    _pstObject->astStructure[eStructureID].u32Flags     = orxOBJECT_KU32_STORAGE_FLAG_NONE; 
  }
  else
  {
    /* !!! MSG !!! */

    /* Wrong structure ID */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Unlinks structure from an object, given its structure ID
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _eStructureID   ID of structure to unlink
 */
orxVOID orxFASTCALL orxObject_UnlinkStructure(orxOBJECT *_pstObject, orxSTRUCTURE_ID _eStructureID)
{
  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_eStructureID < orxSTRUCTURE_ID_LINKABLE_NUMBER);

  /* Needs to be processed? */
  if(_pstObject->astStructure[_eStructureID].pstStructure != orxNULL)
  {
    orxSTRUCTURE *pstStructure;

    /* Gets referenced structure */
    pstStructure = _pstObject->astStructure[_eStructureID].pstStructure;

    /* Decreases structure reference counter */
    orxStructure_DecreaseCounter(pstStructure);

    /* Was internally handled? */
    if(orxFLAG_TEST(_pstObject->astStructure[_eStructureID].u32Flags, orxOBJECT_KU32_STORAGE_FLAG_INTERNAL))
    {
      /* Depending on structure ID */
      switch(_eStructureID)
      {
        case orxSTRUCTURE_ID_FRAME:
        {
          orxFrame_Delete(orxSTRUCTURE_GET_POINTER(pstStructure, FRAME));
          break;
        }

        case orxSTRUCTURE_ID_GRAPHIC:
        {
          orxGraphic_Delete(orxSTRUCTURE_GET_POINTER(pstStructure, GRAPHIC));
          break;
        }

        case orxSTRUCTURE_ID_ANIMPOINTER:
        {
          orxAnimPointer_Delete(orxSTRUCTURE_GET_POINTER(pstStructure, ANIMPOINTER));
          break;
        }

        case orxSTRUCTURE_ID_BODY:
        {
//! TEMP          orxBody_Delete(orxSTRUCTURE_GET_POINTER(pstStructure, BODY));
          break;
        }

        default:
        {
          orxASSERT(orxFALSE && "Can't destroy this structure type directly from an object.");

          /* !!! MSG !!! */
          break;
        }
      }
    }

    /* Cleans it */
    orxMemory_Set(&(_pstObject->astStructure[_eStructureID]), 0, sizeof(orxOBJECT_STORAGE));
  }

  return;
}


/* *** Structure accessors *** */


/***************************************************************************
 _orxObject_GetStructure
 Gets a structure used by an object, given its structure ID.

 returns: pointer to the requested structure (must be cast correctly)
 ***************************************************************************/
orxSTRUCTURE *orxFASTCALL _orxObject_GetStructure(orxCONST orxOBJECT *_pstObject, orxSTRUCTURE_ID _eStructureID)
{
  orxSTRUCTURE *pstStructure = orxNULL;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Offset is valid? */
  if(_eStructureID < orxSTRUCTURE_ID_LINKABLE_NUMBER)
  {
    /* Gets requested structure */
    pstStructure = _pstObject->astStructure[_eStructureID].pstStructure;
  }

  /* Done ! */
  return pstStructure;
}    

/** Enables/disables an object
 * @param[in]   _pstObject    Concerned object
 * @param[in]   _bEnable      enable / disable
 */
orxVOID orxFASTCALL orxObject_Enable(orxOBJECT *_pstObject, orxBOOL _bEnable)
{
  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Enable? */
  if(_bEnable != orxFALSE)
  {
    /* Updates status flags */
    orxStructure_SetFlags(_pstObject, orxOBJECT_KU32_FLAG_ENABLED, orxOBJECT_KU32_FLAG_NONE);
  }
  else
  {
    /* Updates status flags */
    orxStructure_SetFlags(_pstObject, orxOBJECT_KU32_FLAG_NONE, orxOBJECT_KU32_FLAG_ENABLED);
  }

  return;
}

/** Is object enabled?
 * @param[in]   _pstObject    Concerned object
 * @return      orxTRUE if enabled, orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxObject_IsEnabled(orxCONST orxOBJECT *_pstObject)
{
  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Done! */
  return(orxStructure_TestFlags((orxOBJECT *)_pstObject, orxOBJECT_KU32_FLAG_ENABLED));
}

/** Sets object pivot
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _pvPivot        Object pivot
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetPivot(orxOBJECT *_pstObject, orxCONST orxVECTOR *_pvPivot)
{
  orxGRAPHIC *pstGraphic;
  orxSTATUS   eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pvPivot != orxNULL);

  /* Gets graphic */
  pstGraphic = orxOBJECT_GET_STRUCTURE(_pstObject, GRAPHIC);

  /* Valid? */
  if(pstGraphic != orxNULL)
  {
    /* Sets object pivot */
    orxGraphic_SetPivot(pstGraphic, _pvPivot);
  }
  else
  {
    /* !!! MSG !!! */

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets object position
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _pvPosition     Object position
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetPosition(orxOBJECT *_pstObject, orxCONST orxVECTOR *_pvPosition)
{
  orxFRAME *pstFrame;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pvPosition != orxNULL);

  /* Gets frame */
  pstFrame = orxOBJECT_GET_STRUCTURE(_pstObject, FRAME);

  /* Valid? */
  if(pstFrame != orxNULL)
  {
    orxBODY *pstBody;

    /* Sets object position */
    orxFrame_SetPosition(pstFrame, _pvPosition);

    /* Gets body */
    pstBody = orxOBJECT_GET_STRUCTURE(_pstObject, BODY);

    /* Valid? */
    if(pstBody != orxNULL)
    {
      /* Updates body position */
      orxBody_SetPosition(pstBody, _pvPosition);
    }
  }
  else
  {
    /* !!! MSG !!! */

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets object rotation
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _fRotation      Object rotation
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetRotation(orxOBJECT *_pstObject, orxFLOAT _fRotation)
{
  orxFRAME *pstFrame;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets frame */
  pstFrame = orxOBJECT_GET_STRUCTURE(_pstObject, FRAME);

  /* Valid? */
  if(pstFrame != orxNULL)
  {
    orxBODY *pstBody;

    /* Sets Object rotation */
    orxFrame_SetRotation(pstFrame, _fRotation);

    /* Gets body */
    pstBody = orxOBJECT_GET_STRUCTURE(_pstObject, BODY);

    /* Valid? */
    if(pstBody != orxNULL)
    {
      /* Updates body position */
      orxBody_SetRotation(pstBody, _fRotation);
    }
  }
  else
  {
    /* !!! MSG !!! */

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets Object zoom
 * @param[in]   _pstObject      Concerned Object
 * @param[in]   _fScaleX        Object X scale
 * @param[in]   _fScaleY        Object Y scale
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetScale(orxOBJECT *_pstObject, orxFLOAT _fScaleX, orxFLOAT _fScaleY)
{
  orxFRAME *pstFrame;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets frame */
  pstFrame = orxOBJECT_GET_STRUCTURE(_pstObject, FRAME);

  /* Valid? */
  if(pstFrame != orxNULL)
  {
    /* Sets Object zoom */
    orxFrame_SetScale(pstFrame, _fScaleX, _fScaleY);
  }
  else
  {
    /* !!! MSG !!! */

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Get object pivot
 * @param[in]   _pstObject      Concerned object
 * @param[out]  _pvPivot        Object pivot
 * @return      orxVECTOR / orxNULL
 */
orxVECTOR *orxFASTCALL orxObject_GetPivot(orxCONST orxOBJECT *_pstObject, orxVECTOR *_pvPivot)
{
  orxGRAPHIC  *pstGraphic;
  orxVECTOR   *pvResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pvPivot != orxNULL);

  /* Gets graphic */
  pstGraphic = orxOBJECT_GET_STRUCTURE(_pstObject, GRAPHIC);

  /* Valid? */
  if(pstGraphic != orxNULL)
  {
    /* Gets object pivot */
     pvResult = orxGraphic_GetPivot(pstGraphic, _pvPivot);
  }
  else
  {
    /* !!! MSG !!! */

    /* Updates result */
    pvResult = orxNULL;
  }

  /* Done! */
  return pvResult;
}

/** Get object position
 * @param[in]   _pstObject      Concerned object
 * @param[out]  _pvPosition     Object position
 * @return      orxVECTOR / orxNULL
 */
orxVECTOR *orxFASTCALL orxObject_GetPosition(orxCONST orxOBJECT *_pstObject, orxVECTOR *_pvPosition)
{
  orxFRAME  *pstFrame;
  orxVECTOR *pvResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pvPosition != orxNULL);

  /* Gets frame */
  pstFrame = orxOBJECT_GET_STRUCTURE(_pstObject, FRAME);

  /* Valid? */
  if(pstFrame != orxNULL)
  {
    /* Gets object position */
     pvResult = orxFrame_GetPosition(pstFrame, orxFRAME_SPACE_LOCAL, _pvPosition);
  }
  else
  {
    /* !!! MSG !!! */

    /* Updates result */
    pvResult = orxNULL;
  }

  /* Done! */
  return pvResult;
}

/** Get object rotation
 * @param[in]   _pstObject      Concerned object
 * @return      Rotation value
 */
orxFLOAT orxFASTCALL orxObject_GetRotation(orxCONST orxOBJECT *_pstObject)
{
  orxFRAME *pstFrame;
  orxFLOAT fResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets frame */
  pstFrame = orxOBJECT_GET_STRUCTURE(_pstObject, FRAME);

  /* Valid? */
  if(pstFrame != orxNULL)
  {
    /* Gets object rotation */
    fResult = orxFrame_GetRotation(pstFrame, orxFRAME_SPACE_LOCAL);
  }
  else
  {
    /* !!! MSG !!! */

    /* Updates result */
    fResult = orxFLOAT_0;
  }

  /* Done! */
  return fResult;
}

/** Gets object scale
 * @param[in]   _pstObject      Concerned object
 * @param[out]  _pfScaleX       Object X scale
 * @param[out]  _pfScaleY       Object Y scale
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_GetScale(orxCONST orxOBJECT *_pstObject, orxFLOAT *_pfScaleX, orxFLOAT *_pfScaleY)
{
  orxFRAME *pstFrame;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pfScaleX != orxNULL);
  orxASSERT(_pfScaleY != orxNULL);

  /* Gets frame */
  pstFrame = orxOBJECT_GET_STRUCTURE(_pstObject, FRAME);

  /* Valid? */
  if(pstFrame != orxNULL)
  {
    /* Gets object scale */
    eResult = orxFrame_GetScale(pstFrame, orxFRAME_SPACE_LOCAL, _pfScaleX, _pfScaleY);
  }
  else
  {
    /* !!! MSG !!! */

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets an object parent
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _pstParent      Parent object to set / orxNULL
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetParent(orxOBJECT *_pstObject, orxOBJECT *_pstParent)
{
  orxFRAME   *pstFrame;
  orxSTATUS   eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets frame */
  pstFrame = orxOBJECT_GET_STRUCTURE(_pstObject, FRAME);

  /* Updates its parent */
  orxFrame_SetParent(pstFrame, (_pstParent != orxNULL) ? orxOBJECT_GET_STRUCTURE(_pstParent, FRAME) : orxNULL);

  /* Done! */
  return eResult;
}

/** Gets object size
 * @param[in]   _pstObject      Concerned object
 * @param[out]  _pfWidth        Object's width
 * @param[out]  _pfHeight       Object's height
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_GetSize(orxCONST orxOBJECT *_pstObject, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
{
  orxGRAPHIC *pstGraphic;
  orxSTATUS   eResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pfWidth != orxNULL);
  orxASSERT(_pfHeight != orxNULL);

  /* Gets graphic */
  pstGraphic = orxOBJECT_GET_STRUCTURE(_pstObject, GRAPHIC);

  /* Valid? */
  if(pstGraphic != orxNULL)
  {
    /* Gets its size */
    eResult = orxGraphic_GetSize(pstGraphic, _pfWidth, _pfHeight);
  }
  else
  {
    /* No size */
    *_pfWidth  = *_pfHeight = orx2F(-1.0f);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets an object animset
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _pstAnimSet     Animation set to set / orxNULL
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetAnimSet(orxOBJECT *_pstObject, orxANIMSET *_pstAnimSet)
{
  orxANIMPOINTER *pstAnimPointer;
  orxSTATUS       eResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxSTRUCTURE_ASSERT(_pstAnimSet);

  /* Creates animation pointer from animation set */
  pstAnimPointer = orxAnimPointer_Create(_pstAnimSet);

  /* Valid? */
  if(pstAnimPointer != NULL)
  {
    /* Updates result */
    eResult = orxSTATUS_SUCCESS;

    /* Links it to the object */
    eResult = orxObject_LinkStructure(_pstObject, (orxSTRUCTURE *)pstAnimPointer);

    /* Success? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Updates internal flag */
      orxFLAG_SET(_pstObject->astStructure[orxSTRUCTURE_ID_ANIMSET].u32Flags, orxOBJECT_KU32_STORAGE_FLAG_INTERNAL, orxOBJECT_KU32_STORAGE_MASK_ALL);
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

/** Sets current animation for object
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _hAnimHandle    Animation handle
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetCurrentAnim(orxOBJECT *_pstObject, orxHANDLE _hAnimHandle)
{
  orxANIMPOINTER *pstAnimPointer;
  orxSTATUS       eResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets animation pointer */
  pstAnimPointer = orxOBJECT_GET_STRUCTURE(_pstObject, ANIMPOINTER);

  /* Valid? */
  if(pstAnimPointer != NULL)
  {
    /* Sets current animation */
    eResult = orxAnimPointer_SetCurrentAnim(pstAnimPointer, _hAnimHandle);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets target animation for object
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _hAnimHandle    Animation handle
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetTargetAnim(orxOBJECT *_pstObject, orxHANDLE _hAnimHandle)
{
  orxANIMPOINTER *pstAnimPointer;
  orxSTATUS       eResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets animation pointer */
  pstAnimPointer = orxOBJECT_GET_STRUCTURE(_pstObject, ANIMPOINTER);

  /* Valid? */
  if(pstAnimPointer != NULL)
  {
    /* Sets target animation */
    eResult = orxAnimPointer_SetTargetAnim(pstAnimPointer, _hAnimHandle);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets an object speed
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _pvSpeed        Speed to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxObject_SetSpeed(orxOBJECT *_pstObject, orxCONST orxVECTOR *_pvSpeed)
{
  orxBODY  *pstBody;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pvSpeed != orxNULL);

  /* Gets body */
  pstBody = orxOBJECT_GET_STRUCTURE(_pstObject, BODY);

  /* Valid? */
  if(pstBody != orxNULL)
  {
    /* Updates its speed */
    eResult = orxBody_SetSpeed(pstBody, _pvSpeed);
  }
  else
  {
    /* !!! MSG !!! */

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets an object angular velocity
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _fVelocity      Angular velocity to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxObject_SetAngularVelocity(orxOBJECT *_pstObject, orxFLOAT _fVelocity)
{
  orxBODY  *pstBody;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets body */
  pstBody = orxOBJECT_GET_STRUCTURE(_pstObject, BODY);

  /* Valid? */
  if(pstBody != orxNULL)
  {
    /* Updates its angular velocity */
    eResult = orxBody_SetAngularVelocity(pstBody, _fVelocity);
  }
  else
  {
    /* !!! MSG !!! */

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets an object speed
 * @param[in]   _pstObject      Concerned object
 * @param[out]  _pvSpeed        Speed to get
 * @return      Object speed / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL     orxObject_GetSpeed(orxOBJECT *_pstObject, orxVECTOR *_pvSpeed)
{
  orxBODY    *pstBody;
  orxVECTOR  *pvResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pvSpeed != orxNULL);

  /* Gets body */
  pstBody = orxOBJECT_GET_STRUCTURE(_pstObject, BODY);

  /* Valid? */
  if(pstBody != orxNULL)
  {
    /* Gets its speed */
    pvResult = orxBody_GetSpeed(pstBody, _pvSpeed);
  }
  else
  {
    /* !!! MSG !!! */

    /* Updates result */
    pvResult = orxNULL;
  }

  /* Done! */
  return pvResult;
}

/** Gets an object angular velocity
 * @param[in]   _pstObject      Concerned object
 * @return      Object angular velocity
 */
extern orxDLLAPI orxFLOAT orxFASTCALL       orxObject_GetAngularVelocity(orxOBJECT *_pstObject)
{
  orxBODY  *pstBody;
  orxFLOAT  fResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets body */
  pstBody = orxOBJECT_GET_STRUCTURE(_pstObject, BODY);

  /* Valid? */
  if(pstBody != orxNULL)
  {
    /* Gets its angular velocity */
    fResult = orxBody_GetAngularVelocity(pstBody);
  }
  else
  {
    /* !!! MSG !!! */

    /* Updates result */
    fResult = orxFLOAT_0;
  }

  /* Done! */
  return fResult;
}

/** Gets an object center of mass
 * @param[in]   _pstObject      Concerned object
 * @param[out]  _pvMassCenter   Mass center to get
 * @return      Mass center / orxNULL
 */
orxVECTOR *orxFASTCALL orxObject_GetMassCenter(orxOBJECT *_pstObject, orxVECTOR *_pvMassCenter)
{
  orxBODY    *pstBody;
  orxVECTOR  *pvResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pvMassCenter != orxNULL);

  /* Gets body */
  pstBody = orxOBJECT_GET_STRUCTURE(_pstObject, BODY);

  /* Valid? */
  if(pstBody != orxNULL)
  {
    /* Gets its center of mass */
    pvResult = orxBody_GetMassCenter(pstBody, _pvMassCenter);
  }
  else
  {
    /* !!! MSG !!! */

    /* Updates result */
    pvResult = orxNULL;
  }

  /* Done! */
  return pvResult;
}

/** Applies a torque
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _fTorque        Torque to apply
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_ApplyTorque(orxOBJECT *_pstObject, orxFLOAT _fTorque)
{
  orxBODY  *pstBody;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets body */
  pstBody = orxOBJECT_GET_STRUCTURE(_pstObject, BODY);

  /* Valid? */
  if(pstBody != orxNULL)
  {
    /* Applies torque */
    eResult = orxBody_ApplyTorque(pstBody, _fTorque);
  }
  else
  {
    /* !!! MSG !!! */

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Applies a force
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _pvForce        Force to apply
 * @param[in]   _pvPoint        Point (world coordinates) where the force will be applied, if orxNULL, center of mass will be used
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_ApplyForce(orxOBJECT *_pstObject, orxCONST orxVECTOR *_pvForce, orxCONST orxVECTOR *_pvPoint)
{
  orxBODY  *pstBody;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pvForce != orxNULL);

  /* Gets body */
  pstBody = orxOBJECT_GET_STRUCTURE(_pstObject, BODY);

  /* Valid? */
  if(pstBody != orxNULL)
  {
    /* Applies force */
    eResult = orxBody_ApplyForce(pstBody, _pvForce, _pvPoint);
  }
  else
  {
    /* !!! MSG !!! */

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Applies an impulse
 * @param[in]   _pstObject        Concerned object
 * @param[in]   _pvImpulse      Impulse to apply
 * @param[in]   _pvPoint        Point (world coordinates) where the impulse will be applied, if orxNULL, center of mass will be used
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_ApplyImpulse(orxOBJECT *_pstObject, orxCONST orxVECTOR *_pvImpulse, orxCONST orxVECTOR *_pvPoint)
{
  orxBODY  *pstBody;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pvImpulse != orxNULL);

  /* Gets body */
  pstBody = orxOBJECT_GET_STRUCTURE(_pstObject, BODY);

  /* Valid? */
  if(pstBody != orxNULL)
  {
    /* Applies impulse */
    eResult = orxBody_ApplyImpulse(pstBody, _pvImpulse, _pvPoint);
  }
  else
  {
    /* !!! MSG !!! */

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets object's bounding box
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _pstBoundingBox Bounding box result
 * @return      Bounding box
 */
orxAABOX *orxFASTCALL orxObject_GetBoundingBox(orxCONST orxOBJECT *_pstObject, orxAABOX *_pstBoundingBox)
{
  orxAABOX *pstResult = orxNULL;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pstBoundingBox != orxNULL);

  /* Cleans result */
  orxMemory_Set(_pstBoundingBox, 0, sizeof(orxAABOX));

  /* Is 2D ? */
  if(orxStructure_TestFlags((orxOBJECT *)_pstObject, orxOBJECT_KU32_FLAG_2D))
  {
    orxGRAPHIC *pstGraphic;

    /* Has graphic? */
    if((pstGraphic = orxOBJECT_GET_STRUCTURE(_pstObject, GRAPHIC)) != orxNULL)
    {
      orxTEXTURE *pstTexture;

      /* Has Data? */
      if((pstTexture = orxSTRUCTURE_GET_POINTER(orxGraphic_GetData(pstGraphic), TEXTURE)) != orxNULL)
      {
        orxFLOAT fWidth, fHeight;

        /* Gets size */
        if(orxTexture_GetSize(pstTexture, &fWidth, &fHeight) != orxSTATUS_FAILURE)
        {
          orxVECTOR vPivot, vPosition;
          orxFLOAT  fAngle;

          /* Gets pivot, position & rotation */
          orxObject_GetPivot(_pstObject, &vPivot);
          orxObject_GetPosition(_pstObject, &vPosition);
          fAngle = orxObject_GetRotation(_pstObject);

          /* Updates box */
          orxVector_Sub(&(_pstBoundingBox->vTL), &vPosition, &vPivot);
          orxVector_Set(&(_pstBoundingBox->vBR), _pstBoundingBox->vTL.fX + fWidth, _pstBoundingBox->vTL.fY + fHeight, _pstBoundingBox->vTL.fZ);

          /* Has rotation? */
          if(fAngle != orxFLOAT_0)
          {
            /* Rotates bouding box */
            orxAABox_2DRotate(_pstBoundingBox, _pstBoundingBox, fAngle);
          }

          /* Updates result */
          pstResult = _pstBoundingBox;
        }        
      }
    }
  }

  /* Done! */
  return pstResult;
}

/** Creates a list of object at neighboring of the given box (ie. whose bounding volume intersects this box)
 * @param[in]   _pstCheckBox    Box to check intersection with
 * @return      orxBANK / orxNULL
 */
orxBANK *orxFASTCALL orxObject_CreateNeighborList(orxCONST orxAABOX *_pstCheckBox)
{
  orxAABOX    stObjectBox;
  orxOBJECT  *pstObject;
  orxBANK    *pstResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstCheckBox != orxNULL);

  /* Creates bank */
  pstResult = orxBank_Create(orxOBJECT_KU32_NEIGHBOR_LIST_SIZE, sizeof(orxOBJECT *), orxBANK_KU32_FLAG_NOT_EXPANDABLE, orxMEMORY_TYPE_TEMP);

  /* Valid? */
  if(pstResult != orxNULL)
  {
    orxU32 u32Counter;

    /* For all objects */
    for(u32Counter = 0, pstObject = orxSTRUCTURE_GET_POINTER(orxStructure_GetFirst(orxSTRUCTURE_ID_OBJECT), OBJECT);
        (u32Counter < orxOBJECT_KU32_NEIGHBOR_LIST_SIZE) && (pstObject != orxNULL);
        pstObject = orxSTRUCTURE_GET_POINTER(orxStructure_GetNext(pstObject), OBJECT), u32Counter++)
    {
      /* Gets its bounding box */
      if(orxObject_GetBoundingBox(pstObject, &stObjectBox) != orxNULL)
      {
        /* Is intersecting? */
        if(orxAABox_TestIntersection(_pstCheckBox, &stObjectBox) != orxFALSE)
        {
          orxOBJECT **ppstObject;

          /* Creates a new cell */
          ppstObject = (orxOBJECT **)orxBank_Allocate(pstResult);

          /* Valid? */
          if(ppstObject != orxNULL)
          {
            /* Adds object */
            *ppstObject = pstObject;
          }
          else
          {
            /* !!! MSG !!! */
            break;
          }
        }
      }
    }
  }

  /* Done! */
  return pstResult;
}

/** Deletes an object list created with orxObject_CreateNeigborList
 * @param[in]   _astObjectList  Concerned object list
 */
orxVOID orxFASTCALL orxObject_DeleteNeighborList(orxBANK *_pstObjectList)
{
  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);

  /* Non null? */
  if(_pstObjectList != orxNULL)
  {
    /* Deletes it */
    orxBank_Delete(_pstObjectList);
  }
}
