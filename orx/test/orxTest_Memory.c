/**
 * @file orxTest_Memory.c
 * 
 * Memory allocation / Unallocation Test Program
 * 
 */
 
 /***************************************************************************
 orxTest_Memory.c
 Memory allocation / Unallocation Test Program
 
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
#include "memory/orxMemory.h"

/******************************************************
 * DEFINES
 ******************************************************/

#define orxTEST_MEMORY_KU32_ARRAY_NB_ELEM  20
#define orxTEST_MEMORY_KU32_ELEM_SIZE      1024

/******************************************************
 * TYPES AND STRUCTURES
 ******************************************************/

orxSTATIC orxVOID *sapMemory[orxTEST_MEMORY_KU32_ARRAY_NB_ELEM];

/******************************************************
 * TEST FUNCTIONS
 ******************************************************/

/** Display informations about this test module
 */
orxVOID orxTest_Memory_Infos()
{
  printf("This test module will allocate and unallocate memory\n");
  printf("* TODO : Memory is directly allocated by the OS, orx has to add a layer on it to manage it\n");
}

/** Allocate memory and store allocated elements in an array
 */
orxVOID orxTest_Memory_Allocate()
{
  orxU32 u32ElemIndex;  /* Array's cursor */
  
  printf("Allocate %u elements of %u * 32 bits in the main memory...\n", orxTEST_MEMORY_KU32_ARRAY_NB_ELEM, orxTEST_MEMORY_KU32_ELEM_SIZE);

  /* Traverse the array and allocate memory for each elements */  
  for (u32ElemIndex = 0; u32ElemIndex < orxTEST_MEMORY_KU32_ARRAY_NB_ELEM; u32ElemIndex++)
  {
    /* Memory already allocated ? */
    if (sapMemory[u32ElemIndex] != orxNULL)
    {
      printf("Elem %lu Has already been allocated, free it before\n", u32ElemIndex);
    }
    else
    {
      /* Allocate space */
      sapMemory[u32ElemIndex] = orxMemory_Allocate(1024 * sizeof(orxU32), orxMEMORY_TYPE_MAIN);
  
      /* Display success status */
      if (sapMemory[u32ElemIndex] != orxNULL)
      {
        printf("Elem %lu allocation : Ok\n", u32ElemIndex);
      }
      else
      {
        printf("Elem %lu allocation : Failed\n", u32ElemIndex);
      }
    }
  }

  /* Everythings done */
  printf("Done !\n");
}

/** Free allocated memory (don't free if orxNULL pointer : unpredictable behaviour for each system)
 */
orxVOID orxTest_Memory_Free()
{
  orxU32 u32ElemIndex;  /* Array's cursor */
  
  printf("Free allocated memory\n");

  /* Traverse the array and free memory for each allocated elements */  
  for (u32ElemIndex = 0; u32ElemIndex < orxTEST_MEMORY_KU32_ARRAY_NB_ELEM; u32ElemIndex++)
  {
    if (sapMemory[u32ElemIndex] != orxNULL)
    {
      orxMemory_Free(sapMemory[u32ElemIndex]);
      sapMemory[u32ElemIndex] = orxNULL;
      printf("Elem %lu Freed\n", u32ElemIndex);
    }
    else
    {
      /* Element wasn't allocated */
      printf("Elem %lu has not been allocated\n", u32ElemIndex);
    }
  }

  /* Everythings done */
  printf("Done !\n");
}

/******************************************************
 * DYNAMIC LIBRARY ENTRY POINTS
 ******************************************************/
orxVOID orxTest_Memory_Init()
{
  /* Initialize Memory module */
  orxMemory_Init();
  
  /* Register test functions */
  orxTest_Register("Memory", "Display module informations", orxTest_Memory_Infos);
  orxTest_Register("Memory", "Allocate memory", orxTest_Memory_Allocate);
  orxTest_Register("Memory", "free memory", orxTest_Memory_Free);
  
  /* Initialize static structure */
  orxMemory_Set(sapMemory, 0, sizeof(sapMemory));
}

orxVOID orxTest_Memory_Exit()
{
  orxU32 u32ElemIndex;  /* Array's cursor */
 
  /* Cleans up everything before exit */
  for (u32ElemIndex = 0; u32ElemIndex < orxTEST_MEMORY_KU32_ARRAY_NB_ELEM; u32ElemIndex++)
  {
    /* Memory allocated ? */
    if (sapMemory[u32ElemIndex] != orxNULL)
    {
      /* Free it */
      orxMemory_Free(sapMemory[u32ElemIndex]);
      sapMemory[u32ElemIndex] = orxNULL;
    }
  }
    
  /* Uninitialize Memory module */
  orxMemory_Exit();
}

orxTEST_DEFINE_ENTRY_POINT(orxTest_Memory_Init, orxTest_Memory_Exit)
