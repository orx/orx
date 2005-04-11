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

orxVOID orxTest_Memory_Allocate()
{
  printf("plopA\n");
}

orxVOID orxTest_Memory_Free()
{
  printf("plopB\n");
}

orxVOID orxTest_StartRegister()
{
  orxTest_Register("Memory", "Allocate memory", orxTest_Memory_Allocate);
  orxTest_Register("Memory", "free memory", orxTest_Memory_Free);
}
