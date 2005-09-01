/**
 * @file orxTest_FSM.c
 * 
 * State Machine Test Program
 * 
 */
 
 /***************************************************************************
 orxTest_FSM.c
 Hash table Test Program
 
 begin                : 31/08/2005
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

#include "orxInclude.h"
#include "utils/orxTest.h"
#include "utils/orxFSM.h"
#include "io/orxTextIO.h"

/******************************************************
 * DEFINES
 ******************************************************/

#define orxTEST_FSM_KU32_ARRAY_NB_ELEM  3
#define orxTEST_FSM_KU32_ARRAY_NB_INST  3

/******************************************************
 * TYPES AND STRUCTURES
 ******************************************************/

typedef struct __orxTEST_FSM_t
{
  orxFSM *apstFSM[orxTEST_FSM_KU32_ARRAY_NB_ELEM];                    /* Array of FSM. */
  orxU32 u32NbUsedFSM;                                                /* Number of used FSM. */
} orxTEST_FSM;

orxSTATIC orxTEST_FSM sstTest_FSM;

typedef struct __orxTEST_FSM_INST_t
{
  orxFSM_INSTANCE *apstFSM_Inst[orxTEST_FSM_KU32_ARRAY_NB_INST];      /* Array of FSM instances. */
  orxU32 u32NbUsedFSMInst;                                            /* Number of used FSM instances. */
} orxTEST_FSM_INST;

orxSTATIC orxTEST_FSM_INST sstTest_FSM_Inst;

/******************************************************
 * PRIVATE FUNCTIONS
 ******************************************************/

orxVOID orxTest_FSM_PrintUsedID()
{
  orxU32 u32Index;
  
  orxTextIO_PrintLn("List of used FSM ID:");

  /* Tranverse the array and get used ID. */
  for (u32Index = 0; u32Index < orxTEST_FSM_KU32_ARRAY_NB_ELEM; u32Index++)
  {
    if (sstTest_FSM.apstFSM[u32Index] != orxNULL)
    {
      orxTextIO_PrintLn("Used ID: %d", u32Index);
    }
  }
}

orxVOID orxTest_FSM_Inst_PrintUsedIt()
{
  orxU32 u32Index;
  
  orxTextIO_PrintLn("List of used FSM instances ID:");

  /* Tranverse the array and get used ID. */
  for (u32Index = 0; u32Index < orxTEST_FSM_KU32_ARRAY_NB_INST; u32Index++)
  {
    if (sstTest_FSM_Inst.apstFSM_Inst[u32Index] != orxNULL)
    {
      orxTextIO_PrintLn("Used ID: %d", u32Index);
    }
  }
}

orxVOID orxTest_FSM_InitAction()
{
    orxTextIO_PrintLn("Init.");
}

orxVOID orxTest_FSM_ExecuteAction()
{
    orxTextIO_PrintLn("Execute.");
}

orxVOID orxTest_FSM_ExitAction()
{
    orxTextIO_PrintLn("Exit.");
}

orxBOOL orxTest_FSM_Condition()
{
    return orxTRUE;
}

/******************************************************
 * TEST FUNCTIONS
 ******************************************************/

/** Display informations about this test module. */
orxVOID orxTest_FSM_Infos()
{
  orxTextIO_PrintLn("This test module is able to manage %lu FSM and %lu instances.", orxTEST_FSM_KU32_ARRAY_NB_ELEM, orxTEST_FSM_KU32_ARRAY_NB_INST);
  orxTextIO_PrintLn("* When you create a FSM, you can select an index between 0 and %d to set the FSM to use.", orxTEST_FSM_KU32_ARRAY_NB_ELEM - 1);
  orxTextIO_PrintLn("* When you have created a FSM, you can add/remove/get new states and links for it.");
  orxTextIO_PrintLn("* When you create an instance, you can select an index between 0 and %d to set the instance to use.", orxTEST_FSM_KU32_ARRAY_NB_INST - 1);
  orxTextIO_PrintLn("* When you have created an instance, you can update it from the current state.");
}

/** Create a FSM. */
orxVOID orxTest_FSM_Create()
{
  orxS32 s32ID;
  orxS32 s32NbStates, s32NbLinks, s32NbInstances;
  
  /* Is it possible to create a FSM? */
  if (sstTest_FSM.u32NbUsedFSM == orxTEST_FSM_KU32_ARRAY_NB_ELEM)
  {
    orxTextIO_PrintLn("All ID have been used. Delete a FSM before create a new one.");
    return;
  }
  
  /* All ID available? */
  if (sstTest_FSM.u32NbUsedFSM > 0)
  {
    /* Display the list of used ID. */
    orxTest_FSM_PrintUsedID();
  }

  /* Get the ID to use. */
  orxTextIO_ReadS32InRange(&s32ID, 10, 0, orxTEST_FSM_KU32_ARRAY_NB_ELEM - 1, "Choose the FSM ID to use: ", orxTRUE);

  /* Already used ID? */
  if (sstTest_FSM.apstFSM[s32ID] != orxNULL)
  {
    orxTextIO_PrintLn("This ID is already used.");
  }
  else
  {
    /* Get the number of states. */
    orxTextIO_ReadS32InRange(&s32NbStates, 10, 1, 0x7FFFFFFF, "Number of states: ", orxTRUE);
    
    /* Get the number of links. */
    orxTextIO_ReadS32InRange(&s32NbLinks, 10, 1, 0x7FFFFFFF, "Number of links: ", orxTRUE);
    
    /* Get the number of instances. */
    orxTextIO_ReadS32InRange(&s32NbInstances, 10, 1, 0x7FFFFFFF, "Number of instances: ", orxTRUE);
    
    /* Now, allocate s32NbStates states and s32NbLinks links in a new FSM at the index position s32ID. */
    sstTest_FSM.apstFSM[s32ID] = orxFSM_Create(s32NbStates, s32NbLinks, s32NbInstances, orxFSM_KU32_FLAGS_NONE, orxMEMORY_TYPE_MAIN);
    
    if (sstTest_FSM.apstFSM[s32ID] == orxNULL)
    {
      orxTextIO_PrintLn("Can't create the FSM. Not enough memory?");
    }
    else
    {
      orxTextIO_PrintLn("New FSM created with %d states and %d links at the ID %d", s32NbStates, s32NbLinks, s32ID);
    
      /* Increase the counter. */
      sstTest_FSM.u32NbUsedFSM++;
    }
  }
}

/** Destroy a FSM. */
orxVOID orxTest_FSM_Destroy()
{
  orxS32 s32ID;
  
  /* Are there allocated FSM? */
  if (sstTest_FSM.u32NbUsedFSM == 0)
  {
    orxTextIO_PrintLn("No FSM have been created. Create a FSM before trying to delete it.");
    return;
  }
  
  /* Display the list of allocated FSM. */
  orxTest_FSM_PrintUsedID();
  
  /* Get the ID to use. */
  orxTextIO_ReadS32InRange(&s32ID, 10, 0, orxTEST_FSM_KU32_ARRAY_NB_ELEM - 1, "Choose the FSM ID to use: ", orxTRUE);
  
  /* Not used ID? */
  if (sstTest_FSM.apstFSM[s32ID] == orxNULL)
  {
    orxTextIO_PrintLn("This ID is not used, can't destroy it");
  }
  else
  {
    /* Delete the FSM. */
    orxFSM_Delete(sstTest_FSM.apstFSM[s32ID]);
    sstTest_FSM.apstFSM[s32ID] = orxNULL;
    
    /* Decrease the counter. */
    sstTest_FSM.u32NbUsedFSM--;
  
    /* Done! */
    orxTextIO_PrintLn("FSM deleted!");
  }
}

/** Add a new state. */
orxVOID orxTest_FSM_State_Add()
{
  orxS32 s32ID;
  orxS32 s32StateId;
  
  /* Are there allocated FSM? */
  if (sstTest_FSM.u32NbUsedFSM == 0)
  {
    orxTextIO_PrintLn("No FSM have been created. Create a FSM before trying to add states.");
    return;
  }
  
  /* Display the list of FSM. */
  orxTest_FSM_PrintUsedID();
  
  /* Get the ID to use */
  orxTextIO_ReadS32InRange(&s32ID, 10, 0, orxTEST_FSM_KU32_ARRAY_NB_ELEM - 1, "Choose the FSM ID to use: ", orxTRUE);
  
  /* Not used ID? */
  if (sstTest_FSM.apstFSM[s32ID] == orxNULL)
  {
    orxTextIO_PrintLn("This ID is not used.");
  }
  else
  {
    /* Get identifier of the state to add. */
    orxTextIO_ReadS32(&s32StateId, 10, "Identifier for the state to add: ", orxTRUE);
    
    orxTextIO_PrintLn("Trying to add the state %u...", (orxU32)s32StateId);
    if (orxFSM_State_Add(sstTest_FSM.apstFSM[s32ID], (orxU32)s32StateId, orxTest_FSM_InitAction, orxTest_FSM_ExecuteAction, orxTest_FSM_ExitAction) == orxNULL)
    {
      /* Insertion failed. */
      orxTextIO_PrintLn("Insertion failed...");
    }

    orxTextIO_PrintLn("Done!");
  }
}

/** Set an initial state. */
orxVOID orxTest_FSM_State_Initial()
{
  orxS32 s32ID;
  orxS32 s32InitialStateId;
  orxFSM_STATE * pstInitialState;
  
  /* Are there allocated FSM? */
  if (sstTest_FSM.u32NbUsedFSM == 0)
  {
    orxTextIO_PrintLn("No FSM have been created. Create a FSM before trying to set initial state.");
    return;
  }
  
  /* Display the list of FSM. */
  orxTest_FSM_PrintUsedID();
  
  /* Get the ID to use */
  orxTextIO_ReadS32InRange(&s32ID, 10, 0, orxTEST_FSM_KU32_ARRAY_NB_ELEM - 1, "Choose the FSM ID to use: ", orxTRUE);
  
  /* Not used ID? */
  if (sstTest_FSM.apstFSM[s32ID] == orxNULL)
  {
    orxTextIO_PrintLn("This ID is not used.");
  }
  else
  {
    /* Get identifier of the initial state. */
    orxTextIO_ReadS32(&s32InitialStateId, 10, "Identifier for the initial state: ", orxTRUE);
    
    orxTextIO_PrintLn("Trying to add the initial state %u...", (orxU32)s32InitialStateId);
    pstInitialState = orxFSM_State_Get(sstTest_FSM.apstFSM[s32ID], (orxU32)s32InitialStateId);
    if (orxFSM_State_Initial(sstTest_FSM.apstFSM[s32ID], pstInitialState) == orxSTATUS_FAILED)
    {
      /* Insertion failed. */
      orxTextIO_PrintLn("Set of initial state failed...");
    }

    orxTextIO_PrintLn("Done!");
  }
}

/** Remove a state from the FSM with related links. */
orxVOID orxTest_FSM_State_Remove()
{
  orxS32 s32ID;
  orxS32 s32StateId;
  orxFSM_STATE * pstState;
  
  /* Are there allocated FSM? */
  if (sstTest_FSM.u32NbUsedFSM == 0)
  {
    orxTextIO_PrintLn("No FSM have been created. You can't remove states.");
    return;
  }
  
  /* Display the list of allocated FSM. */
  orxTest_FSM_PrintUsedID();
  
  /* Get the ID to use. */
  orxTextIO_ReadS32InRange(&s32ID, 10, 0, orxTEST_FSM_KU32_ARRAY_NB_ELEM - 1, "Choose the FSM ID to use: ", orxTRUE);
  
  /* Not used ID? */
  if (sstTest_FSM.apstFSM[s32ID] == orxNULL)
  {
    orxTextIO_PrintLn("This ID is not used.");
  }
  else
  {
    /* Get the state to remove. */
    orxTextIO_ReadS32(&s32StateId, 10, "State to remove: ", orxTRUE);

    /* Try to remove it. */
    pstState = orxFSM_State_Get(sstTest_FSM.apstFSM[s32ID], (orxU32)s32StateId);
    if (orxFSM_State_Remove(sstTest_FSM.apstFSM[s32ID], pstState, orxTRUE) == orxSTATUS_FAILED)
    {
      /* Failed to remove. */
      orxTextIO_PrintLn("Remove failed...");
    }
  }
}

/** Add a new link. */
orxVOID orxTest_FSM_Link_Add()
{
  orxS32 s32ID;
  orxS32 s32BeginningStateId, s32EndingStateId;
  orxFSM_STATE * pstBeginningState, * pstEndingState;
  
  /* Are there allocated FSM? */
  if (sstTest_FSM.u32NbUsedFSM == 0)
  {
    orxTextIO_PrintLn("No FSM have been created. Create a FSM before trying to add links.");
    return;
  }
  
  /* Display the list of FSM. */
  orxTest_FSM_PrintUsedID();
  
  /* Get the ID to use */
  orxTextIO_ReadS32InRange(&s32ID, 10, 0, orxTEST_FSM_KU32_ARRAY_NB_ELEM - 1, "Choose the FSM ID to use: ", orxTRUE);
  
  /* Not used ID? */
  if (sstTest_FSM.apstFSM[s32ID] == orxNULL)
  {
    orxTextIO_PrintLn("This ID is not used.");
  }
  else
  {
    /* Get identifier of the beginning state for the link. */
    orxTextIO_ReadS32(&s32BeginningStateId, 10, "Identifier of the beginning state: ", orxTRUE);
    
    /* Get identifier of the ending state for the link. */
    orxTextIO_ReadS32(&s32EndingStateId, 10, "Identifier of the ending state: ", orxTRUE);
    
    orxTextIO_PrintLn("Trying to add the link...");
    pstBeginningState = orxFSM_State_Get(sstTest_FSM.apstFSM[s32ID], (orxU32)s32BeginningStateId);
    pstEndingState = orxFSM_State_Get(sstTest_FSM.apstFSM[s32ID], (orxU32)s32EndingStateId);
    if (orxFSM_Link_Add(sstTest_FSM.apstFSM[s32ID], pstBeginningState, pstEndingState, orxTest_FSM_Condition) == orxNULL)
    {
      /* Insertion failed */
      orxTextIO_PrintLn("Insertion failed...");
    }
    
    orxTextIO_PrintLn("Done!");
  }
}

/** Remove a link from the FSM. */
orxVOID orxTest_FSM_Link_Remove()
{
  orxS32 s32ID;
  orxS32 s32BeginningStateId, s32EndingStateId;
  orxFSM_STATE * pstBeginningState, * pstEndingState;
  orxFSM_LINK * pstLink;
  
  /* Are there allocated FSM? */
  if (sstTest_FSM.u32NbUsedFSM == 0)
  {
    orxTextIO_PrintLn("No FSM have been created. You can't remove links.");
    return;
  }
  
  /* Display the list of allocated FSM. */
  orxTest_FSM_PrintUsedID();
  
  /* Get the ID to use. */
  orxTextIO_ReadS32InRange(&s32ID, 10, 0, orxTEST_FSM_KU32_ARRAY_NB_ELEM - 1, "Choose the FSM ID to use: ", orxTRUE);
  
  /* Not used ID? */
  if (sstTest_FSM.apstFSM[s32ID] == orxNULL)
  {
    orxTextIO_PrintLn("This ID is not used.");
  }
  else
  {
    /* Get identifier of the beginning state for the link. */
    orxTextIO_ReadS32(&s32BeginningStateId, 10, "Identifier of the beginning state: ", orxTRUE);
    
    /* Get identifier of the ending state for the link. */
    orxTextIO_ReadS32(&s32EndingStateId, 10, "Identifier of the ending state: ", orxTRUE);

    /* Try to remove it. */
    orxTextIO_PrintLn("Trying to remove the link...");
    pstBeginningState = orxFSM_State_Get(sstTest_FSM.apstFSM[s32ID], (orxU32)s32BeginningStateId);
    pstEndingState = orxFSM_State_Get(sstTest_FSM.apstFSM[s32ID], (orxU32)s32EndingStateId);
    pstLink = orxFSM_Link_Get(sstTest_FSM.apstFSM[s32ID], pstBeginningState, pstEndingState);
    if (orxFSM_Link_Remove(sstTest_FSM.apstFSM[s32ID], pstLink) == orxSTATUS_FAILED)
    {
      /* Failed to remove. */
      orxTextIO_PrintLn("Remove failed...");
    }
  }
}

/** Clear all links from a FSM. */
orxVOID orxTest_FSM_Link_Clear()
{
  orxS32 s32ID;
  
  /* Are there allocated FSM? */
  if (sstTest_FSM.u32NbUsedFSM == 0)
  {
    orxTextIO_PrintLn("No FSM have been created. You can't clear links.");
    return;
  }
  
  /* Display the list of allocated FSM. */
  orxTest_FSM_PrintUsedID();
  
  /* Get the ID to use */
  orxTextIO_ReadS32InRange(&s32ID, 10, 0, orxTEST_FSM_KU32_ARRAY_NB_ELEM - 1, "Choose the FSM ID to use: ", orxTRUE);
  
  /* Not used ID? */
  if (sstTest_FSM.apstFSM[s32ID] == orxNULL)
  {
    orxTextIO_PrintLn("This ID is not used.");
  }
  else
  {
    /* Clear the FSM. */
    orxTextIO_PrintLn("Clear links from FSM...");
    orxFSM_Link_Clear(sstTest_FSM.apstFSM[s32ID]);
  }
}

/** Clear all states and links from a FSM. */
orxVOID orxTest_FSM_Clear()
{
  orxS32 s32ID;
  
  /* Are there allocated FSM? */
  if (sstTest_FSM.u32NbUsedFSM == 0)
  {
    orxTextIO_PrintLn("No FSM have been created. you can't clear it.");
    return;
  }
  
  /* Display the list of allocated FSM. */
  orxTest_FSM_PrintUsedID();
  
  /* Get the ID to use */
  orxTextIO_ReadS32InRange(&s32ID, 10, 0, orxTEST_FSM_KU32_ARRAY_NB_ELEM - 1, "Choose the FSM ID to use: ", orxTRUE);
  
  /* Not used ID? */
  if (sstTest_FSM.apstFSM[s32ID] == orxNULL)
  {
    orxTextIO_PrintLn("This ID is not used.");
  }
  else
  {
    /* Clear the FSM. */
    orxTextIO_PrintLn("Clear FSM...");
    orxFSM_Clear(sstTest_FSM.apstFSM[s32ID]);
  }
}

/** Create an instance of a FSM. */
orxVOID orxTest_FSM_Instance_Create()
{
  orxS32 s32ID;
  orxS32 s32FSMId;
  orxFSM * pstFSM;
  
  /* Is it possible to create an instance? */
  if (sstTest_FSM_Inst.u32NbUsedFSMInst == orxTEST_FSM_KU32_ARRAY_NB_INST)
  {
    orxTextIO_PrintLn("All ID have been used. Delete an instance before create a new one.");
    return;
  }
  
  /* All ID available? */
  if (sstTest_FSM_Inst.u32NbUsedFSMInst > 0)
  {
    /* Display the list of used ID. */
    orxTest_FSM_Inst_PrintUsedIt();
  }

  /* Get the ID to use. */
  orxTextIO_ReadS32InRange(&s32ID, 10, 0, orxTEST_FSM_KU32_ARRAY_NB_INST - 1, "Choose the instance ID to use: ", orxTRUE);

  /* Already used ID? */
  if (sstTest_FSM_Inst.apstFSM_Inst[s32ID] != orxNULL)
  {
    orxTextIO_PrintLn("This ID is already used.");
  }
  else
  {
    /* Get the ID of the FSM. */
    orxTextIO_ReadS32InRange(&s32FSMId, 10, 0, orxTEST_FSM_KU32_ARRAY_NB_ELEM, "FSM ID: ", orxTRUE);

    /* Already used ID? */
    pstFSM = sstTest_FSM.apstFSM[s32FSMId];
    if (pstFSM != orxNULL)
    {
      /* Now, allocate an instance of the FSM at the index position s32ID. */
      sstTest_FSM_Inst.apstFSM_Inst[s32ID] = orxFSM_Instance_Create(pstFSM);
    
      if (sstTest_FSM_Inst.apstFSM_Inst[s32ID] == orxNULL)
      {
        orxTextIO_PrintLn("Can't create the instance. Not enough memory?");
      }
      else
      {
        orxTextIO_PrintLn("New instance created.");
    
        /* Increase the counter. */
        sstTest_FSM_Inst.u32NbUsedFSMInst++;
      }
    }
    else
    {
      orxTextIO_PrintLn("This ID is not used.");
    }
  }
}

/** Destroy an instance of a FSM. */
orxVOID orxTest_FSM_Instance_Destroy()
{
  orxS32 s32ID;
  
  /* Are there allocated instances? */
  if (sstTest_FSM_Inst.u32NbUsedFSMInst == 0)
  {
    orxTextIO_PrintLn("No instances have been created. Create an instance before trying to delete it.");
    return;
  }
  
  /* Display the list of allocated instances. */
  orxTest_FSM_Inst_PrintUsedIt();
  
  /* Get the ID to use. */
  orxTextIO_ReadS32InRange(&s32ID, 10, 0, orxTEST_FSM_KU32_ARRAY_NB_INST - 1, "Choose the instance ID to use: ", orxTRUE);
  
  /* Not used ID? */
  if (sstTest_FSM_Inst.apstFSM_Inst[s32ID] == orxNULL)
  {
    orxTextIO_PrintLn("This ID is not used, can't destroy it");
  }
  else
  {
    /* Delete the instance. */
    orxFSM_Instance_Remove(sstTest_FSM_Inst.apstFSM_Inst[s32ID]);
    sstTest_FSM_Inst.apstFSM_Inst[s32ID] = orxNULL;
    
    /* Decrease the counter. */
    sstTest_FSM_Inst.u32NbUsedFSMInst--;
  
    /* Done! */
    orxTextIO_PrintLn("Instance deleted!");
  }
}

/** Update an instance of a FSM. */
orxVOID orxTest_FSM_Instance_Update()
{
  orxS32 s32ID;
  
  /* Are there allocated instances? */
  if (sstTest_FSM_Inst.u32NbUsedFSMInst == 0)
  {
    orxTextIO_PrintLn("No instances have been created. Create an instance before trying to update it.");
    return;
  }
  
  /* Display the list of allocated instances. */
  orxTest_FSM_Inst_PrintUsedIt();
  
  /* Get the ID to use. */
  orxTextIO_ReadS32InRange(&s32ID, 10, 0, orxTEST_FSM_KU32_ARRAY_NB_INST - 1, "Choose the instance ID to use: ", orxTRUE);
  
  /* Not used ID? */
  if (sstTest_FSM_Inst.apstFSM_Inst[s32ID] == orxNULL)
  {
    orxTextIO_PrintLn("This ID is not used, can't destroy it");
  }
  else
  {
    if (orxFSM_Instance_GetCurrentState(sstTest_FSM_Inst.apstFSM_Inst[s32ID]) != orxNULL)
      orxTextIO_PrintLn("Current state: %u", (orxU16)orxFSM_State_GetId(orxFSM_Instance_GetFSM(sstTest_FSM_Inst.apstFSM_Inst[s32ID]), orxFSM_Instance_GetCurrentState(sstTest_FSM_Inst.apstFSM_Inst[s32ID])));
    
    /* Update the instance. */
    if (orxFSM_Instance_Update(sstTest_FSM_Inst.apstFSM_Inst[s32ID]) == orxSTATUS_FAILED)
    {
      orxTextIO_PrintLn("Update failed...");
    }
    else
    {
      orxTextIO_PrintLn("New state: %u", (orxU16)orxFSM_State_GetId(orxFSM_Instance_GetFSM(sstTest_FSM_Inst.apstFSM_Inst[s32ID]), orxFSM_Instance_GetCurrentState(sstTest_FSM_Inst.apstFSM_Inst[s32ID])));
      orxTextIO_PrintLn("Update done!");
    }
  }
}

/** Update all instances of a FSM. */
orxVOID orxTest_FSM_Update()
{
  orxS32 s32ID;
  
  /* Are there allocated FSM? */
  if (sstTest_FSM.u32NbUsedFSM == 0)
  {
    orxTextIO_PrintLn("No FSM have been created. Create a FSM before trying to update its instances.");
    return;
  }
  
  /* Display the list of allocated FSM. */
  orxTest_FSM_PrintUsedID();
  
  /* Get the ID to use. */
  orxTextIO_ReadS32InRange(&s32ID, 10, 0, orxTEST_FSM_KU32_ARRAY_NB_ELEM - 1, "Choose the FSM ID to use: ", orxTRUE);
  
  /* Not used ID? */
  if (sstTest_FSM.apstFSM[s32ID] == orxNULL)
  {
    orxTextIO_PrintLn("This ID is not used, can't update its instances.");
  }
  else
  {
    /* Update all instances of the FSM. */
    if (orxFSM_Update(sstTest_FSM.apstFSM[s32ID]) == orxSTATUS_FAILED)
    {
      orxTextIO_PrintLn("Update of all instances failed...");
    }
    
    orxTextIO_PrintLn("Update of all instances done!");
  }
}

/******************************************************
 * DYNAMIC LIBRARY ENTRY POINTS
 ******************************************************/
orxVOID orxTest_FSM_Init()
{
  /* Initialize bank module */
  orxFSM_Init();
  
  /* Register test functions */
  orxTest_Register("FSM", "Display module informations", orxTest_FSM_Infos);
  orxTest_Register("FSM", "Create a FSM", orxTest_FSM_Create);
  orxTest_Register("FSM", "Delete a FSM", orxTest_FSM_Destroy);
  orxTest_Register("FSM", "Add a state", orxTest_FSM_State_Add);
  orxTest_Register("FSM", "Set initial state", orxTest_FSM_State_Initial);
  orxTest_Register("FSM", "Remove a state with related links", orxTest_FSM_State_Remove);
  orxTest_Register("FSM", "Add a link", orxTest_FSM_Link_Add);
  orxTest_Register("FSM", "Remove a link", orxTest_FSM_Link_Remove);
  orxTest_Register("FSM", "Clear all links", orxTest_FSM_Link_Clear);
  orxTest_Register("FSM", "Remove all states and links from a FSM", orxTest_FSM_Clear);
  orxTest_Register("FSM", "Create an instance of a FSM", orxTest_FSM_Instance_Create);
  orxTest_Register("FSM", "Delete an instance of a FSM", orxTest_FSM_Instance_Create);
  orxTest_Register("FSM", "Update an instance of a FSM", orxTest_FSM_Instance_Update);
  orxTest_Register("FSM", "Update all instances of a FSM", orxTest_FSM_Update);
  
  /* Initialize static datas */
  orxMemory_Set(&sstTest_FSM, 0, sizeof(orxTEST_FSM));
}

orxVOID orxTest_FSM_Exit()
{
  orxU32 u32FSMIndex; /* Index of the current FSM. */
  
  /* Traverse allocated FSM and free them. */
  for (u32FSMIndex = 0; u32FSMIndex < orxTEST_FSM_KU32_ARRAY_NB_ELEM; u32FSMIndex++)
  {
    /* FSM allocated? */
    if (sstTest_FSM.apstFSM[u32FSMIndex] != orxNULL)
    {
      /* Delete it. */
      orxFSM_Delete(sstTest_FSM.apstFSM[u32FSMIndex]);
      sstTest_FSM.apstFSM[u32FSMIndex] = orxNULL;
    }
  }
  
  /* Uninitialize module. */
  orxFSM_Exit();
}

orxTEST_DEFINE_ENTRY_POINT(orxTest_FSM_Init, orxTest_FSM_Exit)
