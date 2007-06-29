/**
 * @file orxAnim.c
 * 
 * Animation (Data) module
 * 
 */

 /***************************************************************************
 orxAnim.c
 Animation (Data) module
 
 begin                : 12/02/2004
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/


#include "anim/orxAnim.h"

#include "debug/orxDebug.h"
#include "memory/orxMemory.h"


/** Module flags
 */
#define orxANIM_KU32_FLAG_NONE              0x00000000  /**< No flags */

#define orxANIM_KU32_FLAG_READY             0x00000001  /**< Ready flag */


/** orxANIM ID flags/masks/shifts
 */
#define orxANIM_KU32_ID_FLAG_NONE           0x00000000  /**< No ID flags */

#define orxANIM_KU32_ID_MASK_SIZE           0x000000FF  /**< Size ID mask */
#define orxANIM_KU32_ID_MASK_COUNTER        0x0000FF00  /**< Counter ID mask */
#define orxANIM_KU32_ID_MASK_FLAGS          0xFFFF0000  /**< Flags ID mask */

#define orxANIM_KS32_ID_SHIFT_SIZE          0           /**< Size ID shift */
#define orxANIM_KS32_ID_SHIFT_COUNTER       8           /**< Counter ID shift */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Animation structure
 */
struct __orxANIM_t
{
  orxSTRUCTURE  stStructure;                /**< Public structure, first structure member : 16 */
  orxU32        u32IDFlags;                 /**< ID flags : 20 */
  orxTEXTURE  **pastTexture;                /**< Used atom pointer array : 24*/
  orxU32       *au32TimeStamp;              /**< TimeStamp array : 28 */

  orxPAD(28)
};

/** Static structure
 */
typedef struct __orxANIM_STATIC_t
{
  orxU32 u32Flags;                          /**< Control flags : 4 */

} orxANIM_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
orxSTATIC orxANIM_STATIC sstAnim;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Finds a texture index given a timestamp
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _u32TimeStamp   Desired timestamp
 * @return      Texture index / orxU32_Undefined
 */
orxSTATIC orxU32 orxAnim_FindTextureIndex(orxCONST orxANIM *_pstAnim, orxU32 _u32TimeStamp)
{
  orxU32 u32Counter, u32MaxIndex, u32MinIndex, u32Index;

  /* Checks */
  orxASSERT(_pstAnim != orxNULL);

  /* Gets counter */
  u32Counter = orxAnim_GetTextureCounter(_pstAnim);

  /* Is animation not empty? */
  if(u32Counter != 0)
  {
    /* Dichotomic search */
    for(u32MinIndex = 0, u32MaxIndex = u32Counter - 1, u32Index = u32MaxIndex >> 1;
        u32MinIndex < u32MaxIndex;
        u32Index = (u32MinIndex + u32MaxIndex) >> 1)
    {
      /* Updates search range */
      if(_u32TimeStamp > _pstAnim->au32TimeStamp[u32Index])
      {
        u32MinIndex = u32Index + 1;
      }
      else
      {
        u32MaxIndex = u32Index;
      }
    }

    /* Not found? */
    if(_pstAnim->au32TimeStamp[u32Index] < _u32TimeStamp)
    {
      /* !!! MSG !!! */

      /* Not defined */
      u32Index = orxU32_Undefined;
    }
  }
  /* Empty animation */
  else
  {
    /* !!! MSG !!! */

    /* Not defined */
    u32Index = orxU32_Undefined;
  }

  /* Done! */
  return u32Index;
}

/** Sets an animation storage size
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _u32Size        Desired size
 */
orxSTATIC orxINLINE orxVOID orxAnim_SetStorageSize(orxANIM *_pstAnim, orxU32 _u32Size)
{
  /* Checks */
  orxASSERT(_pstAnim != orxNULL);
  orxASSERT(_u32Size <= orxANIM_KU32_ATOM_MAX_NUMBER);

  /* Updates storage size */
  orxAnim_SetFlags(_pstAnim, _u32Size << orxANIM_KS32_ID_SHIFT_SIZE, orxANIM_KU32_ID_MASK_SIZE);

  return;
}  

/** Sets an animation internal atom counter
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _u32AtomCounter Desired atom counter
 */
orxSTATIC orxINLINE orxVOID orxAnim_SetAtomCounter(orxANIM *_pstAnim, orxU32 _u32AtomCounter)
{
  /* Checks */
  orxASSERT(_pstAnim != orxNULL);
  orxASSERT(_u32AtomCounter <= orxAnim_GetTextureStorageSize(_pstAnim));

  /* Updates counter */
  orxAnim_SetFlags(_pstAnim, _u32AtomCounter << orxANIM_KS32_ID_SHIFT_COUNTER, orxANIM_KU32_ID_MASK_COUNTER);

  return;
}

/** Increases an animation internal atom counter
 * @param[in]   _pstAnim        Concerned animation
 */
orxSTATIC orxINLINE orxVOID orxAnim_IncreaseAtomCounter(orxANIM *_pstAnim)
{
  orxREGISTER orxU32 u32AtomCounter;

  /* Checks */
  orxASSERT(_pstAnim != orxNULL);

  /* Gets texture counter */
  u32AtomCounter = orxAnim_GetTextureCounter(_pstAnim);

  /* Updates texture counter*/
  orxAnim_SetAtomCounter(_pstAnim, u32AtomCounter + 1);

  return;
}  

/** Increases an animation internal atom counter
 * @param[in]   _pstAnim        Concerned animation
 */
orxSTATIC orxINLINE orxVOID orxAnim_DecreaseAtomCounter(orxANIM *_pstAnim)
{
  orxREGISTER orxU32 u32AtomCounter;

  /* Checks */
  orxASSERT(_pstAnim != orxNULL);

  /* Gets texture counter */
  u32AtomCounter = orxAnim_GetTextureCounter(_pstAnim);

  /* Updates texture counter*/
  orxAnim_SetAtomCounter(_pstAnim, u32AtomCounter - 1);

  return;
}  

/** Deletes all animations
 */
orxSTATIC orxINLINE orxVOID orxAnim_DeleteAll()
{
  orxREGISTER orxANIM *pstAnim;

  /* Gets first anim */
  pstAnim = (orxANIM *)orxStructure_GetFirst(orxSTRUCTURE_ID_ANIM);

  /* Non empty? */
  while(pstAnim != orxNULL)
  {
    /* Deletes Animation */
    orxAnim_Delete(pstAnim);

    /* Gets first Animation */
    pstAnim = (orxANIM *)orxStructure_GetFirst(orxSTRUCTURE_ID_ANIM);
  }

  return;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Animation module setup
 */
orxVOID orxAnim_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_ANIM, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_ANIM, orxMODULE_ID_TIME);
  orxModule_AddDependency(orxMODULE_ID_ANIM, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_ANIM, orxMODULE_ID_TEXTURE);

  return;
}

/** Inits the Animation system
 */
orxSTATUS orxAnim_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;
  
  /* Not already Initialized? */
  if(!(sstAnim.u32Flags & orxANIM_KU32_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Set(&sstAnim, 0, sizeof(orxANIM_STATIC));

    /* Registers structure type */
    eResult = orxSTRUCTURE_REGISTER(orxSTRUCTURE_ID_ANIM, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxNULL);
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
    sstAnim.u32Flags = orxANIM_KU32_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return eResult;
}

/** Exits from the Animation system
 */
orxVOID orxAnim_Exit()
{
  /* Initialized? */
  if(sstAnim.u32Flags & orxANIM_KU32_FLAG_READY)
  {
    /* Deletes anim list */
    orxAnim_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_ANIM);

    /* Updates flags */
    sstAnim.u32Flags &= ~orxANIM_KU32_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}

/** Creates an empty animation
 * @param[in]   _u32IDFLags     ID flags for created animation
 * @param[in]   _u32Size        Number of atoms for this animation
 * @return      Created orxANIM / orxVOID
 */
orxANIM *orxFASTCALL orxAnim_Create(orxU32 _u32IDFlags, orxU32 _u32Size)
{
  orxANIM *pstAnim;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_FLAG_READY);
  orxASSERT(_u32Size <= orxANIM_KU32_ATOM_MAX_NUMBER);

  /* Creates anim */
  pstAnim = (orxANIM *)orxStructure_Create(orxSTRUCTURE_ID_ANIM);

  /* Valid? */
  if(pstAnim != orxNULL)
  {
    /* Inits flags */
    orxAnim_SetFlags(pstAnim, _u32IDFlags & orxANIM_KU32_ID_MASK_FLAGS, orxANIM_KU32_ID_MASK_FLAGS);

    /* 2D Animation? */
    if(_u32IDFlags & orxANIM_KU32_ID_FLAG_2D)
    {
      /* Allocates texture pointer array */
      pstAnim->pastTexture = (orxTEXTURE **)orxMemory_Allocate(_u32Size * sizeof(orxTEXTURE *), orxMEMORY_TYPE_MAIN);

      /* Not allocated? */
      if(pstAnim->pastTexture == orxNULL)
      {
        /* !!! MSG !!! */

        /* Frees partially allocated texture */
        orxMemory_Free(pstAnim);

        /* Returns nothing */
        return orxNULL;
      }

      /* Allocates timestamp array */
      pstAnim->au32TimeStamp = (orxU32 *)orxMemory_Allocate(_u32Size * sizeof(orxU32), orxMEMORY_TYPE_MAIN);

      /* Not allocated? */
      if(pstAnim->au32TimeStamp == orxNULL)
      {
        /* !!! MSG !!! */

        /* Frees partially allocated texture */
        orxMemory_Free(pstAnim->pastTexture);
        orxMemory_Free(pstAnim);

        /* Returns nothing */
        return orxNULL;
      }

      /* Cleans structure pointers */
      orxMemory_Set(pstAnim->pastTexture, 0, _u32Size * sizeof(orxTEXTURE *));
      orxMemory_Set(pstAnim->au32TimeStamp, 0, _u32Size * sizeof(orxU32)); 

      /* Sets storage size & counter */
      orxAnim_SetStorageSize(pstAnim, _u32Size);
      orxAnim_SetAtomCounter(pstAnim, 0);
    }
    /* Other Animation Type? */
    else
    {
      /* !!! MSG !!! */

      /* Frees partially allocated texture */
      orxMemory_Free(pstAnim);

      /* Returns nothing */
      return orxNULL;
    }
  }

  /* Done! */
  return pstAnim;
}

/** Deletes an animation
 * @param[in]   _pstAnim        Animation to delete
 */
orxSTATUS orxFASTCALL orxAnim_Delete(orxANIM *_pstAnim)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_FLAG_READY);
  orxASSERT(_pstAnim != orxNULL);

  /* Not referenced? */
  if(orxStructure_GetRefCounter((orxSTRUCTURE *)_pstAnim) == 0)
  {
    /* Cleans members */

    /* 2D Animation? */
    if(orxAnim_TestFlags(_pstAnim, orxANIM_KU32_ID_FLAG_2D))
    {
      /* Removes all textures */
      orxAnim_RemoveAllTextures(_pstAnim);
    }
    /* Other Animation Type? */
    else
    {
      /* !!! MSG !!! */
    }

    /* Deletes structure */
    orxStructure_Delete((orxSTRUCTURE *)_pstAnim);
  }
  else
  {
    /* !!! MSG !!! */
    
    /* Referenced by others */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Adds an atom to an animation
 * @param[in]   _pstAnim        Animation concerned
 * @param[in]   _pstTexture     Texture to add
 * @param[in]   _u32TimeStamp   Timestamp for this atom
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnim_AddTexture(orxANIM *_pstAnim, orxTEXTURE *_pstTexture, orxU32 _u32TimeStamp)
{
  orxU32 u32Counter, u32Size;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_FLAG_READY);
  orxASSERT(_pstAnim != orxNULL);
  orxASSERT(_pstTexture != orxNULL);
  orxASSERT(orxAnim_TestFlags(_pstAnim, orxANIM_KU32_ID_FLAG_2D) != orxFALSE);

  /* Gets storage size & counter */
  u32Size     = orxAnim_GetTextureStorageSize(_pstAnim);
  u32Counter  = orxAnim_GetTextureCounter(_pstAnim);

  /* Is there free room? */
  if(u32Counter < u32Size)
  {
    /* Adds the extra texture */
    _pstAnim->pastTexture[u32Counter] = _pstTexture;
    _pstAnim->au32TimeStamp[u32Counter] = _u32TimeStamp;

    /* Updates texture reference counter */
    orxStructure_IncreaseCounter((orxSTRUCTURE *)_pstTexture);

    /* Updates texture counter */
    orxAnim_IncreaseAtomCounter(_pstAnim);
  }
  else
  {
    /* !!! MSG !!! */

    return orxSTATUS_FAILURE;
  }

  /* Done! */
  return orxSTATUS_SUCCESS;
}

/** Removes last added atom from an animation
 * @param[in]   _pstAnim        Concerned animation
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnim_RemoveLastTexture(orxANIM *_pstAnim)
{
  orxU32 u32Counter;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_FLAG_READY);
  orxASSERT(_pstAnim != orxNULL);
  orxASSERT(orxAnim_TestFlags(_pstAnim, orxANIM_KU32_ID_FLAG_2D) != orxFALSE);

  /* Gets counter */
  u32Counter = orxAnim_GetTextureCounter(_pstAnim);

  /* Has texture? */
  if(u32Counter != 0)
  {
    /* Gets real index */
    u32Counter--;

    /* Updates counter */
    orxAnim_DecreaseAtomCounter(_pstAnim);

    /* Updates texture reference counter */
    orxStructure_DecreaseCounter((orxSTRUCTURE *)_pstAnim->pastTexture[u32Counter]);

    /* Removes the texture & cleans info */
    _pstAnim->pastTexture[u32Counter]   = orxNULL;
    _pstAnim->au32TimeStamp[u32Counter] = 0;
  }
  else
  {
    /* !!! MSG !!! */

    return orxSTATUS_FAILURE;
  }

  /* Done! */
  return orxSTATUS_SUCCESS;
}

/** Removes all atoms from an animation
 * @param[in]   _pstAnim        Concerned animation
 */
orxVOID orxFASTCALL orxAnim_RemoveAllTextures(orxANIM *_pstAnim)
{
  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_FLAG_READY);
  orxASSERT(_pstAnim != orxNULL);
  orxASSERT(orxAnim_TestFlags(_pstAnim, orxANIM_KU32_ID_FLAG_2D) != orxFALSE);

  /* Until there are no texture left */
  while(orxAnim_RemoveLastTexture(_pstAnim) != orxSTATUS_FAILURE);

  /* Done! */
  return;
}

/** Computes active atom
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _u32TimeStamp   TimeStamp for animation update
 * @return      Current orxTEXTURE / orxNULL
 */
orxTEXTURE *orxFASTCALL orxAnim_ComputeTexture(orxANIM *_pstAnim, orxU32 _u32TimeStamp)
{
  orxTEXTURE *pstTexture = orxNULL;
  orxU32 u32Index;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_FLAG_READY);
  orxASSERT(_pstAnim != orxNULL);
  orxASSERT(orxAnim_TestFlags(_pstAnim, orxANIM_KU32_ID_FLAG_2D) != orxFALSE);

  /* Finds corresponding texture index */
  u32Index = orxAnim_FindTextureIndex(_pstAnim, _u32TimeStamp);

  /* Found? */
  if(u32Index != orxU32_Undefined)
  {
    pstTexture = _pstAnim->pastTexture[u32Index];
  }

  return pstTexture;
}

/** Animation atom accessor
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _u32Index       Index of desired atom
 * @return      Desired orxTEXTURE / orxNULL
 */
orxTEXTURE *orxFASTCALL orxAnim_GetTexture(orxCONST orxANIM *_pstAnim, orxU32 _u32Index)
{
  orxU32 u32Counter;
  orxTEXTURE *pstTexture = orxNULL;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_FLAG_READY);
  orxASSERT(_pstAnim != orxNULL);
  orxASSERT(orxAnim_TestFlags(_pstAnim, orxANIM_KU32_ID_FLAG_2D) != orxFALSE);

  /* Gets counter */
  u32Counter = orxAnim_GetTextureCounter(_pstAnim);

  /* Is index valid? */
  if(_u32Index < u32Counter)
  {
    /* Gets texture */
    pstTexture = _pstAnim->pastTexture[_u32Index];
  }
  else
  {
    /* !!! MSG !!! */
  }

  return pstTexture;
}

/** Animation atom storage size accessor
 * @param[in]   _pstAnim        Concerned animation
 * @return      Animation storage size
 */
orxU32 orxFASTCALL orxAnim_GetTextureStorageSize(orxCONST orxANIM *_pstAnim)
{
  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_FLAG_READY);
  orxASSERT(_pstAnim != orxNULL);
  orxASSERT(orxAnim_TestFlags(_pstAnim, orxANIM_KU32_ID_FLAG_2D) != orxFALSE);

  /* Gets storage size */
  return((_pstAnim->u32IDFlags & orxANIM_KU32_ID_MASK_SIZE) >> orxANIM_KS32_ID_SHIFT_SIZE);
}  

/** Animation atom counter accessor
 * @param[in]   _pstAnim        Concerned animation
 * @return      Animation atom counter
 */
orxU32 orxFASTCALL orxAnim_GetTextureCounter(orxCONST orxANIM *_pstAnim)
{
  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_FLAG_READY);
  orxASSERT(_pstAnim != orxNULL);
  orxASSERT(orxAnim_TestFlags(_pstAnim, orxANIM_KU32_ID_FLAG_2D) != orxFALSE);

  /* Gets counter */
  return((_pstAnim->u32IDFlags & orxANIM_KU32_ID_MASK_COUNTER) >> orxANIM_KS32_ID_SHIFT_COUNTER);
}

/** Animation time length accessor
 * @param[in]   _pstAnim        Concerned animation
 * @return      Animation time length
 */
orxU32 orxFASTCALL orxAnim_GetLength(orxCONST orxANIM *_pstAnim)
{
  orxU32 u32Counter, u32Length = 0;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_FLAG_READY);
  orxASSERT(_pstAnim != orxNULL);

  /* 2D? */
  if(orxAnim_TestFlags(_pstAnim, orxANIM_KU32_ID_FLAG_2D) != orxFALSE)
  {
    /* Gets texture counter */
    u32Counter = orxAnim_GetTextureCounter(_pstAnim);

    /* Is animation non empty? */
    if(u32Counter != 0)
    {
      /* Gets length */
      u32Length = _pstAnim->au32TimeStamp[u32Counter - 1];
    }
  }
  else
  {
    /* !!! MSG !!! */

    return orxU32_Undefined;
  }

  return u32Length;
}

/** Animation flags test accessor
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _u32Flags       Flags to test
 * @return      orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxAnim_TestFlags(orxCONST orxANIM *_pstAnim, orxU32 _u32Flags)
{
  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_FLAG_READY);
  orxASSERT(_pstAnim != orxNULL);

  return((_pstAnim->u32IDFlags & _u32Flags) != orxANIM_KU32_FLAG_NONE);
}

/** Animation all flags test accessor
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _u32Flags       Flags to test
 * @return      orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxAnim_TestAllFlags(orxCONST orxANIM *_pstAnim, orxU32 _u32Flags)
{
  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_FLAG_READY);
  orxASSERT(_pstAnim != orxNULL);

  return((_pstAnim->u32IDFlags & _u32Flags) == _u32Flags);
}

/** Animation flag set accessor
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _u32AddFlags    Flags to add
 * @param[in]   _u32RemoveFlags Flags to remove
 */
orxVOID orxFASTCALL orxAnim_SetFlags(orxANIM *_pstAnim, orxU32 _u32AddFlags, orxU32 _u32RemoveFlags)
{
  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_FLAG_READY);
  orxASSERT(_pstAnim != orxNULL);

  /* Updates flags */
  _pstAnim->u32IDFlags &= ~_u32RemoveFlags;
  _pstAnim->u32IDFlags |= _u32AddFlags;

  return;
}
