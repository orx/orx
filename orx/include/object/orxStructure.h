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

#include "debug/orxDebug.h"


/** Structure IDs. */
typedef enum __orxSTRUCTURE_ID_t
{
  orxSTRUCTURE_ID_OBJECT = 0,
  orxSTRUCTURE_ID_FRAME,
  orxSTRUCTURE_ID_TEXTURE,
  orxSTRUCTURE_ID_GRAPHIC,
  orxSTRUCTURE_ID_CAMERA,
  orxSTRUCTURE_ID_VIEWPORT,
  orxSTRUCTURE_ID_ANIM,
  orxSTRUCTURE_ID_ANIMSET,
  orxSTRUCTURE_ID_ANIMPOINTER,

  orxSTRUCTURE_ID_NUMBER,
  
  orxSTRUCTURE_ID_MAX_NUMBER = 32,
  orxSTRUCTURE_ID_NONE = 0xFFFFFFFF

} orxSTRUCTURE_ID;

/** Structure storage types. */
typedef enum __orxSTRUCTURE_STORAGE_TYPE_t
{
  orxSTRUCTURE_STORAGE_TYPE_LINKLIST = 0,
  orxSTRUCTURE_STORAGE_TYPE_TREE,

  orxSTRUCTURE_STORAGE_TYPE_NUMBER,

  orxSTRUCTURE_STORAGE_TYPE_NONE = 0xFFFFFFFF,

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

/** Inits the structure system. */
extern orxSTATUS                      orxStructure_Init();
/** Exits from the structure system. */
extern orxVOID                        orxStructure_Exit();

/** Inits a structure with given type. */
extern orxFASTCALL  orxSTATUS         orxStructure_Setup(orxSTRUCTURE *_pstStructure, orxSTRUCTURE_ID _eStructureID);
/** Cleans a structure. */
extern orxFASTCALL  orxVOID           orxStructure_Clean(orxSTRUCTURE *_pstStructure);

/** Gets given type structure number. */
extern orxFASTCALL  orxU32            orxStructure_GetNumber(orxSTRUCTURE_ID _eStructureID);

/** Gets structure storage type. */
extern orxFASTCALL orxSTRUCTURE_STORAGE_TYPE orxStructure_GetStorageType(orxSTRUCTURE_ID _eStructureID);


/** *** Structure storage accessors *** */


/** Gets first stored structure (first list cell or tree root depending on storage type). */
extern orxFASTCALL  orxSTRUCTURE     *orxStructure_GetFirst(orxSTRUCTURE_ID _eStructureID);

/** Structure tree parent get accessor. */
extern orxFASTCALL  orxSTRUCTURE     *orxStructure_GetParent(orxSTRUCTURE *_pstStructure);
/** Structure tree child get accessor. */
extern orxFASTCALL  orxSTRUCTURE     *orxStructure_GetChild(orxSTRUCTURE *_pstStructure);
/** Structure tree sibling get accessor. */
extern orxFASTCALL  orxSTRUCTURE     *orxStructure_GetSibling(orxSTRUCTURE *_pstStructure);
/** Structure list previous get accessor. */

extern orxFASTCALL  orxSTRUCTURE     *orxStructure_GetPrevious(orxSTRUCTURE *_pstStructure);
/** Structure list next get accessor. */
extern orxFASTCALL  orxSTRUCTURE     *orxStructure_GetNext(orxSTRUCTURE *_pstStructure);

/** Structure tree parent set accessor. */
extern orxFASTCALL  orxSTATUS         orxStructure_SetParent(orxSTRUCTURE *_pstStructure, orxSTRUCTURE *_pstParent);


/** *** Inlined structure accessors *** */


/** Increases structure reference counter. */
extern orxINLINE    orxVOID           orxStructure_IncreaseCounter(orxSTRUCTURE *_pstStructure)
{
  /* Checks */
  orxASSERT(_pstStructure != orxNULL);

  /* Increases it */
  _pstStructure->u32RefCounter++;

  return;
}

/** Decreases structure reference counter. */
extern orxINLINE    orxVOID           orxStructure_DecreaseCounter(orxSTRUCTURE *_pstStructure)
{
  /* Checks */
  orxASSERT(_pstStructure != orxNULL);
  orxASSERT(_pstStructure->u32RefCounter > 0);

  /* Decreases it */
  _pstStructure->u32RefCounter--;

  return;
}

/** Gets structure reference counter. */
extern orxINLINE    orxU32            orxStructure_GetRefCounter(orxSTRUCTURE *_pstStructure)
{
  /* Checks */
  orxASSERT(_pstStructure != orxNULL);

  /* Returns it */
  return(_pstStructure->u32RefCounter);
}

/** Gets structure ID. */
extern orxINLINE    orxSTRUCTURE_ID   orxStructure_GetID(orxSTRUCTURE *_pstStructure)
{
  /* Checks */
  orxASSERT(_pstStructure != orxNULL);

  /* Returns it */
  return(_pstStructure->eID);
}


#endif /* _orxSTRUCTURE_H_ */
