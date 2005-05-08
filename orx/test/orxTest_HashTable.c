/**
 * @file orxTest_HashTable.c
 * 
 * Hash table Test Program
 * 
 */
 
 /***************************************************************************
 orxTest_HashTable.c
 Hash table Test Program
 
 begin                : 08/05/2005
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
#include "utils/orxHashTable.h"
#include "utils/orxString.h"

/******************************************************
 * DEFINES
 ******************************************************/

#define orxTEST_HASHTABLE_KU32_ARRAY_NB_ELEM  3

/******************************************************
 * TYPES AND STRUCTURES
 ******************************************************/

typedef struct __orxTEST_HASHTABLE_t
{
  orxHASHTABLE *apstHashTable[orxTEST_HASHTABLE_KU32_ARRAY_NB_ELEM];  /* Array of hash table */
  orxU32 u32NbUsedHash;                                         /* Number of used hash table */
} orxTEST_HASHTABLE;

orxSTATIC orxTEST_HASHTABLE sstTest_HashTable;

/******************************************************
 * PRIVATE FUNCTIONS
 ******************************************************/

orxVOID orxTest_HashTable_PrintUsedID()
{
  orxU32 u32Index;
  
  orxString_PrintLn("List of used Hash Table ID :");

  /* Tranverse the array and get used ID */
  for (u32Index = 0; u32Index < orxTEST_HASHTABLE_KU32_ARRAY_NB_ELEM; u32Index++)
  {
    if (sstTest_HashTable.apstHashTable[u32Index] != orxNULL)
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
orxVOID orxTest_HashTable_Infos()
{
  orxString_PrintLn("This test module is able to manage %ul HashTables", orxTEST_HASHTABLE_KU32_ARRAY_NB_ELEM);
  orxString_PrintLn("* When you create a Hash Table, you can select an index between 0 and %d to set the Hash Table to use", orxTEST_HASHTABLE_KU32_ARRAY_NB_ELEM - 1);
  orxString_PrintLn("* When you have created a bank, you can add/remove/get new key/value for it");
  orxString_PrintLn("* You can also display the list of stored keys/value in a hash table");
}

/** Create a hash table
 */
orxVOID orxTest_HashTable_Create()
{
  orxCHAR zUserValue[64];     /* String where user inputs are stored */
  orxS32 s32ID;
  orxS32 s32NbElem;
  orxBOOL bValidValue;
  
  /* Is it possible to create a Hash Table ? */
  if (sstTest_HashTable.u32NbUsedHash == orxTEST_HASHTABLE_KU32_ARRAY_NB_ELEM)
  {
    orxString_PrintLn("All ID have been used. Delete a hash table before create a new one");
    return;
  }
  
  orxString_PrintLn("This command allows you to create a hash table. To simplify");
  orxString_PrintLn("the test module, you have to give an unused Hash Table ID");
  orxString_PrintLn("between 0 and %d that has not already been used.", orxTEST_HASHTABLE_KU32_ARRAY_NB_ELEM - 1);

  /* All ID available ? */
  if (sstTest_HashTable.u32NbUsedHash > 0)
  {
    /* Display the list of used ID */
    orxTest_HashTable_PrintUsedID();
  }
  
  bValidValue = orxFALSE;
  
  /* We will create a new hash table, get the ID from the user */
  do
  {
    /* Read string value */
    orxString_ReadString(zUserValue, 63, "Choose an ID for the Hash Table : ");
  
    /* Convert it to a number */
    if (orxString_ToS32(&s32ID, zUserValue, 10) != orxSTATUS_FAILED)
    {
      if (s32ID >= 0 && s32ID < orxTEST_HASHTABLE_KU32_ARRAY_NB_ELEM)
      {
        if (sstTest_HashTable.apstHashTable[s32ID] == orxNULL)
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
  
  /* Now, get the number of key to use */
  do
  {
    /* Read string value */
    orxString_ReadString(zUserValue, 63, "Number of key to use : ");
    
    /* Convert it to a number */
    if (orxString_ToS32(&s32NbElem, zUserValue, 10) != orxSTATUS_FAILED)
    {
      if (s32NbElem > 0)
      {
        bValidValue = orxTRUE;
      }
      else
      {
        orxString_PrintLn("Incorrect number of key (must be > 0)");
      }
    }
    else
    {
      orxString_PrintLn("This is not a valid number");
    }
  } while (!bValidValue);
    
  /* Now, allocate s32NbElem elements in a new hash table at the index position s32ID */
  sstTest_HashTable.apstHashTable[s32ID] = orxHashTable_Create(s32NbElem, 0, orxMEMORY_TYPE_MAIN);
  if (sstTest_HashTable.apstHashTable[s32ID] == orxNULL)
  {
    orxString_PrintLn("Can't create the hash table. Not enough memory ?");
  }
  else
  {
    orxString_PrintLn("New bank created with %d keys at the ID %d", s32NbElem, s32ID);
    
    /* Increase the counter */
    sstTest_HashTable.u32NbUsedHash++;
  }
}

/** Destroy a hash table
 */
orxVOID orxTest_HashTable_Destroy()
{
  orxCHAR zUserValue[64];     /* String where user inputs are stored */
  orxS32 s32ID;
  orxBOOL bValidValue;
  
  /* Are there allocated bank ? */
  if (sstTest_HashTable.u32NbUsedHash == 0)
  {
    orxString_PrintLn("No hash table have been created. Create a hash table before trying to delete it");
    return;
  }
  
  /* Display the list of allocated hash table */
  orxTest_HashTable_PrintUsedID();
  
  /* Now, get the hash table ID to delete */
  bValidValue = orxFALSE;
  do
  {
    /* Read string value */
    orxString_ReadString(zUserValue, 63, "Hash table ID to delete : ");
    
    /* Convert it to a number */
    if (orxString_ToS32(&s32ID, zUserValue, 10) != orxSTATUS_FAILED)
    {
      if (((s32ID >= 0) &&
          (s32ID < orxTEST_HASHTABLE_KU32_ARRAY_NB_ELEM)) &&
          (sstTest_HashTable.apstHashTable[s32ID] != orxNULL))
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
  orxString_PrintLn("Destroy a hash table");
  
  /* Delete the hash table */
  orxString_PrintLn("Delete hash table %d", s32ID);
  orxHashTable_Delete(sstTest_HashTable.apstHashTable[s32ID]);
  sstTest_HashTable.apstHashTable[s32ID] = orxNULL;
  
  /* Done !*/
  orxString_PrintLn("hash table deleted !");
}

/** Add a new pair key/value
 */
orxVOID orxTest_HashTable_Add()
{
  orxCHAR zUserValue[64];     /* String where user inputs are stored */
  orxS32 s32ID;
  orxBOOL bValidValue;
  orxS32 s32Key;
  orxS32 s32Data;
  
  /* Are there allocated hash table ? */
  if (sstTest_HashTable.u32NbUsedHash == 0)
  {
    orxString_PrintLn("No hash table have been created. Create a hash table before trying to add datas");
    return;
  }
  
  /* Display the list of hash table */
  orxTest_HashTable_PrintUsedID();
  
  /* Now, get the hash table ID to use */
  bValidValue = orxFALSE;
  do
  {
    /* Read string value */
    orxString_ReadString(zUserValue, 63, "Hash table ID to use (where key/data will be allocated) : ");
    
    /* Convert it to a number */
    if (orxString_ToS32(&s32ID, zUserValue, 10) != orxSTATUS_FAILED)
    {
      if (((s32ID >= 0) &&
          (s32ID < orxTEST_HASHTABLE_KU32_ARRAY_NB_ELEM)) &&
          (sstTest_HashTable.apstHashTable[s32ID] != orxNULL))
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
  
  /* Now, get the key to store */
  bValidValue = orxFALSE;
  do
  {
    /* Read string value */
    orxString_ReadString(zUserValue, 63, "key to use : ");
    
    /* Convert it to a number */
    if (orxString_ToS32(&s32Key, zUserValue, 10) != orxSTATUS_FAILED)
    {
      bValidValue = orxTRUE;
    }
    else
    {
      orxString_PrintLn("This is not a valid key");
    }
  } while (!bValidValue);
  
  /* Now, get the associated value */
  bValidValue = orxFALSE;
  do
  {
    /* Read string value */
    orxString_ReadString(zUserValue, 63, "associated value data : ");
    
    /* Convert it to a number */
    if (orxString_ToS32(&s32Data, zUserValue, 10) != orxSTATUS_FAILED)
    {
      bValidValue = orxTRUE;
    }
    else
    {
      orxString_PrintLn("This is not a valid value");
    }
  } while (!bValidValue);
  
  /* Got the key/value... Now, try to add it in the hash table */
  orxString_PrintLn("Trying to add the pair key/value (%u/%d)...", (orxU32)s32Key, s32Data);
  if (orxHashTable_Add(sstTest_HashTable.apstHashTable[s32ID], (orxU32)s32Key, (orxVOID *)s32Data) == orxSTATUS_FAILED)
  {
    /* Insetion failed */
    orxString_PrintLn("Insertion failed...");
  }
  orxString_PrintLn("Done !");
}

/** Remove a key from the hash table
 */
orxVOID orxTest_HashTable_Remove()
{
  orxCHAR zUserValue[64];     /* String where user inputs are stored */
  orxS32 s32ID;
  orxS32 s32Key;
  orxBOOL bValidValue;
  
  /* Are there allocated hash table ? */
  if (sstTest_HashTable.u32NbUsedHash == 0)
  {
    orxString_PrintLn("No hash table have been created. you can't remove keys");
    return;
  }
  
  /* Display the list of allocated hash table */
  orxTest_HashTable_PrintUsedID();
  
  /* Now, get the hash table ID to use */
  bValidValue = orxFALSE;
  do
  {
    /* Read string value */
    orxString_ReadString(zUserValue, 63, "Hash table ID where are stored keys : ");
    
    /* Convert it to a number */
    if (orxString_ToS32(&s32ID, zUserValue, 10) != orxSTATUS_FAILED)
    {
      if (((s32ID >= 0) &&
          (s32ID < orxTEST_HASHTABLE_KU32_ARRAY_NB_ELEM)) &&
          (sstTest_HashTable.apstHashTable[s32ID] != orxNULL))
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
  
  /* Now, get the key value */
  bValidValue = orxFALSE;
  do
  {
    /* Read string value */
    orxString_ReadString(zUserValue, 63, "key value : ");
    
    /* Convert it to a number */
    if (orxString_ToS32(&s32Key, zUserValue, 10) != orxSTATUS_FAILED)
    {
      bValidValue = orxTRUE;
    }
    else
    {
      orxString_PrintLn("This is not a valid key");
    }
  } while (!bValidValue);

  /* Got the ID and key... Now, try to remove it */
  orxString_PrintLn("Trying to remove key (%d)...", s32Key);
  if (orxHashTable_Remove(sstTest_HashTable.apstHashTable[s32ID], (orxU32)s32Key) == orxSTATUS_FAILED)
  {
    /* Failed to remove */
    orxString_PrintLn("Remove failed...");
  }
  orxString_PrintLn("Done !");
}

/** Print a value associated to a key
 */
orxVOID orxTest_HashTable_Get()
{
  orxCHAR zUserValue[64];     /* String where user inputs are stored */
  orxS32 s32ID;
  orxS32 s32Key;
  orxVOID *pData;
  orxBOOL bValidValue;
  
  /* Are there allocated hash table ? */
  if (sstTest_HashTable.u32NbUsedHash == 0)
  {
    orxString_PrintLn("No hash table have been created. you print key value");
    return;
  }
  
  /* Display the list of allocated hash table */
  orxTest_HashTable_PrintUsedID();
  
  /* Now, get the hash table ID to use */
  bValidValue = orxFALSE;
  do
  {
    /* Read string value */
    orxString_ReadString(zUserValue, 63, "Hash table ID where are stored keys : ");
    
    /* Convert it to a number */
    if (orxString_ToS32(&s32ID, zUserValue, 10) != orxSTATUS_FAILED)
    {
      if (((s32ID >= 0) &&
          (s32ID < orxTEST_HASHTABLE_KU32_ARRAY_NB_ELEM)) &&
          (sstTest_HashTable.apstHashTable[s32ID] != orxNULL))
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
  
  /* Now, get the key value */
  bValidValue = orxFALSE;
  do
  {
    /* Read string value */
    orxString_ReadString(zUserValue, 63, "key value : ");
    
    /* Convert it to a number */
    if (orxString_ToS32(&s32Key, zUserValue, 10) != orxSTATUS_FAILED)
    {
      bValidValue = orxTRUE;
    }
    else
    {
      orxString_PrintLn("This is not a valid key");
    }
  } while (!bValidValue);

  /* Got the ID and key... Now, try to get value from it */
  orxString_PrintLn("Trying to get key value (%d)...", s32Key);
  pData = orxHashTable_Get(sstTest_HashTable.apstHashTable[s32ID], (orxU32)s32Key);

  /* Key found ? */  
  if (pData != orxNULL)
  {
    /* Key found, print value */
    orxString_PrintLn("Key found. Associated value : %d", (orxS32)pData);
  }
  else
  {
    /* Key not found */
    orxString_PrintLn("Key not found.");
  }

  orxString_PrintLn("Done !");
}

/** Clear all cells from a hash table
 */
orxVOID orxTest_HashTable_Clear()
{
  orxCHAR zUserValue[64];     /* String where user inputs are stored */
  orxS32 s32ID;
  orxBOOL bValidValue;
  
  /* Are there allocated hash table ? */
  if (sstTest_HashTable.u32NbUsedHash == 0)
  {
    orxString_PrintLn("No hash table have been created. you can't clear it");
    return;
  }
  
  /* Display the list of allocated bank */
  orxTest_HashTable_PrintUsedID();
  
  /* Now, get the bank ID to use */
  bValidValue = orxFALSE;
  do
  {
    /* Read string value */
    orxString_ReadString(zUserValue, 63, "Hahs Table ID to clear : ");
    
    /* Convert it to a number */
    if (orxString_ToS32(&s32ID, zUserValue, 10) != orxSTATUS_FAILED)
    {
      if (((s32ID >= 0) &&
          (s32ID < orxTEST_HASHTABLE_KU32_ARRAY_NB_ELEM)) &&
          (sstTest_HashTable.apstHashTable[s32ID] != orxNULL))
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
  
  /* Clear the hash table */
  orxString_PrintLn("Trying to clear hash table...");
  if (orxHashTable_Clear(sstTest_HashTable.apstHashTable[s32ID]) == orxSTATUS_FAILED)
  {
    /* Failed to clear hash */
    orxString_PrintLn("Clear failed...");
  }
  orxString_PrintLn("Done !");
}

/** Print content of a hash table
 */
orxVOID orxTest_HashTable_PrintAll()
{
  orxCHAR zUserValue[64];     /* String where user inputs are stored */
  orxS32 s32ID;
  orxBOOL bValidValue;
  
  /* Are there allocated hash table ? */
  if (sstTest_HashTable.u32NbUsedHash == 0)
  {
    orxString_PrintLn("No hash table have been created. you can't print it");
    return;
  }
  
  /* Display the list of allocated hash table */
  orxTest_HashTable_PrintUsedID();
  
  /* Now, get the bank ID to use */
  bValidValue = orxFALSE;
  do
  {
    /* Read string value */
    orxString_ReadString(zUserValue, 63, "Hash table ID to display : ");
    
    /* Convert it to a number */
    if (orxString_ToS32(&s32ID, zUserValue, 10) != orxSTATUS_FAILED)
    {
      if (((s32ID >= 0) &&
          (s32ID < orxTEST_HASHTABLE_KU32_ARRAY_NB_ELEM)) &&
          (sstTest_HashTable.apstHashTable[s32ID] != orxNULL))
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
  orxHashTable_DebugPrint(sstTest_HashTable.apstHashTable[s32ID]);
}

/******************************************************
 * DYNAMIC LIBRARY ENTRY POINTS
 ******************************************************/
orxVOID orxTest_HashTable_Init()
{
  /* Initialize bank module */
  orxHashTable_Init();
  
  /* Register test functions */
  orxTest_Register("Hash", "Display module informations", orxTest_HashTable_Infos);
  orxTest_Register("Hash", "Create a hash table", orxTest_HashTable_Create);
  orxTest_Register("Hash", "Delete a hash table", orxTest_HashTable_Destroy);
  orxTest_Register("Hash", "Add a key associated with a data in a table", orxTest_HashTable_Add);
  orxTest_Register("Hash", "Remove a pai key/vlaue", orxTest_HashTable_Remove);
  orxTest_Register("Hash", "Print the value associated to a key in a hash table", orxTest_HashTable_Get);
  orxTest_Register("Hash", "Remove all stored keys/value from a hash table", orxTest_HashTable_Clear);
  orxTest_Register("Hash", "Print the internal content of a hash table", orxTest_HashTable_PrintAll);
  
  /* Initialize static datas */
  orxMemory_Set(&sstTest_HashTable, 0, sizeof(orxTEST_HASHTABLE));
}

orxVOID orxTest_HashTable_Exit()
{
  orxU32 u32HashTableIndex; /* Index of the current hash table */
  
  /* Traverse allocated hash table and free them */
  for (u32HashTableIndex = 0; u32HashTableIndex < orxTEST_HASHTABLE_KU32_ARRAY_NB_ELEM; u32HashTableIndex++)
  {
    /* hash table allocated ? */
    if (sstTest_HashTable.apstHashTable[u32HashTableIndex] != orxNULL)
    {
      /* Delete it */
      orxHashTable_Delete(sstTest_HashTable.apstHashTable[u32HashTableIndex]);
      sstTest_HashTable.apstHashTable[u32HashTableIndex] = orxNULL;
    }
  }
  
  /* Uninitialize module */
  orxHashTable_Exit();
}

orxTEST_DEFINE_ENTRY_POINT(orxTest_HashTable_Init, orxTest_HashTable_Exit)
