/**
 * @file orxPackage.c
 * 
 * Module for file / directory management.
 * 
 * @todo Maybe find a way to be completly independent of hard disks name and directory seperator
 */ 
 
 /***************************************************************************
 orxPackage.c
 Package management
 
 begin                : 01/05/2005
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

#include "io/orxPackage.h"
#include "debug/orxDebug.h"

/********************
 *  Plugin Related  *
 ********************/
orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(PACKAGE)
 
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PACKAGE, INIT, orxPackage_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PACKAGE, EXIT, orxPackage_Exit)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PACKAGE, OPEN, orxPackage_Open)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PACKAGE, CLOSE, orxPackage_Close)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PACKAGE, SET_FLAGS, orxPackage_SetFlags)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PACKAGE, TEST_FLAGS, orxPackage_TestFlags)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PACKAGE, COMMIT, orxPackage_Commit)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PACKAGE, EXTRACT, orxPackage_Extract)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PACKAGE, FIND_FIRST, orxPackage_FindFirst)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PACKAGE, FIND_NEXT, orxPackage_FindNext)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PACKAGE, FIND_CLOSE, orxPackage_FindClose)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PACKAGE, READ, orxPackage_Read)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(PACKAGE)

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

struct __orxPACKAGE_t
{
};

/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/
