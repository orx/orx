/**
 * @file orxPlugin_File.h
 * 
 * Module to define file plugin registration ID
 */ 
 
 /***************************************************************************
 orxPlugin_File.h
 
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


#ifndef _orxPLUGIN_FILE_H_
#define _orxPLUGIN_FILE_H_

/*********************************************
 Constants
 *********************************************/

#define orxPLUGIN_FILE_KU32_PLUGIN_ID              0x10000600

#define orxPLUGIN_FILE_KU32_FUNCTION_NUMBER        0x0000000B

#define orxPLUGIN_FILE_KU32_ID_INIT                0x10000600
#define orxPLUGIN_FILE_KU32_ID_EXIT                0x10000601
#define orxPLUGIN_FILE_KU32_ID_FIND_FIRST          0x10000602
#define orxPLUGIN_FILE_KU32_ID_FIND_NEXT           0x10000603
#define orxPLUGIN_FILE_KU32_ID_FIND_CLOSE          0x10000604
#define orxPLUGIN_FILE_KU32_ID_INFOS               0x10000605
#define orxPLUGIN_FILE_KU32_ID_COPY                0x10000606
#define orxPLUGIN_FILE_KU32_ID_RENAME              0x10000607
#define orxPLUGIN_FILE_KU32_ID_DELETE              0x10000608
#define orxPLUGIN_FILE_KU32_ID_DIR_CREATE          0x10000609
#define orxPLUGIN_FILE_KU32_ID_DIR_DELETE          0x1000060A

#endif /* _orxPLUGIN_FILE_H_ */
