/**
 * \file plugin_timer.h
 * This header is used to define ID for timer plugin registration.
 */

/*
 begin                : 22/11/2003
 author               : (C) Gdp
 email                : iarwain@ifrance.com
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef _PLUGIN_TIMER_H_
#define _PLUGIN_TIMER_H_

/*********************************************
 Constants
 *********************************************/

#define PLUGIN_TIMER_KU32_PLUGIN_ID              0x10000200

#define PLUGIN_TIMER_KU32_FUNCTION_NUMBER        0x0000000D

#define PLUGIN_TIMER_KU32_ID_INIT                0x10000200
#define PLUGIN_TIMER_KU32_ID_EXIT                0x10000201
#define PLUGIN_TIMER_KU32_ID_CLOCK_UPDATE_SET    0x10000202
#define PLUGIN_TIMER_KU32_ID_CLOCK_REGISTER      0x10000203
#define PLUGIN_TIMER_KU32_ID_CLOCK_UNREGISTER    0x10000204
#define PLUGIN_TIMER_KU32_ID_GAME_TIME_GET       0x10000205
#define PLUGIN_TIMER_KU32_ID_GAME_DT_GET         0x10000206
#define PLUGIN_TIMER_KU32_ID_REAL_TIME_GET       0x10000207
#define PLUGIN_TIMER_KU32_ID_REAL_DT_GET         0x10000208
#define PLUGIN_TIMER_KU32_ID_TIME_COEF_SET       0x10000209
#define PLUGIN_TIMER_KU32_ID_TIME_COEF_GET       0x1000020A
#define PLUGIN_TIMER_KU32_ID_UPDATE              0x1000020B
#define PLUGIN_TIMER_KU32_ID_DATE_WRITE          0x1000020C

#endif /* _PLUGIN_TIMER_H_ */
