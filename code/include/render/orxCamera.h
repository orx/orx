/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2015 Orx-Project
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
 * @file orxCamera.h
 * @date 10/12/2003
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxCamera
 *
 * Camera Module
 * Allows to creates and handle cameras
 * Camera are structures used to render graphic (2D/3D) objects
 * They thus can be referenced by other structures
 *
 * @{
 */


#ifndef _orxCAMERA_H_
#define _orxCAMERA_H_


#include "orxInclude.h"

#include "object/orxFrame.h"
#include "object/orxStructure.h"
#include "math/orxAABox.h"


/** Anim flags
 */
#define orxCAMERA_KU32_FLAG_NONE                        0x00000000 /**< No flags */

#define orxCAMERA_KU32_FLAG_2D                          0x00000001 /**< 2D flag */

#define orxCAMERA_KU32_MASK_USER_ALL                    0x000000FF /**< User all mask */


/** Misc
 */
#define orxCAMERA_KU32_GROUP_ID_NUMBER                  16


/** Internal camera structure
 */
typedef struct __orxCAMERA_t                            orxCAMERA;


/** Camera module setup
 */
extern orxDLLAPI void orxFASTCALL                       orxCamera_Setup();

/** Inits the Camera module
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                  orxCamera_Init();

/** Exits from the Camera module
 */
extern orxDLLAPI void orxFASTCALL                       orxCamera_Exit();


/** Creates a camera
 * @param[in]   _u32Flags       Camera flags (2D / ...)
 * @return      Created orxCAMERA / orxNULL
 */
extern orxDLLAPI orxCAMERA *orxFASTCALL                 orxCamera_Create(orxU32 _u32Flags);

/** Creates a camera from config
 * @param[in]   _zConfigID      Config ID
 * @ return orxCAMERA / orxNULL
 */
extern orxDLLAPI orxCAMERA *orxFASTCALL                 orxCamera_CreateFromConfig(const orxSTRING _zConfigID);

/** Deletes a camera
 * @param[in]   _pstCamera      Camera to delete
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                  orxCamera_Delete(orxCAMERA *_pstCamera);


/** Adds a group ID to a camera
 * @param[in] _pstCamera        Concerned camera
 * @param[in] _u32GroupID       ID of the group to add
 * @param[in] _bAddFirst        If true this group will be used *before* any already added ones, otherwise it'll be used *after* all of them
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                  orxCamera_AddGroupID(orxCAMERA *_pstCamera, orxU32 _u32GroupID, orxBOOL _bAddFirst);

/** Removes a group ID from a camera
 * @param[in] _pstCamera        Concerned camera
 * @param[in] _u32GroupID       ID of the group to remove
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                  orxCamera_RemoveGroupID(orxCAMERA *_pstCamera, orxU32 _u32GroupID);

/** Gets number of group IDs of camera
 * @param[in] _pstCamera        Concerned camera
 * @return Number of group IDs of this camera
 */
extern orxDLLAPI orxU32 orxFASTCALL                     orxCamera_GetGroupIDCounter(const orxCAMERA *_pstCamera);

/** Gets the group ID of a camera at the given index
 * @param[in] _pstCamera        Concerned camera
 * @param[in] _u32Index         Index of group ID
 * @return Group ID if index is valid, orxU32_UNDEFINED otherwise
 */
extern orxDLLAPI orxU32 orxFASTCALL                     orxCamera_GetGroupID(const orxCAMERA *_pstCamera, orxU32 _u32Index);


/** Sets camera frustum
 * @param[in]   _pstCamera      Concerned camera
 * @param[in]   _fWidth         Width of frustum
 * @param[in]   _fHeight        Height of frustum
 * @param[in]   _fNear          Near distance of frustum
 * @param[in]   _fFar           Far distance of frustum
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                  orxCamera_SetFrustum(orxCAMERA *_pstCamera, orxFLOAT _fWidth, orxFLOAT _fHeight, orxFLOAT _fNear, orxFLOAT _fFar);

/** Sets camera position
 * @param[in]   _pstCamera      Concerned camera
 * @param[in]   _pvPosition     Camera position
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                  orxCamera_SetPosition(orxCAMERA *_pstCamera, const orxVECTOR *_pvPosition);

/** Sets camera rotation
 * @param[in]   _pstCamera      Concerned camera
 * @param[in]   _fRotation      Camera rotation (radians)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                  orxCamera_SetRotation(orxCAMERA *_pstCamera, orxFLOAT _fRotation);

/** Sets camera zoom
 * @param[in]   _pstCamera      Concerned camera
 * @param[in]   _fZoom          Camera zoom
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                  orxCamera_SetZoom(orxCAMERA *_pstCamera, orxFLOAT _fZoom);

/** Gets camera frustum (3D box for 2D camera)
 * @param[in]   _pstCamera      Concerned camera
 * @param[out]  _pstFrustum    Frustum box
 * @return      Frustum orxAABOX
 */
extern orxDLLAPI orxAABOX *orxFASTCALL                  orxCamera_GetFrustum(const orxCAMERA *_pstCamera, orxAABOX *_pstFrustum);

/** Get camera position
 * @param[in]   _pstCamera      Concerned camera
 * @param[out]  _pvPosition     Camera position
 * @return      orxVECTOR
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL                 orxCamera_GetPosition(const orxCAMERA *_pstCamera, orxVECTOR *_pvPosition);

/** Get camera rotation
 * @param[in]   _pstCamera      Concerned camera
 * @return      Rotation value (radians)
 */
extern orxDLLAPI orxFLOAT orxFASTCALL                   orxCamera_GetRotation(const orxCAMERA *_pstCamera);

/** Get camera zoom
 * @param[in]   _pstCamera      Concerned camera
 * @return      Zoom value
 */
extern orxDLLAPI orxFLOAT orxFASTCALL                   orxCamera_GetZoom(const orxCAMERA *_pstCamera);

/** Gets camera config name
 * @param[in]   _pstCamera      Concerned camera
 * @return      orxSTRING / orxSTRING_EMPTY
 */
extern orxDLLAPI const orxSTRING orxFASTCALL            orxCamera_GetName(const orxCAMERA *_pstCamera);

/** Gets camera given its name
 * @param[in]   _zName          Camera name
 * @return      orxCAMERA / orxNULL
 */
extern orxDLLAPI orxCAMERA *orxFASTCALL                 orxCamera_Get(const orxSTRING _zName);


/** Gets camera frame
 * @param[in]   _pstCamera      Concerned camera
 * @return      orxFRAME
 */
extern orxDLLAPI orxFRAME *orxFASTCALL                  orxCamera_GetFrame(const orxCAMERA *_pstCamera);


/** Sets camera parent
 * @param[in]   _pstCamera      Concerned camera
 * @param[in]   _pParent        Parent structure to set (object, spawner, camera or frame) / orxNULL
 * @return      orsSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                  orxCamera_SetParent(orxCAMERA *_pstCamera, void *_pParent);

/** Gets camera parent
 * @param[in]   _pstCamera      Concerned camera
 * @return      Parent (object, spawner, camera or frame) / orxNULL
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL              orxCamera_GetParent(const orxCAMERA *_pstCamera);

#endif /* _orxCAMERA_H_ */

/** @} */
