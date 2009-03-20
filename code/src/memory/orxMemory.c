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
 * @file orxMemory.c
 * @date 02/04/2005
 * @author bestel@arcallians.org
 *
 * @todo
 * - Optimizations
 * - Manage the memory instead of use the access to OS allocation
 * - Manage memory alignment
 * - Allow to specifiy memory starting address & maximum size for each memory type
 */

#include "memory/orxMemory.h"
#include "debug/orxDebug.h"

#include <stdlib.h>
#include <string.h>

#define orxMEMORY_KU32_STATIC_FLAG_NONE   0x00000000  /**< No flags have been set */
#define orxMEMORY_KU32_STATIC_FLAG_READY  0x00000001  /**< The module has been initialized */

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/
typedef struct __orxMEMORY_STATIC_t
{
  orxU32 u32Flags;   /**< Flags set by the memory module */
} orxMEMORY_STATIC;

/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/
static orxMEMORY_STATIC sstMemory;

/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/***************************************************************************
 orxMemory_Setup
 Memory module setup.

 returns: nothing
 ***************************************************************************/
void orxMemory_Setup()
{
  /* Adds module dependencies */

  return;
}

/** Initialize memory allocation module
 * @todo Really initialize the memory to be managed by the module and not OS
 */
orxSTATUS orxMemory_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* No dependencies for this module */

  /* Module not already initialized ? */
  /* Not already Initialized? */
  if(!(sstMemory.u32Flags & orxMEMORY_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstMemory, sizeof(orxMEMORY_STATIC));

    /* Module initialized */
    sstMemory.u32Flags = orxMEMORY_KU32_STATIC_FLAG_READY;

    /* Success */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_MEMORY, "Tried to initialize memory module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done */
  return eResult;
}

/** Uninitialize memory allocation module
 */
void orxMemory_Exit()
{
  /* Module initialized ? */
  if((sstMemory.u32Flags & orxMEMORY_KU32_STATIC_FLAG_READY) == orxMEMORY_KU32_STATIC_FLAG_READY)
  {
    /* Module uninitialized */
    sstMemory.u32Flags = orxMEMORY_KU32_STATIC_FLAG_NONE;
  }

  return;
}

/** Get the an aligned data size
 * @param[in] _u32OriginalValue (ex: 70)
 * @param[in] _u32AlignValue (ex : 32) (The value has to be a power of 2 and > 0) (ex : 32)
 * @return the aligned _u32OriginalValue on _u32AlignValue (ex : will return 96 for previous values)
 */
orxU32 orxFASTCALL orxMemory_GetAlign(orxU32 _u32OriginalValue, orxU32 _u32AlignValue)
{
  /* The align value has to be a power of 2 and > 0 */
  orxASSERT(_u32AlignValue > 0);
  orxASSERT((_u32AlignValue & (_u32AlignValue - 1)) == 0);

  return(_orxALIGN(_u32OriginalValue, _u32AlignValue));
}

/** Allocate a portion of memory in the system and returns a pointer on it
 * @param[in] _u32Size    size of the memory to allocate
 * @param[in] _eMemType   Memory zone where datas will be allocated
 * @return  returns a pointer on the memory allocated, or orxNULL if an error has occured
 * @todo Use the memory managed by orxMemory (initialized with orxMemory_Init())
 */
void *orxFASTCALL orxMemory_Allocate(orxU32 _u32Size, orxMEMORY_TYPE _eMemType)
{
  /* Module initialized ? */
  orxASSERT((sstMemory.u32Flags & orxMEMORY_KU32_STATIC_FLAG_READY) == orxMEMORY_KU32_STATIC_FLAG_READY);

  /* Valid parameters ? */
  orxASSERT(_eMemType < orxMEMORY_TYPE_NUMBER);

  /* Returns system allocation function */
  return((void *)malloc(_u32Size));
}

/** Free a portion of memory allocated with orxMemory_Allocateate
 * @param[in] _pMem       Pointer on the memory allocated by orx
 * @todo Use the memory managed by orxMemory (not OS)
 */
void orxFASTCALL    orxMemory_Free(void *_pMem)
{
  /* Module initialized ? */
  orxASSERT((sstMemory.u32Flags & orxMEMORY_KU32_STATIC_FLAG_READY) == orxMEMORY_KU32_STATIC_FLAG_READY);

  /* Valid parameters ? */
  orxASSERT(_pMem != orxNULL);

  /* System call to free memory */
  free(_pMem);

  return;
}

/** Copy a portion of memory into another one
 * @param[out] _pDest     Destination pointer
 * @param[in] _pSrc       Pointer of memory from where data are read
 * @param[in] _u32Size    Size of data
 * @return returns a pointer on _pDest
 * @note if _pSrc and _pDest overlap, use orxMemory_Move instead
 */
void *orxFASTCALL orxMemory_Copy(void *_pDest, const void *_pSrc, orxU32 _u32Size)
{
  return((void *)memcpy(_pDest, _pSrc, _u32Size));
}

/** Copy a portion of memory into another one
 * @param[out] _pDest     Destination pointer
 * @param[in] _pSrc       Pointer of memory from where data are read
 * @param[in] _u32Size    Size of data
 * @return returns a pointer on _pDest
 */
void *orxFASTCALL orxMemory_Move(void *_pDest, void *_pSrc, orxU32 _u32Size)
{
  return((void *)memmove(_pDest, _pSrc, _u32Size));
}

/** Compare two portion of memory
 * @param[in] _pMem1      First potion to test
 * @param[in] _pMem2      Second portion to test
 * @param[in] _u32Size    Size of data to test
 * @return returns a velue less, equals or greater that 0 if _pMem1 is respectively smaller, equal or greater than _pMem2
 */
orxU32 orxFASTCALL orxMemory_Compare(const void *_pMem1, const void *_pMem2, orxU32 _u32Size)
{
  return((orxU32)memcmp(_pMem1, _pMem2, _u32Size));
}

/** Fill a portion of memory with _u32Data
 * @param[out] _pDest     Destination pointer
 * @param[in] _u8Data     Values of the data that will fill the memory
 * @param[in] _u32Size    Size of data
 * @return returns a pointer on _pDest
 */
void *orxFASTCALL orxMemory_Set(void *_pDest, orxU8 _u8Data, orxU32 _u32Size)
{
  return((void *)memset(_pDest, _u8Data, _u32Size));
}

/** Fill a portion of memory with zeroes
 * @param[out] _pDest     Destination pointer
 * @param[in] _u32Size    Size of data
 * @return returns a pointer on _pDest
 */
void *orxFASTCALL orxMemory_Zero(void *_pDest, orxU32 _u32Size)
{
  return((void *)memset(_pDest, 0, _u32Size));
}

/** Realloc a portion of memory if the already allocated memory is not suffisant.
 * @param[in] _pMem	   Memory to reallocate.
 * @param[in] _u32Size Wanted size.
 * @return The pointer reallocated.
 */
orxDLLAPI void *orxFASTCALL orxMemory_Reallocate(void *_pMem, orxU32 _u32Size)
{
  return((void *)realloc(_pMem, _u32Size));
}
