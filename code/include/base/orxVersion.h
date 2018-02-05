/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2018 Orx-Project
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
 * @file orxVersion.h
 * @date 09/11/2016
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxVersion
 *
 * Version
 *
 * @{
 */


#ifndef _orxVERSION_H_
#define _orxVERSION_H_


#include "base/orxType.h"

#if !defined(__orxANDROID__) && !defined(__orxANDROID_NATIVE__) && !defined(__orxIOS__)

#ifndef __orxVERSION_BUILD__
#include "base/orxBuild.h"
#endif /* __orxVERSION_BUILD__ */

#endif /* !__orxANDROID__ && !__orxANDROID_NATIVE__ && !__orxIOS__ */

/** Version numbers
 */

#define __orxVERSION_MAJOR__        1
#define __orxVERSION_MINOR__        10

#ifndef __orxVERSION_RELEASE__
  #define __orxVERSION_RELEASE__    dev
#endif /* __orxVERSION_RELEASE__ */

#ifndef __orxVERSION_BUILD__
  #define __orxVERSION_BUILD__      0UL
#endif /* __orxVERSION_BUILD__ */

#define __orxVERSION_STRING__       orxSTRINGIFY(__orxVERSION_MAJOR__) "." orxSTRINGIFY(__orxVERSION_MINOR__) "-" orxSTRINGIFY(__orxVERSION_RELEASE__)
#define __orxVERSION_FULL_STRING__  orxSTRINGIFY(__orxVERSION_MAJOR__) "." orxSTRINGIFY(__orxVERSION_MINOR__) "." orxSTRINGIFY(__orxVERSION_BUILD__) "-" orxSTRINGIFY(__orxVERSION_RELEASE__)

#define __orxVERSION_MASK_MAJOR__   0xFF000000
#define __orxVERSION_SHIFT_MAJOR__  24
#define __orxVERSION_MASK_MINOR__   0x00FF0000
#define __orxVERSION_SHIFT_MINOR__  16
#define __orxVERSION_MASK_BUILD__   0x0000FFFF
#define __orxVERSION_SHIFT_BUILD__  0

#define __orxVERSION__            (((__orxVERSION_MAJOR__ << __orxVERSION_SHIFT_MAJOR__) & __orxVERSION_MASK_MAJOR__) | ((__orxVERSION_MINOR__ << __orxVERSION_SHIFT_MINOR__) & __orxVERSION_MASK_MINOR__) | ((__orxVERSION_BUILD__ << __orxVERSION_SHIFT_BUILD__) & __orxVERSION_MASK_BUILD__))


/** Version structure
 */

typedef struct __orxVERSION_t
{
  const orxSTRING zRelease;
  orxU32          u32Major;
  orxU32          u32Minor;
  orxU32          u32Build;

} orxVERSION;


#endif /*_orxVERSION_H_*/

/** @} */
