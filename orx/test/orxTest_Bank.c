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
#include "utils/orxString.h"

/******************************************************
 * DEFINES
 ******************************************************/

#define orxTEST_BANK_KU32_ARRAY_NB_ELEM  3

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
  
  orxString_PrintLn("List of used Bank ID :");

  /* Tranver the array and get used ID */
  for (u32Index = 0; u32Index < orxTEST_BANK_KU32_ARRAY_NB_ELEM; u32Index++)
  {
    if (sstTest_Bank.apstBank[u32Index] != orxNULL)
    {
      orxString_PrintLn("Used ID : %d", u32Index);
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
  orxString_PrintLn("This test module is able to manage %ul ChunkBanks", orxTEST_BANK_KU32_ARRAY_NB_ELEM);
  orxString_PrintLn("* When you create a memory bank, you can select an index between 0 and %d to set the Bank to use", orxTEST_BANK_KU32_ARRAY_NB_ELEM - 1);
  orxString_PrintLn("* When you have created a bank, you can add new cells for it");
  orxString_PrintLn("* The function will display the adress associated to this cell for you");
  orxString_PrintLn("* You can also display the list of allocated cells for a complete memory bank");
  orxString_PrintLn("* You can free cells from a memory bank using the returned value at allocation time");
}

/** Create a memory bank
 */
orxVOID orxTest_Bank_Create()
{
  orxCHAR zUserValue[64];     /* String where user inputs are stored */
  orxS32 s32ID;
  orxS32 s32NbElem;
  orxBOOL bValidValue;
  
  /* Is it possible to create a new Bank ? */
  if (sstTest_Bank.u32NbUsedBank == orxTEST_BANK_KU32_ARRAY_NB_ELEM)
  {
    orxString_PrintLn("All ID have been used. Delete a bank before create a new one");
    return;
  }
  
  orxString_PrintLn("This command allows you to create a bank. To simplify");
  orxString_PrintLn("the test module, you have to give an unused Bank ID");
  orxString_PrintLn("between 0 and %d that has not already been used.", orxTEST_BANK_KU32_ARRAY_NB_ELEM - 1);

  /* All ID available ? */
  if (sstTest_Bank.u32NbUsedBank > 0)
  {
    /* Display the list of used ID */
    orxTest_Bank_PrintUsedID();
  }
  
  bValidValue = orxFALSE;
  
  /* We will create a new bank, get the ID from the user */
  do
  {
    /* Read string value */
    orxString_ReadString(zUserValue, 63, "Choose an ID for the new Bank : ");
  
    /* Convert it to a number */
    if (orxString_ToS32(&s32ID, zUserValue, 10) != orxSTATUS_FAILED)
    {
      if (s32ID >= 0 && s32ID < orxTEST_BANK_KU32_ARRAY_NB_ELEM)
      {
        if (sstTest_Bank.apstBank[s32ID] == orxNULL)
        {
          bValidValue = orxTRUE;
        }
        else
        {
          orxString_PrintLn("This ID is already used");
        }
      }
      else
      {
        orxString_PrintLn("This ID is out of range");
      }
    }
    else
    {
      orxString_PrintLn("This value is not a valid ID");
    }
  } while (!bValidValue);
  
  bValidValue = orxFALSE;
  
  /* Now, get the number of elements to allocate per segment */
  do
  {
    /* Read string value */
    orxString_ReadString(zUserValue, 63, "Number of elements to store per segments : ");
    
    /* Convert it to a number */
    if (orxString_ToS32(&s32NbElem, zUserValue, 10) != orxSTATUS_FAILED)
    {
      if (s32NbElem > 0)
      {
        bValidValue = orxTRUE;
      }
      else
      {
        orxString_PrintLn("Incorrect number of elements (must be > 0)");
      }
    }
    else
    {
      orxString_PrintLn("This is not a valid number");
    }
  } while (!bValidValue);
    
  /* Now, allocate s32NbElem elements in a new bank at the index position s32ID */
  /* We will use an arbitrary size of 1024 bytes for each elements */
  sstTest_Bank.apstBank[s32ID] = orxBank_Create(s32NbElem, 1024 * sizeof(orxU8), 0, orxMEMORY_TYPE_MAIN);
  if (sstTest_Bank.apstBank[s32ID] == orxNULL)
  {
    orxString_PrintLn("Can't create the new bank. Not enough memory ?");
  }
  else
  {
    orxString_PrintLn("New bank created with %d elements per segment at the ID %d", s32NbElem, s32ID);
    
    /* Increase the counter */
    sstTest_Bank.u32NbUsedBank++;
  }
}

/** Destroy a memory bank
 */
orxVOID orxTest_Bank_Destroy()
{
  orxCHAR zUserValue[64];     /* String where user inputs are stored */
  orxS32 s32ID;
  orxBOOL bValidValue;
  
  /* Are there allocated bank ? */
  if (sstTest_Bank.u32NbUsedBank == 0)
  {
    orxString_PrintLn("No bank have been created. Create a bank before trying to delete it");
    return;
  }
  
  /* Display the list of allocated bank */
  orxTest_Bank_PrintUsedID();
  
  /* Now, get the bank ID to delete */
  bValidValue = orxFALSE;
  do
  {
    /* Read string value */
    orxString_ReadString(zUserValue, 63, "Bank ID to delete : ");
    
    /* Convert it to a number */
    if (orxString_ToS32(&s32ID, zUserValue, 10) != orxSTATUS_FAILED)
    {
      if (((s32ID >= 0) &&
          (s32ID < orxTEST_BANK_KU32_ARRAY_NB_ELEM)) &&
          (sstTest_Bank.apstBank[s32ID] != orxNULL))
      {
        bValidValue = orxTRUE;
      }
      else
      {
        orxString_PrintLn("Incorrect number (not in a valid ID range)");
      }
    }
    else
    {
      orxString_PrintLn("This is not a valid number");
    }
    
  } while (!bValidValue);
  orxString_PrintLn("Destroy a memory bank");
  
  /* Delete the bank */
  orxString_PrintLn("Delete bank %d", s32ID);
  orxBank_Delete(sstTest_Bank.apstBank[s32ID]);
  sstTest_Bank.apstBank[s32ID] = orxNULL;
  
  /* Done !*/
  orxString_PrintLn("Bank deleted !");
}

/** Allocate a cell in a bank
 */
orxVOID orxTest_Bank_AllocateCell()
{
  orxCHAR zUserValue[64];     /* String where user inputs are stored */
  orxS32 s32ID;
  orxBOOL bValidValue;
  orxCHAR *pstCell;
  
  
  /* Are there allocated bank ? */
  if (sstTest_Bank.u32NbUsedBank == 0)
  {
    orxString_PrintLn("No bank have been created. Create a bank before trying to allocate cells");
    return;
  }
  
  /* Display the list of allocated bank */
  orxTest_Bank_PrintUsedID();
  
  /* Now, get the bank ID to use */
  bValidValue = orxFALSE;
  do
  {
    /* Read string value */
    orxString_ReadString(zUserValue, 63, "Bank ID to use (where cell will be allocated) : ");
    
    /* Convert it to a number */
    if (orxString_ToS32(&s32ID, zUserValue, 10) != orxSTATUS_FAILED)
    {
      if (((s32ID >= 0) &&
          (s32ID < orxTEST_BANK_KU32_ARRAY_NB_ELEM)) &&
          (sstTest_Bank.apstBank[s32ID] != orxNULL))
      {
        bValidValue = orxTRUE;
      }
      else
      {
        orxString_PrintLn("Incorrect number (not in a valid ID range)");
      }
    }
    else
    {
      orxString_PrintLn("This is not a valid number");
    }
  } while (!bValidValue);
  
  orxString_PrintLn("Trying to allocate a new cell from the bank...");

  /* Allocate the cell */
  pstCell = (orxCHAR *)orxBank_Allocate(sstTest_Bank.apstBank[s32ID]);
  
  /* Correct allocation ? */
  if (pstCell != orxNULL)
  {
    /* Allocation success */
    orxString_PrintLn("Allocation success (Adress : %x). Filling up datas with 0xFF (1024 bytes to set)", pstCell);
//    orxMemory_Set(pstCell, 0xFF, 1024 * sizeof(orxU8));
    orxString_Printf(pstCell, "Hello :)");
  }
  else
  {
    orxString_PrintLn("Allocation failed...");
  }
}

/** Free a cell from a bank
 */
orxVOID orxTest_Bank_FreeCell()
{
  orxCHAR zUserValue[64];     /* String where user inputs are stored */
  orxS32 s32ID;
  orxS32 s32Address;
  orxBOOL bValidValue;
  
  
  /* Are there allocated bank ? */
  if (sstTest_Bank.u32NbUsedBank == 0)
  {
    orxString_PrintLn("No bank have been created. you cant unallocate cells");
    return;
  }
  
  /* Display the list of allocated bank */
  orxTest_Bank_PrintUsedID();
  
  /* Now, get the bank ID to use */
  bValidValue = orxFALSE;
  do
  {
    /* Read string value */
    orxString_ReadString(zUserValue, 63, "Bank ID where is stored the cell : ");
    
    /* Convert it to a number */
    if (orxString_ToS32(&s32ID, zUserValue, 10) != orxSTATUS_FAILED)
    {
      if (((s32ID >= 0) &&
          (s32ID < orxTEST_BANK_KU32_ARRAY_NB_ELEM)) &&
          (sstTest_Bank.apstBank[s32ID] != orxNULL))
      {
        bValidValue = orxTRUE;
      }
      else
      {
        orxString_PrintLn("Incorrect number (not in a valid ID range)");
      }
    }
    else
    {
      orxString_PrintLn("This is not a valid number");
    }
  } while (!bValidValue);
  
  /* Now, get the cell address */
  bValidValue = orxFALSE;
  do
  {
    /* Read string value */
    orxString_ReadString(zUserValue, 63, "cell adress : ");
    
    /* Convert it to a number */
    if (orxString_ToS32(&s32Address, zUserValue, 16) != orxSTATUS_FAILED)
    {
      bValidValue = orxTRUE;
    }
    else
    {
      orxString_PrintLn("This is not a valid adress");
    }
  } while (!bValidValue);

  /* Got the adress and bank... Now, try to free the cell */
  orxString_PrintLn("Trying to free the cell (%x)...", s32Address);
  orxBank_Free(sstTest_Bank.apstBank[s32ID], (orxVOID *)s32Address);
  orxString_PrintLn("Done !");
}

/** Print content of a memory bank
 */
orxVOID orxTest_Bank_PrintAll()
{
  orxCHAR zUserValue[64];     /* String where user inputs are stored */
  orxS32 s32ID;
  orxBOOL bValidValue;
  
  
  /* Are there allocated bank ? */
  if (sstTest_Bank.u32NbUsedBank == 0)
  {
    orxString_PrintLn("No bank have been created. you can't print it");
    return;
  }
  
  /* Display the list of allocated bank */
  orxTest_Bank_PrintUsedID();
  
  /* Now, get the bank ID to use */
  bValidValue = orxFALSE;
  do
  {
    /* Read string value */
    orxString_ReadString(zUserValue, 63, "Bank ID to display : ");
    
    /* Convert it to a number */
    if (orxString_ToS32(&s32ID, zUserValue, 10) != orxSTATUS_FAILED)
    {
      if (((s32ID >= 0) &&
          (s32ID < orxTEST_BANK_KU32_ARRAY_NB_ELEM)) &&
          (sstTest_Bank.apstBank[s32ID] != orxNULL))
      {
        bValidValue = orxTRUE;
      }
      else
      {
        orxString_PrintLn("Incorrect number (not in a valid ID range)");
      }
    }
    else
    {
      orxString_PrintLn("This is not a valid number");
    }
  } while (!bValidValue);
  
  /* Display content */
  orxBank_DebugPrint(sstTest_Bank.apstBank[s32ID]);
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
