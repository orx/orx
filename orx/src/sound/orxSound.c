/**
 * @file orxSound.c
 */

/***************************************************************************
 begin                : 13/07/2008
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


#include "orxInclude.h"

#include "sound/orxSound.h"
#include "memory/orxMemory.h"


/** Module flags
 */
#define orxSOUND_KU32_STATIC_FLAG_NONE    0x00000000  /**< No flags */

#define orxSOUND_KU32_STATIC_FLAG_READY   0x00000001  /**< Ready flag */

#define orxSOUND_KU32_STATIC_MASK_ALL     0xFFFFFFFF  /**< All mask */


/** Defines
 */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxSOUND_STATIC_t
{
  orxU32  u32Flags;                       /**< Control flags */

} orxSOUND_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** static data
 */
orxSTATIC orxSOUND_STATIC sstSound;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Sound module setup
 */
orxVOID orxSound_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_SOUND, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_SOUND, orxMODULE_ID_SOUNDSYSTEM);
  orxModule_AddDependency(orxMODULE_ID_SOUND, orxMODULE_ID_CONFIG);

  return;
}

/** Inits the sound module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxSound_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!orxFLAG_TEST(sstSound.u32Flags, orxSOUND_KU32_STATIC_FLAG_READY))
  {
    /* Cleans control structure */
    orxMemory_Zero(&sstSound, sizeof(orxSOUND_STATIC));

    /* Inits Flags */
    orxFLAG_SET(sstSound.u32Flags, orxSOUND_KU32_STATIC_FLAG_READY, orxSOUND_KU32_STATIC_MASK_ALL);

    /* Success */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* !!! MSG !!! */

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Exits from the sound module
 */
orxVOID orxSound_Exit()
{
  /* Initialized? */
  if(orxFLAG_TEST(sstSound.u32Flags, orxSOUND_KU32_STATIC_FLAG_READY))
  {
    /* Updates flags */
    orxFLAG_SET(sstSound.u32Flags, orxSOUND_KU32_STATIC_FLAG_NONE, orxSOUND_KU32_STATIC_MASK_ALL);
  }

  return;
}
