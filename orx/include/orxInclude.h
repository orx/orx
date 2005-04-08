#ifndef _orxINCLUDE_H_
#define _orxINCLUDE_H_

#include <stddef.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <stdarg.h>
//#include <stddef.h>


/* *** TODO LIST
 * - Rewrite utils module : adds storages structures (chunk bank) AND organizing structures (list, tree, hash table)
 * - Add fixed point floats + ops + converts (replace (orxFLOAT) casts by a convert function)
 * - Change coord in orxVEC with all new functions
 * - Add Matrix type + functions
 * - Update camera to use matrix + vector
 * - Update camera to use 3D-like computation
 * - Adds rendering context to camera
 * - Modify timer plugin to make plugin side simpler
 * - Add timer IDs, make game/real (=>IDs) time accessors generic
 * - Animation shouldn't use directly textures but wrapping anim_atom
 * - Simplify plugin declaration with macros/enums instead of flag IDs
 * - Adds interactors
 * - Adds event
 * - Adds LUA integration
 * - Complete orxBANK implementation
 * - Rewrite File and Package module to use plugins system
 * - Complete test module and add a generic makefile to compile each test file as an automatically loaded dynamic library
 * - Write a module to manage string (overload string.h functionalities) + add other functionalities
 */


/* *** Types Defines *** */

/* Base */

#define orxCONST            const
#define orxSTATIC           static
#define orxINLINE           inline
#define orxREGISTER         register

#define orxNULL             NULL

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


/*** Memory definitions ***/

/*** Return Code definitions ***/


#endif /* _orxINCLUDE_H_ */
