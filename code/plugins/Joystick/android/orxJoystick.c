/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2010 Orx-Project
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
 * @file orxJoystick.c
 * @date 13/01/2011
 * @author simons.philippe@gmail.com
 *
 * Android joystick plugin implementation
 *
 * @todo
 */

#include "orxPluginAPI.h"

#include "main/orxAndroid.h"
#include <android/sensor.h>

#define KZ_CONFIG_ANDROID                        "Android"
#define KZ_CONFIG_ACCELEROMETER_FREQUENCY        "AccelerometerFrequency"

/** Module flags
 */
#define orxJOYSTICK_KU32_STATIC_FLAG_NONE     0x00000000 /**< No flags */

#define orxJOYSTICK_KU32_STATIC_FLAG_READY    0x00000001 /**< Ready flag */

#define orxJOYSTICK_KU32_STATIC_MASK_ALL      0xFFFFFFFF /**< All mask */

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

extern ASensorEventQueue* sensorEventQueue;

/** Static structure
 */
typedef struct __orxJOYSTICK_STATIC_t {
	orxU32 u32Flags;
	orxVECTOR vAcceleration;
        orxS32 s32Rotation;
        ASensorManager* sensorManager;
  const ASensor* accelerometerSensor;

} orxJOYSTICK_STATIC;

/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxJOYSTICK_STATIC sstJoystick;

/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/
static void canonicalToScreen(int     displayRotation,
                              const float* canVec,
                              float* screenVec)
{
     struct AxisSwap
     {
          signed char negateX;
          signed char negateY;
          signed char xSrc;
          signed char ySrc;
     };
     static const AxisSwap axisSwap[] = {
          {-1, 1, 0, 1 }, // ROTATION_0
          { 1, 1, 1, 0 }, // ROTATION_90
          { 1, -1, 0, 1 }, // ROTATION_180
          {-1, -1, 1, 0 } }; // ROTATION_270
     const AxisSwap& as = axisSwap[displayRotation];
     screenVec[0] = (float)as.negateX * canVec[ as.xSrc ];
     screenVec[1] = (float)as.negateY * canVec[ as.ySrc ];
     screenVec[2] = canVec[2]; 
} 

static orxSTATUS orxFASTCALL orxJoystick_Android_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  if(_pstEvent->eType == orxANDROID_EVENT_TYPE_SURFACE && _pstEvent->eID == orxANDROID_EVENT_SURFACE_CREATED)
  {
    /* reset rotation */
    sstJoystick.s32Rotation = -1;
  }

  if(_pstEvent->eType == orxEVENT_TYPE_SYSTEM && _pstEvent->eID == orxSYSTEM_EVENT_ACCELERATE)
  {
    static float in[3];
    static float out[3];
    orxSYSTEM_EVENT_PAYLOAD *pstPayload;

    /* Gets payload */
    pstPayload = (orxSYSTEM_EVENT_PAYLOAD *) _pstEvent->pstPayload;

    in[0] = pstPayload->stAccelerometer.vAcceleration.fX;
    in[1] = pstPayload->stAccelerometer.vAcceleration.fY;
    in[2] = pstPayload->stAccelerometer.vAcceleration.fZ;

    if(sstJoystick.s32Rotation == -1)
    {
      sstJoystick.s32Rotation = orxAndroid_JNI_GetRotation();
    }

    canonicalToScreen(sstJoystick.s32Rotation, in, out);

    /* Gets new acceleration */
    orxVector_Set(&(sstJoystick.vAcceleration), out[0], out[1], out[2]);
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxJoystick_Android_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Wasn't already initialized? */
  if (!(sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstJoystick, sizeof(orxJOYSTICK_STATIC));

    sstJoystick.s32Rotation = -1;

    /* Adds our joystick event handlers */
    if ((eResult = orxEvent_AddHandler(orxEVENT_TYPE_SYSTEM, orxJoystick_Android_EventHandler)) != orxSTATUS_FAILURE)
    {
      if ((eResult = orxEvent_AddHandler(orxANDROID_EVENT_TYPE_SURFACE, orxJoystick_Android_EventHandler)) != orxSTATUS_FAILURE)
      {
        /* Updates status */
        sstJoystick.u32Flags |= orxJOYSTICK_KU32_STATIC_FLAG_READY;

        orxConfig_PushSection(KZ_CONFIG_ANDROID);

        if(orxConfig_HasValue(KZ_CONFIG_ACCELEROMETER_FREQUENCY))
        {
          orxU32 u32Frequency;
          u32Frequency = orxConfig_GetU32(KZ_CONFIG_ACCELEROMETER_FREQUENCY);

          if(u32Frequency > 0)
          {
            ALooper* looper = ALooper_forThread();
            sstJoystick.sensorManager = ASensorManager_getInstance();
            sstJoystick.accelerometerSensor = ASensorManager_getDefaultSensor(sstJoystick.sensorManager, ASENSOR_TYPE_ACCELEROMETER);
            sensorEventQueue = ASensorManager_createEventQueue(sstJoystick.sensorManager, looper, LOOPER_ID_SENSOR, NULL, NULL);
            ASensorEventQueue_enableSensor(sensorEventQueue, sstJoystick.accelerometerSensor);
            ASensorEventQueue_setEventRate(sensorEventQueue, sstJoystick.accelerometerSensor, (1000L/u32Frequency)*1000); 
          }
        }
        else
        { /* enable acceleromter with default rate */
          ALooper* looper = ALooper_forThread();
          sstJoystick.sensorManager = ASensorManager_getInstance();
          sstJoystick.accelerometerSensor = ASensorManager_getDefaultSensor(sstJoystick.sensorManager, ASENSOR_TYPE_ACCELEROMETER);
          sensorEventQueue = ASensorManager_createEventQueue(sstJoystick.sensorManager, looper, LOOPER_ID_SENSOR, NULL, NULL);
          ASensorEventQueue_enableSensor(sensorEventQueue, sstJoystick.accelerometerSensor);
          ASensorEventQueue_setEventRate(sensorEventQueue, sstJoystick.accelerometerSensor, (1000L/60)*1000); 
        }

        orxConfig_PopSection();
      }
    }
  }

  /* Done! */
  return eResult;
}

void orxFASTCALL orxJoystick_Android_Exit()
{
  /* Was initialized? */
  if (sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY)
  {
    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_SYSTEM, orxJoystick_Android_EventHandler);
    orxEvent_RemoveHandler(orxANDROID_EVENT_TYPE_SURFACE, orxJoystick_Android_EventHandler);

    /* Cleans static controller */
    orxMemory_Zero(&sstJoystick, sizeof(orxJOYSTICK_STATIC));
  }

  return;
}

orxFLOAT orxFASTCALL orxJoystick_Android_GetAxisValue(orxJOYSTICK_AXIS _eAxis)
{
  orxFLOAT fResult = orxFLOAT_0;

  /* Depending on axis */
  switch (_eAxis)
  {
    case orxJOYSTICK_AXIS_X_1:
    case orxJOYSTICK_AXIS_X_2:
    case orxJOYSTICK_AXIS_X_3:
    case orxJOYSTICK_AXIS_X_4:
    {
      /* Updates result */
      fResult = sstJoystick.vAcceleration.fX;

      break;
    }

    case orxJOYSTICK_AXIS_Y_1:
    case orxJOYSTICK_AXIS_Y_2:
    case orxJOYSTICK_AXIS_Y_3:
    case orxJOYSTICK_AXIS_Y_4:
    {
      /* Updates result */
      fResult = sstJoystick.vAcceleration.fY;

      break;
    }

    case orxJOYSTICK_AXIS_Z_1:
    case orxJOYSTICK_AXIS_Z_2:
    case orxJOYSTICK_AXIS_Z_3:
    case orxJOYSTICK_AXIS_Z_4:
    {
      /* Updates result */
      fResult = sstJoystick.vAcceleration.fZ;

      break;
    }

    default:
    {
		  /* Not available */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_JOYSTICK, "<%s> is not available on this platform!", orxJoystick_GetAxisName(_eAxis));

      break;
    }
  }

  /* Done! */
  return fResult;
}

orxBOOL orxFASTCALL orxJoystick_Android_IsButtonPressed(orxJOYSTICK_BUTTON _eButton)
{
  orxBOOL bResult = orxFALSE;

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_JOYSTICK, "Not available on this platform!");

  /* Done! */
  return bResult;
}

/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/
orxPLUGIN_USER_CORE_FUNCTION_START(JOYSTICK);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_Android_Init, JOYSTICK, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_Android_Exit, JOYSTICK, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_Android_GetAxisValue, JOYSTICK, GET_AXIS_VALUE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_Android_IsButtonPressed, JOYSTICK, IS_BUTTON_PRESSED);
orxPLUGIN_USER_CORE_FUNCTION_END();
