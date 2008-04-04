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
#define orxBODY_KU32_PART_MAX_NUMBER          8


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
 * @param[in]   _pstBodyPartDef Body part definition
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_AddPart(orxBODY *_pstBody, orxU32 _u32Index, orxCONST orxBODY_PART_DEF *_pstBodyPartDef);

/** Gets a body part
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _u32Index       Body part index (should be less than orxBODY_KU32_DATA_MAX_NUMBER)
 * @return      orxBODY_PART handle / orxHANDLE_UNDEFINED
 */
extern orxDLLAPI orxHANDLE orxFASTCALL        orxBody_GetPart(orxCONST orxBODY *_pstBody, orxU32 _u32Index);

/** Removes a body part
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _u32Index       Part index (should be less than orxBODY_KU32_DATA_MAX_NUMBER)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_RemovePart(orxBODY *_pstBody, orxU32 _u32Index);


/** Sets a body template
 * @param[in]   _pstBodyTemplate  Body template to set / orxNULL to remove it
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_SetTemplate(orxCONST orxBODY_DEF *_pstBodyTemplate);

/** Sets a body part template
 * @param[in]   _pstBodyPartTemplate  Body part template to set / orxNULL to remove it
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_SetPartTemplate(orxCONST orxBODY_PART_DEF *_pstBodyPartTemplate);

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

/** Sets a body speed
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _pvSpeed        Speed to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_SetSpeed(orxBODY *_pstBody, orxCONST orxVECTOR *_pvSpeed);

/** Sets a body angular velocity
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _fVelocity      Angular velocity to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_SetAngularVelocity(orxBODY *_pstBody, orxFLOAT _fVelocity);

/** Gets a body position
 * @param[in]   _pstBody        Concerned body
 * @param[out]  _pvPosition     Position to get
 * @return      Body position / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL       orxBody_GetPosition(orxBODY *_pstBody, orxVECTOR *_pvPosition);

/** Gets a body rotation
 * @param[in]   _pstBody        Concerned body
 * @return      Body rotation
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxBody_GetRotation(orxBODY *_pstBody);

/** Gets a body speed
 * @param[in]   _pstBody        Concerned body
 * @param[out]   _pvSpeed       Speed to get
 * @return      Body speed / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL       orxBody_GetSpeed(orxBODY *_pstBody, orxVECTOR *_pvSpeed);

/** Gets a body angular velocity
 * @param[in]   _pstBody        Concerned body
 * @return      Body angular velocity
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxBody_GetAngularVelocity(orxBODY *_pstBody);


#endif /* _orxBODY_H_ */


/** @} */
