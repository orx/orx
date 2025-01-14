/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008- Orx-Project
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

#ifdef __orxDEBUG__

  #define __orxPROFILER__

#endif /* __orxDEBUG__ */

#ifdef __APPLE__

  #include "TargetConditionals.h"

#endif /* __APPLE__ */

#ifdef TARGET_OS_ANDROID

  #include <android/api-level.h>

#endif /* TARGET_OS_ANDROID */

#include <stddef.h>


/* *** Platform dependent base declarations */

/* No processor defines? */
#if !defined(__orxARM__) && !defined(__orxPPC__) && !defined(__orxPPC64__) && !defined(__orxX86_64__) && !defined(__orxX86__) && !defined(__orxARM64__)

  /* ARM? */
  #if defined(__arm__) || defined(__ARMEL__) || defined(__ARM_EABI__)

    #define __orxARM__

  /* ARM64? */
  #elif defined(__arm64) || defined(__aarch64__)

    #define __orxARM64__

  /* PowerPC? */
  #elif defined(__ppc__) || defined(PPC) || defined(__PPC) || defined(__POWERPC__) || defined(__powerpc__)

    #define __orxPPC__

  /* PowerPC 64? */
  #elif defined(__powerpc64__) || defined(__POWERPC64__)

    #define __orxPPC64__

  /* x86_64? */
  #elif defined(__x86_64) || defined(_M_X64) || defined(__ia64__)

    #define __orxX86_64__

  /* x86 */
  #else

    #define __orxX86__

  #endif

#endif /* !__orxARM__ && !__orxPPC__ && !__orxPPC64__ && !__orxX86_64__ && !__orxX86__ && !__orxARM64__ */

/* Has byte order? */
#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__)

  #if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)

    #define __orxBIG_ENDIAN__
    #undef __orxLITTLE_ENDIAN__

  #else /* (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) */

    #define __orxLITTLE_ENDIAN__
    #undef __orxBIG_ENDIAN__

  #endif /* (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) */

#else /* __BYTE_ORDER__ && __ORDER_BIG_ENDIAN__ */

  /* Power PC? */
  #ifdef __orxPPC__

    #define __orxBIG_ENDIAN__
    #undef __orxLITTLE_ENDIAN__

  #else /* __orxPPC__ */

    #define __orxLITTLE_ENDIAN__
    #undef __orxBIG_ENDIAN__

  #endif /* __orxPPC__ */

#endif /* __BYTE_ORDER__ && __ORDER_BIG_ENDIAN__ */

/* No compiler defines? */
#if !defined(__orxLLVM__) && !defined(__orxGCC__) && !defined(__orxMSVC__)

  /* LLVM? */
  #if defined(__llvm__)

    #define __orxLLVM__

  /* GCC? */
  #elif defined(__GNUC__)

    #define __orxGCC__

  /* MSVC? */
  #elif defined(_MSC_VER)

    #define __orxMSVC__

  #else

    #error "Couldn't guess compiler define. Please provide it (__orxLLVM__/__orxGCC__/__orxMSVC__)"

  #endif

#endif /* !__orxLLVM__ && !__orxGCC__ && !__orxMSVC__ */

/* Instruction size */
#if defined(__orxX86_64__) || defined(__orxPPC64__) || defined(__orxARM64__)

  /* 64 bits */
  #define __orx64__

#else /* __orxX86_64__ || __orxPPC64__ || __orxARM64__ */

  /* 32 bits */
  #define __orx32__

#endif /* __orxX86_64__ || __orxPPC64__ || __orxARM64__ */

/* No platform defines? */
#if !defined(__orxWINDOWS__) && !defined(__orxMAC__) && !defined(__orxLINUX__) && !defined(__orxIOS__) && !defined(__orxANDROID__)

  /* Windows? */
  #if defined(_WIN32) || defined(WIN32)

    #define __orxWINDOWS__

  /* iOS? */
  #elif TARGET_OS_IPHONE

    #define __orxIOS__

  /* Android */
  #elif defined(TARGET_OS_ANDROID)

    #define __orxANDROID__

  /* Linux? */
  #elif defined(linux) || defined(__linux__)

    #define __orxLINUX__

    #pragma GCC diagnostic ignored "-Wunused-function"

  /* Mac? */
  #elif TARGET_OS_MAC

    #define __orxMAC__

  /* Web? */
  #elif defined(EMSCRIPTEN) || defined(__EMSCRIPTEN__)

    #define __orxWEB__

  #else

    #error "Couldn't guess platform define. Please provide it (__orxWINDOWS__/__orxMAC__/__orxLINUX__/__orxIOS__/__orxANDROID__/__orxWEB__)"

  #endif

#endif /* !__orxWINDOWS__ && !__orxMAC__ && !__orxLINUX__ && !__orxIOS__ && !__orxANDROID__ && !__orxWEB__ */


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

  #ifndef orxFASTCALL

    #define orxFASTCALL         __fastcall

  #endif /* !orxFASTCALL */

  #define orxSTDCALL            __stdcall

  #define orxCDECL              __cdecl

  /** The function will be exported (dll compilation) */
  #define orxDLLEXPORT          __declspec(dllexport)

  /** The function will be imported (exe compilation) */
  #define orxDLLIMPORT          __declspec(dllimport)

  /** The null address */
  #define orxNULL               (0)

  /** Inline */
  #ifndef orxINLINE

    /* *** Compiler specific *** */
    #if defined(__orxGCC__) || defined(__orxLLVM__)

      /** The function intend to be inlined. */
      #define orxINLINE         inline

    #elif defined(__orxMSVC__)

      /** The function intend to be inlined. */
      #define orxINLINE         __inline

    #endif

  #endif /* !orxINLINE */

#else /* __orxWINDOWS__ */

  /* Linux / Mac / iOS / Android / Web */
  #if defined(__orxLINUX__) || defined(__orxMAC__) || defined(__orxIOS__) || defined(__orxANDROID__) || defined(__orxWEB__)

    /* ARM / ARM64 / LLVM / PPC / PPC64 / X86_64 / iOS / Android / Web */
    #if defined(__orxARM__) || defined(__orxLLVM__) || defined(__orxPPC__) || defined(__orxPPC64__) || defined(__orxX86_64__) || defined(__orxIOS__) || defined(__orxANDROID__) || defined(__orxARM64__) || defined(__orxWEB__)

      #ifndef orxFASTCALL

        #define orxFASTCALL

      #endif /* !orxFASTCALL */

      #define orxSTDCALL

      #define orxCDECL

    #else /* __orxARM__ || __orxLLVM__ || __orxPPC__ || __orxPPC64__ || __orxX86_64__ || __orxIOS__ || __orxANDROID__ || __orxARM64__ || __orxWEB__ */

      #ifndef orxFASTCALL

        #define orxFASTCALL     __attribute__ ((fastcall))

      #endif /* !orxFASTCALL */

      #define orxSTDCALL        __attribute__ ((stdcall))

      #define orxCDECL          __attribute__ ((cdecl))

    #endif /* __orxARM__ || __orxLLVM__ || __orxPPC__ || __orxPPC64__ || __orxX86_64__ || __orxIOS__ || __orxANDROID__ || __orxARM64__ || __orxWEB__ */

    /** The symbol will be exported (dll compilation) */
    #define orxDLLEXPORT        __attribute__ ((visibility("default")))

    /** The symbol will be imported (exe compilation) */
    #define orxDLLIMPORT

    #ifndef orxINLINE

      /** The function intend to be inlined. */
      #define orxINLINE         inline

    #endif /* !orxINLINE */

    /** The null address */
    #define orxNULL             (0)

    #if defined(__orxWEB__)

      #define __orxSTATIC__

    #endif /* __orxWEB__ */

    #if defined(__orxIOS__) || defined(__orxANDROID__)

      /* iOS versions can only be embedded due to the lack of dlfcn presence */
      #define __orxEMBEDDED__

      /* Always use static on iOS & Android */
      #define __orxSTATIC__

    #endif /* __orxIOS__ || __orxANDROID__ */

  #endif /* __orxLINUX__ || __orxMAC__ || __orxIOS__ || __orxANDROID__ || __orxWEB__ */

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
#define orxALIGN(VALUE, BLOCK_SIZE)     (((orxUPTR)(VALUE) + ((orxUPTR)(BLOCK_SIZE) - 1)) & (~((orxUPTR)(BLOCK_SIZE) - 1)))

#define orxALIGN8(VALUE)                orxALIGN(VALUE, 8)
#define orxALIGN16(VALUE)               orxALIGN(VALUE, 16)
#define orxALIGN32(VALUE)               orxALIGN(VALUE, 32)
#define orxALIGN64(VALUE)               orxALIGN(VALUE, 64)


/** Structure macros */
#define orxSTRUCT_GET_FROM_FIELD(TYPE, FIELD, POINTER)   ((TYPE *)((orxU8 *)(POINTER) - offsetof(TYPE, FIELD)))


/** Array macros */
#define orxARRAY_GET_ITEM_COUNT(ARRAY)  (sizeof(ARRAY) / sizeof(ARRAY[0]))


/** Flag macros */

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


/** ANSI macros */

#define orxANSI_KC_MARKER                   '\x1b'

#define orxANSI_KZ_COLOR_RESET              "\x1b[0m"

#define orxANSI_KZ_COLOR_BOLD_ON            "\x1b[1m"
#define orxANSI_KZ_COLOR_ITALICS_ON         "\x1b[3m"
#define orxANSI_KZ_COLOR_UNDERLINE_ON       "\x1b[4m"
#define orxANSI_KZ_COLOR_BLINK_ON           "\x1b[5m"
#define orxANSI_KZ_COLOR_INVERSE_ON         "\x1b[7m"
#define orxANSI_KZ_COLOR_STRIKETHROUGH_ON   "\x1b[9m"

#define orxANSI_KZ_COLOR_BOLD_OFF           "\x1b[22m"
#define orxANSI_KZ_COLOR_ITALICS_OFF        "\x1b[23m"
#define orxANSI_KZ_COLOR_UNDERLINE_OFF      "\x1b[24m"
#define orxANSI_KZ_COLOR_BLINK_OFF          "\x1b[25m"
#define orxANSI_KZ_COLOR_INVERSE_OFF        "\x1b[27m"
#define orxANSI_KZ_COLOR_STRIKETHROUGH_OFF  "\x1b[29m"

#define orxANSI_KZ_COLOR_FG_BLACK           "\x1b[30m"
#define orxANSI_KZ_COLOR_FG_RED             "\x1b[31m"
#define orxANSI_KZ_COLOR_FG_GREEN           "\x1b[32m"
#define orxANSI_KZ_COLOR_FG_YELLOW          "\x1b[33m"
#define orxANSI_KZ_COLOR_FG_BLUE            "\x1b[34m"
#define orxANSI_KZ_COLOR_FG_MAGENTA         "\x1b[35m"
#define orxANSI_KZ_COLOR_FG_CYAN            "\x1b[36m"
#define orxANSI_KZ_COLOR_FG_WHITE           "\x1b[37m"
#define orxANSI_KZ_COLOR_FG_DEFAULT         "\x1b[39m"

#define orxANSI_KZ_COLOR_BG_BLACK           "\x1b[40m"
#define orxANSI_KZ_COLOR_BG_RED             "\x1b[41m"
#define orxANSI_KZ_COLOR_BG_GREEN           "\x1b[42m"
#define orxANSI_KZ_COLOR_BG_YELLOW          "\x1b[43m"
#define orxANSI_KZ_COLOR_BG_BLUE            "\x1b[44m"
#define orxANSI_KZ_COLOR_BG_MAGENTA         "\x1b[45m"
#define orxANSI_KZ_COLOR_BG_CYAN            "\x1b[46m"
#define orxANSI_KZ_COLOR_BG_WHITE           "\x1b[47m"
#define orxANSI_KZ_COLOR_BG_DEFAULT         "\x1b[49m"


#endif /*_orxDECL_H_*/

/** @} */
