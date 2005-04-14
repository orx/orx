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
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef _orxTYPE_H_
#define _orxTYPE_H_

/* *** Uses standard defines *** */
#include <stddef.h>

/* *** Uses orx declarations *** */
#include "base/orxDecl.h"

/* *** Platform depedant base types */

/* Windows */
#ifdef WINDOWS

  typedef void                orxVOID;
  
  typedef orxVOID            *orxHANDLE;
  
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

  typedef orxS8               orxCHAR;
  typedef orxCHAR            *orxSTRING;

#else /* WINDOWS */

  /* Linux */
  #ifdef LINUX
      
    typedef void                orxVOID;
  
    typedef orxVOID            *orxHANDLE;
  
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

    typedef orxS8               orxCHAR;
    typedef orxCHAR            *orxSTRING;

  #endif /* LINUX */
  
#endif /* WINDOWS */  


/* *** Boolean Defines *** */
orxSTATIC orxCONST   orxBOOL   orxFALSE            = (orxBOOL)(1 != 1);
orxSTATIC orxCONST   orxBOOL   orxTRUE             = (orxBOOL)(1 == 1);

/* *** Undefined defines *** */
orxSTATIC orxCONST   orxU64    orxU64_Undefined    = (orxU64)(-1);
orxSTATIC orxCONST   orxU32    orxU32_Undefined    = (orxU32)(-1);
orxSTATIC orxCONST   orxU16    orxU16_Undefined    = (orxU16)(-1);
orxSTATIC orxCONST   orxU8     orxU8_Undefined     = (orxU8)(-1);
orxSTATIC orxCONST   orxHANDLE orxHANDLE_Undefined = (orxHANDLE)(-1);


/* *** Status defines *** */
typedef enum __orxSTATUS_t
{
  orxSTATUS_SUCCESS = 0,
  orxSTATUS_FAILED,

  orxSTATUS_NUMBER,

  orxSTATUS_NONE = 0xFFFFFFFF

} orxSTATUS;


/* Graph */
typedef orxS32 graph_st_bitmap;
typedef orxS32 graph_st_sprite;


#endif /*_orxTYPE_H_*/
