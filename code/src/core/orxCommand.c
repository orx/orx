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
#include "utils/orxHashTable.h"
#include "utils/orxString.h"


/** Module flags
 */
#define orxCOMMAND_KU32_STATIC_FLAG_NONE              0x00000000  /**< No flags */

#define orxCOMMAND_KU32_STATIC_FLAG_READY             0x00000001  /**< Ready flag */

#define orxCOMMAND_KU32_STATIC_MASK_ALL               0xFFFFFFFF  /**< All mask */


/** Misc
 */
#define orxCOMMAND_KZ_CONFIG_SECTION                  "-=orxCommand=-"
#define orxCOMMAND_KZ_CONFIG_STACK_PREFIX             "Stack"

#define orxCOMMAND_KU32_TABLE_SIZE                    256
#define orxCOMMAND_KU32_BANK_SIZE                     128


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Command structure
 */
typedef struct __orxCOMMAND_t
{
  orxCOMMAND_VAR_DEF  stResult;                       /**< Result definition : 8 */
  orxSTRING           zName;                          /**< Name : 12 */
  orxSTRING           zPrototype;                     /**< Prototype : 16 */
  orxU32              u32ParamNumber;                 /**< Param number : 20 */
  orxCOMMAND_VAR_DEF *astParamList;                   /**< Param list : 24 */

} orxCOMMAND;

/** Static structure
 */
typedef struct __orxCOMMAND_STATIC_t
{
  orxBANK          *pstBank;                          /**< Command bank */
  orxHASHTABLE     *pstTable;                         /**< Command table */
  orxU32            u32Flags;                         /**< Control flags */

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


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Command module setup
 */
void orxFASTCALL orxCommand_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_CLOCK, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_CLOCK, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_CLOCK, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_CLOCK, orxMODULE_ID_EVENT);

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
      }
      else
      {
        /* Deletes bank */
        orxBank_Delete(sstCommand.pstBank);

        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Failed to create command table.");
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Failed to create command bank.");
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_CLOCK, "Tried to initialize command module when it was already initialized.");

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
    /* Deletes table */
    orxHashTable_Delete(sstCommand.pstTable);

    /* Deletes bank */
    orxBank_Delete(sstCommand.pstBank);

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
orxSTATUS orxFASTCALL orxCommand_Register(const orxSTRING _zCommand, const orxCOMMAND_FUNCTION *_pfnFunction, orxU32 _u32ParamNumber, const orxCOMMAND_VAR_DEF *_astParamList, const orxCOMMAND_VAR_DEF *_pstResult)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstCommand.u32Flags, orxCOMMAND_KU32_STATIC_FLAG_READY));
  orxASSERT(_zCommand != orxNULL);

  /* Valid? */
  if((_zCommand != orxNULL) && (_zCommand != orxSTRING_EMPTY))
  {
    orxU32 u32ID;

    /* Gets its ID */
    u32ID = orxString_ToCRC(_zCommand);

    /* Not already registered? */
    if(orxHashTable_Get(sstCommand.pstTable, u32ID) == orxNULL)
    {
      //! TODO

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Can't register command: [%s] is already registered with this prototype: %s", _zCommand, orxCommand_GetPrototype(_zCommand));
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
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstCommand.u32Flags, orxCOMMAND_KU32_STATIC_FLAG_READY));
  orxASSERT(_zCommand != orxNULL);

  /* Valid? */
  if((_zCommand != orxNULL) & (_zCommand != orxSTRING_EMPTY))
  {
    orxU32 u32ID;

    /* Gets its ID */
    u32ID = orxString_ToCRC(_zCommand);

    /* Removes it? */
    eResult = orxHashTable_Remove(sstCommand.pstTable, u32ID);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
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

/** Gets command prototype
* @param[in]   _zCommand      Command name
* @return      Literal prototype / orxNULL
*/
const orxSTRING orxFASTCALL orxCommand_GetPrototype(const orxSTRING _zCommand)
{
  const orxSTRING zResult = orxNULL;

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

    /* Gets it */
    pstCommand = (orxCOMMAND *)orxHashTable_Get(sstCommand.pstTable, u32ID);

    /* Found? */
    if(pstCommand != orxNULL)
    {
      /* Updates result */
      zResult = pstCommand->zPrototype;
    }
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
  orxASSERT(_zCommandLine != orxNULL);
  orxASSERT(_pstResult != orxNULL);

  /* Valid? */
  if((_zCommandLine != orxNULL) & (_zCommandLine != orxSTRING_EMPTY))
  {
    orxU32      u32ID;
    orxCOMMAND *pstCommand;

    /* Gets its ID */
    u32ID = orxString_ToCRC(_zCommandLine);

    /* Gets it */
    pstCommand = (orxCOMMAND *)orxHashTable_Get(sstCommand.pstTable, u32ID);

    /* Found? */
    if(pstCommand != orxNULL)
    {
      orxCOMMAND_VAR *astArgList;

      //! TODO

      /* Runs it */
//      pstResult = orxCommand_Run(pstCommand, astArgList, _pstResult);
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
  orxCOMMAND_VAR *pstResult;

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
      orxCOMMAND_VAR *astArgList;

      //! TODO

      /* Runs it */
//      pstResult = orxCommand_Run(pstCommand, astArgList, _pstResult);
    }
  }

  /* Done! */
  return pstResult;
}


/** Gets next command using the base name + last found command (handy for autocompletion and for listing all the commands)
* @param[in]   _zCommand      orxNULL to find the first command, last found command or beginning of a command name otherwise
* @return      Next command name if found, orxNULL otherwise
*/
const orxSTRING orxFASTCALL          orxCommand_GetNext(const orxSTRING _zCommand);
