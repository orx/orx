/***************************************************************************
 orxPlugin.c
 Plugin module
 
 begin                : 04/09/2002
 author               : (C) Arcallians
 email                : david.anderson@calixo.net
                      : iarwain@arcallians.org       (2003->)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "plugin/orxPlugin.h"

#include "debug/orxDebug.h"
#include "io/orxTextIO.h"
#include "memory/orxBank.h"
#include "memory/orxMemory.h"
#include "plugin/orxPluginUser.h"
#include "plugin/orxPluginCore.h"
#include "utils/orxHashTable.h"

#include "msg/msg_plugin.h"


#ifdef __orxLINUX__

  #include <dlfcn.h>

#else /* __orxLINUX__ */

  #ifdef __orxWINDOWS__

    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>

  #endif /* __orxWINDOWS__ */

#endif /* __orxLINUX__ */


/*
 * Platform dependant type & function defines
 */

/* WINDOWS */
#ifdef __orxWINDOWS__

  typedef HINSTANCE                                       orxSYSPLUGIN;

  #define orxPLUGIN_OPEN(PLUGIN)                          LoadLibrary(PLUGIN)
  #define orxPLUGIN_GET_SYMBOL_ADDRESS(PLUGIN, SYMBOL)    GetProcAddress(PLUGIN, SYMBOL)
  #define orxPLUGIN_CLOSE(PLUGIN)                         FreeLibrary(PLUGIN)

  orxSTATIC orxCONST orxSTRING                            szPluginLibraryExt = "dll";

/* OTHERS */
#else

  typedef orxVOID *                                       orxSYSPLUGIN;

  #define orxPLUGIN_OPEN(PLUGIN)                          dlopen(PLUGIN, RTLD_LAZY)
  #define orxPLUGIN_GET_SYMBOL_ADDRESS(PLUGIN, SYMBOL)    dlsym(PLUGIN, SYMBOL)
  #define orxPLUGIN_CLOSE(PLUGIN)                         dlclose(PLUGIN)

  orxSTATIC orxCONST orxSTRING                            szPluginLibraryExt = "so";

#endif



/*
 * Platform independant defines
 */

#define orxPLUGIN_KU32_FLAG_NONE                            0x00000000L
#define orxPLUGIN_KU32_FLAG_READY                           0x00000001L


#define orxPLUGIN_KU32_CORE_INFO_FLAG_NONE                  0x00000000L
#define orxPLUGIN_KU32_CORE_INFO_FLAG_LOADED                0x00000001L
#define orxPLUGIN_KU32_CORE_INFO_FLAG_DIRTY                 0x10000000L


#define orxPLUGIN_KU32_FUNCTION_BANK_SIZE                   16


/*
 * Information structure on a plugin function
 */
typedef struct __orxPLUGIN_FUNCTION_INFO_t
{
  /* Function ID */
  orxPLUGIN_FUNCTION_ID eFunctionID;                        /**< Function ID : 8 */

  /* Function pointer */
  orxPLUGIN_FUNCTION pfnFunction;                           /**< Function Address : 4 */

  /* Funtion name */
  orxSTRING zFunctionName;                                  /**< Function Name : 12 */

  /* Function argument types */
  orxSTRING zFunctionArgs;                                  /**< Function Argument Types : 16 */

} orxPLUGIN_FUNCTION_INFO;


/*
 * Information structure on a plugin
 */
typedef struct __orxPLUGIN_INFO_t
{
  /* System plugin access */
  orxSYSPLUGIN pstSysPlugin;                                /**< Plugin system pointer : 4 */

  /* Plugin handle */
  orxHANDLE hPluginHandle;                                  /**< Plugin handle : 8 */

  /* Function bank */
  orxBANK *pstFunctionBank;                                 /**< Function bank : 12 */

  /* Function hash table */
  orxHASHTABLE *pstFunctionTable;                           /**< Function hash table : 16 */

  /* Plugin name */
  orxSTRING zPluginName;                                    /**< Plugin name : 20 */

  /* Padding */
  orxPAD(20)

} orxPLUGIN_INFO;

/*
 * Core info structure
 */
typedef struct __orxPLUGIN_CORE_INFO_t
{
  /* Core functions : 4 */
  orxPLUGIN_CORE_FUNCTION orxCONST *pstCoreFunctionTable;

  /* Core functions counter : 8 */
  orxU32                            u32CoreFunctionCounter;

  /* Core module ID : 12 */
  orxMODULE_ID                      eModuleID;

  /* Core info status : 16 */
  orxU32                            u32Flags;

  /* Pad */
  orxPAD(16)

} orxPLUGIN_CORE_INFO;


/*
 * Static structure
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

/*
 * Static data
 */
orxSTATIC orxPLUGIN_STATIC sstPlugin;


/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 orxPlugin_UpdateAllModule
 
 This function updates all modules relying on core plugins. 
 Returns nothing.
 ***************************************************************************/
orxSTATIC orxINLINE orxVOID orxPlugin_UpdateAllModule()
{
  orxU32 i;

  /* For all core plugins */
  for(i = 0; i < orxPLUGIN_CORE_ID_NUMBER; i++)
  {
    /* Is plugin dirty? */
    if(sstPlugin.astCoreInfo[i].u32Flags & orxPLUGIN_KU32_CORE_INFO_FLAG_DIRTY)
    {
      orxU32 j;
      orxBOOL bLoaded;

      /* Checks all functions */
      for(j = 0, bLoaded = orxTRUE;
          (j < sstPlugin.astCoreInfo[i].u32CoreFunctionCounter) && (bLoaded == orxTRUE);
          j++)
      {
        /* Tests if function is loaded */
        bLoaded = (sstPlugin.astCoreInfo[i].pstCoreFunctionTable[j].pfnFunction != sstPlugin.astCoreInfo[i].pstCoreFunctionTable[j].pfnFunction) ?
                  orxTRUE :
                  orxFALSE;
      }

      /* Was not already loaded */
      if(!(sstPlugin.astCoreInfo[i].u32Flags & orxPLUGIN_KU32_CORE_INFO_FLAG_LOADED))
      {
        /* Is now loaded? */
        if(bLoaded != orxFALSE)
        {
          /* Marks as loaded */
          sstPlugin.astCoreInfo[i].u32Flags |= orxPLUGIN_KU32_CORE_INFO_FLAG_LOADED;
          
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
          sstPlugin.astCoreInfo[i].u32Flags &= ~orxPLUGIN_KU32_CORE_INFO_FLAG_LOADED;
          
          /* Tries to exit from associated module */
          orxModule_Exit(sstPlugin.astCoreInfo[i].eModuleID);
        }
      }

      /* Removes dirty flag */
      sstPlugin.astCoreInfo[i].u32Flags &= ~orxPLUGIN_KU32_CORE_INFO_FLAG_DIRTY;
    }
  }
  
  /* Done! */
  return;
}

/***************************************************************************
 orxPlugin_CreateFunctionInfo
 
 This function creates & initiates a function_info cell. 
 Returns created cell.
 ***************************************************************************/
orxPLUGIN_FUNCTION_INFO *orxFASTCALL orxPlugin_CreateFunctionInfo(orxPLUGIN_INFO *_pstPluginInfo)
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
    orxMemory_Set(pstFunctionInfo, 0, sizeof(orxPLUGIN_FUNCTION_INFO));
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return pstFunctionInfo;
}

/***************************************************************************
 orxPlugin_DeleteFunctionInfo

 This function deletes a function_info cell

 Returns nothing.
 ***************************************************************************/
orxVOID orxFASTCALL orxPlugin_DeleteFunctionInfo(orxPLUGIN_INFO *_pstPluginInfo, orxPLUGIN_FUNCTION_INFO *_pstFunctionInfo)
{
  /* Checks */
  orxASSERT(_pstPluginInfo != orxNULL);
  orxASSERT(_pstFunctionInfo != orxNULL);

  /* Deletes it */
  orxBank_Free(_pstPluginInfo->pstFunctionBank, _pstFunctionInfo);

  /* Done */
  return;
}

/***************************************************************************
 orxPlugin_RegisterCoreFunction
 
 This function registers a core function.
 Returns orxVOID.
 ***************************************************************************/
orxSTATIC orxINLINE orxVOID orxPlugin_RegisterCoreFunction(orxCONST orxPLUGIN_FUNCTION_INFO *_pfnFunctionInfo)
{
  orxCONST orxPLUGIN_CORE_FUNCTION *pstCoreFunction;
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

    /* Was not already loaded? */
    if(*(pstCoreFunction[u32FunctionIndex].pfnFunction) == pstCoreFunction[u32FunctionIndex].pfnDefaultFunction)
    {
      /* Registers core function */
      *(pstCoreFunction[u32FunctionIndex].pfnFunction) = _pfnFunctionInfo->pfnFunction;

      /* Updates plugin status */
      sstPlugin.astCoreInfo[u32PluginIndex].u32Flags |= orxPLUGIN_KU32_CORE_INFO_FLAG_DIRTY;
    }
    else
    {
      /* !!! MSG !!! */
    }
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return;
}

/***************************************************************************
 orxPlugin_UnregisterCoreFunction
 
 This function unregisters a core function.
 Returns orxVOID.
 ***************************************************************************/
orxSTATIC orxINLINE orxVOID orxPlugin_UnregisterCoreFunction(orxCONST orxPLUGIN_FUNCTION_INFO *_pfnFunctionInfo)
{
  orxCONST orxPLUGIN_CORE_FUNCTION *pstCoreFunction;
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
    sstPlugin.astCoreInfo[u32PluginIndex].u32Flags |= orxPLUGIN_KU32_CORE_INFO_FLAG_DIRTY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return;
}

/***************************************************************************
 orxPlugin_CreatePluginInfo
 
 This function creates & initiates a plugin_info cell. 
 Returns created cell.
 ***************************************************************************/
orxPLUGIN_INFO *orxPlugin_CreatePluginInfo()
{
  orxPLUGIN_INFO *pstPluginInfo;

  /* Creates a plugin info */
  pstPluginInfo = (orxPLUGIN_INFO *)orxBank_Allocate(sstPlugin.pstPluginBank);

  /* Valid? */
  if(pstPluginInfo != orxNULL)
  {
    /* Inits it */
    orxMemory_Set(pstPluginInfo, 0, sizeof(orxPLUGIN_INFO));

    /* Undefines plugin handle */
    pstPluginInfo->hPluginHandle      = orxHANDLE_Undefined;

    /* Creates function bank */
    pstPluginInfo->pstFunctionBank    = orxBank_Create(orxPLUGIN_KU32_FUNCTION_BANK_SIZE, sizeof(orxPLUGIN_FUNCTION_INFO), orxBANK_KU32_FLAGS_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if(pstPluginInfo->pstFunctionBank != orxNULL)
    {
      /* Creates function hash table */
      pstPluginInfo->pstFunctionTable = orxHashTable_Create(orxPLUGIN_KU32_FUNCTION_BANK_SIZE, orxHASHTABLE_KU32_FLAGS_NONE, orxMEMORY_TYPE_MAIN);
      
      /* Invalid? */
      if(pstPluginInfo->pstFunctionTable == orxNULL)
      {
        /* !!! MSG !!! */

        /* Frees previously allocated data */
        orxBank_Delete(pstPluginInfo->pstFunctionBank);
        orxBank_Free(sstPlugin.pstPluginBank, pstPluginInfo);
        
        /* Not successful */
        pstPluginInfo = orxNULL;
      }
    }
    else
    {
      /* !!! MSG !!! */

      /* Frees previously allocated data */
      orxBank_Free(sstPlugin.pstPluginBank, pstPluginInfo);
        
      /* Not successful */
      pstPluginInfo = orxNULL;
    }
  }

  /* Done! */
  return pstPluginInfo;
}

/***************************************************************************
 orxPlugin_DeletePluginInfo
 
 This function completely shuts down the given plugin.
 It should be called from orxPlugin_Unload or orxPlugin_DeleteAll.
 
 Returns nothing.
 ***************************************************************************/
orxVOID orxFASTCALL orxPlugin_DeletePluginInfo(orxPLUGIN_INFO *_pstPluginInfo)
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

/***************************************************************************
 orxPlugin_GetPluginInfo
 
 This function finds a plugin info, and returns a reference to it
 if it exists, otherwise orxNULL.

 ***************************************************************************/
orxSTATIC orxINLINE orxPLUGIN_INFO *orxPlugin_GetPluginInfo(orxHANDLE _hPluginHandle)
{
  orxPLUGIN_INFO *pstPluginInfo = orxNULL;

  /* Checks */
  orxASSERT(_hPluginHandle != orxHANDLE_Undefined);

  /* Gets plugin info */
  pstPluginInfo = (orxPLUGIN_INFO *)_hPluginHandle;

  /* Checks */
  orxASSERT(pstPluginInfo->hPluginHandle == _hPluginHandle);

  /* Done! */
  return pstPluginInfo;
}

/***************************************************************************
 orxPlugin_GetFunctionAddress
 Returns a  pointer to the requested function.
 
 returns: orxPLUGIN_FUNCTION if success, orxNULL otherwise
 ***************************************************************************/
orxPLUGIN_FUNCTION orxFASTCALL orxPlugin_GetFunctionAddress(orxSYSPLUGIN _pstSysPlugin, orxCONST orxSTRING _zFunctionName)
{
  orxPLUGIN_FUNCTION pfnFunction = orxNULL;

  /* Checks */
  orxASSERT(_pstSysPlugin != orxHANDLE_Undefined);
  orxASSERT(_zFunctionName != orxNULL);
  
  /* Gets function */
  pfnFunction = (orxPLUGIN_FUNCTION)orxPLUGIN_GET_SYMBOL_ADDRESS(_pstSysPlugin, _zFunctionName);

  /* Not found? */
  if(pfnFunction == orxNULL)
  {
    /* Logs an error */
    orxDEBUG_LOG(orxDEBUG_LEVEL_PLUGIN, MSG_PLUGIN_KZ_FUNCTION_NOT_FOUND);
  }

  /* Done! */
  return pfnFunction;
}

/***************************************************************************
 orxPlugin_RegisterPlugin
 
 This function registers a plugin.
 Returns orxSTATUS_SUCCESS / orxSTATUS_FAILURE.
 ***************************************************************************/
orxSTATUS orxPlugin_RegisterPlugin(orxSYSPLUGIN _pstSysPlugin, orxPLUGIN_INFO *_pstPluginInfo)
{
  orxPLUGIN_FUNCTION pfnInit;
  orxU32 u32UserFunctionNumber;
  orxPLUGIN_USER_FUNCTION_INFO *astUserFunctionInfo;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(_pstSysPlugin != orxNULL);
  orxASSERT(_pstPluginInfo != orxNULL);

  /* Gets init function */
  pfnInit = orxPlugin_GetFunctionAddress(_pstSysPlugin, orxPLUGIN_KZ_INIT_FUNCTION_NAME);

  /* Valid? */
  if(pfnInit != orxNULL)
  {
    orxU32 i;

    /* Calls it */
    pfnInit(&u32UserFunctionNumber, &astUserFunctionInfo);

    /* Adds all functions to plugin info */
    for(i = 0; i < u32UserFunctionNumber; i++)
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
          orxPlugin_RegisterCoreFunction(pstFunctionInfo);
        }
      }
      else
      {
        /* !!! MSG !!! */
      }
    }
  }
  else
  {
    /* !!! MSG !!! */

    /* Can't load plugin */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxPlugin_AddCoreInfo
 
 This function adds a core plugin info structure to the global info array.
 Returns orxVOID.
 ***************************************************************************/
orxVOID orxFASTCALL orxPlugin_AddCoreInfo(orxPLUGIN_CORE_ID _ePluginCoreID, orxMODULE_ID _eModuleID, orxCONST orxPLUGIN_CORE_FUNCTION *_astCoreFunction, orxU32 _u32CoreFunctionNumber)
{
  /* Checks */
  orxASSERT(sstPlugin.u32Flags & orxPLUGIN_KU32_FLAG_READY);
  orxASSERT(sstPlugin.astCoreInfo[_ePluginCoreID].pstCoreFunctionTable == orxNULL);
  orxASSERT(_ePluginCoreID < orxPLUGIN_CORE_ID_NUMBER);
  orxASSERT(_eModuleID < orxMODULE_ID_NUMBER);
  orxASSERT(_astCoreFunction != orxNULL);

  /* Stores info */
  sstPlugin.astCoreInfo[_ePluginCoreID].pstCoreFunctionTable    = _astCoreFunction;
  sstPlugin.astCoreInfo[_ePluginCoreID].u32CoreFunctionCounter  = _u32CoreFunctionNumber;
  sstPlugin.astCoreInfo[_ePluginCoreID].eModuleID               = _eModuleID;
  sstPlugin.astCoreInfo[_ePluginCoreID].u32Flags                = orxPLUGIN_KU32_CORE_INFO_FLAG_NONE;

  return;
}

/***************************************************************************
 orxPlugin_DeleteAll
 Deletes all plugins.

 returns: orxVOID
 ***************************************************************************/
orxSTATIC orxINLINE orxVOID orxPlugin_DeleteAll()
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


/***************************************************************************
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 orxPlugin_Setup
 Plugin module setup.

 returns: nothing
 ***************************************************************************/
orxVOID orxPlugin_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_PLUGIN, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_PLUGIN, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_PLUGIN, orxMODULE_ID_HASHTABLE);
  orxModule_AddDependency(orxMODULE_ID_PLUGIN, orxMODULE_ID_TEXTIO);

  return;
}

/***************************************************************************
 orxPlugin_Init
 Inits plugin system.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 ***************************************************************************/
orxSTATUS orxPlugin_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstPlugin.u32Flags & orxPLUGIN_KU32_FLAG_READY))
  {
    /* Cleans control structure */
    orxMemory_Set(&sstPlugin, 0, sizeof(orxPLUGIN_STATIC));

    /* Creates an empty spst_plugin_list */
    sstPlugin.pstPluginBank = orxBank_Create(orxPLUGIN_CORE_ID_NUMBER, sizeof(orxPLUGIN_INFO), orxBANK_KU32_FLAGS_NONE, orxMEMORY_TYPE_MAIN);

    /* Is bank valid? */
    if(sstPlugin.pstPluginBank != orxNULL)
    {
      /* Updates status flags */
      sstPlugin.u32Flags = orxPLUGIN_KU32_FLAG_READY;

      /* Registers all core plugins */
      orxPlugin_RegisterCorePlugins();

      /* Successful */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* !!! MSG !!! */

      /* Bank not created */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* !!! MSG !!! */

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxPlugin_Exit
 Exits from the plugin system.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxPlugin_Exit()
{
  /* Initialized? */
  if(sstPlugin.u32Flags & orxPLUGIN_KU32_FLAG_READY)
  {
    /* Deletes plugin list */
    orxPlugin_DeleteAll();

    /* Destroys plugin bank */
    orxBank_Delete(sstPlugin.pstPluginBank);
    sstPlugin.pstPluginBank = orxNULL;

    /* Updates flags */
    sstPlugin.u32Flags &= ~orxPLUGIN_KU32_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}


/***************************************************************************
 orxPlugin_DefaultCoreFunction
 Default function for non-initialized core plugin functions.
 Log the problem.

 returns: orxNULL
 ***************************************************************************/
orxVOID *orxFASTCALL orxPlugin_DefaultCoreFunction(orxCONST orxSTRING _zFunctionName, orxCONST orxSTRING _zFileName, orxU32 _u32Line)
{
  orxDEBUG_FLAG_BACKUP();
  orxDEBUG_FLAG_SET(orxDEBUG_KU32_FLAG_CONSOLE
                   |orxDEBUG_KU32_FLAG_FILE
                   |orxDEBUG_KU32_FLAG_TIMESTAMP
                   |orxDEBUG_KU32_FLAG_TYPE,
                    orxDEBUG_KU32_FLAG_ALL);
  orxDEBUG_LOG3(orxDEBUG_LEVEL_ALL, MSG_PLUGIN_KZ_DEFAULT_NOT_LOADED_ZZI, _zFunctionName, _zFileName, _u32Line);
  orxDEBUG_FLAG_RESTORE();

  return orxNULL;
}

/***************************************************************************
 orxPlugin_Load
 Loads the shared object specified as a null terminated string, and makes
 it subsequently available under the name in the 2nd param.
 
 returns: plugin handle on success, orxHANDLE_Undefined if load error
 ***************************************************************************/
orxHANDLE orxFASTCALL orxPlugin_Load(orxCONST orxSTRING _zPluginFileName, orxCONST orxSTRING _zPluginName)
{
  orxSYSPLUGIN pstSysPlugin;
  orxPLUGIN_INFO *pstPluginInfo;
  orxHANDLE hPluginHandle = orxHANDLE_Undefined;

  /* Checks */
  orxASSERT(sstPlugin.u32Flags & orxPLUGIN_KU32_FLAG_READY);
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
        /* !!! MSG !!! */

        /* Closes plugin */
        orxPLUGIN_CLOSE(pstSysPlugin);

        /* Deletes allocated plugin info */
        orxPlugin_DeletePluginInfo(pstPluginInfo);

        /* Empty plugin */
        hPluginHandle = orxHANDLE_Undefined;
      }
    }
    else
    {
      /* Closes plugin */
      orxPLUGIN_CLOSE(pstSysPlugin);

      /* Logs an error */
      orxDEBUG_LOG(orxDEBUG_LEVEL_PLUGIN, MSG_PLUGIN_KZ_MALLOC_FAILED);
    }
  }
  else
  {
    /* Logs an error */
    orxDEBUG_LOG(orxDEBUG_LEVEL_PLUGIN, MSG_PLUGIN_KZ_LOAD_FAILED);
  }

  /* Returns its handle */
  return hPluginHandle;
}


/***************************************************************************
 orxPlugin_LoadUsingExt
 Loads the shared object specified as a null terminated string using OS library extension,
 and makes it subsequently available under the name in the 2nd param.

 returns: plugin handle on success, orxHANDLE_Undefined if load error
 ***************************************************************************/
orxHANDLE orxFASTCALL orxPlugin_LoadUsingExt(orxCONST orxSTRING _zPluginFileName, orxCONST orxSTRING _zPluginName)
{
  orxCHAR zFileName[256];

  /* Checks */
  orxASSERT(sstPlugin.u32Flags & orxPLUGIN_KU32_FLAG_READY);
  orxASSERT(_zPluginFileName != orxNULL);
  orxASSERT(orxString_Length(_zPluginFileName) < 252);
  orxASSERT(_zPluginName != orxNULL);

  /* Gets complete name */
  orxTextIO_Printf(zFileName, "%s.%s", _zPluginFileName, szPluginLibraryExt);

  /* Does it */
  return(orxPlugin_Load(zFileName, _zPluginName));
}

/***************************************************************************
 orxPlugin_Unload
 Unloads the shared object designated by its handle
 
 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 ***************************************************************************/
orxSTATUS orxFASTCALL orxPlugin_Unload(orxHANDLE _hPluginHandle)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;
  orxPLUGIN_INFO *pstPluginInfo;

  /* Checks */
  orxASSERT(sstPlugin.u32Flags & orxPLUGIN_KU32_FLAG_READY);
  orxASSERT(_hPluginHandle != orxNULL);

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
    /* Logs error */
    orxDEBUG_LOG(orxDEBUG_LEVEL_PLUGIN, MSG_PLUGIN_KZ_NOT_FOUND);

    /* Not found */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxPlugin_GetFunction
 Returns the requested function pointer;
 
 returns: orxPLUGIN_FUNCTION / orxNULL
 ***************************************************************************/
orxPLUGIN_FUNCTION orxFASTCALL orxPlugin_GetFunction(orxHANDLE _hPluginHandle, orxCONST orxSTRING _zFunctionName)
{
  orxPLUGIN_INFO *pstPluginInfo;
  orxPLUGIN_FUNCTION pfnFunction = orxNULL;

  /* Checks */
  orxASSERT(sstPlugin.u32Flags & orxPLUGIN_KU32_FLAG_READY);
  orxASSERT(_hPluginHandle != orxNULL);
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
      /* Logs an error */
      orxDEBUG_LOG(orxDEBUG_LEVEL_PLUGIN, MSG_PLUGIN_KZ_FUNCTION_NOT_FOUND);
    }
  }
  else
  {
    /* Logs an error */
    orxDEBUG_LOG(orxDEBUG_LEVEL_PLUGIN, MSG_PLUGIN_KZ_FUNCTION_NOT_FOUND);
  }

  /* Done! */
  return pfnFunction;
}

/***************************************************************************
 orxPlugin_GetHandle
 
 This function returns the handle of the given plugin.
 
 returns plugin handle on success, orxHANDLE_Undefined otherwise
 ***************************************************************************/
orxHANDLE orxFASTCALL orxPlugin_GetHandle(orxCONST orxSTRING _zPluginName)
{
  orxPLUGIN_INFO *pstPluginInfo;
  orxHANDLE hPluginHandle = orxHANDLE_Undefined;

  /* Checks */
  orxASSERT(sstPlugin.u32Flags & orxPLUGIN_KU32_FLAG_READY);
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

/***************************************************************************
 orxPlugin_GetName
 
 This function returns the name of the given plugin.
 
 returns Name/orxSTRING_Empty
 ***************************************************************************/
orxSTRING orxFASTCALL orxPlugin_GetName(orxHANDLE _hPluginHandle)
{
  orxPLUGIN_INFO *pstPluginInfo;
  orxSTRING zPluginName = orxSTRING_Empty;

  /* Checks */
  orxASSERT(sstPlugin.u32Flags & orxPLUGIN_KU32_FLAG_READY);
  orxASSERT(_hPluginHandle != orxHANDLE_Undefined);

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
    /* Logs an error */
    orxDEBUG_LOG(orxDEBUG_LEVEL_PLUGIN, MSG_PLUGIN_KZ_NOT_FOUND);
  }

  /* Done! */
  return(zPluginName);
}
