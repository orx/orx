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
 * @file orxTimeLine.h
 * @date 22/04/2012
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxTimeLine
 *
 * TimeLine module
 * Allows to creates time lines: sequences of text events
 *
 * @{
 */


#ifndef _orxTIMELINE_H_
#define _orxTIMELINE_H_


#include "orxInclude.h"
#include "object/orxStructure.h"


/** Internal TimeLine structure
 */
typedef struct __orxTIMELINE_t                  orxTIMELINE;


/** Event enum
 */
typedef enum __orxTIMELINE_EVENT_t
{
  orxTIMELINE_EVENT_TRACK_START = 0,            /**< Event sent when a track starts */
  orxTIMELINE_EVENT_TRACK_STOP,                 /**< Event sent when a track stops */
  orxTIMELINE_EVENT_TRACK_ADD,                  /**< Event sent when a track is added */
  orxTIMELINE_EVENT_TRACK_REMOVE,               /**< Event sent when a track is removed */
  orxTIMELINE_EVENT_LOOP,                       /**< Event sent when a track is looping */
  orxTIMELINE_EVENT_TRIGGER,                    /**< Event sent when an event is triggered */

  orxTIMELINE_EVENT_NUMBER,

  orxTIMELINE_EVENT_NONE = orxENUM_NONE

} orxTIMELINE_EVENT;

/** TimeLine event payload
 */
typedef struct __orxTIMELINE_EVENT_PAYLOAD_t
{
  orxTIMELINE    *pstTimeLine;                  /**< TimeLine reference : 4 */
  const orxSTRING zTrackName;                   /**< Track name : 8 */
  const orxSTRING zEvent;                       /**< Event text : 12 */
  orxFLOAT        fTimeStamp;                   /**< Event time : 16 */

} orxTIMELINE_EVENT_PAYLOAD;


/** TimeLine module setup
 */
extern orxDLLAPI void orxFASTCALL               orxTimeLine_Setup();

/** Inits the TimeLine module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxTimeLine_Init();

/** Exits from the TimeLine module
 */
extern orxDLLAPI void orxFASTCALL               orxTimeLine_Exit();

/** Creates an empty TimeLine
 * @param[in]   _pstOwner                       TimeLine's owner used for event callbacks (usually an orxOBJECT)
 * @return orxTIMELINE / orxNULL
 */
extern orxDLLAPI orxTIMELINE *orxFASTCALL       orxTimeLine_Create(const orxSTRUCTURE *_pstOwner);

/** Deletes a TimeLine
 * @param[in] _pstTimeLine            Concerned TimeLine
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxTimeLine_Delete(orxTIMELINE *_pstTimeLine);

/** Gets a TimeLine owner
 * @param[in]   _pstTimeLine          Concerned TimeLine
 * @return      orxSTRUCTURE / orxNULL
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL      orxTimeLine_GetOwner(const orxTIMELINE *_pstTimeLine);

/** Enables/disables a TimeLine
 * @param[in]   _pstTimeLine          Concerned TimeLine
 * @param[in]   _bEnable              Enable / disable
 */
extern orxDLLAPI void orxFASTCALL               orxTimeLine_Enable(orxTIMELINE *_pstTimeLine, orxBOOL _bEnable);

/** Is TimeLine enabled?
 * @param[in]   _pstTimeLine          Concerned TimeLine
 * @return      orxTRUE if enabled, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL            orxTimeLine_IsEnabled(const orxTIMELINE *_pstTimeLine);

/** Adds a track to a TimeLine from config
 * @param[in]   _pstTimeLine          Concerned TimeLine
 * @param[in]   _zTrackID             Config ID
 * return       orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxTimeLine_AddTrackFromConfig(orxTIMELINE *_pstTimeLine, const orxSTRING _zTrackID);

/** Removes a track using its config ID
 * @param[in]   _pstTimeLine          Concerned TimeLine
 * @param[in]   _zTrackID             Config ID of the track to remove
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxTimeLine_RemoveTrackFromConfig(orxTIMELINE *_pstTimeLine, const orxSTRING _zTrackID);

/** Gets a track duration using its config ID
 * @param[in]   _zTrackID             Config ID of the concerned track
 * @return      Duration if found, -orxFLOAT_1 otherwise
 */
extern orxDLLAPI orxFLOAT orxFASTCALL           orxTimeLine_GetTrackDuration(const orxSTRING _zTrackID);

#endif /* _orxTIMELINE_H_ */

/** @} */
