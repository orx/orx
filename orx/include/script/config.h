/***************************************************************************
 config.h
 Config file handler for GameProject

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

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "include.h"

#include "script/lua.h"


bool gp_config_start(const char *filename);
bool gp_config_end();

double gp_config_get_number(const char *varname);
bool   gp_config_get_bool(const char *varname);
const char* gp_config_get_string(const char *varname);

bool gp_config_error();

#endif /* _CONFIG_H_ */
