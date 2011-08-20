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

static float s_aspect = 1.0f;
int32_t s_winWidth = 1;
int32_t s_winHeight = 1;
static unsigned int s_swapCount = 0;

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

int32_t NVEventAppMain(int32_t argc, char** argv)
{
  s_swapCount = 0;
  
  s_aspect = 1.0f;
  s_winWidth = 1;
  s_winHeight = 1;

  /* loop until EGL is ready */

  while (NVEventStatusIsRunning() && !NVEventReadyToRenderEGL(true))
  {
    const NVEvent* ev = NULL;
    while (NVEventStatusIsRunning() && (ev = NVEventGetNextEvent(NVEventStatusIsFocused() ? 1 : 100)))
    {
      if(ev->m_type == NV_EVENT_SURFACE_SIZE)
      {
        s_winWidth = ev->m_data.m_size.m_w;
        s_winHeight = ev->m_data.m_size.m_h;
        DEBUG( "Surface create/resize event: %d x %d", s_winWidth, s_winHeight);

        if ((s_winWidth > 0) && (s_winHeight > 0))
          s_aspect = (float)s_winWidth / (float)s_winHeight;
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

  // A debug printout every 256 frames so we can see when we're
  // actively rendering and swapping
  if (!(s_swapCount++ & 0x00ff))
  {
    orxDEBUG_PRINT(orxDEBUG_LEVEL_LOG, "Swap count is %d", s_swapCount);
  }

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

/** Render inhibiter
 */
static orxSTATUS orxFASTCALL RenderInhibiter(const orxEVENT *_pstEvent)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

extern "C" {

  void MainLoop()
  {
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

              if (ev->m_data.m_key.m_code == NV_KEYCODE_BACK)
              {
                /* Sends event */
                orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_CLOSE);
              }
              else
              {
                NVEventDoneWithEvent(false); 
              }
              ev = NULL;
              break;

            case NV_EVENT_CHAR:
              orxDEBUG_PRINT(orxDEBUG_LEVEL_LOG, "Char event: 0x%02x", ev->m_data.m_char.m_unichar);
              ev = NULL;
              NVEventDoneWithEvent(false);
              break;

            case NV_EVENT_TOUCH:
              orxSYSTEM_EVENT_PAYLOAD stTouchPayload;
              orxMemory_Zero(&stTouchPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
                
              stTouchPayload.stTouch.fX = orx2F(ev->m_data.m_touch.m_x);
              stTouchPayload.stTouch.fY = orx2F(ev->m_data.m_touch.m_y);
              stTouchPayload.stTouch.fPressure = orx2F(0);
                
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

            case NV_EVENT_SURFACE_CREATED:
            case NV_EVENT_SURFACE_SIZE:
              s_winWidth = ev->m_data.m_size.m_w;
              s_winHeight = ev->m_data.m_size.m_h;
              orxDEBUG_PRINT(orxDEBUG_LEVEL_LOG, "Surface create/resize event: %d x %d", s_winWidth, s_winHeight);

              if ((s_winWidth > 0) && (s_winHeight > 0))
                  s_aspect = (float)s_winWidth / (float)s_winHeight;
              break;

            case NV_EVENT_SURFACE_DESTROYED:
              orxDEBUG_PRINT(orxDEBUG_LEVEL_LOG, "Surface destroyed event");
              
              NVEventDestroySurfaceEGL();
              break;

            case NV_EVENT_FOCUS_LOST:
				      orxDEBUG_PRINT(orxDEBUG_LEVEL_LOG, "Focus lost event");
				      
              /* Adds render inhibiter */
              orxEvent_AddHandler(orxEVENT_TYPE_RENDER, RenderInhibiter);
              break;

            case NV_EVENT_FOCUS_GAINED:
              orxDEBUG_PRINT(orxDEBUG_LEVEL_LOG, "Focus gained event");
              
              /* Removes render inhibiter */
              orxEvent_RemoveHandler(orxEVENT_TYPE_RENDER, RenderInhibiter);
              break;

            case NV_EVENT_PAUSE:
				      orxDEBUG_PRINT(orxDEBUG_LEVEL_LOG, "Pause event");
              break;

            case NV_EVENT_RESUME:
				      orxDEBUG_PRINT(orxDEBUG_LEVEL_LOG, "Resume event");
              break;
              
            case NV_EVENT_STOP:
              orxDEBUG_PRINT(orxDEBUG_LEVEL_LOG, "Stop event");
              break;

            case NV_EVENT_ACCEL:
              orxSYSTEM_EVENT_PAYLOAD stAccelPayload;
              orxMemory_Zero(&stAccelPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
              
              stAccelPayload.stAccelerometer.fX = orx2F(ev->m_data.m_accel.m_x);
              stAccelPayload.stAccelerometer.fY = orx2F(ev->m_data.m_accel.m_y);
              stAccelPayload.stAccelerometer.fZ = orx2F(ev->m_data.m_accel.m_z);
              
              /* Sends event */
              orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_ACCELERATE, orxNULL, orxNULL, &stAccelPayload);
              break;

            // Events we simply default:
            case NV_EVENT_MULTITOUCH:
            case NV_EVENT_START:
            case NV_EVENT_RESTART:
            case NV_EVENT_QUIT:
              orxDEBUG_PRINT(orxDEBUG_LEVEL_LOG, "%s event: no specific app action", NVEventGetEventStr(ev->m_type));
              break;

            default:
              orxDEBUG_PRINT(orxDEBUG_LEVEL_LOG, "UNKNOWN event");
              break;
			      };
			      
            // if we do not NULL out the event, then we return that
            // we handled it by default
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
  
}
