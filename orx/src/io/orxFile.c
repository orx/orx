/**
 * @file orxFile.c
 * 
 * Module for file / directory management.
 * 
 * @todo Maybe find a way to be completly independent of hard disks name and directory seperator
 */ 
 
 /***************************************************************************
 orxFile.c
 File / Directory management
 
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

#include "io/orxFile.h"
#include "debug/orxDebug.h"
#include "plugin/orxPluginCore.h"

/********************
 *  Plugin Related  *
 ********************/
 
/***************************************************************************
 orxFile_Setup
 File module setup.

 returns: nothing
 ***************************************************************************/
orxVOID orxFile_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_FILE, orxMODULE_ID_PLUGIN);
  orxModule_AddDependency(orxMODULE_ID_FILE, orxMODULE_ID_MEMORY);

  return;
}


/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(FILE)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILE, INIT, orxFile_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILE, EXIT, orxFile_Exit)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILE, OPEN, orxFile_Open)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILE, READ, orxFile_Read)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILE, WRITE, orxFile_Write)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILE, GETS, orxFile_Gets)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILE, CLOSE, orxFile_Close)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(FILE)


/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION(orxFile_Init, orxSTATUS);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxFile_Exit, orxVOID);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxFile_Open, orxFILE*, orxCONST orxSTRING, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxFile_Read, orxU32, orxVOID*, orxU32, orxU32, orxFILE*);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxFile_Write, orxU32, orxVOID*, orxU32, orxU32, orxFILE*);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxFile_Gets, orxSTATUS, orxSTRING, orxU32, orxFILE*);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxFile_Close, orxSTATUS, orxFILE*);


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/
