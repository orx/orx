/** 
 * \file object.h
 * 
 * Object Module.
 * Allows to creates and handle objects.
 * Objects are structures that can refer to many other structures such as frames, graphics, etc...
 * 
 * \todo
 * Add the required structures when needed.
 * Add Sweep & Prune and specialized storages depending on purposes.
 */


/***************************************************************************
 object.h
 object module
 
 begin                : 01/12/2003
 author               : (C) Gdp
 email                : iarwain@ifrance.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "include.h"

#include "object/structure.h"


/** Internal object structure. */
typedef struct st_object_t object_st_object;


/** Inits the object system. */
extern uint32               object_init();
/** Ends the object system. */
extern void                 object_exit();

/** Creates an empty object. */
extern object_st_object    *object_create();
/** Deletes an object. */
extern void                 object_delete(object_st_object *_pst_object);

/** Test object render status (TRUE : clean / FALSE : dirty)*/
extern bool                 object_render_status_ok(object_st_object *_pst_object);

/** Links a structure to an object. */
extern void                 object_struct_link(object_st_object *_pst_object, structure_st_struct *_pst_struct);
/** Unlinks structures from an object, given their structure IDs (IDs can be OR'ed).*/
extern void                 object_struct_unlink(object_st_object *_pst_object, uint32 _u32_struct_id);

/** !!! Warning : Object accessors don't test parameter validity !!! */

/** Structure used by an object get accessor, given its structure ID. Structure must be cast correctly. */
extern structure_st_struct *object_struct_get(object_st_object *_pst_object, uint32 _u32_struct_id);

#endif /* _OBJECT_H_ */
