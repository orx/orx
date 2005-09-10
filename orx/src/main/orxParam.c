/**
 * @file orxParam.c
 * 
 * Module that will manage command line options sent to orx
 * 
 */
 
 /***************************************************************************
 orxParam.c
 Module that will manage command line options sent to orx
 
 begin                : 09/09/2005
 author               : (C) Arcallians
 email                : bestel@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "main/orxParam.h"
#include "memory/orxMemory.h"
#include "utils/orxString.h"
#include "memory/orxBank.h"
#include "utils/orxHashTable.h"
#include "debug/orxDebug.h"

#define orxPARAM_KU32_MODULE_FLAG_NONE   0x00000000  /**< No flags have been set */
#define orxPARAM_KU32_MODULE_FLAG_READY  0x00000001  /**< The module has been initialized */

#define orxPARAM_KU32_MODULE_BANK_SIZE 32      /**< Average max number of parameter that can be registered */
#define orxPARAM_KZ_MODULE_SHORT_PREFIX   "-"  /**< Prefix for short parameters */
#define orxPARAM_KZ_MODULE_LONG_PREFIX    "--" /**< Prefix for long parameters */

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/
typedef struct __orxPARAM_INFOS_t
{
  orxPARAM stParam; /* Param values */
  orxU32 u32Count;  /* Number of time that this param has been met in the command line */
} orxPARAM_INFOS;
 
typedef struct __orxPARAM_STATIC_t
{
  orxU32        u32Flags;     /* Module flags */
  orxBANK      *pstBank;      /* Bank of registered parameters */
  orxHASHTABLE *pstHashTable; /* HashTable of registered Parameters */
  
} orxPARAM_STATIC;

/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/
orxSTATIC orxPARAM_STATIC sstParam;

/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Return the parameter infos associated to the given value.
 * @param[in] _zParamName Name of the parameter (with short or long prefix inside)
 * @return Returns the pointer on the param infos if found, else returns orxNULL
 */
orxPARAM_INFOS *orxParam_Get(orxU32 _u32ParamName)
{
  orxPARAM_INFOS *pstParamInfos; /* Parameters infos extracted from the Hash Table */
  
  /* Module initialized ? */
  orxASSERT((sstParam.u32Flags & orxPARAM_KU32_MODULE_FLAG_READY) == orxPARAM_KU32_MODULE_FLAG_READY);
  
  /* Get the parameter pointer */
  pstParamInfos = (orxPARAM_INFOS *)orxHashTable_Get(sstParam.pstHashTable, _u32ParamName);
  
  /* Returns it */
  return pstParamInfos;
}

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/
/** Initialize Param Module
 */
orxSTATUS orxParam_Init()
{
  /* Declare variables */
  orxSTATUS eResult = orxSTATUS_FAILED;
  
  /* Init dependencies */
  if ((orxDEPEND_INIT(Depend) &
       orxDEPEND_INIT(Memory) &
       orxDEPEND_INIT(String) &
       orxDEPEND_INIT(HashTable) &
       orxDEPEND_INIT(Bank)) == orxSTATUS_SUCCESS)
  {
    /* Not already Initialized? */
    if(!(sstParam.u32Flags & orxPARAM_KU32_MODULE_FLAG_READY))
    {
      /* Cleans static controller */
      orxMemory_Set(&sstParam, 0, sizeof(orxPARAM_STATIC));
      
      /* Create an empty bank to store parameters */
      sstParam.pstBank = orxBank_Create(orxPARAM_KU32_MODULE_BANK_SIZE,
                                        sizeof(orxPARAM_INFOS),
                                        orxBANK_KU32_FLAGS_NONE,
                                        orxMEMORY_TYPE_MAIN);

      /* Bank successfully created ? */
      if (sstParam.pstBank != orxNULL)
      {
        /* Now create the hash table */
        sstParam.pstHashTable = orxHashTable_Create(orxPARAM_KU32_MODULE_BANK_SIZE * 2,
                                                    orxHASHTABLE_KU32_FLAGS_NONE,
                                                    orxMEMORY_TYPE_MAIN);
                                                    
        /* HashTable Created ? */
        if (sstParam.pstHashTable != orxNULL)
        {
          /* Set module as ready */
          sstParam.u32Flags = orxPARAM_KU32_MODULE_FLAG_READY;
          
          /* Success */
          eResult = orxSTATUS_SUCCESS;
        }
      }
    }
  }
  
  /* Done */
  return eResult;
}

/** Exit Param module
 */
orxVOID orxParam_Exit()
{
  /* Module initialized ? */
  if ((sstParam.u32Flags & orxPARAM_KU32_MODULE_FLAG_READY) == orxPARAM_KU32_MODULE_FLAG_READY)
  {
    /* Module not ready now */
    sstParam.u32Flags = orxPARAM_KU32_MODULE_FLAG_NONE;
  }

  /* Exit dependencies */
  orxDEPEND_EXIT(Bank);
  orxDEPEND_EXIT(HashTable);
  orxDEPEND_EXIT(String);
  orxDEPEND_EXIT(Memory);
  orxDEPEND_EXIT(Depend);
}

/** Register a new parameter
 * @param[in] _pstParam Informations about the option to register
 * @return Returns the status of the registration
 */
orxSTATUS orxFASTCALL orxParam_Register(orxCONST orxPARAM *_pstParam)
{
  orxPARAM_INFOS *pstParamInfos;       /* Parameter stored in the bank */
  orxSTATUS eReult = orxSTATUS_FAILED; /* Result of the operation */
  
  /* Module initialized ? */
  orxASSERT((sstParam.u32Flags & orxPARAM_KU32_MODULE_FLAG_READY) == orxPARAM_KU32_MODULE_FLAG_READY);
  
  /* Correct parameters ? */
  orxASSERT(_pstParam != orxNULL);

  /* Short parameters and callbacks are compulsory */
  if (_pstParam->zShortName != orxNULL &&
      _pstParam->zShortDesc != orxNULL &&
      _pstParam->cbParser   != orxNULL)
  {
    orxU32 u32ShortName;
    
    /* Creates CRC for the Short Name */
    u32ShortName = orxString_ToCRC(orxPARAM_KZ_MODULE_SHORT_PREFIX);
    u32ShortName = orxString_ContinueCRC((orxCONST orxSTRING)_pstParam->zShortName, u32ShortName);
    
    /* Check if options with the same name don't have already been registered */
    if (orxParam_Get(u32ShortName) == orxNULL)
    {
      orxU32 u32LongName;
      orxBOOL bStoreParam = orxTRUE; /* No problem at the moment, we can store the parameter */
      
      /* Check if the long name has not already been registered too */
      if (_pstParam->zLongName != orxNULL)
      {
        /* We are not sure to store the param since it could have a problem with the long name */
        bStoreParam = orxFALSE;
        
        /* Create CRC For the long name */
        u32LongName = orxString_ToCRC(orxPARAM_KZ_MODULE_LONG_PREFIX);
        u32LongName = orxString_ContinueCRC((orxCONST orxSTRING)_pstParam->zLongName, u32LongName);
        
        /* Found ? */
        if (orxParam_Get(u32LongName) == orxNULL)
        {
          /* No Params have been found, we can store it */
          bStoreParam = orxTRUE;
        }
      }
      
      /* Can we store the parameter ? */
      if (bStoreParam)
      {
          /* Allocate a new cell in the bank */
          pstParamInfos = (orxPARAM_INFOS *)orxBank_Allocate(sstParam.pstBank);
        
          /* Allocation success ? */
          if (pstParamInfos != orxNULL)
          {
            /* Copy input values */
            pstParamInfos->stParam  = *_pstParam;
            pstParamInfos->u32Count = 0;
            
            /* Store Params in the hash Table (with short name as key) */
            orxHashTable_Add(sstParam.pstHashTable, u32ShortName, pstParamInfos);
            
            /* Store Param with long name as key if it exists */
            if (_pstParam->zLongName != orxNULL)
            {
              orxHashTable_Add(sstParam.pstHashTable, u32LongName, pstParamInfos);
            }
          }
        }
        else
        {
          orxDEBUG_LOG(orxDEBUG_LEVEL_PARAM,
                       "A parameter with the same long name (%s%s) has already been registered",
                       orxPARAM_KZ_MODULE_LONG_PREFIX,
                       _pstParam->zLongName);
        }
    }
    else
    {
      orxDEBUG_LOG(orxDEBUG_LEVEL_PARAM,
                   "A parameter with the same short name (%s%s) has already been registered",
                   orxPARAM_KZ_MODULE_SHORT_PREFIX,
                   _pstParam->zShortName);
    }
  }
  else
  {
    orxDEBUG_LOG(orxDEBUG_LEVEL_PARAM, "Invalid registered parameter... Forgets it");
  }
  
  /* Done */
  return eReult;
}

/** Parse the command line
 * @param[in] _u32NbParam Number of read parameters
 * @param[in] _azParams   List of parameters
 * @return Returns the parsing status
 */
orxSTATUS orxFASTCALL orxParam_Parse(orxU32 _u32NbParams, orxCONST orxSTRING _azParams[])
{
  /* Module initialized ? */
  orxASSERT((sstParam.u32Flags & orxPARAM_KU32_MODULE_FLAG_READY) == orxPARAM_KU32_MODULE_FLAG_READY);
  
  orxU32 u32Index;
  orxSTATUS eResult = orxSTATUS_SUCCESS;
    
  /* Loop on Extra parameters */
  for (u32Index = 0; (eResult = orxSTATUS_SUCCESS) && (u32Index < _u32NbParams); u32Index++)
  {
    /* Is the short or long prefix is found on the first characters ? */
    if ((orxString_SearchString(_azParams[u32Index], orxPARAM_KZ_MODULE_SHORT_PREFIX) == _azParams[u32Index]) ||
        (orxString_SearchString(_azParams[u32Index], orxPARAM_KZ_MODULE_LONG_PREFIX) == _azParams[u32Index]))
    {
      orxPARAM_INFOS *pstParamInfos;
      
      /* We found a candidate. Now get it's param value */
      pstParamInfos = (orxPARAM_INFOS *)orxParam_Get(orxString_ToCRC(_azParams[u32Index]));
      
      /* Look at the number of time that this parameter as been set on the command line, and if multiple instance are allowed */
      if ((pstParamInfos != orxNULL) &&
          (pstParamInfos->u32Count > 0) &&
          ((pstParamInfos->stParam.u32Flags & orxPARAM_KU32_FLAGS_MULTIPLE_ALLOWED) == orxPARAM_KU32_FLAGS_MULTIPLE_ALLOWED))
      {
        /* Number of extra parameters */
        orxU32 u32NbExtra = 1;

        /* Increases ref counter */
        pstParamInfos->u32Count++;
        
        /* Now, count the number of extra params */
        while (((u32NbExtra + u32Index) < _u32NbParams) &&
               (orxString_SearchString(_azParams[u32NbExtra + u32Index], orxPARAM_KZ_MODULE_SHORT_PREFIX) != _azParams[u32NbExtra + u32Index]) &&
               (orxString_SearchString(_azParams[u32NbExtra + u32Index], orxPARAM_KZ_MODULE_SHORT_PREFIX) != _azParams[u32NbExtra + u32Index]))
        {
          u32NbExtra++;
        }
        
        /* Valid extra value ? */
        if ((u32NbExtra + u32Index) < _u32NbParams)
        {
          /* Call the callback function
           * (It can't be orxNULL since only param with a registered callback can be stored)
           */
          if ((pstParamInfos->stParam.cbParser(u32NbExtra, (orxSTRING *)(&(_azParams[u32Index]))) == orxSTATUS_FAILED) &&
              ((pstParamInfos->stParam.u32Flags & orxPARAM_KU32_FLAGS_STOP_ON_ERROR) == orxPARAM_KU32_FLAGS_STOP_ON_ERROR))
          {
            eResult = orxSTATUS_FAILED;
          }
        }
      }
      else
      {
        /* Increases ref counter */
        pstParamInfos->u32Count++;
        
        /* No multiple instance are allowed. Stop the process ? */
        if ((pstParamInfos->stParam.u32Flags & orxPARAM_KU32_FLAGS_STOP_ON_ERROR) == orxPARAM_KU32_FLAGS_STOP_ON_ERROR)
        {
          eResult = orxSTATUS_FAILED;
        }
      }
    }
  }
  
  /* Done */
  return eResult;
}
