/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2012 Orx-Project
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
 * @file orxCommand.h
 * @date 29/04/2012
 * @author iarwain@orx-project.org
 *
 */

/**
 * @addtogroup orxCommand
 *
 * Command module
 * Module that can execute custom registered commands.
 * @{
 */

#ifndef _orxCOMMAND_H_
#define _orxCOMMAND_H_


#include "orxInclude.h"
#include "utils/orxString.h"


/** Event enum
 */
typedef enum __orxCOMMAND_EVENT_t
{
  orxCOMMAND_EVENT_REGISTER = 0,                      /**< Event sent when a command is registerd */
  orxCOMMAND_EVENT_UNREGISTER,                        /**< Event sent when a command is unregistered */
  orxCOMMAND_EVENT_EXECUTE,                           /**< Event sent when a command is executed */

  orxCOMMAND_EVENT_NUMBER,

  orxCOMMAND_EVENT_NONE = orxENUM_NONE

} orxCOMMAND_EVENT;


/** Command module setup
 */
extern orxDLLAPI void orxFASTCALL                     orxCommand_Setup();

/** Inits the command module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxCommand_Init();

/** Exits from the command module
 */
extern orxDLLAPI void orxFASTCALL                     orxCommand_Exit();


///** Registers a command
// * @param[in]   _zCommand     Command name
// * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
// */
//extern orxDLLAPI orxSTATUS orxFASTCALL                orxCommand_Register(const orxSTRING _zCommand);
//
///** Unregisters a command
// * @param[in]   _zCommand     Command name
// * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
// */
//extern orxDLLAPI orxCOMMAND *orxFASTCALL              orxCommand_Unregister(const orxSTRING _zCommand);
//
///** Is a command registered?
// * @param[in]   _zCommand     Command name
// * @return      orxTRUE / orxFALSE
// */
//extern orxDLLAPI orxBOOL orxFASTCALL                  orxCommand_IsRegistered(const orxSTRING _zCommand);
//
//
///** Executes a command
// * @param[in]   _zCommand     Command name
// * @return      Command result (orxSTRING) if found, orxNULL otherwise
// */
//extern orxDLLAPI const orxSTRING orxFASTCALL          orxCommand_Execute(const orxSTRING _zCommand);
//
//
///** Gets command prototype
// * @param[in]   _zCommand     Command name
// * @return      orxCOMMAND_DEFINITION / orxNULL
// */
//extern orxDLLAPI const orxSTRING orxFASTCALL          orxCommand_GetPrototype(const orxSTRING _zCommand);
//
///** Gets next command using the base name + last found command (handy for autocompletion and for listing all the commands)
// * @param[in]   _zCommand     orxNULL to find the first command, last found command or beginning of a command name otherwise
// * @return      Next command name if found, orxNULL otherwise
// */
//extern orxDLLAPI const orxSTRING orxFASTCALL          orxCommand_GetNext(const orxSTRING _zCommand);


#endif /* _orxCOMMAND_H_ */

/** @} */
