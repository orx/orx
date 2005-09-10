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
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef _orxSTRUCTURE_H_
#define _orxSTRUCTURE_H_

#include "orxInclude.h"

#include "core/orxClock.h"
#include "debug/orxDebug.h"
#include "memory/orxMemory.h"


/** Structure pointer get macro. */
#define orxSTRUCTURE_GET_POINTER(STRUCTURE, TYPE)               \
  (((orxSTRUCTURE *)STRUCTURE)->eID == orxSTRUCTURE_ID_##TYPE) ? (orx##TYPE *)STRUCTURE : orxNULL;


/** Structure IDs. */
typedef enum __orxSTRUCTURE_ID_t
{
  /* *** Following structures can be linked to objects *** */

  orxSTRUCTURE_ID_FRAME = 0,
  orxSTRUCTURE_ID_GRAPHIC,
  orxSTRUCTURE_ID_ANIMPOINTER,

  orxSTRUCTURE_ID_LINKABLE_NUMBER,

  /* *** Below this point, structures can be linked to objects *** */

  orxSTRUCTURE_ID_TEXTURE = orxSTRUCTURE_ID_LINKABLE_NUMBER,
  orxSTRUCTURE_ID_OBJECT,
  orxSTRUCTURE_ID_CAMERA,
  orxSTRUCTURE_ID_VIEWPORT,
  orxSTRUCTURE_ID_ANIM,
  orxSTRUCTURE_ID_ANIMSET,

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
  orxU32 u32RefCounter;

  /* Handle of internal storage node. : 12 */
  orxHANDLE hStorageNode;

  /* 4 extra bytes of padding : 16 */
  orxU8 au8Unused[4];
  
} orxSTRUCTURE;


/** Structure update callback function type. */
typedef orxSTATUS orxFASTCALL (*orxSTRUCTURE_UPDATE_FUNCTION)(orxSTRUCTURE *_pstStructure, orxCONST orxSTRUCTURE *_pstCaller, orxCONST orxCLOCK_INFO *_pstClockInfo);

/** Structure registration info. */
typedef struct __orxSTRUCTURE_REGISTER_INFO_t
{
  /* Structure storage type : 4 */
  orxSTRUCTURE_STORAGE_TYPE eStorageType;

  /* Structure storage size : 8 */
  orxU32 u32Size;

  /* Structure storage memory type : 12 */
  orxMEMORY_TYPE eMemoryType;

  /* Structure update callbacks : 16 */
  orxSTRUCTURE_UPDATE_FUNCTION pfnUpdate;

} orxSTRUCTURE_REGISTER_INFO;


/** Inits the structure system. */
extern orxDLLAPI orxSTATUS                      orxStructure_Init();
/** Exits from the structure system. */
extern orxDLLAPI orxVOID                        orxStructure_Exit();

/** Registers a given ID. */
extern orxDLLAPI orxSTATUS  orxFASTCALL         orxStructure_Register(orxSTRUCTURE_ID _eStructureID, orxCONST orxSTRUCTURE_REGISTER_INFO *_pstRegisterInfo);
/** Unregisters a given ID. */
extern orxDLLAPI orxVOID    orxFASTCALL         orxStructure_Unregister(orxSTRUCTURE_ID _eStructureID);

/** Creates a clean structure for given type. */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL      orxStructure_Create(orxSTRUCTURE_ID _eStructureID);
/** Deletes a structure (needs to be cleaned before). */
extern orxDLLAPI orxVOID    orxFASTCALL         orxStructure_Delete(orxSTRUCTURE *_pstStructure);

/** Gets given type structure number. */
extern orxDLLAPI orxU32     orxFASTCALL         orxStructure_GetNumber(orxSTRUCTURE_ID _eStructureID);

/** Gets structure storage type. */
extern orxDLLAPI orxSTRUCTURE_STORAGE_TYPE orxFASTCALL orxStructure_GetStorageType(orxSTRUCTURE_ID _eStructureID);

/** Updates structure if update function was registered for the structure type. */
extern orxDLLAPI orxSTATUS  orxFASTCALL         orxStructure_Update(orxSTRUCTURE *_pstStructure, orxCONST orxSTRUCTURE *_pstCaller, orxCONST orxCLOCK_INFO *_pstClockInfo);


/** *** Structure storage accessors *** */


/** Gets first stored structure (first list cell or tree root depending on storage type). */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL      orxStructure_GetFirst(orxSTRUCTURE_ID _eStructureID);

/** Structure tree parent get accessor. */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL      orxStructure_GetParent(orxCONST orxSTRUCTURE *_pstStructure);
/** Structure tree child get accessor. */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL      orxStructure_GetChild(orxCONST orxSTRUCTURE *_pstStructure);
/** Structure tree sibling get accessor. */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL      orxStructure_GetSibling(orxCONST orxSTRUCTURE *_pstStructure);
/** Structure list previous get accessor. */

extern orxDLLAPI orxSTRUCTURE *orxFASTCALL      orxStructure_GetPrevious(orxCONST orxSTRUCTURE *_pstStructure);
/** Structure list next get accessor. */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL      orxStructure_GetNext(orxCONST orxSTRUCTURE *_pstStructure);

/** Structure tree parent set accessor. */
extern orxDLLAPI orxSTATUS  orxFASTCALL         orxStructure_SetParent(orxSTRUCTURE *_pstStructure, orxSTRUCTURE *_pstParent);


/** *** Inlined structure accessors *** */


/** Increases structure reference counter. */
orxSTATIC orxINLINE orxDLLAPI  orxVOID          orxStructure_IncreaseCounter(orxSTRUCTURE *_pstStructure)
{
  /* Checks */
  orxASSERT(_pstStructure != orxNULL);

  /* Increases it */
  _pstStructure->u32RefCounter++;

  return;
}

/** Decreases structure reference counter. */
orxSTATIC orxINLINE orxDLLAPI  orxVOID          orxStructure_DecreaseCounter(orxSTRUCTURE *_pstStructure)
{
  /* Checks */
  orxASSERT(_pstStructure != orxNULL);
  orxASSERT(_pstStructure->u32RefCounter > 0);

  /* Decreases it */
  _pstStructure->u32RefCounter--;

  return;
}

/** Gets structure reference counter. */
orxSTATIC orxINLINE orxDLLAPI  orxU32           orxStructure_GetRefCounter(orxCONST orxSTRUCTURE *_pstStructure)
{
  /* Checks */
  orxASSERT(_pstStructure != orxNULL);

  /* Returns it */
  return(_pstStructure->u32RefCounter);
}

/** Gets structure ID. */
orxSTATIC orxINLINE orxDLLAPI  orxSTRUCTURE_ID  orxStructure_GetID(orxCONST orxSTRUCTURE *_pstStructure)
{
  /* Checks */
  orxASSERT(_pstStructure != orxNULL);

  /* Returns it */
  return(_pstStructure->eID);
}


#endif /* _orxSTRUCTURE_H_ */
