/**
 * @file orxTest_Bank.c
 * 
 * Memory bank Test Program
 * 
 */
 
 /***************************************************************************
 orxTest_Bank.c
 Memory bank Test Program
 
 begin                : 10/04/2005
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

#include <stdio.h>

#include "orxInclude.h"
#include "utils/orxTest.h"
#include "memory/orxBank.h"

/******************************************************
 * DEFINES
 ******************************************************/

#define orxTEST_BANK_KU32_ARRAY_NB_ELEM  3

/******************************************************
 * TYPES AND STRUCTURES
 ******************************************************/

orxSTATIC orxBANK *sapstBank[orxTEST_BANK_KU32_ARRAY_NB_ELEM];

/******************************************************
 * TEST FUNCTIONS
 ******************************************************/

/** Display informations about this test module
 */
orxVOID orxTest_Bank_Infos()
{
  printf("This test module is able to manage %ul ChunkBanks\n", orxTEST_BANK_KU32_ARRAY_NB_ELEM);
  printf("* When you create a memory bank, you can select an index between 0 and %d to set the Bank to use\n", orxTEST_BANK_KU32_ARRAY_NB_ELEM - 1);
  printf("* When you have created a bank, you can add new cells for it\n");
  printf("* The function will display the adress associated to this cell for you\n");
  printf("* You can also display the list of allocated cells for a complete memory bank\n");
  printf("* You can free cells from a memory bank using the returned value at allocation time\n");
}

/** Create a memory bank
 */
orxVOID orxTest_Bank_Create()
{
  printf("Create a memory bank\n");
}

/** Destroy a memory bank
 */
orxVOID orxTest_Bank_Destroy()
{
  printf("Destroy a memory bank\n");
}

/** Allocate a cell in a bank
 */
orxVOID orxTest_Bank_AllocateCell()
{
  printf("Allocate a new cell in a memory bank\n");
}

/** Free a cell from a bank
 */
orxVOID orxTest_Bank_FreeCell()
{
  printf("Free a cell from a memory bank\n");
}

/** Print content of a memory bank
 */
orxVOID orxTest_Bank_PrintAll()
{
  printf("Display the internal content of a memory bank\n");
}


/******************************************************
 * DYNAMIC LIBRARY ENTRY POINTS
 ******************************************************/
orxVOID orxTest_Bank_Init()
{
  /* Initialize bank module */
  orxBank_Init();
  
  /* Register test functions */
  orxTest_Register("Bank", "Display module informations", orxTest_Bank_Infos);
  orxTest_Register("Bank", "Create a bank of memory", orxTest_Bank_Create);
  orxTest_Register("Bank", "Destroy a bank of memory", orxTest_Bank_Destroy);
  orxTest_Register("Bank", "Allocate a new cell in a bank", orxTest_Bank_AllocateCell);
  orxTest_Register("Bank", "Free a cell from a bank", orxTest_Bank_FreeCell);
  orxTest_Register("Bank", "Print the internal content of a memory bank", orxTest_Bank_PrintAll);
  
  /* Initialize static datas */
  orxMemory_Set(sapstBank, 0, sizeof(sapstBank));
}

orxVOID orxTest_Bank_Exit()
{
  orxU32 u32BankIndex; /* Index of the current bank */
  
  /* Traverse allocated banks and free them */
  for (u32BankIndex = 0; u32BankIndex < orxTEST_BANK_KU32_ARRAY_NB_ELEM; u32BankIndex++)
  {
    /* Bank allocated ? */
    if (sapstBank[u32BankIndex] != orxNULL)
    {
      /* Delete it */
      orxBank_Delete(sapstBank[u32BankIndex]);
      sapstBank[u32BankIndex] = orxNULL;
    }
  }
  
  /* Uninitialize module */
  orxBank_Exit();
}

orxTEST_DEFINE_ENTRY_POINT(orxTest_Bank_Init, orxTest_Bank_Exit)
