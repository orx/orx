/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2011 Orx-Project
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
 *
 * @{
 */


#ifndef _orxDECL_H_
#define _orxDECL_H_

#ifdef __APPLE__

  #include "TargetConditionals.h"

#endif /* __APPLE__ */

#ifdef TARGET_OS_ANDROID

  #include <android/api-level.h>

#endif /* TARGET_OS_ANDROID */


/* *** Platform dependent base declarations */

/* No processor defines? */
#if !defined(__orxPPC__) && !defined(__orxX86_64__) && !defined(__orxX86__)

  /* PowerPC? */
  #if defined(__ppc__) || defined(PPC) || defined(__PPC) || defined(__POWERPC__)

    #define __orxPPC__

  /* x86_64? */
  #elif defined(__x86_64)

    #define __orxX86_64__

  /* x86 */
  #else

    #define __orxX86__

  #endif

#endif /* !__orxPPC__ && !__orxX86_64__ && !__orxX86__ */

/* Power PC? */
#ifdef __orxPPC__

  #define __orxBIG_ENDIAN__
  #undef __orxLITTLE_ENDIAN__

#else /* __orxPPC__ */

  #define __orxLITTLE_ENDIAN__
  #undef __orxBIG_ENDIAN__

#endif /* __orxPPC__ */

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
#if !defined(__orxWINDOWS__) && !defined(__orxMAC__) && !defined(__orxLINUX__) && !defined(__orxGP2X__) && !defined(__orxWII__) && !defined(__orxIPHONE__) && !defined(__orxANDROID__) && !defined(__orxANDROID_NATIVE__)

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
  #elif defined(TARGET_OS_IPHONE)

    #define __orxIPHONE__

  /* Android */
  #elif defined(TARGET_OS_ANDROID)

    #if __ANDROID_API__ >= 9

      #define __orxANDROID_NATIVE__

    #else /* __ANDROID_API__ >= 9 */

      #define __orxANDROID__

    #endif /* __ANDROID_API__ >= 9 */

  /* Mac? */
  #elif defined(__APPLE__)

    #define __orxMAC__

  /* Wii? */
  #elif defined(__orxPPC__)

    #define __orxWII__

  #else

    #error "Couldn't guess platform define. Please provide it (__orxWINDOWS__/__orxLINUX__/__orxMAC__/__orxGP2X__/__orxWII__/__orxIPHONE__/__orxANDROID__/__orxANDROID_NATIVE__)"

  #endif

#endif /* !__orxWINDOWS__ && !__orxMAC__ && !__orxLINUX__ && !__orxGP2X__ && !__orxWII__ && !__orxIPHONE__ */


#ifdef __cplusplus

  #define orxIMPORT             "C"
  #define __orxCPP__

#else /* __cplusplus */

  #define orxIMPORT
  #undef __orxCPP__

#endif /* __cplusplus */

#ifdef __OBJC__

  #define __orxOBJC__

#else /* __OBJC__ */

  #undef __orxOBJC__

#endif /* __OBJC__ */


/* Windows */
#ifdef __orxWINDOWS__

  #ifdef __orxFREEBASIC__

    #define orxFASTCALL         __stdcall

  #else /* __orxFREEBASIC__ */

    #define orxFASTCALL         __fastcall

  #endif /* __orxFREEBASIC__ */

  #define orxSTDCALL            __stdcall

  #define orxCDECL              __cdecl

  /** The function will be exported (dll compilation) */
  #define orxDLLEXPORT          __declspec(dllexport)

  /** The function will be imported (exe compilation) */
  #define orxDLLIMPORT          __declspec(dllimport)

  /** The null address */
  #define orxNULL               (0)

  /* *** Compiler specific *** */
  /** The function intend to be inlined. */
  #if defined(__orxFREEBASIC__)

    #define orxINLINE

  #elif defined(__orxGCC__)

    #define orxINLINE         inline

  #elif defined(__orxMSVC__)

    #define orxINLINE         __inline

  #endif

#else /* __orxWINDOWS__ */

  /* Linux / Mac / GP2X / Wii / iPhone */
  #if defined(__orxLINUX__) || defined(__orxMAC__) || defined(__orxGP2X__) || defined(__orxWII__) || defined(__orxIPHONE__) || defined(__orxANDROID__) || defined(__orxANDROID_NATIVE__)

    #if defined(__orxGP2X__) || defined(__orxPPC__) || defined(__orxX86_64__) || defined(__orxIPHONE__) || defined(__orxANDROID__) || defined(__orxANDROID_NATIVE__)

      #define orxFASTCALL

      #define orxSTDCALL

      #define orxCDECL

    #else /* __orxGP2X__ || __orxPPC__ || __orxX86_64__ || __orxIPHONE__ || __orxANDROID__ || __orxANDROID_NATIVE__ */

      #ifdef __orxFREEBASIC__

        #define orxFASTCALL     __attribute__ ((stdcall))

      #else /* __orxFREEBASIC__ */

        #define orxFASTCALL     __attribute__ ((fastcall))

      #endif /* __orxFREEBASIC__ */

      #define orxSTDCALL        __attribute__ ((stdcall))

      #define orxCDECL          __attribute__ ((cdecl))

    #endif /* __orxGP2X__ || __orxPPC__ || __orxX86_64__ || __orxIPHONE__ || __orxANDROID__ || __orxANDROID_NATIVE__ */

    /** The function will be exported (dll compilation) */
    #define orxDLLEXPORT        __attribute__ ((visibility("default")))

    /** The function will be imported (exe compilation) */
    #define orxDLLIMPORT

    /** The function intend to be inlined. */
    #ifdef __orxFREEBASIC__

      #define orxINLINE

    #else /* __orxFREEBASIC__ */

      #define orxINLINE         inline

    #endif /* __orxFREEBASIC__ */

    /** The null address */
    #define orxNULL             (0)

    #if defined(__orxWII__) || defined(__orxIPHONE__) || defined(__orxANDROID__) || defined(__orxANDROID_NATIVE__)

      /* Wii and IPhone versions can only be embedded due to the lack of dlfcn presence */
      #define __orxEMBEDDED__

    #endif /* __orxWII__ || __orxIPHONE__ || __orxANDROID__ || __orxANDROID_NATIVE__ */

  #endif /* __orxLINUX__ || __orxMAC__ || __orxGP2X__ || __orxWII__ || __orxIPHONE__ || __orxANDROID__ || __orxANDROID_NATIVE__ */

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
#define orxALIGN(ADDRESS, BLOCK_SIZE)   (((ADDRESS) + ((BLOCK_SIZE) - 1)) & (~((BLOCK_SIZE) - 1)))

#define orxALIGN16(ADDRESS)             orxALIGN(ADDRESS, 16)
#define orxALIGN32(ADDRESS)             orxALIGN(ADDRESS, 32)
#define orxALIGN64(ADDRESS)             orxALIGN(ADDRESS, 64)


/* Padding macro */
#ifdef __orxPADDING__

  #ifdef orxPADDING_SIZE                /* Padding size defined */

    #define orxPAD(SIZE)                orxU8 au8Pad[orxALIGN(SIZE, orxPADDING_SIZE) - SIZE];

  #else /* orxPADDING_SIZE */           /* Padding size not defined */

    #define orxPAD(SIZE)                /* No padding applied */

    #warning orxPADDING_SIZE is undefined : its value should be a power of 2!
    #undef __orxPADDING__

  #endif /* orxPADDING_SIZE */

#else /* __orxPADDING__ */

  #define orxPAD(SIZE)                  /* No padding applied */

#endif /* __orxPADDING__ */


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

/** Swaps flags
 * @param[in] X Flag container
 * @param[in] S Flags to swap
 */
#define orxFLAG_SWAP(X, S) ((X) ^= (S))

#endif /*_orxDECL_H_*/

/** @} */
