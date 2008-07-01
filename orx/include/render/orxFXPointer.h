/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/

/**
 * @file orxFXPointer.h
 * @date 30/06/2008
 * @author (C) Arcallians
 */

/**
 * @addtogroup Render
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


/** Internal FXPointer structure */
typedef struct __orxFXPOINTER_t                 orxFXPOINTER;


/** FXPointer module setup
 */
extern orxDLLAPI orxVOID                        orxFXPointer_Setup();

/** Inits the FXPointer module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS                      orxFXPointer_Init();

/** Exits from the FXPointer module
 */
extern orxDLLAPI orxVOID                        orxFXPointer_Exit();


/** Creates an empty FXPointer
 * @return orxFX / orxNULL
 */
extern orxDLLAPI orxFXPOINTER *                 orxFXPointer_Create();

/** Deletes an FXPointer
 * @param[in] _pstFXPointer     Concerned FX
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFXPointer_Delete(orxFXPOINTER *_pstFXPointer);

/** Enables/disables an FXPointer
 * @param[in]   _pstFXPointer   Concerned FXPointer
 * @param[in]   _bEnable        Enable / disable
 */
extern orxDLLAPI orxVOID orxFASTCALL            orxFXPointer_Enable(orxFXPOINTER *_pstFXPointer, orxBOOL _bEnable);

/** Is FXPointer enabled?
 * @param[in]   _pstFXPointer   Concerned FXPointer
 * @return      orxTRUE if enabled, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL            orxFXPointer_IsEnabled(orxCONST orxFXPOINTER *_pstFXPointer);


/** Adds an FX
 * @param[in]   _pstFXPointer Concerned FXPointer
 * @param[in]   _pstFX        FX to add
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFXPointer_AddFX(orxFXPOINTER *_pstFXPointer, orxFX *_pstFX);

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
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFXPointer_AddFXFromConfig(orxFXPOINTER *_pstFXPointer, orxCONST orxSTRING _zFXConfigID);

/** Removes an FX using using its config ID
 * @param[in]   _pstFXPointer Concerned FXPointer
 * @param[in]   _zFXConfigID  Config ID of the FX to remove
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFXPointer_RemoveFXFromConfig(orxFXPOINTER *_pstFXPointer, orxCONST orxSTRING _zFXConfigID);

#endif /* _orxFXPointer_H_ */

/** @} */
