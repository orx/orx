/* Orx - Portable Game Engine
 *
 * Orx is the legal property of its developers, whose names
 * are listed in the COPYRIGHT file distributed 
 * with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * @file orxFXPointer.h
 * @date 30/06/2008
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxFXPointer
 *
 * FXPointer module
 * Allows to creates rendering special effects containers for objects.
 *
 * @{
 */


#ifndef _orxFXPOINTER_H_
#define _orxFXPOINTER_H_


#include "orxInclude.h"
#include "render/orxFX.h"
#include "object/orxStructure.h"


/** Misc defines
 */
#define orxFXPOINTER_KU32_FX_NUMBER             4


/** Internal FXPointer structure */
typedef struct __orxFXPOINTER_t                 orxFXPOINTER;


/** FXPointer module setup
 */
extern orxDLLAPI void orxFASTCALL               orxFXPointer_Setup();

/** Inits the FXPointer module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFXPointer_Init();

/** Exits from the FXPointer module
 */
extern orxDLLAPI void orxFASTCALL               orxFXPointer_Exit();


/** Creates an empty FXPointer
 * @param[in]   _pstOwner       FXPointer's owner used for event callbacks (usually an orxOBJECT)
 * @return orxFXPOINTER / orxNULL
 */
extern orxDLLAPI orxFXPOINTER *orxFASTCALL      orxFXPointer_Create(const orxSTRUCTURE *_pstOwner);

/** Deletes an FXPointer
 * @param[in] _pstFXPointer     Concerned FXPointer
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFXPointer_Delete(orxFXPOINTER *_pstFXPointer);

/** Gets an FXPointer owner
 * @param[in]   _pstFXPointer   Concerned FXPointer
 * @return      orxSTRUCTURE / orxNULL
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL      orxFXPointer_GetOwner(const orxFXPOINTER *_pstFXPointer);

/** Enables/disables an FXPointer
 * @param[in]   _pstFXPointer   Concerned FXPointer
 * @param[in]   _bEnable        Enable / disable
 */
extern orxDLLAPI void orxFASTCALL               orxFXPointer_Enable(orxFXPOINTER *_pstFXPointer, orxBOOL _bEnable);

/** Is FXPointer enabled?
 * @param[in]   _pstFXPointer   Concerned FXPointer
 * @return      orxTRUE if enabled, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL            orxFXPointer_IsEnabled(const orxFXPOINTER *_pstFXPointer);


/** Adds an FX
 * @param[in]   _pstFXPointer Concerned FXPointer
 * @param[in]   _pstFX        FX to add
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFXPointer_AddFX(orxFXPOINTER *_pstFXPointer, orxFX *_pstFX);

/** Adds a delayed FX
 * @param[in]   _pstFXPointer Concerned FXPointer
 * @param[in]   _pstFX        FX to add
 * @param[in]   _fDelay       Delay time
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFXPointer_AddDelayedFX(orxFXPOINTER *_pstFXPointer, orxFX *_pstFX, orxFLOAT _fDelay);

/** Removes an FX
 * @param[in]   _pstFXPointer Concerned FXPointer
 * @param[in]   _pstFX        FX to remove
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFXPointer_RemoveFX(orxFXPOINTER *_pstFXPointer, orxFX *_pstFX);

/** Adds an FX using its config ID
 * @param[in]   _pstFXPointer Concerned FXPointer
 * @param[in]   _zFXConfigID  Config ID of the FX to add
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFXPointer_AddFXFromConfig(orxFXPOINTER *_pstFXPointer, const orxSTRING _zFXConfigID);

/** Adds a unique FX using its config ID
 * @param[in]   _pstFXPointer Concerned FXPointer
 * @param[in]   _zFXConfigID  Config ID of the FX to add
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFXPointer_AddUniqueFXFromConfig(orxFXPOINTER *_pstFXPointer, const orxSTRING _zFXConfigID);

/** Adds a delayed FX using its config ID
 * @param[in]   _pstFXPointer Concerned FXPointer
 * @param[in]   _zFXConfigID  Config ID of the FX to add
 * @param[in]   _fDelay       Delay time
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFXPointer_AddDelayedFXFromConfig(orxFXPOINTER *_pstFXPointer, const orxSTRING _zFXConfigID, orxFLOAT _fDelay);

/** Adds a unique delayed FX using its config ID
 * @param[in]   _pstFXPointer Concerned FXPointer
 * @param[in]   _zFXConfigID  Config ID of the FX to add
 * @param[in]   _fDelay       Delay time
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFXPointer_AddUniqueDelayedFXFromConfig(orxFXPOINTER *_pstFXPointer, const orxSTRING _zFXConfigID, orxFLOAT _fDelay);

/** Removes an FX using using its config ID
 * @param[in]   _pstFXPointer Concerned FXPointer
 * @param[in]   _zFXConfigID  Config ID of the FX to remove
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFXPointer_RemoveFXFromConfig(orxFXPOINTER *_pstFXPointer, const orxSTRING _zFXConfigID);

#endif /* _orxFXPointer_H_ */

/** @} */
