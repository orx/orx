/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2015 Orx-Project
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
 * @file orxAnimSet.h
 * @date 13/02/2004
 * @author iarwain@orx-project.org
 *
 */


#include "anim/orxAnimSet.h"

#include "core/orxConfig.h"
#include "debug/orxDebug.h"
#include "debug/orxProfiler.h"
#include "memory/orxMemory.h"
#include "utils/orxHashTable.h"
#include "utils/orxString.h"


#ifdef __orxMSVC__

  #pragma warning(disable : 4311 4312)

#endif /* __orxMSVC__ */

#if defined(__orxX86_64__) || defined(__orxPPC64__) || defined(__orxARM64__)

  #define orxANIMSET_CAST_HELPER   (orxU64)

#else /* __orxX86_64__ || __orxPPC64__ || __orxARM64__ */

  #define orxANIMSET_CAST_HELPER

#endif /* __orxX86_64__ || __orxPPC64__ || __orxARM64__ */


/** Module flags
 */
#define orxANIMSET_KU32_STATIC_FLAG_NONE              0x00000000  /**< No flags */

#define orxANIMSET_KU32_STATIC_FLAG_READY             0x00000001  /**< Ready flag */

#define orxANIMSET_KU32_STATIC_MASK_ALL               0xFFFFFFFF  /**< All mask*/


/** orxANIMSET flags
 */
#define orxANIMSET_KU32_FLAG_NONE                     0x00000000  /**< ID flag none */

#define orxANIMSET_KU32_FLAG_INTERNAL                 0x10000000  /**< Internal structure handling flag  */
#define orxANIMSET_KU32_FLAG_REFERENCED               0x20000000  /**< Referenced flag  */
#define orxANIMSET_KU32_FLAG_ID_TABLE                 0x40000000  /**< Has an ID table flag  */

#define orxANIMSET_KU32_MASK_SIZE                     0x000003FF  /**< ID mask for size */
#define orxANIMSET_KU32_MASK_COUNTER                  0x000FFC00  /**< ID mask for counter */
#define orxANIMSET_KU32_MASK_FLAGS                    0xFFF00000  /**< ID mask for flags */

#define orxANIMSET_KU32_ID_SHIFT_SIZE                 0           /**< ID shift for size */
#define orxANIMSET_KU32_ID_SHIFT_COUNTER              10          /**< ID shift for counter */


/** Link table link flags
 */
#define orxANIMSET_KU32_LINK_FLAG_NONE                0x00000000  /**< Link flag none */

#define orxANIMSET_KU32_LINK_FLAG_PATH                0x01000000  /**< Link flag for path */
#define orxANIMSET_KU32_LINK_FLAG_LINK                0x02000000  /**< Link flag for link */

#define orxANIMSET_KU32_LINK_MASK_ANIM                0x000000FF  /**< Link mask for anim */
#define orxANIMSET_KU32_LINK_MASK_LENGTH              0x0000FF00  /**< Link mask for length */
#define orxANIMSET_KU32_LINK_MASK_PRIORITY            0x000F0000  /**< Link mask for priority */
#define orxANIMSET_KU32_LINK_MASK_FLAGS               0xFF000000  /**< Link mask for flags */

#define orxANIMSET_KU32_LINK_SHIFT_ANIM               0           /**< Link shift for anim */
#define orxANIMSET_KU32_LINK_SHIFT_LENGTH             8           /**< Link shift for length */
#define orxANIMSET_KU32_LINK_SHIFT_PRIORITY           16          /**< Link shift for priority */

#define orxANIMSET_KU32_LINK_DEFAULT_NONE             0x00000000  /**< Link default : none */

#define orxANIMSET_KU32_LINK_DEFAULT_PRIORITY         0x00000008  /**< Link default priority */


/** Link table (status) flags
 */
#define orxANIMSET_KU32_LINK_TABLE_FLAG_NONE          0x00000000  /**< Link table flag none */

#define orxANIMSET_KU32_LINK_TABLE_FLAG_READY         0x01000000  /**< Link table flag ready */
#define orxANIMSET_KU32_LINK_TABLE_FLAG_DIRTY         0x02000000  /**< Link table flag dirty */

#define orxANIMSET_KU32_LINK_TABLE_MASK_FLAGS         0xFFFF0000  /**< Link table mask flags */


/** Misc defines
 */
#define orxANIMSET_KZ_CONFIG_ANIM_LIST                "AnimationList"
#define orxANIMSET_KZ_CONFIG_LINK_LIST                "LinkList"
#define orxANIMSET_KZ_CONFIG_LINK_SOURCE              "Source"
#define orxANIMSET_KZ_CONFIG_LINK_DESTINATION         "Destination"
#define orxANIMSET_KZ_CONFIG_LINK_PROPERTY            "Property"
#define orxANIMSET_KZ_CONFIG_LINK_PRIORITY            "Priority"

#define orxANIMSET_KZ_IMMEDIATE                       "immediate"
#define orxANIMSET_KZ_CLEAR_TARGET                    "cleartarget"

#define orxANIMSET_KU32_ID_TABLE_SIZE                 32          /**< ID table size */
#define orxANIMSET_KU32_REFERENCE_TABLE_SIZE          32          /**< Reference table size */
#define orxANIMSET_KU32_BANK_SIZE                     64          /**< Bank size */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Internal Link Update Info structure
 */
typedef struct __orxLINK_UPDATE_INFO_t
{
  orxANIMSET_LINK_TABLE  *pstLinkTable;               /**< Link table pointer : 4 */
  orxU8                  *au8LinkInfo;                /**< Link update info : 8 */
  orxU32                  u32ByteNumber;              /**< Byte number per animation : 12 */

} orxLINK_UPDATE_INFO;

/** Internal Animation Set Link table structure
 */
struct __orxANIMSET_LINK_TABLE_t
{
  orxU32                 *au32LinkArray;              /**< Link array : 4 */
  orxU8                  *au8LoopArray;               /**< Loop array : 8 */
  orxU16                  u16LinkCounter;             /**< Link counter : 10 */
  orxU16                  u16TableSize;               /**< Table size : 12 */
  orxU32                  u32Flags;                   /**< Flags : 16 */
};

/** AnimSet structure
 */
struct __orxANIMSET_t
{
  orxSTRUCTURE            stStructure;                /**< Public structure, first structure member : 32 */
  orxANIM               **pastAnim;                   /**< Used animation pointer array : 20 */
  orxANIMSET_LINK_TABLE  *pstLinkTable;               /**< Link table pointer : 24 */
  orxHASHTABLE           *pstIDTable;                 /**< ID hash table : 28 */
  const orxSTRING         zReference;                 /**< Reference : 32 */
};


/** Static structure
 */
typedef struct __orxANIMSET_STATIC_t
{
  orxU32        u32Flags;                             /**< Control flags : 4 */
  orxHASHTABLE *pstReferenceTable;                    /**< Table to avoid animation set duplication when creating through config file : 8 */

} orxANIMSET_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxANIMSET_STATIC sstAnimSet;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Link table set flag test accessor
 * @param[in]   _pstLinkTable	                Concerned LinkTable
 * @param[in]   _u32Flags                     Flags to test
 * @return      orxTRUE / orxFALSE
 */
static orxINLINE orxBOOL orxAnimSet_TestLinkTableFlag(orxANIMSET_LINK_TABLE *_pstLinkTable, orxU32 _u32Flags)
{
  /* Checks */
  orxASSERT(_pstLinkTable != orxNULL);

  /* Done! */
  return((_pstLinkTable->u32Flags & _u32Flags) == _u32Flags);
}

/** Link table set flag set accessor
 * @param[in]   _pstLinkTable	                Concerned LinkTable
 * @param[in]   _u32AddFlags                  Flags to add
 * @param[in]   _u32RemoveFlags               Flags to remove
 */
static orxINLINE void orxAnimSet_SetLinkTableFlag(orxANIMSET_LINK_TABLE *_pstLinkTable, orxU32 _u32AddFlags, orxU32 _u32RemoveFlags)
{
  /* Checks */
  orxASSERT(_pstLinkTable != orxNULL);

  _pstLinkTable->u32Flags &= ~_u32RemoveFlags;
  _pstLinkTable->u32Flags |= _u32AddFlags;

  return;
}

/** Gets a link value from a link table
 * @param[in]   _pstLinkTable	                Concerned LinkTable
 * @param[in]   _u32Index                     Link index
 * @return      Link value / orxU32_UNDEFINED
 */
static orxINLINE orxU32 orxAnimSet_GetLinkTableLink(orxANIMSET_LINK_TABLE *_pstLinkTable, orxU32 _u32Index)
{
  /* Checks */
  orxASSERT(_pstLinkTable != orxNULL);
  orxASSERT(_u32Index < (orxU32)(_pstLinkTable->u16TableSize) * (orxU32)(_pstLinkTable->u16TableSize));

  /* Done! */
  return _pstLinkTable->au32LinkArray[_u32Index];
}

/** Sets a link value in a link table
 * @param[in]   _pstLinkTable	                Concerned LinkTable
 * @param[in]   _u32Index                     Link index
 * @param[in]		_u32LinkValue									Value to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS orxAnimSet_SetLinkTableLink(orxANIMSET_LINK_TABLE *_pstLinkTable, orxU32 _u32Index, orxU32 _u32LinkValue)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(_pstLinkTable != orxNULL);
  orxASSERT(_u32Index < (orxU32)(_pstLinkTable->u16TableSize) * (orxU32)(_pstLinkTable->u16TableSize));

  /* Sets link */
  _pstLinkTable->au32LinkArray[_u32Index] = _u32LinkValue;

  /* Done! */
  return eResult;
}

/** Sets a link property
 * @param[in]   _pstLinkTable	                Concerned LinkTable
 * @param[in]   _u32LinkIndex                 Link index
 * @param[in]		_u32Property									Property ID
 * @param[in]		_u32Value											Value to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS orxAnimSet_SetLinkTableLinkProperty(orxANIMSET_LINK_TABLE *_pstLinkTable, orxU32 _u32LinkIndex, orxU32 _u32Property, orxU32 _u32Value)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

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
      {
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
      }

      /* Priority */
      case orxANIMSET_KU32_LINK_FLAG_PRIORITY:
      {
        /* Checks */
        orxASSERT(_u32Value <= (orxANIMSET_KU32_LINK_MASK_PRIORITY >> orxANIMSET_KU32_LINK_SHIFT_PRIORITY));

        /* Updates priority */
        if(_u32Value != orxU32_UNDEFINED)
        {
          _pstLinkTable->au32LinkArray[_u32LinkIndex] |= orxANIMSET_KU32_LINK_FLAG_PRIORITY
                                                   + ((_u32Value << orxANIMSET_KU32_LINK_SHIFT_PRIORITY) & orxANIMSET_KU32_LINK_MASK_PRIORITY);
        }
        else
        {
          _pstLinkTable->au32LinkArray[_u32LinkIndex] &= ~(orxANIMSET_KU32_LINK_FLAG_PRIORITY | orxANIMSET_KU32_LINK_MASK_PRIORITY);
        }

        break;
      }

      /* Common flags */
      case orxANIMSET_KU32_LINK_FLAG_IMMEDIATE_CUT:
      case orxANIMSET_KU32_LINK_FLAG_CLEAR_TARGET:
      {
        /* Enable? */
        if((orxBOOL)_u32Value != orxFALSE)
        {
          _pstLinkTable->au32LinkArray[_u32LinkIndex] |= _u32Property & orxANIMSET_KU32_LINK_MASK_FLAGS;
        }
        /* Disable */
        else
        {
          _pstLinkTable->au32LinkArray[_u32LinkIndex] &= ~(_u32Property & orxANIMSET_KU32_LINK_MASK_FLAGS);
        }

        break;
      }

      /* Invalid */
      default :
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Invalid link flag.");

        /* Updates result */
        eResult = orxSTATUS_FAILURE;

        break;
      }
    }

    /* Valid? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Animset has to be computed again */
      orxAnimSet_SetLinkTableFlag(_pstLinkTable, orxANIMSET_KU32_LINK_TABLE_FLAG_DIRTY, orxANIMSET_KU32_LINK_TABLE_FLAG_NONE);
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Link table not linked at index %d.", _u32LinkIndex);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done */
  return eResult;
}

/** Gets a link property
 * @param[in]   _pstLinkTable	                Concerned LinkTable
 * @param[in]   _u32LinkIndex									Link index
 * @param[in]		_u32Property									Link property ID
 * @return      Link property value / orxU32_UNDEFINED;
 */
static orxINLINE orxU32 orxAnimSet_GetLinkTableLinkProperty(orxANIMSET_LINK_TABLE *_pstLinkTable, orxU32 _u32LinkIndex, orxU32 _u32Property)
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
      {
        /* Has a counter? */
        if(_pstLinkTable->au32LinkArray[_u32LinkIndex] & orxANIMSET_KU32_LINK_FLAG_LOOP_COUNTER)
        {
          u32Value = (orxU32)(_pstLinkTable->au8LoopArray[_u32LinkIndex]);
        }
        else
        {
          u32Value = orxU32_UNDEFINED;
        }

        break;
      }

      /* Priority */
      case orxANIMSET_KU32_LINK_FLAG_PRIORITY:
      {
        /* Has priority? */
        if(_pstLinkTable->au32LinkArray[_u32LinkIndex] & orxANIMSET_KU32_LINK_FLAG_PRIORITY)
        {
          u32Value = (_pstLinkTable->au32LinkArray[_u32LinkIndex] & orxANIMSET_KU32_LINK_MASK_PRIORITY) >> orxANIMSET_KU32_LINK_SHIFT_PRIORITY;
        }
        else
        {
          u32Value = (orxU32)orxANIMSET_KU32_LINK_DEFAULT_PRIORITY;
        }

        break;
      }

      /* Common flags */
      case orxANIMSET_KU32_LINK_FLAG_IMMEDIATE_CUT:
      case orxANIMSET_KU32_LINK_FLAG_CLEAR_TARGET:
      {
        /* Updates result */
        u32Value = (orxU32)((_pstLinkTable->au32LinkArray[_u32LinkIndex] & (_u32Property & orxANIMSET_KU32_LINK_MASK_FLAGS)) ? orxTRUE : orxFALSE);

        break;
      }

      /* Invalid */
      default :
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Invalid property while looking in link table element %d.", _u32LinkIndex);

        /* Undefined result */
        u32Value = orxU32_UNDEFINED;

        break;
      }
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Invalid link index %d.", _u32LinkIndex);

    /* Undefined result */
    u32Value = orxU32_UNDEFINED;
  }

  /* Done */
  return u32Value;
}

/** Sets a link info between 2 Anims
 * @param[in]   _pstInfo			                Concerned LinkUpdateInfo
 * @param[in]   _u32SrcIndex									Source Anim index
 * @param[in]		_u32DstIndex									Destination Anim index
 */
static orxINLINE void orxAnimSet_SetLinkInfo(orxLINK_UPDATE_INFO *_pstInfo, orxU32 _u32SrcIndex, orxU32 _u32DstIndex)
{
  register orxU32 u32Index;
  register orxU8 u8Mask;

  /* Checks */
  orxASSERT(_pstInfo != orxNULL);

  /* Computes real index */
  u32Index = (_u32SrcIndex * _pstInfo->u32ByteNumber) + (_u32DstIndex >> 3);

  /* Computes mask */
  u8Mask = (orxU8)(0x01 << (0x07 - (_u32DstIndex & 0x00000007)));

  /* Sets info */
  _pstInfo->au8LinkInfo[u32Index] |= u8Mask;

  return;
}

/** Resets a link info between 2 Anims
 * @param[in]   _pstInfo			                Concerned LinkUpdateInfo
 * @param[in]   _u32SrcIndex									Source Anim index
 * @param[in]		_u32DstIndex									Destination Anim index
 */
static orxINLINE void orxAnimSet_ResetLinkInfo(orxLINK_UPDATE_INFO *_pstInfo, orxU32 _u32SrcIndex, orxU32 _u32DstIndex)
{
  register orxU32 u32Index;
  register orxU8 u8Mask;

  /* Checks */
  orxASSERT(_pstInfo != orxNULL);
  orxASSERT(_u32SrcIndex < (orxU32)(_pstInfo->pstLinkTable->u16TableSize));
  orxASSERT(_u32DstIndex < (orxU32)(_pstInfo->pstLinkTable->u16TableSize));

  /* Computes real index */
  u32Index = (_u32SrcIndex * _pstInfo->u32ByteNumber) + (_u32DstIndex >> 3);

  /* Computes mask */
  u8Mask = (orxU8)(0x01 << (0x07 - (_u32DstIndex & 0x00000007)));

  /* Resets info */
  _pstInfo->au8LinkInfo[u32Index] &= ~u8Mask;

  return;
}

/** Cleans all link info from an Anim
 * @param[in]   _pstInfo			                Concerned LinkUpdateInfo
 * @param[in]   _u32SrcIndex									Source Anim index
 */
static orxINLINE void orxAnimSet_CleanLinkInfo(orxLINK_UPDATE_INFO *_pstInfo, orxU32 _u32SrcIndex)
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

/** Gets a link info between 2 Anims
 * @param[in]   _pstInfo			                Concerned LinkUpdateInfo
 * @param[in]   _u32SrcIndex									Source Anim index
 * @param[in]		_u32DstIndex									Destination Anim index
 * @return			orxTRUE / orxFALSE
 */
static orxINLINE orxBOOL orxAnimSet_GetLinkInfo(orxLINK_UPDATE_INFO *_pstInfo, orxU32 _u32SrcIndex, orxU32 _u32DstIndex)
{
  register orxU32 u32Index;
  register orxU8 u8Mask;

  /* Checks */
  orxASSERT(_pstInfo != orxNULL);
  orxASSERT(_u32SrcIndex < (orxU32)(_pstInfo->pstLinkTable->u16TableSize));
  orxASSERT(_u32DstIndex < (orxU32)(_pstInfo->pstLinkTable->u16TableSize));

  /* Computes real index */
  u32Index = (_u32SrcIndex * _pstInfo->u32ByteNumber) + (_u32DstIndex >> 3);

  /* Computes mask */
  u8Mask = (orxU8)(0x01 << (0x07 - (_u32DstIndex & 0x00000007)));

  /* Returns info */
  return((_pstInfo->au8LinkInfo[u32Index] & u8Mask) ? orxTRUE : orxFALSE);
}

/** Updates links information from one Anim to another
 * @param[in]   _pstInfo			                Concerned LinkUpdateInfo
 * @param[in]   _u32SrcIndex									Source Anim index
 * @param[in]		_u32DstIndex									Destination Anim index
 * @return      orxTRUE / orxFALSE
 */
static orxBOOL orxFASTCALL orxAnimSet_UpdateLinkInfo(orxLINK_UPDATE_INFO *_pstInfo, orxU32 _u32SrcIndex, orxU32 _u32DstIndex)
{
  orxU32                  u32SrcLink, u32DstLink, u32SrcLength, u32DstLength;
  orxU32                  u32DirectLink, u32DirectPriority, u32Priority;
  orxU32                  u32SrcBaseIndex, u32DstBaseIndex, i;
  orxANIMSET_LINK_TABLE  *pstLinkTable;
  orxBOOL                 bChange = orxFALSE;

  /* Checks */
  orxASSERT(_pstInfo != orxNULL);
  orxASSERT(_u32SrcIndex < (orxU32)(_pstInfo->pstLinkTable->u16TableSize));
  orxASSERT(_u32DstIndex < (orxU32)(_pstInfo->pstLinkTable->u16TableSize));

  /* Gets link table */
  pstLinkTable = _pstInfo->pstLinkTable;

  /* Computes base indices */
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
  for(i = 0; i < (orxU32)(pstLinkTable->u16TableSize); i++)
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
                   | ((u32SrcLength + 1) << orxANIMSET_KU32_LINK_SHIFT_LENGTH)
                   | (u32DirectPriority << orxANIMSET_KU32_LINK_SHIFT_PRIORITY)
                   | (_u32SrcIndex << orxANIMSET_KU32_LINK_SHIFT_ANIM);

        /* Stores it */
        pstLinkTable->au32LinkArray[u32DstBaseIndex + i] = u32DstLink;

        /* Changes have been made */
        bChange = orxTRUE;
      }
    }
  }

  /* Done! */
  return bChange;
}

 /** Updates all links for an Anim
 * @param[in]   _u32Index			                Anim index
 * @param[in]   _pstInfo											Associated LinkUpdateInfo
 * @return      orxTRUE if updated, orxFALSE otherwise
 */
static orxBOOL orxFASTCALL orxAnimSet_UpdateLink(orxU32 _u32Index, orxLINK_UPDATE_INFO *_pstInfo)
{
  orxU32  u32BaseIndex, i;
  orxBOOL bResult = orxFALSE;

  /* Checks */
  orxASSERT(_pstInfo != orxNULL);
  orxASSERT(_u32Index < (orxU32)(_pstInfo->pstLinkTable->u16TableSize));

  /* Are links not already computed/updated for this animations? */
  if(orxAnimSet_GetLinkInfo(_pstInfo, _u32Index, _u32Index) == orxFALSE)
  {
    orxANIMSET_LINK_TABLE *pstLinkTable;

    /* Gets link table */
    pstLinkTable = _pstInfo->pstLinkTable;

    /* Mark animation as read */
    orxAnimSet_SetLinkInfo(_pstInfo, _u32Index, _u32Index);

    /* Computes base index */
    u32BaseIndex = _u32Index * (orxU32)(pstLinkTable->u16TableSize);

    /* For all found links */
    for(i = 0; i < (orxU32)(pstLinkTable->u16TableSize); i++)
    {
      /* Link found? */
      if(pstLinkTable->au32LinkArray[u32BaseIndex + i] & orxANIMSET_KU32_LINK_FLAG_LINK)
      {
        /* Is animation 'dirty' for this one? */
        if(orxAnimSet_GetLinkInfo(_pstInfo, i, _u32Index) == orxFALSE)
        {
          /* Updates 'dirty' status */
          orxAnimSet_SetLinkInfo(_pstInfo, i, _u32Index);

          /* Updates info */
          if(orxAnimSet_UpdateLinkInfo(_pstInfo, i, _u32Index) != orxFALSE)
          {
            /* Links have been modified for current animation */
            orxAnimSet_CleanLinkInfo(_pstInfo, _u32Index);
            orxAnimSet_SetLinkInfo(_pstInfo, _u32Index, _u32Index);

            /* Updates result */
            bResult = orxTRUE;
          }
        }
      }
    }

    /* Unmark animation as read */
    orxAnimSet_ResetLinkInfo(_pstInfo, _u32Index, _u32Index);
  }

  /* Done! */
  return bResult;
}

/** Creates and inits a LinkUpdateInfo
 * @param[in]   _pstLinkTable	                Concerned LinkTable
 * @return			Created & up to date LinkUpdateInfo / orxNULL
 */
static orxLINK_UPDATE_INFO *orxFASTCALL orxAnimSet_CreateLinkUpdateInfo(orxANIMSET_LINK_TABLE *_pstLinkTable)
{
  orxLINK_UPDATE_INFO *pstInfo;
  orxU32 u32Number;

  /* Checks */
  orxASSERT(_pstLinkTable != orxNULL);

  /* Allocates memory */
  pstInfo = (orxLINK_UPDATE_INFO *)orxMemory_Allocate(sizeof(orxLINK_UPDATE_INFO), orxMEMORY_TYPE_MAIN);

  /* Was allocated? */
  if(pstInfo != orxNULL)
  {
    /* Cleans it */
    orxMemory_Zero(pstInfo, sizeof(orxLINK_UPDATE_INFO));

    /* Stores link table */
    pstInfo->pstLinkTable = _pstLinkTable;

    /* Computes number of orxU8 needed for one link info : ((size - 1) / 8) + 1 */
    u32Number = (orxU32)(((_pstLinkTable->u16TableSize - 1) >> 3) + 1);

    /* Stores it */
    pstInfo->u32ByteNumber = u32Number;

    /* Allocates link info array */
    pstInfo->au8LinkInfo = (orxU8 *)orxMemory_Allocate(u32Number * (orxU32)(_pstLinkTable->u16TableSize) * sizeof(orxU8), orxMEMORY_TYPE_MAIN);

    /* Was allocated? */
    if(pstInfo->au8LinkInfo != orxNULL)
    {
      /* Cleans it */
      orxMemory_Zero(pstInfo->au8LinkInfo, u32Number * (orxU32)(_pstLinkTable->u16TableSize) * sizeof(orxU8));
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Link info array failed to allocate.");

      /* Frees previously allocated memory */
      orxMemory_Free(pstInfo);

      /* Not created */
      pstInfo = orxNULL;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Link Update Info structure failed to allocate.");

    /* Not created */
    pstInfo = orxNULL;
  }

  /* Done! */
  return pstInfo;
}

/** Deletes a LinkUpdateInfo
 * @param[in]   _pstLinkUpdateInfo            Concerned LinkUpdateInfo
 */
static orxINLINE void orxAnimSet_DeleteLinkUpdateInfo(orxLINK_UPDATE_INFO *_pstLinkUpdateInfo)
{
  /* Checks */
  orxASSERT(_pstLinkUpdateInfo != orxNULL);

  /* Frees all */
  orxMemory_Free(_pstLinkUpdateInfo->au8LinkInfo);
  orxMemory_Free(_pstLinkUpdateInfo);

  return;
}

/** Cleans a LinkTable for a given Anim
 * @param[in]   _pstLinkTable	                Concerned LinkTable
 * @param[in]   _u32AnimIndex									Anim index
 */
static void orxFASTCALL orxAnimSet_CleanLinkTable(orxANIMSET_LINK_TABLE *_pstLinkTable, orxU32 _u32AnimIndex)
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
  for(i = _u32AnimIndex; i < u32Size * u32Size; i += u32Size)
  {
    _pstLinkTable->au32LinkArray[i] = orxANIMSET_KU32_LINK_DEFAULT_NONE;
    _pstLinkTable->au8LoopArray[i] = 0;
  }

  /* Updates flags */
  orxAnimSet_SetLinkTableFlag(_pstLinkTable, orxANIMSET_KU32_LINK_TABLE_FLAG_DIRTY, orxANIMSET_KU32_LINK_TABLE_FLAG_NONE);

  return;
}

/** Creates a LinkTable
 * @param[in]   _u32Size											Size of the desired LinkTable
 * @return 			Created LinkTable / orxNULL
 */
static orxANIMSET_LINK_TABLE *orxFASTCALL orxAnimSet_CreateLinkTable(orxU32 _u32Size)
{
  orxANIMSET_LINK_TABLE *pstLinkTable;

  /* Checks */
  orxASSERT(_u32Size < 0xFFFF);

  /* Allocates link table */
  pstLinkTable = (orxANIMSET_LINK_TABLE *)orxMemory_Allocate(sizeof(orxANIMSET_LINK_TABLE), orxMEMORY_TYPE_MAIN);

  /* Was allocated? */
  if(pstLinkTable != orxNULL)
  {
    orxU32 u32ArraySize;

    /* Cleans it */
    orxMemory_Zero(pstLinkTable, sizeof(orxANIMSET_LINK_TABLE));

    /* Gets array size */
    u32ArraySize = _u32Size * _u32Size;

    /* Creates link array */
    pstLinkTable->au32LinkArray = (orxU32 *)orxMemory_Allocate(u32ArraySize * sizeof(orxU32), orxMEMORY_TYPE_MAIN);

    /* Was allocated? */
    if(pstLinkTable->au32LinkArray != orxNULL)
    {
      /* Allocates loop table */
      pstLinkTable->au8LoopArray = (orxU8 *)orxMemory_Allocate(u32ArraySize * sizeof(orxU8), orxMEMORY_TYPE_MAIN);

      /* Was allocated? */
      if(pstLinkTable->au8LoopArray != orxNULL)
      {
        /* Cleans tables */
        orxMemory_Zero(pstLinkTable->au32LinkArray, u32ArraySize * sizeof(orxU32));
        orxMemory_Zero(pstLinkTable->au8LoopArray, u32ArraySize * sizeof(orxU8));

        /* Inits values */
        pstLinkTable->u16TableSize    = (orxU16)_u32Size;
        pstLinkTable->u16LinkCounter  = 0;

        /* Inits flags */
        orxAnimSet_SetLinkTableFlag(pstLinkTable, orxANIMSET_KU32_LINK_TABLE_FLAG_READY | orxANIMSET_KU32_LINK_TABLE_FLAG_DIRTY, orxANIMSET_KU32_LINK_TABLE_MASK_FLAGS);
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Loop table failed to allocate.");

        /* Frees previously allocated memory */
        orxMemory_Free(pstLinkTable->au32LinkArray);
        orxMemory_Free(pstLinkTable);

        /* Not created */
        pstLinkTable = orxNULL;
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Link array failed to allocate.");

      /* Frees link table */
      orxMemory_Free(pstLinkTable);

      /* Not created */
      pstLinkTable = orxNULL;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Link table failed to allocate.");

    /* Not created */
    pstLinkTable = orxNULL;
  }

  /* Done! */
  return pstLinkTable;
}

/** Copies an AnimSet LinkTable into another one
 * @param[in]   _pstDstLinkTable	            Destination LinkTable
 * @param[in]   _pstSrcLinkTable							Source LinkTable
 */
static orxINLINE void orxAnimSet_CopyLinkTable(orxANIMSET_LINK_TABLE *_pstDstLinkTable, const orxANIMSET_LINK_TABLE *_pstSrcLinkTable)
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

/** Sets an AnimSet storage size
 * @param[in]   _pstAnimSet										Concerned AnimSet
 * @param[in]   _u32Size											Desired size
 */
static orxINLINE void orxAnimSet_SetAnimStorageSize(orxANIMSET *_pstAnimSet, orxU32 _u32Size)
{
  /* Checks */
  orxSTRUCTURE_ASSERT(_pstAnimSet);
  orxASSERT(_u32Size <= orxANIMSET_KU32_MAX_ANIM_NUMBER);

  /* Updates storage size */
  orxStructure_SetFlags(_pstAnimSet, _u32Size << orxANIMSET_KU32_ID_SHIFT_SIZE, orxANIMSET_KU32_MASK_SIZE);

  return;
}

/** Sets an AnimSet internal Anim counter
 * @param[in]   _pstAnimSet										Concerned AnimSet
 * @param[in]   _u32AnimCounter								Counter value
 */
static orxINLINE void orxAnimSet_SetAnimCounter(orxANIMSET *_pstAnimSet, orxU32 _u32AnimCounter)
{
  /* Checks */
  orxASSERT(_u32AnimCounter <= orxAnimSet_GetAnimStorageSize(_pstAnimSet));

  /* Updates counter */
  orxStructure_SetFlags(_pstAnimSet, _u32AnimCounter << orxANIMSET_KU32_ID_SHIFT_COUNTER, orxANIMSET_KU32_MASK_COUNTER);

  return;
}

/** Increases an AnimSet internal Anim counter
 * @param[in]   _pstAnimSet										Concerned AnimSet
 */
static orxINLINE void orxAnimSet_IncreaseAnimCounter(orxANIMSET *_pstAnimSet)
{
  register orxU32 u32AnimCounter;

  /* Checks */
  orxSTRUCTURE_ASSERT(_pstAnimSet);

  /* Gets anim counter */
  u32AnimCounter = orxAnimSet_GetAnimCounter(_pstAnimSet);

  /* Updates anim counter*/
  orxAnimSet_SetAnimCounter(_pstAnimSet, u32AnimCounter + 1);

  return;
}

/** Decreases an AnimSet internal Anim counter
 * @param[in]   _pstAnimSet										Concerned AnimSet
 */
static orxINLINE void orxAnimSet_DecreaseAnimCounter(orxANIMSET *_pstAnimSet)
{
  register orxU32 u32AnimCounter;

  /* Checks */
  orxSTRUCTURE_ASSERT(_pstAnimSet);

  /* Gets anim counter */
  u32AnimCounter = orxAnimSet_GetAnimCounter(_pstAnimSet);

  /* Updates anim counter*/
  orxAnimSet_SetAnimCounter(_pstAnimSet, u32AnimCounter - 1);

  return;
}

/** Deletes all AnimSet
 */
static orxINLINE void orxAnimSet_DeleteAll()
{
  orxANIMSET *pstAnimSet;

  /* Gets first anim set */
  pstAnimSet = orxANIMSET(orxStructure_GetFirst(orxSTRUCTURE_ID_ANIMSET));

  /* Non empty? */
  while(pstAnimSet != orxNULL)
  {
    /* Deletes Animation Set */
    orxAnimSet_Delete(pstAnimSet);

    /* Gets first Animation Set */
    pstAnimSet = orxANIMSET(orxStructure_GetFirst(orxSTRUCTURE_ID_ANIMSET));
  }

  return;
}

/** Computes next Animation, updating LinkTable link status
 * @param[in]   _pstLinkTable									Concerned LinkTable
 * @param[in]   _u32SrcAnim										Source Anim index
 * @param[in]   _u32DstAnim										Destination Anim index / orxU32_UNDEFINED
 * @param[in]   _bSimulate                    Simulation mode, no update will be made on the link table
 * @return 			Next Anim index / orxU32_UNDEFINED
 */
static orxU32 orxFASTCALL orxAnimSet_ComputeNextAnim(orxANIMSET_LINK_TABLE *_pstLinkTable, orxU32 _u32SrcAnim, orxU32 _u32DstAnim, orxBOOL _bSimulate)
{
  orxU32 u32BaseIndex, u32Size;
  orxU32 u32Result = orxU32_UNDEFINED;

  /* Checks */
  orxASSERT(_pstLinkTable != orxNULL);

  /* Gets animation storage size */
  u32Size = (orxU32)(_pstLinkTable->u16TableSize);

  /* Gets animation base index */
  u32BaseIndex = _u32SrcAnim * u32Size;

  /* Has a destination anim? */
  if(_u32DstAnim != orxU32_UNDEFINED)
  {
    orxU32 u32Link;

    /* Gets link value */
    u32Link = _pstLinkTable->au32LinkArray[u32BaseIndex + _u32DstAnim];

    /* Is there a path? */
    if(u32Link & orxANIMSET_KU32_LINK_FLAG_PATH)
    {
      /* Gets anim index */
      u32Result = (u32Link & orxANIMSET_KU32_LINK_MASK_ANIM) >> orxANIMSET_KU32_LINK_SHIFT_ANIM;
    }
  }
  /* No destination anim */
  else
  {
    orxS32 s32ResultPriority;
    orxU32 i;

    /* For all possible links */
    for(i = u32BaseIndex, u32Result = orxU32_UNDEFINED, s32ResultPriority = -1; i < u32BaseIndex + u32Size; i++)
    {
      orxU32 u32Link;

      /* Gets value */
      u32Link = _pstLinkTable->au32LinkArray[i];

      /* Link found? */
      if(u32Link & orxANIMSET_KU32_LINK_FLAG_LINK)
      {
        orxU32 u32Loop;

        /* Gets link loop counter */
        u32Loop = orxAnimSet_GetLinkTableLinkProperty(_pstLinkTable, i, orxANIMSET_KU32_LINK_FLAG_LOOP_COUNTER);

        /* Has no empty loop counter (if no loop, value is orxU32_UNDEFINED)? */
        if(u32Loop != 0)
        {
          orxS32 s32LinkPriority;

          /* Gets path priority */
          s32LinkPriority = (orxS32)orxAnimSet_GetLinkTableLinkProperty(_pstLinkTable, i, orxANIMSET_KU32_LINK_FLAG_PRIORITY);

          /* Higher priority or self looping and same priority? */
          if((s32LinkPriority > s32ResultPriority)
          || ((i == u32BaseIndex + _u32SrcAnim)
           && (s32LinkPriority == s32ResultPriority)))
          {
            /* Stores new link info */
            u32Result         = i;
            s32ResultPriority = s32LinkPriority;
          }
        }
      }
    }

    /* Link found? */
    if(u32Result != orxU32_UNDEFINED)
    {
      /* Gets real anim index */
      u32Result -= u32BaseIndex;
    }
  }

  /* Animation found? */
  if(u32Result != orxU32_UNDEFINED)
  {
    /* Not in simulation mode? */
    if(_bSimulate == orxFALSE)
    {
      orxU32 u32LinkIndex, u32Loop;

      /* Gets direct link id */
      u32LinkIndex = u32BaseIndex + u32Result;

      /* Gets current loop counter */
      u32Loop = orxAnimSet_GetLinkTableLinkProperty(_pstLinkTable, u32LinkIndex, orxANIMSET_KU32_LINK_FLAG_LOOP_COUNTER);

      /* Is loop counter used? */
      if(u32Loop != orxU32_UNDEFINED)
      {
        /* Updates loop counter */
        orxAnimSet_SetLinkTableLinkProperty(_pstLinkTable, u32LinkIndex, orxANIMSET_KU32_LINK_FLAG_LOOP_COUNTER, u32Loop - 1);

        /* Is link table dirty again? */
        if(u32Loop <= 1)
        {
          /* Updates flags */
          orxAnimSet_SetLinkTableFlag(_pstLinkTable, orxANIMSET_KU32_LINK_TABLE_FLAG_DIRTY, orxANIMSET_KU32_LINK_TABLE_FLAG_NONE);
        }
      }
    }
  }

  /* Done! */
  return u32Result;
}

/** Computes all link relations
 * @param[in]   _pstLinkTable									Concerned LinkTable
 * @return 			orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxSTATUS orxFASTCALL orxAnimSet_ComputeLinkTable(orxANIMSET_LINK_TABLE *_pstLinkTable)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxAnimSet_ComputeLinkTable");

  /* Checks */
  orxASSERT(_pstLinkTable != orxNULL);

  /* Is link table dirty? */
  if(orxAnimSet_TestLinkTableFlag(_pstLinkTable, orxANIMSET_KU32_LINK_TABLE_FLAG_DIRTY) != orxFALSE)
  {
    /* Has ready link structure */
    if(orxAnimSet_TestLinkTableFlag(_pstLinkTable, orxANIMSET_KU32_LINK_TABLE_FLAG_READY) != orxFALSE)
    {
      orxLINK_UPDATE_INFO  *pstUpdateInfo;
      orxU32                u32Size, i;

      /* Gets storage size */
      u32Size = (orxU32)(_pstLinkTable->u16TableSize * _pstLinkTable->u16TableSize);

      /* Cleans link table */
      for(i = 0; i < u32Size; i++)
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
        orxBOOL bNeedUpdate = orxTRUE;

        /* While updates are needed */
        while(bNeedUpdate != orxFALSE)
        {
          /* Clears update */
          bNeedUpdate = orxFALSE;

          /* For all animations */
          for(i = 0; i < _pstLinkTable->u16TableSize; i++)
          {
            /* Updates its links */
            bNeedUpdate = orxAnimSet_UpdateLink(i, pstUpdateInfo) || bNeedUpdate;
          }
        }

        /* Updates flags */
        orxAnimSet_SetLinkTableFlag(_pstLinkTable, orxANIMSET_KU32_LINK_TABLE_FLAG_NONE, orxANIMSET_KU32_LINK_TABLE_FLAG_DIRTY);

        /* Deletes the link update info */
        orxAnimSet_DeleteLinkUpdateInfo(pstUpdateInfo);
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Link Update Info failed to allocate.");

        /* Failed */
        eResult = orxSTATUS_FAILURE;
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Link table does not have a ready link structure.");

      /* Failed */
      eResult = orxSTATUS_FAILURE;
    }
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return eResult;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** AnimSet module setup
 */
void orxFASTCALL orxAnimSet_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_ANIMSET, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_ANIMSET, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_ANIMSET, orxMODULE_ID_STRING);
  orxModule_AddDependency(orxMODULE_ID_ANIMSET, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_ANIMSET, orxMODULE_ID_PROFILER);
  orxModule_AddDependency(orxMODULE_ID_ANIMSET, orxMODULE_ID_ANIM);

  return;
}

/** Inits the AnimSet module
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnimSet_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstAnimSet.u32Flags & orxANIMSET_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstAnimSet, sizeof(orxANIMSET_STATIC));

    /* Creates reference table */
    sstAnimSet.pstReferenceTable = orxHashTable_Create(orxANIMSET_KU32_REFERENCE_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if(sstAnimSet.pstReferenceTable != orxNULL)
    {
      /* Registers structure type */
      eResult = orxSTRUCTURE_REGISTER(ANIMSET, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxANIMSET_KU32_BANK_SIZE, orxNULL);
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Already loaded AnimSet module.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Initialized? */
  if(eResult != orxSTATUS_FAILURE)
  {
    /* Inits Flags */
    sstAnimSet.u32Flags = orxANIMSET_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Failed to register AnimSet module.");
  }

  /* Done! */
  return eResult;
}

/** Exits from the AnimSet module
 */
void orxFASTCALL orxAnimSet_Exit()
{
  /* Initialized? */
  if(sstAnimSet.u32Flags & orxANIMSET_KU32_STATIC_FLAG_READY)
  {
    /* Deletes anim list */
    orxAnimSet_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_ANIMSET);

    /* Deletes reference table */
    orxHashTable_Delete(sstAnimSet.pstReferenceTable);

    /* Updates flags */
    sstAnimSet.u32Flags &= ~orxANIMSET_KU32_STATIC_FLAG_READY;
  }

  return;
}

/** Creates an empty AnimSet
 * @param[in]		_u32Size														Storage size
 * return 			Created orxANIMSET / orxNULL
 */
orxANIMSET *orxFASTCALL orxAnimSet_Create(orxU32 _u32Size)
{
  orxANIMSET *pstAnimSet;

  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_STATIC_FLAG_READY);
  orxASSERT(_u32Size <= orxANIMSET_KU32_MAX_ANIM_NUMBER);

  /* Creates animset */
  pstAnimSet = orxANIMSET(orxStructure_Create(orxSTRUCTURE_ID_ANIMSET));

  /* Non null? */
  if(pstAnimSet != orxNULL)
  {
    /* Allocates anim pointer array */
    pstAnimSet->pastAnim = (orxANIM **)orxMemory_Allocate(_u32Size * sizeof(orxANIM *), orxMEMORY_TYPE_MAIN);

    /* Was allocated? */
    if(pstAnimSet->pastAnim != orxNULL)
    {
      /* Cleans it */
      orxMemory_Zero(pstAnimSet->pastAnim, _u32Size * sizeof(orxANIM *));

      /* Set storage size & counter */
      orxAnimSet_SetAnimStorageSize(pstAnimSet, _u32Size);
      orxAnimSet_SetAnimCounter(pstAnimSet, 0);

      /* Creates link table */
      pstAnimSet->pstLinkTable = orxAnimSet_CreateLinkTable(_u32Size);

      /* Was allocated? */
      if(pstAnimSet->pstLinkTable != orxNULL)
      {
        /* Updates flags */
        orxAnimSet_SetLinkTableFlag(pstAnimSet->pstLinkTable, orxANIMSET_KU32_LINK_TABLE_FLAG_READY | orxANIMSET_KU32_LINK_TABLE_FLAG_DIRTY, orxANIMSET_KU32_LINK_TABLE_FLAG_NONE);

        /* Inits flags */
        orxStructure_SetFlags(pstAnimSet, orxANIMSET_KU32_FLAG_LINK_STATIC, orxANIMSET_KU32_MASK_FLAGS);

        /* Increases counter */
        orxStructure_IncreaseCounter(pstAnimSet);
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Failed to create link table.");

        /* Frees partially allocated structures */
        orxMemory_Free(pstAnimSet->pastAnim);
        orxStructure_Delete(pstAnimSet);

        /* Not created */
        pstAnimSet = orxNULL;
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Failed to create anim pointer array.");

      /* Frees partially allocated structure */
      orxStructure_Delete(pstAnimSet);

      /* Not created */
      pstAnimSet = orxNULL;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Failed to create animset structure.");

    /* Not created */
    pstAnimSet = orxNULL;
  }

  /* Done! */
  return pstAnimSet;
}

/** Creates an animation set from config
 * @param[in]   _zConfigID                    Config ID
 * @return      orxANIMSET / orxNULL
 */
orxANIMSET *orxFASTCALL orxAnimSet_CreateFromConfig(const orxSTRING _zConfigID)
{
  orxANIMSET *pstResult;

  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_STATIC_FLAG_READY);

  /* Search for reference */
  pstResult = (orxANIMSET *)orxHashTable_Get(sstAnimSet.pstReferenceTable, orxString_ToCRC(_zConfigID));

  /* Found? */
  if(pstResult != orxNULL)
  {
    /* Increases counter */
    orxStructure_IncreaseCounter(pstResult);
  }
  else
  {
    /* Pushes section */
    if((orxConfig_HasSection(_zConfigID) != orxFALSE)
    && (orxConfig_PushSection(_zConfigID) != orxSTATUS_FAILURE))
    {
      orxU32  u32AnimCounter;

      /* Gets animation counter */
      u32AnimCounter = orxConfig_GetListCounter(orxANIMSET_KZ_CONFIG_ANIM_LIST);

      /* Creates animation set */
      pstResult = orxAnimSet_Create(u32AnimCounter);

      /* Valid? */
      if((pstResult != orxNULL)
      && ((pstResult->pstIDTable = orxHashTable_Create(orxANIMSET_KU32_ID_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN)) != orxNULL))
      {
        orxU32 i, u32LinkCounter;

        /* Updates status flags */
        orxStructure_SetFlags(pstResult, orxANIMSET_KU32_FLAG_ID_TABLE, orxANIMSET_KU32_FLAG_NONE);

        /* Stores its reference key */
        pstResult->zReference = orxConfig_GetCurrentSection();

        /* Protects it */
        orxConfig_ProtectSection(pstResult->zReference, orxTRUE);

        /* For all animations */
        for(i = 0; i < u32AnimCounter; i++)
        {
          const orxSTRING zAnimName;

          /* Gets its name */
          zAnimName = orxConfig_GetListString(orxANIMSET_KZ_CONFIG_ANIM_LIST, i);

          /* Valid? */
          if((zAnimName != orxNULL) && (zAnimName != orxSTRING_EMPTY))
          {
            orxANIM  *pstAnim;
            orxU32    u32AnimID;

            /* Creates it */
            pstAnim = orxAnim_CreateFromConfig(zAnimName);

            /* Valid? */
            if(pstAnim != orxNULL)
            {
              /* Adds it to set */
              u32AnimID = orxAnimSet_AddAnim(pstResult, pstAnim);

              /* Sets its owner */
              orxStructure_SetOwner(pstAnim, pstResult);

              /* Adds it to ID table */
              orxHashTable_Add(pstResult->pstIDTable, orxString_ToCRC(orxAnim_GetName(pstAnim)), (void *) orxANIMSET_CAST_HELPER (u32AnimID + 1));
            }
          }
        }

        /* Optimizes its ID table */
        orxHashTable_Optimize(pstResult->pstIDTable);

        /* For all links */
        for(i = 0, u32LinkCounter = orxConfig_GetListCounter(orxANIMSET_KZ_CONFIG_LINK_LIST); i < u32LinkCounter; i++)
        {
          const orxSTRING zLinkName;

          /* Gets link name */
          orxConfig_SelectSection(_zConfigID);
          zLinkName = orxConfig_GetListString(orxANIMSET_KZ_CONFIG_LINK_LIST, i);

          /* Valid? */
          if((zLinkName != orxNULL) && (zLinkName != orxSTRING_EMPTY))
          {
            const orxSTRING zSrcAnim;
            const orxSTRING zDstAnim;
            orxU32          u32SrcAnim, u32DstAnim, u32Link;

            /* Selects corresponding section */
            orxConfig_SelectSection(zLinkName);

            /* Gets source & destination anim names */
            zSrcAnim = orxConfig_GetString(orxANIMSET_KZ_CONFIG_LINK_SOURCE);
            zDstAnim = orxConfig_GetString(orxANIMSET_KZ_CONFIG_LINK_DESTINATION);

            /* Gets source & destination anim IDs */
            u32SrcAnim = ((orxU32) orxANIMSET_CAST_HELPER orxHashTable_Get(pstResult->pstIDTable, orxString_ToCRC(zSrcAnim)) - 1);
            u32DstAnim = ((orxU32) orxANIMSET_CAST_HELPER orxHashTable_Get(pstResult->pstIDTable, orxString_ToCRC(zDstAnim)) - 1);

            /* Valid? */
            if((u32SrcAnim != orxU32_UNDEFINED) && (u32DstAnim != orxU32_UNDEFINED))
            {
              /* Adds link */
              u32Link = orxAnimSet_AddLink(pstResult, u32SrcAnim, u32DstAnim);

              /* Valid? */
              if(u32Link != orxU32_UNDEFINED)
              {
                orxCHAR   acBuffer[64];
                orxSTRING zProperty;

                /* Gets lower case property */
                acBuffer[sizeof(acBuffer) - 1] = orxCHAR_NULL;
                zProperty = orxString_LowerCase(orxString_NCopy(acBuffer, orxConfig_GetString(orxANIMSET_KZ_CONFIG_LINK_PROPERTY), sizeof(acBuffer) - 1));

                /* Immediate link? */
                if(orxString_SearchString(zProperty, orxANIMSET_KZ_IMMEDIATE) != orxNULL)
                {
                  /* Updates link property */
                  orxAnimSet_SetLinkProperty(pstResult, u32Link, orxANIMSET_KU32_LINK_FLAG_IMMEDIATE_CUT, orxTRUE);
                }
                /* Auto reset? */
                if(orxString_SearchString(zProperty, orxANIMSET_KZ_CLEAR_TARGET) != orxNULL)
                {
                  /* Updates link property */
                  orxAnimSet_SetLinkProperty(pstResult, u32Link, orxANIMSET_KU32_LINK_FLAG_CLEAR_TARGET, orxTRUE);
                }

                /* Has priority? */
                if(orxConfig_HasValue(orxANIMSET_KZ_CONFIG_LINK_PRIORITY) != orxFALSE)
                {
                  /* Updates link priority */
                  orxAnimSet_SetLinkProperty(pstResult, u32Link, orxANIMSET_KU32_LINK_FLAG_PRIORITY, orxConfig_GetU32(orxANIMSET_KZ_CONFIG_LINK_PRIORITY));
                }
              }
            }
            else
            {
              /* Logs message */
              orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Animset [%s]: Couldn't add link %s <[%s] -> [%s]>, please check its declaration.", _zConfigID, orxConfig_GetCurrentSection(), zSrcAnim, zDstAnim);
            }
          }
        }

        /* Adds it to reference table */
        orxHashTable_Add(sstAnimSet.pstReferenceTable, orxString_ToCRC(pstResult->zReference), pstResult);

        /* Updates status flags */
        orxStructure_SetFlags(pstResult, orxANIMSET_KU32_FLAG_INTERNAL | orxANIMSET_KU32_FLAG_REFERENCED, orxANIMSET_KU32_FLAG_NONE);
      }

      /* Pops previous section */
      orxConfig_PopSection();
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Couldn't find config section named (%s).", _zConfigID);

      /* Updates result */
      pstResult = orxNULL;
    }
  }

  /* Done! */
  return pstResult;
}

/** Deletes an AnimSet
 * @param[in]		_pstAnimSet													AnimSet to delete
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnimSet_Delete(orxANIMSET *_pstAnimSet)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimSet);

  /* Decreases counter */
  orxStructure_DecreaseCounter(_pstAnimSet);

  /* Not referenced? */
  if(orxStructure_GetRefCounter(_pstAnimSet) == 0)
  {
    /* Cleans members */
    orxAnimSet_RemoveAllAnims(_pstAnimSet);
    orxAnimSet_DeleteLinkTable(_pstAnimSet->pstLinkTable);

    /* Is referenced? */
    if(orxStructure_TestFlags(_pstAnimSet, orxANIMSET_KU32_FLAG_REFERENCED) != orxFALSE)
    {
      /* Removes it from reference table */
      orxHashTable_Remove(sstAnimSet.pstReferenceTable, orxString_ToCRC(_pstAnimSet->zReference));
    }

    /* Has ID table? */
    if(orxStructure_TestFlags(_pstAnimSet, orxANIMSET_KU32_FLAG_ID_TABLE) != orxFALSE)
    {
      /* Deletes it */
      orxHashTable_Delete(_pstAnimSet->pstIDTable);
    }

    /* Frees anim pointer array */
    orxMemory_Free(_pstAnimSet->pastAnim);

    /* Has reference? */
    if(_pstAnimSet->zReference != orxNULL)
    {
      /* Unprotects it */
      orxConfig_ProtectSection(_pstAnimSet->zReference, orxFALSE);
    }

    /* Deletes structure */
    orxStructure_Delete(_pstAnimSet);
  }
  else
  {
    /* Referenced by others */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Adds a reference to an AnimSet
 * @param[in]		_pstAnimSet													Concerned AnimSet
 */
void orxFASTCALL    orxAnimSet_AddReference(orxANIMSET *_pstAnimSet)
{
  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimSet);

  /* Locks animset */
  orxStructure_SetFlags(_pstAnimSet, orxANIMSET_KU32_FLAG_REFERENCE_LOCK, orxANIMSET_KU32_FLAG_NONE);

  /* Updates reference counter */
  orxStructure_IncreaseCounter(_pstAnimSet);

  return;
}

/** Removes a reference from an AnimSet
 * @param[in]		_pstAnimSet													Concerned AnimSet
 */
void orxFASTCALL    orxAnimSet_RemoveReference(orxANIMSET *_pstAnimSet)
{
  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimSet);

  /* Updates reference counter */
  orxStructure_DecreaseCounter(_pstAnimSet);

  /* No extra reference left? */
  if(orxStructure_GetRefCounter(_pstAnimSet) <= 1)
  {
    /* Unlocks animset */
    orxStructure_SetFlags(_pstAnimSet, orxANIMSET_KU32_FLAG_NONE, orxANIMSET_KU32_FLAG_REFERENCE_LOCK);
  }

  return;
}

/** Clones an AnimSet Link Table
 * @param[in]		_pstAnimSet													AnimSet to clone
 * @return An internally allocated clone of the AnimSet
 */
orxANIMSET_LINK_TABLE *orxFASTCALL orxAnimSet_CloneLinkTable(const orxANIMSET *_pstAnimSet)
{
  orxANIMSET_LINK_TABLE *pstLinkTable;

  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimSet);

  /* Creates a new link table */
  pstLinkTable = orxAnimSet_CreateLinkTable((orxU32)(_pstAnimSet->pstLinkTable->u16TableSize));

  /* Was allocated? */
  if(pstLinkTable != orxNULL)
  {
    /* Copies it */
    orxAnimSet_CopyLinkTable(pstLinkTable, _pstAnimSet->pstLinkTable);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Failed to create link table.");

    /* Can't process */
    pstLinkTable = orxNULL;
  }

  /* Done! */
  return pstLinkTable;
}

/** Deletes a Link Table
 * @param[in]		_pstAnimSet													AnimSet to delete (should have been created using the clone function)
 */
void orxFASTCALL    orxAnimSet_DeleteLinkTable(orxANIMSET_LINK_TABLE *_pstLinkTable)
{
  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstLinkTable != orxNULL);

  /* Frees memory*/
  orxMemory_Free(_pstLinkTable->au32LinkArray);
  orxMemory_Free(_pstLinkTable->au8LoopArray);
  orxMemory_Free(_pstLinkTable);

  return;
}

/** Adds an Anim to an AnimSet
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @param[in]		_pstAnim														Anim to add
 * @return Anim ID in the specified AnimSet
 */
orxU32 orxFASTCALL orxAnimSet_AddAnim(orxANIMSET *_pstAnimSet, orxANIM *_pstAnim)
{
  orxU32 u32Counter, u32Size, u32Index;
  orxU32 u32Result;

  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimSet);
  orxSTRUCTURE_ASSERT(_pstAnim);

  /* Gets storage size & counter */
  u32Size     = orxAnimSet_GetAnimStorageSize(_pstAnimSet);
  u32Counter  = orxAnimSet_GetAnimCounter(_pstAnimSet);

  /* Not locked? */
  if(orxStructure_TestFlags(_pstAnimSet, orxANIMSET_KU32_FLAG_REFERENCE_LOCK) == orxFALSE)
  {
    /* Is there free room? */
    if(u32Counter < u32Size)
    {
      /* Finds the first empty slot */
      for(u32Index = 0; u32Index < u32Size; u32Index++)
      {
        if(_pstAnimSet->pastAnim[u32Index] == (orxANIM*)orxNULL)
        {
          break;
        }
      }

      /* Checks slot found */
      orxASSERT(u32Index < u32Size);

      /* Adds the extra animation */
      _pstAnimSet->pastAnim[u32Index] = _pstAnim;

      /* Updates Animation reference counter */
      orxStructure_IncreaseCounter(_pstAnim);

      /* Updates Animation counter */
      orxAnimSet_IncreaseAnimCounter(_pstAnimSet);

      /* Gets result ID */
      u32Result = u32Index;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "No free room to add anim.");

      /* Can't process */
      u32Result = orxU32_UNDEFINED;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Animset is locked, can't add anim.");

    /* Can't process */
    u32Result = orxU32_UNDEFINED;
  }

  /* Done! */
  return u32Result;
}

/** Removes an Anim from an AnimSet
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @param[in]		_u32AnimID												  ID of the anim to remove
 * @return 			orxSTATUS_SUCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnimSet_RemoveAnim(orxANIMSET *_pstAnimSet, orxU32 _u32AnimID)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimSet);

  /* Not locked? */
  if(orxStructure_TestFlags(_pstAnimSet, orxANIMSET_KU32_FLAG_REFERENCE_LOCK) == orxFALSE)
  {
    orxU32 u32AnimIndex;

    /* Gets animation index */
    u32AnimIndex = _u32AnimID;

    /* Animation found for the given ID? */
    if(_pstAnimSet->pastAnim[u32AnimIndex] != orxNULL)
    {
      /* Updates counter */
      orxAnimSet_DecreaseAnimCounter(_pstAnimSet);

      /* Updates animation reference counter */
      orxStructure_DecreaseCounter((_pstAnimSet->pastAnim[u32AnimIndex]));

      /* Was internally allocated? */
      if(orxStructure_TestFlags(_pstAnimSet, orxANIMSET_KU32_FLAG_INTERNAL) != orxFALSE)
      {
        /* Removes its owner */
        orxStructure_SetOwner(_pstAnimSet->pastAnim[u32AnimIndex], orxNULL);

        /* Deletes animset */
        orxAnim_Delete(_pstAnimSet->pastAnim[u32AnimIndex]);
      }

      /* Removes animation */
      _pstAnimSet->pastAnim[u32AnimIndex] = orxNULL;

      /* Cleans link table for this animation */
      orxAnimSet_CleanLinkTable(_pstAnimSet->pstLinkTable, u32AnimIndex);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Invalid index(%d) given.", u32AnimIndex);

      /* Failed */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Animset is locked, can't remove anim.");

    /* Failed */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Removes all Anim from the AnimSet
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @return 			orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnimSet_RemoveAllAnims(orxANIMSET *_pstAnimSet)
{
  orxU32    u32Counter;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimSet);

  /* Not locked? */
  if(orxStructure_TestFlags(_pstAnimSet, orxANIMSET_KU32_FLAG_REFERENCE_LOCK) == orxFALSE)
  {
    orxU32 i;

    /* Gets animation counter */
    u32Counter = orxAnimSet_GetAnimCounter(_pstAnimSet);

    /* Until there are no animation left */
    for(i = 0, eResult = orxSTATUS_SUCCESS; (i < u32Counter) && (eResult != orxSTATUS_FAILURE); i++)
    {
      eResult = orxAnimSet_RemoveAnim(_pstAnimSet, i);
    }

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Animset is locked, can't remove all anims.");

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Adds a link between two Anims of the AnimSet
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @param[in]		_u32SrcAnim													Source Anim of the link
 * @param[in]		_u32DstAnim													Destination Anim of the link
 * @return 			ID of the created link / orxU32_UNDEFINED
 */
orxU32 orxFASTCALL orxAnimSet_AddLink(orxANIMSET *_pstAnimSet, orxU32 _u32SrcAnim, orxU32 _u32DstAnim)
{
  orxU32 u32Size, u32Link, u32Index;
  orxU32 u32Result = orxU32_UNDEFINED;

  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimSet);

  /* Gets storage size */
  u32Size = orxAnimSet_GetAnimStorageSize(_pstAnimSet);

  /* Checks anim index validity */
  orxASSERT(_u32SrcAnim < u32Size);
  orxASSERT(_u32DstAnim < u32Size);

  /* Not locked? */
  if(orxStructure_TestFlags(_pstAnimSet, orxANIMSET_KU32_FLAG_REFERENCE_LOCK) == orxFALSE)
  {
    orxANIMSET_LINK_TABLE *pstLinkTable;

    /* Gets link table */
    pstLinkTable  = _pstAnimSet->pstLinkTable;

    /* Computes link index */
    u32Index      = (_u32SrcAnim * u32Size) + _u32DstAnim;

    /* Gets link */
    u32Link       = orxAnimSet_GetLinkTableLink(pstLinkTable, u32Index);

    /* Isn't there already a link? */
    if((u32Link & orxANIMSET_KU32_LINK_FLAG_LINK) == orxANIMSET_KU32_LINK_FLAG_NONE)
    {
      /* Adds link */
      u32Link = (orxANIMSET_KU32_LINK_FLAG_LINK | orxANIMSET_KU32_LINK_FLAG_PATH)
              | (0x00000001 << orxANIMSET_KU32_LINK_SHIFT_LENGTH)
              | (orxANIMSET_KU32_LINK_DEFAULT_PRIORITY << orxANIMSET_KU32_LINK_SHIFT_PRIORITY)
              | (_u32DstAnim << orxANIMSET_KU32_LINK_SHIFT_ANIM);

      /* Stores it */
      orxAnimSet_SetLinkTableLink(pstLinkTable, u32Index, u32Link);

      /* Updates counter */
      pstLinkTable->u16LinkCounter++;

      /* Animset has to be computed again */
      orxAnimSet_SetLinkTableFlag(pstLinkTable, orxANIMSET_KU32_LINK_TABLE_FLAG_DIRTY, orxANIMSET_KU32_LINK_TABLE_FLAG_NONE);

      /* Gets link ID */
      u32Result = u32Index;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Anims [%s] & [%s] are already linked, couldn't add another link.", orxAnim_GetName(orxAnimSet_GetAnim(_pstAnimSet, _u32SrcAnim)), orxAnim_GetName(orxAnimSet_GetAnim(_pstAnimSet, _u32DstAnim)));

      /* Link not added */
      u32Result = orxU32_UNDEFINED;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Animset [%s] is locked, couldn't add link.", _pstAnimSet->zReference);

    /* Link not added */
    u32Result = orxU32_UNDEFINED;
  }

  /* Done! */
  return u32Result;
}

/** Removes a link from the AnimSet
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @param[in]		_u32LinkID												  ID of the link
 * @return			orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnimSet_RemoveLink(orxANIMSET *_pstAnimSet, orxU32 _u32LinkID)
{
  orxU32    u32Size;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimSet);

  /* Not locked? */
  if(orxStructure_TestFlags(_pstAnimSet, orxANIMSET_KU32_FLAG_REFERENCE_LOCK) == orxFALSE)
  {
    orxANIMSET_LINK_TABLE *pstLinkTable;

    /* Gets link table */
    pstLinkTable = _pstAnimSet->pstLinkTable;

    /* Gets storage size */
    u32Size = orxAnimSet_GetAnimStorageSize(_pstAnimSet);

    /* Checks link index validity */
    orxASSERT(_u32LinkID < u32Size * u32Size);

    /* Link found? */
    if(orxAnimSet_GetLinkTableLink(pstLinkTable, _u32LinkID) & orxANIMSET_KU32_LINK_FLAG_LINK)
    {
      /* Updates link table */
      orxAnimSet_SetLinkTableLink(pstLinkTable, _u32LinkID, orxANIMSET_KU32_LINK_DEFAULT_NONE);
      orxAnimSet_SetLinkTableLinkProperty(pstLinkTable, _u32LinkID, orxANIMSET_KU32_LINK_FLAG_PRIORITY, orxU32_UNDEFINED);

      /* Animset has to be computed again */
      orxAnimSet_SetLinkTableFlag(pstLinkTable, orxANIMSET_KU32_LINK_TABLE_FLAG_DIRTY, orxANIMSET_KU32_LINK_TABLE_FLAG_NONE);

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "No link found to remove.");

      /* Can't process */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Animset is locked, can't remove link.");

    /* Can't process */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets a direct link between two Anims, if exists
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @param[in]		_u32SrcAnim													ID of the source Anim
 * @param[in]		_u32DstAnim												  ID of the destination Anim
 * @return			ID of the direct link, orxU32_UNDEFINED if none
 */
orxU32 orxFASTCALL orxAnimSet_GetLink(const orxANIMSET *_pstAnimSet, orxU32 _u32SrcAnim, orxU32 _u32DstAnim)
{
  orxU32                  u32Index, u32Size;
  orxANIMSET_LINK_TABLE  *pstLinkTable;
  orxU32                  u32Result = orxU32_UNDEFINED;

  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimSet);

  /* Gets storage size */
  u32Size = orxAnimSet_GetAnimStorageSize(_pstAnimSet);

  /* Checks anim index validity */
  orxASSERT(_u32SrcAnim < u32Size);
  orxASSERT(_u32DstAnim < u32Size);

  /* Computes link index */
  u32Index = (_u32SrcAnim * u32Size) + _u32DstAnim;

  /* Gets link table */
  pstLinkTable = _pstAnimSet->pstLinkTable;

  /* Is there a link? */
  if(orxAnimSet_GetLinkTableLink(pstLinkTable, u32Index) & orxANIMSET_KU32_LINK_FLAG_LINK)
  {
    /* Gets link ID */
    u32Result = u32Index;
  }
  else
  {
    /* No link found */
    u32Result = orxU32_UNDEFINED;
  }

  /* Done! */
  return u32Result;
}

/** Sets a link property
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @param[in]		_u32LinkID												  ID of the concerned link
 * @param[in]		_u32Property												ID of the property to set
 * @param[in]		_32Value														Value of the property to set
 * @return			orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnimSet_SetLinkProperty(orxANIMSET *_pstAnimSet, orxU32 _u32LinkID, orxU32 _u32Property, orxU32 _u32Value)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimSet);
  orxASSERT(_u32LinkID < orxAnimSet_GetAnimStorageSize(_pstAnimSet) * orxAnimSet_GetAnimStorageSize(_pstAnimSet));

  /* Not locked? */
  if(orxStructure_TestFlags(_pstAnimSet, orxANIMSET_KU32_FLAG_REFERENCE_LOCK) == orxFALSE)
  {
    /* Gets work done */
    eResult = orxAnimSet_SetLinkTableLinkProperty(_pstAnimSet->pstLinkTable, _u32LinkID, _u32Property, _u32Value);

    /* Changes occurred? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Added loop counter? */
      if(_u32Property == orxANIMSET_KU32_LINK_FLAG_LOOP_COUNTER)
      {
        /* Link table should be locally stored by animation pointers now */
        orxStructure_SetFlags(_pstAnimSet, orxANIMSET_KU32_FLAG_NONE, orxANIMSET_KU32_FLAG_LINK_STATIC);
      }
    }
  }
  else
  {
    /* Can't process */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets a link property
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @param[in]		_u32LinkID 												  ID of the concerned link
 * @param[in]		_u32Property												ID of the property to get
 * @return			Property value / orx32_Undefined
 */
orxU32 orxFASTCALL orxAnimSet_GetLinkProperty(const orxANIMSET *_pstAnimSet, orxU32 _u32LinkID, orxU32 _u32Property)
{
  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimSet);

  /* Returns property */
  return(orxAnimSet_GetLinkTableLinkProperty(_pstAnimSet->pstLinkTable, _u32LinkID, _u32Property));
}

/** Computes active Anim given current and destination Anim IDs & a relative timestamp
 * @param[in]   _pstAnimSet                         Concerned AnimSet
 * @param[in]   _u32SrcAnim                         Source (current) Anim ID
 * @param[in]   _u32DstAnim                         Destination Anim ID, if none (auto mode) set it to orxU32_UNDEFINED
 * @param[in,out] _pfTime                           Pointer to the current timestamp relative to the source Anim (time elapsed since the beginning of this anim)
 * @param[in,out] _pstLinkTable                     Anim Pointer link table (updated if AnimSet link table isn't static, when using loop counters for example)
 * @param[out] _pbCut                               Animation has been cut
 * @param[out] _pbClearTarget                       Animation has requested a target clearing
 * @return Current Anim ID. If it's not the source one, _pu32Time will contain the new timestamp, relative to the new Anim
*/
orxU32 orxFASTCALL orxAnimSet_ComputeAnim(orxANIMSET *_pstAnimSet, orxU32 _u32SrcAnim, orxU32 _u32DstAnim, orxFLOAT *_pfTime, orxANIMSET_LINK_TABLE *_pstLinkTable, orxBOOL *_pbCut, orxBOOL *_pbClearTarget)
{
  orxANIMSET_LINK_TABLE  *pstWorkTable;
  orxU32                  u32Result = _u32SrcAnim;

  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimSet);
  orxASSERT(_pfTime != orxNULL);
  orxASSERT(_pbCut != orxNULL);
  orxASSERT(_pbClearTarget != orxNULL);
  orxASSERT(_u32SrcAnim < orxAnimSet_GetAnimCounter(_pstAnimSet));
  orxASSERT((_u32DstAnim < orxAnimSet_GetAnimCounter(_pstAnimSet)) || (_u32DstAnim == orxU32_UNDEFINED));

  /* Gets Link Table */
  if(orxStructure_TestFlags(_pstAnimSet, orxANIMSET_KU32_FLAG_LINK_STATIC) == orxFALSE)
  {
    /* Use animation pointer local one */
    pstWorkTable = _pstLinkTable;
  }
  else
  {
    /* Uses common static one */
    pstWorkTable = _pstAnimSet->pstLinkTable;
  }

  /* Checks working link table */
  orxASSERT(pstWorkTable != orxNULL);

  /* Computes link table if needed */
  if(orxAnimSet_ComputeLinkTable(pstWorkTable) != orxSTATUS_FAILURE)
  {
    orxU32  u32Anim, u32LinkIndex, u32LinkProperty, u32RoutingAnim, u32TargetAnim;

    /* Defaults to not cut */
    *_pbCut = orxFALSE;

    /* Gets current animation */
    u32Anim = _u32SrcAnim;

    /* Gets routing animation in simulation mode */
    u32RoutingAnim = orxAnimSet_ComputeNextAnim(pstWorkTable, u32Anim, _u32DstAnim, orxTRUE);

    /* Valid? */
    if(u32RoutingAnim != orxU32_UNDEFINED)
    {
      /* Gets link index */
      u32LinkIndex = ((orxU32)(pstWorkTable->u16TableSize) * u32Anim) + u32RoutingAnim;

      /* Gets immediate cut property */
      u32LinkProperty = orxAnimSet_GetLinkTableLinkProperty(pstWorkTable, u32LinkIndex, orxANIMSET_KU32_LINK_FLAG_IMMEDIATE_CUT);

      /* Updates cut status */
      *_pbCut = (u32LinkProperty != orxU32_UNDEFINED) ? (orxBOOL)u32LinkProperty : orxFALSE;
    }

    /* Should cut? */
    if(*_pbCut != orxFALSE)
    {
      /* Get next animation according to destination aim */
      u32TargetAnim = orxAnimSet_ComputeNextAnim(pstWorkTable, u32Anim, _u32DstAnim, orxFALSE);

      /* Resets time stamp */
      *_pfTime = orxFLOAT_0;

      /* Gets link index */
      u32LinkIndex = ((orxU32)(pstWorkTable->u16TableSize) * u32Anim) + u32TargetAnim;

      /* Gets clear target property */
      u32LinkProperty = orxAnimSet_GetLinkTableLinkProperty(pstWorkTable, u32LinkIndex, orxANIMSET_KU32_LINK_FLAG_CLEAR_TARGET);

      /* Updates target clearing status */
      *_pbClearTarget = (u32LinkProperty != orxU32_UNDEFINED) ? (orxBOOL)u32LinkProperty : orxFALSE;

      /* Updates result */
      u32Result = u32TargetAnim;
    }
    else
    {
      orxFLOAT fLength;

      /* Defaults to no target clearing */
      *_pbClearTarget = orxFALSE;

      /* Gets current animation duration */
      fLength = orxAnim_GetLength(_pstAnimSet->pastAnim[u32Anim]);

      /* Next animation? */
      if((fLength == orxFLOAT_0) || (*_pfTime > fLength))
      {
        /* Get next animation */
        u32TargetAnim = orxAnimSet_ComputeNextAnim(pstWorkTable, u32Anim, _u32DstAnim, orxFALSE);

        /* Updates timestamp */
        *_pfTime -= fLength;

        /* Has next animation? */
        if((u32TargetAnim != orxU32_UNDEFINED)
        && ((fLength > orxFLOAT_0)
         || (u32TargetAnim != _u32SrcAnim)))
        {
          /* Gets link index */
          u32LinkIndex = ((orxU32)(pstWorkTable->u16TableSize) * u32Anim) + u32TargetAnim;

          /* Gets clear target property */
          u32LinkProperty = orxAnimSet_GetLinkTableLinkProperty(pstWorkTable, u32LinkIndex, orxANIMSET_KU32_LINK_FLAG_CLEAR_TARGET);

          /* Updates target clearing status */
          *_pbClearTarget |= (u32LinkProperty != orxU32_UNDEFINED) ? (orxBOOL)u32LinkProperty : orxFALSE;

          /* Stores current result ID */
          u32Result = u32TargetAnim;
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Couldn't compute next animation.");

          /* Not found */
          u32Result = orxU32_UNDEFINED;
        }
      }
    }
  }

  /* Done! */
  return u32Result;
}

/** Finds next Anim given current and destination Anim IDs
 * @param[in]   _pstAnimSet                         Concerned AnimSet
 * @param[in]   _u32SrcAnim                         Source (current) Anim ID
 * @param[in]   _u32DstAnim                         Destination Anim ID, if none (auto mode) set it to orxU32_UNDEFINED
 * @return Next Anim ID if found, orxU32_UNDEFINED otherwise
*/
orxU32 orxFASTCALL orxAnimSet_FindNextAnim(orxANIMSET *_pstAnimSet, orxU32 _u32SrcAnim, orxU32 _u32DstAnim)
{
  orxU32 u32Result = orxU32_UNDEFINED;

  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimSet);
  orxASSERT(_u32SrcAnim < orxAnimSet_GetAnimCounter(_pstAnimSet));
  orxASSERT((_u32DstAnim < orxAnimSet_GetAnimCounter(_pstAnimSet)) || (_u32DstAnim == orxU32_UNDEFINED));

  /* Computes link table if needed */
  if(orxAnimSet_ComputeLinkTable(_pstAnimSet->pstLinkTable) != orxSTATUS_FAILURE)
  {
    /* Updates result */
    u32Result = orxAnimSet_ComputeNextAnim(_pstAnimSet->pstLinkTable, _u32SrcAnim, _u32DstAnim, orxTRUE);
  }

  /* Done! */
  return u32Result;
}

/** AnimSet Anim get accessor
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @param[in]		_u32AnimID                          Anim ID
 * @return Anim pointer / orxNULL
 */
orxANIM *orxFASTCALL orxAnimSet_GetAnim(const orxANIMSET *_pstAnimSet, orxU32 _u32AnimID)
{
  orxU32 u32Counter;
  orxANIM *pstAnim = orxNULL;

  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimSet);

  /* Gets counter */
  u32Counter = orxAnimSet_GetAnimCounter(_pstAnimSet);

  /* Is index valid? */
  if(_u32AnimID < u32Counter)
  {
    /* Gets Animation */
    pstAnim = _pstAnimSet->pastAnim[_u32AnimID];
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Invalid index in animset.");
  }

  /* Done! */
  return pstAnim;
}

/** AnimSet Anim storage size get accessor
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @return			AnimSet Storage size / orxU32_UNDEFINED
 */
orxU32 orxFASTCALL orxAnimSet_GetAnimStorageSize(const orxANIMSET *_pstAnimSet)
{
  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimSet);

  /* Gets storage size */
  return(orxStructure_GetFlags(_pstAnimSet, orxANIMSET_KU32_MASK_SIZE) >> orxANIMSET_KU32_ID_SHIFT_SIZE);
}

/** AnimSet Anim counter get accessor
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @return			Anim counter / orxU32_UNDEFINED
 */
orxU32 orxFASTCALL orxAnimSet_GetAnimCounter(const orxANIMSET *_pstAnimSet)
{
  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimSet);

  /* Gets counter */
  return(orxStructure_GetFlags(_pstAnimSet, orxANIMSET_KU32_MASK_COUNTER) >> orxANIMSET_KU32_ID_SHIFT_COUNTER);
}

/** Gets animation ID from name
 * @param[in]   _pstAnimSet                         Concerned AnimSet
 * @param[in]   _zAnimName                          Animation name (config's section)
 * @return Anim ID / orxU32_UNDEFINED
 */
orxU32 orxFASTCALL orxAnimSet_GetAnimIDFromName(const orxANIMSET *_pstAnimSet, const orxSTRING _zAnimName)
{
  orxU32 u32Result = orxU32_UNDEFINED;

  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimSet);

  /* Has ID table? */
  if(orxStructure_TestFlags(_pstAnimSet, orxANIMSET_KU32_FLAG_ID_TABLE) != orxFALSE)
  {
    /* Gets corresponding ID */
    u32Result = ((orxU32) orxANIMSET_CAST_HELPER orxHashTable_Get(_pstAnimSet->pstIDTable, orxString_ToCRC(_zAnimName)) - 1);
  }

  /* Done! */
  return u32Result;
}

/** AnimSet name get accessor
 * @param[in]   _pstAnimSet                         Concerned AnimSet
 * @return      orxSTRING / orxSTRING_EMPTY
 */
const orxSTRING orxFASTCALL orxAnimSet_GetName(const orxANIMSET *_pstAnimSet)
{
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(sstAnimSet.u32Flags & orxANIMSET_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimSet);

  /* Updates result */
  zResult = _pstAnimSet->zReference;

  /* Done! */
  return zResult;
}


#ifdef __orxMSVC__

  #pragma warning(default : 4311 4312)

#endif /* __orxMSVC__ */
