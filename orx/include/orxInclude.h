#ifndef _orxINCLUDE_H_
#define _orxINCLUDE_H_

//#include <stdio.h>
//#include <stdlib.h>
//#include <stdarg.h>
//#include <stddef.h>


/* *** TODO LIST
 * - Rewrite utils module : adds storages structures (chunk bank) AND organizing structures (list, tree, hash table)
 * - Add CRC
 * - Add string handling (using CRC + hash)
 * - Add bitmap referencing in texture with duplication check -> reference increase
 * - Remove ref point from texture to a higher graph object (no created yet)
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


/* *** Types include *** */
#include "base/orxType.h"


/*** Memory definitions ***/

/*** Return Code definitions ***/


#endif /* _orxINCLUDE_H_ */
