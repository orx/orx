/**
 * @file
 * 
 * Clock Test module
 * 
 */
 
 /***************************************************************************
 orxTest_Clock.c
 Clocks' Test Program
 
 begin                : 10/09/2005
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

#include "orxInclude.h"
#include "utils/orxTest.h"
#include "core/orxClock.h"
#include "io/orxTextIO.h"
#include "memory/orxBank.h"
#include "math/orxMath.h"

/******************************************************
 * DEFINES
 ******************************************************/

#define orxTEST_CLOCK_KU32_MAX_CLOCK 8 /* Max number of clock that the test module will use */

/******************************************************
 * TYPES AND STRUCTURES
 ******************************************************/

typedef struct __orxTEST_CLOCK_INFO_t
{
  orxCLOCK *pstClock;
} orxTEST_CLOCK_INFO;

typedef struct __orxTEST_CLOCK_STATIC_t
{
  orxBANK *pstBank;         /* Bank of clock */
  orxBOOL  bExit;           /* Become orxTRUE when the program have to exit the engine simulation loop */
  orxBOOL  bInit;           /* Init has been called */
  orxU32   u32InitCounter;  /* Number of called init */
} orxTEST_CLOCK_STATIC;

/******************************************************
 * GLOBAL VARIABLES
 ******************************************************/
orxSTATIC orxTEST_CLOCK_STATIC sstTest_Clock;

/******************************************************
 * PRIVATE FUNCTIONS
 ******************************************************/
orxSTATUS orxTest_Clock_Depend()
{
  /* Try to initialize the clock module (needed since it can't be done on loading */  
  if (!sstTest_Clock.bInit)
  {
    /* Increase counter */
    sstTest_Clock.u32InitCounter++;
    
    /* Init Clock */
    if (orxDEPEND_INIT(Clock) == orxSTATUS_SUCCESS)
    {
      sstTest_Clock.bInit = orxTRUE;
    }
  }
  
  if (sstTest_Clock.bInit)
  {
    return orxSTATUS_SUCCESS;
  }
  else
  {
    orxTextIO_PrintLn("Time plugin not loaded");
    return orxSTATUS_FAILED;
  }
}
 
orxVOID orxTest_Clock_DisplayInfos(orxCONST orxCLOCK_INFO *_pstClockInfo)
{
  /* Correct parameters ? */
  orxASSERT(_pstClockInfo != orxNULL);

  /* Display informations */
  orxTextIO_PrintLn("* eType          = %d",    _pstClockInfo->eType);
  orxTextIO_PrintLn("* u32TickCounter = %lu",   _pstClockInfo->u32TickCounter);
  orxTextIO_PrintLn("* u32TickSize     = %lu",   _pstClockInfo->u32TickSize);
  orxTextIO_PrintLn("* u32TickValue    = %lu",   _pstClockInfo->u32TickValue);
  orxTextIO_PrintLn("* eModType       = %d",    _pstClockInfo->eModType);
  orxTextIO_PrintLn("* fModValue      = %f",    _pstClockInfo->fModValue);
  orxTextIO_PrintLn("* u32DT           = %lu",   _pstClockInfo->u32DT);
  orxTextIO_PrintLn("* Time           = %lu\n", _pstClockInfo->u32Time);
}

orxVOID orxFASTCALL orxTest_Clock_EndLoop(orxCONST orxCLOCK_INFO *_pstClockInfo, orxVOID *_pstContext)
{
  /* Exit the loop */
  sstTest_Clock.bExit = orxTRUE;
}

orxVOID orxFASTCALL orxTest_Clock_ShowInfos(orxCONST orxCLOCK_INFO *_pstClockInfo, orxVOID *_pstContext)
{
  /* Correct parameters ? */
  orxASSERT(_pstClockInfo != orxNULL);
  orxASSERT(_pstContext != orxNULL);
  
  /* Show clock ID */
  orxTextIO_PrintLn("Clock Address = %x", _pstContext);
  
  /* Show informations */
  orxTest_Clock_DisplayInfos(_pstClockInfo);
}

/******************************************************
 * TEST FUNCTIONS
 ******************************************************/
/** Display informations about this test module
 */
orxVOID orxTest_Clock_Infos()
{
  orxTextIO_PrintLn("This test module is able to runs few clocks");
  orxTextIO_PrintLn("* You can register some clock with a different tick for each one and Simulate the loop engine");
  orxTextIO_PrintLn("* A 'time' plugin must have been loaded before using this test module");
}

orxVOID orxTest_Clock_Create()
{
  orxU32 u32TickSize;
  orxCLOCK *pstClock;
  orxTEST_CLOCK_INFO *pstClockCell;

  if (orxTest_Clock_Depend() == orxSTATUS_SUCCESS)
  {
    orxTextIO_PrintLn("This command allows you to create a Clock. To simplify");
    orxTextIO_PrintLn("the test module, you won't be able to chose the clock type.");
    orxTextIO_PrintLn("Only the tick size.");
    
    /* Read tick size */
    orxTextIO_ReadS32InRange(&u32TickSize, 10, 1, 0x7FFFFFFF, "Choose a tick size", orxTRUE);
    
    /* Try to create the clock */
    pstClock = orxClock_Create(u32TickSize, orxCLOCK_TYPE_USER);
    
    /* Clock created ? */
    if (pstClock != orxNULL)
    {
      /* Now, try to register the update function */
      if (orxClock_Register(pstClock, orxTest_Clock_ShowInfos, pstClock) == orxSTATUS_SUCCESS)
      {
        /* Store the clock address */
        if ((pstClockCell = (orxTEST_CLOCK_INFO *)orxBank_Allocate(sstTest_Clock.pstBank)))
        {
          pstClockCell->pstClock = pstClock;
          
          /* Clock Created  */
          orxTextIO_PrintLn("Clock %x created.", pstClock);
        }
        else
        {
          /* Can't allocate a new cell. */
          orxTextIO_PrintLn("Can't allocate a new bank cell");
          orxTextIO_Print("Unregister function : ");
          if (orxClock_Unregister(pstClock, orxTest_Clock_ShowInfos) == orxSTATUS_SUCCESS)
          {
            orxTextIO_PrintLn("Success");
          }
          else
          {
            orxTextIO_PrintLn("Failed");
          }
          
          /* Delete clock */
          orxClock_Delete(pstClock);
        }
      }
      else
      {
        /* Print error message */
        orxTextIO_PrintLn("Can't register update function for the clock :(.. Destroy it");
        
        /* Destroy the clock */
        orxClock_Delete(pstClock);
      }
    }
    else
    {
      /* Print error message */
      orxTextIO_PrintLn("Can't Create clock...");
    }
  }
}

orxVOID orxTest_Clock_Delete()
{
  orxS32 s32Address;
  orxTEST_CLOCK_INFO *pstInfos = orxNULL;
  orxCLOCK *pstClock = orxNULL;
  
  if (orxTest_Clock_Depend() == orxSTATUS_SUCCESS)
  {
    orxTextIO_PrintLn("This command allows you to delete a Clock.");
    
    /* Read the clock address */
    orxTextIO_ReadS32(&s32Address, 16, "Clock ID : ", orxTRUE);
  
    /* Cast the address */
    pstClock = (orxCLOCK *)s32Address;
    
    /* Loop on Cell until the clock has been found */
    while ((pstInfos = orxBank_GetNext(sstTest_Clock.pstBank, pstInfos)) &&
           (pstInfos->pstClock != pstClock))
    {}
    
    /* Clock found ? */
    if (pstInfos != orxNULL)
    {
      /* Delete the clock */
      orxTextIO_PrintLn("Delete the clock...");
      orxClock_Delete(pstClock);
      
      /* Unallocate cell from bank */
      orxBank_Free(sstTest_Clock.pstBank, pstInfos);
    }
    else
    {
      /* Not found */
      orxTextIO_PrintLn("Clock ID not found");
    }
  }
}

orxVOID orxTest_Clock_List()
{
  orxTEST_CLOCK_INFO *pstInfos = orxNULL;
    
  if (orxTest_Clock_Depend() == orxSTATUS_SUCCESS)
  {
    orxTextIO_PrintLn("Print list or created clocks : ");
    
    /* Loop on allocated Cells */
    while ((pstInfos = orxBank_GetNext(sstTest_Clock.pstBank, pstInfos)))
    {
      /* Show clock ID */
      orxTextIO_PrintLn("Clock Address = %x", pstInfos->pstClock);
      
      /* Show informations */
      orxTest_Clock_DisplayInfos(orxClock_GetInfo(pstInfos->pstClock));
    }
    
    orxTextIO_PrintLn("Done.");
  }
}

orxVOID orxTest_Clock_Simulate()
{
  orxCLOCK *pstClock;
  
  if (orxTest_Clock_Depend() == orxSTATUS_SUCCESS)
  {
    /* Create a clock of 10 seconds */
    pstClock = orxClock_Create(10000, orxCLOCK_TYPE_USER);

    /* Register the callback */
    orxClock_Register(pstClock, orxTest_Clock_EndLoop, orxNULL);
    
    /* Loop Until Exit received */
    while (!sstTest_Clock.bExit)
    {
      /* Update clocks */
      orxClock_Update();
  
      /* Sleep the program for 1ms (to help the scheduler) */
      orxTime_Delay(1);
    }
    
    /* Unregister the callback */
    orxClock_Unregister(pstClock, orxTest_Clock_EndLoop);
    
    /* Delete the clock */
    orxClock_Delete(pstClock);
    
    /* Reset exit bool */
    sstTest_Clock.bExit = orxFALSE;
  }
}

/******************************************************
 * DYNAMIC LIBRARY ENTRY POINTS
 ******************************************************/
orxVOID orxTest_Clock_Init()
{
  /* Register test functions */
  orxTest_Register("Clock", "Display module informations", orxTest_Clock_Infos);
  orxTest_Register("Clock", "Create a clock", orxTest_Clock_Create);
  orxTest_Register("Clock", "Delete a clock", orxTest_Clock_Delete);
  orxTest_Register("Clock", "Show all clocks informations", orxTest_Clock_List);
  orxTest_Register("Clock", "Simulate the loop engine for a short period", orxTest_Clock_Simulate);
  
  /* Initialize static datas */
  orxMemory_Set(&sstTest_Clock, 0, sizeof(orxTEST_CLOCK_STATIC));
  
  /* Create a bank to store clocks */
  sstTest_Clock.pstBank = orxBank_Create(orxTEST_CLOCK_KU32_MAX_CLOCK, sizeof(orxTEST_CLOCK_INFO), orxBANK_KU32_FLAGS_NONE, orxMEMORY_TYPE_MAIN);
  sstTest_Clock.bExit   = orxFALSE;
  sstTest_Clock.bInit   = orxFALSE;
  sstTest_Clock.u32InitCounter = 0;
}

orxVOID orxTest_Clock_Exit()
{
  orxTEST_CLOCK_INFO *pstInfos = orxNULL;
  
  /* Release bank */
  while ((pstInfos = orxBank_GetNext(sstTest_Clock.pstBank, orxNULL)))
  {
    /* Delete clock */
    orxClock_Delete(pstInfos->pstClock);
    
    /* Unallocate Cell */
    orxBank_Free(sstTest_Clock.pstBank, pstInfos);
  }
  
  /* Destroy bank */
  orxBank_Delete(sstTest_Clock.pstBank);
  
  /* Uninitialize module */
  if (sstTest_Clock.bInit)
  {
    while (sstTest_Clock.u32InitCounter > 0)
    {
      orxDEPEND_EXIT(Clock);
      sstTest_Clock.u32InitCounter--;
    }
  }
}

orxTEST_DEFINE_ENTRY_POINT(orxTest_Clock_Init, orxTest_Clock_Exit)

