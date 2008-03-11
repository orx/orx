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
#include "anim/orxAnimSet.h"
#include "math/orxVector.h"


/** Flags */

#define orxOBJECT_KU32_FLAG_NONE            0x00000000  /**< No flags */

#define orxOBJECT_KU32_FLAG_2D              0x00000001  /**< 2D flags */
#define orxOBJECT_KU32_FLAG_BODY            0x00000002  /**< Body flags */
#define orxOBJECT_KU32_FLAG_NEIGHBOR_LIST   0x00000004  /**< Neighbor list flag */  
#define orxOBJECT_KU32_MASK_USER_ALL        0x000000FF  /**< User all ID mask */


/** Defines */
#define orxOBJECT_GET_STRUCTURE(OBJECT, TYPE) orxSTRUCTURE_GET_POINTER(_orxObject_GetStructure(OBJECT, orxSTRUCTURE_ID_##TYPE), TYPE)


/** Internal object structure */
typedef struct __orxOBJECT_t                orxOBJECT;


/** Object module setup. */
extern orxDLLAPI orxVOID                    orxObject_Setup();
/** Inits the object system. */
extern orxDLLAPI orxSTATUS                  orxObject_Init();
/** Ends the object system. */
extern orxDLLAPI orxVOID                    orxObject_Exit();

/** Creates an empty object. */
extern orxDLLAPI orxOBJECT *                orxObject_Create();

/** Creates a specific object
 * @param[in]   _u32Flags             Object flags (2D / body / ...)
 * @return  orxOBJECT / orxNULL
 */
extern orxDLLAPI orxOBJECT *orxFASTCALL     orxObject_CreateSpecificObject(orxU32 _u32Flags);

/** Creates a specific object from bitmap file
 * @param[in]   _zBitmapFileName      Bitmap file name to associate with the 2D object
 * @param[in]   _u32Flags             Object flags (2D / body / ...)
 * @ return orxOBJECT / orxNULL
 */
extern orxDLLAPI orxOBJECT *orxFASTCALL     orxObject_CreateSpecificObjectFromFile(orxCONST orxSTRING _zBitmapFileName, orxU32 _u32Flags);

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

/** Links a structure to an object
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _pstStructure   Structure to link
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxObject_LinkStructure(orxOBJECT *_pstObject, orxSTRUCTURE *_pstStructure);

/** Unlinks structure from an object, given its structure ID
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _eStructureID   ID of structure to unlink
 */
extern orxDLLAPI orxVOID orxFASTCALL        orxObject_UnlinkStructure(orxOBJECT *_pstObject, orxSTRUCTURE_ID _eStructureID);


/* *** Object accessors *** */


/** Structure used by an object get accessor, given its structure ID. Structure must be cast correctly. */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL  _orxObject_GetStructure(orxCONST orxOBJECT *_pstObject, orxSTRUCTURE_ID _eStructureID);


/** Sets object pivot
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _pvPivot        Object pivot
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxObject_SetPivot(orxOBJECT *_pstObject, orxCONST orxVECTOR *_pvPivot);

/** Sets object position
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _pvPosition     Object position
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxObject_SetPosition(orxOBJECT *_pstObject, orxCONST orxVECTOR *_pvPosition);

/** Sets object rotation
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _fRotation      Object rotation
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxObject_SetRotation(orxOBJECT *_pstObject, orxFLOAT _fRotation);

/** Sets object scale
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _fScaleX        Object X scale
 * @param[in]   _fScaleY        Object Y scale
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxObject_SetScale(orxOBJECT *_pstObject, orxFLOAT _fScaleX, orxFLOAT _fScaleY);

/** Get object pivot
 * @param[in]   _pstObject      Concerned object
 * @param[out]  _pvPivot        Object pivot
 * @return      orxVECTOR / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL     orxObject_GetPivot(orxCONST orxOBJECT *_pstObject, orxVECTOR *_pvPivot);

/** Get object position
 * @param[in]   _pstObject      Concerned object
 * @param[out]  _pvPosition     Object position
 * @return      orxVECTOR / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL     orxObject_GetPosition(orxCONST orxOBJECT *_pstObject, orxVECTOR *_pvPosition);

/** Get object rotation
 * @param[in]   _pstObject      Concerned object
 * @return      Rotation value
 */
extern orxDLLAPI orxFLOAT orxFASTCALL       orxObject_GetRotation(orxCONST orxOBJECT *_pstObject);

/** Get object scale
 * @param[in]   _pstObject      Concerned object
 * @param[out]  _pfScaleX       Object X scale
 * @param[out]  _pfScaleY       Object Y scale
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxObject_GetScale(orxCONST orxOBJECT *_pstObject, orxFLOAT *_pfScaleX, orxFLOAT *_pfScaleY);


/** Sets an object parent
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _pstParent      Parent object to set / orxNULL
 * @return      orsSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxObject_SetParent(orxOBJECT *_pstObject, orxOBJECT *_pstParent);


/** Gets object size
 * @param[in]   _pstObject      Concerned object
 * @param[out]  _pfWidth        Object's width
 * @param[out]  _pfHeight       Object's height
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxObject_GetSize(orxCONST orxOBJECT *_pstObject, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight);


/** Sets an object animset
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _pstAnimSet     Animation set to set / orxNULL
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxObject_SetAnimSet(orxOBJECT *_pstObject, orxANIMSET *_pstAnimSet);


/** Sets current animation for object
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _hAnimHandle    Animation handle
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxObject_SetCurrentAnim(orxOBJECT *_pstObject, orxHANDLE _hAnimHandle);

/** Sets target animation for object
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _hAnimHandle    Animation handle
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxObject_SetTargetAnim(orxOBJECT *_pstObject, orxHANDLE _hAnimHandle);


/** Gets object's bounding box
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _pstBoundingBox Bounding box result
 * @return      Bounding box
 */
extern orxDLLAPI orxAABOX *orxFASTCALL      orxObject_GetBoundingBox(orxCONST orxOBJECT *_pstObject, orxAABOX *_pstBoundingBox);


/** Creates a list of object at proximity of the given box (ie. whose bounding volume intersects this box)
 * @param[in]   _pstCheckBox    Box to check intersection with
 * @param[in]   _pastObjectList Created object list / orxNULL if none found
 * @return      Number of objects contained in the list
 */
extern orxDLLAPI orxU32 orxFASTCALL         orxObject_CreateProximityList(orxCONST orxAABOX *_pstCheckBox, orxOBJECT **_pastObjectList);

/** Deletes an object list created with orxObject_CreateProximityList
 * @param[in]   _astObjectList  Concerned object list
 */
extern orxDLLAPI orxVOID orxFASTCALL        orxObject_DeleteProximityList(orxOBJECT *_astObjectList);


#endif /* _orxOBJECT_H_ */
