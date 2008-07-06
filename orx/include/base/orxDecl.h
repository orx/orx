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
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/


#ifndef _orxDECL_H_
#define _orxDECL_H_

/* *** Platform depedant base declarations */

/* Windows */
#ifdef __orxWINDOWS__

  /** The function will be called fastly (use registers for parameters as far as possible).*/
  #ifdef __orxMSVC__

    #define orxFASTCALL

  #else /* __orxMSVC__ */

    #define orxFASTCALL         __fastcall

  #endif /* __orxMSVC__ */

  /** The function will be called using stdcall convention.*/
  #define orxSTDCALL            __stdcall

  /** The function will be called using cdecl convention.*/
  #define orxCDECL              __cdecl

  /** The function will be exported (dll compilation) */
  #define orxDLLEXPORT          __declspec(dllexport)

  /** The function will be imported (exe compilation) */
  #define orxDLLIMPORT          __declspec(dllimport)

  /** The function will not be exported nor imported */
  #define orxDLLLOCAL

  /** The function or the object will be constant. */
  #define orxCONST              const
  /** The function or the object will be static. */
  #define orxSTATIC             static
  /** The function or the object will use registers as far as possible. */
  #define orxREGISTER           register

  /** The null adress. */
  #define orxNULL               (0)

  /* *** Compiler specific *** */
  /** The function intend to be inlined. */
  #ifdef __orxGCC__
    #define orxINLINE           inline
  #else /* __orxGCC__ */
    #ifdef __orxMSVC__
      #define orxINLINE         __inline
    #endif /* __orxMSVC__ */
  #endif /* __orxGCC__ */

#else /* __orxWINDOWS__ */

  /* Linux / Mac */
  #if defined(__orxLINUX__) || defined(__orxMAC__) || defined(__orxGP2X__)

    #ifdef __orxGP2X__
    
      /** The function will be called fastly (use registers for parameters as far as possible).*/
      #define orxFASTCALL

      /** The function will be called using stdcall convention.*/
      #define orxSTDCALL

      /** The function will be called using cdecl convention.*/
      #define orxCDECL

    #else /* __orxGP2X__ */

      /** The function will be called fastly (use registers for parameters as far as possible).*/
      #define orxFASTCALL         __attribute__ ((fastcall))
    
      /** The function will be called using stdcall convention.*/
      #define orxSTDCALL          __attribute__ ((stdcall))

      /** The function will be called using cdecl convention.*/
      #define orxCDECL            __attribute__ ((cdecl))

    #endif /* __orxGP2X__ */

    /** The function will be exported (dll compilation) */
    #define orxDLLEXPORT        __attribute__ ((visibility("default")))

    /** The function will be imported (exe compilation) */
    #define orxDLLIMPORT

    /** The function will not be exported nor imported */
    #define orxDLLLOCAL         __attribute__ ((visibility("hidden")))

    /** The function or the object will be constant. */
    #define orxCONST            const
    /** The function or the object will be static. */
    #define orxSTATIC           static
    /** The function intend to be inlined. */
    #define orxINLINE           inline
    /** The function or the object will use registers as far as possible. */
    #define orxREGISTER         register

    /** The null adress. */
    #define orxNULL             ((void *)0)

  #endif /* __orxLINUX__ || __orxMAC__ || __orxGP2X__ */

#endif /* __orxWINDOWS__ */

/* DLL? */
#ifdef __orxDLL__          /* orx compiled as a dynamic library */
  #ifdef __orxDLLEXPORT__  /* export functions (orx.dll compilation) */
    #define orxDLLAPI orxDLLEXPORT
  #else                    /* no __orxDLLEXPORT__ */
    #define orxDLLAPI orxDLLIMPORT
  #endif                   /* end orxDLLEXPORT */
#else                      /* no __orxDLL__ (it should be __orxLIB__) */
  #define orxDLLAPI
#endif                     /* end __orxDLL__ */

#ifdef __orxDLLEXPORT__     /* export functions (orx.dll compilation) */
  #define orxSDKAPI orxDLLEXPORT
#else                       /* no __orxDLLEXPORT__ */
  #define orxSDKAPI orxDLLIMPORT
#endif                      /* end orxDLLEXPORT */


/** Memory alignment macros */
#define _orxALIGN(ADDRESS, BLOCK_SIZE)  ((ADDRESS + (BLOCK_SIZE - 1)) & (~(BLOCK_SIZE - 1)))

#define orxALIGN16(ADDRESS)             _orxALIGN(ADRESS, 16)
#define orxALIGN32(ADDRESS)             _orxALIGN(ADRESS, 32)
#define orxALIGN64(ADDRESS)             _orxALIGN(ADRESS, 64)


/* Padding macro */
#ifdef __orxPADDING__

  #ifdef orxPADDING_SIZE                /* Padding size defined */

    #define orxPAD(SIZE)                /* No padding applied */

  #else /* orxPADDING_SIZE */           /* Padding size not defined */

    #define orxPAD(SIZE)                orxU8 au8Pad[_orxALIGN(SIZE, orxPADDING_SIZE) - SIZE];

    #warning orxPADDING_SIZE is undefined : its value should be a power of 2!
    #undef __orxPADDING__

  #endif /* orxPADDING_SIZE */

#else /* __orxPADDING__ */

  #define orxPAD(SIZE)                  /* No padding applied */

#endif /* __orxPADDING__ */


/** Fast in-place swap macros for two 32-bits objects. */
#define orxSWAP32(A, B)                                       \
do                                                            \
{                                                             \
  *((orxU32 *)&(A)) = *((orxU32 *)&(A)) ^ *((orxU32 *)&(B));  \
  *((orxU32 *)&(B)) = *((orxU32 *)&(A)) ^ *((orxU32 *)&(B));  \
  *((orxU32 *)&(A)) = *((orxU32 *)&(A)) ^ *((orxU32 *)&(B));  \
} while(orxFALSE)


/** Tests all flags
 * @param[in] X Flag container
 * @param[in] F Flags to test
 * @return true if flags are all presents
 */
#define orxFLAG_TEST_ALL(X, F) (((X) & (F)) == (F))

/** Tests flags
 * @param[in] X Flag container
 * @param[in] F Flags to test
 * @return true if at least one flag is present
 */
#define orxFLAG_TEST(X, F) (((X) & (F)) != 0)

/** Gets flags
 * @param[in] X Flag container
 * @param[in] F Flags to test
 * @return Masked flags
 */
#define orxFLAG_GET(X, M) ((X) & (M))

/** Sets / unsets flags
 * @param[in] X Flag container
 * @param[in] A Flags to add
 * @param[in] R Flags to remove
 */
#define orxFLAG_SET(X, A, R)  \
do                            \
{                             \
  (X) &= ~(R);                \
  (X) |= (A);                 \
} while(orxFALSE)


#endif /*_orxDECL_H_*/
