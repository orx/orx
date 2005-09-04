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
#ifdef __orxWINDOWS__

  /** The function will be called fastly (use registers for parameters as far as possible).*/
  #define orxFASTCALL           __fastcall
  
  /** The function will be exported (dll compilation) */
  #define orxEXPORT             __declspec(dllexport)
  
  /** The function will be imported (exe comoilation) */
  #define orxIMPORT             __declspec(dllimport)
  
  /** The function will not be exported nor imported */
  #define orxLOCAL

  /** The function or the object will be constant. */
  #define orxCONST              const
  /** The function or the object will be static. */
  #define orxSTATIC             static
  /** The function intend to be inlined. */
  #define orxINLINE             inline
  /** The function or the object will use registers as far as possible. */
  #define orxREGISTER           register

  /** The null adress. */
  #define orxNULL               NULL

#else /* __orxWINDOWS__ */

  /* Linux */
  #ifdef __orxLINUX__

    /** The function will be called fastly (use registers for parameters as far as possible).*/
    #define orxFASTCALL         __attribute__ ((fastcall))
    
    /** The function will be exported (dll compilation) */
    #define orxEXPORT           __attribute__ ((visibility("default")))
  
    /** The function will be imported (exe comoilation) */
    #define orxIMPORT
  
    /** The function will not be exported nor imported */
    #define orxLOCAL            __attribute__ ((visibility("hidden")))

    /** The function or the object will be constant. */    
    #define orxCONST            const
    /** The function or the object will be static. */
    #define orxSTATIC           static
    /** The function intend to be inlined. */
    #define orxINLINE           inline
    /** The function or the object will use registers as far as possible. */
    #define orxREGISTER         register

    /** The null adress. */
    #define orxNULL             NULL

  #endif /* __orxLINUX__ */
  
#endif /* __orxWINDOWS__ */  

/* DLL? */
#ifdef __orxDLL__          /* orx compiled as a dynamic library */
  #ifdef __orxDLLEXPORT__  /* export functions (orx.dll compilation) */
    #define orxDLLAPI orxEXPORT
  #else                    /* no __orxDLLEXPORT__ */
    #define orxDLLAPI orxIMPORT
  #endif                   /* end orxDLLEXPORT */
#else                      /* no __orxDLL__ (it should be __orxLIB__) */
  #define orxDLLAPI
#endif                     /* end __orxDLL__ */


/** Extremely fast swap macros for two 32-bits objects. */
#define orxSWAP32(A, B)                                       \
do                                                            \
{                                                             \
  *((orxU32 *)&(A)) = *((orxU32 *)&(A)) ^ *((orxU32 *)&(B));  \
  *((orxU32 *)&(B)) = *((orxU32 *)&(A)) ^ *((orxU32 *)&(B));  \
  *((orxU32 *)&(A)) = *((orxU32 *)&(A)) ^ *((orxU32 *)&(B));  \
} while(orxFALSE);


/** Test a flag. 
 * Tool macro for testing a bit-set in a 32-bits object.
 * @param I Integer to test.
 * @param F Flag to test.
 * @return true if flag is completely found.
 */
#define orxFLAG32_TEST(I, F) (((I)&(F))==(F))


/** Set a flag
 * Tool macro for set bit-set in a 32-bits object.
 * Add S into T juste where F bits are 1.
 * @param T Target
 * @param F Filter
 * @param S Source
 */
#define orxFLAG32_SET(T, F, S) ((T)=((T)&!(F))|((F)&(S)))

#endif /*_orxDECL_H_*/
