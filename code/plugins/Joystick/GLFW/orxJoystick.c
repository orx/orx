/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2022 Orx-Project
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
 * @date 26/06/2010
 * @author iarwain@orx-project.org
 *
 * GLFW joystick plugin implementation
 *
 * @todo
 */


#include "orxPluginAPI.h"

#include "GLFW/glfw3.h"

#ifndef __orxEMBEDDED__
  #ifdef __orxMSVC__
    #pragma message("!!WARNING!! This plugin will only work in non-embedded mode when linked against a *DYNAMIC* version of GLFW!")
  #else /* __orxMSVC__ */
    #warning !!WARNING!! This plugin will only work in non-embedded mode when linked against a *DYNAMIC* version of GLFW!
  #endif /* __orxMSVC__ */
#endif /* __orxEMBEDDED__ */


/** Module flags
 */
#define orxJOYSTICK_KU32_STATIC_FLAG_NONE     0x00000000 /**< No flags */

#define orxJOYSTICK_KU32_STATIC_FLAG_READY    0x00000001 /**< Ready flag */

#define orxJOYSTICK_KU32_STATIC_MASK_ALL      0xFFFFFFFF /**< All mask */

#define orxJOYSTICK_KZ_CONFIG_MAPPING_LIST    "MappingList"
#define orxJOYSTICK_KZ_CONFIG_NAME            "JoyName"
#define orxJOYSTICK_KZ_CONFIG_ID              "JoyID"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Internal joystick info structure
 */
typedef struct __orxJOYSTICK_INFO_t
{
  orxBOOL               bIsConnected;
  orxBOOL               bIsGamepad;
  orxBOOL               bIsLTriggerRemapped, bIsRTriggerRemapped;
  float                 afAxisInfoList[orxJOYSTICK_AXIS_SINGLE_NUMBER];
  unsigned char         au8ButtonInfoList[orxJOYSTICK_BUTTON_SINGLE_NUMBER];

} orxJOYSTICK_INFO;

/** Static structure
 */
typedef struct __orxJOYSTICK_STATIC_t
{
  orxU32                u32Flags;
  orxJOYSTICK_INFO      astJoyInfoList[GLFW_JOYSTICK_LAST + 1];

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

/** Updated joystick info
 */
static void orxFASTCALL orxJoystick_GLFW_UpdateInfo(orxU32 _u32ID)
{
  /* Checks */
  orxASSERT(_u32ID <= GLFW_JOYSTICK_LAST);

  /* Is connected? */
  if(glfwJoystickPresent((int)_u32ID) != GLFW_FALSE)
  {
    GLFWgamepadstate  stState;
    orxS32            iButtonCount = 0;
    const orxU8      *au8Buttons;

    /* Wasn't connected? */
    if(sstJoystick.astJoyInfoList[_u32ID].bIsConnected == orxFALSE)
    {
      orxCHAR acJoystick[16];

      /* Pushes input section */
      orxConfig_PushSection(orxINPUT_KZ_CONFIG_SECTION);

      /* Stores its name */
      acJoystick[orxString_NPrint(acJoystick, sizeof(acJoystick) - 1, "%s%u", orxJOYSTICK_KZ_CONFIG_NAME, _u32ID + 1)] = orxCHAR_NULL;
      orxConfig_SetString(acJoystick, glfwGetJoystickName((int)_u32ID));

      /* Stores its id */
      acJoystick[orxString_NPrint(acJoystick, sizeof(acJoystick) - 1, "%s%u", orxJOYSTICK_KZ_CONFIG_ID, _u32ID + 1)] = orxCHAR_NULL;
      orxConfig_SetString(acJoystick, glfwGetJoystickGUID((int)_u32ID));

      /* Pops config section */
      orxConfig_PopSection();
    }

    /* Updates connection status */
    sstJoystick.astJoyInfoList[_u32ID].bIsConnected = orxTRUE;
    sstJoystick.astJoyInfoList[_u32ID].bIsGamepad   = glfwJoystickIsGamepad((int)_u32ID);

    /* Clears current buttons */
    orxMemory_Zero(sstJoystick.astJoyInfoList[_u32ID].au8ButtonInfoList, orxJOYSTICK_BUTTON_SINGLE_NUMBER * sizeof(unsigned char));

    /* Gets button values */
    au8Buttons = glfwGetJoystickButtons((int)_u32ID, (int *)&iButtonCount);
    orxMemory_Copy(sstJoystick.astJoyInfoList[_u32ID].au8ButtonInfoList + orxJOYSTICK_BUTTON_1_1, au8Buttons, orxMIN(iButtonCount, (orxS32)(orxJOYSTICK_BUTTON_SINGLE_NUMBER - orxJOYSTICK_BUTTON_1_1)) * sizeof(unsigned char));

    /* Is gamepad and can retrieve its state? */
    if((sstJoystick.astJoyInfoList[_u32ID].bIsGamepad != GLFW_FALSE)
    && (glfwGetGamepadState((int)_u32ID, &stState) != GLFW_FALSE))
    {
      /* Stores button values */
      orxMemory_Copy(sstJoystick.astJoyInfoList[_u32ID].au8ButtonInfoList, stState.buttons, orxMIN(GLFW_GAMEPAD_BUTTON_LAST + 1, orxJOYSTICK_BUTTON_1_1) * sizeof(unsigned char));

      /* Stores axes values */
      orxMemory_Copy(sstJoystick.astJoyInfoList[_u32ID].afAxisInfoList, stState.axes, orxMIN(GLFW_GAMEPAD_AXIS_LAST + 1, orxJOYSTICK_AXIS_SINGLE_NUMBER) * sizeof(float));
    }
    else
    {
      orxS32          iAxisCount = 0;
      const orxFLOAT *afAxes;

      /* Clears current axes */
      orxMemory_Zero(sstJoystick.astJoyInfoList[_u32ID].afAxisInfoList, orxJOYSTICK_AXIS_SINGLE_NUMBER * sizeof(orxFLOAT));

      /* Gets axes values */
      afAxes = glfwGetJoystickAxes((int)_u32ID, (int *)&iAxisCount);
      orxMemory_Copy(sstJoystick.astJoyInfoList[_u32ID].afAxisInfoList, afAxes, orxMIN(iAxisCount, (orxS32)orxJOYSTICK_AXIS_SINGLE_NUMBER) * sizeof(orxFLOAT));

      /* Mirrors low level buttons to high level ones */
      orxMemory_Copy(sstJoystick.astJoyInfoList[_u32ID].au8ButtonInfoList, sstJoystick.astJoyInfoList[_u32ID].au8ButtonInfoList + orxJOYSTICK_BUTTON_1_1, (GLFW_GAMEPAD_BUTTON_LAST + 1) * sizeof(unsigned char));
    }

    /* Should remap left trigger? */
    if(sstJoystick.astJoyInfoList[_u32ID].afAxisInfoList[orxJOYSTICK_AXIS_LTRIGGER_1] < -orxMATH_KF_EPSILON)
    {
      /* Updates status */
      sstJoystick.astJoyInfoList[_u32ID].bIsLTriggerRemapped = orxTRUE;
    }
    /* Should remap right trigger? */
    if(sstJoystick.astJoyInfoList[_u32ID].afAxisInfoList[orxJOYSTICK_AXIS_RTRIGGER_1] < -orxMATH_KF_EPSILON)
    {
      /* Updates status */
      sstJoystick.astJoyInfoList[_u32ID].bIsRTriggerRemapped = orxTRUE;
    }

    /* Is left trigger remapped? */
    if(sstJoystick.astJoyInfoList[_u32ID].bIsLTriggerRemapped != orxFALSE)
    {
      /* Applies remapping */
      sstJoystick.astJoyInfoList[_u32ID].afAxisInfoList[orxJOYSTICK_AXIS_LTRIGGER_1] = 0.5f * sstJoystick.astJoyInfoList[_u32ID].afAxisInfoList[orxJOYSTICK_AXIS_LTRIGGER_1] + 0.5f;
    }
    /* Is right trigger remapped? */
    if(sstJoystick.astJoyInfoList[_u32ID].bIsRTriggerRemapped != orxFALSE)
    {
      /* Applies remapping */
      sstJoystick.astJoyInfoList[_u32ID].afAxisInfoList[orxJOYSTICK_AXIS_RTRIGGER_1] = 0.5f * sstJoystick.astJoyInfoList[_u32ID].afAxisInfoList[orxJOYSTICK_AXIS_RTRIGGER_1] + 0.5f;
    }
  }
  else
  {
    /* Was connected? */
    if(sstJoystick.astJoyInfoList[_u32ID].bIsConnected != orxFALSE)
    {
      orxCHAR acJoystick[16];

      /* Pushes input section */
      orxConfig_PushSection(orxINPUT_KZ_CONFIG_SECTION);

      /* Removes its name */
      acJoystick[orxString_NPrint(acJoystick, sizeof(acJoystick) - 1, "%s%u", orxJOYSTICK_KZ_CONFIG_NAME, _u32ID + 1)] = orxCHAR_NULL;
      orxConfig_ClearValue(acJoystick);

      /* Removes its id */
      acJoystick[orxString_NPrint(acJoystick, sizeof(acJoystick) - 1, "%s%u", orxJOYSTICK_KZ_CONFIG_ID, _u32ID + 1)] = orxCHAR_NULL;
      orxConfig_ClearValue(acJoystick);

      /* Pops config section */
      orxConfig_PopSection();
    }

    /* Clears info */
    orxMemory_Zero(&sstJoystick.astJoyInfoList[_u32ID], sizeof(orxJOYSTICK_INFO));
  }

  /* Done! */
  return;
}

/** Update callback
 */
static void orxFASTCALL orxJoystick_GLFW_Update(const orxCLOCK_INFO *_pstClockInfo, void *_pContext)
{
  orxU32 i;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxJoystick_Update");

  /* For all joysticks */
  for(i = 0; i <= GLFW_JOYSTICK_LAST; i++)
  {
    /* Updates its info */
    orxJoystick_GLFW_UpdateInfo(i);
  }

  /* Is connected? */
  if(sstJoystick.astJoyInfoList[0].bIsConnected != orxFALSE)
  {
    orxSYSTEM_EVENT_PAYLOAD stPayload;

    /* Inits event payload */
    orxMemory_Zero(&stPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
    stPayload.stAccelerometer.dTime = orxSystem_GetTime();
    stPayload.stAccelerometer.vAcceleration.fX = sstJoystick.astJoyInfoList[0].afAxisInfoList[orxJOYSTICK_AXIS_LX_1];
    stPayload.stAccelerometer.vAcceleration.fY = sstJoystick.astJoyInfoList[0].afAxisInfoList[orxJOYSTICK_AXIS_LY_1];
    stPayload.stAccelerometer.vAcceleration.fZ = sstJoystick.astJoyInfoList[0].afAxisInfoList[orxJOYSTICK_AXIS_RX_1];

    /* Sends accelerometer event */
    orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_ACCELERATE, orxNULL, orxNULL, &stPayload);
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return;
}

orxSTATUS orxFASTCALL orxJoystick_GLFW_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Was already initialized. */
  if(!(sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstJoystick, sizeof(orxJOYSTICK_STATIC));

    /* Registers event update function */
    eResult = orxClock_Register(orxClock_Get(orxCLOCK_KZ_CORE), orxJoystick_GLFW_Update, orxNULL, orxMODULE_ID_JOYSTICK, orxCLOCK_PRIORITY_HIGHER);

    /* Success? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Is config module initialized? */
      if(orxModule_IsInitialized(orxMODULE_ID_CONFIG) != orxFALSE)
      {
        orxS32 i, s32Count;

        /* Pushes input section */
        orxConfig_PushSection(orxINPUT_KZ_CONFIG_SECTION);

        /* For all defined mappings */
        for(i = 0, s32Count = orxConfig_GetListCount(orxJOYSTICK_KZ_CONFIG_MAPPING_LIST); i < s32Count; i++)
        {
          /* Updates database mappings */
          glfwUpdateGamepadMappings(orxConfig_GetListString(orxJOYSTICK_KZ_CONFIG_MAPPING_LIST, i));
        }

        /* Clears any error */
        glfwGetError(NULL);

        /* Pops config section */
        orxConfig_PopSection();
      }

      /* Updates status */
      sstJoystick.u32Flags |= orxJOYSTICK_KU32_STATIC_FLAG_READY;

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
  }

  /* Done! */
  return eResult;
}

void orxFASTCALL orxJoystick_GLFW_Exit()
{
  /* Was initialized? */
  if(sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY)
  {
    /* Unregisters update function */
    orxClock_Unregister(orxClock_Get(orxCLOCK_KZ_CORE), orxJoystick_GLFW_Update);

    /* Cleans static controller */
    orxMemory_Zero(&sstJoystick, sizeof(orxJOYSTICK_STATIC));
  }

  return;
}

orxFLOAT orxFASTCALL orxJoystick_GLFW_GetAxisValue(orxJOYSTICK_AXIS _eAxis)
{
  orxU32    u32ID;
  orxFLOAT  fResult;

  /* Checks */
  orxASSERT((sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY) == orxJOYSTICK_KU32_STATIC_FLAG_READY);
  orxASSERT(_eAxis < orxJOYSTICK_AXIS_NUMBER);

  /* Gets ID */
  u32ID = (orxU32)_eAxis / orxJOYSTICK_AXIS_SINGLE_NUMBER;

  /* Is ID valid? */
  if(u32ID <= (orxU32)GLFW_JOYSTICK_LAST)
  {
    /* Plugged? */
    if(sstJoystick.astJoyInfoList[u32ID].bIsConnected != orxFALSE)
    {
      orxS32 s32Axis;

      /* Gets axis */
      s32Axis = _eAxis % orxJOYSTICK_AXIS_SINGLE_NUMBER;

      /* Updates result */
      fResult = orx2F(sstJoystick.astJoyInfoList[u32ID].afAxisInfoList[s32Axis]);
    }
    else
    {
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

  /* Done! */
  return fResult;
}

orxBOOL orxFASTCALL orxJoystick_GLFW_IsButtonPressed(orxJOYSTICK_BUTTON _eButton)
{
  orxU32  u32ID;
  orxBOOL bResult;

  /* Checks */
  orxASSERT((sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY) == orxJOYSTICK_KU32_STATIC_FLAG_READY);
  orxASSERT(_eButton < orxJOYSTICK_BUTTON_NUMBER);

  /* Gets ID */
  u32ID = (orxU32)_eButton / orxJOYSTICK_BUTTON_SINGLE_NUMBER;

  /* Is ID valid? */
  if(u32ID <= (orxU32)GLFW_JOYSTICK_LAST)
  {
    /* Plugged? */
    if(sstJoystick.astJoyInfoList[u32ID].bIsConnected != orxFALSE)
    {
      orxS32 s32Button;

      /* Gets button */
      s32Button = _eButton % orxJOYSTICK_BUTTON_SINGLE_NUMBER;

      /* Updates result */
      bResult = (sstJoystick.astJoyInfoList[u32ID].au8ButtonInfoList[s32Button] != GLFW_RELEASE) ? orxTRUE : orxFALSE;
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

orxBOOL orxFASTCALL orxJoystick_GLFW_IsConnected(orxU32 _u32ID)
{
  /* Checks */
  orxASSERT((sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY) == orxJOYSTICK_KU32_STATIC_FLAG_READY);
  orxASSERT((_u32ID >= orxJOYSTICK_KU32_MIN_ID) && (_u32ID <= orxJOYSTICK_KU32_MAX_ID));

  /* Done! */
  return sstJoystick.astJoyInfoList[_u32ID - orxJOYSTICK_KU32_MIN_ID].bIsConnected;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(JOYSTICK);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_GLFW_Init, JOYSTICK, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_GLFW_Exit, JOYSTICK, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_GLFW_GetAxisValue, JOYSTICK, GET_AXIS_VALUE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_GLFW_IsButtonPressed, JOYSTICK, IS_BUTTON_PRESSED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_GLFW_IsConnected, JOYSTICK, IS_CONNECTED);
orxPLUGIN_USER_CORE_FUNCTION_END();
