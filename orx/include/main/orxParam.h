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
 * @date 09/09/2005
 * @author (C) Arcallians
 * 
 * @todo 
 */

/**
 * @addtogroup Param
 * 
 * Module dedicated to parameters analysis and generation
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
typedef orxSTATUS (*orxPARAM_KU32_FLAG_FUNCTION)(orxU32 _u32NbParam, orxSTRING _azParams[]);
 
/* Parameter's infos */
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
  orxPARAM_KU32_FLAG_FUNCTION pfnParser;
  
} orxPARAM;


/** Clock module setup
 */
extern orxDLLAPI orxVOID                orxParam_Setup();
/** Initialize Param Module
 */
extern orxDLLAPI orxSTATUS              orxParam_Init();

/** Exit Param module
 */
extern orxDLLAPI orxVOID                orxParam_Exit();

/** Register a new parameter
 * @param[in] _pstParam Informations about the option to register
 * @return Returns the status of the registration
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxParam_Register(orxCONST orxPARAM *_pstParam);

/** Parse the command line
 * @param[in] _u32NbParam Number of read parameters
 * @param[in] _azParams   List of parameters
 * @return Returns the parsing status
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxParam_Parse(orxU32 _u32NbParams, orxCONST orxSTRING _azParams[]);


#endif /* _orxPARAM_H_ */

/** @} */
