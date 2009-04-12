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
 * @file orxShader.c
 * @date 11/04/2009
 * @author iarwain@orx-project.org
 *
 * @todo
 */


#include "render/orxShader.h"

#include "debug/orxDebug.h"
#include "memory/orxMemory.h"
#include "core/orxConfig.h"
#include "object/orxStructure.h"
#include "utils/orxHashTable.h"
#include "utils/orxString.h"


/** Module flags
 */
#define orxSHADER_KU32_STATIC_FLAG_NONE     0x00000000

#define orxSHADER_KU32_STATIC_FLAG_READY    0x00000001

#define orxSHADER_KU32_STATIC_MASK_ALL      0xFFFFFFFF


/** Flags
 */
#define orxSHADER_KU32_FLAG_NONE            0x00000000  /**< No flags */

#define orxSHADER_KU32_FLAG_ENABLED         0x10000000  /**< Enabled flag */

#define orxSHADER_KU32_MASK_ALL             0xFFFFFFFF  /**< All mask */

/** Misc defines
 */
#define orxSHADER_KU32_REFERENCE_TABLE_SIZE 16

#define orxSHADER_KZ_CONFIG_CODE            "Code"
#define orxSHADER_KZ_CONFIG_KEEP_IN_CACHE   "KeepInCache"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Shader structure
 */
struct __orxSHADER_t
{
  orxSTRUCTURE  stStructure;                            /**< Public structure, first structure member : 16 */
  orxSTRING     zReference;                             /**< shader reference : 20 */

  /* Padding */
  orxPAD(20)
};

/** Static structure
 */
typedef struct __orxSHADER_STATIC_t
{
  orxU32        u32Flags;                               /**< Control flags */
  orxHASHTABLE *pstReferenceTable;                      /**< Reference hash table */

} orxSHADER_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxSHADER_STATIC sstShader;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Deletes all the shaders
 */
static orxINLINE void orxShader_DeleteAll()
{
  orxSHADER *pstShader;

  /* Gets first shader */
  pstShader = orxSHADER(orxStructure_GetFirst(orxSTRUCTURE_ID_SHADER));

  /* Non empty? */
  while(pstShader != orxNULL)
  {
    /* Deletes it */
    orxShader_Delete(pstShader);

    /* Gets first shader */
    pstShader = orxSHADER(orxStructure_GetFirst(orxSTRUCTURE_ID_SHADER));
  }

  return;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Shader module setup
 */
void orxShader_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_SHADER, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_SHADER, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_SHADER, orxMODULE_ID_CONFIG);

  return;
}

/** Inits the shader module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxShader_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstShader, sizeof(orxSHADER_STATIC));

    /* Creates reference table */
    sstShader.pstReferenceTable = orxHashTable_Create(orxSHADER_KU32_REFERENCE_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if(sstShader.pstReferenceTable != orxNULL)
    {
      /* Registers structure type */
      eResult = orxSTRUCTURE_REGISTER(SHADER, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxNULL);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Failed to create shader hashtable storage.");
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Tried to initialize the shader module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Initialized? */
  if(eResult == orxSTATUS_SUCCESS)
  {
    /* Inits Flags */
    orxFLAG_SET(sstShader.u32Flags, orxSHADER_KU32_STATIC_FLAG_READY, orxSHADER_KU32_STATIC_FLAG_NONE);
  }

  /* Done! */
  return eResult;
}

/** Exits from the shader module
 */
void orxShader_Exit()
{
  /* Initialized? */
  if(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY)
  {
    /* Deletes shader list */
    orxShader_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_SHADER);

    /* Deletes reference table */
    orxHashTable_Delete(sstShader.pstReferenceTable);

    /* Updates flags */
    sstShader.u32Flags &= ~orxSHADER_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Tried to exit from the shader module when it wasn't initialized.");
  }

  return;
}

/** Creates an empty shader
 * @return orxSHADER / orxNULL
 */
orxSHADER *orxShader_Create()
{
  orxSHADER *pstResult;

  /* Checks */
  orxASSERT(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY);

  /* Creates shader */
  pstResult = orxSHADER(orxStructure_Create(orxSTRUCTURE_ID_SHADER));

  /* Created? */
  if(pstResult != orxNULL)
  {
    /* Inits flags */
    orxStructure_SetFlags(pstResult, orxSHADER_KU32_FLAG_ENABLED, orxSHADER_KU32_MASK_ALL);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Failed to create shader structure.");
  }

  /* Done! */
  return pstResult;
}

/** Creates a shader from config
 * @param[in]   _zConfigID            Config ID
 * @ return orxSHADER / orxNULL
 */
orxSHADER *orxFASTCALL orxShader_CreateFromConfig(const orxSTRING _zConfigID)
{
  orxU32  u32ID;
  orxSHADER  *pstResult;

  /* Checks */
  orxASSERT(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY);
  orxASSERT((_zConfigID != orxNULL) && (_zConfigID != orxSTRING_EMPTY));

  /* Gets shader ID */
  u32ID = orxString_ToCRC(_zConfigID);

  /* Search for reference */
  pstResult = orxHashTable_Get(sstShader.pstReferenceTable, u32ID);

  /* Not already created? */
  if(pstResult == orxNULL)
  {
    orxSTRING zPreviousSection;

    /* Gets previous config section */
    zPreviousSection = orxConfig_GetCurrentSection();

    /* Selects section */
    if((orxConfig_HasSection(_zConfigID) != orxFALSE)
    && (orxConfig_SelectSection(_zConfigID) != orxSTATUS_FAILURE))
    {
      /* Creates shader */
      pstResult = orxShader_Create();

      /* Valid? */
      if(pstResult != orxNULL)
      {
        /* Stores its reference */
        pstResult->zReference = orxConfig_GetCurrentSection();

        /* Adds it to reference table */
        if(orxHashTable_Add(sstShader.pstReferenceTable, u32ID, pstResult) != orxSTATUS_FAILURE)
        {
          //! TODO: Inits shader values

          /* Should keep it in cache? */
          if(orxConfig_GetBool(orxSHADER_KZ_CONFIG_KEEP_IN_CACHE) != orxFALSE)
          {
            /* Increases its reference counter to keep it in cache table */
            orxStructure_IncreaseCounter(pstResult);
          }
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Failed to add hash table.");

          /* Deletes it */
          orxShader_Delete(pstResult);

          /* Updates result */
          pstResult = orxNULL;
        }
      }

      /* Restores previous section */
      orxConfig_SelectSection(zPreviousSection);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Couldn't create shader because config section (%s) couldn't be found.", _zConfigID);

      /* Updates result */
      pstResult = orxNULL;
    }
  }
  else
  {
    /* Updates reference counter */
    orxStructure_IncreaseCounter(pstResult);
  }

  /* Done! */
  return pstResult;
}

/** Deletes a shader
 * @param[in] _pstShader              Concerned Shader
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxShader_Delete(orxSHADER *_pstShader)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShader);

  /* Has an ID? */
  if((_pstShader->zReference != orxNULL)
  && (_pstShader->zReference != orxSTRING_EMPTY))
  {
    /* Not referenced? */
    if(orxStructure_GetRefCounter(_pstShader) == 0)
    {
      /* Removes from hashtable */
      orxHashTable_Remove(sstShader.pstReferenceTable, orxString_ToCRC(_pstShader->zReference));

      /* Deletes structure */
      orxStructure_Delete(_pstShader);
    }
    else
    {
      /* Decreases its reference counter */
      orxStructure_DecreaseCounter(_pstShader);
    }
  }
  else
  {
    /* Not referenced? */
    if(orxStructure_GetRefCounter(_pstShader) == 0)
    {
      /* Deletes structure */
      orxStructure_Delete(_pstShader);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Cannot delete shader while it is still being referenced.");

      /* Referenced by others */
      eResult = orxSTATUS_FAILURE;
    }
  }

  /* Done! */
  return eResult;
}

/** Enables/disables a shader
 * @param[in]   _pstShader            Concerned Shader
 * @param[in]   _bEnable              Enable / disable
 */
void orxFASTCALL    orxShader_Enable(orxSHADER *_pstShader, orxBOOL _bEnable)
{
  /* Checks */
  orxASSERT(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShader);

  /* Enable? */
  if(_bEnable != orxFALSE)
  {
    /* Updates status flags */
    orxStructure_SetFlags(_pstShader, orxSHADER_KU32_FLAG_ENABLED, orxSHADER_KU32_FLAG_NONE);
  }
  else
  {
    /* Updates status flags */
    orxStructure_SetFlags(_pstShader, orxSHADER_KU32_FLAG_NONE, orxSHADER_KU32_FLAG_ENABLED);
  }

  return;
}

/** Is shader enabled?
 * @param[in]   _pstShader            Concerned Shader
 * @return      orxTRUE if enabled, orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxShader_IsEnabled(const orxSHADER *_pstShader)
{
  /* Checks */
  orxASSERT(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShader);

  /* Done! */
  return(orxStructure_TestFlags(_pstShader, orxSHADER_KU32_FLAG_ENABLED));
}

/** Gets shader name
 * @param[in]   _pstShader            Concerned Shader
 * @return      orxSTRING / orxSTRING_EMPTY
 */
const orxSTRING orxFASTCALL orxShader_GetName(const orxSHADER *_pstShader)
{
  orxSTRING zResult;

  /* Checks */
  orxASSERT(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShader);

  /* Has reference? */
  if(_pstShader->zReference != orxNULL)
  {
    /* Updates result */
    zResult = _pstShader->zReference;
  }
  else
  {
    /* Updates result */
    zResult = orxSTRING_EMPTY;
  }

  /* Done! */
  return zResult;
}
