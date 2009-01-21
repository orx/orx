/* Orx - Portable Game Engine
 *
 * Orx is the legal property of its developers, whose names
 * are listed in the COPYRIGHT file distributed 
 * with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * @file orxParam.c
 * @date 09/09/2005
 * @author bestel@arcallians.org
 *
 * @todo
 */


#include "main/orxParam.h"
#include "memory/orxMemory.h"
#include "memory/orxBank.h"
#include "core/orxEvent.h"
#include "core/orxConfig.h"
#include "core/orxSystem.h"
#include "utils/orxString.h"
#include "utils/orxHashTable.h"
#include "debug/orxDebug.h"


#define orxPARAM_KU32_MODULE_FLAG_NONE    0x00000000  /**< No flags have been set */
#define orxPARAM_KU32_MODULE_FLAG_READY   0x00000001  /**< The module has been initialized */

#define orxPARAM_KU32_FLAG_PROCESSED      0x10000000 /**< Param has already been processed */

#define orxPARAM_KU32_MODULE_BANK_SIZE    32          /**< Average max number of parameter that can be registered */
#define orxPARAM_KU32_MAX_CONFIG_PARAM    32          /**< Maximum of value for one parameter in config file */
#define orxPARAM_KZ_MODULE_SHORT_PREFIX   "-"         /**< Prefix for short parameters */
#define orxPARAM_KZ_MODULE_LONG_PREFIX    "--"        /**< Prefix for long parameters */

#define orxPARAM_KZ_CONFIG_SECTION        "Param"     /**< Param config section name */


#ifdef __orxGCC__

  #define orxPARAM_LOG(STRING, ...)                                                                         \
  do                                                                                                        \
  {                                                                                                         \
    _orxDebug_BackupFlags();                                                                                \
    _orxDebug_SetFlags(orxDEBUG_KU32_STATIC_FLAG_CONSOLE,                                                   \
                       orxDEBUG_KU32_STATIC_MASK_USER_ALL);                                                 \
    _orxDebug_Log(orxDEBUG_LEVEL_PARAM, (orxCONST orxSTRING)__FUNCTION__, __FILE__, __LINE__, STRING, ##__VA_ARGS__); \
    _orxDebug_RestoreFlags();                                                                               \
  } while(orxFALSE)

#else /* __orxGCC__ */
  #ifdef __orxMSVC__

    #define orxPARAM_LOG(STRING, ...)                                                                       \
    do                                                                                                      \
    {                                                                                                       \
      _orxDebug_BackupFlags();                                                                              \
      _orxDebug_SetFlags(orxDEBUG_KU32_STATIC_FLAG_CONSOLE,                                                 \
                         orxDEBUG_KU32_STATIC_MASK_USER_ALL);                                               \
      _orxDebug_Log(orxDEBUG_LEVEL_PARAM, (orxCONST orxSTRING)__FUNCTION__, __FILE__, __LINE__, STRING, __VA_ARGS__); \
      _orxDebug_RestoreFlags();                                                                             \
    } while(orxFALSE)

  #endif /* __orxMSVC__ */
#endif /* __orcGCC__ */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/
 
typedef struct __orxPARAM_INFO_t
{
  orxPARAM  stParam;              /* Param values */
  orxU32    u32Count;             /* Number of time that this param has been met in the command line */
} orxPARAM_INFO;

typedef struct __orxPARAM_STATIC_t
{
  orxBANK      *pstBank;          /* Bank of registered parameters */
  orxHASHTABLE *pstHashTable;     /* HashTable of registered Parameters */

  orxU32        u32ParamNumber;   /* Param counter */
  orxSTRING    *azParams;         /* Params */

  orxU32        u32Flags;         /* Module flags */

} orxPARAM_STATIC;


/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/

orxSTATIC orxPARAM_STATIC sstParam;

/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Return the parameter info associated to the given value.
 * @param[in] _zParamName Name of the parameter (with short or long prefix inside)
 * @return Returns the pointer on the param info if found, else returns orxNULL
 */
orxPARAM_INFO *orxParam_Get(orxU32 _u32ParamName)
{
  orxPARAM_INFO *pstParamInfo; /* Parameters info extracted from the Hash Table */

  /* Module initialized ? */
  orxASSERT((sstParam.u32Flags & orxPARAM_KU32_MODULE_FLAG_READY) == orxPARAM_KU32_MODULE_FLAG_READY);

  /* Get the parameter pointer */
  pstParamInfo = (orxPARAM_INFO *)orxHashTable_Get(sstParam.pstHashTable, _u32ParamName);

  /* Returns it */
  return pstParamInfo;
}

/** Help callback (for -h or --help)
 * @param[in] _u32NbParam Number of extra parameters read for this option
 * @param[in] _azParams   Array of extra parameters (the first one is always the option name)
 * @return Returns orxSTATUS_SUCCESS if informations read are correct, orxSTATUS_FAILURE if a problem has occured
 */
orxSTATUS orxFASTCALL orxParam_Help(orxU32 _u32NbParam, orxCONST orxSTRING _azParams[])
{
  orxASSERT((sstParam.u32Flags & orxPARAM_KU32_MODULE_FLAG_READY) == orxPARAM_KU32_MODULE_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_u32NbParam > 0);

  orxPARAM_LOG("Options:");

  /* Extra parameters ? */
  if(_u32NbParam == 1)
  {
    orxPARAM_INFO *pstParamInfo = orxNULL;

    /* No => display the full list of registered option with short description */
    while((pstParamInfo = orxBank_GetNext(sstParam.pstBank, pstParamInfo)) != orxNULL)
    {
      orxPARAM_LOG("%s%s %s%s\t\t%s",
             orxPARAM_KZ_MODULE_SHORT_PREFIX,
             pstParamInfo->stParam.zShortName,
             orxPARAM_KZ_MODULE_LONG_PREFIX,
             pstParamInfo->stParam.zLongName,
             pstParamInfo->stParam.zShortDesc);
    }
  }
  else
  {
    orxU32 u32Index;          /* Index to traverse extra parameters */
    orxU32 u32LongPrefixCRC;  /* CRC for the long prefix string */

    /* Create the CRC value of the prefix */
    u32LongPrefixCRC = orxString_ToCRC(orxPARAM_KZ_MODULE_LONG_PREFIX);

    /* Display the long description of the extra parameters only */
    for(u32Index = 1; u32Index < _u32NbParam; u32Index++)
    {
      orxU32 u32Name;               /* CRC Name of the long option */
      orxPARAM_INFO *pstParamInfo;  /* Stored parameter value */

      /* Create the full CRC Value */
      u32Name = orxString_ContinueCRC((orxCONST orxSTRING)_azParams[u32Index], u32LongPrefixCRC);

      /* Get the parameter info */
      pstParamInfo = (orxPARAM_INFO *)orxParam_Get(u32Name);

      /* Valid info ? */
      if(pstParamInfo != orxNULL)
      {
        /* Display its help */
        orxPARAM_LOG("%s%s %s%s\t\t%s",
               orxPARAM_KZ_MODULE_SHORT_PREFIX,
               pstParamInfo->stParam.zShortName,
               orxPARAM_KZ_MODULE_LONG_PREFIX,
               pstParamInfo->stParam.zLongName,
               pstParamInfo->stParam.zLongDesc);
      }
      else
      {
        orxPARAM_LOG("<%s>\t\tUnknown command line parameter", _azParams[u32Index]);
      }
    }
  }

  /* Help request always fail => Show help instead of starting the engine */
  return orxSTATUS_FAILURE;
}

/** Process registered params
 * @return Returns the process status
 */
orxSTATIC orxSTATUS orxFASTCALL orxParam_Process(orxPARAM_INFO *_pstParamInfo)
{
  orxU32      i, u32RemainingNumber;
  orxBOOL     bUseConfig = orxFALSE;
  orxSTRING  *azParamList = orxNULL;
  orxSTATUS   eResult = orxSTATUS_SUCCESS;
  orxSTRING   azConfigParamList[orxPARAM_KU32_MAX_CONFIG_PARAM];

  /* Module initialized ? */
  orxASSERT((sstParam.u32Flags & orxPARAM_KU32_MODULE_FLAG_READY) == orxPARAM_KU32_MODULE_FLAG_READY);
  orxASSERT(_pstParamInfo != orxNULL);

  /* Not already processed? */
  if(!(_pstParamInfo->stParam.u32Flags & orxPARAM_KU32_FLAG_PROCESSED))
  {
    /* Look at the number of time that this parameter as been set on the command line, and if multiple instance are allowed */
    if(((_pstParamInfo->u32Count == 0)
    || ((_pstParamInfo->u32Count > 0)
     && (orxFLAG_TEST(_pstParamInfo->stParam.u32Flags, orxPARAM_KU32_FLAG_MULTIPLE_ALLOWED)))))
    {
      orxU32 u32ParamCounter;

      /* Loop on Extra parameters */
      for(i = 0; i < sstParam.u32ParamNumber; i++)
      {
        /* Short prefix found? */
        if(((orxString_SearchString(sstParam.azParams[i], orxPARAM_KZ_MODULE_SHORT_PREFIX) == sstParam.azParams[i])
         && (orxString_Compare(sstParam.azParams[i] + orxString_GetLength(orxPARAM_KZ_MODULE_SHORT_PREFIX), _pstParamInfo->stParam.zShortName) == 0))
        || ((orxString_SearchString(sstParam.azParams[i], orxPARAM_KZ_MODULE_LONG_PREFIX) == sstParam.azParams[i])
         && (orxString_Compare(sstParam.azParams[i] + orxString_GetLength(orxPARAM_KZ_MODULE_LONG_PREFIX), _pstParamInfo->stParam.zLongName) == 0)))
        {
          /* Gets start of list */
          azParamList         = &sstParam.azParams[i];
          u32RemainingNumber  = sstParam.u32ParamNumber - i;

          break;
        }
      }

      /* Not found? */
      if(azParamList == orxNULL)
      {
        orxSTRING zParamValue;

        /* Selects config section */
        orxConfig_SelectSection(orxPARAM_KZ_CONFIG_SECTION);

        /* Gets its value */
        zParamValue = orxConfig_GetString(_pstParamInfo->stParam.zLongName);

        /* Found? */
        if(zParamValue != orxSTRING_EMPTY)
        {
          orxU32    u32Length;
          orxSTRING zWorkString;

          /* Gets string length */
          u32Length = orxString_GetLength(orxPARAM_KZ_MODULE_LONG_PREFIX) + orxString_GetLength(_pstParamInfo->stParam.zLongName) + 1 + orxString_GetLength(zParamValue) + 1;

          /* Allocates it */
          zWorkString = (orxSTRING)orxMemory_Allocate(u32Length * sizeof(orxCHAR), orxMEMORY_TYPE_TEXT);

          /* Valid? */
          if(zWorkString != orxNULL)
          {
            orxU32    u32Counter;
            orxCHAR  *pcCurrent, *pcLast;

            /* Copies text to work on */
            orxString_Print(zWorkString, "%s%s %s", orxPARAM_KZ_MODULE_LONG_PREFIX, _pstParamInfo->stParam.zLongName, zParamValue);

            /* For all characters */
            for(u32Counter = 0, pcCurrent = zWorkString, pcLast = zWorkString;
                (pcCurrent < zWorkString + u32Length) && (u32Counter < orxPARAM_KU32_MAX_CONFIG_PARAM - 1);
                pcCurrent++)
            {
              orxCHAR *pc;

              /* Skips all separator? */
              for(pc = pcCurrent;
                  (pc < zWorkString + u32Length) && ((*pc == ' ') || (*pc == '\t') || (*pc == orxCHAR_EOL));
                  pc++);

              /* Needs to update string? */
              if(pc > pcCurrent)
              {
                /* Cuts string */
                *pcCurrent = orxCHAR_NULL;

                /* Adds it to array */
                azConfigParamList[u32Counter] = pcLast;

                /* Updates counter */
                u32Counter++;

                /* Updates pointers */
                pcCurrent = pc - 1;
                pcLast    = pc;
              }
            }

            /* Stores last param */
            azConfigParamList[u32Counter] = pcLast;

            /* Updates param list */
            azParamList         = azConfigParamList;
            u32RemainingNumber  = u32Counter + 1;

            /* Updates status */
            bUseConfig = orxTRUE;
          }
        }
      }

      /* Found? */
      if(azParamList != orxNULL)
      {
        /* Increases ref counter */
        _pstParamInfo->u32Count++;

        /* Now, count the number of extra params */
        for(u32ParamCounter = 1;
            (u32ParamCounter  < u32RemainingNumber)
         && (orxString_SearchString(azParamList[u32ParamCounter], orxPARAM_KZ_MODULE_SHORT_PREFIX) != azParamList[u32ParamCounter])
         && (orxString_SearchString(azParamList[u32ParamCounter], orxPARAM_KZ_MODULE_LONG_PREFIX) != azParamList[u32ParamCounter]);
            u32ParamCounter++);

        /* Can process it? */
        if(_pstParamInfo->stParam.pfnParser(u32ParamCounter, azParamList) != orxSTATUS_FAILURE)
        {
          /* Updates status */
          _pstParamInfo->stParam.u32Flags |= orxPARAM_KU32_FLAG_PROCESSED;
        }
        /* Stop on error? */
        else if(orxFLAG_TEST(_pstParamInfo->stParam.u32Flags, orxPARAM_KU32_FLAG_STOP_ON_ERROR))
        {
          /* Send the Exit Event to the engine */
          orxEvent_SendSimple(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_CLOSE);

          /* Updates result */
          eResult = orxSTATUS_FAILURE;
        }

        /* Was using config? */
        if(bUseConfig != orxFALSE)
        {
          /* Frees allocated string */
          orxMemory_Free(azParamList[0]);
        }
      }
    }
  }

  /* Done */
  return eResult;
}

/** Processes command line parameters
 * @param[in] _u32ParamCount  Number of extra parameters read for this option
 * @param[in] _azParams       Array of extra parameters (the first one is always the option name)
 * @return Returns orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxParam_ProcessConfigParams(orxU32 _u32ParamCount, orxCONST orxSTRING _azParams[])
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;
  orxU32    i;

  /* For all specified plugin names */
  for(i = 1; (eResult == orxSTATUS_SUCCESS) && (i < _u32ParamCount); i++)
  {
    /* Loads config file */
    eResult = orxConfig_Load(_azParams[i]);
  }

  /* Done! */
  return eResult;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/***************************************************************************
 orxParam_Setup
 Param module setup.

 returns: nothing
 ***************************************************************************/
orxVOID orxParam_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_PARAM, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_PARAM, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_PARAM, orxMODULE_ID_HASHTABLE);
  orxModule_AddDependency(orxMODULE_ID_PARAM, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_PARAM, orxMODULE_ID_EVENT);

  return;
}


/** Initialize Param Module
 */
orxSTATUS orxParam_Init()
{
  /* Declare variables */
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstParam.u32Flags & orxPARAM_KU32_MODULE_FLAG_READY))
  {
    orxSTRING  *azParamBackup;
    orxU32      u32ParamBackup;

    /* Backups params */
    azParamBackup   = sstParam.azParams;
    u32ParamBackup  = sstParam.u32ParamNumber;

    /* Cleans static controller */
    orxMemory_Zero(&sstParam, sizeof(orxPARAM_STATIC));

    /* Restores parameters */
    sstParam.azParams       = azParamBackup;
    sstParam.u32ParamNumber = u32ParamBackup;

    /* Create an empty bank to store parameters */
    sstParam.pstBank = orxBank_Create(orxPARAM_KU32_MODULE_BANK_SIZE,
                                      sizeof(orxPARAM_INFO),
                                      orxBANK_KU32_FLAG_NONE,
                                      orxMEMORY_TYPE_MAIN);

    /* Bank successfully created ? */
    if(sstParam.pstBank != orxNULL)
    {
      /* Now create the hash table */
      sstParam.pstHashTable = orxHashTable_Create(orxPARAM_KU32_MODULE_BANK_SIZE * 2,
                                                  orxHASHTABLE_KU32_FLAG_NONE,
                                                  orxMEMORY_TYPE_MAIN);

      /* HashTable Created ? */
      if(sstParam.pstHashTable != orxNULL)
      {
        orxPARAM stParams;

        /* Set module as ready */
        sstParam.u32Flags   = orxPARAM_KU32_MODULE_FLAG_READY;

        /* Inits the param structure */
        orxMemory_Zero(&stParams, sizeof(orxPARAM));
        stParams.pfnParser  = orxParam_ProcessConfigParams;
        stParams.u32Flags   = orxPARAM_KU32_FLAG_MULTIPLE_ALLOWED;
        stParams.zShortName = "c";
        stParams.zLongName  = "config";
        stParams.zShortDesc = "Loads the specified configuration file.";
        stParams.zLongDesc  = "Loads the specified configuration file from the current execution folder. More than one file can be specified.";

        /* Registers it */
        eResult = orxParam_Register(&stParams);

        /* If registration failed, module become unready */
        if(eResult == orxSTATUS_FAILURE)
        {
          sstParam.u32Flags = orxPARAM_KU32_MODULE_FLAG_NONE;
        }
      }
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Tried to initialize param module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done */
  return eResult;
}

/** Exit Param module
 */
orxVOID orxParam_Exit()
{
  /* Module initialized ? */
  if((sstParam.u32Flags & orxPARAM_KU32_MODULE_FLAG_READY) == orxPARAM_KU32_MODULE_FLAG_READY)
  {
    /* Module not ready now */
    sstParam.u32Flags = orxPARAM_KU32_MODULE_FLAG_NONE;
  }

  return;
}

/** Register a new parameter
 * @param[in] _pstParam Informations about the option to register
 * @return Returns the status of the registration
 */
orxSTATUS orxFASTCALL orxParam_Register(orxCONST orxPARAM *_pstParam)
{
  orxPARAM_INFO *pstParamInfo;       /* Parameter stored in the bank */
  orxSTATUS eResult = orxSTATUS_FAILURE; /* Result of the operation */

  /* Module initialized ? */
  orxASSERT((sstParam.u32Flags & orxPARAM_KU32_MODULE_FLAG_READY) == orxPARAM_KU32_MODULE_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstParam != orxNULL);

  /* Short parameters and callbacks are compulsory */
  if(_pstParam->zShortName != orxNULL &&
      _pstParam->zShortDesc != orxNULL &&
      _pstParam->pfnParser  != orxNULL)
  {
    orxU32 u32ShortName;

    /* Creates CRC for the Short Name */
    u32ShortName = orxString_ToCRC(orxPARAM_KZ_MODULE_SHORT_PREFIX);
    u32ShortName = orxString_ContinueCRC((orxCONST orxSTRING)_pstParam->zShortName, u32ShortName);

    /* Check if options with the same name don't have already been registered */
    if(orxParam_Get(u32ShortName) == orxNULL)
    {
      orxU32 u32LongName = 0;
      orxBOOL bStoreParam = orxTRUE; /* No problem at the moment, we can store the parameter */

      /* Check if the long name has not already been registered too */
      if(_pstParam->zLongName != orxNULL)
      {
        /* We are not sure to store the param since it could have a problem with the long name */
        bStoreParam = orxFALSE;

        /* Create CRC For the long name */
        u32LongName = orxString_ToCRC(orxPARAM_KZ_MODULE_LONG_PREFIX);
        u32LongName = orxString_ContinueCRC((orxCONST orxSTRING)_pstParam->zLongName, u32LongName);

        /* Found ? */
        if(orxParam_Get(u32LongName) == orxNULL)
        {
          /* No Params have been found, we can store it */
          bStoreParam = orxTRUE;
        }
      }

      /* Can we store the parameter ? */
      if(bStoreParam)
      {
        /* Allocate a new cell in the bank */
        pstParamInfo = (orxPARAM_INFO *)orxBank_Allocate(sstParam.pstBank);

        /* Allocation success ? */
        if(pstParamInfo != orxNULL)
        {
          /* Cleans it */
          orxMemory_Zero(pstParamInfo, sizeof(orxPARAM_INFO));

          /* Copy input values */
          orxMemory_Copy(&(pstParamInfo->stParam), _pstParam, sizeof(orxPARAM));

          /* Store Params in the hash Table (with short name as key) */
          orxHashTable_Add(sstParam.pstHashTable, u32ShortName, pstParamInfo);

          /* Store Param with long name as key if it exists */
          if(_pstParam->zLongName != orxNULL)
          {
            /* Adds it to table */
            orxHashTable_Add(sstParam.pstHashTable, u32LongName, pstParamInfo);

            /* Process params */
            eResult = orxParam_Process(pstParamInfo);
          }
        }
      }
      else
      {
        orxDEBUG_PRINT(orxDEBUG_LEVEL_PARAM,
                     "A parameter with the same long name (%s%s) has already been registered",
                     orxPARAM_KZ_MODULE_LONG_PREFIX,
                     _pstParam->zLongName);
      }
    }
    else
    {
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PARAM,
                   "A parameter with the same short name (%s%s) has already been registered",
                   orxPARAM_KZ_MODULE_SHORT_PREFIX,
                   _pstParam->zShortName);
    }
  }
  else
  {
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PARAM, "Invalid registered parameter... Forgets it");
  }

  /* Done */
  return eResult;
}

/** Sets the command line arguments
 * @param[in] _u32NbParam Number of read parameters
 * @param[in] _azParams   List of parameters
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxParam_SetArgs(orxU32 _u32NbParams, orxSTRING _azParams[])
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Stores info */
  sstParam.u32ParamNumber = _u32NbParams;
  sstParam.azParams       = _azParams;
orxLOG("TATA");
  /* Has parameters? */
  if((_u32NbParams > 0) && (_azParams != orxNULL))
  {
    orxS32 s32Index, s32NextIndex;

    /* Finds last '.' */
    for(s32Index = orxString_SearchCharIndex(sstParam.azParams[0], '.', 0);
        (s32Index >= 0) && ((s32NextIndex = orxString_SearchCharIndex(sstParam.azParams[0], '.', s32Index + 1)) > 0);
        s32Index = s32NextIndex);

    /* Does base name have a '.'? */
    if((s32Index > 0) && (orxString_SearchCharIndex(sstParam.azParams[0], orxCHAR_DIRECTORY_SEPARATOR, s32Index + 1) < 0))
    {
      orxCHAR zLocalName[256];

      /* Copies it locally */
      orxString_NCopy(zLocalName, sstParam.azParams[0], 256);

      /* Ends basename before extension */
      *(zLocalName + s32Index) = orxCHAR_NULL;

      /* Stores base name for config */
      orxConfig_SetBaseName(zLocalName);
    }
    else
    {
      /* Stores base name for config */
      orxConfig_SetBaseName(sstParam.azParams[0]);
    }
  }

  /* Done! */
  return eResult;
}

/** Displays help if requested
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxParam_DisplayHelp()
{
  orxPARAM  stParams;
  orxSTATUS eResult;

  /* Module initialized ? */
  orxASSERT((sstParam.u32Flags & orxPARAM_KU32_MODULE_FLAG_READY) == orxPARAM_KU32_MODULE_FLAG_READY);

  /* Everything seems ok. Register the module help function */
  stParams.u32Flags   = orxPARAM_KU32_FLAG_STOP_ON_ERROR;
  stParams.pfnParser  = orxParam_Help;
  stParams.zShortName = "h";
  stParams.zLongName  = "help";
  stParams.zShortDesc = "Display this help. You can use extra parameter to display complete description (-h <param>).";
  stParams.zLongDesc  = "h or help without parameter display the full list of parameters. if you supply extra parameters, their full description will be printed.";

  /* Register */
  eResult = orxParam_Register(&stParams);

  /* Done! */
  return eResult;
}
