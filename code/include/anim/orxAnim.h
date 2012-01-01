/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2011 Orx-Project
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
  orxANIM_EVENT_CUSTOM_EVENT,                 /**< Event sent when a custom event is reached */

  orxANIM_EVENT_NUMBER,

  orxANIM_EVENT_NONE = orxENUM_NONE

} orxANIM_EVENT;

/** Anim custom event
 */
typedef struct __orxANIM_CUSTOM_EVENT_t
{
  orxFLOAT              fTime;                /**< Timestamp : 4 */
  orxFLOAT              fValue;               /**< Event value : 8 */
  const orxSTRING       zName;                /**< Event name : 12 */

} orxANIM_CUSTOM_EVENT;

/** Anim event payload
 */
typedef struct __orxANIM_EVENT_PAYLOAD_t
{
  orxANIM              *pstAnim;              /**< Animation reference : 4 */
  const orxSTRING       zAnimName;            /**< Animation name : 8 */
  orxANIM_CUSTOM_EVENT  stCustomEvent;        /**< Animation custom event : 20 */

} orxANIM_EVENT_PAYLOAD;


/** Anim channel type enum
 */
typedef enum __orxANIM_CHANNEL_ID_t
{
  orxANIM_CHANNEL_ID_GRAPHIC = 0,
  orxANIM_CHANNEL_ID_EVENT,

  orxANIM_CHANNEL_ID_NUMBER,

  orxANIM_CHANNEL_ID_NONE = orxENUM_NONE

} orxANIM_CHANNEL_ID;


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

/** Creates an animation from config
 * @param[in]   _zConfigID                    Config ID
 * @return      orxANIMSET / orxNULL
 */
extern orxDLLAPI orxANIM *orxFASTCALL         orxAnim_CreateFromConfig(const orxSTRING _zConfigID);

/** Deletes an animation
 * @param[in]   _pstAnim        Anim to delete
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxAnim_Delete(orxANIM *_pstAnim);


/** Adds a structure key to an animation channel
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _eChannelID     Concerned channel ID
 * @param[in]   _fTime          Timestamp for this key
 * @param[in]   _pData          Key data to add (its type depends on the channel type)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxAnim_AddKey(orxANIM *_pstAnim, orxANIM_CHANNEL_ID _eChannelID, orxFLOAT _fTime, void *_pData);

/** Removes last added key from an animation
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _eChannelID     Concerned channel ID
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxAnim_RemoveLastKey(orxANIM *_pstAnim, orxANIM_CHANNEL_ID _eChannelID);

/** Removes all keys from an animation
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _eChannelID     Concerned channel ID
 */
extern orxDLLAPI void orxFASTCALL             orxAnim_RemoveAllKeys(orxANIM *_pstAnim, orxANIM_CHANNEL_ID _eChannelID);


///** Gets next event after given timestamp
// * @param[in]   _pstAnim        Concerned animation
// * @param[in]   _fTimeStamp     Time stamp, excluded
// * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
// */
//extern orxDLLAPI const orxANIM_CUSTOM_EVENT *orxFASTCALL orxAnim_GetNextEvent(const orxANIM *_pstAnim, orxFLOAT _fTimeStamp);


/** Updates anim channel given a timestamp
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _eChannelID     Concerned channel ID
 * @param[in]   _fTimeStamp     TimeStamp for animation update
 * @param[out]  _pu32CurrentKey Current key as a result of update
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxAnim_Update(orxANIM *_pstAnim, orxANIM_CHANNEL_ID _eChannelID, orxFLOAT _fTimeStamp, orxU32 *_pu32CurrentKey);

/** Anim key data accessor
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _eChannelID     Concerned channel ID
 * @param[in]   _u32Index       Index of desired key data
 * @return      Desired key data / orxNULL
 */
extern orxDLLAPI void *orxFASTCALL            orxAnim_GetKeyData(const orxANIM *_pstAnim, orxANIM_CHANNEL_ID _eChannelID, orxU32 _u32Index);


/** Anim channel key storage size accessor
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _eChannelID     Concerned channel ID
 * @return      Anim channel key storage size
 */
extern orxDLLAPI orxU32 orxFASTCALL           orxAnim_GetKeyStorageSize(const orxANIM *_pstAnim, orxANIM_CHANNEL_ID _eChannelID);

/** Anim channel key counter accessor
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _eChannelID     Concerned channel ID
 * @return      Anim channel key counter
 */
extern orxDLLAPI orxU32 orxFASTCALL           orxAnim_GetKeyCounter(const orxANIM *_pstAnim, orxANIM_CHANNEL_ID _eChannelID);


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
