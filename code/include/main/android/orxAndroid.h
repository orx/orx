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
 * @file orxAndroid.h
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


#ifndef _orxANDROID_H_
#define _orxANDROID_H_

#include <jni.h>
#include <pthread.h>
#include <stdlib.h>

#define KZ_CONFIG_ANDROID                        "Android"
#define KZ_CONFIG_SURFACE_SCALE                  "SurfaceScale"
#define KZ_CONFIG_ACCELEROMETER_FREQUENCY        "AccelerometerFrequency"

#include <android/native_window.h>

#define LOOPER_ID_SENSOR                      LOOPER_ID_USER

#if defined(__cplusplus)
extern "C"
{
#endif

typedef struct __orxANDROID_KEY_EVENT_t {
  orxU32 u32Action;
  orxU32 u32KeyCode;

} orxANDROID_KEY_EVENT;

typedef struct __orxANDROID_SURFACE_CHANGED_EVENT_t {
  orxU32   u32Width;
  orxU32   u32Height;

} orxANDROID_SURFACE_CHANGED_EVENT;

/**
  Gets the window
  */
ANativeWindow *orxAndroid_GetNativeWindow();

/**
  Get the internal storage path
  */
const char *orxAndroid_GetInternalStoragePath();

/**
  Get the orientation
  */
orxU32 orxAndroid_JNI_GetRotation();

/**
  Register APK resources IO
  */
orxSTATUS orxAndroid_RegisterAPKResource();

/**
  Pumps events
  */
void orxAndroid_PumpEvents();

/**
  Sets up JNI thread
  */
orxSTATUS orxFASTCALL orxAndroid_JNI_SetupThread(void *_pContext);

#if defined(__cplusplus)
}
#endif

#define orxANDROID_EVENT_TYPE_KEYBOARD       (orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + 0)
#define orxANDROID_EVENT_KEYBOARD_DOWN       0
#define orxANDROID_EVENT_KEYBOARD_UP         1

#define orxANDROID_EVENT_TYPE_SURFACE        (orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + 1)
#define orxANDROID_EVENT_SURFACE_DESTROYED   0
#define orxANDROID_EVENT_SURFACE_CREATED     1
#define orxANDROID_EVENT_SURFACE_CHANGED     2

#define orxANDROID_EVENT_TYPE_ACCELERATE     (orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + 2)

#endif /* _orxANDROID_H_ */

/** @} */
