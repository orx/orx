/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008- Orx-Project
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

#include "main/android/orxAndroid.h"
#include "main/android/orxAndroidActivity.h"
#include <android/sensor.h>

/** Module flags
 */
#define orxJOYSTICK_KU32_STATIC_FLAG_NONE     0x00000000 /**< No flags */

#define orxJOYSTICK_KU32_STATIC_FLAG_READY    0x00000001 /**< Ready flag */

#define orxJOYSTICK_KU32_STATIC_MASK_ALL      0xFFFFFFFF /**< All mask */

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxJOYSTICK_STATIC_t {
  orxU32                u32Flags;

  orxU32                u32JoystickCount;

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

static void orxJoystick_Android_GameControllerStatusCallback(const int32_t controllerIndex,
                                                             const Paddleboat_ControllerStatus status,
                                                             void *)
{
  if(status == PADDLEBOAT_CONTROLLER_JUST_CONNECTED)
  {
    orxDEBUG_PRINT(orxDEBUG_LEVEL_JOYSTICK, "Joystick ID <%d> is becoming active", controllerIndex);

    /* Updates joystick count */
    sstJoystick.u32JoystickCount++;
  }
  else if(status == PADDLEBOAT_CONTROLLER_JUST_DISCONNECTED)
  {
    orxDEBUG_PRINT(orxDEBUG_LEVEL_JOYSTICK, "Joystick ID <%d> is becoming inactive", controllerIndex);

    /* Checks */
    orxASSERT(sstJoystick.u32JoystickCount > 0);

    /* Updates joystick count */
    sstJoystick.u32JoystickCount--;
  }
}

uint32_t orxJoystick_Android_GetButtonMask(orxS32 s32Button)
{
  uint32_t mask;

  switch (s32Button)
  {
    case orxJOYSTICK_BUTTON_1_1:
    case orxJOYSTICK_BUTTON_A_1:
      mask = PADDLEBOAT_BUTTON_A;
      break;
    case orxJOYSTICK_BUTTON_2_1:
    case orxJOYSTICK_BUTTON_B_1:
      mask = PADDLEBOAT_BUTTON_B;
      break;
    case orxJOYSTICK_BUTTON_3_1:
    case orxJOYSTICK_BUTTON_X_1:
      mask = PADDLEBOAT_BUTTON_X;
      break;
    case orxJOYSTICK_BUTTON_4_1:
    case orxJOYSTICK_BUTTON_Y_1:
      mask = PADDLEBOAT_BUTTON_Y;
      break;
    case orxJOYSTICK_BUTTON_5_1:
    case orxJOYSTICK_BUTTON_LBUMPER_1:
      mask = PADDLEBOAT_BUTTON_L1;
      break;
    case orxJOYSTICK_BUTTON_6_1:
    case orxJOYSTICK_BUTTON_RBUMPER_1:
      mask = PADDLEBOAT_BUTTON_R1;
      break;
    case orxJOYSTICK_BUTTON_7_1:
    case orxJOYSTICK_BUTTON_BACK_1:
      mask = PADDLEBOAT_BUTTON_SELECT;
      break;
    case orxJOYSTICK_BUTTON_8_1:
    case orxJOYSTICK_BUTTON_START_1:
      mask = PADDLEBOAT_BUTTON_START;
      break;
    case orxJOYSTICK_BUTTON_9_1:
    case orxJOYSTICK_BUTTON_LTHUMB_1:
      // TODO: Verify this one!
      mask = PADDLEBOAT_BUTTON_L3;
      break;
    case orxJOYSTICK_BUTTON_10_1:
    case orxJOYSTICK_BUTTON_RTHUMB_1:
      // TODO: Verify this one!
      mask = PADDLEBOAT_BUTTON_R3;
      break;
    case orxJOYSTICK_BUTTON_11_1:
    case orxJOYSTICK_BUTTON_UP_1:
      mask = PADDLEBOAT_BUTTON_DPAD_UP;
      break;
    case orxJOYSTICK_BUTTON_12_1:
    case orxJOYSTICK_BUTTON_RIGHT_1:
      mask = PADDLEBOAT_BUTTON_DPAD_RIGHT;
      break;
    case orxJOYSTICK_BUTTON_13_1:
    case orxJOYSTICK_BUTTON_DOWN_1:
      mask = PADDLEBOAT_BUTTON_DPAD_DOWN;
      break;
    case orxJOYSTICK_BUTTON_14_1:
    case orxJOYSTICK_BUTTON_LEFT_1:
      mask = PADDLEBOAT_BUTTON_DPAD_LEFT;
      break;
    case orxJOYSTICK_BUTTON_15_1:
    case orxJOYSTICK_BUTTON_GUIDE_1:
      mask = PADDLEBOAT_BUTTON_SYSTEM;
      break;
    default:
      mask = 0;
      break;
  }

  return mask;
}

static void orxJoystick_Android_CanonicalToScreen(int displayRotation,
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
    {-1, 1, 0, 1 },    /* ROTATION_0 */
    { 1, 1, 1, 0 },    /* ROTATION_90 */
    { 1, -1, 0, 1 },   /* ROTATION_180 */
    {-1, -1, 1, 0 } }; /* ROTATION_270 */

  const AxisSwap& as = axisSwap[displayRotation];
  screenVec[0] = (float)as.negateX * canVec[ as.xSrc ];
  screenVec[1] = (float)as.negateY * canVec[ as.ySrc ];
  screenVec[2] = canVec[2];
}

static void orxJoystick_Android_EnableSensorManager()
{
  if(!sstJoystick.bAccelerometerEnabled && sstJoystick.u32Frequency > 0)
  {
    ASensorEventQueue_enableSensor(sstJoystick.sensorEventQueue, sstJoystick.accelerometerSensor);
    ASensorEventQueue_setEventRate(sstJoystick.sensorEventQueue, sstJoystick.accelerometerSensor, (1000L / sstJoystick.u32Frequency) * 1000);
    sstJoystick.bAccelerometerEnabled = orxTRUE;
  }
}

static void orxJoystick_Android_DisableSensorManager()
{
  if(sstJoystick.bAccelerometerEnabled)
  {
    ASensorEventQueue_disableSensor(sstJoystick.sensorEventQueue, sstJoystick.accelerometerSensor);
    orxVector_Copy(&(sstJoystick.vAcceleration), &orxVECTOR_0);
    sstJoystick.bAccelerometerEnabled = orxFALSE;
  }
}

static orxFLOAT orxJoystick_Android_GetAccelerometerValue(orxJOYSTICK_AXIS _eAxis)
{
  orxFLOAT fResult;

  /* Depending on axis */
  switch (_eAxis)
  {
    case orxJOYSTICK_AXIS_LX_1:
      /* Updates result */
      fResult = sstJoystick.vAcceleration.fX;
      break;
    case orxJOYSTICK_AXIS_LY_1:
      /* Updates result */
      fResult = sstJoystick.vAcceleration.fY;
      break;
    case orxJOYSTICK_AXIS_RX_1:
      /* Updates result */
      fResult = sstJoystick.vAcceleration.fZ;
      break;
    default:
      /* Not available */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_JOYSTICK, "<%s> is not available on this platform!", orxJoystick_GetAxisName(_eAxis));
      fResult = orxFLOAT_0;
      break;
  }

  return fResult;
}

static orxSTATUS orxFASTCALL orxJoystick_Android_AccelerometerEventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  if(_pstEvent->eType == orxANDROID_EVENT_TYPE_SURFACE && _pstEvent->eID == orxANDROID_EVENT_SURFACE_CHANGED)
  {
    /* Resets rotation */
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

    while(ASensorEventQueue_getEvents(sstJoystick.sensorEventQueue, &event, 1) > 0)
    {
      in[0] = event.acceleration.x;
      in[1] = event.acceleration.y;
      in[2] = event.acceleration.z;

      orxJoystick_Android_CanonicalToScreen(sstJoystick.s32ScreenRotation, in, out);

      /* Gets new acceleration */
      orxVector_Set(&(sstJoystick.vAcceleration), out[0], out[1], out[2]);
    }
  }

  if(_pstEvent->eType == orxEVENT_TYPE_SYSTEM)
  {
    switch(_pstEvent->eID)
    {
      case orxSYSTEM_EVENT_FOCUS_GAINED:
        orxJoystick_Android_EnableSensorManager();
        break;
      case orxSYSTEM_EVENT_FOCUS_LOST:
        orxJoystick_Android_DisableSensorManager();
        break;
      default:
        break;
    }
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxJoystick_Android_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Wasn't already initialized? */
  if(!(sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstJoystick, sizeof(orxJOYSTICK_STATIC));

    orxASSERT(Paddleboat_isInitialized());
    /* Adds controller status callback */
    /* Note : Undocumented quirk. Without a controller status callback,
     *        connected devices will remain inactive. */
    Paddleboat_setControllerStatusCallback(orxJoystick_Android_GameControllerStatusCallback, NULL);

    orxConfig_PushSection(KZ_CONFIG_ANDROID);

    sstJoystick.s32ScreenRotation = -1;
    sstJoystick.bAccelerometerEnabled = orxFALSE;

    if(orxConfig_HasValue(KZ_CONFIG_ACCELEROMETER_FREQUENCY))
    {
      sstJoystick.u32Frequency = orxConfig_GetU32(KZ_CONFIG_ACCELEROMETER_FREQUENCY);
    }

    orxConfig_PopSection();

    /* Note : Avoid consuming battery if accelerometer is not used. */
    if(sstJoystick.u32Frequency > 0)
    {
      sstJoystick.sensorManager = ASensorManager_getInstance();
      sstJoystick.accelerometerSensor = ASensorManager_getDefaultSensor(sstJoystick.sensorManager, ASENSOR_TYPE_ACCELEROMETER);

      if(sstJoystick.accelerometerSensor != NULL)
      {
        /* Adds event handlers */
        if((eResult = orxEvent_AddHandler(orxEVENT_TYPE_SYSTEM, orxJoystick_Android_AccelerometerEventHandler)) != orxSTATUS_FAILURE)
        {
          orxEvent_SetHandlerIDFlags(orxJoystick_Android_AccelerometerEventHandler, orxEVENT_TYPE_SYSTEM, orxNULL, orxEVENT_GET_FLAG(orxSYSTEM_EVENT_FOCUS_LOST) | orxEVENT_GET_FLAG(orxSYSTEM_EVENT_FOCUS_GAINED), orxEVENT_KU32_MASK_ID_ALL);

          if((eResult = orxEvent_AddHandler(orxANDROID_EVENT_TYPE_ACCELERATE, orxJoystick_Android_AccelerometerEventHandler)) != orxSTATUS_FAILURE)
          {
            if((eResult = orxEvent_AddHandler(orxANDROID_EVENT_TYPE_SURFACE, orxJoystick_Android_AccelerometerEventHandler)) != orxSTATUS_FAILURE)
            {
              ALooper* looper = ALooper_forThread();
              sstJoystick.sensorEventQueue = ASensorManager_createEventQueue(sstJoystick.sensorManager, looper, LOOPER_ID_SENSOR, NULL, NULL);

              /* Enables sensor */
              orxJoystick_Android_EnableSensorManager();
            }
          }
        }
      }
    }

    /* Updates status */
    sstJoystick.u32Flags |= orxJOYSTICK_KU32_STATIC_FLAG_READY;
  }

  /* Done! */
  return eResult;
}

void orxFASTCALL orxJoystick_Android_Exit()
{
  /* Was initialized? */
  if(sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY)
  {
    /* Removes controller status callback */
    Paddleboat_setControllerStatusCallback(NULL, NULL);

    if(sstJoystick.accelerometerSensor != NULL)
    {
      /* Removes event handler */
      orxEvent_RemoveHandler(orxEVENT_TYPE_SYSTEM, orxJoystick_Android_AccelerometerEventHandler);
      orxEvent_RemoveHandler(orxANDROID_EVENT_TYPE_SURFACE, orxJoystick_Android_AccelerometerEventHandler);
      orxEvent_RemoveHandler(orxANDROID_EVENT_TYPE_ACCELERATE, orxJoystick_Android_AccelerometerEventHandler);

      orxJoystick_Android_DisableSensorManager();

      /* Destroys event queue */
      ASensorManager_destroyEventQueue(sstJoystick.sensorManager, sstJoystick.sensorEventQueue);
    }

    /* Cleans static controller */
    orxMemory_Zero(&sstJoystick, sizeof(orxJOYSTICK_STATIC));
  }

  return;
}

orxFLOAT orxFASTCALL orxJoystick_Android_GetAxisValue(orxJOYSTICK_AXIS _eAxis)
{
  orxU32    u32ID;
  orxFLOAT  fResult;
  orxBOOL   hasAccel;

  /* Checks */
  orxASSERT((sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY) == orxJOYSTICK_KU32_STATIC_FLAG_READY);
  orxASSERT(_eAxis < orxJOYSTICK_AXIS_NUMBER);

  /* Gets ID */
  u32ID = orxJOYSTICK_GET_ID_FROM_AXIS(_eAxis) - 1;

  /* Is ID valid? */
  if(u32ID < orxANDROID_KU32_MAX_JOYSTICK_NUMBER)
  {
    /* Has connected joysticks? */
    if(sstJoystick.u32JoystickCount > 0)
    {
      /* Plugged? */
      Paddleboat_Controller_Data controllerData;
      if(Paddleboat_getControllerData(u32ID, &controllerData) == PADDLEBOAT_NO_ERROR)
      {
        orxS32 s32Axis;
        
        /* Gets axis */
        s32Axis = _eAxis % orxJOYSTICK_AXIS_SINGLE_NUMBER;
        
        /* Updates result */
        switch(s32Axis)
        {
          case orxJOYSTICK_AXIS_LX_1:
            fResult = controllerData.leftStick.stickX;
            break;
          case orxJOYSTICK_AXIS_LY_1:
            fResult = controllerData.leftStick.stickY;
            break;
          case orxJOYSTICK_AXIS_RX_1:
            fResult = controllerData.rightStick.stickX;
            break;
          case orxJOYSTICK_AXIS_RY_1:
            fResult = controllerData.rightStick.stickY;
            break;
          case orxJOYSTICK_AXIS_LTRIGGER_1:
            fResult = controllerData.triggerL1;
            break;
          case orxJOYSTICK_AXIS_RTRIGGER_1:
            fResult = controllerData.triggerR1;
            break;
          default:
            /* Logs message */
            orxASSERT(orxFALSE && "Invalid joystick axis %d, this should *not* happen.");
            fResult = orxFLOAT_0;
            break;
        }
      }
      else
      {
        /* Updates result */
        fResult = orxFLOAT_0;
      }
    }
    else
    {
      if(sstJoystick.bAccelerometerEnabled)
      {
        /* Updates result */
        fResult = orxJoystick_Android_GetAccelerometerValue(_eAxis);
      }
      else
      {
        /* Updates result */
        fResult = orxFLOAT_0;
      }
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_JOYSTICK, "Requested joystick ID <%d> is out of range.", u32ID);

    /* Updates result */
    fResult = orxFLOAT_0;
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
  u32ID = orxJOYSTICK_GET_ID_FROM_BUTTON(_eButton) - 1;

  /* Is ID valid? */
  if(u32ID < orxANDROID_KU32_MAX_JOYSTICK_NUMBER)
  {
    /* Gets controller data if plugged */
    Paddleboat_Controller_Data controllerData;
    if(Paddleboat_getControllerData(u32ID, &controllerData) == PADDLEBOAT_NO_ERROR)
    {
      orxS32 s32Button;
      uint32_t buttonMask;

      /* Gets button */
      s32Button = _eButton % orxJOYSTICK_BUTTON_SINGLE_NUMBER;

      if (controllerData.buttonsDown)
      {
        orxDEBUG_PRINT(orxDEBUG_LEVEL_JOYSTICK, "Joystick buttons DOWN: <%d>", controllerData.buttonsDown);
      }

      buttonMask = orxJoystick_Android_GetButtonMask(s32Button);
      /* Updates result */
      bResult = orxFLAG_TEST(controllerData.buttonsDown, buttonMask) ? orxTRUE : orxFALSE;
    }
    else
    {
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

orxBOOL orxFASTCALL orxJoystick_Android_IsConnected(orxU32 _u32ID)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT((sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY) == orxJOYSTICK_KU32_STATIC_FLAG_READY);
  orxASSERT((_u32ID >= orxJOYSTICK_KU32_MIN_ID) && (_u32ID <= orxJOYSTICK_KU32_MAX_ID));

  /* Updates result */
  bResult = (_u32ID <= orxANDROID_KU32_MAX_JOYSTICK_NUMBER) && (Paddleboat_getControllerStatus(_u32ID - orxJOYSTICK_KU32_MIN_ID) == PADDLEBOAT_CONTROLLER_ACTIVE);

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
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_Android_IsConnected, JOYSTICK, IS_CONNECTED);
orxPLUGIN_USER_CORE_FUNCTION_END();
