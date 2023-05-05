/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2022 Orx-Project
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
 * @file orxShaderPrecision.h
 * @date 06/05/2023
 * @author iarwain@orx-project.org
 *
 * @todo
 */

#ifndef _orxSHADERPRECISION_H_
#define _orxSHADERPRECISION_H_


/** Shader precision
 */
typedef enum __orxSHADER_PRECISION_t
{
  orxSHADER_PRECISION_SYSTEM = 0,
  orxSHADER_PRECISION_LOW,
  orxSHADER_PRECISION_MEDIUM,
  orxSHADER_PRECISION_HIGH,
  
  orxSHADER_PRECISION_NUMBER,

  orxSHADER_PRECISION_NONE = orxENUM_NONE

} orxSHADER_PRECISION;


#endif /* _orxSHADERPRECISION_H_ */

/** @} */
