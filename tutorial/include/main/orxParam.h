/* Orx - Portable Game Engine
 *
 * Orx is the legal property of its developers, whose names
 * are listed in the COPYRIGHT file distributed 
 * with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
 * @return Returns orxSTATUS_SUCCESS if informations read are correct, orxSTATUS_FAILURE if a problem has occured
 */
typedef orxSTATUS (orxFASTCALL *orxPARAM_INIT_FUNCTION)(orxU32 _u32NbParam, const orxSTRING _azParams[]);

/* Parameter's info */
typedef struct __orxPARAM_t
{
  /* Flags associated to the parameters' parser */
  orxU32                  u32Flags;
  
  /* Short parameter name (ex: "h" for help (will be -h)) */
  orxSTRING               zShortName;

  /* Long parameter name (ex: "help" for help (will be --help)) */
  orxSTRING               zLongName;

  /* Short description (ex: "shows params' list") */
  orxSTRING               zShortDesc;

  /* Long description (ex: "Display the list of parameters.
   * -help <param> displays the <param> long description") */
  orxSTRING               zLongDesc;
  
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
 * @param[in] _pstParam Informations about the option to register
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
