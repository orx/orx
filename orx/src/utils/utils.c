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

#include <string.h>

/* definitions */
typedef struct struct_map privmap;
typedef struct struct_map_cell map_cell;
typedef struct struct_tree privtree;
typedef struct struct_tree_cell priv_tree_cell;

struct struct_map_cell
{
    uchar *p_uc_hash_key;
    uchar *p_uc_hash_value;
    struct struct_map_cell *p_next_cell;

  /* 4 extra bytes of padding : 16 */
  uint8 auc_unused[4];
};

struct struct_map
{
    pfn_cmp_fct cellcmp;
    uint32 value_size;
    uint32 key_size;
    uint32 count;
    map_cell *p_cell;

  /* 5 extra bytes of padding : 16 */
  uint8 auc_unused[5];
};

struct struct_tree_cell
{
    uchar *p_data;

    struct struct_tree_cell *p_parent;
    struct struct_tree_cell *p_first_child;
    struct struct_tree_cell *p_right_sibling;
    struct struct_tree_cell *p_left_sibling;

  /* 12 extra bytes of padding : 32 */
  uint8 auc_unused[12];
};

struct struct_tree
{
    pfn_tree_cmp_fct cell_cmp_fct;
    uint32 cell_size;

    priv_tree_cell *p_tree_root;

  /* 7 extra bytes of padding : 16 */
  uint8 auc_unused[7];
};

/* forward declarations */
static map_cell *map_goto(privmap *p_map, int32 pos);
static uint32 map_get_value_size(privmap *p_map);
static map_cell *map_find_cell(privmap *p_map, uchar *p_key);
static void tree_delete_from(priv_tree_cell *p_cell);

/* NOT STABLE, do not use */
static bool map_insert(privmap *p_map, uint32 pos, uchar *p_key, uchar *p_value);
/* NOT STABLE */

/******************************************************
 * map functions
 ******************************************************/
map map_create(uint32 key_size, uint32 value_size, pfn_cmp_fct pfn_cmp)
{
    privmap *p_map = (privmap *)malloc(sizeof(privmap));

    if (p_map)
    {
        p_map->cellcmp    = pfn_cmp;
        p_map->value_size = value_size;
        p_map->key_size   = key_size;
        p_map->count      = 0;
        p_map->p_cell     = NULL;
    }

    return (map) p_map;
}

bool map_add(map mymap, uchar *p_key, uchar *p_value)
{
    privmap  *p_map          = (privmap *)mymap;
    map_cell *p_currmap_cell = NULL;
    map_cell *p_cell         = NULL;
    map_cell *p_old_cell     = NULL;

    if (p_map == NULL)
        return FALSE;

    /* try to find if an entry with this key already exists */
    p_old_cell = map_find_cell(mymap, p_key);

    if (p_old_cell != NULL)
    {
        /* yes, so free it and reassign new data */
        free(p_old_cell->p_uc_hash_value);

        p_old_cell->p_uc_hash_value = (uchar*)malloc(p_map->value_size * sizeof(uchar));
        memcpy(p_old_cell->p_uc_hash_value, p_value, p_map->value_size);
        
        return TRUE;
    }

    p_cell = (map_cell *)malloc(sizeof(map_cell));
    
    p_cell->p_uc_hash_value = (uchar *)malloc(p_map->value_size * sizeof(uchar));
    memcpy(p_cell->p_uc_hash_value, p_value, p_map->value_size);
    
    p_cell->p_uc_hash_key = (uchar *)malloc(p_map->key_size * sizeof(uchar));
    memcpy(p_cell->p_uc_hash_key, p_key, p_map->key_size);
    
    p_cell->p_next_cell = NULL;
    
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
    return TRUE;
}

uchar *map_find(map mymap, uchar *p_key)
{
    map_cell *p_currmap_cell;
    privmap *p_map = (privmap *)mymap;

    if (p_map == NULL)
        return NULL;
    
    p_currmap_cell = p_map->p_cell;

    while(p_currmap_cell)
    {
        /* if comparaison function == NULL, by default use == operator for int32 key*/
        if ((p_map->cellcmp == NULL && (*(int32 *)p_currmap_cell->p_uc_hash_key == *(int32 *)p_key))
            || (p_map->cellcmp != NULL && p_map->cellcmp(p_currmap_cell->p_uc_hash_key, p_key)))
        {
            /* found the corresponding cell ? */
            return p_currmap_cell->p_uc_hash_value;
        }

        p_currmap_cell = p_currmap_cell->p_next_cell;
    }

    return NULL;
}

bool map_delete(map mymap, uchar *p_key)
{
    map_cell *p_currmap_cell;
    map_cell *p_lastmap_cell;
    privmap *p_map = (privmap *)mymap;

    if (p_map == NULL)
        return TRUE;
    
    p_currmap_cell = p_lastmap_cell = p_map->p_cell;

    while(p_currmap_cell)
    {
        if ((p_map->cellcmp == NULL && (*(int32 *)p_currmap_cell->p_uc_hash_key == *(int32 *)p_key))
            || (p_map->cellcmp != NULL && p_map->cellcmp(p_currmap_cell->p_uc_hash_key, p_key)))
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

            free(p_currmap_cell->p_uc_hash_value);
            free(p_currmap_cell->p_uc_hash_key);
            free(p_currmap_cell);
            
            p_map->count--;
            return TRUE;
        }

        p_lastmap_cell = p_currmap_cell;
        p_currmap_cell = p_currmap_cell->p_next_cell;
    }

    return FALSE;
}

bool map_destroy(map mymap)
{
    map_cell *p_currmap_cell;
    map_cell *p_nextmap_cell;
    privmap *p_map = (privmap *)mymap;

    if (p_map == NULL)
        return TRUE;

    p_currmap_cell = p_map->p_cell;
    while(p_currmap_cell)
    {
        p_nextmap_cell = p_currmap_cell;
        
        free(p_currmap_cell->p_uc_hash_value);
        free(p_currmap_cell->p_uc_hash_key);
        free(p_currmap_cell);

        p_currmap_cell = p_nextmap_cell;
    }

    free(p_map);
    return TRUE;
}


uint32 map_count(map mymap)
{
    privmap *p_map = (privmap *)mymap;

    if (p_map == NULL)
        return 0;

    return p_map->count;
}


/******************************************************
 * vector functions
 ******************************************************/

vector vector_create(uint32 value_size)
{
    return map_create(sizeof(int), value_size, NULL);
}

bool vector_set(vector myvector, uint32 pos, uchar *p_value)
{
    if (pos >= vector_count(myvector))
        return FALSE;

    return map_add(myvector, (uchar *)&pos, p_value);
}

bool vector_pushback(vector myvector, uchar *p_value)
{
    uint32 ikey = vector_count(myvector);
    return map_add(myvector, (uchar *)&ikey, p_value);
}

bool vector_insert(vector myvector, uchar *p_value)
{
    privmap *p_map           = (privmap *)myvector;
    map_cell *p_currmap_cell = NULL;
    uint32 ikey                = MAP_BEGIN;

    if (p_map == NULL)
        return FALSE;

    p_currmap_cell = p_map->p_cell;
    while(p_currmap_cell)
    {
        /* since it s a vector the key is an int32 */
        /* just add 1 to each key to shift everything up */
        *((int32 *)p_currmap_cell->p_uc_hash_key) = *((int32 *)p_currmap_cell->p_uc_hash_key) + 1;
        
        p_currmap_cell = p_currmap_cell->p_next_cell;
    }

    /* add key = 0 */
    /* SN: this can be optimized by inserting this cell at the list beginning and */
    /* not at the end */
    return map_add(myvector, (uchar *)&ikey, p_value);
}

uchar *vector_get(vector myvector, uint32 pos)
{
    return map_find(myvector, (uchar *)&pos);
}

bool vector_delete(vector myvector, uint32 pos)
{
    return map_delete(myvector, (uchar *)&pos);
}

bool vector_destroy(vector myvector)
{
    return map_destroy(myvector);
}

uint32 vector_count(vector myvector)
{
    return map_count(myvector);
}
 

/******************************************************
 * stack functions
 ******************************************************/

stack stack_create(uint32 value_size)
{
    return vector_create(value_size);
}

bool stack_push(stack mystack, uchar *p_value)
{
    return vector_pushback(mystack, p_value);
}

uchar *stack_pop(stack mystack)
{
    uchar *p_value = stack_peek(mystack);

    if (p_value != NULL)
    {
        vector_delete(mystack, stack_count(mystack) - 1);
    }

    return p_value;
}

uchar *stack_peek(stack mystack)
{
    uint32 pos = stack_count(mystack) - 1;
    uchar *p_value = vector_get(mystack, pos);
    uint32  size = map_get_value_size(mystack);
    uchar *tmpvalue = NULL;
    
    if (p_value != NULL)
    {
        tmpvalue = (uchar *)sizeof(size * sizeof(uchar));
        memcpy(tmpvalue, p_value, size);
    }

    return tmpvalue;
}

bool stack_destroy(stack mystack)
{
    return vector_destroy(mystack);
}

uint32 stack_count(stack mystack)
{
    return vector_count(mystack);
}


/******************************************************
 * tree functions
 ******************************************************/
/* TO BE DONE, use find with OID */
tree_cell tree_find(tree mytree, tree_cell mycell, void *key, void *type, short method_type)
{
    privtree *p_tree = (privtree *) mytree;
    priv_tree_cell *p_cell_start = (priv_tree_cell *) mycell;
    priv_tree_cell *p_current_cell = NULL;
    bool bDone = FALSE;

    if (p_tree == NULL)
        return (tree_cell) NULL;

    /* if start cell is not null start find from this position */
    if (p_cell_start != NULL)
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
                bDone = FALSE;
            }
            else if (p_current_cell->p_right_sibling)
            {
                p_current_cell = p_current_cell->p_right_sibling;
                bDone = FALSE;
            }
            else
            {
                /* no child, no sibling, go up one level */
                p_current_cell = p_current_cell->p_parent;

                bDone = TRUE;
            }
        }
        else if (method_type == WIDTH_SEARCH)
        {
            /* to be DONE */

            /* if (!bDone && p_current_cell->p_right_sibling) */
/*             { */
/*                 p_current_cell = p_current_cell->p_right_sibling; */
/*                 bDone = FALSE; */
/*             } */
/*             else if (p_current_cell->p_first_child) */
/*             { */
/*                 p_current_cell = p_current_cell->p_first_child; */
/*                 bDone = FALSE; */
/*             } */
/*             else */
/*             { */
/*                 /\* no sibling, no child, go back to left *\/ */
/*                 if (p_current_cell->p_left_sibling) */
/*                     p_current_cell = p_current_cell->p_left_sibling; */
/*                 else */
/*                     p_current_cell = p_current_cell->p_parent; */

/*                 bDone = TRUE; */
/*             } */
        }
    }

    return (tree_cell) NULL;
}

tree tree_create(uint32 tree_cell_size, pfn_tree_cmp_fct pfn_tree_cmp)
{
    privtree *p_tree = (privtree *)malloc(sizeof(privtree));

    if (p_tree)
    {
        p_tree->cell_cmp_fct = pfn_tree_cmp;
        p_tree->cell_size    = tree_cell_size;
        p_tree->p_tree_root  = NULL;
    }

    return (tree) p_tree;
}

uchar *tree_cell_get_data(tree_cell mycell)
{
    priv_tree_cell *p_cell = (priv_tree_cell *) mycell;

    if (p_cell)
        return p_cell->p_data;

    return NULL;
}

tree_cell tree_get_parent(tree_cell mycell)
{
    priv_tree_cell *p_cell = (priv_tree_cell *) mycell;

    if (p_cell)
        return (tree_cell *)p_cell->p_parent;

    return NULL;
}

tree_cell tree_get_child(tree_cell mycell)
{
    priv_tree_cell *p_cell = (priv_tree_cell *) mycell;

    if (p_cell)
        return (tree_cell *)p_cell->p_first_child;

    return NULL;
}

tree_cell tree_get_sibling(tree_cell mycell)
{
    priv_tree_cell *p_cell = (priv_tree_cell *) mycell;

    if (p_cell)
        return (tree_cell *)p_cell->p_right_sibling;

    return NULL;
}

bool tree_move_cell(tree_cell mycell_pos, tree_cell mycell_data, short position_type)
{
    priv_tree_cell *p_cell_pos = (priv_tree_cell *) mycell_pos;
    priv_tree_cell *p_cell_data = (priv_tree_cell *) mycell_data;
    priv_tree_cell *p_temp_cell = NULL;

    if (p_cell_pos == NULL ||
        p_cell_data == NULL)
        return FALSE;

    /* change all reference to mycell_data */
    /* check if cell data is a first child */
    if (p_cell_data->p_parent->p_first_child == p_cell_data)
    {
        if (p_cell_data->p_right_sibling != NULL)
        {
            p_cell_data->p_right_sibling->p_left_sibling = NULL;
        }

        p_cell_data->p_parent->p_first_child = p_cell_data->p_right_sibling;
    }
    else
    {
        /* no, so it s just a sibling */
        p_cell_data->p_left_sibling->p_right_sibling = p_cell_data->p_right_sibling;

        if (p_cell_data->p_right_sibling != NULL)
        {
            p_cell_data->p_right_sibling->p_left_sibling = p_cell_data->p_left_sibling;
        }
    }

    /* check what is the type */
    if (position_type == TREE_CHILD)
    {
        /* test if first child exists */
        if (p_cell_pos->p_first_child != NULL)
        {
            p_temp_cell = p_cell_pos->p_first_child;
        }
        else
        {
            p_cell_pos->p_first_child = p_cell_data;
            p_cell_data->p_parent = p_cell_pos;
            p_cell_data->p_right_sibling = NULL;
            p_cell_data->p_left_sibling = NULL;
        }
    }
    else if (position_type == TREE_SIBLING)
    {
        p_temp_cell = p_cell_pos;
    }

    /* common case */
    if (p_temp_cell != NULL)
    {
        p_cell_data->p_right_sibling = p_temp_cell->p_right_sibling;
        
        if (p_cell_data->p_right_sibling != NULL)
        {
            p_cell_data->p_right_sibling->p_left_sibling = p_cell_data;
        }

        p_temp_cell->p_right_sibling = p_cell_data;
        p_cell_data->p_parent = p_temp_cell->p_parent;
        p_cell_data->p_left_sibling = p_temp_cell;
    }

    return TRUE;
}

bool tree_is_leaf(tree_cell mycell)
{
    priv_tree_cell *p_cell = (priv_tree_cell *) mycell;

    if (p_cell)
        return (p_cell->p_first_child == NULL);

    return FALSE;
}

bool tree_add_sibling(tree mytree, tree_cell mycell_pos, tree_cell_data mydata)
{
    privtree *p_tree = (privtree *) mytree;
    priv_tree_cell *p_cell_pos = (priv_tree_cell *) mycell_pos;
    priv_tree_cell *p_cell_new = (priv_tree_cell *)malloc(sizeof(priv_tree_cell));
    
    if (p_tree == NULL ||
        p_cell_pos == NULL ||
        p_cell_new == NULL)
        return FALSE;

    /* assign data */
    p_cell_new->p_data = (uchar *)malloc(p_tree->cell_size * sizeof(uchar));
    memcpy(p_cell_new->p_data, (uchar *) mydata, p_tree->cell_size);
    p_cell_new->p_first_child = NULL;
    p_cell_new->p_parent = p_cell_pos->p_parent;

    /* insert new cell */
    p_cell_new->p_left_sibling = p_cell_pos;
    p_cell_new->p_right_sibling = p_cell_pos->p_right_sibling;

    p_cell_pos->p_right_sibling = p_cell_new;
    
    return TRUE;
}

bool tree_add_child(tree mytree, tree_cell mycell_pos, tree_cell_data mydata)
{
    privtree *p_tree = (privtree *) mytree;
    priv_tree_cell *p_cell_pos = (priv_tree_cell *) mycell_pos;
    priv_tree_cell *p_cell_new = (priv_tree_cell *)malloc(sizeof(priv_tree_cell));
    
    if (p_tree == NULL ||
        p_cell_pos == NULL ||
        p_cell_new == NULL)
        return FALSE;

    /* assign data */
    p_cell_new->p_data = (uchar *)malloc(p_tree->cell_size * sizeof(uchar));
    memcpy(p_cell_new->p_data, (uchar *) mydata, p_tree->cell_size);

    p_cell_new->p_parent = p_cell_pos;
    p_cell_new->p_first_child = NULL;
    p_cell_new->p_left_sibling = NULL;

    /* if current cell has no child just add it as a child */
    if (p_cell_pos->p_first_child == NULL)
    {
        p_cell_new->p_right_sibling = NULL;

        p_cell_pos->p_first_child = p_cell_new;
    }
    else /* add it as its first child and move sibling */
    {
        p_cell_new->p_right_sibling = p_cell_pos->p_first_child;

        p_cell_pos->p_first_child->p_left_sibling = p_cell_new;

        p_cell_pos->p_first_child = p_cell_new;
    }
    
    return TRUE;
}

bool tree_remove(tree_cell mycell)
{
    priv_tree_cell *p_cell = (priv_tree_cell *) mycell;
    priv_tree_cell *p_parent, *p_right, *p_left;

    if (!p_cell)
        return FALSE;

    p_right  = p_cell->p_right_sibling;
    p_left   = p_cell->p_left_sibling;
    p_parent = p_cell->p_parent;

    if (p_right)
    {
        if (!p_left)
        {
            /* first child */
            p_parent->p_first_child = p_right;
            p_right->p_left_sibling = NULL;
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
        p_parent->p_first_child = NULL;
    }
    
    tree_delete_from(p_cell);
    
    return TRUE;
}

void tree_destroy(tree mytree)
{
    privtree *p_tree = (privtree*) mytree;

    tree_delete_from(p_tree->p_tree_root);

    free(p_tree);
}

uint32 tree_child_number(tree_cell mycell)
{
    priv_tree_cell *p_cell = (priv_tree_cell *) mycell;
    priv_tree_cell *p_current_cell = (p_cell) ? p_cell->p_first_child : NULL;
    uint32 nb_child = 0;
    
    while (p_current_cell)
    {
        nb_child++;

        p_current_cell = p_current_cell->p_right_sibling;
    }

    return nb_child;
}

uint32 tree_sibling_number(tree_cell mycell)
{
    priv_tree_cell *p_cell = (priv_tree_cell *) mycell;
    priv_tree_cell *p_current_cell = (p_cell) ? p_cell->p_right_sibling : NULL;
    uint32 nb_sibling = 0;
    
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

static uint32 map_get_value_size(privmap *p_map)
{
    return p_map->value_size;
}

static bool map_insert(privmap *p_map, uint32 pos, uchar *p_key, uchar *p_value)
{
    map_cell *p_currmap_cell = NULL;
    map_cell *p_cell = NULL;

    if (p_map == NULL)
        return FALSE;

    p_cell = (map_cell *)malloc(sizeof(map_cell));
        
    p_cell->p_uc_hash_value = (uchar *)malloc(p_map->value_size * sizeof(uchar));
    memcpy(p_cell->p_uc_hash_value, p_value, p_map->value_size);
    
    p_cell->p_uc_hash_key = (uchar *)malloc(p_map->key_size * sizeof(uchar));
    memcpy(p_cell->p_uc_hash_key, p_key, p_map->key_size);

    if (pos == MAP_BEGIN)
    {
        p_cell->p_next_cell = p_map->p_cell;
        p_map->p_cell = p_cell;
    }
    else
    {
        p_currmap_cell = map_goto(p_map, pos - 1);

        p_cell->p_next_cell = p_currmap_cell->p_next_cell;
        p_currmap_cell->p_next_cell = p_cell;
    }
    
    p_map->count++;
    return TRUE;
}

static map_cell *map_goto(privmap *p_map, int32 pos)
{
    int32 i = 0;
    map_cell *p_currmap_cell;

    if (p_map == NULL)
        return NULL;

    p_currmap_cell = p_map->p_cell;

    while(p_currmap_cell)
    {
        if (pos == MAP_END)
        {
            if (p_currmap_cell->p_next_cell == NULL)
                return p_currmap_cell;
        }

        if (i == pos)
            return p_currmap_cell;
    
        p_currmap_cell = p_currmap_cell->p_next_cell;

        ++pos;
    }

    return NULL;
}


static map_cell *map_find_cell(privmap *p_map, uchar *p_key)
{
    map_cell *p_currmap_cell;

    if (p_map == NULL)
        return NULL;
    
    p_currmap_cell = p_map->p_cell;

    while(p_currmap_cell)
    {
        /* if comparaison function == NULL, by default use == operator for int32 key*/
        if ((p_map->cellcmp == NULL && (*(int32 *)p_currmap_cell->p_uc_hash_key == *(int32 *)p_key))
            || (p_map->cellcmp != NULL && p_map->cellcmp(p_currmap_cell->p_uc_hash_key, p_key)))
        {
            /* found the corresponding cell ? */
            return p_currmap_cell;
        }

        p_currmap_cell = p_currmap_cell->p_next_cell;
    }

    return NULL;
}


static void tree_delete_from(priv_tree_cell *p_cell)
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
                p_current_cell->p_first_child = NULL;
            }

            free(p_old_cell->p_data);
            free(p_old_cell);
        }
    }

    /* delete the last cell */
    free(p_cell->p_data);
    free(p_cell);
}
