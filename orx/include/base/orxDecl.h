/** 
 * \file orxDecl.h
 * 
 * Declarations.
 * Defines all base declarations.
 * 
 * \todo
 * Add the required declarations at need.
 */


/***************************************************************************
 orxDecl.h
 Base declarations
 
 begin                : 14/04/2005
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


#ifndef _orxDECL_H_
#define _orxDECL_H_

/* *** Platform depedant base declarations */

/* Windows */
#ifdef WINDOWS

  #define orxFASTCALL         __fastcall

  #define orxCONST            const
  #define orxSTATIC           static
  #define orxINLINE           inline
  #define orxREGISTER         register

  #define orxNULL             NULL

#else /* WINDOWS */

  /* Linux */
  #ifdef LINUX

    #define orxFASTCALL
    
    #define orxCONST            const
    #define orxSTATIC           static
    #define orxINLINE           inline
    #define orxREGISTER         register

    #define orxNULL             NULL

  #endif /* LINUX */
  
#endif /* WINDOWS */  


#endif /*_orxDECL_H_*/
