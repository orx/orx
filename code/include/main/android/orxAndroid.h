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

#define KZ_CONFIG_ANDROID                     "Android"
#define KZ_CONFIG_SURFACE_SCALE               "SurfaceScale"
#define KZ_CONFIG_ACCELEROMETER_FREQUENCY     "AccelerometerFrequency"

#define orxEVENT_TYPE_ANDROID                 (orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + 0)

#include <android/native_window.h>

#define LOOPER_ID_SENSOR                      LOOPER_ID_USER

#if defined(__cplusplus)
extern "C"
{
#endif

/** Event enum
 */
typedef enum __orxANDROID_EVENT_t
{
  orxANDROID_EVENT_KEY_DOWN = 0,
  orxANDROID_EVENT_KEY_UP,
  orxANDROID_EVENT_SURFACE_CREATE,
  orxANDROID_EVENT_SURFACE_DESTROY,
  orxANDROID_EVENT_SURFACE_CHANGE,
  orxANDROID_EVENT_ACCELERATE,

  orxANDROID_EVENT_NUMBER,

  orxANDROID_EVENT_NONE = orxENUM_NONE

} orxANDROID_EVENT;

/** Event payload
 */
typedef struct __orxANDROID_EVENT_PAYLOAD_t
{
  union
  {
    struct
    {
      orxU32 u32KeyCode;
    } stKey;

    struct
    {
      orxU32 u32Width;
      orxU32 u32Height;
    } stSurface;
  };

} orxANDROID_EVENT_PAYLOAD;

/**
  Gets the window
  */
ANativeWindow *orxAndroid_GetNativeWindow();

/**
  Gets the internal storage path
  */
const char *orxAndroid_GetInternalStoragePath();

/**
  Gets the orientation
  */
orxU32 orxAndroid_JNI_GetRotation();

/**
  Gets the physical frame rate
  */
orxFLOAT orxAndroid_JNI_GetPhysicalFrameRate();

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

#endif /* _orxANDROID_H_ */

/** @} */
