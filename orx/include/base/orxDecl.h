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

  #define orxFASTCALL           __fastcall
  
  /* The function will be exported (dll compilation) */
  #define orxEXPORT             __declspec(dllexport)
  
  /* The function will be imported (exe comoilation) */
  #define orxIMPORT             __declspec(dllimport)
  
  /* The function will not be exported nor imported */
  #define orxLOCAL


  #define orxCONST              const
  #define orxSTATIC             static
  #define orxINLINE             inline
  #define orxREGISTER           register

  #define orxNULL               NULL

#else /* WINDOWS */

  /* Linux */
  #ifdef LINUX

    #define orxFASTCALL         fastcall
    
    /* The function will be exported (dll compilation) */
    #define orxEXPORT           __attribute__ ((visibility("default")))
  
    /* The function will be imported (exe comoilation) */
    #define orxIMPORT
  
    /* The function will not be exported nor imported */
    #define orxLOCAL            __attribute__ ((visibility("hidden")))

    
    #define orxCONST            const
    #define orxSTATIC           static
    #define orxINLINE           inline
    #define orxREGISTER         register

    #define orxNULL             NULL

  #endif /* LINUX */
  
#endif /* WINDOWS */  

/* DLL? */
#ifdef orxDLL          /* orx compiled as a dynamic library */
  #ifdef orxDLLEXPORT  /* export functions (orx.dll compilation) */
    #define orxDLLAPI orxEXPORT
  #else                /* no orxDLLEXPORT */
    #define orxDLLAPI orxIMPORT
  #endif               /* end orxDLLEXPORT */
#else                  /* no orxDLL */
  #define orxDLLAPI
#endif                 /* end orxDLL */


#endif /*_orxDECL_H_*/
