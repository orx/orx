/**
 * @file orxMain.c
 * 
 * Main program implementation
 * 
 */
 
 /***************************************************************************
 orxMain.c
 Main program implementation
 
 begin                : 21/07/2005
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

#include "main/orxMain.h"
#include "memory/orxMemory.h"

#define orxMAIN_KU32_FLAG_NONE  0x00000000  /**< No flags have been set */
#define orxMAIN_KU32_FLAG_READY 0x00000001  /**< The module has been initialized */

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/
typedef struct __orxMAIN_MODULE_INFOS_t
{
  orxSTATUS eStatus;              /**< Module init status */
  orxU32 u32RefCount;             /**< Number of time that a Init has been called */
} orxMAIN_MODULE_INFOS;

typedef struct __orxMAIN_STATIC_t
{
  orxU32 u32Flags;                                              /**< Flags set by the main module */
  orxMAIN_MODULE_INFOS astModuleInfos[orxMAIN_MODULE_NUMBER];   /**< Array on infos for each declared modules */
} orxMAIN_STATIC;

/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/
orxSTATIC orxMAIN_STATIC sstMain;

/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/
/** Initialize the main module (will initialize all needed modules)
 */
orxSTATUS orxMain_Init()
{
  orxU32    u32Index;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Don't call twice the init function */
  orxASSERT((sstMain.u32Flags & orxMAIN_KU32_FLAG_READY) != orxMAIN_KU32_FLAG_READY);
  
  /* Require orxMemory to clear datas */
  orxMAIN_INIT_MODULE(Memory);
  
  /* Clear the static control */
  orxMemory_Set(&sstMain, 0, sizeof(orxMAIN_STATIC));
  
  /* Initialize modules datas */
  for (u32Index = 0; u32Index < orxMAIN_MODULE_NUMBER; u32Index++)
  {
    sstMain.astModuleInfos[u32Index].eStatus = orxSTATUS_FAILED;
  }
  
  /* Set module as initialized */
  sstMain.u32Flags |= orxMAIN_KU32_FLAG_READY;

  /* Done! */
  return eResult;
}

/** Exit main module
 */
orxVOID orxMain_Exit()
{
  /* Module initialized ? */
  orxASSERT((sstMain.u32Flags & orxMAIN_KU32_FLAG_READY) == orxMAIN_KU32_FLAG_READY);

  /* Set module as not ready */
  sstMain.u32Flags &= ~orxMAIN_KU32_FLAG_READY;
}

/** Call the Init callback function for a module
 * @param[in] _zName    Module's name
 * @param[in] _eModule  Module's type
 * @param[in] _cbInit   Init function
 * @return Module's Init status
 */
orxSTATUS orxMain_InitModule(orxCONST orxSTRING _zName, orxMAIN_MODULE _eModule, orxMAIN_MODULE_INIT_CB _cbInit)
{
  /* If not initialized yet, Init the module */
  if (sstMain.astModuleInfos[_eModule].u32RefCount == 0)
  {
    /* Call Init function */
    sstMain.astModuleInfos[_eModule].eStatus = _cbInit();
    
    /* First Call to the Init function, really Initialize it */
    if (sstMain.astModuleInfos[_eModule].eStatus == orxSTATUS_SUCCESS)
    {
      orxDEBUG_LOG(orxDEBUG_LEVEL_LOG, "INIT %s : First call => Init function success", _zName);
    }
    else
    {
      orxDEBUG_LOG(orxDEBUG_LEVEL_LOG, "INIT %s : First call => Init function FAILED !", _zName);
    }
  }
  
  /* Increases ref counter */
  sstMain.astModuleInfos[_eModule].u32RefCount++;
  
  if (sstMain.astModuleInfos[_eModule].u32RefCount > 1)
  {
    orxDEBUG_LOG(orxDEBUG_LEVEL_LOG, "INIT %s : %lu Init call", _zName, sstMain.astModuleInfos[_eModule].u32RefCount);
  }
  
  /* Return Init status */
  return sstMain.astModuleInfos[_eModule].eStatus;
}

/** Call the Exit callback function for a module
 * @param[in] _zName    Module's name
 * @param[in] _eModule  Module's type
 * @param[in] _cbExit   Exit function
 */
orxVOID orxMain_ExitModule(orxCONST orxSTRING _zName, orxMAIN_MODULE _eModule, orxMAIN_MODULE_EXIT_CB _cbExit)
{
  /* It's not possible that there are more Exit than Init */
  orxASSERT(sstMain.astModuleInfos[_eModule].u32RefCount > 0);
  
  /* Decreases the ref counter */
  sstMain.astModuleInfos[_eModule].u32RefCount--;
  
  /* Counter has reached 0 And module successfully initialized ? */
  if ((sstMain.astModuleInfos[_eModule].u32RefCount == 0) && (sstMain.astModuleInfos[_eModule].eStatus == orxSTATUS_SUCCESS))
  {
    /* Call Exit callback */
    _cbExit();
    
    /* Log Exit */
    orxDEBUG_LOG(orxDEBUG_LEVEL_LOG, "EXIT %s : Last call => Exit function called", _zName);
  }
  else
  {
    orxDEBUG_LOG(orxDEBUG_LEVEL_LOG, "EXIT %s : %lu call remaining", _zName, sstMain.astModuleInfos[_eModule].u32RefCount);
  }
}
