/**
 * @file orxTest.c
 * 
 * Test Module - Offers a basic interface to display a menu of registered test functions
 * 
 */
 
 /***************************************************************************
 orxTest.c
 Test Module
 
 begin                : 02/04/2005
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

#include "utils/orxTest.h"
#include "debug/orxDebug.h"
#include "memory/orxMemory.h"
#include "io/orxTextIO.h"
#include "utils/orxString.h"

#define orxTEST_KU32_FLAG_NONE  0x00000000  /**< No flags have been set */
#define orxTEST_KU32_FLAG_READY 0x00000001  /**< The module has been initialized */

#define orxTEST_KU32_MAX_REGISTERED_FUNCTIONS 256

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/
typedef struct __orxTEST_t
{
  orxCHAR zModule[32];            /**< Module Name */
  orxCHAR zMenuEntry[256];        /**< Description of the command */
  orxTEST_FUNCTION pfnFunction; /**< Pointer on the registered function to execute */
  orxBOOL bDisplayed;             /**< Menu entry displayed in the menu */
} orxTEST;

typedef struct __orxTEST_STATIC_t
{
  orxU32 u32Flags;                                                  /**< Module flags */
  orxTEST astTestFunctions[orxTEST_KU32_MAX_REGISTERED_FUNCTIONS];  /**< List of functions */
  orxU32 u32NbRegisteredFunc;                                       /**< Number of registered functions */
} orxTEST_STATIC;

/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/
static orxTEST_STATIC sstTest;

/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Display list of functions associated to a module
 * @param _zModuleName  (IN)  Name of the module
 */
orxSTATIC orxINLINE orxVOID orxTest_PrintModuleFunc(orxCONST orxSTRING _zModuleName)
{
  orxU32 u32Index;  /* Index used to traverse the function array */

  /* Module initialized ? */
  orxASSERT((sstTest.u32Flags & orxTEST_KU32_FLAG_READY) == orxTEST_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_zModuleName != orxNULL);
  
  /* Display Module Name */
  orxTextIO_PrintLn("\n**** MODULE : %s ****", _zModuleName);
  
  /* Display all functions not already displayed and that have the same module name */
  for (u32Index = 0; u32Index < sstTest.u32NbRegisteredFunc; u32Index++)
  {
    if (!(sstTest.astTestFunctions[u32Index].bDisplayed) && (orxMemory_Compare(sstTest.astTestFunctions[u32Index].zModule, _zModuleName, orxString_Length(_zModuleName) * sizeof(orxCHAR)) == 0))
    {
      sstTest.astTestFunctions[u32Index].bDisplayed = orxTRUE;
      orxTextIO_PrintLn("* %lu - %s", u32Index, sstTest.astTestFunctions[u32Index].zMenuEntry);
    }
  }
}

/** Reset all visibility flag (set with orxTest_PrintModuleFunc to knwow if a function has already been displayed
 */
orxSTATIC orxINLINE orxVOID orxTest_ResetVisibility()
{
  orxU32 u32Index;  /* Index used to traverse the function array */
  
  /* Module initialized ? */
  orxASSERT((sstTest.u32Flags & orxTEST_KU32_FLAG_READY) == orxTEST_KU32_FLAG_READY);
  
  for (u32Index = 0; u32Index < sstTest.u32NbRegisteredFunc; u32Index++)
  {
    sstTest.astTestFunctions[u32Index].bDisplayed = orxFALSE;
  }
}

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Initialize the test module
 */
orxSTATUS orxTest_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILED;

  /* Init dependencies */
  if ((orxDEPEND_INIT(Depend) &
       orxDEPEND_INIT(Memory)) == orxSTATUS_SUCCESS)
  {
    /* Not already Initialized? */
    if(!(sstTest.u32Flags & orxTEST_KU32_FLAG_READY))
    {
      /* Initialize values */
      orxMemory_Set(&sstTest, 0, sizeof(orxTEST_STATIC));
      sstTest.u32NbRegisteredFunc = 0;
    
      /* Module ready */
      sstTest.u32Flags |= orxTEST_KU32_FLAG_READY;
      
      /* Success */
      eResult = orxSTATUS_SUCCESS;
    }
  }
    
  return eResult;
}

/** Uninitialize the test module
 */
orxVOID orxTest_Exit()
{
  /* Module initialized ? */
  if ((sstTest.u32Flags & orxTEST_KU32_FLAG_READY) == orxTEST_KU32_FLAG_READY)
  {
    /* Module becomes not ready */
    sstTest.u32Flags &= ~orxTEST_KU32_FLAG_READY;
  }
  
  /* Exit dependencies */
  orxDEPEND_EXIT(Memory);
  orxDEPEND_EXIT(Depend);
}

/** Register a new function
 * @param   (IN)  _zModuleName      Name of the module (to group a list of functions)
 * @param   (IN)  _zMenuEntry       Text displayed to describe the test function
 * @param   (IN)  _pfnFunction       Function executed when the menu entry is selected
 * @return Returns an Handle on the function
 */
orxHANDLE orxFASTCALL orxTest_Register(orxCONST orxSTRING _zModuleName, orxCONST orxSTRING _zMenuEntry, orxCONST orxTEST_FUNCTION _pfnFunction)
{
  orxTEST *pstTest; /* Structure that will store new datas */
  orxHANDLE hRet;   /* Returnd handle value */
  
  /* Module initialized ? */
  orxASSERT((sstTest.u32Flags & orxTEST_KU32_FLAG_READY) == orxTEST_KU32_FLAG_READY);
    
  /* Correct parameters ? */
  orxASSERT(_zModuleName != orxNULL);
  orxASSERT(_zMenuEntry != orxNULL);
  orxASSERT(_pfnFunction != orxNULL);

  /* Module full ? */
  if (sstTest.u32NbRegisteredFunc < orxTEST_KU32_MAX_REGISTERED_FUNCTIONS)
  {
    /* Copy datas in the registered function array */
    pstTest = &(sstTest.astTestFunctions[sstTest.u32NbRegisteredFunc]);
    orxMemory_Copy(pstTest->zModule, _zModuleName, 31 * sizeof(orxCHAR));
    orxMemory_Copy(pstTest->zMenuEntry, _zMenuEntry, 255 * sizeof(orxCHAR));
    pstTest->pfnFunction = _pfnFunction;
    pstTest->bDisplayed = orxFALSE;
    
    /* Set the returned handle (array index value) */
    hRet = (orxHANDLE)sstTest.u32NbRegisteredFunc;
    
    /* Increase the number of stored functions */
    sstTest.u32NbRegisteredFunc++;
  }
  else
  {
    /* No space left for new function, return an undefined handle */
    hRet = orxHANDLE_Undefined;
  }
  
  return hRet;
}

/** Execute a registered function
 * @param   (IN)  _hRegisteredFunc  Handle of the registered function to execute
 * @return  Returns the success / fail status (failed when the Handle is unknown)
 */
orxSTATUS orxFASTCALL orxTest_Execute(orxHANDLE _hRegisteredFunc)
{
  /* Module initialized ? */
  orxASSERT((sstTest.u32Flags & orxTEST_KU32_FLAG_READY) == orxTEST_KU32_FLAG_READY);

  /* Correct parameters ? */
  if ((_hRegisteredFunc != orxHANDLE_Undefined) && ((orxU32)_hRegisteredFunc < sstTest.u32NbRegisteredFunc))
  {
    /* Execute the function */
    sstTest.astTestFunctions[(orxU32)_hRegisteredFunc].pfnFunction();
    
    return orxSTATUS_SUCCESS;
  }
  
  return orxSTATUS_FAILED;
}

/** Display a Menu with registered function list
 */
orxVOID orxTest_DisplayMenu()
{
  orxU32 u32Index;  /* Index used to traverse the function array */
  
  /* Module initialized ? */
  orxASSERT((sstTest.u32Flags & orxTEST_KU32_FLAG_READY) == orxTEST_KU32_FLAG_READY);
  
  /* Display menu header */
  printf("\n\n*************************************\n");
  printf("********** orx TEST MENU ************\n");
  printf("*************************************\n");

  /* Display menu for each module */
  for(u32Index = 0; u32Index < sstTest.u32NbRegisteredFunc; u32Index++)
  {
    if (!sstTest.astTestFunctions[u32Index].bDisplayed)
    {
      orxTest_PrintModuleFunc(sstTest.astTestFunctions[u32Index].zModule);
    }
  }
  
  /* Reset temp helper bool */
  orxTest_ResetVisibility();
}

/** Run the test engine
 * @param[in] _u32NbParam Number of parameters read
 * @param[in] _azParams   Array of parameters
 */
orxVOID orxTest_Run(orxU32 _u32NbParam, orxSTRING _azParams[])
{
  
}
