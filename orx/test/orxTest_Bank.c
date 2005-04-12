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

/******************************************************
 * TEST FUNCTIONS
 ******************************************************/
orxVOID orxTest_Bank_Create()
{
  printf("plop1\n");
}

orxVOID orxTest_Bank_Destroy()
{
  printf("plop2\n");
}

/******************************************************
 * DYNAMIC LIBRARY ENTRY POINTS
 ******************************************************/
orxVOID orxTest_Bank_Init()
{
  /* Initialize bank module */
  orxBank_Init();
  
  /* Register test functions */
  orxTest_Register("Bank", "Create a bank of memory", orxTest_Bank_Create);
  orxTest_Register("Bank", "Destroy a bank of memory", orxTest_Bank_Destroy);
}

orxVOID orxTest_Bank_Exit()
{
  /* Uninitialize module */
  orxBank_Exit();
}

orxTEST_DEFINE_ENTRY_POINT(orxTest_Bank_Init, orxTest_Bank_Exit)
