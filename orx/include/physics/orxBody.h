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
* @date 05/02/2008
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


/** Body flags
 */
#define orxBODY_KU32_FLAG_NONE                0x00000000  /**< No flags */

#define orxBODY_KU32_FLAG_2D                  0x00000001  /**< 2D type body flag */

#define orxBODY_KU32_MASK_USER_ALL            0x000000FF  /**< User all ID mask */


#define orxBODY_KU32_PART_MAX_NUMBER          4


/** Internal Body structure
 */
typedef struct __orxBODY_t                    orxBODY;


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
 * @param[in]   _u32Flags                     Body flags (2D / ...)
 * @return      Created orxGRAPHIC / orxNULL
 */
extern orxDLLAPI orxBODY *orxFASTCALL         orxBody_Create(orxU32 _u32Flags);

/** Deletes a body
 * @param[in]   _pstBody        Concerned body
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_Delete(orxBODY *_pstBody);

/** Sets body part data
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _u32Index       Data index (should be less than orxBODY_KU32_DATA_MAX_NUMBER)
 * @param[in]   _pstData        Data structure to set / orxNULL
 * @param[in]   _u16SelfFlags   Self defining flags
 * @param[in]   _u16CheckMask   Mask to check against other body parts
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_SetPartData(orxBODY *_pstBody, orxU32 _u32Index, orxSTRUCTURE *_pstData, orxU16 _u16SelfFlags, orxU16 _u16CheckMask);

/** Gets body part data
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _u32Index       Part index (should be less than orxBODY_KU32_DATA_MAX_NUMBER)
 * @return      OrxSTRUCTURE / orxNULL
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL    orxBody_GetPartData(orxCONST orxBODY *_pstBody, orxU32 _u32Index);

/** Gets body part self flags
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _u32Index       Part index (should be less than orxBODY_KU32_DATA_MAX_NUMBER)
 * @return      Body part self flags / orxU16_UNDEFINED
 */
extern orxDLLAPI orxU16 orxFASTCALL           orxBody_GetPartSelfFlags(orxCONST orxBODY *_pstBody, orxU32 _u32Index);

/** Gets body part self flags
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _u32Index       Part index (should be less than orxBODY_KU32_DATA_MAX_NUMBER)
 * @return      Body part check mask / orxU16_UNDEFINED
 */
extern orxDLLAPI orxU16 orxFASTCALL           orxBody_GetPartCheckMask(orxCONST orxBODY *_pstBody, orxU32 _u32Index);

/** Enables / disables a body part
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _u32Index       Part index (should be less than orxBODY_KU32_DATA_MAX_NUMBER)
 * @param[in]   _bEnable        Enable / disable
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_EnablePart(orxBODY *_pstBody, orxU32 _u32Index, orxBOOL _bEnable);

/** Part enabled get accessor
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _u32Index       Part index (should be less than orxBODY_KU32_DATA_MAX_NUMBER)
 * @return      orxTRUE if enabled, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL          orxBody_IsPartEnabled(orxCONST orxBODY *_pstBody, orxU32 _u32Index);

#endif /* _orxBODY_H_ */


/** @} */
