/** 
 * \file orxType.h
 * 
 * Base types.
 * Defines all base types.
 * 
 * \todo
 * Add the required types at need.
 */


/***************************************************************************
 orxType.h
 Base types
 
 begin                : 12/04/2005
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


#ifndef _orxTYPE_H_
#define _orxTYPE_H_

/* *** Uses orx declarations *** */
#include "base/orxDecl.h"

/* *** Platform depedant base types */

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

  #define orx2F(V)              (orxFLOAT)(V)

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
  #if defined(__orxLINUX__) || defined(__orxMAC__)

    typedef void                orxVOID;
  
    typedef orxVOID *						orxHANDLE;
  
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
    typedef orxCHAR *						orxSTRING;

    typedef orxU32              orxRGBA;

    #define orx2F(V)            (orxFLOAT)(V)

    #define orxENUM_NONE        0xFFFFFFFFL

  #endif /* __orxLINUX__ || __orxMAC__ */
  
#endif /* __orxWINDOWS__ */  


/* *** Boolean Defines *** */
orxSTATIC orxCONST  orxBOOL     orxFALSE            = (orxBOOL)(1 != 1);
orxSTATIC orxCONST  orxBOOL     orxTRUE             = (orxBOOL)(1 == 1);

/* *** Undefined defines *** */
orxSTATIC orxCONST  orxU64      orxU64_UNDEFINED    = (orxU64)(-1);
orxSTATIC orxCONST  orxU32      orxU32_UNDEFINED    = (orxU32)(-1);
orxSTATIC orxCONST  orxU16      orxU16_UNDEFINED    = (orxU16)(-1);
orxSTATIC orxCONST  orxU8       orxU8_UNDEFINED     = (orxU8)(-1);
orxSTATIC orxCONST  orxHANDLE   orxHANDLE_UNDEFINED = (orxHANDLE)(-1);

orxSTATIC orxCONST  orxSTRING   orxSTRING_EMPTY     = "";
orxSTATIC orxCONST  orxSTRING   orxSTRING_TRUE      = "true";
orxSTATIC orxCONST  orxSTRING   orxSTRING_FALSE     = "false";
orxSTATIC orxCONST  orxCHAR     orxCHAR_NULL        = '\0';
orxSTATIC orxCONST  orxCHAR     orxCHAR_EOL         = '\n';
orxSTATIC orxCONST  orxCHAR     orxCHAR_CR          = '\r';


/* *** Directory separators *** */
#define orxCHAR_DIRECTORY_SEPARATOR_WINDOWS         '\\'
#define orxCHAR_DIRECTORY_SEPARATOR_LINUX           '/'
#define orxSTRING_DIRECTORY_SEPARATOR_WINDOWS       "\\"     
#define orxSTRING_DIRECTORY_SEPARATOR_LINUX         "/"

#ifdef __orxWINDOWS__

  #define orxCHAR_DIRECTORY_SEPARATOR               orxCHAR_DIRECTORY_SEPARATOR_WINDOWS
  #define orxSTRING_DIRECTORY_SEPARATOR             orxSTRING_DIRECTORY_SEPARATOR_WINDOWS

#else /* __orxWINDOWS__ */

  /* Linux / Mac */
  #if defined(__orxLINUX__) || defined(__orxMAC__)

    #define orxCHAR_DIRECTORY_SEPARATOR             orxCHAR_DIRECTORY_SEPARATOR_LINUX
    #define orxSTRING_DIRECTORY_SEPARATOR           orxSTRING_DIRECTORY_SEPARATOR_LINUX

  #endif /* __orxLINUX__ || __orxMAC__ */

#endif /* __orxWINDOWS__ */


/* *** Float defines *** */
#define orxFLOAT_0                                  orx2F(0.0f)
#define orxFLOAT_1                                  orx2F(1.0f)


/* *** Status defines *** */
typedef enum __orxSTATUS_t
{
  orxSTATUS_FAILURE = 0,        /**< Failure status, the operation has failed */
  orxSTATUS_SUCCESS,            /**< Success status, the operation has worked has expected */

  orxSTATUS_NUMBER,             /**< Sentinel : Number of status */

  orxSTATUS_NONE = orxENUM_NONE /**< Invalid status */

} orxSTATUS;


#endif /*_orxTYPE_H_*/
