/**
 * @file orxPlugin_Package.h
 * 
 * Module to define package plugin registration ID
 */ 
 
 /***************************************************************************
 orxPlugin_Package.h
 
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


#ifndef _orxPLUGIN_PACKAGE_H_
#define _orxPLUGIN_PACKAGE_H_

/*********************************************
 Constants
 *********************************************/

#define orxPLUGIN_PACKAGE_KU32_PLUGIN_ID              0x10000700

#define orxPLUGIN_PACKAGE_KU32_FUNCTION_NUMBER        0x0000000C

#define orxPLUGIN_PACKAGE_KU32_ID_INIT                0x10000700
#define orxPLUGIN_PACKAGE_KU32_ID_EXIT                0x10000701
#define orxPLUGIN_PACKAGE_KU32_ID_OPEN                0x10000702
#define orxPLUGIN_PACKAGE_KU32_ID_CLOSE               0x10000703
#define orxPLUGIN_PACKAGE_KU32_ID_SET_FLAGS           0x10000704
#define orxPLUGIN_PACKAGE_KU32_ID_TEST_FLAGS          0x10000705
#define orxPLUGIN_PACKAGE_KU32_ID_COMMIT              0x10000706
#define orxPLUGIN_PACKAGE_KU32_ID_EXTRACT             0x10000707
#define orxPLUGIN_PACKAGE_KU32_ID_FIND_FIRST          0x10000708
#define orxPLUGIN_PACKAGE_KU32_ID_FIND_NEXT           0x10000709
#define orxPLUGIN_PACKAGE_KU32_ID_FIND_CLOSE          0x1000070A
#define orxPLUGIN_PACKAGE_KU32_ID_READ                0x1000070B

#endif /* _orxPLUGIN_PACKAGE_H_ */
