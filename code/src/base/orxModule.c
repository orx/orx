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
 * @file orxModule.c
 * @date 12/09/2005
 * @author iarwain@orx-project.org
 *
 */


#include "base/orxModule.h"

#include "orxKernel.h"
#include "orxUtils.h"

#ifdef __orxMSVC__
  #pragma warning(disable : 4276)
#endif /* __orxMSVC__ */

/** Module registration macro
 */
#define orxMODULE_REGISTER(MODULE_ID, MODULE_BASENAME)  orxModule_Register(orxMODULE_ID_##MODULE_ID, #MODULE_ID, MODULE_BASENAME##_Setup, MODULE_BASENAME##_Init, MODULE_BASENAME##_Exit)


/** Registers all engine modules
 */
static orxINLINE void orxModule_RegisterAll()
{
  /* *** All modules registration *** */
  orxMODULE_REGISTER(ANIM, orxAnim);
  orxMODULE_REGISTER(ANIMPOINTER, orxAnimPointer);
  orxMODULE_REGISTER(ANIMSET, orxAnimSet);
  orxMODULE_REGISTER(BANK, orxBank);
  orxMODULE_REGISTER(BODY, orxBody);
  orxMODULE_REGISTER(CAMERA, orxCamera);
  orxMODULE_REGISTER(CLOCK, orxClock);
  orxMODULE_REGISTER(COMMAND, orxCommand);
  orxMODULE_REGISTER(CONFIG, orxConfig);
  orxMODULE_REGISTER(CONSOLE, orxConsole);
  orxMODULE_REGISTER(DISPLAY, orxDisplay);
  orxMODULE_REGISTER(EVENT, orxEvent);
  orxMODULE_REGISTER(FILE, orxFile);
  orxMODULE_REGISTER(FONT, orxFont);
  orxMODULE_REGISTER(FPS, orxFPS);
  orxMODULE_REGISTER(FRAME, orxFrame);
  orxMODULE_REGISTER(FX, orxFX);
  orxMODULE_REGISTER(FXPOINTER, orxFXPointer);
  orxMODULE_REGISTER(GRAPHIC, orxGraphic);
  orxMODULE_REGISTER(INPUT, orxInput);
  orxMODULE_REGISTER(JOYSTICK, orxJoystick);
  orxMODULE_REGISTER(KEYBOARD, orxKeyboard);
  orxMODULE_REGISTER(LOCALE, orxLocale);
  orxMODULE_REGISTER(MEMORY, orxMemory);
  orxMODULE_REGISTER(MOUSE, orxMouse);
  orxMODULE_REGISTER(OBJECT, orxObject);
  orxMODULE_REGISTER(PARAM, orxParam);
  orxMODULE_REGISTER(PHYSICS, orxPhysics);
  orxMODULE_REGISTER(PLUGIN, orxPlugin);
  orxMODULE_REGISTER(PROFILER, orxProfiler);
  orxMODULE_REGISTER(RENDER, orxRender);
  orxMODULE_REGISTER(RESOURCE, orxResource);
  orxMODULE_REGISTER(SCREENSHOT, orxScreenshot);
  orxMODULE_REGISTER(SHADER, orxShader);
  orxMODULE_REGISTER(SHADERPOINTER, orxShaderPointer);
  orxMODULE_REGISTER(SOUND, orxSound);
  orxMODULE_REGISTER(SOUNDPOINTER, orxSoundPointer);
  orxMODULE_REGISTER(SOUNDSYSTEM, orxSoundSystem);
  orxMODULE_REGISTER(SPAWNER, orxSpawner);
  orxMODULE_REGISTER(STRING, orxString);
  orxMODULE_REGISTER(STRUCTURE, orxStructure);
  orxMODULE_REGISTER(SYSTEM, orxSystem);
  orxMODULE_REGISTER(TEXT, orxText);
  orxMODULE_REGISTER(TEXTURE, orxTexture);
  orxMODULE_REGISTER(THREAD, orxThread);
  orxMODULE_REGISTER(TIMELINE, orxTimeLine);
  orxMODULE_REGISTER(VIEWPORT, orxViewport);

  /* Done! */
  return;
}

/** Module flags
 */
#define orxMODULE_KU32_STATIC_FLAG_NONE         0x00000000


/** Module status flags
 */
#define orxMODULE_KU32_STATUS_FLAG_NONE         0x00000000

#define orxMODULE_KU32_STATUS_FLAG_REGISTERED   0x00000001
#define orxMODULE_KU32_STATUS_FLAG_INITIALIZED  0x00000002
#define orxMODULE_KU32_STATUS_FLAG_PENDING      0x00010000

#define orxMODULE_KU32_STATUS_MASK_ALL          0xFFFFFFFF


/** Misc
 */
#define orxMODULE_KU32_NAME_SIZE                32


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Internal module info structure
 */
typedef struct __orxMODULE_INFO_t
{
  orxU64                    u64DependFlags;                 /**< Dependency flags : 8 */
  orxU64                    u64OptionalDependFlags;         /**< Optional dependency flags : 16 */
  orxU64                    u64ParentFlags;                 /**< Parent flags : 24 */
  orxMODULE_SETUP_FUNCTION  pfnSetup;                       /**< Setup function : 28 */
  orxMODULE_INIT_FUNCTION   pfnInit;                        /**< Init function : 32 */
  orxMODULE_EXIT_FUNCTION   pfnExit;                        /**< Exit function : 36 */
  orxU32                    u32StatusFlags;                 /**< Status flags : 40 */
  orxCHAR                   acName[orxMODULE_KU32_NAME_SIZE]; /**< Name : 72 */

} orxMODULE_INFO;

/** Static structure
 */
typedef struct __orxMODULE_STATIC_t
{
  orxMODULE_INFO            astModuleInfo[orxMODULE_ID_TOTAL_NUMBER];
  orxU32                    u32InitLoopCount;
  orxU32                    u32InitCount;

} orxMODULE_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** static data
 */
static orxMODULE_STATIC sstModule;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Calls a module setup
 */
static orxINLINE void orxModule_Setup(orxMODULE_ID _eModuleID)
{
  /* Checks */
  orxASSERT(_eModuleID < orxMODULE_ID_TOTAL_NUMBER);

  /* Is registered? */
  if(sstModule.astModuleInfo[_eModuleID].u32StatusFlags & orxMODULE_KU32_STATUS_FLAG_REGISTERED)
  {
    /* Has setup function? */
    if(sstModule.astModuleInfo[_eModuleID].pfnSetup != orxNULL)
    {
      /* Calls it */
      sstModule.astModuleInfo[_eModuleID].pfnSetup();
    }
  }

  /* Done! */
  return;
}

/** Calls all module setups
 */
static orxINLINE void orxModule_SetupAll()
{
  orxU32 eID;

  /* Clears static variable */
  sstModule.u32InitLoopCount = 0;

  /* For all modules */
  for(eID = 0; eID < orxMODULE_ID_TOTAL_NUMBER; eID++)
  {
    /* Calls module setup */
    orxModule_Setup((orxMODULE_ID)eID);
  }

  /* Done! */
  return;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Registers a module
 * @param[in]   _eModuleID                Concerned module ID
 * @param[in]   _zModuleName              Module name
 * @param[in]   _pfnSetup                 Module setup callback
 * @param[in]   _pfnInit                  Module init callback
 * @param[in]   _pfnExit                  Module exit callback
 */
void orxFASTCALL orxModule_Register(orxMODULE_ID _eModuleID, const orxSTRING _zModuleName, const orxMODULE_SETUP_FUNCTION _pfnSetup, const orxMODULE_INIT_FUNCTION _pfnInit, const orxMODULE_EXIT_FUNCTION _pfnExit)
{
  /* Checks */
  orxASSERT(_eModuleID < orxMODULE_ID_TOTAL_NUMBER);

  /* Clears module info */
  orxMemory_Zero(&(sstModule.astModuleInfo[_eModuleID]), sizeof(orxMODULE_INFO));

  /* Stores its name */
  orxString_NPrint(sstModule.astModuleInfo[_eModuleID].acName, orxMODULE_KU32_NAME_SIZE - 1, _zModuleName);

  /* Stores module functions */
  sstModule.astModuleInfo[_eModuleID].pfnSetup  = _pfnSetup;
  sstModule.astModuleInfo[_eModuleID].pfnInit   = _pfnInit;
  sstModule.astModuleInfo[_eModuleID].pfnExit   = _pfnExit;

  /* Updates module status flags */
  sstModule.astModuleInfo[_eModuleID].u32StatusFlags = orxMODULE_KU32_STATUS_FLAG_REGISTERED;

  /* Done! */
  return;
}

/** Adds dependencies between 2 modules
 */
void orxFASTCALL orxModule_AddDependency(orxMODULE_ID _eModuleID, orxMODULE_ID _eDependID)
{
  /* Checks */
  orxASSERT(_eModuleID < orxMODULE_ID_TOTAL_NUMBER);
  orxASSERT(_eDependID < orxMODULE_ID_TOTAL_NUMBER);

  /* Stores dependency */
  sstModule.astModuleInfo[_eModuleID].u64DependFlags |= ((orxU64)1) << _eDependID;

  /* Done! */
  return;
}

/** Adds optional dependencies between 2 modules
 */
void orxFASTCALL orxModule_AddOptionalDependency(orxMODULE_ID _eModuleID, orxMODULE_ID _eDependID)
{
  /* Checks */
  orxASSERT(_eModuleID < orxMODULE_ID_TOTAL_NUMBER);
  orxASSERT(_eDependID < orxMODULE_ID_TOTAL_NUMBER);

  /* Stores dependency */
  sstModule.astModuleInfo[_eModuleID].u64OptionalDependFlags |= ((orxU64)1) << _eDependID;

  /* Done! */
  return;
}

/** Inits a module recursively
 */
orxSTATUS orxFASTCALL orxModule_Init(orxMODULE_ID _eModuleID)
{
  orxU32    u32Index;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(orxMODULE_ID_CORE_NUMBER <= orxMODULE_ID_TOTAL_NUMBER);
  orxASSERT(_eModuleID < orxMODULE_ID_TOTAL_NUMBER);

  /* First init? */
  if((sstModule.u32InitLoopCount == 0) && (sstModule.u32InitCount == 0))
  {
    /* Registers all other modules */
    orxModule_RegisterAll();

    /* Setups all modules */
    orxModule_SetupAll();
  }

  /* Increases loop count */
  sstModule.u32InitLoopCount++;

  /* Is module registered? */
  if(sstModule.astModuleInfo[_eModuleID].u32StatusFlags & orxMODULE_KU32_STATUS_FLAG_REGISTERED)
  {
    /* Is not initialized? */
    if(!(sstModule.astModuleInfo[_eModuleID].u32StatusFlags & (orxMODULE_KU32_STATUS_FLAG_INITIALIZED|orxMODULE_KU32_STATUS_FLAG_PENDING)))
    {
      orxU64 u64Depend;

      /* For all dependencies */
      for(u64Depend = sstModule.astModuleInfo[_eModuleID].u64DependFlags, u32Index = 0;
          u64Depend != (orxU64)0;
          u64Depend >>= 1, u32Index++)
      {
        /* Depends? */
        if(u64Depend & (orxU64)1)
        {
          /* Not already initialized */
          if(!(sstModule.astModuleInfo[u32Index].u32StatusFlags & (orxMODULE_KU32_STATUS_FLAG_INITIALIZED|orxMODULE_KU32_STATUS_FLAG_PENDING)))
          {
            /* Inits it */
            eResult = orxModule_Init((orxMODULE_ID)u32Index);

            /* Success ? */
            if(eResult != orxSTATUS_FAILURE)
            {
              /* Updates flags */
              sstModule.astModuleInfo[u32Index].u64ParentFlags |= (orxU64)1 << _eModuleID;
            }
            else
            {
              /* Stops init here */
              break;
            }
          }
        }
      }

      /* Success? */
      if(eResult != orxSTATUS_FAILURE)
      {
        /* For all optional dependencies */
        for(u64Depend = sstModule.astModuleInfo[_eModuleID].u64OptionalDependFlags, u32Index = 0;
            u64Depend != (orxU64)0;
            u64Depend >>= 1, u32Index++)
        {
          /* Depends? */
          if(u64Depend & (orxU64)1)
          {
            /* Not already initialized */
            if(!(sstModule.astModuleInfo[u32Index].u32StatusFlags & orxMODULE_KU32_STATUS_FLAG_INITIALIZED))
            {
              /* Inits it */
              if(orxModule_Init((orxMODULE_ID)u32Index) != orxSTATUS_FAILURE)
              {
                /* Updates flags */
                sstModule.astModuleInfo[u32Index].u64ParentFlags |= (orxU64)1 << _eModuleID;
              }
            }
          }
        }

        /* Not already initialized */
        if(!(sstModule.astModuleInfo[_eModuleID].u32StatusFlags & orxMODULE_KU32_STATUS_FLAG_INITIALIZED))
        {
          /* Updates temp flag */
          sstModule.astModuleInfo[_eModuleID].u32StatusFlags |= orxMODULE_KU32_STATUS_FLAG_PENDING;

          /* Calls module init function */
          eResult = (sstModule.astModuleInfo[_eModuleID].pfnInit != orxNULL) ? sstModule.astModuleInfo[_eModuleID].pfnInit() : orxSTATUS_SUCCESS;

          /* Successful? */
          if(eResult != orxSTATUS_FAILURE)
          {
            /* Updates initialized flag */
            sstModule.astModuleInfo[_eModuleID].u32StatusFlags |= orxMODULE_KU32_STATUS_FLAG_INITIALIZED;

            /* Updates count */
            sstModule.u32InitCount++;
          }
          else
          {
            /* Updates temp flag */
            sstModule.astModuleInfo[_eModuleID].u32StatusFlags &= ~orxMODULE_KU32_STATUS_FLAG_PENDING;
          }
        }
      }
    }
  }
  else
  {
    /* Not initialized */
    eResult = orxSTATUS_FAILURE;
  }

  /* Decreases loop count */
  sstModule.u32InitLoopCount--;

  /* Was external call? */
  if(sstModule.u32InitLoopCount == 0)
  {
    /* Success? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* For all modules */
      for(u32Index = 0; u32Index < orxMODULE_ID_TOTAL_NUMBER; u32Index++)
      {
        /* Cleans temp status */
        sstModule.astModuleInfo[u32Index].u32StatusFlags &= ~orxMODULE_KU32_STATUS_FLAG_PENDING;
      }
    }

    /* Is param initialized? */
    if(orxModule_IsInitialized(orxMODULE_ID_PARAM) != orxFALSE)
    {
      /* Displays help */
      if(orxParam_DisplayHelp() == orxSTATUS_FAILURE)
      {
        /* Updates result */
        eResult = orxSTATUS_FAILURE;
      }
    }

    /* Failure? */
    if(eResult == orxSTATUS_FAILURE)
    {
      /* Updates temp flag */
      sstModule.astModuleInfo[_eModuleID].u32StatusFlags |= orxMODULE_KU32_STATUS_FLAG_PENDING;

      /* Exits from module */
      orxModule_Exit(_eModuleID);
    }
  }

  /* Done! */
  return eResult;
}

/** Exits from a module recursively
 */
void orxFASTCALL orxModule_Exit(orxMODULE_ID _eModuleID)
{
  /* Checks */
  orxASSERT(_eModuleID < orxMODULE_ID_TOTAL_NUMBER);

  /* Is initialized? */
  if(sstModule.astModuleInfo[_eModuleID].u32StatusFlags & (orxMODULE_KU32_STATUS_FLAG_INITIALIZED|orxMODULE_KU32_STATUS_FLAG_PENDING))
  {
    orxU64 u64Depend;
    orxU32 u32Index;

    /* Cleans flags */
    sstModule.astModuleInfo[_eModuleID].u32StatusFlags &= ~(orxMODULE_KU32_STATUS_FLAG_INITIALIZED|orxMODULE_KU32_STATUS_FLAG_PENDING);
    sstModule.astModuleInfo[_eModuleID].u64ParentFlags  = 0;

    /* Computes dependency flag */
    u64Depend = (orxU64)1 << _eModuleID;

    /* For all modules */
    for(u32Index = 0; u32Index < orxMODULE_ID_TOTAL_NUMBER; u32Index++)
    {
      /* Is module dependent? */
      if(sstModule.astModuleInfo[u32Index].u64DependFlags & u64Depend)
      {
        /* Exits from it */
        orxModule_Exit((orxMODULE_ID)u32Index);
      }
    }

    /* For all optional modules */
    for(u32Index = 0; u32Index < orxMODULE_ID_TOTAL_NUMBER; u32Index++)
    {
      /* Is module dependent? */
      if(sstModule.astModuleInfo[u32Index].u64OptionalDependFlags & u64Depend)
      {
        /* Exits from it */
        orxModule_Exit((orxMODULE_ID)u32Index);
      }
    }

    /* Has module exit function? */
    if(sstModule.astModuleInfo[_eModuleID].pfnExit != orxNULL)
    {
      /* Calls it */
      sstModule.astModuleInfo[_eModuleID].pfnExit();
    }

    /* For all modules */
    for(u32Index = 0; u32Index < orxMODULE_ID_TOTAL_NUMBER; u32Index++)
    {
      /* Has parents? */
      if(sstModule.astModuleInfo[u32Index].u64ParentFlags != 0)
      {
        /* Was this module one of its parents? */
        if(sstModule.astModuleInfo[u32Index].u64ParentFlags & u64Depend)
        {
          /* Removes self as parent */
          sstModule.astModuleInfo[u32Index].u64ParentFlags &= ~u64Depend;

          /* No more parents? */
          if(sstModule.astModuleInfo[u32Index].u64ParentFlags == 0)
          {
            /* Exits from it */
            orxModule_Exit((orxMODULE_ID)u32Index);
          }
        }
      }
    }

    /* Updates count */
    sstModule.u32InitCount--;
  }

  /* Done! */
  return;
}

/** Is module initialized?
 * @param[in] _eModulueID       Concerned module ID
 * @return orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxModule_IsInitialized(orxMODULE_ID _eModuleID)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT(_eModuleID < orxMODULE_ID_TOTAL_NUMBER);

  /* Updates result */
  bResult = orxFLAG_TEST(sstModule.astModuleInfo[_eModuleID].u32StatusFlags, orxMODULE_KU32_STATUS_FLAG_INITIALIZED) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

/** Gets module name
 * @param[in]   _eModuleID                Concerned module ID
 * @return Module name / orxSTRING_EMPTY
 */
const orxSTRING orxFASTCALL orxModule_GetName(orxMODULE_ID _eModuleID)
{
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(_eModuleID < orxMODULE_ID_TOTAL_NUMBER);

  /* Updates result */
  zResult = sstModule.astModuleInfo[_eModuleID].acName;

  /* Done! */
  return zResult;
}
