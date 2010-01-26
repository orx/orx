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
 * @file orxStructure.h
 * @date 08/12/2003
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxStructure
 *
 * Structure module
 * Allows to creates and handle structures.
 * Structures can be referenced by other structures (or objects).
 *
 * @{
 */


#ifndef _orxSTRUCTURE_H_
#define _orxSTRUCTURE_H_

#include "orxInclude.h"

#include "core/orxClock.h"
#include "debug/orxDebug.h"
#include "memory/orxMemory.h"


/** Structure pointer get helpers
 */
#define orxSTRUCTURE_GET_POINTER(STRUCTURE, TYPE) ((orx##TYPE *)_orxStructure_GetPointer(STRUCTURE, orxSTRUCTURE_ID_##TYPE))

#define orxSTRUCTURE(STRUCTURE)     (((STRUCTURE != orxNULL) && ((((orxSTRUCTURE *)STRUCTURE)->eID ^ orxSTRUCTURE_MAGIC_TAG_ACTIVE) < orxSTRUCTURE_ID_NUMBER)) ? (orxSTRUCTURE *)STRUCTURE : (orxSTRUCTURE *)orxNULL)

#define orxANIM(STRUCTURE)          orxSTRUCTURE_GET_POINTER(STRUCTURE, ANIM)
#define orxANIMPOINTER(STRUCTURE)   orxSTRUCTURE_GET_POINTER(STRUCTURE, ANIMPOINTER)
#define orxANIMSET(STRUCTURE)       orxSTRUCTURE_GET_POINTER(STRUCTURE, ANIMSET)
#define orxBODY(STRUCTURE)          orxSTRUCTURE_GET_POINTER(STRUCTURE, BODY)
#define orxCAMERA(STRUCTURE)        orxSTRUCTURE_GET_POINTER(STRUCTURE, CAMERA)
#define orxCLOCK(STRUCTURE)         orxSTRUCTURE_GET_POINTER(STRUCTURE, CLOCK)
#define orxFRAME(STRUCTURE)         orxSTRUCTURE_GET_POINTER(STRUCTURE, FRAME)
#define orxFX(STRUCTURE)            orxSTRUCTURE_GET_POINTER(STRUCTURE, FX)
#define orxFXPOINTER(STRUCTURE)     orxSTRUCTURE_GET_POINTER(STRUCTURE, FXPOINTER)
#define orxGRAPHIC(STRUCTURE)       orxSTRUCTURE_GET_POINTER(STRUCTURE, GRAPHIC)
#define orxOBJECT(STRUCTURE)        orxSTRUCTURE_GET_POINTER(STRUCTURE, OBJECT)
#define orxSOUND(STRUCTURE)         orxSTRUCTURE_GET_POINTER(STRUCTURE, SOUND)
#define orxSOUNDPOINTER(STRUCTURE)  orxSTRUCTURE_GET_POINTER(STRUCTURE, SOUNDPOINTER)
#define orxSHADER(STRUCTURE)        orxSTRUCTURE_GET_POINTER(STRUCTURE, SHADER)
#define orxSHADERPOINTER(STRUCTURE) orxSTRUCTURE_GET_POINTER(STRUCTURE, SHADERPOINTER)
#define orxSPAWNER(STRUCTURE)       orxSTRUCTURE_GET_POINTER(STRUCTURE, SPAWNER)
#define orxTEXT(STRUCTURE)          orxSTRUCTURE_GET_POINTER(STRUCTURE, TEXT)
#define orxTEXTURE(STRUCTURE)       orxSTRUCTURE_GET_POINTER(STRUCTURE, TEXTURE)
#define orxVIEWPORT(STRUCTURE)      orxSTRUCTURE_GET_POINTER(STRUCTURE, VIEWPORT)

/** Structure register macro
 */
#define orxSTRUCTURE_REGISTER(TYPE, STORAGE_TYPE, MEMORY_TYPE, UPDATE_FUNCTION) \
  orxStructure_Register(orxSTRUCTURE_ID_##TYPE, STORAGE_TYPE, MEMORY_TYPE, sizeof(orx##TYPE), UPDATE_FUNCTION)

/** Structure assert
 */
#define orxSTRUCTURE_ASSERT(STRUCTURE)                          \
  orxASSERT((STRUCTURE) != orxNULL);                            \
  orxASSERT((((orxSTRUCTURE *)(STRUCTURE))->eID ^ orxSTRUCTURE_MAGIC_TAG_ACTIVE) < orxSTRUCTURE_ID_NUMBER);

/** Structure magic number
 */
#ifdef __orxDEBUG__
  #define orxSTRUCTURE_MAGIC_TAG_ACTIVE   0xDEFACED0
#else
  #define orxSTRUCTURE_MAGIC_TAG_ACTIVE   0x00000000
#endif

#define orxSTRUCTURE_MAGIC_TAG_DELETED    0xDEADC0DE


/** Structure IDs
 */
typedef enum __orxSTRUCTURE_ID_t
{
  /* *** Following structures can be linked to objects *** */

  orxSTRUCTURE_ID_ANIMPOINTER = 0,
  orxSTRUCTURE_ID_BODY,
  orxSTRUCTURE_ID_CLOCK,
  orxSTRUCTURE_ID_FRAME,
  orxSTRUCTURE_ID_FXPOINTER,
  orxSTRUCTURE_ID_GRAPHIC,
  orxSTRUCTURE_ID_SHADERPOINTER,
  orxSTRUCTURE_ID_SOUNDPOINTER,
  orxSTRUCTURE_ID_SPAWNER,

  orxSTRUCTURE_ID_LINKABLE_NUMBER,

  /* *** Below this point, structures can not be linked to objects *** */

  orxSTRUCTURE_ID_ANIM = orxSTRUCTURE_ID_LINKABLE_NUMBER,
  orxSTRUCTURE_ID_ANIMSET,
  orxSTRUCTURE_ID_CAMERA,
  orxSTRUCTURE_ID_FX,
  orxSTRUCTURE_ID_OBJECT,
  orxSTRUCTURE_ID_SHADER,
  orxSTRUCTURE_ID_SOUND,
  orxSTRUCTURE_ID_TEXT,
  orxSTRUCTURE_ID_TEXTURE,
  orxSTRUCTURE_ID_VIEWPORT,

  orxSTRUCTURE_ID_NUMBER,

  orxSTRUCTURE_ID_NONE = orxENUM_NONE

} orxSTRUCTURE_ID;

/** Structure storage types
 */
typedef enum __orxSTRUCTURE_STORAGE_TYPE_t
{
  orxSTRUCTURE_STORAGE_TYPE_LINKLIST = 0,
  orxSTRUCTURE_STORAGE_TYPE_TREE,

  orxSTRUCTURE_STORAGE_TYPE_NUMBER,

  orxSTRUCTURE_STORAGE_TYPE_NONE = orxENUM_NONE,

} orxSTRUCTURE_STORAGE_TYPE;

/** Public structure (Must be first derived structure member!)
 */
typedef struct __orxSTRUCTURE_t
{
  orxSTRUCTURE_ID eID;            /**< Structure ID : 4 */
  orxU32          u32RefCounter;  /**< Reference counter : 8 */
  orxU32          u32Flags;       /**< Flags : 12 */
  orxHANDLE       hStorageNode;   /**< Internal storage node handle : 16 */

} orxSTRUCTURE;


/** Structure update callback function type
 */
typedef orxSTATUS (orxFASTCALL *orxSTRUCTURE_UPDATE_FUNCTION)(orxSTRUCTURE *_pstStructure, const orxSTRUCTURE *_pstCaller, const orxCLOCK_INFO *_pstClockInfo);


/** Gets structure pointer / debug mode
 * @param[in]   _pStructure    Concerned structure
 * @param[in]   _eStructureID   ID to test the structure against
 * @return      Valid orxSTRUCTURE, orxNULL otherwise
 */
static orxINLINE orxSTRUCTURE *_orxStructure_GetPointer(const void *_pStructure, orxSTRUCTURE_ID _eStructureID)
{
  orxSTRUCTURE *pstResult;

  /* Updates result */
  pstResult = ((_pStructure != orxNULL) && (((orxSTRUCTURE *)_pStructure)->eID ^ orxSTRUCTURE_MAGIC_TAG_ACTIVE) == _eStructureID) ? (orxSTRUCTURE *)_pStructure : (orxSTRUCTURE *)orxNULL;

  /* Done! */
  return pstResult;
}


/** Structure module setup
 */
extern orxDLLAPI void orxFASTCALL                       orxStructure_Setup();

/** Initializess the structure module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                  orxStructure_Init();

/** Exits from the structure module
 */
extern orxDLLAPI void orxFASTCALL                       orxStructure_Exit();


/** Registers a given ID
 * @param[in]   _eStructureID   Concerned structure ID
 * @param[in]   _eStorageType   Storage type to use for this structure type
 * @param[in]   _eMemoryType    Memory type to store this structure type
 * @param[in]   _u32Size        Structure size
 * @param[in]   _pfnUpdate      Structure update function
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS  orxFASTCALL                 orxStructure_Register(orxSTRUCTURE_ID _eStructureID, orxSTRUCTURE_STORAGE_TYPE _eStorageType, orxMEMORY_TYPE _eMemoryType, orxU32 _u32Size, const orxSTRUCTURE_UPDATE_FUNCTION _pfnUpdate);

/** Unregisters a given ID
 * @param[in]   _eStructureID   Concerned structure ID
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                  orxStructure_Unregister(orxSTRUCTURE_ID _eStructureID);


/** Creates a clean structure for given type
 * @param[in]   _eStructureID   Concerned structure ID
 * @return      orxSTRUCTURE / orxNULL
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL              orxStructure_Create(orxSTRUCTURE_ID _eStructureID);

/** Deletes a structure (needs to be cleaned beforehand)
 * @param[in]   _pStructure    Concerned structure
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                  orxStructure_Delete(void *_pStructure);


/** Gets structure storage type
 * @param[in]   _eStructureID   Concerned structure ID
 * @return      orxSTRUCTURE_STORAGE_TYPE
 */
extern orxDLLAPI orxSTRUCTURE_STORAGE_TYPE orxFASTCALL  orxStructure_GetStorageType(orxSTRUCTURE_ID _eStructureID);

/** Gets given type structure counter
 * @param[in]   _eStructureID   Concerned structure ID
 * @return      orxU32 / orxU32_UNDEFINED
 */
extern orxDLLAPI orxU32     orxFASTCALL                 orxStructure_GetCounter(orxSTRUCTURE_ID _eStructureID);

/** Updates structure if update function was registered for the structure type
 * @param[in]   _pStructure    Concerned structure
 * @param[in]   _phCaller       Caller structure
 * @param[in]   _pstClockInfo   Update associated clock info
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS  orxFASTCALL                 orxStructure_Update(void *_pStructure, const void *_phCaller, const orxCLOCK_INFO *_pstClockInfo);


/** *** Structure storage accessors *** */


/** Gets first stored structure (first list cell or tree root depending on storage type)
 * @param[in]   _eStructureID   Concerned structure ID
 * @return      orxSTRUCTURE
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL              orxStructure_GetFirst(orxSTRUCTURE_ID _eStructureID);

/** Gets last stored structure (last list cell or tree root depending on storage type)
 * @param[in]   _eStructureID   Concerned structure ID
 * @return      orxSTRUCTURE
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL              orxStructure_GetLast(orxSTRUCTURE_ID _eStructureID);

/** Gets structure tree parent
 * @param[in]   _pStructure    Concerned structure
 * @return      orxSTRUCTURE
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL              orxStructure_GetParent(const void *_pStructure);

/** Gets structure tree child
 * @param[in]   _pStructure    Concerned structure
 * @return      orxSTRUCTURE
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL              orxStructure_GetChild(const void *_pStructure);

/** Gets structure tree sibling
 * @param[in]   _pStructure    Concerned structure
 * @return      orxSTRUCTURE
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL              orxStructure_GetSibling(const void *_pStructure);

/** Gets structure list previous
 * @param[in]   _pStructure    Concerned structure
 * @return      orxSTRUCTURE
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL              orxStructure_GetPrevious(const void *_pStructure);

/** Gets structure list next
 * @param[in]   _pStructure    Concerned structure
 * @return      orxSTRUCTURE
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL              orxStructure_GetNext(const void *_pStructure);

/** Sets structure tree parent
 * @param[in]   _pStructure    Concerned structure
 * @param[in]   _phParent       Structure to set as parent
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS  orxFASTCALL                 orxStructure_SetParent(void *_pStructure, void *_phParent);


/** *** Inlined structure accessors *** */


/** Increases structure reference counter
 * @param[in]   _pStructure    Concerned structure
 */
static orxINLINE void                                   orxStructure_IncreaseCounter(void *_pStructure)
{
  /* Checks */
  orxSTRUCTURE_ASSERT(_pStructure);

  /* Increases it */
  (orxSTRUCTURE(_pStructure))->u32RefCounter++;

  return;
}

/** Decreases structure reference counter
 * @param[in]   _pStructure    Concerned structure
 */
static orxINLINE void                                   orxStructure_DecreaseCounter(void *_pStructure)
{
  /* Checks */
  orxSTRUCTURE_ASSERT(_pStructure);
  orxASSERT(orxSTRUCTURE(_pStructure)->u32RefCounter > 0);

  /* Decreases it */
  orxSTRUCTURE(_pStructure)->u32RefCounter--;

  return;
}

/** Gets structure reference counter
 * @param[in]   _pStructure    Concerned structure
 * @return      orxU32
 */
static orxINLINE orxU32                                 orxStructure_GetRefCounter(const void *_pStructure)
{
  /* Checks */
  orxSTRUCTURE_ASSERT(_pStructure);

  /* Returns it */
  return(orxSTRUCTURE(_pStructure)->u32RefCounter);
}

/** Gets structure ID
 * @param[in]   _pStructure    Concerned structure
 * @return      orxSTRUCTURE_ID
 */
static orxINLINE orxSTRUCTURE_ID                        orxStructure_GetID(const void *_pStructure)
{
  /* Checks */
  orxSTRUCTURE_ASSERT(_pStructure);

  /* Returns it */
  return((orxSTRUCTURE_ID)(orxSTRUCTURE(_pStructure)->eID ^ orxSTRUCTURE_MAGIC_TAG_ACTIVE));
}

/** Tests flags against structure ones
 * @param[in]   _pStructure    Concerned structure
 * @param[in]   _u32Flags       Flags to test
 * @return      orxTRUE / orxFALSE
 */
static orxINLINE orxBOOL                                orxStructure_TestFlags(const void *_pStructure, orxU32 _u32Flags)
{
  /* Checks */
  orxSTRUCTURE_ASSERT(_pStructure);

  /* Done! */
  return(orxFLAG_TEST(orxSTRUCTURE(_pStructure)->u32Flags, _u32Flags));
}

/** Tests all flags against structure ones
 * @param[in]   _pStructure    Concerned structure
 * @param[in]   _u32Flags       Flags to test
 * @return      orxTRUE / orxFALSE
 */
static orxINLINE orxBOOL                                orxStructure_TestAllFlags(const void *_pStructure, orxU32 _u32Flags)
{
  /* Checks */
  orxSTRUCTURE_ASSERT(_pStructure);

  /* Done! */
  return(orxFLAG_TEST_ALL(orxSTRUCTURE(_pStructure)->u32Flags, _u32Flags));
}

/** Gets structure flags
 * @param[in]   _pStructure    Concerned structure
 * @param[in]   _u32Mask       Mask to use for getting flags
 * @return      orxU32
 */
static orxINLINE orxU32                                 orxStructure_GetFlags(const void *_pStructure, orxU32 _u32Mask)
{
  /* Checks */
  orxSTRUCTURE_ASSERT(_pStructure);

  /* Done! */
  return(orxFLAG_GET(orxSTRUCTURE(_pStructure)->u32Flags, _u32Mask));
}

/** Sets structure flags
 * @param[in]   _pStructure    Concerned structure
 * @param[in]   _u32AddFlags    Flags to add
 * @param[in]   _u32RemoveFlags Flags to remove
 */
static orxINLINE void                                   orxStructure_SetFlags(void *_pStructure, orxU32 _u32AddFlags, orxU32 _u32RemoveFlags)
{
  /* Checks */
  orxSTRUCTURE_ASSERT(_pStructure);

  orxFLAG_SET(orxSTRUCTURE(_pStructure)->u32Flags, _u32AddFlags, _u32RemoveFlags);

  return;
}

#endif /* _orxSTRUCTURE_H_ */

/** @} */
