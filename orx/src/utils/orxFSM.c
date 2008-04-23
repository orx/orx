/**
 *@file orxFSM.c
 *
 *State Machine
 *
 *@todo 
 */
 
 /***************************************************************************
 orxFSM.c
 State Machine.
 
 begin                : 20/06/2005
 author               : (C) Arcallians
 email                : corrosifdev@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/

#include "utils/orxFSM.h"
#include "utils/orxHashTable.h"
#include "memory/orxBank.h"
#include "debug/orxDebug.h"

#define orxFSM_KU32_STATIC_FLAG_NONE  0x00000000  /**< No flags have been set. */
#define orxFSM_KU32_STATIC_FLAG_READY 0x00000001  /**< The module has been initialized. */

#define orxFSM_ALLOC_NB               10          /**< Number of state machines to allocate at first. */
#define orxFSM_INST_ALLOC_NB          10          /**< Number of instances to allocate at first. */

/***************************************************************************
 * Macro declaration                                                       *
 ***************************************************************************/

#ifndef orxU32KeyGen
#define orxU32KeyGen(a, b) (((orxU32)a << 16) + b)
#endif


/***************************************************************************
 * Enum declaration                                                        *
 ***************************************************************************/

/** State position. */
typedef enum __orxFSM_STATE_POSITION_t
{
  orxFSM_STATE_POSITION_INIT = 0,
  orxFSM_STATE_POSITION_EXECUTE,
  orxFSM_STATE_POSITION_EXIT,

  orxFSM_STATE_POSITION_NUMBER,

  orxFSM_STATE_POSITION_NONE = orxENUM_NONE,

} orxFSM_STATE_POSITION;


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** States structure. */
struct __orxFSM_STATE_t
{
  /* State identifier. */
  orxU16 u16Id;
  
  /* Action to realize when entering this state. */
  orxFSM_ACTION_FUNCTION pfnInit;
  
  /* Action to realize when executing this state. */
  orxFSM_ACTION_FUNCTION pfnExecute;
  
  /* Action to realize when exiting this state. */
  orxFSM_ACTION_FUNCTION pfnExit;
};

/** Link structure. */
struct __orxFSM_LINK_t
{
  /* Condition to follow the link. */
  orxFSM_CONDITION_FUNCTION pfnCondition;
  
  /* The state marking the beginning of the link. */
  orxFSM_STATE *pstBeginningState;
  
  /* The state marking the end of the link. */
  orxFSM_STATE *pstEndingState;
};

/** State machines structure. */
struct __orxFSM_t
{
  /* Initial state. */
  orxFSM_STATE *pstInitialState;
  
  /* Bank where states are stored. */
  orxBANK *pstStatesBank;

  /* Hash table where states are stored. */
  orxHASHTABLE *pstStatesHashTable;

  /* Bank where links are stored. */
  orxBANK *pstLinksBank;
  
  /* Hash table where links are stored. */
  orxHASHTABLE *pstLinksHashTable;
  
  /* Bank where instances are stored. */
  orxBANK *pstInstancesBank;
};

/** Instance structure. */
struct __orxFSM_INSTANCE_t
{
  /* The state machine. */
  orxFSM *pstStateMachine;
  
  /* The current state. */
  orxFSM_STATE *pstCurrentState;
  
  /* The current state position. */
  orxFSM_STATE_POSITION eStatePosition;
};

/** Module static structure. */
typedef struct __orxFSM_STATIC_t
{
  /* Bank where state machines are stored. */
  orxBANK *pstStateMachinesBank;

  /* Modules flags. */
  orxU32 u32Flags;
  
} orxFSM_STATIC;


/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/

orxSTATIC orxFSM_STATIC sstFSM;


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/***************************************************************************
 orxFSM_Setup
 FSM module setup.

 returns: nothing
 ***************************************************************************/
orxVOID orxFSM_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_FSM, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_FSM, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_FSM, orxMODULE_ID_HASHTABLE);

  return;
}

/** Initialize state machine module.
 */
orxSTATUS orxFSM_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstFSM.u32Flags & orxFSM_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller. */
    orxMemory_Set(&sstFSM, 0, sizeof(orxFSM_STATIC));
  
    /* Allocate bank for state machines. */
    sstFSM.pstStateMachinesBank = orxBank_Create(orxFSM_ALLOC_NB, sizeof(orxFSM), orxFSM_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
    
    if(sstFSM.pstStateMachinesBank != orxNULL)
    {
      /* Set module as ready. */
      sstFSM.u32Flags = orxFSM_KU32_STATIC_FLAG_READY;
      
      /* Success */
      eResult = orxSTATUS_SUCCESS;
    }
  }
  else
  {
    /* !!! MSG !!! */

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Module successfully initialized. */
  return eResult;
}

/** Exit state machine module.
 */
orxVOID orxFSM_Exit()
{
  /* Module initialized? */
  orxASSERT((sstFSM.u32Flags & orxFSM_KU32_STATIC_FLAG_READY) == orxFSM_KU32_STATIC_FLAG_READY);
  
  /* Free bank for state machines. */
  if(sstFSM.pstStateMachinesBank != orxNULL)
  {
    orxBank_Delete(sstFSM.pstStateMachinesBank);
    sstFSM.pstStateMachinesBank = orxNULL;
  }
  
  /* Module not ready now. */
  sstFSM.u32Flags = orxFSM_KU32_STATIC_FLAG_NONE;

  return;
}

/** Create a state machine and return a pointer on it.
 * @param[in] _u16NbStates          Number of states.
 * @param[in] _u32NbLinks           Number of links.
 * @param[in] _u32NbInstances       Number of instances.
 * @param[in] _u32Flags             Flags used by the state machine.
 * @param[in] _eMemType             Memory type to use.
 * @return Returns a pointer on the state machine or orxNULL if failed.
 */
orxFSM *orxFASTCALL orxFSM_Create(orxU16 _u16NbStates, orxU32 _u32NbLinks, orxU32 _u32NbInstances, orxU32 _u32Flags, orxMEMORY_TYPE _eMemType)
{
  orxFSM *pstStateMachine = orxNULL;              /* New created state machine. */
  orxU32 u32BankFlags;                            /* Flags used for bank creation. */
  orxU32 u32LinkFlags;                            /* Flags used for hash table creation. */
  orxU32 u32InstanceFlags;                        /* Flags used for instance creation. */
  
  /* Module initialized? */
  orxASSERT((sstFSM.u32Flags & orxFSM_KU32_STATIC_FLAG_READY) == orxFSM_KU32_STATIC_FLAG_READY);
  
  /* Correct parameters? */
  orxASSERT(_eMemType < orxMEMORY_TYPE_NUMBER);
  orxASSERT(_u16NbStates > 0);
  orxASSERT(_u32NbLinks > 0);
  orxASSERT(_u32NbInstances > 0);
  orxASSERT(_eMemType < orxMEMORY_TYPE_NUMBER);
  
  /* Allocate memory for a state machine if possible. */
  pstStateMachine = (orxFSM *)orxBank_Allocate(sstFSM.pstStateMachinesBank);

  /* Allocation succeeded ? */
  if(pstStateMachine != orxNULL)
  { 
    /* Cleans it */
    orxMemory_Set(pstStateMachine, 0, sizeof(orxFSM));

    /* Set flags */
    if(_u32Flags == orxFSM_KU32_FLAG_NOT_EXPANDABLE)
    {
      u32BankFlags = u32LinkFlags = u32InstanceFlags = orxBANK_KU32_FLAG_NOT_EXPANDABLE;
    }
    else
    {
      u32BankFlags = u32LinkFlags = u32InstanceFlags = orxBANK_KU32_FLAG_NONE;
    }
    
    /* Allocate bank for states. */
    pstStateMachine->pstStatesBank = orxBank_Create(_u16NbStates, sizeof(orxFSM_STATE), u32BankFlags, _eMemType);
    
    /* Correct bank allocations? */
    if(pstStateMachine->pstStatesBank != orxNULL)
    { 
      /* Allocate hash table for states. */
      pstStateMachine->pstStatesHashTable = orxHashTable_Create(_u16NbStates, u32BankFlags, _eMemType);
      
      /* Allocate bank for links. */
      pstStateMachine->pstLinksBank = orxBank_Create((orxU16)_u32NbLinks, sizeof(orxFSM_LINK), u32LinkFlags, _eMemType);
    
      /* Correct bank allocations? */
      if(pstStateMachine->pstLinksBank != orxNULL)
      {
        /* Allocate hash table for links. */
        pstStateMachine->pstLinksHashTable = orxHashTable_Create(_u32NbLinks, u32LinkFlags, _eMemType);
        
        /* Allocate bank for instances. */
        pstStateMachine->pstInstancesBank = orxBank_Create((orxU16)_u32NbInstances, sizeof(orxFSM_INSTANCE), u32InstanceFlags, _eMemType);
        
        /* Correct bank allocations? */
        if(pstStateMachine->pstInstancesBank == orxNULL)
        {
          /* Free bank for links. */
          orxBank_Delete(pstStateMachine->pstLinksBank);
          pstStateMachine->pstLinksBank = orxNULL;
          
          /* Free bank for states. */
          orxBank_Delete(pstStateMachine->pstStatesBank);
          pstStateMachine->pstStatesBank = orxNULL;
          
          /* Free hash table for states. */
          orxHashTable_Delete(pstStateMachine->pstStatesHashTable);
          pstStateMachine->pstStatesHashTable = orxNULL;
          
          /* Remove state machine from bank. */
          orxBank_Free(sstFSM.pstStateMachinesBank, pstStateMachine);
          pstStateMachine = orxNULL;
        }
      }
      else
      {
        /* Free bank for states. */
        orxBank_Delete(pstStateMachine->pstStatesBank);
        pstStateMachine->pstStatesBank = orxNULL;
        
        /* Free hash table for states. */
        orxHashTable_Delete(pstStateMachine->pstStatesHashTable);
        pstStateMachine->pstStatesHashTable = orxNULL;
        
        /* Remove state machine from bank. */
        orxBank_Free(sstFSM.pstStateMachinesBank, pstStateMachine);
        pstStateMachine = orxNULL;
      }
    }
    else
    {
      /* Remove state machine from bank. */
      orxBank_Free(sstFSM.pstStateMachinesBank, pstStateMachine);
      pstStateMachine = orxNULL;
    }
  }
  
  return pstStateMachine;
}

/** Delete a state machine.
 * @param[in] _pstStateMachine      The state machine to remove.
 * @return Returns the status of the operation.
 */
orxVOID orxFASTCALL orxFSM_Delete(orxFSM *_pstStateMachine)
{
  /* Module initialized? */
  orxASSERT((sstFSM.u32Flags & orxFSM_KU32_STATIC_FLAG_READY) == orxFSM_KU32_STATIC_FLAG_READY);

  /* Correct parameters? */
  orxASSERT(_pstStateMachine != orxNULL);

  /* Free banks and hash tables. */
  orxBank_Delete(_pstStateMachine->pstStatesBank);
  orxHashTable_Delete(_pstStateMachine->pstStatesHashTable);
  orxBank_Delete(_pstStateMachine->pstLinksBank);
  orxHashTable_Delete(_pstStateMachine->pstLinksHashTable);
  orxBank_Delete(_pstStateMachine->pstInstancesBank);

  /* Remove state machine from bank. */
  orxBank_Free(sstFSM.pstStateMachinesBank, _pstStateMachine);

  return;
}

/** Clear a state machine
 * @param[in] _pstStateMachine      The state machine to clear.
 */
orxVOID orxFASTCALL orxFSM_Clear(orxFSM *_pstStateMachine)
{
  /* Module initialized? */
  orxASSERT((sstFSM.u32Flags & orxFSM_KU32_STATIC_FLAG_READY) == orxFSM_KU32_STATIC_FLAG_READY);
  
  /* Correct parameters? */
  orxASSERT(_pstStateMachine != orxNULL);
  
  /* Set values to default. */
  _pstStateMachine->pstInitialState = orxNULL;
  
  /* Clear memory banks and hash tables. */
  orxBank_Clear(_pstStateMachine->pstStatesBank);
  orxHashTable_Clear(_pstStateMachine->pstStatesHashTable);
  orxBank_Clear(_pstStateMachine->pstLinksBank);
  orxHashTable_Clear(_pstStateMachine->pstLinksHashTable);
  orxBank_Clear(_pstStateMachine->pstInstancesBank);
}

/** Add a state, setting it as the initial state if it is the first one.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _u16Id                Identifier for the state.
 * @param[in] _pfnInit               Init callback.
 * @param[in] _pfnExecute            Execute callback.
 * @param[in] _pfnExit               Exit callback.
 * @return Returns the new state.
 */
orxFSM_STATE *orxFASTCALL orxFSM_AddState(orxFSM *_pstStateMachine, orxU16 _u16Id, orxCONST orxFSM_ACTION_FUNCTION _pfnInit, orxCONST orxFSM_ACTION_FUNCTION _pfnExecute, orxCONST orxFSM_ACTION_FUNCTION _pfnExit)
{
  orxFSM_STATE *pstState;          /* New state to add. */
  orxBOOL bFirst;                   /* First state to be added? */
  
  /* Module initialized? */
  orxASSERT((sstFSM.u32Flags & orxFSM_KU32_STATIC_FLAG_READY) == orxFSM_KU32_STATIC_FLAG_READY);
  
  /* Correct parameters? */
  orxASSERT(_pstStateMachine != orxNULL);
  orxASSERT(_pfnInit != orxNULL);
  orxASSERT(_pfnExecute != orxNULL);
  orxASSERT(_pfnExit != orxNULL);
  
  /* Is it the first state? */
  bFirst = (orxBank_GetNext(_pstStateMachine->pstStatesBank, orxNULL) == orxNULL);

  /* Allocate a new state if possible. */
  pstState = (orxFSM_STATE *)orxBank_Allocate(_pstStateMachine->pstStatesBank);
  
  /* Define the new state. */
  if(pstState != orxNULL)
  {
    /* Cleans it */
    orxMemory_Set(pstState, 0, sizeof(orxFSM_STATE));

    if(bFirst)
    {
      /* It is the first state: set it as the initial state. */
      _pstStateMachine->pstInitialState = pstState;
    }
    
    /* Set datas. */
    pstState->u16Id       = _u16Id;
    pstState->pfnInit     = _pfnInit;
    pstState->pfnExecute  = _pfnExecute;
    pstState->pfnExit     = _pfnExit;
    
    /* Try to add the state to the hash table. */
    if(orxHashTable_Add(_pstStateMachine->pstStatesHashTable, _u16Id, pstState) != orxSTATUS_SUCCESS)
    {
      /* Couldn't insert the state in the hash table, so free it from the bank. */
      orxBank_Free(_pstStateMachine->pstStatesBank, pstState);
      pstState = orxNULL;
    }
  }
  
  return pstState;
}

/** Set an initial state.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _pstInitialState      The initial state.
 * @return Returns the status of the operation.
 */
orxSTATUS orxFASTCALL orxFSM_SetInitState(orxFSM *_pstStateMachine, orxFSM_STATE *_pstInitialState)
{
  orxFSM_STATE *pstState;                  /* The explored state. */
  orxSTATUS eStatus = orxSTATUS_FAILURE;     /* Status to return. */
  
  /* Module initialized? */
  orxASSERT((sstFSM.u32Flags & orxFSM_KU32_STATIC_FLAG_READY) == orxFSM_KU32_STATIC_FLAG_READY);
  
  /* Correct parameters? */
  orxASSERT(_pstStateMachine != orxNULL);
  orxASSERT(_pstInitialState != orxNULL);
  
  /* Verify that the proposed initial state is part of the FSM. */
  pstState = orxBank_GetNext(_pstStateMachine->pstStatesBank, orxNULL);
  while(pstState != orxNULL && eStatus == orxSTATUS_FAILURE)
  {
    if(pstState == _pstInitialState)
    {
      /* The initial state is part of the FSM. */
      eStatus = orxSTATUS_SUCCESS;
    }
    
    /* Explore the next state. */
    pstState = orxBank_GetNext(_pstStateMachine->pstStatesBank, pstState);
  }
  
  if(eStatus == orxSTATUS_SUCCESS)
  {
    /* Set the initial state. */
    _pstStateMachine->pstInitialState = _pstInitialState;
  }
  
  return eStatus;
}

/** Get a state.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _u16Id                The identifier of the state.
 * @return Returns the state.
 */
orxFSM_STATE *orxFASTCALL orxFSM_GetState(orxCONST orxFSM *_pstStateMachine, orxU16 _u16Id)
{
  /* Module initialized? */
  orxASSERT((sstFSM.u32Flags & orxFSM_KU32_STATIC_FLAG_READY) == orxFSM_KU32_STATIC_FLAG_READY);
  
  /* Correct parameters? */
  orxASSERT(_pstStateMachine != orxNULL);
  
  return orxHashTable_Get(_pstStateMachine->pstStatesHashTable, _u16Id);
}

/** Get a state Id.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _pstState             The state.
 * @return Returns the Id of the state.
 */
orxU16 orxFASTCALL orxFSM_GetStateID(orxCONST orxFSM *_pstStateMachine, orxCONST orxFSM_STATE *_pstState)
{
  /* Module initialized? */
  orxASSERT((sstFSM.u32Flags & orxFSM_KU32_STATIC_FLAG_READY) == orxFSM_KU32_STATIC_FLAG_READY);
  
  /* Correct parameters? */
  orxASSERT(_pstStateMachine != orxNULL);
  orxASSERT(_pstState != orxNULL);
  
  return _pstState->u16Id;
}

/** Remove a state.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _pstState             The state to remove.
 * @param[in] _bRemoveLinks         Choose whether or not to remove the associated links.
 * @return Returns the status of the operation.
 */
orxSTATUS orxFASTCALL orxFSM_RemoveState(orxFSM *_pstStateMachine, orxFSM_STATE *_pstState, orxBOOL _bRemoveLinks)
{
  orxFSM_LINK *pstLink;                  /* Explored link. */
  orxSTATUS eStatus = orxSTATUS_SUCCESS;  /* Status to return. */
  
  /* Module initialized? */
  orxASSERT((sstFSM.u32Flags & orxFSM_KU32_STATIC_FLAG_READY) == orxFSM_KU32_STATIC_FLAG_READY);
  
  /* Correct parameters? */
  orxASSERT(_pstStateMachine != orxNULL);
  orxASSERT(_pstState != orxNULL);
  
  /* Search and manage related links. */
  pstLink = orxBank_GetNext(_pstStateMachine->pstLinksBank, orxNULL);
  while(pstLink != orxNULL && eStatus == orxSTATUS_SUCCESS)
  {
    if(pstLink->pstBeginningState == _pstState || pstLink->pstEndingState == _pstState)
    {
      /* The link has a connection with this state. */
      if(_bRemoveLinks)
      {
        /* Remove the link. */
        if(orxHashTable_Remove(_pstStateMachine->pstLinksHashTable, orxU32KeyGen(pstLink->pstBeginningState->u16Id, pstLink->pstEndingState->u16Id)) == orxSTATUS_SUCCESS)
        {
          /* Remove the state from the bank. */
          orxBank_Free(_pstStateMachine->pstLinksBank, pstLink);
        }
        else
        {
          /* The state is not properly referenced in the hash table. */
          eStatus = orxSTATUS_FAILURE;
        }
      }
      else
      {
        /* Not allowed to remove links, but one is associated... don't remove the state. */
        eStatus = orxSTATUS_FAILURE;
      }
    }
    
    /* Explore the next link. */
    pstLink = orxBank_GetNext(_pstStateMachine->pstLinksBank, pstLink);
  }
  
  if(eStatus == orxSTATUS_SUCCESS)
  {
    /* Try to remove state from the hash table. */
    if(orxHashTable_Remove(_pstStateMachine->pstStatesHashTable, _pstState->u16Id) == orxSTATUS_SUCCESS)
    {
      /* Delete state from the bank. */
      orxBank_Free(_pstStateMachine->pstStatesBank, _pstState);
    }
    else
    {
      /* Can't remove state from hash table... problem. */
      eStatus = orxSTATUS_FAILURE;
    }
  }
  
  return eStatus;
}

/** Add a link.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _pstBeginningState    The state marking the beginning of the link.
 * @param[in] _pstEndingState       The state marking the ending of the link.
 * @param[in] _pfnCondition          Condition callback.
 * @return Returns the new link.
 */
orxFSM_LINK *orxFASTCALL orxFSM_AddLink(orxFSM *_pstStateMachine, orxFSM_STATE *_pstBeginningState, orxFSM_STATE *_pstEndingState, orxCONST orxFSM_CONDITION_FUNCTION _pfnCondition)
{
  orxFSM_LINK *pstLink;     /* New link to add. */
  
  /* Module initialized? */
  orxASSERT((sstFSM.u32Flags & orxFSM_KU32_STATIC_FLAG_READY) == orxFSM_KU32_STATIC_FLAG_READY);
  
  /* Correct parameters? */
  orxASSERT(_pstStateMachine != orxNULL);
  orxASSERT(_pstBeginningState != orxNULL);
  orxASSERT(_pstEndingState != orxNULL);
  orxASSERT(_pfnCondition != orxNULL);
  
  /* Allocate a new link if possible. */
  pstLink = (orxFSM_LINK *)orxBank_Allocate(_pstStateMachine->pstLinksBank);
  
  /* Define the new link. */
  if(pstLink != orxNULL)
  {
    /* Cleans it */
    orxMemory_Set(pstLink, 0, sizeof(orxFSM_LINK));

    /* Set datas. */
    pstLink->pfnCondition = _pfnCondition;
    pstLink->pstBeginningState = _pstBeginningState;
    pstLink->pstEndingState = _pstEndingState;
    
    /* Try to add the link to the hash table. */
    if(orxHashTable_Add(_pstStateMachine->pstLinksHashTable, orxU32KeyGen(pstLink->pstBeginningState->u16Id, pstLink->pstEndingState->u16Id), pstLink) != orxSTATUS_SUCCESS)
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
orxFSM_LINK *orxFASTCALL orxFSM_GetLink(orxCONST orxFSM *_pstStateMachine, orxCONST orxFSM_STATE *_pstBeginningState, orxCONST orxFSM_STATE *_pstEndingState)
{
  /* Module initialized? */
  orxASSERT((sstFSM.u32Flags & orxFSM_KU32_STATIC_FLAG_READY) == orxFSM_KU32_STATIC_FLAG_READY);
  
  /* Correct parameters? */
  orxASSERT(_pstStateMachine != orxNULL);
  orxASSERT(_pstBeginningState != orxNULL);
  orxASSERT(_pstEndingState != orxNULL);
  
  return orxHashTable_Get(_pstStateMachine->pstLinksHashTable, orxU32KeyGen(_pstBeginningState->u16Id, _pstEndingState->u16Id));
}

/**Remove a link.
 *@param[in] _pstStateMachine      The state machine.
 *@param[in] _pstLink              The link to remove.
 *@return Returns the status of the operation.
 */
orxSTATUS orxFASTCALL orxFSM_RemoveLink(orxFSM *_pstStateMachine, orxFSM_LINK *_pstLink)
{
  orxSTATUS eStatus = orxSTATUS_FAILURE;   /* Status to return. */
  
  /* Module initialized? */
  orxASSERT((sstFSM.u32Flags & orxFSM_KU32_STATIC_FLAG_READY) == orxFSM_KU32_STATIC_FLAG_READY);
  
  /* Correct parameters? */
  orxASSERT(_pstStateMachine != orxNULL);
  orxASSERT(_pstLink != orxNULL);
  
  /* Try to remove link from the hash table. */
  if(orxHashTable_Remove(_pstStateMachine->pstLinksHashTable, orxU32KeyGen(_pstLink->pstBeginningState->u16Id, _pstLink->pstEndingState->u16Id)) == orxSTATUS_SUCCESS)
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
orxVOID orxFASTCALL orxFSM_ClearLink(orxFSM *_pstStateMachine)
{
  /* Module initialized? */
  orxASSERT((sstFSM.u32Flags & orxFSM_KU32_STATIC_FLAG_READY) == orxFSM_KU32_STATIC_FLAG_READY);
  
  /* Correct parameters? */
  orxASSERT(_pstStateMachine != orxNULL);
  
  /* Clear memory banks and hash tables. */
  orxBank_Clear(_pstStateMachine->pstLinksBank);
  orxHashTable_Clear(_pstStateMachine->pstLinksHashTable);
}

/** Create an instance of a state machine.
 * @param[in] _pstStateMachine      The state machine.
 * @return Returns the instance.
 */
orxFSM_INSTANCE *orxFASTCALL orxFSM_CreateInstance(orxFSM *_pstStateMachine)
{
  orxFSM_INSTANCE *pstInstance = orxNULL;     /* New created instance. */
  
  /* Module initialized? */
  orxASSERT((sstFSM.u32Flags & orxFSM_KU32_STATIC_FLAG_READY) == orxFSM_KU32_STATIC_FLAG_READY);
  
  /* Correct parameters? */
  orxASSERT(_pstStateMachine != orxNULL);
  
  /* Allocate memory for an instance if possible. */
  pstInstance = (orxFSM_INSTANCE *)orxBank_Allocate(_pstStateMachine->pstInstancesBank);
  
  /* Allocation succeeded? */
  if(pstInstance != orxNULL)
  {
    /* Cleans it */
    orxMemory_Set(pstInstance, 0, sizeof(orxFSM_INSTANCE));

    pstInstance->pstStateMachine = _pstStateMachine;
    pstInstance->pstCurrentState = orxNULL;
    pstInstance->eStatePosition = orxFSM_STATE_POSITION_NONE;
  }
  
  return pstInstance;
}

/** Remove an instance of a state machine.
 * @param[in] _pstInstance          The instance to remove.
 * @return Returns the status of the operation.
 */
orxSTATUS orxFASTCALL orxFSM_DeleteInstance(orxFSM_INSTANCE *_pstInstance)
{
  orxSTATUS eStatus = orxSTATUS_SUCCESS;  /* Status to return. */
    
  /* Module initialized? */
  orxASSERT((sstFSM.u32Flags & orxFSM_KU32_STATIC_FLAG_READY) == orxFSM_KU32_STATIC_FLAG_READY);
  
  /* Correct parameters? */
  orxASSERT(_pstInstance != orxNULL);
  
  /* Delete instance from the bank. */
  orxBank_Free(_pstInstance->pstStateMachine->pstInstancesBank, _pstInstance);
  _pstInstance = orxNULL;
    
  return eStatus;
}

/** Get the state machine associated to an instance.
 * @param[in] _pstInstance          The instance.
 * @return Returns the state machine.
 */
orxFSM *orxFASTCALL orxFSM_GetFSM(orxCONST orxFSM_INSTANCE *_pstInstance)
{
  /* Module initialized? */
  orxASSERT((sstFSM.u32Flags & orxFSM_KU32_STATIC_FLAG_READY) == orxFSM_KU32_STATIC_FLAG_READY);
  
  /* Correct parameters? */
  orxASSERT(_pstInstance != orxNULL);
  
  return _pstInstance->pstStateMachine;
}

/** Get the current state of an instance.
 * @param[in] _pstInstance          The instance.
 * @return Returns the current state.
 */
orxFSM_STATE *orxFASTCALL orxFSM_GetInstanceState(orxCONST orxFSM_INSTANCE *_pstInstance)
{
  /* Module initialized? */
  orxASSERT((sstFSM.u32Flags & orxFSM_KU32_STATIC_FLAG_READY) == orxFSM_KU32_STATIC_FLAG_READY);
  
  /* Correct parameters? */
  orxASSERT(_pstInstance != orxNULL);
  
  return _pstInstance->pstCurrentState;
}

/** Update an instance of a state machine.
 * @param[in] _pstInstance          The instance.
 * @return Returns the status of the operation. It fails if nothing has happend.
 */
orxSTATUS orxFASTCALL orxFSM_UpdateInstance(orxFSM_INSTANCE *_pstInstance)
{
  orxSTATUS eStatus = orxSTATUS_FAILURE;     /* Status to return. */
  
  /* Module initialized? */
  orxASSERT((sstFSM.u32Flags & orxFSM_KU32_STATIC_FLAG_READY) == orxFSM_KU32_STATIC_FLAG_READY);
  
  /* Correct parameters? */
  orxASSERT(_pstInstance != orxNULL);
  
  if(_pstInstance->pstCurrentState == orxNULL)
  {
    /* Enter the initial state. */
    _pstInstance->pstCurrentState = _pstInstance->pstStateMachine->pstInitialState;
    
    /* Set current position to 'Init'. */
    _pstInstance->eStatePosition = orxFSM_STATE_POSITION_INIT;
    
    /* Execute 'Init' callback */
    _pstInstance->pstCurrentState->pfnInit();
    
    eStatus = orxSTATUS_SUCCESS;
  }
  else
  {
    switch (_pstInstance->eStatePosition)
    {
      case orxFSM_STATE_POSITION_INIT:
      {
        /* Set current position to 'Execute'. */
        _pstInstance->eStatePosition = orxFSM_STATE_POSITION_EXECUTE;
        
        /* Execute 'Execute' callback. */
        _pstInstance->pstCurrentState->pfnExecute();
        
        eStatus = orxSTATUS_SUCCESS;
        
        break;
      }
      case orxFSM_STATE_POSITION_EXECUTE:
      {
        /* Set current position to 'Exit'. */
        _pstInstance->eStatePosition = orxFSM_STATE_POSITION_EXIT;
        
        /* Execute 'Exit' callback. */
        _pstInstance->pstCurrentState->pfnExit();
        
        eStatus = orxSTATUS_SUCCESS;
        
        break;
      }
      case orxFSM_STATE_POSITION_EXIT:
      {
        orxFSM_STATE *pstTestedState;     /* Tested state. */
        orxFSM_LINK *pstLink;             /* Explored link. */
        
        /* Find an applicable link to follow. */
        pstTestedState = orxBank_GetNext(_pstInstance->pstStateMachine->pstStatesBank, orxNULL);
        while(pstTestedState != orxNULL)
        {
          pstLink = orxFSM_GetLink(_pstInstance->pstStateMachine, _pstInstance->pstCurrentState, pstTestedState);
          if(pstLink == orxNULL)
          {
            /* A link has not been found yet... continue the search. */
            pstTestedState = orxBank_GetNext(_pstInstance->pstStateMachine->pstStatesBank, pstTestedState);
          }
          else
          {
            /* A link has been found... verify it. */
            if(pstLink->pfnCondition())
            {
              /* The link is valid... enter the new state. */
              _pstInstance->pstCurrentState = pstLink->pstEndingState;
              
              /* Set current position to 'Init'. */
              _pstInstance->eStatePosition = orxFSM_STATE_POSITION_INIT;
              
              /* Execute 'Init' callback. */
              _pstInstance->pstCurrentState->pfnInit();
              
              eStatus = orxSTATUS_SUCCESS;
              
              /* Stop search. */
              pstTestedState = orxNULL;
            }
            else
            {
              /* The link was not valid... search a new one. */
              pstTestedState = orxBank_GetNext(_pstInstance->pstStateMachine->pstStatesBank, pstTestedState);
            }
          }
        }
        
        break;
      }
      default:
      {
        /* No position defined: problem! */
        eStatus = orxSTATUS_FAILURE;
      }
    }
  }
  
  return eStatus;
}

/** Update all instances of a state machine. If current state is orxNULL, it enters the initial state.
 * @param[in] _pstStateMachine      The state machine.
 * @return Returns the status of the operation. It fails if nothing has happend.
 */
orxSTATUS orxFASTCALL orxFSM_Update(orxFSM *_pstStateMachine)
{
  orxFSM_INSTANCE *pstInstance;            /* Explored instance. */
  orxSTATUS eStatus = orxSTATUS_SUCCESS;    /* Status to return. */
  
  /* Module initialized? */
  orxASSERT((sstFSM.u32Flags & orxFSM_KU32_STATIC_FLAG_READY) == orxFSM_KU32_STATIC_FLAG_READY);
  
  /* Correct parameters? */
  orxASSERT(_pstStateMachine != orxNULL);
  
  /* Loop while there are instances and no problem. */
  pstInstance = orxBank_GetNext(_pstStateMachine->pstInstancesBank, orxNULL);
  while(pstInstance != orxNULL && eStatus == orxSTATUS_SUCCESS)
  {
    eStatus = orxFSM_UpdateInstance(pstInstance);
  }
  
  return eStatus;
}
