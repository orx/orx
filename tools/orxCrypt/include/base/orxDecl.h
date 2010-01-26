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


/* *** Platform dependent base declarations */

/* No processor defines? */
#if !defined(__orxPPC__) && !defined(__orxX86_64__)

  /* PowerPC? */
  #if defined(__ppc__) || defined(PPC) || defined(__PPC) || defined(__POWERPC__)

    #define __orxPPC__

  /* x86_64? */
  #elif defined(__x86_64)

    #define __orxX86_64__

  #endif

#endif /* !__orxPPC__ && !__orxX86_64__ */


/* No compiler defines? */
#if !defined(__orxGCC__) && !defined(__orxMSVC__)

  /* GCC? */
  #if defined(__GNUC__)

    #define __orxGCC__

  /* MSVC? */
  #elif defined(_MSC_VER)

    #define __orxMSVC__

  #else

    #error "Couldn't guess compiler define. Please provide it (__orxGCC__/__orxMSVC__)"

  #endif

#endif /* !__orxGCC__ && !__orxMSVC__ */


/* No platform defines? */
#if !defined(__orxWINDOWS__) && !defined(__orxMAC__) && !defined(__orxLINUX__) && !defined(__orxGP2X__) && !defined(__orxWII__) && !defined(__orxIPHONE__)

  /* Windows? */
  #if defined(_WIN32) || defined(WIN32)

    #define __orxWINDOWS__

  /* GP2X? */
  #elif defined(__linux__) && defined(__arm__)

    #define __orxGP2X__

  /* Linux? */
  #elif defined(linux) || defined(__linux__)

    #define __orxLINUX__

  /* IPhone? */
  #elif defined(__IPHONE_OS_VERSION_MIN_REQUIRED)

    #define __orxIPHONE__

  /* Mac? */
  #elif defined(__APPLE__)

    #define __orxMAC__

  /* Wii? */
  #elif defined(__orxPPC__)

    #define __orxWII__

  #else

    #error "Couldn't guess platform define. Please provide it (__orxWINDOWS__/__orxLINUX__/__orxMAC__/__orxGP2X__/__orxWII__/__orxIPHONE__)"

  #endif

#endif /* !__orxWINDOWS__ && !__orxMAC__ && !__orxLINUX__ && !__orxGP2X__ && !__orxWII__ && !__orxIPHONE__ */


#ifdef __cplusplus

  #define orxIMPORT             "C"
  #define __orxCPP__

#else /* __cplusplus */

  #define orxIMPORT
  #undef __orxCPP__

#endif /* __cplusplus */


/* Windows */
#ifdef __orxWINDOWS__

  #define orxFASTCALL           __fastcall

  #define orxSTDCALL            __stdcall

  #define orxCDECL              __cdecl

  /** The function will be exported (dll compilation) */
  #define orxDLLEXPORT          __declspec(dllexport)

  /** The function will be imported (exe compilation) */
  #define orxDLLIMPORT          __declspec(dllimport)

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

  /* Linux / Mac / GP2X / Wii */
  #if defined(__orxLINUX__) || defined(__orxMAC__) || defined(__orxGP2X__) || defined(__orxWII__) || defined(__orxIPHONE__)

    #if defined(__orxGP2X__) || defined(__orxPPC__) || defined(__orxX86_64__)

      #define orxFASTCALL

      #define orxSTDCALL

      #define orxCDECL

    #else /* __orxGP2X__ || __orxPPC__ || __orxX86_64__ */

      #define orxFASTCALL       __attribute__ ((fastcall))

      #define orxSTDCALL        __attribute__ ((stdcall))

      #define orxCDECL          __attribute__ ((cdecl))

    #endif /* __orxGP2X__ || __orxPPC__ || __orxX86_64__ */

    /** The function will be exported (dll compilation) */
    #define orxDLLEXPORT        __attribute__ ((visibility("default")))

    /** The function will be imported (exe compilation) */
    #define orxDLLIMPORT

    /** The function intend to be inlined. */
    #define orxINLINE           inline

    /** The null adress. */
    #define orxNULL             (0)

    #if defined(__orxWII__) || defined(__orxIPHONE__)

      /* Wii and IPhone versions can only be embedded due to the lack of dlfcn presence */
      #define __orxEMBEDDED__

    #endif /* __orxWII__ || __orxIPHONE__ */

  #endif /* __orxLINUX__ || __orxMAC__ || __orxGP2X__ || __orxWII__ || __orxIPHONE__ */

#endif /* __orxWINDOWS__ */


/* Plugin include? */
#if defined(__orxPLUGIN__)

  #ifdef __orxEMBEDDED__

    #define orxDLLAPI orxIMPORT /* Compiling embedded plug-in => API doesn't need to be imported */

  #else

    #define orxDLLAPI orxIMPORT orxDLLIMPORT /* Compiling plug-in => API needs to be imported */

  #endif /* __orxEMBEDDED__ */

/* External include? */
#elif defined(__orxEXTERN__)

  #ifdef __orxSTATIC__

    #define orxDLLAPI orxIMPORT /* Linking executable against orx static library */

  #else /* __orxSTATIC__ */

    #define orxDLLAPI orxIMPORT orxDLLIMPORT /* Linking executable against orx dynamic library */

  #endif /* __orxSTATIC__ */

/* Internal (library) include */
#else

  #define orxDLLAPI orxIMPORT orxDLLEXPORT /* Compiling orx library => API needs to be exported */

#endif


/** Memory alignment macros */
#define _orxALIGN(ADDRESS, BLOCK_SIZE)  ((ADDRESS + (BLOCK_SIZE - 1)) & (~(BLOCK_SIZE - 1)))

#define orxALIGN16(ADDRESS)             _orxALIGN(ADRESS, 16)
#define orxALIGN32(ADDRESS)             _orxALIGN(ADRESS, 32)
#define orxALIGN64(ADDRESS)             _orxALIGN(ADRESS, 64)


/* Padding macro */
#ifdef __orxPADDING__

  #ifdef orxPADDING_SIZE                /* Padding size defined */

    #define orxPAD(SIZE)                orxU8 au8Pad[_orxALIGN(SIZE, orxPADDING_SIZE) - SIZE];

  #else /* orxPADDING_SIZE */           /* Padding size not defined */

    #define orxPAD(SIZE)                /* No padding applied */

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
} while(0)


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
} while(0)

#endif /*_orxDECL_H_*/

/** @} */
