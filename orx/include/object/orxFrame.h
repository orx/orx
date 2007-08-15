/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/

/**
 * @file orxFrame.h
 * @date 01/12/2003
 * @author (C) Arcallians
 * 
 * @todo 
 * Use matrix instead of disjoint pos vector/angle float/scale float for frame data structure.
 * 3D system (later).
 */

/**
 * @addtogroup Object
 * 
 * Frame (scene node) Module.
 * Allows to handles frame (scene nodes).
 * It consists in a nodes (arranged in a hierarchical tree)
 * having their position given in their parent local coordinate subsystem
 * and caching their global one (in the root coordinate system).
 * They're used to give position, orientation & scale to an object or a group of objects.
 * Frames are structures.
 * They thus can be referenced by Object Module.
 *
 * @{
 */
 

#ifndef _orxFRAME_H_
#define _orxFRAME_H_

#include "orxInclude.h"

#include "math/orxVector.h"


/** Frame ID Flags
 */
#define orxFRAME_KU32_ID_FLAG_NONE            0x00000000  /**< No flags */

#define orxFRAME_KU32_ID_FLAG_SCROLL_X        0x00000001  /**< X axis differential scrolling ID flag */
#define orxFRAME_KU32_ID_FLAG_SCROLL_Y        0x00000002  /**< Y axis differential scrolling ID flag */
#define orxFRAME_KU32_ID_MASK_SCROLL_BOTH     0x00000003  /**< Both axis differential scrolling ID mask */

#define orxFRAME_KU32_ID_MASK_USER_ALL        0x000000FF  /**< User all ID mask */


/** Frame space enum
 */
typedef enum __orxFRAME_SPACE_t
{
  orxFRAME_SPACE_GLOBAL = 0,
  orxFRAME_SPACE_LOCAL,
  
  orxFRAME_SPACE_NUMBER,
  
  orxFRAME_SPACE_NONE = orxENUM_NONE
  
} orxFRAME_SPACE;


/** Internal Frame structure
 */
typedef struct __orxFRAME_t                   orxFRAME;


/** Frame module setup
 */
extern orxDLLAPI orxVOID                      orxFrame_Setup();

/** Inits the frame system
 */
extern orxDLLAPI orxSTATUS                    orxFrame_Init();

/** Ends the frame system
 */
extern orxDLLAPI orxVOID                      orxFrame_Exit();


/** Creates a frame
 * @param[in]   _u32IDFLags     ID flags for created animation
 * @return      Created orxFRAME / orxNULL
 */
extern orxDLLAPI orxFRAME *                   orxFrame_Create(orxU32 _u32IDFlags);

/** Deletes a frame
 * @param[in]   _pstFrame       Frame to delete
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxFrame_Delete(orxFRAME *_pstFrame);


/** Cleans all frames render status
 */
extern orxDLLAPI orxVOID                      orxFrame_CleanAllRenderStatus();

/** Test frame render status
 * @param[in]   _pstFrame       Frame to test
 * @return      orxTRUE / orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL          orxFrame_IsRenderStatusClean(orxCONST orxFRAME *_pstFrame);


/** Sets a frame parent
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _pstParent      Parent frame to set
 */
extern orxDLLAPI orxVOID orxFASTCALL          orxFrame_SetParent(orxFRAME *_pstFrame, orxFRAME * _pstParent);


/** Sets a frame position
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _pvPos          Position to set
 */
extern orxDLLAPI orxVOID orxFASTCALL          orxFrame_SetPosition(orxFRAME *_pstFrame, orxCONST orxVECTOR *_pvPos);

/** Sets a frame rotation
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _fAngle         Angle to set
 */
extern orxDLLAPI orxVOID orxFASTCALL          orxFrame_SetRotation(orxFRAME *_pstFrame, orxFLOAT _fAngle);

/** Sets a frame scale
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _fScale         Scale to set
 */
extern orxDLLAPI orxVOID orxFASTCALL          orxFrame_SetScale(orxFRAME *_pstFrame, orxFLOAT _fScale);


/** Gets a frame position
 * @param[in]   _pstFrame       Concerned frame
 * @param[out]  _pvPos          Position of the given frame
 * @param[in]   _eSpace         Coordinate space system to use
 * @return orxVECTOR / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL       orxFrame_GetPosition(orxFRAME *_pstFrame, orxVECTOR *_pvPos, orxFRAME_SPACE _eSpace);

/** Gets a frame rotation
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _eSpace         Coordinate space system to use
 * @return Rotation of the given frame */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxFrame_GetRotation(orxFRAME *_pstFrame, orxFRAME_SPACE _eSpace);

/** Gets a frame scale
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _eSpace         Coordinate space system to use
 * @return Scale of the given frame
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxFrame_GetScale(orxFRAME *_pstFrame, orxFRAME_SPACE _eSpace);


#endif /* _orxFRAME_H_ */


/** @} */
