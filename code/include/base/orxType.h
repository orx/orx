/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2015 Orx-Project
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
 * @file orxType.h
 * @date 12/04/2005
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxType
 *
 * Base types
 *
 * @{
 */


#ifndef _orxTYPE_H_
#define _orxTYPE_H_

/* *** Uses orx declarations *** */
#include "base/orxDecl.h"

/* *** Platform dependent base types */

/* Windows */
#ifdef __orxWINDOWS__

  typedef void *                  orxHANDLE;

  #ifdef __orxX86_64__

  typedef unsigned  int           orxU32;
  typedef unsigned  short         orxU16;
  typedef unsigned  char          orxU8;

  typedef signed    int           orxS32;
  typedef signed    short         orxS16;
  typedef signed    char          orxS8;

  typedef unsigned  int           orxBOOL;

  #else /* __orxX86_64__ */

  typedef unsigned  long          orxU32;
  typedef unsigned  short         orxU16;
  typedef unsigned  char          orxU8;

  typedef signed    long          orxS32;
  typedef signed    short         orxS16;
  typedef signed    char          orxS8;

  typedef unsigned  long          orxBOOL;

  #endif /* __orxX86_64__ */

  typedef float                   orxFLOAT;
  typedef double                  orxDOUBLE;

  typedef char                    orxCHAR;
  #define orxSTRING               orxCHAR *

  typedef orxU32                  orxENUM;

  #define orx2F(V)                ((orxFLOAT)(V))
  #define orx2D(V)                ((orxDOUBLE)(V))

  #define orxENUM_NONE            0xFFFFFFFFL

  /* Compiler specific */
  #ifdef __orxGCC__

    typedef unsigned  long long   orxU64;
    typedef signed    long long   orxS64;

  #endif /* __orxGCC__ */
  #ifdef __orxLLVM__

    typedef unsigned  long long   orxU64;
    typedef signed    long long   orxS64;

  #endif /* __orxLLVM__ */
  #ifdef __orxMSVC__

    typedef unsigned  __int64     orxU64;
    typedef signed    __int64     orxS64;

  #endif /* __orxMSVC__ */

#else /* __orxWINDOWS__ */

  /* Linux / Mac / iOS / Android */
  #if defined(__orxLINUX__) || defined(__orxMAC__) || defined(__orxIOS__) || defined(__orxANDROID__) || defined(__orxANDROID_NATIVE__)

    typedef void *                orxHANDLE;

    #if defined(__orxX86_64__) || defined(__orxPPC64__) || defined(__orxARM64__)

    typedef unsigned  long long   orxU64;
    typedef unsigned  int         orxU32;
    typedef unsigned  short       orxU16;
    typedef unsigned  char        orxU8;

    typedef signed    long long   orxS64;
    typedef signed    int         orxS32;
    typedef signed    short       orxS16;
    typedef signed    char        orxS8;

    typedef unsigned  int         orxBOOL;

    #else /* __orxX86_64__ || __orxPPC64__ || __orxARM64__ */

    typedef unsigned  long long   orxU64;
    typedef unsigned  long        orxU32;
    typedef unsigned  short       orxU16;
    typedef unsigned  char        orxU8;

    typedef signed    long long   orxS64;
    typedef signed    long        orxS32;
    typedef signed    short       orxS16;
    typedef signed    char        orxS8;

    typedef unsigned  long        orxBOOL;

    #endif /* __orxX86_64__ || __orxPPC64__ || __orxARM64__ */

    typedef float                 orxFLOAT;
    typedef double                orxDOUBLE;

    typedef char                  orxCHAR;
    #define orxSTRING             orxCHAR *

    typedef orxU32                orxENUM;

    #define orx2F(V)              ((orxFLOAT)(V))
    #define orx2D(V)              ((orxDOUBLE)(V))

    #define orxENUM_NONE          0xFFFFFFFFL

  #endif /* __orxLINUX__ || __orxMAC__ || __orxIOS__ || __orxANDROID__ */

#endif /* __orxWINDOWS__ */

/* *** Misc constants *** */

/* *** Seek offset constants *** */
typedef enum __orxSEEK_OFFSET_WHENCE_t
{
  orxSEEK_OFFSET_WHENCE_START = 0,
  orxSEEK_OFFSET_WHENCE_CURRENT,
  orxSEEK_OFFSET_WHENCE_END,

  orxSEEK_OFFSET_WHENCE_NUMBER,

  orxSEEK_OFFSET_WHENCE_NONE = orxENUM_NONE

} orxSEEK_OFFSET_WHENCE;


/* *** Boolean constants *** */
#define orxFALSE                  ((orxBOOL)(1 != 1))
#define orxTRUE                   ((orxBOOL)(1 == 1))


/* *** Float constants *** */
static const orxFLOAT             orxFLOAT_0            = orx2F(0.0f);
static const orxFLOAT             orxFLOAT_1            = orx2F(1.0f);
static const orxFLOAT             orxFLOAT_MAX          = orx2F(3.402823466e+38f);

/* *** Double constants *** */
static const orxDOUBLE            orxDOUBLE_0           = orx2D(0.0);
static const orxDOUBLE            orxDOUBLE_1           = orx2D(1.0);
static const orxDOUBLE            orxDOUBLE_MAX         = orx2D(1.7976931348623158e+308);


/* *** Undefined constants *** */
static const orxU64               orxU64_UNDEFINED      = (orxU64)(-1);
static const orxU32               orxU32_UNDEFINED      = (orxU32)(-1);
static const orxU16               orxU16_UNDEFINED      = (orxU16)(-1);
static const orxU8                orxU8_UNDEFINED       = (orxU8)(-1);
static const orxHANDLE            orxHANDLE_UNDEFINED   = (orxHANDLE)(-1);


/* *** String & character constants *** */
extern orxDLLAPI const orxSTRING  orxSTRING_EMPTY;
extern orxDLLAPI const orxSTRING  orxSTRING_TRUE;
extern orxDLLAPI const orxSTRING  orxSTRING_FALSE;
extern orxDLLAPI const orxSTRING  orxSTRING_EOL;

#define orxCHAR_NULL              '\0'
#define orxCHAR_CR                '\r'
#define orxCHAR_LF                '\n'
#define orxCHAR_EOL               '\n'
#define orxCHAR_ASCII_NUMBER      128


/* *** Directory separators *** */

extern orxDLLAPI const orxSTRING  orxSTRING_DIRECTORY_SEPARATOR;

#define orxCHAR_DIRECTORY_SEPARATOR_WINDOWS   '\\'
#define orxCHAR_DIRECTORY_SEPARATOR_LINUX     '/'

#if defined(__orxWINDOWS__)

#define orxCHAR_DIRECTORY_SEPARATOR           '\\'

#elif defined(__orxLINUX__) || defined(__orxMAC__) || defined(__orxIOS__) || defined(__orxANDROID__) || defined(__orxANDROID_NATIVE__)

#define orxCHAR_DIRECTORY_SEPARATOR           '/'

#endif


/* *** Status defines *** */
typedef enum __orxSTATUS_t
{
  orxSTATUS_FAILURE = 0,          /**< Failure status, the operation has failed */
  orxSTATUS_SUCCESS,              /**< Success status, the operation has worked has expected */

  orxSTATUS_NUMBER,               /**< Sentinel : Number of status */

  orxSTATUS_NONE = orxENUM_NONE   /**< Invalid status */

} orxSTATUS;

#endif /*_orxTYPE_H_*/

/** @} */
