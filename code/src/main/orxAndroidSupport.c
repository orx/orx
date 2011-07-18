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
 * @file orxAndSupport.c
 * @date 26/06/2011
 * @author simons.philippe@gmail.com
 *
 * Android support
 *
 */

 
#include <jni.h>

#include "orxInclude.h"
#include "orxKernel.h"

static const char  *zOrxCommandLineKey = "android.orx.cmd_line";

/* Main function pointer */
orxMODULE_RUN_FUNCTION  spfnRun;
/* Global payload for step function */
orxSYSTEM_EVENT_PAYLOAD sstPayload;
/* Global java variables */
jobject  oActivity;
JNIEnv  *poJEnv;
/* Command line parameters (get from manifest) */
orxS32              s32NbParams;
orxSTRING          *azParams;

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

/* Get command line parameter from manifest */
void orxAndroid_GetMainArgs()
{
	jclass activityClass = (*poJEnv)->GetObjectClass(poJEnv, oActivity);
	orxASSERT(activityClass != orxNULL);

	/* retrieve intent */
	jmethodID getIntent = (*poJEnv)->GetMethodID(poJEnv, activityClass, "getIntent", "()Landroid/content/Intent;");
	orxASSERT(getIntent != orxNULL);
	jobject intent = (*poJEnv)->CallObjectMethod(poJEnv, oActivity, getIntent);
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
	jobject packageManager = (*poJEnv)->CallObjectMethod(poJEnv, oActivity, getPackageManager);
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
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "android.orx.cmd_line = %s", cmd_line);
    
    if (argc_argv (cmd_line, (int *) &s32NbParams, (char***) &azParams) == -1)
    {
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Something went wrong.");
    }
    
    (*poJEnv)->ReleaseStringUTFChars(poJEnv, orx_cmd_line, cmd_line);
	}
	else
	{
	  s32NbParams = 0;
	  azParams = orxNULL;
	  orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "android.orx.cmd_line not defined");
	}
}

/* release command line parameters */
void orxAndroid_ReleaseMainArgs()
{
  int i;
  for(i = 0; i < s32NbParams; i++)
  {
    free(azParams[i]);
  }
  
  free(azParams);
}