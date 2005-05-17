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

#include "orxInclude.h"
#include "utils/orxTest.h"
#include "memory/orxBank.h"
#include "io/orxTextIO.h"

/******************************************************
 * DEFINES
 ******************************************************/

#define orxTEST_BANK_KU32_ARRAY_NB_ELEM  3
#define orxTEST_BANK_KU32_CELLS_SIZE     32

/******************************************************
 * TYPES AND STRUCTURES
 ******************************************************/

typedef struct __orxTEST_BANK_t
{
  orxBANK *apstBank[orxTEST_BANK_KU32_ARRAY_NB_ELEM];  /* Array of bank */
  orxU32 u32NbUsedBank;                                 /* Number of used bank */
} orxTEST_BANK;

orxSTATIC orxTEST_BANK sstTest_Bank;

/******************************************************
 * PRIVATE FUNCTIONS
 ******************************************************/

orxVOID orxTest_Bank_PrintUsedID()
{
  orxU32 u32Index;
  
  orxTextIO_PrintLn("List of used Bank ID :");

  /* Tranver the array and get used ID */
  for (u32Index = 0; u32Index < orxTEST_BANK_KU32_ARRAY_NB_ELEM; u32Index++)
  {
    if (sstTest_Bank.apstBank[u32Index] != orxNULL)
    {
      orxTextIO_PrintLn("Used ID : %d", u32Index);
    }
  }
}

/******************************************************
 * TEST FUNCTIONS
 ******************************************************/

/** Display informations about this test module
 */
orxVOID orxTest_Bank_Infos()
{
  orxTextIO_PrintLn("This test module is able to manage %ul ChunkBanks", orxTEST_BANK_KU32_ARRAY_NB_ELEM);
  orxTextIO_PrintLn("* When you create a memory bank, you can select an index between 0 and %d to set the Bank to use", orxTEST_BANK_KU32_ARRAY_NB_ELEM - 1);
  orxTextIO_PrintLn("* When you have created a bank, you can add new cells for it");
  orxTextIO_PrintLn("* The function will display the adress associated to this cell for you");
  orxTextIO_PrintLn("* You can also display the list of allocated cells for a complete memory bank");
  orxTextIO_PrintLn("* You can free cells from a memory bank using the returned value at allocation time");
}

/** Create a memory bank
 */
orxVOID orxTest_Bank_Create()
{
  orxS32 s32ID;
  orxS32 s32NbElem;
  
  /* Is it possible to create a new Bank ? */
  if (sstTest_Bank.u32NbUsedBank == orxTEST_BANK_KU32_ARRAY_NB_ELEM)
  {
    orxTextIO_PrintLn("All ID have been used. Delete a bank before create a new one");
    return;
  }
  
  orxTextIO_PrintLn("This command allows you to create a bank. To simplify");
  orxTextIO_PrintLn("the test module, you have to give an unused Bank ID");
  orxTextIO_PrintLn("between 0 and %d that has not already been used.", orxTEST_BANK_KU32_ARRAY_NB_ELEM - 1);

  /* All ID available ? */
  if (sstTest_Bank.u32NbUsedBank > 0)
  {
    /* Display the list of used ID */
    orxTest_Bank_PrintUsedID();
  }
  
  /* We will create a new bank, get the ID from the user */
  orxTextIO_ReadS32InRange(&s32ID, 10, 0, orxTEST_BANK_KU32_ARRAY_NB_ELEM - 1, "Choose an ID for the new Bank : ", orxTRUE);

  /* Already used ID ? */
  if (sstTest_Bank.apstBank[s32ID] != orxNULL)
  {
    orxTextIO_PrintLn("This ID is already used");
  }
  else
  {
    /* Get the number of elements */
    orxTextIO_ReadS32InRange(&s32NbElem, 10, 0, 0x7FFFFFFF, "Choose an ID for the new Bank : ", orxTRUE);

    /* Now, allocate s32NbElem elements in a new bank at the index position s32ID */
    /* We will use an arbitrary size of 1 bytes for each elements */
    sstTest_Bank.apstBank[s32ID] = orxBank_Create(s32NbElem, orxTEST_BANK_KU32_CELLS_SIZE * sizeof(orxU8), orxBANK_KU32_FLAGS_NONE, orxMEMORY_TYPE_MAIN);
    if (sstTest_Bank.apstBank[s32ID] == orxNULL)
    {
      orxTextIO_PrintLn("Can't create the new bank. Not enough memory ?");
    }
    else
    {
      orxTextIO_PrintLn("New bank created with %d elements per segment at the ID %d", s32NbElem, s32ID);
    
      /* Increase the counter */
      sstTest_Bank.u32NbUsedBank++;
    }
  }
}

/** Destroy a memory bank
 */
orxVOID orxTest_Bank_Destroy()
{
  orxS32 s32ID;
  
  /* Are there allocated bank ? */
  if (sstTest_Bank.u32NbUsedBank == 0)
  {
    orxTextIO_PrintLn("No bank have been created. Create a bank before trying to delete it");
    return;
  }
  
  /* Display the list of allocated bank */
  orxTest_Bank_PrintUsedID();
  
  /* Now, get the bank ID to delete */
  orxTextIO_ReadS32InRange(&s32ID, 10, 0, orxTEST_BANK_KU32_ARRAY_NB_ELEM - 1, "Choose an ID for the new Bank : ", orxTRUE);

  /* Already used ID ? */
  if (sstTest_Bank.apstBank[s32ID] == orxNULL)
  {
    orxTextIO_PrintLn("This ID is not used");
  }
  else
  {
    /* Delete the bank */
    orxBank_Delete(sstTest_Bank.apstBank[s32ID]);
    sstTest_Bank.apstBank[s32ID] = orxNULL;

    /* Decrease the counter */
    sstTest_Bank.u32NbUsedBank--;

    /* Done !*/
    orxTextIO_PrintLn("Bank deleted !");
  }
}

/** Allocate a cell in a bank
 */
orxVOID orxTest_Bank_AllocateCell()
{
  orxS32 s32ID;
  orxCHAR *pstCell;
  
  /* Are there allocated bank ? */
  if (sstTest_Bank.u32NbUsedBank == 0)
  {
    orxTextIO_PrintLn("No bank have been created. Create a bank before trying to allocate cells");
    return;
  }
  
  /* Display the list of allocated bank */
  orxTest_Bank_PrintUsedID();
  
  /* Now, get the bank ID to use */
  orxTextIO_ReadS32InRange(&s32ID, 10, 0, orxTEST_BANK_KU32_ARRAY_NB_ELEM - 1, "Bank ID to use (where cell will be allocated) : ", orxTRUE);

  /* Already used ID ? */
  if (sstTest_Bank.apstBank[s32ID] == orxNULL)
  {
    orxTextIO_PrintLn("This ID is not used");
  }
  else
  {
    orxTextIO_PrintLn("Trying to allocate a new cell from the bank...");
    
    /* Allocate the cell */
    pstCell = (orxCHAR *)orxBank_Allocate(sstTest_Bank.apstBank[s32ID]);
  
    /* Correct allocation ? */
    if (pstCell != orxNULL)
    {
      /* Allocation success */
      orxTextIO_PrintLn("Allocation success (Adress : %x). Filling up datas with 0xFF (1024 bytes to set)", pstCell);
    }
    else
    {
      orxTextIO_PrintLn("Allocation failed...");
    }
  }
}

/** Free a cell from a bank
 */
orxVOID orxTest_Bank_FreeCell()
{
  orxS32 s32ID;
  orxS32 s32Address;
  
  /* Are there allocated bank ? */
  if (sstTest_Bank.u32NbUsedBank == 0)
  {
    orxTextIO_PrintLn("No bank have been created. you cant unallocate cells");
    return;
  }
  
  /* Display the list of allocated bank */
  orxTest_Bank_PrintUsedID();
  
  /* Now, get the bank ID to use */
  orxTextIO_ReadS32InRange(&s32ID, 10, 0, orxTEST_BANK_KU32_ARRAY_NB_ELEM - 1, "Bank ID where is stored the cell : ", orxTRUE);

  if (sstTest_Bank.apstBank[s32ID] == orxNULL)
  {
    orxTextIO_PrintLn("This ID is not used");
  }
  else
  {
    /* Now, get the cell address */
    orxTextIO_ReadS32(&s32Address, 16, "Cell Address : ", orxTRUE);
    
    /* Got the adress and bank... Now, try to free the cell */
    orxTextIO_PrintLn("Trying to free the cell (%x)...", s32Address);
    orxBank_Free(sstTest_Bank.apstBank[s32ID], (orxVOID *)s32Address);
  }

  orxTextIO_PrintLn("Done !");
}

/** Clear all cells from a bank
 */
orxVOID orxTest_Bank_Clear()
{
  orxS32 s32ID;
  
  /* Are there allocated bank ? */
  if (sstTest_Bank.u32NbUsedBank == 0)
  {
    orxTextIO_PrintLn("No bank have been created. you can't clear it");
    return;
  }
  
  /* Display the list of allocated bank */
  orxTest_Bank_PrintUsedID();
  
  /* Now, get the bank ID to use */
  orxTextIO_ReadS32InRange(&s32ID, 10, 0, orxTEST_BANK_KU32_ARRAY_NB_ELEM - 1, "Bank ID to clear : ", orxTRUE);

  if (sstTest_Bank.apstBank[s32ID] == orxNULL)
  {
    orxTextIO_PrintLn("This ID is not used");
  }
  else
  {
    orxBank_Clear(sstTest_Bank.apstBank[s32ID]);
  }

  orxTextIO_PrintLn("Done !");
}

/** Print content of a memory bank
 */
orxVOID orxTest_Bank_PrintAll()
{
  orxS32 s32ID;
  
  /* Are there allocated bank ? */
  if (sstTest_Bank.u32NbUsedBank == 0)
  {
    orxTextIO_PrintLn("No bank have been created. you can't print it");
    return;
  }
  
  /* Display the list of allocated bank */
  orxTest_Bank_PrintUsedID();
  
  /* Now, get the bank ID to use */
  orxTextIO_ReadS32InRange(&s32ID, 10, 0, orxTEST_BANK_KU32_ARRAY_NB_ELEM - 1, "Bank ID to display : ", orxTRUE);

  if (sstTest_Bank.apstBank[s32ID] == orxNULL)
  {
    orxTextIO_PrintLn("This ID is not used");
  }
  else
  {
    /* Display content */
    orxBank_DebugPrint(sstTest_Bank.apstBank[s32ID]);
  }

  orxTextIO_PrintLn("Done !");
}

/** Print the adress of all allocated cell from a bank
 */
orxVOID orxTest_Bank_DisplayCells()
{
  orxS32 s32ID;
  orxVOID *pCell;
  
  /* Are there allocated bank ? */
  if (sstTest_Bank.u32NbUsedBank == 0)
  {
    orxTextIO_PrintLn("No bank have been created. you can't print it");
    return;
  }
  
  /* Display the list of allocated bank */
  orxTest_Bank_PrintUsedID();
  
  /* Now, get the bank ID to use */
  orxTextIO_ReadS32InRange(&s32ID, 10, 0, orxTEST_BANK_KU32_ARRAY_NB_ELEM - 1, "Bank ID to display : ", orxTRUE);

  if (sstTest_Bank.apstBank[s32ID] == orxNULL)
  {
    orxTextIO_PrintLn("This ID is not used");
  }
  else
  {
    /* Traverse bank datas and print address */
    pCell = orxNULL;
    while ((pCell = orxBank_GetNext(sstTest_Bank.apstBank[s32ID], pCell)))
    {
      orxTextIO_PrintLn("Cell : %x", pCell);
    }
  }

  /* Done ! */
  orxTextIO_PrintLn("Done !");

  return;
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
  orxTest_Register("Bank", "Clear all cells from a bank", orxTest_Bank_Clear);
  orxTest_Register("Bank", "Print the internal content of a memory bank", orxTest_Bank_PrintAll);
  orxTest_Register("Bank", "Display adress of all allocated cell", orxTest_Bank_DisplayCells);
  
  /* Initialize static datas */
  orxMemory_Set(&sstTest_Bank, 0, sizeof(orxTEST_BANK));
}

orxVOID orxTest_Bank_Exit()
{
  orxU32 u32BankIndex; /* Index of the current bank */
  
  /* Traverse allocated banks and free them */
  for (u32BankIndex = 0; u32BankIndex < orxTEST_BANK_KU32_ARRAY_NB_ELEM; u32BankIndex++)
  {
    /* Bank allocated ? */
    if (sstTest_Bank.apstBank[u32BankIndex] != orxNULL)
    {
      /* Delete it */
      orxBank_Delete(sstTest_Bank.apstBank[u32BankIndex]);
      sstTest_Bank.apstBank[u32BankIndex] = orxNULL;
    }
  }
  
  /* Uninitialize module */
  orxBank_Exit();
}

orxTEST_DEFINE_ENTRY_POINT(orxTest_Bank_Init, orxTest_Bank_Exit)
