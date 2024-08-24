/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008- Orx-Project
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
 * Allows to create and handle structures.
 * Structures can be referenced by other structures.
 *
 * @{
 */


#ifndef _orxSTRUCTURE_H_
#define _orxSTRUCTURE_H_

#include "orxInclude.h"

#include "core/orxClock.h"
#include "memory/orxMemory.h"
#include "utils/orxLinkList.h"
#include "utils/orxTree.h"


/** Structure pointer get helpers
 */
#define orxSTRUCTURE_GET_POINTER(STRUCTURE, TYPE) ((orx##TYPE *)_orxStructure_GetPointer(STRUCTURE, orxSTRUCTURE_ID_##TYPE))

#define orxSTRUCTURE(STRUCTURE)     ((((STRUCTURE) != orxNULL) && (((((orxSTRUCTURE *)STRUCTURE)->u64GUID & orxSTRUCTURE_GUID_MASK_STRUCTURE_ID) >> orxSTRUCTURE_GUID_SHIFT_STRUCTURE_ID) < orxSTRUCTURE_ID_NUMBER)) ? (orxSTRUCTURE *)(STRUCTURE) : (orxSTRUCTURE *)orxNULL)

#define orxANIM(STRUCTURE)          orxSTRUCTURE_GET_POINTER(STRUCTURE, ANIM)
#define orxANIMPOINTER(STRUCTURE)   orxSTRUCTURE_GET_POINTER(STRUCTURE, ANIMPOINTER)
#define orxANIMSET(STRUCTURE)       orxSTRUCTURE_GET_POINTER(STRUCTURE, ANIMSET)
#define orxBODY(STRUCTURE)          orxSTRUCTURE_GET_POINTER(STRUCTURE, BODY)
#define orxCAMERA(STRUCTURE)        orxSTRUCTURE_GET_POINTER(STRUCTURE, CAMERA)
#define orxCLOCK(STRUCTURE)         orxSTRUCTURE_GET_POINTER(STRUCTURE, CLOCK)
#define orxFONT(STRUCTURE)          orxSTRUCTURE_GET_POINTER(STRUCTURE, FONT)
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
#define orxTIMELINE(STRUCTURE)      orxSTRUCTURE_GET_POINTER(STRUCTURE, TIMELINE)
#define orxTRIGGER(STRUCTURE)       orxSTRUCTURE_GET_POINTER(STRUCTURE, TRIGGER)
#define orxVIEWPORT(STRUCTURE)      orxSTRUCTURE_GET_POINTER(STRUCTURE, VIEWPORT)

/** Structure register macro
 */
#define orxSTRUCTURE_REGISTER(TYPE, STORAGE_TYPE, MEMORY_TYPE, BANK_SIZE, UPDATE_FUNCTION) \
  orxStructure_Register(orxSTRUCTURE_ID_##TYPE, STORAGE_TYPE, MEMORY_TYPE, sizeof(orx##TYPE), BANK_SIZE, UPDATE_FUNCTION)

/** Structure assert
 */
#define orxSTRUCTURE_ASSERT(STRUCTURE) \
  orxASSERT((STRUCTURE != orxNULL) && (((((orxSTRUCTURE *)(STRUCTURE))->u64GUID & orxSTRUCTURE_GUID_MASK_STRUCTURE_ID) >> orxSTRUCTURE_GUID_SHIFT_STRUCTURE_ID) < orxSTRUCTURE_ID_NUMBER));

/** Structure magic number
 */
#define orxSTRUCTURE_GUID_MAGIC_TAG_DELETED   0xDEFACED0DEADC0DEULL

/** Structure GUID masks/shifts
 */
#define orxSTRUCTURE_GUID_MASK_STRUCTURE_ID   0x00000000000000FFULL
#define orxSTRUCTURE_GUID_SHIFT_STRUCTURE_ID  0

#define orxSTRUCTURE_GUID_MASK_ITEM_ID        0x00000000FFFFFF00ULL
#define orxSTRUCTURE_GUID_SHIFT_ITEM_ID       8

#define orxSTRUCTURE_GUID_MASK_INSTANCE_ID    0xFFFFFFFF00000000ULL
#define orxSTRUCTURE_GUID_SHIFT_INSTANCE_ID   32


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
  orxSTRUCTURE_ID_TIMELINE,
  orxSTRUCTURE_ID_TRIGGER,

  orxSTRUCTURE_ID_LINKABLE_NUMBER,

  /* *** Below this point, structures can not be linked to objects *** */

  orxSTRUCTURE_ID_ANIM = orxSTRUCTURE_ID_LINKABLE_NUMBER,
  orxSTRUCTURE_ID_ANIMSET,
  orxSTRUCTURE_ID_CAMERA,
  orxSTRUCTURE_ID_FONT,
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
  orxU64              u64GUID;        /**< Structure GUID : 8/8 */
  orxU64              u64OwnerGUID;   /**< Owner's GUID : 16/16 */
  orxU32              u32Flags;       /**< Flags : 20/20 */
  orxU32              u32RefCount;    /**< Ref count : 24/24 */

  union
  {
    orxLINKLIST_NODE  stLinkListNode; /**< Linklist node : 36/48 */
    orxTREE_NODE      stTreeNode;     /**< Tree node : 44/64 */
  } stStorage;                        /**< Storage node union : 48/64 */

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
  pstResult = ((_pStructure != orxNULL) && ((((orxSTRUCTURE *)_pStructure)->u64GUID & orxSTRUCTURE_GUID_MASK_STRUCTURE_ID) >> orxSTRUCTURE_GUID_SHIFT_STRUCTURE_ID) == _eStructureID) ? (orxSTRUCTURE *)_pStructure : (orxSTRUCTURE *)orxNULL;

  /* Done! */
  return pstResult;
}

/** Gets structure ID string
 * @param[in]   _eID                       Concerned ID
 * @return      Corresponding literal string
 */
static orxINLINE const orxSTRING orxStructure_GetIDString(orxSTRUCTURE_ID _eID)
{
  const orxSTRING zResult;

#define orxSTRUCTURE_DECLARE_ID_ENTRY(ID)    case orxSTRUCTURE_ID_##ID: zResult = #ID; break

  /* Depending on ID */
  switch(_eID)
  {
    orxSTRUCTURE_DECLARE_ID_ENTRY(ANIMPOINTER);
    orxSTRUCTURE_DECLARE_ID_ENTRY(BODY);
    orxSTRUCTURE_DECLARE_ID_ENTRY(CLOCK);
    orxSTRUCTURE_DECLARE_ID_ENTRY(FRAME);
    orxSTRUCTURE_DECLARE_ID_ENTRY(FXPOINTER);
    orxSTRUCTURE_DECLARE_ID_ENTRY(GRAPHIC);
    orxSTRUCTURE_DECLARE_ID_ENTRY(SHADERPOINTER);
    orxSTRUCTURE_DECLARE_ID_ENTRY(SOUNDPOINTER);
    orxSTRUCTURE_DECLARE_ID_ENTRY(SPAWNER);
    orxSTRUCTURE_DECLARE_ID_ENTRY(TIMELINE);
    orxSTRUCTURE_DECLARE_ID_ENTRY(TRIGGER);
    orxSTRUCTURE_DECLARE_ID_ENTRY(ANIM);
    orxSTRUCTURE_DECLARE_ID_ENTRY(ANIMSET);
    orxSTRUCTURE_DECLARE_ID_ENTRY(CAMERA);
    orxSTRUCTURE_DECLARE_ID_ENTRY(FONT);
    orxSTRUCTURE_DECLARE_ID_ENTRY(FX);
    orxSTRUCTURE_DECLARE_ID_ENTRY(OBJECT);
    orxSTRUCTURE_DECLARE_ID_ENTRY(SHADER);
    orxSTRUCTURE_DECLARE_ID_ENTRY(SOUND);
    orxSTRUCTURE_DECLARE_ID_ENTRY(TEXT);
    orxSTRUCTURE_DECLARE_ID_ENTRY(TEXTURE);
    orxSTRUCTURE_DECLARE_ID_ENTRY(VIEWPORT);

    default: zResult = "INVALID STRUCTURE ID"; break;
  }

#undef orxSTRUCTURE_DECLARE_ID_ENTRY

  /* Done! */
  return zResult;
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
 * @param[in]   _u32BankSize    Bank (segment) size
 * @param[in]   _pfnUpdate      Structure update function
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                  orxStructure_Register(orxSTRUCTURE_ID _eStructureID, orxSTRUCTURE_STORAGE_TYPE _eStorageType, orxMEMORY_TYPE _eMemoryType, orxU32 _u32Size, orxU32 _u32BankSize, const orxSTRUCTURE_UPDATE_FUNCTION _pfnUpdate);

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

/** Gets given type structure count
 * @param[in]   _eStructureID   Concerned structure ID
 * @return      orxU32 / orxU32_UNDEFINED
 */
extern orxDLLAPI orxU32 orxFASTCALL                     orxStructure_GetCount(orxSTRUCTURE_ID _eStructureID);

/** Updates structure if update function was registered for the structure type
 * @param[in]   _pStructure     Concerned structure
 * @param[in]   _phCaller       Caller structure
 * @param[in]   _pstClockInfo   Update associated clock info
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                  orxStructure_Update(void *_pStructure, const void *_phCaller, const orxCLOCK_INFO *_pstClockInfo);


/** *** Structure storage accessors *** */


/** Gets structure given its GUID
 * @param[in]   _u64GUID        Structure's GUID
 * @return      orxSTRUCTURE / orxNULL if not found/alive
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL              orxStructure_Get(orxU64 _u64GUID);

/** Gets structure's owner
 * @param[in]   _pStructure     Concerned structure
 * @return      orxSTRUCTURE / orxNULL if not found/alive
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL              orxStructure_GetOwner(const void *_pStructure);

/** Sets structure owner
 * @param[in]   _pStructure     Concerned structure
 * @param[in]   _pOwner         Structure to set as owner
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                  orxStructure_SetOwner(void *_pStructure, void *_pOwner);

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
 * @param[in]   _pStructure     Concerned structure
 * @return      orxSTRUCTURE
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL              orxStructure_GetParent(const void *_pStructure);

/** Gets structure tree child
 * @param[in]   _pStructure     Concerned structure
 * @return      orxSTRUCTURE
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL              orxStructure_GetChild(const void *_pStructure);

/** Gets structure tree sibling
 * @param[in]   _pStructure     Concerned structure
 * @return      orxSTRUCTURE
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL              orxStructure_GetSibling(const void *_pStructure);

/** Gets structure list previous
 * @param[in]   _pStructure     Concerned structure
 * @return      orxSTRUCTURE
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL              orxStructure_GetPrevious(const void *_pStructure);

/** Gets structure list next
 * @param[in]   _pStructure     Concerned structure
 * @return      orxSTRUCTURE
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL              orxStructure_GetNext(const void *_pStructure);

/** Sets structure tree parent
 * @param[in]   _pStructure     Concerned structure
 * @param[in]   _phParent       Structure to set as parent
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                  orxStructure_SetParent(void *_pStructure, void *_phParent);


/** Logs all user-generated (& optionally private) active structures
 * @param[in]   _bPrivate       Include all private structures in the log
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                  orxStructure_LogAll(orxBOOL _bPrivate);


/** *** Inlined structure accessors *** */


/** Increases structure reference count
 * @param[in]   _pStructure     Concerned structure
 */
static orxINLINE void                                   orxStructure_IncreaseCount(void *_pStructure)
{
  /* Checks */
  orxSTRUCTURE_ASSERT(_pStructure);
  orxASSERT(orxSTRUCTURE(_pStructure)->u32RefCount < 0xFFFFFFFF);

  /* Increases ref count */
  orxSTRUCTURE(_pStructure)->u32RefCount++;

  /* Done! */
  return;
}

/** Decreases structure reference count
 * @param[in]   _pStructure     Concerned structure
 */
static orxINLINE void                                   orxStructure_DecreaseCount(void *_pStructure)
{
  /* Checks */
  orxSTRUCTURE_ASSERT(_pStructure);
  orxASSERT(orxSTRUCTURE(_pStructure)->u32RefCount != 0);

  /* Decreases ref count */
  orxSTRUCTURE(_pStructure)->u32RefCount--;

  /* Done! */
  return;
}

/** Gets structure reference count
 * @param[in]   _pStructure     Concerned structure
 * @return      orxU32
 */
static orxINLINE orxU32                                 orxStructure_GetRefCount(const void *_pStructure)
{
  /* Checks */
  orxSTRUCTURE_ASSERT(_pStructure);

  /* Done! */
  return orxSTRUCTURE(_pStructure)->u32RefCount;
}

/** Gets structure GUID
 * @param[in]   _pStructure     Concerned structure
 * @return      orxU64
 */
static orxINLINE orxU64                                 orxStructure_GetGUID(const void *_pStructure)
{
  /* Checks */
  orxSTRUCTURE_ASSERT(_pStructure);

  /* Done! */
  return orxSTRUCTURE(_pStructure)->u64GUID;
}

/** Gets structure ID
 * @param[in]   _pStructure     Concerned structure
 * @return      orxSTRUCTURE_ID
 */
static orxINLINE orxSTRUCTURE_ID                        orxStructure_GetID(const void *_pStructure)
{
  /* Checks */
  orxSTRUCTURE_ASSERT(_pStructure);

  /* Done! */
  return((orxSTRUCTURE_ID)((orxSTRUCTURE(_pStructure)->u64GUID & orxSTRUCTURE_GUID_MASK_STRUCTURE_ID) >> orxSTRUCTURE_GUID_SHIFT_STRUCTURE_ID));
}

/** Tests flags against structure ones
 * @param[in]   _pStructure     Concerned structure
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
 * @param[in]   _pStructure     Concerned structure
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
 * @param[in]   _pStructure     Concerned structure
 * @param[in]   _u32Mask        Mask to use for getting flags
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
 * @param[in]   _pStructure     Concerned structure
 * @param[in]   _u32AddFlags    Flags to add
 * @param[in]   _u32RemoveFlags Flags to remove
 */
static orxINLINE void                                   orxStructure_SetFlags(void *_pStructure, orxU32 _u32AddFlags, orxU32 _u32RemoveFlags)
{
  /* Checks */
  orxSTRUCTURE_ASSERT(_pStructure);

  orxFLAG_SET(orxSTRUCTURE(_pStructure)->u32Flags, _u32AddFlags, _u32RemoveFlags);

  /* Done! */
  return;
}

#endif /* _orxSTRUCTURE_H_ */

/** @} */
