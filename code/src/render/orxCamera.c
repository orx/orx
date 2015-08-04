/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2015 Orx-Project
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
 * @file orxCamera.c
 * @date 10/12/2003
 * @author iarwain@orx-project.org
 *
 */


#include "render/orxCamera.h"

#include "debug/orxDebug.h"
#include "core/orxCommand.h"
#include "core/orxConfig.h"
#include "memory/orxMemory.h"
#include "object/orxObject.h"
#include "object/orxSpawner.h"
#include "object/orxStructure.h"
#include "utils/orxHashTable.h"
#include "utils/orxString.h"


/** Module flags
 */
#define orxCAMERA_KU32_STATIC_FLAG_NONE       0x00000000  /**< No flags */

#define orxCAMERA_KU32_STATIC_FLAG_READY      0x00000001  /**< Ready flag */

#define orxCAMERA_KU32_STATIC_MASK_ALL        0xFFFFFFFF  /**< All mask */


/** orxCAMERA flags / masks
 */
#define orxCAMERA_KU32_FLAG_REFERENCED        0x10000000  /**< Referenced flag */
#define orxCAMERA_KU32_MASK_ALL               0xFFFFFFFF  /**< All mask */


/** Misc defines
 */
#define orxCAMERA_KZ_CONFIG_GROUP_LIST        "GroupList"
#define orxCAMERA_KZ_CONFIG_ZOOM              "Zoom"
#define orxCAMERA_KZ_CONFIG_POSITION          "Position"
#define orxCAMERA_KZ_CONFIG_ROTATION          "Rotation"
#define orxCAMERA_KZ_CONFIG_FRUSTUM_NEAR      "FrustumNear"
#define orxCAMERA_KZ_CONFIG_FRUSTUM_FAR       "FrustumFar"
#define orxCAMERA_KZ_CONFIG_FRUSTUM_WIDTH     "FrustumWidth"
#define orxCAMERA_KZ_CONFIG_FRUSTUM_HEIGHT    "FrustumHeight"
#define orxCAMERA_KZ_CONFIG_PARENT_CAMERA     "ParentCamera"

#define orxCAMERA_KU32_REFERENCE_TABLE_SIZE   16          /**< Reference table size */
#define orxCAMERA_KU32_BANK_SIZE              16          /**< Bank size */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Camera structure
 */
struct __orxCAMERA_t
{
  orxSTRUCTURE    stStructure;                /**< Public structure, first structure member : 32 */
  orxFRAME       *pstFrame;                   /**< Frame : 20 */
  orxAABOX        stFrustum;                  /**< Frustum : 44 */
  const orxSTRING zReference;                 /**< Reference : 48 */
  orxU32          au32GroupIDList[orxCAMERA_KU32_GROUP_ID_NUMBER]; /**< Group ID list : 112 */
};

/** Static structure
 */
typedef struct __orxCAMERA_STATIC_t
{
  orxU32        u32Flags;                     /**< Control flags : 4 */
  orxHASHTABLE *pstReferenceTable;            /**< Table to avoid camera duplication when creating through config file : 8 */

} orxCAMERA_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxCAMERA_STATIC sstCamera;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Command: Create
 */
void orxFASTCALL orxCamera_CommandCreate(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCAMERA *pstCamera;

  /* Creates camera */
  pstCamera = orxCamera_CreateFromConfig(_astArgList[0].zValue);

  /* Updates result */
  _pstResult->u64Value = (pstCamera != orxNULL) ? orxStructure_GetGUID(pstCamera) : orxU64_UNDEFINED;

  /* Done! */
  return;
}

/** Command: Delete
 */
void orxFASTCALL orxCamera_CommandDelete(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCAMERA *pstCamera;

  /* Gets camera */
  pstCamera = orxCAMERA(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstCamera != orxNULL)
  {
    /* Deletes it */
    orxCamera_Delete(pstCamera);

    /* Updates result */
    _pstResult->u64Value = _astArgList[0].u64Value;
  }
  else
  {
    /* Updates result */
    _pstResult->u64Value = orxU64_UNDEFINED;
  }

  /* Done! */
  return;
}

/** Command: GetID
 */
void orxFASTCALL orxCamera_CommandGetID(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCAMERA *pstCamera;

  /* Gets camera */
  pstCamera = orxCAMERA(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstCamera != orxNULL)
  {
    /* Updates result */
    _pstResult->u64Value = _astArgList[0].u64Value;
  }
  else
  {
    /* Updates result */
    _pstResult->u64Value = orxU64_UNDEFINED;
  }

  /* Done! */
  return;
}

/** Command: SetPosition
 */
void orxFASTCALL orxCamera_CommandSetPosition(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCAMERA *pstCamera;

  /* Gets camera */
  pstCamera = orxCAMERA(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstCamera != orxNULL)
  {
    /* Sets its position */
    orxCamera_SetPosition(pstCamera, &(_astArgList[1].vValue));

    /* Updates result */
    _pstResult->u64Value = _astArgList[0].u64Value;
  }
  else
  {
    /* Updates result */
    _pstResult->u64Value = orxU64_UNDEFINED;
  }

  /* Done! */
  return;
}

/** Command: SetRotation
 */
void orxFASTCALL orxCamera_CommandSetRotation(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCAMERA *pstCamera;

  /* Gets camera */
  pstCamera = orxCAMERA(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstCamera != orxNULL)
  {
    /* Sets its rotation */
    orxCamera_SetRotation(pstCamera, orxMATH_KF_DEG_TO_RAD * _astArgList[1].fValue);

    /* Updates result */
    _pstResult->u64Value = _astArgList[0].u64Value;
  }
  else
  {
    /* Updates result */
    _pstResult->u64Value = orxU64_UNDEFINED;
  }

  /* Done! */
  return;
}

/** Command: SetZoom
 */
void orxFASTCALL orxCamera_CommandSetZoom(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCAMERA *pstCamera;

  /* Gets camera */
  pstCamera = orxCAMERA(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstCamera != orxNULL)
  {
    /* Sets its zoom */
    orxCamera_SetZoom(pstCamera, _astArgList[1].fValue);

    /* Updates result */
    _pstResult->u64Value = _astArgList[0].u64Value;
  }
  else
  {
    /* Updates result */
    _pstResult->u64Value = orxU64_UNDEFINED;
  }

  /* Done! */
  return;
}

/** Command: GetPosition
 */
void orxFASTCALL orxCamera_CommandGetPosition(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCAMERA *pstCamera;

  /* Gets camera */
  pstCamera = orxCAMERA(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstCamera != orxNULL)
  {
    /* Gets its position */
    orxCamera_GetPosition(pstCamera, &(_pstResult->vValue));
  }
  else
  {
    /* Updates result */
    orxVector_Copy(&(_pstResult->vValue), &orxVECTOR_0);
  }

  /* Done! */
  return;
}

/** Command: GetRotation
 */
void orxFASTCALL orxCamera_CommandGetRotation(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCAMERA *pstCamera;

  /* Gets camera */
  pstCamera = orxCAMERA(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstCamera != orxNULL)
  {
    /* Gets its position */
    _pstResult->fValue = orxMATH_KF_RAD_TO_DEG * orxCamera_GetRotation(pstCamera);
  }
  else
  {
    /* Updates result */
    _pstResult->fValue = orxFLOAT_0;
  }

  /* Done! */
  return;
}

/** Command: GetZoom
 */
void orxFASTCALL orxCamera_CommandGetZoom(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCAMERA *pstCamera;

  /* Gets camera */
  pstCamera = orxCAMERA(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstCamera != orxNULL)
  {
    /* Gets its zoom */
    _pstResult->fValue = orxCamera_GetZoom(pstCamera);
  }
  else
  {
    /* Updates result */
    _pstResult->fValue = orxFLOAT_0;
  }

  /* Done! */
  return;
}

/** Command: SetFrustum
 */
void orxFASTCALL orxCamera_CommandSetFrustum(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCAMERA *pstCamera;

  /* Gets camera */
  pstCamera = orxCAMERA(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if((pstCamera != orxNULL)
  && (_astArgList[1].fValue >= orxFLOAT_0)
  && (_astArgList[2].fValue >= orxFLOAT_0)
  && (_astArgList[3].fValue) <= (_astArgList[4].fValue))
  {
    /* Sets its zoom */
    orxCamera_SetFrustum(pstCamera, _astArgList[1].fValue, _astArgList[2].fValue, _astArgList[3].fValue, _astArgList[4].fValue);

    /* Updates result */
    _pstResult->u64Value = _astArgList[0].u64Value;
  }
  else
  {
    /* Updates result */
    _pstResult->u64Value = orxU64_UNDEFINED;
  }

  /* Done! */
  return;
}

/** Command: Get
 */
void orxFASTCALL orxCamera_CommandGet(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCAMERA *pstCamera;

  /* Gets camera */
  pstCamera = orxCamera_Get(_astArgList[0].zValue);

  /* Valid? */
  if(pstCamera != orxNULL)
  {
    /* Updates result */
    _pstResult->u64Value = orxStructure_GetGUID(pstCamera);
  }
  else
  {
    /* Updates result */
    _pstResult->u64Value = orxU64_UNDEFINED;
  }

  /* Done! */
  return;
}

/** Command: GetName
 */
void orxFASTCALL orxCamera_CommandGetName(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCAMERA *pstCamera;

  /* Gets camera */
  pstCamera = orxCAMERA(orxStructure_Get(_astArgList[0].u64Value));

  /* Updates result */
  _pstResult->zValue = (pstCamera != orxNULL) ? orxCamera_GetName(pstCamera) : orxSTRING_EMPTY;

  /* Done! */
  return;
}

/** Command: SetParent
 */
void orxFASTCALL orxCamera_CommandSetParent(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCAMERA *pstCamera;

  /* Gets camera */
  pstCamera = orxCAMERA(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstCamera != orxNULL)
  {
    /* Has parent? */
    if((_u32ArgNumber > 1) && (_astArgList[1].u64Value != 0))
    {
      orxSTRUCTURE *pstParent;

      /* Gets parent */
      pstParent = orxStructure_Get(_astArgList[1].u64Value);

      /* Valid? */
      if(pstParent != orxNULL)
      {
        /* Updates its parent */
        orxCamera_SetParent(pstCamera, pstParent);
      }
    }
    else
    {
      /* Removes parent */
      orxCamera_SetParent(pstCamera, orxNULL);
    }

    /* Updates result */
    _pstResult->u64Value = _astArgList[0].u64Value;
  }
  else
  {
    /* Updates result */
    _pstResult->u64Value = orxU64_UNDEFINED;
  }

  /* Done! */
  return;
}

/** Command: GetParent
 */
void orxFASTCALL orxCamera_CommandGetParent(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCAMERA *pstCamera;

  /* Gets camera */
  pstCamera = orxCAMERA(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstCamera != orxNULL)
  {
    orxSTRUCTURE *pstParent;

    /* Gets its parent */
    pstParent = orxCamera_GetParent(pstCamera);

    /* Valid? */
    if(pstParent != orxNULL)
    {
      /* Updates result */
      _pstResult->u64Value = orxStructure_GetGUID(pstParent);
    }
    else
    {
      /* Updates result */
      _pstResult->u64Value = orxU64_UNDEFINED;
    }
  }
  else
  {
    /* Updates result */
    _pstResult->u64Value = orxU64_UNDEFINED;
  }

  /* Done! */
  return;
}

/** Registers all the camera commands
 */
static orxINLINE void orxCamera_RegisterCommands()
{
  /* Command: Create */
  orxCOMMAND_REGISTER_CORE_COMMAND(Camera, Create, "Camera", orxCOMMAND_VAR_TYPE_U64, 1, 0, {"Name", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: Delete */
  orxCOMMAND_REGISTER_CORE_COMMAND(Camera, Delete, "Camera", orxCOMMAND_VAR_TYPE_U64, 1, 0, {"Camera", orxCOMMAND_VAR_TYPE_U64});

  /* Command: GetID */
  orxCOMMAND_REGISTER_CORE_COMMAND(Camera, GetID, "Camera", orxCOMMAND_VAR_TYPE_U64, 1, 0, {"Camera", orxCOMMAND_VAR_TYPE_U64});

  /* Command: SetPosition */
  orxCOMMAND_REGISTER_CORE_COMMAND(Camera, SetPosition, "Camera", orxCOMMAND_VAR_TYPE_U64, 2, 0, {"Camera", orxCOMMAND_VAR_TYPE_U64}, {"Position", orxCOMMAND_VAR_TYPE_VECTOR});
  /* Command: SetRotation */
  orxCOMMAND_REGISTER_CORE_COMMAND(Camera, SetRotation, "Camera", orxCOMMAND_VAR_TYPE_U64, 2, 0, {"Camera", orxCOMMAND_VAR_TYPE_U64}, {"Rotation", orxCOMMAND_VAR_TYPE_FLOAT});
  /* Command: SetZoom */
  orxCOMMAND_REGISTER_CORE_COMMAND(Camera, SetZoom, "Camera", orxCOMMAND_VAR_TYPE_U64, 2, 0, {"Camera", orxCOMMAND_VAR_TYPE_U64}, {"Zoom", orxCOMMAND_VAR_TYPE_FLOAT});
  /* Command: GetPosition */
  orxCOMMAND_REGISTER_CORE_COMMAND(Camera, GetPosition, "Position", orxCOMMAND_VAR_TYPE_VECTOR, 1, 0, {"Camera", orxCOMMAND_VAR_TYPE_U64});
  /* Command: GetRotation */
  orxCOMMAND_REGISTER_CORE_COMMAND(Camera, GetRotation, "Rotation", orxCOMMAND_VAR_TYPE_FLOAT, 1, 0, {"Camera", orxCOMMAND_VAR_TYPE_U64});
  /* Command: GetZoom */
  orxCOMMAND_REGISTER_CORE_COMMAND(Camera, GetZoom, "Zoom", orxCOMMAND_VAR_TYPE_FLOAT, 1, 0, {"Camera", orxCOMMAND_VAR_TYPE_U64});

  /* Command: SetFrustum */
  orxCOMMAND_REGISTER_CORE_COMMAND(Camera, SetFrustum, "Camera", orxCOMMAND_VAR_TYPE_U64, 5, 0, {"Camera", orxCOMMAND_VAR_TYPE_U64}, {"Width", orxCOMMAND_VAR_TYPE_FLOAT}, {"Height", orxCOMMAND_VAR_TYPE_FLOAT}, {"Near", orxCOMMAND_VAR_TYPE_FLOAT}, {"Far", orxCOMMAND_VAR_TYPE_FLOAT});

  /* Command: Get */
  orxCOMMAND_REGISTER_CORE_COMMAND(Camera, Get, "Camera", orxCOMMAND_VAR_TYPE_U64, 1, 0, {"Name", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: GetName */
  orxCOMMAND_REGISTER_CORE_COMMAND(Camera, GetName, "Name", orxCOMMAND_VAR_TYPE_STRING, 1, 0, {"Camera", orxCOMMAND_VAR_TYPE_U64});

  /* Command: SetParent */
  orxCOMMAND_REGISTER_CORE_COMMAND(Camera, SetParent, "Camera", orxCOMMAND_VAR_TYPE_U64, 1, 1, {"Camera", orxCOMMAND_VAR_TYPE_U64}, {"Parent = <void>", orxCOMMAND_VAR_TYPE_U64});
  /* Command: GetParent */
  orxCOMMAND_REGISTER_CORE_COMMAND(Camera, GetParent, "Parent", orxCOMMAND_VAR_TYPE_U64, 1, 0, {"Camera", orxCOMMAND_VAR_TYPE_U64});
}

/** Unregisters all the camera commands
 */
static orxINLINE void orxCamera_UnregisterCommands()
{
  /* Command: Create */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Camera, Create);
  /* Command: Delete */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Camera, Delete);

  /* Command: GetID */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Camera, GetID);

  /* Command: SetPosition */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Camera, SetPosition);
  /* Command: SetRotation */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Camera, SetRotation);
  /* Command: SetZoom */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Camera, SetZoom);
  /* Command: GetPosition */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Camera, GetPosition);
  /* Command: GetRotation */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Camera, GetRotation);
  /* Command: GetZoom */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Camera, GetZoom);

  /* Command: SetFrustum */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Camera, SetFrustum);

  /* Command: Get */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Camera, Get);
  /* Command: GetName */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Camera, GetName);

  /* Command: SetParent */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Camera, SetParent);
  /* Command: GetParent */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Camera, GetParent);
}

/** Deletes all cameras
 */
static orxINLINE void orxCamera_DeleteAll()
{
  orxCAMERA *pstCamera;

  /* Gets first camera */
  pstCamera = orxCAMERA(orxStructure_GetFirst(orxSTRUCTURE_ID_CAMERA));

  /* Non empty? */
  while(pstCamera != orxNULL)
  {
    /* Deletes camera */
    orxCamera_Delete(pstCamera);

    /* Gets first remaining camera */
    pstCamera = orxCAMERA(orxStructure_GetFirst(orxSTRUCTURE_ID_CAMERA));
  }

  return;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Camera module setup
 */
void orxFASTCALL orxCamera_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_CAMERA, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_CAMERA, orxMODULE_ID_STRING);
  orxModule_AddDependency(orxMODULE_ID_CAMERA, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_CAMERA, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_CAMERA, orxMODULE_ID_FRAME);
  orxModule_AddDependency(orxMODULE_ID_CAMERA, orxMODULE_ID_COMMAND);

  /* Done! */
  return;
}

/** Inits Camera module
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxCamera_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY))
  {
    /* Cleans control structure */
    orxMemory_Zero(&sstCamera, sizeof(orxCAMERA_STATIC));

    /* Creates reference table */
    sstCamera.pstReferenceTable = orxHashTable_Create(orxCAMERA_KU32_REFERENCE_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if(sstCamera.pstReferenceTable != orxNULL)
    {
      /* Register commands */
      orxCamera_RegisterCommands();

      /* Registers structure type */
      eResult = orxSTRUCTURE_REGISTER(CAMERA, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxCAMERA_KU32_BANK_SIZE, orxNULL);
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Tried to initialize camera module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Initialized? */
  if(eResult != orxSTATUS_FAILURE)
  {
    /* Inits Flags */
    sstCamera.u32Flags = orxCAMERA_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Has reference table? */
    if(sstCamera.pstReferenceTable != orxNULL)
    {
      /* Deletes it */
      orxHashTable_Delete(sstCamera.pstReferenceTable);
      sstCamera.pstReferenceTable = orxNULL;
    }

    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Initializing camera module failed.");
  }

  /* Done! */
  return eResult;
}

/** Exits from Camera module
 */
void orxFASTCALL orxCamera_Exit()
{
  /* Initialized? */
  if(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY)
  {
    /* Unregisters commands */
    orxCamera_UnregisterCommands();

    /* Deletes camera list */
    orxCamera_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_CAMERA);

    /* Deletes reference table */
    orxHashTable_Delete(sstCamera.pstReferenceTable);
    sstCamera.pstReferenceTable = orxNULL;

    /* Updates flags */
    sstCamera.u32Flags &= ~orxCAMERA_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Tried to exit camera module when it wasn't initialized.");
  }

  return;
}

/** Creates a camera
 * @param[in]   _u32Flags               Camera flags (2D / ...)
 * @return      Created orxCAMERA / orxNULL
 */
orxCAMERA *orxFASTCALL orxCamera_Create(orxU32 _u32Flags)
{
  orxCAMERA *pstCamera;
  orxFRAME  *pstFrame;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxASSERT((_u32Flags & orxCAMERA_KU32_MASK_USER_ALL) == _u32Flags);

  /* Creates camera */
  pstCamera = orxCAMERA(orxStructure_Create(orxSTRUCTURE_ID_CAMERA));

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

        /* Updates its owner */
        orxStructure_SetOwner(pstFrame, pstCamera);

        /* Increases its reference counter */
        orxStructure_IncreaseCounter(pstFrame);

        /* Adds default group ID */
        orxCamera_AddGroupID(pstCamera, orxString_GetID(orxOBJECT_KZ_DEFAULT_GROUP), orxFALSE);

        /* Updates flags */
        orxStructure_SetFlags(pstCamera, orxCAMERA_KU32_FLAG_2D, orxCAMERA_KU32_FLAG_NONE);

        /* Increases counter */
        orxStructure_IncreaseCounter(pstCamera);
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Camera currently only supports 2d.");

        /* Fress partially allocated camera */
        orxFrame_Delete(pstFrame);
        orxStructure_Delete(pstCamera);

        /* Updates result */
        pstCamera = orxNULL;
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Failed to create camera's frame.");

      /* Fress partially allocated camera */
      orxStructure_Delete(pstCamera);

      /* Updates result */
      pstCamera = orxNULL;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Failed to create camera.");
  }

  /* Done! */
  return pstCamera;
}

/** Creates a camera from config
 * @param[in]   _zConfigID    Config ID
 * @ return orxCAMERA / orxNULL
 */
orxCAMERA *orxFASTCALL orxCamera_CreateFromConfig(const orxSTRING _zConfigID)
{
  orxCAMERA *pstResult;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);

  /* Search for camera */
  pstResult = orxCamera_Get(_zConfigID);

  /* Found? */
  if(pstResult != orxNULL)
  {
    /* Increases counter */
    orxStructure_IncreaseCounter(pstResult);
  }
  else
  {
    /* Pushes section */
    if((orxConfig_HasSection(_zConfigID) != orxFALSE)
    && (orxConfig_PushSection(_zConfigID) != orxSTATUS_FAILURE))
    {
      /* Creates 2D default camera */
      pstResult = orxCamera_Create(orxCAMERA_KU32_FLAG_2D);

      /* Valid? */
      if(pstResult != orxNULL)
      {
        orxVECTOR       vPosition;
        const orxSTRING zParentName;
        orxFLOAT        fNear, fFar, fWidth, fHeight;

        /* Has group list? */
        if(orxConfig_HasValue(orxCAMERA_KZ_CONFIG_GROUP_LIST) != orxFALSE)
        {
          orxU32 i, u32Number;

          /* Removes default group ID */
          orxCamera_RemoveGroupID(pstResult, orxString_GetID(orxOBJECT_KZ_DEFAULT_GROUP));

          /* For all groups */
          for(i = 0, u32Number = orxConfig_GetListCounter(orxCAMERA_KZ_CONFIG_GROUP_LIST); i < u32Number; i++)
          {
            /* Adds it */
            orxCamera_AddGroupID(pstResult, orxString_GetID(orxConfig_GetListString(orxCAMERA_KZ_CONFIG_GROUP_LIST, i)), orxFALSE);
          }
        }

        /* Gets frustum info */
        fNear   = orxConfig_GetFloat(orxCAMERA_KZ_CONFIG_FRUSTUM_NEAR);
        fFar    = orxConfig_GetFloat(orxCAMERA_KZ_CONFIG_FRUSTUM_FAR);
        fWidth  = orxConfig_GetFloat(orxCAMERA_KZ_CONFIG_FRUSTUM_WIDTH);
        fHeight = orxConfig_GetFloat(orxCAMERA_KZ_CONFIG_FRUSTUM_HEIGHT);

        /* Applies it */
        orxCamera_SetFrustum(pstResult, fWidth, fHeight, fNear, fFar);

        /* Gets parent name */
        zParentName = orxConfig_GetString(orxCAMERA_KZ_CONFIG_PARENT_CAMERA);

        /* Valid? */
        if((zParentName != orxNULL) && (zParentName != orxSTRING_EMPTY))
        {
          orxCAMERA *pstCamera;

          /* Gets camera */
          pstCamera = orxCamera_CreateFromConfig(zParentName);

          /* Valid? */
          if(pstCamera != orxNULL)
          {
            /* No owner? */
            if(orxStructure_GetOwner(pstCamera) == orxNULL)
            {
              /* Sets it as its own owner */
              orxStructure_SetOwner(pstCamera, pstCamera);
            }

            /* Sets it as parent */
            orxCamera_SetParent(pstResult, pstCamera);
          }
        }

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

        /* Stores its reference key */
        pstResult->zReference = orxConfig_GetCurrentSection();

        /* Protects it */
        orxConfig_ProtectSection(pstResult->zReference, orxTRUE);

        /* Adds it to reference table */
        orxHashTable_Add(sstCamera.pstReferenceTable, orxString_ToCRC(pstResult->zReference), pstResult);

        /* Updates status flags */
        orxStructure_SetFlags(pstResult, orxCAMERA_KU32_FLAG_REFERENCED, orxCAMERA_KU32_FLAG_NONE);
      }

      /* Pops previous section */
      orxConfig_PopSection();
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Cannot find config section named (%s).", _zConfigID);

      /* Updates result */
      pstResult = orxNULL;
    }
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

  /* Decreases counter */
  orxStructure_DecreaseCounter(_pstCamera);

  /* Not referenced? */
  if(orxStructure_GetRefCounter(_pstCamera) == 0)
  {
    /* Removes frame reference */
    orxStructure_DecreaseCounter(_pstCamera->pstFrame);

    /* Removes its owner */
    orxStructure_SetOwner(_pstCamera->pstFrame, orxNULL);

    /* Deletes frame*/
    orxFrame_Delete(_pstCamera->pstFrame);

    /* Is referenced? */
    if(orxStructure_TestFlags(_pstCamera, orxCAMERA_KU32_FLAG_REFERENCED) != orxFALSE)
    {
      /* Removes it from reference table */
      orxHashTable_Remove(sstCamera.pstReferenceTable, orxString_ToCRC(_pstCamera->zReference));
    }

    /* Has reference? */
    if(_pstCamera->zReference != orxNULL)
    {
      /* Unprotects it */
      orxConfig_ProtectSection(_pstCamera->zReference, orxFALSE);
    }

    /* Deletes structure */
    orxStructure_Delete(_pstCamera);
  }
  else
  {
    /* Referenced by others */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Adds a group ID to a camera
 * @param[in] _pstCamera        Concerned camera
 * @param[in] _u32GroupID       ID of the group to add
 * @param[in] _bAddFirst        If true this group will be used *before* any already added ones, otherwise it'll be used *after* all of them
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxCamera_AddGroupID(orxCAMERA *_pstCamera, orxU32 _u32GroupID, orxBOOL _bAddFirst)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstCamera);

  /* Valid? */
  if((_u32GroupID != 0) && (_u32GroupID != orxU32_UNDEFINED))
  {
    orxU32 u32Counter;

    /* Gets number of current stored IDs */
    u32Counter = orxCamera_GetGroupIDCounter(_pstCamera);

    /* Room left? */
    if(u32Counter < orxCAMERA_KU32_GROUP_ID_NUMBER)
    {
      /* Should add it first? */
      if(_bAddFirst != orxFALSE)
      {
        /* For all stored IDs */
        for(; u32Counter > 0; u32Counter--)
        {
          /* Pushes it one slot further */
          _pstCamera->au32GroupIDList[u32Counter] = _pstCamera->au32GroupIDList[u32Counter - 1];
        }

        /* Stores new ID */
        _pstCamera->au32GroupIDList[0] = _u32GroupID;
      }
      else
      {
        /* Stores it */
        _pstCamera->au32GroupIDList[u32Counter] = _u32GroupID;
      }

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Can't add group ID to camera: maximum of IDs <%u> already reached.", u32Counter);

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Can't add group ID to camera: invalid ID <0x%X>.", _u32GroupID);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Removes a group ID from a camera
 * @param[in] _pstCamera        Concerned camera
 * @param[in] _u32GroupID       ID of the group to remove
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxCamera_RemoveGroupID(orxCAMERA *_pstCamera, orxU32 _u32GroupID)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstCamera);

  /* Valid? */
  if((_u32GroupID != 0) && (_u32GroupID != orxU32_UNDEFINED))
  {
    orxU32 i;

    /* For all stored ID */
    for(i = 0; (i < orxCAMERA_KU32_GROUP_ID_NUMBER) && (_pstCamera->au32GroupIDList[i] != 0); i++)
    {
      /* Found? */
      if(_pstCamera->au32GroupIDList[i] == _u32GroupID)
      {
        /* For all stored IDs after current one */
        for(; (i < orxCAMERA_KU32_GROUP_ID_NUMBER - 1) && (_pstCamera->au32GroupIDList[i] != 0); i++)
        {
          /* Moves it one slot closer */
          _pstCamera->au32GroupIDList[i] = _pstCamera->au32GroupIDList[i + 1];
        }

        /* Clears last slot */
        _pstCamera->au32GroupIDList[i] = 0;

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;

        break;
      }
    }

    /* Not found? */
    if(eResult == orxSTATUS_FAILURE)
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Can't remove group ID from camera: ID <0x%X> not found.", _u32GroupID);
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Can't remove group ID from camera: invalid ID <0x%X>.", _u32GroupID);
  }

  /* Done! */
  return eResult;
}

/** Gets number of group IDs of camera
 * @param[in] _pstCamera        Concerned camera
 * @return Number of group IDs of this camera
 */
orxU32 orxFASTCALL orxCamera_GetGroupIDCounter(const orxCAMERA *_pstCamera)
{
  orxU32 u32Result;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstCamera);

  /* Updates result*/
  for(u32Result = 0; (u32Result < orxCAMERA_KU32_GROUP_ID_NUMBER) && (_pstCamera->au32GroupIDList[u32Result] != 0); u32Result++);

  /* Done! */
  return u32Result;
}

/** Gets the group ID of a camera at the given index
 * @param[in] _pstCamera        Concerned camera
 * @param[in] _u32Index         Index of group ID
 * @return Group ID if index is valid, orxU32_UNDEFINED otherwise
 */
orxU32 orxFASTCALL orxCamera_GetGroupID(const orxCAMERA *_pstCamera, orxU32 _u32Index)
{
  orxU32 u32Result;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstCamera);
  orxASSERT(_u32Index < orxCAMERA_KU32_GROUP_ID_NUMBER);

  /* Updates result */
  u32Result = (_pstCamera->au32GroupIDList[_u32Index] != 0) ? _pstCamera->au32GroupIDList[_u32Index] : orxU32_UNDEFINED;

  /* Done! */
  return u32Result;
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
orxSTATUS orxFASTCALL orxCamera_SetPosition(orxCAMERA *_pstCamera, const orxVECTOR *_pvPosition)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstCamera);
  orxASSERT(_pvPosition != orxNULL);

  /* Sets camera position */
  orxFrame_SetPosition(_pstCamera->pstFrame, orxFRAME_SPACE_LOCAL, _pvPosition);

  /* Done! */
  return eResult;
}

/** Sets camera rotation
 * @param[in]   _pstCamera      Concerned camera
 * @param[in]   _fRotation      Camera rotation (radians)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxCamera_SetRotation(orxCAMERA *_pstCamera, orxFLOAT _fRotation)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstCamera);

   /* Sets camera rotation */
  orxFrame_SetRotation(_pstCamera->pstFrame, orxFRAME_SPACE_LOCAL, _fRotation);

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
  orxVECTOR vRecZoom;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxASSERT(_fZoom > orxFLOAT_0);
  orxSTRUCTURE_ASSERT(_pstCamera);

  /* Gets reciprocal zoom */
  orxVector_SetAll(&vRecZoom, orxFLOAT_1 / _fZoom);

  /* Sets camera zoom */
  orxFrame_SetScale(_pstCamera->pstFrame, orxFRAME_SPACE_LOCAL, &vRecZoom);

  /* Done! */
  return eResult;
}

/** Gets camera frustum (3D box for 2D camera)
 * @param[in]   _pstCamera      Concerned camera
 * @param[out]  _pstFrustum    Frustum box
 * @return      Frustum orxAABOX
 */
orxAABOX *orxFASTCALL orxCamera_GetFrustum(const orxCAMERA *_pstCamera, orxAABOX *_pstFrustum)
{
  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstCamera);
  orxASSERT(_pstFrustum != orxNULL);

  /* Stores frustum */
  orxVector_Copy(&(_pstFrustum->vTL), &(_pstCamera->stFrustum.vTL));
  orxVector_Copy(&(_pstFrustum->vBR), &(_pstCamera->stFrustum.vBR));

  /* Done! */
  return _pstFrustum;
}

/** Get camera position
 * @param[in]   _pstCamera      Concerned camera
 * @param[out]  _pvPosition     Camera position
 * @return      orxVECTOR
 */
orxVECTOR *orxFASTCALL orxCamera_GetPosition(const orxCAMERA *_pstCamera, orxVECTOR *_pvPosition)
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
 * @return      Rotation value (radians)
 */
orxFLOAT orxFASTCALL orxCamera_GetRotation(const orxCAMERA *_pstCamera)
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
orxFLOAT orxFASTCALL orxCamera_GetZoom(const orxCAMERA *_pstCamera)
{
  orxVECTOR vScale;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstCamera);

  /* Gets camera scale */
  orxFrame_GetScale(_pstCamera->pstFrame, orxFRAME_SPACE_LOCAL, &vScale);

  /* Done! */
  return(orxFLOAT_1 / vScale.fX);
}

/** Gets camera config name
 * @param[in]   _pstCamera      Concerned camera
 * @return      orxSTRING / orxSTRING_EMPTY
 */
const orxSTRING orxFASTCALL orxCamera_GetName(const orxCAMERA *_pstCamera)
{
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstCamera);

  /* Updates result */
  zResult = (_pstCamera->zReference != orxNULL) ? _pstCamera->zReference : orxSTRING_EMPTY;

  /* Done! */
  return zResult;
}

/** Gets camera given its name
 * @param[in]   _zName          Camera name
 * @return      orxCAMERA / orxNULL
 */
orxCAMERA *orxFASTCALL orxCamera_Get(const orxSTRING _zName)
{
  orxCAMERA *pstResult;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxASSERT(_zName != orxNULL);

  /* Updates result */
  pstResult = (orxCAMERA *)orxHashTable_Get(sstCamera.pstReferenceTable, orxString_ToCRC(_zName));

  /* Done! */
  return pstResult;
}

/** Gets camera frame
 * @param[in]   _pstCamera      Concerned camera
 * @return      orxFRAME
 */
orxFRAME *orxFASTCALL orxCamera_GetFrame(const orxCAMERA *_pstCamera)
{
  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstCamera);

  /* Gets camera frame */
  return(_pstCamera->pstFrame);
}

/** Sets camera parent
 * @param[in]   _pstCamera      Concerned camera
 * @param[in]   _pParent        Parent structure to set (object, spawner, camera or frame) / orxNULL
 * @return      orsSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxCamera_SetParent(orxCAMERA *_pstCamera, void *_pParent)
{
  orxFRAME   *pstFrame;
  orxSTATUS   eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstCamera);
  orxASSERT((_pParent == orxNULL) || (orxStructure_GetID((orxSTRUCTURE *)_pParent) < orxSTRUCTURE_ID_NUMBER));

  /* Gets frame */
  pstFrame = _pstCamera->pstFrame;

  /* No parent? */
  if(_pParent == orxNULL)
  {
    /* Removes parent */
    orxFrame_SetParent(pstFrame, orxNULL);
  }
  else
  {
    /* Depending on parent ID */
    switch(orxStructure_GetID(_pParent))
    {
      case orxSTRUCTURE_ID_CAMERA:
      {
        /* Updates its parent */
        orxFrame_SetParent(pstFrame, orxCAMERA(_pParent)->pstFrame);

        break;
      }

      case orxSTRUCTURE_ID_FRAME:
      {
        /* Updates its parent */
        orxFrame_SetParent(pstFrame, orxFRAME(_pParent));

        break;
      }

      case orxSTRUCTURE_ID_OBJECT:
      {
        /* Updates its parent */
        orxFrame_SetParent(pstFrame, orxOBJECT_GET_STRUCTURE(orxOBJECT(_pParent), FRAME));

        break;
      }

      case orxSTRUCTURE_ID_SPAWNER:
      {
        /* Updates its parent */
        orxFrame_SetParent(pstFrame, orxSpawner_GetFrame(orxSPAWNER(_pParent)));

        break;
      }

      default:
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Invalid ID for structure.");

        /* Updates result */
        eResult = orxSTATUS_FAILURE;

        break;
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Gets camera parent
 * @param[in]   _pstCamera      Concerned camera
 * @return      Parent (object, spawner, camera or frame) / orxNULL
 */
orxSTRUCTURE *orxFASTCALL orxCamera_GetParent(const orxCAMERA *_pstCamera)
{
  orxFRAME     *pstFrame, *pstParentFrame;
  orxSTRUCTURE *pstResult;

  /* Checks */
  orxASSERT(sstCamera.u32Flags & orxCAMERA_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstCamera);

  /* Gets frame */
  pstFrame = _pstCamera->pstFrame;

  /* Checks */
  orxSTRUCTURE_ASSERT(pstFrame);

  /* Gets frame's parent */
  pstParentFrame = orxFrame_GetParent(pstFrame);

  /* Valid? */
  if(pstParentFrame != orxNULL)
  {
    /* Gets its owner */
    pstResult = orxStructure_GetOwner(pstParentFrame);

    /* No owner? */
    if(pstResult == orxNULL)
    {
      /* Updates result with frame itself */
      pstResult = (orxSTRUCTURE *)pstParentFrame;
    }
  }
  else
  {
    /* Updates result */
    pstResult = orxNULL;
  }

  /* Done! */
  return pstResult;
}
