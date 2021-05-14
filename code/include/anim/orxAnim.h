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
 * @file orxAnim.h
 * @date 11/02/2004
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxAnim
 *
 * Animation (Data) Module.
 * Allows to creates and handle Animations data.
 * It consists of a structure containing data for a single animation
 * and functions for handling and accessing them.
 * Animations are structures.
 * They thus can be referenced by Animation Sets (orxAnimSet) Module.
 *
 * @{
 */


#ifndef _orxANIM_H_
#define _orxANIM_H_


#include "orxInclude.h"

#include "object/orxStructure.h"


/** Anim flags
 */
#define orxANIM_KU32_FLAG_NONE                0x00000000  /**< No flags */

#define orxANIM_KU32_FLAG_2D                  0x00000001  /**< 2D type animation ID flag */

#define orxANIM_KU32_MASK_USER_ALL            0x0000000F  /**< User all ID mask */

#define orxANIM_KU32_MASK_ALL                 0xFFFFFFFF  /**< All mask */

/** Anim defines
 */
#define orxANIM_KU32_KEY_MAX_NUMBER           65535       /**< Maximum number of keys for an animation */
#define orxANIM_KU32_EVENT_MAX_NUMBER         65535       /**< Maximum number of events for an animation */


/** Internal Anim structure
 */
typedef struct __orxANIM_t                    orxANIM;


/** Event enum
 */
typedef enum __orxANIM_EVENT_t
{
  orxANIM_EVENT_START = 0,                    /**< Event sent when an animation starts */
  orxANIM_EVENT_STOP,                         /**< Event sent when an animation stops */
  orxANIM_EVENT_CUT,                          /**< Event sent when an animation is cut */
  orxANIM_EVENT_LOOP,                         /**< Event sent when an animation has looped */
  orxANIM_EVENT_UPDATE,                       /**< Event sent when an animation has been updated (current key) */
  orxANIM_EVENT_CUSTOM_EVENT,                 /**< Event sent when a custom event is reached */

  orxANIM_EVENT_NUMBER,

  orxANIM_EVENT_NONE = orxENUM_NONE

} orxANIM_EVENT;

/** Anim event payload
 */
typedef struct __orxANIM_EVENT_PAYLOAD_t
{
  orxANIM            *pstAnim;                /**< Animation reference : 4 */
  const orxSTRING     zAnimName;              /**< Animation name : 8 */

  union
  {
    /* Loop event */
    struct
    {
      orxU32          u32Count;               /**< Loop count : 12 */
    } stLoop;

    /* Cut event */
    struct
    {
      orxFLOAT        fTime;                  /**< Anim time when cut: 12 */
    } stCut;

    /* Custom event */
    struct
    {
      const orxSTRING zName;                  /**< Custom event name : 12 */
      orxFLOAT        fValue;                 /**< Custom event value : 16 */
      orxFLOAT        fTime;                  /**< Custom event time : 20 */
    } stCustom;
  };

} orxANIM_EVENT_PAYLOAD;

/** Anim custom event
 */
typedef struct __orxANIM_CUSTOM_EVENT_t
{
  const orxSTRING zName;                      /**< Event name : 4 */
  orxFLOAT        fValue;                     /**< Event value : 8 */
  orxFLOAT        fTimeStamp;                 /**< Timestamp : 12 */

} orxANIM_CUSTOM_EVENT;


/** Anim module setup
 */
extern orxDLLAPI void orxFASTCALL             orxAnim_Setup();

/** Inits the Anim module
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxAnim_Init();

/** Exits from the Anim module
 */
extern orxDLLAPI void orxFASTCALL             orxAnim_Exit();


/** Creates an empty animation
 * @param[in]   _u32Flags       Flags for created animation
 * @param[in]   _u32KeyNumber   Number of keys for this animation
 * @param[in]   _u32EventNumber Number of events for this animation
 * @return      Created orxANIM / orxNULL
 */
extern orxDLLAPI orxANIM *orxFASTCALL         orxAnim_Create(orxU32 _u32Flags, orxU32 _u32KeyNumber, orxU32 _u32EventNumber);

/** Deletes an animation
 * @param[in]   _pstAnim        Anim to delete
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxAnim_Delete(orxANIM *_pstAnim);


/** Adds a key to an animation
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _pstData        Key data to add
 * @param[in]   _fTimeStamp     Timestamp for this key
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxAnim_AddKey(orxANIM *_pstAnim, orxSTRUCTURE *_pstData, orxFLOAT _fTimeStamp);

/** Removes last added key from an animation
 * @param[in]   _pstAnim        Concerned animation
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxAnim_RemoveLastKey(orxANIM *_pstAnim);

/** Removes all keys from an animation
 * @param[in]   _pstAnim        Concerned animation
 */
extern orxDLLAPI void orxFASTCALL             orxAnim_RemoveAllKeys(orxANIM *_pstAnim);


/** Adds an event to an animation
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _zEventName     Event name to add
 * @param[in]   _fTimeStamp     Timestamp for this event
 * @param[in]   _fValue         Value for this event
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxAnim_AddEvent(orxANIM *_pstAnim, const orxSTRING _zEventName, orxFLOAT _fTimeStamp, orxFLOAT _fValue);

/** Removes last added event from an animation
 * @param[in]   _pstAnim        Concerned animation
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxAnim_RemoveLastEvent(orxANIM *_pstAnim);

/** Removes all events from an animation
 * @param[in]   _pstAnim        Concerned animation
 */
extern orxDLLAPI void orxFASTCALL             orxAnim_RemoveAllEvents(orxANIM *_pstAnim);

/** Gets next event after given timestamp
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _fTimeStamp     Time stamp, excluded
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI const orxANIM_CUSTOM_EVENT *orxFASTCALL orxAnim_GetNextEvent(const orxANIM *_pstAnim, orxFLOAT _fTimeStamp);


/** Gets animation's key index from a time stamp
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _fTimeStamp     TimeStamp of the desired animation key
 * @return      Animation key index / orxU32_UNDEFINED
 */
extern orxDLLAPI orxU32 orxFASTCALL           orxAnim_GetKey(const orxANIM *_pstAnim, orxFLOAT _fTimeStamp);

/** Anim key data accessor
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _u32Index       Index of desired key data
 * @return      Desired orxSTRUCTURE / orxNULL
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL    orxAnim_GetKeyData(const orxANIM *_pstAnim, orxU32 _u32Index);


/** Anim key storage size accessor
 * @param[in]   _pstAnim        Concerned animation
 * @return      Anim key storage size
 */
extern orxDLLAPI orxU32 orxFASTCALL           orxAnim_GetKeyStorageSize(const orxANIM *_pstAnim);

/** Anim key count accessor
 * @param[in]   _pstAnim        Concerned animation
 * @return      Anim key count
 */
extern orxDLLAPI orxU32 orxFASTCALL           orxAnim_GetKeyCount(const orxANIM *_pstAnim);


/** Anim event storage size accessor
 * @param[in]   _pstAnim        Concerned animation
 * @return      Anim event storage size
 */
extern orxDLLAPI orxU32 orxFASTCALL           orxAnim_GetEventStorageSize(const orxANIM *_pstAnim);

/** Anim event count accessor
 * @param[in]   _pstAnim        Concerned animation
 * @return      Anim event count
 */
extern orxDLLAPI orxU32 orxFASTCALL           orxAnim_GetEventCount(const orxANIM *_pstAnim);


/** Anim time length accessor
 * @param[in]   _pstAnim        Concerned animation
 * @return      Anim time length
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxAnim_GetLength(const orxANIM *_pstAnim);

/** Anim name get accessor
 * @param[in]   _pstAnim        Concerned animation
 * @return      orxSTRING / orxSTRING_EMPTY
 */
extern orxDLLAPI const orxSTRING orxFASTCALL  orxAnim_GetName(const orxANIM *_pstAnim);

#endif /* _orxANIM_H_ */

/** @} */
