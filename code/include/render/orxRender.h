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
 * @file orxRender.h
 * @date 15/12/2003
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxRender
 *
 * Render plugin module
 * Renders visible objects on screen, using active cameras/viewports.
 *
 * @{
 */


#ifndef _orxRENDER_H_
#define _orxRENDER_H_


#include "orxInclude.h"
#include "plugin/orxPluginCore.h"

#include "display/orxDisplay.h"
#include "object/orxFrame.h"
#include "object/orxObject.h"
#include "render/orxViewport.h"


/** Misc defines
 */
#define orxRENDER_KZ_CONFIG_SECTION                 "Render"
#define orxRENDER_KZ_CONFIG_SHOW_FPS                "ShowFPS"
#define orxRENDER_KZ_CONFIG_SHOW_PROFILER           "ShowProfiler"
#define orxRENDER_KZ_CONFIG_MIN_FREQUENCY           "MinFrequency"
#define orxRENDER_KZ_CONFIG_CONSOLE_COLOR           "ConsoleColor"


/** Inputs
 */
#define orxRENDER_KZ_INPUT_SET                      "-=RenderSet=-"

#define orxRENDER_KZ_INPUT_PROFILER_TOGGLE_HISTORY  "ProfilerToggleHistory"
#define orxRENDER_KZ_INPUT_PROFILER_PAUSE           "ProfilerPause"
#define orxRENDER_KZ_INPUT_PROFILER_PREVIOUS_FRAME  "ProfilerPreviousFrame"
#define orxRENDER_KZ_INPUT_PROFILER_NEXT_FRAME      "ProfilerNextFrame"
#define orxRENDER_KZ_INPUT_PROFILER_PREVIOUS_DEPTH  "ProfilerPreviousDepth"
#define orxRENDER_KZ_INPUT_PROFILER_NEXT_DEPTH      "ProfilerNextDepth"
#define orxRENDER_KZ_INPUT_PROFILER_PREVIOUS_THREAD "ProfilerPreviousThread"
#define orxRENDER_KZ_INPUT_PROFILER_NEXT_THREAD     "ProfilerNextThread"


/** Event enum
 */
typedef enum __orxRENDER_EVENT_t
{
  orxRENDER_EVENT_START = 0,              /**< Event sent when rendering starts */
  orxRENDER_EVENT_STOP,                   /**< Event sent when rendering stops */
  orxRENDER_EVENT_VIEWPORT_START,         /**< Event sent when a viewport rendering starts */
  orxRENDER_EVENT_VIEWPORT_STOP,          /**< Event sent when a viewport rendering stops */
  orxRENDER_EVENT_OBJECT_START,           /**< Event sent when an object rendering starts */
  orxRENDER_EVENT_OBJECT_STOP,            /**< Event sent when an object rendering stops */

  orxRENDER_EVENT_NUMBER,

  orxRENDER_EVENT_NONE = orxENUM_NONE

} orxRENDER_EVENT;

/** Event payload
 */
typedef struct __orxRENDER_EVENT_PAYLOAD_t
{
  struct
  {
    orxDISPLAY_TRANSFORM *pstTransform;   /**< Object display transform : 4 / 8 */
  } stObject;

} orxRENDER_EVENT_PAYLOAD;


/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/** Render module setup
 */
extern orxDLLAPI void orxFASTCALL             orxRender_Setup();


/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

/** Inits the render module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxRender_Init();

/** Exits from the render module
 */
extern orxDLLAPI void orxFASTCALL             orxRender_Exit();

/** Get a world position given a screen one (absolute picking)
 * @param[in]   _pvScreenPosition                     Concerned screen position
 * @param[in]   _pstViewport                          Concerned viewport, if orxNULL then either the last viewport that contains the position (if any), or the last viewport with a camera in the list if none contains the position
 * @param[out]  _pvWorldPosition                      Corresponding world position
 * @return      orxVECTOR if found *inside* the display surface, orxNULL otherwise
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL       orxRender_GetWorldPosition(const orxVECTOR *_pvScreenPosition, const orxVIEWPORT *_pstViewport, orxVECTOR *_pvWorldPosition);

/** Get a screen position given a world one and a viewport (rendering position)
 * @param[in]   _pvWorldPosition                      Concerned world position
 * @param[in]   _pstViewport                          Concerned viewport, if orxNULL then the last viewport with a camera will be used
 * @param[out]  _pvScreenPosition                     Corresponding screen position
 * @return      orxVECTOR if found (can be off-screen), orxNULL otherwise
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL       orxRender_GetScreenPosition(const orxVECTOR *_pvWorldPosition, const orxVIEWPORT *_pstViewport, orxVECTOR *_pvScreenPosition);

#endif /* _orxRENDER_H_ */

/** @} */
