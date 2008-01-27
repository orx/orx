/***************************************************************************
 orxObject.c
 Object module
 
 begin                : 01/12/2003
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


#include "object/orxObject.h"

#include "debug/orxDebug.h"
#include "display/orxGraphic.h"
#include "io/orxFileSystem.h"
#include "object/orxFrame.h"
#include "core/orxClock.h"
#include "memory/orxMemory.h"


/*
 * Platform independant defines
 */

#define orxOBJECT_KU32_STATIC_FLAG_NONE         0x00000000

#define orxOBJECT_KU32_STATIC_FLAG_READY        0x00000001
#define orxOBJECT_KU32_STATIC_FLAG_CLOCK        0x00000002

#define orxOBJECT_KU32_STATIC_MASK_ALL          0xFFFFFFFF

#define orxOBJECT_KU32_FLAG_NONE                0x00000000

#define orxOBJECT_KU32_FLAG_ENABLED             0x00000001

#define orxOBJECT_KU32_MASK_ALL                 0xFFFFFFFF


#define orxOBJECT_KC_MARKER_START               '$'
#define orxOBJECT_KC_MARKER_WIDTH               'w'
#define orxOBJECT_KC_MARKER_HEIGHT              'h'


/*
 * Object structure
 */
struct __orxOBJECT_t
{
  /* Public structure, first structure member : 16 */
  orxSTRUCTURE stStructure;

  /* Used structures : 28 */
  orxSTRUCTURE *pastStructure[orxSTRUCTURE_ID_LINKABLE_NUMBER];

  /* Padding */
  orxPAD(28)
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
        if(pstObject->pastStructure[i] != orxNULL)
        {
          /* Updates it */
          if(orxStructure_Update(pstObject->pastStructure[i], pstObject, _pstClockInfo) == orxSTATUS_FAILURE)
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
  orxModule_AddDependency(orxMODULE_ID_OBJECT, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_OBJECT, orxMODULE_ID_FRAME);
  orxModule_AddDependency(orxMODULE_ID_OBJECT, orxMODULE_ID_GRAPHIC);
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

/** Creates a 2D object
 * @return  orxOBJECT / orxNULL
 */
orxOBJECT *orxObject_Create2DObject()
{
  orxTEXTURE *pstTexture;
  orxOBJECT  *pstObject = orxNULL;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);

  /* Loads textures */
  pstTexture = orxTexture_Create();

  /* Valid? */
  if(pstTexture != orxNULL)
  {
    orxGRAPHIC *pstGraphic;

    /* Creates & inits 2D graphic object from texture */
    pstGraphic = orxGraphic_Create();

    /* Valid? */
    if(pstGraphic != orxNULL)
    {
      /* Sets graphic 2D data */
      if(orxGraphic_SetData(pstGraphic, (orxSTRUCTURE *)pstTexture) == orxSTATUS_SUCCESS)
      {
        orxFRAME *pstFrame;

        /* Creates its frame */
        pstFrame = orxFrame_Create(orxFRAME_KU32_FLAG_NONE);

        /* Valid? */
        if(pstFrame != orxNULL)
        {
          /* Creates & inits object */
          pstObject = orxObject_Create();

          /* Valid? */
          if(pstObject != orxNULL)
          {
            /* Links all structures */
            if((orxObject_LinkStructure(pstObject, (orxSTRUCTURE *)pstGraphic) == orxSTATUS_FAILURE)
            || (orxObject_LinkStructure(pstObject, (orxSTRUCTURE *)pstFrame) == orxSTATUS_FAILURE))
            {
              /* !!! MSG !!! */

              /* Deletes all structures */
              orxTexture_Delete(pstTexture);
              orxGraphic_Delete(pstGraphic);
              orxFrame_Delete(pstFrame);

              /* Updates result */
              pstObject = orxNULL;
            }
          }
          else
          {
            /* !!! MSG !!! */

            /* Deletes all structures */
            orxTexture_Delete(pstTexture);
            orxGraphic_Delete(pstGraphic);
            orxFrame_Delete(pstFrame);
          }
        }
        else
        {
          /* !!! MSG !!! */

          /* Deletes all structures */
          orxTexture_Delete(pstTexture);
          orxGraphic_Delete(pstGraphic);
        }
      }
      else
      {
        /* !!! MSG !!! */

        /* Deletes all structures */
        orxTexture_Delete(pstTexture);
        orxGraphic_Delete(pstGraphic);
      }
    }
    else
    {
      /* !!! MSG !!! */

      /* Deletes all structures */
      orxTexture_Delete(pstTexture);
    }
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return pstObject;
}

/** Creates a 2D object from bitmap file
 * @param[in]   _zBitmapFileName      Bitmap file name to associate with the 2D object
 * @ return orxOBJECT / orxNULL
 */
orxOBJECT *orxFASTCALL orxObject_Create2DObjectFromFile(orxCONST orxSTRING _zBitmapFileName)
{
  orxS32      s32FirstMarkerIndex, s32Width, s32Height;
  orxTEXTURE *pstTexture = orxNULL;
  orxOBJECT  *pstObject = orxNULL;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxASSERT(_zBitmapFileName != orxNULL);

  /* Gets marker index */
  s32FirstMarkerIndex = orxString_SearchCharIndex(_zBitmapFileName, orxOBJECT_KC_MARKER_START, 0); 

  /* Use marker? */
  if(s32FirstMarkerIndex >= 0)
  {
    orxFILESYSTEM_INFO  stFileInfo;
    orxBOOL             bDone;
    orxCHAR             zBaseName[256];

    /* Checks */
    orxASSERT(s32FirstMarkerIndex < 255);

    /* Clears buffer */
    orxMemory_Set(zBaseName, 0, 256 * sizeof(orxCHAR));

    /* Gets base name */
    orxString_NCopy(zBaseName, _zBitmapFileName, s32FirstMarkerIndex);

    /* Adds wildcard */
    zBaseName[s32FirstMarkerIndex] = '*';

    /* For all matching file */
    for(bDone = (orxFileSystem_FindFirst(zBaseName, &stFileInfo) != orxSTATUS_FAILURE) ? orxFALSE : orxTRUE;
        bDone == orxFALSE;
        bDone = (orxFileSystem_FindNext(&stFileInfo) != orxSTATUS_FAILURE) ? orxFALSE : orxTRUE)
    {
      orxS32   *ps32Value;
      orxSTRING zRemaining;

      /* Height? */
      if(_zBitmapFileName[s32FirstMarkerIndex + 1] == orxOBJECT_KC_MARKER_HEIGHT)
      {
        /* Updates value pointer */
        ps32Value = &s32Height;
      }
      /* Width? */
      else if(_zBitmapFileName[s32FirstMarkerIndex + 1] == orxOBJECT_KC_MARKER_WIDTH)
      {
        /* Updates value pointer */
        ps32Value = &s32Width;
      }
      else
      {
        /* Updates value pointer */
        ps32Value = orxNULL;
      }

      /* Valid? */
      if(ps32Value != orxNULL)
      {
        /* Gets value */
        if(orxString_ToS32(stFileInfo.zFullName + s32FirstMarkerIndex, 10, ps32Value, &zRemaining) != orxSTATUS_FAILURE)
        {
          orxS32 s32SecondMarkerIndex;

          /* Gets second marker index */
          s32SecondMarkerIndex = orxString_SearchCharIndex(_zBitmapFileName, orxOBJECT_KC_MARKER_START, s32FirstMarkerIndex + 1);

          /* Valid? */
          if(s32SecondMarkerIndex >= 0)
          {
            /* Height? */
            if((_zBitmapFileName[s32SecondMarkerIndex + 1] == orxOBJECT_KC_MARKER_HEIGHT) && (ps32Value == &s32Width))
            {
              /* Updates value pointer */
              ps32Value = &s32Height;
            }
            /* Width? */
            else if((_zBitmapFileName[s32SecondMarkerIndex + 1] == orxOBJECT_KC_MARKER_WIDTH) && (ps32Value == &s32Height))
            {
              /* Updates value pointer */
              ps32Value = &s32Width;
            }
            else
            {
              /* Updates value pointer */
              ps32Value = orxNULL;
            }

            /* Valid? */
            if(ps32Value != orxNULL)
            {
              /* Gets value */
              if(orxString_ToS32(zRemaining + (s32SecondMarkerIndex - s32FirstMarkerIndex - 2), 10, ps32Value, &zRemaining) != orxSTATUS_FAILURE)
              {
                /* Loads texture */
                pstTexture = orxTexture_CreateFromFile(stFileInfo.zFullName);

                /* Valid? */
                if(pstTexture != orxNULL)
                {
                  /* Done! */
                  bDone = orxTRUE;
                }
              }
            }
          }
        }
      }
    }

    /* Closes search */
    orxFileSystem_FindClose(&stFileInfo);
  }
  else
  {  
    /* Loads textures */
    pstTexture = orxTexture_CreateFromFile(_zBitmapFileName);
  }

  /* Valid? */
  if(pstTexture != orxNULL)
  {
    orxGRAPHIC *pstGraphic;

    /* Creates & inits 2D graphic object from texture */
    pstGraphic = orxGraphic_Create();

    /* Valid? */
    if(pstGraphic != orxNULL)
    {
      /* Sets graphic 2D data */
      if(orxGraphic_SetData(pstGraphic, (orxSTRUCTURE *)pstTexture) == orxSTATUS_SUCCESS)
      {
        orxFRAME *pstFrame;

        /* Creates its frame */
        pstFrame = orxFrame_Create(orxFRAME_KU32_FLAG_NONE);

        /* Valid? */
        if(pstFrame != orxNULL)
        {
          /* Creates & inits object */
          pstObject = orxObject_Create();

          /* Valid? */
          if(pstObject != orxNULL)
          {
            /* Links all structures */
            if((orxObject_LinkStructure(pstObject, (orxSTRUCTURE *)pstGraphic) == orxSTATUS_FAILURE)
            || (orxObject_LinkStructure(pstObject, (orxSTRUCTURE *)pstFrame) == orxSTATUS_FAILURE))
            {
              /* !!! MSG !!! */

              /* Deletes all structures */
              orxTexture_Delete(pstTexture);
              orxGraphic_Delete(pstGraphic);
              orxFrame_Delete(pstFrame);

              /* Updates result */
              pstObject = orxNULL;
            }
            else
            {
              /* Uses a pivot? */
              if(s32FirstMarkerIndex >= 0)
              {
                orxVECTOR vPivot;

                /* Sets pivot vector */
                orxVector_Set(&vPivot, orxS2F(s32Width), orxS2F(s32Height), orxFLOAT_0);

                /* Updates graphic */
                orxGraphic_SetPivot(pstGraphic, &vPivot);
              }
            }
          }
          else
          {
            /* !!! MSG !!! */

            /* Deletes all structures */
            orxTexture_Delete(pstTexture);
            orxGraphic_Delete(pstGraphic);
            orxFrame_Delete(pstFrame);
          }
        }
        else
        {
          /* !!! MSG !!! */

          /* Deletes all structures */
          orxTexture_Delete(pstTexture);
          orxGraphic_Delete(pstGraphic);
        }
      }
      else
      {
        /* !!! MSG !!! */

        /* Deletes all structures */
        orxTexture_Delete(pstTexture);
        orxGraphic_Delete(pstGraphic);
      }
    }
    else
    {
      /* !!! MSG !!! */

      /* Deletes all structures */
      orxTexture_Delete(pstTexture);
    }
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return pstObject;
}

/***************************************************************************
 orxObject_LinkStructure
 Links a structure to an object given.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 ***************************************************************************/
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
    _pstObject->pastStructure[eStructureID] = _pstStructure;
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

/***************************************************************************
 orxObject_UnlinkStructure
 Unlinks structure from an object given its ID.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFASTCALL orxObject_UnlinkStructure(orxOBJECT *_pstObject, orxSTRUCTURE_ID _eStructureID)
{
  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_eStructureID < orxSTRUCTURE_ID_LINKABLE_NUMBER);

  /* Needs to be processed? */
  if(_pstObject->pastStructure[_eStructureID] != orxNULL)
  {
    orxSTRUCTURE *pstStructure;

    /* Gets referenced structure */
    pstStructure = _pstObject->pastStructure[_eStructureID];

    /* Decreases structure reference counter */
    orxStructure_DecreaseCounter(pstStructure);

    /* Unlinks structure */
    _pstObject->pastStructure[_eStructureID] = orxNULL;
  }

  return;
}


/* *** Structure accessors *** */


/***************************************************************************
 orxObject_GetStructure
 Gets a structure used by an object, given its structure ID.

 returns: pointer to the requested structure (must be cast correctly)
 ***************************************************************************/
orxSTRUCTURE *orxFASTCALL orxObject_GetStructure(orxCONST orxOBJECT *_pstObject, orxSTRUCTURE_ID _eStructureID)
{
  orxSTRUCTURE *pstStructure = orxNULL;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Offset is valid? */
  if(_eStructureID < orxSTRUCTURE_ID_LINKABLE_NUMBER)
  {
    /* Gets requested structure */
    pstStructure = _pstObject->pastStructure[_eStructureID];
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
  pstGraphic = (orxGRAPHIC *)orxObject_GetStructure(_pstObject, orxSTRUCTURE_ID_GRAPHIC);

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
  pstFrame = (orxFRAME *)orxObject_GetStructure(_pstObject, orxSTRUCTURE_ID_FRAME);

  /* Valid? */
  if(pstFrame != orxNULL)
  {
    /* Sets object position */
    orxFrame_SetPosition(pstFrame, _pvPosition);
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
  pstFrame = (orxFRAME *)orxObject_GetStructure(_pstObject, orxSTRUCTURE_ID_FRAME);

  /* Valid? */
  if(pstFrame != orxNULL)
  {
    /* Sets Object rotation */
    orxFrame_SetRotation(pstFrame, _fRotation);
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
  pstFrame = (orxFRAME *)orxObject_GetStructure(_pstObject, orxSTRUCTURE_ID_FRAME);

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
  pstGraphic = (orxGRAPHIC *)orxObject_GetStructure(_pstObject, orxSTRUCTURE_ID_GRAPHIC);

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
  pstFrame = (orxFRAME *)orxObject_GetStructure(_pstObject, orxSTRUCTURE_ID_FRAME);

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
  pstFrame = (orxFRAME *)orxObject_GetStructure(_pstObject, orxSTRUCTURE_ID_FRAME);

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
  pstFrame = (orxFRAME *)orxObject_GetStructure(_pstObject, orxSTRUCTURE_ID_FRAME);

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
 */
orxVOID orxFASTCALL orxObject_SetParent(orxOBJECT *_pstObject, orxOBJECT *_pstParent)
{
  orxFRAME *pstFrame;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets frame */
  pstFrame = (orxFRAME *)orxObject_GetStructure(_pstObject, orxSTRUCTURE_ID_FRAME);

  /* Updates its parent */
  return(orxFrame_SetParent(pstFrame, (_pstParent != orxNULL) ? (orxFRAME *)orxObject_GetStructure(_pstParent, orxSTRUCTURE_ID_FRAME) : orxNULL));
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
  pstGraphic = (orxGRAPHIC *)orxObject_GetStructure(_pstObject, orxSTRUCTURE_ID_GRAPHIC);

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
