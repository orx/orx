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

#define orxBODY_KU32_FLAG_2D                  0x00000001  /**< 2D type graphic flag  */

#define orxBODY_KU32_MASK_USER_ALL            0x000000FF  /**< User all ID mask */


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
 * @return      Created orxGRAPHIC / orxNULL
 */
extern orxDLLAPI orxBODY *                    orxBody_Create();

/** Deletes a body
 * @param[in]   _pstBody        Concerned body
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_Delete(orxBODY *_pstBody);

/** Sets body data
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _pstData        Data structure to set / orxNULL
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_SetData(orxBODY *_pstBody, orxSTRUCTURE *_pstData);

/** Gets body data
 * @param[in]   _pstBody     Concerned body
 * @return      OrxSTRUCTURE / orxNULL
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL    orxBody_GetData(orxCONST orxBODY *_pstBody);


#endif /* _orxGRAPHIC_H_ */


/** @} */
