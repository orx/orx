/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/**
 * @file
 * @date 11/07/2005
 * @author (C) Arcallians
 * 
 */

/**
 * @addtogroup Main
 * 
 * Header for the main module. The main module will run the main loop
 * and hold the init system.
 * @{
 */
 
#ifndef _orxMAIN_H_
#define _orxMAIN_H_

#include "orxInclude.h"

/** WARNING : This enum definition doesn't respect the standard coding style.
 * The complete enum name should use upper case but this excepetion is needed
 * to have an automatic working init/exit macro system.
 */
typedef enum __orxMAIN_MODULE_t
{
  orxMAIN_MODULE_Memory = 0,
  orxMAIN_MODULE_Bank,

  orxMAIN_MODULE_NUMBER,
  orxMAIN_MODULE_NONE = 0xFFFFFFFF
} orxMAIN_MODULE;

/** Initialize the main module (will initialize all needed modules)
 */
extern orxSTATUS orxDLLAPI orxMain_Init();

/** Exit bank module
 */
extern orxVOID orxDLLAPI orxMain_Exit();

#define orxMAIN_INIT_MODULE(ModuleName)
{ \
  /* TODO : Check the current module status, call the init function 
   * if needed, store the init status and increment the ref counter
   */
}

#endif /*_orxMAIN_H_*/

/** @} */
