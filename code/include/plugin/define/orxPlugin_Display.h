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
 * @file orxPlugin_Display.h
 * @date 23/04/2003
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxPlugin
 *
 * Header that defines all IDs of the display plugin
 *
 * @{
 */


#ifndef _orxPLUGIN_DISPLAY_H_
#define _orxPLUGIN_DISPLAY_H_

#include "plugin/define/orxPlugin_CoreID.h"


typedef enum __orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_t
{
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_INIT = 0,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_EXIT,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_SWAP,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_GET_SCREEN_BITMAP,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_GET_SCREEN_SIZE,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_CREATE_BITMAP,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_DELETE_BITMAP,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_LOAD_BITMAP,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_SAVE_BITMAP,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_SET_TEMP_BITMAP,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_GET_TEMP_BITMAP,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_SET_DESTINATION_BITMAPS,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_CLEAR_BITMAP,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_SET_BLEND_MODE,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_SET_BITMAP_CLIPPING,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_SET_BITMAP_DATA,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_GET_BITMAP_DATA,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_SET_PARTIAL_BITMAP_DATA,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_GET_BITMAP_SIZE,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_GET_BITMAP_ID,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_TRANSFORM_BITMAP,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_TRANSFORM_TEXT,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_DRAW_LINE,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_DRAW_POLYLINE,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_DRAW_POLYGON,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_DRAW_CIRCLE,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_DRAW_OBOX,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_DRAW_MESH,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_HAS_SHADER_SUPPORT,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_CREATE_SHADER,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_DELETE_SHADER,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_START_SHADER,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_STOP_SHADER,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_GET_PARAMETER_ID,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_SET_SHADER_BITMAP,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_SET_SHADER_FLOAT,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_SET_SHADER_VECTOR,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_GET_SHADER_ID,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_ENABLE_VSYNC,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_IS_VSYNC_ENABLED,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_SET_FULL_SCREEN,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_IS_FULL_SCREEN,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_GET_VIDEO_MODE_COUNT,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_GET_VIDEO_MODE,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_SET_VIDEO_MODE,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_IS_VIDEO_MODE_AVAILABLE,

  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_NUMBER,

  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_NONE = orxENUM_NONE

} orxPLUGIN_FUNCTION_BASE_ID_DISPLAY;

#endif /* _orxPLUGIN_DISPLAY_H_ */

/** @} */
