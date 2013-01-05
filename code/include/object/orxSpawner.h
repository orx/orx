/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2013 Orx-Project
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


/** Spawner flags
 */
#define orxSPAWNER_KU32_FLAG_NONE                 0x00000000  /**< No flags */

#define orxSPAWNER_KU32_FLAG_AUTO_DELETE          0x00000001  /**< Auto delete flag */
#define orxSPAWNER_KU32_FLAG_AUTO_RESET           0x00000002  /**< Auto delete flag */
#define orxSPAWNER_KU32_FLAG_USE_ALPHA            0x00000004  /**< Use alpha flag */
#define orxSPAWNER_KU32_FLAG_USE_COLOR            0x00000008  /**< Use color flag */
#define orxSPAWNER_KU32_FLAG_USE_ROTATION         0x00000010  /**< Use rotation flag */
#define orxSPAWNER_KU32_FLAG_USE_SCALE            0x00000020  /**< Use scale flag */
#define orxSPAWNER_KU32_FLAG_USE_RELATIVE_SPEED   0x00000040  /**< Use relative speed flag */
#define orxSPAWNER_KU32_FLAG_USE_SELF_AS_PARENT   0x00000080  /**< Use self as parent flag */
#define orxSPAWNER_KU32_FLAG_CLEAN_ON_DELETE      0x00000100  /**< Clean on delete flag */

#define orxSPAWNER_KU32_MASK_USER_ALL             0x000000FF  /**< All mask */


/** Event enum
 */
typedef enum __orxSPAWNER_EVENT_t
{
  orxSPAWNER_EVENT_SPAWN = 0,
  orxSPAWNER_EVENT_CREATE,
  orxSPAWNER_EVENT_DELETE,
  orxSPAWNER_EVENT_RESET,
  orxSPAWNER_EVENT_EMPTY,
  orxSPAWNER_EVENT_WAVE_START,
  orxSPAWNER_EVENT_WAVE_STOP,

  orxSPAWNER_EVENT_NUMBER,

  orxSPAWNER_EVENT_NONE = orxENUM_NONE

} orxSPAWNER_EVENT;


/** Internal spawner structure */
typedef struct __orxSPAWNER_t               orxSPAWNER;


/** Spawner module setup
 */
extern orxDLLAPI void orxFASTCALL           orxSpawner_Setup();

/** Inits the spawner module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxSpawner_Init();

/** Exits from the spawner module
 */
extern orxDLLAPI void orxFASTCALL           orxSpawner_Exit();

/** Creates an empty spawner
 * @return orxSPAWNER / orxNULL
 */
extern orxDLLAPI orxSPAWNER *orxFASTCALL    orxSpawner_Create();

/** Creates a spawner from config
 * @param[in]   _zConfigID    Config ID
 * @ return orxSPAWNER / orxNULL
 */
extern orxDLLAPI orxSPAWNER *orxFASTCALL    orxSpawner_CreateFromConfig(const orxSTRING _zConfigID);

/** Deletes a spawner
 * @param[in] _pstSpawner       Concerned spawner
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxSpawner_Delete(orxSPAWNER *_pstSpawner);

/** Enables/disables a spawner
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[in]   _bEnable      Enable / disable
 */
extern orxDLLAPI void orxFASTCALL           orxSpawner_Enable(orxSPAWNER *_pstSpawner, orxBOOL _bEnable);

/** Is spawner enabled?
 * @param[in]   _pstSpawner     Concerned spawner
 * @return      orxTRUE if enabled, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL        orxSpawner_IsEnabled(const orxSPAWNER *_pstSpawner);


/** Resets (and disables) a spawner
 * @param[in]   _pstSpawner     Concerned spawner
 */
extern orxDLLAPI void orxFASTCALL           orxSpawner_Reset(orxSPAWNER *_pstSpawner);

/** Sets spawner total object limit
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[in]   _u32TotalObjectLimit Total object limit, 0 for unlimited
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxSpawner_SetTotalObjectLimit(orxSPAWNER *_pstSpawner, orxU32 _u32TotalObjectLimit);

/** Sets spawner active object limit
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[in]   _u32ActiveObjectLimit Active object limit, 0 for unlimited
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxSpawner_SetActiveObjectLimit(orxSPAWNER *_pstSpawner, orxU32 _u32ActiveObjectLimit);

/** Gets spawner total object limit
 * @param[in]   _pstSpawner     Concerned spawner
 * @return      Total object limit, 0 for unlimited
 */
extern orxDLLAPI orxU32 orxFASTCALL         orxSpawner_GetTotalObjectLimit(const orxSPAWNER *_pstSpawner);

/** Gets spawner active object limit
 * @param[in]   _pstSpawner     Concerned spawner
 * @return      Active object limit, 0 for unlimited
 */
extern orxDLLAPI orxU32 orxFASTCALL         orxSpawner_GetActiveObjectLimit(const orxSPAWNER *_pstSpawner);

/** Gets spawner total object counter
 * @param[in]   _pstSpawner     Concerned spawner
 * @return      Total object counter
 */
extern orxDLLAPI orxU32 orxFASTCALL         orxSpawner_GetTotalObjectCounter(const orxSPAWNER *_pstSpawner);

/** Gets spawner active object counter
 * @param[in]   _pstSpawner     Concerned spawner
 * @return      Active object counter
 */
extern orxDLLAPI orxU32 orxFASTCALL         orxSpawner_GetActiveObjectCounter(const orxSPAWNER *_pstSpawner);


/** Sets spawner wave size
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[in]   _u32WaveSize    Number of objects to spawn in a wave / 0 for deactivating wave mode
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxSpawner_SetWaveSize(orxSPAWNER *_pstSpawner, orxU32 _u32WaveSize);

/** Sets spawner wave delay
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[in]   _fWaveDelay     Delay between two waves / -1 for deactivating wave mode
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxSpawner_SetWaveDelay(orxSPAWNER *_pstSpawner, orxFLOAT _fWaveDelay);

/** Gets spawner wave size
 * @param[in]   _pstSpawner     Concerned spawner
 * @return      Number of objects spawned in a wave / 0 if not in wave mode
 */
extern orxDLLAPI orxU32 orxFASTCALL         orxSpawner_GetWaveSize(const orxSPAWNER *_pstSpawner);

/** Gets spawner wave delay
 * @param[in]   _pstSpawner     Concerned spawner
 * @return      Delay between two waves / -1 if not in wave mode
 */
extern orxDLLAPI orxFLOAT orxFASTCALL       orxSpawner_GetWaveDelay(const orxSPAWNER *_pstSpawner);

/** Gets spawner next wave delay
 * @param[in]   _pstSpawner     Concerned spawner
 * @return      Delay before next wave is spawned / -1 if not in wave mode
 */
extern orxDLLAPI orxFLOAT orxFASTCALL       orxSpawner_GetNextWaveDelay(const orxSPAWNER *_pstSpawner);


/** Sets spawner object speed
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[in]   _pvObjectSpeed  Speed to apply to every spawned object / orxNULL to not apply any speed
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxSpawner_SetObjectSpeed(orxSPAWNER *_pstSpawner, const orxVECTOR *_pvObjectSpeed);

/** Gets spawner object speed
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[in]   _pvObjectSpeed  Speed applied to every spawned object
 * @return      Speed applied to every spawned object / orxNULL if none is applied
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL     orxSpawner_GetObjectSpeed(const orxSPAWNER *_pstSpawner, orxVECTOR *_pvObjectSpeed);


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
extern orxDLLAPI orxFRAME *orxFASTCALL      orxSpawner_GetFrame(const orxSPAWNER *_pstSpawner);


/** Sets spawner position
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[in]   _pvPosition     Spawner position
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxSpawner_SetPosition(orxSPAWNER *_pstSpawner, const orxVECTOR *_pvPosition);

/** Sets spawner rotation
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[in]   _fRotation      Spawner rotation (radians)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxSpawner_SetRotation(orxSPAWNER *_pstSpawner, orxFLOAT _fRotation);

/** Sets spawner scale
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[in]   _pvScale        Spawner scale vector
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxSpawner_SetScale(orxSPAWNER *_pstSpawner, const orxVECTOR *_pvScale);

/** Get spawner position
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[out]  _pvPosition     Spawner position
 * @return      orxVECTOR / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL     orxSpawner_GetPosition(const orxSPAWNER *_pstSpawner, orxVECTOR *_pvPosition);

/** Get spawner world position
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[out]  _pvPosition     Spawner world position
 * @return      orxVECTOR / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL     orxSpawner_GetWorldPosition(const orxSPAWNER *_pstSpawner, orxVECTOR *_pvPosition);

/** Get spawner rotation
 * @param[in]   _pstSpawner     Concerned spawner
 * @return      orxFLOAT (radians)
 */
extern orxDLLAPI orxFLOAT orxFASTCALL       orxSpawner_GetRotation(const orxSPAWNER *_pstSpawner);

/** Get spawner world rotation
 * @param[in]   _pstSpawner     Concerned spawner
 * @return      orxFLOAT (radians)
 */
extern orxDLLAPI orxFLOAT orxFASTCALL       orxSpawner_GetWorldRotation(const orxSPAWNER *_pstSpawner);

/** Get spawner scale
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[out]  _pvScale        Spawner scale vector
 * @return      Scale vector
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL     orxSpawner_GetScale(const orxSPAWNER *_pstSpawner, orxVECTOR *_pvScale);

/** Gets spawner world scale
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[out]  _pvScale        Spawner world scale
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL     orxSpawner_GetWorldScale(const orxSPAWNER *_pstSpawner, orxVECTOR *_pvScale);


/** Sets an spawner parent
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[in]   _pParent        Parent structure to set (spawner, spawner, camera or frame) / orxNULL
 * @return      orsSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxSpawner_SetParent(orxSPAWNER *_pstSpawner, void *_pParent);


/** Sets owner for a spawner
 * @param[in]   _pstSpawner   Concerned spawner
 * @param[in]   _pOwner       Owner to set / orxNULL
 */
extern orxDLLAPI void orxFASTCALL           orxSpawner_SetOwner(orxSPAWNER *_pstSpawner, void *_pOwner);

/** Gets spawner's owner
 * @param[in]   _pstSpawner   Concerned object
 * @return      Owner / orxNULL
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL  orxSpawner_GetOwner(const orxSPAWNER *_pstSpawner);


/** Gets spawner name
 * @param[in]   _pstSpawner     Concerned spawner
 * @return      orxSTRING / orxSTRING_EMPTY
 */
extern orxDLLAPI const orxSTRING orxFASTCALL orxSpawner_GetName(const orxSPAWNER *_pstSpawner);

#endif /* _orxSPAWNER_H_ */

/** @} */
