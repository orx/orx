#ifndef _orxINCLUDE_H_
#define _orxINCLUDE_H_

//#include <stdio.h>
//#include <stdlib.h>
//#include <stdarg.h>
//#include <stddef.h>


/* *** TODO LIST
 * C Rewrite utils module : adds storages structures (chunk bank) AND organizing structures (list, tree, graph, hash table)
 * - Rewrite include hierarchy / group includes/init/exit by module
 * - Add unicode support
 * - Replace simple mallocs in written modules with banks
 * - Simplify structure module (everything controlled from there / no cross references?)
 * - Add BV/box module + extract AABox code from vector module
 * C Rewrite sound module/plugin + timer module/plugin
 * V Add CRC
 * - Add string handling (using CRC + hash)
 * V Add texture referencing with duplication check -> reference increase
 * - Remove ref point from texture to a higher graph object (not created yet)
 * - Rewrite graph system using modulable architecture (2D data => sprite / 3D => mesh/skel...)
 * - Rewrite anim system using packed format (indexing + key) + data type separation at a lower level
 * - Add animation format, expendable for 3D (with anim_key/anim_channel structures)
 * - Add multi animation interpreter (for blends, transitions, etc...)
 * C Add fixed point floats + ops + converts (replace (orxFLOAT) casts by a convert function)
 * V Change coord in orxVEC with all new functions
 * - Add Matrix type + functions
 * - Update camera to use matrix + vector
 * - Update camera to use 3D-like computation
 * - Adds rendering context to camera
 * - Modify timer plugin to make plugin side simpler
 * - Add timer IDs, make game/real (=>IDs) time accessors generic
 * - Animation shouldn't use directly textures but wrapping anim_atomic
 * C Simplify plugin declaration with macros/enums instead of flag IDs
 * - Adds interactors
 * - Adds events
 * C Adds LUA integration
 * V Complete orxBANK implementation
 * V Complete orxHashMap module (refactor to orxHash and complete test module)
 * V Rewrite File and Package module to use plugins system
 * C Write plugins implementation for file and package management
 * V Complete test module and add a generic makefile to compile each test file as an automatically loaded dynamic library
 * C Write a module to manage string (overload string.h functionalities) + add other functionalities
 * - Add network :)
 * - Add FSM
 * - Add waypoint/way system
 * - Add physics
 * - Add menu/ui plugin/support (???)
 * - Use a real task manager :)
 */


/* *** Types include *** */
#include "base/orxType.h"


/*** Memory definitions ***/

/*** Return Code definitions ***/


#endif /* _orxINCLUDE_H_ */
