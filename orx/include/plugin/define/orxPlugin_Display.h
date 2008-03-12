/**
 * \file orxPlugin_Display.h
 * This header is used to define ID for display plugin registration.
 */

/*
 begin                : 23/04/2003
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 */

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/

#ifndef _orxPLUGIN_DISPLAY_H_
#define _orxPLUGIN_DISPLAY_H_

#include "orxPlugin_CoreID.h"


/*********************************************
 Constants
 *********************************************/

typedef enum __orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_t
{
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_INIT = 0,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_EXIT,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_SWAP,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_CREATE_BITMAP,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_DELETE_BITMAP,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_SAVE_BITMAP,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_LOAD_BITMAP,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_TRANSFORM_BITMAP,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_CLEAR_BITMAP,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_BLIT_BITMAP,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_SET_BITMAP_COLOR_KEY,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_SET_BITMAP_CLIPPING,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_GET_BITMAP_SIZE,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_GET_SCREEN_BITMAP,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_DRAW_TEXT,
  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_GET_APPLICATION_INPUT,

  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_NUMBER,

  orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_NONE = orxENUM_NONE
    
} orxPLUGIN_FUNCTION_BASE_ID_DISPLAY;


#endif /* _orxPLUGIN_DISPLAY_H_ */
