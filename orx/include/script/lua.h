/***************************************************************************
 lua.h
 LUA Bindings for GameProject

 begin                : 9/07/2003
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

#ifndef _LUA_H_
#define _LUA_H_

#include <lualib.h>

#include "include.h"


typedef int32(*pfn_lua_fct)(lua_State *ls);

bool gp_lua_start();
bool gp_lua_end();
bool gp_lua_read_file(const char *filename);

/* if varname == NULL, just get first value on the stack */
/* should be changed to 2 differents set of func */
/* somthing like: getglobal which put on the stack the var and call these func after */
double gp_lua_get_number(const char *varname);
bool   gp_lua_get_bool(const char *varname);
const char* gp_lua_get_string(const char *varname);

bool gp_lua_error();

/* To call a LUA function from C:
test.lua:
function f(x, y)
  return x + y
end

gp_lua_call("f", 1, "%d%d", 1, 2);
int ret = gp_lua_get_number(NULL);
*/

bool gp_lua_call(const char *funcname, int nb_return, const char *arg_proto, ...);

/* To register a C function which can be call from LUA scripts:
int my_func(int a, int b)
{ 
  return a+b;
}

gp_lua_register_function("my_func", my_func, "%d%d", "%d");
*/

bool gp_lua_register_function(const char *funcname, pfn_lua_fct pfct, const char *arg_type, const char *ret_type);

/* put on the stack the given return value according to the format given at registration */
bool gp_lua_set_func_return(const char *funcname, ...);

/* a faire 
- on recupere une table (creer un typedef qui va bien), peut etre faire un lock et un release sur la table
pour pouvoir manipuler la pile
- on peut iterer sur la table pour tout recuperer
- on peut demander un index particulier de la table
*/

#endif /* _LUA_H_ */
