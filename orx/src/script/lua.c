///***************************************************************************
// lua.c
//
// begin                : 08/04/2004
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
// ****************************************************************************/
//
//#include "script/lua.h"
//
//#include <lauxlib.h>
//
//
//static lua_State *s_LS = NULL;
//static bool s_ERROR;
//
//bool gp_lua_start()
//{
//    s_LS = lua_open();
//
//    lua_baselibopen(s_LS);
//    lua_iolibopen(s_LS);
//    lua_strlibopen(s_LS);
//    lua_mathlibopen(s_LS);
//
//    /* clear the stack, just sanity op */
//    lua_settop(s_LS, 0);
//
//    /* initialize default error */
//    s_ERROR = FALSE;
//    
//    return TRUE;
//}
//
//
//bool gp_lua_read_file(const char *filename)
//{
//    /* if LUA not initialized do it first */
//    if (s_LS == NULL)
//        gp_lua_start();
//
//    if (lua_dofile(s_LS, filename) != 0)
//    {
//        return FALSE;
//    }
//
//    return TRUE;
//}
//
//bool gp_lua_end()
//{
//    lua_close(s_LS);
//
//    s_LS = NULL;
//    
//    return TRUE;
//}
//
//
//bool gp_lua_error()
//{
//    bool tmp = s_ERROR;
//
//    s_ERROR = FALSE;
//
//    return tmp;
//}
//
//
//double gp_lua_get_number(const char *varname)
//{
//    if (varname != NULL)
//        lua_getglobal(s_LS, varname);
//
//    /* check it s a number */
//    if (!lua_isnumber(s_LS, -1))
//    {
//        /* set error */
//        s_ERROR = TRUE;
//        return 0;
//    }
//
//    return lua_tonumber(s_LS, -1);
//}
//
//bool   gp_lua_get_bool(const char *varname)
//{
//    if (varname != NULL)
//        lua_getglobal(s_LS, varname);
//
//    /* check it s a boolean */
//    if (!lua_isboolean(s_LS, -1))
//    {
//        /* set error */
//        s_ERROR = TRUE;
//        return FALSE;
//    }
//
//    return lua_toboolean(s_LS, -1);
//}
//
//const char* gp_lua_get_string(const char *varname)
//{
//    if (varname != NULL)
//        lua_getglobal(s_LS, varname);
//
//    /* check it s a string */
//    if (!lua_isstring(s_LS, -1))
//    {
//        /* set error */
//        s_ERROR = TRUE;
//        return 0;
//    }
//
//    return lua_tostring(s_LS, -1);
//}
//
//
//bool gp_lua_call(const char *funcname, int nb_return, const char *arg_proto, ...)
//{
//    va_list pa;
//    int i;
//    char *p;
//    int nbarg = 0;
//
//    lua_getglobal(s_LS, funcname);
//
//    if (!lua_isfunction(s_LS, -1))
//        return FALSE;
//
//    va_start(pa, arg_proto);
//
//    for(p = (char *)arg_proto; *p != '\0'; p++)
//    {
//        if (*p != '%')
//        {
//            /* error */
//            va_end(pa);
//            return FALSE;
//        }
//
//        switch(*++p)
//        {
//            case 'd': /* int */
//                lua_pushnumber(s_LS, va_arg(pa, int));
//                nbarg++;
//                break;
//            case 'f': /* double */
//                lua_pushnumber(s_LS, va_arg(pa, double));
//                nbarg++;
//                break;
//            case 's': /* string */
//                lua_pushstring(s_LS, va_arg(pa, char *));
//                nbarg++;
//                break;
//            default:
//                /* error */
//                /* remove from LUA stack already pushed values */
//                lua_pop(s_LS, nbarg);
//
//                va_end(pa);
//                return FALSE;
//        }
//    }
//    
//    va_end(pa);
//
//    lua_call(s_LS, nbarg, nb_return);
//
//    
//    /* TO BE DONE: check number of return, pass type of return and check types */
//    /* return the returned value here instead of querying them with other functions */
//    return TRUE;
//}
//
//
//bool gp_lua_register_function(const char *funcname, pfn_lua_fct pfct, const char *arg_type, const char *ret_type)
//{
//    return TRUE;
//}
