/**
 * @file orxStateMachine.c
 * 
 * State Machine
 * 
 * @todo 
 */
 
 /***************************************************************************
 orxStateMachine.c
 State Machine.
 
 begin                : 20/06/2005
 author               : (C) Arcallians
 email                : corrosifdev@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "utils/orxStateMachine.h"
#include "utils/orxHashTable.h"
#include "memory/orxBank.h"
#include "debug/orxDebug.h"
#include "io/orxTextIO.h"

#define orxSTATEMACHINE_KU32_FLAG_NONE   0x00000000  /**< No flags have been set */
#define orxSTATEMACHINE_KU32_FLAG_READY  0x00000001  /**< The module has been initialized */


/***************************************************************************
 * Macro declaration                                                       *
 ***************************************************************************/

#ifndef orxU32KeyGen
#define orxU32KeyGen(a, b) (((orxU32)a << 16) + b)
#endif


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** States structure. */
struct __orxSTATEMACHINE_STATE_t
{
  /* State identifier. */
  orxU16 u16Id;
  
  /* State type. */
  orxSTATEMACHINE_STATE_TYPE eStateType;
  
  /* Action to realize when accessing this state. */
  orxSTATEMACHINE_ACTION_PTR cbAction;
};

/** Link structure. */
struct __orxSTATEMACHINE_LINK_t
{
  /* Condition to follow the link. */
  orxSTATEMACHINE_CONDITION_PTR cbCondition;
  
  /* The state marking the beginning of the link. */
  orxSTATEMACHINE_STATE * pstBeginningState;
  
  /* The state marking the end of the link. */
  orxSTATEMACHINE_STATE * pstEndingState;
  
  /* Action to realize when following this link. */
  orxSTATEMACHINE_ACTION_PTR cbAction;
};

/** State machines structure. */
struct __orxSTATEMACHINE_t
{
  /* Initial state. */
  orxSTATEMACHINE_STATE * stInitialState;
  
  /* Bank where states are stored. */
  orxBANK * pstStatesBank;

  /* Hash table where states are stored. */
  orxHASHTABLE * pstStatesHashTable;

  /* Bank where links are stored. */
  orxBANK * pstLinksBank;
  
  /* Hash table where links are stored. */
  orxHASHTABLE * pstLinksHashTable;
};

/** Module static structure. */
typedef struct __orxSTATEMACHINE_STATIC_t
{
  orxU32 u32Flags;        /**< Module flags. */
} orxSTATEMACHINE_STATIC;


/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/
orxSTATIC orxSTATEMACHINE_STATIC sstStateMachine;


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Initialize state machine module.
 */
orxSTATUS orxStateMachine_Init()
{
  /* Module not already initialized? */
  orxASSERT(!(sstStateMachine.u32Flags & orxSTATEMACHINE_KU32_FLAG_READY));
  
  /* Cleans static controller. */
  orxMemory_Set(&sstStateMachine, 0, sizeof(orxSTATEMACHINE_STATIC));
  
  /* Set module as ready. */
  sstStateMachine.u32Flags = orxSTATEMACHINE_KU32_FLAG_READY;
  
  /* Module successfully initialized. */
  return orxSTATUS_SUCCESS;
}

/** Exit state machine module.
 */
orxVOID orxStateMachine_Exit()
{
  /* Module initialized? */
  orxASSERT((sstStateMachine.u32Flags & orxSTATEMACHINE_KU32_FLAG_READY) == orxSTATEMACHINE_KU32_FLAG_READY);
  
  /* Module not ready now. */
  sstStateMachine.u32Flags = orxSTATEMACHINE_KU32_FLAG_NONE;
}

/** Create a state machine and return a pointer on it.
 * @param[in] _u16NbStates          Number of states.
 * @param[in] _u32NbLinks           Number of links.
 * @param[in] _u32Flags             Flags used by the state machine.
 * @param[in] _eMemType             Memory type to use.
 * @return Returns a pointer on the state machine or orxNULL if failed.
 */
orxSTATEMACHINE * orxStateMachine_Create(orxU16 _u16NbStates, orxU32 _u32NbLinks, orxU32 _u32Flags, orxMEMORY_TYPE _eMemType)
{
  orxSTATEMACHINE * pstStateMachine = orxNULL;    /* New created state machine. */
  orxU32 u32BankFlags;                            /* Flags used for bank creation. */
  orxU32 u32LinkFlags;                            /* Flags used for hash table creation. */
  
  /* Module initialized? */
  orxASSERT((sstStateMachine.u32Flags & orxSTATEMACHINE_KU32_FLAG_READY) == orxSTATEMACHINE_KU32_FLAG_READY);
  
  /* Correct parameters? */
  orxASSERT(_eMemType < orxMEMORY_TYPE_NUMBER);
  orxASSERT(_u16NbStates > 0);
  orxASSERT(_u32NbLinks > 0);
  orxASSERT(_eMemType < orxMEMORY_TYPE_NUMBER);
  
  /* Allocate memory for a state machine. */
  pstStateMachine = (orxSTATEMACHINE *)orxMemory_Allocate(sizeof(orxSTATEMACHINE), _eMemType);
  
  /* Enough memory ? */
  if (pstStateMachine != orxNULL)
  {
    /* Set flags */
    if (_u32Flags == orxSTATEMACHINE_KU32_FLAGS_NOT_EXPANDABLE)
    {
      u32BankFlags = u32LinkFlags = orxBANK_KU32_FLAGS_NOT_EXPANDABLE;
    }
    else
    {
      u32BankFlags = u32LinkFlags = orxBANK_KU32_FLAGS_NONE;
    }
    
    /* Clean values. */
    orxMemory_Set(pstStateMachine, 0, sizeof(orxSTATEMACHINE));
    
    /* Allocate bank for states. */
    pstStateMachine->pstStatesBank = orxBank_Create(_u16NbStates, sizeof(orxSTATEMACHINE_STATE), u32BankFlags, _eMemType);
    
    /* Allocate hash table for states. */
    pstStateMachine->pstStatesHashTable = orxHashTable_Create(_u16NbStates, u32BankFlags, _eMemType);
    
    /* Allocate bank for links. */
    pstStateMachine->pstLinksBank = orxBank_Create(_u32NbLinks, sizeof(orxSTATEMACHINE_LINK), u32LinkFlags, _eMemType);
    
    /* Allocate hash table for links. */
    pstStateMachine->pstLinksHashTable = orxHashTable_Create(_u32NbLinks, u32LinkFlags, _eMemType);
    
    /* Correct bank allocations? */
    if (pstStateMachine->pstStatesBank == orxNULL || pstStateMachine->pstLinksHashTable == orxNULL)
    {
      /* Allocation problem, return orxNULL. */
      orxMemory_Free(pstStateMachine);
      pstStateMachine = orxNULL;
    }
  }
  
  return pstStateMachine;
}

/** Delete a state machine.
 * @param[in] _pstStateMachine      The state machine to remove.
 * @return Returns the status of the operation.
 */
orxVOID orxStateMachine_Delete(orxSTATEMACHINE * _pstStateMachine)
{
  /* Module initialized? */
  orxASSERT((sstStateMachine.u32Flags & orxSTATEMACHINE_KU32_FLAG_READY) == orxSTATEMACHINE_KU32_FLAG_READY);
  
  /* Correct parameters? */
  orxASSERT(_pstStateMachine != orxNULL);
  
  /* Clear state machine (unallocate cells from the bank). */
  orxStateMachine_Clear(_pstStateMachine);
  
  /* Free banks and hash tables. */
  orxBank_Delete(_pstStateMachine->pstStatesBank);
  orxHashTable_Delete(_pstStateMachine->pstStatesHashTable);
  orxBank_Delete(_pstStateMachine->pstLinksBank);
  orxHashTable_Delete(_pstStateMachine->pstLinksHashTable);
  
  /* Unallocate memory. */
  orxMemory_Free(_pstStateMachine);
}

/** Clear a state machine
 * @param[in] _pstStateMachine      The state machine to clear.
 */
orxVOID orxStateMachine_Clear(orxSTATEMACHINE * _pstStateMachine)
{
  /* Module initialized? */
  orxASSERT((sstStateMachine.u32Flags & orxSTATEMACHINE_KU32_FLAG_READY) == orxSTATEMACHINE_KU32_FLAG_READY);
  
  /* Correct parameters? */
  orxASSERT(_pstStateMachine != orxNULL);
  
  /* Clear memory banks and hash tables. */
  orxBank_Clear(_pstStateMachine->pstStatesBank);
  orxHashTable_Clear(_pstStateMachine->pstStatesHashTable);
  orxBank_Clear(_pstStateMachine->pstLinksBank);
  orxHashTable_Clear(_pstStateMachine->pstLinksHashTable);
}

/** Add a state.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _u16Id                Identifier for the state.
 * @param[in] _eStateType           Type of state.
 * @param[in] _cbAction              Action callback.
 * @return Returns the new state.
 */
orxSTATEMACHINE_STATE * orxStateMachine_State_Add(orxSTATEMACHINE * _pstStateMachine, orxU16 _u16Id, orxSTATEMACHINE_STATE_TYPE _eStateType, orxSTATEMACHINE_ACTION_PTR _cbAction)
{
  orxSTATEMACHINE_STATE * pstState;       /* New state to add. */
  
  /* Module initialized? */
  orxASSERT((sstStateMachine.u32Flags & orxSTATEMACHINE_KU32_FLAG_READY) == orxSTATEMACHINE_KU32_FLAG_READY);
  
  /* Correct parameters? */
  orxASSERT(_pstStateMachine != orxNULL);
  orxASSERT(_u16Id > 0);
  orxASSERT(_cbAction != orxNULL);
  
  /* Allocate a new state if possible. */
  pstState = (orxSTATEMACHINE_STATE *)orxBank_Allocate(_pstStateMachine->pstStatesBank);
  
  /* Define the new state. */
  if (pstState != orxNULL)
  {
    /* Set datas. */
    pstState->u16Id = _u16Id;
    pstState->eStateType = _eStateType;
    pstState->cbAction = _cbAction;
    
    /* Try to add the state to the hash table. */
    if (orxHashTable_Add(_pstStateMachine->pstStatesHashTable, _u16Id, pstState) != orxSTATUS_SUCCESS)
    {
      /* Couldn't insert the state in the hash table, so free it from the bank. */
      orxBank_Free(_pstStateMachine->pstStatesBank, pstState);
      pstState = orxNULL;
    }
  }
  
  return pstState;
}

/** Find a state.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _u16Id                The identifier of the state.
 * @return Returns the state.
 */
orxSTATEMACHINE_STATE * orxStateMachine_State_Get(orxSTATEMACHINE * _pstStateMachine, orxU16 _u16Id)
{
  /* Module initialized? */
  orxASSERT((sstStateMachine.u32Flags & orxSTATEMACHINE_KU32_FLAG_READY) == orxSTATEMACHINE_KU32_FLAG_READY);
  
  /* Correct parameters? */
  orxASSERT(_pstStateMachine != orxNULL);
  orxASSERT(_u16Id > 0);
  
  return orxHashTable_Get(_pstStateMachine->pstStatesHashTable, _u16Id);
}

/** Remove a state.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _pstState             The state to remove.
 * @param[in] _bRemoveLinks         Choose whether or not to remove the associated links.
 * @return Returns the status of the operation.
 */
orxSTATUS orxStateMachine_State_Remove(orxSTATEMACHINE * _pstStateMachine, orxSTATEMACHINE_STATE * _pstState, orxBOOL _bRemoveLinks)
{
  orxSTATEMACHINE_LINK * pstLink;         /* Explored link. */
  orxSTATUS eStatus = orxSTATUS_SUCCESS;  /* Status to return. */
  
  /* Module initialized? */
  orxASSERT((sstStateMachine.u32Flags & orxSTATEMACHINE_KU32_FLAG_READY) == orxSTATEMACHINE_KU32_FLAG_READY);
  
  /* Correct parameters? */
  orxASSERT(_pstStateMachine != orxNULL);
  orxASSERT(_pstState != orxNULL);
  
  /* Search and manage related links. */
  pstLink = orxBank_GetNext(_pstStateMachine->pstLinksBank, orxNULL);
  while (pstLink != orxNULL && eStatus == orxSTATUS_SUCCESS)
  {
    if (pstLink->pstBeginningState == _pstState || pstLink->pstEndingState == _pstState)
    {
      /* The link has a connection with this state. */
      if (_bRemoveLinks)
      {
        /* Remove the link. */
        if (orxHashTable_Remove(_pstStateMachine->pstLinksHashTable, orxU32KeyGen(pstLink->pstBeginningState->u16Id, pstLink->pstEndingState->u16Id)) == orxSTATUS_SUCCESS)
        {
          /* Remove the state from the bank. */
          orxBank_Free(_pstStateMachine->pstLinksBank, pstLink);
        }
        else
        {
          /* The state is not properly referenced in the hash table. */
          eStatus = orxSTATUS_FAILED;
        }
      }
      else
      {
        /* Not allowed to remove links, but one is associated... don't remove the state. */
        eStatus = orxSTATUS_FAILED;
      }
    }
    
    /* Explore the next link. */
    pstLink = orxBank_GetNext(_pstStateMachine->pstLinksBank, pstLink);
  }
  
  if (eStatus == orxSTATUS_SUCCESS)
  {
    /* Try to remove state from the hash table. */
    if (orxHashTable_Remove(_pstStateMachine->pstStatesHashTable, _pstState->u16Id) == orxSTATUS_SUCCESS)
    {
      /* Delete state from the bank. */
      orxBank_Free(_pstStateMachine->pstStatesBank, _pstState);
    }
    else
    {
      /* Can't remove state from hash table... problem. */
      eStatus = orxSTATUS_FAILED;
    }
  }
  
  return eStatus;
}

/** Add a link.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _pstBeginningState    The state marking the beginning of the link.
 * @param[in] _pstEndingState       The state marking the ending of the link.
 * @param[in] _cbCondition           Condition callback.
 * @param[in] _cbAction              Action callback.
 * @return Returns the new link.
 */
orxSTATEMACHINE_LINK * orxStateMachine_Link_Add(orxSTATEMACHINE * _pstStateMachine, orxSTATEMACHINE_STATE * _pstBeginningState, orxSTATEMACHINE_STATE * _pstEndingState, orxSTATEMACHINE_CONDITION_PTR _cbCondition, orxSTATEMACHINE_ACTION_PTR _cbAction)
{
  orxSTATEMACHINE_LINK * pstLink;     /* New link to add. */
  
  /* Module initialized? */
  orxASSERT((sstStateMachine.u32Flags & orxSTATEMACHINE_KU32_FLAG_READY) == orxSTATEMACHINE_KU32_FLAG_READY);
  
  /* Correct parameters? */
  orxASSERT(_pstStateMachine != orxNULL);
  orxASSERT(_pstBeginningState != orxNULL);
  orxASSERT(_pstEndingState != orxNULL);
  orxASSERT(_cbCondition != orxNULL);
  
  /* Allocate a new link if possible. */
  pstLink = (orxSTATEMACHINE_LINK *)orxBank_Allocate(_pstStateMachine->pstLinksBank);
  
  /* Define the new link. */
  if (pstLink != orxNULL)
  {
    /* Set datas. */
    pstLink->cbCondition = _cbCondition;
    pstLink->pstBeginningState = _pstBeginningState;
    pstLink->pstEndingState = _pstEndingState;
    pstLink->cbAction = _cbAction;
    
    /* Try to add the link to the hash table. */
    if (orxHashTable_Add(_pstStateMachine->pstLinksHashTable, orxU32KeyGen(pstLink->pstBeginningState->u16Id, pstLink->pstEndingState->u16Id), pstLink) != orxSTATUS_SUCCESS)
    {
      /* Couldn't insert the link in the hash table, so free it from the bank. */
      orxBank_Free(_pstStateMachine->pstLinksBank, pstLink);
      pstLink = orxNULL;
    }
  }
  
  return pstLink;
}

/** Find a link.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _pstBeginningState    The state marking the beginning of the link.
 * @param[in] _pstEndingState       The state marking the ending of the link.
 * @return Returns the corresponding link.
 */
orxSTATEMACHINE_LINK * orxStateMachine_Link_Get(orxSTATEMACHINE * _pstStateMachine, orxSTATEMACHINE_STATE * _pstBeginningState, orxSTATEMACHINE_STATE * _pstEndingState)
{
  /* Module initialized? */
  orxASSERT((sstStateMachine.u32Flags & orxSTATEMACHINE_KU32_FLAG_READY) == orxSTATEMACHINE_KU32_FLAG_READY);
  
  /* Correct parameters? */
  orxASSERT(_pstStateMachine != orxNULL);
  orxASSERT(_pstBeginningState != orxNULL);
  orxASSERT(_pstEndingState != orxNULL);
  
  return orxHashTable_Get(_pstStateMachine->pstLinksHashTable, orxU32KeyGen(_pstBeginningState->u16Id, _pstEndingState->u16Id));
}

/** Remove a link.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _pstLink              The link to remove.
 * @return Returns the status of the operation.
 */
orxSTATUS orxStateMachine_Link_Remove(orxSTATEMACHINE * _pstStateMachine, orxSTATEMACHINE_LINK * _pstLink)
{
  orxSTATUS eStatus = orxSTATUS_FAILED;   /* Status to return. */
  
  /* Module initialized? */
  orxASSERT((sstStateMachine.u32Flags & orxSTATEMACHINE_KU32_FLAG_READY) == orxSTATEMACHINE_KU32_FLAG_READY);
  
  /* Correct parameters? */
  orxASSERT(_pstStateMachine != orxNULL);
  orxASSERT(_pstLink != orxNULL);
  
  /* Try to remove link from the hash table. */
  if (orxHashTable_Remove(_pstStateMachine->pstLinksHashTable, orxU32KeyGen(_pstLink->pstBeginningState->u16Id, _pstLink->pstEndingState->u16Id)) == orxSTATUS_SUCCESS)
  {
    /* Delete link from the bank. */
    orxBank_Free(_pstStateMachine->pstLinksBank, _pstLink);
    
    eStatus = orxSTATUS_SUCCESS;
  }
  
  return eStatus;
}

/** Clear all links.
 * @param[in] _pstStateMachine      The state machine.
 */
orxVOID orxStateMachine_Link_Clear(orxSTATEMACHINE * _pstStateMachine)
{
  /* Module initialized? */
  orxASSERT((sstStateMachine.u32Flags & orxSTATEMACHINE_KU32_FLAG_READY) == orxSTATEMACHINE_KU32_FLAG_READY);
  
  /* Correct parameters? */
  orxASSERT(_pstStateMachine != orxNULL);
  
  /* Clear memory banks and hash tables. */
  orxBank_Clear(_pstStateMachine->pstLinksBank);
  orxHashTable_Clear(_pstStateMachine->pstLinksHashTable);
}

/** Get next state.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _pstState             The current state.
 * @return Returns the next state by following a link (and executes respective callbacks). If _pstState is orxNULL, the initial state will be returned (and its callback executed). Returns orxNULL when no more state can be returned.
 */
orxSTATEMACHINE_STATE * orxStateMachine_Link_GetNext(orxSTATEMACHINE * _pstStateMachine, orxSTATEMACHINE_STATE * _pstState)
{
  orxSTATEMACHINE_STATE * pstTestedState;             /* Tested state. */
  orxSTATEMACHINE_LINK * pstLink;                     /* Explored link. */
  orxSTATEMACHINE_STATE * pstNextState = orxNULL;     /* Next state. */
  
  /* Module initialized? */
  orxASSERT((sstStateMachine.u32Flags & orxSTATEMACHINE_KU32_FLAG_READY) == orxSTATEMACHINE_KU32_FLAG_READY);
  
  /* Correct parameters? */
  orxASSERT(_pstStateMachine != orxNULL);
  
  if (_pstState == orxNULL)
  {
    /* Enter the initial state. */
    pstNextState = _pstStateMachine->stInitialState;

    /* Execute callback. */
    pstNextState->cbAction();
  }
  else
  {
    /* Find an applicable link to follow. */
    pstTestedState = orxBank_GetNext(_pstStateMachine->pstStatesBank, orxNULL);
    while (pstTestedState != orxNULL)
    {
      pstLink = orxStateMachine_Link_Get(_pstStateMachine, _pstState, pstTestedState);
      if (pstLink == orxNULL)
      {
        /* A link has not been found yet... continue the search. */
        pstTestedState = orxBank_GetNext(_pstStateMachine->pstStatesBank, pstTestedState);
      }
      else
      {
        /* A link has been found... verify it. */
        if (pstLink->cbCondition())
        {
          /* The link is valid. Execute callback... */
          pstLink->cbAction();
          
          /* Define new state... */
          pstNextState = pstLink->pstEndingState;
          
          /* Execute callback. */
          pstNextState->cbAction();
          
          /* Stop search. */
          pstTestedState = orxNULL;
        }
        else
        {
          /* The link was not valid... search a new one. */
          pstTestedState = orxBank_GetNext(_pstStateMachine->pstStatesBank, pstTestedState);
        }
      }
    }
  }
  
  return pstNextState;
}
