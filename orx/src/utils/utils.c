/***************************************************************************
 utils.c
 package utils, 

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
 ****************************************************************************/

#include "utils/utils.h"

#include "memory/orxMemory.h"

#include <string.h>

/* definitions */
typedef struct struct_map privmap;
typedef struct struct_map_cell map_cell;
typedef struct struct_tree privtree;
typedef struct struct_tree_cell priv_tree_cell;

struct struct_map_cell
{
    orxU8 *p_uc_hash_key;
    orxU8 *p_uc_hash_value;
    struct struct_map_cell *p_next_cell;

  /* 4 extra bytes of padding : 16 */
  orxU8 au8Unused[4];
};

struct struct_map
{
    pfn_cmp_fct cellcmp;
    orxU32 value_size;
    orxU32 key_size;
    orxU32 count;
    map_cell *p_cell;

  /* 5 extra bytes of padding : 16 */
  orxU8 au8Unused[5];
};

struct struct_tree_cell
{
    orxU8 *p_data;

    struct struct_tree_cell *p_parent;
    struct struct_tree_cell *p_first_child;
    struct struct_tree_cell *p_right_sibling;
    struct struct_tree_cell *p_left_sibling;

  /* 12 extra bytes of padding : 32 */
  orxU8 au8Unused[12];
};

struct struct_tree
{
    pfn_tree_cmp_fct cell_cmp_fct;
    orxU32 cell_size;

    priv_tree_cell *p_tree_root;

  /* 7 extra bytes of padding : 16 */
  orxU8 au8Unused[7];
};

/* forward declarations */
static map_cell *map_goto(privmap *p_map, orxS32 pos);
static orxU32 map_get_value_size(privmap *p_map);
static map_cell *map_find_cell(privmap *p_map, orxU8 *p_key);
static orxVOID tree_delete_from(priv_tree_cell *p_cell);

/* NOT STABLE, do not use */
//static orxBOOL map_insert(privmap *p_map, orxU32 pos, orxU8 *p_key, orxU8 *p_value);
/* NOT STABLE */

/******************************************************
 * map functions
 ******************************************************/
map map_create(orxU32 key_size, orxU32 value_size, pfn_cmp_fct pfn_cmp)
{
    privmap *p_map = (privmap *)orxMemory_Allocate(sizeof(privmap), orxMEMORY_TYPE_MAIN);

    if (p_map)
    {
        p_map->cellcmp    = pfn_cmp;
        p_map->value_size = value_size;
        p_map->key_size   = key_size;
        p_map->count      = 0;
        p_map->p_cell     = orxNULL;
    }

    return (map) p_map;
}

orxBOOL map_add(map mymap, orxU8 *p_key, orxU8 *p_value)
{
    privmap  *p_map          = (privmap *)mymap;
    map_cell *p_currmap_cell = orxNULL;
    map_cell *p_cell         = orxNULL;
    map_cell *p_old_cell     = orxNULL;

    if (p_map == orxNULL)
        return orxFALSE;

    /* try to find if an entry with this key already exists */
    p_old_cell = map_find_cell(mymap, p_key);

    if (p_old_cell != orxNULL)
    {
        /* yes, so free it and reassign new data */
        orxMemory_Free(p_old_cell->p_uc_hash_value);

        p_old_cell->p_uc_hash_value = (orxU8*)orxMemory_Allocate(p_map->value_size * sizeof(orxU8), orxMEMORY_TYPE_MAIN);
        orxMemory_Copy(p_old_cell->p_uc_hash_value, p_value, p_map->value_size);
        
        return orxTRUE;
    }

    p_cell = (map_cell *)orxMemory_Allocate(sizeof(map_cell), orxMEMORY_TYPE_MAIN);
    
    p_cell->p_uc_hash_value = (orxU8 *)orxMemory_Allocate(p_map->value_size * sizeof(orxU8), orxMEMORY_TYPE_MAIN);
    orxMemory_Copy(p_cell->p_uc_hash_value, p_value, p_map->value_size);
    
    p_cell->p_uc_hash_key = (orxU8 *)orxMemory_Allocate(p_map->key_size * sizeof(orxU8), orxMEMORY_TYPE_MAIN);
    orxMemory_Copy(p_cell->p_uc_hash_key, p_key, p_map->key_size);
    
    p_cell->p_next_cell = orxNULL;
    
    if (map_count(p_map) == 0)
    {
        p_map->p_cell = p_cell;
    }
    else
    {
        p_currmap_cell = map_goto(p_map, MAP_END);
        
        p_currmap_cell->p_next_cell = p_cell;
    }
    
    p_map->count++;
    return orxTRUE;
}

orxU8 *map_find(map mymap, orxU8 *p_key)
{
    map_cell *p_currmap_cell;
    privmap *p_map = (privmap *)mymap;

    if (p_map == orxNULL)
        return orxNULL;
    
    p_currmap_cell = p_map->p_cell;

    while(p_currmap_cell)
    {
        /* if comparaison function == orxNULL, by default use == operator for orxS32 key*/
        if ((p_map->cellcmp == orxNULL && (*(orxS32 *)p_currmap_cell->p_uc_hash_key == *(orxS32 *)p_key))
            || (p_map->cellcmp != orxNULL && p_map->cellcmp(p_currmap_cell->p_uc_hash_key, p_key)))
        {
            /* found the corresponding cell ? */
            return p_currmap_cell->p_uc_hash_value;
        }

        p_currmap_cell = p_currmap_cell->p_next_cell;
    }

    return orxNULL;
}

orxBOOL map_delete(map mymap, orxU8 *p_key)
{
    map_cell *p_currmap_cell;
    map_cell *p_lastmap_cell;
    privmap *p_map = (privmap *)mymap;

    if (p_map == orxNULL)
        return orxTRUE;
    
    p_currmap_cell = p_lastmap_cell = p_map->p_cell;

    while(p_currmap_cell)
    {
        if ((p_map->cellcmp == orxNULL && (*(orxS32 *)p_currmap_cell->p_uc_hash_key == *(orxS32 *)p_key))
            || (p_map->cellcmp != orxNULL && p_map->cellcmp(p_currmap_cell->p_uc_hash_key, p_key)))
        {
            /* found the corresponding cell ? */
           
            /* is it the first one ?*/
            if (p_currmap_cell == p_map->p_cell)
            {
                p_map->p_cell = p_currmap_cell->p_next_cell;
            }
            else
            {
                p_lastmap_cell->p_next_cell = p_currmap_cell->p_next_cell;
            }

            orxMemory_Free(p_currmap_cell->p_uc_hash_value);
            orxMemory_Free(p_currmap_cell->p_uc_hash_key);
            orxMemory_Free(p_currmap_cell);
            
            p_map->count--;
            return orxTRUE;
        }

        p_lastmap_cell = p_currmap_cell;
        p_currmap_cell = p_currmap_cell->p_next_cell;
    }

    return orxFALSE;
}

orxBOOL map_destroy(map mymap)
{
    map_cell *p_currmap_cell;
    map_cell *p_nextmap_cell;
    privmap *p_map = (privmap *)mymap;

    if (p_map == orxNULL)
        return orxTRUE;

    p_currmap_cell = p_map->p_cell;
    while(p_currmap_cell)
    {
        p_nextmap_cell = p_currmap_cell;
        
        orxMemory_Free(p_currmap_cell->p_uc_hash_value);
        orxMemory_Free(p_currmap_cell->p_uc_hash_key);
        orxMemory_Free(p_currmap_cell);

        p_currmap_cell = p_nextmap_cell;
    }

    orxMemory_Free(p_map);
    return orxTRUE;
}


orxU32 map_count(map mymap)
{
    privmap *p_map = (privmap *)mymap;

    if (p_map == orxNULL)
        return 0;

    return p_map->count;
}


/******************************************************
 * vector functions
 ******************************************************/

vector vector_create(orxU32 value_size)
{
    return map_create(sizeof(int), value_size, orxNULL);
}

orxBOOL vector_set(vector myvector, orxU32 pos, orxU8 *p_value)
{
    if (pos >= vector_count(myvector))
        return orxFALSE;

    return map_add(myvector, (orxU8 *)&pos, p_value);
}

orxBOOL vector_pushback(vector myvector, orxU8 *p_value)
{
    orxU32 ikey = vector_count(myvector);
    return map_add(myvector, (orxU8 *)&ikey, p_value);
}

orxBOOL vector_insert(vector myvector, orxU8 *p_value)
{
    privmap *p_map           = (privmap *)myvector;
    map_cell *p_currmap_cell = orxNULL;
    orxU32 ikey                = MAP_BEGIN;

    if (p_map == orxNULL)
        return orxFALSE;

    p_currmap_cell = p_map->p_cell;
    while(p_currmap_cell)
    {
        /* since it s a vector the key is an orxS32 */
        /* just add 1 to each key to shift everything up */
        *((orxS32 *)p_currmap_cell->p_uc_hash_key) = *((orxS32 *)p_currmap_cell->p_uc_hash_key) + 1;
        
        p_currmap_cell = p_currmap_cell->p_next_cell;
    }

    /* add key = 0 */
    /* SN: this can be optimized by inserting this cell at the list beginning and */
    /* not at the end */
    return map_add(myvector, (orxU8 *)&ikey, p_value);
}

orxU8 *vector_get(vector myvector, orxU32 pos)
{
    return map_find(myvector, (orxU8 *)&pos);
}

orxBOOL vector_delete(vector myvector, orxU32 pos)
{
    return map_delete(myvector, (orxU8 *)&pos);
}

orxBOOL vector_destroy(vector myvector)
{
    return map_destroy(myvector);
}

orxU32 vector_count(vector myvector)
{
    return map_count(myvector);
}
 

/******************************************************
 * stack functions
 ******************************************************/

stack stack_create(orxU32 value_size)
{
    return vector_create(value_size);
}

orxBOOL stack_push(stack mystack, orxU8 *p_value)
{
    return vector_pushback(mystack, p_value);
}

orxU8 *stack_pop(stack mystack)
{
    orxU8 *p_value = stack_peek(mystack);

    if (p_value != orxNULL)
    {
        vector_delete(mystack, stack_count(mystack) - 1);
    }

    return p_value;
}

orxU8 *stack_peek(stack mystack)
{
    orxU32 pos = stack_count(mystack) - 1;
    orxU8 *p_value = vector_get(mystack, pos);
    orxU32  size = map_get_value_size(mystack);
    orxU8 *tmpvalue = orxNULL;
    
    if (p_value != orxNULL)
    {
        tmpvalue = (orxU8 *)sizeof(size * sizeof(orxU8));
        orxMemory_Copy(tmpvalue, p_value, size);
    }

    return tmpvalue;
}

orxBOOL stack_destroy(stack mystack)
{
    return vector_destroy(mystack);
}

orxU32 stack_count(stack mystack)
{
    return vector_count(mystack);
}


/******************************************************
 * tree functions
 ******************************************************/
/* TO BE DONE, use find with OID */
tree_cell tree_find(tree mytree, tree_cell mycell, orxVOID *key, orxVOID *type, short method_type)
{
    privtree *p_tree = (privtree *) mytree;
    priv_tree_cell *p_cell_start = (priv_tree_cell *) mycell;
    priv_tree_cell *p_current_cell = orxNULL;
    orxBOOL bDone = orxFALSE;

    if (p_tree == orxNULL)
        return (tree_cell) orxNULL;

    /* if start cell is not null start find from this position */
    if (p_cell_start != orxNULL)
        p_current_cell = p_cell_start;
    else
        p_current_cell = p_tree->p_tree_root->p_first_child; /* start directly here to save one test */

    while (p_current_cell && (p_current_cell != p_tree->p_tree_root))
    {
        if (!bDone && p_tree->cell_cmp_fct(key, (tree_cell) p_current_cell, type))
        {
            /* we find it, return it */
            return (tree_cell) p_current_cell;
        }


        /* assign next cell according to method */
        if (method_type == DEPTH_SEARCH)
        {
            if (!bDone && p_current_cell->p_first_child)
            {
                p_current_cell = p_current_cell->p_first_child;
                bDone = orxFALSE;
            }
            else if (p_current_cell->p_right_sibling)
            {
                p_current_cell = p_current_cell->p_right_sibling;
                bDone = orxFALSE;
            }
            else
            {
                /* no child, no sibling, go up one level */
                p_current_cell = p_current_cell->p_parent;

                bDone = orxTRUE;
            }
        }
        else if (method_type == WIDTH_SEARCH)
        {
            /* to be DONE */

            /* if (!bDone && p_current_cell->p_right_sibling) */
/*             { */
/*                 p_current_cell = p_current_cell->p_right_sibling; */
/*                 bDone = orxFALSE; */
/*             } */
/*             else if (p_current_cell->p_first_child) */
/*             { */
/*                 p_current_cell = p_current_cell->p_first_child; */
/*                 bDone = orxFALSE; */
/*             } */
/*             else */
/*             { */
/*                 /\* no sibling, no child, go back to left *\/ */
/*                 if (p_current_cell->p_left_sibling) */
/*                     p_current_cell = p_current_cell->p_left_sibling; */
/*                 else */
/*                     p_current_cell = p_current_cell->p_parent; */

/*                 bDone = orxTRUE; */
/*             } */
        }
    }

    return (tree_cell) orxNULL;
}

tree tree_create(orxU32 tree_cell_size, pfn_tree_cmp_fct pfn_tree_cmp)
{
    privtree *p_tree = (privtree *)orxMemory_Allocate(sizeof(privtree), orxMEMORY_TYPE_MAIN);

    if (p_tree)
    {
        p_tree->cell_cmp_fct = pfn_tree_cmp;
        p_tree->cell_size    = tree_cell_size;
        p_tree->p_tree_root  = orxNULL;
    }

    return (tree) p_tree;
}

orxU8 *tree_cell_get_data(tree_cell mycell)
{
    priv_tree_cell *p_cell = (priv_tree_cell *) mycell;

    if (p_cell)
        return p_cell->p_data;

    return orxNULL;
}

tree_cell tree_get_parent(tree_cell mycell)
{
    priv_tree_cell *p_cell = (priv_tree_cell *) mycell;

    if (p_cell)
        return (tree_cell *)p_cell->p_parent;

    return orxNULL;
}

tree_cell tree_get_child(tree_cell mycell)
{
    priv_tree_cell *p_cell = (priv_tree_cell *) mycell;

    if (p_cell)
        return (tree_cell *)p_cell->p_first_child;

    return orxNULL;
}

tree_cell tree_get_sibling(tree_cell mycell)
{
    priv_tree_cell *p_cell = (priv_tree_cell *) mycell;

    if (p_cell)
        return (tree_cell *)p_cell->p_right_sibling;

    return orxNULL;
}

orxBOOL tree_move_cell(tree_cell mycell_pos, tree_cell mycell_data, short position_type)
{
    priv_tree_cell *p_cell_pos = (priv_tree_cell *) mycell_pos;
    priv_tree_cell *p_cell_data = (priv_tree_cell *) mycell_data;
    priv_tree_cell *p_temp_cell = orxNULL;

    if (p_cell_pos == orxNULL ||
        p_cell_data == orxNULL)
        return orxFALSE;

    /* change all reference to mycell_data */
    /* check if cell data is a first child */
    if (p_cell_data->p_parent->p_first_child == p_cell_data)
    {
        if (p_cell_data->p_right_sibling != orxNULL)
        {
            p_cell_data->p_right_sibling->p_left_sibling = orxNULL;
        }

        p_cell_data->p_parent->p_first_child = p_cell_data->p_right_sibling;
    }
    else
    {
        /* no, so it s just a sibling */
        p_cell_data->p_left_sibling->p_right_sibling = p_cell_data->p_right_sibling;

        if (p_cell_data->p_right_sibling != orxNULL)
        {
            p_cell_data->p_right_sibling->p_left_sibling = p_cell_data->p_left_sibling;
        }
    }

    /* check what is the type */
    if (position_type == TREE_CHILD)
    {
        /* test if first child exists */
        if (p_cell_pos->p_first_child != orxNULL)
        {
            p_temp_cell = p_cell_pos->p_first_child;
        }
        else
        {
            p_cell_pos->p_first_child = p_cell_data;
            p_cell_data->p_parent = p_cell_pos;
            p_cell_data->p_right_sibling = orxNULL;
            p_cell_data->p_left_sibling = orxNULL;
        }
    }
    else if (position_type == TREE_SIBLING)
    {
        p_temp_cell = p_cell_pos;
    }

    /* common case */
    if (p_temp_cell != orxNULL)
    {
        p_cell_data->p_right_sibling = p_temp_cell->p_right_sibling;
        
        if (p_cell_data->p_right_sibling != orxNULL)
        {
            p_cell_data->p_right_sibling->p_left_sibling = p_cell_data;
        }

        p_temp_cell->p_right_sibling = p_cell_data;
        p_cell_data->p_parent = p_temp_cell->p_parent;
        p_cell_data->p_left_sibling = p_temp_cell;
    }

    return orxTRUE;
}

orxBOOL tree_is_leaf(tree_cell mycell)
{
    priv_tree_cell *p_cell = (priv_tree_cell *) mycell;

    if (p_cell)
        return (p_cell->p_first_child == orxNULL);

    return orxFALSE;
}

orxBOOL tree_add_sibling(tree mytree, tree_cell mycell_pos, tree_cell_data mydata)
{
    privtree *p_tree = (privtree *) mytree;
    priv_tree_cell *p_cell_pos = (priv_tree_cell *) mycell_pos;
    priv_tree_cell *p_cell_new = (priv_tree_cell *)orxMemory_Allocate(sizeof(priv_tree_cell), orxMEMORY_TYPE_MAIN);
    
    if (p_tree == orxNULL ||
        p_cell_pos == orxNULL ||
        p_cell_new == orxNULL)
        return orxFALSE;

    /* assign data */
    p_cell_new->p_data = (orxU8 *)orxMemory_Allocate(p_tree->cell_size * sizeof(orxU8), orxMEMORY_TYPE_MAIN);
    orxMemory_Copy(p_cell_new->p_data, (orxU8 *) mydata, p_tree->cell_size);
    p_cell_new->p_first_child = orxNULL;
    p_cell_new->p_parent = p_cell_pos->p_parent;

    /* insert new cell */
    p_cell_new->p_left_sibling = p_cell_pos;
    p_cell_new->p_right_sibling = p_cell_pos->p_right_sibling;

    p_cell_pos->p_right_sibling = p_cell_new;
    
    return orxTRUE;
}

orxBOOL tree_add_child(tree mytree, tree_cell mycell_pos, tree_cell_data mydata)
{
    privtree *p_tree = (privtree *) mytree;
    priv_tree_cell *p_cell_pos = (priv_tree_cell *) mycell_pos;
    priv_tree_cell *p_cell_new = (priv_tree_cell *)orxMemory_Allocate(sizeof(priv_tree_cell), orxMEMORY_TYPE_MAIN);
    
    if (p_tree == orxNULL ||
        p_cell_pos == orxNULL ||
        p_cell_new == orxNULL)
        return orxFALSE;

    /* assign data */
    p_cell_new->p_data = (orxU8 *)orxMemory_Allocate(p_tree->cell_size * sizeof(orxU8), orxMEMORY_TYPE_MAIN);
    orxMemory_Copy(p_cell_new->p_data, (orxU8 *) mydata, p_tree->cell_size);

    p_cell_new->p_parent = p_cell_pos;
    p_cell_new->p_first_child = orxNULL;
    p_cell_new->p_left_sibling = orxNULL;

    /* if current cell has no child just add it as a child */
    if (p_cell_pos->p_first_child == orxNULL)
    {
        p_cell_new->p_right_sibling = orxNULL;

        p_cell_pos->p_first_child = p_cell_new;
    }
    else /* add it as its first child and move sibling */
    {
        p_cell_new->p_right_sibling = p_cell_pos->p_first_child;

        p_cell_pos->p_first_child->p_left_sibling = p_cell_new;

        p_cell_pos->p_first_child = p_cell_new;
    }
    
    return orxTRUE;
}

orxBOOL tree_remove(tree_cell mycell)
{
    priv_tree_cell *p_cell = (priv_tree_cell *) mycell;
    priv_tree_cell *p_parent, *p_right, *p_left;

    if (!p_cell)
        return orxFALSE;

    p_right  = p_cell->p_right_sibling;
    p_left   = p_cell->p_left_sibling;
    p_parent = p_cell->p_parent;

    if (p_right)
    {
        if (!p_left)
        {
            /* first child */
            p_parent->p_first_child = p_right;
            p_right->p_left_sibling = orxNULL;
        }
        else
        {
            /* just a child */
            p_left->p_right_sibling = p_right;
            p_right->p_left_sibling = p_left;
        }
    }
    else
    {
        /* only child */
        p_parent->p_first_child = orxNULL;
    }
    
    tree_delete_from(p_cell);
    
    return orxTRUE;
}

orxVOID tree_destroy(tree mytree)
{
    privtree *p_tree = (privtree*) mytree;

    tree_delete_from(p_tree->p_tree_root);

    orxMemory_Free(p_tree);
}

orxU32 tree_child_number(tree_cell mycell)
{
    priv_tree_cell *p_cell = (priv_tree_cell *) mycell;
    priv_tree_cell *p_current_cell = (p_cell) ? p_cell->p_first_child : orxNULL;
    orxU32 nb_child = 0;
    
    while (p_current_cell)
    {
        nb_child++;

        p_current_cell = p_current_cell->p_right_sibling;
    }

    return nb_child;
}

orxU32 tree_sibling_number(tree_cell mycell)
{
    priv_tree_cell *p_cell = (priv_tree_cell *) mycell;
    priv_tree_cell *p_current_cell = (p_cell) ? p_cell->p_right_sibling : orxNULL;
    orxU32 nb_sibling = 0;
    
    while (p_current_cell)
    {
        nb_sibling++;

        p_current_cell = p_current_cell->p_right_sibling;
    }

    return nb_sibling;
}

/******************************************************
 * Private functions
 ******************************************************/

static orxU32 map_get_value_size(privmap *p_map)
{
    return p_map->value_size;
}

//static orxBOOL map_insert(privmap *p_map, orxU32 pos, orxU8 *p_key, orxU8 *p_value)
//{
//    map_cell *p_currmap_cell = orxNULL;
//    map_cell *p_cell = orxNULL;
//
//    if (p_map == orxNULL)
//        return orxFALSE;
//
//    p_cell = (map_cell *)orxMemory_Allocate(sizeof(map_cell), orxMEMORY_TYPE_MAIN);
//        
//    p_cell->p_uc_hash_value = (orxU8 *)orxMemory_Allocate(p_map->value_size * sizeof(orxU8), orxMEMORY_TYPE_MAIN);
//    orxMemory_Copy(p_cell->p_uc_hash_value, p_value, p_map->value_size);
//    
//    p_cell->p_uc_hash_key = (orxU8 *)orxMemory_Allocate(p_map->key_size * sizeof(orxU8), orxMEMORY_TYPE_MAIN);
//    orxMemory_Copy(p_cell->p_uc_hash_key, p_key, p_map->key_size);
//
//    if (pos == MAP_BEGIN)
//    {
//        p_cell->p_next_cell = p_map->p_cell;
//        p_map->p_cell = p_cell;
//    }
//    else
//    {
//        p_currmap_cell = map_goto(p_map, pos - 1);
//
//        p_cell->p_next_cell = p_currmap_cell->p_next_cell;
//        p_currmap_cell->p_next_cell = p_cell;
//    }
//    
//    p_map->count++;
//    return orxTRUE;
//}

static map_cell *map_goto(privmap *p_map, orxS32 pos)
{
    orxS32 i = 0;
    map_cell *p_currmap_cell;

    if (p_map == orxNULL)
        return orxNULL;

    p_currmap_cell = p_map->p_cell;

    while(p_currmap_cell)
    {
        if (pos == MAP_END)
        {
            if (p_currmap_cell->p_next_cell == orxNULL)
                return p_currmap_cell;
        }

        if (i == pos)
            return p_currmap_cell;
    
        p_currmap_cell = p_currmap_cell->p_next_cell;

        ++pos;
    }

    return orxNULL;
}


static map_cell *map_find_cell(privmap *p_map, orxU8 *p_key)
{
    map_cell *p_currmap_cell;

    if (p_map == orxNULL)
        return orxNULL;
    
    p_currmap_cell = p_map->p_cell;

    while(p_currmap_cell)
    {
        /* if comparaison function == orxNULL, by default use == operator for orxS32 key*/
        if ((p_map->cellcmp == orxNULL && (*(orxS32 *)p_currmap_cell->p_uc_hash_key == *(orxS32 *)p_key))
            || (p_map->cellcmp != orxNULL && p_map->cellcmp(p_currmap_cell->p_uc_hash_key, p_key)))
        {
            /* found the corresponding cell ? */
            return p_currmap_cell;
        }

        p_currmap_cell = p_currmap_cell->p_next_cell;
    }

    return orxNULL;
}


static orxVOID tree_delete_from(priv_tree_cell *p_cell)
{
    priv_tree_cell *p_current_cell, *p_old_cell;
    
    p_current_cell = p_cell->p_first_child;
    /* remove and clean all the sub_tree */
    while (p_current_cell != p_cell)
    {
        p_old_cell = p_current_cell;
        if(p_current_cell->p_first_child)
            p_current_cell = p_current_cell->p_first_child;
        else
        {
            if (p_current_cell->p_right_sibling)
                p_current_cell = p_current_cell->p_right_sibling;
            else
            {
                p_current_cell = p_current_cell->p_parent;
                p_current_cell->p_first_child = orxNULL;
            }

            orxMemory_Free(p_old_cell->p_data);
            orxMemory_Free(p_old_cell);
        }
    }

    /* delete the last cell */
    orxMemory_Free(p_cell->p_data);
    orxMemory_Free(p_cell);
}
