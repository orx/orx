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
 * @file orx.h
 * @date 02/09/2005
 * @author
 *
 * @todo
 */

/**
 * @addtogroup Orx
 *
 * Main orx include
 *
 * @{
 */


#ifndef _orx_H_
#define _orx_H_


#include "orxInclude.h"

#include "orxKernel.h"

#include "orxUtils.h"


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Should stop execution by default event handling?
 */
orxSTATIC orxBOOL sbStopByEvent = 0;


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Orx default basic event handler
 * @param[in]   _pstEvent                     Sent event
 * @return      orxSTATUS_SUCCESS if handled / orxSTATUS_FAILURE otherwise
 */
orxSTATIC orxSTATUS orxFASTCALL orx_DefaultEventHandler(orxCONST orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(_pstEvent->eType == orxEVENT_TYPE_SYSTEM);

  /* Depending on event ID */
  switch(_pstEvent->eID)
  {
    /* Close event */
    case orxSYSTEM_EVENT_CLOSE:
    {
      /* Updates status */
      sbStopByEvent = orxTRUE;

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;

      break;
    }

    default:
    {
      break;
    }
  }

  /* Done! */
  return eResult;
}

/** Orx main execution function
 * @param[in]   _u32NbParams                  Main function parameters number (argc)
 * @param[in]   _azParams                     Main function parameter list (argv)
 * @param[in]   _pfnSetup                     Main setup function (should declares dependencies)
 * @param[in]   _pfnInit                      Main init function (should init all the main stuff and register the main event handler to override the default one)
 * @param[in]   _pfnRun                       Main run function (will be called once per frame, should return orxSTATUS_SUCCESS to continue processing)
 * @param[in]   _pfnExit                      Main exit function (should clean all the main stuff)
 */
orxSTATIC orxINLINE orxVOID orx_Execute(orxU32 _u32NbParams, orxSTRING _azParams[], orxCONST orxMODULE_SETUP_FUNCTION _pfnSetup, orxCONST orxMODULE_INIT_FUNCTION _pfnInit, orxCONST orxMODULE_RUN_FUNCTION _pfnRun, orxCONST orxMODULE_EXIT_FUNCTION _pfnExit)
{
  /* Inits the Debug System */
  orxDEBUG_INIT();

  /* Checks */
  orxASSERT(_u32NbParams > 0);
  orxASSERT(_azParams != orxNULL);
  orxASSERT(_pfnSetup != orxNULL);
  orxASSERT(_pfnInit != orxNULL);
  orxASSERT(_pfnRun != orxNULL);
  orxASSERT(_pfnExit != orxNULL);

  /* Registers main module */
  orxModule_Register(orxMODULE_ID_MAIN, _pfnSetup, _pfnInit, _pfnExit);

  /* Registers all other modules */
  orxModule_RegisterAll();

  /* Calls all modules setup */
  orxModule_SetupAll();

  /* Sends the command line arguments to orxParam module */
  if(orxParam_SetArgs(_u32NbParams, _azParams) == orxSTATUS_SUCCESS)
  {
    /* Inits the engine */
    if(orxModule_Init(orxMODULE_ID_MAIN) == orxSTATUS_SUCCESS)
    {
      orxSTATUS eClockStatus, eMainStatus;
      orxBOOL   bStop;

      /* Registers default event handler */
      orxEvent_AddHandler(orxEVENT_TYPE_SYSTEM, orx_DefaultEventHandler);

      /* Main loop */
      for(bStop = orxFALSE;
          bStop == orxFALSE;
          bStop = ((sbStopByEvent != orxFALSE) || (eMainStatus == orxSTATUS_FAILURE) || (eClockStatus == orxSTATUS_FAILURE)) ? orxTRUE : orxFALSE)
      {
        /* Runs the engine */
        eMainStatus = _pfnRun();

        /* Updates clock system */
        eClockStatus = orxClock_Update();
      }

      /* Exits from engine */
      orxModule_Exit(orxMODULE_ID_MAIN);
    }

    /* Exits from all modules */
    orxModule_ExitAll();
  }

  /* Exits from the Debug system */
  orxDEBUG_EXIT();
}

#endif /*_orx_H_*/

/** @} */
