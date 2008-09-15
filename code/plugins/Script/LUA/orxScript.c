/**
 * @file orxScript.cpp
 * 
 * LUA Script plugin
 * 
 */
 
 /***************************************************************************
 orxScript.cpp
 LUA Script plugin
 
 begin                : 12/04/2008
 author               : (C) Arcallians
 email                : bestel@arcallians.org
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

#include "plugin/orxPluginUser.h"
#include "script/orxScript.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>


/** Module flags
 */
#define orxSCRIPT_KU32_STATIC_FLAG_NONE        0x00000000 /**< No flags */

#define orxSCRIPT_KU32_STATIC_FLAG_READY       0x00000001 /**< Ready flag */

#define orxSCRIPT_KU32_STATIC_MASK_ALL         0xFFFFFFFF /**< All mask */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxSCRIPT_STATIC_t
{
  orxU32            u32Flags;                   /**< Control flags */
  lua_State        *pstGlobalState;             /**< Gloabal and main LUA state */
 
} orxSCRIPT_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
orxSTATIC orxSCRIPT_STATIC sstScript;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Gets type from a state
 * @param _pState   (IN) Lua state
 * @param _iIndex   (IN) State index
 * @return Returns the type associated to the index in the state.
 */
orxSCRIPT_TYPE orxScript_LUA_GetTypeFromState(lua_State *_pState, int _iIndex)
{
  /* Default return value */
  orxSCRIPT_TYPE eRet = orxSCRIPT_TYPE_NONE;
  
  /* Checks inputs */
  orxASSERT(_pState != orxNULL);
  
  /* Gets type */
  switch(lua_type(_pState, _iIndex))
  {
    case LUA_TNIL:
      eRet = orxSCRIPT_TYPE_NULL;
      break;
      
    case LUA_TINTEGER:
      eRet = orxSCRIPT_TYPE_S32;
      break;
      
    case LUA_TNUMBER:
    case LUA_TBOOLEAN:
      eRet = orxSCRIPT_TYPE_FLOAT;
      break;
      
    case LUA_TSTRING:
      eRet = orxSCRIPT_TYPE_STRING;
      break;
      
    case LUA_TTABLE:
    case LUA_TFUNCTION:
    case LUA_TUSERDATA:
    case LUA_TTHREAD:
    case LUA_TLIGHTUSERDATA:
      eRet = orxSCRIPT_TYPE_NONE;
      break;
  }
  
  /* Returns type */
  return eRet;
}

/** Private function called by LUA when a script function is executed
 * @param _pState   (IN) LUA state
 * @return Returns ???
 */
orxSTATIC int orxScript_LUA_Execute(lua_State *_pState)
{
  /* Default return value */
  int iRet = 0;
  
  /* Checks parameters */
  orxASSERT(_pState != orxNULL);
  
  /* Gets the function index */
  s32Index = lua_tointeger(_pState, 1);
  
  /* Gets the function info */
  orxSCRIPT_FUNCTION *pstFunctionInfo = orxScript_GetFunctionInfo(s32Index);
  
  /* Checks */
  orxASSERT(pstFunctionInfo != orxNULL);
  if(pstFunctionInfo != orxNULL)
  {
    /* Checks the number of parameters */
    if(pstFunctionInfo->s32NbParams != (lua_gettop(_pState) - 1))
    {
      lua_pushstring(_pState, "invalid argument count");
      lua_error(_pState);
    }
    else
    {
      /* Creates parameters */
      orxSCRIPT_PARAM aeInputs[SCRIPT_MAX_PARAMS];
      orxSCRIPT_PARAM aeOutputs[1];

      /* Creates inputs parameters */
      if(pstFunctionInfo->s32NbParams > 1)
      {
        /* Loop on stack */
        for(int i = 2; i <= pstFunctionInfo->s32NbParams; i++)
        {
          /* Gets requested type */
          orxSCRIPT_TYPE eType = pstFunctionInfo->aeParamsType[i - 1];
          
          /* Checks type */
          orxASSERT(orxScript_LUA_GetTypeFromState(_pState, i) == pstFunctionInfo->aeParamsType[i - 1]);

          /* Store type */
          aeInputs[i - 2].eType = eType;
          
          /* Switch on type */
          switch(eType)
          {
            case orxSCRIPT_TYPE_NONE:
              break;

            case orxSCRIPT_TYPE_NULL:
              break;

            case orxSCRIPT_TYPE_S32:
              *(aeInputs[i - 2].ps32Value) = lua_tointeger(_pState, i);
              break;

            case orxSCRIPT_TYPE_FLOAT:
              *(aeInputs[i - 2].pfValue) = lua_tonumber(_pState, i);
              break;

            case orxSCRIPT_TYPE_STRING:
              *(aeInputs[i - 2].pzValue) = lua_tostring(_pState, i);
              break;
          }
        }
      }
      
      /* Calls the generic function system to sends arguments in the system */
      orxSTATUS eStatus = orxScript_ExecuteFunction(pstFunctionInfo, aeInputs, aeOutputs);
      
      /* Valid return type ? */
      if((eStatus == orxSTATUS_SUCCESS)
      && (pstFunctionInfo->aeParamsType[0] != orxSCRIPT_VOID))
      {
        /* Returns 1 result */
        iRet = 1;

        /* push the result on the stack */
        /* Switch on type */
        switch(eType)
        {
          case orxSCRIPT_TYPE_S32:
            lua_pushinteger(_pState, *(aeOutputs[0].ps32Value));
            break;

          case orxSCRIPT_TYPE_FLOAT:
            lua_pushnumber(_pState, *(aeOutputs[0].pfValue));
            break;

          case orxSCRIPT_TYPE_STRING:
            lua_pushstring(_pState, *(aeOutputs[0].pzValue));
            break;
            
          default:
            /* Unknown type - checks code */
            orxASSERT(orxFALSE);
            iRet = 0;
            break;
        }
      }
    }
  }
  
  /* returns the number of results */
  return iRet;
}

/** Initialize the Script Module
 * @return Returns the status of the operation
 */
orxSTATUS orxScript_LUA_Init()
{
  orxSTATUS eResult;

  /* Not already Initialized? */
  if(!(sstScript.u32Flags & orxSCRIPT_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstScript, sizeof(orxSCRIPT_STATIC));
    
    /* Creates a new LUA state */
    sstScript.pstGlobalState  = lua_open();
    luaL_openlibs(sstScript.pstGlobalState);
  }
  else
  {
    /* !!! MSG !!! */

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Initialized? */
  if(eResult == orxSTATUS_SUCCESS)
  {
    /* Inits Flags */
    sstScript.u32Flags = orxSCRIPT_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return eResult;
}

/** Uninitialize the Script Module
 */
orxVOID orxScript_LUA_Exit()
{
  /* Was initialized? */
  if(sstScript.u32Flags & orxSCRIPT_KU32_STATIC_FLAG_READY)
  {
    /* Close LUA global state */
    lua_close(sstScript.pstGlobalState);

    /* Cleans static controller */
    orxMemory_Zero(&sstScript, sizeof(orxPHYSICS_STATIC));
  }

  return;
}

/** Parse and run a script file
 * @param _zFileName  (IN)  File name
 * @return Returns Success if valid parsing/execution, else returns orxSTATUS_FAILURE
 */
orxSTATUS orxScript_LUA_RunFile(orxCONST orxSTRING _zFileName)
{
  /* Default return value */
  orxSTATUS eResult = orxSTATUS_FAILURE;
  
  /* Checks inputs */
  orxASSERT(_zFileName != orxNULL);
  
  /* Loads and run files */
  luaL_dofile(sstScript.pstGlobalState, _zFileName);
  
  /* Success */
  eResult = orxSTATUS_SUCCESS;
  
  /* Returns status */
  return eResult;
}

/** Parse and run a script string
 * @param _zScript    (IN)  script to parse
 * @return Returns Success if valid parsing/execution, else returns orxSTATUS_FAILURE
 */
orxSTATUS orxScript_LUA_RunString(orxCONST orxSTRING _zScript)
{
  /* Default return value */
  orxSTATUS eResult = orxSTATUS_FAILURE;
  
  /* Checks inputs */
  orxASSERT(_zFileName != orxNULL);
  
  /* Loads and run files */
  luaL_dostring(sstScript.pstGlobalState, _zFileName);
  
  /* Success */
  eResult = orxSTATUS_SUCCESS;
  
  /* Returns status */
  return eResult;
}


/** Gets a global variable type
 * @param _zVar       (IN) Variable name
 * @return Returns the type of the given variable. Returns orxScript_LUA_TYPE_NONE if variable not found.
 */
orxSCRIPT_TYPE orxScript_LUA_GetType(orxCONST orxSTRING _zVar)
{
  /* Default return value */
  orxSCRIPT_TYPE eRet = orxSCRIPT_TYPE_NONE;
  
  /* Checks inputs */
  orxASSERT(_zVar != orxNULL);
  
  /* Gets global value */
  lua_getglobal(sstScript.pstGlobalState, _zVar);
  
  /* Returns the type of the variable */
  eRet = orxScript_LUA_GetTypeFromState(sstScript.pstGlobalState, 1);
  
  /* Returns type */
  return eRet;
}

/** Gets a global script variable value
 * @param _zVar       (IN)  Variable name
 * @param _bOutValue  (OUT) Signed 32 bits returns value
 * @return Returns orxSTATUS_SUCCESS is variable exists and is of the right type, else returns orxSTATUS_FAILURE
 */
orxSTATUS orxScript_LUA_GetS32Value(orxCONST orxSTRING _zVar, orxS32 *_s32OutValue)
{
  /* Default return value */
  orxSTATUS eRet = orxSTATUS_FAILURE;
  
  /* Checks inputs */
  orxASSERT(_zVar != orxNULL);
  
  /* Gets the global value */
  lua_getglobal(sstScript.pstGlobalState, _zVar);
  
  /* Gets the returns integer */
  *_s32OutValue = lua_tointeger(sstScript.pstGlobalState, 0);
  
  /* Valid */
  eRet = orxSTATUS_SUCCESS;
  
  /* Returns type */
  return eRet;
}

/** Gets a global script variable value
 * @param _zVar       (IN)  Variable name
 * @param _fOutValue  (OUT) Float returns value
 * @return Returns orxSTATUS_SUCCESS is variable exists and is of the right type, else returns orxSTATUS_FAILURE
 */
orxSTATUS orxScript_LUA_GetFloatValue(orxCONST orxSTRING _zVar, orxFLOAT *_fOutValue)
{
  /* Default return value */
  orxSTATUS eRet = orxSTATUS_FAILURE;
  
  /* Checks inputs */
  orxASSERT(_zVar != orxNULL);
  
  /* Gets the global value */
  lua_getglobal(sstScript.pstGlobalState, _zVar);
  
  /* Gets the returns integer */
  *_fOutValue = lua_tonumber(sstScript.pstGlobalState, 0);
  
  /* Valid */
  eRet = orxSTATUS_SUCCESS;
  
  /* Returns type */
  return eRet;
}

/** Gets a global script variable value
 * @param _zVar       (IN)  Variable name
 * @param _zOutValue  (OUT) String returns value
 * @return Returns orxSTATUS_SUCCESS is variable exists and is of the right type, else returns orxSTATUS_FAILURE
 */
orxSTATUS orxScript_LUA_GetStringValue(orxCONST orxSTRING _zVar, orxSTRING *_zOutValue)
{
  /* Default return value */
  orxSTATUS eRet = orxSTATUS_FAILURE;
  
  /* Checks inputs */
  orxASSERT(_zVar != orxNULL);
  
  /* Gets the global value */
  lua_getglobal(sstScript.pstGlobalState, _zVar);
  
  /* Gets the returns integer */
  *_zOutValue = lua_tostring(sstScript.pstGlobalState, 0);
  
  /* Valid */
  eRet = orxSTATUS_SUCCESS;
  
  /* Returns type */
  return eRet;
}

/** Sets a global script variable value
 * @param _zVar       (IN) Variable name
 * @param _s32Value   (IN) Signed 32 bits value
 * @return Returns orxSTATUS_SUCCESS is variable exists and is of the right type, else returns orxSTATUS_FAILURE
 */
orxSTATUS orxScript_LUA_SetS32Value(orxCONST orxSTRING _zVar, orxS32 _s32Value)
{
  /* Default return value */
  orxSTATUS eRet = orxSTATUS_FAILURE;
  
  /* Checks inputs */
  orxASSERT(_zVar != orxNULL);

  /* Push value on the stack */
  lua_pushinteger(sstScript.pstGlobalState, _s32Value);
  
  /* Gets the global value */
  lua_setglobal(sstScript.pstGlobalState, _zVar);
  
  /* Valid */
  eRet = orxSTATUS_SUCCESS;
  
  /* Returns type */
  return eRet;
}

/** Sets a global script variable value
 * @param _zVar       (IN) Variable name
 * @param _fValue     (IN) Float value
 * @return Returns orxSTATUS_SUCCESS is variable exists and is of the right type, else returns orxSTATUS_FAILURE
 */
orxSTATUS orxScript_LUA_SetFloatValue(orxCONST orxSTRING _zVar, orxFLOAT _fValue)
{
  /* Default return value */
  orxSTATUS eRet = orxSTATUS_FAILURE;
  
  /* Checks inputs */
  orxASSERT(_zVar != orxNULL);

  /* Push value on the stack */
  lua_pushnumber(sstScript.pstGlobalState, _fValue);
  
  /* Gets the global value */
  lua_setglobal(sstScript.pstGlobalState, _zVar);
  
  /* Valid */
  eRet = orxSTATUS_SUCCESS;
  
  /* Returns type */
  return eRet;
}

/** Sets a global script variable value
 * @param _zVar       (IN) Variable name
 * @param _zValue     (IN) String value
 * @return Returns orxSTATUS_SUCCESS is variable exists and is of the right type, else returns orxSTATUS_FAILURE
 */
orxSTATUS orxScript_LUA_SetStringValue(orxCONST orxSTRING _zVar, orxSTRING _zValue)
{
  /* Default return value */
  orxSTATUS eRet = orxSTATUS_FAILURE;
  
  /* Checks inputs */
  orxASSERT(_zVar != orxNULL);

  /* Push value on the stack */
  lua_pushstring(sstScript.pstGlobalState, _zValue);
  
  /* Gets the global value */
  lua_setglobal(sstScript.pstGlobalState, _zVar);
  
  /* Valid */
  eRet = orxSTATUS_SUCCESS;
  
  /* Returns type */
  return eRet;
}

/** Sets a global script variable value
 * @param _s32Index   (IN) Index of the function data from the global list
 * @return Returns orxSTATUS_SUCCESS if valid registration, else orxSTATUS_FAILURE
 */
orxSTATUS orxScript_LUA_RegisterFunction(orxS32 _s32Index)
{
  /* Default return value */
  orxSTATUS eStatus = orxSTATUS_FAILURE;
  
  /* Checks inputs */
  orxASSERT(_s32Index >= 0);
  
  /* Gets function info */
  orxSCRIPT_FUNCTION *pstFunctionInfo = orxScript_GetFunctionInfo(_s32Index);
  
  /* Valid info ? */
  if(pstFunctionInfo != orxNULL)
  {
    /* Register function */
    lua_pushinteger(sstScript.pstGlobalState, _s32Index);
    lua_pushcclosure(sstScript.pstGlobalState, pstFunctionInfo->pfnFunction, 1);
    lua_setglobal(sstScript.pstGlobalState, pstFunctionInfo->zFunctionName);

    /* Success */
    eStatus = orxSTATUS_SUCCESS;
  }
  
  /* Returns status */
  return eStatus;
}

/********************
 *  Plugin Related  *
 ********************/

orxPLUGIN_USER_CORE_FUNCTION_START(SCRIPT);

orxPLUGIN_USER_CORE_FUNCTION_ADD(orxScript_LUA_Init, SCRIPT, INIT)
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxScript_LUA_Exit, SCRIPT, EXIT)
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxScript_LUA_RunFile, SCRIPT, RUN_FILE)
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxScript_LUA_RunString, SCRIPT, RUN_STRING)
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxScript_LUA_GetType, SCRIPT, GET_TYPE)
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxScript_LUA_GetS32Value, SCRIPT, GET_S32_VALUE)
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxScript_LUA_GetFloatValue, SCRIPT, GET_FLOAT_VALUE)
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxScript_LUA_GetStringValue, SCRIPT, GET_STRING_VALUE)
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxScript_LUA_SetS32Value, SCRIPT, SET_S32_VALUE)
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxScript_LUA_SetFloatValue, SCRIPT, SET_FLOAT_VALUE)
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxScript_LUA_SetStringValue, SCRIPT, SET_STRING_VALUE)
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxScript_LUA_RegisterFunction, SCRIPT, REGISTER_FUNCTION)

orxPLUGIN_USER_CORE_FUNCTION_END();
