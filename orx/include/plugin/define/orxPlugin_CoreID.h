/**
 * @file orxPlugin_CoreID.h
 * 
 * Module to define all core plugin IDs.
 */ 
 
 /***************************************************************************
 orxPlugin_CoreID.h
 
 begin                : 02/05/2005
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef _orxPLUGIN_COREID_H_
#define _orxPLUGIN_COREID_H_


/* *** Core plugin id enum *** */
typedef enum __orxPLUGIN_CORE_ID_t
{
  orxPLUGIN_CORE_ID_DISPLAY = 0,
  orxPLUGIN_CORE_ID_FILE,
  orxPLUGIN_CORE_ID_FILESYSTEM,
  orxPLUGIN_CORE_ID_JOYSTICK,
  orxPLUGIN_CORE_ID_KEYBOARD,
  orxPLUGIN_CORE_ID_MOUSE,
  orxPLUGIN_CORE_ID_PACKAGE,
  orxPLUGIN_CORE_ID_SCRIPT,
  orxPLUGIN_CORE_ID_SOUND,
  orxPLUGIN_CORE_ID_TIME,

  orxPLUGIN_CORE_ID_NUMBER,

  orxPLUGIN_CORE_ID_NONE = orxENUM_NONE
    
} orxPLUGIN_CORE_ID;


#endif /* _orxPLUGIN_COREID_H_ */
