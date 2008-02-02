/**
 * @file orxRegistry.c
 */

/***************************************************************************
 orxRegistry.c
 Lib C / file implementation of the Registry module
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
#include "plugin/orxPluginUser.h"
#include "debug/orxDebug.h"
#include "io/orxRegistry.h"

#include "io/orxFile.h"
#include "memory/orxBank.h"
#include "utils/orxString.h"


/** Module flags
 */
#define orxREGISTRY_KU32_STATIC_FLAG_NONE   0x00000000  /**< No flags */

#define orxREGISTRY_KU32_STATIC_FLAG_READY  0x00000001  /**< Ready flag */

#define orxREGISTRY_KU32_STATIC_MASK_ALL    0xFFFFFFFF  /**< All mask */


/* Defines
 */
#define orxREGISTRY_KU32_BANK_SIZE          256         /**< Default bank size */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Registry node structure
 */
typedef struct __orxREGISTRY_NODE_t
{
	orxSTRING                    zName;	      /**< Name of the node. */
	orxSTRING                    zValue;	    /**< Node value. */
	struct __orxREGISTRY_NODE_t* pstNext;     /**< Next node. */
	struct __orxREGISTRY_NODE_t* pstChild;    /**< First child node. */
	
} orxREGISTRY_NODE;

/** Static structure
 */
typedef struct __orxREGISTRY_STATIC_t
{
  orxU32            u32Flags;               /**< Flags set by the mouse plugin module */
  orxBANK*          pstBank;		            /**< Bank to allocate node. */
  orxREGISTRY_NODE  stRootNode;	            /**< Root node. */

} orxREGISTRY_STATIC;


orxVOID orxRegistry_LibC_SetString(orxCONST orxSTRING _zKey, orxCONST orxSTRING _zValue);


/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/
orxSTATIC orxREGISTRY_STATIC sstRegistry;

/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Create a list of node names from a path.
 * The user must free the returned list.
 */
orxSTATIC orxSTRING orxRegistry_LibC_CreatePathList(orxCONST orxCHAR* _zPath)
{
	orxCHAR* zStr  = (orxCHAR*)orxMemory_Allocate(orxString_GetLength((orxCONST orxSTRING)_zPath)+1, orxMEMORY_TYPE_MAIN);
    orxCHAR* zRes  = zStr;
    orxBOOL   bTest = orxFALSE;

    while(*_zPath=='/'||*_zPath=='\\')
    	_zPath++;

    while(*_zPath!=0)
    {
        if(*_zPath=='/'||*_zPath=='\\')
        {
        	_zPath++;
            if(!bTest)
            {
                *zStr = 0;
                zStr++;
                bTest = orxTRUE;
            }
        }
        else
        {
            bTest = orxFALSE;
            *zStr = *_zPath;
            _zPath++;
            zStr++;
        }
    }

    *zStr = 0;
    zStr++;
    *zStr = 0;
    return zRes;
}

/** Find a node from a list of node names.
 * @param _zList 	(IN) List of node names to search (@see orxRegistry_LibC_CreatePathList)
 * @param _bCreate	(IN) True to create leak nodes.
 * @return Wanted node or orxNULL if not found.
 */
orxSTATIC orxREGISTRY_NODE* orxRegistry_LibC_FindList(orxCONST orxCHAR* _zList, orxBOOL _bCreate)
{
    orxREGISTRY_NODE *pstParent = &sstRegistry.stRootNode,
    				 *pstBase = orxNULL;

    /* Create first node */
    if(!pstParent->pstChild)
    {
        if(!_bCreate)
            return NULL;
        else
        {
            pstParent->pstChild = (orxREGISTRY_NODE*) orxMemory_Allocate(sizeof(orxREGISTRY_NODE), orxMEMORY_TYPE_MAIN);
            pstParent->pstChild->zName    = orxString_Duplicate((orxCONST orxSTRING)_zList);
            pstParent->pstChild->zValue   = orxNULL;
            pstParent->pstChild->pstNext  = orxNULL;
            pstParent->pstChild->pstChild = orxNULL;
        }
    }

    pstBase = pstParent->pstChild;

    while(*_zList!=0)
    {
        if(!pstBase)
        {
            if(_bCreate)
            {
                orxREGISTRY_NODE* pstTmp = (orxREGISTRY_NODE*) orxMemory_Allocate(sizeof(orxREGISTRY_NODE), orxMEMORY_TYPE_MAIN);
                pstTmp->zName  = orxString_Duplicate((orxCONST orxSTRING)_zList);
                _zList += orxString_GetLength((orxCONST orxSTRING)_zList)+1;
                pstTmp->zValue   = orxNULL;
                pstTmp->pstChild = orxNULL;
                pstTmp->pstNext  = pstParent->pstChild;
                pstParent = pstParent->pstChild = pstTmp;
                pstBase   = orxNULL;
                continue;
            }
            else
            {
                return NULL;
            }
        }

        if(!strcmp(pstBase->zName, _zList))
        {
            pstParent = pstBase;
            pstBase = pstBase->pstChild;
            _zList += orxString_GetLength((orxCONST orxSTRING)_zList)+1;
        }
        else
            pstBase = pstBase->pstNext;

    }

    if(*_zList==0)
        return pstParent;

    return orxNULL;
}

/** Find a node with the specified path.
 * If not found, create it (and all subnode) if bCreate is orxTRUE.
 */
orxSTATIC orxREGISTRY_NODE* orxRegistry_LibC_FindNode(orxCONST orxSTRING _zPath, orxBOOL _bCreate)
{
	/* Module not already initialized ? */
	orxASSERT(_zPath);

	orxSTRING zList = orxRegistry_LibC_CreatePathList(_zPath);
    orxREGISTRY_NODE* pstRes = orxRegistry_LibC_FindList(zList, _bCreate);
    orxMemory_Free(zList);
    return pstRes;
}

/** Create a node. (not used yet)
 * @param _zPath	(IN) Path of the node to create.
 * @return Create node.
 */ 
orxSTATIC orxREGISTRY_NODE* orxRegistry_LibC_CreateNode(orxCONST orxSTRING _zPath)
{
	orxSTRING zList = orxRegistry_LibC_CreatePathList(_zPath);
    orxREGISTRY_NODE* pstRes = orxRegistry_LibC_FindNode(zList, orxTRUE);
    orxMemory_Free(zList);
    return pstRes;
}

/** Remove a node (not used yet).
 * The node is not really destroyed but all its children are.
 * @param _pstNode	(IN) Node to destroy.
 */
orxSTATIC orxVOID orxRegistry_LibC_RemoveNode(orxREGISTRY_NODE* _pstNode)
{
    if(_pstNode)
    {
        if(_pstNode->pstChild)
        {
            orxREGISTRY_NODE *pstNode1 = _pstNode->pstChild, *pstNode2;
            while(pstNode1)
            {
            	pstNode2 = pstNode1->pstNext;
            	orxRegistry_LibC_RemoveNode(pstNode1);
            	orxMemory_Free(pstNode1->zName);
            	orxMemory_Free(pstNode1);
                pstNode1 = pstNode2;
            }
            _pstNode->pstChild = NULL;
        }
        if(_pstNode->zValue)
        	orxMemory_Free(_pstNode->zValue);
    }
}

/** Remove a node from its path.
 * @param _zPath	(IN) Path of the node to remove.
 */
orxSTATIC orxVOID orxRegistry_LibC_Remove(orxCONST orxSTRING _zPath)
{
	orxRegistry_LibC_RemoveNode(orxRegistry_LibC_FindNode(_zPath, orxFALSE));
}

/** Remove all config.
 */
orxSTATIC orxVOID orxRegistry_LibC_Clear()
{
	orxRegistry_LibC_RemoveNode(&sstRegistry.stRootNode);
}

/** Dump a regitry subtree.
 * @param _pstNode Node to dump.
 * @param _s32Offset Offset to show.
 */
orxSTATIC orxVOID orxRegistry_LibC_Dump(orxREGISTRY_NODE* pstNode, orxS32 _s32Offset)
{
    if(pstNode==&sstRegistry.stRootNode)
    	pstNode = pstNode->pstChild;

    while(pstNode)
    {
        orxS32 s32;
        for(s32=0; s32<_s32Offset; s32++)
            printf("  ");
        printf("%s = %s\n", pstNode->zName, pstNode->zValue);
        orxRegistry_LibC_Dump(pstNode->pstChild, _s32Offset+1);
        pstNode = pstNode->pstNext;
    }
}

/** Load registry content from file..
 * @param _zFile File path.
 */
orxSTATIC orxVOID  orxRegistry_LibC_LoadFromFile(orxCONST orxSTRING _zFile)
{
    orxCHAR  zBuffer[1024], zSection[1024], zVar[1024];
    orxCHAR *zName, *zValue;
    orxS32   s32;
    orxFILE* pstFile = orxFile_Open(_zFile, orxFILE_KU32_FLAG_OPEN_READ);
    if(pstFile)
    {
        while(orxFile_Gets(zBuffer, 1024, pstFile))
        {
            orxSTRING zStr = zBuffer;
            while(*zStr==' '||*zStr=='\t')
            	zStr++;
            if(*zStr==';')
                continue; /* comment */
            else if(*zStr=='[')
            {
                /* section name */
            	zName = zStr+1;
                zStr = orxString_SearchChar(zStr, ']');
                if(!zStr)
                    continue; /* section name not closed. */
                *zStr = 0;
                orxString_Copy(zSection, zName);
            }
            else if(*zStr!=0||*zStr!='\n')
            {
                /* Non empty line. */
            	zValue = orxString_SearchChar(zBuffer, '=');
                if(!zValue)
                    continue; /* No equal character. */
                s32 = orxString_GetLength(zValue);
                if(zValue[s32-1]=='\n')
                	zValue[--s32] = 0;
                if(zValue==zStr+s32)
                {
                    /* Default section value. */
                	orxRegistry_LibC_SetString(zSection, (orxCONST orxSTRING)zValue+1);
                }
                else
                {
                    /* Classic name=value. */
                    *zValue = 0;
                    sprintf(zVar, "%s/%s", zSection, zStr);
                    orxRegistry_LibC_SetString(zVar, zValue+1);
                }
            }
        }
        orxFile_Close(pstFile);
    }
}

/**
 * Save registry node to file.
 * @param _pstNode (IN) Node to save
 * @param _zPrefix (IN) Prefix of node
 * @param _pstFile (IN) File where save the node.
 */
orxSTATIC orxVOID  orxRegistry_LibC_SaveNodeToFile(orxREGISTRY_NODE* _pstNode, orxCONST orxSTRING _zPrefix, orxFILE* _pstFile)
{
	orxREGISTRY_NODE* pstChild;
    if(_pstNode)
    {
        orxBOOL bSave = orxFALSE;
        if(_pstNode->zValue)
        	bSave = orxTRUE;
        else
        {
            pstChild = _pstNode->pstChild;
            while(pstChild)
            {
                if(pstChild->zValue && !pstChild->pstChild)
                {
                	bSave = orxTRUE;
                    break;
                }
                pstChild = pstChild->pstNext;
            }
        }
        orxCHAR zCateg[1024] = {0};
        orxCHAR zBuffer[4096] = {0};
        orxSTRING zName;
        if(_zPrefix)
        {
        	orxString_Print(zCateg, "%s/%s", _zPrefix, _pstNode->zName);
            zName = zCateg;
        }
        else
        	zName = _pstNode->zName;
        if(bSave)
        {
            orxString_Print(zBuffer, "\n[%s]\n", zName);
            orxFile_Write(zBuffer, orxString_GetLength(zBuffer), 1, _pstFile);
            
            if(_pstNode->zValue)
            {
                orxString_Print(zBuffer, "=%s\n", _pstNode->zValue);
                orxFile_Write(zBuffer, orxString_GetLength(zBuffer), 1, _pstFile);
            }
            pstChild = _pstNode->pstChild;
            while(pstChild)
            {
                if(pstChild->zValue && !pstChild->pstChild)
                {
                    orxString_Print(zBuffer, "%s=%s\n", pstChild->zName, pstChild->zValue);
                    orxFile_Write(zBuffer, orxString_GetLength(zBuffer), 1, _pstFile);
                }
                pstChild = pstChild->pstNext;
            }
        }
        pstChild = _pstNode->pstChild;
        while(pstChild)
        {
            if(pstChild->pstChild)
            	orxRegistry_LibC_SaveNodeToFile(pstChild, zName, _pstFile);
            pstChild = pstChild->pstNext;
        }

    }
}

/**
 * Save registry node to file.
 * @param _zFile (IN) File path where save the node.
 */
orxSTATIC orxVOID  orxRegistry_LibC_SaveToFile(orxCONST orxSTRING _zFile)
{
    orxFILE* pstFile = orxFile_Open(_zFile, orxFILE_KU32_FLAG_OPEN_WRITE);
    if(pstFile)
    {
    	orxREGISTRY_NODE* pstNode = &sstRegistry.stRootNode;
        while(pstNode)
        {
        	orxRegistry_LibC_SaveNodeToFile(pstNode, orxNULL, pstFile);
            pstNode = pstNode->pstNext;
        }
        orxFile_Close(pstFile);
    }
}



/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Initialize the Registry Module
 * @return Returns the status of the module initialization
 */
orxSTATUS orxRegistry_LibC_Init()
{
  /* Module not already initialized ? */
  orxASSERT(!(sstRegistry.u32Flags & orxREGISTRY_KU32_STATIC_FLAG_READY));

  /* Cleans static controller */
  orxMemory_Set(&sstRegistry, 0, sizeof(orxREGISTRY_STATIC));

  /** @todo Add orxBank and orxFile initialisation. */
  
  sstRegistry.pstBank = orxBank_Create(orxREGISTRY_KU32_BANK_SIZE, sizeof(orxREGISTRY_NODE), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
  
  if(sstRegistry.pstBank)
  {
	  /* Set module has ready */
	  sstRegistry.u32Flags = orxREGISTRY_KU32_STATIC_FLAG_READY;
  }

  /* Module successfully initialized ? */
  if(sstRegistry.u32Flags & orxREGISTRY_KU32_STATIC_FLAG_READY)
  {
    return orxSTATUS_SUCCESS;
  }
  else
  {
    return orxSTATUS_FAILURE;
  }
}

/** Uninitialize the File Module
 */
orxVOID orxRegistry_LibC_Exit()
{
  /* Module initialized ? */
  orxASSERT((sstRegistry.u32Flags & orxREGISTRY_KU32_STATIC_FLAG_READY) == orxREGISTRY_KU32_STATIC_FLAG_READY);
  
  /** Free all nodes. */
  orxRegistry_LibC_Clear();
  
  if(sstRegistry.pstBank)
  {
	  orxBank_Delete(sstRegistry.pstBank);
  }
  
  /* Module not ready now */
  sstRegistry.u32Flags = orxREGISTRY_KU32_STATIC_FLAG_NONE;
}


/** Read config registry from source.
 */
orxSTATUS orxRegistry_LibC_Load()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Done! */
  return eResult;
}

/** Write config registry to source.
 */
orxSTATUS orxRegistry_LibC_Save()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Done! */
  return eResult;
}

/** Read an integer value from registry.
 */
orxS32 orxRegistry_LibC_GetInt32(orxCONST orxSTRING _zKey, orxS32 _s32DefaultValue)
{
	orxREGISTRY_NODE* pstNode = orxRegistry_LibC_FindNode(_zKey, orxFALSE);
    if(pstNode && pstNode->zValue)
        return atoi(pstNode->zValue);
    return _s32DefaultValue;
}

/** Read a string value from registry.
 */
orxSTRING orxRegistry_LibC_GetString(orxCONST orxSTRING _zKey, orxCONST orxSTRING _zDefaultValue)
{
	orxREGISTRY_NODE* pstNode = orxRegistry_LibC_FindNode(_zKey, orxFALSE);
    if(pstNode && pstNode->zValue)
        return pstNode->zValue;
    return _zDefaultValue;
}

/** Read a boolean value from registry.
 */
orxBOOL orxRegistry_LibC_GetBool(orxCONST orxSTRING _zKey, orxBOOL _bDefaultValue)
{
	orxREGISTRY_NODE* pstNode = orxRegistry_LibC_FindNode(_zKey, orxFALSE);
    if(pstNode && pstNode->zValue)
        return *(pstNode->zValue)=='y' || *(pstNode->zValue)=='Y' ||
        		*(pstNode->zValue)=='o' || *(pstNode->zValue)=='O' ||
        		*(pstNode->zValue)=='1';
    return _bDefaultValue;
}

/** Write an integer value to registry.
 */
orxVOID orxRegistry_LibC_SetInt32(orxCONST orxSTRING _zKey, orxS32 _s32Value)
{
	orxREGISTRY_NODE* pstNode = orxRegistry_LibC_FindNode(_zKey, orxTRUE);
    if(pstNode)
    {
        orxCHAR zStr[32];
        orxString_Print(zStr, "%d", _s32Value);
        if(pstNode->zValue)
        {
        	orxU32 u32Size = orxString_GetLength(zStr);
        	pstNode->zValue = orxMemory_Reallocate(pstNode->zValue, u32Size+1);
        	orxString_Copy(pstNode->zValue, zStr);
        }
        else
        	pstNode->zValue = orxString_Duplicate(zStr);
    }
}

/** Write a string value to registry.
 */
orxVOID orxRegistry_LibC_SetString(orxCONST orxSTRING _zKey, orxCONST orxSTRING _zValue)
{
	orxREGISTRY_NODE* pstNode = orxRegistry_LibC_FindNode(_zKey, orxTRUE);
    if(pstNode)
    {
        if(pstNode->zValue)
        {
        	orxU32 u32Size = orxString_GetLength(_zValue);
        	pstNode->zValue = orxMemory_Reallocate(pstNode->zValue, u32Size+1);
        	orxString_Copy(pstNode->zValue, _zValue);
        }
        else
        	pstNode->zValue = orxString_Duplicate(_zValue);
    }
}

/** Write a boolean value from registry.
 */
orxVOID orxRegistry_LibC_SetBool(orxCONST orxSTRING _zKey, orxBOOL _bValue)
{
	orxREGISTRY_NODE* pstNode = orxRegistry_LibC_FindNode(_zKey, orxTRUE);
    if(pstNode)
    {
        if(pstNode->zValue)
        	pstNode->zValue = orxMemory_Reallocate(pstNode->zValue, 4);
        else
        	pstNode->zValue = orxMemory_Allocate(4, orxMEMORY_TYPE_MAIN);

    	if(_bValue)
    		orxString_Copy(pstNode->zValue, "yes");
    	else
    		orxString_Copy(pstNode->zValue, "no");
    }
}



/***************************************************************************
 * Plugin Related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(REGISTRY);

orxPLUGIN_USER_CORE_FUNCTION_ADD(orxRegistry_LibC_Init, REGISTRY, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxRegistry_LibC_Exit, REGISTRY, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxRegistry_LibC_Load, REGISTRY, LOAD);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxRegistry_LibC_Save, REGISTRY, SAVE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxRegistry_LibC_GetInt32, REGISTRY, GET_INT32)
//orxPLUGIN_USER_CORE_FUNCTION_ADD(orxRegistry_LibC_GetFloat, REGISTRY, GET_FLOAT)
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxRegistry_LibC_GetString, REGISTRY, GET_STRING)
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxRegistry_LibC_GetBool, REGISTRY, GET_BOOL)
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxRegistry_LibC_SetInt32, REGISTRY, SET_INT32)
//orxPLUGIN_USER_CORE_FUNCTION_ADD(orxRegistry_LibC_SetFloat, REGISTRY, SET_FLOAT)
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxRegistry_LibC_SetString, REGISTRY, SET_STRING)
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxRegistry_LibC_SetBool, REGISTRY, SET_BOOL)

orxPLUGIN_USER_CORE_FUNCTION_END();

