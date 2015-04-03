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

#define orxANDROID_LAST_JOYSTICK                  3

/** Module flags
 */
#define orxJOYSTICK_KU32_STATIC_FLAG_NONE     0x00000000 /**< No flags */

#define orxJOYSTICK_KU32_STATIC_FLAG_READY    0x00000001 /**< Ready flag */

#define orxJOYSTICK_KU32_STATIC_MASK_ALL      0xFFFFFFFF /**< All mask */

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Internal joystick info structure
 */
typedef struct __orxJOYSTICK_INFO_t
{
  float                 afAxisInfoList[orxJOYSTICK_AXIS_SINGLE_NUMBER];
  unsigned char         au8ButtonInfoList[orxJOYSTICK_BUTTON_SINGLE_NUMBER];

} orxJOYSTICK_INFO;

/** Static structure
 */
typedef struct __orxJOYSTICK_STATIC_t {
  orxU32                u32Flags;

  orxJOYSTICK_INFO      astJoyInfoList[orxANDROID_LAST_JOYSTICK + 1];
  orxS32                au32DeviceIds[orxANDROID_LAST_JOYSTICK + 1];
  orxBOOL               bUseJoystick;

  orxVECTOR             vAcceleration;
  orxS32                s32ScreenRotation;
  orxU32                u32Frequency;
  orxBOOL               bAccelerometerEnabled;

  ASensorManager*       sensorManager;
  const ASensor*        accelerometerSensor;
  ASensorEventQueue*    sensorEventQueue;

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

static void enableSensorManager()
{
  if(sstJoystick.u32Frequency > 0 && !sstJoystick.bAccelerometerEnabled)
  {
    ASensorEventQueue_enableSensor(sstJoystick.sensorEventQueue, sstJoystick.accelerometerSensor);
    ASensorEventQueue_setEventRate(sstJoystick.sensorEventQueue, sstJoystick.accelerometerSensor, (1000L/sstJoystick.u32Frequency)*1000);
    sstJoystick.bAccelerometerEnabled = orxTRUE;
  }
}

static void disableSensorManager()
{
  if(sstJoystick.bAccelerometerEnabled)
  {
    ASensorEventQueue_disableSensor(sstJoystick.sensorEventQueue, sstJoystick.accelerometerSensor);
    sstJoystick.bAccelerometerEnabled = orxFALSE;
  }
}

static orxS32 getDeviceIndex(orxU32 _u32DeviceId)
{
  for(orxS32 i = 0; i < orxANDROID_LAST_JOYSTICK + 1; i++)
  {
    if(sstJoystick.au32DeviceIds[i] == _u32DeviceId)
    {
      return i;
    }
  }

  return -1;
}

static orxSTATUS newDeviceIndex(orxU32 _u32DeviceId)
{
  for(orxS32 i = 0; i < orxANDROID_LAST_JOYSTICK + 1; i++)
  {
    if(sstJoystick.au32DeviceIds[i] == 0)
    {
      sstJoystick.au32DeviceIds[i] = _u32DeviceId;

      return orxSTATUS_SUCCESS;
    }
  }

  return orxSTATUS_FAILURE;
}

static orxSTATUS orxFASTCALL orxJoystick_Android_JoystickEventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;
  orxANDROID_JOYSTICK_EVENT *pstJoystickEvent;
  orxS32 s32DeviceIndex;

  /* Gets payload */
  pstJoystickEvent = (orxANDROID_JOYSTICK_EVENT *) _pstEvent->pstPayload;

  /* Depending on ID */
  switch(pstJoystickEvent->u32Type)
  {
    case orxANDROID_EVENT_JOYSTICK_ADDED:
      if(newDeviceIndex(pstJoystickEvent->u32DeviceId) != orxSTATUS_SUCCESS)
      {
        orxDEBUG_PRINT(orxDEBUG_LEVEL_JOYSTICK, "couldn't add new device %d", pstJoystickEvent->u32DeviceId);
      }
      break;

    case orxANDROID_EVENT_JOYSTICK_REMOVED:
      s32DeviceIndex = getDeviceIndex(pstJoystickEvent->u32DeviceId);
      if(s32DeviceIndex != -1)
      {
        sstJoystick.au32DeviceIds[s32DeviceIndex] = 0;
      }
      else
      {
        orxDEBUG_PRINT(orxDEBUG_LEVEL_JOYSTICK, "unknown device %d", pstJoystickEvent->u32DeviceId);
      }
      break;

    case orxANDROID_EVENT_JOYSTICK_CHANGED:
      break;

    case orxANDROID_EVENT_JOYSTICK_DOWN:
      s32DeviceIndex = getDeviceIndex(pstJoystickEvent->u32DeviceId);

      if(s32DeviceIndex != -1)
      {
        switch(pstJoystickEvent->u32KeyCode)
        {
          case 19: // KEYCODE_DPAD_UP
            sstJoystick.astJoyInfoList[s32DeviceIndex].afAxisInfoList[orxJOYSTICK_AXIS_V_1] = -orxFLOAT_1;
            break;
          case 20: // KEYCODE_DPAD_DOWN
            sstJoystick.astJoyInfoList[s32DeviceIndex].afAxisInfoList[orxJOYSTICK_AXIS_V_1] = orxFLOAT_1;
            break;
          case 21: // KEYCODE_DPAD_LEFT
            sstJoystick.astJoyInfoList[s32DeviceIndex].afAxisInfoList[orxJOYSTICK_AXIS_U_1] = -orxFLOAT_1;
            break;
          case 22: // KEYCODE_DPAD_RIGHT
            sstJoystick.astJoyInfoList[s32DeviceIndex].afAxisInfoList[orxJOYSTICK_AXIS_U_1] = orxFLOAT_1;
            break;
          case 96: // KEYCODE_BUTTON_A
            sstJoystick.astJoyInfoList[s32DeviceIndex].au8ButtonInfoList[orxJOYSTICK_BUTTON_1_1] = 1;
            break;
          case 97: // KEYCODE_BUTTON_B
            sstJoystick.astJoyInfoList[s32DeviceIndex].au8ButtonInfoList[orxJOYSTICK_BUTTON_2_1] = 1;
            break;
          case 98: // KEYCODE_BUTTON_C
            sstJoystick.astJoyInfoList[s32DeviceIndex].au8ButtonInfoList[orxJOYSTICK_BUTTON_3_1] = 1;
            break;
          case 99: // KEYCODE_BUTTON_X
            sstJoystick.astJoyInfoList[s32DeviceIndex].au8ButtonInfoList[orxJOYSTICK_BUTTON_4_1] = 1;
            break;
          case 100: // KEYCODE_BUTTON_Y
            sstJoystick.astJoyInfoList[s32DeviceIndex].au8ButtonInfoList[orxJOYSTICK_BUTTON_5_1] = 1;
            break;
          case 101: // KEYCODE_BUTTON_Z
            sstJoystick.astJoyInfoList[s32DeviceIndex].au8ButtonInfoList[orxJOYSTICK_BUTTON_6_1] = 1;
            break;
          case 102: // KEYCODE_BUTTON_L1
            sstJoystick.astJoyInfoList[s32DeviceIndex].au8ButtonInfoList[orxJOYSTICK_BUTTON_7_1] = 1;
            break;
          case 103: // KEYCODE_BUTTON_R1
            sstJoystick.astJoyInfoList[s32DeviceIndex].au8ButtonInfoList[orxJOYSTICK_BUTTON_8_1] = 1;
            break;
          case 104: // KEYCODE_BUTTON_L2
            sstJoystick.astJoyInfoList[s32DeviceIndex].au8ButtonInfoList[orxJOYSTICK_BUTTON_9_1] = 1;
            break;
          case 105: // KEYCODE_BUTTON_R2
            sstJoystick.astJoyInfoList[s32DeviceIndex].au8ButtonInfoList[orxJOYSTICK_BUTTON_10_1] = 1;
            break;
          case 106: // KEYCODE_BUTTON_THUMBL
            sstJoystick.astJoyInfoList[s32DeviceIndex].au8ButtonInfoList[orxJOYSTICK_BUTTON_11_1] = 1;
            break;
          case 107: // KEYCODE_BUTTON_THUMBR
            sstJoystick.astJoyInfoList[s32DeviceIndex].au8ButtonInfoList[orxJOYSTICK_BUTTON_12_1] = 1;
            break;
          case 108: // KEYCODE_BUTTON_START
            sstJoystick.astJoyInfoList[s32DeviceIndex].au8ButtonInfoList[orxJOYSTICK_BUTTON_13_1] = 1;
            break;
          case 109: // KEYCODE_BUTTON_SELECT
            sstJoystick.astJoyInfoList[s32DeviceIndex].au8ButtonInfoList[orxJOYSTICK_BUTTON_14_1] = 1;
            break;
          default:
            orxDEBUG_PRINT(orxDEBUG_LEVEL_JOYSTICK, "unknown keycode %d", pstJoystickEvent->u32KeyCode);
            break;
        }
      }
      else
      {
        orxDEBUG_PRINT(orxDEBUG_LEVEL_JOYSTICK, "unknown device %d", pstJoystickEvent->u32DeviceId);
      }
      break;

    case orxANDROID_EVENT_JOYSTICK_UP:
      s32DeviceIndex = getDeviceIndex(pstJoystickEvent->u32DeviceId);

      if(s32DeviceIndex != -1)
      {
        switch(pstJoystickEvent->u32KeyCode)
        {
          case 19: // KEYCODE_DPAD_UP
          case 20: // KEYCODE_DPAD_DOWN
            sstJoystick.astJoyInfoList[s32DeviceIndex].afAxisInfoList[orxJOYSTICK_AXIS_V_1] = orxFLOAT_0;
            break;
          case 21: // KEYCODE_DPAD_LEFT
          case 22: // KEYCODE_DPAD_RIGHT
            sstJoystick.astJoyInfoList[s32DeviceIndex].afAxisInfoList[orxJOYSTICK_AXIS_U_1] = orxFLOAT_0;
            break;
          case 96: // KEYCODE_BUTTON_A
            sstJoystick.astJoyInfoList[s32DeviceIndex].au8ButtonInfoList[orxJOYSTICK_BUTTON_1_1] = 0;
            break;
          case 97: // KEYCODE_BUTTON_B
            sstJoystick.astJoyInfoList[s32DeviceIndex].au8ButtonInfoList[orxJOYSTICK_BUTTON_2_1] = 0;
            break;
          case 98: // KEYCODE_BUTTON_C
            sstJoystick.astJoyInfoList[s32DeviceIndex].au8ButtonInfoList[orxJOYSTICK_BUTTON_3_1] = 0;
            break;
          case 99: // KEYCODE_BUTTON_X
            sstJoystick.astJoyInfoList[s32DeviceIndex].au8ButtonInfoList[orxJOYSTICK_BUTTON_4_1] = 0;
            break;
          case 100: // KEYCODE_BUTTON_Y
            sstJoystick.astJoyInfoList[s32DeviceIndex].au8ButtonInfoList[orxJOYSTICK_BUTTON_5_1] = 0;
            break;
          case 101: // KEYCODE_BUTTON_Z
            sstJoystick.astJoyInfoList[s32DeviceIndex].au8ButtonInfoList[orxJOYSTICK_BUTTON_6_1] = 0;
            break;
          case 102: // KEYCODE_BUTTON_L1
            sstJoystick.astJoyInfoList[s32DeviceIndex].au8ButtonInfoList[orxJOYSTICK_BUTTON_7_1] = 0;
            break;
          case 103: // KEYCODE_BUTTON_R1
            sstJoystick.astJoyInfoList[s32DeviceIndex].au8ButtonInfoList[orxJOYSTICK_BUTTON_8_1] = 0;
            break;
          case 104: // KEYCODE_BUTTON_L2
            sstJoystick.astJoyInfoList[s32DeviceIndex].au8ButtonInfoList[orxJOYSTICK_BUTTON_9_1] = 0;
            break;
          case 105: // KEYCODE_BUTTON_R2
            sstJoystick.astJoyInfoList[s32DeviceIndex].au8ButtonInfoList[orxJOYSTICK_BUTTON_10_1] = 0;
            break;
          case 106: // KEYCODE_BUTTON_THUMBL
            sstJoystick.astJoyInfoList[s32DeviceIndex].au8ButtonInfoList[orxJOYSTICK_BUTTON_11_1] = 0;
            break;
          case 107: // KEYCODE_BUTTON_THUMBR
            sstJoystick.astJoyInfoList[s32DeviceIndex].au8ButtonInfoList[orxJOYSTICK_BUTTON_12_1] = 0;
            break;
          case 108: // KEYCODE_BUTTON_START
            sstJoystick.astJoyInfoList[s32DeviceIndex].au8ButtonInfoList[orxJOYSTICK_BUTTON_13_1] = 0;
            break;
          case 109: // KEYCODE_BUTTON_SELECT
            sstJoystick.astJoyInfoList[s32DeviceIndex].au8ButtonInfoList[orxJOYSTICK_BUTTON_14_1] = 0;
            break;
          default:
            orxDEBUG_PRINT(orxDEBUG_LEVEL_JOYSTICK, "unknown keycode %d", pstJoystickEvent->u32KeyCode);
            break;
        }
      }
      else
      {
        orxDEBUG_PRINT(orxDEBUG_LEVEL_JOYSTICK, "unknown device %d", pstJoystickEvent->u32DeviceId);
      }
      break;

    case orxANDROID_EVENT_JOYSTICK_MOVE:
      s32DeviceIndex = getDeviceIndex(pstJoystickEvent->u32DeviceId);

      if(s32DeviceIndex != -1)
      {
        sstJoystick.astJoyInfoList[s32DeviceIndex].afAxisInfoList[orxJOYSTICK_AXIS_X_1] = pstJoystickEvent->stAxisData.fX;
        sstJoystick.astJoyInfoList[s32DeviceIndex].afAxisInfoList[orxJOYSTICK_AXIS_Y_1] = pstJoystickEvent->stAxisData.fY;
        sstJoystick.astJoyInfoList[s32DeviceIndex].afAxisInfoList[orxJOYSTICK_AXIS_Z_1] = pstJoystickEvent->stAxisData.fZ;
        sstJoystick.astJoyInfoList[s32DeviceIndex].afAxisInfoList[orxJOYSTICK_AXIS_R_1] = pstJoystickEvent->stAxisData.fRZ;
        sstJoystick.astJoyInfoList[s32DeviceIndex].afAxisInfoList[orxJOYSTICK_AXIS_U_1] = pstJoystickEvent->stAxisData.fHAT_X;
        sstJoystick.astJoyInfoList[s32DeviceIndex].afAxisInfoList[orxJOYSTICK_AXIS_V_1] = pstJoystickEvent->stAxisData.fHAT_Y;
        sstJoystick.astJoyInfoList[s32DeviceIndex].afAxisInfoList[orxJOYSTICK_AXIS_POV_1] = pstJoystickEvent->stAxisData.fRTRIGGER != orxFLOAT_0 ? pstJoystickEvent->stAxisData.fRTRIGGER : -pstJoystickEvent->stAxisData.fLTRIGGER;
      }
      else
      {
        orxDEBUG_PRINT(orxDEBUG_LEVEL_JOYSTICK, "unknown device %d", pstJoystickEvent->u32DeviceId);
      }
      break;
  }

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxJoystick_Android_AccelerometerEventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  if(_pstEvent->eType == orxANDROID_EVENT_TYPE_SURFACE && _pstEvent->eID == orxANDROID_EVENT_SURFACE_CHANGED)
  {
    /* reset rotation */
    sstJoystick.s32ScreenRotation = -1;
  }

  if(_pstEvent->eType == orxANDROID_EVENT_TYPE_ACCELERATE)
  {
    static float in[3];
    static float out[3];
    ASensorEvent event;

    if(sstJoystick.s32ScreenRotation == -1)
    {
      sstJoystick.s32ScreenRotation = orxAndroid_JNI_GetRotation();
    }

    while (ASensorEventQueue_getEvents(sstJoystick.sensorEventQueue, &event, 1) > 0)
    {
      in[0] = event.acceleration.x;
      in[1] = event.acceleration.y;
      in[2] = event.acceleration.z;

      canonicalToScreen(sstJoystick.s32ScreenRotation, in, out);

      /* Gets new acceleration */
      orxVector_Set(&(sstJoystick.vAcceleration), out[0], out[1], out[2]);
    }
  }

  if(_pstEvent->eType == orxEVENT_TYPE_SYSTEM)
  {
    switch(_pstEvent->eID)
    {
      case orxSYSTEM_EVENT_FOCUS_GAINED:
      {
        enableSensorManager();

        break;
      }

      case orxSYSTEM_EVENT_FOCUS_LOST:
      {
        disableSensorManager();

        break;
      }

      default:
      {
        break;
      }
    }
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

    orxConfig_PushSection(KZ_CONFIG_ANDROID);

    sstJoystick.s32ScreenRotation = -1;
    sstJoystick.bAccelerometerEnabled = orxFALSE;
    sstJoystick.bUseJoystick = orxConfig_GetBool(KZ_CONFIG_USE_JOYSTICK);

    if(sstJoystick.bUseJoystick == orxTRUE)
    {
      orxAndroid_JNI_GetDeviceIds(sstJoystick.au32DeviceIds);
      orxEvent_AddHandler(orxANDROID_EVENT_TYPE_JOYSTICK, orxJoystick_Android_JoystickEventHandler);
    }
    else
    {
      sstJoystick.sensorManager = ASensorManager_getInstance();
      sstJoystick.accelerometerSensor = ASensorManager_getDefaultSensor(sstJoystick.sensorManager, ASENSOR_TYPE_ACCELEROMETER);

      if(sstJoystick.accelerometerSensor != NULL)
      {
        /* Adds our Accelerometer event handlers */
        if ((eResult = orxEvent_AddHandler(orxEVENT_TYPE_SYSTEM, orxJoystick_Android_AccelerometerEventHandler)) != orxSTATUS_FAILURE)
        {
          if ((eResult = orxEvent_AddHandler(orxANDROID_EVENT_TYPE_ACCELERATE, orxJoystick_Android_AccelerometerEventHandler)) != orxSTATUS_FAILURE)
          {
            if ((eResult = orxEvent_AddHandler(orxANDROID_EVENT_TYPE_SURFACE, orxJoystick_Android_AccelerometerEventHandler)) != orxSTATUS_FAILURE)
            {
              ALooper* looper = ALooper_forThread();
              sstJoystick.sensorEventQueue = ASensorManager_createEventQueue(sstJoystick.sensorManager, looper, LOOPER_ID_SENSOR, NULL, NULL);

              if(orxConfig_HasValue(KZ_CONFIG_ACCELEROMETER_FREQUENCY))
              {
                sstJoystick.u32Frequency = orxConfig_GetU32(KZ_CONFIG_ACCELEROMETER_FREQUENCY);
              }
              else
              { /* enable acceleromter with default rate */
                sstJoystick.u32Frequency = 60;
              }

              /* enable sensor */
              enableSensorManager();
            }
          }
        }
      }
    }

    orxConfig_PopSection();

    /* Updates status */
    sstJoystick.u32Flags |= orxJOYSTICK_KU32_STATIC_FLAG_READY;
  }

  /* Done! */
  return eResult;
}

void orxFASTCALL orxJoystick_Android_Exit()
{
  /* Was initialized? */
  if (sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY)
  {
    if(sstJoystick.accelerometerSensor != NULL)
    {
      /* Removes event handler */
      orxEvent_RemoveHandler(orxEVENT_TYPE_SYSTEM, orxJoystick_Android_AccelerometerEventHandler);
      orxEvent_RemoveHandler(orxANDROID_EVENT_TYPE_SURFACE, orxJoystick_Android_AccelerometerEventHandler);
      orxEvent_RemoveHandler(orxANDROID_EVENT_TYPE_ACCELERATE, orxJoystick_Android_AccelerometerEventHandler);

      disableSensorManager();

      /* destroy event queue */
      ASensorManager_destroyEventQueue(sstJoystick.sensorManager, sstJoystick.sensorEventQueue);
    }

    orxEvent_RemoveHandler(orxANDROID_EVENT_TYPE_JOYSTICK, orxJoystick_Android_JoystickEventHandler);

    /* Cleans static controller */
    orxMemory_Zero(&sstJoystick, sizeof(orxJOYSTICK_STATIC));
  }

  return;
}

orxFLOAT orxFASTCALL orxJoystick_Android_GetAxisValue(orxJOYSTICK_AXIS _eAxis)
{
  orxU32    u32ID;
  orxFLOAT  fResult;

  /* Checks */
  orxASSERT((sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY) == orxJOYSTICK_KU32_STATIC_FLAG_READY);
  orxASSERT(_eAxis < orxJOYSTICK_AXIS_NUMBER);

  if(!sstJoystick.bUseJoystick)
  {
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
        fResult = orxFLOAT_0;

        break;
      }
    }
  }
  else
  {
    /* Gets ID */
    u32ID = (orxU32)_eAxis / orxJOYSTICK_AXIS_SINGLE_NUMBER;

    /* Is ID valid? */
    if(u32ID <= (orxU32)orxANDROID_LAST_JOYSTICK)
    {
      /* Plugged? */
      if(sstJoystick.au32DeviceIds[u32ID] != 0)
      {
        orxS32 s32Axis;

        /* Gets axis */
        s32Axis = _eAxis % orxJOYSTICK_AXIS_SINGLE_NUMBER;
    
        /* Updates result */
        fResult = orx2F(sstJoystick.astJoyInfoList[u32ID].afAxisInfoList[s32Axis]);
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_JOYSTICK, "Requested joystick ID <%d> is not connected.", u32ID);

        /* Updates result */
        fResult = orxFLOAT_0;
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_JOYSTICK, "Requested joystick ID <%d> is out of range.", u32ID);

      /* Updates result */
      fResult = orxFLOAT_0;
    }
  }

  /* Done! */
  return fResult;
}

orxBOOL orxFASTCALL orxJoystick_Android_IsButtonPressed(orxJOYSTICK_BUTTON _eButton)
{
  orxU32  u32ID;
  orxBOOL bResult;

  /* Checks */
  orxASSERT((sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY) == orxJOYSTICK_KU32_STATIC_FLAG_READY);
  orxASSERT(_eButton < orxJOYSTICK_BUTTON_NUMBER);

  /* Gets ID */
  u32ID = (orxU32)_eButton / orxJOYSTICK_BUTTON_SINGLE_NUMBER;

  /* Is ID valid? */
  if(u32ID <= orxANDROID_LAST_JOYSTICK)
  {
    /* Plugged? */
    if(sstJoystick.au32DeviceIds[u32ID] != 0)
    {
      orxS32 s32Button;

      /* Gets button */
      s32Button = _eButton % orxJOYSTICK_BUTTON_SINGLE_NUMBER;

      /* Updates result */
      bResult = (sstJoystick.astJoyInfoList[u32ID].au8ButtonInfoList[s32Button] != 0) ? orxTRUE : orxFALSE;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_JOYSTICK, "Requested joystick ID <%d> is not connected.", u32ID);

      /* Updates result */
      bResult = orxFALSE;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_JOYSTICK, "Requested joystick ID <%d> is out of range.", u32ID);

    /* Updates result */
    bResult = orxFALSE;
  }

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
