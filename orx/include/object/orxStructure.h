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


/* *** Structure IDs. *** */

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


/** Internal storage cell structure. */
typedef orxVOID                      *orxSTRUCTURE_CELL;

/** Public struct structure (Must be first structure member!).*/
typedef struct __orxSTRUCTURE_t
{
  /* Structure ID. : 4 */
  orxSTRUCTURE_ID eID;

  /* Reference counter. : 8 */
  orxU32 u32RefCounter;

  /* Pointer to storage cell. : 12 */
  orxSTRUCTURE_CELL *pstCell;

  /* 4 extra bytes of padding : 16 */
  orxU8 au8Unused[4];
  
} orxSTRUCTURE;

/** Inits the structure system. */
extern orxSTATUS                      orxStructure_Init();
/** Exits from the structure system. */
extern orxVOID                        orxStructure_Exit();

/** Inits a structure with given type. */
extern orxSTATUS                      orxStructure_Setup(orxSTRUCTURE *_pstStructure, orxSTRUCTURE_ID _eStructureID);
/** Cleans a structure. */
extern orxVOID                        orxStructure_Clean(orxSTRUCTURE *_pstStructure);

/** Increase structure reference counter. */
extern orxVOID                        orxStructure_IncreaseCounter(orxSTRUCTURE *_pstStructure);
/** Decrease structure reference counter. */
extern orxVOID                        orxStructure_DecreaseCounter(orxSTRUCTURE *_pstStructure);
/** Gets structure reference counter. */
extern orxU32                         orxStructure_GetCounter(orxSTRUCTURE *_pstStructure);
/** Gets structure ID. */
extern orxSTRUCTURE_ID                orxStructure_GetID(orxSTRUCTURE *_pstStructure);

/** Gets given type structure number. */
extern orxU32                         orxStructure_GetNumber(orxSTRUCTURE_ID _eStructureID);
/** Gets first stored structure (first list cell or tree root depending on storage type). */
extern orxSTRUCTURE                  *orxStructure_GetFirst(orxSTRUCTURE_ID _eStructureID);


/** *** Structure accessors *** */

/** Structure tree parent get accessor. */
extern orxSTRUCTURE                  *orxStructure_GetParent(orxSTRUCTURE *_pstStructure);
/** Structure tree child get accessor. */
extern orxSTRUCTURE                  *orxStructure_GetChild(orxSTRUCTURE *_pstStructure);
/** Structure tree left sibling get accessor. */
extern orxSTRUCTURE                  *orxStructure_GetLeftSibling(orxSTRUCTURE *_pstStructure);
/** Structure tree right sibling get accessor. */
extern orxSTRUCTURE                  *orxStructure_GetRightSibling(orxSTRUCTURE *_pstStructure);
/** Structure list previous get accessor. */
extern orxSTRUCTURE                  *orxStructure_GetPrevious(orxSTRUCTURE *_pstStructure);
/** Structure list next get accessor. */
extern orxSTRUCTURE                  *orxStructure_GetNext(orxSTRUCTURE *_pstStructure);

/** Structure tree parent set accessor. */
extern orxVOID                        orxStructure_SetParent(orxSTRUCTURE *_pstStructure, orxSTRUCTURE *_pstParent);


#endif /* _orxSTRUCTURE_H_ */
