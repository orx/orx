/* Orx - Portable Game Engine
 *
 * Copyright (c) 2010 Orx-Project
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 */

/**
 * @file orxVector.c
 * @date 27/04/2005
 * @author iarwain@orx-project.org
 *
 */


#include "math/orxVector.h"


/* *** Vector constants *** */

const orxVECTOR orxVECTOR_X      = {{orx2F(1.0f)}, {orx2F(0.0f)}, {orx2F(0.0f)}};
const orxVECTOR orxVECTOR_Y      = {{orx2F(0.0f)}, {orx2F(1.0f)}, {orx2F(0.0f)}};
const orxVECTOR orxVECTOR_Z      = {{orx2F(0.0f)}, {orx2F(0.0f)}, {orx2F(1.0f)}};

const orxVECTOR orxVECTOR_0      = {{orx2F(0.0f)}, {orx2F(0.0f)}, {orx2F(0.0f)}};
const orxVECTOR orxVECTOR_1      = {{orx2F(1.0f)}, {orx2F(1.0f)}, {orx2F(1.0f)}};

const orxVECTOR orxVECTOR_RED    = {{orx2F(1.0f)}, {orx2F(0.0f)}, {orx2F(0.0f)}};
const orxVECTOR orxVECTOR_GREEN  = {{orx2F(0.0f)}, {orx2F(1.0f)}, {orx2F(0.0f)}};
const orxVECTOR orxVECTOR_BLUE   = {{orx2F(0.0f)}, {orx2F(0.0f)}, {orx2F(1.0f)}};

const orxVECTOR orxVECTOR_BLACK  = {{orx2F(0.0f)}, {orx2F(0.0f)}, {orx2F(0.0f)}};
const orxVECTOR orxVECTOR_WHITE  = {{orx2F(1.0f)}, {orx2F(1.0f)}, {orx2F(1.0f)}};
