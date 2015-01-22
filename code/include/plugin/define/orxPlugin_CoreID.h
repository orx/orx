/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2015 Orx-Project
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 */

/**
 * @file orxPlugin_CoreID.h
 * @date 02/05/2005
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxPlugin
 * 
 * Header that defines all the plugin IDs
 *
 * @{
 */


#ifndef _orxPLUGIN_COREID_H_
#define _orxPLUGIN_COREID_H_


/* *** Core plugin id enum *** */
typedef enum __orxPLUGIN_CORE_ID_t
{
  orxPLUGIN_CORE_ID_DISPLAY = 0,
  orxPLUGIN_CORE_ID_JOYSTICK,
  orxPLUGIN_CORE_ID_KEYBOARD,
  orxPLUGIN_CORE_ID_MOUSE,
  orxPLUGIN_CORE_ID_PHYSICS,
  orxPLUGIN_CORE_ID_RENDER,
  orxPLUGIN_CORE_ID_SOUNDSYSTEM,

  orxPLUGIN_CORE_ID_NUMBER,

  orxPLUGIN_CORE_ID_NONE = orxENUM_NONE
    
} orxPLUGIN_CORE_ID;

#endif /* _orxPLUGIN_COREID_H_ */

/** @} */
