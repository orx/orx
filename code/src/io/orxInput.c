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
 * @file orxInput.c
 * @date 04/11/2008
 * @author iarwain@orx-project.org
 *
 * @todo
 */


#include "orxInclude.h"

#include "io/orxInput.h"
#include "debug/orxDebug.h"
#include "memory/orxBank.h"
#include "utils/orxString.h"


/** Module flags
 */
#define orxINPUT_KU32_STATIC_FLAG_NONE            0x00000000  /**< No flags */

#define orxINPUT_KU32_STATIC_FLAG_READY           0x00000001  /**< Ready flag */

#define orxINPUT_KU32_STATIC_MASK_ALL             0xFFFFFFFF  /**< All mask */


/** Defines
 */
#define orxINPUT_KZ_CONFIG_SECTION                "Input"     /**< Input section name */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxINPUT_STATIC_t
{
  orxU32  u32Flags;                               /**< Control flags */

} orxINPUT_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** static data
 */
orxSTATIC orxINPUT_STATIC sstInput;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Input module setup
 */
orxVOID orxInput_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_INPUT, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_INPUT, orxMODULE_ID_CONFIG);

  return;
}

/** Inits the input module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxInput_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY))
  {
    /* Updates flags */
    orxFLAG_SET(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY, orxINPUT_KU32_STATIC_FLAG_NONE);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Tried to initialize input module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Exits from the input module
 */
orxVOID orxInput_Exit()
{
  /* Initialized? */
  if(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY))
  {
    /* Updates flags */
    orxFLAG_SET(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_NONE, orxINPUT_KU32_STATIC_MASK_ALL);
  }

  return;
}
