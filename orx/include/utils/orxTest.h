/**
 * @file orxTest.h
 * 
 * Test Module - Offers a basic interface to display a menu of registered test functions
 * 
 * @todo Improve the system with different sub-menu levels
 * @note Maybe that the system could be improved allowing to add other types of functions
 * @note Needs may appears with each module needs.
 * 
 * @note At the moment, each test modules will be staticly linked inside the Test program. (first version)
 * 
 * @note A good extension would be to only put test programs in the test directory.
 * @note Makefile can detect and Build it without modification. Each modules are compiled
 * @note in dynamic libraries. All dynamic libraries present in the test directory are
 * @note automatically used by the Main Test program.
 */
 
 /***************************************************************************
 orxTest.h
 Test Module
 
 begin                : 08/04/2005
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

#ifndef _orxTEST_H_
#define _orxTEST_H_

#include "orxInclude.h"

typedef orxVOID (*orxTEST_FUNCTIONCB)(orxVOID);

/** Initialize the test module
 */
extern orxSTATUS orxTest_Init();

/** Uninitialize the test module
 */
extern orxVOID orxTest_Exit();

/** Register a new function
 * @param   (IN)  _zModuleName      Name of the module (to group a list of functions)
 * @param   (IN)  _zMenuEntry       Text displayed to describe the test function
 * @param   (IN)  _cbFunction       Function executed when the menu entry is selected
 * @return Returns an Handle on the function
 */
extern orxHANDLE orxTest_Register(orxCONST orxSTRING _zModuleName, orxCONST orxSTRING _zMenuEntry, orxTEST_FUNCTIONCB _cbFunction);

/** Execute a registered function
 * @param   (IN)  _hRegisteredFunc  Handle of the registered function to execute
 * @return  Returns the success / fail status (failed when the Handle is unknown)
 */
extern orxSTATUS orxTest_Execute(orxHANDLE _hRegisteredFunc);

/** Display a Menu with registered function list
 */
extern orxVOID orxTestDisplayMenu();

#endif /* _orxTEST_H_ */
