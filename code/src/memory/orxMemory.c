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
 * @file orxMemory.c
 * @date 02/04/2005
 * @author bestel@arcallians.org
 *
 */


#include "memory/orxMemory.h"
#include "debug/orxDebug.h"


#define USE_DL_PREFIX
#include "malloc.c"


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

/** Memory module setup
 */
void orxFASTCALL orxMemory_Setup()
{
  /* Adds module dependencies */

  return;
}

/** Initializes memory allocation module
 */
orxSTATUS orxFASTCALL orxMemory_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

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

/** Exits from the memory module
 */
void orxFASTCALL orxMemory_Exit()
{
  /* Module initialized ? */
  if((sstMemory.u32Flags & orxMEMORY_KU32_STATIC_FLAG_READY) == orxMEMORY_KU32_STATIC_FLAG_READY)
  {
    /* Module uninitialized */
    sstMemory.u32Flags = orxMEMORY_KU32_STATIC_FLAG_NONE;
  }

  return;
}

/** Allocates a portion of memory in the system and returns a pointer on it
 * @param[in] _u32Size    size of the memory to allocate
 * @param[in] _eMemType   Memory zone where datas will be allocated
 * @return  returns a pointer on the memory allocated, or orxNULL if an error has occured
 */
void *orxFASTCALL orxMemory_Allocate(orxU32 _u32Size, orxMEMORY_TYPE _eMemType)
{
  /* Module initialized ? */
  orxASSERT((sstMemory.u32Flags & orxMEMORY_KU32_STATIC_FLAG_READY) == orxMEMORY_KU32_STATIC_FLAG_READY);

  /* Valid parameters ? */
  orxASSERT(_eMemType < orxMEMORY_TYPE_NUMBER);

  /* Returns system allocation function */
  return((void *)dlmalloc((size_t)_u32Size));
}

/** Frees a portion of memory allocated with orxMemory_Allocateate
 * @param[in] _pMem       Pointer on the memory allocated by orx
 */
void orxFASTCALL orxMemory_Free(void *_pMem)
{
  /* Module initialized ? */
  orxASSERT((sstMemory.u32Flags & orxMEMORY_KU32_STATIC_FLAG_READY) == orxMEMORY_KU32_STATIC_FLAG_READY);

  /* Valid parameters ? */
  orxASSERT(_pMem != orxNULL);

  /* System call to free memory */
  dlfree(_pMem);

  return;
}

/** Copies a portion of memory into another one
 * @param[out] _pDest     Destination pointer
 * @param[in] _pSrc       Pointer of memory from where data are read
 * @param[in] _u32Size    Size of data
 * @return returns a pointer on _pDest
 * @note if _pSrc and _pDest overlap, use orxMemory_Move instead
 */
void *orxFASTCALL orxMemory_Copy(void *_pDest, const void *_pSrc, orxU32 _u32Size)
{
  return((void *)memcpy(_pDest, _pSrc, (size_t)_u32Size));
}

/** Moves a portion of memory into another one
 * @param[out] _pDest     Destination pointer
 * @param[in] _pSrc       Pointer of memory from where data are read
 * @param[in] _u32Size    Size of data
 * @return returns a pointer on _pDest
 */
void *orxFASTCALL orxMemory_Move(void *_pDest, void *_pSrc, orxU32 _u32Size)
{
  return((void *)memmove(_pDest, _pSrc, (size_t)_u32Size));
}

/** Compares two portions of memory
 * @param[in] _pMem1      First potion to test
 * @param[in] _pMem2      Second portion to test
 * @param[in] _u32Size    Size of data to test
 * @return returns a value less, equals or greater that 0 if _pMem1 is respectively smaller, equal or greater than _pMem2
 */
orxU32 orxFASTCALL orxMemory_Compare(const void *_pMem1, const void *_pMem2, orxU32 _u32Size)
{
  return((orxU32)memcmp(_pMem1, _pMem2, (size_t)_u32Size));
}

/** Fills a portion of memory with _u32Data
 * @param[out] _pDest     Destination pointer
 * @param[in] _u8Data     Values of the data that will fill the memory
 * @param[in] _u32Size    Size of data
 * @return returns a pointer on _pDest
 */
void *orxFASTCALL orxMemory_Set(void *_pDest, orxU8 _u8Data, orxU32 _u32Size)
{
  return((void *)memset(_pDest, _u8Data, (size_t)_u32Size));
}

/** Fills a portion of memory with zeroes
 * @param[out] _pDest     Destination pointer
 * @param[in] _u32Size    Size of data
 * @return returns a pointer on _pDest
 */
void *orxFASTCALL orxMemory_Zero(void *_pDest, orxU32 _u32Size)
{
  return((void *)memset(_pDest, 0, (size_t)_u32Size));
}

/** Reallocates a portion of memory if the already allocated memory is not suffisant.
 * @param[in] _pMem	   Memory to reallocate.
 * @param[in] _u32Size Wanted size.
 * @return The pointer reallocated.
 */
void *orxFASTCALL orxMemory_Reallocate(void *_pMem, orxU32 _u32Size)
{
  return((void *)dlrealloc(_pMem, (size_t)_u32Size));
}
