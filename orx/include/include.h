#ifndef _INCLUDE_H_
#define _INCLUDE_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>

#include <math.h>

/*** Boolean Definitions ***/
#define FALSE       	    0x00000000
#define TRUE        	    (1 == 1)

/*** Types Definitions ***/

/* Base */

typedef unsigned  long long uint64;
typedef unsigned  long      uint32;
typedef unsigned  short     uint16;
typedef unsigned  char      uint8;
typedef unsigned  char      uchar;

typedef signed    long long int64;
typedef signed    long      int32;
typedef signed    short     int16;
typedef signed    char      int8;

typedef unsigned  long      udword;
typedef unsigned  short     uword;
typedef unsigned  char      ubyte;
typedef signed    long      dword;
typedef signed    short     word;
typedef signed    char      byte;

typedef unsigned  long      bool;

#define KUL_UNDEFINED       0xFFFFFFFF


/* Graph */
typedef int32 graph_st_bitmap;
typedef int32 graph_st_sprite;


/*** Memory definitions ***/

/*** Return Code definitions ***/

/*** String Definitions ***/
#define KZ_STRING_NULL      ""

/*** Math Definitions ***/
#define KF_MATH_SQRT2       1.414213562f

#endif /* _INCLUDE_H_ */

