/**
 * \file timer.h
 */

/***************************************************************************
 begin                : 04/02/2004
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

#ifndef _TIMER_TYPE_H_
#define _TIMER_TYPE_H_


/** Clock update callback prototype, used when registering a clock. */
typedef orxVOID  (*timer_fn_clock_update)(orxVOID *);

/** Timer coefficient types */
#define TIMER_KU32_COEF_TYPE_FIXED       0x000000001 /**< Fixed time coefficient. */
#define TIMER_KU32_COEF_TYPE_MULTIPLY    0x000000002 /**< Multiply time coefficient. */

#endif /* _TIMER_TYPE_H_ */
