/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008- Orx-Project
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
 * @file orxTrigger.h
 * @date 05/07/2024
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxTrigger
 *
 * Trigger module
 * Allows to creates triggers: chained sequence of events
 *
 * @{
 */


#ifndef _orxTRIGGER_H_
#define _orxTRIGGER_H_


#include "orxInclude.h"
#include "object/orxStructure.h"


/** Misc defines
 */
#define orxTRIGGER_KC_SEPARATOR                 ':'
#define orxTRIGGER_KC_STOP_MARKER               '!'


/** Internal Trigger structure
 */
typedef struct __orxTRIGGER_t                   orxTRIGGER;


/** Event enum
 */
typedef enum __orxTRIGGER_EVENT_t
{
  orxTRIGGER_EVENT_SET_ADD = 0,                 /**< Event sent when a set is added to a trigger */
  orxTRIGGER_EVENT_SET_REMOVE,                  /**< Event sent when a set is removed from a trigger */
  orxTRIGGER_EVENT_FIRE,                        /**< Event sent when a trigger fires */

  orxTRIGGER_EVENT_NUMBER,

  orxTRIGGER_EVENT_NONE = orxENUM_NONE

} orxTRIGGER_EVENT;

/** Trigger event payload
 */
typedef struct __orxTRIGGER_EVENT_PAYLOAD_t
{
  orxTRIGGER     *pstTrigger;                   /**< Trigger reference : 4 / 8 */
  const orxSTRING zSetName;                     /**< Set name : 8 / 16 */
  const orxSTRING zEvent;                       /**< Event : 12 / 24 */

} orxTRIGGER_EVENT_PAYLOAD;


/** Trigger module setup
 */
extern orxDLLAPI void orxFASTCALL               orxTrigger_Setup();

/** Inits the Trigger module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxTrigger_Init();

/** Exits from the Trigger module
 */
extern orxDLLAPI void orxFASTCALL               orxTrigger_Exit();

/** Creates an empty Trigger
 * @return orxTrigger / orxNULL
 */
extern orxDLLAPI orxTRIGGER *orxFASTCALL        orxTrigger_Create();

/** Deletes a Trigger
 * @param[in] _pstTrigger             Concerned Trigger
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxTrigger_Delete(orxTRIGGER *_pstTrigger);

/** Clears cache (if any Trigger is still in active use, it'll remain in memory until not referenced anymore)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxTrigger_ClearCache();

/** Enables/disables a Trigger
 * @param[in]   _pstTrigger           Concerned Trigger
 * @param[in]   _bEnable              Enable / disable
 */
extern orxDLLAPI void orxFASTCALL               orxTrigger_Enable(orxTRIGGER *_pstTrigger, orxBOOL _bEnable);

/** Is Trigger enabled?
 * @param[in]   _pstTrigger           Concerned Trigger
 * @return      orxTRUE if enabled, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL            orxTrigger_IsEnabled(const orxTRIGGER *_pstTrigger);

/** Adds a set to a Trigger from config
 * @param[in]   _pstTrigger           Concerned Trigger
 * @param[in]   _zConfigID            Config ID of the set to add
 * return       orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxTrigger_AddSetFromConfig(orxTRIGGER *_pstTrigger, const orxSTRING _zConfigID);

/** Removes a set from a Trigger using its config ID
 * @param[in]   _pstTrigger           Concerned Trigger
 * @param[in]   _zConfigID            Config ID of the set to remove
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxTrigger_RemoveSetFromConfig(orxTRIGGER *_pstTrigger, const orxSTRING _zConfigID);

/** Gets how many sets are defined in the trigger
 * @param[in]   _pstTrigger           Concerned Trigger
 * @return      Count of sets defined in the trigger
 */
extern orxDLLAPI orxU32 orxFASTCALL             orxTrigger_GetCount(const orxTRIGGER *_pstTrigger);

/** Fire a Trigger's event
 * @param[in]   _pstTrigger           Concerned Trigger
 * @param[in]   _zEvent               Event to fire
 * @param[in]   _azRefinementList     List of refinements for this event, unused if _u32Size == 0
 * @param[in]   _u32Size              Size of the refinement list, 0 for none
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxTrigger_Fire(orxTRIGGER *_pstTrigger, const orxSTRING _zEvent, const orxSTRING *_azRefinementList, orxU32 _u32Size);

#endif /* _orxTRIGGER_H_ */

/** @} */
