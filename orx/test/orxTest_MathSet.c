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
#include "debug/orxTest.h"
#include "math/orxMathSet.h"
#include "io/orxTextIO.h"


/******************************************************
 * One global object for each type.
 ******************************************************/
static orxSET_FLOAT sstSetFloat;
static orxSET_INT32 sstSetInt32;

/** Global interval used for usr manipulations.*/
static orxINTERVAL_FLOAT sstIntervFloat;
/** Global interval used for usr manipulations.*/
static orxINTERVAL_INT32 sstIntervInt32;

/** Global interval used for autotest with sstIntervFloat.*/
static orxINTERVAL_FLOAT sstIntervFloatTest;
/** Global interval used for autotest with sstIntervInt32.*/
static orxINTERVAL_INT32 sstIntervInt32Test;


/******************************************************
 * Tool functions.
 ******************************************************/
 
/** Read a float-based interval from user.*/
orxVOID orxTest_MathSet_ReadIntervalFloat(orxINTERVAL_FLOAT* pstIntervFloat)
{
    orxS32 s32FlagMin, s32FlagMax;
    orxTextIO_ReadFloat(&(pstIntervFloat->fMin), "Float MIN range : ", orxTRUE);
    orxTextIO_ReadS32InRange(&s32FlagMin, 2, 0, 1, "Is MIN is included (0/1) ? ", orxTRUE);
    orxTextIO_ReadFloat(&(pstIntervFloat->fMax), "Float MAX range : ", orxTRUE);
    orxTextIO_ReadS32InRange(&s32FlagMax, 2, 0, 1, "Is MAX is included (0/1) ? ", orxTRUE);
    pstIntervFloat->u32Flags = ((s32FlagMin&0x0001)<<orxINTERVALFLOAT_MIN_SHIFT) + ((s32FlagMax&0x0001)<<orxINTERVALFLOAT_MAX_SHIFT);
    orxIntervalFloat_Validate(pstIntervFloat);
}

/** Read an integer-based interval from user.*/
orxVOID orxTest_MathSet_ReadIntervalInt32(orxINTERVAL_INT32* pstIntervInt32)
{
    orxTextIO_ReadS32(&(pstIntervInt32->s32Min), 10, "Integer MIN range : ", orxTRUE);
    orxTextIO_ReadS32(&(pstIntervInt32->s32Max), 10, "Integer MAX range : ", orxTRUE);
    orxIntervalInt32_Validate(pstIntervInt32);
}

/** Output a float-based interval to std output.*/
orxVOID orxTest_MathSet_PrintIntervalFloat(orxINTERVAL_FLOAT stIntervFloat)
{
    orxTextIO_Print("%s%g;%g%s", 
            (((stIntervFloat.u32Flags&orxINTERVALFLOAT_MIN_INCLUDED)==orxINTERVALFLOAT_MIN_INCLUDED)?"[":"]"),
            stIntervFloat.fMin,
            stIntervFloat.fMax,
            (((stIntervFloat.u32Flags&orxINTERVALFLOAT_MAX_INCLUDED)==orxINTERVALFLOAT_MAX_INCLUDED)?"]":"[") );
}

/** Output an integer-based interval to std output.*/
orxVOID orxTest_MathSet_PrintIntervalInt32(orxINTERVAL_INT32 stIntervInt32)
{
    orxTextIO_Print("[%ld;%ld]", stIntervInt32.s32Min, stIntervInt32.s32Max);
}

/******************************************************
 * TEST FUNCTIONS
 ******************************************************/

/** Display informations about this test module
 */
orxVOID orxTest_MathSet_Infos()
{
  orxTextIO_PrintLn("Mathematical set test module :");
  orxTextIO_PrintLn("This will alloc and manipulate interger and float-based intervals and sets.");
  orxTextIO_PrintLn("Only one of them will be manipulated at each time.");
}

/** Reset all objects.
 */
orxVOID orxTest_MathSet_Reset()
{
  orxIntervalInt32_Set(&sstIntervInt32, 0, 0);
  orxIntervalFloat_Set(&sstIntervFloat, 0.0, 0.0, orxFALSE, orxFALSE);
  
  orxSetInt32_Clear(&sstSetInt32);
  orxSetFloat_Clear(&sstSetFloat);
  
  orxTextIO_PrintLn("All object are cleared.");
}

/** Display objects content.
 */
orxVOID orxTest_MathSet_DisplayContent()
{
    orxTextIO_Print("Float-based interval : ");
    orxTest_MathSet_PrintIntervalFloat(sstIntervFloat);
    orxTextIO_PrintLn(orxSTRING_Empty);
    
    orxTextIO_Print("Integer-based interval : ");
    orxTest_MathSet_PrintIntervalInt32(sstIntervInt32);
    orxTextIO_PrintLn(orxSTRING_Empty);
}

/** Set float-based interval.
 */
orxVOID orxTest_MathSet_InputFloatInterval()
{
    orxTest_MathSet_ReadIntervalFloat(&sstIntervFloat);
}

/** Set integer-based interval.
 */
orxVOID orxTest_MathSet_InputInt32Interval()
{
    orxTest_MathSet_ReadIntervalInt32(&sstIntervInt32);
}


/** Great auto-test function.
 * Auto test all intervals functions.
 */
orxVOID orxTest_MathSet_AutoTestIntervals()
{
    orxTextIO_PrintLn("Integer-based tests :");
    orxTextIO_PrintLn("---------------------");
    
    orxTextIO_Print(" - Assignation (orxIntervalInt32_Set)                           ");
    orxIntervalInt32_Set(&sstIntervInt32, 25, 32);
    if ((sstIntervInt32.s32Min==25) && (sstIntervInt32.s32Max==32))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    orxTextIO_Print(" - Copy (orxIntervalInt32_Copy)                                 ");
    orxIntervalInt32_Copy(sstIntervInt32, &sstIntervInt32Test);
    if ((sstIntervInt32Test.s32Min==25) && (sstIntervInt32Test.s32Max==32))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    orxTextIO_Print(" - Validation (orxIntervalInt32_Validate)                       ");
    orxIntervalInt32_Set(&sstIntervInt32Test, 55, 28);
    orxIntervalInt32_Validate(&sstIntervInt32Test);
    if ((sstIntervInt32Test.s32Min==28) && (sstIntervInt32Test.s32Max==55))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    orxTextIO_Print(" - Sawp (orxIntervalInt32_Swap)                                 ");
    orxIntervalInt32_Swap(&sstIntervInt32, &sstIntervInt32Test);
    if ((sstIntervInt32.s32Min==28) && (sstIntervInt32.s32Max==55) && (sstIntervInt32Test.s32Min==25) && (sstIntervInt32Test.s32Max==32))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");
    
    orxTextIO_Print(" - Eguality (orxIntervalInt32_AreEgual)                         ");
    if ((orxIntervalInt32_AreEgual(sstIntervInt32, sstIntervInt32)==orxTRUE) && (orxIntervalInt32_AreEgual(sstIntervInt32, sstIntervInt32Test)==orxFALSE))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");
    
    orxTextIO_Print(" - Difference (orxIntervalInt32_AreDifferent)                   ");
    if ((orxIntervalInt32_AreDifferent(sstIntervInt32, sstIntervInt32)==orxFALSE) && (orxIntervalInt32_AreDifferent(sstIntervInt32, sstIntervInt32Test)==orxTRUE))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    orxTextIO_Print(" - Comparison (orxIntervalInt32_IsLess)                         ");
    if ((orxIntervalInt32_IsLess(orxIntervalInt32(10, 20), orxIntervalInt32(30, 50))==orxTRUE) &&
        (orxIntervalInt32_IsLess(orxIntervalInt32(10, 40), orxIntervalInt32(30, 50))==orxFALSE) &&
        (orxIntervalInt32_IsLess(orxIntervalInt32(35, 45), orxIntervalInt32(30, 50))==orxFALSE) && 
        (orxIntervalInt32_IsLess(orxIntervalInt32(40, 60), orxIntervalInt32(30, 50))==orxFALSE) && 
        (orxIntervalInt32_IsLess(orxIntervalInt32(60, 80), orxIntervalInt32(30, 50))==orxFALSE) &&
        (orxIntervalInt32_IsLess(orxIntervalInt32(30, 50), orxIntervalInt32(35, 45))==orxFALSE) )
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    orxTextIO_Print(" - Comparison (orxIntervalInt32_IsLessOrBottomThrow)            ");
    if ((orxIntervalInt32_IsLessOrBottomThrow(orxIntervalInt32(10, 20), orxIntervalInt32(30, 50))==orxTRUE) &&
        (orxIntervalInt32_IsLessOrBottomThrow(orxIntervalInt32(10, 40), orxIntervalInt32(30, 50))==orxTRUE) &&
        (orxIntervalInt32_IsLessOrBottomThrow(orxIntervalInt32(35, 45), orxIntervalInt32(30, 50))==orxFALSE) && 
        (orxIntervalInt32_IsLessOrBottomThrow(orxIntervalInt32(40, 60), orxIntervalInt32(30, 50))==orxFALSE) && 
        (orxIntervalInt32_IsLessOrBottomThrow(orxIntervalInt32(60, 80), orxIntervalInt32(30, 50))==orxFALSE) &&
        (orxIntervalInt32_IsLessOrBottomThrow(orxIntervalInt32(30, 50), orxIntervalInt32(35, 45))==orxFALSE) )
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    orxTextIO_Print(" - Comparison (orxIntervalInt32_IsGreater)                      ");
    if ((orxIntervalInt32_IsGreater(orxIntervalInt32(10, 20), orxIntervalInt32(30, 50))==orxFALSE) &&
        (orxIntervalInt32_IsGreater(orxIntervalInt32(10, 40), orxIntervalInt32(30, 50))==orxFALSE) &&
        (orxIntervalInt32_IsGreater(orxIntervalInt32(35, 45), orxIntervalInt32(30, 50))==orxFALSE) && 
        (orxIntervalInt32_IsGreater(orxIntervalInt32(40, 60), orxIntervalInt32(30, 50))==orxFALSE) && 
        (orxIntervalInt32_IsGreater(orxIntervalInt32(60, 80), orxIntervalInt32(30, 50))==orxTRUE) &&
        (orxIntervalInt32_IsGreater(orxIntervalInt32(30, 50), orxIntervalInt32(35, 45))==orxFALSE) )
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    orxTextIO_Print(" - Comparison (orxIntervalInt32_IsGreaterOrTopThrow)            ");
    if ((orxIntervalInt32_IsGreaterOrTopThrow(orxIntervalInt32(10, 20), orxIntervalInt32(30, 50))==orxFALSE) &&
        (orxIntervalInt32_IsGreaterOrTopThrow(orxIntervalInt32(10, 40), orxIntervalInt32(30, 50))==orxFALSE) &&
        (orxIntervalInt32_IsGreaterOrTopThrow(orxIntervalInt32(35, 45), orxIntervalInt32(30, 50))==orxFALSE) && 
        (orxIntervalInt32_IsGreaterOrTopThrow(orxIntervalInt32(40, 60), orxIntervalInt32(30, 50))==orxTRUE) && 
        (orxIntervalInt32_IsGreaterOrTopThrow(orxIntervalInt32(60, 80), orxIntervalInt32(30, 50))==orxTRUE) &&
        (orxIntervalInt32_IsGreaterOrTopThrow(orxIntervalInt32(30, 50), orxIntervalInt32(35, 45))==orxFALSE) )
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    orxTextIO_Print(" - Comparison (orxIntervalInt32_IsIn)                           ");
    if ((orxIntervalInt32_IsIn(orxIntervalInt32(10, 20), orxIntervalInt32(30, 50))==orxFALSE) &&
        (orxIntervalInt32_IsIn(orxIntervalInt32(10, 40), orxIntervalInt32(30, 50))==orxFALSE) &&
        (orxIntervalInt32_IsIn(orxIntervalInt32(35, 45), orxIntervalInt32(30, 50))==orxTRUE) && 
        (orxIntervalInt32_IsIn(orxIntervalInt32(40, 60), orxIntervalInt32(30, 50))==orxFALSE) && 
        (orxIntervalInt32_IsIn(orxIntervalInt32(60, 80), orxIntervalInt32(30, 50))==orxFALSE) &&
        (orxIntervalInt32_IsIn(orxIntervalInt32(30, 50), orxIntervalInt32(35, 45))==orxFALSE) )
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    orxTextIO_Print(" - Search (orxIntervalInt32_HasValue)                           ");
    if ((orxIntervalInt32_HasValue(orxIntervalInt32(10, 20), 5)  == orxFALSE) && 
        (orxIntervalInt32_HasValue(orxIntervalInt32(10, 20), 10) == orxTRUE) && 
        (orxIntervalInt32_HasValue(orxIntervalInt32(10, 20), 15) == orxTRUE) && 
        (orxIntervalInt32_HasValue(orxIntervalInt32(10, 20), 20) == orxTRUE) && 
        (orxIntervalInt32_HasValue(orxIntervalInt32(10, 20), 25) == orxFALSE))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    orxTextIO_Print(" - Extand (orxIntervalInt32_Extand) - test 01                   ");
    orxIntervalInt32_Set(&sstIntervInt32, 25, 32);
    orxIntervalInt32_Extand(&sstIntervInt32, 48);
    if ((sstIntervInt32.s32Min==25)&&(sstIntervInt32.s32Max==48))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    orxTextIO_Print(" - Extand (orxIntervalInt32_Extand) - test 02                   ");
    orxIntervalInt32_Set(&sstIntervInt32, 25, 32);
    orxIntervalInt32_Extand(&sstIntervInt32, 15);
    if ((sstIntervInt32.s32Min==15)&&(sstIntervInt32.s32Max==32))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    orxTextIO_Print(" - Extand (orxIntervalInt32_Extand) - test 03                   ");
    orxIntervalInt32_Set(&sstIntervInt32, 25, 32);
    orxIntervalInt32_Extand(&sstIntervInt32, 28);
    if ((sstIntervInt32.s32Min==25)&&(sstIntervInt32.s32Max==32))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");



    orxTextIO_PrintLn("Float-based tests :");
    orxTextIO_PrintLn("-------------------");
    
    orxTextIO_Print(" - Point comparison (orxIntervalFloat_IsPointMinLessMin)        ");
    if ((orxIntervalFloat_IsPointMinLessMin(orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE), orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE)) == orxFALSE) &&
        (orxIntervalFloat_IsPointMinLessMin(orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE), orxIntervalFloat(10.0, 20.0, orxTRUE , orxTRUE )) == orxFALSE) &&
        (orxIntervalFloat_IsPointMinLessMin(orxIntervalFloat(10.0, 20.0, orxTRUE , orxTRUE ), orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE)) == orxTRUE ) &&
        (orxIntervalFloat_IsPointMinLessMin(orxIntervalFloat(10.0, 20.0, orxTRUE , orxTRUE ), orxIntervalFloat(10.0, 20.0, orxTRUE , orxTRUE )) == orxFALSE) &&
        (orxIntervalFloat_IsPointMinLessMin(orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE), orxIntervalFloat(11.5, 21.5, orxFALSE, orxFALSE)) == orxTRUE ) &&
        (orxIntervalFloat_IsPointMinLessMin(orxIntervalFloat(10.0, 20.0, orxFALSE, orxTRUE ), orxIntervalFloat(09.5, 19.5, orxFALSE, orxFALSE)) == orxFALSE))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    orxTextIO_Print(" - Point comparison (orxIntervalFloat_IsPointMinLessMax)        ");
    if ((orxIntervalFloat_IsPointMinLessMax(orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE), orxIntervalFloat(05.0, 10.0, orxFALSE, orxFALSE)) == orxFALSE) &&
        (orxIntervalFloat_IsPointMinLessMax(orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE), orxIntervalFloat(05.0, 10.0, orxTRUE , orxTRUE )) == orxFALSE) &&
        (orxIntervalFloat_IsPointMinLessMax(orxIntervalFloat(10.0, 20.0, orxTRUE , orxTRUE ), orxIntervalFloat(05.0, 10.0, orxFALSE, orxFALSE)) == orxFALSE) &&
        (orxIntervalFloat_IsPointMinLessMax(orxIntervalFloat(10.0, 20.0, orxTRUE , orxTRUE ), orxIntervalFloat(05.0, 10.0, orxTRUE , orxTRUE )) == orxFALSE) &&
        (orxIntervalFloat_IsPointMinLessMax(orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE), orxIntervalFloat(05.0, 11.5, orxFALSE, orxFALSE)) == orxTRUE ) &&
        (orxIntervalFloat_IsPointMinLessMax(orxIntervalFloat(10.0, 20.0, orxFALSE, orxTRUE ), orxIntervalFloat(05.0, 09.5, orxFALSE, orxFALSE)) == orxFALSE))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    orxTextIO_Print(" - Point comparison (orxIntervalFloat_IsPointMaxLessMin)        ");
    if ((orxIntervalFloat_IsPointMaxLessMin(orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE), orxIntervalFloat(20.0, 30.0, orxFALSE, orxFALSE)) == orxTRUE ) &&
        (orxIntervalFloat_IsPointMaxLessMin(orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE), orxIntervalFloat(20.0, 30.0, orxTRUE , orxTRUE )) == orxTRUE ) &&
        (orxIntervalFloat_IsPointMaxLessMin(orxIntervalFloat(10.0, 20.0, orxTRUE , orxTRUE ), orxIntervalFloat(20.0, 30.0, orxFALSE, orxFALSE)) == orxTRUE ) &&
        (orxIntervalFloat_IsPointMaxLessMin(orxIntervalFloat(10.0, 20.0, orxTRUE , orxTRUE ), orxIntervalFloat(20.0, 30.0, orxTRUE , orxTRUE )) == orxFALSE) &&
        (orxIntervalFloat_IsPointMaxLessMin(orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE), orxIntervalFloat(21.5, 30.0, orxFALSE, orxFALSE)) == orxTRUE ) &&
        (orxIntervalFloat_IsPointMaxLessMin(orxIntervalFloat(10.0, 20.0, orxFALSE, orxTRUE ), orxIntervalFloat(19.5, 30.0, orxFALSE, orxFALSE)) == orxFALSE))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    
    orxTextIO_Print(" - Point comparison (orxIntervalFloat_IsPointMaxLessMax)        ");
    if ((orxIntervalFloat_IsPointMaxLessMax(orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE), orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE)) == orxFALSE) &&
        (orxIntervalFloat_IsPointMaxLessMax(orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE), orxIntervalFloat(10.0, 20.0, orxTRUE , orxTRUE )) == orxTRUE ) &&
        (orxIntervalFloat_IsPointMaxLessMax(orxIntervalFloat(10.0, 20.0, orxTRUE , orxTRUE ), orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE)) == orxFALSE) &&
        (orxIntervalFloat_IsPointMaxLessMax(orxIntervalFloat(10.0, 20.0, orxTRUE , orxTRUE ), orxIntervalFloat(10.0, 20.0, orxTRUE , orxTRUE )) == orxFALSE) &&
        (orxIntervalFloat_IsPointMaxLessMax(orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE), orxIntervalFloat(11.5, 21.5, orxFALSE, orxFALSE)) == orxTRUE ) &&
        (orxIntervalFloat_IsPointMaxLessMax(orxIntervalFloat(10.0, 20.0, orxFALSE, orxTRUE ), orxIntervalFloat(09.5, 19.5, orxFALSE, orxFALSE)) == orxFALSE))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    orxTextIO_Print(" - Point comparison (orxIntervalFloat_IsPointMinGreaterMin)     ");
    if ((orxIntervalFloat_IsPointMinGreaterMin(orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE), orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE)) == orxFALSE) &&
        (orxIntervalFloat_IsPointMinGreaterMin(orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE), orxIntervalFloat(10.0, 20.0, orxTRUE , orxTRUE )) == orxTRUE ) &&
        (orxIntervalFloat_IsPointMinGreaterMin(orxIntervalFloat(10.0, 20.0, orxTRUE , orxTRUE ), orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE)) == orxFALSE) &&
        (orxIntervalFloat_IsPointMinGreaterMin(orxIntervalFloat(10.0, 20.0, orxTRUE , orxTRUE ), orxIntervalFloat(10.0, 20.0, orxTRUE , orxTRUE )) == orxFALSE) &&
        (orxIntervalFloat_IsPointMinGreaterMin(orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE), orxIntervalFloat(11.5, 21.5, orxFALSE, orxFALSE)) == orxFALSE) &&
        (orxIntervalFloat_IsPointMinGreaterMin(orxIntervalFloat(10.0, 20.0, orxFALSE, orxTRUE ), orxIntervalFloat(09.5, 19.5, orxFALSE, orxFALSE)) == orxTRUE ))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");
    
    orxTextIO_Print(" - Point comparison (orxIntervalFloat_IsPointMinGreaterMax)     ");
    if ((orxIntervalFloat_IsPointMinGreaterMax(orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE), orxIntervalFloat(05.0, 10.0, orxFALSE, orxFALSE)) == orxTRUE ) &&
        (orxIntervalFloat_IsPointMinGreaterMax(orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE), orxIntervalFloat(05.0, 10.0, orxTRUE , orxTRUE )) == orxTRUE ) &&
        (orxIntervalFloat_IsPointMinGreaterMax(orxIntervalFloat(10.0, 20.0, orxTRUE , orxTRUE ), orxIntervalFloat(05.0, 10.0, orxFALSE, orxFALSE)) == orxTRUE ) &&
        (orxIntervalFloat_IsPointMinGreaterMax(orxIntervalFloat(10.0, 20.0, orxTRUE , orxTRUE ), orxIntervalFloat(05.0, 10.0, orxTRUE , orxTRUE )) == orxFALSE) &&
        (orxIntervalFloat_IsPointMinGreaterMax(orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE), orxIntervalFloat(05.0, 11.5, orxFALSE, orxFALSE)) == orxFALSE) &&
        (orxIntervalFloat_IsPointMinGreaterMax(orxIntervalFloat(10.0, 20.0, orxFALSE, orxTRUE ), orxIntervalFloat(05.0, 09.5, orxFALSE, orxFALSE)) == orxTRUE ))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    orxTextIO_Print(" - Point comparison (orxIntervalFloat_IsPointMaxGreaterMin)     ");
    if ((orxIntervalFloat_IsPointMaxGreaterMin(orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE), orxIntervalFloat(20.0, 30.0, orxFALSE, orxFALSE)) == orxFALSE) &&
        (orxIntervalFloat_IsPointMaxGreaterMin(orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE), orxIntervalFloat(20.0, 30.0, orxTRUE , orxTRUE )) == orxFALSE) &&
        (orxIntervalFloat_IsPointMaxGreaterMin(orxIntervalFloat(10.0, 20.0, orxTRUE , orxTRUE ), orxIntervalFloat(20.0, 30.0, orxFALSE, orxFALSE)) == orxFALSE) &&
        (orxIntervalFloat_IsPointMaxGreaterMin(orxIntervalFloat(10.0, 20.0, orxTRUE , orxTRUE ), orxIntervalFloat(20.0, 30.0, orxTRUE , orxTRUE )) == orxFALSE) &&
        (orxIntervalFloat_IsPointMaxGreaterMin(orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE), orxIntervalFloat(21.5, 30.0, orxFALSE, orxFALSE)) == orxFALSE) &&
        (orxIntervalFloat_IsPointMaxGreaterMin(orxIntervalFloat(10.0, 20.0, orxFALSE, orxTRUE ), orxIntervalFloat(19.5, 30.0, orxFALSE, orxFALSE)) == orxTRUE))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    orxTextIO_Print(" - Point comparison (orxIntervalFloat_IsPointMaxGreaterMax)     ");
    if ((orxIntervalFloat_IsPointMaxGreaterMax(orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE), orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE)) == orxFALSE) &&
        (orxIntervalFloat_IsPointMaxGreaterMax(orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE), orxIntervalFloat(10.0, 20.0, orxTRUE , orxTRUE )) == orxFALSE) &&
        (orxIntervalFloat_IsPointMaxGreaterMax(orxIntervalFloat(10.0, 20.0, orxTRUE , orxTRUE ), orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE)) == orxTRUE ) &&
        (orxIntervalFloat_IsPointMaxGreaterMax(orxIntervalFloat(10.0, 20.0, orxTRUE , orxTRUE ), orxIntervalFloat(10.0, 20.0, orxTRUE , orxTRUE )) == orxFALSE) &&
        (orxIntervalFloat_IsPointMaxGreaterMax(orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE), orxIntervalFloat(11.5, 21.5, orxFALSE, orxFALSE)) == orxFALSE) &&
        (orxIntervalFloat_IsPointMaxGreaterMax(orxIntervalFloat(10.0, 20.0, orxFALSE, orxTRUE ), orxIntervalFloat(09.5, 19.5, orxFALSE, orxFALSE)) == orxTRUE ))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");


    orxTextIO_Print(" - Assignation (orxIntervalFloat_Set) - test 01                 ");
    orxIntervalFloat_Set(&sstIntervFloat, 25.0, 32.0, orxFALSE, orxFALSE);
    if ((sstIntervFloat.fMin==25.0) && (sstIntervFloat.fMax==32.0) && (sstIntervFloat.u32Flags==orxINTERVALFLOAT_ALL_EXCLUDED))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    orxTextIO_Print(" - Assignation (orxIntervalFloat_Set) - test 02                 ");
    orxIntervalFloat_Set(&sstIntervFloat, 25.0, 32.0, orxTRUE, orxFALSE);
    if ((sstIntervFloat.fMin==25.0) && (sstIntervFloat.fMax==32.0) && (sstIntervFloat.u32Flags==orxINTERVALFLOAT_MIN_INCLUDED))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    orxTextIO_Print(" - Assignation (orxIntervalFloat_Set) - test 03                 ");
    orxIntervalFloat_Set(&sstIntervFloat, 25.0, 32.0, orxFALSE, orxTRUE);
    if ((sstIntervFloat.fMin==25.0) && (sstIntervFloat.fMax==32.0) && (sstIntervFloat.u32Flags==orxINTERVALFLOAT_MAX_INCLUDED))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    orxTextIO_Print(" - Assignation (orxIntervalFloat_Set) - test 04                 ");
    orxIntervalFloat_Set(&sstIntervFloat, 25.0, 32.0, orxTRUE, orxTRUE);
    if ((sstIntervFloat.fMin==25.0) && (sstIntervFloat.fMax==32.0) && (sstIntervFloat.u32Flags==orxINTERVALFLOAT_ALL_INCLUDED))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    orxTextIO_Print(" - Copy (orxIntervalFloat_Copy)                                 ");
    orxIntervalFloat_Copy(sstIntervFloat, &sstIntervFloatTest);
    if ((sstIntervFloatTest.fMin==25.0) && (sstIntervFloatTest.fMax==32.0) && (sstIntervFloatTest.u32Flags==orxINTERVALFLOAT_ALL_INCLUDED))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");
    
    orxTextIO_Print(" - Validation (orxIntervalFloat_Validate)                       ");
    orxIntervalFloat_Set(&sstIntervFloat, 32.0, 25.0, orxTRUE, orxTRUE);
    orxIntervalFloat_Validate(&sstIntervFloat);
    if ((sstIntervFloat.fMin==25.0) && (sstIntervFloat.fMax==32.0) && (sstIntervFloat.u32Flags==orxINTERVALFLOAT_ALL_INCLUDED))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");
    
    orxTextIO_Print(" - Swap (orxIntervalFloat_Swap)                                 ");
    orxIntervalFloat_Set(&sstIntervFloat, 25.0, 32.0, orxTRUE, orxTRUE);
    orxIntervalFloat_Set(&sstIntervFloatTest, 18.0, 24.0, orxTRUE, orxTRUE);
    orxIntervalFloat_Swap(&sstIntervFloat, &sstIntervFloatTest);
    if ((sstIntervFloat.fMin==18.0) && (sstIntervFloat.fMax==24.0) && (sstIntervFloat.u32Flags==orxINTERVALFLOAT_ALL_INCLUDED) &&
        (sstIntervFloatTest.fMin==25.0) && (sstIntervFloatTest.fMax==32.0) && (sstIntervFloatTest.u32Flags==orxINTERVALFLOAT_ALL_INCLUDED))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");
 
    orxTextIO_Print(" - Eguality (orxIntervalFloat_AreEgual)                         ");
    if ((orxIntervalFloat_AreEgual(orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE), orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE)) == orxTRUE) &&
        (orxIntervalFloat_AreEgual(orxIntervalFloat(10.0, 20.0, orxTRUE , orxFALSE), orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE)) == orxFALSE) &&
        (orxIntervalFloat_AreEgual(orxIntervalFloat(10.0, 20.0, orxTRUE , orxTRUE ), orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE)) == orxFALSE) &&
        (orxIntervalFloat_AreEgual(orxIntervalFloat(10.0, 20.0, orxFALSE, orxTRUE ), orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE)) == orxFALSE) &&
        (orxIntervalFloat_AreEgual(orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE), orxIntervalFloat(11.5, 20.0, orxFALSE, orxFALSE)) == orxFALSE) &&
        (orxIntervalFloat_AreEgual(orxIntervalFloat(10.0, 20.0, orxTRUE , orxTRUE ), orxIntervalFloat(11.5, 20.0, orxFALSE, orxFALSE)) == orxFALSE) &&
        (orxIntervalFloat_AreEgual(orxIntervalFloat(10.0, 20.0, orxFALSE, orxTRUE ), orxIntervalFloat(11.5, 20.0, orxFALSE, orxFALSE)) == orxFALSE))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");
    
    orxTextIO_Print(" - Difference (orxIntervalFloat_AreDifferent)                   ");
    if ((orxIntervalFloat_AreDifferent(orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE), orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE)) == orxFALSE) &&
        (orxIntervalFloat_AreDifferent(orxIntervalFloat(10.0, 20.0, orxTRUE , orxFALSE), orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE)) == orxTRUE) &&
        (orxIntervalFloat_AreDifferent(orxIntervalFloat(10.0, 20.0, orxTRUE , orxTRUE ), orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE)) == orxTRUE) &&
        (orxIntervalFloat_AreDifferent(orxIntervalFloat(10.0, 20.0, orxFALSE, orxTRUE ), orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE)) == orxTRUE) &&
        (orxIntervalFloat_AreDifferent(orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE), orxIntervalFloat(11.5, 20.0, orxFALSE, orxFALSE)) == orxTRUE) &&
        (orxIntervalFloat_AreDifferent(orxIntervalFloat(10.0, 20.0, orxTRUE , orxTRUE ), orxIntervalFloat(11.5, 20.0, orxFALSE, orxFALSE)) == orxTRUE) &&
        (orxIntervalFloat_AreDifferent(orxIntervalFloat(10.0, 20.0, orxFALSE, orxTRUE ), orxIntervalFloat(11.5, 20.0, orxFALSE, orxFALSE)) == orxTRUE))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    orxTextIO_Print(" - Comparison (orxIntervalFloat_IsLess)                         ");
    if ((orxIntervalFloat_IsLess(orxIntervalFloat(10.0, 20.0, orxTRUE, orxFALSE), orxIntervalFloat(30.0, 50.0, orxFALSE, orxTRUE )) == orxTRUE) &&
        (orxIntervalFloat_IsLess(orxIntervalFloat(10.0, 20.0, orxTRUE, orxFALSE), orxIntervalFloat(20.0, 50.0, orxFALSE, orxTRUE )) == orxTRUE) &&
        (orxIntervalFloat_IsLess(orxIntervalFloat(10.0, 20.0, orxTRUE, orxTRUE ), orxIntervalFloat(20.0, 50.0, orxFALSE, orxTRUE )) == orxTRUE) &&
        (orxIntervalFloat_IsLess(orxIntervalFloat(10.0, 20.0, orxTRUE, orxTRUE ), orxIntervalFloat(20.0, 50.0, orxTRUE , orxTRUE )) == orxFALSE) &&
        (orxIntervalFloat_IsLess(orxIntervalFloat(10.0, 30.0, orxTRUE, orxTRUE ), orxIntervalFloat(20.0, 50.0, orxTRUE , orxFALSE)) == orxFALSE) &&
        (orxIntervalFloat_IsLess(orxIntervalFloat(25.0, 30.0, orxTRUE, orxTRUE ), orxIntervalFloat(20.0, 50.0, orxFALSE, orxFALSE)) == orxFALSE) &&
        (orxIntervalFloat_IsLess(orxIntervalFloat(45.0, 60.0, orxTRUE, orxTRUE ), orxIntervalFloat(20.0, 50.0, orxFALSE, orxFALSE)) == orxFALSE) &&
        (orxIntervalFloat_IsLess(orxIntervalFloat(50.0, 60.0, orxTRUE, orxTRUE ), orxIntervalFloat(20.0, 50.0, orxFALSE, orxFALSE)) == orxFALSE) &&
        (orxIntervalFloat_IsLess(orxIntervalFloat(10.0, 60.0, orxTRUE, orxTRUE ), orxIntervalFloat(20.0, 50.0, orxFALSE, orxFALSE)) == orxFALSE) )
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    orxTextIO_Print(" - Comparison (orxIntervalFloat_IsLessOrBottomThrow)            ");
    if ((orxIntervalFloat_IsLessOrBottomThrow(orxIntervalFloat(10.0, 20.0, orxTRUE, orxFALSE), orxIntervalFloat(30.0, 50.0, orxFALSE, orxTRUE )) == orxTRUE ) &&
        (orxIntervalFloat_IsLessOrBottomThrow(orxIntervalFloat(10.0, 20.0, orxTRUE, orxFALSE), orxIntervalFloat(20.0, 50.0, orxFALSE, orxTRUE )) == orxTRUE ) &&
        (orxIntervalFloat_IsLessOrBottomThrow(orxIntervalFloat(10.0, 20.0, orxTRUE, orxTRUE ), orxIntervalFloat(20.0, 50.0, orxFALSE, orxTRUE )) == orxTRUE ) &&
        (orxIntervalFloat_IsLessOrBottomThrow(orxIntervalFloat(10.0, 20.0, orxTRUE, orxTRUE ), orxIntervalFloat(20.0, 50.0, orxTRUE , orxTRUE )) == orxTRUE ) &&
        (orxIntervalFloat_IsLessOrBottomThrow(orxIntervalFloat(10.0, 30.0, orxTRUE, orxTRUE ), orxIntervalFloat(20.0, 50.0, orxTRUE , orxTRUE )) == orxTRUE ) &&
        (orxIntervalFloat_IsLessOrBottomThrow(orxIntervalFloat(10.0, 50.0, orxTRUE, orxFALSE), orxIntervalFloat(20.0, 50.0, orxTRUE , orxTRUE )) == orxTRUE ) &&
        (orxIntervalFloat_IsLessOrBottomThrow(orxIntervalFloat(10.0, 50.0, orxTRUE, orxTRUE ), orxIntervalFloat(20.0, 50.0, orxTRUE , orxTRUE )) == orxFALSE) &&
        (orxIntervalFloat_IsLessOrBottomThrow(orxIntervalFloat(10.0, 50.0, orxTRUE, orxTRUE ), orxIntervalFloat(20.0, 50.0, orxTRUE , orxFALSE)) == orxFALSE) &&
        (orxIntervalFloat_IsLessOrBottomThrow(orxIntervalFloat(25.0, 30.0, orxTRUE, orxTRUE ), orxIntervalFloat(20.0, 50.0, orxFALSE, orxFALSE)) == orxFALSE) &&
        (orxIntervalFloat_IsLessOrBottomThrow(orxIntervalFloat(45.0, 60.0, orxTRUE, orxTRUE ), orxIntervalFloat(20.0, 50.0, orxFALSE, orxFALSE)) == orxFALSE) &&
        (orxIntervalFloat_IsLessOrBottomThrow(orxIntervalFloat(50.0, 60.0, orxTRUE, orxTRUE ), orxIntervalFloat(20.0, 50.0, orxFALSE, orxFALSE)) == orxFALSE) &&
        (orxIntervalFloat_IsLessOrBottomThrow(orxIntervalFloat(10.0, 60.0, orxTRUE, orxTRUE ), orxIntervalFloat(20.0, 50.0, orxFALSE, orxFALSE)) == orxFALSE) )
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    orxTextIO_Print(" - Comparison (orxIntervalFloat_IsGreater)                      ");
    if ((orxIntervalFloat_IsGreater(orxIntervalFloat(30.0, 50.0, orxFALSE, orxTRUE ), orxIntervalFloat(10.0, 20.0, orxTRUE, orxFALSE)) == orxTRUE) &&
        (orxIntervalFloat_IsGreater(orxIntervalFloat(20.0, 50.0, orxFALSE, orxTRUE ), orxIntervalFloat(10.0, 20.0, orxTRUE, orxFALSE)) == orxTRUE) &&
        (orxIntervalFloat_IsGreater(orxIntervalFloat(20.0, 50.0, orxFALSE, orxTRUE ), orxIntervalFloat(10.0, 20.0, orxTRUE, orxTRUE )) == orxTRUE) &&
        (orxIntervalFloat_IsGreater(orxIntervalFloat(20.0, 50.0, orxTRUE , orxTRUE ), orxIntervalFloat(10.0, 20.0, orxTRUE, orxTRUE )) == orxFALSE) &&
        (orxIntervalFloat_IsGreater(orxIntervalFloat(20.0, 50.0, orxTRUE , orxFALSE), orxIntervalFloat(10.0, 30.0, orxTRUE, orxTRUE )) == orxFALSE) &&
        (orxIntervalFloat_IsGreater(orxIntervalFloat(20.0, 50.0, orxFALSE, orxFALSE), orxIntervalFloat(25.0, 30.0, orxTRUE, orxTRUE )) == orxFALSE) &&
        (orxIntervalFloat_IsGreater(orxIntervalFloat(20.0, 50.0, orxFALSE, orxFALSE), orxIntervalFloat(45.0, 60.0, orxTRUE, orxTRUE )) == orxFALSE) &&
        (orxIntervalFloat_IsGreater(orxIntervalFloat(20.0, 50.0, orxFALSE, orxFALSE), orxIntervalFloat(50.0, 60.0, orxTRUE, orxTRUE )) == orxFALSE) &&
        (orxIntervalFloat_IsGreater(orxIntervalFloat(20.0, 50.0, orxFALSE, orxFALSE), orxIntervalFloat(10.0, 60.0, orxTRUE, orxTRUE )) == orxFALSE) )
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    orxTextIO_Print(" - Comparison (orxIntervalFloat_IsGreaterOrTopThrow)            ");
    if ((orxIntervalFloat_IsGreaterOrTopThrow(orxIntervalFloat(30.0, 50.0, orxFALSE, orxTRUE ), orxIntervalFloat(10.0, 20.0, orxTRUE, orxFALSE) ) == orxTRUE ) &&
        (orxIntervalFloat_IsGreaterOrTopThrow(orxIntervalFloat(20.0, 50.0, orxFALSE, orxTRUE ), orxIntervalFloat(10.0, 20.0, orxTRUE, orxFALSE) ) == orxTRUE ) &&
        (orxIntervalFloat_IsGreaterOrTopThrow(orxIntervalFloat(20.0, 50.0, orxFALSE, orxTRUE ), orxIntervalFloat(10.0, 20.0, orxTRUE, orxTRUE ) ) == orxTRUE ) &&
        (orxIntervalFloat_IsGreaterOrTopThrow(orxIntervalFloat(20.0, 50.0, orxTRUE , orxTRUE ), orxIntervalFloat(10.0, 20.0, orxTRUE, orxTRUE ) ) == orxTRUE ) &&
        (orxIntervalFloat_IsGreaterOrTopThrow(orxIntervalFloat(20.0, 50.0, orxTRUE , orxTRUE ), orxIntervalFloat(10.0, 30.0, orxTRUE, orxTRUE ) ) == orxTRUE ) &&
        (orxIntervalFloat_IsGreaterOrTopThrow(orxIntervalFloat(20.0, 50.0, orxTRUE , orxTRUE ), orxIntervalFloat(10.0, 50.0, orxTRUE, orxFALSE) ) == orxTRUE ) &&
        (orxIntervalFloat_IsGreaterOrTopThrow(orxIntervalFloat(20.0, 50.0, orxTRUE , orxTRUE ), orxIntervalFloat(10.0, 50.0, orxTRUE, orxTRUE ) ) == orxFALSE) &&
        (orxIntervalFloat_IsGreaterOrTopThrow(orxIntervalFloat(20.0, 50.0, orxTRUE , orxFALSE), orxIntervalFloat(10.0, 50.0, orxTRUE, orxTRUE ) ) == orxFALSE) &&
        (orxIntervalFloat_IsGreaterOrTopThrow(orxIntervalFloat(20.0, 50.0, orxFALSE, orxFALSE), orxIntervalFloat(25.0, 30.0, orxTRUE, orxTRUE ) ) == orxFALSE) &&
        (orxIntervalFloat_IsGreaterOrTopThrow(orxIntervalFloat(20.0, 50.0, orxFALSE, orxFALSE), orxIntervalFloat(45.0, 60.0, orxTRUE, orxTRUE ) ) == orxFALSE) &&
        (orxIntervalFloat_IsGreaterOrTopThrow(orxIntervalFloat(20.0, 50.0, orxFALSE, orxFALSE), orxIntervalFloat(50.0, 60.0, orxTRUE, orxTRUE ) ) == orxFALSE) &&
        (orxIntervalFloat_IsGreaterOrTopThrow(orxIntervalFloat(20.0, 50.0, orxFALSE, orxFALSE), orxIntervalFloat(10.0, 60.0, orxTRUE, orxTRUE ) ) == orxFALSE) )
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    orxTextIO_Print(" - Comparison (orxIntervalFloat_IsIn)                           ");
    if ((orxIntervalFloat_IsIn(orxIntervalFloat(20.0, 30.0, orxFALSE, orxFALSE), orxIntervalFloat(20.0, 30.0, orxFALSE, orxFALSE)) == orxTRUE ) &&
        (orxIntervalFloat_IsIn(orxIntervalFloat(20.0, 30.0, orxFALSE, orxTRUE ), orxIntervalFloat(20.0, 30.0, orxFALSE, orxTRUE )) == orxTRUE ) &&
        (orxIntervalFloat_IsIn(orxIntervalFloat(20.0, 30.0, orxTRUE , orxFALSE), orxIntervalFloat(20.0, 30.0, orxTRUE , orxFALSE)) == orxTRUE ) &&
        (orxIntervalFloat_IsIn(orxIntervalFloat(20.0, 30.0, orxTRUE , orxTRUE ), orxIntervalFloat(20.0, 30.0, orxTRUE , orxTRUE )) == orxTRUE ) &&
        (orxIntervalFloat_IsIn(orxIntervalFloat(20.0, 30.0, orxFALSE, orxFALSE), orxIntervalFloat(20.0, 30.0, orxTRUE , orxTRUE )) == orxTRUE ) &&
        (orxIntervalFloat_IsIn(orxIntervalFloat(20.0, 30.0, orxTRUE , orxTRUE ), orxIntervalFloat(20.0, 30.0, orxFALSE, orxFALSE)) == orxFALSE) &&
        (orxIntervalFloat_IsIn(orxIntervalFloat(20.0, 32.0, orxTRUE , orxTRUE ), orxIntervalFloat(20.0, 30.0, orxFALSE, orxFALSE)) == orxFALSE) &&
        (orxIntervalFloat_IsIn(orxIntervalFloat(12.0, 18.0, orxTRUE , orxTRUE ), orxIntervalFloat(20.0, 30.0, orxFALSE, orxFALSE)) == orxFALSE) )
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    orxTextIO_Print(" - Search (orxIntervalFloat_HasValue)                           ");
    if ((orxIntervalFloat_HasValue(orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE), 05.0) == orxFALSE) && 
        (orxIntervalFloat_HasValue(orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE), 10.0) == orxFALSE) && 
        (orxIntervalFloat_HasValue(orxIntervalFloat(10.0, 20.0, orxTRUE , orxFALSE), 10.0) == orxTRUE ) && 
        (orxIntervalFloat_HasValue(orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE), 15.0) == orxTRUE ) && 
        (orxIntervalFloat_HasValue(orxIntervalFloat(10.0, 20.0, orxFALSE, orxTRUE ), 20.0) == orxTRUE ) && 
        (orxIntervalFloat_HasValue(orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE), 20.0) == orxFALSE) && 
        (orxIntervalFloat_HasValue(orxIntervalFloat(10.0, 20.0, orxFALSE, orxFALSE), 25.0) == orxFALSE))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    orxTextIO_Print(" - Extand (orxIntervalFloat_Extand) - test 01                   ");
    orxIntervalFloat_Set(&sstIntervFloat, 10.0, 20.0, orxFALSE, orxFALSE);
    orxIntervalFloat_Extand(&sstIntervFloat, 20.0, orxTRUE);
    if ((sstIntervFloat.fMin==10)&&(sstIntervFloat.fMax==20)&&(sstIntervFloat.u32Flags==orxINTERVALFLOAT_MAX_INCLUDED))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    orxTextIO_Print(" - Extand (orxIntervalFloat_Extand) - test 02                   ");
    orxIntervalFloat_Set(&sstIntervFloat, 10.0, 20.0, orxFALSE, orxFALSE);
    orxIntervalFloat_Extand(&sstIntervFloat, 10.0, orxTRUE);
    if ((sstIntervFloat.fMin==10)&&(sstIntervFloat.fMax==20)&&(sstIntervFloat.u32Flags==orxINTERVALFLOAT_MIN_INCLUDED))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    orxTextIO_Print(" - Extand (orxIntervalFloat_Extand) - test 03                   ");
    orxIntervalFloat_Set(&sstIntervFloat, 10.0, 20.0, orxFALSE, orxFALSE);
    orxIntervalFloat_Extand(&sstIntervFloat, 20.0, orxFALSE);
    if ((sstIntervFloat.fMin==10)&&(sstIntervFloat.fMax==20)&&(sstIntervFloat.u32Flags==orxINTERVALFLOAT_ALL_EXCLUDED))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    orxTextIO_Print(" - Extand (orxIntervalFloat_Extand) - test 04                   ");
    orxIntervalFloat_Set(&sstIntervFloat, 10.0, 20.0, orxFALSE, orxFALSE);
    orxIntervalFloat_Extand(&sstIntervFloat, 10.0, orxFALSE);
    if ((sstIntervFloat.fMin==10)&&(sstIntervFloat.fMax==20)&&(sstIntervFloat.u32Flags==orxINTERVALFLOAT_ALL_EXCLUDED))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    orxTextIO_Print(" - Extand (orxIntervalFloat_Extand) - test 05                   ");
    orxIntervalFloat_Set(&sstIntervFloat, 10.0, 20.0, orxFALSE, orxFALSE);
    orxIntervalFloat_Extand(&sstIntervFloat, 30.0, orxTRUE);
    if ((sstIntervFloat.fMin==10)&&(sstIntervFloat.fMax==30)&&(sstIntervFloat.u32Flags==orxINTERVALFLOAT_MAX_INCLUDED))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

    orxTextIO_Print(" - Extand (orxIntervalFloat_Extand) - test 06                   ");
    orxIntervalFloat_Set(&sstIntervFloat, 10.0, 20.0, orxFALSE, orxTRUE);
    orxIntervalFloat_Extand(&sstIntervFloat, -5.0, orxFALSE);
    if ((sstIntervFloat.fMin==-5.0)&&(sstIntervFloat.fMax==20)&&(sstIntervFloat.u32Flags==orxINTERVALFLOAT_MAX_INCLUDED))
        orxTextIO_PrintLn(" OK  ");
    else
        orxTextIO_PrintLn("ERROR");

}



/******************************************************
 * DYNAMIC LIBRARY ENTRY POINTS
 ******************************************************/
orxVOID orxTest_MathSet_Init()
{
  /* Initialize module */
  orxModule_Init(orxMODULE_ID_MATHSET);

  /* NB: Here, we should have a call to orxMAIN_INIT_MODULE(MathSet) But it seems that
   * MathSet module doesn't respects the coding style for function naming convention
   * and initialization : change it later
   */
  
  /* Register test functions */
  orxTest_Register("MathSet", "Display module informations", orxTest_MathSet_Infos);
  orxTest_Register("MathSet", "Auto-test intervals", orxTest_MathSet_AutoTestIntervals);
  orxTest_Register("MathSet", "Clear all objects", orxTest_MathSet_Reset);
  orxTest_Register("MathSet", "Display content of tested intervals and sets", orxTest_MathSet_DisplayContent);
  orxTest_Register("MathSet", "Set float-based interval", orxTest_MathSet_InputFloatInterval);
  orxTest_Register("MathSet", "Set integer-based interval", orxTest_MathSet_InputInt32Interval);
}

orxVOID orxTest_MathSet_Exit()
{
}

orxTEST_DEFINE_ENTRY_POINT(orxTest_MathSet_Init, orxTest_MathSet_Exit)

