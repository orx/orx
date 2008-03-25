/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/

/**
 * @file orxBody.h
 * @date 10/03/2008
 * @author (C) Arcallians
 * 
 * @todo 
 */

/**
 * @addtogroup Physics
 * 
 * Body Module
 * Allows to creates and handle physical bodies
 * They are used as container with associated properties.
 * Bodies are used by objects.
 * They thus can be referenced by objects as structures.
 *
 * @{
 */


#ifndef _orxBODY_H_
#define _orxBODY_H_

#include "orxInclude.h"

#include "object/orxStructure.h"
#include "physics/orxPhysics.h"


/** Misc defines
 */
#define orxBODY_KU32_PART_MAX_NUMBER          4


/** Internal Body structure
 */
typedef struct __orxBODY_t                    orxBODY;

/** Internal Body part structure
 */
typedef struct __orxBODY_PART_t               orxBODY_PART;


/** Body module setup
 */
extern orxDLLAPI orxVOID                      orxBody_Setup();

/** Inits the Body module
 */
extern orxDLLAPI orxSTATUS                    orxBody_Init();

/** Exits from the Body module
 */
extern orxDLLAPI orxVOID                      orxBody_Exit();


/** Creates an empty body
 * @param[in]   _pstBodyDef                   Body definition
 * @return      Created orxGRAPHIC / orxNULL
 */
extern orxDLLAPI orxBODY *orxFASTCALL         orxBody_Create(orxCONST orxBODY_DEF *_pstBodyDef);

/** Deletes a body
 * @param[in]   _pstBody        Concerned body
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_Delete(orxBODY *_pstBody);

/** Adds a part to body
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _u32Index       Part index (should be less than orxBODY_KU32_PART_MAX_NUMBER)
 * @param[in]   _pstPartDef     Body part definition
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_AddPart(orxBODY *_pstBody, orxU32 _u32Index, orxBODY_PART_DEF *_pstPartDef);

/** Gets a body part
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _u32Index       Body part index (should be less than orxBODY_KU32_DATA_MAX_NUMBER)
 * @return      orxBODY_PART / orxNULL
 */
extern orxDLLAPI orxBODY_PART *orxFASTCALL    orxBody_GetPart(orxCONST orxBODY *_pstBody, orxU32 _u32Index);

/** Removes a body part
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _u32Index       Part index (should be less than orxBODY_KU32_DATA_MAX_NUMBER)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_RemovePart(orxBODY *_pstBody, orxU32 _u32Index);

/** Gets body part self flags
 * @param[in]   _pstBodyPart    Concerned body part
 * @return      Body part self flags / orxU16_UNDEFINED
 */
extern orxDLLAPI orxU16 orxFASTCALL           orxBody_GetPartSelfFlags(orxCONST orxBODY_PART *_pstBodyPart);

/** Gets body part check mask
 * @param[in]   _pstBodyPart    Concerned body part
 * @return      Body part check mask / orxU16_UNDEFINED
 */
extern orxDLLAPI orxU16 orxFASTCALL           orxBody_GetPartCheckMask(orxCONST orxBODY_PART *_pstBodyPart);


/** Sets a body position
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _pvPosition     Position to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_SetPosition(orxBODY *_pstBody, orxCONST orxVECTOR *_pvPosition);

/** Sets a body rotation
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _fRotation      Rotation to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_SetRotation(orxBODY *_pstBody, orxFLOAT _fRotation);

#endif /* _orxBODY_H_ */


/** @} */
