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


/** Internal spawner structure */
typedef struct __orxSPAWNER_t                orxSPAWNER;


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
extern orxDLLAPI orxSPAWNER *                orxSpawner_Create();

/** Creates a spawner from config
 * @param[in]   _zConfigID    Config ID
 * @ return orxSPAWNER / orxNULL
 */
extern orxDLLAPI orxSPAWNER *orxFASTCALL     orxSpawner_CreateFromConfig(orxCONST orxSTRING _zConfigID);

/** Deletes a spawner
 * @param[in] _pstSpawner     Concerned spawner
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxSpawner_Delete(orxSPAWNER *_pstSpawner);

/** Enables/disables a spawner
 * @param[in]   _pstSpawner    Concerned spawner
 * @param[in]   _bEnable      Enable / disable
 */
extern orxDLLAPI orxVOID orxFASTCALL        orxSpawner_Enable(orxSPAWNER *_pstSpawner, orxBOOL _bEnable);

/** Is spawner enabled?
 * @param[in]   _pstSpawner    Concerned spawner
 * @return      orxTRUE if enabled, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL        orxSpawner_IsEnabled(orxCONST orxSPAWNER *_pstSpawner);


#endif /* _orxSPAWNER_H_ */

/** @} */
