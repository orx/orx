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

#include "include.h"


/* define pos */
#define MAP_BEGIN  0
#define MAP_END   -1

/* define position type */
#define TREE_CHILD 0
#define TREE_SIBLING 1

/* define search type */
#define DEPTH_SEARCH 0
#define WIDTH_SEARCH 1

typedef void *map;
typedef void *vector;
typedef void *stack;
typedef void *tree;
typedef uchar *tree_cell_data;
typedef void *tree_cell;


typedef bool(*pfn_cmp_fct)(uchar *key1, uchar *key2);

/* the type can be an indication for the function to know what to compare */
typedef bool(*pfn_tree_cmp_fct)(void *key, tree_cell mycell, void *type);

/*************************************************/ 
/* map definitions */
/*************************************************/ 

/*
 * create a map
 *  if pfn_cmp is NULL, the == is used to compare 2 int
 */

map map_create(uint32 key_size, uint32 value_size, pfn_cmp_fct pfn_cmp);

/*
 * Add an entry to the given map.
 *  the entry is copied since the map becomes the owner of the data
 *  if an entry with the same key already exists it s freed
 */

bool map_add(map mymap, uchar *p_key, uchar *p_value);

/*
 * return the corresponding data
 * the map still remain the owner of the data, DO NOT DELETE IT
 */

uchar *map_find(map mymap, uchar *p_key);

/*
 * delete the corresponding map entry
 *  the data is freed
 */

bool map_delete(map mymap, uchar *p_key);

/*
 * destroy the given map, everyting is freed
 */

bool map_destroy(map mymap);

/*
 * return the number of element in the map
 */

uint32 map_count(map mymap);

/*************************************************/ 
/* vector definitions */
/*************************************************/ 

/*
 * create a vector
 */

vector vector_create(uint32 value_size);

/*
 * set the value of the given pos
 *  the old entry is freed
 *  the pos must be 0 <= pos < vector_count
 */

bool vector_set(vector myvector, uint32 pos, uchar *p_value);

/*
 * add at the vector end the given data
 */

bool vector_pushback(vector myvector, uchar *p_value);

/*
 * insert at pos = 0 the given data, everything is shifted one value up
 */

bool vector_insert(vector myvector, uchar *p_value);

/*
 * return the data at the given pos
 *  vector still remain owner of the data
 */

uchar *vector_get(vector myvector, uint32 pos);

/*
 * delete and free data at the given pos
 */

bool vector_delete(vector myvector, uint32 pos);

/*
 * destroy and free the given vector
 */

bool vector_destroy(vector myvector);

/*
 * return the number of element of the given vector
 */

uint32 vector_count(vector myvector);

/*************************************************/ 
/* stack definitions */
/*************************************************/ 

/*
 * create a stack
 */

stack stack_create(uint32 value_size);

/*
 * push on top of the stack the given value
 */

bool stack_push(stack mystack, uchar *p_value);

/*
 * return the top of the stack and delete it from the stack
 *  the caller becomes the owner of the data
 */

uchar *stack_pop(stack mystack);

/*
 * look at the top of the stack
 *  the stack still remain owner of the data
 */

uchar *stack_peek(stack mystack);

/*
 * delete and free the given stack
 */

bool stack_destroy(stack mystack);

/*
 * return the number of element of the given stack
 */

uint32 stack_count(stack mystack);

/*************************************************/ 
/* tree definitions */
/*************************************************/ 

/**
 * Create a tree
 * 
 * @return the created tree
 */

tree tree_create(uint32 tree_cell_size, pfn_tree_cmp_fct pfn_tree_cmp);

/**
 * Return the data of the cell
 *
 */

uchar *tree_cell_get_data(tree_cell mycell);

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
 * @param cell the starting cell, if NULL starting from root
 * @param key  the key to find, the key can be of SNMP OID form
 * @param type indication for the cmp fct to know what to check
 * @param method_type either depth or width
 */

tree_cell tree_find(tree mytree, tree_cell mycell, void *key, void *type, short method_type);

/* IMPORTANT : this function is very important should be redefined */

/**
 * Move the given cell to the new position
 *
 * @param position_type either CHILD or SIBLING
 */

bool tree_move_cell(tree_cell mycell_pos, tree_cell mycell_data, short position_type);

/**
 * Is the given cell a leaf ?
 * 
 */

bool tree_is_leaf(tree_cell mycell);


/**
 * Add to the given cell a sibling
 * 
 */

bool tree_add_sibling(tree mytree, tree_cell mycell_pos, tree_cell_data mydata);


/**
 * Add child
 * 
 */

bool tree_add_child(tree mytree, tree_cell mycell_pos, tree_cell_data mydata);


/**
 * Remove a cell
 * 
 */

bool tree_remove(tree_cell mycell);

/* maybe we should use a key here instead of the cell */

/**
 * Destroy and clean the given tree
 * 
 */

void tree_destroy(tree mytree);


/**
 * Return the number of child of the given cell (at first level)
 * 
 */

uint32 tree_child_number(tree_cell mycell);


/**
 * Return the number of sibling of the given cell
 * 
 */

uint32 tree_sibling_number(tree_cell mycell);

#endif /* _UTILS_H_ */
