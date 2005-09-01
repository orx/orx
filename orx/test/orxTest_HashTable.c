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
#include "io/orxTextIO.h"

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
  orxU32 u32NbUsedHash;                                               /* Number of used hash table */
} orxTEST_HASHTABLE;

orxSTATIC orxTEST_HASHTABLE sstTest_HashTable;

/******************************************************
 * PRIVATE FUNCTIONS
 ******************************************************/

orxVOID orxTest_HashTable_PrintUsedID()
{
  orxU32 u32Index;
  
  orxTextIO_PrintLn("List of used Hash Table ID :");

  /* Tranverse the array and get used ID */
  for (u32Index = 0; u32Index < orxTEST_HASHTABLE_KU32_ARRAY_NB_ELEM; u32Index++)
  {
    if (sstTest_HashTable.apstHashTable[u32Index] != orxNULL)
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
orxVOID orxTest_HashTable_Infos()
{
  orxTextIO_PrintLn("This test module is able to manage %lu HashTables", orxTEST_HASHTABLE_KU32_ARRAY_NB_ELEM);
  orxTextIO_PrintLn("* When you create a Hash Table, you can select an index between 0 and %d to set the Hash Table to use", orxTEST_HASHTABLE_KU32_ARRAY_NB_ELEM - 1);
  orxTextIO_PrintLn("* When you have created a bank, you can add/remove/get new key/value for it");
  orxTextIO_PrintLn("* You can also display the list of stored keys/value in a hash table");
}

/** Create a hash table
 */
orxVOID orxTest_HashTable_Create()
{
  orxS32 s32ID;
  orxS32 s32NbElem;
  
  /* Is it possible to create a Hash Table ? */
  if (sstTest_HashTable.u32NbUsedHash == orxTEST_HASHTABLE_KU32_ARRAY_NB_ELEM)
  {
    orxTextIO_PrintLn("All ID have been used. Delete a hash table before create a new one");
    return;
  }
  
  orxTextIO_PrintLn("This command allows you to create a hash table. To simplify");
  orxTextIO_PrintLn("the test module, you have to give an unused Hash Table ID");
  orxTextIO_PrintLn("between 0 and %d that has not already been used.", orxTEST_HASHTABLE_KU32_ARRAY_NB_ELEM - 1);

  /* All ID available ? */
  if (sstTest_HashTable.u32NbUsedHash > 0)
  {
    /* Display the list of used ID */
    orxTest_HashTable_PrintUsedID();
  }

  /* Get the ID to use */
  orxTextIO_ReadS32InRange(&s32ID, 10, 0, orxTEST_HASHTABLE_KU32_ARRAY_NB_ELEM - 1, "Choose the hast table ID to use : ", orxTRUE);

  /* Already used ID ? */
  if (sstTest_HashTable.apstHashTable[s32ID] != orxNULL)
  {
    orxTextIO_PrintLn("This ID is already used");
  }
  else
  {
    /* Get the number of elements */
    orxTextIO_ReadS32InRange(&s32NbElem, 10, 1, 0x7FFFFFFF, "Number of key to store : ", orxTRUE);

   /* Now, allocate s32NbElem elements in a new hash table at the index position s32ID */
    sstTest_HashTable.apstHashTable[s32ID] = orxHashTable_Create(s32NbElem, orxHASHTABLE_KU32_FLAGS_NONE, orxMEMORY_TYPE_MAIN);
    if (sstTest_HashTable.apstHashTable[s32ID] == orxNULL)
    {
      orxTextIO_PrintLn("Can't create the hash table. Not enough memory ?");
    }
    else
    {
      orxTextIO_PrintLn("New hash table created with %d keys at the ID %d", s32NbElem, s32ID);
    
      /* Increase the counter */
      sstTest_HashTable.u32NbUsedHash++;
    }
  }
}

/** Destroy a hash table
 */
orxVOID orxTest_HashTable_Destroy()
{
  orxS32 s32ID;
  
  /* Are there allocated hash table ? */
  if (sstTest_HashTable.u32NbUsedHash == 0)
  {
    orxTextIO_PrintLn("No hash table have been created. Create a hash table before trying to delete it");
    return;
  }
  
  /* Display the list of allocated hash table */
  orxTest_HashTable_PrintUsedID();
 
  /* Get the ID to use */
  orxTextIO_ReadS32InRange(&s32ID, 10, 0, orxTEST_HASHTABLE_KU32_ARRAY_NB_ELEM - 1, "Choose the hast table ID to use : ", orxTRUE);

  /* not used ID ? */
  if (sstTest_HashTable.apstHashTable[s32ID] == orxNULL)
  {
    orxTextIO_PrintLn("This ID is not used, can't destroy it");
  }
  else
  {
    /* Delete the hash table */
    orxHashTable_Delete(sstTest_HashTable.apstHashTable[s32ID]);
    sstTest_HashTable.apstHashTable[s32ID] = orxNULL;
    
    /* Decrease the counter */
    sstTest_HashTable.u32NbUsedHash--;
  
    /* Done !*/
    orxTextIO_PrintLn("hash table deleted !");
  }
}

/** Add a new pair key/value
 */
orxVOID orxTest_HashTable_Add()
{
  orxS32 s32ID;
  orxS32 s32Key;
  orxS32 s32Data;
  
  /* Are there allocated hash table ? */
  if (sstTest_HashTable.u32NbUsedHash == 0)
  {
    orxTextIO_PrintLn("No hash table have been created. Create a hash table before trying to add datas");
    return;
  }
  
  /* Display the list of hash table */
  orxTest_HashTable_PrintUsedID();

  /* Get the ID to use */
  orxTextIO_ReadS32InRange(&s32ID, 10, 0, orxTEST_HASHTABLE_KU32_ARRAY_NB_ELEM - 1, "Choose the hast table ID to use : ", orxTRUE);

  /* not used ID ? */
  if (sstTest_HashTable.apstHashTable[s32ID] == orxNULL)
  {
    orxTextIO_PrintLn("This ID is not used");
  }
  else
  {
    /* Get key and data to add */
    orxTextIO_ReadS32(&s32Key, 10, "Key to Add : ", orxTRUE);
    orxTextIO_ReadS32(&s32Data, 10, "Data to Add : ", orxTRUE);
    
    orxTextIO_PrintLn("Trying to add the pair key/value (%u/%d)...", (orxU32)s32Key, s32Data);
    if (orxHashTable_Add(sstTest_HashTable.apstHashTable[s32ID], (orxU32)s32Key, (orxVOID *)s32Data) == orxSTATUS_FAILED)
    {
      /* Insertion failed */
      orxTextIO_PrintLn("Insertion failed...");
    }

    orxTextIO_PrintLn("Done !");
  }
}

/** Remove a key from the hash table
 */
orxVOID orxTest_HashTable_Remove()
{
  orxS32 s32ID;
  orxS32 s32Key;
  
  /* Are there allocated hash table ? */
  if (sstTest_HashTable.u32NbUsedHash == 0)
  {
    orxTextIO_PrintLn("No hash table have been created. you can't remove keys");
    return;
  }
  
  /* Display the list of allocated hash table */
  orxTest_HashTable_PrintUsedID();

  /* Get the ID to use */
  orxTextIO_ReadS32InRange(&s32ID, 10, 0, orxTEST_HASHTABLE_KU32_ARRAY_NB_ELEM - 1, "Choose the hast table ID to use : ", orxTRUE);

  /* not used ID ? */
  if (sstTest_HashTable.apstHashTable[s32ID] == orxNULL)
  {
    orxTextIO_PrintLn("This ID is not used");
  }
  else
  {
    /* Get the key to remove */
    orxTextIO_ReadS32(&s32Key, 10, "Key to remove : ", orxTRUE);

    /* Try to remove it */    
    if (orxHashTable_Remove(sstTest_HashTable.apstHashTable[s32ID], (orxU32)s32Key) == orxSTATUS_FAILED)
    {
      /* Failed to remove */
      orxTextIO_PrintLn("Remove failed...");
    }
  }
}

/** Print a value associated to a key
 */
orxVOID orxTest_HashTable_Get()
{
  orxS32 s32ID;
  orxS32 s32Key;
  orxVOID *pData;
  
  /* Are there allocated hash table ? */
  if (sstTest_HashTable.u32NbUsedHash == 0)
  {
    orxTextIO_PrintLn("No hash table have been created. you print key value");
    return;
  }
  
  /* Display the list of allocated hash table */
  orxTest_HashTable_PrintUsedID();

  /* Get the ID to use */
  orxTextIO_ReadS32InRange(&s32ID, 10, 0, orxTEST_HASHTABLE_KU32_ARRAY_NB_ELEM - 1, "Choose the hast table ID to use : ", orxTRUE);

  /* not used ID ? */
  if (sstTest_HashTable.apstHashTable[s32ID] == orxNULL)
  {
    orxTextIO_PrintLn("This ID is not used");
  }
  else
  {
    /* Get the key to remove */
    orxTextIO_ReadS32(&s32Key, 10, "Key to get : ", orxTRUE);

    /* Get data */
    pData = orxHashTable_Get(sstTest_HashTable.apstHashTable[s32ID], (orxU32)s32Key);

    /* Key found ? */  
    if (pData != orxNULL)
    {
      /* Key found, print value */
      orxTextIO_PrintLn("Key found. Associated value : %d", (orxS32)pData);
    }
    else
    {
      /* Key not found */
      orxTextIO_PrintLn("Key not found.");
    }
  }
}

/** Clear all cells from a hash table
 */
orxVOID orxTest_HashTable_Clear()
{
  orxS32 s32ID;
  
  /* Are there allocated hash table ? */
  if (sstTest_HashTable.u32NbUsedHash == 0)
  {
    orxTextIO_PrintLn("No hash table have been created. you can't clear it");
    return;
  }
  
  /* Display the list of allocated bank */
  orxTest_HashTable_PrintUsedID();

  /* Get the ID to use */
  orxTextIO_ReadS32InRange(&s32ID, 10, 0, orxTEST_HASHTABLE_KU32_ARRAY_NB_ELEM - 1, "Choose the hast table ID to use : ", orxTRUE);

  /* not used ID ? */
  if (sstTest_HashTable.apstHashTable[s32ID] == orxNULL)
  {
    orxTextIO_PrintLn("This ID is not used");
  }
  else
  {
    /* Clear the hash table */
    orxTextIO_PrintLn("Clear hash table...");
    orxHashTable_Clear(sstTest_HashTable.apstHashTable[s32ID]);
  }
}

/** Print content of a hash table
 */
orxVOID orxTest_HashTable_PrintAll()
{
  orxS32 s32ID;
  
  /* Are there allocated hash table ? */
  if (sstTest_HashTable.u32NbUsedHash == 0)
  {
    orxTextIO_PrintLn("No hash table have been created. you can't print it");
    return;
  }
  
  /* Display the list of allocated hash table */
  orxTest_HashTable_PrintUsedID();

  /* Get the ID to use */
  orxTextIO_ReadS32InRange(&s32ID, 10, 0, orxTEST_HASHTABLE_KU32_ARRAY_NB_ELEM - 1, "Choose the hast table ID to use : ", orxTRUE);

  /* not used ID ? */
  if (sstTest_HashTable.apstHashTable[s32ID] == orxNULL)
  {
    orxTextIO_PrintLn("This ID is not used");
  }
  else
  {
    /* Display content */
    orxHashTable_DebugPrint(sstTest_HashTable.apstHashTable[s32ID]);
  }
}

/******************************************************
 * DYNAMIC LIBRARY ENTRY POINTS
 ******************************************************/
orxVOID orxTest_HashTable_Init()
{
  /* Initialize bank module */
  orxMAIN_INIT_MODULE(HashTable);
  
  /* Register test functions */
  orxTest_Register("Hash", "Display module informations", orxTest_HashTable_Infos);
  orxTest_Register("Hash", "Create a hash table", orxTest_HashTable_Create);
  orxTest_Register("Hash", "Delete a hash table", orxTest_HashTable_Destroy);
  orxTest_Register("Hash", "Add a key associated with a data in a table", orxTest_HashTable_Add);
  orxTest_Register("Hash", "Remove a pair key/value", orxTest_HashTable_Remove);
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
  orxMAIN_EXIT_MODULE(HashTable);
}

orxTEST_DEFINE_ENTRY_POINT(orxTest_HashTable_Init, orxTest_HashTable_Exit)
