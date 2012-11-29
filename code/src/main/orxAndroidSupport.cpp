/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2012 Orx-Project
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

#if defined(__orxANDROID__)

#include <jni.h>
#include <android/log.h>

#define MODULE "orx"
#define DEBUG(args...) __android_log_print(ANDROID_LOG_DEBUG, MODULE, ## args)

#include "orxInclude.h"
#include "orxKernel.h"

#include "thread.h"
#include "file.h"

int32_t s_winWidth = 1;
int32_t s_winHeight = 1;

/* Main function pointer */
orxMODULE_RUN_FUNCTION  pfnRun;

static orxSTATUS               seClockStatus, seMainStatus;
static orxSYSTEM_EVENT_PAYLOAD sstPayload;

static void renderFrame()
{
  /* Clears payload */
  orxMemory_Zero(&sstPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));

  /* Sends frame start event */
  orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_GAME_LOOP_START, orxNULL, orxNULL, &sstPayload);

  /* Runs the engine */
  seMainStatus = pfnRun();

  /* Updates clock system */
  seClockStatus = orxClock_Update();

  /* Sends frame stop event */
  orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_GAME_LOOP_STOP, orxNULL, orxNULL, &sstPayload);

  /* Updates frame counter */
  sstPayload.u32FrameCounter++;
}

/** Should stop execution by default event handling?
 */
static orxBOOL sbStopByEvent = orxFALSE;

jobject oActivity;

/* Main function to call */
extern int main(int argc, char *argv[]);

extern "C" {

    jint JNI_OnLoad(JavaVM *vm, void *reserved)
    {
        ThreadInit(vm);
        FInit();
        return JNI_VERSION_1_4;
    }

    JNIEXPORT void JNICALL Java_org_orx_lib_OrxActivity_nativeInit(JNIEnv * env, jobject thiz) {
      oActivity = env->NewGlobalRef(thiz);
    }

    JNIEXPORT void JNICALL Java_org_orx_lib_OrxRenderer_nativeInit(JNIEnv* env, jobject thiz, jint width, jint height) {
        s_winWidth = width;
        s_winHeight = height;

        /* Call main function */
        main(0, orxNULL);
    }

    JNIEXPORT void JNICALL Java_org_orx_lib_OrxRenderer_nativeRender(JNIEnv* env, jobject thiz) {
        renderFrame();
    }

    JNIEXPORT void JNICALL Java_org_orx_lib_OrxRenderer_nativeOnPause(JNIEnv* env, jobject thiz) {
    }

    JNIEXPORT void JNICALL Java_org_orx_lib_OrxRenderer_nativeOnResume(JNIEnv* env, jobject thiz) {
    }

    JNIEXPORT void JNICALL Java_org_orx_lib_OrxRenderer_nativeTouchesBegin(JNIEnv * env, jobject thiz, jint id, jfloat x, jfloat y) {
    }

    JNIEXPORT void JNICALL Java_org_orx_lib_OrxRenderer_nativeTouchesEnd(JNIEnv * env, jobject thiz, jint id, jfloat x, jfloat y) {
    }

    JNIEXPORT void JNICALL Java_org_orx_lib_OrxRenderer_nativeTouchesMove(JNIEnv * env, jobject thiz, jintArray ids, jfloatArray xs, jfloatArray ys) {
        int size = env->GetArrayLength(ids);
        jint id[size];
        jfloat x[size];
        jfloat y[size];

        env->GetIntArrayRegion(ids, 0, size, id);
        env->GetFloatArrayRegion(xs, 0, size, x);
        env->GetFloatArrayRegion(ys, 0, size, y);
    }

    JNIEXPORT void JNICALL Java_org_orx_lib_OrxRenderer_nativeTouchesCancel(JNIEnv * env, jobject thiz, jintArray ids, jfloatArray xs, jfloatArray ys) {
        int size = env->GetArrayLength(ids);
        jint id[size];
        jfloat x[size];
        jfloat y[size];

        env->GetIntArrayRegion(ids, 0, size, id);
        env->GetFloatArrayRegion(xs, 0, size, x);
        env->GetFloatArrayRegion(ys, 0, size, y);
    }

    #define KEYCODE_BACK 0x04
    #define KEYCODE_MENU 0x52

    JNIEXPORT jboolean JNICALL Java_org_orx_lib_OrxRenderer_nativeKeyDown(JNIEnv * env, jobject thiz, jint keyCode) {
        return JNI_FALSE;
    }

    JNIEXPORT jboolean JNICALL Java_org_orx_lib_OrxRenderer_nativeKeyUp(JNIEnv * env, jobject thiz, jint keyCode) {
        return JNI_FALSE;
    }
}

#endif
