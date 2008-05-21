/**
 * @file orxEvent.c
 */

/***************************************************************************
 orxEvent.c
 Event module
 begin                : 20/05/2008
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

#include "core/orxEvent.h"
#include "debug/orxDebug.h"
#include "memory/orxBank.h"
#include "utils/orxHashTable.h"


/** Module flags
 */
#define orxEVENT_KU32_STATIC_FLAG_NONE    0x00000000  /**< No flags */

#define orxEVENT_KU32_STATIC_FLAG_READY   0x00000001  /**< Ready flag */

#define orxEVENT_KU32_STATIC_MASK_ALL     0xFFFFFFFF  /**< All mask */


/** Misc defines
 */
#define orxEVENT_KU32_HANDLER_TABLE_SIZE  4


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxEVENT_STATIC_t
{
  orxU32        u32Flags;                             /**< Control flags */
  orxHASHTABLE *pstHandlerTable;                      /**< Handler table */
  
} orxEVENT_STATIC;


/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/

orxSTATIC orxEVENT_STATIC sstEvent;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Event module setup
 */
orxVOID orxEvent_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_EVENT, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_EVENT, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_EVENT, orxMODULE_ID_HASHTABLE);

  return;
}

/** Inits the event module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxEvent_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!orxFLAG_TEST(sstEvent.u32Flags, orxEVENT_KU32_STATIC_FLAG_READY))
  {
    /* Cleans control structure */
    orxMemory_Set(&sstEvent, 0, sizeof(orxEVENT_STATIC));

    /* Creates handler table */
    sstEvent.pstHandlerTable = orxHashTable_Create(orxEVENT_KU32_HANDLER_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_NONE);

    /* Valid? */
    if(sstEvent.pstHandlerTable != orxNULL)
    {
      /* Inits Flags */
      orxFLAG_SET(sstEvent.u32Flags, orxEVENT_KU32_STATIC_FLAG_READY, orxEVENT_KU32_STATIC_MASK_ALL);

      /* Success */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* !!! MSG !!! */

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
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

/** Exits from the event module
 */
orxVOID orxEvent_Exit()
{
  /* Initialized? */
  if(orxFLAG_TEST(sstEvent.u32Flags, orxEVENT_KU32_STATIC_FLAG_READY))
  {
    /* Deletes hashtable */
    orxHashTable_Delete(sstEvent.pstHandlerTable);

    /* Updates flags */
    orxFLAG_SET(sstEvent.u32Flags, orxEVENT_KU32_STATIC_FLAG_NONE, orxEVENT_KU32_STATIC_MASK_ALL);
  }

  return;
}

/** Adds an event handler
 * @param _eEventType           Concerned type of event
 * @param _pfnHandler           Event handler to add
 * return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxEvent_AddHandler(orxEVENT_TYPE _eEventType, orxEVENT_HANDLER _pfnEventHandler)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstEvent.u32Flags, orxEVENT_KU32_STATIC_FLAG_READY));
  orxASSERT(_eEventType < orxEVENT_TYPE_MAX_NUMBER);
  orxASSERT(_pfnEventHandler != orxNULL);

  /* Done! */
  return eResult;
}

/** Sends an event
 * @param _pstEvent             Event to send
 */
orxSTATUS orxFASTCALL orxEvent_Send(orxCONST orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstEvent.u32Flags, orxEVENT_KU32_STATIC_FLAG_READY));
  orxASSERT(_pstEvent != orxNULL);
  orxASSERT(_pstEvent->eType < orxEVENT_TYPE_MAX_NUMBER);

  /* Done! */
  return eResult;
}

/** Removes an event handler
 * @param _eEventType           Concerned type of event
 * @param _pfnHandler           Event handler to remove
 * return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxEvent_RemoveHandler(orxEVENT_TYPE _eEventType, orxEVENT_HANDLER _pfnEventHandler)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstEvent.u32Flags, orxEVENT_KU32_STATIC_FLAG_READY));
  orxASSERT(_eEventType < orxEVENT_TYPE_MAX_NUMBER);
  orxASSERT(_pfnEventHandler != orxNULL);

  /* Done! */
  return eResult;
}
