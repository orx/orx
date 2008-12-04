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
 * @file orxType.h
 * @date 12/04/2005
 * @author iarwain@orx-project.org
 *
 * @todo
 * - Add the required types at need.
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

  typedef void                  orxVOID;

  typedef orxVOID *             orxHANDLE;

  typedef unsigned  long        orxU32;
  typedef unsigned  short       orxU16;
  typedef unsigned  char        orxU8;

  typedef signed    long        orxS32;
  typedef signed    short       orxS16;
  typedef signed    char        orxS8;

  typedef unsigned  long        orxBOOL;

  typedef float                 orxFLOAT;

  typedef char                  orxCHAR;
  typedef orxCHAR *             orxSTRING;

  typedef orxU32                orxRGBA;
  typedef orxU32                orxENUM;

  #define orx2F(V)              (orxFLOAT)(V)

  #define orx2RGBA(R, G, B, A)  ((((R) & 0xFF) << 24) | (((G) & 0xFF) << 16) | (((B) & 0xFF) << 8) | ((A) & 0xFF))
  #define orxRGBA_R(RGBA)       (orxU8)(((RGBA) >> 24) & 0xFF)
  #define orxRGBA_G(RGBA)       (orxU8)(((RGBA) >> 16) & 0xFF)
  #define orxRGBA_B(RGBA)       (orxU8)(((RGBA) >> 8) & 0xFF)
  #define orxRGBA_A(RGBA)       (orxU8)((RGBA) & 0xFF)
  #define orxRGBA_NORMALIZER    (orx2F(1.0f / 255.0f))

  #define orxENUM_NONE          0xFFFFFFFFL

  /* Platform specific */
  #ifdef __orxGCC__

    typedef unsigned  long long   orxU64;
    typedef signed    long long   orxS64;

  #endif /* __orxGCC__ */
  #ifdef __orxMSVC__

    typedef unsigned  __int64     orxU64;
    typedef signed    __int64     orxS64;

  #endif /* __orxMSVC__ */



#else /* __orxWINDOWS__ */

  /* Linux / Mac */
  #if defined(__orxLINUX__) || defined(__orxMAC__) || defined(__orxGP2X__)

    typedef void                orxVOID;

    typedef orxVOID *           orxHANDLE;

    typedef unsigned  long long orxU64;
    typedef unsigned  long      orxU32;
    typedef unsigned  short     orxU16;
    typedef unsigned  char      orxU8;

    typedef signed    long long orxS64;
    typedef signed    long      orxS32;
    typedef signed    short     orxS16;
    typedef signed    char      orxS8;

    typedef unsigned  long      orxBOOL;

    typedef float               orxFLOAT;

    typedef char                orxCHAR;
    typedef orxCHAR *           orxSTRING;

    typedef orxU32              orxRGBA;
    typedef orxU32              orxENUM;

    #define orx2F(V)            (orxFLOAT)(V)

    #define orx2RGBA(R, G, B, A) ((((R) & 0xFF) << 24) | (((G) & 0xFF) << 16) | (((B) & 0xFF) << 8) | ((A) & 0xFF))
    #define orxRGBA_R(RGBA)     (orxU8)(((RGBA) >> 24) & 0xFF)
    #define orxRGBA_G(RGBA)     (orxU8)(((RGBA) >> 16) & 0xFF)
    #define orxRGBA_B(RGBA)     (orxU8)(((RGBA) >> 8) & 0xFF)
    #define orxRGBA_A(RGBA)     (orxU8)((RGBA) & 0xFF)
    #define orxRGBA_NORMALIZER  (orx2F(1.0f / 255.0f))

    #define orxENUM_NONE        0xFFFFFFFFL

  #endif /* __orxLINUX__ || __orxMAC__ || __orxGP2X__ */

#endif /* __orxWINDOWS__ */

/* *** Misc constants *** */

/* *** Boolean constants *** */
extern orxDLLAPI orxCONST orxBOOL     orxFALSE;
extern orxDLLAPI orxCONST orxBOOL     orxTRUE;

/* *** Undefined constants *** */
extern orxDLLAPI orxCONST orxU64      orxU64_UNDEFINED;
extern orxDLLAPI orxCONST orxU32      orxU32_UNDEFINED;
extern orxDLLAPI orxCONST orxU16      orxU16_UNDEFINED;
extern orxDLLAPI orxCONST orxU8       orxU8_UNDEFINED;
extern orxDLLAPI orxCONST orxHANDLE   orxHANDLE_UNDEFINED;

extern orxDLLAPI orxCONST orxSTRING   orxSTRING_EMPTY;
extern orxDLLAPI orxCONST orxSTRING   orxSTRING_TRUE;
extern orxDLLAPI orxCONST orxSTRING   orxSTRING_FALSE;
extern orxDLLAPI orxCONST orxCHAR     orxCHAR_NULL;
extern orxDLLAPI orxCONST orxCHAR     orxCHAR_CR;
extern orxDLLAPI orxCONST orxCHAR     orxCHAR_LF;

extern orxDLLAPI orxCONST orxCHAR     orxCHAR_EOL;
extern orxDLLAPI orxCONST orxSTRING   orxSTRING_EOL;


/* *** Directory separators *** */

extern orxDLLAPI orxCONST orxCHAR     orxCHAR_DIRECTORY_SEPARATOR_WINDOWS;
extern orxDLLAPI orxCONST orxCHAR     orxCHAR_DIRECTORY_SEPARATOR_LINUX;
extern orxDLLAPI orxCONST orxSTRING   orxSTRING_DIRECTORY_SEPARATOR_WINDOWS;
extern orxDLLAPI orxCONST orxSTRING   orxSTRING_DIRECTORY_SEPARATOR_LINUX;

extern orxDLLAPI orxCONST orxCHAR     orxCHAR_DIRECTORY_SEPARATOR;
extern orxDLLAPI orxCONST orxSTRING   orxSTRING_DIRECTORY_SEPARATOR;


/* *** Float constants *** */
extern orxDLLAPI orxCONST orxFLOAT    orxFLOAT_0;
extern orxDLLAPI orxCONST orxFLOAT    orxFLOAT_1;


/* *** Status defines *** */
typedef enum __orxSTATUS_t
{
  orxSTATUS_SUCCESS = 0,        /**< Success status, the operation has worked has expected */
  orxSTATUS_FAILURE,            /**< Failure status, the operation has failed */

  orxSTATUS_NUMBER,             /**< Sentinel : Number of status */

  orxSTATUS_NONE = orxENUM_NONE /**< Invalid status */

} orxSTATUS;


#endif /*_orxTYPE_H_*/


/** @} */
