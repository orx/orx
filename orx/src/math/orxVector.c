/***************************************************************************
 orxVector.c
 Vector module

 begin                : 27/04/2005
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/


#include "math/orxVector.h"


/* *** Vector constants *** */

orxCONST orxVECTOR orxVECTOR_X      = {{orx2F(1.0f)}, {orx2F(0.0f)}, {orx2F(0.0f)}};
orxCONST orxVECTOR orxVECTOR_Y      = {{orx2F(0.0f)}, {orx2F(1.0f)}, {orx2F(0.0f)}};
orxCONST orxVECTOR orxVECTOR_Z      = {{orx2F(0.0f)}, {orx2F(0.0f)}, {orx2F(1.0f)}};

orxCONST orxVECTOR orxVECTOR_0      = {{orx2F(0.0f)}, {orx2F(0.0f)}, {orx2F(0.0f)}};
orxCONST orxVECTOR orxVECTOR_WHITE  = {{orx2F(255.0f)}, {orx2F(255.0f)}, {orx2F(255.0f)}};
