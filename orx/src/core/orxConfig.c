/**
 * @file orxConfig.c
 */

/***************************************************************************
 orxConfig.c
 Lib C / file implementation of the Config module
 begin                : 09/12/2007
 author               : (C) Arcallians
 email                : cursor@arcallians.org
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

#include "core/orxConfig.h"
#include "debug/orxDebug.h"
#include "memory/orxBank.h"
#include "utils/orxLinkList.h"
#include "utils/orxString.h"


/** Module flags
 */
#define orxCONFIG_KU32_STATIC_FLAG_NONE   0x00000000  /**< No flags */

#define orxCONFIG_KU32_STATIC_FLAG_READY  0x00000001  /**< Ready flag */

#define orxCONFIG_KU32_STATIC_MASK_ALL    0xFFFFFFFF  /**< All mask */


/** Defines
 */
#define orxCONFIG_KU32_SECTION_BANK_SIZE  16         /**< Default section bank size */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Config node structure
 */
typedef struct __orxCONFIG_NODE_t
{
  orxLINKLIST_NODE  stNode;                 /**< Link list node : 12 */
  orxSTRING         zKey;                   /**< Key of node : 16 */
  orxSTRING         zValue;                 /**< Value of data : 20 */

  orxPAD(20)

} orxCONFIG_DATA;

/** Config section structure
 */
typedef struct __orxCONFIG_SECTION_t
{
  orxLINKLIST       stList;                 /**< List for nodes : 8 */

  orxPAD(8)

} orxCONFIG_SECTION;

/** Static structure
 */
typedef struct __orxCONFIG_STATIC_t
{
  orxBANK            *pstSectionBank;		/**< Bank of sections */
  orxCONFIG_SECTION  *pstCurrentSection;    /**< Current working section */
  orxSTRING           zFileName;            /**< Config file name */
  orxU32              u32Flags;             /**< Control flags */

} orxCONFIG_STATIC;


/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/

orxSTATIC orxCONFIG_STATIC sstConfig;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

///**
// * Save config node to file.
// * @param _pstNode (IN) Node to save
// * @param _zPrefix (IN) Prefix of node
// * @param _pstFile (IN) File where save the node.
// */
//orxSTATIC orxVOID  orxConfig_SaveNodeToFile(orxCONFIG_NODE* _pstNode, orxCONST orxSTRING _zPrefix, orxFILE* _pstFile)
//{
//	orxCONFIG_NODE* pstChild;
//    if(_pstNode)
//    {
//        orxBOOL bSave = orxFALSE;
//        if(_pstNode->zValue)
//        	bSave = orxTRUE;
//        else
//        {
//            pstChild = _pstNode->pstChild;
//            while(pstChild)
//            {
//                if(pstChild->zValue && !pstChild->pstChild)
//                {
//                	bSave = orxTRUE;
//                    break;
//                }
//                pstChild = pstChild->pstNext;
//            }
//        }
//        orxCHAR zCateg[1024] = {0};
//        orxCHAR zBuffer[4096] = {0};
//        orxSTRING zName;
//        if(_zPrefix)
//        {
//        	orxString_Print(zCateg, "%s/%s", _zPrefix, _pstNode->zName);
//            zName = zCateg;
//        }
//        else
//        	zName = _pstNode->zName;
//        if(bSave)
//        {
//            orxString_Print(zBuffer, "\n[%s]\n", zName);
//            orxFile_Write(zBuffer, orxString_GetLength(zBuffer), 1, _pstFile);
//            
//            if(_pstNode->zValue)
//            {
//                orxString_Print(zBuffer, "=%s\n", _pstNode->zValue);
//                orxFile_Write(zBuffer, orxString_GetLength(zBuffer), 1, _pstFile);
//            }
//            pstChild = _pstNode->pstChild;
//            while(pstChild)
//            {
//                if(pstChild->zValue && !pstChild->pstChild)
//                {
//                    orxString_Print(zBuffer, "%s=%s\n", pstChild->zName, pstChild->zValue);
//                    orxFile_Write(zBuffer, orxString_GetLength(zBuffer), 1, _pstFile);
//                }
//                pstChild = pstChild->pstNext;
//            }
//        }
//        pstChild = _pstNode->pstChild;
//        while(pstChild)
//        {
//            if(pstChild->pstChild)
//            	orxConfig_SaveNodeToFile(pstChild, zName, _pstFile);
//            pstChild = pstChild->pstNext;
//        }
//
//    }
//}
//
///**
// * Save config node to file.
// * @param _zFile (IN) File path where save the node.
// */
//orxSTATIC orxVOID  orxConfig_SaveToFile(orxCONST orxSTRING _zFile)
//{
//    orxFILE* pstFile = orxFile_Open(_zFile, orxFILE_KU32_FLAG_OPEN_WRITE);
//    if(pstFile)
//    {
//    	orxCONFIG_NODE* pstNode = &sstConfig.stRootNode;
//        while(pstNode)
//        {
//        	orxConfig_SaveNodeToFile(pstNode, orxNULL, pstFile);
//            pstNode = pstNode->pstNext;
//        }
//        orxFile_Close(pstFile);
//    }
//}

/** Deletes a section
 * @param[in] _pstSection       Section to delete
 */
orxSTATIC orxINLINE orxVOID orxConfig_DeleteSection(orxCONFIG_SECTION *_pstSection)
{
  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_pstSection != orxNULL);

  return;
}

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Inits the config module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxConfig_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY))
  {
    /* Cleans control structure */
    orxMemory_Set(&sstConfig, 0, sizeof(orxCONFIG_STATIC));

    /* Creates section bank */
    sstConfig.pstSectionBank = orxBank_Create(orxCONFIG_KU32_SECTION_BANK_SIZE, sizeof(orxCONFIG_SECTION), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if(sstConfig.pstSectionBank != orxNULL)
    {
      /* Inits Flags */
      orxFLAG_SET(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY, orxCONFIG_KU32_STATIC_MASK_ALL);

      /* Success */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* !!! MSG !!! */

      /* Section bank not created */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* !!! MSG !!! */
    
    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Exits from the config module
 */
orxVOID orxConfig_Exit()
{
  /* Initialized? */
  if(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY))
  {
    orxCONFIG_SECTION *pstSection;

    /* For all sections */
    for(pstSection = orxBank_GetNext(sstConfig.pstSectionBank, orxNULL);
        pstSection != orxNULL;
        pstSection = orxBank_GetNext(sstConfig.pstSectionBank, pstSection))
    {
      /* Deletes it */
      orxConfig_DeleteSection(pstSection);
    }

    /* Deletes section bank */
    orxBank_Delete(sstConfig.pstSectionBank);
    sstConfig.pstSectionBank = orxNULL;

    /* Updates flags */
    orxFLAG_SET(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_NONE, orxCONFIG_KU32_STATIC_FLAG_READY);
  }

  return;
}

/** Gets config file name
 * @return File name if loaded, orxSTRING_EMPTY otherwise
 */
orxSTRING orxConfig_GetFileName()
{
  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));

  /* Done! */
  return sstConfig.zFileName;
}

/** Selects current working section
* @param[in] _zSectionName     Section name to select
*/
orxVOID orxConfig_SelectSection(orxCONST orxSTRING _zSectionName)
{
  /* Checks */
  orxASSERT(orxFLAG_TEST(sstConfig.u32Flags, orxCONFIG_KU32_STATIC_FLAG_READY));
  orxASSERT(_zSectionName != orxNULL);
  orxASSERT(_zSectionName != orxSTRING_EMPTY);

  /* Done! */
  return;
}

/** Read config config from source.
 * @param[in] _zFileName        File name
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConfig_Load(orxCONST orxSTRING _zFileName)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Done! */
  return eResult;
}

/** Write config config to source.
 */
orxSTATUS orxConfig_Save()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Done! */
  return eResult;
}

/** Reads an integer value from config
 * @param[in] _zKey             Key name
 * @param[in] _s32DefaultValue  Default value if key is not found
 * @return The value
 */
orxS32 orxFASTCALL orxConfig_GetInt32(orxCONST orxSTRING _zKey)
{
  orxS32 s32Result = 0;

  /* Done! */
  return s32Result;
}

/** Reads a float value from config
 * @param[in] _zKey             Key name
 * @param[in] _fDefaultValue    Default value if key is not found
 * @return The value
 */
orxFLOAT orxFASTCALL orxConfig_GetFloat(orxCONST orxSTRING _zKey)
{
  orxFLOAT fResult = orxFLOAT_0;

  /* Done! */
  return fResult;
}

/** Reads a string value from config
 * @param[in] _zKey             Key name
 * @param[in] _zDefaultValue    Default value if key is not found
 * @return The value
 */
orxSTRING orxFASTCALL orxConfig_GetString(orxCONST orxSTRING _zKey)
{
  orxSTRING zResult = orxSTRING_EMPTY;

  /* Done! */
  return zResult;
}

/** Reads a boolean value from config
 * @param[in] _zKey             Key name
 * @param[in] _bDefaultValue    Default value if key is not found
 * @return The value
 */
orxBOOL orxFASTCALL orxConfig_GetBool(orxCONST orxSTRING _zKey)
{
  orxBOOL bResult = orxFALSE;

  /* Done! */
  return bResult;
}

/** Writes an integer value to config
 * @param[in] _zKey             Key name
 * @param[in] _s32Value         Value
 */
orxVOID orxFASTCALL orxConfig_SetInt32(orxCONST orxSTRING _zKey, orxS32 _s32Value)
{
  return;
}

/** Writes a float value to config
 * @param[in] _zKey             Key name
 * @param[in] _fValue           Value
 */
orxVOID orxFASTCALL orxConfig_SetFloat(orxCONST orxSTRING _zKey, orxFLOAT _fValue)
{
    return;
}

/** Writes a string value to config
 * @param[in] _zKey             Key name
 * @param[in] _fValue           Value
 */
orxVOID orxFASTCALL orxConfig_SetString(orxCONST orxSTRING _zKey, orxCONST orxSTRING _zValue)
{
  return;
}

/** Writes a boolean value to config
 * @param[in] _zKey             Key name
 * @param[in] _fValue           Value
 */
orxVOID orxFASTCALL orxConfig_SetBool(orxCONST orxSTRING _zKey, orxBOOL _bValue)
{
  return;
}
