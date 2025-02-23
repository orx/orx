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
 * @file orxPlugin.c
 * @date 04/09/2002
 * @author iarwain@orx-project.org
 *
 */


#include "plugin/orxPlugin.h"

#include "debug/orxDebug.h"
#include "core/orxConfig.h"
#include "core/orxEvent.h"
#include "core/orxResource.h"
#include "main/orxParam.h"
#include "memory/orxBank.h"
#include "memory/orxMemory.h"
#include "plugin/orxPluginUser.h"
#include "plugin/orxPluginCore.h"
#include "utils/orxHashTable.h"


/** Platform dependent type & function defines
 */

/* Windows */
#ifdef __orxWINDOWS__

  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  #undef WIN32_LEAN_AND_MEAN

  typedef HINSTANCE                                         orxSYSPLUGIN;

  #define orxPLUGIN_OPEN(PLUGIN)                            LoadLibrary(PLUGIN)
  #define orxPLUGIN_GET_SYMBOL_ADDRESS(PLUGIN, SYMBOL)      GetProcAddress(PLUGIN, SYMBOL)
  #define orxPLUGIN_CLOSE(PLUGIN)                           FreeLibrary(PLUGIN)

  static const orxSTRING                                    szPluginLibraryExt = "dll";

  #define __orxPLUGIN_DYNAMIC__

  #define __orxPLUGIN_MULTI_SHADOW__

#else /* __orxWINDOWS__ */

  typedef void *                                            orxSYSPLUGIN;

  /* iOS */
  #if defined(__orxIOS__)

    #undef __orxPLUGIN_DYNAMIC__
    #undef orxPLUGIN_OPEN
    #undef orxPLUGIN_GET_SYMBOL_ADDRESS
    #undef orxPLUGIN_CLOSE

  #else /* __orxIOS__ */

    #include <dlfcn.h>

    #define orxPLUGIN_OPEN(PLUGIN)                          dlopen(PLUGIN, RTLD_LAZY)
    #define orxPLUGIN_GET_SYMBOL_ADDRESS(PLUGIN, SYMBOL)    dlsym(PLUGIN, SYMBOL)
    #define orxPLUGIN_CLOSE(PLUGIN)                         dlclose(PLUGIN)

      static const orxSTRING                                szPluginLibraryExt = "so";

    #define __orxPLUGIN_DYNAMIC__

  #endif /* __orxIOS__ */

#endif /* __orxWINDOWS__ */


/** Module flags
 */

#define orxPLUGIN_KU32_STATIC_FLAG_NONE                     0x00000000
#define orxPLUGIN_KU32_STATIC_FLAG_SWAP                     0x10000000
#define orxPLUGIN_KU32_STATIC_FLAG_READY                    0x00000001


#define orxPLUGIN_KU32_CORE_KU32_FLAG_FLAG_NONE             0x00000000
#define orxPLUGIN_KU32_CORE_KU32_FLAG_FLAG_LOADED           0x00000001
#define orxPLUGIN_KU32_CORE_KU32_FLAG_FLAG_DIRTY            0x10000000


/** Misc
 */
#define orxPLUGIN_KU32_FUNCTION_BANK_SIZE                   16
#define orxPLUGIN_KZ_INIT_FUNCTION_NAME                     orxSTRINGIFY(orxPLUGIN_K_INIT_FUNCTION_NAME) /**< Plugin init function name */
#define orxPLUGIN_KZ_EXIT_FUNCTION_NAME                     orxSTRINGIFY(orxPLUGIN_K_EXIT_FUNCTION_NAME) /**< Plugin exit function name */
#define orxPLUGIN_KZ_SWAP_FUNCTION_NAME                     orxSTRINGIFY(orxPLUGIN_K_SWAP_FUNCTION_NAME) /**< Plugin swap function name */

#define orxPLUGIN_KC_DIRECTORY_SEPARATOR                    '/'


#define orxPLUGIN_KZ_CONFIG_SECTION                         "Plugin"
#define orxPLUGIN_KZ_CONFIG_SWAP_SECTION                    "orx:plugin:runtime"

#define orxPLUGIN_KU32_SHADOW_BUFFER_SIZE                   131072

#ifdef __orxPLUGIN_MULTI_SHADOW__
#define orxPLUGIN_KZ_SHADOW_FORMAT                          "_Shadow%03u"
#else /* __orxPLUGIN_MULTI_SHADOW__ */
#define orxPLUGIN_KZ_SHADOW_FORMAT                          "_Shadow"
#endif /* __orxPLUGIN_MULTI_SHADOW__ */


#if defined(__orxDEBUG__)

  #define orxPLUGIN_KZ_CONFIG_SUFFIX                  "DebugSuffix"

  #define orxPLUGIN_KZ_DEFAULT_SUFFIX                 "d"

#elif defined(__orxPROFILER__)

  #define orxPLUGIN_KZ_CONFIG_SUFFIX                  "ProfileSuffix"

  #define orxPLUGIN_KZ_DEFAULT_SUFFIX                 "p"

#endif


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Information structure on a plugin function
 */
typedef struct __orxPLUGIN_FUNCTION_INFO_t
{
  orxPLUGIN_FUNCTION_ID eFunctionID;                        /**< Function ID : 8 */
  orxPLUGIN_FUNCTION    pfnFunction;                        /**< Function Address : 4 */
  const orxSTRING       zFunctionName;                      /**< Function Name : 12 */
  const orxSTRING       zFunctionArgs;                      /**< Function Argument Types : 16 */

} orxPLUGIN_FUNCTION_INFO;

/** Information structure on a plugin
 */
typedef struct __orxPLUGIN_INFO_t
{
  orxSYSPLUGIN    pstSysPlugin;                             /**< Plugin system pointer : 4 */
  orxHANDLE       hPluginHandle;                            /**< Plugin handle : 8 */
  orxBANK        *pstFunctionBank;                          /**< Function bank : 12 */
  orxHASHTABLE   *pstFunctionTable;                         /**< Function hash table : 16 */
  orxSTRINGID     stNameID;                                 /**< Name ID : 24 */
  orxSTRINGID     stResourceID;                             /**< Resource ID : 32 */
  orxSTRINGID     stShadowID;                               /**< Shadow ID : 40 */

} orxPLUGIN_INFO;

/** Core info structure
 */
typedef struct __orxPLUGIN_CORE_INFO_t
{
  /* Core functions : 4 */
  orxPLUGIN_CORE_FUNCTION const    *pstCoreFunctionTable;

  /* Core functions count : 8 */
  orxU32                            u32CoreFunctionCount;

  /* Core module ID : 12 */
  orxMODULE_ID                      eModuleID;

  /* Core info status : 16 */
  orxU32                            u32Flags;

} orxPLUGIN_CORE_INFO;

/** Static structure
 */
typedef struct __orxPLUGIN_STATIC_t
{
  /* Plugin bank */
  orxBANK *pstPluginBank;

  /* Core info table */
  orxPLUGIN_CORE_INFO astCoreInfo[orxPLUGIN_CORE_ID_NUMBER];

  /* Resource group ID */
  orxSTRINGID stResourceGroupID;

#ifdef __orxPLUGIN_MULTI_SHADOW__
  /* Shadow count */
  orxU32 u32ShadowCount;
#endif /* __orxPLUGIN_MULTI_SHADOW__ */

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
          (j < sstPlugin.astCoreInfo[i].u32CoreFunctionCount) && (bLoaded != orxFALSE);
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

#ifndef __orxEMBEDDED__
          /* Tries to init associated module */
          orxModule_Init(sstPlugin.astCoreInfo[i].eModuleID);
#endif /* !__orxEMBEDDED__ */
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

#ifndef __orxEMBEDDED__
          /* Tries to exit from associated module */
          orxModule_Exit(sstPlugin.astCoreInfo[i].eModuleID);
#endif /* !__orxEMBEDDED__ */
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
 * @param[in] _eFunctionID            Concerned function ID
 * @param[in] _pfnFunction            Concerned function implementation
 * @param[in] _bEmbedded              Embedded mode?
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS orxPlugin_RegisterCoreFunction(orxPLUGIN_FUNCTION_ID _eFunctionID, orxPLUGIN_FUNCTION _pfnFunction, orxBOOL _bEmbedded)
{
  const orxPLUGIN_CORE_FUNCTION  *pstCoreFunction;
  orxU32                          u32PluginIndex;
  orxSTATUS                       eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(_pfnFunction != orxNULL);

  /* Gets plugin index */
  u32PluginIndex = (_eFunctionID & orxPLUGIN_KU32_MASK_PLUGIN_ID) >> orxPLUGIN_KU32_SHIFT_PLUGIN_ID;

  /* Checks */
  orxASSERT(u32PluginIndex < orxPLUGIN_CORE_ID_NUMBER);

  /* Gets core function table */
  pstCoreFunction = sstPlugin.astCoreInfo[u32PluginIndex].pstCoreFunctionTable;

  /* Core plugin defined? */
  if(pstCoreFunction != orxNULL)
  {
    orxU32 u32FunctionIndex;

    /* Gets function index */
    u32FunctionIndex = _eFunctionID & orxPLUGIN_KU32_MASK_FUNCTION_ID;

    /* Checks */
    orxASSERT(u32FunctionIndex < sstPlugin.astCoreInfo[u32PluginIndex].u32CoreFunctionCount);
    orxASSERT(pstCoreFunction[u32FunctionIndex].pfnFunction != orxNULL);

    /* Embedded mode or was not already loaded? */
    if((_bEmbedded != orxFALSE)
    || (*(pstCoreFunction[u32FunctionIndex].pfnFunction) == pstCoreFunction[u32FunctionIndex].pfnDefaultFunction))
    {
      /* Registers core function */
      *(pstCoreFunction[u32FunctionIndex].pfnFunction) = _pfnFunction;

      /* Updates plugin status */
      sstPlugin.astCoreInfo[u32PluginIndex].u32Flags |= orxPLUGIN_KU32_CORE_KU32_FLAG_FLAG_DIRTY;

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "Function address is already loaded.");
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
  orxU32                          u32PluginIndex;

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
    orxU32 u32FunctionIndex;

    /* Gets function index */
    u32FunctionIndex = _pfnFunctionInfo->eFunctionID & orxPLUGIN_KU32_MASK_FUNCTION_ID;

    /* Checks */
    orxASSERT(u32FunctionIndex < sstPlugin.astCoreInfo[u32PluginIndex].u32CoreFunctionCount);
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
  orxPLUGIN_FUNCTION_INFO  *pstFunctionInfo;
  const orxSTRING           zShadowLocation;

  /* Checks */
  orxASSERT(_pstPluginInfo != orxNULL);

  /* Gets shadow location */
  zShadowLocation = (_pstPluginInfo->stShadowID != orxSTRINGID_UNDEFINED) ? orxString_GetFromID(_pstPluginInfo->stShadowID) : orxNULL;

  /* Deletes all function info */
  for(pstFunctionInfo = (orxPLUGIN_FUNCTION_INFO *)orxBank_GetNext(_pstPluginInfo->pstFunctionBank, orxNULL);
      pstFunctionInfo != orxNULL;
      pstFunctionInfo = (orxPLUGIN_FUNCTION_INFO *)orxBank_GetNext(_pstPluginInfo->pstFunctionBank, orxNULL))
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

#ifdef __orxPLUGIN_DYNAMIC__

  /* Linked to system plugin? */
  if(_pstPluginInfo->pstSysPlugin != orxNULL)
  {
    /* Closes it */
    orxPLUGIN_CLOSE(_pstPluginInfo->pstSysPlugin);
    _pstPluginInfo->pstSysPlugin = orxNULL;

    /* Has shadow? */
    if(zShadowLocation != orxNULL)
    {
      /* Deletes it */
      orxResource_Delete(zShadowLocation);
    }
  }

#endif /* __orxPLUGIN_DYNAMIC__ */

  /* Done */
  return;
}

/** Gets a plugin info
 * @param[in] _hPluginHandle          Concerned plugin handle
 * @return orxPLUGIN_INFO / orxNULL
 */
static orxINLINE orxPLUGIN_INFO *orxPlugin_GetPluginInfo(orxHANDLE _hPluginHandle)
{
  orxPLUGIN_INFO *pstPluginInfo;

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

#ifdef __orxPLUGIN_DYNAMIC__

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

#endif /* __orxPLUGIN_DYNAMIC__ */

  /* Done! */
  return pfnFunction;
}

/** Registers a plugin
 * @param[in] _pstSysPlugin           Concerned plugin
 * @param[in] _pstPluginInfo          Info of the plugin to register
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxSTATUS orxPlugin_RegisterPlugin(orxPLUGIN_INFO *_pstPluginInfo)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(_pstPluginInfo != orxNULL);

  /* Swapping? */
  if(orxFLAG_TEST(sstPlugin.u32Flags, orxPLUGIN_KU32_STATIC_FLAG_SWAP))
  {
    orxPLUGIN_SWAP_FUNCTION pfnSwap;

    /* Gets swap function */
    pfnSwap = (orxPLUGIN_SWAP_FUNCTION)orxPlugin_GetFunctionAddress(_pstPluginInfo->pstSysPlugin, orxPLUGIN_KZ_SWAP_FUNCTION_NAME);

    /* Found? */
    if(pfnSwap != orxNULL)
    {
      /* Calls it */
      eResult = pfnSwap(orxPLUGIN_ENTRY_MODE_SWAP_IN);
    }
  }
  else
  {
    orxPLUGIN_INIT_FUNCTION pfnInit;
    orxU32 u32UserFunctionNumber;
    orxPLUGIN_USER_FUNCTION_INFO *astUserFunctionInfo;

    /* Gets init function */
    pfnInit = (orxPLUGIN_INIT_FUNCTION)orxPlugin_GetFunctionAddress(_pstPluginInfo->pstSysPlugin, orxPLUGIN_KZ_INIT_FUNCTION_NAME);

    /* Valid? */
    if(pfnInit != orxNULL)
    {
      orxU32 i;

      /* Calls it */
      eResult = pfnInit(&u32UserFunctionNumber, &astUserFunctionInfo);

      /* Adds all functions to plugin info */
      for(i = 0, eResult = orxSTATUS_SUCCESS; (eResult != orxSTATUS_FAILURE) && (i < u32UserFunctionNumber); i++)
      {
        /* Is function valid? */
        if(astUserFunctionInfo[i].pfnFunction != orxNULL)
        {
          orxPLUGIN_FUNCTION_INFO *pstFunctionInfo;

          /* Creates function info */
          pstFunctionInfo = orxPlugin_CreateFunctionInfo(_pstPluginInfo);

          /* Copies info */
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
            eResult = orxPlugin_RegisterCoreFunction(pstFunctionInfo->eFunctionID, pstFunctionInfo->pfnFunction, orxFALSE);
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
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "Failed to get plugin init entry point.");
    }
  }

  /* Done! */
  return eResult;
}

/** Adds an info structure for the given core module
 * Has to be called during a core module init
 * @param[in] _ePluginCoreID          The numeric id of the core plugin
 * @param[in] _eModuleID              Corresponding module ID
 * @param[in] _astCoreFunction        The pointer to the core functions info array
 * @param[in] _u32CoreFunctionNumber  Number of functions in the array
 */
void orxFASTCALL orxPlugin_AddCoreInfo(orxPLUGIN_CORE_ID _ePluginCoreID, orxMODULE_ID _eModuleID, const orxPLUGIN_CORE_FUNCTION *_astCoreFunction, orxU32 _u32CoreFunctionNumber)
{
  /* Checks */
  orxASSERT(sstPlugin.u32Flags & orxPLUGIN_KU32_STATIC_FLAG_READY);
  orxASSERT(sstPlugin.astCoreInfo[_ePluginCoreID].pstCoreFunctionTable == orxNULL);
  orxASSERT(_ePluginCoreID < orxPLUGIN_CORE_ID_NUMBER);
  orxASSERT(_eModuleID < orxMODULE_ID_CORE_NUMBER);
  orxASSERT(_astCoreFunction != orxNULL);

  /* Stores info */
  sstPlugin.astCoreInfo[_ePluginCoreID].pstCoreFunctionTable    = _astCoreFunction;
  sstPlugin.astCoreInfo[_ePluginCoreID].u32CoreFunctionCount    = _u32CoreFunctionNumber;
  sstPlugin.astCoreInfo[_ePluginCoreID].eModuleID               = _eModuleID;
  sstPlugin.astCoreInfo[_ePluginCoreID].u32Flags                = orxPLUGIN_KU32_CORE_KU32_FLAG_FLAG_NONE;

  return;
}

#ifdef __orxEMBEDDED__

/** Binds a core plugin to its embedded implementation
 * Has to be called during a core module init
 * @param[in] _ePluginCoreID          The numeric id of the core plugin
 * @param[in] _pfnPluginInit          Embedded plug-in init function
 */
void orxFASTCALL orxPlugin_BindCoreInfo(orxPLUGIN_CORE_ID _ePluginCoreID, orxPLUGIN_INIT_FUNCTION _pfnPluginInit)
{
  orxU32                        u32UserFunctionNumber, i;
  orxPLUGIN_USER_FUNCTION_INFO *astUserFunctionInfo;
  orxSTATUS                     eResult;

  /* Checks */
  orxASSERT(sstPlugin.u32Flags & orxPLUGIN_KU32_STATIC_FLAG_READY);
  orxASSERT(_ePluginCoreID < orxPLUGIN_CORE_ID_NUMBER);
  orxASSERT(_pfnPluginInit != orxNULL);

  /* Calls init function it */
  eResult = _pfnPluginInit(&u32UserFunctionNumber, &astUserFunctionInfo);

  /* Adds all functions to plugin info */
  for(i = 0; (eResult != orxSTATUS_FAILURE) && (i < u32UserFunctionNumber); i++)
  {
    /* Checks */
    orxASSERT(astUserFunctionInfo[i].pfnFunction != orxNULL);
    orxASSERT(astUserFunctionInfo[i].eFunctionID & orxPLUGIN_KU32_FLAG_CORE_ID);

    /* Registers core function */
    eResult = orxPlugin_RegisterCoreFunction(astUserFunctionInfo[i].eFunctionID, astUserFunctionInfo[i].pfnFunction, orxTRUE);
  }

  /* Done! */
  return;
}

#endif /* __orxEMBEDDED__ */

/** Deletes all the plugins
 */
static orxINLINE void orxPlugin_DeleteAll()
{
  orxPLUGIN_INFO *pstPluginInfo;

  /* Gets first plugin info */
  pstPluginInfo = (orxPLUGIN_INFO *)orxBank_GetNext(sstPlugin.pstPluginBank, orxNULL);

  /* Not empty */
  while(pstPluginInfo != orxNULL)
  {
    /* Deletes it */
    orxPlugin_DeletePluginInfo(pstPluginInfo);

    /* Gets first plugin info */
    pstPluginInfo = (orxPLUGIN_INFO *)orxBank_GetNext(sstPlugin.pstPluginBank, orxNULL);
  }

  return;
}

/** Processes command line parameters
 * @param[in] _u32ParamCount  Number of extra parameters read for this option
 * @param[in] _azParams       Array of extra parameters (the first one is always the option name)
 * @return Returns orxSTATUS_SUCCESS if read information is correct, orxSTATUS_FAILURE if a problem occurred
 */
static orxSTATUS orxFASTCALL orxPlugin_ProcessParams(orxU32 _u32ParamCount, const orxSTRING _azParams[])
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;
  orxU32    i;

  /* For all specified plugin names */
  for(i = 1; (eResult != orxSTATUS_FAILURE) && (i < _u32ParamCount); i++)
  {
    /* Loads plugin */
    eResult = (orxPlugin_Load(_azParams[i]) != orxHANDLE_UNDEFINED) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
  }

  /* Updates all modules */
  orxPlugin_UpdateAllModule();

  /* Done! */
  return eResult;
}

/** Event handler
 */
static orxSTATUS orxFASTCALL orxPlugin_EventHandler(const orxEVENT *_pstEvent)
{
  orxRESOURCE_EVENT_PAYLOAD  *pstPayload;
  orxSTATUS                   eResult = orxSTATUS_SUCCESS;

  /* Gets payload */
  pstPayload = (orxRESOURCE_EVENT_PAYLOAD *)_pstEvent->pstPayload;

  /* Is plugin group? */
  if(pstPayload->stGroupID == sstPlugin.stResourceGroupID)
  {
    orxPLUGIN_INFO *pstPluginInfo;

    /* For all plugin info */
    for(pstPluginInfo = (orxPLUGIN_INFO *)orxBank_GetNext(sstPlugin.pstPluginBank, orxNULL);
        pstPluginInfo != orxNULL;
        pstPluginInfo = (orxPLUGIN_INFO *)orxBank_GetNext(sstPlugin.pstPluginBank, pstPluginInfo))
    {
      /* Found? */
      if(pstPluginInfo->stResourceID == pstPayload->stNameID)
      {
        orxPLUGIN_SWAP_FUNCTION pfnSwap;

        /* Gets swap function */
        pfnSwap = (orxPLUGIN_SWAP_FUNCTION)orxPlugin_GetFunctionAddress(pstPluginInfo->pstSysPlugin, orxPLUGIN_KZ_SWAP_FUNCTION_NAME);

        /* Found? */
        if(pfnSwap != orxNULL)
        {
          /* Updates status */
          orxFLAG_SET(sstPlugin.u32Flags, orxPLUGIN_KU32_STATIC_FLAG_SWAP, orxPLUGIN_KU32_STATIC_FLAG_NONE);

          /* Pushes config swap section */
          orxConfig_PushSection(orxPLUGIN_KZ_CONFIG_SWAP_SECTION);

          /* Can unload it? */
          if(orxPlugin_Unload(pstPluginInfo->hPluginHandle) != orxSTATUS_FAILURE)
          {
            /* Logs message */
            orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "Hotloading plugin [%s]", orxString_GetFromID(pstPluginInfo->stNameID));

            /* Reloads it */
            orxPlugin_Load(orxString_GetFromID(pstPluginInfo->stNameID));
          }

          /* Pops config section */
          orxConfig_PopSection();

          /* Clears config swap section */
          orxConfig_ClearSection(orxPLUGIN_KZ_CONFIG_SWAP_SECTION);

          /* Updates status */
          orxFLAG_SET(sstPlugin.u32Flags, orxPLUGIN_KU32_STATIC_FLAG_NONE, orxPLUGIN_KU32_STATIC_FLAG_SWAP);
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "Can't hotload plugin [%]: no swap entry point found, aborting!", orxString_GetFromID(pstPluginInfo->stNameID));
        }

        break;
      }
    }
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
  orxModule_AddDependency(orxMODULE_ID_PLUGIN, orxMODULE_ID_STRING);
  orxModule_AddDependency(orxMODULE_ID_PLUGIN, orxMODULE_ID_PARAM);
  orxModule_AddDependency(orxMODULE_ID_PLUGIN, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_PLUGIN, orxMODULE_ID_EVENT);
  orxModule_AddDependency(orxMODULE_ID_PLUGIN, orxMODULE_ID_RESOURCE);

  /* Done! */
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

    /* Inits values */
    sstPlugin.stResourceGroupID = orxString_GetID(orxPLUGIN_KZ_RESOURCE_GROUP);

    /* Creates an empty spst_plugin_list */
    sstPlugin.pstPluginBank = orxBank_Create(orxPLUGIN_CORE_ID_NUMBER, sizeof(orxPLUGIN_INFO), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Is bank valid? */
    if(sstPlugin.pstPluginBank != orxNULL)
    {
#ifndef __orxEMBEDDED__

      orxPARAM stParams;

#endif /* !__orxEMBEDDED__ */

      /* Updates status flags */
      sstPlugin.u32Flags = orxPLUGIN_KU32_STATIC_FLAG_READY;

      /* Registers all core plugins */
      orxPlugin_RegisterCorePlugins();

#ifdef __orxEMBEDDED__

      /* Updates all modules */
      orxPlugin_UpdateAllModule();

#else /* __orxEMBEDDED__ */

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

#endif /* __orxEMBEDDED__ */

      /* Adds event handler */
      orxEvent_AddHandler(orxEVENT_TYPE_RESOURCE, orxPlugin_EventHandler);
      orxEvent_SetHandlerIDFlags(orxPlugin_EventHandler, orxEVENT_TYPE_RESOURCE, orxNULL, orxEVENT_GET_FLAG(orxRESOURCE_EVENT_ADD) | orxEVENT_GET_FLAG(orxRESOURCE_EVENT_UPDATE), orxEVENT_KU32_MASK_ID_ALL);

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
    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_RESOURCE, orxPlugin_EventHandler);

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
  orxU32 u32DebugFlags;

  u32DebugFlags = orxDEBUG_GET_FLAGS();
  orxDEBUG_SET_FLAGS(orxDEBUG_KU32_STATIC_FLAG_TERMINAL
                    |orxDEBUG_KU32_STATIC_FLAG_FILE
                    |orxDEBUG_KU32_STATIC_FLAG_TIMESTAMP
                    |orxDEBUG_KU32_STATIC_FLAG_TYPE,
                     orxDEBUG_KU32_STATIC_MASK_USER_ALL);
  orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "The function <%s() @ %s:%d> has been called before being loaded!%sPlease verify that the corresponding plugin has been correctly loaded and that it contains this function.", _zFunctionName, _zFileName, _u32Line, orxSTRING_EOL);
  orxDEBUG_SET_FLAGS(u32DebugFlags,
                     orxDEBUG_KU32_STATIC_MASK_USER_ALL);

  return orxNULL;
}

/** Loads a plugin, doing a shadow copy and watching for any change on-disk to trigger an auto-swap
 * @param[in] _zPluginName  The name of the plugin, with or without its library extension
 * @return The plugin handle on success, orxHANDLE_UNDEFINED on failure
 */
orxHANDLE orxFASTCALL orxPlugin_Load(const orxSTRING _zPluginName)
{
  orxHANDLE       hResult = orxHANDLE_UNDEFINED;

#ifdef __orxPLUGIN_DYNAMIC__

  orxHANDLE       hPlugin;
  const orxSTRING zLocation;
  const orxSTRING zExtension;
  const orxSTRING zFileName;
  orxCHAR         acFileName[384];

  /* Checks */
  orxASSERT(sstPlugin.u32Flags & orxPLUGIN_KU32_STATIC_FLAG_READY);
  orxASSERT(_zPluginName != orxNULL);

  /* Is already loaded? */
  if((hPlugin = orxPlugin_GetHandle(_zPluginName)) != orxHANDLE_UNDEFINED)
  {
    /* Unloads it */
    orxPlugin_Unload(hPlugin);
  }

  /* Gets extension */
  zExtension = _zPluginName + orxString_GetLength(_zPluginName) - orxString_GetLength(szPluginLibraryExt) - 1;

  /* Already contains extension? */
  if((*zExtension++ == '.')
  && (orxMemory_Compare(zExtension, szPluginLibraryExt, orxString_GetLength(szPluginLibraryExt)) == 0))
  {
    /* Locates it without any additional extension/suffix */
    zLocation = orxResource_Locate(orxPLUGIN_KZ_RESOURCE_GROUP, _zPluginName);
    zFileName = _zPluginName;
  }
  else
  {
#if defined(__orxDEBUG__) || defined(__orxPROFILER__)

    orxSTRING zSuffix;

    /* Pushes section */
    orxConfig_PushSection(orxPLUGIN_KZ_CONFIG_SECTION);

    /* Checks */
    orxASSERT(orxString_GetLength(_zPluginName) + orxMAX(orxString_GetLength(orxConfig_GetString(orxPLUGIN_KZ_CONFIG_SUFFIX)), orxString_GetLength(orxPLUGIN_KZ_DEFAULT_SUFFIX)) < 252);

#endif /* __orxDEBUG__ || __orxPROFILER__ */

    /* Inits buffer */
    acFileName[sizeof(acFileName) - 1] = orxCHAR_NULL;
    zFileName = acFileName;

#if defined(__orxDEBUG__) || defined(__orxPROFILER__)

    /* Gets debug suffix */
    zSuffix = (orxSTRING)((orxConfig_HasValue(orxPLUGIN_KZ_CONFIG_SUFFIX) != orxFALSE) ? orxConfig_GetString(orxPLUGIN_KZ_CONFIG_SUFFIX) : orxPLUGIN_KZ_DEFAULT_SUFFIX);

    /* Pops previous section */
    orxConfig_PopSection();

    /* Gets complete name */
    orxString_NPrint(acFileName, sizeof(acFileName) - 1, "%s%s.%s", _zPluginName, zSuffix, szPluginLibraryExt);

    /* Locates it */
    zLocation = orxResource_Locate(orxPLUGIN_KZ_RESOURCE_GROUP, zFileName);

    /* Not found? */
    if(zLocation == orxNULL)
    {
#endif /* __orxDEBUG__ || __orxPROFILER__ */

    /* Gets complete name */
    orxString_NPrint(acFileName, sizeof(acFileName) - 1, "%s.%s", _zPluginName, szPluginLibraryExt);

    /* Locates it */
    zLocation = orxResource_Locate(orxPLUGIN_KZ_RESOURCE_GROUP, zFileName);

#if defined(__orxDEBUG__) || defined(__orxPROFILER__)
    }

#endif /* __orxDEBUG__ || __orxPROFILER__ */
  }

  /* Found? */
  if(zLocation != orxNULL)
  {
    orxSYSPLUGIN                  pstSysPlugin;
    const orxRESOURCE_TYPE_INFO  *pstResourceType;
    orxBOOL                       bUseShadow = orxFALSE;
    orxCHAR                       acShadowLocation[384];

    /* Gets resource type */
    pstResourceType = orxResource_GetType(zLocation);
    orxASSERT(pstResourceType != orxNULL);

    /* Not file type? */
    if(orxString_Compare(pstResourceType->zTag, orxRESOURCE_KZ_TYPE_TAG_FILE) != 0)
    {
      /* Requests local shadow */
      bUseShadow = orxTRUE;
    }
    else
    {
      orxS32 i, iCount;

      /* Pushes resource section */
      orxConfig_PushSection(orxRESOURCE_KZ_CONFIG_SECTION);

      /* For all watchlist entries */
      for(i = 0, iCount = orxConfig_GetListCount(orxRESOURCE_KZ_CONFIG_WATCH_LIST); i < iCount; i++)
      {
        /* Found? */
        if(orxString_ICompare(orxPLUGIN_KZ_RESOURCE_GROUP, orxConfig_GetListString(orxRESOURCE_KZ_CONFIG_WATCH_LIST, i)) == 0)
        {
          /* Updates status */
          bUseShadow = orxTRUE;
          break;
        }
      }

      /* Pops config section */
      orxConfig_PopSection();
    }

    /* Should make a shadow copy? */
    if(bUseShadow != orxFALSE)
    {
      orxHANDLE       hResource, hShadowResource;
      const orxSTRING zPath;

      /* Gets resource path */
      zPath = orxResource_GetPath(zLocation);

      /* Inits buffer */
      acShadowLocation[sizeof(acShadowLocation) - 1] = orxCHAR_NULL;

      /* Gets shadow location */
  #ifdef __orxPLUGIN_MULTI_SHADOW__
      orxString_NPrint(acShadowLocation, sizeof(acShadowLocation) - 1, "%s%c%.*s" orxPLUGIN_KZ_SHADOW_FORMAT ".%s", orxRESOURCE_KZ_TYPE_TAG_FILE, orxRESOURCE_KC_LOCATION_SEPARATOR, orxString_GetLength(zFileName) - orxString_GetLength(szPluginLibraryExt) - 1, zFileName, sstPlugin.u32ShadowCount++, szPluginLibraryExt);
  #else /* __orxPLUGIN_MULTI_SHADOW__ */
      orxString_NPrint(acShadowLocation, sizeof(acShadowLocation) - 1, "%s%c%.*s" orxPLUGIN_KZ_SHADOW_FORMAT ".%s", orxRESOURCE_KZ_TYPE_TAG_FILE, orxRESOURCE_KC_LOCATION_SEPARATOR, orxString_GetLength(zFileName) - orxString_GetLength(szPluginLibraryExt) - 1, zFileName, szPluginLibraryExt);
  #endif /* __orxPLUGIN_MULTI_SHADOW__ */

      /* Opens both resources */
      hResource       = orxResource_Open(zLocation, orxFALSE);
      hShadowResource = orxResource_Open(acShadowLocation, orxTRUE);

      /* Valid? */
      if((hResource != orxHANDLE_UNDEFINED)
      && (hShadowResource != orxHANDLE_UNDEFINED))
      {
        orxCHAR acBuffer[orxPLUGIN_KU32_SHADOW_BUFFER_SIZE];
        orxS64  s64Size;

        /* Makes the shadow copy */
        for(s64Size = orxResource_Read(hResource, sizeof(acBuffer), acBuffer, orxNULL, orxNULL);
            s64Size != 0;
            s64Size = orxResource_Read(hResource, sizeof(acBuffer), acBuffer, orxNULL, orxNULL))
        {
          orxResource_Write(hShadowResource, s64Size, acBuffer, orxNULL, orxNULL);
        }

        /* Updates location */
        zLocation = acShadowLocation;
      }

      /* Closes handles */
      orxResource_Close(hResource);
      orxResource_Close(hShadowResource);
    }

    /* Opens plugin */
    pstSysPlugin = orxPLUGIN_OPEN(orxResource_GetPath(zLocation));

    /* Valid? */
    if(pstSysPlugin != orxNULL)
    {
      orxPLUGIN_INFO *pstPluginInfo;

      /* Creates plugin info */
      pstPluginInfo = orxPlugin_CreatePluginInfo();

      /* Valid? */
      if(pstPluginInfo != orxNULL)
      {
        /* Stores plugin info */
        pstPluginInfo->pstSysPlugin   = pstSysPlugin;
        pstPluginInfo->hPluginHandle  = (orxHANDLE)pstPluginInfo;

        /* Registers plugin */
        if(orxPlugin_RegisterPlugin(pstPluginInfo) != orxSTATUS_FAILURE)
        {
          /* Stores plugin IDs */
          pstPluginInfo->stNameID     = orxString_GetID(_zPluginName);
          pstPluginInfo->stResourceID = orxString_GetID(zFileName);
          pstPluginInfo->stShadowID   = (zLocation == acShadowLocation) ? orxString_GetID(zLocation) : orxSTRINGID_UNDEFINED;

          /* Gets plugin handle */
          hResult = pstPluginInfo->hPluginHandle;
        }
        else
        {
          /* Logs an error */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "Couldn't register the plugin <%s>, closing it.", _zPluginName);

          /* Closes plugin */
          orxPLUGIN_CLOSE(pstSysPlugin);

          /* Deletes allocated plugin info */
          orxPlugin_DeletePluginInfo(pstPluginInfo);
        }
      }
      else
      {
        /* Logs an error */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "Couldn't create a plugin info for plugin <%s>.", _zPluginName);

        /* Closes plugin */
        orxPLUGIN_CLOSE(pstSysPlugin);
      }
    }
    else
    {
      /* Logs an error */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "Couldn't load the plugin <%s>.", _zPluginName);
    }
  }

#else /* __orxPLUGIN_DYNAMIC__ */

  /* Logs message */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "Ignoring function call: this version of orx has been compiled without dynamic plugin support.");

#endif /* __orxPLUGIN_DYNAMIC__ */

  /* Done! */
  return hResult;
}

/** Unloads a plugin
 * @param[in] _hPluginHandle The handle of the plugin to unload
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxPlugin_Unload(orxHANDLE _hPluginHandle)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

#ifdef __orxPLUGIN_DYNAMIC__

  orxPLUGIN_INFO *pstPluginInfo;

  /* Checks */
  orxASSERT(sstPlugin.u32Flags & orxPLUGIN_KU32_STATIC_FLAG_READY);
  orxASSERT(_hPluginHandle != orxHANDLE_UNDEFINED);

  /* Gets plugin info */
  pstPluginInfo = orxPlugin_GetPluginInfo(_hPluginHandle);

  /* Valid? */
  if(pstPluginInfo != orxNULL)
  {
    /* Swapping? */
    if(orxFLAG_TEST(sstPlugin.u32Flags, orxPLUGIN_KU32_STATIC_FLAG_SWAP))
    {
      orxPLUGIN_SWAP_FUNCTION pfnSwap;

      /* Gets swap function */
      pfnSwap = (orxPLUGIN_SWAP_FUNCTION)orxPlugin_GetFunctionAddress(pstPluginInfo->pstSysPlugin, orxPLUGIN_KZ_SWAP_FUNCTION_NAME);

      /* Found? */
      if(pfnSwap != orxNULL)
      {
        /* Calls it */
        eResult = pfnSwap(orxPLUGIN_ENTRY_MODE_SWAP_OUT);
      }
      else
      {
        /* Updates result */
        eResult = orxSTATUS_FAILURE;
      }
    }
    else
    {
      orxPLUGIN_EXIT_FUNCTION pfnExit;

      /* Gets exit function */
      pfnExit = (orxPLUGIN_EXIT_FUNCTION)orxPlugin_GetFunctionAddress(pstPluginInfo->pstSysPlugin, orxPLUGIN_KZ_EXIT_FUNCTION_NAME);

      /* Found? */
      if(pfnExit != orxNULL)
      {
        /* Calls it */
        eResult = pfnExit(orxPLUGIN_ENTRY_MODE_EXIT);
      }
      else
      {
        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
    }

    /* Deletes plugin */
    orxPlugin_DeletePluginInfo(pstPluginInfo);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "Failed to get plugin info for handle.");
  }

#else /* __orxPLUGIN_DYNAMIC__ */

  /* Logs message */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "Ignoring function call: this version of orx has been compiled without dynamic plugin support.");

#endif /* __orxPLUGIN_DYNAMIC__ */

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
  orxPLUGIN_FUNCTION pfnFunction = orxNULL;

#ifdef __orxPLUGIN_DYNAMIC__

  orxPLUGIN_INFO *pstPluginInfo;

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
    pfnFunction = orxPlugin_GetFunctionAddress((orxSYSPLUGIN)pstPluginInfo->hPluginHandle, _zFunctionName);

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

#else /* __orxPLUGIN_DYNAMIC__ */

  /* Logs message */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "Ignoring function call: this version of orx has been compiled without dynamic plugin support.");

#endif /* __orxPLUGIN_DYNAMIC__ */

  /* Done! */
  return pfnFunction;
}

/** Gets the handle of a plugin given its name
 * @param[in] _zPluginName The plugin name
 * @return Its orxHANDLE / orxHANDLE_UNDEFINED
 */
orxHANDLE orxFASTCALL orxPlugin_GetHandle(const orxSTRING _zPluginName)
{
  orxHANDLE hPluginHandle = orxHANDLE_UNDEFINED;

#ifdef __orxPLUGIN_DYNAMIC__

  orxPLUGIN_INFO *pstPluginInfo;
  orxSTRINGID     stPluginID;

  /* Checks */
  orxASSERT(sstPlugin.u32Flags & orxPLUGIN_KU32_STATIC_FLAG_READY);
  orxASSERT(_zPluginName != orxNULL);

  /* Gets plugin ID */
  stPluginID = orxString_Hash(_zPluginName);

  /* Search all plugin info */
  for(pstPluginInfo = (orxPLUGIN_INFO *)orxBank_GetNext(sstPlugin.pstPluginBank, orxNULL);
      pstPluginInfo != orxNULL;
      pstPluginInfo = (orxPLUGIN_INFO *)orxBank_GetNext(sstPlugin.pstPluginBank, pstPluginInfo))
  {
    /* Found? */
    if(stPluginID == pstPluginInfo->stNameID)
    {
      /* Gets its handle */
      hPluginHandle = pstPluginInfo->hPluginHandle;
      break;
    }
  }

#else /* __orxPLUGIN_DYNAMIC__ */

  /* Logs message */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "Ignoring function call: this version of orx has been compiled without dynamic plugin support.");

#endif /* __orxPLUGIN_DYNAMIC__ */

  /* Done! */
  return hPluginHandle;
}

/** Gets the name of a plugin given its handle
 * @param[in] _hPluginHandle The plugin handle
 * @return The plugin name / orxSTRING_EMPTY
 */
const orxSTRING orxFASTCALL orxPlugin_GetName(orxHANDLE _hPluginHandle)
{
  const orxSTRING zPluginName = orxSTRING_EMPTY;

#ifdef __orxPLUGIN_DYNAMIC__

  orxPLUGIN_INFO *pstPluginInfo;

  /* Checks */
  orxASSERT(sstPlugin.u32Flags & orxPLUGIN_KU32_STATIC_FLAG_READY);
  orxASSERT(_hPluginHandle != orxHANDLE_UNDEFINED);

  /* Gets plugin info */
  pstPluginInfo = orxPlugin_GetPluginInfo(_hPluginHandle);

  /* Valid? */
  if(pstPluginInfo != orxNULL)
  {
    /* Gets plugin name */
    zPluginName = orxString_GetFromID(pstPluginInfo->stNameID);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "Failed to get plugin info for handle.");
  }

#else /* __orxPLUGIN_DYNAMIC__ */

  /* Logs message */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_PLUGIN, "Ignoring function call: this version of orx has been compiled without dynamic plugin support.");

#endif /* __orxPLUGIN_DYNAMIC__ */

  /* Done! */
  return zPluginName;
}

#undef orxPLUGIN_OPEN
#undef orxPLUGIN_GET_SYMBOL_ADDRESS
#undef orxPLUGIN_CLOSE
