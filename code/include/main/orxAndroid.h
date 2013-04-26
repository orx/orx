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

#ifdef __orxANDROID__

#include <jni.h>
#include <pthread.h>
#include <stdlib.h>

#include <android/native_window.h>

#if defined(__cplusplus)
extern "C"
{
#endif

enum {
    /**
     * Looper data ID of commands coming from the app's main thread, which
     * is returned as an identifier from ALooper_pollOnce().  The data for this
     * identifier is a pointer to an android_poll_source structure.
     * These can be retrieved and processed with android_app_read_cmd()
     * and android_app_exec_cmd().
     */
    LOOPER_ID_MAIN = 1,
    LOOPER_ID_SENSOR = 2,
    LOOPER_ID_KEY_EVENT = 3,
    LOOPER_ID_TOUCH_EVENT = 4
};

enum {
    APP_CMD_PAUSE,
    APP_CMD_RESUME,
    APP_CMD_SURFACE_DESTROYED,
    APP_CMD_SURFACE_READY,
    APP_CMD_QUIT
};

ANativeWindow * orxAndroid_GetNativeWindow();

/**
  Get the internal storage path
  */
const char * orxAndroid_GetInternalStoragePath();

orxU32 orxAndroid_JNI_GetRotation();

/**
  Register APK resources IO
  */
orxSTATUS orxAndroid_RegisterAPKResource();

void orxAndroid_PumpEvents();

#if defined(__cplusplus)
}
#endif

#define orxANDROID_EVENT_KEYBOARD       0
#define orxANDROID_EVENT_KEYBOARD_DOWN  0
#define orxANDROID_EVENT_KEYBOARD_UP    1
#define KEYCODE_BACK 0x04
#define KEYCODE_MENU 0x52

#endif /* __orxANDROID__ */

#endif /* _orxANDROID_H_ */

/** @} */
