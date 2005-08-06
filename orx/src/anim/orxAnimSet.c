/***************************************************************************
 orxAnimSet.c
 Animation Set module
 
 begin                : 13/02/2004
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "anim/orxAnimSet.h"

#include "debug/orxDebug.h"
#include "memory/orxMemory.h"


/*
 * Platform independant defines
 */

/* Global Flags */
#define orxANIMSET_KU32_FLAG_NONE             0x00000000
#define orxANIMSET_KU32_FLAG_READY            0x00000001
#define orxANIMSET_KU32_FLAG_DEFAULT          0x00000000


/* Animset ID flags */
#define orxANIMSET_KU32_ID_FLAG_NONE          0x00000000

#define orxANIMSET_KU32_ID_MASK_SIZE          0x000000FF
#define orxANIMSET_KU32_ID_MASK_COUNTER       0x0000FF00
#define orxANIMSET_KU32_ID_MASK_FLAGS         0xFFFF0000

#define orxANIMSET_KU32_ID_SHIFT_SIZE         0
#define orxANIMSET_KU32_ID_SHIFT_COUNTER      8

/* Link table link flags */
#define orxANIMSET_KU32_LINK_FLAG_NONE        0x00000000

#define orxANIMSET_KU32_LINK_FLAG_PATH        0x01000000
#define orxANIMSET_KU32_LINK_FLAG_LINK        0x02000000

#define orxANIMSET_KU32_LINK_MASK_ANIM        0x000000FF
#define orxANIMSET_KU32_LINK_MASK_LENGTH      0x0000FF00
#define orxANIMSET_KU32_LINK_MASK_PRIORITY    0x000F0000
#define orxANIMSET_KU32_LINK_MASK_FLAGS       0xFF000000

#define orxANIMSET_KU32_LINK_SHIFT_ANIM       0
#define orxANIMSET_KU32_LINK_SHIFT_LENGTH     8
#define orxANIMSET_KU32_LINK_SHIFT_PRIORITY   16

#define orxANIMSET_KU32_LINK_DEFAULT_NONE     0x00000000

#define orxANIMSET_KU32_LINK_DEFAULT_PRIORITY 0x00000008

/* Link table (status) flags */
#define orxANIMSET_KU32_LINK_TABLE_FLAG_NONE  0x00000000

#define orxANIMSET_KU32_LINK_TABLE_FLAG_READY 0x01000000
#define orxANIMSET_KU32_LINK_TABLE_FLAG_DIRTY 0x02000000

#define orxANIMSET_KU32_LINK_TABLE_MASK_FLAGS 0xFFFF0000


/*
 * Internal Link Update Info structure
 */
typedef struct __orxLINK_UPDATE_INFO_t
{
  /* Link Table : 4 */
  orxANIM_SET_LINK_TABLE *pstLinkTable;

  /* Link update info : 8 */
  orxU8 *au8LinkInfo;

  /* Byte number per animation : 12 */
  orxU32 u32ByteNumber;

  /* 4 extra bytes of padding : 16 */
  orxU8 au8Unused[4];

} orxLINK_UPDATE_INFO;

/*
 * Internal Animation Set Link table structure
 */
struct __orxANIM_SET_LINK_TABLE_t
{
  /* Link array : 4 */
  orxU32 *au32LinkArray;

  /* Loop array : 8 */
  orxU8 *au8LoopArray;

  /* Link Counter : 10 */
  orxU16 u16LinkCounter;

  /* Table size : 12 */
  orxU16 u16TableSize;

  /* Flags : 16 */
  orxU32 u32Flags;
};

/*
 * Animation Set structure
 */
struct __orxANIM_SET_t
{
  /* Public structure, first structure member : 16 */
  orxSTRUCTURE stStructure;

  /* Id flags : 20 */
  orxU32 u32IDFlags;

  /* Used Animation pointer array : 24 */
  orxANIM **pastAnim;

  /* Link table pointer : 28 */
  orxANIM_SET_LINK_TABLE *pstLinkTable;

  /* 4 extra bytes of padding : 32 */
  orxU8 au8Unused[4];
};

/*
 * Static structure
 */
typedef struct __orxANIM_SET_STATIC_t
{

  /* Control flags */
  orxU32 u32Flags;

} orxANIM_SET_STATIC;


/*
 * Static data
 */
orxSTATIC orxANIM_SET_STATIC sstAnimSet;


/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 orxAnimSet_TestLinkTableFlag
 Link table set flag test accessor.

 returns: orxBOOL
 ***************************************************************************/
orxBOOL orxAnimSet_TestLinkTableFlag(orxANIM_SET_LINK_TABLE *_pstLinkTable, orxU32 _u32Flag)
{
  /* Checks */
  orxASSERT(_pstLinkTable != orxNULL);

  return((_pstLinkTable->u32Flags & _u32Flag) == _u32Flag);
}

/***************************************************************************
 orxAnimSet_SetLinkTableFlag
 Link table set flag get/set accessor.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxAnimSet_SetLinkTableFlag(orxANIM_SET_LINK_TABLE *_pstLinkTable, orxU32 _u32AddFlags, orxU32 _u32RemoveFlags)
{
  /* Checks */
  orxASSERT(_pstLinkTable != orxNULL);

  _pstLinkTable->u32Flags &= ~_u32RemoveFlags;
  _pstLinkTable->u32Flags |= _u32AddFlags;

  return;
}

/***************************************************************************
 orxAnimSet_GetLinkTableLink
 Gets a link value from link table.

 returns: orxU32 link value
 ***************************************************************************/
orxU32 orxAnimSet_GetLinkTableLink(orxANIM_SET_LINK_TABLE *_pstLinkTable, orxU32 _u32Index)
{
  /* Checks */
  orxASSERT(_pstLinkTable != orxNULL);
  orxASSERT(_u32Index < (orxU32)(_pstLinkTable->u16TableSize) * (orxU32)(_pstLinkTable->u16TableSize));

  return _pstLinkTable->au32LinkArray[_u32Index];
}

/***************************************************************************
 orxAnimSet_SetLinkTableLink
 Sets a link value in a link table.

 returns: orxSTATUS_SUCCESS / orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxAnimSet_SetLinkTableLink(orxANIM_SET_LINK_TABLE *_pstLinkTable, orxU32 _u32Index, orxU32 _u32LinkValue)
{
  /* Checks */
  orxASSERT(_pstLinkTable != orxNULL);
  orxASSERT(_u32Index < (orxU32)(_pstLinkTable->u16TableSize) * (orxU32)(_pstLinkTable->u16TableSize));

  /* Sets link */
  _pstLinkTable->au32LinkArray[_u32Index] = _u32LinkValue;

  return orxSTATUS_SUCCESS;
}

/***************************************************************************
 orxAnimSet_SetLinkTableLinkProperty
 Sets a link property at the given value.

 returns: orxSTATUS_SUCCESS / orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxAnimSet_SetLinkTableLinkProperty(orxANIM_SET_LINK_TABLE *_pstLinkTable, orxU32 _u32LinkIndex, orxU32 _u32Property, orxU32 _u32Value)
{
  /* Checks */
  orxASSERT(_pstLinkTable != orxNULL);
  orxASSERT(_u32LinkIndex < (orxU32)(_pstLinkTable->u16LinkCounter * _pstLinkTable->u16LinkCounter));

  /* Has link? */
  if(_pstLinkTable->au32LinkArray[_u32LinkIndex] & orxANIMSET_KU32_LINK_FLAG_LINK)
  {
    /* Depends on property */
    switch(_u32Property & orxANIMSET_KU32_LINK_MASK_FLAGS)
    {
      /* Loop Counter */
      case orxANIMSET_KU32_LINK_FLAG_LOOP_COUNTER:

        /* Sets loop counter value */
        _pstLinkTable->au8LoopArray[_u32LinkIndex] = (orxU8)_u32Value;
        
        /* Updates flags */
        if(_u32Value != 0)
        {
          _pstLinkTable->au32LinkArray[_u32LinkIndex] |= orxANIMSET_KU32_LINK_FLAG_LOOP_COUNTER;
        }
        else
        {
          _pstLinkTable->au32LinkArray[_u32LinkIndex] &= ~orxANIMSET_KU32_LINK_FLAG_LOOP_COUNTER;
        }
        
        break;
  
      /* Priority */
      case orxANIMSET_KU32_LINK_FLAG_PRIORITY:
      
        /* Updates priority */
        if(_u32Value != 0)
        {
          _pstLinkTable->au32LinkArray[_u32LinkIndex] |= orxANIMSET_KU32_LINK_FLAG_PRIORITY
                                                   + ((_u32Value << orxANIMSET_KU32_LINK_SHIFT_PRIORITY) & orxANIMSET_KU32_LINK_MASK_PRIORITY);
        }
        else
        {
          _pstLinkTable->au32LinkArray[_u32LinkIndex] &= ~(orxANIMSET_KU32_LINK_FLAG_PRIORITY | orxANIMSET_KU32_LINK_MASK_PRIORITY);
        }

        break;
  
      /* Invalid */
      default :

        /* !!! MSG !!! */
  
        return orxSTATUS_FAILED;
    }
  
    /* Animset has to be computed again */
    orxAnimSet_SetLinkTableFlag(_pstLinkTable, orxANIMSET_KU32_LINK_TABLE_FLAG_DIRTY, orxANIMSET_KU32_LINK_TABLE_FLAG_NONE);
  }
  else
  {
    /* !!! MSG !!! */

    return orxSTATUS_FAILED;
  }

  /* Done */
  return orxSTATUS_SUCCESS;
}

/***************************************************************************
 orxAnimSet_GetLinkTableLinkProperty
 Gets a link property.

 returns: orxU32 Property value / orxU32_Undefined
 ***************************************************************************/
orxU32 orxAnimSet_GetLinkTableLinkProperty(orxANIM_SET_LINK_TABLE *_pstLinkTable, orxU32 _u32LinkIndex, orxU32 _u32Property)
{
  orxU32 u32Value;

  /* Checks */
  orxASSERT(_pstLinkTable != orxNULL);
  orxASSERT(_u32LinkIndex < (orxU32)(_pstLinkTable->u16TableSize * _pstLinkTable->u16TableSize));

  /* Has link? */
  if(_pstLinkTable->au32LinkArray[_u32LinkIndex] & orxANIMSET_KU32_LINK_FLAG_LINK)
  {
    /* Depends on property */
    switch(_u32Property & orxANIMSET_KU32_LINK_MASK_FLAGS)
    {
      /* Loop Counter */
      case orxANIMSET_KU32_LINK_FLAG_LOOP_COUNTER:
      
        /* Has a counter? */
        if(_pstLinkTable->au32LinkArray[_u32LinkIndex] & orxANIMSET_KU32_LINK_FLAG_LOOP_COUNTER)
        {
          u32Value = (orxU32)(_pstLinkTable->au8LoopArray[_u32LinkIndex]);
        }
        else
        {
          /* !!! MSG !!! */
  
          u32Value = orxU32_Undefined;
        }
        
        break;
  
      /* Priority */
      case orxANIMSET_KU32_LINK_FLAG_PRIORITY:
      
      /* Has priority? */
      if(_pstLinkTable->au32LinkArray[_u32LinkIndex] & orxANIMSET_KU32_LINK_FLAG_PRIORITY)
      {
        u32Value = (_pstLinkTable->au32LinkArray[_u32LinkIndex] & orxANIMSET_KU32_LINK_MASK_PRIORITY) >> orxANIMSET_KU32_LINK_SHIFT_PRIORITY;
      }
      else
      {
        /* !!! MSG !!! */
  
        u32Value = (orxU32)orxANIMSET_KU32_LINK_DEFAULT_PRIORITY;
      }
      
        break;
  
      /* Invalid */
      default :
      
        /* !!! MSG !!! */
  
        /* Undefined result */
        u32Value = orxU32_Undefined;
        break;
    }
  }
  else
  {
    /* !!! MSG !!! */

    /* Undefined result */
    u32Value = orxU32_Undefined;
  }

  /* Done */
  return u32Value;
}

/***************************************************************************
 orxAnimSet_SetLinkInfo
 Sets a link info.

 returns: Nothing
 ***************************************************************************/
orxINLINE orxVOID orxAnimSet_SetLinkInfo(orxLINK_UPDATE_INFO *_pstInfo, orxU32 _u32SrcIndex, orxU32 _u32DstIndex)
{
  orxREGISTER orxU32 u32Index;
  orxREGISTER orxU8 u8Mask;

  /* Checks */
  orxASSERT(_pstInfo != orxNULL);

  /* Computes real index */
  u32Index = (_u32SrcIndex * _pstInfo->u32ByteNumber) + (_u32DstIndex >> 3);

  /* Computes mask */
  u8Mask = 0x01 << (0x07 - (_u32DstIndex & 0x00000007));

  /* Sets info */
  _pstInfo->au8LinkInfo[u32Index] |= u8Mask;

  return;
}

/***************************************************************************
 orxAnimSet_ResetLinkInfo
 Resets a link info.

 returns: Nothing
 ***************************************************************************/
orxINLINE orxVOID orxAnimSet_ResetLinkInfo(orxLINK_UPDATE_INFO *_pstInfo, orxU32 _u32SrcIndex, orxU32 _u32DstIndex)
{
  orxREGISTER orxU32 u32Index;
  orxREGISTER orxU8 u8Mask;

  /* Checks */
  orxASSERT(_pstInfo != orxNULL);
  orxASSERT(_u32SrcIndex < (orxU32)(_pstInfo->pstLinkTable->u16TableSize));
  orxASSERT(_u32DstIndex < (orxU32)(_pstInfo->pstLinkTable->u16TableSize));

  /* Computes real index */
  u32Index = (_u32SrcIndex * _pstInfo->u32ByteNumber) + (_u32DstIndex >> 3);

  /* Computes mask */
  u8Mask = 0x01 << (0x07 - (_u32DstIndex & 0x00000007));

  /* Resets info */
  _pstInfo->au8LinkInfo[u32Index] &= ~u8Mask;

  return;
}

/***************************************************************************
 orxAnimSet_CleanLinkInfo
 Cleans a link info.

 returns: Nothing
 ***************************************************************************/
orxINLINE orxVOID orxAnimSet_CleanLinkInfo(orxLINK_UPDATE_INFO *_pstInfo, orxU32 _u32SrcIndex)
{
  orxU32 u32Index, i;

  /* Checks */
  orxASSERT(_pstInfo != orxNULL);
  orxASSERT(_u32SrcIndex < (orxU32)(_pstInfo->pstLinkTable->u16TableSize));

  /* Computes real index */
  u32Index = _u32SrcIndex * _pstInfo->u32ByteNumber;

  /* Cleans info */
  for(i = u32Index; i < u32Index + _pstInfo->u32ByteNumber; i++)
  {
    _pstInfo->au8LinkInfo[i] = 0x00;
  }

  return;
}

/***************************************************************************
 orxAnimSet_GetLinkInfo
 Gets a link info.

 returns: orxTRUE on success / orxFALSE on failure
 ***************************************************************************/
orxINLINE orxBOOL orxAnimSet_GetLinkInfo(orxLINK_UPDATE_INFO *_pstInfo, orxU32 _u32SrcIndex, orxU32 _u32DstIndex)
{
  orxREGISTER orxU32 u32Index;
  orxREGISTER orxU8 u8Mask;

  /* Checks */
  orxASSERT(_pstInfo != orxNULL);
  orxASSERT(_u32SrcIndex < (orxU32)(_pstInfo->pstLinkTable->u16TableSize));
  orxASSERT(_u32DstIndex < (orxU32)(_pstInfo->pstLinkTable->u16TableSize));

  /* Computes real index */
  u32Index = (_u32SrcIndex * _pstInfo->u32ByteNumber) + (_u32DstIndex >> 3);

  /* Computes mask */
  u8Mask = 0x01 << (0x07 - (_u32DstIndex & 0x00000007));

  /* Returns info */
  return((_pstInfo->au8LinkInfo[u32Index] & u8Mask) ? orxTRUE : orxFALSE);
}

/***************************************************************************
 orxAnimSet_UpdateLinkInfo
 Updates links information from one animation to another.

 returns: orxTRUE if there are changes / orxFALSE otherwise
 ***************************************************************************/
orxSTATIC orxBOOL orxAnimSet_UpdateLinkInfo(orxLINK_UPDATE_INFO *_pstInfo, orxU32 _u32SrcIndex, orxU32 _u32DstIndex)
{
  orxBOOL bChange = orxFALSE;
  orxU32 u32SrcLink, u32DstLink, u32SrcLength, u32DstLength;
  orxU32 u32DirectLink, u32DirectPriority, u32Priority;
  orxU32 u32SrcBaseIndex, u32DstBaseIndex, i;
  orxANIM_SET_LINK_TABLE *pstLinkTable;

  /* Checks */
  orxASSERT(_pstInfo != orxNULL);
  orxASSERT(_u32SrcIndex < (orxU32)(_pstInfo->pstLinkTable->u16TableSize));
  orxASSERT(_u32DstIndex < (orxU32)(_pstInfo->pstLinkTable->u16TableSize));

  /* Gets link table */
  pstLinkTable = _pstInfo->pstLinkTable;

  /* Computes base indexes */
  u32SrcBaseIndex = _u32SrcIndex * (orxU32)(pstLinkTable->u16TableSize);
  u32DstBaseIndex = _u32DstIndex * (orxU32)(pstLinkTable->u16TableSize);

  /* Gets direct link */
  u32DirectLink = pstLinkTable->au32LinkArray[u32DstBaseIndex + _u32SrcIndex];

  /* Checks link */
  orxASSERT(u32DirectLink & orxANIMSET_KU32_LINK_FLAG_LINK);

  /* Gets direct link priority */
  if(u32DirectLink & orxANIMSET_KU32_LINK_FLAG_PRIORITY)
  {
    u32DirectPriority = (u32DirectLink & orxANIMSET_KU32_LINK_MASK_PRIORITY) >> orxANIMSET_KU32_LINK_SHIFT_PRIORITY;
  }
  else
  {
    u32DirectPriority = (orxU32)orxANIMSET_KU32_LINK_DEFAULT_PRIORITY;
  }

  /* Compares all paths */
  for(i = 0; i < (orxU32)(pstLinkTable->u16LinkCounter); i++)
  {
    /* Gets both values */
    u32SrcLink = pstLinkTable->au32LinkArray[u32SrcBaseIndex + i];
    u32DstLink = pstLinkTable->au32LinkArray[u32DstBaseIndex + i];

    /* Is not a direct link? */
    if((u32DstLink & orxANIMSET_KU32_LINK_FLAG_LINK) == orxANIMSET_KU32_LINK_FLAG_NONE)
    {
      /* New path found? */
      if(u32SrcLink & orxANIMSET_KU32_LINK_FLAG_PATH)
      {
        /* Gets both lengths */
        u32SrcLength = (u32SrcLink & orxANIMSET_KU32_LINK_MASK_LENGTH) >> orxANIMSET_KU32_LINK_SHIFT_LENGTH;
        u32DstLength = (u32DstLink & orxANIMSET_KU32_LINK_MASK_LENGTH) >> orxANIMSET_KU32_LINK_SHIFT_LENGTH;

        /* Has already a path? */
        if(u32DstLink & orxANIMSET_KU32_LINK_FLAG_PATH)
        {
          /* Computes old path priority */
          if(u32DstLink & orxANIMSET_KU32_LINK_FLAG_PRIORITY)
          {
            u32Priority = (u32DstLink & orxANIMSET_KU32_LINK_MASK_PRIORITY) >> orxANIMSET_KU32_LINK_SHIFT_PRIORITY;
          }
          else
          {
            u32Priority = (orxU32)orxANIMSET_KU32_LINK_DEFAULT_PRIORITY;
          }

          /* Lowest priority found? */
          if(u32Priority > u32DirectPriority)
          {
            /* Next path comparison */
            continue;
          }
          /* Same priority */
          else if(u32Priority == u32DirectPriority)
          {
            /* No shortest path found? */
            if(u32DstLength <= u32SrcLength + 1)
            {
              /* Next path comparison */
              continue;
            }
          }
        }

        /* New found path has higher priority or shortest length */

        /* Computes new path */
        u32DstLink = orxANIMSET_KU32_LINK_FLAG_PATH
          + ((u32SrcLength + 1) << orxANIMSET_KU32_LINK_SHIFT_LENGTH)
          + (_u32SrcIndex << orxANIMSET_KU32_LINK_SHIFT_ANIM);

        /* Stores it */
        pstLinkTable->au32LinkArray[u32DstBaseIndex + i] = u32DstLink;

        /* Changes have been made */
        bChange = orxTRUE;
      }
    }
  }

  return bChange;
}

/***************************************************************************
 orxAnimSet_UpdateLink
 Updates link for the given animation.

 returns: Nothing
 ***************************************************************************/
orxVOID orxAnimSet_UpdateLink(orxU32 _u32Index, orxLINK_UPDATE_INFO *_pstInfo)
{
  orxU32 u32BaseIndex;
  orxANIM_SET_LINK_TABLE *pstLinkTable;
  orxU32 i;

  /* Checks */
  orxASSERT(_pstInfo != orxNULL);
  orxASSERT(_u32Index < (orxU32)(_pstInfo->pstLinkTable->u16TableSize));

  /* Are links already computed/updated for this animations */
  if(orxAnimSet_GetLinkInfo(_pstInfo, _u32Index, _u32Index) != orxFALSE)
  {
    return;
  }

  /* Gets link table */
  pstLinkTable = _pstInfo->pstLinkTable;

  /* Mark animation as read */
  orxAnimSet_SetLinkInfo(_pstInfo, _u32Index, _u32Index);

  /* Computes base index */
  u32BaseIndex = _u32Index * (orxU32)(pstLinkTable->u16TableSize);

  /* For all found links */
  for(i = 0; i < (orxU32)(pstLinkTable->u16LinkCounter); i++)
  {
    /* Link found? */
    if(pstLinkTable->au32LinkArray[u32BaseIndex + i] & orxANIMSET_KU32_LINK_FLAG_LINK)
    {
      /* Is animation 'dirty' for this one? */
      if(orxAnimSet_GetLinkInfo(_pstInfo, i, _u32Index) == orxFALSE)
      {
        /* Updates this animation */
        orxAnimSet_UpdateLink(i, _pstInfo);

        /* Updates 'dirty' status */
        orxAnimSet_SetLinkInfo(_pstInfo, i, _u32Index);

        /* Updates info */
        if(orxAnimSet_UpdateLinkInfo(_pstInfo, i, _u32Index) != orxFALSE)
        {
          /* Links have been modified for current animation */
          orxAnimSet_CleanLinkInfo(_pstInfo, _u32Index);
          orxAnimSet_SetLinkInfo(_pstInfo, _u32Index, _u32Index);
        }
      }
    }
  }

  /* Unmark animation as read */
  orxAnimSet_ResetLinkInfo(_pstInfo, _u32Index, _u32Index);

  return;
}

/***************************************************************************
 orxAnimSet_CreateLinkUpdateInfo
 Creates and init the Link Update Info.

 returns: orxLINK_UPDATE_INFO pointer / orxNULL on failure
 ***************************************************************************/
orxSTATIC orxLINK_UPDATE_INFO *orxAnimSet_CreateLinkUpdateInfo(orxANIM_SET_LINK_TABLE *_pstLinkTable)
{
  orxLINK_UPDATE_INFO *pstInfo = orxNULL;
  orxU32 u32Number;

  /* Checks */
  orxASSERT(_pstLinkTable != orxNULL);

  /* Allocates memory */
  pstInfo = (orxLINK_UPDATE_INFO *)orxMemory_Allocate(sizeof(orxLINK_UPDATE_INFO), orxMEMORY_TYPE_MAIN);

  /* Was allocated? */
  if(pstInfo != orxNULL)
  {
    /* Cleans it */
    orxMemory_Set(pstInfo, 0, sizeof(orxLINK_UPDATE_INFO));
    
    /* Stores link table */
    pstInfo->pstLinkTable = _pstLinkTable;
  
    /* Computes number of orxU8 needed for one link info : ((size - 1) / 8) + 1 */
    u32Number = (orxU32)(((_pstLinkTable->u16LinkCounter - 1) >> 3) + 1);
  
    /* Stores it */
    pstInfo->u32ByteNumber = u32Number;
  
    /* Allocates link info array */
    pstInfo->au8LinkInfo = (orxU8 *)orxMemory_Allocate(u32Number * (orxU32)(_pstLinkTable->u16LinkCounter) * sizeof(orxU8), orxMEMORY_TYPE_MAIN);
  
    /* Was allocated? */
    if(pstInfo->au8LinkInfo == orxNULL)
    {
      /* Cleans it */
      orxMemory_Set(pstInfo->au8LinkInfo, 0, (orxU32)(_pstLinkTable->u16LinkCounter) * u32Number * sizeof(orxU8));
    }
    else
    {
      /* !!! MSG !!! */

      /* Frees previously allocated memory */
      orxMemory_Free(pstInfo);

      /* Not created */
      pstInfo = orxNULL;
    }
  }
  else
  {
    /* !!! MSG !!! */

    /* Not created */
    pstInfo = orxNULL;
  }

  /* Done! */
  return pstInfo;
}

/***************************************************************************
 orxAnimSet_DeleteLinkUpdateInfo
 Deletes the Link Update Info.

 returns: Nothing
 ***************************************************************************/
orxINLINE orxVOID orxAnimSet_DeleteLinkUpdateInfo(orxLINK_UPDATE_INFO *_pstLinkUpdateInfo)
{
  /* Checks */
  orxASSERT(_pstLinkUpdateInfo != orxNULL);

  /* Frees all */
  orxMemory_Free(_pstLinkUpdateInfo->au8LinkInfo);
  orxMemory_Free(_pstLinkUpdateInfo);

  return;
}

/***************************************************************************
 orxAnimSet_CleanLinkTable
 Cleans a Link Table for a given animation.

 returns: orxVOID
 ***************************************************************************/
orxSTATIC orxVOID orxAnimSet_CleanLinkTable(orxANIM_SET_LINK_TABLE *_pstLinkTable, orxU32 _u32AnimIndex)
{
  orxU32 u32Size, u32AnimBaseIndex, i;

  /* Checks */
  orxASSERT(_pstLinkTable != orxNULL);

  /* Gets storage size */
  u32Size = (orxU32)(_pstLinkTable->u16TableSize);

  /* Checks anim index */
  orxASSERT(_u32AnimIndex < u32Size);

  /* Gets base index */
  u32AnimBaseIndex = _u32AnimIndex * u32Size;

  /* Cleans paths coming from this animation */
  for(i = u32AnimBaseIndex; i < u32AnimBaseIndex + u32Size; i++)
  {
    _pstLinkTable->au32LinkArray[i] = orxANIMSET_KU32_LINK_DEFAULT_NONE;
    _pstLinkTable->au8LoopArray[i] = 0;
  }

  /* Cleans paths going to this animation */
  for(i = _u32AnimIndex; i < u32Size; i += u32Size)
  {
    _pstLinkTable->au32LinkArray[i] = orxANIMSET_KU32_LINK_DEFAULT_NONE;
    _pstLinkTable->au8LoopArray[i] = 0;
  }

  /* Updates flags */
  orxAnimSet_SetLinkTableFlag(_pstLinkTable, orxANIMSET_KU32_LINK_TABLE_FLAG_DIRTY, orxANIMSET_KU32_LINK_TABLE_FLAG_NONE);

  return;
}

/***************************************************************************
 orxAnimSet_CreateLinkTable
 Creates and init the Link Table.

 returns: orxANIM_SET_LINK_TABLE *
 ***************************************************************************/
orxSTATIC orxANIM_SET_LINK_TABLE *orxAnimSet_CreateLinkTable(orxU32 _u32Size)
{
  orxANIM_SET_LINK_TABLE *pstLinkTable = orxNULL;

  /* Checks */
  orxASSERT(_u32Size < 0xFFFF);

  /* Allocates link table */
  pstLinkTable = (orxANIM_SET_LINK_TABLE *)orxMemory_Allocate(sizeof(orxANIM_SET_LINK_TABLE), orxMEMORY_TYPE_MAIN);

  /* Was allocated? */
  if(pstLinkTable != orxNULL)
  {
    /* Cleans it */
    orxMemory_Set(pstLinkTable, 0, sizeof(orxANIM_SET_LINK_TABLE));
    
    /* Creates link array */
    pstLinkTable->au32LinkArray = (orxU32 *)orxMemory_Allocate(_u32Size * _u32Size * sizeof(orxU32), orxMEMORY_TYPE_MAIN);
  
    /* Was allocated? */
    if(pstLinkTable->au32LinkArray != orxNULL)
    {
      /* Allocates loop table */
      pstLinkTable->au8LoopArray = (orxU8 *)orxMemory_Allocate(_u32Size * _u32Size * sizeof(orxU8), orxMEMORY_TYPE_MAIN);
    
      /* Was allocated? */
      if(pstLinkTable->au8LoopArray != orxNULL)
      {
        /* Cleans tables */
        orxMemory_Set(pstLinkTable->au32LinkArray, 0, _u32Size * _u32Size * sizeof(orxU32));
        orxMemory_Set(pstLinkTable->au8LoopArray, 0, _u32Size * _u32Size * sizeof(orxU8));

        /* Inits values */
        pstLinkTable->u16TableSize = (orxU16)_u32Size;
        pstLinkTable->u16LinkCounter = 0;
      
        /* Inits flags */
        orxAnimSet_SetLinkTableFlag(pstLinkTable, orxANIMSET_KU32_LINK_TABLE_FLAG_READY | orxANIMSET_KU32_LINK_TABLE_FLAG_DIRTY, orxANIMSET_KU32_LINK_TABLE_MASK_FLAGS);
      }
      else
      {
        /* !!! MSG !!! */
    
        /* Frees previously allocated memory */
        orxMemory_Free(pstLinkTable->au32LinkArray);
        orxMemory_Free(pstLinkTable);

        /* Not created */
        pstLinkTable = orxNULL;
      }
    }
    else
    {
      /* !!! MSG !!! */
  
      /* Frees link table */
      orxMemory_Free(pstLinkTable);

      /* Not created */
      pstLinkTable = orxNULL;
    }
  }
  else
  {
    /* !!! MSG !!! */

    /* Not created */
    pstLinkTable = orxNULL;
  }

  /* Done! */
  return pstLinkTable;
}

/***************************************************************************
 orxAnimSet_CopyLinkTable
 Copies an Animset link table info into another one.

 returns: Nothing
 ***************************************************************************/
orxINLINE orxVOID orxAnimSet_CopyLinkTable(orxANIM_SET_LINK_TABLE *_pstSrcLinkTable, orxANIM_SET_LINK_TABLE *_pstDstLinkTable)
{
  /* Checks */
  orxASSERT(_pstSrcLinkTable != orxNULL);
  orxASSERT(_pstDstLinkTable != orxNULL);

  /* Copies all */
  _pstDstLinkTable->u32Flags       = _pstSrcLinkTable->u32Flags;
  _pstDstLinkTable->u16LinkCounter = _pstSrcLinkTable->u16LinkCounter;
  _pstDstLinkTable->u16TableSize   = _pstSrcLinkTable->u16TableSize;

  /* Copies tables */
  orxMemory_Copy(_pstDstLinkTable->au32LinkArray, _pstSrcLinkTable->au32LinkArray, (orxU32)(_pstSrcLinkTable->u16TableSize) * (orxU32)(_pstSrcLinkTable->u16TableSize) * sizeof(orxU32));
  orxMemory_Copy(_pstDstLinkTable->au8LoopArray, _pstSrcLinkTable->au8LoopArray, (orxU32)(_pstSrcLinkTable->u16TableSize) * (orxU32)(_pstSrcLinkTable->u16TableSize) * sizeof(orxU8));

  return;
}

/***************************************************************************
 orxAnimSet_SetAnimStorageSize
 Sets an animation set anim storage size.

 returns: orxVOID
 ***************************************************************************/
orxINLINE orxVOID orxAnimSet_SetAnimStorageSize(orxANIM_SET *_pstAnimset, orxU32 _u32Size)
{
  /* Checks */
  orxASSERT(_pstAnimset != orxNULL);
  orxASSERT(_u32Size <= orxANIMSET_KU32_MAX_ANIM_NUMBER);

  /* Updates storage size */
  _pstAnimset->u32IDFlags &= ~orxANIMSET_KU32_ID_MASK_SIZE;
  _pstAnimset->u32IDFlags |= _u32Size << orxANIMSET_KU32_ID_SHIFT_SIZE;

  return;
}  

/***************************************************************************
 orxAnimSet_SetAnimCounter
 Sets an animation set internal anim counter.

 returns: orxVOID
 ***************************************************************************/
orxINLINE orxVOID orxAnimSet_SetAnimCounter(orxANIM_SET *_pstAnimset, orxU32 _u32AnimCounter)
{
  /* Checks */
  orxASSERT(_u32AnimCounter <= orxAnimSet_GetAnimStorageSize(_pstAnimset));

  /* Updates counter */
  _pstAnimset->u32IDFlags &= ~orxANIMSET_KU32_ID_MASK_COUNTER;
  _pstAnimset->u32IDFlags |= _u32AnimCounter << orxANIMSET_KU32_ID_SHIFT_COUNTER;

  return;
}

/***************************************************************************
 orxAnimSet_IncreaseAnimCounter
 Increases an animation set internal anim counter.

 returns: orxVOID
 ***************************************************************************/
orxINLINE orxVOID orxAnimSet_IncreaseAnimCounter(orxANIM_SET *_pstAnimset)
{
  orxREGISTER orxU32 u32AnimCounter;

  /* Checks */
  orxASSERT(_pstAnimset != orxNULL);

  /* Gets anim counter */
  u32AnimCounter = orxAnimSet_GetAnimCounter(_pstAnimset);

  /* Updates anim counter*/
  orxAnimSet_SetAnimCounter(_pstAnimset, u32AnimCounter + 1);

  return;
}  

/***************************************************************************
 orxAnimSet_DecreaseAnimCounter
 Decreases an animation set internal anim counter.

 returns: orxVOID
 ***************************************************************************/
orxINLINE orxVOID orxAnimSet_DecreaseAnimCounter(orxANIM_SET *_pstAnimset)
{
  orxREGISTER orxU32 u32AnimCounter;

  /* Checks */
  orxASSERT(_pstAnimset != orxNULL);

  /* Gets anim counter */
  u32AnimCounter = orxAnimSet_GetAnimCounter(_pstAnimset);

  /* Updates anim counter*/
  orxAnimSet_SetAnimCounter(_pstAnimset, u32AnimCounter - 1);

  return;
}  

/***************************************************************************
 orxAnimSet_DeleteAll
 Deletes all Animation Sets.

 returns: orxVOID
 ***************************************************************************/
orxSTATIC orxVOID orxAnimSet_DeleteAll()
{
  orxANIM_SET *pstAnimset;
  
  /* Gets first anim set */
  pstAnimset = (orxANIM_SET *)orxStructure_GetFirst(orxSTRUCTURE_ID_ANIM_SET);

  /* Non empty? */
  while(pstAnimset != orxNULL)
  {
    /* Deletes Animation Set */
    orxAnimSet_Delete(pstAnimset);

    /* Gets first Animation Set */
    pstAnimset = (orxANIM_SET *)orxStructure_GetFirst(orxSTRUCTURE_ID_ANIM_SET);
  }

  return;
}

/***************************************************************************
 orxAnimSet_ComputeNextAnim
 Gets next animation, updating link status.

 returns: orxU32 animation index / orxU32_Undefined if none found
 ***************************************************************************/
orxSTATIC orxU32 orxAnimSet_ComputeNextAnim(orxANIM_SET_LINK_TABLE *_pstLinkTable, orxU32 _u32AnimIndex)
{
  orxU32 u32Size, u32BaseIndex;
  orxU32 u32LinkPriority, u32ResLinkPriority, u32Loop;
  orxU32 u32ResAnim = orxU32_Undefined, u32ResLink, u32Link;
  orxU32 i;

  /* Checks */
  orxASSERT(_pstLinkTable != orxNULL);

  /* Gets animation storage size */
  u32Size             = (orxU32)(_pstLinkTable->u16TableSize);

  /* Gets animation base index */
  u32BaseIndex        = _u32AnimIndex * u32Size;

  /* Inits anim & link value */
  u32ResAnim          = orxU32_Undefined;
  u32ResLink          = orxANIMSET_KU32_LINK_DEFAULT_NONE;
  u32ResLinkPriority  = 0;

  /* Search for all links */
  for(i = u32BaseIndex; i < u32BaseIndex + u32Size; i++)
  {
    /* Gets value */
    u32Link = _pstLinkTable->au32LinkArray[i];

    /* Link found? */
    if(u32Link & orxANIMSET_KU32_LINK_FLAG_LINK)
    {
      /* Gets link loop counter */
      u32Loop = orxAnimSet_GetLinkTableLinkProperty(_pstLinkTable, i, orxANIMSET_KU32_LINK_FLAG_LOOP_COUNTER);

      /* Has an empty loop counter (if no loop, value is orxU32_Undefined)? */
      if(u32Loop == 0)
      {
        /* Checks next link */
        continue;
      }

      /* Gets path priority */
      u32LinkPriority = orxAnimSet_GetLinkTableLinkProperty(_pstLinkTable, i, orxANIMSET_KU32_LINK_FLAG_PRIORITY);

      /* Is priority lower or equal than previous one? */
      if(u32LinkPriority <= u32ResLinkPriority)
      {
        /* Checks next link */
        continue;
      }

      /* Stores new link info */
      u32ResAnim          = i;
      u32ResLink          = u32Link;
      u32ResLinkPriority  = u32LinkPriority;
    }
  }

  /* Link found? */
  if(u32ResAnim != orxU32_Undefined)
  {
    /* Gets current loop counter */
    u32Loop = orxAnimSet_GetLinkTableLinkProperty(_pstLinkTable, u32ResAnim, orxANIMSET_KU32_LINK_FLAG_PRIORITY);

    /* Is loop counter used? */
    if(u32Loop != orxU32_Undefined)
    {
      /* Updates loop counter */
      orxAnimSet_SetLinkTableLinkProperty(_pstLinkTable, u32ResAnim, orxANIMSET_KU32_LINK_FLAG_PRIORITY, u32Loop - 1);

      /* Is link table dirty again? */
      if(u32Loop <= 1)
      {
        /* Updates flags */
        orxAnimSet_SetLinkTableFlag(_pstLinkTable, orxANIMSET_KU32_LINK_TABLE_FLAG_DIRTY, orxANIMSET_KU32_LINK_TABLE_FLAG_NONE);
      }
    }

    /* Gets real anim index */
    u32ResAnim = u32ResAnim - u32BaseIndex;
  }

  /* Done! */
  return u32ResAnim;
}

/***************************************************************************
 orxAnimSet_ComputeNextAnimUsingDest
 Gets next animation using destination one, updating link status.

 returns: orxU32 animation index / orxU32_Undefined if none found
 ***************************************************************************/
orxSTATIC orxU32 orxAnimSet_ComputeNextAnimUsingDest(orxANIM_SET_LINK_TABLE *_pstLinkTable, orxU32 _u32SrcAnim, orxU32 _u32DstAnim)
{
  orxU32 u32BaseIndex, u32Size, u32Loop;
  orxU32 u32Anim = orxU32_Undefined, u32Link, u32LinkIndex;

  /* Checks */
  orxASSERT(_pstLinkTable != orxNULL);

  /* Gets animation storage size */
  u32Size = (orxU32)(_pstLinkTable->u16TableSize);

  /* Gets animation base index */
  u32BaseIndex = _u32SrcAnim * u32Size;

  /* Gets link value */
  u32Link = _pstLinkTable->au32LinkArray[u32BaseIndex + _u32DstAnim];

  /* Is there a path? */
  if(u32Link & orxANIMSET_KU32_LINK_FLAG_PATH)
  {
    /* Gets anim index */
    u32Anim = (u32Link & orxANIMSET_KU32_LINK_MASK_ANIM) >> orxANIMSET_KU32_LINK_SHIFT_ANIM;

    /* Gets direct link id */
    u32LinkIndex = u32BaseIndex + u32Anim;

    /* Gets direct link */
    u32Link = _pstLinkTable->au32LinkArray[u32LinkIndex];

    /* Gets current loop counter */
    u32Loop = orxAnimSet_GetLinkTableLinkProperty(_pstLinkTable, u32LinkIndex, orxANIMSET_KU32_LINK_FLAG_PRIORITY);

    /* Is loop counter used? */
    if(u32Loop != orxU32_Undefined)
    {
      /* Updates loop counter */
      orxAnimSet_SetLinkTableLinkProperty(_pstLinkTable, u32LinkIndex, orxANIMSET_KU32_LINK_FLAG_PRIORITY, u32Loop - 1);

      /* Is link table dirty again? */
      if(u32Loop <= 1)
      {
        /* Updates flags */
        orxAnimSet_SetLinkTableFlag(_pstLinkTable, orxANIMSET_KU32_LINK_TABLE_FLAG_DIRTY, orxANIMSET_KU32_LINK_TABLE_FLAG_NONE);
      }
    }
  }

  /* Done! */
  return u32Anim;
}

/***************************************************************************
 orxAnimSet_ComputeLinkTable
 Computes all link relations.

 returns: orxSTATUS_SUCCESS / orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxAnimSet_ComputeLinkTable(orxANIM_SET_LINK_TABLE *_pstLinkTable)
{
  orxLINK_UPDATE_INFO *pstUpdateInfo;
  orxSTATUS eResult = orxSTATUS_SUCCESS;
  orxU32 u32Counter, u32Size;
  orxU32 i;

  /* Checks */
  orxASSERT(_pstLinkTable != orxNULL);

  /* Gets anim counter */
  u32Counter = (orxU32)(_pstLinkTable->u16LinkCounter);

  /* Checks it */
  orxASSERT(u32Counter > 0);

  /* Is link table dirty? */
  if(orxAnimSet_TestLinkTableFlag(_pstLinkTable, orxANIMSET_KU32_LINK_TABLE_FLAG_DIRTY) != orxFALSE)
  {
    /* Has ready link structure */
    if(orxAnimSet_TestLinkTableFlag(_pstLinkTable, orxANIMSET_KU32_LINK_TABLE_FLAG_READY) != orxFALSE)
    {
      /* Gets storage size */
      u32Size = (orxU32)_pstLinkTable->u16TableSize;
    
      /* Cleans link table */
      for(i = 0; i < u32Size * u32Size; i++)
      {
        /* No link found? */
        if(!(_pstLinkTable->au32LinkArray[i] & orxANIMSET_KU32_LINK_FLAG_LINK))
        {
          _pstLinkTable->au32LinkArray[i] = orxANIMSET_KU32_LINK_DEFAULT_NONE;
        }
      }
    
      /* Creates a link update info */
      pstUpdateInfo = orxAnimSet_CreateLinkUpdateInfo(_pstLinkTable);
    
      /* Was allocated? */
      if(pstUpdateInfo != orxNULL)
      {
        /* Updates all animations */
        for(i = 0; i < u32Counter; i++)
        {
          orxAnimSet_UpdateLink(i, pstUpdateInfo);
        }
      
        /* Updates flags */
        orxAnimSet_SetLinkTableFlag(_pstLinkTable, orxANIMSET_KU32_LINK_TABLE_FLAG_NONE, orxANIMSET_KU32_LINK_TABLE_FLAG_DIRTY);
      
        /* Deletes the link update info */
        orxAnimSet_DeleteLinkUpdateInfo(pstUpdateInfo);
      }
      else
      {
        /* !!! MSG !!! */

        /* Failed */
        eResult = orxSTATUS_FAILED;
      }
    }
    else
    {
      /* !!! MSG !!! */
  
      /* Failed */
      eResult = orxSTATUS_FAILED;
    }
  }

  /* Done! */
  return eResult;
}


/***************************************************************************
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 orxAnimSet_Init
 Inits Animation Set system.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxAnimSet_Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Not already Initialized? */
  if(!(sstAnimSet.u32Flags & orxANIMSET_KU32_FLAG_READY))
  {
    /* Cleans control structure */
    orxMemory_Set(&sstAnimSet, 0, sizeof(orxANIM_SET_STATIC));

    /* Inits Anim before */
    eResult = orxAnim_Init();

    /* Initialized? */
    if(eResult == orxSTATUS_SUCCESS)
    {
      orxSTRUCTURE_REGISTER_INFO stRegisterInfo;

      /* Registers structure type */
      stRegisterInfo.eStorageType = orxSTRUCTURE_STORAGE_TYPE_LINKLIST;
      stRegisterInfo.u32Size      = sizeof(orxANIM_SET);
      stRegisterInfo.eMemoryType  = orxMEMORY_TYPE_MAIN;
      stRegisterInfo.pfnUpdate    = orxNULL;

      eResult = orxStructure_Register(orxSTRUCTURE_ID_ANIM_SET, &stRegisterInfo);
    }
    else
    {
      /* !!! MSG !!! */
    }
  }
  else
  {
    /* !!! MSG !!! */

    /* Already initialized */
    eResult = orxSTATUS_FAILED;
  }

  /* Initialized? */
  if(eResult == orxSTATUS_SUCCESS)
  {
    /* Inits Flags */
    sstAnimSet.u32Flags = orxANIMSET_KU32_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxAnimSet_Exit
 Exits from the Animation Set system.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxAnimSet_Exit()
{
  /* Initialized? */
  if(sstAnimSet.u32Flags & orxANIMSET_KU32_FLAG_READY)
  {
    /* Deletes anim list */
    orxAnimSet_DeleteAll();

    /* Updates flags */
    sstAnimSet.u32Flags &= ~orxANIMSET_KU32_FLAG_READY;

    /* Exit from Anim after */
    orxAnim_Exit();
  }

  return;
}

/***************************************************************************
 orxAnimSet_Create
 Creates an empty Animation Set, given a storage size (<= orxANIMSET_KS32_TEXTURE_MAX_NUMBER).

 returns: Created animset.
 ***************************************************************************/
orxANIM_SET *orxAnimSet_Create(orxU32 _u32Size)
{
  orxANIM_SET *pstAnimset;

  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_FLAG_READY);
  orxASSERT(_u32Size <= orxANIMSET_KU32_MAX_ANIM_NUMBER);

  /* Creates animset */
  pstAnimset = (orxANIM_SET *)orxMemory_Allocate(sizeof(orxANIM_SET), orxMEMORY_TYPE_MAIN);

  /* Non null? */
  if(pstAnimset != orxNULL)
  {
    /* Cleans it */
    orxMemory_Set(pstAnimset, 0, sizeof(orxANIM_SET));

    /* Inits structure */
    if(orxStructure_Setup((orxSTRUCTURE *)pstAnimset, orxSTRUCTURE_ID_ANIM_SET) == orxSTATUS_SUCCESS)
    {
      /* Allocates anim pointer array */
      pstAnimset->pastAnim = (orxANIM **)orxMemory_Allocate(_u32Size * sizeof(orxANIM *), orxMEMORY_TYPE_MAIN);

      /* Was allocated? */  
      if(pstAnimset->pastAnim != orxNULL)
      {
        /* Cleans it */
        orxMemory_Set(pstAnimset->pastAnim, 0, _u32Size * sizeof(orxANIM *));

        /* Set storage size & counter */
        orxAnimSet_SetAnimStorageSize(pstAnimset, _u32Size);
        orxAnimSet_SetAnimCounter(pstAnimset, 0);

        /* Creates link table */
        pstAnimset->pstLinkTable = orxAnimSet_CreateLinkTable(_u32Size);

        /* Was allocated? */
        if(pstAnimset->pstLinkTable != orxNULL)
        {
          /* Updates flags */
          orxAnimSet_SetLinkTableFlag(pstAnimset->pstLinkTable, orxANIMSET_KU32_LINK_TABLE_FLAG_READY | orxANIMSET_KU32_LINK_TABLE_FLAG_DIRTY, orxANIMSET_KU32_LINK_TABLE_FLAG_NONE);
      
          /* Inits flags */
          orxAnimSet_SetFlag(pstAnimset, orxANIMSET_KU32_ID_FLAG_LINK_STATIC, orxANIMSET_KU32_ID_MASK_FLAGS);
        }
        else
        {
          /* !!! MSG !!! */
    
          /* Frees partially allocated texture */
          orxMemory_Free(pstAnimset->pastAnim);
          orxMemory_Free(pstAnimset);
    
          /* Not created */
          pstAnimset = orxNULL;
        }
      }
      else
      {
        /* !!! MSG !!! */
  
        /* Frees partially allocated texture */
        orxMemory_Free(pstAnimset);
  
        /* Not created */
        pstAnimset = orxNULL;
      }
    }
    else
    {
      /* !!! MSG !!! */

      /* Frees partially allocated texture */
      orxMemory_Free(pstAnimset);

      /* Not created */
      pstAnimset = orxNULL;
    }
  }
  else
  {
    /* !!! MSG !!! */

    /* Not created */
    pstAnimset = orxNULL;
  }

  return pstAnimset;
}

/***************************************************************************
 orxAnimSet_Delete
 Deletes an Animation Set.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxAnimSet_Delete(orxANIM_SET *_pstAnimset)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_FLAG_READY);
  orxASSERT(_pstAnimset != orxNULL);

  /* Not referenced? */
  if(orxStructure_GetRefCounter((orxSTRUCTURE *)_pstAnimset) == 0)
  {
    /* Cleans members */
    orxAnimSet_RemoveAllAnims(_pstAnimset);
    orxAnimSet_DeleteLinkTable(_pstAnimset->pstLinkTable);

    /* Cleans structure */
    orxStructure_Clean((orxSTRUCTURE *)_pstAnimset);

    /* Frees animset memory */
    orxMemory_Free(_pstAnimset);
  }
  else
  {
    /* !!! MSG !!! */
    
    /* Referenced by others */
    eResult = orxSTATUS_FAILED;
  }

  return eResult;
}

/***************************************************************************
 orxAnimSet_AddReference
 Adds a reference on an AnimationSet.

 returns: Nothing
 ***************************************************************************/
orxVOID orxAnimSet_AddReference(orxANIM_SET *_pstAnimset)
{
  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_FLAG_READY);
  orxASSERT(_pstAnimset != orxNULL);

  /* Locks animset */
  orxAnimSet_SetFlag(_pstAnimset, orxANIMSET_KU32_ID_FLAG_REFERENCE_LOCK, orxANIMSET_KU32_ID_FLAG_NONE);

  /* Updates reference counter */
  orxStructure_IncreaseCounter((orxSTRUCTURE *)_pstAnimset);

  return;
}

/***************************************************************************
 orxAnimSet_RemoveReference
 Removes a reference from an AnimationSet.

 returns: Nothing
 ***************************************************************************/
orxVOID orxAnimSet_RemoveReference(orxANIM_SET *_pstAnimset)
{
  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_FLAG_READY);
  orxASSERT(_pstAnimset != orxNULL);

  /* Updates reference counter */
  orxStructure_DecreaseCounter((orxSTRUCTURE *)_pstAnimset);

  /* No reference left? */
  if(orxStructure_GetRefCounter((orxSTRUCTURE *)_pstAnimset) == 0)
  {
    /* Unlocks animset */
    orxAnimSet_SetFlag(_pstAnimset, orxANIMSET_KU32_ID_FLAG_NONE, orxANIMSET_KU32_ID_FLAG_REFERENCE_LOCK);
  }

  return;
}

/***************************************************************************
 orxAnimSet_AddAnim
 Adds an Animation to an Animation Set.

 returns: Added Animation handle / orxHANDLE_Undefined
 ***************************************************************************/
orxHANDLE orxAnimSet_AddAnim(orxANIM_SET *_pstAnimset, orxANIM *_pstAnim)
{
  orxU32 u32Counter, u32Size, u32Index;
  orxHANDLE hResult = orxHANDLE_Undefined;

  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_FLAG_READY);
  orxASSERT(_pstAnimset != orxNULL);
  orxASSERT(_pstAnim != orxNULL);

  /* Gets storage size & counter */
  u32Size     = orxAnimSet_GetAnimStorageSize(_pstAnimset);
  u32Counter  = orxAnimSet_GetAnimCounter(_pstAnimset);

  /* Not locked? */
  if(orxAnimSet_TestFlag(_pstAnimset, orxANIMSET_KU32_ID_FLAG_REFERENCE_LOCK) == orxFALSE)
  {
    /* Is there free room? */
    if(u32Counter < u32Size)
    {
      /* Finds the first empty slot */
      for(u32Index = 0; u32Index < u32Size; u32Index++)
      {
        if(_pstAnimset->pastAnim[u32Index] == (orxANIM*)orxNULL)
        {
          break;
        }
      }

      /* Checks slot found */
      orxASSERT(u32Index < u32Size);

      /* Adds the extra animation */
      _pstAnimset->pastAnim[u32Index] = _pstAnim;
  
      /* Updates Animation reference counter */
      orxStructure_IncreaseCounter((orxSTRUCTURE *)_pstAnim);

      /* Updates Animation counter */
      orxAnimSet_IncreaseAnimCounter(_pstAnimset);
      
      /* Gets result handle */
      hResult = (orxHANDLE)u32Index;
    }
    else
    {
      /* !!! MSG !!! */

      /* Can't process */
      hResult = orxHANDLE_Undefined;
    }
  }
  else
  {
    /* !!! MSG !!! */

    /* Can't process */
    hResult = orxHANDLE_Undefined;
  }

  /* Done! */
  return hResult; 
}

/***************************************************************************
 orxAnimSet_RemoveAnim
 Removes an Animation from an Animation Set given its ID.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxAnimSet_RemoveAnim(orxANIM_SET *_pstAnimset, orxHANDLE _hAnimHandle)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_FLAG_READY);
  orxASSERT(_pstAnimset != orxNULL);

  /* Not locked? */
  if(orxAnimSet_TestFlag(_pstAnimset, orxANIMSET_KU32_ID_FLAG_REFERENCE_LOCK) == orxFALSE)
  {
    orxU32 u32AnimIndex;
    
    /* Gets animation index */
    u32AnimIndex = (orxU32)_hAnimHandle;

    /* Animation found for the given ID? */
    if(_pstAnimset->pastAnim[u32AnimIndex] == orxNULL)
    {
      /* Updates counter */
      orxAnimSet_DecreaseAnimCounter(_pstAnimset);
  
      /* Updates animation reference counter */
      orxStructure_DecreaseCounter((orxSTRUCTURE *)(_pstAnimset->pastAnim[u32AnimIndex]));
  
      /* Removes animation */
      _pstAnimset->pastAnim[u32AnimIndex] = orxNULL;

      /* Cleans link table for this animation */
      orxAnimSet_CleanLinkTable(_pstAnimset->pstLinkTable, u32AnimIndex);
    }
    else
    {
      /* !!! MSG !!! */

      /* Failed */
      eResult = orxSTATUS_FAILED;
    }
  }
  else
  {
    /* !!! MSG !!! */

    /* Failed */
    eResult = orxSTATUS_FAILED;
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxAnimSet_RemoveAllAnims
 Cleans all referenced Animations from an Animation Set.

 returns: orxSTATUS_SUCCESS on success / orxSTATUS_FAILED otherwise
 ***************************************************************************/
orxSTATUS orxAnimSet_RemoveAllAnims(orxANIM_SET *_pstAnimset)
{
  orxU32 u32Counter, i;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_FLAG_READY);
  orxASSERT(_pstAnimset != orxNULL);

  /* Locked? */
  if(orxAnimSet_TestFlag(_pstAnimset, orxANIMSET_KU32_ID_FLAG_REFERENCE_LOCK) != orxFALSE)
  {
    /* !!! MSG !!! */

    return orxSTATUS_FAILED;
  }

  /* Gets animation counter */
  u32Counter = orxAnimSet_GetAnimCounter(_pstAnimset);

  /* Until there are no texture left */
  for(i = 0; (i < u32Counter) && (eResult == orxSTATUS_SUCCESS); i++)
  {
    eResult = orxAnimSet_RemoveAnim(_pstAnimset, (orxHANDLE)i);
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxAnimSet_GetAnim
 Animation used by an Animation Set get accessor, given its index.

 returns: orxAnim *
 ***************************************************************************/
orxANIM *orxAnimSet_GetAnim(orxANIM_SET *_pstAnimset, orxHANDLE _hAnimHandle)
{
  orxU32 u32Counter;
  orxANIM *pstAnim = orxNULL;

  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_FLAG_READY);
  orxASSERT(_pstAnimset != orxNULL);

  /* Gets counter */
  u32Counter = orxAnimSet_GetAnimCounter(_pstAnimset);

  /* Is index valid? */
  if((orxU32)_hAnimHandle < u32Counter)
  {
    /* Gets Animation */
    pstAnim = _pstAnimset->pastAnim[(orxU32)_hAnimHandle];
  }
  else
  {
    /* !!! MSG !!! */

  }

  /* Done! */
  return pstAnim;
}

/***************************************************************************
 orxAnimSet_AddLink
 Adds a link between 2 Animations.

 returns: orxHANDLE Link handle / orxHANDLE_Undefined
 ***************************************************************************/
orxHANDLE orxAnimSet_AddLink(orxANIM_SET *_pstAnimset, orxHANDLE _hSrcAnim, orxHANDLE _hDstAnim)
{
  orxU32 u32Size, u32Link, u32Index;
  orxANIM_SET_LINK_TABLE *pstLinkTable;
  orxHANDLE hResult = orxHANDLE_Undefined;

  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_FLAG_READY);
  orxASSERT(_pstAnimset != orxNULL);

  /* Gets storage size */
  u32Size = orxAnimSet_GetAnimStorageSize(_pstAnimset);

  /* Checks anim index validity */
  orxASSERT((orxU32)_hSrcAnim < u32Size);
  orxASSERT((orxU32)_hDstAnim < u32Size);

  /* Not locked? */
  if(orxAnimSet_TestFlag(_pstAnimset, orxANIMSET_KU32_ID_FLAG_REFERENCE_LOCK) == orxFALSE)
  {
    /* Gets link table */
    pstLinkTable  = _pstAnimset->pstLinkTable;
  
    /* Computes link index */
    u32Index      = ((orxU32)_hSrcAnim * u32Size) + (orxU32)_hDstAnim;
  
    /* Gets link */
    u32Link       = orxAnimSet_GetLinkTableLink(pstLinkTable, u32Index);
  
    /* Isn't there already a link? */
    if((u32Link & orxANIMSET_KU32_LINK_FLAG_LINK) == orxANIMSET_KU32_LINK_FLAG_NONE)
    {
      /* Adds link */
      u32Link = (orxANIMSET_KU32_LINK_FLAG_LINK | orxANIMSET_KU32_LINK_FLAG_PATH)
              + (0x00000001 << orxANIMSET_KU32_LINK_SHIFT_LENGTH)
              + (orxANIMSET_KU32_LINK_DEFAULT_PRIORITY << orxANIMSET_KU32_LINK_SHIFT_PRIORITY)
              + ((orxU32)_hDstAnim << orxANIMSET_KU32_LINK_SHIFT_ANIM);
    
      /* Stores it */
      orxAnimSet_SetLinkTableLink(pstLinkTable, u32Index, u32Link);

      /* Updates counter */
      pstLinkTable->u16LinkCounter++;

      /* Animset has to be computed again */
      orxAnimSet_SetLinkTableFlag(pstLinkTable, orxANIMSET_KU32_LINK_TABLE_FLAG_DIRTY, orxANIMSET_KU32_LINK_TABLE_FLAG_NONE);
      
      /* Gets link handle */
      hResult = (orxHANDLE)u32Index;
    }
    else
    {
      /* !!! MSG !!! */

      /* Link not added */
      hResult = orxHANDLE_Undefined;
    }
  }
  else
  {
    /* !!! MSG !!! */

    /* Link not added */
    hResult = orxHANDLE_Undefined;
  }

  /* Done! */
  return hResult;
}

/***************************************************************************
 orxAnimSet_RemoveLink
 Removes a link given its ID.

 returns: orxSTATUS_SUCCESS / orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxAnimSet_RemoveLink(orxANIM_SET *_pstAnimset, orxHANDLE _hLinkHandle)
{
  orxU32 u32Size;
  orxANIM_SET_LINK_TABLE *pstLinkTable;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_FLAG_READY);
  orxASSERT(_pstAnimset != orxNULL);

  /* Not locked? */
  if(orxAnimSet_TestFlag(_pstAnimset, orxANIMSET_KU32_ID_FLAG_REFERENCE_LOCK) == orxFALSE)
  {
    /* Gets link table */
    pstLinkTable = _pstAnimset->pstLinkTable;
  
    /* Gets storage size */
    u32Size = orxAnimSet_GetAnimStorageSize(_pstAnimset);
  
    /* Checks link index validity */
    orxASSERT((orxU32)_hLinkHandle < u32Size * u32Size);
  
    /* Link found? */
    if(orxAnimSet_GetLinkTableLink(pstLinkTable, (orxU32)_hLinkHandle) & orxANIMSET_KU32_LINK_FLAG_LINK)
    {
      /* Updates link table */
      orxAnimSet_SetLinkTableLink(pstLinkTable, (orxU32)_hLinkHandle, orxANIMSET_KU32_LINK_DEFAULT_NONE);
      orxAnimSet_SetLinkTableLinkProperty(pstLinkTable, (orxU32)_hLinkHandle, orxANIMSET_KU32_LINK_FLAG_PRIORITY, 0);
    
      /* Animset has to be computed again */
      orxAnimSet_SetLinkTableFlag(pstLinkTable, orxANIMSET_KU32_LINK_TABLE_FLAG_DIRTY, orxANIMSET_KU32_LINK_TABLE_FLAG_NONE);
    }
    else
    {
      /* !!! MSG !!! */

      /* Can't process */
      eResult = orxSTATUS_FAILED;
    }
  }
  else
  {
    /* !!! MSG !!! */

    /* Can't process */
    eResult = orxSTATUS_FAILED;
  }

  /* Done! */
  return orxSTATUS_SUCCESS;
}

/***************************************************************************
 orxAnimSet_ComputeLinks
 Computes all link relations.

 returns: orxSTATUS_SUCCESS / orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxAnimSet_ComputeLinks(orxANIM_SET *_pstAnimset)
{
  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_FLAG_READY);
  orxASSERT(_pstAnimset != orxNULL);

  /* Locked? */
  if(orxAnimSet_TestFlag(_pstAnimset, orxANIMSET_KU32_ID_FLAG_REFERENCE_LOCK) != orxFALSE)
  {
    /* !!! MSG !!! */

    return orxSTATUS_FAILED;
  }

  /* Gets work done */
  return(orxAnimSet_ComputeLinkTable(_pstAnimset->pstLinkTable));
}

/***************************************************************************
 orxAnimSet_GetLink
 Gets a direct link between two animations (if none, result is orxU32_Undefined).

 returns: orxU32 Link ID / orxU32_Undefined
 ***************************************************************************/
orxHANDLE orxAnimSet_GetLink(orxANIM_SET *_pstAnimset, orxHANDLE _hSrcAnim, orxHANDLE _hDstAnim)
{
  orxU32 u32Index, u32Size;
  orxANIM_SET_LINK_TABLE *pstLinkTable;
  orxHANDLE hResult = orxHANDLE_Undefined;

  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_FLAG_READY);
  orxASSERT(_pstAnimset != orxNULL);

  /* Gets storage size */
  u32Size = orxAnimSet_GetAnimStorageSize(_pstAnimset);

  /* Checks anim index validity */
  orxASSERT((orxU32)_hSrcAnim < u32Size);
  orxASSERT((orxU32)_hDstAnim < u32Size);

  /* Computes link index */
  u32Index = ((orxU32)_hSrcAnim * u32Size) + (orxU32)_hDstAnim;

  /* Gets link table */
  pstLinkTable = _pstAnimset->pstLinkTable;

  /* Is there a link? */
  if(orxAnimSet_GetLinkTableLink(pstLinkTable, u32Index) & orxANIMSET_KU32_LINK_FLAG_LINK)
  {
    /* Gets link handle */
    hResult = (orxHANDLE)u32Index;
  }
  else
  {
    /* No link found */
    hResult = orxHANDLE_Undefined;
  }

  /* Done! */
  return hResult;
}

/***************************************************************************
 orxAnimSet_SetLinkProperty
 Sets a link property at the given value.

 returns: orxSTATUS_SUCCESS / orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxAnimSet_SetLinkProperty(orxANIM_SET *_pstAnimset, orxHANDLE _hLinkHandle, orxU32 _u32Property, orxU32 _u32Value)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_FLAG_READY);
  orxASSERT(_pstAnimset != orxNULL);
  orxASSERT((orxU32)_hLinkHandle < orxAnimSet_GetAnimStorageSize(_pstAnimset) * orxAnimSet_GetAnimStorageSize(_pstAnimset));

  /* Not locked? */
  if(orxAnimSet_TestFlag(_pstAnimset, orxANIMSET_KU32_ID_FLAG_REFERENCE_LOCK) != orxFALSE)
  {
    /* Gets work done */
    eResult = orxAnimSet_SetLinkTableLinkProperty(_pstAnimset->pstLinkTable, (orxU32)_hLinkHandle, _u32Property, _u32Value);
  
    /* Changes occured? */
    if(eResult == orxSTATUS_SUCCESS)
    {
      /* Added loop counter? */
      if(_u32Property == orxANIMSET_KU32_LINK_FLAG_LOOP_COUNTER)
      {
        /* Link table should be locally stored by animation pointers now */
        orxAnimSet_SetFlag(_pstAnimset, orxANIMSET_KU32_ID_FLAG_NONE, orxANIMSET_KU32_ID_FLAG_LINK_STATIC);
      }
    }
  }
  else
  {
    /* !!! MSG !!! */

    /* Can't process */
    eResult = orxSTATUS_FAILED;
  }

  /* Done! */
  return eResult; 
}

/***************************************************************************
 orxAnimSet_GetLinkProperty
 Gets a link property.

 returns: orxU32 Property value / orxU32_Undefined
 ***************************************************************************/
orxU32 orxAnimSet_GetLinkProperty(orxANIM_SET *_pstAnimset, orxHANDLE _hLinkHandle, orxU32 _u32Property)
{
  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_FLAG_READY);
  orxASSERT(_pstAnimset != orxNULL);

  /* Returns property */
  return(orxAnimSet_GetLinkTableLinkProperty(_pstAnimset->pstLinkTable, (orxU32)_hLinkHandle, _u32Property));
}

/***************************************************************************
 orxAnimSet_ComputeAnim
 Computes active animation given current and destination Animation ID & a relative timestamp (writable).

 returns: active animation handle (_pu32Time is updated at need) / orxHANDLE_Undefined if none
 ***************************************************************************/
orxHANDLE orxAnimSet_ComputeAnim(orxANIM_SET *_pstAnimset, orxHANDLE _hSrcAnim, orxHANDLE _hDstAnim, orxU32 *_pu32Time, orxANIM_SET_LINK_TABLE *_pstLinkTable)
{
  orxU32 u32Length, u32Anim;
  orxHANDLE hResult = orxHANDLE_Undefined;
  orxANIM_SET_LINK_TABLE *pstWorkTable;

  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_FLAG_READY);
  orxASSERT(_pstAnimset != orxNULL);
  orxASSERT(_pu32Time != orxNULL);
  orxASSERT((orxU32)_hSrcAnim < orxAnimSet_GetAnimCounter(_pstAnimset));
  orxASSERT(((orxU32)_hDstAnim < orxAnimSet_GetAnimCounter(_pstAnimset)) || (_hDstAnim == orxHANDLE_Undefined));

  /* Gets Link Table */
  if(orxAnimSet_TestFlag(_pstAnimset, orxANIMSET_KU32_ID_FLAG_LINK_STATIC) == orxFALSE)
  {
    /* Use animation pointer local one */
    pstWorkTable = _pstLinkTable;
  }
  else
  {
    /* Uses common static one */
    pstWorkTable = _pstAnimset->pstLinkTable;
  }

  /* Checks working link table */
  orxASSERT(pstWorkTable != orxNULL);

  /* Computes link table if needed */
  if(orxAnimSet_ComputeLinkTable(pstWorkTable) == orxSTATUS_SUCCESS)
  {
    /* Gets current animation */
    u32Anim   = (orxU32)_hSrcAnim;
  
    /* Gets current animation duration */
    u32Length = orxAnim_GetLength(_pstAnimset->pastAnim[u32Anim]);

    /* Next animation? */
    while(*_pu32Time > u32Length)
    {
      /* Auto mode? */
      if(_hDstAnim == orxHANDLE_Undefined)
      {
        /* Get next animation */
        u32Anim = orxAnimSet_ComputeNextAnim(pstWorkTable, u32Anim);
      }
      /* Destination mode */
      else
      {
        /* Get next animation according to destination aim */
        u32Anim = orxAnimSet_ComputeNextAnimUsingDest(pstWorkTable, u32Anim, (orxU32)_hDstAnim);
      }
  
      /* Updates timestamp */
      *_pu32Time -= u32Length;
  
      /* Has next animation? */
      if(u32Anim != orxU32_Undefined)
      {
        /* Gets new duration */
        u32Length = orxAnim_GetLength(_pstAnimset->pastAnim[u32Anim]);
        
        /* Stores current result handle */
        hResult   = (orxHANDLE)u32Anim;
      }
      else
      {
        /* !!! MSG !!! */

        /* Not found */
        hResult = orxHANDLE_Undefined;
        break;
      }
    }
  }

  /* Done! */
  return hResult;
}


/* *** Structure accessors *** */


/***************************************************************************
 orxAnimSet_GetAnimCounter
 Animation Set internal Animation counter get accessor.

 returns: orxU32 counter
 ***************************************************************************/
orxU32 orxAnimSet_GetAnimCounter(orxANIM_SET *_pstAnimset)
{
  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_FLAG_READY);
  orxASSERT(_pstAnimset != orxNULL);

  /* Gets counter */
  return((_pstAnimset->u32IDFlags & orxANIMSET_KU32_ID_MASK_COUNTER) >> orxANIMSET_KU32_ID_SHIFT_COUNTER);
}

/***************************************************************************
 orxAnimSet_GetAnimStorageSize
 Animation Set internal Animation storage size get accessor.

 returns: orxU32 storage size
 ***************************************************************************/
orxU32 orxAnimSet_GetAnimStorageSize(orxANIM_SET *_pstAnimset)
{
  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_FLAG_READY);
  orxASSERT(_pstAnimset != orxNULL);

  /* Gets storage size */
  return((_pstAnimset->u32IDFlags & orxANIMSET_KU32_ID_MASK_SIZE) >> orxANIMSET_KU32_ID_SHIFT_SIZE);
}  

/***************************************************************************
 orxAnimSet_TestFlag
 Animation set flag test accessor.

 returns: orxBOOL
 ***************************************************************************/
orxBOOL orxAnimSet_TestFlag(orxANIM_SET *_pstAnimset, orxU32 _u32Flag)
{
  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_FLAG_READY);
  orxASSERT(_pstAnimset != orxNULL);

  /* Tests flags */
  return((_pstAnimset->u32IDFlags & _u32Flag) == _u32Flag);
}

/***************************************************************************
 orxAnimSet_SetFlag
 Animation set flag get/set accessor.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxAnimSet_SetFlag(orxANIM_SET *_pstAnimset, orxU32 _u32AddFlags, orxU32 _u32RemoveFlags)
{
  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_FLAG_READY);
  orxASSERT(_pstAnimset != orxNULL);

  _pstAnimset->u32IDFlags &= ~_u32RemoveFlags;
  _pstAnimset->u32IDFlags |= _u32AddFlags;

  return;
}


/* *** Link Table public functions *** */

/***************************************************************************
 orxAnimSet_DeleteLinkTable
 Deletes the AnimationSet link table.

 returns: Nothing
 ***************************************************************************/
orxVOID orxAnimSet_DeleteLinkTable(orxANIM_SET_LINK_TABLE *_pstLinkTable)
{
  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_FLAG_READY);
  orxASSERT(_pstLinkTable != orxNULL);

  /* Frees memory*/
  orxMemory_Free(_pstLinkTable->au32LinkArray);
  orxMemory_Free(_pstLinkTable->au8LoopArray);
  orxMemory_Free(_pstLinkTable);

  return;
}

/***************************************************************************
 orxAnimSet_DuplicateLinkTable
 Duplicates an Animset link table.

 returns: orxANIM_SET_LINK_TABLE *
 ***************************************************************************/
orxANIM_SET_LINK_TABLE *orxAnimSet_DuplicateLinkTable(orxANIM_SET *_pstAnimset)
{
  orxANIM_SET_LINK_TABLE *pstLinkTable = orxNULL;

  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_FLAG_READY);
  orxASSERT(_pstAnimset != orxNULL);

  /* Creates a new link table */
  pstLinkTable = orxAnimSet_CreateLinkTable((orxU32)(_pstAnimset->pstLinkTable->u16TableSize));

  /* Was allocated? */
  if(pstLinkTable != orxNULL)
  {
    /* Copies it */
    orxAnimSet_CopyLinkTable(_pstAnimset->pstLinkTable, pstLinkTable);
  }
  else
  {
    /* !!! MSG !!! */

    /* Can't process */
    pstLinkTable = orxNULL;
  }
  
  /* Done! */
  return pstLinkTable;
}
