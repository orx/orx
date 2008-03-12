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
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/

#include "io/orxPackage.h"
#include "debug/orxDebug.h"


/***************************************************************************
 orxPackage_Setup
 Package module setup.

 returns: nothing
 ***************************************************************************/
orxVOID orxPackage_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_PACKAGE, orxMODULE_ID_PLUGIN);
  orxModule_AddDependency(orxMODULE_ID_PACKAGE, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_PACKAGE, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_PACKAGE, orxMODULE_ID_LINKLIST);
  orxModule_AddDependency(orxMODULE_ID_PACKAGE, orxMODULE_ID_TREE);

  return;
}


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


/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_Init,      orxSTATUS);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_Exit,      orxVOID);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_Close,     orxSTATUS,    orxPACKAGE *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_FindNext,  orxBOOL,      orxPACKAGE_INFOS *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_FindClose, orxVOID,      orxPACKAGE_INFOS *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_TestFlags, orxBOOL,      orxPACKAGE *,       orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_Commit,    orxSTATUS,    orxPACKAGE *,       orxCONST orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_Extract,   orxSTATUS,    orxPACKAGE *,       orxCONST orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_Open,      orxPACKAGE *, orxCONST orxSTRING, orxCONST orxSTRING,   orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_SetFlags,  orxVOID,      orxPACKAGE *,       orxU32,               orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_FindFirst, orxBOOL,      orxPACKAGE *,       orxCONST orxSTRING,   orxPACKAGE_INFOS *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_Read,      orxU32,       orxVOID *,          orxU32,               orxPACKAGE *,      orxCONST orxSTRING);


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/


/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/
