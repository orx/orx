/**
 * \file orxStructure.h
 *
 * Structure Module.
 * Allows to creates and handle structures.
 * Structures can be referenced by other structures (or objects).
 *
 * \todo
 * Add the required structures when needed.
 * Do a generic system for structure registering (id given at realtime)
 */


/***************************************************************************
 orxStructure.h
 Structure module

 begin                : 08/12/2003
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


#ifndef _orxSTRUCTURE_H_
#define _orxSTRUCTURE_H_

#include "orxInclude.h"

#include "core/orxClock.h"
#include "debug/orxDebug.h"
#include "memory/orxMemory.h"


/** Structure pointer get macro. */
#ifdef __orxDEBUG__

  #define orxSTRUCTURE_GET_POINTER(STRUCTURE, TYPE)               \
    ((STRUCTURE != orxNULL) && ((((orxSTRUCTURE *)STRUCTURE)->eID ^ orxSTRUCTURE_MAGIC_NUMBER) == orxSTRUCTURE_ID_##TYPE)) ? (orx##TYPE *)STRUCTURE : orxNULL

#else /* __orxDEBUG__ */

  #define orxSTRUCTURE_GET_POINTER(STRUCTURE, TYPE) (orx##TYPE *)STRUCTURE

#endif /* __orxDEBUG__ */

/** Structure register macro. */
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


/** Structure IDs. */
typedef enum __orxSTRUCTURE_ID_t
{
  /* *** Following structures can be linked to objects *** */

  orxSTRUCTURE_ID_FRAME = 0,
  orxSTRUCTURE_ID_GRAPHIC,
  orxSTRUCTURE_ID_ANIMPOINTER,
  orxSTRUCTURE_ID_BODY,

  orxSTRUCTURE_ID_LINKABLE_NUMBER,

  /* *** Below this point, structures can not be linked to objects *** */

  orxSTRUCTURE_ID_ANIM = orxSTRUCTURE_ID_LINKABLE_NUMBER,
  orxSTRUCTURE_ID_ANIMSET,
  orxSTRUCTURE_ID_CAMERA,
  orxSTRUCTURE_ID_OBJECT,
  orxSTRUCTURE_ID_TEXTURE,
  orxSTRUCTURE_ID_VIEWPORT,

  orxSTRUCTURE_ID_NUMBER,

  orxSTRUCTURE_ID_NONE = orxENUM_NONE

} orxSTRUCTURE_ID;

/** Structure storage types. */
typedef enum __orxSTRUCTURE_STORAGE_TYPE_t
{
  orxSTRUCTURE_STORAGE_TYPE_LINKLIST = 0,
  orxSTRUCTURE_STORAGE_TYPE_TREE,

  orxSTRUCTURE_STORAGE_TYPE_NUMBER,

  orxSTRUCTURE_STORAGE_TYPE_NONE = orxENUM_NONE,

} orxSTRUCTURE_STORAGE_TYPE;

/** Public struct structure (Must be first structure member!). */
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


/** Structure update callback function type. */
typedef orxSTATUS (orxFASTCALL *orxSTRUCTURE_UPDATE_FUNCTION)(orxSTRUCTURE *_pstStructure, orxCONST orxSTRUCTURE *_pstCaller, orxCONST orxCLOCK_INFO *_pstClockInfo);


/** Structure module setup. */
extern orxDLLAPI orxVOID                                orxStructure_Setup();
/** Inits the structure system. */
extern orxDLLAPI orxSTATUS                              orxStructure_Init();
/** Exits from the structure system. */
extern orxDLLAPI orxVOID                                orxStructure_Exit();

/** Registers a given ID. */
extern orxDLLAPI orxSTATUS  orxFASTCALL                 orxStructure_Register(orxSTRUCTURE_ID _eStructureID, orxSTRUCTURE_STORAGE_TYPE _eStorageType, orxMEMORY_TYPE _eMemoryType, orxU32 _u32Size, orxCONST orxSTRUCTURE_UPDATE_FUNCTION _pfnUpdate);
/** Unregisters a given ID. */
extern orxDLLAPI orxVOID    orxFASTCALL                 orxStructure_Unregister(orxSTRUCTURE_ID _eStructureID);

/** Creates a clean structure for given type. */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL              orxStructure_Create(orxSTRUCTURE_ID _eStructureID);
/** Deletes a structure (needs to be cleaned before). */
extern orxDLLAPI orxVOID    orxFASTCALL                 orxStructure_Delete(orxHANDLE _phStructure);

/** Gets given type structure number. */
extern orxDLLAPI orxU32     orxFASTCALL                 orxStructure_GetNumber(orxSTRUCTURE_ID _eStructureID);

/** Gets structure storage type. */
extern orxDLLAPI orxSTRUCTURE_STORAGE_TYPE orxFASTCALL  orxStructure_GetStorageType(orxSTRUCTURE_ID _eStructureID);

/** Updates structure if update function was registered for the structure type. */
extern orxDLLAPI orxSTATUS  orxFASTCALL                 orxStructure_Update(orxHANDLE _phStructure, orxCONST orxHANDLE _phCaller, orxCONST orxCLOCK_INFO *_pstClockInfo);


/** *** Structure storage accessors *** */


/** Gets first stored structure (first list cell or tree root depending on storage type). */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL              orxStructure_GetFirst(orxSTRUCTURE_ID _eStructureID);
/** Gets last stored structure (last list cell or tree root depending on storage type). */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL              orxStructure_GetLast(orxSTRUCTURE_ID _eStructureID);

/** Structure tree parent get accessor. */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL              orxStructure_GetParent(orxCONST orxHANDLE _phStructure);
/** Structure tree child get accessor. */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL              orxStructure_GetChild(orxCONST orxHANDLE _phStructure);
/** Structure tree sibling get accessor. */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL              orxStructure_GetSibling(orxCONST orxHANDLE _phStructure);
/** Structure list previous get accessor. */

extern orxDLLAPI orxSTRUCTURE *orxFASTCALL              orxStructure_GetPrevious(orxCONST orxHANDLE _phStructure);
/** Structure list next get accessor. */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL              orxStructure_GetNext(orxCONST orxHANDLE _phStructure);

/** Structure tree parent set accessor. */
extern orxDLLAPI orxSTATUS  orxFASTCALL                 orxStructure_SetParent(orxHANDLE _phStructure, orxHANDLE _phParent);


/** *** Inlined structure accessors *** */


/** Increases structure reference counter. */
orxSTATIC orxINLINE orxVOID                             orxStructure_IncreaseCounter(orxHANDLE _phStructure)
{
  /* Checks */
  orxSTRUCTURE_ASSERT(_phStructure);

  /* Increases it */
  ((orxSTRUCTURE *)_phStructure)->u32RefCounter++;

  return;
}

/** Decreases structure reference counter. */
orxSTATIC orxINLINE orxVOID                             orxStructure_DecreaseCounter(orxHANDLE _phStructure)
{
  /* Checks */
  orxSTRUCTURE_ASSERT(_phStructure);
  orxASSERT(((orxSTRUCTURE *)_phStructure)->u32RefCounter > 0);

  /* Decreases it */
  ((orxSTRUCTURE *)_phStructure)->u32RefCounter--;

  return;
}

/** Gets structure reference counter. */
orxSTATIC orxINLINE orxU32                              orxStructure_GetRefCounter(orxCONST orxHANDLE _phStructure)
{
  /* Checks */
  orxSTRUCTURE_ASSERT(_phStructure);

  /* Returns it */
  return(((orxSTRUCTURE *)_phStructure)->u32RefCounter);
}

/** Gets structure ID. */
orxSTATIC orxINLINE orxSTRUCTURE_ID                     orxStructure_GetID(orxCONST orxHANDLE _phStructure)
{
  /* Checks */
  orxSTRUCTURE_ASSERT(_phStructure);

  /* Returns it */
  return(((orxSTRUCTURE *)_phStructure)->eID ^ orxSTRUCTURE_MAGIC_NUMBER);
}

/** Structure flags test accessor
 * @param[in]   _phStructure    Concerned structure
 * @param[in]   _u32Flags       Flags to test
 * @return      orxTRUE / orxFALSE
 */
orxSTATIC orxINLINE orxBOOL                             orxStructure_TestFlags(orxCONST orxHANDLE _phStructure, orxU32 _u32Flags)
{
  /* Checks */
  orxSTRUCTURE_ASSERT(_phStructure);

  /* Done! */
  return(orxFLAG_TEST(((orxSTRUCTURE *)_phStructure)->u32Flags, _u32Flags));
}

/** Structure all flags test accessor
 * @param[in]   _phStructure    Concerned structure
 * @param[in]   _u32Flags       Flags to test
 * @return      orxTRUE / orxFALSE
 */
orxSTATIC orxINLINE orxBOOL                             orxStructure_TestAllFlags(orxCONST orxHANDLE _phStructure, orxU32 _u32Flags)
{
  /* Checks */
  orxSTRUCTURE_ASSERT(_phStructure);

  /* Done! */
  return(orxFLAG_TEST_ALL(((orxSTRUCTURE *)_phStructure)->u32Flags, _u32Flags));
}

/** Structure all flags get accessor
 * @param[in]   _phStructure    Concerned structure
 * @param[in]   _u32Flags       Mask to use for getting flags
 * @return      orxU32
 */
orxSTATIC orxINLINE orxU32                              orxStructure_GetFlags(orxCONST orxHANDLE _phStructure, orxU32 _u32Mask)
{
  /* Checks */
  orxSTRUCTURE_ASSERT(_phStructure);

  /* Done! */
  return(orxFLAG_GET(((orxSTRUCTURE *)_phStructure)->u32Flags, _u32Mask));
}

/** Structure flag set accessor
 * @param[in]   _phStructure    Concerned structure
 * @param[in]   _u32AddFlags    Flags to add
 * @param[in]   _u32RemoveFlags Flags to remove
 */
orxSTATIC orxINLINE orxVOID                             orxStructure_SetFlags(orxHANDLE _phStructure, orxU32 _u32AddFlags, orxU32 _u32RemoveFlags)
{
  /* Checks */
  orxSTRUCTURE_ASSERT(_phStructure);

  orxFLAG_SET(((orxSTRUCTURE *)_phStructure)->u32Flags, _u32AddFlags, _u32RemoveFlags);

  return;
}


#endif /* _orxSTRUCTURE_H_ */
