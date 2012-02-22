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
 * @file orxAndroidNativeSupport.c
 * @date 26/06/2011
 * @author simons.philippe@gmail.com
 *
 * Android native support
 *
 */

#include <android/log.h>
#include <android/native_activity.h>
#include <android/sensor.h>
#include <pthread.h>

#include "orxInclude.h"
#include "orxKernel.h"

#define  LOG_TAG    "orxAndroidNativeSupport.c"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static const char  *zOrxCommandLineKey = "orx.cmd_line";

ANativeActivity* orxAndroid_GetNativeActivity();

static jobject             oWakeLock;
static JavaVM*             s_vm = NULL;
static pthread_key_t       s_jniEnvKey = 0;

orxU32                     u32NbParams;
orxSTRING                 *azParams;

void orxAndroid_SetJavaVM(JavaVM* vm)
{
	s_vm = vm;
}

JNIEnv* orxAndroid_GetCurrentJNIEnv()
{
    JNIEnv* env = NULL;
    if (s_jniEnvKey)
	{
		env = (JNIEnv*)pthread_getspecific(s_jniEnvKey);
	}
	else
	{
		pthread_key_create(&s_jniEnvKey, NULL);
	}

	if (!env)
	{
		// do we have a VM cached?
		if (!s_vm)
		{
			LOGE("Error - could not find JVM!");
			return NULL;
		}

		// Hmm - no env for this thread cached yet
		int error = (*s_vm)->AttachCurrentThread(s_vm, &env, NULL);
		LOGI("AttachCurrentThread: %d, 0x%p", error, env);
		if (error || !env)
		{
			LOGE("Error - could not attach thread to JVM!");
			return NULL;
		}

		pthread_setspecific(s_jniEnvKey, env);
	}

	return env;
}


void orxAndroid_DetachThread()
{
  (*s_vm)->DetachCurrentThread(s_vm);
}

void orxAndroid_AcquireWakeLock()
{
        ANativeActivity *activity = orxAndroid_GetNativeActivity();
        JNIEnv *poJEnv = orxAndroid_GetCurrentJNIEnv();

        /* Acquires wake lock */
        jclass contextClass = (*poJEnv)->FindClass(poJEnv, "android/content/Context");
        orxASSERT(contextClass != orxNULL);
        
        jmethodID getSystemService = (*poJEnv)->GetMethodID(poJEnv, contextClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
        orxASSERT(getSystemService != orxNULL);
        jfieldID POWER_SERVICE_ID = (*poJEnv)->GetStaticFieldID(poJEnv, contextClass, "POWER_SERVICE", "Ljava/lang/String;");
        orxASSERT(POWER_SERVICE_ID != orxNULL);
        jstring POWER_SERVICE = (*poJEnv)->GetStaticObjectField(poJEnv, contextClass, POWER_SERVICE_ID);
        orxASSERT(POWER_SERVICE != orxNULL);
        
        /* Retrieves the PowerManager service and wakelock */
        jobject powerManager = (*poJEnv)->CallObjectMethod(poJEnv, activity->clazz, getSystemService, POWER_SERVICE);
        orxASSERT(powerManager != orxNULL);
        jclass powerManagerClass = (*poJEnv)->FindClass(poJEnv, "android/os/PowerManager");
        orxASSERT(powerManagerClass != orxNULL);
        jmethodID newWakeLock = (*poJEnv)->GetMethodID(poJEnv, powerManagerClass, "newWakeLock", "(ILjava/lang/String;)Landroid/os/PowerManager$WakeLock;");
        orxASSERT(newWakeLock != orxNULL);
        jfieldID SCREEN_BRIGHT_WAKE_LOCK_ID = (*poJEnv)->GetStaticFieldID(poJEnv, powerManagerClass, "SCREEN_BRIGHT_WAKE_LOCK", "I");
        orxASSERT(SCREEN_BRIGHT_WAKE_LOCK_ID != orxNULL);
        jfieldID ON_AFTER_RELEASE_ID = (*poJEnv)->GetStaticFieldID(poJEnv, powerManagerClass, "ON_AFTER_RELEASE", "I");
        orxASSERT(ON_AFTER_RELEASE_ID != orxNULL);
        jint SCREEN_BRIGHT_WAKE_LOCK = (*poJEnv)->GetStaticIntField(poJEnv, contextClass, SCREEN_BRIGHT_WAKE_LOCK_ID);
        jint ON_AFTER_RELEASE = (*poJEnv)->GetStaticIntField(poJEnv, contextClass, ON_AFTER_RELEASE_ID);
        jstring tag = (*poJEnv)->NewStringUTF(poJEnv, "orx");
        orxASSERT(tag != orxNULL);
        
        jobject wakelock = (*poJEnv)->CallObjectMethod(poJEnv, powerManager, newWakeLock, SCREEN_BRIGHT_WAKE_LOCK | ON_AFTER_RELEASE, tag);
        orxASSERT(wakelock != orxNULL);
        /* keep a global reference to release on Exit */
        oWakeLock = (*poJEnv)->NewGlobalRef(poJEnv, wakelock);

        /* acquire wake lock */
        jclass wakeLockClass = (*poJEnv)->GetObjectClass(poJEnv, wakelock);
        orxASSERT(wakeLockClass != orxNULL);
        jmethodID acquire = (*poJEnv)->GetMethodID(poJEnv, wakeLockClass, "acquire", "()V");
        orxASSERT(acquire != orxNULL);
        (*poJEnv)->CallVoidMethod(poJEnv, wakelock, acquire);
        
        jthrowable exc = (*poJEnv)->ExceptionOccurred(poJEnv);
        if(exc)
        {
          orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "java.lang.SecurityException: android.permission.WAKE_LOCK missing");
          (*poJEnv)->ExceptionDescribe(poJEnv);
          (*poJEnv)->ExceptionClear(poJEnv);
        }
}

void orxAndroid_ReleaseWakeLock()
{
        JNIEnv *poJEnv = orxAndroid_GetCurrentJNIEnv();

        /* acquire wake lock */
        jclass wakeLockClass = (*poJEnv)->GetObjectClass(poJEnv, oWakeLock);
        orxASSERT(wakeLockClass != orxNULL);
        jmethodID release = (*poJEnv)->GetMethodID(poJEnv, wakeLockClass, "release", "()V");
        orxASSERT(release != orxNULL);
        (*poJEnv)->CallVoidMethod(poJEnv, oWakeLock, release);

        jthrowable exc = (*poJEnv)->ExceptionOccurred(poJEnv);
        if(exc)
        {
          orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "java.lang.SecurityException: android.permission.WAKE_LOCK missing");
          (*poJEnv)->ExceptionDescribe(poJEnv);
          (*poJEnv)->ExceptionClear(poJEnv);
        }
        
        /* delete global reference */
        (*poJEnv)->DeleteGlobalRef(poJEnv, oWakeLock);
        oWakeLock = orxNULL;
}

#define SKIP(p) while (*p && isspace (*p)) p++
#define WANT(p) *p && !isspace (*p)

/* Count the number of arguments. */

static int count_args (const char * input)
{
    const char * p;
    int argc = 0;
    p = input;
    while (*p) {
        SKIP (p);
        if (WANT (p)) {
            argc++;
            while (WANT (p))
                p++;
        }
    }
    return argc;
}

/* Copy each non-whitespace argument into its own allocated space. */

static int copy_args (const char * input, int argc, char ** argv)
{
    int i = 0;
    const char *p;

    p = input;
    while (*p) {
        SKIP (p);
        if (WANT (p)) {
            const char * end = p;
            char * copy;
            while (WANT (end))
                end++;
            copy = argv[i] = malloc (end - p + 1);
            if (! argv[i])
                return -1;
            while (WANT (p))
                *copy++ = *p++;
            *copy = 0;
            i++;
        }
    }
    if (i != argc) 
        return -1;
    return 0;
}

#undef SKIP
#undef WANT

static int argc_argv (const char * input, int * argc_ptr, char *** argv_ptr)
{
    int argc;
    char ** argv;

    argc = count_args (input);
    if (argc == 0)
        return -1;
    argv = malloc (sizeof (char *) * argc);
    if (! argv)
        return -1;
    if (copy_args (input, argc, argv) == -1)
        return -1;
    *argc_ptr = argc;
    *argv_ptr = argv;
    return 0;
}

void orxAndroid_GetMainArgs()
{
        ANativeActivity *activity = orxAndroid_GetNativeActivity();
        JNIEnv *poJEnv = orxAndroid_GetCurrentJNIEnv();

	jclass activityClass = (*poJEnv)->GetObjectClass(poJEnv, activity->clazz);
	orxASSERT(activityClass != orxNULL);

	/* retrieve intent */
	jmethodID getIntent = (*poJEnv)->GetMethodID(poJEnv, activityClass, "getIntent", "()Landroid/content/Intent;");
	orxASSERT(getIntent != orxNULL);
	jobject intent = (*poJEnv)->CallObjectMethod(poJEnv, activity->clazz, getIntent);
	orxASSERT(intent != orxNULL);
	jclass intentClass = (*poJEnv)->GetObjectClass(poJEnv, intent);
	orxASSERT(intentClass != orxNULL);
	
	/* retrieve componentName */
	jmethodID getComponent = (*poJEnv)->GetMethodID(poJEnv, intentClass, "getComponent", "()Landroid/content/ComponentName;");
	orxASSERT(getComponent != orxNULL);
	jobject componentName = (*poJEnv)->CallObjectMethod(poJEnv, intent, getComponent);
	orxASSERT(componentName != orxNULL);
	
	/* retrieve PackageManager */
	jmethodID getPackageManager = (*poJEnv)->GetMethodID(poJEnv, activityClass, "getPackageManager", "()Landroid/content/pm/PackageManager;");
	orxASSERT(getPackageManager != orxNULL);
	jobject packageManager = (*poJEnv)->CallObjectMethod(poJEnv, activity->clazz, getPackageManager);
	orxASSERT(packageManager != orxNULL);
	jclass packageManagerClass = (*poJEnv)->GetObjectClass(poJEnv, packageManager);
	orxASSERT(packageManagerClass != orxNULL);
	
	/* retrieve GET_META_DATA static field */
	jclass abstractPackageManagerClass = (*poJEnv)->FindClass(poJEnv, "android/content/pm/PackageManager");
	orxASSERT(abstractPackageManagerClass != orxNULL);
	jfieldID GET_META_DATA_ID = (*poJEnv)->GetStaticFieldID(poJEnv, abstractPackageManagerClass, "GET_META_DATA", "I");
	orxASSERT(GET_META_DATA_ID != orxNULL);
	jint GET_META_DATA = (*poJEnv)->GetStaticIntField(poJEnv, abstractPackageManagerClass, GET_META_DATA_ID);
	
	/* retrive ActivityInfo */
	jmethodID getActivityInfo = (*poJEnv)->GetMethodID(poJEnv, packageManagerClass, "getActivityInfo", "(Landroid/content/ComponentName;I)Landroid/content/pm/ActivityInfo;");
	orxASSERT(getActivityInfo != orxNULL);
	jobject activityInfo = (*poJEnv)->CallObjectMethod(poJEnv, packageManager, getActivityInfo, componentName, GET_META_DATA);
	orxASSERT(activityInfo != orxNULL);
	jclass activityInfoClass = (*poJEnv)->GetObjectClass(poJEnv, activityInfo);
	orxASSERT(activityInfoClass != orxNULL);
	
	/* retrieve metaData Bundle */
	jfieldID metaDataID = (*poJEnv)->GetFieldID(poJEnv, activityInfoClass, "metaData", "Landroid/os/Bundle;");
	orxASSERT(metaDataID != orxNULL);
	jobject metaData = (*poJEnv)->GetObjectField(poJEnv, activityInfo, metaDataID);
	orxASSERT(metaData != orxNULL);
	jclass metaDataClass = (*poJEnv)->GetObjectClass(poJEnv, metaData);
	orxASSERT(metaDataClass != orxNULL);
	
	/* retrieve cmd_line String */
	jmethodID getString = (*poJEnv)->GetMethodID(poJEnv, metaDataClass, "getString", "(Ljava/lang/String;)Ljava/lang/String;");
	orxASSERT(getString != orxNULL);
	jstring key = (*poJEnv)->NewStringUTF(poJEnv, zOrxCommandLineKey);
	orxASSERT(key != orxNULL);
	jstring orx_cmd_line = (*poJEnv)->CallObjectMethod(poJEnv, metaData, getString, key);
	if(orx_cmd_line != orxNULL)
	{
    const char* cmd_line = (*poJEnv)->GetStringUTFChars(poJEnv, orx_cmd_line, NULL);
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "orx.cmd_line = %s", cmd_line);
    
    if (argc_argv (cmd_line, (int *) &u32NbParams, (char***) &azParams) == -1)
    {
      LOGE("Something went wrong.");
    }
    
    (*poJEnv)->ReleaseStringUTFChars(poJEnv, orx_cmd_line, cmd_line);
	}
	else
	{
	  u32NbParams = 0;
	  azParams = orxNULL;
	  LOGI("orx.cmd_line not defined");
	}
}

void orxAndroid_ReleaseMainArgs()
{
  int i;
  for(i = 0; i < u32NbParams; i++)
  {
    free(azParams[i]);
  }
  
  free(azParams);
}


