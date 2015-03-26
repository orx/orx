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

#define orxSHADERPOINTER_KU32_MASK_ALL              0xFFFFFFFF  /**< All mask */


/** Holder flags
 */
#define orxSHADERPOINTER_HOLDER_KU32_FLAG_NONE      0x00000000  /**< No flags */

#define orxSHADERPOINTER_HOLDER_KU32_FLAG_INTERNAL  0x10000000  /**< Internal flag */

#define orxSHADERPOINTER_HOLDER_KU32_MASK_ALL       0xFFFFFFFF  /**< All mask */


/** Misc defines
 */
#define orxSHADERPOINTER_KU32_BANK_SIZE             512         /**< Bank size */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** ShaderPointer holder structure
 */
typedef struct __orxSHADERPOINTER_HOLDER_t
{
  orxSHADER  *pstShader;                                        /**< Shader reference : 4 */
  orxU32      u32Flags;                                         /**< Flags : 8 */

} orxSHADERPOINTER_HOLDER;

/** ShaderPointer structure
 */
struct __orxSHADERPOINTER_t
{
  orxSTRUCTURE            stStructure;                          /**< Public structure, first structure member : 32 */
  orxSHADERPOINTER_HOLDER astShaderList[orxSHADERPOINTER_KU32_SHADER_NUMBER]; /**< Shader list : 48 */
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
    case orxRENDER_EVENT_OBJECT_START:
    {
      const orxSHADERPOINTER *pstShaderPointer;

      /* Gets its shader pointer */
      pstShaderPointer = orxOBJECT_GET_STRUCTURE(orxOBJECT(_pstEvent->hSender), SHADERPOINTER);

      /* Found? */
      if(pstShaderPointer != orxNULL)
      {
        /* Starts it */
        orxShaderPointer_Start(pstShaderPointer);
      }

      break;
    }

    case orxRENDER_EVENT_OBJECT_STOP:
    {
      const orxSHADERPOINTER *pstShaderPointer;

      /* Gets its shader pointer */
      pstShaderPointer = orxOBJECT_GET_STRUCTURE(orxOBJECT(_pstEvent->hSender), SHADERPOINTER);

      /* Found? */
      if(pstShaderPointer != orxNULL)
      {
        /* Stops it */
        orxShaderPointer_Stop(pstShaderPointer);
      }

      break;
    }

    case orxRENDER_EVENT_VIEWPORT_STOP:
    {
      orxVIEWPORT            *pstViewport;
      const orxSHADERPOINTER *pstShaderPointer;

      /* Gets viewport */
      pstViewport = orxVIEWPORT(_pstEvent->hSender);

      /* Gets its shader pointer */
      pstShaderPointer = orxViewport_GetShaderPointer(pstViewport);

      /* Found and enabled? */
      if((pstShaderPointer != orxNULL) && (orxShaderPointer_IsEnabled(pstShaderPointer) != orxFALSE))
      {
        /* Updates blend mode */
        orxDisplay_SetBlendMode(orxViewport_GetBlendMode(pstViewport));

        /* Starts & stops it */
        orxShaderPointer_Start(pstShaderPointer);
        orxShaderPointer_Stop(pstShaderPointer);
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
      eResult = orxSTRUCTURE_REGISTER(SHADERPOINTER, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxSHADERPOINTER_KU32_BANK_SIZE, orxNULL);

      /* Initialized? */
      if(eResult != orxSTATUS_FAILURE)
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
 * @return      Created orxSHADERPOINTER / orxNULL
 */
orxSHADERPOINTER *orxFASTCALL orxShaderPointer_Create()
{
  orxSHADERPOINTER *pstResult;

  /* Checks */
  orxASSERT(sstShaderPointer.u32Flags & orxSHADERPOINTER_KU32_STATIC_FLAG_READY);

  /* Creates ShaderPointer */
  pstResult = orxSHADERPOINTER(orxStructure_Create(orxSTRUCTURE_ID_SHADERPOINTER));

  /* Created? */
  if(pstResult != orxNULL)
  {
    /* Inits flags */
    orxStructure_SetFlags(pstResult, orxSHADERPOINTER_KU32_FLAG_ENABLED, orxSHADERPOINTER_KU32_MASK_ALL);

    /* Increases counter */
    orxStructure_IncreaseCounter(pstResult);
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

  /* Decreases counter */
  orxStructure_DecreaseCounter(_pstShaderPointer);

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
          /* Removes its owner */
          orxStructure_SetOwner(_pstShaderPointer->astShaderList[i].pstShader, orxNULL);

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
    /* Referenced by others */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Starts a ShaderPointer
 * @param[in] _pstShaderPointer     Concerned ShaderPointer
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxShaderPointer_Start(const orxSHADERPOINTER *_pstShaderPointer)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstShaderPointer.u32Flags & orxSHADERPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShaderPointer);

  /* Enabled? */
  if(orxStructure_TestFlags(_pstShaderPointer, orxSHADERPOINTER_KU32_FLAG_ENABLED))
  {
    orxU32        i;
    orxSTRUCTURE *pstOwner = orxNULL;

    /* For all shaders */
    for(i = 0; i < orxSHADERPOINTER_KU32_SHADER_NUMBER; i++)
    {
      /* Valid? */
      if(_pstShaderPointer->astShaderList[i].pstShader != orxNULL)
      {
        /* No owner retrieved yet? */
        if(pstOwner == orxNULL)
        {
          /* Gets it */
          pstOwner = orxStructure_GetOwner(_pstShaderPointer);
        }

        /* Renders it */
        eResult = orxShader_Start(_pstShaderPointer->astShaderList[i].pstShader, pstOwner);
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

/** Stops a ShaderPointer
 * @param[in] _pstShaderPointer     Concerned ShaderPointer
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxShaderPointer_Stop(const orxSHADERPOINTER *_pstShaderPointer)
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
        eResult = orxShader_Stop(_pstShaderPointer->astShaderList[i].pstShader);
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

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
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
          /* Removes its owner */
          orxStructure_SetOwner(pstShader, orxNULL);

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

/** Gets a shader
 * @param[in]   _pstShaderPointer Concerned ShaderPointer
 * @param[in]   _u32Index         Index of shader to get
 * @return      orxSJADER / orxNULL
 */
const orxSHADER *orxFASTCALL orxShaderPointer_GetShader(const orxSHADERPOINTER *_pstShaderPointer, orxU32 _u32Index)
{
  const orxSHADER *pstResult;

  /* Checks */
  orxASSERT(sstShaderPointer.u32Flags & orxSHADERPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShaderPointer);
  orxASSERT(_u32Index < orxSHADERPOINTER_KU32_SHADER_NUMBER);

  /* Updates result */
  pstResult = _pstShaderPointer->astShaderList[_u32Index].pstShader;

  /* Done! */
  return pstResult;
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

      /* Updates its owner */
      orxStructure_SetOwner(pstShader, _pstShaderPointer);

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

/** Removes a shader using its config ID
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
          /* Removes its owner */
          orxStructure_SetOwner(pstShader, orxNULL);

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
