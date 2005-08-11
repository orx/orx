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
#include "debug/orxDebug.h"

#define orxMAIN_KU32_FLAG_NONE  0x00000000  /**< No flags have been set */
#define orxMAIN_KU32_FLAG_READY 0x00000001  /**< The module has been initialized */

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/
typedef struct __orxMAIN_MODULE_INFOS_t
{
  orxSTATUS eStatus;              /**< Module init status */
  orxU32 u32RefCount;             /**< Number of time that a Init has been called */
  orxMAIN_MODULE_INIT_CB cbInit;  /**< Init module function */
  orxMAIN_MODULE_EXIT_CB cbExit;  /**< Exit module function */
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

/** Set Init and Exit Callback for a Module
 * @param[in] _eModule  Module's type
 * @param[in] _cbInit   Init callback
 * @param[in] _cbExit   Exit callback
 */
orxVOID orxMain_SetModuleCallback(orxMAIN_MODULE _eModule, orxMAIN_MODULE_INIT_CB _cbInit, orxMAIN_MODULE_EXIT_CB _cbExit)
{
  /* Module initialized ? */
  orxASSERT((sstMain.u32Flags & orxMAIN_KU32_FLAG_READY) == orxMAIN_KU32_FLAG_READY);

  /* Set callbacks */
  sstMain.astModuleInfos[_eModule].cbInit = _cbInit;
  sstMain.astModuleInfos[_eModule].cbExit = _cbExit;
}

/** Call the Init callback function for a module
 * @param[in] _eModule  Module's type
 */
orxVOID orxMain_InitModule(orxMAIN_MODULE _eModule)
{
  /* Module initialized ? */
  orxASSERT((sstMain.u32Flags & orxMAIN_KU32_FLAG_READY) == orxMAIN_KU32_FLAG_READY);
  
  /* If not initialized yet, Init the module */
  if (sstMain.astModuleInfos[_eModule].u32RefCount == 0)
  {
    /* Call Init function */
    sstMain.astModuleInfos[_eModule].eStatus = sstMain.astModuleInfos[_eModule].cbInit();
  }
  
  /* Increases ref counter */
  sstMain.astModuleInfos[_eModule].u32RefCount++;
}

/** Call the Exit callback function for a module
 * @param[in] _eModule  Module's type
 */
orxVOID orxMain_ExitModule(orxMAIN_MODULE _eModule)
{
  /* Module initialized ? */
  orxASSERT((sstMain.u32Flags & orxMAIN_KU32_FLAG_READY) == orxMAIN_KU32_FLAG_READY);
  
  /* Decreases the ref counter */
  sstMain.astModuleInfos[_eModule].u32RefCount--;
  
  /* Counter has reached 0 And module successfully initialized ? */
  if ((sstMain.astModuleInfos[_eModule].u32RefCount == 0) && (sstMain.astModuleInfos[_eModule].eStatus == orxSTATUS_SUCCESS))
  {
    /* Call Exit callback */
    sstMain.astModuleInfos[_eModule].cbExit();
  }
}
