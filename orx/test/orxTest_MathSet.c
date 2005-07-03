/**
 * @file orxTest_MathTest.c
 * 
 * Mathematical set Test Program
 * 
 */
 
 /***************************************************************************
 orxTest_MathSet.c
 Mathematical set Test Program
 
 begin                : 10/04/2005
 author               : (C) Arcallians
 email                : cursor@arcallians.org
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
#include "math/orxMathSet.h"
#include "io/orxTextIO.h"


/******************************************************
 * TEST FUNCTIONS
 ******************************************************/

/** Display informations about this test module
 */
orxVOID orxTest_MathSet_Infos()
{
  orxTextIO_PrintLn("Mathematical set :");
}



/******************************************************
 * DYNAMIC LIBRARY ENTRY POINTS
 ******************************************************/
orxVOID orxTest_MathSet_Init()
{
  /* Initialize bank module */
  orxMathSet_Init();
  
  /* Register test functions */
  orxTest_Register("MathSet", "Display module informations", orxTest_MathSet_Infos);
   
}

orxVOID orxTest_MathSet_Exit()
{
  /* Uninitialize module */
  orxMathSet_Exit();
}

orxTEST_DEFINE_ENTRY_POINT(orxTest_MathSet_Init, orxTest_MathSet_Exit)

