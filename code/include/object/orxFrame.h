/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2021 Orx-Project
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 */

/**
 * @file orxFrame.h
 * @date 01/12/2003
 * @author iarwain@orx-project.org
 *
 * @todo
 * - Use matrix instead of disjoint position vector/rotation float/scale vector
 * - Add 3D support
 */

/**
 * @addtogroup orxFrame
 *
 * Frame (scene node) Module.
 * Allows to handles frame (scene nodes).
 * It consists in a nodes (arranged in a hierarchical tree)
 * having their position given in their parent local coordinate subsystem
 * and caching their global one (in the root coordinate system).
 * They're used to give position, rotation & scale to an object or a group of objects.
 * Frames are structures.
 * They thus can be referenced by Object Module.
 *
 * @{
 */


#ifndef _orxFRAME_H_
#define _orxFRAME_H_

#include "orxInclude.h"

#include "math/orxVector.h"


/** Frame flags
 */
#define orxFRAME_KU32_FLAG_NONE                       0x00000000  /**< No flags */

#define orxFRAME_KU32_FLAG_SCROLL_X                   0x00000001  /**< X axis differential scrolling flag */
#define orxFRAME_KU32_FLAG_SCROLL_Y                   0x00000002  /**< Y axis differential scrolling flag */
#define orxFRAME_KU32_MASK_SCROLL_BOTH                0x00000003  /**< Both axis differential scrolling mask */

#define orxFRAME_KU32_FLAG_DEPTH_SCALE                0x00000004  /**< Relative depth scaling flag */

#define orxFRAME_KU32_FLAG_FLIP_X                     0x00000010  /**< X axis flipping flag */
#define orxFRAME_KU32_FLAG_FLIP_Y                     0x00000020  /**< Y axis flipping flag */
#define orxFRAME_KU32_MASK_FLIP_BOTH                  0x00000030  /**< Both axis flippinf mask */

#define orxFRAME_KU32_FLAG_IGNORE_NONE                0x00000000  /**< Rotation, scale and position are affected by parent */
#define orxFRAME_KU32_FLAG_IGNORE_ROTATION            0x00000100  /**< Rotation is unaffected by parent */
#define orxFRAME_KU32_FLAG_IGNORE_SCALE_X             0x00000200  /**< Scale is unaffected by parent on X axis */
#define orxFRAME_KU32_FLAG_IGNORE_SCALE_Y             0x00000400  /**< Scale is unaffected by parent on Y axis */
#define orxFRAME_KU32_FLAG_IGNORE_SCALE_Z             0x00000800  /**< Scale is unaffected by parent on Z axis */
#define orxFRAME_KU32_MASK_IGNORE_SCALE               0x00000E00  /**< Scale is unaffected by parent */
#define orxFRAME_KU32_FLAG_IGNORE_POSITION_ROTATION   0x00001000  /**< Position is unaffected by parent's rotation */
#define orxFRAME_KU32_FLAG_IGNORE_POSITION_SCALE_X    0x00002000  /**< Position is unaffected by parent's scale on X axis */
#define orxFRAME_KU32_FLAG_IGNORE_POSITION_SCALE_Y    0x00004000  /**< Position is unaffected by parent's scale on Y axis */
#define orxFRAME_KU32_FLAG_IGNORE_POSITION_SCALE_Z    0x00008000  /**< Position is unaffected by parent's scale on Z axis */
#define orxFRAME_KU32_MASK_IGNORE_POSITION_SCALE      0x0000E000  /**< Position is unaffected by parent's scale */
#define orxFRAME_KU32_FLAG_IGNORE_POSITION_POSITION_X 0x00010000  /**< Position is unaffected by parent's position on X axis */
#define orxFRAME_KU32_FLAG_IGNORE_POSITION_POSITION_Y 0x00020000  /**< Position is unaffected by parent's position on Y axis */
#define orxFRAME_KU32_FLAG_IGNORE_POSITION_POSITION_Z 0x00040000  /**< Position is unaffected by parent's position on Z axis */
#define orxFRAME_KU32_MASK_IGNORE_POSITION_POSITION   0x00070000  /**< Position is unaffected by parent's position */
#define orxFRAME_KU32_MASK_IGNORE_POSITION            0x0007F000  /**< Position is unaffected by parent */
#define orxFRAME_KU32_MASK_IGNORE_ALL                 0x0007FF00  /**< Rotation, scale and position are unaffected by parent */

#define orxFRAME_KU32_MASK_USER_ALL                   0x0007FFFF  /**< User all ID mask */


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


/** Get ignore flag values
 * @param[in]   _zFlags         Literal ignore flags
 * @return Ignore flags
 */
extern orxDLLAPI orxU32 orxFASTCALL           orxFrame_GetIgnoreFlagValues(const orxSTRING _zFlags);

/** Get ignore flag names (beware: result won't persist from one call to the other)
 * @param[in]   _u32Flags       Literal ignore flags
 * @return Ignore flags names
 */
extern orxDLLAPI const orxSTRING orxFASTCALL  orxFrame_GetIgnoreFlagNames(orxU32 _u32Flags);


/** Setups the frame module
 */
extern orxDLLAPI void orxFASTCALL             orxFrame_Setup();

/** Inits the frame module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxFrame_Init();

/** Exits from the frame module
 */
extern orxDLLAPI void orxFASTCALL             orxFrame_Exit();


/** Creates a frame
 * @param[in]   _u32Flags       Flags for created animation
 * @return      Created orxFRAME / orxNULL
 */
extern orxDLLAPI orxFRAME *orxFASTCALL        orxFrame_Create(orxU32 _u32Flags);

/** Deletes a frame
 * @param[in]   _pstFrame       Frame to delete
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxFrame_Delete(orxFRAME *_pstFrame);


/** Sets frame parent
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _pstParent      Parent frame to set
 */
extern orxDLLAPI void orxFASTCALL             orxFrame_SetParent(orxFRAME *_pstFrame, orxFRAME * _pstParent);

/** Get frame parent
 * @param[in]   _pstFrame       Concerned frame
 * @return orxFRAME / orxNULL
 */
extern orxDLLAPI orxFRAME *orxFASTCALL        orxFrame_GetParent(const orxFRAME *_pstFrame);

/** Gets frame first child
 * @param[in]   _pstFrame       Concerned frame
 * @return orxFRAME / orxNULL
 */
extern orxDLLAPI orxFRAME *orxFASTCALL        orxFrame_GetChild(const orxFRAME *_pstFrame);

/** Gets frame next sibling
 * @param[in]   _pstFrame       Concerned frame
 * @return orxFRAME / orxNULL
 */
extern orxDLLAPI orxFRAME *orxFASTCALL        orxFrame_GetSibling(const orxFRAME *_pstFrame);

/** Is a root child?
 * @param[in]   _pstFrame       Concerned frame
 * @return orxTRUE if its parent is root, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL          orxFrame_IsRootChild(const orxFRAME *_pstFrame);


/** Sets frame position
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _eSpace         Coordinate space system to use
 * @param[in]   _pvPos          Position to set
 */
extern orxDLLAPI void orxFASTCALL             orxFrame_SetPosition(orxFRAME *_pstFrame, orxFRAME_SPACE _eSpace, const orxVECTOR *_pvPos);

/** Sets frame rotation
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _eSpace         Coordinate space system to use
 * @param[in]   _fRotation      Rotation angle to set (radians)
 */
extern orxDLLAPI void orxFASTCALL             orxFrame_SetRotation(orxFRAME *_pstFrame, orxFRAME_SPACE _eSpace, orxFLOAT _fRotation);

/** Sets frame scale
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _eSpace         Coordinate space system to use
 * @param[in]   _pvScale        Scale to set
 */
extern orxDLLAPI void orxFASTCALL             orxFrame_SetScale(orxFRAME *_pstFrame, orxFRAME_SPACE _eSpace, const orxVECTOR *_pvScale);


/** Gets frame position
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _eSpace         Coordinate space system to use
 * @param[out]  _pvPos          Position of the given frame
 * @return orxVECTOR / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL       orxFrame_GetPosition(orxFRAME *_pstFrame, orxFRAME_SPACE _eSpace, orxVECTOR *_pvPos);

/** Gets frame rotation
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _eSpace         Coordinate space system to use
 * @return Rotation of the given frame (radians)
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxFrame_GetRotation(orxFRAME *_pstFrame, orxFRAME_SPACE _eSpace);

/** Gets frame scale
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _eSpace         Coordinate space system to use
 * @param[out]  _pvScale        Scale
 * @return      orxVECTOR / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL       orxFrame_GetScale(orxFRAME *_pstFrame, orxFRAME_SPACE _eSpace, orxVECTOR *_pvScale);


/** Transforms a position given its input space (local -> global or global -> local)
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _eSpace         Input coordinate space system to use
 * @param[out]  _pvPos          Concerned position
 * @return orxVECTOR / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL       orxFrame_TransformPosition(orxFRAME *_pstFrame, orxFRAME_SPACE _eSpace, orxVECTOR *_pvPos);

/** Transforms a rotation given its input space (local -> global or global -> local)
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _eSpace         Input coordinate space system to use
 * @param[out]  _fRotation      Concerned rotation
 * @return Transformed rotation (radians)
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxFrame_TransformRotation(orxFRAME *_pstFrame, orxFRAME_SPACE _eSpace, orxFLOAT _fRotation);

/** Transforms a scale given its input space (local -> global or global -> local)
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _eSpace         Input coordinate space system to use
 * @param[out]  _pvScale        Concerned scale
 * @return      orxVECTOR / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL       orxFrame_TransformScale(orxFRAME *_pstFrame, orxFRAME_SPACE _eSpace, orxVECTOR *_pvScale);

#endif /* _orxFRAME_H_ */

/** @} */
