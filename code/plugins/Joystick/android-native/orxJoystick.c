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
 * @file orxJoystick.c
 * @date 13/01/2011
 * @author simons.philippe@gmail.com
 *
 * Android joystick plugin implementation
 *
 * @todo
 */


#include "orxPluginAPI.h"

#include <android/sensor.h>

/** Module flags
 */
#define orxJOYSTICK_KU32_STATIC_FLAG_NONE     0x00000000 /**< No flags */

#define orxJOYSTICK_KU32_STATIC_FLAG_READY    0x00000001 /**< Ready flag */

#define orxJOYSTICK_KU32_STATIC_MASK_ALL      0xFFFFFFFF /**< All mask */


#define orxANDROID_KZ_CONFIG_SECTION                   "Android"
#define orxANDROID_KZ_CONFIG_ACCELEROMETER_FREQUENCY   "AccelerometerFrequency"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxJOYSTICK_STATIC_t
{
  orxU32      u32Flags;
  orxVECTOR   vAcceleration;
  orxFLOAT    fFrequency;
} orxJOYSTICK_STATIC;

/* Defined in orxAndroid.h */

extern const ASensor       *poAccelerometerSensor;
extern ASensorEventQueue   *poSensorEventQueue;

#ifdef	__cplusplus
extern "C" {
#endif
  void orxAndroid_EnableAccelerometer(orxFLOAT fFrequency);
#ifdef	__cplusplus
}
#endif

/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxJOYSTICK_STATIC sstJoystick;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/
static orxSTATUS orxFASTCALL orxJoystick_Android_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

	/* Depending on ID */
	switch (_pstEvent->eID)
	{
	  case orxSYSTEM_EVENT_ACCELERATE:
	  {
	    orxSYSTEM_EVENT_PAYLOAD *pstPayload;

	    /* Gets payload */
	    pstPayload = (orxSYSTEM_EVENT_PAYLOAD *) _pstEvent->pstPayload;

      /* Gets new acceleration */
      orxVector_Set(&(sstJoystick.vAcceleration), orx2F(-pstPayload->stAccelerometer.vAcceleration.fX), orx2F(-pstPayload->stAccelerometer.vAcceleration.fY), orx2F(pstPayload->stAccelerometer.vAcceleration.fZ));

	    break;
	  }
	  case orxSYSTEM_EVENT_BACKGROUND:
	  {
      if (poAccelerometerSensor != orxNULL)
      {
        ASensorEventQueue_disableSensor(poSensorEventQueue, poAccelerometerSensor);
      }

      break;
	  }
	  case orxSYSTEM_EVENT_FOREGROUND:
	  {
      if (poAccelerometerSensor != orxNULL)
      {
        ASensorEventQueue_enableSensor(poSensorEventQueue, poAccelerometerSensor);
        ASensorEventQueue_setEventRate(poSensorEventQueue, poAccelerometerSensor, (1000L/sstJoystick.fFrequency)*1000);
      }

      break;
	  }
  }
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
    
    /* Pushes config section */
    orxConfig_PushSection(orxANDROID_KZ_CONFIG_SECTION);

    /* Has valid accelerometer frequency? */
    if(orxConfig_HasValue(orxANDROID_KZ_CONFIG_ACCELEROMETER_FREQUENCY) != orxFALSE)
    {
      orxFLOAT fFrequency;
      
      /* Valid? */
      if((fFrequency = orxConfig_GetFloat(orxANDROID_KZ_CONFIG_ACCELEROMETER_FREQUENCY)) > orxFLOAT_0)
      {
        sstJoystick.fFrequency = fFrequency;
 
        orxAndroid_EnableAccelerometer(fFrequency);

        orxEvent_AddHandler(orxEVENT_TYPE_SYSTEM, orxJoystick_Android_EventHandler);
      }
      else
      {
        poAccelerometerSensor = orxNULL;
        poSensorEventQueue = orxNULL;
        sstJoystick.fFrequency = orx2F(0);
      }
    }
    else
    {
      poAccelerometerSensor = orxNULL;
      poSensorEventQueue = orxNULL;
      sstJoystick.fFrequency = orx2F(0);
    }

    /* Updates status */
    sstJoystick.u32Flags |= orxJOYSTICK_KU32_STATIC_FLAG_READY;

    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

void orxFASTCALL orxJoystick_Android_Exit()
{
  /* Was initialized? */
  if(sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY)
  {
    if(poAccelerometerSensor != orxNULL)
    {
      /* Removes event handler */
      orxEvent_RemoveHandler(orxEVENT_TYPE_SYSTEM, orxJoystick_Android_EventHandler);

      ASensorEventQueue_disableSensor(poSensorEventQueue, poAccelerometerSensor);
    }
    /* Cleans static controller */
    orxMemory_Zero(&sstJoystick, sizeof(orxJOYSTICK_STATIC));
  }
  
  return;
}

orxFLOAT orxFASTCALL orxJoystick_Android_GetAxisValue(orxJOYSTICK_AXIS _eAxis)
{
  orxFLOAT fResult = orxFLOAT_0;

  /* Depending on axis */
  switch(_eAxis)
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
