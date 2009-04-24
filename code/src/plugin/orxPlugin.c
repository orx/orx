/* Orx - Portable Game Engine
 *
 * Orx is the legal property of its developers, whose names
 * are listed in the COPYRIGHT file distributed
 * with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * @file orxPlugin.c
 * @date 04/09/2002
 * @author iarwain@orx-project.org
 *
 */


#include "plugin/orxPlugin.h"

#include "debug/orxDebug.h"
#include "core/orxConfig.h"
#include "main/orxParam.h"
#include "memory/orxBank.h"
#include "memory/orxMemory.h"
#include "plugin/orxPluginUser.h"
#include "plugin/orxPluginCore.h"
#include "utils/orxHashTable.h"


#if defined(__orxLINUX__) || defined (__orxMAC__) || defined(__orxGP2X__)

  #include <dlfcn.h>

#else /* __orxLINUX__ || __orxMAC__ || __orxGP2X__ */

  #ifdef __orxWINDOWS__

    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>

  #endif /* __orxWINDOWS__ */

#endif /* __orxLINUX__ || __orxMAC__ || __orxGP2X__ */


/** Platform dependent type & function defines
 */

/* WINDOWS */
#ifdef __orxWINDOWS__

  typedef HINSTANCE                                       orxSYSPLUGIN;

  #define orxPLUGIN_OPEN(PLUGIN)                          LoadLibrary(PLUGIN)
  #define orxPLUGIN_GET_SYMBOL_ADDRESS(PLUGIN, SYMBOL)    GetProcAddress(PLUGIN, SYMBOL)
  #define orxPLUGIN_CLOSE(PLUGIN)                         FreeLibrary(PLUGIN)

  static const orxSTRING                            szPluginLibraryExt = "dll";

/* OTHERS */
#else /* __orxWINDOWS__ */

  typedef void *                                       orxSYSPLUGIN;

  #define orxPLUGIN_OPEN(PLUGIN)                          dlopen(PLUGIN, RTLD_LAZY)
  #define orxPLUGIN_GET_SYMBOL_ADDRESS(PLUGIN, SYMBOL)    dlsym(PLUGIN, SYMBOL)
  #define orxPLUGIN_CLOSE(PLUGIN)                         dlclose(PLUGIN)

#ifdef __orxMAC__

  static const orxSTRING                            szPluginLibraryExt = "so";

#else /* __orxMAC__ */

  static const orxSTRING                            szPluginLibraryExt = "so";

#endif /* __orxMAC__ */

#endif /* __orxWINDOWS__ */


/** Module flags
 */

#define orxPLUGIN_KU32_STATIC_FLAG_NONE                     0x00000000L
#define orxPLUGIN_KU32_STATIC_FLAG_READY                    0x00000001L


#define orxPLUGIN_KU32_CORE_KU32_FLAG_FLAG_NONE             0x00000000L
#define orxPLUGIN_KU32_CORE_KU32_FLAG_FLAG_LOADED           0x00000001L
#define orxPLUGIN_KU32_CORE_KU32_FLAG_FLAG_DIRTY            0x10000000L


/** Misc
 */
#define orxPLUGIN_KU32_FUNCTION_BANK_SIZE                   16
#define orxPLUGIN_KZ_INIT_FUNCTION_NAME                     "orxPlugin_MainInit"  /**< Plugin init function name */

#define orxPLUGIN_KC_DIRECTORY_SEPARATOR                    '/'


#define orxPLUGIN_KZ_CONFIG_SECTION                         "Plugin"


#ifdef __orxDEBUG__

  #define orxPLUGIN_KZ_CONFIG_DEBUG_SUFFIX                  "DebugSuffix"

  #define orxPLUGIN_KZ_DEFAULT_DEBUG_SUFFIX                 "d"

#endif /* __orxDEBUG__ */


/** Plugin main function prototype
 */
typedef orxSTATUS (*orxPLUGIN_MAIN_FUNCTION)();


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Information structure on a plugin function
 */
typedef struct __orxPLUGIN_FUNCTION_INFO_t
{
  /* Function ID */
  orxPLUGIN_FUNCTION_ID eFunctionID;                        /**< Function ID : 8 */

  /* Function pointer */
  orxPLUGIN_FUNCTION    pfnFunction;                        /**< Function Address : 4 */

  /* Funtion name */
  orxSTRING             zFunctionName;                      /**< Function Name : 12 */

  /* Function argument types */
  orxSTRING             zFunctionArgs;                      /**< Function Argument Types : 16 */

} orxPLUGIN_FUNCTION_INFO;

/** Information structure on a plugin
 */
typedef struct __orxPLUGIN_INFO_t
{
  /* System plugin access */
  orxSYSPLUGIN  pstSysPlugin;                               /**< Plugin system pointer : 4 */

  /* Plugin handle */
  orxHANDLE     hPluginHandle;                              /**< Plugin handle : 8 */

  /* Function bank */
  orxBANK      *pstFunctionBank;                            /**< Function bank : 12 */

  /* Function hash table */
  orxHASHTABLE *pstFunctionTable;                           /**< Function hash table : 16 */

  /* Plugin name */
  orxSTRING     zPluginName;                                /**< Plugin name : 20 */

  /* Padding */
  orxPAD(20)

} orxPLUGIN_INFO;

/** Core info structure
 */
typedef struct __orxPLUGIN_CORE_INFO_t
{
  /* Core functions : 4 */
  orxPLUGIN_CORE_FUNCTION const *pstCoreFunctionTable;

  /* Core functions counter : 8 */
  orxU32                            u32CoreFunctionCounter;

  /* Core module ID : 12 */
  orxMODULE_ID                      eModuleID;

  /* Core info status : 16 */
  orxU32                            u32Flags;

  /* Pad */
  orxPAD(16)

} orxPLUGIN_CORE_INFO;

/** Static structure
 */
typedef struct __orxPLUGIN_STATIC_t
{
  /* Plugin bank */
  orxBANK *pstPluginBank;

  /* Core info table */
  orxPLUGIN_CORE_INFO astCoreInfo[orxPLUGIN_CORE_ID_NUMBER];

  /* Control flags */
  orxU32 u32Flags;

} orxPLUGIN_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxPLUGIN_STATIC sstPlugin;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Updates all modules
 */
static orxINLINE void orxPlugin_UpdateAllModule()
{
  orxU32 i;

  /* For all core plugins */
  for(i = 0; i < orxPLUGIN_CORE_ID_NUMBER; i++)
  {
    /* Is plugin dirty? */
    if(sstPlugin.astCoreInfo[i].u32Flags & orxPLUGIN_KU32_CORE_KU32_FLAG_FLAG_DIRTY)
    {
      orxU32  j;
      orxBOOL bLoaded;

      /* Checks all functions */
      for(j = 0, bLoaded = orxTRUE;
          (j < sstPlugin.astCoreInfo[i].u32CoreFunctionCounter) && (bLoaded != orxFALSE);
          j++)
      {
        /* Tests if function is loaded */
        bLoaded = (*(sstPlugin.astCoreInfo[i].pstCoreFunctionTable[j].pfnFunction) != sstPlugin.astCoreInfo[i].pstCoreFunctionTable[j].pfnDefaultFunction)
                  ? orxTRUE
                  : orxFALSE;
      }

      /* Was not already loaded */
      if(!(sstPlugin.astCoreInfo[i].u32Flags & orxPLUGIN_KU32_CORE_KU32_FLAG_FLAG_LOADED))
      {
        /* Is now loaded? */
        if(bLoaded != orxFALSE)
        {
          /* Marks as loaded */
          sstPlugin.astCoreInfo[i].u32Flags |= orxPLUGIN_KU32_CORE_KU32_FLAG_FLAG_LOADED;

          /* Tries to init associated module */
          orxModule_Init(sstPlugin.astCoreInfo[i].eModuleID);
        }
      }
      /* Was already loaded */
      else
      {
        /* Isn't loaded any longer */
        if(bLoaded == orxFALSE)
        {
          /* Marks as not loaded */
          sstPlugin.astCoreInfo[i].u32Flags &= ~orxPLUGIN_KU32_CORE_KU32_FLAG_FLAG_LOADED;

          /* Tries to exit from associated module */
          orxModule_Exit(sstPlugin.astCoreInfo[i].eModuleID);
        }
      }

      /* Removes dirty flag */
      sstPlugin.astCoreInfo[i].u32Flags &= ~orxPLUGIN_KU32_CORE_KU32_FLAG_FLAG_DIRTY;
    }
  }

  /* Done! */
  return;
}

/** Creates a function info
 * @param[in] _pstPluginInfo          Concerned plugin info
 * @return orxPLUGIN_FUNCTION_INFO / orxNULL
 */
static orxPLUGIN_FUNCTION_INFO *orxFASTCALL orxPlugin_CreateFunctionInfo(orxPLUGIN_INFO *_pstPluginInfo)
{
  orxPLUGIN_FUNCTION_INFO *pstFunctionInfo;

  /* Checks */
  orxASSERT(_pstPluginInfo != orxNULL);

  /* Creates a function info */
  pstFunctionInfo = (orxPLUGIN_FUNCTION_INFO *)orxBank_Allocate(_pstPluginInfo->pstFunctionBank);

  /* Valid? */
  if(pstFunctionInfo != orxNULL)
  {
    /* Inits it */
    orxMemory_Zero(pstFunctionInfo, sizeof(orxPLUGIN_FUNCTION_INFO));
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "Function into failed to allocate.");
  }

  /* Done! */
  return pstFunctionInfo;
}

/** Deletes a function info
 * @param[in] _pstPluginInfo          Concerned plugin info
 * @param[in] _pstFunctionInfo        Concerned function info
 */
static void orxFASTCALL orxPlugin_DeleteFunctionInfo(orxPLUGIN_INFO *_pstPluginInfo, orxPLUGIN_FUNCTION_INFO *_pstFunctionInfo)
{
  /* Checks */
  orxASSERT(_pstPluginInfo != orxNULL);
  orxASSERT(_pstFunctionInfo != orxNULL);

  /* Deletes it */
  orxBank_Free(_pstPluginInfo->pstFunctionBank, _pstFunctionInfo);

  /* Done */
  return;
}

/** Registers a core function
 * @param[in] _pfnFunctionInfo        Concerned function info
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS orxPlugin_RegisterCoreFunction(const orxPLUGIN_FUNCTION_INFO *_pfnFunctionInfo)
{
  const orxPLUGIN_CORE_FUNCTION *pstCoreFunction;
  orxU32                            u32PluginIndex, u32FunctionIndex;
  orxSTATUS                         eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(_pfnFunctionInfo != orxNULL);

  /* Gets plugin index */
  u32PluginIndex = (_pfnFunctionInfo->eFunctionID & orxPLUGIN_KU32_MASK_PLUGIN_ID) >> orxPLUGIN_KU32_SHIFT_PLUGIN_ID;

  /* Checks */
  orxASSERT(u32PluginIndex < orxPLUGIN_CORE_ID_NUMBER);

  /* Gets core function table */
  pstCoreFunction = sstPlugin.astCoreInfo[u32PluginIndex].pstCoreFunctionTable;

  /* Core plugin defined? */
  if(pstCoreFunction != orxNULL)
  {
    /* Gets function index */
    u32FunctionIndex = _pfnFunctionInfo->eFunctionID & orxPLUGIN_KU32_MASK_FUNCTION_ID;

    /* Checks */
    orxASSERT(u32FunctionIndex < sstPlugin.astCoreInfo[u32PluginIndex].u32CoreFunctionCounter);
    orxASSERT(pstCoreFunction[u32FunctionIndex].pfnFunction != orxNULL);

    /* Was not already loaded? */
    if(*(pstCoreFunction[u32FunctionIndex].pfnFunction) == pstCoreFunction[u32FunctionIndex].pfnDefaultFunction)
    {
      /* Registers core function */
      *(pstCoreFunction[u32FunctionIndex].pfnFunction) = _pfnFunctionInfo->pfnFunction;

      /* Updates plugin status */
      sstPlugin.astCoreInfo[u32PluginIndex].u32Flags |= orxPLUGIN_KU32_CORE_KU32_FLAG_FLAG_DIRTY;

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "Function address is not already loaded.");
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "Core plugin is not defined.");
  }

  /* Done! */
  return eResult;
}

/** Unregisters a core function
 * @param[in] _pfnFunctionInfo        Concerned function info
 */
static orxINLINE void orxPlugin_UnregisterCoreFunction(const orxPLUGIN_FUNCTION_INFO *_pfnFunctionInfo)
{
  const orxPLUGIN_CORE_FUNCTION *pstCoreFunction;
  orxU32 u32PluginIndex, u32FunctionIndex;

  /* Checks */
  orxASSERT(_pfnFunctionInfo != orxNULL);

  /* Gets plugin index */
  u32PluginIndex = (_pfnFunctionInfo->eFunctionID & orxPLUGIN_KU32_MASK_PLUGIN_ID) >> orxPLUGIN_KU32_SHIFT_PLUGIN_ID;

  /* Checks */
  orxASSERT(u32PluginIndex < orxPLUGIN_CORE_ID_NUMBER);

  /* Gets core function table */
  pstCoreFunction = sstPlugin.astCoreInfo[u32PluginIndex].pstCoreFunctionTable;

  /* Core plugin defined? */
  if(pstCoreFunction != orxNULL)
  {
    /* Gets function index */
    u32FunctionIndex = _pfnFunctionInfo->eFunctionID & orxPLUGIN_KU32_MASK_FUNCTION_ID;

    /* Checks */
    orxASSERT(u32FunctionIndex < sstPlugin.astCoreInfo[u32PluginIndex].u32CoreFunctionCounter);
    orxASSERT(pstCoreFunction[u32FunctionIndex].pfnFunction != orxNULL);

    /* Restores default core function */
    *(pstCoreFunction[u32FunctionIndex].pfnFunction) = pstCoreFunction[u32FunctionIndex].pfnDefaultFunction;

    /* Marks plugin as dirty */
    sstPlugin.astCoreInfo[u32PluginIndex].u32Flags |= orxPLUGIN_KU32_CORE_KU32_FLAG_FLAG_DIRTY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "Core plugin is not defined.");
  }

  /* Done! */
  return;
}

/** Creates a plugin info
 * @return orxPLUGIN_INFO / orxNULL
 */
static orxPLUGIN_INFO *orxFASTCALL orxPlugin_CreatePluginInfo()
{
  orxPLUGIN_INFO *pstPluginInfo;

  /* Creates a plugin info */
  pstPluginInfo = (orxPLUGIN_INFO *)orxBank_Allocate(sstPlugin.pstPluginBank);

  /* Valid? */
  if(pstPluginInfo != orxNULL)
  {
    /* Inits it */
    orxMemory_Zero(pstPluginInfo, sizeof(orxPLUGIN_INFO));

    /* Undefines plugin handle */
    pstPluginInfo->hPluginHandle      = orxHANDLE_UNDEFINED;

    /* Creates function bank */
    pstPluginInfo->pstFunctionBank    = orxBank_Create(orxPLUGIN_KU32_FUNCTION_BANK_SIZE, sizeof(orxPLUGIN_FUNCTION_INFO), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if(pstPluginInfo->pstFunctionBank != orxNULL)
    {
      /* Creates function hash table */
      pstPluginInfo->pstFunctionTable = orxHashTable_Create(orxPLUGIN_KU32_FUNCTION_BANK_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

      /* Invalid? */
      if(pstPluginInfo->pstFunctionTable == orxNULL)
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "Invalid function hash table.");

        /* Frees previously allocated data */
        orxBank_Delete(pstPluginInfo->pstFunctionBank);
        orxBank_Free(sstPlugin.pstPluginBank, pstPluginInfo);

        /* Not successful */
        pstPluginInfo = orxNULL;
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "Invalid function bank.");

      /* Frees previously allocated data */
      orxBank_Free(sstPlugin.pstPluginBank, pstPluginInfo);

      /* Not successful */
      pstPluginInfo = orxNULL;
    }
  }

  /* Done! */
  return pstPluginInfo;
}

/** Deletes a plugin info
 * @param[in] _pstPluginInfo          Concerned plugin info
 */
static void orxFASTCALL orxPlugin_DeletePluginInfo(orxPLUGIN_INFO *_pstPluginInfo)
{
  orxPLUGIN_FUNCTION_INFO *pstFunctionInfo;

  /* Checks */
  orxASSERT(_pstPluginInfo != orxNULL);

  /* Deletes all function info */
  for(pstFunctionInfo = orxBank_GetNext(_pstPluginInfo->pstFunctionBank, orxNULL);
      pstFunctionInfo != orxNULL;
      pstFunctionInfo = orxBank_GetNext(_pstPluginInfo->pstFunctionBank, orxNULL))
  {
    /* Is it a core function? */
    if(pstFunctionInfo->eFunctionID & orxPLUGIN_KU32_FLAG_CORE_ID)
    {
      /* Registers core function */
      orxPlugin_UnregisterCoreFunction(pstFunctionInfo);
    }

    /* Deletes it */
    orxPlugin_DeleteFunctionInfo(_pstPluginInfo, pstFunctionInfo);
  }

  /* Updates all modules */
  orxPlugin_UpdateAllModule();

  /* Deletes function hash table */
  orxHashTable_Delete(_pstPluginInfo->pstFunctionTable);

  /* Deletes function bank */
  orxBank_Delete(_pstPluginInfo->pstFunctionBank);

  /* Deletes plugin info */
  orxBank_Free(sstPlugin.pstPluginBank, _pstPluginInfo);

  /* Linked to system plugin? */
  if(_pstPluginInfo->pstSysPlugin != orxNULL)
  {
    /* Closes it */
    orxPLUGIN_CLOSE(_pstPluginInfo->pstSysPlugin);
    _pstPluginInfo->pstSysPlugin = orxNULL;
  }

  /* Done */
  return;
}

/** Gets a plugin info
 * @param[in] _hPluginHandle          Concerned plugin handle
 * @return orxPLUGIN_INFO / orxNULL
 */
static orxINLINE orxPLUGIN_INFO *orxPlugin_GetPluginInfo(orxHANDLE _hPluginHandle)
{
  orxPLUGIN_INFO *pstPluginInfo = orxNULL;

  /* Checks */
  orxASSERT(_hPluginHandle != orxHANDLE_UNDEFINED);

  /* Gets plugin info */
  pstPluginInfo = (orxPLUGIN_INFO *)_hPluginHandle;

  /* Checks */
  orxASSERT(pstPluginInfo->hPluginHandle == _hPluginHandle);

  /* Done! */
  return pstPluginInfo;
}

/** Gets a function address
 * @param[in] _pstSysPlugin           Concerned plugin
 * @param[in] _zFunctionName          Name of the function to get
 * @return orxPLUGIN_FUNCTION / orxNULL
 */
static orxPLUGIN_FUNCTION orxFASTCALL orxPlugin_GetFunctionAddress(orxSYSPLUGIN _pstSysPlugin, const orxSTRING _zFunctionName)
{
  orxPLUGIN_FUNCTION pfnFunction = orxNULL;

  /* Checks */
  orxASSERT(_pstSysPlugin != orxHANDLE_UNDEFINED);
  orxASSERT(_zFunctionName != orxNULL);

  /* Gets function */
  pfnFunction = (orxPLUGIN_FUNCTION)orxPLUGIN_GET_SYMBOL_ADDRESS(_pstSysPlugin, _zFunctionName);

  /* Not found? */
  if(pfnFunction == orxNULL)
  {
    /* Logs an error */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "The requested function <%s> couldn't be find.", _zFunctionName);
  }

  /* Done! */
  return pfnFunction;
}

/** Registers a plugin
 * @param[in] _pstSysPlugin           Concerned plugin
 * @param[in] _pstPluginInfo          Info of the plugin to register
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxSTATUS orxPlugin_RegisterPlugin(orxSYSPLUGIN _pstSysPlugin, orxPLUGIN_INFO *_pstPluginInfo)
{
  orxPLUGIN_MAIN_FUNCTION pfnInit;
  orxU32 u32UserFunctionNumber;
  orxPLUGIN_USER_FUNCTION_INFO *astUserFunctionInfo;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(_pstSysPlugin != orxNULL);
  orxASSERT(_pstPluginInfo != orxNULL);

  /* Gets init function */
  pfnInit = (orxPLUGIN_MAIN_FUNCTION)orxPlugin_GetFunctionAddress(_pstSysPlugin, orxPLUGIN_KZ_INIT_FUNCTION_NAME);

  /* Valid? */
  if(pfnInit != orxNULL)
  {
    orxU32 i;

    /* Calls it */
    eResult = pfnInit(&u32UserFunctionNumber, &astUserFunctionInfo);

    /* Adds all functions to plugin info */
    for(i = 0; (eResult == orxSTATUS_SUCCESS) && (i < u32UserFunctionNumber); i++)
    {
      /* Is function valid? */
      if(astUserFunctionInfo[i].pfnFunction != orxNULL)
      {
        orxPLUGIN_FUNCTION_INFO *pstFunctionInfo;

        /* Creates function info */
        pstFunctionInfo = orxPlugin_CreateFunctionInfo(_pstPluginInfo);

        /* Copies infos */
        pstFunctionInfo->pfnFunction    = astUserFunctionInfo[i].pfnFunction;
        pstFunctionInfo->eFunctionID    = astUserFunctionInfo[i].eFunctionID;
        pstFunctionInfo->zFunctionArgs  = astUserFunctionInfo[i].zFunctionArgs;
        pstFunctionInfo->zFunctionName  = astUserFunctionInfo[i].zFunctionName;

        /* Adds function info in plugin info structure */
        orxHashTable_Add(_pstPluginInfo->pstFunctionTable, pstFunctionInfo->eFunctionID, pstFunctionInfo);

        /* Is it a core function? */
        if(pstFunctionInfo->eFunctionID & orxPLUGIN_KU32_FLAG_CORE_ID)
        {
          /* Registers core function */
          eResult = orxPlugin_RegisterCoreFunction(pstFunctionInfo);
        }
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "Invalid function.");

        /* Updates result */
        eResult = orxSTATUS_FAILURE;
      }
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "Failed to get function address.");

    /* Can't load plugin */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Adds core info for a plugin
 * @param[in] _ePluginCoreID          Concerned plugin ID
 * @param[in] _eModuleID              ID of the corresponding module
 * @param[in] _astCoreFunction        Array containing the core functions
 * @param[in] _u32CoreFunctionNumber  Number of core function in the array
 */
void orxFASTCALL orxPlugin_AddCoreInfo(orxPLUGIN_CORE_ID _ePluginCoreID, orxMODULE_ID _eModuleID, const orxPLUGIN_CORE_FUNCTION *_astCoreFunction, orxU32 _u32CoreFunctionNumber)
{
  /* Checks */
  orxASSERT(sstPlugin.u32Flags & orxPLUGIN_KU32_STATIC_FLAG_READY);
  orxASSERT(sstPlugin.astCoreInfo[_ePluginCoreID].pstCoreFunctionTable == orxNULL);
  orxASSERT(_ePluginCoreID < orxPLUGIN_CORE_ID_NUMBER);
  orxASSERT(_eModuleID < orxMODULE_ID_NUMBER);
  orxASSERT(_astCoreFunction != orxNULL);

  /* Stores info */
  sstPlugin.astCoreInfo[_ePluginCoreID].pstCoreFunctionTable    = _astCoreFunction;
  sstPlugin.astCoreInfo[_ePluginCoreID].u32CoreFunctionCounter  = _u32CoreFunctionNumber;
  sstPlugin.astCoreInfo[_ePluginCoreID].eModuleID               = _eModuleID;
  sstPlugin.astCoreInfo[_ePluginCoreID].u32Flags                = orxPLUGIN_KU32_CORE_KU32_FLAG_FLAG_NONE;

  return;
}

/** Deletes all the plugins
 */
static orxINLINE void orxPlugin_DeleteAll()
{
  orxPLUGIN_INFO *pstPluginInfo;

  /* Gets first plugin info */
  pstPluginInfo = orxBank_GetNext(sstPlugin.pstPluginBank, orxNULL);

  /* Not empty */
  while(pstPluginInfo != orxNULL)
  {
    /* Deletes it */
    orxPlugin_DeletePluginInfo(pstPluginInfo);

    /* Gets first plugin info */
    pstPluginInfo = orxBank_GetNext(sstPlugin.pstPluginBank, orxNULL);
  }

  return;
}

/** Processes command line parameters
 * @param[in] _u32ParamCount  Number of extra parameters read for this option
 * @param[in] _azParams       Array of extra parameters (the first one is always the option name)
 * @return Returns orxSTATUS_SUCCESS if informations read are correct, orxSTATUS_FAILURE if a problem has occured
 */
static orxSTATUS orxFASTCALL orxPlugin_ProcessParams(orxU32 _u32ParamCount, const orxSTRING _azParams[])
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;
  orxU32    i;

  /* For all specified plugin names */
  for(i = 1; (eResult == orxSTATUS_SUCCESS) && (i < _u32ParamCount); i++)
  {
    orxSTRING zPluginName;
    orxS32    s32LastSeparatorIndex, s32SeparatorIndex;

    /* Gets last separator index */
    for(s32LastSeparatorIndex = 0, s32SeparatorIndex = orxString_SearchCharIndex(_azParams[i], orxPLUGIN_KC_DIRECTORY_SEPARATOR, 1);
        s32SeparatorIndex >= s32LastSeparatorIndex;
        s32LastSeparatorIndex = s32SeparatorIndex + 1, s32SeparatorIndex = orxString_SearchCharIndex(_azParams[i], orxPLUGIN_KC_DIRECTORY_SEPARATOR, s32LastSeparatorIndex));

    /* Gets plugin base name */
    zPluginName = _azParams[i] + s32LastSeparatorIndex;

    /* Loads plugin */
    orxPlugin_LoadUsingExt(_azParams[i], zPluginName);
  }

  /* Done! */
  return eResult;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Plugin module setup
 */
void orxFASTCALL orxPlugin_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_PLUGIN, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_PLUGIN, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_PLUGIN, orxMODULE_ID_PARAM);
  orxModule_AddDependency(orxMODULE_ID_PLUGIN, orxMODULE_ID_CONFIG);

  return;
}

/** Inits the plugin module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxPlugin_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstPlugin.u32Flags & orxPLUGIN_KU32_STATIC_FLAG_READY))
  {
    /* Cleans control structure */
    orxMemory_Zero(&sstPlugin, sizeof(orxPLUGIN_STATIC));

    /* Creates an empty spst_plugin_list */
    sstPlugin.pstPluginBank = orxBank_Create(orxPLUGIN_CORE_ID_NUMBER, sizeof(orxPLUGIN_INFO), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Is bank valid? */
    if(sstPlugin.pstPluginBank != orxNULL)
    {
      orxPARAM stParams;

      /* Updates status flags */
      sstPlugin.u32Flags = orxPLUGIN_KU32_STATIC_FLAG_READY;

      /* Registers all core plugins */
      orxPlugin_RegisterCorePlugins();

      /* Inits the param structure */
      orxMemory_Zero(&stParams, sizeof(orxPARAM));
      stParams.pfnParser  = orxPlugin_ProcessParams;
      stParams.u32Flags   = orxPARAM_KU32_FLAG_MULTIPLE_ALLOWED;
      stParams.zShortName = "p";
      stParams.zLongName  = "plugin";
      stParams.zShortDesc = "Loads the specified plugins.";
      stParams.zLongDesc  = "Loads the specified plugins from the current execution folder. More than one plugin can be specified. They can be core or user plugins.";

      /* Registers it */
      orxParam_Register(&stParams);

      /* Successful */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "Failed to create bank.");

      /* Bank not created */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "Tried to initialize plugin module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Exits from the plugin module
 */
void orxFASTCALL orxPlugin_Exit()
{
  /* Initialized? */
  if(sstPlugin.u32Flags & orxPLUGIN_KU32_STATIC_FLAG_READY)
  {
    /* Deletes plugin list */
    orxPlugin_DeleteAll();

    /* Destroys plugin bank */
    orxBank_Delete(sstPlugin.pstPluginBank);
    sstPlugin.pstPluginBank = orxNULL;

    /* Updates flags */
    sstPlugin.u32Flags &= ~orxPLUGIN_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "Tried to exit plugin module when it wasn't initialized.");
  }

  return;
}

/** Function called by default by all core functions that should be replaced by a plugin one
 * @param[in] _zFunctionName  Name of the called function
 * @param[in] _zFileName      Name of the file containing the called function
 * @param[in] _u32Line        Line number of the called function
 * @return orxNULL
 */
void *orxFASTCALL orxPlugin_DefaultCoreFunction(const orxSTRING _zFunctionName, const orxSTRING _zFileName, orxU32 _u32Line)
{
  orxDEBUG_FLAG_BACKUP();
  orxDEBUG_FLAG_SET(orxDEBUG_KU32_STATIC_FLAG_CONSOLE
                   |orxDEBUG_KU32_STATIC_FLAG_FILE
                   |orxDEBUG_KU32_STATIC_FLAG_TIMESTAMP
                   |orxDEBUG_KU32_STATIC_FLAG_TYPE,
                    orxDEBUG_KU32_STATIC_MASK_USER_ALL);
  orxDEBUG_PRINT(orxDEBUG_LEVEL_ALL, "The function <%s() - %s:%ld> has been called before being loaded!\nPlease verify that the corresponding plugin has been correctly loaded and that it contains this function.", _zFunctionName, _zFileName, _u32Line);
  orxDEBUG_FLAG_RESTORE();

  return orxNULL;
}

/** Loads a plugin (using its exact complete name)
 * @param[in] _zPluginFileName  The complete path of the plugin file, including its extension
 * @param[in] _zPluginName      The name that the plugin will be given in the plugin list
 * @return The plugin handle on success, orxHANDLE_UNDEFINED on failure
 */
orxHANDLE orxFASTCALL orxPlugin_Load(const orxSTRING _zPluginFileName, const orxSTRING _zPluginName)
{
  orxSYSPLUGIN pstSysPlugin;
  orxPLUGIN_INFO *pstPluginInfo;
  orxHANDLE hPluginHandle = orxHANDLE_UNDEFINED;

  /* Checks */
  orxASSERT(sstPlugin.u32Flags & orxPLUGIN_KU32_STATIC_FLAG_READY);
  orxASSERT(_zPluginFileName != orxNULL);
  orxASSERT(_zPluginName != orxNULL);

  /* Opens plugin */
  pstSysPlugin = orxPLUGIN_OPEN(_zPluginFileName);

  /* Valid? */
  if(pstSysPlugin != orxNULL)
  {
    /* Creates plugin info */
    pstPluginInfo = orxPlugin_CreatePluginInfo();

    /* Valid? */
    if(pstPluginInfo != orxNULL)
    {
      /* Stores plugin info */
      pstPluginInfo->pstSysPlugin = pstSysPlugin;
      pstPluginInfo->hPluginHandle = (orxHANDLE)pstPluginInfo;

      /* Registers plugin */
      if(orxPlugin_RegisterPlugin(pstSysPlugin, pstPluginInfo) == orxSTATUS_SUCCESS)
      {
        /* Stores plugin name */
        pstPluginInfo->zPluginName = _zPluginName;

        /* Gets plugin handle */
        hPluginHandle = pstPluginInfo->hPluginHandle;

        /* Updates all modules */
        orxPlugin_UpdateAllModule();
      }
      else
      {
        /* Logs an error */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "Couldn't register the plugin <%s>, closing it.", _zPluginFileName);

        /* Closes plugin */
        orxPLUGIN_CLOSE(pstSysPlugin);

        /* Deletes allocated plugin info */
        orxPlugin_DeletePluginInfo(pstPluginInfo);

        /* Empty plugin */
        hPluginHandle = orxHANDLE_UNDEFINED;
      }
    }
    else
    {
      /* Logs an error */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "Couldn't create a plugin info for plugin <%s>.", _zPluginFileName);

      /* Closes plugin */
      orxPLUGIN_CLOSE(pstSysPlugin);
    }
  }
  else
  {
    /* Logs an error */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "Couldn't load the plugin <%s>.", _zPluginFileName);
  }

  /* Returns its handle */
  return hPluginHandle;
}


/** Loads a plugin using OS common library extension + release/debug suffixes
 * @param[in] _zPluginFileName  The complete path of the plugin file, without its library extension
 * @param[in] _zPluginName      The name that the plugin will be given in the plugin list
 * @return The plugin handle on success, orxHANDLE_UNDEFINED on failure
 */
orxHANDLE orxFASTCALL orxPlugin_LoadUsingExt(const orxSTRING _zPluginFileName, const orxSTRING _zPluginName)
{
  orxCHAR   zFileName[256];
  orxHANDLE hResult = orxHANDLE_UNDEFINED;

#ifdef __orxDEBUG__

  orxSTRING zPreviousSection, zDebugSuffix;

  /* Gets config current section */
  zPreviousSection = orxConfig_GetCurrentSection();

  /* Selects section */
  orxConfig_SelectSection(orxPLUGIN_KZ_CONFIG_SECTION);

#endif /* __ orxDEBUG__ */

  /* Checks */
  orxASSERT(sstPlugin.u32Flags & orxPLUGIN_KU32_STATIC_FLAG_READY);
  orxASSERT(_zPluginFileName != orxNULL);
  orxASSERT(orxString_GetLength(_zPluginFileName) + orxMAX(orxString_GetLength(orxConfig_GetString(orxPLUGIN_KZ_CONFIG_DEBUG_SUFFIX)), orxString_GetLength(orxPLUGIN_KZ_DEFAULT_DEBUG_SUFFIX)) < 252);
  orxASSERT(_zPluginName != orxNULL);

#ifdef __orxDEBUG__

  /* Gets debug suffix */
  zDebugSuffix = (orxConfig_HasValue(orxPLUGIN_KZ_CONFIG_DEBUG_SUFFIX) != orxFALSE) ? orxConfig_GetString(orxPLUGIN_KZ_CONFIG_DEBUG_SUFFIX) : orxPLUGIN_KZ_DEFAULT_DEBUG_SUFFIX;

  /* Gets complete name */
  orxString_NPrint(zFileName, 256, "%s%s.%s", _zPluginFileName, zDebugSuffix, szPluginLibraryExt);

  /* Loads it */
  hResult = orxPlugin_Load(zFileName, _zPluginName);

  /* Not valid? */
  if(hResult == orxHANDLE_UNDEFINED)
  {

#endif /* __orxDEBUG__ */
    
  /* Gets complete name */
  orxString_NPrint(zFileName, 256, "%s.%s", _zPluginFileName, szPluginLibraryExt);

  /* Loads it */
  hResult = orxPlugin_Load(zFileName, _zPluginName);

#ifdef __orxDEBUG__

  }

  /* Restores previous section */
  orxConfig_SelectSection(zPreviousSection);

#endif /* __orxDEBUG__ */

  /* Done! */
  return hResult;
}

/** Unloads a plugin
 * @param[in] _hPluginHandle The handle of the plugin to unload
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxPlugin_Unload(orxHANDLE _hPluginHandle)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;
  orxPLUGIN_INFO *pstPluginInfo;

  /* Checks */
  orxASSERT(sstPlugin.u32Flags & orxPLUGIN_KU32_STATIC_FLAG_READY);
  orxASSERT(_hPluginHandle != orxHANDLE_UNDEFINED);

  /* Gets plugin info */
  pstPluginInfo = orxPlugin_GetPluginInfo(_hPluginHandle);

  /* Valid? */
  if(pstPluginInfo != orxNULL)
  {
    /* Deletes plugin */
    orxPlugin_DeletePluginInfo(pstPluginInfo);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "Failed to get plugin info for handle.");

    /* Not found */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets a function from a plugin
 * @param[in] _hPluginHandle The plugin handle
 * @param[in] _zFunctionName The name of the function to find
 * @return orxPLUGIN_FUNCTION / orxNULL
 */
orxPLUGIN_FUNCTION orxFASTCALL orxPlugin_GetFunction(orxHANDLE _hPluginHandle, const orxSTRING _zFunctionName)
{
  orxPLUGIN_INFO *pstPluginInfo;
  orxPLUGIN_FUNCTION pfnFunction = orxNULL;

  /* Checks */
  orxASSERT(sstPlugin.u32Flags & orxPLUGIN_KU32_STATIC_FLAG_READY);
  orxASSERT(_hPluginHandle != orxHANDLE_UNDEFINED);
  orxASSERT(_zFunctionName != orxNULL);

  /* Gets the plugin info */
  pstPluginInfo = orxPlugin_GetPluginInfo(_hPluginHandle);

  /* Valid? */
  if(pstPluginInfo != orxNULL)
  {
    /* Tries to get the function handle */
    pfnFunction = orxPlugin_GetFunctionAddress(pstPluginInfo->hPluginHandle, _zFunctionName);

    /* Not found? */
    if(pfnFunction == orxNULL)
    {
      /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "Failed to get function address for plugin.");
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "Failed to get plugin info for handle.");
  }

  /* Done! */
  return pfnFunction;
}

/** Gets the handle of a plugin given its name
 * @param[in] _zPluginName The plugin name
 * @return Its orxHANDLE / orxHANDLE_UNDEFINED
 */
orxHANDLE orxFASTCALL orxPlugin_GetHandle(const orxSTRING _zPluginName)
{
  orxPLUGIN_INFO *pstPluginInfo;
  orxHANDLE hPluginHandle = orxHANDLE_UNDEFINED;

  /* Checks */
  orxASSERT(sstPlugin.u32Flags & orxPLUGIN_KU32_STATIC_FLAG_READY);
  orxASSERT(_zPluginName != orxNULL);

  /* Search all plugin info */
  for(pstPluginInfo = orxBank_GetNext(sstPlugin.pstPluginBank, orxNULL);
      pstPluginInfo != orxNULL;
      pstPluginInfo = orxBank_GetNext(sstPlugin.pstPluginBank, pstPluginInfo))
  {
    /* Found? */
    if(orxString_Compare(_zPluginName, pstPluginInfo->zPluginName) == 0)
    {
      /* Gets its handle */
      hPluginHandle = pstPluginInfo->hPluginHandle;
      break;
    }
  }

  /* Done! */
  return hPluginHandle;
}

/** Gets the name of a plugin given its handle
 * @param[in] _hPluginHandle The plugin handle
 * @return The plugin name / orxSTRING_EMPTY
 */
const orxSTRING orxFASTCALL orxPlugin_GetName(orxHANDLE _hPluginHandle)
{
  orxPLUGIN_INFO *pstPluginInfo;
  orxSTRING zPluginName = orxSTRING_EMPTY;

  /* Checks */
  orxASSERT(sstPlugin.u32Flags & orxPLUGIN_KU32_STATIC_FLAG_READY);
  orxASSERT(_hPluginHandle != orxHANDLE_UNDEFINED);

  /* Gets plugin info */
  pstPluginInfo = orxPlugin_GetPluginInfo(_hPluginHandle);

  /* Valid? */
  if(pstPluginInfo != orxNULL)
  {
    /* Gets plugin name */
    zPluginName = pstPluginInfo->zPluginName;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "Failed to get plugin info for handle.");
  }

  /* Done! */
  return(zPluginName);
}
