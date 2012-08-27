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
#include "debug/orxProfiler.h"
#include "core/orxEvent.h"
#include "memory/orxMemory.h"
#include "memory/orxBank.h"
#include "object/orxTimeLine.h"
#include "utils/orxHashTable.h"
#include "utils/orxString.h"
#include "utils/orxTree.h"

#ifdef __orxMSVC__

  #include "malloc.h"
  #pragma warning(disable : 4200)

#endif /* __orxMSVC__ */


/** Module flags
 */
#define orxCOMMAND_KU32_STATIC_FLAG_NONE              0x00000000                      /**< No flags */

#define orxCOMMAND_KU32_STATIC_FLAG_READY             0x00000001                      /**< Ready flag */
#define orxCOMMAND_KU32_STATIC_FLAG_PROCESSING_EVENT  0x10000000                      /** <Processing event flag */

#define orxCOMMAND_KU32_STATIC_MASK_ALL               0xFFFFFFFF                      /**< All mask */


/** Misc
 */
#define orxCOMMAND_KC_STRING_MARKER                   '"'                             /**< String marker character */
#define orxCOMMAND_KC_PUSH_MARKER                     '>'                             /**< Push marker character */
#define orxCOMMAND_KC_POP_MARKER                      '<'                             /**< Pop marker character */
#define orxCOMMAND_KC_GUID_MARKER                     '^'                             /**< GUID marker character */


#define orxCOMMAND_KU32_TABLE_SIZE                    256
#define orxCOMMAND_KU32_BANK_SIZE                     128
#define orxCOMMAND_KU32_TRIE_BANK_SIZE                256
#define orxCOMMAND_KU32_RESULT_BANK_SIZE              16

#define orxCOMMAND_KU32_EVALUATE_BUFFER_SIZE          4096
#define orxCOMMAND_KU32_PROTOTYPE_BUFFER_SIZE         512

#define orxCOMMAND_KZ_ERROR_VALUE                     "STACK_ERROR"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Command stack entry
 */
typedef struct __orxCOMMAND_STACK_ENTRY_t
{
  orxCOMMAND_VAR            stValue;                                                  /**< Value : 28 */

} orxCOMMAND_STACK_ENTRY;

/** Command structure
 */
typedef struct __orxCOMMAND_t
{
  orxCOMMAND_FUNCTION       pfnFunction;                                              /**< Function : 4 */
  orxSTRING                 zName;                                                    /**< Name : 8 */
  orxCOMMAND_VAR_DEF        stResult;                                                 /**< Result definition : 16 */
  orxU16                    u16RequiredParamNumber;                                   /**< Required param number : 18 */
  orxU16                    u16OptionalParamNumber;                                   /**< Optional param number : 20 */
  orxCOMMAND_VAR_DEF       *astParamList;                                             /**< Param list : 24 */

} orxCOMMAND;

/** Command trie node
 */
typedef struct __orxCOMMAND_TRIE_NODE_t
{
  orxTREE_NODE              stNode;
  const orxCOMMAND         *pstCommand;
  orxCHAR                   cLetter;

} orxCOMMAND_TRIE_NODE;

/** Static structure
 */
typedef struct __orxCOMMAND_STATIC_t
{
  orxBANK                  *pstBank;                                                  /**< Command bank */
  orxBANK                  *pstTrieBank;                                              /**< Command trie bank */
  orxTREE                   stCommandTrie;                                            /**< Command trie */
  orxHASHTABLE             *pstTable;                                                 /**< Command table */
  orxBANK                  *pstResultBank;                                            /**< Command result bank */
  orxCHAR                   acEvaluateBuffer[orxCOMMAND_KU32_EVALUATE_BUFFER_SIZE];   /**< Evaluate buffer */
  orxCHAR                   acPrototypeBuffer[orxCOMMAND_KU32_PROTOTYPE_BUFFER_SIZE]; /**< Prototype buffer */
  orxU32                    u32Flags;                                                 /**< Control flags */

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

/** Command: Help
 */
void orxFASTCALL orxCommand_CommandHelp(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* No argument? */
  if(_u32ArgNumber == 0)
  {
    /* Updates result */
    _pstResult->zValue = "Usage: Command.Help <Command> to get the prototype of a command.";
  }
  else
  {
    /* Updates result */
    _pstResult->zValue = orxCommand_GetPrototype(_astArgList[0].zValue);
  }

  /* Done! */
  return;
}

/** Registers all the command commands
 */
static orxINLINE void orxCommand_RegisterCommands()
{
  /* Command: Help */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, Help, "Help", orxCOMMAND_VAR_TYPE_STRING, 0, 1, {"Command", orxCOMMAND_VAR_TYPE_STRING});
}

/** Unregisters all the command commands
 */
static orxINLINE void orxCommand_UnregisterCommands()
{
  /* Command: Help */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, Help);
}

/** Gets literal name of a command var type
 */
static orxINLINE const orxSTRING orxCommand_GetTypeString(orxCOMMAND_VAR_TYPE _eType)
{
  const orxSTRING zResult;

#define orxCOMMAND_DECLARE_TYPE_NAME(TYPE) case orxCOMMAND_VAR_TYPE_##TYPE: zResult = "orx"#TYPE; break

  /* Depending on type */
  switch(_eType)
  {
    orxCOMMAND_DECLARE_TYPE_NAME(STRING);
    orxCOMMAND_DECLARE_TYPE_NAME(FLOAT);
    orxCOMMAND_DECLARE_TYPE_NAME(S32);
    orxCOMMAND_DECLARE_TYPE_NAME(U32);
    orxCOMMAND_DECLARE_TYPE_NAME(S64);
    orxCOMMAND_DECLARE_TYPE_NAME(U64);
    orxCOMMAND_DECLARE_TYPE_NAME(BOOL);
    orxCOMMAND_DECLARE_TYPE_NAME(VECTOR);

    default:
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "No name defined for command var type #%d.", _eType);

      /* Updates result */
      zResult = orxSTRING_EMPTY;
    }
  }

  /* Done! */
  return zResult;
}

/** Runs a command
 */
static orxINLINE orxCOMMAND_VAR *orxCommand_Run(const orxCOMMAND *_pstCommand, orxBOOL _bCheckArgList, orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCOMMAND_VAR *pstResult = orxNULL;

  /* Valid number of arguments? */
  if((_bCheckArgList == orxFALSE)
  || ((_u32ArgNumber >= (orxU32)_pstCommand->u16RequiredParamNumber)
   && (_u32ArgNumber <= (orxU32)_pstCommand->u16RequiredParamNumber + (orxU32)_pstCommand->u16OptionalParamNumber)))
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
          orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Can't execute command [%s]: invalid type for argument #%d (%s).", _pstCommand->zName, i + 1, _pstCommand->astParamList[i].zName);

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
      pstResult = _pstResult;
    }
  }

  /* Done! */
  return pstResult;
}

static orxCOMMAND_VAR *orxFASTCALL orxCommand_Process(const orxSTRING _zCommandLine, const orxU64 _u64GUID, orxCOMMAND_VAR *_pstResult)
{
  orxSTRING       zCommand;
  orxCOMMAND_VAR *pstResult = orxNULL;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxCommand_Process");

  /* Gets start of command */
  zCommand = (orxCHAR *)orxString_SkipWhiteSpaces(_zCommandLine);

  /* Valid? */
  if(zCommand != orxSTRING_EMPTY)
  {
    orxU32      u32PushCounter, u32ID;
    orxCHAR    *pcCommandEnd;
    orxCOMMAND *pstCommand;
    orxCHAR     acGUID[20];

    /* For all push markers / spaces */
    for(u32PushCounter = 0; (*zCommand == orxCOMMAND_KC_PUSH_MARKER) || (*zCommand == ' ') || (*zCommand == '\t'); zCommand++)
    {
      /* Is a push marker? */
      if(*zCommand == orxCOMMAND_KC_PUSH_MARKER)
      {
        /* Updates push counter */
        u32PushCounter++;
      }
    }

    /* Finds end of command */
    for(pcCommandEnd = zCommand + 1; (*pcCommandEnd != orxCHAR_NULL) && (*pcCommandEnd != ' ') && (*pcCommandEnd != '\t') && (*pcCommandEnd != orxCHAR_CR) && (*pcCommandEnd != orxCHAR_LF); pcCommandEnd++);

    /* Ends command */
    *(orxCHAR *)pcCommandEnd = orxCHAR_NULL;

    /* Gets its ID */
    u32ID = orxString_ToCRC(zCommand);

    /* Restores command end */
    *(orxCHAR *)pcCommandEnd = ' ';

    /* Gets it */
    pstCommand = (orxCOMMAND *)orxHashTable_Get(sstCommand.pstTable, u32ID);

    /* Found? */
    if(pstCommand != orxNULL)
    {
      orxSTATUS       eStatus;
      orxS32          s32GUIDLength;
      const orxCHAR  *pcSrc;
      orxCHAR        *pcDst;
      const orxSTRING zArg;
      orxU32          u32ArgNumber, u32ParamNumber = (orxU32)pstCommand->u16RequiredParamNumber + (orxU32)pstCommand->u16OptionalParamNumber;

#ifdef __orxMSVC__

      orxCOMMAND_VAR *astArgList = (orxCOMMAND_VAR *)alloca(u32ParamNumber * sizeof(orxCOMMAND_VAR));

#else /* __orxMSVC__ */

      orxCOMMAND_VAR astArgList[u32ParamNumber];

#endif /* __orxMSVC__ */

      /* Gets owner's GUID */
      acGUID[19]    = orxCHAR_NULL;
      s32GUIDLength = orxString_NPrint(acGUID, 19, "0x%016llX", _u64GUID);

      /* For all characters */
      for(pcDst = sstCommand.acEvaluateBuffer, pcSrc = pcCommandEnd + 1; (*pcSrc != orxCHAR_NULL) && (pcDst - sstCommand.acEvaluateBuffer < orxCOMMAND_KU32_EVALUATE_BUFFER_SIZE - 1); pcSrc++)
      {
        /* Depending on character */
        switch(*pcSrc)
        {
          case orxCOMMAND_KC_GUID_MARKER:
          {
            /* Replaces it with GUID */
            orxString_Copy(pcDst, acGUID);

            /* Updates pointer */
            pcDst += s32GUIDLength;

            break;
          }

          case orxCOMMAND_KC_POP_MARKER:
          {
            /* Valid? */
            if(orxBank_GetCounter(sstCommand.pstResultBank) > 0)
            {
              orxCOMMAND_STACK_ENTRY *pstEntry;
              orxCHAR                 acValue[64];
              const orxSTRING         zValue = acValue;

              /* Gets last stack entry */
              pstEntry = (orxCOMMAND_STACK_ENTRY *)orxBank_GetAtIndex(sstCommand.pstResultBank, orxBank_GetCounter(sstCommand.pstResultBank) - 1);

              /* Inits value */
              acValue[63] = orxCHAR_NULL;

              /* Depending on type */
              switch(pstEntry->stValue.eType)
              {
                default:
                case orxCOMMAND_VAR_TYPE_STRING:
                {
                  /* Updates pointer */
                  zValue = pstEntry->stValue.zValue;

                  break;
                }

                case orxCOMMAND_VAR_TYPE_FLOAT:
                {
                  /* Stores it */
                  orxString_NPrint(acValue, 63, "%g", pstEntry->stValue.fValue);

                  break;
                }

                case orxCOMMAND_VAR_TYPE_S32:
                {
                  /* Stores it */
                  orxString_NPrint(acValue, 63, "%d", pstEntry->stValue.s32Value);

                  break;
                }

                case orxCOMMAND_VAR_TYPE_U32:
                {
                  /* Stores it */
                  orxString_NPrint(acValue, 63, "%u", pstEntry->stValue.u32Value);

                  break;
                }

                case orxCOMMAND_VAR_TYPE_S64:
                {
                  /* Stores it */
                  orxString_NPrint(acValue, 63, "%lld", pstEntry->stValue.s64Value);

                  break;
                }

                case orxCOMMAND_VAR_TYPE_U64:
                {
                  /* Stores it */
                  orxString_NPrint(acValue, 63, "0x%016llX", pstEntry->stValue.u64Value);

                  break;
                }

                case orxCOMMAND_VAR_TYPE_BOOL:
                {
                  /* Stores it */
                  orxString_NPrint(acValue, 63, "%s", (pstEntry->stValue.bValue == orxFALSE) ? orxSTRING_FALSE : orxSTRING_TRUE);

                  break;
                }

                case orxCOMMAND_VAR_TYPE_VECTOR:
                {
                  /* Gets literal value */
                  orxString_NPrint(acValue, 63, "%c%g%c %g%c %g%c", orxSTRING_KC_VECTOR_START, pstEntry->stValue.vValue.fX, orxSTRING_KC_VECTOR_SEPARATOR, pstEntry->stValue.vValue.fY, orxSTRING_KC_VECTOR_SEPARATOR, pstEntry->stValue.vValue.fZ, orxSTRING_KC_VECTOR_END);

                  break;
                }
              }

              /* Replaces marker with stacked value */
              orxString_Copy(pcDst, zValue);

              /* Updates pointers */
              pcDst += orxString_GetLength(zValue);

              /* Is a string value? */
              if(pstEntry->stValue.eType == orxCOMMAND_VAR_TYPE_STRING)
              {
                /* Deletes it */
                orxString_Delete((orxCHAR *)pstEntry->stValue.zValue);
              }

              /* Deletes stack entry */
              orxBank_Free(sstCommand.pstResultBank, pstEntry);
            }
            else
            {
              /* Logs message */
              orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Can't pop stacked argument for command line [%s]: stack is empty.", _zCommandLine);
            }

            break;
          }

          default:
          {
            /* Copies it */
            *pcDst++ = *pcSrc;

            break;
          }
        }
      }

      /* Copies end of string */
      *pcDst = orxCHAR_NULL;

      /* For all characters in the buffer */
      for(pcSrc = sstCommand.acEvaluateBuffer, eStatus = orxSTATUS_SUCCESS, zArg = orxSTRING_EMPTY, u32ArgNumber = 0;
          (u32ArgNumber < u32ParamNumber) && (pcSrc - sstCommand.acEvaluateBuffer < orxCOMMAND_KU32_EVALUATE_BUFFER_SIZE) && (*pcSrc != orxCHAR_NULL);
          pcSrc++, u32ArgNumber++)
      {
        /* Skips all whitespaces */
        pcSrc = orxString_SkipWhiteSpaces(pcSrc);

        /* Valid? */
        if(*pcSrc != orxCHAR_NULL)
        {
          /* Gets arg's beginning */
          zArg = pcSrc;

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
              if(*pcSrc == orxCOMMAND_KC_STRING_MARKER)
              {
                /* Updates arg pointer */
                zArg++;
                pcSrc++;

                /* Is not a block delimiter or triple block delimiter? */
                if((*pcSrc != orxCOMMAND_KC_STRING_MARKER)
                || (*(pcSrc + 1) == orxCOMMAND_KC_STRING_MARKER))
                {
                  /* Updates string status */
                  bInString = orxTRUE;
                }
              }

              /* Finds end of argument */
              for(; *pcSrc != orxCHAR_NULL; pcSrc++)
              {
                /* In string? */
                if(bInString != orxFALSE)
                {
                  /* Is a string marker? */
                  if(*pcSrc == orxCOMMAND_KC_STRING_MARKER)
                  {
                    /* Isn't next one also a string marker? */
                    if(*(pcSrc + 1) != orxCOMMAND_KC_STRING_MARKER)
                    {
                      /* Stops */
                      break;
                    }
                    else
                    {
                      orxCHAR *pcTemp;

                      /* Erases it */
                      for(pcTemp = (orxCHAR *)pcSrc + 1; *pcTemp != orxNULL; pcTemp++)
                      {
                        *pcTemp = *(pcTemp + 1);
                      }
                    }
                  }
                }
                else
                {
                  /* End of string? */
                  if((*pcSrc == ' ') || (*pcSrc == '\t'))
                  {
                    /* Stops */
                    break;
                  }
                }
              }

              /* Stores its value */
              astArgList[u32ArgNumber].zValue = zArg;

              break;
            }

            case orxCOMMAND_VAR_TYPE_FLOAT:
            {
              /* Gets its value */
              eStatus = orxString_ToFloat(zArg, &(astArgList[u32ArgNumber].fValue), &pcSrc);

              break;
            }

            case orxCOMMAND_VAR_TYPE_S32:
            {
              /* Gets its value */
              eStatus = orxString_ToS32(zArg, &(astArgList[u32ArgNumber].s32Value), &pcSrc);

              break;
            }

            case orxCOMMAND_VAR_TYPE_U32:
            {
              /* Gets its value */
              eStatus = orxString_ToU32(zArg, &(astArgList[u32ArgNumber].u32Value), &pcSrc);

              break;
            }

            case orxCOMMAND_VAR_TYPE_S64:
            {
              /* Gets its value */
              eStatus = orxString_ToS64(zArg, &(astArgList[u32ArgNumber].s64Value), &pcSrc);

              break;
            }

            case orxCOMMAND_VAR_TYPE_U64:
            {
              /* Gets its value */
              eStatus = orxString_ToU64(zArg, &(astArgList[u32ArgNumber].u64Value), &pcSrc);

              break;
            }

            case orxCOMMAND_VAR_TYPE_BOOL:
            {
              /* Gets its value */
              eStatus = orxString_ToBool(zArg, &(astArgList[u32ArgNumber].bValue), &pcSrc);

              break;
            }

            case orxCOMMAND_VAR_TYPE_VECTOR:
            {
              /* Gets its value */
              eStatus = orxString_ToVector(zArg, &(astArgList[u32ArgNumber].vValue), &pcSrc);

              break;
            }
          }

          /* Interrupted? */
          if((eStatus == orxSTATUS_FAILURE) || (*pcSrc == orxCHAR_NULL))
          {
            /* Updates argument counter */
            u32ArgNumber++;

            /* Stops processing */
            break;
          }
          else
          {
            /* Ends current argument */
            *(orxCHAR *)pcSrc = orxCHAR_NULL;
          }
        }
      }

      /* Error? */
      if((eStatus == orxSTATUS_FAILURE) || (u32ArgNumber < (orxU32)pstCommand->u16RequiredParamNumber))
      {
        /* Incorrect parameter? */
        if(eStatus == orxSTATUS_FAILURE)
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Can't evaluate command line [%s], invalid argument #%d.", _zCommandLine, u32ArgNumber);
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Can't evaluate command line [%s], expected %d[+%d] arguments, found %d.", _zCommandLine, (orxU32)pstCommand->u16RequiredParamNumber, (orxU32)pstCommand->u16OptionalParamNumber, u32ArgNumber);
        }
      }
      else
      {
        /* Runs it */
        pstResult = orxCommand_Run(pstCommand, orxFALSE, u32ArgNumber, astArgList, _pstResult);
      }
    }
    else
    {
      /* Not processing event? */
      if(!orxFLAG_TEST(sstCommand.u32Flags, orxCOMMAND_KU32_STATIC_FLAG_PROCESSING_EVENT))
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Can't evaluate command line [%s], invalid command.", _zCommandLine);
      }
    }

    /* Failure? */
    if(pstResult == orxNULL)
    {
      /* Stores error */
      _pstResult->eType   = orxCOMMAND_VAR_TYPE_STRING;
      _pstResult->zValue  = orxCOMMAND_KZ_ERROR_VALUE;
    }

    /* For all requested pushes */
    while(u32PushCounter > 0)
    {
      orxCOMMAND_STACK_ENTRY *pstEntry;

      /* Allocates stack entry */
      pstEntry = (orxCOMMAND_STACK_ENTRY *)orxBank_Allocate(sstCommand.pstResultBank);

      /* Is a string value? */
      if(_pstResult->eType == orxCOMMAND_VAR_TYPE_STRING)
      {
        /* Duplicates it */
        pstEntry->stValue.eType = orxCOMMAND_VAR_TYPE_STRING;
        pstEntry->stValue.zValue = orxString_Duplicate(_pstResult->zValue);
      }
      else
      {
        /* Stores value */
        orxMemory_Copy(&(pstEntry->stValue), _pstResult, sizeof(orxCOMMAND_STACK_ENTRY));
      }

      /* Updates push counter */
      u32PushCounter--;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Can't evaluate command line [%s]: [%s] is not a registered command.", _zCommandLine, zCommand);
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return pstResult;
}

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

      /* Gets payload */
      pstPayload = (orxTIMELINE_EVENT_PAYLOAD *)_pstEvent->pstPayload;

      /* Updates internal status */
      orxFLAG_SET(sstCommand.u32Flags, orxCOMMAND_KU32_STATIC_FLAG_PROCESSING_EVENT, orxCOMMAND_KU32_STATIC_FLAG_NONE);

      /* Processes command */
      orxCommand_Process(pstPayload->zEvent, orxStructure_GetGUID(orxSTRUCTURE(_pstEvent->hSender)), &stResult);

      /* Updates internal status */
      orxFLAG_SET(sstCommand.u32Flags, orxCOMMAND_KU32_STATIC_FLAG_NONE, orxCOMMAND_KU32_STATIC_FLAG_PROCESSING_EVENT);

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

static orxINLINE void orxCommand_InsertInTrie(const orxCOMMAND *_pstCommand)
{
  //! TODO
}

static orxINLINE void orxCommand_RemoveFromTrie(const orxCOMMAND *_pstCommand)
{
  //! TODO
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
  orxModule_AddDependency(orxMODULE_ID_COMMAND, orxMODULE_ID_EVENT);
  orxModule_AddDependency(orxMODULE_ID_COMMAND, orxMODULE_ID_PROFILER);

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
      /* Creates banks */
      sstCommand.pstBank        = orxBank_Create(orxCOMMAND_KU32_BANK_SIZE, sizeof(orxCOMMAND), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
      sstCommand.pstTrieBank    = orxBank_Create(orxCOMMAND_KU32_TRIE_BANK_SIZE, sizeof(orxCOMMAND_TRIE_NODE), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
      sstCommand.pstResultBank  = orxBank_Create(orxCOMMAND_KU32_RESULT_BANK_SIZE, sizeof(orxCOMMAND_STACK_ENTRY), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

      /* Valid? */
      if((sstCommand.pstBank != orxNULL) && (sstCommand.pstTrieBank != orxNULL) && (sstCommand.pstResultBank != orxNULL))
      {
        /* Creates table */
        sstCommand.pstTable = orxHashTable_Create(orxCOMMAND_KU32_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

        /* Valid? */
        if(sstCommand.pstTable != orxNULL)
        {
          /* Inits Flags */
          sstCommand.u32Flags = orxCOMMAND_KU32_STATIC_FLAG_READY;

          /* Registers commands */
          orxCommand_RegisterCommands();

          /* Updates result */
          eResult = orxSTATUS_SUCCESS;
        }
        else
        {
          /* Removes event handler */
          orxEvent_RemoveHandler(orxEVENT_TYPE_TIMELINE, orxCommand_EventHandler);

          /* Deletes banks */
          orxBank_Delete(sstCommand.pstBank);
          orxBank_Delete(sstCommand.pstTrieBank);
          orxBank_Delete(sstCommand.pstResultBank);

          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Failed to create command table.");
        }
      }
      else
      {
        /* Partly initialized? */
        if(sstCommand.pstTrieBank != orxNULL)
        {
          /* Deletes bank */
          orxBank_Delete(sstCommand.pstTrieBank);
        }

        /* Partly initialized? */
        if(sstCommand.pstBank != orxNULL)
        {
          /* Deletes bank */
          orxBank_Delete(sstCommand.pstBank);
        }

        /* Removes event handler */
        orxEvent_RemoveHandler(orxEVENT_TYPE_TIMELINE, orxCommand_EventHandler);

        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Failed to create command banks.");
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
    orxU32      i;

    /* Unregisters commands */
    orxCommand_UnregisterCommands();

    /* For all entries in the result stack */
    for(i = 0; i < orxBank_GetCounter(sstCommand.pstResultBank); i++)
    {
      orxCOMMAND_STACK_ENTRY *pstEntry;

      /* Gets it */
      pstEntry = (orxCOMMAND_STACK_ENTRY *)orxBank_GetAtIndex(sstCommand.pstResultBank, i);

      /* Is a string value? */
      if(pstEntry->stValue.eType == orxCOMMAND_VAR_TYPE_STRING)
      {
        /* Deletes it */
        orxString_Delete((orxCHAR *)pstEntry->stValue.zValue);
      }
    }

    /* While there are registered commands */
    while(orxHashTable_GetNext(sstCommand.pstTable, orxNULL, orxNULL, (void **)&pstCommand) != orxHANDLE_UNDEFINED)
    {
      /* Unregisters it */
      orxCommand_Unregister(pstCommand->zName);
    }

    /* Deletes table */
    orxHashTable_Delete(sstCommand.pstTable);

    /* Deletes banks */
    orxBank_Delete(sstCommand.pstBank);
    orxBank_Delete(sstCommand.pstTrieBank);
    orxBank_Delete(sstCommand.pstResultBank);

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
* @param[in]   _u32RequiredParamNumber Number of required parameters of the command
* @param[in]   _u32OptionalParamNumber Number of optional parameters of the command
* @param[in]   _astParamList  List of parameters of the command
* @param[in]   _pstResult     Result
* @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
*/
orxSTATUS orxFASTCALL orxCommand_Register(const orxSTRING _zCommand, const orxCOMMAND_FUNCTION _pfnFunction, orxU32 _u32RequiredParamNumber, orxU32 _u32OptionalParamNumber, const orxCOMMAND_VAR_DEF *_astParamList, const orxCOMMAND_VAR_DEF *_pstResult)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstCommand.u32Flags, orxCOMMAND_KU32_STATIC_FLAG_READY));
  orxASSERT(_zCommand != orxNULL);
  orxASSERT(_pfnFunction != orxNULL);
  orxASSERT(_u32RequiredParamNumber <= 0xFFFF);
  orxASSERT(_u32OptionalParamNumber <= 0xFFFF);
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
        pstCommand->pfnFunction             = _pfnFunction;
        pstCommand->stResult.zName          = orxString_Duplicate(_pstResult->zName);
        pstCommand->stResult.eType          = _pstResult->eType;
        pstCommand->zName                   = orxString_Duplicate(_zCommand);
        pstCommand->u16RequiredParamNumber  = (orxU16)_u32RequiredParamNumber;
        pstCommand->u16OptionalParamNumber  = (orxU16)_u32OptionalParamNumber;

        /* Allocates parameter list */
        pstCommand->astParamList = (orxCOMMAND_VAR_DEF *)orxMemory_Allocate((_u32RequiredParamNumber + _u32OptionalParamNumber) * sizeof(orxCOMMAND_VAR_DEF), orxMEMORY_TYPE_MAIN);

        /* Checks */
        orxASSERT(pstCommand->astParamList != orxNULL);

        /* For all parameters */
        for(i = 0; i < _u32RequiredParamNumber + _u32OptionalParamNumber; i++)
        {
          /* Inits it */
          pstCommand->astParamList[i].zName = orxString_Duplicate(_astParamList[i].zName);
          pstCommand->astParamList[i].eType = _astParamList[i].eType;
        }

        /* Adds it to the table */
        orxHashTable_Add(sstCommand.pstTable, u32ID, pstCommand);

        /* Inserts in trie */
        orxCommand_InsertInTrie(pstCommand);

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

      /* Removes it from trie */
      orxCommand_RemoveFromTrie(pstCommand);

      /* Removes it */
      eResult = orxHashTable_Remove(sstCommand.pstTable, u32ID);

      /* For all its parameters */
      for(i = 0; i < (orxU32)pstCommand->u16RequiredParamNumber + (orxU32)pstCommand->u16OptionalParamNumber; i++)
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

/** Gets a command's (text) prototype (beware: result won't persist from one call to the other)
* @param[in]   _zCommand      Command name
* @return      Command prototype / orxSTRING_EMPTY
*/
const orxSTRING orxFASTCALL orxCommand_GetPrototype(const orxSTRING _zCommand)
{
  const orxSTRING zResult = orxSTRING_EMPTY;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstCommand.u32Flags, orxCOMMAND_KU32_STATIC_FLAG_READY));
  orxASSERT(_zCommand != orxNULL);

  /* Valid? */
  if((_zCommand != orxNULL) & (_zCommand != orxSTRING_EMPTY))
  {
    orxU32      u32ID;
    orxCOMMAND *pstCommand;

    /* Gets its ID */
    u32ID = orxString_ToCRC(_zCommand);

    /* Gets command */
    pstCommand = (orxCOMMAND *)orxHashTable_Get(sstCommand.pstTable, u32ID);

    /* Success? */
    if(pstCommand != orxNULL)
    {
      orxU32 i, u32Size;

      /* Prints result and function name */
      u32Size = orxString_NPrint(sstCommand.acPrototypeBuffer, orxCOMMAND_KU32_PROTOTYPE_BUFFER_SIZE - 1, "{%s %s} %s", orxCommand_GetTypeString(pstCommand->stResult.eType), pstCommand->stResult.zName, pstCommand->zName);

      /* For all required arguments */
      for(i = 0; i < (orxU32)pstCommand->u16RequiredParamNumber; i++)
      {
        /* Prints it */
        u32Size += orxString_NPrint(sstCommand.acPrototypeBuffer + u32Size, orxCOMMAND_KU32_PROTOTYPE_BUFFER_SIZE - 1 - u32Size, " (%s %s)", orxCommand_GetTypeString(pstCommand->astParamList[i].eType), pstCommand->astParamList[i].zName);
      }

      /* For all optional arguments */
      for(; i < (orxU32)pstCommand->u16RequiredParamNumber + (orxU32)pstCommand->u16OptionalParamNumber; i++)
      {
        /* Prints it */
        u32Size += orxString_NPrint(sstCommand.acPrototypeBuffer + u32Size, orxCOMMAND_KU32_PROTOTYPE_BUFFER_SIZE - 1 - u32Size, " [%s %s]", orxCommand_GetTypeString(pstCommand->astParamList[i].eType), pstCommand->astParamList[i].zName);
      }

      /* Had no parameters? */
      if(i == 0)
      {
        /* Prints function end */
        u32Size += orxString_NPrint(sstCommand.acPrototypeBuffer + u32Size, orxCOMMAND_KU32_PROTOTYPE_BUFFER_SIZE - 1 - u32Size, " <void>");
      }

      /* Updates result */
      zResult = sstCommand.acPrototypeBuffer;
    }
  }

  /* Done! */
  return zResult;
}

/** Gets next command using an optional base
* @param[in]   _zBase         Base name, can be set to orxNULL for no base
* @param[in]   _zPrevious     Previous command, orxNULL to get the first command
* @return      Next command found, orxNULL if none
*/
const orxSTRING orxFASTCALL orxCommand_GetNext(const orxSTRING _zBase, const orxSTRING _zPrevious)
{
  const orxSTRING zResult = orxNULL;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstCommand.u32Flags, orxCOMMAND_KU32_STATIC_FLAG_READY));

  //! TODO

  /* Done! */
  return zResult;
}

/** Evaluates a command
* @param[in]   _zCommandLine  Command name + arguments
* @param[out]  _pstResult     Variable that will contain the result
* @return      Command result if found, orxNULL otherwise
*/
orxCOMMAND_VAR *orxFASTCALL orxCommand_Evaluate(const orxSTRING _zCommandLine, orxCOMMAND_VAR *_pstResult)
{
  orxCOMMAND_VAR *pstResult = orxNULL;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxCommand_Evaluate");

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstCommand.u32Flags, orxCOMMAND_KU32_STATIC_FLAG_READY));
  orxASSERT(_zCommandLine != orxNULL);
  orxASSERT(_pstResult != orxNULL);

  /* Valid? */
  if((_zCommandLine != orxNULL) & (_zCommandLine != orxSTRING_EMPTY))
  {
    /* Processes it */
    pstResult = orxCommand_Process(_zCommandLine, orxU64_UNDEFINED, _pstResult);
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

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

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxCommand_Execute");

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

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return pstResult;
}

#ifdef __orxMSVC__

  #pragma warning(default : 4200)

#endif /* __orxMSVC__ */
