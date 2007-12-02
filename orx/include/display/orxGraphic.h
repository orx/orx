/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/

/**
 * @file orxGraphic.h
 * @date 08/12/2003
 * @author (C) Arcallians
 * 
 * @todo 
 * Later on, add a texture cache system for rotated/scaled texture.
 * This cache system should be activated or not by user.
 * Adds 3D system (Not planned yet).
 */

/**
 * @addtogroup Displat
 * 
 * Graphic Module.
 * Allows to creates and handle 2D/3D Graphics.
 * They are used as texture/animation container, with associated properties.
 * 2D Graphics are used by objects.
 * They thus can be referenced by objects as structures.
 *
 * @{
 */


#ifndef _orxGRAPHIC_H_
#define _orxGRAPHIC_H_

#include "orxInclude.h"

#include "display/orxTexture.h"
#include "object/orxStructure.h"


/** Graphic flags
 */
#define orxGRAPHIC_KU32_FLAG_NONE             0x00000000  /**< No flags */

#define orxGRAPHIC_KU32_FLAG_2D               0x00000001  /**< 2D type graphic ID flag  */

#define orxGRAPHIC_KU32_MASK_ALL              0xFFFFFFFF  /**< All flags */


/** Internal Graphic structure
 */
typedef struct __orxGRAPHIC_t                 orxGRAPHIC;


/** Graphic module setup
 */
extern orxDLLAPI orxVOID                      orxGraphic_Setup();

/** Inits the Graphic module
 */
extern orxDLLAPI orxSTATUS                    orxGraphic_Init();

/** Exits from the Graphic module
 */
extern orxDLLAPI orxVOID                      orxGraphic_Exit();


/** Creates an empty graphic
 * @return      Created orxGRAPHIC / orxNULL
 */
extern orxDLLAPI orxGRAPHIC *                 orxGraphic_Create();

/** Deletes a graphic
 * @param[in]   _pstGraphic       Graphic to delete
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxGraphic_Delete(orxGRAPHIC *_pstGraphic);


/** Sets graphic data
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[in]   _pstData        Data structure to set / orxNULL
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxGraphic_SetData(orxGRAPHIC *_pstGraphic, orxSTRUCTURE *_pstData);

/** Gets graphic data
 * @param[in]   _pstGraphic     Concerned graphic
 * @return      OrxSTRUCTURE / orxNULL
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL    orxGraphic_GetData(orxCONST orxGRAPHIC *_pstGraphic);

/** Sets graphic pivot
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[in]   _pvPivot        Pivot to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxGraphic_SetPivot(orxGRAPHIC *_pstGraphic, orxCONST orxVECTOR *_pvPivot);

/** Gets graphic pivot
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[out]  _pvPivot        Graphic pivot
 * @return      orxPIVOT / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL       orxGraphic_GetPivot(orxCONST orxGRAPHIC *_pstGraphic, orxVECTOR *_pvPivot);


/** Gets graphic size
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[out]  _pfWidth        Object's width
 * @param[out]  _pfHeight       Object's height
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxGraphic_GetSize(orxCONST orxGRAPHIC *_pstGraphic, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight);

#endif /* _orxGRAPHIC_H_ */


/** @} */
