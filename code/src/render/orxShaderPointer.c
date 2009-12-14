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
 * @file orxShaderPointer.c
 * @date 08/04/2009
 * @author iarwain@orx-project.org
 *
 */


#include "render/orxShaderPointer.h"

#include "debug/orxDebug.h"
#include "core/orxEvent.h"
#include "memory/orxMemory.h"
#include "object/orxStructure.h"
#include "object/orxObject.h"
#include "display/orxGraphic.h"
#include "display/orxTexture.h"
#include "render/orxRender.h"
#include "render/orxViewport.h"


/** Module flags
 */
#define orxSHADERPOINTER_KU32_STATIC_FLAG_NONE      0x00000000

#define orxSHADERPOINTER_KU32_STATIC_FLAG_READY     0x00000001

#define orxSHADERPOINTER_KU32_STATIC_MASK_ALL       0xFFFFFFFF


/** Flags
 */
#define orxSHADERPOINTER_KU32_FLAG_NONE             0x00000000  /**< No flags */

#define orxSHADERPOINTER_KU32_FLAG_ENABLED          0x10000000  /**< Enabled flag */
#define orxSHADERPOINTER_KU32_FLAG_INTERNAL         0x20000000  /**< Internal flag */
#define orxSHADERPOINTER_KU32_FLAG_OBJECT_OWNER     0x40000000  /**< Object owner flag */
#define orxSHADERPOINTER_KU32_FLAG_VIEWPORT_OWNER   0x80000000  /**< Viewport owner flag */

#define orxSHADERPOINTER_KU32_MASK_ALL              0xFFFFFFFF  /**< All mask */


/** Holder flags
 */
#define orxSHADERPOINTER_HOLDER_KU32_FLAG_NONE      0x00000000  /**< No flags */

#define orxSHADERPOINTER_HOLDER_KU32_FLAG_INTERNAL  0x10000000  /**< Internal flag */

#define orxSHADERPOINTER_HOLDER_KU32_MASK_ALL       0xFFFFFFFF  /**< All mask */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** ShaderPointer holder structure
 */
typedef struct __orxSHADERPOINTER_HOLDER_t
{
  orxSHADER  *pstShader;                                        /**< Shader reference : 4 */
  orxU32      u32Flags;                                         /**< Flags : 8 */

} orxSOUNDPOINTER_HOLDER;

/** ShaderPointer structure
 */
struct __orxSHADERPOINTER_t
{
  orxSTRUCTURE            stStructure;                          /**< Public structure, first structure member : 16 */
  orxSOUNDPOINTER_HOLDER  astShaderList[orxSHADERPOINTER_KU32_SHADER_NUMBER]; /**< Shader list : 48 */
  const orxSTRUCTURE     *pstOwner;                             /**< Owner structure : 52 */
};

/** Static structure
 */
typedef struct __orxSHADERPOINTER_STATIC_t
{
  orxU32 u32Flags;                                              /**< Control flags */

} orxSHADERPOINTER_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxSHADERPOINTER_STATIC sstShaderPointer;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Event handler
 * @param[in]   _pstEvent                     Sent event
 * @return      orxSTATUS_SUCCESS if handled / orxSTATUS_FAILURE otherwise
 */
static orxSTATUS orxFASTCALL orxShaderPointer_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(_pstEvent->eType == orxEVENT_TYPE_RENDER);

  /* Depending on event ID */
  switch(_pstEvent->eID)
  {
    case orxRENDER_EVENT_VIEWPORT_STOP:
    case orxRENDER_EVENT_OBJECT_STOP:
    {
      const orxSHADERPOINTER *pstShaderPointer;

      /* Gets its shader pointer */
      pstShaderPointer = (_pstEvent->eID == orxRENDER_EVENT_VIEWPORT_STOP) ? orxViewport_GetShaderPointer(orxVIEWPORT(_pstEvent->hSender)) : orxOBJECT_GET_STRUCTURE(orxOBJECT(_pstEvent->hSender), SHADERPOINTER);

      /* Found? */
      if(pstShaderPointer != orxNULL)
      {
        /* Renders it */
        orxShaderPointer_Render(pstShaderPointer);
      }

      break;
    }

    case orxRENDER_EVENT_STOP:
    {
      orxSHADERPOINTER *pstShaderPointer;

      /* For all shader pointers */
      for(pstShaderPointer = orxSHADERPOINTER(orxStructure_GetFirst(orxSTRUCTURE_ID_SHADERPOINTER));
          pstShaderPointer != orxNULL;
          pstShaderPointer = orxSHADERPOINTER(orxStructure_GetNext(pstShaderPointer)))
      {
        /* No owner? */
        if(pstShaderPointer->pstOwner == orxNULL)
        {
          /* Renders it */
          orxShaderPointer_Render(pstShaderPointer);
        }
      }

      break;
    }

    default:
    {
      break;
    }
  }

  /* Done! */
  return eResult;
}

/** Deletes all the ShaderPointers
 */
static orxINLINE void orxShaderPointer_DeleteAll()
{
  orxSHADERPOINTER *pstShaderPointer;

  /* Gets first ShaderPointer */
  pstShaderPointer = orxSHADERPOINTER(orxStructure_GetFirst(orxSTRUCTURE_ID_SHADERPOINTER));

  /* Non empty? */
  while(pstShaderPointer != orxNULL)
  {
    /* Deletes it */
    orxShaderPointer_Delete(pstShaderPointer);

    /* Gets first ShaderPointer */
    pstShaderPointer = orxSHADERPOINTER(orxStructure_GetFirst(orxSTRUCTURE_ID_SHADERPOINTER));
  }

  return;
}

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** ShaderPointer module setup
 */
void orxFASTCALL orxShaderPointer_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_SHADERPOINTER, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_SHADERPOINTER, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_SHADERPOINTER, orxMODULE_ID_EVENT);
  orxModule_AddDependency(orxMODULE_ID_SHADERPOINTER, orxMODULE_ID_SHADER);
  orxModule_AddDependency(orxMODULE_ID_SHADERPOINTER, orxMODULE_ID_TEXTURE);

  return;
}

/** Inits the ShaderPointer module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxShaderPointer_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstShaderPointer.u32Flags & orxSHADERPOINTER_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstShaderPointer, sizeof(orxSHADERPOINTER_STATIC));

    /* Adds event handler */
    eResult = orxEvent_AddHandler(orxEVENT_TYPE_RENDER, orxShaderPointer_EventHandler);

    /* Success? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Registers structure type */
      eResult = orxSTRUCTURE_REGISTER(SHADERPOINTER, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxNULL);

      /* Initialized? */
      if(eResult == orxSTATUS_SUCCESS)
      {
        /* Inits Flags */
        sstShaderPointer.u32Flags = orxSHADERPOINTER_KU32_STATIC_FLAG_READY;
      }
      else
      {
        /* Removes event handler */
        orxEvent_RemoveHandler(orxEVENT_TYPE_RENDER, orxShaderPointer_EventHandler);

        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Failed to register linked list structure.");
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Failed to register event handler.");
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Tried to load shader pointer module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Exits from the ShaderPointer module
 */
void orxFASTCALL orxShaderPointer_Exit()
{
  /* Initialized? */
  if(sstShaderPointer.u32Flags & orxSHADERPOINTER_KU32_STATIC_FLAG_READY)
  {
    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_RENDER, orxShaderPointer_EventHandler);

    /* Deletes ShaderPointer list */
    orxShaderPointer_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_SHADERPOINTER);

    /* Updates flags */
    sstShaderPointer.u32Flags &= ~orxSHADERPOINTER_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Tried to exit shader pointer module when it wasn't initialized.");
  }

  return;
}

/** Creates an empty ShaderPointer
 * @param[in]   _pstOwner       ShaderPointer's owner used for rendering (usually an orxOBJECT)
 * @return      Created orxSHADERPOINTER / orxNULL
 */
orxSHADERPOINTER *orxFASTCALL orxShaderPointer_Create(const orxSTRUCTURE *_pstOwner)
{
  orxSHADERPOINTER *pstResult;

  /* Checks */
  orxASSERT(sstShaderPointer.u32Flags & orxSHADERPOINTER_KU32_STATIC_FLAG_READY);

  /* Creates ShaderPointer */
  pstResult = orxSHADERPOINTER(orxStructure_Create(orxSTRUCTURE_ID_SHADERPOINTER));

  /* Created? */
  if(pstResult != orxNULL)
  {
    /* Has owner? */
    if(_pstOwner != orxNULL)
    {
      /* Stores it */
      pstResult->pstOwner = _pstOwner;

      /* Depending on owner ID */
      switch(orxStructure_GetID(_pstOwner))
      {
        case orxSTRUCTURE_ID_OBJECT:
        {
          /* Updates flags */
          orxStructure_SetFlags(pstResult, orxSHADERPOINTER_KU32_FLAG_OBJECT_OWNER, orxSHADERPOINTER_KU32_MASK_ALL);

          break;
        }

        case orxSTRUCTURE_ID_VIEWPORT:
        {
          /* Updates flags */
          orxStructure_SetFlags(pstResult, orxSHADERPOINTER_KU32_FLAG_VIEWPORT_OWNER, orxSHADERPOINTER_KU32_MASK_ALL);

          break;
        }

        default:
        {
          break;
        }
      }
    }

    /* Inits flags */
    orxStructure_SetFlags(pstResult, orxSHADERPOINTER_KU32_FLAG_ENABLED, orxSHADERPOINTER_KU32_MASK_ALL);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Failed to create shader pointer structure.");
  }

  /* Done! */
  return pstResult;
}

/** Deletes an ShaderPointer
 * @param[in] _pstShaderPointer     Concerned ShaderPointer
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxShaderPointer_Delete(orxSHADERPOINTER *_pstShaderPointer)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstShaderPointer.u32Flags & orxSHADERPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShaderPointer);

  /* Not referenced? */
  if(orxStructure_GetRefCounter(_pstShaderPointer) == 0)
  {
    orxU32 i;

    /* For all shaders */
    for(i = 0; i < orxSHADERPOINTER_KU32_SHADER_NUMBER; i++)
    {
      /* Valid? */
      if(_pstShaderPointer->astShaderList[i].pstShader != orxNULL)
      {
        /* Decreases its reference counter */
        orxStructure_DecreaseCounter(_pstShaderPointer->astShaderList[i].pstShader);

        /* Is internal? */
        if(orxFLAG_TEST(_pstShaderPointer->astShaderList[i].u32Flags, orxSHADERPOINTER_HOLDER_KU32_FLAG_INTERNAL))
        {
          /* Deletes it */
          orxShader_Delete(_pstShaderPointer->astShaderList[i].pstShader);
        }
      }
    }

    /* Deletes structure */
    orxStructure_Delete(_pstShaderPointer);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Cannot delete shader pointer while it is still referenced.");

    /* Referenced by others */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Renders a ShaderPointer
 * @param[in] _pstShaderPointer     Concerned ShaderPointer
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxShaderPointer_Render(const orxSHADERPOINTER *_pstShaderPointer)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstShaderPointer.u32Flags & orxSHADERPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShaderPointer);

  /* Enabled? */
  if(orxStructure_TestFlags(_pstShaderPointer, orxSHADERPOINTER_KU32_FLAG_ENABLED))
  {
    orxU32 i;

    /* For all shaders */
    for(i = 0; i < orxSHADERPOINTER_KU32_SHADER_NUMBER; i++)
    {
      /* Valid? */
      if(_pstShaderPointer->astShaderList[i].pstShader != orxNULL)
      {
        /* Renders it */
        eResult = orxShader_Render(_pstShaderPointer->astShaderList[i].pstShader, orxSTRUCTURE(_pstShaderPointer->pstOwner));
      }
    }
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Gets an ShaderPointer owner
 * @param[in]   _pstShaderPointer   Concerned ShaderPointer
 * @return      orxSTRUCTURE / orxNULL
 */
orxSTRUCTURE *orxFASTCALL orxShaderPointer_GetOwner(const orxSHADERPOINTER *_pstShaderPointer)
{
  orxSTRUCTURE *pstResult;

  /* Checks */
  orxASSERT(sstShaderPointer.u32Flags & orxSHADERPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShaderPointer);

  /* Updates result */
  pstResult = orxSTRUCTURE(_pstShaderPointer->pstOwner);

  /* Done! */
  return pstResult;
}

/** Enables/disables an ShaderPointer
 * @param[in]   _pstShaderPointer Concerned ShaderPointer
 * @param[in]   _bEnable      enable / disable
 */
void orxFASTCALL    orxShaderPointer_Enable(orxSHADERPOINTER *_pstShaderPointer, orxBOOL _bEnable)
{
  /* Checks */
  orxASSERT(sstShaderPointer.u32Flags & orxSHADERPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShaderPointer);

  /* Enable? */
  if(_bEnable != orxFALSE)
  {
    /* Updates status flags */
    orxStructure_SetFlags(_pstShaderPointer, orxSHADERPOINTER_KU32_FLAG_ENABLED, orxSHADERPOINTER_KU32_FLAG_NONE);
  }
  else
  {
    /* Updates status flags */
    orxStructure_SetFlags(_pstShaderPointer, orxSHADERPOINTER_KU32_FLAG_NONE, orxSHADERPOINTER_KU32_FLAG_ENABLED);
  }

  return;
}

/** Is ShaderPointer enabled?
 * @param[in]   _pstShaderPointer Concerned ShaderPointer
 * @return      orxTRUE if enabled, orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxShaderPointer_IsEnabled(const orxSHADERPOINTER *_pstShaderPointer)
{
  /* Checks */
  orxASSERT(sstShaderPointer.u32Flags & orxSHADERPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShaderPointer);

  /* Done! */
  return(orxStructure_TestFlags(_pstShaderPointer, orxSHADERPOINTER_KU32_FLAG_ENABLED));
}

/** Adds a shader
 * @param[in]   _pstShaderPointer Concerned ShaderPointer
 * @param[in]   _pstShader        Shader to add
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxShaderPointer_AddShader(orxSHADERPOINTER *_pstShaderPointer, orxSHADER *_pstShader)
{
  orxU32    u32Index;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstShaderPointer.u32Flags & orxSHADERPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShaderPointer);
  orxSTRUCTURE_ASSERT(_pstShader);

  /* Finds an empty slot */
  for(u32Index = 0; (u32Index < orxSHADERPOINTER_KU32_SHADER_NUMBER) && (_pstShaderPointer->astShaderList[u32Index].pstShader != orxNULL); u32Index++);

  /* Found? */
  if(u32Index < orxSHADERPOINTER_KU32_SHADER_NUMBER)
  {
    /* Increases its reference counter */
    orxStructure_IncreaseCounter(_pstShader);

    /* Adds it to holder */
    _pstShaderPointer->astShaderList[u32Index].pstShader = _pstShader;

    /* Updates its flags */
    orxFLAG_SET(_pstShaderPointer->astShaderList[u32Index].u32Flags, orxSHADERPOINTER_HOLDER_KU32_FLAG_NONE, orxSHADERPOINTER_HOLDER_KU32_MASK_ALL);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "No available slots for shader.");

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Removes a shader
 * @param[in]   _pstShaderPointer Concerned ShaderPointer
 * @param[in]   _pstShader        Shader to remove
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxShaderPointer_RemoveShader(orxSHADERPOINTER *_pstShaderPointer, orxSHADER *_pstShader)
{
  orxU32    i;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstShaderPointer.u32Flags & orxSHADERPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShaderPointer);
  orxSTRUCTURE_ASSERT(_pstShader);

  /* For all slots */
  for(i = 0; i < orxSHADERPOINTER_KU32_SHADER_NUMBER; i++)
  {
    orxSHADER *pstShader;

    /* Gets Shader */
    pstShader = _pstShaderPointer->astShaderList[i].pstShader;

    /* Valid? */
    if(pstShader != orxNULL)
    {
      /* Found? */
      if(pstShader == _pstShader)
      {
        /* Decreases its reference counter */
        orxStructure_DecreaseCounter(pstShader);

        /* Removes its reference */
        _pstShaderPointer->astShaderList[i].pstShader = orxNULL;

        /* Is internal? */
        if(orxFLAG_TEST(_pstShaderPointer->astShaderList[i].u32Flags, orxSHADERPOINTER_HOLDER_KU32_FLAG_INTERNAL))
        {
          /* Deletes it */
          orxShader_Delete(pstShader);
        }

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;

        break;
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Adds a shader using its config ID
 * @param[in]   _pstShaderPointer Concerned ShaderPointer
 * @param[in]   _zShaderConfigID  Config ID of the shader to add
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxShaderPointer_AddShaderFromConfig(orxSHADERPOINTER *_pstShaderPointer, const orxSTRING _zShaderConfigID)
{
  orxU32    u32Index;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstShaderPointer.u32Flags & orxSHADERPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShaderPointer);
  orxASSERT((_zShaderConfigID != orxNULL) && (_zShaderConfigID != orxSTRING_EMPTY));

  /* Finds an empty slot */
  for(u32Index = 0; (u32Index < orxSHADERPOINTER_KU32_SHADER_NUMBER) && (_pstShaderPointer->astShaderList[u32Index].pstShader != orxNULL); u32Index++);

  /* Found? */
  if(u32Index < orxSHADERPOINTER_KU32_SHADER_NUMBER)
  {
    orxSHADER *pstShader;

    /* Creates Shader */
    pstShader = orxShader_CreateFromConfig(_zShaderConfigID);

    /* Valid? */
    if(pstShader != orxNULL)
    {
      /* Increases its reference counter */
      orxStructure_IncreaseCounter(pstShader);

      /* Adds it to holder */
      _pstShaderPointer->astShaderList[u32Index].pstShader = pstShader;

      /* Updates its flags */
      orxFLAG_SET(_pstShaderPointer->astShaderList[u32Index].u32Flags, orxSHADERPOINTER_HOLDER_KU32_FLAG_INTERNAL, orxSHADERPOINTER_HOLDER_KU32_MASK_ALL);

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Loading shader <%s> from config failed.", _zShaderConfigID);

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Failed to find an empty slot to put shader <%s> into.", _zShaderConfigID);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Removes a shader using using its config ID
 * @param[in]   _pstShaderPointer Concerned ShaderPointer
 * @param[in]   _zShaderConfigID  Config ID of the shader to remove
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxShaderPointer_RemoveShaderFromConfig(orxSHADERPOINTER *_pstShaderPointer, const orxSTRING _zShaderConfigID)
{
  orxU32    i, u32ID;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstShaderPointer.u32Flags & orxSHADERPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShaderPointer);
  orxASSERT((_zShaderConfigID != orxNULL) && (_zShaderConfigID != orxSTRING_EMPTY));

  /* Gets ID */
  u32ID = orxString_ToCRC(_zShaderConfigID); 

  /* For all slots */
  for(i = 0; i < orxSHADERPOINTER_KU32_SHADER_NUMBER; i++)
  {
    orxSHADER *pstShader;

    /* Gets Shader */
    pstShader = _pstShaderPointer->astShaderList[i].pstShader;

    /* Valid? */
    if(pstShader != orxNULL)
    {
      /* Found? */
      if(orxString_ToCRC(orxShader_GetName(pstShader)) == u32ID)
      {
        /* Decreases its reference counter */
        orxStructure_DecreaseCounter(pstShader);

        /* Removes its reference */
        _pstShaderPointer->astShaderList[i].pstShader = orxNULL;

        /* Is internal? */
        if(orxFLAG_TEST(_pstShaderPointer->astShaderList[i].u32Flags, orxSHADERPOINTER_HOLDER_KU32_FLAG_INTERNAL))
        {
          /* Deletes it */
          orxShader_Delete(pstShader);
        }

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
        break;
      }
    }
  }

  /* Done! */
  return eResult;
}
