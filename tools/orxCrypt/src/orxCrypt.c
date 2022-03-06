/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2022 Orx-Project
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
 * @file orxCrypt.c
 * @date 09/09/2009
 * @author iarwain@orx-project.org
 *
 */


#include "orx.h"


/** Module flags
 */
#define orxCRYPT_KU32_STATIC_FLAG_NONE            0x00000000  /**< No flags */

#define orxCRYPT_KU32_STATIC_FLAG_INPUT_LOADED    0x00000001  /**< Input loaded flag */
#define orxCRYPT_KU32_STATIC_FLAG_USE_ENCRYPTION  0x00000002  /**< Use encryption flag */

#define orxCRYPT_KU32_STATIC_MASK_ALL             0xFFFFFFFF  /**< All mask */


/** Defines
 */
#define orxCRYPT_KZ_DEFAULT_OUTPUT                "orxcrypt.out"

#define orxCRYPT_KZ_LOG_TAG_LENGTH                "10"

#if defined(__orxGCC__) || defined(__orxLLVM__)

  #define orxCRYPT_LOG(TAG, FORMAT, ...) orxLOG(orxANSI_KZ_COLOR_FG_YELLOW "%-" orxCRYPT_KZ_LOG_TAG_LENGTH "s" orxANSI_KZ_COLOR_FG_DEFAULT FORMAT, "[" #TAG "]", ##__VA_ARGS__)

#else // __orxGCC__ || __orxLLVM__

  #define orxCRYPT_LOG(TAG, FORMAT, ...) orxLOG(orxANSI_KZ_COLOR_FG_YELLOW "%-" orxCRYPT_KZ_LOG_TAG_LENGTH "s" orxANSI_KZ_COLOR_FG_DEFAULT FORMAT, "[" #TAG "]", __VA_ARGS__)

#endif //__orxGCC__ || __orxLLVM__


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxCRYPT_STATIC_t
{
  orxSTRING  *azInputFile;
  orxSTRING   zOutputFile;
  orxU32      u32InputNumber;
  orxU32      u32Flags;

} orxCRYPT_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** static data
 */
static orxCRYPT_STATIC sstCrypt;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

static orxBOOL orxFASTCALL SaveFilter(const orxSTRING _zSectionName, const orxSTRING _zKeyName, const orxSTRING _zFileName, orxBOOL _bUseEncryption)
{
  orxBOOL bResult = orxTRUE;

  // Is param section?
  if(!orxString_Compare(_zSectionName, "Param"))
  {
    // Section?
    if(!_zKeyName)
    {
      // Pushes it
      orxConfig_PushSection("Param");

      // Is empty?
      if(orxConfig_GetKeyCount() == 0)
      {
        // Don't save it
        bResult = orxFALSE;
      }

      // Pops previous section
      orxConfig_PopSection();
    }
    // Is one of our keys?
    else if(_zKeyName
    && (!orxString_Compare(_zKeyName, "merge")
     || !orxString_Compare(_zKeyName, "filelist")
     || !orxString_Compare(_zKeyName, "key")
     || !orxString_Compare(_zKeyName, "output")
     || !orxString_Compare(_zKeyName, "decrypt")))
    {
      // Don't save it
      bResult = orxFALSE;
    }
  }
  // Is config section?
  else if(!orxString_Compare(_zSectionName, "Config"))
  {
    // Section?
    if(!_zKeyName)
    {
      // Pushes it
      orxConfig_PushSection("Config");

      // Is empty?
      if(orxConfig_GetKeyCount() == 0)
      {
        // Don't save it
        bResult = orxFALSE;
      }

      // Pops previous section
      orxConfig_PopSection();
    }
  }

  // Done!
  return bResult;
}

static orxSTATUS orxFASTCALL ProcessInputParams(orxU32 _u32ParamCount, const orxSTRING _azParams[])
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  // Has a valid key parameter?
  if(_u32ParamCount > 1)
  {
    orxU32 i;

    // Stores input number
    sstCrypt.u32InputNumber = _u32ParamCount - 1;

    // Allocates input array
    sstCrypt.azInputFile = (orxSTRING *)orxMemory_Allocate(sstCrypt.u32InputNumber * sizeof(orxSTRING *), orxMEMORY_TYPE_TEMP);

    // Checks
    orxASSERT(sstCrypt.azInputFile);

    // For all config files
    for(i = 1; i < _u32ParamCount; i++)
    {
      // Loads input file
      eResult = orxConfig_Load(_azParams[i]);

      // Success?
      if(eResult != orxSTATUS_FAILURE)
      {
        // Stores it
        sstCrypt.azInputFile[i - 1] = orxString_Duplicate(_azParams[i]);

        // Logs message
        orxCRYPT_LOG(LOAD, "%3u: %-24.24s SUCCESS", i, _azParams[i]);
      }
      else
      {
        // Logs message
        orxCRYPT_LOG(LOAD, "%3u: %-24.24s FAILURE, aborting.", i, _azParams[i]);
        break;
      }
    }

    // Success?
    if(eResult != orxSTATUS_FAILURE)
    {
      // Updates status
      orxFLAG_SET(sstCrypt.u32Flags, orxCRYPT_KU32_STATIC_FLAG_INPUT_LOADED, orxCRYPT_KU32_STATIC_FLAG_NONE);
    }
    else
    {
      // Clears input values
      orxMemory_Free(sstCrypt.azInputFile);
      sstCrypt.azInputFile    = orxNULL;
      sstCrypt.u32InputNumber = 0;
    }
  }
  else
  {
    // Logs message
    orxCRYPT_LOG(INPUT, "No valid file list found, aborting");
  }

  // Done!
  return eResult;
}

static orxSTATUS orxFASTCALL ProcessOutputParams(orxU32 _u32ParamCount, const orxSTRING _azParams[])
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // Has a valid output parameter?
  if(_u32ParamCount > 1)
  {
    // Stores it
    sstCrypt.zOutputFile = orxString_Duplicate(_azParams[1]);
  }
  else
  {
    // Logs message
    orxCRYPT_LOG(OUTPUT, "No valid output found, using default");
  }

  // Done!
  return eResult;
}

static orxSTATUS orxFASTCALL ProcessKeyParams(orxU32 _u32ParamCount, const orxSTRING _azParams[])
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  // Has a valid key parameter?
  if(_u32ParamCount > 1)
  {
    const orxSTRING zKey = orxNULL;

    // More than one parameter?
    if(_u32ParamCount > 2)
    {
      // Pushes param config section
      orxConfig_PushSection("Param");

      // Has key?
      if(orxConfig_HasValue("key"))
      {
        // Gets it
        zKey = orxConfig_GetString("key");
      }
      else
      {
        // Logs message
        orxCRYPT_LOG(KEY, "If you want to use a key containing spaces, it *NEEDS* to be provided in a config file as value for Param.key, aborting");
      }

      // Pops to previous section
      orxConfig_PopSection();
    }
    else
    {
      // Gets it
      zKey = _azParams[1];
    }

    // Has valid key?
    if(zKey)
    {
      // Sets it
      if(orxConfig_SetEncryptionKey(zKey) != orxSTATUS_FAILURE)
      {
        // Logs message
        orxCRYPT_LOG(KEY, "Key set to [%s]", zKey);

        // Updates result
        eResult = orxSTATUS_SUCCESS;
      }
      else
      {
        // Logs message
        orxCRYPT_LOG(KEY, "Couldn't set [%s] as encryption key, aborting", zKey);
      }
    }
  }
  else
  {
    // Logs message
    orxCRYPT_LOG(KEY, "No valid key found, using default");

    // Updates result
    eResult = orxSTATUS_SUCCESS;
  }

  // Done!
  return eResult;
}

static orxSTATUS orxFASTCALL ProcessDecryptParams(orxU32 _u32ParamCount, const orxSTRING _azParams[])
{
  orxSTATUS eResult;

  // Has a valid decrypt parameter?
  if(_u32ParamCount >= 1)
  {
    // Updates status
    orxFLAG_SET(sstCrypt.u32Flags, orxCRYPT_KU32_STATIC_FLAG_NONE, orxCRYPT_KU32_STATIC_FLAG_USE_ENCRYPTION);

    // Updates result
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    // Updates result
    eResult = orxSTATUS_FAILURE;
  }

  // Done!
  return eResult;
}

static void orxFASTCALL Setup()
{
  // Adds module dependencies
  orxModule_AddDependency(orxMODULE_ID_MAIN, orxMODULE_ID_PARAM);
  orxModule_AddDependency(orxMODULE_ID_MAIN, orxMODULE_ID_CONFIG);
}

static orxSTATUS orxFASTCALL Init()
{
#define orxCRYPT_DECLARE_PARAM(SN, LN, SD, LD, FN) {orxPARAM_KU32_FLAG_STOP_ON_ERROR, SN, LN, SD, LD, &FN},

  orxU32    i;
  orxSTATUS eResult = orxSTATUS_SUCCESS;
  orxPARAM  astParamList[] =
  {
    orxCRYPT_DECLARE_PARAM("k", "key", "Key for decoding/encoding", "Key used for decoding/encoding provided config files", ProcessKeyParams)
    orxCRYPT_DECLARE_PARAM("f", "filelist", "Input file list", "List of root config files to decode/encode, only 1 input file is allowed in non-merge mode", ProcessInputParams)
    orxCRYPT_DECLARE_PARAM("o", "output", "Output file", "Single output file where decoded/encoded config info will be saved", ProcessOutputParams)
    orxCRYPT_DECLARE_PARAM("d", "decrypt", "decrypt mode", "If this switch is provided, the saved file will *NOT* be encrypted, otherwise it will, by default", ProcessDecryptParams)
  };

  // Clears static controller
  orxMemory_Zero(&sstCrypt, sizeof(orxCRYPT_STATIC));

  // Defaults to encryption mode
  orxFLAG_SET(sstCrypt.u32Flags, orxCRYPT_KU32_STATIC_FLAG_USE_ENCRYPTION, orxCRYPT_KU32_STATIC_MASK_ALL);

  // For all params
  for(i = 0; (i < sizeof(astParamList) / sizeof(astParamList[0])) && (eResult != orxSTATUS_FAILURE); i++)
  {
    // Registers param
    eResult = orxParam_Register(&astParamList[i]);
  }

  // Done!
  return eResult;
}

static void orxFASTCALL Exit()
{
  // Has input file?
  if(orxFLAG_TEST(sstCrypt.u32Flags, orxCRYPT_KU32_STATIC_FLAG_INPUT_LOADED))
  {
    orxU32 i;

    // For all inputs
    for(i = 0; i < sstCrypt.u32InputNumber; i++)
    {
      // Frees its string
      orxString_Delete(sstCrypt.azInputFile[i]);
    }

    // Frees input array
    orxMemory_Free(sstCrypt.azInputFile);
  }

  // Has output file?
  if(sstCrypt.zOutputFile)
  {
    // Frees its string
    orxString_Delete(sstCrypt.zOutputFile);
  }
}

static void Run()
{
  // Has loaded input?
  if(orxFLAG_TEST(sstCrypt.u32Flags, orxCRYPT_KU32_STATIC_FLAG_INPUT_LOADED))
  {
    const orxSTRING zOutputFile;
    orxBOOL         bEncrypt;

    // Gets encryption status
    bEncrypt = orxFLAG_TEST(sstCrypt.u32Flags, orxCRYPT_KU32_STATIC_FLAG_USE_ENCRYPTION);

    // Selects correct output file
    zOutputFile = (sstCrypt.zOutputFile) ? sstCrypt.zOutputFile : orxCRYPT_KZ_DEFAULT_OUTPUT;

    // Merge files
    if(orxConfig_MergeFiles(zOutputFile, (const orxSTRING *)sstCrypt.azInputFile, sstCrypt.u32InputNumber, bEncrypt ? orxConfig_GetEncryptionKey() : orxNULL) != orxSTATUS_FAILURE)
    {
      // Logs message
      orxCRYPT_LOG(SAVE, "==== %-24.24s SUCCESS%s%s", zOutputFile, (sstCrypt.u32InputNumber > 1) ? " (MERGED)" : orxSTRING_EMPTY, bEncrypt ? " (ENCRYPTED)" : orxSTRING_EMPTY);
    }
    else
    {
      // Logs message
      orxCRYPT_LOG(SAVE, "==== %-24.24s FAILURE, aborting.", zOutputFile);
    }
  }
  else
  {
    // Logs message
    orxCRYPT_LOG(PROCESS, "No loaded files, can't process.");
  }
}

int main(int argc, char **argv)
{
  // Inits the Debug System
  orxDEBUG_INIT();

  // Sets debug flags
  orxDEBUG_SET_FLAGS(orxDEBUG_KU32_STATIC_FLAG_TIMESTAMP, orxDEBUG_KU32_STATIC_FLAG_FULL_TIMESTAMP | orxDEBUG_KU32_STATIC_FLAG_TYPE | orxDEBUG_KU32_STATIC_FLAG_TAGGED);

  // Registers main module
  orxModule_Register(orxMODULE_ID_MAIN, "MAIN", Setup, Init, Exit);

  // Sends the command line arguments to orxParam module
  if(orxParam_SetArgs(argc, argv) != orxSTATUS_FAILURE)
  {
    // Inits the engine
    if(orxModule_Init(orxMODULE_ID_MAIN) != orxSTATUS_FAILURE)
    {
      // Runs
      Run();

      // Exits from engine
      orxModule_Exit(orxMODULE_ID_MAIN);
    }
  }

  // Exits from the Debug system
  orxDEBUG_EXIT();

  // Done!
  return EXIT_SUCCESS;
}
