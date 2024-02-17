/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008- Orx-Project
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
#include "anim/orxAnim.h"
#include "core/orxConsole.h"
#include "core/orxEvent.h"
#include "io/orxFile.h"
#include "memory/orxMemory.h"
#include "memory/orxBank.h"
#include "object/orxTimeLine.h"
#include "utils/orxString.h"
#include "utils/orxTree.h"

#ifdef __orxMSVC__

  #include <malloc.h>
  #pragma warning(disable : 4200)

#endif /* __orxMSVC__ */


/** Module flags
 */
#define orxCOMMAND_KU32_STATIC_FLAG_NONE              0x00000000                      /**< No flags */

#define orxCOMMAND_KU32_STATIC_FLAG_READY             0x00000001                      /**< Ready flag */

#define orxCOMMAND_KU32_STATIC_MASK_ALL               0xFFFFFFFF                      /**< All mask */


/** Misc
 */
#define orxCOMMAND_KU32_BANK_SIZE                     512
#define orxCOMMAND_KU32_TRIE_BANK_SIZE                4096
#define orxCOMMAND_KU32_RESULT_BANK_SIZE              32
#define orxCOMMAND_KU32_RESULT_BUFFER_SIZE            256

#define orxCOMMAND_KU32_STACK_ENTRY_BUFFER_SIZE       256

#define orxCOMMAND_KU32_EVALUATE_BUFFER_SIZE          65536
#define orxCOMMAND_KU32_PROCESS_BUFFER_SIZE           65536
#define orxCOMMAND_KU32_PROTOTYPE_BUFFER_SIZE         512

#define orxCOMMAND_KZ_ERROR_VALUE                     "ERROR"
#define orxCOMMAND_KZ_STACK_ERROR_VALUE               "STACK_ERROR"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Command stack entry
 */
typedef struct __orxCOMMAND_STACK_ENTRY_t
{
  orxCOMMAND_VAR            stValue;                                                  /**< Value : 28 */
  orxCHAR                   acBuffer[orxCOMMAND_KU32_STACK_ENTRY_BUFFER_SIZE];        /**< Buffer: 284 */

} orxCOMMAND_STACK_ENTRY;

/** Command variable info structure
 */
typedef struct __orxCOMMAND_VAR_INFO_t
{
  orxSTRINGID               stNameID;                                                 /**< Name ID : 4 */
  orxCOMMAND_VAR_TYPE       eType;                                                    /**< Type : 8 */

} orxCOMMAND_VAR_INFO;

/** Command structure
 */
typedef struct __orxCOMMAND_t
{
  orxSTRINGID               stNameID;                                                 /**< Name : 4 */
  orxBOOL                   bIsAlias;                                                 /**> Is an alias? : 8 */

  union
  {
    struct
    {
      orxSTRING             zAliasedCommandName;                                      /**< Aliased command name : 12 */
      orxSTRING             zArgs;                                                    /**< Arguments : 16 */
    };

    struct
    {
      orxCOMMAND_FUNCTION   pfnFunction;                                              /**< Function : 12 */
      orxCOMMAND_VAR_INFO   stResult;                                                 /**< Result definition : 20 */
      orxU16                u16RequiredParamNumber;                                   /**< Required param number : 22 */
      orxU16                u16OptionalParamNumber;                                   /**< Optional param number : 24 */
      orxCOMMAND_VAR_INFO  *astParamList;                                             /**< Param list : 28 */
    };
  };

} orxCOMMAND;

/** Command trie node
 */
typedef struct __orxCOMMAND_TRIE_NODE_t
{
  orxTREE_NODE              stNode;
  orxCOMMAND               *pstCommand;
  orxU32                    u32CharacterCodePoint;

} orxCOMMAND_TRIE_NODE;

/** Static structure
 */
typedef struct __orxCOMMAND_STATIC_t
{
  orxBANK                  *pstBank;                                                  /**< Command bank */
  orxBANK                  *pstTrieBank;                                              /**< Command trie bank */
  orxTREE                   stCommandTrie;                                            /**< Command trie */
  orxBANK                  *pstResultBank;                                            /**< Command result bank */
  orxCHAR                   acEvaluateBuffer[orxCOMMAND_KU32_EVALUATE_BUFFER_SIZE];   /**< Evaluate buffer */
  orxCHAR                   acProcessBuffer[orxCOMMAND_KU32_PROCESS_BUFFER_SIZE];     /**< Process buffer */
  orxCHAR                   acPrototypeBuffer[orxCOMMAND_KU32_PROTOTYPE_BUFFER_SIZE]; /**< Prototype buffer */
  orxCHAR                   acResultBuffer[orxCOMMAND_KU32_RESULT_BUFFER_SIZE];       /**< Result buffer */
  orxS32                    s32EvaluateOffset;                                        /**< Evaluate buffer offset */
  orxS32                    s32ProcessOffset;                                         /**< Process buffer offset */
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

/** Is character a white space?
 */
static orxINLINE orxBOOL orxCommand_IsWhiteSpace(orxCHAR _cChar)
{
  /* Done! */
  return ((_cChar == ' ') || (_cChar == '\t')) ? orxTRUE : orxFALSE;
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
    orxCOMMAND_DECLARE_TYPE_NAME(NUMERIC);

    default:
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_COMMAND, "No name defined for command var type #%d.", _eType);

      /* Updates result */
      zResult = orxSTRING_EMPTY;
    }
  }

#undef orxCOMMAND_DECLARE_TYPE_NAME

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
          orxDEBUG_PRINT(orxDEBUG_LEVEL_COMMAND, "Can't execute command [%s]: invalid type for argument #%d (%s).", orxString_GetFromID(_pstCommand->stNameID), i + 1, orxString_GetFromID(_pstCommand->astParamList[i].stNameID));

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

static orxINLINE orxCOMMAND_TRIE_NODE *orxCommand_FindTrieNode(const orxSTRING _zName, orxBOOL _bInsert)
{
  const orxSTRING       zName;
  orxU32                u32CharacterCodePoint;
  orxCOMMAND_TRIE_NODE *pstNode, *pstResult = orxNULL;

  /* Gets trie root */
  pstNode = (orxCOMMAND_TRIE_NODE *)orxTree_GetRoot(&(sstCommand.stCommandTrie));

  /* For all characters */
  for(u32CharacterCodePoint = orxString_GetFirstCharacterCodePoint(_zName, &zName);
      u32CharacterCodePoint != orxCHAR_NULL;
      u32CharacterCodePoint = orxString_GetFirstCharacterCodePoint(zName, &zName))
  {
    orxCOMMAND_TRIE_NODE *pstChild, *pstPrevious;

    /* Is an upper case ASCII character? */
    if((u32CharacterCodePoint <= 'Z')
    && (u32CharacterCodePoint >= 'A'))
    {
      /* Gets its lower case version */
      u32CharacterCodePoint |= 0x20;
    }

    /* Finds the matching place in children */
    for(pstPrevious = orxNULL, pstChild = (orxCOMMAND_TRIE_NODE *)orxTree_GetChild(&(pstNode->stNode));
        (pstChild != orxNULL) && (pstChild->u32CharacterCodePoint < u32CharacterCodePoint);
        pstPrevious = pstChild, pstChild = (orxCOMMAND_TRIE_NODE *)orxTree_GetSibling(&(pstChild->stNode)));

    /* Not found? */
    if((pstChild == orxNULL)
    || (pstChild->u32CharacterCodePoint != u32CharacterCodePoint))
    {
      /* Insertion allowed? */
      if(_bInsert != orxFALSE)
      {
        /* Creates new trie node */
        pstChild = (orxCOMMAND_TRIE_NODE *)orxBank_Allocate(sstCommand.pstTrieBank);

        /* Checks */
        orxASSERT(pstChild != orxNULL);

        /* Inits it */
        orxMemory_Zero(pstChild, sizeof(orxCOMMAND_TRIE_NODE));

        /* Has previous? */
        if(pstPrevious != orxNULL)
        {
          /* Inserts it as sibling */
          orxTree_AddSibling(&(pstPrevious->stNode), &(pstChild->stNode));
        }
        else
        {
          /* Inserts it as child */
          orxTree_AddChild(&(pstNode->stNode), &(pstChild->stNode));
        }

        /* Stores character code point */
        pstChild->u32CharacterCodePoint = u32CharacterCodePoint;
      }
      else
      {
        /* Stops search */
        break;
      }
    }

    /* End of name? */
    if(*zName == orxCHAR_NULL)
    {
      /* Updates result */
      pstResult = pstChild;

      break;
    }
    else
    {
      /* Stores next node */
      pstNode = pstChild;
    }
  }

  /* Done! */
  return pstResult;
}

static orxINLINE orxCOMMAND *orxCommand_FindNoAlias(const orxSTRING _zCommand)
{
  orxCOMMAND_TRIE_NODE *pstNode;
  orxCOMMAND           *pstResult;

  /* Finds right command */
  for(pstNode = orxCommand_FindTrieNode(_zCommand, orxFALSE);
      (pstNode != orxNULL) && (pstNode->pstCommand != orxNULL) && (pstNode->pstCommand->bIsAlias != orxFALSE);
      pstNode = orxCommand_FindTrieNode(pstNode->pstCommand->zAliasedCommandName, orxFALSE));

  /* Updates result */
  pstResult = (pstNode != orxNULL) ? pstNode->pstCommand : orxNULL;

  /* Done! */
  return pstResult;
}

static orxINLINE void orxCommand_InsertInTrie(orxCOMMAND *_pstCommand)
{
  orxCOMMAND_TRIE_NODE *pstNode;

  /* Gets command trie node */
  pstNode = orxCommand_FindTrieNode(orxString_GetFromID(_pstCommand->stNameID), orxTRUE);

  /* Checks */
  orxASSERT(pstNode != orxNULL);
  orxASSERT(pstNode->pstCommand == orxNULL);

  /* Inserts command */
  pstNode->pstCommand = _pstCommand;

  /* Done! */
  return;
}

static orxINLINE void orxCommand_RemoveFromTrie(orxCOMMAND *_pstCommand)
{
  orxCOMMAND_TRIE_NODE *pstNode;

  /* Finds command trie node */
  pstNode = orxCommand_FindTrieNode(orxString_GetFromID(_pstCommand->stNameID), orxFALSE);

  /* Checks */
  orxASSERT(pstNode != orxNULL);
  orxASSERT(pstNode->pstCommand == _pstCommand);

  /* Removes command */
  pstNode->pstCommand = orxNULL;

  /* Done! */
  return;
}

static orxINLINE const orxCOMMAND *orxCommand_FindNext(const orxCOMMAND_TRIE_NODE *_pstNode, orxCOMMAND_TRIE_NODE **_ppstPreviousNode)
{
  const orxCOMMAND *pstResult = orxNULL;

  /* Valid node? */
  if(_pstNode != orxNULL)
  {
    /* Has reached previous command node and found a new command? */
    if((*_ppstPreviousNode == orxNULL)
    && (_pstNode->pstCommand != orxNULL))
    {
      /* Updates result */
      pstResult = _pstNode->pstCommand;
    }
    else
    {
      /* Passed previous command node? */
      if(*_ppstPreviousNode == _pstNode)
      {
        /* Resets previous command node value */
        *_ppstPreviousNode = orxNULL;
      }

      /* Finds next command from child */
      pstResult = orxCommand_FindNext((orxCOMMAND_TRIE_NODE *)orxTree_GetChild(&(_pstNode->stNode)), _ppstPreviousNode);

      /* No command found? */
      if(pstResult == orxNULL)
      {
        /* Finds next command from sibling */
        pstResult = orxCommand_FindNext((orxCOMMAND_TRIE_NODE *)orxTree_GetSibling(&(_pstNode->stNode)), _ppstPreviousNode);
      }
    }
  }

  /* Done! */
  return pstResult;
}

static orxCOMMAND_VAR *orxFASTCALL orxCommand_Process(const orxSTRING _zCommandLine, const orxU64 _u64GUID, orxCOMMAND_VAR *_pstResult, orxBOOL _bSilent)
{
  const orxSTRING zCommand;
  orxBOOL         bProcessed = orxFALSE;
  orxCOMMAND_VAR *pstResult = orxNULL;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxCommand_Process");

  /* Gets start of command */
  zCommand = orxString_SkipWhiteSpaces(_zCommandLine);

  /* For all commands */
  while(*zCommand != orxCHAR_NULL)
  {
    orxU32          u32PushCount;
    const orxCHAR  *pcCommandEnd;
    orxCOMMAND     *pstCommand;
    orxCHAR         cBackupChar;

    /* Updates status */
    bProcessed = orxTRUE;

    /* For all push markers / spaces */
    for(u32PushCount = 0; (*zCommand == orxCOMMAND_KC_PUSH_MARKER) || (orxCommand_IsWhiteSpace(*zCommand) != orxFALSE); zCommand++)
    {
      /* Is a push marker? */
      if(*zCommand == orxCOMMAND_KC_PUSH_MARKER)
      {
        /* Updates push count */
        u32PushCount++;
      }
    }

    /* Finds end of command */
    for(pcCommandEnd = zCommand + 1; (*pcCommandEnd != orxCHAR_NULL) && (orxCommand_IsWhiteSpace(*pcCommandEnd) == orxFALSE) && (*pcCommandEnd != orxCHAR_CR) && (*pcCommandEnd != orxCHAR_LF) && (*pcCommandEnd != orxCOMMAND_KC_SEPARATOR); pcCommandEnd++)
      ;

    /* Ends command */
    cBackupChar               = *pcCommandEnd;
    *(orxCHAR *)pcCommandEnd  = orxCHAR_NULL;

    /* Gets it */
    pstCommand = orxCommand_FindNoAlias(zCommand);

    /* Found? */
    if(pstCommand != orxNULL)
    {
#define orxCOMMAND_KU32_ALIAS_MAX_DEPTH             32
      orxSTATUS             eStatus;
      orxS32                s32GUIDLength, s32BufferCount = 0, s32VectorDepth = 0, s32Offset, i;
      orxBOOL               bInBlock = orxFALSE;
      orxCOMMAND_TRIE_NODE *pstCommandNode;
      const orxCHAR        *pcSrc;
      orxCHAR              *pcDst;
      const orxSTRING       zArg;
      const orxSTRING       azBufferList[orxCOMMAND_KU32_ALIAS_MAX_DEPTH];
      orxU32                u32ArgNumber, u32ParamNumber = (orxU32)pstCommand->u16RequiredParamNumber + (orxU32)pstCommand->u16OptionalParamNumber;
      orxCHAR               acGUID[20];

#ifdef __orxMSVC__

      orxCOMMAND_VAR *astArgList = (orxCOMMAND_VAR *)alloca(u32ParamNumber * sizeof(orxCOMMAND_VAR));

#else /* __orxMSVC__ */

      orxCOMMAND_VAR astArgList[u32ParamNumber];

#endif /* __orxMSVC__ */

      /* Is GUID valid? */
      if(_u64GUID != orxU64_UNDEFINED)
      {
        /* Gets owner's GUID */
        s32GUIDLength = orxString_NPrint(acGUID, sizeof(acGUID), "0x%016llX", _u64GUID);
      }
      else
      {
        /* No GUID */
        s32GUIDLength = 0;
      }

      /* Adds input to the buffer list */
      azBufferList[s32BufferCount++] = pcCommandEnd;

      /* For all alias nodes */
      for(pstCommandNode = orxCommand_FindTrieNode(zCommand, orxFALSE);
          (pstCommandNode->pstCommand->bIsAlias != orxFALSE) && (s32BufferCount < orxCOMMAND_KU32_ALIAS_MAX_DEPTH);
          pstCommandNode = orxCommand_FindTrieNode(pstCommandNode->pstCommand->zAliasedCommandName, orxFALSE))
      {
        /* Has args? */
        if(pstCommandNode->pstCommand->zArgs != orxNULL)
        {
          /* Adds it to the buffer list */
          azBufferList[s32BufferCount++] = pstCommandNode->pstCommand->zArgs;
        }
      }

      /* Restores command end */
      *(orxCHAR *)pcCommandEnd = cBackupChar;

      /* For all stacked buffers */
      for(i = s32BufferCount - 1, pcDst = sstCommand.acProcessBuffer + sstCommand.s32ProcessOffset; i >= 0; i--)
      {
        orxBOOL bStop;

        /* Has room for next buffer? */
        if((i != s32BufferCount - 1) && (*azBufferList[i] != orxCHAR_NULL) && (pcDst - sstCommand.acProcessBuffer < orxCOMMAND_KU32_PROCESS_BUFFER_SIZE - 2))
        {
          /* Inserts space */
          *pcDst++ = ' ';
        }

        /* For all characters */
        for(pcSrc = azBufferList[i], bStop = orxFALSE; (bStop == orxFALSE) && (*pcSrc != orxCHAR_NULL) && (pcDst - sstCommand.acProcessBuffer < orxCOMMAND_KU32_PROCESS_BUFFER_SIZE - 2); pcSrc++)
        {
          /* Depending on character */
          switch(*pcSrc)
          {
            case orxCOMMAND_KC_GUID_MARKER:
            {
              /* Has valid GUID? */
              if(s32GUIDLength != 0)
              {
                /* Replaces it with GUID */
                orxString_NCopy(pcDst, acGUID, orxCOMMAND_KU32_PROCESS_BUFFER_SIZE - 1 - (orxU32)(pcDst - sstCommand.acProcessBuffer));

                /* Updates pointer */
                pcDst += s32GUIDLength;
              }
              else
              {
                /* Copies character */
                *pcDst++ = *pcSrc;
              }

              break;
            }

            case orxCOMMAND_KC_POP_MARKER:
            {
              /* Valid? */
              if(orxBank_GetCount(sstCommand.pstResultBank) > 0)
              {
                orxCOMMAND_STACK_ENTRY *pstEntry;
                orxCHAR                 acValue[64];
                orxBOOL                 bUseStringMarker = orxFALSE;
                const orxSTRING         zValue = acValue;

                /* Gets last stack entry */
                pstEntry = (orxCOMMAND_STACK_ENTRY *)orxBank_GetAtIndex(sstCommand.pstResultBank, orxBank_GetCount(sstCommand.pstResultBank) - 1);

                /* Inits value */
                acValue[sizeof(acValue) - 1] = orxCHAR_NULL;

                /* Depending on type */
                switch(pstEntry->stValue.eType)
                {
                  case orxCOMMAND_VAR_TYPE_STRING:
                  {
                    /* Updates pointer */
                    zValue = pstEntry->stValue.zValue;

                    /* Is not in block? */
                    if(bInBlock == orxFALSE)
                    {
                      const orxCHAR *pc = zValue;

                      /* For all characters */
                      do
                      {
                        /* Is a white space? */
                        if((*pc == orxCHAR_NULL) || (orxCommand_IsWhiteSpace(*pc) != orxFALSE))
                        {
                          /* Has room? */
                          if(pcDst - sstCommand.acProcessBuffer < orxCOMMAND_KU32_PROCESS_BUFFER_SIZE - 1)
                          {
                            /* Adds block marker */
                            *pcDst++ = orxCOMMAND_KC_BLOCK_MARKER;

                            /* Updates string marker status */
                            bUseStringMarker = orxTRUE;
                          }

                          break;
                        }

                        /* Gets next character */
                        pc++;

                      } while(*pc != orxCHAR_NULL);
                    }

                    break;
                  }

                  default:
                  {
                    orxCommand_PrintVar(acValue, sizeof(acValue), &(pstEntry->stValue));

                    break;
                  }
                }

                /* Replaces marker with stacked value */
                orxString_NCopy(pcDst, zValue, orxCOMMAND_KU32_PROCESS_BUFFER_SIZE - 1 - (orxU32)(pcDst - sstCommand.acProcessBuffer));

                /* Updates pointers */
                pcDst += orxString_GetLength(zValue);

                /* Used a string marker? */
                if(bUseStringMarker != orxFALSE)
                {
                  /* Has room? */
                  if(pcDst - sstCommand.acProcessBuffer < orxCOMMAND_KU32_PROCESS_BUFFER_SIZE - 1)
                  {
                    *pcDst++ = orxCOMMAND_KC_BLOCK_MARKER;
                  }
                }

                /* Deletes stack entry */
                orxBank_Free(sstCommand.pstResultBank, pstEntry);
              }
              else
              {
                /* Logs message */
                orxDEBUG_PRINT(orxDEBUG_LEVEL_COMMAND, "Can't pop stacked argument for command line [%s]: stack is empty.", _zCommandLine);

                /* Replaces marker with stack error */
                orxString_NCopy(pcDst, orxCOMMAND_KZ_STACK_ERROR_VALUE, orxCOMMAND_KU32_PROCESS_BUFFER_SIZE - 1 - (orxU32)(pcDst - sstCommand.acProcessBuffer));

                /* Updates pointers */
                pcDst += orxString_GetLength(orxCOMMAND_KZ_ERROR_VALUE);
              }

              break;
            }

            case orxCOMMAND_KC_SEPARATOR:
            case orxCHAR_CR:
            case orxCHAR_LF:
            {
              /* Not in block or in vector? */
              if((bInBlock == orxFALSE)
              && (s32VectorDepth == 0))
              {
                /* Stops */
                bStop = orxTRUE;
              }
              else
              {
                /* Copies it */
                *pcDst++ = *pcSrc;
              }

              break;
            }

            case orxSTRING_KC_VECTOR_START:
            case orxSTRING_KC_VECTOR_START_ALT:
            {
              /* Increments vector depth */
              s32VectorDepth++;

              /* Copies it */
              *pcDst++ = *pcSrc;

              break;
            }

            case orxSTRING_KC_VECTOR_END:
            case orxSTRING_KC_VECTOR_END_ALT:
            {
              /* Decrements vector depth */
              s32VectorDepth--;

              /* Copies it */
              *pcDst++ = *pcSrc;

              break;
            }

            case orxCOMMAND_KC_BLOCK_MARKER:
            {
              /* Toggles block status */
              bInBlock = !bInBlock;

              /* Fall through */
            }

            default:
            {
              /* Copies it */
              *pcDst++ = *pcSrc;

              break;
            }
          }
        }
      }

      /* Copies end of string */
      *pcDst = orxCHAR_NULL;

      /* Gets new additional offset */
      s32Offset = (orxS32)(pcDst + 1 - sstCommand.acProcessBuffer - sstCommand.s32ProcessOffset);

      /* Updates next command expression */
      zCommand = orxString_SkipWhiteSpaces(pcSrc);

      /* For all characters in the buffer */
      for(pcSrc = sstCommand.acProcessBuffer + sstCommand.s32ProcessOffset, eStatus = orxSTATUS_SUCCESS, zArg = orxSTRING_EMPTY, u32ArgNumber = 0;
          (u32ArgNumber < u32ParamNumber) && (pcSrc - sstCommand.acProcessBuffer < orxCOMMAND_KU32_PROCESS_BUFFER_SIZE) && (*pcSrc != orxCHAR_NULL);
          pcSrc++, u32ArgNumber++)
      {
        /* Skips all whitespaces */
        pcSrc = orxString_SkipWhiteSpaces(pcSrc);

        /* Valid? */
        if(*pcSrc != orxCHAR_NULL)
        {
          orxBOOL bInBlock = orxFALSE, bUseDefault = orxFALSE;

          /* Inits vector depth */
          s32VectorDepth = ((*pcSrc == orxSTRING_KC_VECTOR_START) || (*pcSrc == orxSTRING_KC_VECTOR_START_ALT)) ? 0 : -1;

          /* Gets arg's beginning */
          zArg = pcSrc;

          /* Is a block marker? */
          if(*pcSrc == orxCOMMAND_KC_BLOCK_MARKER)
          {
            /* Updates arg pointer */
            zArg++;
            pcSrc++;

            /* Is an empty block? */
            if((*pcSrc == orxCOMMAND_KC_BLOCK_MARKER)
            && (*(pcSrc + 1) != orxCOMMAND_KC_BLOCK_MARKER))
            {
              /* Uses default */
              bUseDefault = orxTRUE;

              /* Updates arg pointer */
              zArg++;
              pcSrc++;
            }
            else
            {
              /* Updates block status */
              bInBlock = orxTRUE;
            }
          }

          /* Stores its type */
          astArgList[u32ArgNumber].eType = pstCommand->astParamList[u32ArgNumber].eType;

          /* Depending on its type */
          switch(pstCommand->astParamList[u32ArgNumber].eType)
          {
            case orxCOMMAND_VAR_TYPE_NUMERIC:
            {
              /* Should use default? */
              if(bUseDefault != orxFALSE)
              {
                /* Replaces block end marker with 0 */
                pcSrc--;
                zArg--;
                *((orxCHAR *)pcSrc) = '0';

                /* Fall through */
              }
              /* Not in block? */
              else if(bInBlock == orxFALSE)
              {
                orxVECTOR vValue;

                /* Is a vector */
                if(orxString_ToVector(zArg, &vValue, &pcSrc) != orxSTATUS_FAILURE)
                {
                  /* Stores its value */
                  astArgList[u32ArgNumber].zValue = zArg;

                  /* Stops */
                  break;
                }
                else
                {
                  /* Fall through */
                }
              }
            }

            default:
            case orxCOMMAND_VAR_TYPE_STRING:
            {
              /* For all argument characters */
              for(; *pcSrc != orxCHAR_NULL; pcSrc++)
              {
                orxCHAR *pcTemp;

                /* Depending on character */
                switch(*pcSrc)
                {
                  case orxSTRING_KC_VECTOR_START:
                  case orxSTRING_KC_VECTOR_START_ALT:
                  {
                    /* Should update? */
                    if(s32VectorDepth >= 0)
                    {
                      /* Increments vector depth */
                      s32VectorDepth++;
                    }
                    break;
                  }

                  case orxSTRING_KC_VECTOR_END:
                  case orxSTRING_KC_VECTOR_END_ALT:
                  {
                    /* Should update? */
                    if(s32VectorDepth >= 0)
                    {
                      /* Decrements vector depth */
                      s32VectorDepth--;
                    }
                    break;
                  }

                  case orxCOMMAND_KC_BLOCK_MARKER:
                  {
                    /* Erases it */
                    for(pcTemp = (orxCHAR *)pcSrc; *pcTemp != orxCHAR_NULL; pcTemp++)
                    {
                      *pcTemp = *(pcTemp + 1);
                    }

                    /* Not double marker? */
                    if(*pcSrc != orxCOMMAND_KC_BLOCK_MARKER)
                    {
                      /* Updates block status */
                      bInBlock = !bInBlock;

                      /* Handles current character in new mode */
                      pcSrc--;
                    }
                    continue;
                  }
                  default:
                  {
                    break;
                  }
                }

                /* Not in block or in vector? */
                if((bInBlock == orxFALSE) && (s32VectorDepth <= 0))
                {
                  /* End of string? */
                  if(orxCommand_IsWhiteSpace(*pcSrc) != orxFALSE)
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
              /* Should use default? */
              if(bUseDefault != orxFALSE)
              {
                /* Uses default value */
                astArgList[u32ArgNumber].fValue = orxFLOAT_0;
              }
              else
              {
                /* Gets its value */
                eStatus = orxString_ToFloat(zArg, &(astArgList[u32ArgNumber].fValue), &pcSrc);
              }

              break;
            }

            case orxCOMMAND_VAR_TYPE_S32:
            {
              /* Should use default? */
              if(bUseDefault != orxFALSE)
              {
                /* Uses default value */
                astArgList[u32ArgNumber].s32Value = 0;
              }
              else
              {
                /* Gets its value */
                eStatus = orxString_ToS32(zArg, &(astArgList[u32ArgNumber].s32Value), &pcSrc);
              }

              break;
            }

            case orxCOMMAND_VAR_TYPE_U32:
            {
              /* Should use default? */
              if(bUseDefault != orxFALSE)
              {
                /* Uses default value */
                astArgList[u32ArgNumber].u32Value = 0;
              }
              else
              {
                /* Gets its value */
                eStatus = orxString_ToU32(zArg, &(astArgList[u32ArgNumber].u32Value), &pcSrc);
              }

              break;
            }

            case orxCOMMAND_VAR_TYPE_S64:
            {
              /* Should use default? */
              if(bUseDefault != orxFALSE)
              {
                /* Uses default value */
                astArgList[u32ArgNumber].s64Value = 0;
              }
              else
              {
                /* Gets its value */
                eStatus = orxString_ToS64(zArg, &(astArgList[u32ArgNumber].s64Value), &pcSrc);
              }

              break;
            }

            case orxCOMMAND_VAR_TYPE_U64:
            {
              /* Should use default? */
              if(bUseDefault != orxFALSE)
              {
                /* Uses default value */
                astArgList[u32ArgNumber].u64Value = 0;
              }
              else
              {
                /* Gets its value */
                eStatus = orxString_ToU64(zArg, &(astArgList[u32ArgNumber].u64Value), &pcSrc);
              }

              break;
            }

            case orxCOMMAND_VAR_TYPE_BOOL:
            {
              /* Should use default? */
              if(bUseDefault != orxFALSE)
              {
                /* Uses default value */
                astArgList[u32ArgNumber].bValue = orxFALSE;
              }
              else
              {
                /* Gets its value */
                eStatus = orxString_ToBool(zArg, &(astArgList[u32ArgNumber].bValue), &pcSrc);
              }

              break;
            }

            case orxCOMMAND_VAR_TYPE_VECTOR:
            {
              /* Should use default? */
              if(bUseDefault != orxFALSE)
              {
                /* Uses default value */
                orxVector_Copy(&(astArgList[u32ArgNumber].vValue), &orxVECTOR_0);
              }
              else
              {
                /* Gets its value */
                eStatus = orxString_ToVector(zArg, &(astArgList[u32ArgNumber].vValue), &pcSrc);
              }

              break;
            }
          }

          /* Interrupted? */
          if((eStatus == orxSTATUS_FAILURE) || (*pcSrc == orxCHAR_NULL))
          {
            /* Updates argument count */
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
        else
        {
          /* Stops */
          break;
        }
      }

      /* Error? */
      if((eStatus == orxSTATUS_FAILURE) || (u32ArgNumber < (orxU32)pstCommand->u16RequiredParamNumber))
      {
        /* Incorrect parameter? */
        if(eStatus == orxSTATUS_FAILURE)
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_COMMAND, "Can't evaluate command line [%s], invalid argument #%d for command [%s].", _zCommandLine, u32ArgNumber, orxString_GetFromID(pstCommand->stNameID));
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_COMMAND, "Can't evaluate command line [%s], expected %d[+%d] arguments for command [%s], found %d.", _zCommandLine, (orxU32)pstCommand->u16RequiredParamNumber, (orxU32)pstCommand->u16OptionalParamNumber, orxString_GetFromID(pstCommand->stNameID), u32ArgNumber);
        }
      }
      else
      {
        /* Runs it */
        sstCommand.s32ProcessOffset += s32Offset;
        pstResult = orxCommand_Run(pstCommand, orxFALSE, u32ArgNumber, astArgList, _pstResult);
        sstCommand.s32ProcessOffset -= s32Offset;
      }
#undef orxCOMMAND_KU32_ALIAS_MAX_DEPTH
    }
    else
    {
      /* Restores command end */
      *(orxCHAR *)pcCommandEnd = cBackupChar;

      /* Not silent? */
      if(_bSilent == orxFALSE)
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_COMMAND, "Can't evaluate command line [%s]: [%s] is not a registered command.", _zCommandLine, zCommand);
      }

      /* Stops */
      zCommand = orxSTRING_EMPTY;
    }

    /* Failure? */
    if(pstResult == orxNULL)
    {
      /* Stores error */
      _pstResult->eType   = orxCOMMAND_VAR_TYPE_STRING;
      _pstResult->zValue  = orxCOMMAND_KZ_ERROR_VALUE;
    }

    /* For all requested pushes */
    while(u32PushCount > 0)
    {
      orxCOMMAND_STACK_ENTRY *pstEntry;

      /* Allocates stack entry */
      pstEntry = (orxCOMMAND_STACK_ENTRY *)orxBank_Allocate(sstCommand.pstResultBank);

      /* Checks */
      orxASSERT(pstEntry != orxNULL);

      /* Is a string or numeric value? */
      if((_pstResult->eType == orxCOMMAND_VAR_TYPE_STRING)
      || (_pstResult->eType == orxCOMMAND_VAR_TYPE_NUMERIC))
      {
        /* Checks */
        orxASSERT(orxString_GetLength(_pstResult->zValue) < orxCOMMAND_KU32_STACK_ENTRY_BUFFER_SIZE);

        /* Duplicates it */
        pstEntry->stValue.eType   = _pstResult->eType;
        pstEntry->stValue.zValue  = pstEntry->acBuffer;
        orxString_NCopy(pstEntry->acBuffer, _pstResult->zValue, sizeof(pstEntry->acBuffer) - 1);
        pstEntry->acBuffer[sizeof(pstEntry->acBuffer) - 1] = orxCHAR_NULL;
      }
      else
      {
        /* Stores value */
        orxMemory_Copy(&(pstEntry->stValue), _pstResult, sizeof(orxCOMMAND_VAR));
      }

      /* Updates push count */
      u32PushCount--;
    }
  }

  /* Unprocessed? */
  if(bProcessed == orxFALSE)
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_COMMAND, "Can't evaluate command line [%s]: [%s] is not a registered command.", _zCommandLine, zCommand);
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

  /* Depends on event type */
  switch(_pstEvent->eType)
  {
    case orxEVENT_TYPE_TIMELINE:
    {
      /* Depending on event ID */
      switch(_pstEvent->eID)
      {
        /* Timeline Trigger */
        case orxTIMELINE_EVENT_TRIGGER:
        {
          orxCOMMAND_VAR              stResult;
          orxTIMELINE_EVENT_PAYLOAD  *pstPayload;

          /* Gets payload */
          pstPayload = (orxTIMELINE_EVENT_PAYLOAD *)_pstEvent->pstPayload;

          /* Processes command */
          orxCommand_Process(pstPayload->zEvent, orxStructure_GetGUID(orxSTRUCTURE(_pstEvent->hSender)), &stResult, orxTRUE);

          break;
        }

        default:
        {
          break;
        }
      }

      break;
    }

    case orxEVENT_TYPE_ANIM:
    {
      /* Depending on event ID */
      switch(_pstEvent->eID)
      {
        /* Anim Custom Event */
        case orxANIM_EVENT_CUSTOM_EVENT:
        {
          orxCOMMAND_VAR          stResult;
          orxANIM_EVENT_PAYLOAD  *pstPayload;

          /* Gets payload */
          pstPayload = (orxANIM_EVENT_PAYLOAD *)_pstEvent->pstPayload;

          /* Processes command */
          orxCommand_Process(pstPayload->stCustom.zName, orxStructure_GetGUID(orxSTRUCTURE(_pstEvent->hSender)), &stResult, orxTRUE);

          break;
        }

        default:
        {
          break;
        }
      }

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

/** Command: Exit
 */
void orxFASTCALL orxCommand_CommandExit(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* No argument? */
  if(_u32ArgNumber == 0)
  {
    /* Updates result */
    _pstResult->zValue = "Exiting.";
  }
  else
  {
    /* Prints value */
    orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "Exiting: %s.", _astArgList[0].zValue);

    /* Updates result */
    _pstResult->zValue = sstCommand.acResultBuffer;
  }

  /* Sends close event */
  orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_CLOSE);

  /* Done! */
  return;
}

/** Command: ListCommands
 */
void orxFASTCALL orxCommand_CommandListCommands(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  const orxSTRING zPrefix;
  const orxSTRING zCommand;
  orxU32          u32Count, u32Flags;

  /* Backups debug flags */
  u32Flags = orxDEBUG_GET_FLAGS();

  /* Deactivates all tagging */
  orxDEBUG_SET_FLAGS(orxDEBUG_KU32_STATIC_FLAG_NONE, orxDEBUG_KU32_STATIC_FLAG_TIMESTAMP|orxDEBUG_KU32_STATIC_FLAG_FULL_TIMESTAMP|orxDEBUG_KU32_STATIC_FLAG_TYPE|orxDEBUG_KU32_STATIC_FLAG_TAGGED);

  /* Logs header */
  orxLOG("Listing %scommands%s%s%s:", (_u32ArgNumber > 0) ? orxSTRING_EMPTY : "all ", (_u32ArgNumber > 0) ? " with prefix [" : orxSTRING_EMPTY, (_u32ArgNumber > 0) ? _astArgList[0].zValue : orxSTRING_EMPTY, (_u32ArgNumber > 0) ? "]" : orxSTRING_EMPTY);

  /* Gets prefix */
  zPrefix = (_u32ArgNumber > 0) ? _astArgList[0].zValue : orxNULL;

  /* For all commands */
  for(zCommand = orxNULL, zCommand = (orxCommand_IsRegistered(zPrefix) != orxFALSE) ? zPrefix : orxCommand_GetNext(zPrefix, zCommand, orxNULL), u32Count = 0;
      zCommand != orxNULL;
      zCommand = orxCommand_GetNext(zPrefix, zCommand, orxNULL))
  {
    /* Is a command? */
    if(orxCommand_IsAlias(zCommand) == orxFALSE)
    {
      /* Logs it */
      orxLOG(zCommand);

      /* Updates count */
      u32Count++;
    }
  }

  /* Restores debug flags */
  orxDEBUG_SET_FLAGS(u32Flags, orxDEBUG_KU32_STATIC_MASK_USER_ALL);

  /* Updates result */
  _pstResult->u32Value = u32Count;

  /* Done! */
  return;
}

/* Command: AddAlias */
void orxFASTCALL orxCommand_CommandAddAlias(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  const orxSTRING zArgs;

  /* Has arguments? */
  if(_u32ArgNumber > 2)
  {
    /* Uses them */
    zArgs = _astArgList[2].zValue;
  }
  else
  {
    /* No args */
     zArgs = orxNULL;
  }

  /* Adds alias */
  if(orxCommand_AddAlias(_astArgList[0].zValue, _astArgList[1].zValue, zArgs) != orxSTATUS_FAILURE)
  {
    /* Updates result */
    _pstResult->zValue = _astArgList[0].zValue;
  }
  else
  {
    /* Updates result */
    _pstResult->zValue = orxSTRING_EMPTY;
  }

  /* Done! */
  return;
}
/* Command: RemoveAlias */
void orxFASTCALL orxCommand_CommandRemoveAlias(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Removes alias */
  if(orxCommand_RemoveAlias(_astArgList[0].zValue) != orxSTATUS_FAILURE)
  {
    /* Updates result */
    _pstResult->zValue = _astArgList[0].zValue;
  }
  else
  {
    /* Updates result */
    _pstResult->zValue = orxSTRING_EMPTY;
  }

  /* Done! */
  return;
}

/** Command: ListAliases
 */
void orxFASTCALL orxCommand_CommandListAliases(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  const orxSTRING zPrefix;
  const orxSTRING zAlias;
  orxU32          u32Count, u32Flags;

  /* Backups debug flags */
  u32Flags = orxDEBUG_GET_FLAGS();

  /* Deactivates all tagging */
  orxDEBUG_SET_FLAGS(orxDEBUG_KU32_STATIC_FLAG_NONE, orxDEBUG_KU32_STATIC_FLAG_TIMESTAMP|orxDEBUG_KU32_STATIC_FLAG_FULL_TIMESTAMP|orxDEBUG_KU32_STATIC_FLAG_TYPE|orxDEBUG_KU32_STATIC_FLAG_TAGGED);

  /* Logs header */
  orxLOG("Listing %saliases%s%s%s:", (_u32ArgNumber > 0) ? orxSTRING_EMPTY : "all ", (_u32ArgNumber > 0) ? " with prefix [" : orxSTRING_EMPTY, (_u32ArgNumber > 0) ? _astArgList[0].zValue : orxSTRING_EMPTY, (_u32ArgNumber > 0) ? "]" : orxSTRING_EMPTY);

  /* Gets prefix */
  zPrefix = (_u32ArgNumber > 0) ? _astArgList[0].zValue : orxNULL;

  /* For all commands */
  for(zAlias = orxNULL, zAlias = (orxCommand_IsAlias(zPrefix) != orxFALSE) ? zPrefix : orxCommand_GetNext(zPrefix, zAlias, orxNULL), u32Count = 0;
      zAlias != orxNULL;
      zAlias = orxCommand_GetNext(zPrefix, zAlias, orxNULL))
  {
    /* Is an alias? */
    if(orxCommand_IsAlias(zAlias) != orxFALSE)
    {
      orxCOMMAND_TRIE_NODE *pstAliasNode, *pstCommandNode;
      orxCHAR               acBuffer[256];

      /* Finds alias node */
      pstAliasNode = orxCommand_FindTrieNode(zAlias, orxFALSE);

      /* Checks */
      orxASSERT((pstAliasNode != orxNULL) && (pstAliasNode->pstCommand != orxNULL));

      /* Finds aliased node */
      pstCommandNode = orxCommand_FindTrieNode(pstAliasNode->pstCommand->zAliasedCommandName, orxFALSE);

      /* Valid? */
      if((pstCommandNode != orxNULL) && (pstCommandNode->pstCommand != orxNULL))
      {
        /* Has args? */
        if(pstAliasNode->pstCommand->zArgs != orxNULL)
        {
          /* Writes log */
          orxString_NPrint(acBuffer, sizeof(acBuffer), "%s -> %s +<%s> [%s]", zAlias, orxString_GetFromID(pstCommandNode->pstCommand->stNameID), pstAliasNode->pstCommand->zArgs, (pstCommandNode->pstCommand->bIsAlias != orxFALSE) ? "ALIAS" : "COMMAND");
        }
        else
        {
          /* Writes log */
          orxString_NPrint(acBuffer, sizeof(acBuffer), "%s -> %s [%s]", zAlias, orxString_GetFromID(pstCommandNode->pstCommand->stNameID), (pstCommandNode->pstCommand->bIsAlias != orxFALSE) ? "ALIAS" : "COMMAND");
        }
      }
      else
      {
        /* Writes log */
        orxString_NPrint(acBuffer, sizeof(acBuffer), "%s -> %s [UNBOUND]", zAlias, pstAliasNode->pstCommand->zAliasedCommandName);
      }

      /* Logs it */
      orxLOG(acBuffer);

      /* Updates count */
      u32Count++;
    }
  }

  /* Restores debug flags */
  orxDEBUG_SET_FLAGS(u32Flags, orxDEBUG_KU32_STATIC_MASK_USER_ALL);

  /* Updates result */
  _pstResult->u32Value = u32Count;

  /* Done! */
  return;
}

/* Command: Evaluate */
void orxFASTCALL orxCommand_CommandEvaluate(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Disables marker operations */
  orxProfiler_EnableMarkerOperations(orxFALSE);

  /* Has GUID? */
  if(_u32ArgNumber > 1)
  {
    /* Evaluates command with GUID */
    orxCommand_EvaluateWithGUID(_astArgList[0].zValue, _astArgList[1].u64Value, _pstResult);
  }
  else
  {
    /* Evaluates command */
    orxCommand_Evaluate(_astArgList[0].zValue, _pstResult);
  }

  /* Re-enables marker operations */
  orxProfiler_EnableMarkerOperations(orxTRUE);

  /* Done! */
  return;
}

/* Command: EvaluateIf  */
void orxFASTCALL orxCommand_CommandEvaluateIf(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxBOOL bTest;

  /* Disables marker operations */
  orxProfiler_EnableMarkerOperations(orxFALSE);

  /* Is true? */
  if((orxString_ToBool(_astArgList[0].zValue, &bTest, orxNULL) != orxSTATUS_FAILURE) && (bTest != orxFALSE))
  {
    /* Evaluates first command */
    orxCommand_Evaluate(_astArgList[1].zValue, _pstResult);
  }
  else
  {
    /* Has an alternate command? */
    if(_u32ArgNumber > 2)
    {
      /* Evaluates it */
      orxCommand_Evaluate(_astArgList[2].zValue, _pstResult);
    }
    else
    {
      /* Updates result */
      _pstResult->zValue = orxSTRING_EMPTY;
    }
  }

  /* Re-enables marker operations */
  orxProfiler_EnableMarkerOperations(orxTRUE);

  /* Done! */
  return;
}

/* Command: If */
void orxFASTCALL orxCommand_CommandIf(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxBOOL bTest;

  /* Is true? */
  if((orxString_ToBool(_astArgList[0].zValue, &bTest, orxNULL) != orxSTATUS_FAILURE) && (bTest != orxFALSE))
  {
    /* Updates result */
    _pstResult->zValue = _astArgList[1].zValue;
  }
  else
  {
    /* Updates result */
    _pstResult->zValue = (_u32ArgNumber > 2) ? _astArgList[2].zValue : orxSTRING_EMPTY;
  }

  /* Done! */
  return;
}

/* Command: Repeat */
void orxFASTCALL orxCommand_CommandRepeat(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxS32  s32Count;
  orxCHAR acBuffer[orxCOMMAND_KU32_RESULT_BUFFER_SIZE];

  /* Disables marker operations */
  orxProfiler_EnableMarkerOperations(orxFALSE);

  /* Gets count */
  s32Count = _astArgList[0].s32Value;

  /* Copies command */
  orxString_NCopy(acBuffer, _astArgList[1].zValue, sizeof(acBuffer) - 1);
  acBuffer[sizeof(acBuffer) - 1] = orxCHAR_NULL;

  /* For all iterations */
  while(s32Count--)
  {
    /* Evaluates first command */
    orxCommand_Evaluate(acBuffer, _pstResult);
  }

  /* Re-enables marker operations */
  orxProfiler_EnableMarkerOperations(orxTRUE);

  /* Done! */
  return;
}

/* Command: Return */
void orxFASTCALL orxCommand_CommandReturn(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->zValue = _astArgList[0].zValue;

  /* Done! */
  return;
}

/* Command: Not */
void orxFASTCALL orxCommand_CommandNot(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->bValue = !_astArgList[0].bValue;

  /* Done! */
  return;
}

/* Command: And */
void orxFASTCALL orxCommand_CommandAnd(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->bValue = _astArgList[0].bValue && _astArgList[1].bValue;

  /* Done! */
  return;
}

/* Command: Or */
void orxFASTCALL orxCommand_CommandOr(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->bValue = _astArgList[0].bValue || _astArgList[1].bValue;

  /* Done! */
  return;
}

/* Command: XOr */
void orxFASTCALL orxCommand_CommandXOr(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->bValue = (_astArgList[0].bValue || _astArgList[1].bValue) && !(_astArgList[0].bValue && _astArgList[1].bValue);

  /* Done! */
  return;
}

/* Command: AreEqual */
void orxFASTCALL orxCommand_CommandAreEqual(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCOMMAND_VAR astOperandList[2];

  /* Parses numerical arguments */
  if(orxCommand_ParseNumericalArguments(_u32ArgNumber, _astArgList, astOperandList) != orxSTATUS_FAILURE)
  {
    /* Both floats? */
    if((astOperandList[0].eType == orxCOMMAND_VAR_TYPE_FLOAT)
    && (astOperandList[1].eType == orxCOMMAND_VAR_TYPE_FLOAT))
    {
      /* Updates result */
      _pstResult->bValue = (astOperandList[0].fValue == astOperandList[1].fValue) ? orxTRUE : orxFALSE;
    }
    /* Both vectors? */
    else if((astOperandList[0].eType == orxCOMMAND_VAR_TYPE_VECTOR)
         && (astOperandList[1].eType == orxCOMMAND_VAR_TYPE_VECTOR))
    {
      /* Updates result */
      _pstResult->bValue = orxVector_AreEqual(&(astOperandList[0].vValue), &(astOperandList[1].vValue));
    }
    else
    {
      /* Updates result */
      _pstResult->bValue = (orxString_ICompare(_astArgList[0].zValue, _astArgList[1].zValue) == 0) ? orxTRUE : orxFALSE;
    }
  }
  else
  {
    orxBOOL bOperand1, bOperand2;

    /* Gets bool operands */
    if((orxString_ToBool(_astArgList[0].zValue, &bOperand1, orxNULL) != orxSTATUS_FAILURE)
    && (orxString_ToBool(_astArgList[1].zValue, &bOperand2, orxNULL) != orxSTATUS_FAILURE))
    {
      /* Updates result */
      _pstResult->bValue = (bOperand1 == bOperand2) ? orxTRUE : orxFALSE;
    }
    else
    {
      /* Updates result */
      _pstResult->bValue = (orxString_ICompare(_astArgList[0].zValue, _astArgList[1].zValue) == 0) ? orxTRUE : orxFALSE;
    }
  }

  /* Done! */
  return;
}

/* Command: IsGreater */
void orxFASTCALL orxCommand_CommandIsGreater(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCOMMAND_VAR astOperandList[2];
  orxBOOL bStrict;

  /* Updates strict status */
  bStrict = ((_u32ArgNumber <= 2) || (_astArgList[2].bValue != orxFALSE)) ? orxTRUE : orxFALSE;

  /* Parses numerical arguments */
  if(orxCommand_ParseNumericalArguments(_u32ArgNumber, _astArgList, astOperandList) != orxSTATUS_FAILURE)
  {
    /* Both floats? */
    if((astOperandList[0].eType == orxCOMMAND_VAR_TYPE_FLOAT)
    && (astOperandList[1].eType == orxCOMMAND_VAR_TYPE_FLOAT))
    {
      /* Updates result */
      _pstResult->bValue = (bStrict != orxFALSE)
                           ? (astOperandList[0].fValue > astOperandList[1].fValue)
                           : (astOperandList[0].fValue >= astOperandList[1].fValue);
    }
    /* Both vectors? */
    else if((astOperandList[0].eType == orxCOMMAND_VAR_TYPE_VECTOR)
         && (astOperandList[1].eType == orxCOMMAND_VAR_TYPE_VECTOR))
    {
      /* Updates result */
      _pstResult->bValue = (bStrict != orxFALSE)
                           ? ((astOperandList[0].vValue.fX > astOperandList[1].vValue.fX)
                           && (astOperandList[0].vValue.fY > astOperandList[1].vValue.fY)
                           && (astOperandList[0].vValue.fZ > astOperandList[1].vValue.fZ))
                           : ((astOperandList[0].vValue.fX >= astOperandList[1].vValue.fX)
                           && (astOperandList[0].vValue.fY >= astOperandList[1].vValue.fY)
                           && (astOperandList[0].vValue.fZ >= astOperandList[1].vValue.fZ));
    }
    else
    {
      /* Updates result */
      _pstResult->bValue = (bStrict != orxFALSE)
                           ? ((orxString_ICompare(_astArgList[0].zValue, _astArgList[1].zValue) > 0) ? orxTRUE : orxFALSE)
                           : ((orxString_ICompare(_astArgList[0].zValue, _astArgList[1].zValue) >= 0) ? orxTRUE : orxFALSE);
    }
  }
  else
  {
    /* Updates result */
    _pstResult->bValue = (bStrict != orxFALSE)
                         ? ((orxString_ICompare(_astArgList[0].zValue, _astArgList[1].zValue) > 0) ? orxTRUE : orxFALSE)
                         : ((orxString_ICompare(_astArgList[0].zValue, _astArgList[1].zValue) >= 0) ? orxTRUE : orxFALSE);
  }

  /* Done! */
  return;
}

/* Command: IsLesser */
void orxFASTCALL orxCommand_CommandIsLesser(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCOMMAND_VAR astOperandList[2];
  orxBOOL bStrict;

  /* Updates strict status */
  bStrict = ((_u32ArgNumber <= 2) || (_astArgList[2].bValue != orxFALSE)) ? orxTRUE : orxFALSE;

  /* Parses numerical arguments */
  if(orxCommand_ParseNumericalArguments(_u32ArgNumber, _astArgList, astOperandList) != orxSTATUS_FAILURE)
  {
    /* Both floats? */
    if((astOperandList[0].eType == orxCOMMAND_VAR_TYPE_FLOAT)
    && (astOperandList[1].eType == orxCOMMAND_VAR_TYPE_FLOAT))
    {
      /* Updates result */
      _pstResult->bValue = (bStrict != orxFALSE)
                           ? (astOperandList[0].fValue < astOperandList[1].fValue)
                           : (astOperandList[0].fValue <= astOperandList[1].fValue);
    }
    /* Both vectors? */
    else if((astOperandList[0].eType == orxCOMMAND_VAR_TYPE_VECTOR)
         && (astOperandList[1].eType == orxCOMMAND_VAR_TYPE_VECTOR))
    {
      /* Updates result */
      _pstResult->bValue = (bStrict != orxFALSE)
                           ? ((astOperandList[0].vValue.fX < astOperandList[1].vValue.fX)
                           && (astOperandList[0].vValue.fY < astOperandList[1].vValue.fY)
                           && (astOperandList[0].vValue.fZ < astOperandList[1].vValue.fZ))
                           : ((astOperandList[0].vValue.fX <= astOperandList[1].vValue.fX)
                           && (astOperandList[0].vValue.fY <= astOperandList[1].vValue.fY)
                           && (astOperandList[0].vValue.fZ <= astOperandList[1].vValue.fZ));
    }
    else
    {
      /* Updates result */
      _pstResult->bValue = (bStrict != orxFALSE)
                           ? ((orxString_ICompare(_astArgList[0].zValue, _astArgList[1].zValue) < 0) ? orxTRUE : orxFALSE)
                           : ((orxString_ICompare(_astArgList[0].zValue, _astArgList[1].zValue) <= 0) ? orxTRUE : orxFALSE);
    }
  }
  else
  {
    /* Updates result */
    _pstResult->bValue = (bStrict != orxFALSE)
                         ? ((orxString_ICompare(_astArgList[0].zValue, _astArgList[1].zValue) < 0) ? orxTRUE : orxFALSE)
                         : ((orxString_ICompare(_astArgList[0].zValue, _astArgList[1].zValue) <= 0) ? orxTRUE : orxFALSE);
  }

  /* Done! */
  return;
}

/* Command: Add */
void orxFASTCALL orxCommand_CommandAdd(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCOMMAND_VAR astOperandList[2];

  /* Parses numerical arguments */
  if(orxCommand_ParseNumericalArguments(_u32ArgNumber, _astArgList, astOperandList) != orxSTATUS_FAILURE)
  {
    /* Both floats? */
    if((astOperandList[0].eType == orxCOMMAND_VAR_TYPE_FLOAT)
    && (astOperandList[1].eType == orxCOMMAND_VAR_TYPE_FLOAT))
    {
      /* Prints value */
      orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%g", astOperandList[0].fValue + astOperandList[1].fValue);
    }
    else
    {
      orxVECTOR vResult;

      /* Is operand1 a float? */
      if(astOperandList[0].eType == orxCOMMAND_VAR_TYPE_FLOAT)
      {
        /* Converts it */
        orxVector_SetAll(&(astOperandList[0].vValue), astOperandList[0].fValue);
      }
      /* Is operand2 a float? */
      else if(astOperandList[1].eType == orxCOMMAND_VAR_TYPE_FLOAT)
      {
        /* Converts it */
        orxVector_SetAll(&(astOperandList[1].vValue), astOperandList[1].fValue);
      }

      /* Updates intermediate result */
      orxVector_Add(&vResult, &(astOperandList[0].vValue), &(astOperandList[1].vValue));

      /* Prints it */
      orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%c%g%c %g%c %g%c", orxSTRING_KC_VECTOR_START, vResult.fX, orxSTRING_KC_VECTOR_SEPARATOR, vResult.fY, orxSTRING_KC_VECTOR_SEPARATOR, vResult.fZ, orxSTRING_KC_VECTOR_END);
    }

    /* Updates result */
    _pstResult->zValue = sstCommand.acResultBuffer;
  }
  else
  {
    /* Updates result */
    _pstResult->zValue = orxSTRING_EMPTY;
  }

  /* Done! */
  return;
}

/* Command: Subtract */
void orxFASTCALL orxCommand_CommandSubtract(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCOMMAND_VAR astOperandList[2];

  /* Parses numerical arguments */
  if(orxCommand_ParseNumericalArguments(_u32ArgNumber, _astArgList, astOperandList) != orxSTATUS_FAILURE)
  {
    /* Both floats? */
    if((astOperandList[0].eType == orxCOMMAND_VAR_TYPE_FLOAT)
    && (astOperandList[1].eType == orxCOMMAND_VAR_TYPE_FLOAT))
    {
      /* Prints value */
      orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%g", astOperandList[0].fValue - astOperandList[1].fValue);
    }
    else
    {
      orxVECTOR vResult;

      /* Is operand1 a float? */
      if(astOperandList[0].eType == orxCOMMAND_VAR_TYPE_FLOAT)
      {
        /* Converts it */
        orxVector_SetAll(&(astOperandList[0].vValue), astOperandList[0].fValue);
      }
      /* Is operand2 a float? */
      else if(astOperandList[1].eType == orxCOMMAND_VAR_TYPE_FLOAT)
      {
        /* Converts it */
        orxVector_SetAll(&(astOperandList[1].vValue), astOperandList[1].fValue);
      }

      /* Updates intermediate result */
      orxVector_Sub(&vResult, &(astOperandList[0].vValue), &(astOperandList[1].vValue));

      /* Prints it */
      orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%c%g%c %g%c %g%c", orxSTRING_KC_VECTOR_START, vResult.fX, orxSTRING_KC_VECTOR_SEPARATOR, vResult.fY, orxSTRING_KC_VECTOR_SEPARATOR, vResult.fZ, orxSTRING_KC_VECTOR_END);
    }

    /* Updates result */
    _pstResult->zValue = sstCommand.acResultBuffer;
  }
  else
  {
    /* Updates result */
    _pstResult->zValue = orxSTRING_EMPTY;
  }

  /* Done! */
  return;
}

/* Command: Multiply */
void orxFASTCALL orxCommand_CommandMultiply(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCOMMAND_VAR astOperandList[2];

  /* Parses numerical arguments */
  if(orxCommand_ParseNumericalArguments(_u32ArgNumber, _astArgList, astOperandList) != orxSTATUS_FAILURE)
  {
    /* Both floats? */
    if((astOperandList[0].eType == orxCOMMAND_VAR_TYPE_FLOAT)
    && (astOperandList[1].eType == orxCOMMAND_VAR_TYPE_FLOAT))
    {
      /* Prints value */
      orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%g", astOperandList[0].fValue * astOperandList[1].fValue);
    }
    else
    {
      orxVECTOR vResult;

      /* Is operand1 a float? */
      if(astOperandList[0].eType == orxCOMMAND_VAR_TYPE_FLOAT)
      {
        /* Converts it */
        orxVector_SetAll(&(astOperandList[0].vValue), astOperandList[0].fValue);
      }
      /* Is operand2 a float? */
      else if(astOperandList[1].eType == orxCOMMAND_VAR_TYPE_FLOAT)
      {
        /* Converts it */
        orxVector_SetAll(&(astOperandList[1].vValue), astOperandList[1].fValue);
      }

      /* Updates intermediate result */
      orxVector_Mul(&vResult, &(astOperandList[0].vValue), &(astOperandList[1].vValue));

      /* Prints it */
      orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%c%g%c %g%c %g%c", orxSTRING_KC_VECTOR_START, vResult.fX, orxSTRING_KC_VECTOR_SEPARATOR, vResult.fY, orxSTRING_KC_VECTOR_SEPARATOR, vResult.fZ, orxSTRING_KC_VECTOR_END);
    }

    /* Updates result */
    _pstResult->zValue = sstCommand.acResultBuffer;
  }
  else
  {
    /* Updates result */
    _pstResult->zValue = orxSTRING_EMPTY;
  }

  /* Done! */
  return;
}

/* Command: Divide */
void orxFASTCALL orxCommand_CommandDivide(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCOMMAND_VAR astOperandList[2];

  /* Parses numerical arguments */
  if(orxCommand_ParseNumericalArguments(_u32ArgNumber, _astArgList, astOperandList) != orxSTATUS_FAILURE)
  {
    /* Both floats? */
    if((astOperandList[0].eType == orxCOMMAND_VAR_TYPE_FLOAT)
    && (astOperandList[1].eType == orxCOMMAND_VAR_TYPE_FLOAT))
    {
      /* Prints value */
      orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%g", astOperandList[0].fValue / astOperandList[1].fValue);
    }
    else
    {
      orxVECTOR vResult;

      /* Is operand1 a float? */
      if(astOperandList[0].eType == orxCOMMAND_VAR_TYPE_FLOAT)
      {
        /* Converts it */
        orxVector_SetAll(&(astOperandList[0].vValue), astOperandList[0].fValue);
      }
      /* Is operand2 a float? */
      else if(astOperandList[1].eType == orxCOMMAND_VAR_TYPE_FLOAT)
      {
        /* Converts it */
        orxVector_SetAll(&(astOperandList[1].vValue), astOperandList[1].fValue);
      }

      /* Updates intermediate result */
      orxVector_Div(&vResult, &(astOperandList[0].vValue), &(astOperandList[1].vValue));

      /* Prints it */
      orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%c%g%c %g%c %g%c", orxSTRING_KC_VECTOR_START, vResult.fX, orxSTRING_KC_VECTOR_SEPARATOR, vResult.fY, orxSTRING_KC_VECTOR_SEPARATOR, vResult.fZ, orxSTRING_KC_VECTOR_END);
    }

    /* Updates result */
    _pstResult->zValue = sstCommand.acResultBuffer;
  }
  else
  {
    /* Updates result */
    _pstResult->zValue = orxSTRING_EMPTY;
  }

  /* Done! */
  return;
}

/* Command: Modulo */
void orxFASTCALL orxCommand_CommandModulo(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCOMMAND_VAR astOperandList[2];

  /* Parses numerical arguments */
  if(orxCommand_ParseNumericalArguments(_u32ArgNumber, _astArgList, astOperandList) != orxSTATUS_FAILURE)
  {
    /* Both floats? */
    if((astOperandList[0].eType == orxCOMMAND_VAR_TYPE_FLOAT)
    && (astOperandList[1].eType == orxCOMMAND_VAR_TYPE_FLOAT))
    {
      /* Prints value */
      orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%g", orxMath_Mod(astOperandList[0].fValue, astOperandList[1].fValue));
    }
    else
    {
      orxVECTOR vResult;

      /* Is operand1 a float? */
      if(astOperandList[0].eType == orxCOMMAND_VAR_TYPE_FLOAT)
      {
        /* Converts it */
        orxVector_SetAll(&(astOperandList[0].vValue), astOperandList[0].fValue);
      }
      /* Is operand2 a float? */
      else if(astOperandList[1].eType == orxCOMMAND_VAR_TYPE_FLOAT)
      {
        /* Converts it */
        orxVector_SetAll(&(astOperandList[1].vValue), astOperandList[1].fValue);
      }

      /* Updates intermediate result */
      orxVector_Mod(&vResult, &(astOperandList[0].vValue), &(astOperandList[1].vValue));

      /* Prints it */
      orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%c%g%c %g%c %g%c", orxSTRING_KC_VECTOR_START, vResult.fX, orxSTRING_KC_VECTOR_SEPARATOR, vResult.fY, orxSTRING_KC_VECTOR_SEPARATOR, vResult.fZ, orxSTRING_KC_VECTOR_END);
    }

    /* Updates result */
    _pstResult->zValue = sstCommand.acResultBuffer;
  }
  else
  {
    /* Updates result */
    _pstResult->zValue = orxSTRING_EMPTY;
  }

  /* Done! */
  return;
}

/* Command: Absolute */
void orxFASTCALL orxCommand_CommandAbsolute(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCOMMAND_VAR stOperand;

  /* Parses numerical argument */
  if(orxCommand_ParseNumericalArguments(1, _astArgList, &stOperand) != orxSTATUS_FAILURE)
  {
    /* Float? */
    if(stOperand.eType == orxCOMMAND_VAR_TYPE_FLOAT)
    {
      /* Prints value */
      orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%g", orxMath_Abs(stOperand.fValue));
    }
    else
    {
      orxVECTOR vResult;

      /* Updates intermediate result */
      orxVector_Abs(&vResult, &(stOperand.vValue));

      /* Prints it */
      orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%c%g%c %g%c %g%c", orxSTRING_KC_VECTOR_START, vResult.fX, orxSTRING_KC_VECTOR_SEPARATOR, vResult.fY, orxSTRING_KC_VECTOR_SEPARATOR, vResult.fZ, orxSTRING_KC_VECTOR_END);
    }

    /* Updates result */
    _pstResult->zValue = sstCommand.acResultBuffer;
  }
  else
  {
    /* Updates result */
    _pstResult->zValue = orxSTRING_EMPTY;
  }

  /* Done! */
  return;
}

/* Command: Negate */
void orxFASTCALL orxCommand_CommandNegate(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCOMMAND_VAR stOperand;

  /* Parses numerical argument */
  if(orxCommand_ParseNumericalArguments(1, _astArgList, &stOperand) != orxSTATUS_FAILURE)
  {
    /* Float? */
    if(stOperand.eType == orxCOMMAND_VAR_TYPE_FLOAT)
    {
      /* Prints value */
      orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%g", -stOperand.fValue);
    }
    else
    {
      orxVECTOR vResult;

      /* Updates intermediate result */
      orxVector_Neg(&vResult, &(stOperand.vValue));

      /* Prints it */
      orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%c%g%c %g%c %g%c", orxSTRING_KC_VECTOR_START, vResult.fX, orxSTRING_KC_VECTOR_SEPARATOR, vResult.fY, orxSTRING_KC_VECTOR_SEPARATOR, vResult.fZ, orxSTRING_KC_VECTOR_END);
    }

    /* Updates result */
    _pstResult->zValue = sstCommand.acResultBuffer;
  }
  else
  {
    /* Updates result */
    _pstResult->zValue = orxSTRING_EMPTY;
  }

  /* Done! */
  return;
}

/* Command: Random */
void orxFASTCALL orxCommand_CommandRandom(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCOMMAND_VAR astOperandList[3];

  /* Parses numerical arguments */
  if(orxCommand_ParseNumericalArguments(_u32ArgNumber, _astArgList, astOperandList) != orxSTATUS_FAILURE)
  {
    orxVECTOR vResult;
    orxFLOAT  fResult;

    /* Depending on the number of arguments */
    switch(_u32ArgNumber)
    {
      case 0:
      {
        /* Gets normalized random value */
        fResult = orxMath_GetRandomFloat(orxFLOAT_0, orxFLOAT_1);

        /* Prints it */
        orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%g", fResult);

        break;
      }

      case 1:
      {
        /* Float? */
        if(astOperandList[0].eType == orxCOMMAND_VAR_TYPE_FLOAT)
        {
          /* Gets random value */
          fResult = orxMath_GetRandomFloat(orxFLOAT_0, astOperandList[0].fValue);

          /* Prints it */
          orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%g", fResult);
        }
        else
        {
          /* Gets random value */
          orxVector_Set(&vResult, orxMath_GetRandomFloat(orxFLOAT_0, astOperandList[0].vValue.fX), orxMath_GetRandomFloat(orxFLOAT_0, astOperandList[0].vValue.fY), orxMath_GetRandomFloat(orxFLOAT_0, astOperandList[0].vValue.fZ));

          /* Prints it */
          orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%c%g%c %g%c %g%c", orxSTRING_KC_VECTOR_START, vResult.fX, orxSTRING_KC_VECTOR_SEPARATOR, vResult.fY, orxSTRING_KC_VECTOR_SEPARATOR, vResult.fZ, orxSTRING_KC_VECTOR_END);
        }

        break;
      }

      default:
      {
        /* All floats? */
        if((astOperandList[0].eType == orxCOMMAND_VAR_TYPE_FLOAT)
        && (astOperandList[1].eType == orxCOMMAND_VAR_TYPE_FLOAT)
        && ((_u32ArgNumber < 3)
         || (astOperandList[2].eType == orxCOMMAND_VAR_TYPE_FLOAT)))
        {
          /* Gets random value */
          fResult = (_u32ArgNumber < 3) ? orxMath_GetRandomFloat(astOperandList[0].fValue, astOperandList[1].fValue) : orxMath_GetSteppedRandomFloat(astOperandList[0].fValue, astOperandList[1].fValue, astOperandList[2].fValue);

          /* Prints it */
          orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%g", fResult);
        }
        else
        {
          /* Is boundary1 a float? */
          if(astOperandList[0].eType == orxCOMMAND_VAR_TYPE_FLOAT)
          {
            /* Converts it */
            orxVector_SetAll(&(astOperandList[0].vValue), astOperandList[0].fValue);
          }
          /* Is boundary2 a float? */
          if(astOperandList[1].eType == orxCOMMAND_VAR_TYPE_FLOAT)
          {
            /* Converts it */
            orxVector_SetAll(&(astOperandList[1].vValue), astOperandList[1].fValue);
          }
          /* Is step a float? */
          if((_u32ArgNumber >= 3) && (astOperandList[2].eType == orxCOMMAND_VAR_TYPE_FLOAT))
          {
            /* Converts it */
            orxVector_SetAll(&(astOperandList[2].vValue), astOperandList[2].fValue);
          }

          /* Gets random value */
          if(_u32ArgNumber < 3)
          {
            orxVector_Set(&vResult, orxMath_GetRandomFloat(astOperandList[0].vValue.fX, astOperandList[1].vValue.fX), orxMath_GetRandomFloat(astOperandList[0].vValue.fY, astOperandList[1].vValue.fY), orxMath_GetRandomFloat(astOperandList[0].vValue.fZ, astOperandList[1].vValue.fZ));
          }
          else
          {
            orxVector_Set(&vResult, orxMath_GetSteppedRandomFloat(astOperandList[0].vValue.fX, astOperandList[1].vValue.fX, astOperandList[2].vValue.fX), orxMath_GetSteppedRandomFloat(astOperandList[0].vValue.fY, astOperandList[1].vValue.fY, astOperandList[2].vValue.fY), orxMath_GetSteppedRandomFloat(astOperandList[0].vValue.fZ, astOperandList[1].vValue.fZ, astOperandList[2].vValue.fZ));
          }

          /* Prints it */
          orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%c%g%c %g%c %g%c", orxSTRING_KC_VECTOR_START, vResult.fX, orxSTRING_KC_VECTOR_SEPARATOR, vResult.fY, orxSTRING_KC_VECTOR_SEPARATOR, vResult.fZ, orxSTRING_KC_VECTOR_END);
        }

        break;
      }
    }

    /* Updates result */
    _pstResult->zValue = sstCommand.acResultBuffer;
  }
  else
  {
    /* Updates result */
    _pstResult->zValue = orxSTRING_EMPTY;
  }

  /* Done! */
  return;
}

/* Command: Normalize */
void orxFASTCALL orxCommand_CommandNormalize(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  orxVector_Normalize(&(_pstResult->vValue), &(_astArgList[0].vValue));

  /* Done! */
  return;
}

/* Command: ToSpherical */
void orxFASTCALL orxCommand_CommandToSpherical(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  orxVector_FromCartesianToSpherical(&(_pstResult->vValue), &(_astArgList[0].vValue));
  _pstResult->vValue.fTheta *= orxMATH_KF_RAD_TO_DEG;

  /* Done! */
  return;
}

/* Command: ToCartesian */
void orxFASTCALL orxCommand_CommandToCartesian(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxVECTOR vTemp;

  /* Updates result */
  orxVector_Set(&vTemp, _astArgList[0].vValue.fRho, orxMATH_KF_DEG_TO_RAD * _astArgList[0].vValue.fTheta, _astArgList[0].vValue.fPhi);
  orxVector_FromSphericalToCartesian(&(_pstResult->vValue), &vTemp);

  /* Done! */
  return;
}

/* Command: FromRGBToHSV */
void orxFASTCALL orxCommand_CommandFromRGBToHSV(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCOLOR stColor;

  /* Inits color */
  orxVector_Copy(&(stColor.vRGB), &(_astArgList[0].vValue));

  /* Converts color */
  orxColor_FromRGBToHSV(&stColor, &stColor);

  /* Updates result */
  orxVector_Copy(&(_pstResult->vValue), &(stColor.vHSV));

  /* Done! */
  return;
}

/* Command: FromHSVToRGB */
void orxFASTCALL orxCommand_CommandFromHSVToRGB(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCOLOR stColor;

  /* Inits color */
  orxVector_Copy(&(stColor.vHSV), &(_astArgList[0].vValue));

  /* Converts color */
  orxColor_FromHSVToRGB(&stColor, &stColor);

  /* Updates result */
  orxVector_Copy(&(_pstResult->vValue), &(stColor.vRGB));

  /* Done! */
  return;
}

/* Command: FromRGBToHSL */
void orxFASTCALL orxCommand_CommandFromRGBToHSL(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCOLOR stColor;

  /* Inits color */
  orxVector_Copy(&(stColor.vRGB), &(_astArgList[0].vValue));

  /* Converts color */
  orxColor_FromRGBToHSL(&stColor, &stColor);

  /* Updates result */
  orxVector_Copy(&(_pstResult->vValue), &(stColor.vHSL));

  /* Done! */
  return;
}

/* Command: FromHSLToRGB */
void orxFASTCALL orxCommand_CommandFromHSLToRGB(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCOLOR stColor;

  /* Inits color */
  orxVector_Copy(&(stColor.vHSL), &(_astArgList[0].vValue));

  /* Converts color */
  orxColor_FromHSLToRGB(&stColor, &stColor);

  /* Updates result */
  orxVector_Copy(&(_pstResult->vValue), &(stColor.vRGB));

  /* Done! */
  return;
}

/* Command: VectorX */
void orxFASTCALL orxCommand_CommandVectorX(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->fValue = _astArgList[0].vValue.fX;

  /* Done! */
  return;
}

/* Command: VectorY */
void orxFASTCALL orxCommand_CommandVectorY(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->fValue = _astArgList[0].vValue.fY;

  /* Done! */
  return;
}

/* Command: VectorZ */
void orxFASTCALL orxCommand_CommandVectorZ(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->fValue = _astArgList[0].vValue.fZ;

  /* Done! */
  return;
}

/* Command: Lerp */
void orxFASTCALL orxCommand_CommandLerp(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCOMMAND_VAR astOperandList[2];

  /* Parses numerical arguments */
  if(orxCommand_ParseNumericalArguments(2, _astArgList, astOperandList) != orxSTATUS_FAILURE)
  {
    /* All floats? */
    if((astOperandList[0].eType == orxCOMMAND_VAR_TYPE_FLOAT)
    && (astOperandList[1].eType == orxCOMMAND_VAR_TYPE_FLOAT))
    {
      /* Prints value */
      orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%g", orxLERP(astOperandList[0].fValue, astOperandList[1].fValue, _astArgList[2].fValue));

      /* Updates result */
      _pstResult->zValue = sstCommand.acResultBuffer;
    }
    else
    {
      orxVECTOR vResult;

      /* Is A a float? */
      if(astOperandList[0].eType == orxCOMMAND_VAR_TYPE_FLOAT)
      {
        /* Converts it */
        orxVector_SetAll(&(astOperandList[0].vValue), astOperandList[0].fValue);
      }
      /* Is B a float? */
      else if(astOperandList[1].eType == orxCOMMAND_VAR_TYPE_FLOAT)
      {
        /* Converts it */
        orxVector_SetAll(&(astOperandList[1].vValue), astOperandList[1].fValue);
      }

      /* Updates intermediate result */
      orxVector_Lerp(&vResult, &(astOperandList[0].vValue), &(astOperandList[1].vValue), _astArgList[2].fValue);

      /* Prints it */
      orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%c%g%c %g%c %g%c", orxSTRING_KC_VECTOR_START, vResult.fX, orxSTRING_KC_VECTOR_SEPARATOR, vResult.fY, orxSTRING_KC_VECTOR_SEPARATOR, vResult.fZ, orxSTRING_KC_VECTOR_END);

      /* Updates result */
      _pstResult->zValue = sstCommand.acResultBuffer;
    }
  }
  else
  {
    /* Updates result */
    _pstResult->zValue = orxSTRING_EMPTY;
  }

  /* Done! */
  return;
}

/* Command: Remap */
void orxFASTCALL orxCommand_CommandRemap(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCOMMAND_VAR astOperandList[5];

  /* Parses numerical arguments */
  if(orxCommand_ParseNumericalArguments(_u32ArgNumber, _astArgList, astOperandList) != orxSTATUS_FAILURE)
  {
    /* All floats? */
    if((astOperandList[0].eType == orxCOMMAND_VAR_TYPE_FLOAT)
    && (astOperandList[1].eType == orxCOMMAND_VAR_TYPE_FLOAT)
    && (astOperandList[2].eType == orxCOMMAND_VAR_TYPE_FLOAT)
    && (astOperandList[3].eType == orxCOMMAND_VAR_TYPE_FLOAT)
    && (astOperandList[4].eType == orxCOMMAND_VAR_TYPE_FLOAT))
    {
      /* Prints value */
      orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%g", orxREMAP(astOperandList[0].fValue, astOperandList[1].fValue, astOperandList[2].fValue, astOperandList[3].fValue, astOperandList[4].fValue));

      /* Updates result */
      _pstResult->zValue = sstCommand.acResultBuffer;
    }
    else
    {
      orxVECTOR vResult;

      /* Is A1 a float? */
      if(astOperandList[0].eType == orxCOMMAND_VAR_TYPE_FLOAT)
      {
        /* Converts it */
        orxVector_SetAll(&(astOperandList[0].vValue), astOperandList[0].fValue);
      }
      /* Is B1 a float? */
      if(astOperandList[1].eType == orxCOMMAND_VAR_TYPE_FLOAT)
      {
        /* Converts it */
        orxVector_SetAll(&(astOperandList[1].vValue), astOperandList[1].fValue);
      }
      /* Is A2 a float? */
      if(astOperandList[2].eType == orxCOMMAND_VAR_TYPE_FLOAT)
      {
        /* Converts it */
        orxVector_SetAll(&(astOperandList[2].vValue), astOperandList[2].fValue);
      }
      /* Is B2 a float? */
      if(astOperandList[3].eType == orxCOMMAND_VAR_TYPE_FLOAT)
      {
        /* Converts it */
        orxVector_SetAll(&(astOperandList[3].vValue), astOperandList[3].fValue);
      }
      /* Is V a float? */
      if(astOperandList[4].eType == orxCOMMAND_VAR_TYPE_FLOAT)
      {
        /* Converts it */
        orxVector_SetAll(&(astOperandList[4].vValue), astOperandList[4].fValue);
      }

      /* Updates intermediate result */
      orxVector_Remap(&vResult, &(astOperandList[0].vValue), &(astOperandList[1].vValue), &(astOperandList[2].vValue), &(astOperandList[3].vValue), &(astOperandList[4].vValue));

      /* Prints it */
      orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%c%g%c %g%c %g%c", orxSTRING_KC_VECTOR_START, vResult.fX, orxSTRING_KC_VECTOR_SEPARATOR, vResult.fY, orxSTRING_KC_VECTOR_SEPARATOR, vResult.fZ, orxSTRING_KC_VECTOR_END);

      /* Updates result */
      _pstResult->zValue = sstCommand.acResultBuffer;
    }
  }
  else
  {
    /* Updates result */
    _pstResult->zValue = orxSTRING_EMPTY;
  }

  /* Done! */
  return;
}

/* Command: Minimum */
void orxFASTCALL orxCommand_CommandMinimum(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCOMMAND_VAR astOperandList[2];

  /* Parses numerical arguments */
  if(orxCommand_ParseNumericalArguments(_u32ArgNumber, _astArgList, astOperandList) != orxSTATUS_FAILURE)
  {
    /* Both floats? */
    if((astOperandList[0].eType == orxCOMMAND_VAR_TYPE_FLOAT)
    && (astOperandList[1].eType == orxCOMMAND_VAR_TYPE_FLOAT))
    {
      /* Prints value */
      orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%g", orxMIN(astOperandList[0].fValue, astOperandList[1].fValue));

      /* Updates result */
      _pstResult->zValue = sstCommand.acResultBuffer;
    }
    else
    {
      orxVECTOR vResult;

      /* Is operand1 a float? */
      if(astOperandList[0].eType == orxCOMMAND_VAR_TYPE_FLOAT)
      {
        /* Converts it */
        orxVector_SetAll(&(astOperandList[0].vValue), astOperandList[0].fValue);
      }
      /* Is operand2 a float? */
      else if(astOperandList[1].eType == orxCOMMAND_VAR_TYPE_FLOAT)
      {
        /* Converts it */
        orxVector_SetAll(&(astOperandList[1].vValue), astOperandList[1].fValue);
      }

      /* Updates intermediate result */
      orxVector_Min(&vResult, &(astOperandList[0].vValue), &(astOperandList[1].vValue));

      /* Prints it */
      orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%c%g%c %g%c %g%c", orxSTRING_KC_VECTOR_START, vResult.fX, orxSTRING_KC_VECTOR_SEPARATOR, vResult.fY, orxSTRING_KC_VECTOR_SEPARATOR, vResult.fZ, orxSTRING_KC_VECTOR_END);

      /* Updates result */
      _pstResult->zValue = sstCommand.acResultBuffer;
    }
  }
  else
  {
    /* Updates result */
    _pstResult->zValue = orxSTRING_EMPTY;
  }

  /* Done! */
  return;
}

/* Command: Maximum */
void orxFASTCALL orxCommand_CommandMaximum(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCOMMAND_VAR astOperandList[2];

  /* Parses numerical arguments */
  if(orxCommand_ParseNumericalArguments(_u32ArgNumber, _astArgList, astOperandList) != orxSTATUS_FAILURE)
  {
    /* Both floats? */
    if((astOperandList[0].eType == orxCOMMAND_VAR_TYPE_FLOAT)
    && (astOperandList[1].eType == orxCOMMAND_VAR_TYPE_FLOAT))
    {
      /* Prints value */
      orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%g", orxMAX(astOperandList[0].fValue, astOperandList[1].fValue));

      /* Updates result */
      _pstResult->zValue = sstCommand.acResultBuffer;
    }
    else
    {
      orxVECTOR vResult;

      /* Is operand1 a float? */
      if(astOperandList[0].eType == orxCOMMAND_VAR_TYPE_FLOAT)
      {
        /* Converts it */
        orxVector_SetAll(&(astOperandList[0].vValue), astOperandList[0].fValue);
      }
      /* Is operand2 a float? */
      else if(astOperandList[1].eType == orxCOMMAND_VAR_TYPE_FLOAT)
      {
        /* Converts it */
        orxVector_SetAll(&(astOperandList[1].vValue), astOperandList[1].fValue);
      }

      /* Updates intermediate result */
      orxVector_Max(&vResult, &(astOperandList[0].vValue), &(astOperandList[1].vValue));

      /* Prints it */
      orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%c%g%c %g%c %g%c", orxSTRING_KC_VECTOR_START, vResult.fX, orxSTRING_KC_VECTOR_SEPARATOR, vResult.fY, orxSTRING_KC_VECTOR_SEPARATOR, vResult.fZ, orxSTRING_KC_VECTOR_END);

      /* Updates result */
      _pstResult->zValue = sstCommand.acResultBuffer;
    }
  }
  else
  {
    /* Updates result */
    _pstResult->zValue = orxSTRING_EMPTY;
  }

  /* Done! */
  return;
}

/* Command: Clamp */
void orxFASTCALL orxCommand_CommandClamp(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCOMMAND_VAR astOperandList[3];

  /* Parses numerical arguments */
  if(orxCommand_ParseNumericalArguments(_u32ArgNumber, _astArgList, astOperandList) != orxSTATUS_FAILURE)
  {
    /* All floats? */
    if((astOperandList[0].eType == orxCOMMAND_VAR_TYPE_FLOAT)
    && (astOperandList[1].eType == orxCOMMAND_VAR_TYPE_FLOAT)
    && (astOperandList[2].eType == orxCOMMAND_VAR_TYPE_FLOAT))
    {
      /* Prints value */
      orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%g", orxCLAMP(astOperandList[0].fValue, astOperandList[1].fValue, astOperandList[2].fValue));

      /* Updates result */
      _pstResult->zValue = sstCommand.acResultBuffer;
    }
    else
    {
      orxVECTOR vResult;

      /* Is operand1 a float? */
      if(astOperandList[0].eType == orxCOMMAND_VAR_TYPE_FLOAT)
      {
        /* Converts it */
        orxVector_SetAll(&(astOperandList[0].vValue), astOperandList[0].fValue);
      }
      /* Is operand2 a float? */
      if(astOperandList[1].eType == orxCOMMAND_VAR_TYPE_FLOAT)
      {
        /* Converts it */
        orxVector_SetAll(&(astOperandList[1].vValue), astOperandList[1].fValue);
      }
      /* Is operand3 a float? */
      if(astOperandList[2].eType == orxCOMMAND_VAR_TYPE_FLOAT)
      {
        /* Converts it */
        orxVector_SetAll(&(astOperandList[2].vValue), astOperandList[2].fValue);
      }

      /* Updates intermediate result */
      orxVector_Clamp(&vResult, &(astOperandList[0].vValue), &(astOperandList[1].vValue), &(astOperandList[2].vValue));

      /* Prints it */
      orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%c%g%c %g%c %g%c", orxSTRING_KC_VECTOR_START, vResult.fX, orxSTRING_KC_VECTOR_SEPARATOR, vResult.fY, orxSTRING_KC_VECTOR_SEPARATOR, vResult.fZ, orxSTRING_KC_VECTOR_END);

      /* Updates result */
      _pstResult->zValue = sstCommand.acResultBuffer;
    }
  }
  else
  {
    /* Updates result */
    _pstResult->zValue = orxSTRING_EMPTY;
  }

  /* Done! */
  return;
}

/* Command: Compare */
void orxFASTCALL orxCommand_CommandCompare(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->s32Value = ((_u32ArgNumber <= 2) || (_astArgList[2].bValue == orxFALSE)) ? orxString_ICompare(_astArgList[0].zValue, _astArgList[1].zValue) : orxString_Compare(_astArgList[0].zValue, _astArgList[1].zValue);

  /* Done! */
  return;
}

/* Command: Hash */
void orxFASTCALL orxCommand_CommandHash(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->u64Value = orxString_Hash(_astArgList[0].zValue);

  /* Done! */
  return;
}

/* Command: GetStringLength */
void orxFASTCALL orxCommand_CommandGetStringLength(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->u64Value = orxString_GetLength(_astArgList[0].zValue);

  /* Done! */
  return;
}

/* Command: GetStringID */
void orxFASTCALL orxCommand_CommandGetStringID(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->u64Value = orxString_GetID(_astArgList[0].zValue);

  /* Done! */
  return;
}

/* Command: GetStringFromID */
void orxFASTCALL orxCommand_CommandGetStringFromID(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->zValue = orxString_GetFromID(_astArgList[0].u32Value);

  /* Done! */
  return;
}

/* Command: Version */
void orxFASTCALL orxCommand_CommandVersion(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Full? */
  if((_u32ArgNumber == 0)
  || (!orxString_ICompare(_astArgList[0].zValue, "full")))
  {
    /* Prints value */
    orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%s", orxSystem_GetVersionString());
  }
  /* Numeric? */
  else if(!orxString_ICompare(_astArgList[0].zValue, "numeric"))
  {
    /* Prints value */
    orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "0x%08X", orxSystem_GetVersionNumeric());
  }
  else
  {
    orxVERSION stVersion;

    /* Retrieves version */
    orxSystem_GetVersion(&stVersion);

    /* Major? */
    if(!orxString_ICompare(_astArgList[0].zValue, "major"))
    {
      /* Prints value */
      orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%u", stVersion.u32Major);
    }
    /* Minor? */
    else if(!orxString_ICompare(_astArgList[0].zValue, "minor"))
    {
      /* Prints value */
      orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%u", stVersion.u32Minor);
    }
    /* Build? */
    else if(!orxString_ICompare(_astArgList[0].zValue, "build"))
    {
      /* Prints value */
      orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%u", stVersion.u32Build);
    }
    /* Release? */
    else if(!orxString_ICompare(_astArgList[0].zValue, "release"))
    {
      /* Prints value */
      orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%s", stVersion.zRelease);
    }
    /* Full */
    else
    {
      /* Prints value */
      orxString_NPrint(sstCommand.acResultBuffer, sizeof(sstCommand.acResultBuffer), "%s", orxSystem_GetVersionString());
    }
  }

  /* Updates result */
  _pstResult->zValue = sstCommand.acResultBuffer;

  /* Done! */
  return;
}

/** Command: GetHomeDirectory
 */
void orxFASTCALL orxCommand_CommandGetHomeDirectory(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->zValue = orxFile_GetHomeDirectory((_u32ArgNumber > 0) ? _astArgList[0].zValue : orxSTRING_EMPTY);

  /* Done! */
  return;
}

/** Command: GetApplicationSaveDirectory
 */
void orxFASTCALL orxCommand_CommandGetApplicationSaveDirectory(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->zValue = orxFile_GetApplicationSaveDirectory((_u32ArgNumber > 0) ? _astArgList[0].zValue : orxSTRING_EMPTY);

  /* Done! */
  return;
}

/** Command: LogAllStructures
 */
void orxFASTCALL orxCommand_CommandLogAllStructures(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxSTATUS eResult;

  /* Logs all structures */
  eResult = orxStructure_LogAll((_u32ArgNumber > 0) ? _astArgList[0].bValue : orxFALSE);

  /* Updates result */
  _pstResult->u32Value = (eResult != orxSTATUS_FAILURE) ? orxTRUE : orxFALSE;

  /* Done! */
  return;
}

/** Command: GetClipboard
 */
void orxFASTCALL orxCommand_CommandGetClipboard(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->zValue = orxSystem_GetClipboard();

  /* Done! */
  return;
}

/** Command: SetClipboard
 */
void orxFASTCALL orxCommand_CommandSetClipboard(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->bValue = (orxSystem_SetClipboard(_astArgList[0].zValue) != orxSTATUS_FAILURE) ? orxTRUE : orxFALSE;

  /* Done! */
  return;
}

/** Registers all the command commands
 */
static orxINLINE void orxCommand_RegisterCommands()
{
  /* Command: Help */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, Help, "Help", orxCOMMAND_VAR_TYPE_STRING, 0, 1, {"Command = <void>", orxCOMMAND_VAR_TYPE_STRING});

  /* Command: Exit */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, Exit, "Exit", orxCOMMAND_VAR_TYPE_STRING, 0, 1, {"Message = <void>", orxCOMMAND_VAR_TYPE_STRING});

  /* Command: ListCommands */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, ListCommands, "Count", orxCOMMAND_VAR_TYPE_U32, 0, 1, {"Prefix = <void>", orxCOMMAND_VAR_TYPE_STRING});

  /* Command: AddAlias */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, AddAlias, "Alias", orxCOMMAND_VAR_TYPE_STRING, 2, 1, {"Alias", orxCOMMAND_VAR_TYPE_STRING}, {"Command/Alias", orxCOMMAND_VAR_TYPE_STRING}, {"Arguments = <void>", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: RemoveAlias */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, RemoveAlias, "Alias", orxCOMMAND_VAR_TYPE_STRING, 1, 0, {"Alias", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: ListAliases */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, ListAliases, "Count", orxCOMMAND_VAR_TYPE_U32, 0, 1, {"Prefix = <void>", orxCOMMAND_VAR_TYPE_STRING});

  /* Command: Evaluate */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, Evaluate, "Result", orxCOMMAND_VAR_TYPE_STRING, 1, 1, {"Command", orxCOMMAND_VAR_TYPE_STRING}, {"GUID = <void>", orxCOMMAND_VAR_TYPE_U64});
  /* Command: EvaluateIf */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, EvaluateIf, "Result", orxCOMMAND_VAR_TYPE_STRING, 2, 1, {"Test", orxCOMMAND_VAR_TYPE_STRING}, {"If-Command", orxCOMMAND_VAR_TYPE_STRING}, {"Else-Command = <void>", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: If */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, If, "Select?", orxCOMMAND_VAR_TYPE_STRING, 2, 1, {"Test", orxCOMMAND_VAR_TYPE_STRING}, {"If-Result", orxCOMMAND_VAR_TYPE_STRING}, {"Else-Result = <void>", orxCOMMAND_VAR_TYPE_STRING});

  /* Command: Repeat */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, Repeat, "Result", orxCOMMAND_VAR_TYPE_STRING, 2, 0, {"Iterations", orxCOMMAND_VAR_TYPE_S32}, {"Command", orxCOMMAND_VAR_TYPE_STRING});

  /* Command: Return */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, Return, "Result", orxCOMMAND_VAR_TYPE_STRING, 1, 0, {"Value", orxCOMMAND_VAR_TYPE_STRING});

  /* Command: Not */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, Not, "Not", orxCOMMAND_VAR_TYPE_BOOL, 1, 0, {"Operand", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: And */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, And, "And", orxCOMMAND_VAR_TYPE_BOOL, 2, 0, {"Operand1", orxCOMMAND_VAR_TYPE_BOOL}, {"Operand2", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: Or */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, Or, "Or", orxCOMMAND_VAR_TYPE_BOOL, 2, 0, {"Operand1", orxCOMMAND_VAR_TYPE_BOOL}, {"Operand2", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: XOr */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, XOr, "XOr", orxCOMMAND_VAR_TYPE_BOOL, 2, 0, {"Operand1", orxCOMMAND_VAR_TYPE_BOOL}, {"Operand2", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: AreEqual */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, AreEqual, "Equal?", orxCOMMAND_VAR_TYPE_BOOL, 2, 0, {"Operand1", orxCOMMAND_VAR_TYPE_NUMERIC}, {"Operand2", orxCOMMAND_VAR_TYPE_NUMERIC});
  /* Command: IsGreater */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, IsGreater, "Greater?", orxCOMMAND_VAR_TYPE_BOOL, 2, 1, {"Operand1", orxCOMMAND_VAR_TYPE_NUMERIC}, {"Operand2", orxCOMMAND_VAR_TYPE_NUMERIC}, {"Strict = true", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: IsLesser */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, IsLesser, "Lesser?", orxCOMMAND_VAR_TYPE_BOOL, 2, 1, {"Operand1", orxCOMMAND_VAR_TYPE_NUMERIC}, {"Operand2", orxCOMMAND_VAR_TYPE_NUMERIC}, {"Strict = true", orxCOMMAND_VAR_TYPE_BOOL});

  /* Command: Add */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, Add, "Result", orxCOMMAND_VAR_TYPE_NUMERIC, 2, 0, {"Operand1", orxCOMMAND_VAR_TYPE_NUMERIC}, {"Operand2", orxCOMMAND_VAR_TYPE_NUMERIC});
  /* Command: Subtract */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, Subtract, "Result", orxCOMMAND_VAR_TYPE_NUMERIC, 2, 0, {"Operand1", orxCOMMAND_VAR_TYPE_NUMERIC}, {"Operand2", orxCOMMAND_VAR_TYPE_NUMERIC});
  /* Command: Multiply */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, Multiply, "Result", orxCOMMAND_VAR_TYPE_NUMERIC, 2, 0, {"Operand1", orxCOMMAND_VAR_TYPE_NUMERIC}, {"Operand2", orxCOMMAND_VAR_TYPE_NUMERIC});
  /* Command: Divide */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, Divide, "Result", orxCOMMAND_VAR_TYPE_NUMERIC, 2, 0, {"Operand1", orxCOMMAND_VAR_TYPE_NUMERIC}, {"Operand2", orxCOMMAND_VAR_TYPE_NUMERIC});
  /* Command: Modulo */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, Modulo, "Result", orxCOMMAND_VAR_TYPE_NUMERIC, 2, 0, {"Operand1", orxCOMMAND_VAR_TYPE_NUMERIC}, {"Operand2", orxCOMMAND_VAR_TYPE_NUMERIC});
  /* Command: Absolute */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, Absolute, "Result", orxCOMMAND_VAR_TYPE_NUMERIC, 1, 0, {"Operand", orxCOMMAND_VAR_TYPE_NUMERIC});
  /* Command: Negate */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, Negate, "Result", orxCOMMAND_VAR_TYPE_NUMERIC, 1, 0, {"Operand", orxCOMMAND_VAR_TYPE_NUMERIC});

  /* Command: Random */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, Random, "Result", orxCOMMAND_VAR_TYPE_NUMERIC, 0, 3, {"Boundary1 = 1.0", orxCOMMAND_VAR_TYPE_NUMERIC}, {"Boundary2 = 0.0", orxCOMMAND_VAR_TYPE_NUMERIC}, {"Step = <void>", orxCOMMAND_VAR_TYPE_NUMERIC});

  /* Command: Normalize */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, Normalize, "Result", orxCOMMAND_VAR_TYPE_VECTOR, 1, 0, {"Operand", orxCOMMAND_VAR_TYPE_VECTOR});

  /* Command: ToSpherical */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, ToSpherical, "Result", orxCOMMAND_VAR_TYPE_VECTOR, 1, 0, {"Operand", orxCOMMAND_VAR_TYPE_VECTOR});
  /* Command: ToCartesian */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, ToCartesian, "Result", orxCOMMAND_VAR_TYPE_VECTOR, 1, 0, {"Operand", orxCOMMAND_VAR_TYPE_VECTOR});

  /* Command: FromRGBToHSV */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, FromRGBToHSV, "Result", orxCOMMAND_VAR_TYPE_VECTOR, 1, 0, {"Operand", orxCOMMAND_VAR_TYPE_VECTOR});
  /* Command: FromHSVToRGB */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, FromHSVToRGB, "Result", orxCOMMAND_VAR_TYPE_VECTOR, 1, 0, {"Operand", orxCOMMAND_VAR_TYPE_VECTOR});
  /* Command: FromRGBToHSL */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, FromRGBToHSL, "Result", orxCOMMAND_VAR_TYPE_VECTOR, 1, 0, {"Operand", orxCOMMAND_VAR_TYPE_VECTOR});
  /* Command: FromHSLToRGB */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, FromHSLToRGB, "Result", orxCOMMAND_VAR_TYPE_VECTOR, 1, 0, {"Operand", orxCOMMAND_VAR_TYPE_VECTOR});

  /* Command: VectorX */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, VectorX, "Result", orxCOMMAND_VAR_TYPE_FLOAT, 1, 0, {"Operand", orxCOMMAND_VAR_TYPE_VECTOR});
  /* Command: VectorY */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, VectorY, "Result", orxCOMMAND_VAR_TYPE_FLOAT, 1, 0, {"Operand", orxCOMMAND_VAR_TYPE_VECTOR});
  /* Command: VectorZ */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, VectorZ, "Result", orxCOMMAND_VAR_TYPE_FLOAT, 1, 0, {"Operand", orxCOMMAND_VAR_TYPE_VECTOR});

  /* Command: Lerp */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, Lerp, "Result", orxCOMMAND_VAR_TYPE_NUMERIC, 3, 0, {"A", orxCOMMAND_VAR_TYPE_NUMERIC}, {"B", orxCOMMAND_VAR_TYPE_NUMERIC}, {"T", orxCOMMAND_VAR_TYPE_FLOAT});
  /* Command: Remap */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, Remap, "Result", orxCOMMAND_VAR_TYPE_NUMERIC, 5, 0, {"A1", orxCOMMAND_VAR_TYPE_NUMERIC}, {"B1", orxCOMMAND_VAR_TYPE_NUMERIC}, {"A2", orxCOMMAND_VAR_TYPE_NUMERIC}, {"B2", orxCOMMAND_VAR_TYPE_NUMERIC}, {"V", orxCOMMAND_VAR_TYPE_NUMERIC});

  /* Command: Minimum */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, Minimum, "Result", orxCOMMAND_VAR_TYPE_NUMERIC, 2, 0, {"Operand1", orxCOMMAND_VAR_TYPE_NUMERIC}, {"Operand2", orxCOMMAND_VAR_TYPE_NUMERIC});
  /* Command: Maximum */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, Maximum, "Result", orxCOMMAND_VAR_TYPE_NUMERIC, 2, 0, {"Operand1", orxCOMMAND_VAR_TYPE_NUMERIC}, {"Operand2", orxCOMMAND_VAR_TYPE_NUMERIC});
  /* Command: Clamp */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, Clamp, "Result", orxCOMMAND_VAR_TYPE_NUMERIC, 3, 0, {"Value", orxCOMMAND_VAR_TYPE_NUMERIC}, {"Minimum", orxCOMMAND_VAR_TYPE_NUMERIC}, {"Maximum", orxCOMMAND_VAR_TYPE_NUMERIC});

  /* Command: Compare */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, Compare, "Result", orxCOMMAND_VAR_TYPE_S32, 2, 1, {"String1", orxCOMMAND_VAR_TYPE_STRING}, {"String2", orxCOMMAND_VAR_TYPE_STRING}, {"CaseSensitive = false", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: Hash */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, Hash, "Hash", orxCOMMAND_VAR_TYPE_U64, 1, 0, {"String", orxCOMMAND_VAR_TYPE_STRING});

  /* Command: GetStringLength */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, GetStringLength, "Length", orxCOMMAND_VAR_TYPE_U32, 1, 0, {"String", orxCOMMAND_VAR_TYPE_STRING});

  /* Command: GetStringID */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, GetStringID, "ID", orxCOMMAND_VAR_TYPE_U64, 1, 0, {"String", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: GetStringFromID */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, GetStringFromID, "String", orxCOMMAND_VAR_TYPE_STRING, 1, 0, {"ID", orxCOMMAND_VAR_TYPE_U64});

  /* Command: Version */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, Version, "String", orxCOMMAND_VAR_TYPE_STRING, 0, 1, {"Type = full [minor|major|build|release|numeric|full]", orxCOMMAND_VAR_TYPE_STRING});

  /* Command: GetHomeDirectory */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, GetHomeDirectory, "Result", orxCOMMAND_VAR_TYPE_STRING, 0, 1, {"SubPath = <void>", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: GetApplicationSaveDirectory */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, GetApplicationSaveDirectory, "Result", orxCOMMAND_VAR_TYPE_STRING, 0, 1, {"SubPath = <void>", orxCOMMAND_VAR_TYPE_STRING});

  /* Command: LogAllStructures */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, LogAllStructures, "Success?", orxCOMMAND_VAR_TYPE_BOOL, 0, 1, {"Private = false", orxCOMMAND_VAR_TYPE_STRING});

  /* Command: GetClipboard */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, GetClipboard, "Content", orxCOMMAND_VAR_TYPE_STRING, 0, 0);
  /* Command: SetClipboard */
  orxCOMMAND_REGISTER_CORE_COMMAND(Command, SetClipboard, "Success?", orxCOMMAND_VAR_TYPE_BOOL, 1, 0, {"Content", orxCOMMAND_VAR_TYPE_STRING});

  /* Alias: Help */
  orxCommand_AddAlias("Help", "Command.Help", orxNULL);
  orxCommand_AddAlias("?", "Command.Help", orxNULL);

  /* Alias: Exit */
  orxCommand_AddAlias("Exit", "Command.Exit", orxNULL);
  /* Alias: Quit */
  orxCommand_AddAlias("Quit", "Command.Exit", orxNULL);

  /* Alias: Eval */
  orxCommand_AddAlias("Eval", "Command.Evaluate", orxNULL);
  /* Alias: EvalIf */
  orxCommand_AddAlias("EvalIf", "Command.EvaluateIf", orxNULL);

  /* Alias: Repeat */
  orxCommand_AddAlias("Repeat", "Command.Repeat", orxNULL);

  /* Alias: Return */
  orxCommand_AddAlias("Return", "Command.Return", orxNULL);

  /* Alias: Logic.If */
  orxCommand_AddAlias("Logic.If", "Command.If", orxNULL);
  /* Alias: Logic.Not */
  orxCommand_AddAlias("Logic.Not", "Command.Not", orxNULL);
  /* Alias: Logic.And */
  orxCommand_AddAlias("Logic.And", "Command.And", orxNULL);
  /* Alias: Logic.Or */
  orxCommand_AddAlias("Logic.Or", "Command.Or", orxNULL);
  /* Alias: Logic.XOr */
  orxCommand_AddAlias("Logic.XOr", "Command.XOr", orxNULL);
  /* Alias: Logic.AreEqual */
  orxCommand_AddAlias("Logic.AreEqual", "Command.AreEqual", orxNULL);
  /* Alias: Logic.IsGreater */
  orxCommand_AddAlias("Logic.IsGreater", "Command.IsGreater", orxNULL);
  /* Alias: Logic.IsLesser */
  orxCommand_AddAlias("Logic.IsLesser", "Command.IsLesser", orxNULL);

  /* Alias: If */
  orxCommand_AddAlias("If", "Logic.If", orxNULL);
  /* Alias: Not */
  orxCommand_AddAlias("Not", "Logic.Not", orxNULL);
  /* Alias: And */
  orxCommand_AddAlias("And", "Logic.And", orxNULL);
  /* Alias: Or */
  orxCommand_AddAlias("Or", "Logic.Or", orxNULL);
  /* Alias: XOr */
  orxCommand_AddAlias("XOr", "Logic.XOr", orxNULL);
  /* Alias: == */
  orxCommand_AddAlias("==", "Logic.AreEqual", orxNULL);
  /* Alias: gt */
  orxCommand_AddAlias("gt", "Logic.IsGreater", orxNULL);
  /* Alias: ly */
  orxCommand_AddAlias("lt", "Logic.IsLesser", orxNULL);

  /* Alias: Math.Add */
  orxCommand_AddAlias("Math.Add", "Command.Add", orxNULL);
  /* Alias: Math.Sub */
  orxCommand_AddAlias("Math.Sub", "Command.Subtract", orxNULL);
  /* Alias: Math.Mul */
  orxCommand_AddAlias("Math.Mul", "Command.Multiply", orxNULL);
  /* Alias: Math.Div */
  orxCommand_AddAlias("Math.Div", "Command.Divide", orxNULL);
  /* Alias: Math.Mod */
  orxCommand_AddAlias("Math.Mod", "Command.Modulo", orxNULL);
  /* Alias: Math.Abs */
  orxCommand_AddAlias("Math.Abs", "Command.Absolute", orxNULL);
  /* Alias: Math.Neg */
  orxCommand_AddAlias("Math.Neg", "Command.Negate", orxNULL);

  /* Alias: + */
  orxCommand_AddAlias("+", "Math.Add", orxNULL);
  /* Alias: - */
  orxCommand_AddAlias("-", "Math.Sub", orxNULL);
  /* Alias: * */
  orxCommand_AddAlias("*", "Math.Mul", orxNULL);
  /* Alias: / */
  orxCommand_AddAlias("/", "Math.Div", orxNULL);
  /* Alias: Mod */
  orxCommand_AddAlias("Mod", "Math.Mod", orxNULL);
  /* Alias: Abs */
  orxCommand_AddAlias("Abs", "Math.Abs", orxNULL);
  /* Alias: Neg */
  orxCommand_AddAlias("Neg", "Math.Neg", orxNULL);

  /* Alias: Math.Random */
  orxCommand_AddAlias("Math.Random", "Command.Random", orxNULL);

  /* Alias: Random */
  orxCommand_AddAlias("Random", "Math.Random", orxNULL);

  /* Alias: Vector.Normalize */
  orxCommand_AddAlias("Vector.Normalize", "Command.Normalize", orxNULL);

  /* Alias: Normalize */
  orxCommand_AddAlias("Normalize", "Vector.Normalize", orxNULL);

  /* Alias: Vector.ToSpherical */
  orxCommand_AddAlias("Vector.ToSpherical", "Command.ToSpherical", orxNULL);
  /* Alias: Vector.ToCartesian */
  orxCommand_AddAlias("Vector.ToCartesian", "Command.ToCartesian", orxNULL);

  /* Alias: ToSpherical */
  orxCommand_AddAlias("ToSpherical", "Vector.ToSpherical", orxNULL);
  /* Alias: ToCartesian */
  orxCommand_AddAlias("ToCartesian", "Vector.ToCartesian", orxNULL);

  /* Alias: Vector.FromRGBToHSV */
  orxCommand_AddAlias("Vector.FromRGBToHSV", "Command.FromRGBToHSV", orxNULL);
  /* Alias: Vector.FromHSVToRGB */
  orxCommand_AddAlias("Vector.FromHSVToRGB", "Command.FromHSVToRGB", orxNULL);
  /* Alias: Vector.FromRGBToHSL */
  orxCommand_AddAlias("Vector.FromRGBToHSL", "Command.FromRGBToHSL", orxNULL);
  /* Alias: Vector.FromHSLToRGB */
  orxCommand_AddAlias("Vector.FromHSLToRGB", "Command.FromHSLToRGB", orxNULL);

  /* Alias: FromRGBToHSV */
  orxCommand_AddAlias("FromRGBToHSV", "Vector.FromRGBToHSV", orxNULL);
  /* Alias: FromHSVToRGB */
  orxCommand_AddAlias("FromHSVToRGB", "Vector.FromHSVToRGB", orxNULL);
  /* Alias: FromRGBToHSL */
  orxCommand_AddAlias("FromRGBToHSL", "Vector.FromRGBToHSL", orxNULL);
  /* Alias: FromHSLToRGB */
  orxCommand_AddAlias("FromHSLToRGB", "Vector.FromHSLToRGB", orxNULL);

  /* Alias: Vector.X */
  orxCommand_AddAlias("Vector.X", "Command.VectorX", orxNULL);
  /* Alias: Vector.Y */
  orxCommand_AddAlias("Vector.Y", "Command.VectorY", orxNULL);
  /* Alias: Vector.Z */
  orxCommand_AddAlias("Vector.Z", "Command.VectorZ", orxNULL);

  /* Alias: Math.Lerp */
  orxCommand_AddAlias("Math.Lerp", "Command.Lerp", orxNULL);
  /* Alias: Math.Remap */
  orxCommand_AddAlias("Math.Remap", "Command.Remap", orxNULL);

  /* Alias: Math.Min */
  orxCommand_AddAlias("Math.Min", "Command.Minimum", orxNULL);
  /* Alias: Math.Max */
  orxCommand_AddAlias("Math.Max", "Command.Maximum", orxNULL);
  /* Alias: Math.Clamp */
  orxCommand_AddAlias("Math.Clamp", "Command.Clamp", orxNULL);

  /* Alias: Lerp */
  orxCommand_AddAlias("Lerp", "Math.Lerp", orxNULL);
  /* Alias: Remap */
  orxCommand_AddAlias("Remap", "Math.Remap", orxNULL);

  /* Alias: Min */
  orxCommand_AddAlias("Min", "Math.Min", orxNULL);
  /* Alias: Max */
  orxCommand_AddAlias("Max", "Math.Max", orxNULL);
  /* Alias: Clamp */
  orxCommand_AddAlias("Clamp", "Math.Clamp", orxNULL);

  /* Alias: String.Compare */
  orxCommand_AddAlias("String.Compare", "Command.Compare", orxNULL);
  /* Alias: String.Hash */
  orxCommand_AddAlias("String.Hash", "Command.Hash", orxNULL);
  /* Alias: String.CRC */
  orxCommand_AddAlias("String.CRC", "String.Hash", orxNULL);

  /* Alias: String.GetLength */
  orxCommand_AddAlias("String.GetLength", "Command.GetStringLength", orxNULL);

  /* Alias: String.GetID */
  orxCommand_AddAlias("String.GetID", "Command.GetStringID", orxNULL);
  /* Alias: String.GetFromID */
  orxCommand_AddAlias("String.GetFromID", "Command.GetStringFromID", orxNULL);

  /* Alias: Version */
  orxCommand_AddAlias("Version", "Command.Version", orxNULL);

  /* Alias: File.GetHomeDirectory */
  orxCommand_AddAlias("File.GetHomeDirectory", "Command.GetHomeDirectory", orxNULL);
  /* Alias: File.GetApplicationSaveDirectory */
  orxCommand_AddAlias("File.GetApplicationSaveDirectory", "Command.GetApplicationSaveDirectory", orxNULL);

  /* Alias: Structure.LogAll */
  orxCommand_AddAlias("Structure.LogAll", "Command.LogAllStructures", orxNULL);

  /* Alias: Clipboard.Get */
  orxCommand_AddAlias("Clipboard.Get", "Command.GetClipboard", orxNULL);
  /* Alias: Clipboard.Set */
  orxCommand_AddAlias("Clipboard.Set", "Command.SetClipboard", orxNULL);

  /* Done! */
  return;
}

/** Unregisters all the command commands
 */
static orxINLINE void orxCommand_UnregisterCommands()
{
  /* Alias: Help */
  orxCommand_RemoveAlias("Help");
  orxCommand_RemoveAlias("?");

  /* Alias: Exit */
  orxCommand_RemoveAlias("Exit");
  /* Alias: Quit */
  orxCommand_RemoveAlias("Quit");

  /* Alias: Eval */
  orxCommand_RemoveAlias("Eval");
  /* Alias: EvalIf */
  orxCommand_RemoveAlias("EvalIf");

  /* Alias: Repeat */
  orxCommand_RemoveAlias("Repeat");

  /* Alias: Return */
  orxCommand_RemoveAlias("Return");

  /* Alias: Logic.If */
  orxCommand_RemoveAlias("Logic.If");
  /* Alias: Logic.Not */
  orxCommand_RemoveAlias("Logic.Not");
  /* Alias: Logic.And */
  orxCommand_RemoveAlias("Logic.And");
  /* Alias: Logic.Or */
  orxCommand_RemoveAlias("Logic.Or");
  /* Alias: Logic.XOr */
  orxCommand_RemoveAlias("Logic.XOr");
  /* Alias: Logic.AreEqual */
  orxCommand_RemoveAlias("Logic.AreEqual");
  /* Alias: Logic.IsGreater */
  orxCommand_RemoveAlias("Logic.IsGreater");
  /* Alias: Logic.IsLesser */
  orxCommand_RemoveAlias("Logic.IsLesser");

  /* Alias: If */
  orxCommand_RemoveAlias("If");
  /* Alias: Not */
  orxCommand_RemoveAlias("Not");
  /* Alias: And */
  orxCommand_RemoveAlias("And");
  /* Alias: Or */
  orxCommand_RemoveAlias("Or");
  /* Alias: XOr */
  orxCommand_RemoveAlias("XOr");
  /* Alias: == */
  orxCommand_RemoveAlias("==");
  /* Alias: gt */
  orxCommand_RemoveAlias("gt");
  /* Alias: lt */
  orxCommand_RemoveAlias("lt");

  /* Alias: Math.Add */
  orxCommand_RemoveAlias("Math.Add");
  /* Alias: Math.Sub */
  orxCommand_RemoveAlias("Math.Sub");
  /* Alias: Math.Mul */
  orxCommand_RemoveAlias("Math.Mul");
  /* Alias: Math.Div */
  orxCommand_RemoveAlias("Math.Div");
  /* Alias: Math.Mod */
  orxCommand_RemoveAlias("Math.Mod");
  /* Alias: Math.Abs */
  orxCommand_RemoveAlias("Math.Abs");
  /* Alias: Math.Neg */
  orxCommand_RemoveAlias("Math.Neg");

  /* Alias: + */
  orxCommand_RemoveAlias("+");
  /* Alias: - */
  orxCommand_RemoveAlias("-");
  /* Alias: * */
  orxCommand_RemoveAlias("*");
  /* Alias: / */
  orxCommand_RemoveAlias("/");
  /* Alias: Mod */
  orxCommand_RemoveAlias("Mod");
  /* Alias: Abs */
  orxCommand_RemoveAlias("Abs");
  /* Alias: Neg */
  orxCommand_RemoveAlias("Neg");

  /* Alias: Math.Random */
  orxCommand_RemoveAlias("Math.Random");

  /* Alias: Random */
  orxCommand_RemoveAlias("Random");

  /* Alias: Vector.Normalize */
  orxCommand_RemoveAlias("Vector.Normalize");

  /* Alias: Normalize */
  orxCommand_RemoveAlias("Normalize");

  /* Alias: Vector.ToSpherical */
  orxCommand_RemoveAlias("Vector.ToSpherical");
  /* Alias: Vector.ToCartesian */
  orxCommand_RemoveAlias("Vector.ToCartesian");

  /* Alias: ToSpherical */
  orxCommand_RemoveAlias("ToSpherical");
  /* Alias: ToCartesian */
  orxCommand_RemoveAlias("ToCartesian");

  /* Alias: Vector.FromRGBToHSV */
  orxCommand_RemoveAlias("Vector.FromRGBToHSV");
  /* Alias: Vector.FromHSVToRGB */
  orxCommand_RemoveAlias("Vector.FromHSVToRGB");
  /* Alias: Vector.FromRGBToHSL */
  orxCommand_RemoveAlias("Vector.FromRGBToHSL");
  /* Alias: Vector.FromHSLToRGB */
  orxCommand_RemoveAlias("Vector.FromHSLToRGB");

  /* Alias: FromRGBToHSV */
  orxCommand_RemoveAlias("FromRGBToHSV");
  /* Alias: FromHSVToRGB */
  orxCommand_RemoveAlias("FromHSVToRGB");
  /* Alias: FromRGBToHSL */
  orxCommand_RemoveAlias("FromRGBToHSL");
  /* Alias: FromHSLToRGB */
  orxCommand_RemoveAlias("FromHSLToRGB");

  /* Alias: Vector.X */
  orxCommand_RemoveAlias("Vector.X");
  /* Alias: Vector.Y */
  orxCommand_RemoveAlias("Vector.Y");
  /* Alias: Vector.Z */
  orxCommand_RemoveAlias("Vector.Z");

  /* Alias: Math.Lerp */
  orxCommand_RemoveAlias("Math.Lerp");
  /* Alias: Math.Remap */
  orxCommand_RemoveAlias("Math.Remap");

  /* Alias: Math.Min */
  orxCommand_RemoveAlias("Math.Min");
  /* Alias: Math.Max */
  orxCommand_RemoveAlias("Math.Max");
  /* Alias: Math.Clamp */
  orxCommand_RemoveAlias("Math.Clamp");

  /* Alias: Lerp */
  orxCommand_RemoveAlias("Lerp");
  /* Alias: Remap */
  orxCommand_RemoveAlias("Remap");

  /* Alias: Min */
  orxCommand_RemoveAlias("Min");
  /* Alias: Max */
  orxCommand_RemoveAlias("Max");
  /* Alias: Clamp */
  orxCommand_RemoveAlias("Clamp");

  /* Alias: String.Compare */
  orxCommand_RemoveAlias("String.Compare");
  /* Alias: String.Hash */
  orxCommand_RemoveAlias("String.Hash");
  /* Alias: String.CRC */
  orxCommand_RemoveAlias("String.CRC");

  /* Alias: String.GetLength */
  orxCommand_RemoveAlias("String.GetLength");

  /* Alias: String.GetID */
  orxCommand_RemoveAlias("String.GetID");
  /* Alias: String.GetFromID */
  orxCommand_RemoveAlias("String.GetFromID");

  /* Alias: Version */
  orxCommand_RemoveAlias("Version");

  /* Alias: File.GetHomeDirectory */
  orxCommand_RemoveAlias("File.GetHomeDirectory");
  /* Alias: File.GetApplicationSaveDirectory */
  orxCommand_RemoveAlias("File.GetApplicationSaveDirectory");

  /* Alias: Structure.LogAll */
  orxCommand_RemoveAlias("Structure.LogAll");

  /* Alias: Clipboard.Get */
  orxCommand_RemoveAlias("Clipboard.Get");
  /* Alias: Clipboard.Set */
  orxCommand_RemoveAlias("Clipboard.Set");

  /* Command: Help */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, Help);

  /* Command: Exit */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, Exit);

  /* Command: ListCommands */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, ListCommands);

  /* Command: AddAlias */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, AddAlias);
  /* Command: RemoveAlias */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, RemoveAlias);
  /* Command: ListAliases */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, ListAliases);

  /* Command: Evaluate */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, Evaluate);
  /* Command: EvaluateIf */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, EvaluateIf);
  /* Command: If */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, If);

  /* Command: Repeat */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, Repeat);

  /* Command: Return */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, Return);

  /* Command: Not */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, Not);
  /* Command: And */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, And);
  /* Command: Or */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, Or);
  /* Command: XOr */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, XOr);
  /* Command: AreEqual */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, AreEqual);
  /* Command: IsGreater */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, IsGreater);
  /* Command: IsLesser */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, IsLesser);

  /* Command: Add */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, Add);
  /* Command: Subtract */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, Subtract);
  /* Command: Multiply */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, Multiply);
  /* Command: Divide */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, Divide);
  /* Command: Modulo */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, Modulo);
  /* Command: Absolute */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, Absolute);
  /* Command: Negate */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, Negate);

  /* Command: Random */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, Random);

  /* Command: Normalize */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, Normalize);

  /* Command: ToSpherical */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, ToSpherical);
  /* Command: ToCartesian */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, ToCartesian);

  /* Command: FromRGBToHSV */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, FromRGBToHSV);
  /* Command: FromHSVToRGB */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, FromHSVToRGB);
  /* Command: FromRGBToHSL */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, FromRGBToHSL);
  /* Command: FromHSLToRGB */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, FromHSLToRGB);

  /* Command: VectorX */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, VectorX);
  /* Command: VectorY */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, VectorY);
  /* Command: VectorZ */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, VectorZ);

  /* Command: Lerp */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, Lerp);
  /* Command: Remap */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, Remap);

  /* Command: Minimum */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, Minimum);
  /* Command: Maximum */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, Maximum);
  /* Command: Clamp */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, Clamp);

  /* Command: Compare */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, Compare);
  /* Command: Hash */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, Hash);

  /* Command: GetStringLength */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, GetStringLength);

  /* Command: GetStringID */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, GetStringID);
  /* Command: GetStringFromID */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, GetStringFromID);

  /* Command: Version */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, Version);

  /* Command: GetHomeDirectory */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, GetHomeDirectory);
  /* Command: GetApplicationSaveDirectory */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, GetApplicationSaveDirectory);

  /* Command: LogAllStructures */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, LogAllStructures);

  /* Command: GetClipboard */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, GetClipboard);
  /* Command: SetClipboard */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Command, SetClipboard);

  /* Done! */
  return;
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
  orxModule_AddDependency(orxMODULE_ID_COMMAND, orxMODULE_ID_STRING);
  orxModule_AddDependency(orxMODULE_ID_COMMAND, orxMODULE_ID_EVENT);
  orxModule_AddDependency(orxMODULE_ID_COMMAND, orxMODULE_ID_FILE);
  orxModule_AddDependency(orxMODULE_ID_COMMAND, orxMODULE_ID_SYSTEM);
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
    if((orxEvent_AddHandler(orxEVENT_TYPE_TIMELINE, orxCommand_EventHandler) != orxSTATUS_FAILURE)
    && (orxEvent_AddHandler(orxEVENT_TYPE_ANIM, orxCommand_EventHandler) != orxSTATUS_FAILURE))
    {
      /* Filters relevant event IDs */
      orxEvent_SetHandlerIDFlags(orxCommand_EventHandler, orxEVENT_TYPE_TIMELINE, orxNULL, orxEVENT_GET_FLAG(orxTIMELINE_EVENT_TRIGGER), orxEVENT_KU32_MASK_ID_ALL);
      orxEvent_SetHandlerIDFlags(orxCommand_EventHandler, orxEVENT_TYPE_ANIM, orxNULL, orxEVENT_GET_FLAG(orxANIM_EVENT_CUSTOM_EVENT), orxEVENT_KU32_MASK_ID_ALL);

      /* Creates banks */
      sstCommand.pstBank        = orxBank_Create(orxCOMMAND_KU32_BANK_SIZE, sizeof(orxCOMMAND), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
      sstCommand.pstTrieBank    = orxBank_Create(orxCOMMAND_KU32_TRIE_BANK_SIZE, sizeof(orxCOMMAND_TRIE_NODE), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
      sstCommand.pstResultBank  = orxBank_Create(orxCOMMAND_KU32_RESULT_BANK_SIZE, sizeof(orxCOMMAND_STACK_ENTRY), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

      /* Valid? */
      if((sstCommand.pstBank != orxNULL) && (sstCommand.pstTrieBank != orxNULL) && (sstCommand.pstResultBank != orxNULL))
      {
        orxCOMMAND_TRIE_NODE *pstTrieRoot;

        /* Allocates trie root */
        pstTrieRoot = (orxCOMMAND_TRIE_NODE *)orxBank_Allocate(sstCommand.pstTrieBank);

        /* Success? */
        if(pstTrieRoot != orxNULL)
        {
          /* Inits it */
          orxMemory_Zero(pstTrieRoot, sizeof(orxCOMMAND_TRIE_NODE));

          /* Adds it to the trie */
          if(orxTree_AddRoot(&(sstCommand.stCommandTrie), &(pstTrieRoot->stNode)) != orxSTATUS_FAILURE)
          {
            /* Inits Flags */
            sstCommand.u32Flags = orxCOMMAND_KU32_STATIC_FLAG_READY;

            /* Registers commands */
            orxCommand_RegisterCommands();

            /* Inits buffers */
            sstCommand.acEvaluateBuffer[orxCOMMAND_KU32_EVALUATE_BUFFER_SIZE - 1]   = orxCHAR_NULL;
            sstCommand.acProcessBuffer[orxCOMMAND_KU32_PROCESS_BUFFER_SIZE - 1]     = orxCHAR_NULL;
            sstCommand.acPrototypeBuffer[orxCOMMAND_KU32_PROTOTYPE_BUFFER_SIZE - 1] = orxCHAR_NULL;

            /* Inits offsets */
            sstCommand.s32EvaluateOffset  =
            sstCommand.s32ProcessOffset   = 0;

            /* Updates result */
            eResult = orxSTATUS_SUCCESS;
          }
        }

        /* Failure? */
        if(eResult == orxSTATUS_FAILURE)
        {
          /* Removes event handler */
          orxEvent_RemoveHandler(orxEVENT_TYPE_TIMELINE, orxCommand_EventHandler);
          orxEvent_RemoveHandler(orxEVENT_TYPE_ANIM, orxCommand_EventHandler);

          /* Deletes banks */
          orxBank_Delete(sstCommand.pstBank);
          orxBank_Delete(sstCommand.pstTrieBank);
          orxBank_Delete(sstCommand.pstResultBank);

          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_COMMAND, "Failed to create command trie.");
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
        orxEvent_RemoveHandler(orxEVENT_TYPE_ANIM, orxCommand_EventHandler);

        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_COMMAND, "Failed to create command banks.");
      }
    }
    else
    {
      /* Removes event handler */
      orxEvent_RemoveHandler(orxEVENT_TYPE_TIMELINE, orxCommand_EventHandler);
      orxEvent_RemoveHandler(orxEVENT_TYPE_ANIM, orxCommand_EventHandler);

      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_COMMAND, "Failed to register event handler.");
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_COMMAND, "Tried to initialize command module when it was already initialized.");

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

    /* Unregisters commands */
    orxCommand_UnregisterCommands();

    /* Clears trie */
    orxTree_Clean(&(sstCommand.stCommandTrie));

    /* For all remaining commands */
    for(pstCommand = (orxCOMMAND *)orxBank_GetNext(sstCommand.pstBank, orxNULL);
        pstCommand != orxNULL;
        pstCommand = (orxCOMMAND *)orxBank_GetNext(sstCommand.pstBank, pstCommand))
    {
      /* Is an alias? */
      if(pstCommand->bIsAlias != orxFALSE)
      {
        /* Deletes its aliased command name */
        orxString_Delete(pstCommand->zAliasedCommandName);

        /* Has arguments? */
        if(pstCommand->zArgs != orxNULL)
        {
          /* Deletes it */
          orxString_Delete(pstCommand->zArgs);
        }
      }
      /* Command */
      else
      {
        /* Deletes its variables */
        orxMemory_Free(pstCommand->astParamList);
      }
    }

    /* Deletes banks */
    orxBank_Delete(sstCommand.pstBank);
    orxBank_Delete(sstCommand.pstTrieBank);
    orxBank_Delete(sstCommand.pstResultBank);

    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_TIMELINE, orxCommand_EventHandler);
    orxEvent_RemoveHandler(orxEVENT_TYPE_ANIM, orxCommand_EventHandler);

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
  orxASSERT(_pfnFunction != orxNULL);
  orxASSERT(_u32RequiredParamNumber <= 0xFFFF);
  orxASSERT(_u32OptionalParamNumber <= 0xFFFF);
  orxASSERT((_astParamList != orxNULL) || ((_u32RequiredParamNumber == 0) && (_u32OptionalParamNumber == 0)));
  orxASSERT(_pstResult != orxNULL);

  /* Valid? */
  if((_zCommand != orxNULL) && (*_zCommand != orxCHAR_NULL))
  {
    /* Not already registered? */
    if(orxCommand_FindNoAlias(_zCommand) == orxNULL)
    {
      orxCOMMAND *pstCommand;

      /* Allocates command */
      pstCommand = (orxCOMMAND *)orxBank_Allocate(sstCommand.pstBank);

      /* Valid? */
      if(pstCommand != orxNULL)
      {
        orxU32 i;

        /* Inits it */
        orxMemory_Zero(pstCommand, sizeof(orxCOMMAND));
        pstCommand->stResult.stNameID       = orxString_GetID(_pstResult->zName);
        pstCommand->stResult.eType          = _pstResult->eType;
        pstCommand->bIsAlias                = orxFALSE;
        pstCommand->pfnFunction             = _pfnFunction;
        pstCommand->stNameID                = orxString_GetID(_zCommand);
        pstCommand->u16RequiredParamNumber  = (orxU16)_u32RequiredParamNumber;
        pstCommand->u16OptionalParamNumber  = (orxU16)_u32OptionalParamNumber;

        /* Allocates parameter list */
        pstCommand->astParamList = (orxCOMMAND_VAR_INFO *)orxMemory_Allocate((_u32RequiredParamNumber + _u32OptionalParamNumber) * sizeof(orxCOMMAND_VAR_INFO), orxMEMORY_TYPE_MAIN);

        /* Checks */
        orxASSERT(pstCommand->astParamList != orxNULL);

        /* For all parameters */
        for(i = 0; i < _u32RequiredParamNumber + _u32OptionalParamNumber; i++)
        {
          /* Inits it */
          pstCommand->astParamList[i].stNameID  = orxString_GetID(_astParamList[i].zName);
          pstCommand->astParamList[i].eType     = _astParamList[i].eType;
        }

        /* Inserts in trie */
        orxCommand_InsertInTrie(pstCommand);

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_COMMAND, "Can't allocate memory for command [%s], aborting.", _zCommand);
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_COMMAND, "Can't register command: [%s] is already registered.", _zCommand);
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

  /* Valid? */
  if(_zCommand != orxNULL)
  {
    orxCOMMAND *pstCommand;

    /* Gets it */
    pstCommand = orxCommand_FindNoAlias(_zCommand);

    /* Found? */
    if((pstCommand != orxNULL) && (pstCommand->bIsAlias == orxFALSE))
    {
      /* Removes it from trie */
      orxCommand_RemoveFromTrie(pstCommand);

      /* Deletes its variables */
      orxMemory_Free(pstCommand->astParamList);

      /* Deletes it */
      orxBank_Free(sstCommand.pstBank, pstCommand);

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_COMMAND, "Can't unregister command: [%s] is not registered.", _zCommand);
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

  /* Valid? */
  if(_zCommand != orxNULL)
  {
    /* Updates result */
    bResult = (orxCommand_FindNoAlias(_zCommand) != orxNULL) ? orxTRUE : orxFALSE;
  }
  else
  {
    /* Updates result */
    bResult = orxFALSE;
  }

  /* Done! */
  return bResult;
}

/** Adds a command alias
* @param[in]   _zAlias        Command alias
* @param[in]   _zCommand      Command name
* @param[in]   _zArgs         Command argument, orxNULL for none
* @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
*/
orxSTATUS orxFASTCALL orxCommand_AddAlias(const orxSTRING _zAlias, const orxSTRING _zCommand, const orxSTRING _zArgs)
{
  const orxSTRING zAlias;
  orxSTATUS       eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstCommand.u32Flags, orxCOMMAND_KU32_STATIC_FLAG_READY));
  orxASSERT(_zCommand != orxNULL);

  /* Finds start of alias */
  zAlias = orxString_SkipWhiteSpaces(_zAlias);

  /* Valid? */
  if((zAlias != orxNULL) && (*zAlias != orxCHAR_NULL))
  {
    orxCOMMAND_TRIE_NODE *pstAliasNode;

    /* Finds alias node */
    pstAliasNode = orxCommand_FindTrieNode(zAlias, orxTRUE);

    /* Valid? */
    if(pstAliasNode != orxNULL)
    {
      /* Not already used as a command? */
      if((pstAliasNode->pstCommand == orxNULL) || (pstAliasNode->pstCommand->bIsAlias != orxFALSE))
      {
        /* Not self referencing? */
        if(orxString_Compare(zAlias, _zCommand) != 0)
        {
          orxCOMMAND_TRIE_NODE *pstNode;

          /* Updates result */
          eResult = orxSTATUS_SUCCESS;

          /* For all aliases */
          for(pstNode = orxCommand_FindTrieNode(_zCommand, orxFALSE);
              (pstNode != orxNULL) && (pstNode->pstCommand != orxNULL) && (pstNode->pstCommand->bIsAlias != orxFALSE);
              pstNode = orxCommand_FindTrieNode(pstNode->pstCommand->zAliasedCommandName, orxFALSE))
          {
            /* Creates a loop? */
            if(orxString_Compare(zAlias, pstNode->pstCommand->zAliasedCommandName) == 0)
            {
              /* Updates result */
              eResult = orxSTATUS_FAILURE;

              break;
            }
          }
        }

        /* Valid? */
        if(eResult != orxSTATUS_FAILURE)
        {
          /* Not existing? */
          if(pstAliasNode->pstCommand == orxNULL)
          {
            /* Creates it */
            pstAliasNode->pstCommand = (orxCOMMAND *)orxBank_Allocate(sstCommand.pstBank);

            /* Valid? */
            if(pstAliasNode->pstCommand != orxNULL)
            {
              /* Inits */
              orxMemory_Zero(pstAliasNode->pstCommand, sizeof(orxCOMMAND));
              pstAliasNode->pstCommand->stNameID  = orxString_GetID(zAlias);
              pstAliasNode->pstCommand->bIsAlias  = orxTRUE;
            }
            else
            {
              /* Logs message */
              orxDEBUG_PRINT(orxDEBUG_LEVEL_COMMAND, "Can't allocate memory for alias [%s], aborting.", zAlias);

              /* Updates result */
              eResult = orxSTATUS_FAILURE;
            }
          }
          else
          {
            /* Logs message */
            orxDEBUG_PRINT(orxDEBUG_LEVEL_COMMAND, "Updating alias [%s]: now pointing to [%s], previously [%s].", zAlias, _zCommand, pstAliasNode->pstCommand->zAliasedCommandName);

            /* Delete old aliased name */
            orxString_Delete(pstAliasNode->pstCommand->zAliasedCommandName);

            /* Had arguments? */
            if(pstAliasNode->pstCommand->zArgs != orxNULL)
            {
              /* Deletes them */
              orxString_Delete(pstAliasNode->pstCommand->zArgs);
            }
          }

          /* Success? */
          if(eResult != orxSTATUS_FAILURE)
          {
            /* Updates aliased name */
            pstAliasNode->pstCommand->zAliasedCommandName = orxString_Duplicate(_zCommand);
            pstAliasNode->pstCommand->zArgs               = (_zArgs != orxNULL) ? orxString_Duplicate(_zArgs) : orxNULL;
          }
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_COMMAND, "Can't add/modify alias [%s] -> [%s] as it's creating a loop, aborting.", zAlias, _zCommand);
        }
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_COMMAND, "Failed to add alias: [%s] is already registered as a command.", zAlias);
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_COMMAND, "Failed to add alias [%s]: couldn't insert it in trie.", zAlias);
    }
  }

  /* Done! */
  return eResult;
}

/** Removes a command alias
* @param[in]   _zAlias        Command alias
* @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
*/
orxSTATUS orxFASTCALL orxCommand_RemoveAlias(const orxSTRING _zAlias)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstCommand.u32Flags, orxCOMMAND_KU32_STATIC_FLAG_READY));

  /* Valid? */
  if(_zAlias != orxNULL)
  {
    orxCOMMAND_TRIE_NODE *pstNode;

    /* Finds its node */
    pstNode = orxCommand_FindTrieNode(_zAlias, orxFALSE);

    /* Success? */
    if(pstNode != orxNULL)
    {
      /* Is an alias? */
      if((pstNode->pstCommand != orxNULL) && (pstNode->pstCommand->bIsAlias != orxFALSE))
      {
        /* Deletes its aliased name */
        orxString_Delete(pstNode->pstCommand->zAliasedCommandName);

        /* Has arguments? */
        if(pstNode->pstCommand->zArgs != orxNULL)
        {
          /* Deletes it */
          orxString_Delete(pstNode->pstCommand->zArgs);
        }
        /* Deletes it */
        orxBank_Free(sstCommand.pstBank, pstNode->pstCommand);

        /* Removes its reference */
        pstNode->pstCommand = orxNULL;

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_COMMAND, "Failed to remove alias: [%s] is a command, not an alias.", _zAlias);
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_COMMAND, "Failed to remove alias: [%s] alias not found.", _zAlias);
    }
  }

  /* Done! */
  return eResult;
}

/** Is a command alias?
* @param[in]   _zAlias        Command alias
* @return      orxTRUE / orxFALSE
*/
orxBOOL orxFASTCALL orxCommand_IsAlias(const orxSTRING _zAlias)
{
  orxBOOL bResult = orxFALSE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstCommand.u32Flags, orxCOMMAND_KU32_STATIC_FLAG_READY));

  /* Valid? */
  if(_zAlias != orxNULL)
  {
    orxCOMMAND_TRIE_NODE *pstNode;

    /* Finds its node */
    pstNode = orxCommand_FindTrieNode(_zAlias, orxFALSE);

    /* Success? */
    if(pstNode != orxNULL)
    {
      /* Is an alias? */
      if((pstNode->pstCommand != orxNULL) && (pstNode->pstCommand->bIsAlias != orxFALSE))
      {
        /* Updates result */
        bResult = orxTRUE;
      }
    }
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

  /* Valid? */
  if((_zCommand != orxNULL) & (_zCommand != orxSTRING_EMPTY))
  {
    orxCOMMAND *pstCommand;

    /* Gets command */
    pstCommand = orxCommand_FindNoAlias(_zCommand);

    /* Success? */
    if(pstCommand != orxNULL)
    {
      orxU32 i, u32Size;

      /* Prints result and function name */
      u32Size = orxString_NPrint(sstCommand.acPrototypeBuffer, sizeof(sstCommand.acPrototypeBuffer), "{%s %s} %s", orxCommand_GetTypeString(pstCommand->stResult.eType), orxString_GetFromID(pstCommand->stResult.stNameID), orxString_GetFromID(pstCommand->stNameID));

      /* For all required arguments */
      for(i = 0; i < (orxU32)pstCommand->u16RequiredParamNumber; i++)
      {
        /* Prints it */
        u32Size += orxString_NPrint(sstCommand.acPrototypeBuffer + u32Size, sizeof(sstCommand.acPrototypeBuffer) - u32Size, " (%s %s)", orxCommand_GetTypeString(pstCommand->astParamList[i].eType), orxString_GetFromID(pstCommand->astParamList[i].stNameID));
      }

      /* For all optional arguments */
      for(; i < (orxU32)pstCommand->u16RequiredParamNumber + (orxU32)pstCommand->u16OptionalParamNumber; i++)
      {
        /* Prints it */
        u32Size += orxString_NPrint(sstCommand.acPrototypeBuffer + u32Size, sizeof(sstCommand.acPrototypeBuffer) - u32Size, " [%s %s]", orxCommand_GetTypeString(pstCommand->astParamList[i].eType), orxString_GetFromID(pstCommand->astParamList[i].stNameID));
      }

      /* Had no parameters? */
      if(i == 0)
      {
        /* Prints function end */
        u32Size += orxString_NPrint(sstCommand.acPrototypeBuffer + u32Size, sizeof(sstCommand.acPrototypeBuffer) - u32Size, " <void>");
      }

      /* Updates result */
      zResult = sstCommand.acPrototypeBuffer;
    }
  }

  /* Done! */
  return zResult;
}

/** Gets next command using an optional base
* @param[in]   _zBase             Base name, can be set to orxNULL for no base
* @param[in]   _zPrevious         Previous command, orxNULL to get the first command
* @param[out]  _pu32CommonLength  Length of the common prefix of all potential results, orxNULL to ignore
* @return      Next command found, orxNULL if none
*/
const orxSTRING orxFASTCALL orxCommand_GetNext(const orxSTRING _zBase, const orxSTRING _zPrevious, orxU32 *_pu32CommonLength)
{
  orxCOMMAND_TRIE_NODE *pstBaseNode;
  const orxSTRING       zResult = orxNULL;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstCommand.u32Flags, orxCOMMAND_KU32_STATIC_FLAG_READY));

  /* Has a base? */
  if(_zBase != orxNULL)
  {
    /* Finds base node */
    pstBaseNode = orxCommand_FindTrieNode(_zBase, orxFALSE);

    /* Inits common length */
    if(_pu32CommonLength != orxNULL)
    {
      *_pu32CommonLength = orxString_GetLength(_zBase);
    }
  }
  else
  {
    /* Uses root as base node */
    pstBaseNode = (orxCOMMAND_TRIE_NODE *)orxTree_GetRoot(&(sstCommand.stCommandTrie));

    /* Clears common length */
    if(_pu32CommonLength != orxNULL)
    {
      *_pu32CommonLength = 0;
    }
  }

  /* Found a valid base? */
  if(pstBaseNode != orxNULL)
  {
    orxCOMMAND_TRIE_NODE *pstPreviousNode;
    const orxCOMMAND     *pstNextCommand = orxNULL;
    const orxSTRING       zCommandName;

    /* Has previous command? */
    if(_zPrevious != orxNULL)
    {
      /* Gets its node */
      pstPreviousNode = orxCommand_FindTrieNode(_zPrevious, orxFALSE);

      /* Found? */
      if((pstPreviousNode != orxNULL) && (pstPreviousNode->pstCommand != orxNULL))
      {
        /* Different than base? */
        if(pstPreviousNode != pstBaseNode)
        {
          orxCOMMAND_TRIE_NODE *pstParent;

          /* Finds parent base node */
          for(pstParent = (orxCOMMAND_TRIE_NODE *)orxTree_GetParent(&(pstPreviousNode->stNode));
              (pstParent != orxNULL) && (pstParent != pstBaseNode);
              pstParent = (orxCOMMAND_TRIE_NODE *)orxTree_GetParent(&(pstParent->stNode)));

          /* Not found? */
          if(pstParent == orxNULL)
          {
            /* Logs message */
            orxDEBUG_PRINT(orxDEBUG_LEVEL_COMMAND, "[%s] is not a valid base of command [%s]: ignoring previous command parameter.", _zBase, _zPrevious);

            /* Resets previous command node */
            pstPreviousNode = orxNULL;
          }
        }
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_COMMAND, "[%s] is not a valid previous command: ignoring previous command parameter.", _zPrevious);
      }
    }
    else
    {
      /* Ignores previous command */
      pstPreviousNode = orxNULL;
    }

    /* Is child of base valid? */
    if(orxTree_GetChild(&(pstBaseNode->stNode)) != orxNULL)
    {
      /* Finds next command */
      pstNextCommand = orxCommand_FindNext((pstBaseNode->pstCommand == orxNULL) ? (orxCOMMAND_TRIE_NODE *)orxTree_GetChild(&(pstBaseNode->stNode)) : pstBaseNode, &pstPreviousNode);

      /* Found? */
      if(pstNextCommand != orxNULL)
      {
        /* Gets command name */
        zCommandName = orxString_GetFromID(pstNextCommand->stNameID);

        /* Isn't base a command? */
        if(pstBaseNode->pstCommand == orxNULL)
        {
          orxCOMMAND_TRIE_NODE *pstNode, *pstParent;
          orxS32                i, u32Position;

          /* Finds prefix node position */
          for(pstNode = orxCommand_FindTrieNode(zCommandName, orxFALSE), pstParent = (orxCOMMAND_TRIE_NODE *)orxTree_GetParent(&(pstNode->stNode)), i = 0, u32Position = -1;
              pstNode != pstBaseNode;
              pstNode = pstParent, pstParent = (orxCOMMAND_TRIE_NODE *)orxTree_GetParent(&(pstNode->stNode)), i++)
          {
            /* Has sibling, or is a command? */
            if((orxTree_GetSibling(&(pstNode->stNode)) != orxNULL) || ((orxCOMMAND_TRIE_NODE *)orxTree_GetChild(&(pstParent->stNode)) != pstNode) || (pstParent->pstCommand != orxNULL))
            {
              /* Updates position */
              u32Position = i;
            }
          }

          /* Updates prefix length */
          if(_pu32CommonLength != orxNULL)
          {
            *_pu32CommonLength = orxString_GetLength(zCommandName) - u32Position - 1;
          }
        }
      }
    }

    /* No command found? */
    if(pstNextCommand == orxNULL)
    {
      /* Is base a different command? */
      if((pstBaseNode->pstCommand != orxNULL) && (pstPreviousNode != orxNULL) && (pstBaseNode != pstPreviousNode))
      {
        /* Gets it */
        pstNextCommand = pstBaseNode->pstCommand;

        /* Gets its name */
        zCommandName = orxString_GetFromID(pstNextCommand->stNameID);
      }
      else
      {
        /* Clears command & name */
        pstNextCommand  = orxNULL;
        zCommandName    = orxNULL;
      }
    }

    /* Found? */
    if(pstNextCommand != orxNULL)
    {
      /* Updates result */
      zResult = zCommandName;
    }
    else
    {
      /* Clears common length */
      if(_pu32CommonLength != orxNULL)
      {
        *_pu32CommonLength = 0;
      }
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_COMMAND, "Failed to get next command using base [%s]: base not found.", _zBase);
  }

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
  orxCOMMAND_VAR *pstResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstCommand.u32Flags, orxCOMMAND_KU32_STATIC_FLAG_READY));
  orxASSERT(_pstResult != orxNULL);

  /* Evaluates it */
  pstResult = orxCommand_EvaluateWithGUID(_zCommandLine, orxU64_UNDEFINED, _pstResult);

  /* Done! */
  return pstResult;
}

/** Evaluates a command with a specific GUID
* @param[in]   _zCommandLine  Command name + arguments
* @param[in]   _u64GUID       GUID to use in place of the GUID markers in the command
* @param[out]  _pstResult     Variable that will contain the result
* @return      Command result if found, orxNULL otherwise
*/
orxCOMMAND_VAR *orxFASTCALL orxCommand_EvaluateWithGUID(const orxSTRING _zCommandLine, orxU64 _u64GUID, orxCOMMAND_VAR *_pstResult)
{
  orxCOMMAND_VAR *pstResult = orxNULL;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstCommand.u32Flags, orxCOMMAND_KU32_STATIC_FLAG_READY));
  orxASSERT(_pstResult != orxNULL);

  /* Valid? */
  if((_zCommandLine != orxNULL) && (*_zCommandLine != orxCHAR_NULL))
  {
    orxS32 s32Offset;

    /* Stores it in buffer */
    s32Offset = orxString_NPrint(sstCommand.acEvaluateBuffer + sstCommand.s32EvaluateOffset, sizeof(sstCommand.acEvaluateBuffer) - sstCommand.s32EvaluateOffset, "%s", _zCommandLine) + 1;
    sstCommand.s32EvaluateOffset += s32Offset;

    /* Processes it */
    pstResult = orxCommand_Process(sstCommand.acEvaluateBuffer + sstCommand.s32EvaluateOffset - s32Offset, _u64GUID, _pstResult, orxFALSE);

    /* Restores buffer offset */
    sstCommand.s32EvaluateOffset -= s32Offset;
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

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxCommand_Execute");

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstCommand.u32Flags, orxCOMMAND_KU32_STATIC_FLAG_READY));
  orxASSERT(_pstResult != orxNULL);

  /* Valid? */
  if((_zCommand != orxNULL) & (_zCommand != orxSTRING_EMPTY))
  {
    orxCOMMAND_TRIE_NODE *pstCommandNode;

    /* Gets its node */
    pstCommandNode = orxCommand_FindTrieNode(_zCommand, orxFALSE);

    /* Found? */
    if((pstCommandNode != orxNULL) && (pstCommandNode->pstCommand != orxNULL))
    {
      /* Not an alias? */
      if(pstCommandNode->pstCommand->bIsAlias == orxFALSE)
      {
        /* Runs it */
        pstResult = orxCommand_Run(pstCommandNode->pstCommand, orxTRUE, _u32ArgNumber, _astArgList, _pstResult);
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_COMMAND, "Can't execute command: [%s] is an alias, not a command.", _zCommand);
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_COMMAND, "Can't execute command: [%s] is not registered.", _zCommand);
    }
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return pstResult;
}

/** Parses numerical arguments, string arguments will be evaluated to vectors or float when possible
* @param[in]   _u32ArgNumber  Number of arguments to parse
* @param[in]   _astArgList    List of arguments to parse
* @param[out]  _astOperandList List of parsed arguments
* @return orxSTATUS_SUCCESS if all numerical arguments have been correctly interpreted, orxSTATUS_FAILURE otherwise
*/
orxSTATUS orxFASTCALL orxCommand_ParseNumericalArguments(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_astOperandList)
{
  orxU32    i;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstCommand.u32Flags, orxCOMMAND_KU32_STATIC_FLAG_READY));
  orxASSERT(_astArgList != orxNULL);
  orxASSERT(_astOperandList != orxNULL);

  /* For all arguments */
  for(i = 0; i < _u32ArgNumber; i++)
  {
    /* Is a string or a numeric? */
    if((_astArgList[i].eType == orxCOMMAND_VAR_TYPE_STRING)
    || (_astArgList[i].eType == orxCOMMAND_VAR_TYPE_NUMERIC))
    {
      /* Gets vector operand */
      if(orxString_ToVector(_astArgList[i].zValue, &(_astOperandList[i].vValue), orxNULL) != orxSTATUS_FAILURE)
      {
        /* Updates its type */
        _astOperandList[i].eType = orxCOMMAND_VAR_TYPE_VECTOR;
      }
      else
      {
        /* Hexadecimal, binary or octal?? */
        if((_astArgList[i].zValue[0] == '0')
        && (((_astArgList[i].zValue[1] | 0x20) == 'x')
         || ((_astArgList[i].zValue[1] | 0x20) == 'b')
         || ((_astArgList[i].zValue[1] >= '0')
          && (_astArgList[i].zValue[1] <= '9'))))
        {
          /* Gets U64 operand */
          if(orxString_ToU64(_astArgList[i].zValue, &(_astOperandList[i].u64Value), orxNULL) != orxSTATUS_FAILURE)
          {
            /* Gets its float value */
            _astOperandList[i].fValue = orxU2F(_astOperandList[i].u64Value);

            /* Updates its type */
            _astOperandList[i].eType = orxCOMMAND_VAR_TYPE_FLOAT;
          }
          else
          {
            /* Updates result */
            eResult = orxSTATUS_FAILURE;

            break;
          }
        }
        else
        {
          /* Gets float operand */
          if(orxString_ToFloat(_astArgList[i].zValue, &(_astOperandList[i].fValue), orxNULL) != orxSTATUS_FAILURE)
          {
            /* Updates its type */
            _astOperandList[i].eType = orxCOMMAND_VAR_TYPE_FLOAT;
          }
          else
          {
            /* Updates result */
            eResult = orxSTATUS_FAILURE;

            break;
          }
        }
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Prints a variable to a buffer, according to its type (and ignoring any block/special character)
* @param[out]  _zDstString    Destination string
* @param[in]   _u32Size       Available string size, including terminating null character
* @param[in]   _pstVar        Variable to print
* @return Number of written characters, excluding terminating null character
*/
orxU32 orxFASTCALL orxCommand_PrintVar(orxSTRING _zDstString, orxU32 _u32Size, const orxCOMMAND_VAR *_pstVar)
{
  orxU32 u32Result;

  /* Depending on type */
  switch(_pstVar->eType)
  {
    case orxCOMMAND_VAR_TYPE_STRING:
    case orxCOMMAND_VAR_TYPE_NUMERIC:
    {
      /* Updates pointer */
      u32Result = orxString_NPrint(_zDstString, _u32Size, "%s", _pstVar->zValue);

      break;
    }

    case orxCOMMAND_VAR_TYPE_FLOAT:
    {
      /* Stores it */
      u32Result = orxString_NPrint(_zDstString, _u32Size, "%g", _pstVar->fValue);

      break;
    }

    case orxCOMMAND_VAR_TYPE_S32:
    {
      /* Stores it */
      u32Result = orxString_NPrint(_zDstString, _u32Size, "%d", _pstVar->s32Value);

      break;
    }

    case orxCOMMAND_VAR_TYPE_U32:
    {
      /* Stores it */
      u32Result = orxString_NPrint(_zDstString, _u32Size, "%u", _pstVar->u32Value);

      break;
    }

    case orxCOMMAND_VAR_TYPE_S64:
    {
      /* Stores it */
      u32Result = orxString_NPrint(_zDstString, _u32Size, "%lld", _pstVar->s64Value);

      break;
    }

    case orxCOMMAND_VAR_TYPE_U64:
    {
      /* Stores it */
      u32Result = orxString_NPrint(_zDstString, _u32Size, "0x%016llX", _pstVar->u64Value);

      break;
    }

    case orxCOMMAND_VAR_TYPE_BOOL:
    {
      /* Stores it */
      u32Result = orxString_NPrint(_zDstString, _u32Size, "%s", (_pstVar->bValue == orxFALSE) ? orxSTRING_FALSE : orxSTRING_TRUE);

      break;
    }

    case orxCOMMAND_VAR_TYPE_VECTOR:
    {
      /* Gets literal value */
      u32Result = orxString_NPrint(_zDstString, _u32Size, "%c%g%c %g%c %g%c", orxSTRING_KC_VECTOR_START, _pstVar->vValue.fX, orxSTRING_KC_VECTOR_SEPARATOR, _pstVar->vValue.fY, orxSTRING_KC_VECTOR_SEPARATOR, _pstVar->vValue.fZ, orxSTRING_KC_VECTOR_END);

      break;
    }

    case orxCOMMAND_VAR_TYPE_NONE:
    {
      /* Ends string */
      _zDstString[0]  = orxCHAR_NULL;
      u32Result       = 0;

      break;
    }

    default:
    {
      /* Updates result */
      u32Result = 0;

      break;
    }
  }

  /* Done! */
  return u32Result;
}

#ifdef __orxMSVC__

  #pragma warning(default : 4200)

#endif /* __orxMSVC__ */
