/** 
 * \file orxObject.h
 * 
 * Object Module.
 * Allows to creates and handle objects.
 * Objects are structures containers that can refer to many other structures such as frames, graphics, etc...
 * 
 * \todo
 * Call the anim IsRenderStatusClean when it has been created.
 * Add the required structures when needed.
 * Add Sweep & Prune and specialized storages depending on purposes.
 */


/***************************************************************************
 orxObject.h
 Object module
 
 begin                : 01/12/2003
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


#ifndef _orxOBJECT_H_
#define _orxOBJECT_H_


#include "orxInclude.h"

#include "object/orxStructure.h"


/** Internal object structure. */
typedef struct __orxOBJECT_t                orxOBJECT;


/** Object module setup. */
extern orxDLLAPI orxVOID                    orxObject_Setup();
/** Inits the object system. */
extern orxDLLAPI orxSTATUS                  orxObject_Init();
/** Ends the object system. */
extern orxDLLAPI orxVOID                    orxObject_Exit();

/** Creates an empty object. */
extern orxDLLAPI orxOBJECT *                orxObject_Create();
/** Deletes an object. */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxObject_Delete(orxOBJECT *_pstObject);

/** Enables/disables an object
 * @param[in]   _pstObject    Concerned object
 * @param[in]   _bEnable      enable / disable
 */
extern orxDLLAPI orxVOID orxFASTCALL        orxObject_Enable(orxOBJECT *_pstObject, orxBOOL _bEnable);

/** Is object enabled?
 * @param[in]   _pstObject    Concerned object
 * @return      orxTRUE if enabled, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL        orxObject_IsEnabled(orxCONST orxOBJECT *_pstObject);

/** Links a structure to an object. */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxObject_LinkStructure(orxOBJECT *_pstObject, orxSTRUCTURE *_pstStructure);
/** Unlinks structure from an object, given its structure ID. */
extern orxDLLAPI orxVOID orxFASTCALL        orxObject_UnlinkStructure(orxOBJECT *_pstObject, orxSTRUCTURE_ID _eStructureID);


/* *** Object accessors *** */


/** Structure used by an object get accessor, given its structure ID. Structure must be cast correctly. */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL  orxObject_GetStructure(orxCONST orxOBJECT *_pstObject, orxSTRUCTURE_ID _eStructureID);


#endif /* _orxOBJECT_H_ */
