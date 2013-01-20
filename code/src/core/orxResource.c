/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2013 Orx-Project
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
 * @file orxResource.c
 * @date 20/01/2013
 * @author iarwain@orx-project.org
 *
 */


#include "core/orxResource.h"

#include "debug/orxDebug.h"
#include "debug/orxProfiler.h"
#include "core/orxConfig.h"
#include "memory/orxMemory.h"
#include "utils/orxString.h"


/** Module flags
 */
#define orxRESOURCE_KU32_STATIC_FLAG_NONE             0x00000000                      /**< No flags */

#define orxRESOURCE_KU32_STATIC_FLAG_READY            0x00000001                      /**< Ready flag */

#define orxRESOURCE_KU32_STATIC_MASK_ALL              0xFFFFFFFF                      /**< All mask */


/** Misc
 */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxRESOURCE_STATIC_t
{
  orxU32                    u32Flags;                                                 /**< Control flags */

} orxRESOURCE_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxRESOURCE_STATIC sstResource;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Resource module setup
 */
void orxFASTCALL orxResource_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_RESOURCE, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_RESOURCE, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_RESOURCE, orxMODULE_ID_PROFILER);

  /* Done! */
  return;
}

/** Inits resource module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxResource_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstResource.u32Flags & orxRESOURCE_KU32_STATIC_FLAG_READY))
  {
    /* Cleans control structure */
    orxMemory_Zero(&sstResource, sizeof(orxRESOURCE_STATIC));

    /* Inits Flags */
    sstResource.u32Flags = orxRESOURCE_KU32_STATIC_FLAG_READY;
  }

  /* Done! */
  return eResult;
}

/** Exits from resource module
 */
void orxFASTCALL orxResource_Exit()
{
  /* Initialized? */
  if(sstResource.u32Flags & orxRESOURCE_KU32_STATIC_FLAG_READY)
  {
    /* Updates flags */
    sstResource.u32Flags &= ~orxRESOURCE_KU32_STATIC_FLAG_READY;
  }

  /* Done! */
  return;
}
