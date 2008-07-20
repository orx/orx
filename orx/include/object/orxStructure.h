/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/

/**
 * @file orxStructure.h
 * @date 08/12/2003
 * @author (C) Arcallians
 */

/**
 * @addtogroup Object
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
#ifdef __orxDEBUG__

  #define orxSTRUCTURE_GET_POINTER(STRUCTURE, TYPE) ((orx##TYPE *)_orxStructure_GetPointer(STRUCTURE, orxSTRUCTURE_ID_##TYPE))

#else /* __orxDEBUG__ */

  #define orxSTRUCTURE_GET_POINTER(STRUCTURE, TYPE) ((orx##TYPE *)(STRUCTURE))

#endif /* __orxDEBUG__ */

#define orxSTRUCTURE(STRUCTURE)   (((STRUCTURE != orxNULL) && ((((orxSTRUCTURE *)STRUCTURE)->eID ^ orxSTRUCTURE_MAGIC_NUMBER) < orxSTRUCTURE_ID_NUMBER)) ? (orxSTRUCTURE *)STRUCTURE : (orxSTRUCTURE *)orxNULL)

#define orxANIM(STRUCTURE)        orxSTRUCTURE_GET_POINTER(STRUCTURE, ANIM)
#define orxANIMPOINTER(STRUCTURE) orxSTRUCTURE_GET_POINTER(STRUCTURE, ANIMPOINTER)
#define orxANIMSET(STRUCTURE)     orxSTRUCTURE_GET_POINTER(STRUCTURE, ANIMSET)
#define orxBODY(STRUCTURE)        orxSTRUCTURE_GET_POINTER(STRUCTURE, BODY)
#define orxCAMERA(STRUCTURE)      orxSTRUCTURE_GET_POINTER(STRUCTURE, CAMERA)
#define orxFRAME(STRUCTURE)       orxSTRUCTURE_GET_POINTER(STRUCTURE, FRAME)
#define orxFX(STRUCTURE)          orxSTRUCTURE_GET_POINTER(STRUCTURE, FX)
#define orxFXPOINTER(STRUCTURE)   orxSTRUCTURE_GET_POINTER(STRUCTURE, FXPOINTER)
#define orxGRAPHIC(STRUCTURE)     orxSTRUCTURE_GET_POINTER(STRUCTURE, GRAPHIC)
#define orxOBJECT(STRUCTURE)      orxSTRUCTURE_GET_POINTER(STRUCTURE, OBJECT)
#define orxSOUND(STRUCTURE)       orxSTRUCTURE_GET_POINTER(STRUCTURE, SOUND)
#define orxTEXTURE(STRUCTURE)     orxSTRUCTURE_GET_POINTER(STRUCTURE, TEXTURE)
#define orxVIEWPORT(STRUCTURE)    orxSTRUCTURE_GET_POINTER(STRUCTURE, VIEWPORT)

/** Structure register macro
 */
#define orxSTRUCTURE_REGISTER(TYPE, STORAGE_TYPE, MEMORY_TYPE, UPDATE_FUNCTION) \
  orxStructure_Register(orxSTRUCTURE_ID_##TYPE, STORAGE_TYPE, MEMORY_TYPE, sizeof(orx##TYPE), UPDATE_FUNCTION)

/** Structure assert
 */
#define orxSTRUCTURE_ASSERT(STRUCTURE)                          \
  orxASSERT((STRUCTURE) != orxNULL);                            \
  orxASSERT((((orxSTRUCTURE *)(STRUCTURE))->eID ^ orxSTRUCTURE_MAGIC_NUMBER) < orxSTRUCTURE_ID_NUMBER);

/** Structure magic number
 */
#ifdef __orxDEBUG__
  #define orxSTRUCTURE_MAGIC_NUMBER   0xDEFACED0
#else
  #define orxSTRUCTURE_MAGIC_NUMBER   0x00000000
#endif


/** Structure IDs
 */
typedef enum __orxSTRUCTURE_ID_t
{
  /* *** Following structures can be linked to objects *** */

  orxSTRUCTURE_ID_FRAME = 0,
  orxSTRUCTURE_ID_GRAPHIC,
  orxSTRUCTURE_ID_ANIMPOINTER,
  orxSTRUCTURE_ID_BODY,
  orxSTRUCTURE_ID_FXPOINTER,

  orxSTRUCTURE_ID_LINKABLE_NUMBER,

  /* *** Below this point, structures can not be linked to objects *** */

  orxSTRUCTURE_ID_ANIM = orxSTRUCTURE_ID_LINKABLE_NUMBER,
  orxSTRUCTURE_ID_ANIMSET,
  orxSTRUCTURE_ID_CAMERA,
  orxSTRUCTURE_ID_FX,
  orxSTRUCTURE_ID_OBJECT,
  orxSTRUCTURE_ID_SOUND,
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

/** Public struct structure (Must be first derived structure member!)
 */
typedef struct __orxSTRUCTURE_t
{
  /* Structure ID. : 4 */
  orxSTRUCTURE_ID eID;

  /* Reference counter. : 8 */
  orxU32          u32RefCounter;

  /* Flags : 12 */
  orxU32          u32Flags;

  /* Handle of internal storage node. : 16 */
  orxHANDLE       hStorageNode;

} orxSTRUCTURE;


/** Structure update callback function type
 */
typedef orxSTATUS (orxFASTCALL *orxSTRUCTURE_UPDATE_FUNCTION)(orxSTRUCTURE *_pstStructure, orxCONST orxSTRUCTURE *_pstCaller, orxCONST orxCLOCK_INFO *_pstClockInfo);


#ifdef __orxDEBUG__

/** Gets structure pointer / debug mode
 * @param[in]   _pStructure    Concerned structure
 * @param[in]   _eStructureID   ID to test the structure against
 * @return      Valid orxSTRUCTURE, orxNULL otherwise
 */
orxSTATIC orxINLINE orxSTRUCTURE *_orxStructure_GetPointer(orxCONST orxVOID *_pStructure, orxSTRUCTURE_ID _eStructureID)
{
  orxSTRUCTURE *pstResult;

  /* Updates result */
  pstResult = ((_pStructure != orxNULL) && (((orxSTRUCTURE *)_pStructure)->eID ^ orxSTRUCTURE_MAGIC_NUMBER) == _eStructureID) ? (orxSTRUCTURE *)_pStructure : (orxSTRUCTURE *)orxNULL;

  /* Done! */
  return pstResult;
}

#endif /* __orxDEBUG__ */


/** Structure module setup
 */
extern orxDLLAPI orxVOID                                orxStructure_Setup();

/** Initializess the structure module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS                              orxStructure_Init();

/** Exits from the structure module
 */
extern orxDLLAPI orxVOID                                orxStructure_Exit();


/** Registers a given ID
 * @param[in]   _eStructureID   Concerned structure ID
 * @param[in]   _eStorageType   Storage type to use for this structure type
 * @param[in]   _eMemoryTyp     Memory type to store this structure type
 * @param[in]   _u32Size        Structure size
 * @param[in]   _pfnUpdate      Structure update function
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS  orxFASTCALL                 orxStructure_Register(orxSTRUCTURE_ID _eStructureID, orxSTRUCTURE_STORAGE_TYPE _eStorageType, orxMEMORY_TYPE _eMemoryType, orxU32 _u32Size, orxCONST orxSTRUCTURE_UPDATE_FUNCTION _pfnUpdate);

/** Unregisters a given ID
 * @param[in]   _eStructureID   Concerned structure ID
 */
extern orxDLLAPI orxVOID    orxFASTCALL                 orxStructure_Unregister(orxSTRUCTURE_ID _eStructureID);


/** Creates a clean structure for given type
 * @param[in]   _eStructureID   Concerned structure ID
 * @return      orxSTRUCTURE / orxNULL
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL              orxStructure_Create(orxSTRUCTURE_ID _eStructureID);

/** Deletes a structure (needs to be cleaned beforehand)
 * @param[in]   _pStructure    Concerned structure
 */
extern orxDLLAPI orxVOID    orxFASTCALL                 orxStructure_Delete(orxVOID *_pStructure);


/** Gets structure storage type
 * @param[in]   _eStructureID   Concerned structure ID
 * @return      orxSTRUCTURE_STORAGE_TYPE
 */
extern orxDLLAPI orxSTRUCTURE_STORAGE_TYPE orxFASTCALL  orxStructure_GetStorageType(orxSTRUCTURE_ID _eStructureID);

/** Gets given type structure number
 * @param[in]   _eStructureID   Concerned structure ID
 * @return      orxU32 / orxU32_UNDEFINED
 */
extern orxDLLAPI orxU32     orxFASTCALL                 orxStructure_GetNumber(orxSTRUCTURE_ID _eStructureID);

/** Updates structure if update function was registered for the structure type
 * @param[in]   _pStructure    Concerned structure
 * @param[in]   _phCaller       Caller structure
 * @param[in]   _pstClockInfo   Update associated clock info
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS  orxFASTCALL                 orxStructure_Update(orxVOID *_pStructure, orxCONST orxVOID *_phCaller, orxCONST orxCLOCK_INFO *_pstClockInfo);


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
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL              orxStructure_GetParent(orxCONST orxVOID *_pStructure);

/** Gets structure tree child
 * @param[in]   _pStructure    Concerned structure
 * @return      orxSTRUCTURE
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL              orxStructure_GetChild(orxCONST orxVOID *_pStructure);

/** Gets structure tree sibling
 * @param[in]   _pStructure    Concerned structure
 * @return      orxSTRUCTURE
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL              orxStructure_GetSibling(orxCONST orxVOID *_pStructure);

/** Gets structure list previous
 * @param[in]   _pStructure    Concerned structure
 * @return      orxSTRUCTURE
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL              orxStructure_GetPrevious(orxCONST orxVOID *_pStructure);

/** Gets structure list next
 * @param[in]   _pStructure    Concerned structure
 * @return      orxSTRUCTURE
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL              orxStructure_GetNext(orxCONST orxVOID *_pStructure);

/** Sets structure tree parent
 * @param[in]   _pStructure    Concerned structure
 * @param[in]   _phParent       Structure to set as parent
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS  orxFASTCALL                 orxStructure_SetParent(orxVOID *_pStructure, orxVOID *_phParent);


/** *** Inlined structure accessors *** */


/** Increases structure reference counter
 * @param[in]   _pStructure    Concerned structure
 */
orxSTATIC orxINLINE orxVOID                             orxStructure_IncreaseCounter(orxVOID *_pStructure)
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
orxSTATIC orxINLINE orxVOID                             orxStructure_DecreaseCounter(orxVOID *_pStructure)
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
orxSTATIC orxINLINE orxU32                              orxStructure_GetRefCounter(orxCONST orxVOID *_pStructure)
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
orxSTATIC orxINLINE orxSTRUCTURE_ID                     orxStructure_GetID(orxCONST orxVOID *_pStructure)
{
  /* Checks */
  orxSTRUCTURE_ASSERT(_pStructure);

  /* Returns it */
  return((orxSTRUCTURE_ID)(orxSTRUCTURE(_pStructure)->eID ^ orxSTRUCTURE_MAGIC_NUMBER));
}

/** Tests flags against structure ones
 * @param[in]   _pStructure    Concerned structure
 * @param[in]   _u32Flags       Flags to test
 * @return      orxTRUE / orxFALSE
 */
orxSTATIC orxINLINE orxBOOL                             orxStructure_TestFlags(orxCONST orxVOID *_pStructure, orxU32 _u32Flags)
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
orxSTATIC orxINLINE orxBOOL                             orxStructure_TestAllFlags(orxCONST orxVOID *_pStructure, orxU32 _u32Flags)
{
  /* Checks */
  orxSTRUCTURE_ASSERT(_pStructure);

  /* Done! */
  return(orxFLAG_TEST_ALL(orxSTRUCTURE(_pStructure)->u32Flags, _u32Flags));
}

/** Gets structure flags
 * @param[in]   _pStructure    Concerned structure
 * @param[in]   _u32Flags       Mask to use for getting flags
 * @return      orxU32
 */
orxSTATIC orxINLINE orxU32                              orxStructure_GetFlags(orxCONST orxVOID *_pStructure, orxU32 _u32Mask)
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
orxSTATIC orxINLINE orxVOID                             orxStructure_SetFlags(orxVOID *_pStructure, orxU32 _u32AddFlags, orxU32 _u32RemoveFlags)
{
  /* Checks */
  orxSTRUCTURE_ASSERT(_pStructure);

  orxFLAG_SET(orxSTRUCTURE(_pStructure)->u32Flags, _u32AddFlags, _u32RemoveFlags);

  return;
}


#endif /* _orxSTRUCTURE_H_ */

/** @} */
