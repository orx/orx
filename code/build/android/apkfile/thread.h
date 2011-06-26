#ifndef __INCLUDED_THREAD_H
#define __INCLUDED_THREAD_H

#include <jni.h>
#include <pthread.h>
#include <stdlib.h>

#if defined(__cplusplus)
extern "C"
{
#endif

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
void ThreadInit(JavaVM* vm);

/**
  Retrieves the JNIEnv object associated with the current thread, allowing
  any thread that was creating with ThreadSpawnJNIThread() to access the
  JNI at will.  This JNIEnv is NOT usable across multiple calls or threads
  The function should be called in each function that requires a JNIEnv
  @return The current thread's JNIEnv, or NULL if the thread was not created
  by ThreadSpawnJNIThread
  @see ThreadSpawnJNIThread
  */
JNIEnv* ThreadGetCurrentJNIEnv();

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
int ThreadSpawnJNIThread(pthread_t *thread, pthread_attr_t const * attr,
    void *(*start_routine)(void *), void * arg);

/**
  Sleeps the current thread for the specified number of milliseconds
  @param millisec Sleep time in ms
  @return 0 on success, -1 on failure
*/
int ThreadSleep(unsigned long millisec);

#if defined(__cplusplus)
}
#endif

#endif
