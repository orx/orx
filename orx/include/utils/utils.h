/***************************************************************************
 utils.h
 package utils

 begin                : 16/09/2002
 author               : (C) Gdp
 email                : snegri@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _UTILS_H_
#define _UTILS_H_

#include "orxInclude.h"


/* define pos */
#define MAP_BEGIN  0
#define MAP_END   -1

/* define position type */
#define TREE_CHILD 0
#define TREE_SIBLING 1

/* define search type */
#define DEPTH_SEARCH 0
#define WIDTH_SEARCH 1

typedef orxVOID *map;
typedef orxVOID *vector;
typedef orxVOID *stack;
typedef orxVOID *tree;
typedef orxU8   *tree_cell_data;
typedef orxVOID *tree_cell;


typedef orxBOOL(*pfn_cmp_fct)(orxU8 *key1, orxU8 *key2);

/* the type can be an indication for the function to know what to compare */
typedef orxBOOL(*pfn_tree_cmp_fct)(orxVOID *key, tree_cell mycell, orxVOID *type);

/*************************************************/ 
/* map definitions */
/*************************************************/ 

/*
 * create a map
 *  if pfn_cmp is orxNULL, the == is used to compare 2 int
 */

map map_create(orxU32 key_size, orxU32 value_size, pfn_cmp_fct pfn_cmp);

/*
 * Add an entry to the given map.
 *  the entry is copied since the map becomes the owner of the data
 *  if an entry with the same key already exists it s freed
 */

orxBOOL map_add(map mymap, orxU8 *p_key, orxU8 *p_value);

/*
 * return the corresponding data
 * the map still remain the owner of the data, DO NOT DELETE IT
 */

orxU8 *map_find(map mymap, orxU8 *p_key);

/*
 * delete the corresponding map entry
 *  the data is freed
 */

orxBOOL map_delete(map mymap, orxU8 *p_key);

/*
 * destroy the given map, everyting is freed
 */

orxBOOL map_destroy(map mymap);

/*
 * return the number of element in the map
 */

orxU32 map_count(map mymap);

/*************************************************/ 
/* vector definitions */
/*************************************************/ 

/*
 * create a vector
 */

vector vector_create(orxU32 value_size);

/*
 * set the value of the given pos
 *  the old entry is freed
 *  the pos must be 0 <= pos < vector_count
 */

orxBOOL vector_set(vector myvector, orxU32 pos, orxU8 *p_value);

/*
 * add at the vector end the given data
 */

orxBOOL vector_pushback(vector myvector, orxU8 *p_value);

/*
 * insert at pos = 0 the given data, everything is shifted one value up
 */

orxBOOL vector_insert(vector myvector, orxU8 *p_value);

/*
 * return the data at the given pos
 *  vector still remain owner of the data
 */

orxU8 *vector_get(vector myvector, orxU32 pos);

/*
 * delete and free data at the given pos
 */

orxBOOL vector_delete(vector myvector, orxU32 pos);

/*
 * destroy and free the given vector
 */

orxBOOL vector_destroy(vector myvector);

/*
 * return the number of element of the given vector
 */

orxU32 vector_count(vector myvector);

/*************************************************/ 
/* stack definitions */
/*************************************************/ 

/*
 * create a stack
 */

stack stack_create(orxU32 value_size);

/*
 * push on top of the stack the given value
 */

orxBOOL stack_push(stack mystack, orxU8 *p_value);

/*
 * return the top of the stack and delete it from the stack
 *  the caller becomes the owner of the data
 */

orxU8 *stack_pop(stack mystack);

/*
 * look at the top of the stack
 *  the stack still remain owner of the data
 */

orxU8 *stack_peek(stack mystack);

/*
 * delete and free the given stack
 */

orxBOOL stack_destroy(stack mystack);

/*
 * return the number of element of the given stack
 */

orxU32 stack_count(stack mystack);

/*************************************************/ 
/* tree definitions */
/*************************************************/ 

/**
 * Create a tree
 * 
 * @return the created tree
 */

tree tree_create(orxU32 tree_cell_size, pfn_tree_cmp_fct pfn_tree_cmp);

/**
 * Return the data of the cell
 *
 */

orxU8 *tree_cell_get_data(tree_cell mycell);

/**
 * Return the given cell's parent
 * 
 */

tree_cell tree_get_parent(tree_cell mycell);


/**
 * Return the first child of the given cell
 * 
 */

tree_cell tree_get_child(tree_cell mycell);


/**
 * Return the first sibling of the given cell
 * 
 */

tree_cell tree_get_sibling(tree_cell mycell);


/**
 * Return the found cell
 * 
 * @param tree the tree
 * @param cell the starting cell, if orxNULL starting from root
 * @param key  the key to find, the key can be of SNMP OID form
 * @param type indication for the cmp fct to know what to check
 * @param method_type either depth or width
 */

tree_cell tree_find(tree mytree, tree_cell mycell, orxVOID *key, orxVOID *type, short method_type);

/* IMPORTANT : this function is very important should be redefined */

/**
 * Move the given cell to the new position
 *
 * @param position_type either CHILD or SIBLING
 */

orxBOOL tree_move_cell(tree_cell mycell_pos, tree_cell mycell_data, short position_type);

/**
 * Is the given cell a leaf ?
 * 
 */

orxBOOL tree_is_leaf(tree_cell mycell);


/**
 * Add to the given cell a sibling
 * 
 */

orxBOOL tree_add_sibling(tree mytree, tree_cell mycell_pos, tree_cell_data mydata);


/**
 * Add child
 * 
 */

orxBOOL tree_add_child(tree mytree, tree_cell mycell_pos, tree_cell_data mydata);


/**
 * Remove a cell
 * 
 */

orxBOOL tree_remove(tree_cell mycell);

/* maybe we should use a key here instead of the cell */

/**
 * Destroy and clean the given tree
 * 
 */

orxVOID tree_destroy(tree mytree);


/**
 * Return the number of child of the given cell (at first level)
 * 
 */

orxU32 tree_child_number(tree_cell mycell);


/**
 * Return the number of sibling of the given cell
 * 
 */

orxU32 tree_sibling_number(tree_cell mycell);

#endif /* _UTILS_H_ */
