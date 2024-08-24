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
 * @file orxAndroidActivity.h
 * @date 26/06/2011
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxAndroid
 *
 * Android support module
 *
 * @{
 */


#ifndef _orxANDROIDACTIVITY_H_
#define _orxANDROIDACTIVITY_H_

#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <game-activity/GameActivity.h>
#include <paddleboat/paddleboat.h>

#include <swappy/swappyGL.h>
#include <swappy/swappyGL_extra.h>

#define orxANDROID_KU32_MAX_JOYSTICK_NUMBER   PADDLEBOAT_MAX_CONTROLLERS

#if defined(__cplusplus)
extern "C"
{
#endif

/**
  Gets Game Activity
  */
GameActivity *orxAndroid_GetGameActivity();

/**
  Set key filter callback
  */
void orxAndroid_SetKeyFilter(android_key_event_filter filter);

#if defined(__cplusplus)
}
#endif

#endif /* __orxANDROIDACTIVITY_H_ */

/** @} */
