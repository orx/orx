/**
 * @file orxTest_Hash.c
 * 
 * Hash map Test Program
 * 
 */
 
 /***************************************************************************
 orxTest_Hash.c
 Hash map Test Program
 
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
#include "utils/orxHashMap.h"
#include "utils/orxString.h"

/******************************************************
 * DEFINES
 ******************************************************/

#define orxTEST_HASHMAP_KU32_ARRAY_NB_ELEM  3

/******************************************************
 * TYPES AND STRUCTURES
 ******************************************************/

typedef struct __orxTEST_HASHMAP_t
{
  orxHASHMAP *apstHashMap[orxTEST_HASHMAP_KU32_ARRAY_NB_ELEM];  /* Array of hash map */
  orxU32 u32NbUsedHash;                                         /* Number of used hash map */
} orxTEST_HASHMAP;

orxSTATIC orxTEST_HASHMAP sstTest_HashMap;

/******************************************************
 * PRIVATE FUNCTIONS
 ******************************************************/

orxVOID orxTest_HashMap_PrintUsedID()
{
  orxU32 u32Index;
  
  orxString_PrintLn("List of used Hash Map ID :");

  /* Tranverse the array and get used ID */
  for (u32Index = 0; u32Index < orxTEST_HASHMAP_KU32_ARRAY_NB_ELEM; u32Index++)
  {
    if (sstTest_HashMap.apstHashMap[u32Index] != orxNULL)
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
orxVOID orxTest_HashMap_Infos()
{
  orxString_PrintLn("This test module is able to manage %ul HashMaps", orxTEST_HASHMAP_KU32_ARRAY_NB_ELEM);
  orxString_PrintLn("* When you create a Hash Map, you can select an index between 0 and %d to set the Hash Map to use", orxTEST_HASHMAP_KU32_ARRAY_NB_ELEM - 1);
  orxString_PrintLn("* When you have created a bank, you can add/remove/get new key/value for it");
  orxString_PrintLn("* You can also display the list of stored keys/value in a hash map");
}

/** Create a hash map
 */
orxVOID orxTest_HashMap_Create()
{
  orxCHAR zUserValue[64];     /* String where user inputs are stored */
  orxS32 s32ID;
  orxS32 s32NbElem;
  orxBOOL bValidValue;
  
  /* Is it possible to create a Hash Map ? */
  if (sstTest_HashMap.u32NbUsedHash == orxTEST_HASHMAP_KU32_ARRAY_NB_ELEM)
  {
    orxString_PrintLn("All ID have been used. Delete a hash table before create a new one");
    return;
  }
  
  orxString_PrintLn("This command allows you to create a hash table. To simplify");
  orxString_PrintLn("the test module, you have to give an unused Hash Table ID");
  orxString_PrintLn("between 0 and %d that has not already been used.", orxTEST_HASHMAP_KU32_ARRAY_NB_ELEM - 1);

  /* All ID available ? */
  if (sstTest_HashMap.u32NbUsedHash > 0)
  {
    /* Display the list of used ID */
    orxTest_HashMap_PrintUsedID();
  }
  
  bValidValue = orxFALSE;
  
  /* We will create a new hash map, get the ID from the user */
  do
  {
    /* Read string value */
    orxString_ReadString(zUserValue, 63, "Choose an ID for the Hash Table : ");
  
    /* Convert it to a number */
    if (orxString_ToS32(&s32ID, zUserValue, 10) != orxSTATUS_FAILED)
    {
      if (s32ID >= 0 && s32ID < orxTEST_HASHMAP_KU32_ARRAY_NB_ELEM)
      {
        if (sstTest_HashMap.apstHashMap[s32ID] == orxNULL)
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
    
  /* Now, allocate s32NbElem elements in a new hash map at the index position s32ID */
  sstTest_HashMap.apstHashMap[s32ID] = orxHashMap_Create(s32NbElem, 0, orxMEMORY_TYPE_MAIN);
  if (sstTest_HashMap.apstHashMap[s32ID] == orxNULL)
  {
    orxString_PrintLn("Can't create the hash map. Not enough memory ?");
  }
  else
  {
    orxString_PrintLn("New bank created with %d keys at the ID %d", s32NbElem, s32ID);
    
    /* Increase the counter */
    sstTest_HashMap.u32NbUsedHash++;
  }
}

/** Destroy a hash map
 */
orxVOID orxTest_HashMap_Destroy()
{
  orxCHAR zUserValue[64];     /* String where user inputs are stored */
  orxS32 s32ID;
  orxBOOL bValidValue;
  
  /* Are there allocated bank ? */
  if (sstTest_HashMap.u32NbUsedHash == 0)
  {
    orxString_PrintLn("No hash map have been created. Create a hash map before trying to delete it");
    return;
  }
  
  /* Display the list of allocated hash map */
  orxTest_HashMap_PrintUsedID();
  
  /* Now, get the hash map ID to delete */
  bValidValue = orxFALSE;
  do
  {
    /* Read string value */
    orxString_ReadString(zUserValue, 63, "Hash map ID to delete : ");
    
    /* Convert it to a number */
    if (orxString_ToS32(&s32ID, zUserValue, 10) != orxSTATUS_FAILED)
    {
      if (((s32ID >= 0) &&
          (s32ID < orxTEST_HASHMAP_KU32_ARRAY_NB_ELEM)) &&
          (sstTest_HashMap.apstHashMap[s32ID] != orxNULL))
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
  orxString_PrintLn("Destroy a hash map");
  
  /* Delete the hash map */
  orxString_PrintLn("Delete hash map %d", s32ID);
  orxHashMap_Delete(sstTest_HashMap.apstHashMap[s32ID]);
  sstTest_HashMap.apstHashMap[s32ID] = orxNULL;
  
  /* Done !*/
  orxString_PrintLn("hash map deleted !");
}

/** Add a new pair key/value
 */
orxVOID orxTest_HashMap_Add()
{
  orxCHAR zUserValue[64];     /* String where user inputs are stored */
  orxS32 s32ID;
  orxBOOL bValidValue;
  orxS32 s32Key;
  orxS32 s32Data;
  
  /* Are there allocated hash table ? */
  if (sstTest_HashMap.u32NbUsedHash == 0)
  {
    orxString_PrintLn("No hash map have been created. Create a hash map before trying to add datas");
    return;
  }
  
  /* Display the list of hash map */
  orxTest_HashMap_PrintUsedID();
  
  /* Now, get the hash map ID to use */
  bValidValue = orxFALSE;
  do
  {
    /* Read string value */
    orxString_ReadString(zUserValue, 63, "Hash map ID to use (where key/data will be allocated) : ");
    
    /* Convert it to a number */
    if (orxString_ToS32(&s32ID, zUserValue, 10) != orxSTATUS_FAILED)
    {
      if (((s32ID >= 0) &&
          (s32ID < orxTEST_HASHMAP_KU32_ARRAY_NB_ELEM)) &&
          (sstTest_HashMap.apstHashMap[s32ID] != orxNULL))
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
    if (orxString_ToS32(&s32Key, zUserValue, 16) != orxSTATUS_FAILED)
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
    if (orxString_ToS32(&s32Data, zUserValue, 16) != orxSTATUS_FAILED)
    {
      bValidValue = orxTRUE;
    }
    else
    {
      orxString_PrintLn("This is not a valid value");
    }
  } while (!bValidValue);
  
  /* Got the key/value... Now, try to add it in the hash map */
  orxString_PrintLn("Trying to add the pair kay/value (%d/%d)...", s32Key, s32Data);
  if (orxHashMap_Add(sstTest_HashMap.apstHashMap[s32ID], (orxU32)s32Key, (orxVOID *)s32Data) == orxSTATUS_FAILED)
  {
    /* Insetion failed */
    orxString_PrintLn("Insertion failed...");
  }
  orxString_PrintLn("Done !");
}

/** Remove a key from the hash table
 */
orxVOID orxTest_HashMap_Remove()
{
  orxCHAR zUserValue[64];     /* String where user inputs are stored */
  orxS32 s32ID;
  orxS32 s32Key;
  orxBOOL bValidValue;
  
  /* Are there allocated hash map ? */
  if (sstTest_HashMap.u32NbUsedHash == 0)
  {
    orxString_PrintLn("No hash map have been created. you can't remove keys");
    return;
  }
  
  /* Display the list of allocated hash map */
  orxTest_HashMap_PrintUsedID();
  
  /* Now, get the hash map ID to use */
  bValidValue = orxFALSE;
  do
  {
    /* Read string value */
    orxString_ReadString(zUserValue, 63, "Hash map ID where are stored keys : ");
    
    /* Convert it to a number */
    if (orxString_ToS32(&s32ID, zUserValue, 10) != orxSTATUS_FAILED)
    {
      if (((s32ID >= 0) &&
          (s32ID < orxTEST_HASHMAP_KU32_ARRAY_NB_ELEM)) &&
          (sstTest_HashMap.apstHashMap[s32ID] != orxNULL))
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
    if (orxString_ToS32(&s32Key, zUserValue, 16) != orxSTATUS_FAILED)
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
  if (orxHashMap_Remove(sstTest_HashMap.apstHashMap[s32ID], (orxU32)s32Key) == orxSTATUS_FAILED)
  {
    /* Failed to remove */
    orxString_PrintLn("Remove failed...");
  }
  orxString_PrintLn("Done !");
}

/** Print a value associated to a key
 */
orxVOID orxTest_HashMap_Get()
{
  orxCHAR zUserValue[64];     /* String where user inputs are stored */
  orxS32 s32ID;
  orxS32 s32Key;
  orxVOID *pData;
  orxBOOL bValidValue;
  
  /* Are there allocated hash map ? */
  if (sstTest_HashMap.u32NbUsedHash == 0)
  {
    orxString_PrintLn("No hash map have been created. you print key value");
    return;
  }
  
  /* Display the list of allocated hash map */
  orxTest_HashMap_PrintUsedID();
  
  /* Now, get the hash map ID to use */
  bValidValue = orxFALSE;
  do
  {
    /* Read string value */
    orxString_ReadString(zUserValue, 63, "Hash map ID where are stored keys : ");
    
    /* Convert it to a number */
    if (orxString_ToS32(&s32ID, zUserValue, 10) != orxSTATUS_FAILED)
    {
      if (((s32ID >= 0) &&
          (s32ID < orxTEST_HASHMAP_KU32_ARRAY_NB_ELEM)) &&
          (sstTest_HashMap.apstHashMap[s32ID] != orxNULL))
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
    if (orxString_ToS32(&s32Key, zUserValue, 16) != orxSTATUS_FAILED)
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
  pData = orxHashMap_Get(sstTest_HashMap.apstHashMap[s32ID], (orxU32)s32Key);

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

/** Clear all cells from a hash map
 */
orxVOID orxTest_HashMap_Clear()
{
  orxCHAR zUserValue[64];     /* String where user inputs are stored */
  orxS32 s32ID;
  orxBOOL bValidValue;
  
  /* Are there allocated hash map ? */
  if (sstTest_HashMap.u32NbUsedHash == 0)
  {
    orxString_PrintLn("No hash map have been created. you can't clear it");
    return;
  }
  
  /* Display the list of allocated bank */
  orxTest_HashMap_PrintUsedID();
  
  /* Now, get the bank ID to use */
  bValidValue = orxFALSE;
  do
  {
    /* Read string value */
    orxString_ReadString(zUserValue, 63, "Hahs Map ID to clear : ");
    
    /* Convert it to a number */
    if (orxString_ToS32(&s32ID, zUserValue, 10) != orxSTATUS_FAILED)
    {
      if (((s32ID >= 0) &&
          (s32ID < orxTEST_HASHMAP_KU32_ARRAY_NB_ELEM)) &&
          (sstTest_HashMap.apstHashMap[s32ID] != orxNULL))
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
  
  /* Clear the hash map */
  orxString_PrintLn("Trying to clear hash map...");
  if (orxHashMap_Clear(sstTest_HashMap.apstHashMap[s32ID]) == orxSTATUS_FAILED)
  {
    /* Failed to clear hash */
    orxString_PrintLn("Clear failed...");
  }
  orxString_PrintLn("Done !");
}

/** Print content of a hash map
 */
orxVOID orxTest_HashMap_PrintAll()
{
  orxCHAR zUserValue[64];     /* String where user inputs are stored */
  orxS32 s32ID;
  orxBOOL bValidValue;
  
  /* Are there allocated hash map ? */
  if (sstTest_HashMap.u32NbUsedHash == 0)
  {
    orxString_PrintLn("No hash map have been created. you can't print it");
    return;
  }
  
  /* Display the list of allocated hash map */
  orxTest_HashMap_PrintUsedID();
  
  /* Now, get the bank ID to use */
  bValidValue = orxFALSE;
  do
  {
    /* Read string value */
    orxString_ReadString(zUserValue, 63, "Hash map ID to display : ");
    
    /* Convert it to a number */
    if (orxString_ToS32(&s32ID, zUserValue, 10) != orxSTATUS_FAILED)
    {
      if (((s32ID >= 0) &&
          (s32ID < orxTEST_HASHMAP_KU32_ARRAY_NB_ELEM)) &&
          (sstTest_HashMap.apstHashMap[s32ID] != orxNULL))
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
  orxHashMap_DebugPrint(sstTest_HashMap.apstHashMap[s32ID]);
}

/******************************************************
 * DYNAMIC LIBRARY ENTRY POINTS
 ******************************************************/
orxVOID orxTest_HashMap_Init()
{
  /* Initialize bank module */
  orxHashMap_Init();
  
  /* Register test functions */
  orxTest_Register("Hash", "Display module informations", orxTest_HashMap_Infos);
  orxTest_Register("Hash", "Create a hash map", orxTest_HashMap_Create);
  orxTest_Register("Hash", "Delete a hash map", orxTest_HashMap_Destroy);
  orxTest_Register("Hash", "Add a key associated with a data in a map", orxTest_HashMap_Add);
  orxTest_Register("Hash", "Remove a pai key/vlaue", orxTest_HashMap_Remove);
  orxTest_Register("Hash", "Print the value associated to a key in a hash map", orxTest_HashMap_Get);
  orxTest_Register("Hash", "Remove all stored keys/value from a hash map", orxTest_HashMap_Clear);
  orxTest_Register("Hash", "Print the internal content of a hash map", orxTest_HashMap_PrintAll);
  
  /* Initialize static datas */
  orxMemory_Set(&sstTest_HashMap, 0, sizeof(orxTEST_HASHMAP));
}

orxVOID orxTest_HashMap_Exit()
{
  orxU32 u32HashMapIndex; /* Index of the current hash map */
  
  /* Traverse allocated hash map and free them */
  for (u32HashMapIndex = 0; u32HashMapIndex < orxTEST_HASHMAP_KU32_ARRAY_NB_ELEM; u32HashMapIndex++)
  {
    /* hash map allocated ? */
    if (sstTest_HashMap.apstHashMap[u32HashMapIndex] != orxNULL)
    {
      /* Delete it */
      orxHashMap_Delete(sstTest_HashMap.apstHashMap[u32HashMapIndex]);
      sstTest_HashMap.apstHashMap[u32HashMapIndex] = orxNULL;
    }
  }
  
  /* Uninitialize module */
  orxHashMap_Exit();
}

orxTEST_DEFINE_ENTRY_POINT(orxTest_HashMap_Init, orxTest_HashMap_Exit)
