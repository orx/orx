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
 * @file orxDecl.h
 * @date 14/04/2005
 * @author iarwain@orx-project.org
 *
 * @todo
 * - Add the required declarations at need.
 */

/**
 * @addtogroup orxDecl
 *
 * Base declarations
 * Allows to creates and handle Sets of Anims.
 * It consists of a structure containing Anims and their relations.
 * It also contains functions for handling and accessing them.
 * AnimSets are structures.
 * They thus can be referenced by Anim Pointers.
 *
 * @{
 */


#ifndef _orxDECL_H_
#define _orxDECL_H_


/* *** Platform dependant base declarations */

/* PowerPC? */
#ifdef __ppc__

  #define __orxPPC__

#endif /* __ppc__ */

/* Has platform defines? */
#if defined(__orxWINDOWS__) || defined(__orxMAC__) || defined(__orxLINUX__) || defined(__orxGP2X__)

  #undef __orxPLATFORM_GUESS__

#else /* __orxWINDOWS__ || __orxMAC__ || __orxLINUX__ || __orxGP2X__ */

  #define __orxPLATFORM_GUESS__

#endif /* __orxWINDOWS__ || __orxMAC__ || __orxLINUX__ || __orxGP2X__ */


/* Should guess platform? */
#ifdef __orxPLATFORM_GUESS__

  /* Removes compiler defines */
  #undef __orxGCC__
  #undef __orxMSVC__

  /* Windows? */
  #if defined(_WIN32) || defined(WIN32)

    #define __orxWINDOWS__

    /* GCC? */
    #ifdef __GNUC__

      #define __orxGCC__

    #else /* __GNUC__ */

      #define __orxMSVC__

    #endif

  /* Linux? */
  #elif defined(LINUX)

    #define __orxLINUX__
    #define __orxGCC__

  /* Mac? */
  #elif defined(MAC)

    #define __orxMAC__
    #define __orxGCC__

  /* GP2X? */
  #elif defined(GP2X)

    #define __orxGP2X__
    #define __orxGCC__

  #else

    #error "Couldn't guess platform/compiler defines. Please provide them (__orxWINDOWS__/__orxLINUX__/__orxMAC__/__orxGP2X__) / (__orxGCC__/__orxMSVC__)

  #endif

#endif /* __orxPLATFORM_GUESS__ */


/* Windows */
#ifdef __orxWINDOWS__

  #ifdef __orxMSVC__

    #define orxFASTCALL

  #else /* __orxMSVC__ */

    #define orxFASTCALL         __fastcall

  #endif /* __orxMSVC__ */

  #define orxSTDCALL            __stdcall

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

    #if defined(__orxGP2X__) || defined(__orxPPC__)

      #define orxFASTCALL

      #define orxSTDCALL

      #define orxCDECL

    #else /* __orxGP2X__ || __orxPPC__ */

      #define orxFASTCALL       __attribute__ ((fastcall))

      #define orxSTDCALL        __attribute__ ((stdcall))

      #define orxCDECL          __attribute__ ((cdecl))

    #endif /* __orxGP2X__ || __orxPPC__ */

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
 * @param[in] M Filtering mask
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


/** @} */
