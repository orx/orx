/* Orx - Portable Game Engine
 *
 * Orx is the legal property of its developers, whose names
 * are listed in the COPYRIGHT file distributed 
 * with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * @file orxType.c
 * @date 09/06/2008
 * @author iarwain@orx-project.org
 *
 * @todo
 * - Add the required types at need.
 */

/**
 * @addtogroup Base
 * 
 * Base types
 *
 * @{
 */


#include "orxInclude.h"


/* *** Undefined constants *** */
orxCONST orxU64       orxU64_UNDEFINED    = (orxU64)(-1);
orxCONST orxU32       orxU32_UNDEFINED    = (orxU32)(-1);
orxCONST orxU16       orxU16_UNDEFINED    = (orxU16)(-1);
orxCONST orxU8        orxU8_UNDEFINED     = (orxU8)(-1);
orxCONST orxHANDLE    orxHANDLE_UNDEFINED = (orxHANDLE)(-1);


/* *** Misc constants *** */
orxCONST orxBOOL      orxFALSE            = (orxBOOL)(1 != 1);
orxCONST orxBOOL      orxTRUE             = (orxBOOL)(1 == 1);

orxCONST orxSTRING    orxSTRING_EMPTY     = "";
orxCONST orxSTRING    orxSTRING_TRUE      = "true";
orxCONST orxSTRING    orxSTRING_FALSE     = "false";
orxCONST orxCHAR      orxCHAR_NULL        = '\0';
orxCONST orxCHAR      orxCHAR_CR          = '\r';
orxCONST orxCHAR      orxCHAR_LF          = '\n';

#ifdef __orxWINDOWS__

  orxCONST orxCHAR    orxCHAR_EOL         = '\n';
  orxCONST orxSTRING  orxSTRING_EOL       = "\r\n";

#elif defined(__orxLINUX__) || defined (__orxGP2X__)

  orxCONST orxCHAR    orxCHAR_EOL         = '\n';
  orxCONST orxSTRING  orxSTRING_EOL       = "\n";

#elif defined(__orxMAC__)

  orxCONST orxCHAR    orxCHAR_EOL         = '\r';
  orxCONST orxSTRING  orxSTRING_EOL       = "\r";

#endif


/* *** Directory separators *** */

orxCONST orxCHAR     orxCHAR_DIRECTORY_SEPARATOR_WINDOWS    = '\\';
orxCONST orxCHAR     orxCHAR_DIRECTORY_SEPARATOR_LINUX      = '/';
orxCONST orxSTRING   orxSTRING_DIRECTORY_SEPARATOR_WINDOWS  = "\\";
orxCONST orxSTRING   orxSTRING_DIRECTORY_SEPARATOR_LINUX    = "/";

#ifdef __orxWINDOWS__

  orxCONST orxCHAR      orxCHAR_DIRECTORY_SEPARATOR         = '\\';
  orxCONST orxSTRING    orxSTRING_DIRECTORY_SEPARATOR       = "\\";

#else /* __orxWINDOWS__ */

  /* Linux / Mac / GP2X */
  #if defined(__orxLINUX__) || defined(__orxMAC__) || defined(__orxGP2X__)

    orxCONST orxCHAR    orxCHAR_DIRECTORY_SEPARATOR         = '/';
    orxCONST orxSTRING  orxSTRING_DIRECTORY_SEPARATOR       = "/";

  #endif /* __orxLINUX__ || __orxMAC__ || __orxGP2X__ */

#endif /* __orxWINDOWS__ */


/* *** Float constants *** */

orxCONST orxFLOAT orxFLOAT_0 = orx2F(0.0f);
orxCONST orxFLOAT orxFLOAT_1 = orx2F(1.0f);
