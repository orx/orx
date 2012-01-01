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
#include <android/log.h>

#define MODULE "orx"
#define DEBUG(args...) __android_log_print(ANDROID_LOG_DEBUG, MODULE, ## args)

#include "orxInclude.h"
#include "orxKernel.h"

#include <nv_event/nv_event.h>
#include <nv_thread/nv_thread.h>
#include <nv_file/nv_file.h>

int32_t s_winWidth = 1;
int32_t s_winHeight = 1;
int s_displayRotation = -1;
static bool s_glesLoaded = true;

jobject oActivity;

#ifdef __orxDEBUG__
static unsigned int s_swapCount = 0;
#endif

/* Main function pointer */
orxMODULE_RUN_FUNCTION  pfnRun;

static orxSTATUS               seClockStatus, seMainStatus;
static orxSYSTEM_EVENT_PAYLOAD sstPayload;

int32_t NVEventAppInit(int32_t argc, char** argv)
{
  NvFInit();
  
  return 0;
}

/* Main function to call */
extern int main(int argc, char *argv[]);

int GetRotation()
{
  jint rotation;
  
  JNIEnv *poJEnv = NVThreadGetCurrentJNIEnv();
    
  jclass clsContext = poJEnv->FindClass("android/content/Context");
  orxASSERT(clsContext != orxNULL);
  jmethodID getSystemService = poJEnv->GetMethodID(clsContext, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
  orxASSERT(getSystemService != orxNULL);
  jfieldID WINDOW_SERVICE_ID = poJEnv->GetStaticFieldID(clsContext, "WINDOW_SERVICE", "Ljava/lang/String;");
  orxASSERT(WINDOW_SERVICE_ID != orxNULL);
  jstring WINDOW_SERVICE = (jstring) poJEnv->GetStaticObjectField(clsContext, WINDOW_SERVICE_ID);
  orxASSERT(WINDOW_SERVICE != orxNULL);
  jobject windowManager = poJEnv->CallObjectMethod(oActivity, getSystemService, WINDOW_SERVICE);
  orxASSERT(windowManager != orxNULL);
  jclass clsWindowManager = poJEnv->FindClass("android/view/WindowManager");
  orxASSERT(clsWindowManager != orxNULL);
  jmethodID getDefaultDisplay = poJEnv->GetMethodID(clsWindowManager, "getDefaultDisplay", "()Landroid/view/Display;");
  orxASSERT(getDefaultDisplay != orxNULL);
  jobject defaultDisplay = poJEnv->CallObjectMethod(windowManager, getDefaultDisplay);
  orxASSERT(defaultDisplay != orxNULL);
  jclass clsDisplay = poJEnv->FindClass("android/view/Display");
  orxASSERT(clsDisplay != orxNULL);
  jmethodID getRotation = poJEnv->GetMethodID(clsDisplay, "getRotation", "()I");
  if(getRotation != orxNULL)
  {
    orxDEBUG_PRINT(orxDEBUG_LEVEL_LOG, "getRotation() found");
    rotation =  poJEnv->CallIntMethod(defaultDisplay, getRotation);
  }
  else
  {
    orxDEBUG_PRINT(orxDEBUG_LEVEL_LOG, "getRotation() not found");
    jmethodID getOrientation = poJEnv->GetMethodID(clsDisplay, "getOrientation", "()I");
    orxASSERT(getOrientation != orxNULL);
    rotation =  poJEnv->CallIntMethod(defaultDisplay, getOrientation);
  }
  
  orxDEBUG_PRINT(orxDEBUG_LEVEL_LOG, "rotation = %d", (int) rotation);
  
  return (int) rotation;
}

int32_t NVEventAppMain(int32_t argc, char** argv)
{
#ifdef __orxDEBUG__
  s_swapCount = 0;
#endif

  s_winWidth = 1;
  s_winHeight = 1;
  oActivity = (jobject) NVEventGetPlatformAppHandle();

  /* loop until EGL is ready */

  while (NVEventStatusIsRunning() && !NVEventReadyToRenderEGL(true))
  {
    const NVEvent* ev = NULL;
    while (NVEventStatusIsRunning() && (ev = NVEventGetNextEvent(NVEventStatusIsFocused() ? 1 : 100)))
    {
      if(ev->m_type == NV_EVENT_SURFACE_SIZE)
      {
        DEBUG( "Surface create/resize event: %d x %d", s_winWidth, s_winHeight);

        s_winWidth = ev->m_data.m_size.m_w;
        s_winHeight = ev->m_data.m_size.m_h;
      }
      if (ev)
        NVEventDoneWithEvent(true);
    }
  }
  
  DEBUG("EGL ready !!!");

  /* Call main function */
  main(argc, argv);

  DEBUG("cleanup!!!");

  NVEventCleanupEGL();

  return 0;
}

static bool renderFrame(bool allocateIfNeeded)
{
  if (!NVEventReadyToRenderEGL(allocateIfNeeded))
      return false;

    if (!s_glesLoaded)
    {
        if (!allocateIfNeeded)
            return false;

        orxEvent_SendShort(orxEVENT_TYPE_DISPLAY, orxDISPLAY_EVENT_RESTORE_CONTEXT);
        s_glesLoaded = true;
    }

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

#ifdef __orxDEBUG__
  // A debug printout every 256 frames so we can see when we're
  // actively rendering and swapping
  if (!(s_swapCount++ & 0x00ff))
  {
    orxDEBUG_PRINT(orxDEBUG_LEVEL_LOG, "Swap count is %d", s_swapCount);
  }
#endif

  return true;
}

/** Should stop execution by default event handling?
 */
static orxBOOL sbStopByEvent = orxFALSE;

/** Orx default basic event handler
 * @param[in]   _pstEvent                     Sent event
 * @return      orxSTATUS_SUCCESS if handled / orxSTATUS_FAILURE otherwise
 */
static orxSTATUS orxFASTCALL orx_DefaultEventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(_pstEvent->eType == orxEVENT_TYPE_SYSTEM);

  /* Depending on event ID */
  switch(_pstEvent->eID)
  {
    /* Close event */
    case orxSYSTEM_EVENT_CLOSE:
    {
      /* Updates status */
      sbStopByEvent = orxTRUE;

      break;
    }

    default:
    {
      break;
    }
  }

  /* Done! */
  return eResult;
}

static void canonicalToScreen(const float *canVec, float *screenVec)
{
  struct AxisSwap
  {
    signed char negateX;
    signed char negateY;
    signed char xSrc;
    signed char ySrc;
  };
  
  static const AxisSwap axisSwap[] = {
    {-1, -1, 0, 1 },   // ROTATION_0
    { 1, -1, 1, 0 },   // ROTATION_90
    { 1,  1, 0, 1 },   // ROTATION_180
    {-1,  1, 1, 0 } }; // ROTATION_270
    
  const AxisSwap& as = axisSwap[s_displayRotation];
  
  screenVec[0] = (float)as.negateX * canVec[ as.xSrc ];
  screenVec[1] = (float)as.negateY * canVec[ as.ySrc ];
  screenVec[2] = canVec[2];
}

  extern "C" void orxFASTCALL MainLoop()
{
  s_displayRotation = GetRotation();
  s_glesLoaded = true;

  /* Inits the engine */
  if(orxModule_Init(orxMODULE_ID_MAIN) != orxSTATUS_FAILURE)
  {
    /* Registers default event handler */
    orxEvent_AddHandler(orxEVENT_TYPE_SYSTEM, orx_DefaultEventHandler);

    /* Displays help */
    if(orxParam_DisplayHelp() != orxSTATUS_FAILURE)
    {
      orxBOOL                 bStop;
      /* Clears payload */
      orxMemory_Zero(&sstPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));

      /* Main loop */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_LOG, "App entering main loop");

      for(bStop = orxFALSE, sbStopByEvent = orxFALSE;
          bStop == orxFALSE;
          bStop = ((NVEventStatusIsRunning() != true) || (sbStopByEvent != orxFALSE) || (seMainStatus == orxSTATUS_FAILURE) || (seClockStatus == orxSTATUS_FAILURE)) ? orxTRUE : orxFALSE)
      {
        const NVEvent* ev = NULL;
        while (NVEventStatusIsRunning() && (ev = NVEventGetNextEvent(NVEventStatusIsFocused() ? 1 : 100)))
        {
          switch (ev->m_type)
          {
          case NV_EVENT_KEY:
            orxDEBUG_PRINT(orxDEBUG_LEVEL_LOG, "Key event: 0x%02x %s", 
            ev->m_data.m_key.m_code, 
            (ev->m_data.m_key.m_action == NV_KEYACTION_DOWN) ? "down" : "up");

            orxKEYBOARD_EVENT_PAYLOAD stKeyPayload;
            orxMemory_Zero(&stKeyPayload, sizeof(orxKEYBOARD_EVENT_PAYLOAD));

            /* Stores keycode */ 
            stKeyPayload.eKey = (orxKEYBOARD_KEY)ev->m_data.m_key.m_code;

            switch(ev->m_data.m_key.m_action)
            {
            case NV_KEYACTION_DOWN:
              /* Asks the keyboard plugin for translation */
              orxEVENT_SEND(orxEVENT_TYPE_FIRST_RESERVED + orxEVENT_TYPE_KEYBOARD, orxKEYBOARD_EVENT_KEY_PRESSED, orxNULL, orxNULL, &stKeyPayload);
              /* Sends the final event */
              orxEVENT_SEND(orxEVENT_TYPE_KEYBOARD, orxKEYBOARD_EVENT_KEY_PRESSED, orxNULL, orxNULL, &stKeyPayload);
              break;
            case NV_KEYACTION_UP:
              /* Asks the keyboard plugin for translation */
              orxEVENT_SEND(orxEVENT_TYPE_FIRST_RESERVED + orxEVENT_TYPE_KEYBOARD, orxKEYBOARD_EVENT_KEY_RELEASED, orxNULL, orxNULL, &stKeyPayload);
              /* Sends the final event */
              orxEVENT_SEND(orxEVENT_TYPE_KEYBOARD, orxKEYBOARD_EVENT_KEY_RELEASED, orxNULL, orxNULL, &stKeyPayload);
              break;
            }
            break;

          case NV_EVENT_CHAR:
            orxDEBUG_PRINT(orxDEBUG_LEVEL_LOG, "Char event: 0x%02x", ev->m_data.m_char.m_unichar);
            ev = NULL;
            NVEventDoneWithEvent(false);
            break;

          case NV_EVENT_TOUCH:
            orxSYSTEM_EVENT_PAYLOAD stTouchPayload;
            orxMemory_Zero(&stTouchPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
            stTouchPayload.stTouch.u32Count = 1;
            stTouchPayload.stTouch.astTouch[0].u32Id = 0;
            stTouchPayload.stTouch.astTouch[0].fX = orx2F(ev->m_data.m_touch.m_x);
            stTouchPayload.stTouch.astTouch[0].fY = orx2F(ev->m_data.m_touch.m_y);
            switch (ev->m_data.m_touch.m_action)
            {
            case NV_TOUCHACTION_DOWN:
              orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_BEGIN, orxNULL, orxNULL, &stTouchPayload);
              break;
                
            case NV_TOUCHACTION_UP:
              orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_END, orxNULL, orxNULL, &stTouchPayload);
              break;
                
            case NV_TOUCHACTION_MOVE:
              orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_MOVE, orxNULL, orxNULL, &stTouchPayload);
              break;
            }
            break;

          case NV_EVENT_MULTITOUCH:
            orxSYSTEM_EVENT_PAYLOAD stTouchPayloadMulti;
            orxMemory_Zero(&stTouchPayloadMulti, sizeof(orxSYSTEM_EVENT_PAYLOAD));

            stTouchPayloadMulti.stTouch.u32AdditionnalTouch = ev->m_data.m_multi.m_additionnalPointer;
            stTouchPayloadMulti.stTouch.u32Count = ev->m_data.m_multi.m_nCount;
      
            orxU32 uMax;
            uMax = orxMIN(ev->m_data.m_multi.m_nCount,orxANDROID_TOUCH_NUMBER);

            for (orxU32 i = 0; i < uMax; i++)
            {
              stTouchPayloadMulti.stTouch.astTouch[i].u32Id = ev->m_data.m_multi.m_astTouch[i].m_id;
              stTouchPayloadMulti.stTouch.astTouch[i].fX = ev->m_data.m_multi.m_astTouch[i].m_x;
              stTouchPayloadMulti.stTouch.astTouch[i].fY = ev->m_data.m_multi.m_astTouch[i].m_y;
            }
            
            switch (ev->m_data.m_multi.m_action)
            {
            case NV_MULTITOUCH_DOWN:
            case NV_MULTITOUCH_POINTER_DOWN:
              orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_BEGIN, orxNULL, orxNULL, &stTouchPayloadMulti);
              break;
                
            case NV_MULTITOUCH_UP:
            case NV_MULTITOUCH_POINTER_UP:
              orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_END, orxNULL, orxNULL, &stTouchPayloadMulti);
              break;
                
            case NV_MULTITOUCH_MOVE:
              orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_MOVE, orxNULL, orxNULL, &stTouchPayloadMulti);
              break;
            }
            break;
          case NV_EVENT_SURFACE_CREATED:
          case NV_EVENT_SURFACE_SIZE:
            orxDEBUG_PRINT(orxDEBUG_LEVEL_LOG, "Surface create/resize event: %d x %d", s_winWidth, s_winHeight);

            s_winWidth = ev->m_data.m_size.m_w;
            s_winHeight = ev->m_data.m_size.m_h;
            break;

          case NV_EVENT_SURFACE_DESTROYED:
            orxDEBUG_PRINT(orxDEBUG_LEVEL_LOG, "Surface destroyed event");
            orxEvent_SendShort(orxEVENT_TYPE_DISPLAY, orxDISPLAY_EVENT_SAVE_CONTEXT);
            s_glesLoaded = false;
              
            NVEventDestroySurfaceEGL();
            break;

          case NV_EVENT_FOCUS_LOST:
            orxDEBUG_PRINT(orxDEBUG_LEVEL_LOG, "Focus lost event");
            orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_FOCUS_LOST);
            break;

          case NV_EVENT_FOCUS_GAINED:
            orxDEBUG_PRINT(orxDEBUG_LEVEL_LOG, "Focus gained event");
            orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_FOCUS_GAINED);
            break;

          case NV_EVENT_PAUSE:
            orxDEBUG_PRINT(orxDEBUG_LEVEL_LOG, "Pause event");
            orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_BACKGROUND);
            break;

          case NV_EVENT_RESUME:
            orxDEBUG_PRINT(orxDEBUG_LEVEL_LOG, "Resume event");
            orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_FOREGROUND);
            break;

          case NV_EVENT_START:
            orxDEBUG_PRINT(orxDEBUG_LEVEL_LOG, "Start event");
            break;
              
          case NV_EVENT_STOP:
            orxDEBUG_PRINT(orxDEBUG_LEVEL_LOG, "Stop event");
            break;

          case NV_EVENT_ACCEL:
            orxSYSTEM_EVENT_PAYLOAD stAccelPayload;
            orxMemory_Zero(&stAccelPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
              
            float canVec[3];
            float screenVec[3];
            
            canVec[0] = orx2F(ev->m_data.m_accel.m_x);
            canVec[1] = orx2F(ev->m_data.m_accel.m_y);
            canVec[2] = orx2F(ev->m_data.m_accel.m_z);
              
            canonicalToScreen(canVec, screenVec);
             
            stAccelPayload.stAccelerometer.fX = orx2F(screenVec[0]);
            stAccelPayload.stAccelerometer.fY = orx2F(screenVec[1]);
            stAccelPayload.stAccelerometer.fZ = orx2F(screenVec[2]);
              
            /* Sends event */
            orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_ACCELERATE, orxNULL, orxNULL, &stAccelPayload);
            break;

          /* Events we simply default: */
          case NV_EVENT_RESTART:
          case NV_EVENT_QUIT:
            orxDEBUG_PRINT(orxDEBUG_LEVEL_LOG, "%s event: no specific app action", NVEventGetEventStr(ev->m_type));
            break;

          default:
            orxDEBUG_PRINT(orxDEBUG_LEVEL_LOG, "UNKNOWN event");
            break;
          };
            
          /* if we do not NULL out the event, then we return that we handled it by default */
          if (ev)
            NVEventDoneWithEvent(true);
        }

        // Do not bother to initialize _any_ of EGL, much less go ahead
        // and render to the screen unless we have all we need to go 
        // ahead and do our thing.  In many cases,
        // devices will bring us part-way up and then take us down.
        // So, before we bother to init EGL (much less the rendering
        // surface, check that:
        // - we are focused
        // - we have a rendering surface
        // - the surface size is not 0x0
        // - we are resumed, not paused
        if (NVEventStatusIsInteractable())
        {
          // This will try to set up EGL if it isn't set up
          // When we first set up EGL completely, we also load our GLES resources
          // If these are already set up or we succeed at setting them all up now, then
          // we go ahead and render.
          renderFrame(true);
        }
      }
    }

    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_SYSTEM, orx_DefaultEventHandler);

    /* Exits from engine */
    orxModule_Exit(orxMODULE_ID_MAIN);
  }

  /* Exits from all modules */
  orxModule_ExitAll();
}

