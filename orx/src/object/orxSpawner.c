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
 * @file orxSpawner.c
 * @date 06/09/2008
 * @author iarwain@orx-project.org
 *
 * @todo
 */


#include "object/orxSpawner.h"

#include "debug/orxDebug.h"
#include "core/orxConfig.h"
#include "memory/orxMemory.h"


/** Module flags
 */
#define orxSPAWNER_KU32_STATIC_FLAG_NONE          0x00000000

#define orxSPAWNER_KU32_STATIC_FLAG_READY         0x00000001

#define orxSPAWNER_KU32_STATIC_MASK_ALL           0xFFFFFFFF


/** Flags
 */
#define orxSPAWNER_KU32_FLAG_NONE                 0x00000000  /**< No flags */

#define orxSPAWNER_KU32_FLAG_ENABLED              0x10000000  /**< Enabled flag */

#define orxSPAWNER_KU32_MASK_ALL                  0xFFFFFFFF  /**< All mask */


/** Misc defines
 */
#define orxSPAWNER_KZ_CONFIG_OBJECT               "Object"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Spawner structure
 */
struct __orxSPAWNER_t
{
  orxSTRUCTURE       stStructure;                 /**< Public structure, first structure member : 16 */

  /* Padding */
  orxPAD(16)
};

/** Static structure
 */
typedef struct __orxSPAWNER_STATIC_t
{
  orxU32 u32Flags;                                /**< Control flags */

} orxSPAWNER_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
orxSTATIC orxSPAWNER_STATIC sstSpawner;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Deletes all the spawners
 */
orxSTATIC orxINLINE orxVOID orxSpawner_DeleteAll()
{
  orxSPAWNER *pstSpawner;

  /* Gets first spawner */
  pstSpawner = orxSPAWNER(orxStructure_GetFirst(orxSTRUCTURE_ID_SPAWNER));

  /* Non empty? */
  while(pstSpawner != orxNULL)
  {
    /* Deletes spawner */
    orxSpawner_Delete(pstSpawner);

    /* Gets first spawner */
    pstSpawner = orxSPAWNER(orxStructure_GetFirst(orxSTRUCTURE_ID_SPAWNER));
  }

  return;
}

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Spawner module setup
 */
orxVOID orxSpawner_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_SPAWNER, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_SPAWNER, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_SPAWNER, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_SPAWNER, orxMODULE_ID_OBJECT);
  orxModule_AddDependency(orxMODULE_ID_SPAWNER, orxMODULE_ID_CONFIG);

  return;
}

/** Inits the spawner module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxSpawner_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!orxFLAG_TEST(sstSpawner.u32Flags, orxSPAWNER_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstSpawner, sizeof(orxSPAWNER_STATIC));

    /* Registers structure type */
    eResult = orxSTRUCTURE_REGISTER(SPAWNER, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxNULL);

    /* Initialized? */
    if(eResult == orxSTATUS_SUCCESS)
    {
      /* Inits Flags */
      orxFLAG_SET(sstSpawner.u32Flags, orxSPAWNER_KU32_STATIC_FLAG_READY, orxSPAWNER_KU32_STATIC_MASK_ALL);
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

/** Exits from the spawner module
 */
orxVOID orxSpawner_Exit()
{
  /* Initialized? */
  if(orxFLAG_TEST(sstSpawner.u32Flags, orxSPAWNER_KU32_STATIC_FLAG_READY))
  {
    /* Deletes spawner list */
    orxSpawner_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_SPAWNER);

    /* Updates flags */
    orxFLAG_SET(sstSpawner.u32Flags, orxSPAWNER_KU32_FLAG_NONE, orxSPAWNER_KU32_STATIC_FLAG_READY);
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}

/** Creates an empty spawner
 * @return      Created orxSPAWNER / orxNULL
 */
orxSPAWNER *orxSpawner_Create()
{
  orxSPAWNER *pstSpawner;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);

  /* Creates spawner */
  pstSpawner = orxSPAWNER(orxStructure_Create(orxSTRUCTURE_ID_SPAWNER));

  /* Created? */
  if(pstSpawner != orxNULL)
  {
    /* Inits flags */
    orxStructure_SetFlags(pstSpawner, orxSPAWNER_KU32_FLAG_ENABLED, orxSPAWNER_KU32_MASK_ALL);
  }
  else
  {
    /* !!! MSG !!! */
  }

  return pstSpawner;
}

/** Deletes a spawner
 * @param[in] _pstSpawner        Concerned spawner
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSpawner_Delete(orxSPAWNER *_pstSpawner)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);

  /* Not referenced? */
  if(orxStructure_GetRefCounter(_pstSpawner) == 0)
  {
    /* Deletes structure */
    orxStructure_Delete(_pstSpawner);
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

/** Creates a spawner from config
 * @param[in]   _zConfigID            Config ID
 * @ return orxSPAWNER / orxNULL
 */
orxSPAWNER *orxFASTCALL orxSpawner_CreateFromConfig(orxCONST orxSTRING _zConfigID)
{
  orxSPAWNER  *pstResult;
  orxSTRING   zPreviousSection;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxASSERT((_zConfigID != orxNULL) && (*_zConfigID != *orxSTRING_EMPTY));

  /* Gets previous config section */
  zPreviousSection = orxConfig_GetCurrentSection();

  /* Selects section */
  if((orxConfig_HasSection(_zConfigID) != orxFALSE)
  && (orxConfig_SelectSection(_zConfigID) != orxSTATUS_FAILURE))
  {
    /* Creates spawner */
    pstResult = orxSpawner_Create();

    /* Valid? */
    if(pstResult != orxNULL)
    {
      //! TODO
    }

    /* Restores previous section */
    orxConfig_SelectSection(zPreviousSection);
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
