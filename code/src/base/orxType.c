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
 */


#include "orxInclude.h"


/* *** Misc constants *** */
const orxSTRING    orxSTRING_EMPTY     = "";
const orxSTRING    orxSTRING_TRUE      = "true";
const orxSTRING    orxSTRING_FALSE     = "false";
const orxCHAR      orxCHAR_NULL        = '\0';
const orxCHAR      orxCHAR_CR          = '\r';
const orxCHAR      orxCHAR_LF          = '\n';

#ifdef __orxWINDOWS__

  const orxCHAR    orxCHAR_EOL         = '\n';
  const orxSTRING  orxSTRING_EOL       = "\r\n";

#elif defined(__orxLINUX__) || defined(__orxGP2X__) || defined(__orxWII__) || defined(__orxIPHONE__)

  const orxCHAR    orxCHAR_EOL         = '\n';
  const orxSTRING  orxSTRING_EOL       = "\n";

#elif defined(__orxMAC__)

  const orxCHAR    orxCHAR_EOL         = '\r';
  const orxSTRING  orxSTRING_EOL       = "\r";

#endif


/* *** Directory separators *** */

const orxCHAR     orxCHAR_DIRECTORY_SEPARATOR_WINDOWS    = '\\';
const orxCHAR     orxCHAR_DIRECTORY_SEPARATOR_LINUX      = '/';
const orxSTRING   orxSTRING_DIRECTORY_SEPARATOR_WINDOWS  = "\\";
const orxSTRING   orxSTRING_DIRECTORY_SEPARATOR_LINUX    = "/";

#ifdef __orxWINDOWS__

  const orxCHAR      orxCHAR_DIRECTORY_SEPARATOR         = '\\';
  const orxSTRING    orxSTRING_DIRECTORY_SEPARATOR       = "\\";

#else /* __orxWINDOWS__ */

  /* Linux / Mac / GP2X / Wii */
  #if defined(__orxLINUX__) || defined(__orxMAC__) || defined(__orxGP2X__) || defined(__orxWII__) || defined(__orxIPHONE__)

    const orxCHAR    orxCHAR_DIRECTORY_SEPARATOR         = '/';
    const orxSTRING  orxSTRING_DIRECTORY_SEPARATOR       = "/";

  #endif /* __orxLINUX__ || __orxMAC__ || __orxGP2X__ || __orxWII__ || __orxIPHONE__ */

#endif /* __orxWINDOWS__ */
