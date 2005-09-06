/**
 * @file orxDepend.c
 * 
 * Dependencies' module
 * 
 */
 
 /***************************************************************************
 orxDepend.c
 Dependencies' module
 
 begin                : 04/09/2005
 author               : (C) Arcallians
 email                : bestel@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "base/orxDepend.h"
#include "memory/orxMemory.h"
#include "debug/orxDebug.h"

#define orxDEPEND_KU32_FLAG_NONE  0x00000000  /**< No flags have been set */
#define orxDEPEND_KU32_FLAG_READY 0x00000001  /**< The module has been initialized */

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/
typedef struct __orxDEPEND_MODULE_INFOS_t
{
  orxSTATUS eStatus;              /**< Module init status */
  orxU32 u32RefCount;             /**< Number of time that a Init has been called */
} orxDEPEND_MODULE_INFOS;

typedef struct __orxDEPEND_STATIC_t
{
  orxU32 u32Flags;                                                /**< Flags set by the main module */
  orxDEPEND_MODULE_INFOS astModuleInfos[orxDEPEND_MODULE_NUMBER]; /**< Array on infos for each declared modules */
} orxDEPEND_STATIC;

/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/
orxSTATIC orxDEPEND_STATIC sstDepend;

/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/
/** Initialize the Dependency's module
 */
orxSTATUS orxDepend_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILED;
  
  /* Not already Initialized? */
  if(!(sstDepend.u32Flags & orxDEPEND_KU32_FLAG_READY))
  {
    /* Set module has ready */
    sstDepend.u32Flags = orxDEPEND_KU32_FLAG_READY;
      
    /* Success */
    eResult = orxSTATUS_SUCCESS;
  }
  
  /* Done */
  return eResult;
}

/** Uninitialize the Dependency's module
 */
orxVOID orxDepend_Exit()
{
  /* Module initialized ? */
  if ((sstDepend.u32Flags & orxDEPEND_KU32_FLAG_READY) == orxDEPEND_KU32_FLAG_READY)
  {
    /* Module not ready now */
    sstDepend.u32Flags = orxDEPEND_KU32_FLAG_NONE;
  }
}

/** Call the Init callback function for a module
 * @param[in] _zName    Module's name
 * @param[in] _eModule  Module's type
 * @param[in] _cbInit   Init function
 * @return Module's Init status
 */
orxSTATUS orxDepend_InitModule(orxCONST orxSTRING _zName, orxDEPEND_MODULE _eModule, orxDEPEND_INIT_CB _cbInit)
{
  /* If not initialized yet, Init the module */
  if (sstDepend.astModuleInfos[_eModule].u32RefCount == 0)
  {
    /* Call Init function */
    sstDepend.astModuleInfos[_eModule].eStatus = _cbInit();
    
    /* First Call to the Init function, really Initialize it */
    if (sstDepend.astModuleInfos[_eModule].eStatus == orxSTATUS_SUCCESS)
    {
      orxDEBUG_LOG(orxDEBUG_LEVEL_LOG, "INIT %s : First call => Init function success", _zName);
    }
    else
    {
      orxDEBUG_LOG(orxDEBUG_LEVEL_LOG, "INIT %s : First call => Init function FAILED !", _zName);
    }
  }
  
  /* Increases ref counter */
  sstDepend.astModuleInfos[_eModule].u32RefCount++;
  
  if (sstDepend.astModuleInfos[_eModule].u32RefCount > 1)
  {
    orxDEBUG_LOG(orxDEBUG_LEVEL_LOG, "INIT %s : %lu Init call", _zName, sstDepend.astModuleInfos[_eModule].u32RefCount);
  }
  
  /* Return Init status */
  return sstDepend.astModuleInfos[_eModule].eStatus;
}

/** Call the Exit callback function for a module
 * @param[in] _zName    Module's name
 * @param[in] _eModule  Module's type
 * @param[in] _cbExit   Exit function
 */
orxVOID orxDepend_ExitModule(orxCONST orxSTRING _zName, orxDEPEND_MODULE _eModule, orxDEPEND_EXIT_CB _cbExit)
{
  /* It's not possible that there are more Exit than Init */
  orxASSERT(sstDepend.astModuleInfos[_eModule].u32RefCount > 0);
  
  /* Decreases the ref counter */
  sstDepend.astModuleInfos[_eModule].u32RefCount--;
  
  /* Counter has reached 0 And module successfully initialized ? */
  if ((sstDepend.astModuleInfos[_eModule].u32RefCount == 0) && (sstDepend.astModuleInfos[_eModule].eStatus == orxSTATUS_SUCCESS))
  {
    /* Call Exit callback */
    _cbExit();
    
    /* Log Exit */
    orxDEBUG_LOG(orxDEBUG_LEVEL_LOG, "EXIT %s : Last call => Exit function called", _zName);
  }
  else
  {
    orxDEBUG_LOG(orxDEBUG_LEVEL_LOG, "EXIT %s : %lu call remaining", _zName, sstDepend.astModuleInfos[_eModule].u32RefCount);
  }
}

