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
 * @file orxSpawner.h
 * @date 06/09/2008
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxSpawner
 *
 * Spawner module
 * Allows to spawn orxSPAWNERS
 * Spawners derived from structures
 *
 * @{
 */


#ifndef _orxSPAWNER_H_
#define _orxSPAWNER_H_


#include "orxInclude.h"

#include "object/orxStructure.h"
#include "object/orxFrame.h"


/** Event enum
 */
typedef enum __orxSPAWNER_EVENT_t
{
  orxSPAWNER_EVENT_SPAWN = 0,
  orxSPAWNER_EVENT_DELETE,
  orxSPAWNER_EVENT_RESET,
  orxSPAWNER_EVENT_EMPTY,

  orxSPAWNER_EVENT_NUMBER,

  orxSPAWNER_EVENT_NONE = orxENUM_NONE

} orxSPAWNER_EVENT;


/** Internal spawner structure */
typedef struct __orxSPAWNER_t               orxSPAWNER;


/** Spawner module setup
 */
extern orxDLLAPI orxVOID                    orxSpawner_Setup();

/** Inits the spawner module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS                  orxSpawner_Init();

/** Exits from the spawner module
 */
extern orxDLLAPI orxVOID                    orxSpawner_Exit();

/** Creates an empty spawner
 * @return orxSPAWNER / orxNULL
 */
extern orxDLLAPI orxSPAWNER *               orxSpawner_Create();

/** Creates a spawner from config
 * @param[in]   _zConfigID    Config ID
 * @ return orxSPAWNER / orxNULL
 */
extern orxDLLAPI orxSPAWNER *orxFASTCALL    orxSpawner_CreateFromConfig(orxCONST orxSTRING _zConfigID);

/** Deletes a spawner
 * @param[in] _pstSpawner       Concerned spawner
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxSpawner_Delete(orxSPAWNER *_pstSpawner);

/** Enables/disables a spawner
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[in]   _bEnable      Enable / disable
 */
extern orxDLLAPI orxVOID orxFASTCALL        orxSpawner_Enable(orxSPAWNER *_pstSpawner, orxBOOL _bEnable);

/** Is spawner enabled?
 * @param[in]   _pstSpawner     Concerned spawner
 * @return      orxTRUE if enabled, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL        orxSpawner_IsEnabled(orxCONST orxSPAWNER *_pstSpawner);


/** Resets (and re-enables) a spawner
 * @param[in]   _pstSpawner     Concerned spawner
 */
extern orxDLLAPI orxVOID orxFASTCALL        orxSpawner_Reset(orxSPAWNER *_pstSpawner);


/** Spawns items
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[in]   _u32Number      Number of items to spawn
* @return      Number of spawned items
  */
extern orxDLLAPI orxU32 orxFASTCALL         orxSpawner_Spawn(orxSPAWNER *_pstSpawner, orxU32 _u32Number);


/** Gets spawner frame
 * @param[in]   _pstSpawner     Concerned spawner
 * @return      orxFRAME
 */
extern orxDLLAPI orxFRAME *orxFASTCALL      orxSpawner_GetFrame(orxCONST orxSPAWNER *_pstSpawner);


/** Sets spawner position
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[in]   _pvPosition     Spawner position
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxSpawner_SetPosition(orxSPAWNER *_pstSpawner, orxCONST orxVECTOR *_pvPosition);

/** Sets spawner rotation
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[in]   _fRotation      Spawner rotation
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxSpawner_SetRotation(orxSPAWNER *_pstSpawner, orxFLOAT _fRotation);

/** Sets spawner scale
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[in]   _pvScale        Spawner scale vector
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxSpawner_SetScale(orxSPAWNER *_pstSpawner, orxCONST orxVECTOR *_pvScale);

/** Get spawner position
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[out]  _pvPosition     Spawner position
 * @return      orxVECTOR / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL     orxSpawner_GetPosition(orxCONST orxSPAWNER *_pstSpawner, orxVECTOR *_pvPosition);

/** Get spawner world position
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[out]  _pvPosition     Spawner world position
 * @return      orxVECTOR / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL     orxSpawner_GetWorldPosition(orxCONST orxSPAWNER *_pstSpawner, orxVECTOR *_pvPosition);

/** Get spawner rotation
 * @param[in]   _pstSpawner     Concerned spawner
 * @return      orxFLOAT
 */
extern orxDLLAPI orxFLOAT orxFASTCALL       orxSpawner_GetRotation(orxCONST orxSPAWNER *_pstSpawner);

/** Get spawner world rotation
 * @param[in]   _pstSpawner     Concerned spawner
 * @return      orxFLOAT
 */
extern orxDLLAPI orxFLOAT orxFASTCALL       orxSpawner_GetWorldRotation(orxCONST orxSPAWNER *_pstSpawner);

/** Get spawner scale
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[out]  _pvScale        Spawner scale vector
 * @return      Scale vector
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL     orxSpawner_GetScale(orxCONST orxSPAWNER *_pstSpawner, orxVECTOR *_pvScale);

/** Gets spawner world scale
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[out]  _pvScale        Spawner world scale
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL     orxSpawner_GetWorldScale(orxCONST orxSPAWNER *_pstSpawner, orxVECTOR *_pvScale);


/** Sets an spawner parent
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[in]   _pParent        Parent structure to set (spawner, spawner, camera or frame) / orxNULL
 * @return      orsSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxSpawner_SetParent(orxSPAWNER *_pstSpawner, orxVOID *_pParent);


/** Gets spawner name
 * @param[in]   _pstSpawner     Concerned spawner
 * @return      orxSTRING / orxSTRING_EMPTY
 */
extern orxDLLAPI orxSTRING orxFASTCALL      orxSpawner_GetName(orxCONST orxSPAWNER *_pstSpawner);

#endif /* _orxSPAWNER_H_ */

/** @} */
