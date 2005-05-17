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

#include "orxInclude.h"
#include "utils/orxTest.h"
#include "memory/orxMemory.h"
#include "io/orxTextIO.h"

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
  orxTextIO_PrintLn("This test module will allocate and unallocate memory");
  orxTextIO_PrintLn("* TODO : Memory is directly allocated by the OS, orx has to add a layer on it to manage it");
}

/** Allocate memory and store allocated elements in an array
 */
orxVOID orxTest_Memory_Allocate()
{
  orxU32 u32ElemIndex;  /* Array's cursor */
  
  orxTextIO_PrintLn("Allocate %u elements of %u * 32 bits in the main memory...", orxTEST_MEMORY_KU32_ARRAY_NB_ELEM, orxTEST_MEMORY_KU32_ELEM_SIZE);

  /* Traverse the array and allocate memory for each elements */  
  for (u32ElemIndex = 0; u32ElemIndex < orxTEST_MEMORY_KU32_ARRAY_NB_ELEM; u32ElemIndex++)
  {
    /* Memory already allocated ? */
    if (sapMemory[u32ElemIndex] != orxNULL)
    {
      orxTextIO_PrintLn("Elem %lu Has already been allocated, free it before", u32ElemIndex);
    }
    else
    {
      /* Allocate space */
      sapMemory[u32ElemIndex] = orxMemory_Allocate(1024 * sizeof(orxU32), orxMEMORY_TYPE_MAIN);
  
      /* Display success status */
      if (sapMemory[u32ElemIndex] != orxNULL)
      {
        orxTextIO_PrintLn("Elem %lu allocation : Ok", u32ElemIndex);
      }
      else
      {
        orxTextIO_PrintLn("Elem %lu allocation : Failed", u32ElemIndex);
      }
    }
  }

  /* Everythings done */
  orxTextIO_PrintLn("Done !");
}

/** Free allocated memory (don't free if orxNULL pointer : unpredictable behaviour for each system)
 */
orxVOID orxTest_Memory_Free()
{
  orxU32 u32ElemIndex;  /* Array's cursor */
  
  orxTextIO_PrintLn("Free allocated memory");

  /* Traverse the array and free memory for each allocated elements */  
  for (u32ElemIndex = 0; u32ElemIndex < orxTEST_MEMORY_KU32_ARRAY_NB_ELEM; u32ElemIndex++)
  {
    if (sapMemory[u32ElemIndex] != orxNULL)
    {
      orxMemory_Free(sapMemory[u32ElemIndex]);
      sapMemory[u32ElemIndex] = orxNULL;
      orxTextIO_PrintLn("Elem %lu Freed", u32ElemIndex);
    }
    else
    {
      /* Element wasn't allocated */
      orxTextIO_PrintLn("Elem %lu has not been allocated", u32ElemIndex);
    }
  }

  /* Everythings done */
  orxTextIO_PrintLn("Done !");
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
