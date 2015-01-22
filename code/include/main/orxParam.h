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
 * @file orxParam.h
 * @date 09/09/2005
 * @author bestel@arcallians.org
 *
 * @todo
 */

/**
 * @addtogroup orxParam
 *
 * Param module
 * Module that handles command line parameters
 *
 * @{
 */


#ifndef _orxPARAM_H_
#define _orxPARAM_H_


#include "orxInclude.h"


/* Macro definition */

/* Flags definitions */
#define orxPARAM_KU32_FLAG_NONE             0x00000000 /**< No flags (default behaviour) */
#define orxPARAM_KU32_FLAG_MULTIPLE_ALLOWED 0x00000001 /**< It's possible to have multiple instance of the same param */
#define orxPARAM_KU32_FLAG_STOP_ON_ERROR    0x00000002 /**< If the param extra flag parsing fails, stop the engine */

/** Callback definition (called when a parameter has been found
 * @param[in] _u32NbParam Number of extra parameters read for this option
 * @param[in] _azParams   Array of extra parameters (the first one is always the option name)
 * @return Returns orxSTATUS_SUCCESS if read information is correct, orxSTATUS_FAILURE if a problem occurred
 */
typedef orxSTATUS (orxFASTCALL *orxPARAM_INIT_FUNCTION)(orxU32 _u32NbParam, const orxSTRING _azParams[]);

/* Parameter's info */
typedef struct __orxPARAM_t
{
  /* Flags associated to the parameters' parser */
  orxU32                  u32Flags;

  /* Short parameter name (ex: "h" for help (will be -h)) */
  const orxSTRING         zShortName;

  /* Long parameter name (ex: "help" for help (will be --help)) */
  const orxSTRING         zLongName;

  /* Short description (ex: "shows params' list") */
  const orxSTRING         zShortDesc;

  /* Long description (ex: "Display the list of parameters.
   * -help <param> displays the <param> long description") */
  const orxSTRING         zLongDesc;

  /* Function called when the parameter is detected in the command line */
  orxPARAM_INIT_FUNCTION  pfnParser;

} orxPARAM;


/** Setups param module
 */
extern orxDLLAPI void orxFASTCALL       orxParam_Setup();

/** Inits the param Module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxParam_Init();

/** Exits from the param module
 */
extern orxDLLAPI void orxFASTCALL       orxParam_Exit();


/** Registers a new parameter
 * @param[in] _pstParam Information about the option to register
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxParam_Register(const orxPARAM *_pstParam);

/** Displays help if requested
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxParam_DisplayHelp();

/** Sets the command line arguments
 * @param[in] _u32NbParams  Number of read parameters
 * @param[in] _azParams     List of parameters
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxParam_SetArgs(orxU32 _u32NbParams, orxSTRING _azParams[]);

#endif /* _orxPARAM_H_ */

/** @} */
