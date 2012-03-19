/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2012 Orx-Project
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

#ifdef __orxWINDOWS__

  const orxCHAR    orxCHAR_EOL         = '\n';
  const orxSTRING  orxSTRING_EOL       = "\r\n";

#elif defined(__orxLINUX__) || defined(__orxIOS__) || defined(__orxANDROID__) || defined(__orxANDROID_NATIVE__)

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

  /* Linux / Mac / iOS / Android */
  #if defined(__orxLINUX__) || defined(__orxMAC__) || defined(__orxIOS__) || defined(__orxANDROID__) || defined(__orxANDROID_NATIVE__)

    const orxCHAR    orxCHAR_DIRECTORY_SEPARATOR         = '/';
    const orxSTRING  orxSTRING_DIRECTORY_SEPARATOR       = "/";

  #endif /* __orxLINUX__ || __orxMAC__ || __orxIOS__ || __orxANDROID__ || __orxANDROID_NATIVE__ */

#endif /* __orxWINDOWS__ */
