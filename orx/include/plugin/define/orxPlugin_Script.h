/**
 * @file orxPlugin_Script.h
 * 
 * Module to define script plugin registration ID
 */ 
 
 /***************************************************************************
 orxPlugin_Script.h
 
 begin                : 03/05/2005
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


#ifndef _orxPLUGIN_SCRIPT_H_
#define _orxPLUGIN_SCRIPT_H_

/*********************************************
 Constants
 *********************************************/

#define orxPLUGIN_SCRIPT_KU32_PLUGIN_ID              0x10000800

#define orxPLUGIN_SCRIPT_KU32_FUNCTION_NUMBER        0x00000007

#define orxPLUGIN_SCRIPT_KU32_ID_INIT                0x10000600
#define orxPLUGIN_SCRIPT_KU32_ID_EXIT                0x10000601
#define orxPLUGIN_SCRIPT_KU32_ID_CREATE              0x10000602
#define orxPLUGIN_SCRIPT_KU32_ID_DELETE              0x10000603
#define orxPLUGIN_SCRIPT_KU32_ID_LOAD_FILE           0x10000604
#define orxPLUGIN_SCRIPT_KU32_ID_CALL_FUNC           0x10000605
#define orxPLUGIN_SCRIPT_KU32_ID_EXECUTE             0x10000606

#endif /* _orxPLUGIN_SCRIPT_H_ */
