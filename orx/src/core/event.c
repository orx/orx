///***************************************************************************
// event.c
// messaging system
//
// begin                : 02/09/2002
// author               : (C) Gdp
// email                : snegri@free.fr
// ***************************************************************************/
//
///***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************/
//
//#include "core/event.h"
//
//
///*
// * STATIC declarations
// */
//
//typedef struct st_callback_cell
//{
//    pfn_event_callback pfn_callback;
//    struct st_callback_cell *p_next_cell;
//} st_callback_cell;
//
//static st_callback_cell *s_event_table[EVENT_ALL];
//
///*
// * forward declarations
// */
//
//static uint32 add_event(e_event_type type, st_callback_cell *p_new_cell);
//static uint32 remove_function(pfn_event_callback fn);
//static uint32 remove_function_with_type(e_event_type type, pfn_event_callback fn);
//
//
//
///*
// * PUBLIC functions
// */
//
//void event_init()
//{
//    int i;
//
//    for(i = EVENT_NONE; i < EVENT_ALL; ++i)
//    {
//        s_event_table[i] = NULL;;
//    }
//}
//
//GP_RC event_register(e_event_type type, pfn_event_callback pfn)
//{
//    st_callback_cell *p_cell;
//
//    /* sanity check */
//    if (type <= EVENT_NONE || type >= EVENT_ALL)
//        return ERROR;
//
//    /* create new cell */
//    p_cell = (st_callback_cell *)malloc(sizeof(st_callback_cell));
//    
//    p_cell->pfn_callback = pfn;
//    p_cell->p_next_cell  = NULL;
//
//    /* is it first entry ?*/
//    if (s_event_table[type] == NULL)
//    {
//        /* assign first entry */
//        s_event_table[type] = p_cell;
//    }
//    else
//    {
//        /* no, add cell */
//        add_event(type, p_cell);
//    }
//
//    return NO_ERROR;
//}
//
//
//GP_RC event_unregister_function(e_event_type type, pfn_event_callback pfn)
//{
//    st_callback_cell *p_current_cell, *p_last_cell;
//
//    /* sanity check */
//    if (type <= EVENT_NONE || type >= EVENT_ALL)
//        return ERROR;
//
//    p_current_cell = s_event_table[type];
//    p_last_cell    = NULL;
//
//    while(p_current_cell)
//    {
//        /* is it the right callback ? */
//        if (p_current_cell->pfn_callback == pfn)
//        {
//            /* yes, delete it */
//            if (p_last_cell != NULL)
//            {
//                /* not first entry */
//                p_last_cell->p_next_cell = p_current_cell->p_next_cell;
//            }
//            else
//            {
//                /* first entry */
//                s_event_table[type] = NULL;
//            }
//
//            /* remove cell */
//            free(p_current_cell);
//
//            /* exit loop */
//            break;
//        }
//
//        p_last_cell    = p_current_cell;
//        p_current_cell = p_current_cell->p_next_cell;
//    }
//
//    return NO_ERROR;
//}
//
//GP_RC event_send(e_event_type type)
//{
//    st_callback_cell *p_current_cell;
//
//    /* sanity check */
//    if (type <= EVENT_NONE || type >= EVENT_ALL)
//        return ERROR;
//
//    p_current_cell = s_event_table[type];
//
//    while(p_current_cell)
//    {
//        (p_current_cell->pfn_callback)(type);
//        p_current_cell = p_current_cell->p_next_cell;
//    }
//    
//    return NO_ERROR;
//}
//
//
///*
// * PRIVATE functions
// */
//
//static GP_RC add_event(e_event_type type, st_callback_cell *p_new_cell)
//{
//    st_callback_cell *p_current_cell;
// 
//    p_current_cell = s_event_table[type];
//  
//    /* find the next available slot */
//    while (p_current_cell->p_next_cell)
//    {
//        p_current_cell = p_current_cell->p_next_cell;
//    }
//
//    /* add new cell*/
//    p_current_cell->p_next_cell = p_new_cell;
//    
//    return NO_ERROR;
//}
//
//static GP_RC remove_function_with_type(e_event_type type, pfn_event_callback fn)
//{
//    return NO_ERROR;
//}
//
//static GP_RC remove_function(pfn_event_callback fn)
//{
//    return NO_ERROR;
//}
//
