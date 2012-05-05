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
 * @file orxCommand.c
 * @date 29/04/2012
 * @author iarwain@orx-project.org
 *
 */


#include "core/orxCommand.h"

#include "debug/orxDebug.h"
#include "core/orxConfig.h"
#include "core/orxEvent.h"
#include "memory/orxMemory.h"
#include "memory/orxBank.h"
#include "object/orxTimeLine.h"
#include "utils/orxHashTable.h"
#include "utils/orxString.h"

#ifdef __orxMSVC__

  #include "malloc.h"

#endif /* __orxMSVC__ */


/** Module flags
 */
#define orxCOMMAND_KU32_STATIC_FLAG_NONE              0x00000000  /**< No flags */

#define orxCOMMAND_KU32_STATIC_FLAG_READY             0x00000001  /**< Ready flag */
#define orxCOMMAND_KU32_STATIC_FLAG_INTERNAL_CALL     0x10000000  /** <Internal call flag */

#define orxCOMMAND_KU32_STATIC_MASK_ALL               0xFFFFFFFF  /**< All mask */


/** Misc
 */
#define orxCOMMAND_KZ_CONFIG_SECTION                  "-=orxCommand=-"
#define orxCOMMAND_KZ_CONFIG_STACK_PREFIX             "Stack"

#define orxCOMMAND_KC_STRING_MARKER                   '"'         /**< String marker character */
#define orxCOMMAND_KC_PUSH_MARKER                     '>'         /**< Push marker character */
#define orxCOMMAND_KC_POP_MARKER                      '<'         /**< Pop marker character */
#define orxCOMMAND_KC_GUID_MARKER                     '-'         /**< GUID marker character */


#define orxCOMMAND_KU32_TABLE_SIZE                    256
#define orxCOMMAND_KU32_BANK_SIZE                     128

#define orxCOMMAND_KU32_BUFFER_SIZE                   4096


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Command structure
 */
typedef struct __orxCOMMAND_t
{
  orxCOMMAND_FUNCTION       pfnFunction;              /**< Function : 4 */
  orxCOMMAND_VAR_DEF        stResult;                 /**< Result definition : 12 */
  orxSTRING                 zName;                    /**< Name : 16 */
  orxU32                    u32ParamNumber;           /**< Param number : 20 */
  orxCOMMAND_VAR_DEF       *astParamList;             /**< Param list : 24 */

} orxCOMMAND;

/** Static structure
 */
typedef struct __orxCOMMAND_STATIC_t
{
  orxBANK                  *pstBank;                  /**< Command bank */
  orxHASHTABLE             *pstTable;                 /**< Command table */
  orxS32                    s32StackIndex;            /**< Stack index */
  orxCHAR                   acBuffer[orxCOMMAND_KU32_BUFFER_SIZE]; /**< Evaluate buffer */
  orxU32                    u32Flags;                 /**< Control flags */

} orxCOMMAND_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxCOMMAND_STATIC sstCommand;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Event handler
 * @param[in]   _pstEvent                     Sent event
 * @return      orxSTATUS_SUCCESS if handled / orxSTATUS_FAILURE otherwise
 */
static orxSTATUS orxFASTCALL orxCommand_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(_pstEvent->eType == orxEVENT_TYPE_TIMELINE);

  /* Depending on event ID */
  switch(_pstEvent->eID)
  {
    /* Trigger */
    case orxTIMELINE_EVENT_TRIGGER:
    {
      orxCOMMAND_VAR              stResult;
      orxTIMELINE_EVENT_PAYLOAD  *pstPayload;
      const orxCHAR              *pcSrc;
      orxCHAR                    *pcDst;
      orxU32                      u32PushCounter;
      orxS32                      s32GUIDLength;
      orxCHAR                     acGUID[20];

      /* Pushes config section */
      orxConfig_PushSection(orxCOMMAND_KZ_CONFIG_SECTION);

      /* Gets owner's GUID */
      s32GUIDLength = orxString_NPrint(acGUID, 20, "0x%llx", orxStructure_GetGUID(orxSTRUCTURE(_pstEvent->hSender)));

      /* Gets payload */
      pstPayload = (orxTIMELINE_EVENT_PAYLOAD *)_pstEvent->pstPayload;

      /* For all push markers / spaces */
      for(pcSrc = pstPayload->zEvent, u32PushCounter = 0; (*pcSrc == orxCOMMAND_KC_PUSH_MARKER) || (*pcSrc == ' ') || (*pcSrc == '\t'); pcSrc++)
      {
        /* Is a push marker? */
        if(*pcSrc == orxCOMMAND_KC_PUSH_MARKER)
        {
          /* Updates push counter */
          u32PushCounter++;
        }
      }

      /* For all characters */
      for(pcDst = sstCommand.acBuffer; (*pcSrc != orxCHAR_NULL) && (pcDst - sstCommand.acBuffer < orxCOMMAND_KU32_BUFFER_SIZE - 1);)
      {
        /* Depending on character */
        switch(*pcSrc)
        {
          case orxCOMMAND_KC_GUID_MARKER:
          {
            /* Replaces it with GUID */
            orxString_Copy(pcDst, acGUID);

            /* Updates pointers */
            pcSrc++;
            pcDst += s32GUIDLength;

            break;
          }

          case orxCOMMAND_KC_POP_MARKER:
          {
            const orxSTRING zValue;
            orxCHAR         acStackKey[16];

            /* Sets key name */
            orxString_NPrint(acStackKey, 16, orxCOMMAND_KZ_CONFIG_STACK_PREFIX"%ld", --sstCommand.s32StackIndex);

            /* Checks */
            orxASSERT(sstCommand.s32StackIndex >= 0);

            /* Gets stacked value */
            zValue = orxConfig_GetString(acStackKey);

            /* Replaces it with GUID */
            orxString_Copy(pcDst, zValue);

            /* Updates pointers */
            pcSrc++;
            pcDst += orxString_GetLength(zValue);

            break;
          }

          default:
          {
            /* Copies it */
            *pcDst++ = *pcSrc++;

            break;
          }
        }
      }

      /* Copies end of string */
      *pcDst = orxCHAR_NULL;

      /* Updates internal status */
      orxFLAG_SET(sstCommand.u32Flags, orxCOMMAND_KU32_STATIC_FLAG_INTERNAL_CALL, orxCOMMAND_KU32_STATIC_FLAG_NONE);

      /* Evaluates command */
      if(orxCommand_Evaluate(sstCommand.acBuffer, &stResult) != orxNULL)
      {
        /* For all requested pushes */
        while(u32PushCounter > 0)
        {
          orxCHAR acStackKey[16];

          /* Sets key name */
          orxString_NPrint(acStackKey, 16, orxCOMMAND_KZ_CONFIG_STACK_PREFIX"%ld", sstCommand.s32StackIndex++);

          /* Depending on result type */
          switch(stResult.eType)
          {
            default:
            case orxCOMMAND_VAR_TYPE_STRING:
            {
              /* Stores it */
              orxConfig_SetString(acStackKey, stResult.zValue);

              break;
            }

            case orxCOMMAND_VAR_TYPE_FLOAT:
            {
              /* Stores it */
              orxConfig_SetFloat(acStackKey, stResult.fValue);

              break;
            }

            case orxCOMMAND_VAR_TYPE_S32:
            {
              /* Stores it */
              orxConfig_SetS32(acStackKey, stResult.s32Value);

              break;
            }

            case orxCOMMAND_VAR_TYPE_U32:
            {
              /* Stores it */
              orxConfig_SetU32(acStackKey, stResult.u32Value);

              break;
            }

            case orxCOMMAND_VAR_TYPE_S64:
            {
              /* Stores it */
              orxConfig_SetS64(acStackKey, stResult.s64Value);

              break;
            }

            case orxCOMMAND_VAR_TYPE_U64:
            {
              /* Stores it */
              orxConfig_SetU64(acStackKey, stResult.u64Value);

              break;
            }

            case orxCOMMAND_VAR_TYPE_BOOL:
            {
              /* Stores it */
              orxConfig_SetBool(acStackKey, stResult.bValue);

              break;
            }

            case orxCOMMAND_VAR_TYPE_VECTOR:
            {
              /* Stores it */
              orxConfig_SetVector(acStackKey, &(stResult.vValue));

              break;
            }
          }

          /* Updates push counter */
          u32PushCounter--;
        }
      }

      /* Updates internal status */
      orxFLAG_SET(sstCommand.u32Flags, orxCOMMAND_KU32_STATIC_FLAG_NONE, orxCOMMAND_KU32_STATIC_FLAG_INTERNAL_CALL);

      /* Pops config section */
      orxConfig_PopSection();

      break;
    }

    default:
    {
      break;
    }
  }

  /* Done! */
  return eResult;
}

/** Runs a command
 * @param[in]   _zCommand      Command name
 * @param[in]   _bCheckArgList Check list of arguments?
 * @param[in]   _u32ArgNumber  Number of arguments sent to the command
 * @param[in]   _astArgList    List of arguments sent to the command
 * @param[out]  _pstResult     Variable that will contain the result
 * @return      Command result if found, orxNULL otherwise
 */
static orxINLINE orxCOMMAND_VAR *orxCommand_Run(const orxCOMMAND *_pstCommand, orxBOOL _bCheckArgList, orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCOMMAND_VAR *pstResult = orxNULL;

  /* Valid number of arguments? */
  if(_u32ArgNumber == _pstCommand->u32ParamNumber)
  {
    orxU32 i;

    /* Checks ? */
    if(_bCheckArgList != orxFALSE)
    {
      /* For all arguments */
      for(i = 0; i < _u32ArgNumber; i++)
      {
        /* Invalid? */
        if(_astArgList[i].eType != _pstCommand->astParamList[i].eType)
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Can't execute command [%s]: invalid type for argument #%ld (%s).", _pstCommand->zName, i + 1, _pstCommand->astParamList[i].zName);

          /* Stops */
          break;
        }
      }
    }
    else
    {
      /* Validates it */
      i = _u32ArgNumber;
    }

    /* Valid? */
    if(i == _u32ArgNumber)
    {
      /* Inits result */
      _pstResult->eType = _pstCommand->stResult.eType;

      /* Runs command */
      _pstCommand->pfnFunction(_u32ArgNumber, _astArgList, _pstResult);

      /* Updates result */
      pstResult = (_pstResult->eType != orxCOMMAND_VAR_TYPE_NONE) ? _pstResult : orxNULL;
    }
  }

  /* Done! */
  return pstResult;
}

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Command module setup
 */
void orxFASTCALL orxCommand_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_COMMAND, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_COMMAND, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_COMMAND, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_COMMAND, orxMODULE_ID_EVENT);

  /* Done! */
  return;
}

/** Inits command module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxCommand_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstCommand.u32Flags & orxCOMMAND_KU32_STATIC_FLAG_READY))
  {
    /* Cleans control structure */
    orxMemory_Zero(&sstCommand, sizeof(orxCOMMAND_STATIC));

    /* Registers event handler */
    eResult = orxEvent_AddHandler(orxEVENT_TYPE_TIMELINE, orxCommand_EventHandler);

    /* Valid? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Creates bank */
      sstCommand.pstBank = orxBank_Create(orxCOMMAND_KU32_BANK_SIZE, sizeof(orxCOMMAND), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

      /* Valid? */
      if(sstCommand.pstBank != orxNULL)
      {
        /* Creates table */
        sstCommand.pstTable = orxHashTable_Create(orxCOMMAND_KU32_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

        /* Valid? */
        if(sstCommand.pstTable != orxNULL)
        {
          /* Inits Flags */
          sstCommand.u32Flags = orxCOMMAND_KU32_STATIC_FLAG_READY;

          /* Updates result */
          eResult = orxSTATUS_SUCCESS;
        }
        else
        {
          /* Removes event handler */
          orxEvent_RemoveHandler(orxEVENT_TYPE_TIMELINE, orxCommand_EventHandler);

          /* Deletes bank */
          orxBank_Delete(sstCommand.pstBank);

          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Failed to create command table.");
        }
      }
      else
      {
        /* Removes event handler */
        orxEvent_RemoveHandler(orxEVENT_TYPE_TIMELINE, orxCommand_EventHandler);

        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Failed to create command bank.");
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Failed to register event handler.");
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Tried to initialize command module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Exits from command module
 */
void orxFASTCALL orxCommand_Exit()
{
  /* Initialized? */
  if(sstCommand.u32Flags & orxCOMMAND_KU32_STATIC_FLAG_READY)
  {
    orxCOMMAND *pstCommand;

    /* While there are registered commands */
    while(orxHashTable_GetNext(sstCommand.pstTable, orxNULL, orxNULL, (void **)&pstCommand) != orxHANDLE_UNDEFINED)
    {
      /* Unregisters it */
      orxCommand_Unregister(pstCommand->zName);
    }

    /* Deletes table */
    orxHashTable_Delete(sstCommand.pstTable);

    /* Deletes bank */
    orxBank_Delete(sstCommand.pstBank);

    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_TIMELINE, orxCommand_EventHandler);

    /* Updates flags */
    sstCommand.u32Flags &= ~orxCOMMAND_KU32_STATIC_FLAG_READY;
  }

  /* Done! */
  return;
}

/** Registers a command
* @param[in]   _zCommand      Command name
* @param[in]   _pfnFunction   Associated function
* @param[in]   _u32ParamNumber Number of arguments sent to the command
* @param[in]   _astParamList  List of parameters of the command
* @param[in]   _pstResult     Result
* @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
*/
orxSTATUS orxFASTCALL orxCommand_Register(const orxSTRING _zCommand, const orxCOMMAND_FUNCTION _pfnFunction, orxU32 _u32ParamNumber, const orxCOMMAND_VAR_DEF *_astParamList, const orxCOMMAND_VAR_DEF *_pstResult)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstCommand.u32Flags, orxCOMMAND_KU32_STATIC_FLAG_READY));
  orxASSERT(_zCommand != orxNULL);
  orxASSERT(_pfnFunction != orxNULL);
  orxASSERT(_astParamList != orxNULL);
  orxASSERT(_pstResult != orxNULL);

  /* Valid? */
  if((_zCommand != orxNULL) && (_zCommand != orxSTRING_EMPTY))
  {
    orxU32 u32ID;

    /* Gets its ID */
    u32ID = orxString_ToCRC(_zCommand);

    /* Not already registered? */
    if(orxHashTable_Get(sstCommand.pstTable, u32ID) == orxNULL)
    {
      orxCOMMAND *pstCommand;

      /* Allocates command */
      pstCommand = (orxCOMMAND *)orxBank_Allocate(sstCommand.pstBank);

      /* Valid? */
      if(pstCommand != orxNULL)
      {
        orxU32 i;

        /* Inits it */
        pstCommand->pfnFunction     = _pfnFunction;
        pstCommand->stResult.zName  = orxString_Duplicate(_pstResult->zName);
        pstCommand->stResult.eType  = _pstResult->eType;
        pstCommand->zName           = orxString_Duplicate(_zCommand);
        pstCommand->u32ParamNumber  = _u32ParamNumber;

        /* Allocates parameter list */
        pstCommand->astParamList    = (orxCOMMAND_VAR_DEF *)orxMemory_Allocate(_u32ParamNumber * sizeof(orxCOMMAND_VAR_DEF), orxMEMORY_TYPE_MAIN);

        /* Checks */
        orxASSERT(pstCommand->astParamList != orxNULL);

        /* For all parameters */
        for(i = 0; i < _u32ParamNumber; i++)
        {
          /* Inits it */
          pstCommand->astParamList[i].zName = orxString_Duplicate(_astParamList[i].zName);
          pstCommand->astParamList[i].eType = _astParamList[i].eType;
        }

        /* Adds it to the table */
        orxHashTable_Add(sstCommand.pstTable, u32ID, pstCommand);

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Can't allocate memory for command [%s], aborting.", _zCommand);
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Can't register command: [%s] is already registered.", _zCommand);
    }
  }

  /* Done! */
  return eResult;
}

/** Unregisters a command
* @param[in]   _zCommand      Command name
* @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
*/
orxSTATUS orxFASTCALL orxCommand_Unregister(const orxSTRING _zCommand)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstCommand.u32Flags, orxCOMMAND_KU32_STATIC_FLAG_READY));
  orxASSERT(_zCommand != orxNULL);

  /* Valid? */
  if((_zCommand != orxNULL) & (_zCommand != orxSTRING_EMPTY))
  {
    orxCOMMAND *pstCommand;
    orxU32      u32ID;

    /* Gets its ID */
    u32ID = orxString_ToCRC(_zCommand);

    /* Gets it */
    pstCommand = (orxCOMMAND *)orxHashTable_Get(sstCommand.pstTable, u32ID);

    /* Found? */
    if(pstCommand != orxNULL)
    {
      orxU32 i;

      /* Removes it */
      eResult = orxHashTable_Remove(sstCommand.pstTable, u32ID);

      /* For all its parameters */
      for(i = 0; i < pstCommand->u32ParamNumber; i++)
      {
        /* Deletes its name */
        orxString_Delete(pstCommand->astParamList[i].zName);
      }

      /* Deletes its variables */
      orxString_Delete(pstCommand->stResult.zName);
      orxString_Delete(pstCommand->zName);
      orxMemory_Free(pstCommand->astParamList);

      /* Deletes it */
      orxBank_Free(sstCommand.pstBank, pstCommand);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Can't unregister command: [%s] is not registered.", _zCommand);
    }
  }

  /* Done! */
  return eResult;
}

/** Is a command registered?
* @param[in]   _zCommand      Command name
* @return      orxTRUE / orxFALSE
*/
orxBOOL orxFASTCALL orxCommand_IsRegistered(const orxSTRING _zCommand)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstCommand.u32Flags, orxCOMMAND_KU32_STATIC_FLAG_READY));
  orxASSERT(_zCommand != orxNULL);

  /* Valid? */
  if((_zCommand != orxNULL) & (_zCommand != orxSTRING_EMPTY))
  {
    orxU32 u32ID;

    /* Gets its ID */
    u32ID = orxString_ToCRC(_zCommand);

    /* Updates result */
    bResult = (orxHashTable_Get(sstCommand.pstTable, u32ID) != orxNULL) ? orxTRUE : orxFALSE;
  }
  else
  {
    /* Updates result */
    bResult = orxFALSE;
  }

  /* Done! */
  return bResult;
}

/** Evaluates a command
* @param[in]   _zCommandLine  Command name + arguments
* @param[out]  _pstResult     Variable that will contain the result
* @return      Command result if found, orxNULL otherwise
*/
orxCOMMAND_VAR *orxFASTCALL orxCommand_Evaluate(const orxSTRING _zCommandLine, orxCOMMAND_VAR *_pstResult)
{
  orxCOMMAND_VAR *pstResult = orxNULL;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstCommand.u32Flags, orxCOMMAND_KU32_STATIC_FLAG_READY));
  orxASSERT(_zCommandLine != orxNULL);
  orxASSERT(_pstResult != orxNULL);

  /* Valid? */
  if((_zCommandLine != orxNULL) & (_zCommandLine != orxSTRING_EMPTY))
  {
    const orxSTRING zCommand;

    /* Not an internal call? */
    if(!orxFLAG_TEST(sstCommand.u32Flags, orxCOMMAND_KU32_STATIC_FLAG_INTERNAL_CALL))
    {
      /* Copies command line to work buffer */
      orxString_NCopy(sstCommand.acBuffer, _zCommandLine, orxCOMMAND_KU32_BUFFER_SIZE);
    }

    /* Gets start of command */
    zCommand = orxString_SkipWhiteSpaces(sstCommand.acBuffer);

    /* Valid? */
    if(zCommand != orxSTRING_EMPTY)
    {
      orxU32          u32ID;
      const orxCHAR  *pc;
      orxCOMMAND     *pstCommand;

      /* Finds end of command */
      for(pc = zCommand + 1; (*pc != orxCHAR_NULL) && (*pc != ' ') && (*pc != '\t') && (*pc != orxCHAR_CR) && (*pc != orxCHAR_LF); pc++);

      /* Ends command */
      *(orxCHAR *)pc = orxCHAR_NULL;

      /* Gets its ID */
      u32ID = orxString_ToCRC(zCommand);

      /* Gets it */
      pstCommand = (orxCOMMAND *)orxHashTable_Get(sstCommand.pstTable, u32ID);

      /* Found? */
      if(pstCommand != orxNULL)
      {
        orxSTATUS       eStatus;
        const orxSTRING zArg;
        orxU32          u32ArgNumber;

#ifdef __orxMSVC__

        orxCOMMAND_VAR *astArgList = (orxCOMMAND_VAR *)alloca(pstCommand->u32ParamNumber * sizeof(orxCOMMAND_VAR));

#else /* __orxMSVC__ */

        orxCOMMAND_VAR astArgList[pstCommand->u32ParamNumber];

#endif /* __orxMSVC__ */

        /* For the remainder of the buffer? */
        for(pc++, eStatus = orxSTATUS_SUCCESS, zArg = orxSTRING_EMPTY, u32ArgNumber = 0;
            (u32ArgNumber < pstCommand->u32ParamNumber) && (eStatus != orxSTATUS_FAILURE) && (pc - sstCommand.acBuffer < orxCOMMAND_KU32_BUFFER_SIZE) && (*pc != orxCHAR_NULL);
            pc++, u32ArgNumber++)
        {
          /* Skips all whitespaces */
          pc = orxString_SkipWhiteSpaces(pc);

          /* Valid? */
          if(*pc != orxCHAR_NULL)
          {
            /* Gets arg's beginning */
            zArg = pc;

            /* Stores its type */
            astArgList[u32ArgNumber].eType = pstCommand->astParamList[u32ArgNumber].eType;

            /* Depending on its type */
            switch(pstCommand->astParamList[u32ArgNumber].eType)
            {
              default:
              case orxCOMMAND_VAR_TYPE_STRING:
              {
                orxBOOL bInString = orxFALSE;

                /* Is a string marker? */
                if(*pc == orxCOMMAND_KC_STRING_MARKER)
                {
                  /* Updates arg pointer */
                  zArg++;
                  pc++;

                  /* Is not a block delimiter or triple block delimiter? */
                  if((*pc != orxCOMMAND_KC_STRING_MARKER)
                  || (*(pc + 1) == orxCOMMAND_KC_STRING_MARKER))
                  {
                    /* Updates string status */
                    bInString = orxTRUE;
                  }
                }

                /* Finds end of argument */
                for(; *pc != orxCHAR_NULL; pc++)
                {
                  /* In string? */
                  if(bInString != orxFALSE)
                  {
                    /* Is a string marker? */
                    if(*pc == orxCOMMAND_KC_STRING_MARKER)
                    {
                      /* Isn't next one also a string marker? */
                      if(*(pc + 1) != orxCOMMAND_KC_STRING_MARKER)
                      {
                        /* Stops */
                        break;
                      }
                      else
                      {
                        orxCHAR *pcTemp;

                        /* Erases it */
                        for(pcTemp = (orxCHAR *)pc + 1; *pcTemp != orxNULL; pcTemp++)
                        {
                          *pcTemp = *(pcTemp + 1);
                        }
                      }
                    }
                  }
                  else
                  {
                    /* End of string? */
                    if((*pc == ' ') || (*pc == '\t'))
                    {
                      /* Stops */
                      break;
                    }
                  }
                }

                /* Ends it */
                *(orxCHAR *)pc = orxCHAR_NULL;

                /* Stores its value */
                astArgList[u32ArgNumber].zValue = zArg;

                break;
              }

              case orxCOMMAND_VAR_TYPE_FLOAT:
              {
                /* Gets its value */
                eStatus = orxString_ToFloat(zArg, &(astArgList[u32ArgNumber].fValue), &pc);

                break;
              }

              case orxCOMMAND_VAR_TYPE_S32:
              {
                /* Gets its value */
                eStatus = orxString_ToS32(zArg, &(astArgList[u32ArgNumber].s32Value), &pc);

                break;
              }

              case orxCOMMAND_VAR_TYPE_U32:
              {
                /* Gets its value */
                eStatus = orxString_ToU32(zArg, &(astArgList[u32ArgNumber].u32Value), &pc);

                break;
              }

              case orxCOMMAND_VAR_TYPE_S64:
              {
                /* Gets its value */
                eStatus = orxString_ToS64(zArg, &(astArgList[u32ArgNumber].s64Value), &pc);

                break;
              }

              case orxCOMMAND_VAR_TYPE_U64:
              {
                /* Gets its value */
                eStatus = orxString_ToU64(zArg, &(astArgList[u32ArgNumber].u64Value), &pc);

                break;
              }

              case orxCOMMAND_VAR_TYPE_BOOL:
              {
                /* Gets its value */
                eStatus = orxString_ToBool(zArg, &(astArgList[u32ArgNumber].bValue), &pc);

                break;
              }

              case orxCOMMAND_VAR_TYPE_VECTOR:
              {
                /* Gets its value */
                eStatus = orxString_ToVector(zArg, &(astArgList[u32ArgNumber].vValue), &pc);

                break;
              }
            }
          }
        }

        /* Error? */
        if(eStatus == orxSTATUS_FAILURE)
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Can't evaluate command line [%s], wrong argument #%ld <%s>.", _zCommandLine, u32ArgNumber, zArg);
        }
        /* Not all arguments? */
        else if(u32ArgNumber != pstCommand->u32ParamNumber)
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Can't evaluate command line [%s], expected (%ld) arguments, found (%ld).", _zCommandLine, pstCommand->u32ParamNumber, u32ArgNumber);
        }
        else
        {
          /* Runs it */
          pstResult = orxCommand_Run(pstCommand, orxFALSE, u32ArgNumber, astArgList, _pstResult);
        }
      }
      else
      {
        /* Not an internal call? */
        if(!orxFLAG_TEST(sstCommand.u32Flags, orxCOMMAND_KU32_STATIC_FLAG_INTERNAL_CALL))
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Can't evaluate command line [%s], invalid command.", _zCommandLine);
        }
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Can't evaluate command line [%s]: [%s] is not a registered command.", _zCommandLine, zCommand);
    }
  }

  /* Done! */
  return pstResult;
}

/** Executes a command
* @param[in]   _zCommand      Command name
* @param[in]   _u32ArgNumber  Number of arguments sent to the command
* @param[in]   _astArgList    List of arguments sent to the command
* @param[out]  _pstResult     Variable that will contain the result
* @return      Command result if found, orxNULL otherwise
*/
orxCOMMAND_VAR *orxFASTCALL orxCommand_Execute(const orxSTRING _zCommand, orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCOMMAND_VAR *pstResult = orxNULL;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstCommand.u32Flags, orxCOMMAND_KU32_STATIC_FLAG_READY));
  orxASSERT(_zCommand != orxNULL);
  orxASSERT(_pstResult != orxNULL);

  /* Valid? */
  if((_zCommand != orxNULL) & (_zCommand != orxSTRING_EMPTY))
  {
    orxU32      u32ID;
    orxCOMMAND *pstCommand;

    /* Gets its ID */
    u32ID = orxString_ToCRC(_zCommand);

    /* Gets it */
    pstCommand = (orxCOMMAND *)orxHashTable_Get(sstCommand.pstTable, u32ID);

    /* Found? */
    if(pstCommand != orxNULL)
    {
      /* Runs it */
      pstResult = orxCommand_Run(pstCommand, orxTRUE, _u32ArgNumber, _astArgList, _pstResult);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Can't execute command: [%s] is not registered.", _zCommand);
    }
  }

  /* Done! */
  return pstResult;
}
