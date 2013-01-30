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

#if defined(__cplusplus)
extern "C"
{
#endif

/**
  Retrieve the Java Activity
  */
jobject orxAndroid_GetActivity();

/**
  Register APK resources IO
  */
orxSTATUS orxAndroid_RegisterAPKResource();

/** @file thread.h
  The Thread library makes it easy to create native threads that can acess
  JNI objects.  By default, pthreads created in the Android NDK are NOT connected
  to the JVM and JNI calls will fail.  This library wraps thread creation in
  such a way that pthreads created using it will connect to and disconnect from
  the JVM as appropriate.  Applications creating all of their threads with these
  interfaces can use the provided ThreadGetCurrentJNIEnv() function to
  get the current thread's JNI context at any time.

  Note that native-created threads still have JNI limitations over threads
  that are calls down to native from Java.  The JNI function FindClass will
  NOT find application-specific classes when called from native threads.
  Native code that needs to call FindClass and record the indices of Java
  class members for later access must call FindClass and Get*FieldID/Get*MethodID
  in threads calling from Java, such as JNI_OnLoad
 */

/**
  Initializes the thread system by connecting it to the JVM.  This
  function must be called as early as possible in the native code's
  JNI_OnLoad function, so that the thread system is prepared for any
  JNI-dependent library initialization calls.  
  @param vm The VM pointer - should be the JavaVM pointer sent to JNI_OnLoad.
  */
void orxAndroid_ThreadInit(JavaVM* vm);

/**
  Retrieves the JNIEnv object associated with the current thread, allowing
  any thread that was creating with ThreadSpawnJNIThread() to access the
  JNI at will.  This JNIEnv is NOT usable across multiple calls or threads
  The function should be called in each function that requires a JNIEnv
  @return The current thread's JNIEnv, or NULL if the thread was not created
  by ThreadSpawnJNIThread
  @see ThreadSpawnJNIThread
  */
JNIEnv* orxAndroid_ThreadGetCurrentJNIEnv();

/**
  Spwans a new native thread that is registered for use with JNI.  Threads
  created with this function will have access to JNI data via the JNIEnv
  available from ThreadGetCurrentJNIEnv().
  @param thread is the same as in pthread_create
  @param attr is the same as in pthread_create
  @param start_routine is the same as in pthread_create
  @param arg is the same as in pthread_create
  @return 0 on success, -1 on failure
  @see ThreadGetCurrentJNIEnv
*/
int orxAndroid_ThreadSpawnJNIThread(pthread_t *thread, pthread_attr_t const * attr,
    void *(*start_routine)(void *), void * arg);

/**
  Sleeps the current thread for the specified number of milliseconds
  @param millisec Sleep time in ms
  @return 0 on success, -1 on failure
*/
int orxAndroid_ThreadSleep(unsigned long millisec);

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
