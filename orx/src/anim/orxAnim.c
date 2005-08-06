/***************************************************************************
 orxAnim.c
 Animation (Data) module
 
 begin                : 12/02/2004
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


#include "anim/orxAnim.h"

#include "debug/orxDebug.h"
#include "memory/orxMemory.h"


/*
 * Platform independant defines
 */

#define orxANIM_KU32_FLAG_NONE              0x00000000

#define orxANIM_KU32_FLAG_READY             0x00000001


#define orxANIM_KU32_ID_FLAG_NONE           0x00000000

#define orxANIM_KU32_ID_MASK_SIZE           0x000000FF
#define orxANIM_KU32_ID_MASK_COUNTER        0x0000FF00
#define orxANIM_KU32_ID_MASK_FLAGS          0xFFFF0000


#define orxANIM_KS32_ID_SHIFT_SIZE          0
#define orxANIM_KS32_ID_SHIFT_COUNTER       8


/*
 * Animation structure
 */
struct __orxANIM_t
{
  /* Public structure, first structure member : 16 */
  orxSTRUCTURE stStructure;

  /* Id flags : 20 */
  orxU32 u32IDFlags;

  /* Used textures pointer array : 24 */
  orxTEXTURE **pastTexture;

  /* Timestamp array : 32 */
  orxU32 *au32Time;
};


/*
 * Static structure
 */
typedef struct __orxANIM_STATIC_t
{

  /* Control flags */
  orxU32 u32Flags;

} orxANIM_STATIC;


/*
 * Static data
 */
orxSTATIC orxANIM_STATIC sstAnim;


/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 orxAnim_FindTextureIndex
 Gets a texture index given a timestamp

 returns: texture index / orxU32_Undefined
 ***************************************************************************/
orxSTATIC orxU32 orxAnim_FindTextureIndex(orxANIM *_pstAnim, orxU32 _u32Time)
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
      if(_u32Time > _pstAnim->au32Time[u32Index])
      {
        u32MinIndex = u32Index + 1;
      }
      else
      {
        u32MaxIndex = u32Index;
      }
    }

    /* Not found? */
    if(_pstAnim->au32Time[u32Index] < _u32Time)
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

/***************************************************************************
 orxAnim_SetTextureStorageSize
 Sets 2D animation texture storage size.

 returns: orxVOID
 ***************************************************************************/
orxINLINE orxVOID orxAnim_SetTextureStorageSize(orxANIM *_pstAnim, orxU32 _u32Size)
{
  /* Checks */
  orxASSERT(_pstAnim != orxNULL);
  orxASSERT(_u32Size <= orxANIM_KS32_TEXTURE_MAX_NUMBER);

  /* Updates storage size */
  orxAnim_SetFlag(_pstAnim, _u32Size << orxANIM_KS32_ID_SHIFT_SIZE, orxANIM_KU32_ID_MASK_SIZE);

  return;
}  

/***************************************************************************
 orxAnim_SetTextureCounter
 Sets a 2D animation internal texture counter.

 returns: orxVOID
 ***************************************************************************/
orxINLINE orxVOID orxAnim_SetTextureCounter(orxANIM *_pstAnim, orxU32 _u32TextureCounter)
{
  /* Checks */
  orxASSERT(_pstAnim != orxNULL);
  orxASSERT(_u32TextureCounter <= orxAnim_GetTextureStorageSize(_pstAnim));

  /* Updates counter */
  orxAnim_SetFlag(_pstAnim, _u32TextureCounter << orxANIM_KS32_ID_SHIFT_COUNTER, orxANIM_KU32_ID_MASK_COUNTER);

  return;
}

/***************************************************************************
 orxAnim_IncreaseTextureCounter
 Increases a 2D animation internal texture counter.

 returns: orxVOID
 ***************************************************************************/
orxINLINE orxVOID orxAnim_IncreaseTextureCounter(orxANIM *_pstAnim)
{
  orxREGISTER orxU32 u32TextureCounter;

  /* Checks */
  orxASSERT(_pstAnim != orxNULL);

  /* Gets texture counter */
  u32TextureCounter = orxAnim_GetTextureCounter(_pstAnim);

  /* Updates texture counter*/
  orxAnim_SetTextureCounter(_pstAnim, u32TextureCounter + 1);

  return;
}  

/***************************************************************************
 orxAnim_DecreaseTextureCounter
 Decreases a 2D animation internal texture counter.

 returns: orxVOID
 ***************************************************************************/
orxINLINE orxVOID orxAnim_DecreaseTextureCounter(orxANIM *_pstAnim)
{
  orxREGISTER orxU32 u32TextureCounter;

  /* Checks */
  orxASSERT(_pstAnim != orxNULL);

  /* Gets texture counter */
  u32TextureCounter = orxAnim_GetTextureCounter(_pstAnim);

  /* Updates texture counter*/
  orxAnim_SetTextureCounter(_pstAnim, u32TextureCounter - 1);

  return;
}  

/***************************************************************************
 orxAnim_DeleteAll
 Deletes all Animations.

 returns: orxVOID
 ***************************************************************************/
orxSTATIC orxVOID orxAnim_DeleteAll()
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
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/


/***************************************************************************
 orxAnim_Init
 Inits Animation system.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxAnim_Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Not already Initialized? */
  if(!(sstAnim.u32Flags & orxANIM_KU32_FLAG_READY))
  {
    orxSTRUCTURE_REGISTER_INFO stRegisterInfo;

    /* Cleans static controller */
    orxMemory_Set(&sstAnim, 0, sizeof(orxANIM_STATIC));

    /* Registers structure type */
    stRegisterInfo.eStorageType = orxSTRUCTURE_STORAGE_TYPE_LINKLIST;
    stRegisterInfo.u32Size      = sizeof(orxANIM);
    stRegisterInfo.eMemoryType  = orxMEMORY_TYPE_MAIN;
    stRegisterInfo.pfnUpdate    = orxNULL;

    eResult = orxStructure_Register(orxSTRUCTURE_ID_ANIM, &stRegisterInfo);
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
    sstAnim.u32Flags = orxANIM_KU32_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxAnim_Exit
 Exits from the Animation system.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxAnim_Exit()
{
  /* Initialized? */
  if(sstAnim.u32Flags & orxANIM_KU32_FLAG_READY)
  {
    /* Deletes anim list */
    orxAnim_DeleteAll();

    /* Updates flags */
    sstAnim.u32Flags &= ~orxANIM_KU32_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}

/***************************************************************************
 orxAnim_Create
 Creates an empty Animation, given its id type and storage size (<= orxANIM_KS32_TEXTURE_MAX_NUMBER).

 returns: Created anim.
 ***************************************************************************/
orxANIM *orxAnim_Create(orxU32 _u32IDFlag, orxU32 _u32Size)
{
  orxANIM *pstAnim;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_FLAG_READY);
  orxASSERT(_u32Size <= orxANIM_KS32_TEXTURE_MAX_NUMBER);

  /* Creates anim */
  pstAnim = (orxANIM *)orxMemory_Allocate(sizeof(orxANIM), orxMEMORY_TYPE_MAIN);

  /* Non null? */
  if(pstAnim != orxNULL)
  {
    /* Cleans it */
    orxMemory_Set(pstAnim, 0, sizeof(orxANIM));
    
    /* Inits structure */
    if(orxStructure_Setup((orxSTRUCTURE *)pstAnim, orxSTRUCTURE_ID_ANIM) != orxSTATUS_SUCCESS)
    {
      /* !!! MSG !!! */

      /* Frees partially allocated texture */
      orxMemory_Free(pstAnim);

      /* Returns nothing */
      return orxNULL;
    }

    /* Inits flags */
    orxAnim_SetFlag(pstAnim, _u32IDFlag & orxANIM_KU32_ID_MASK_FLAGS, orxANIM_KU32_ID_MASK_FLAGS);

    /* 2D Animation? */
    if(_u32IDFlag & orxANIM_KU32_ID_FLAG_2D)
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
      pstAnim->au32Time = (orxU32 *)orxMemory_Allocate(_u32Size * sizeof(orxU32), orxMEMORY_TYPE_MAIN);

      /* Not allocated? */
      if(pstAnim->au32Time == orxNULL)
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
      orxMemory_Set(pstAnim->au32Time, 0, _u32Size * sizeof(orxU32)); 

      /* Sets storage size & counter */
      orxAnim_SetTextureStorageSize(pstAnim, _u32Size);
      orxAnim_SetTextureCounter(pstAnim, 0);
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

/***************************************************************************
 orxAnim_Delete
 Deletes an Animation.

 returns: orxVOID
 ***************************************************************************/
orxSTATUS orxAnim_Delete(orxANIM *_pstAnim)
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
    if(orxAnim_TestFlag(_pstAnim, orxANIM_KU32_ID_FLAG_2D))
    {
      /* Removes all textures */
      orxAnim_RemoveAllTextures(_pstAnim);
    }
    /* Other Animation Type? */
    else
    {
      /* !!! MSG !!! */
    }

    /* Cleans structure */
    orxStructure_Clean((orxSTRUCTURE *)_pstAnim);

    /* Frees anim memory */
    orxMemory_Free(_pstAnim);
  }
  else
  {
    /* !!! MSG !!! */
    
    /* Referenced by others */
    eResult = orxSTATUS_FAILED;
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxAnim_AddTexture
 Adds a texture to a 2D Animation.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxAnim_AddTexture(orxANIM *_pstAnim, orxTEXTURE *_pstTexture, orxU32 _u32Time)
{
  orxU32 u32Counter, u32Size;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_FLAG_READY);
  orxASSERT(_pstAnim != orxNULL);
  orxASSERT(_pstTexture != orxNULL);
  orxASSERT(orxAnim_TestFlag(_pstAnim, orxANIM_KU32_ID_FLAG_2D) != orxFALSE);

  /* Gets storage size & counter */
  u32Size     = orxAnim_GetTextureStorageSize(_pstAnim);
  u32Counter  = orxAnim_GetTextureCounter(_pstAnim);

  /* Is there free room? */
  if(u32Counter < u32Size)
  {
    /* Adds the extra texture */
    _pstAnim->pastTexture[u32Counter] = _pstTexture;
    _pstAnim->au32Time[u32Counter] = _u32Time;

    /* Updates texture reference counter */
    orxStructure_IncreaseCounter((orxSTRUCTURE *)_pstTexture);

    /* Updates texture counter */
    orxAnim_IncreaseTextureCounter(_pstAnim);
  }
  else
  {
    /* !!! MSG !!! */

    return orxSTATUS_FAILED;
  }

  /* Done! */
  return orxSTATUS_SUCCESS;
}

/***************************************************************************
 orxAnim_RemoveTexture
 Removes last added texture from a 2D Animation.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxAnim_RemoveTexture(orxANIM *_pstAnim)
{
  orxU32 u32Counter;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_FLAG_READY);
  orxASSERT(_pstAnim != orxNULL);
  orxASSERT(orxAnim_TestFlag(_pstAnim, orxANIM_KU32_ID_FLAG_2D) != orxFALSE);

  /* Gets counter */
  u32Counter = orxAnim_GetTextureCounter(_pstAnim);

  /* Has texture? */
  if(u32Counter != 0)
  {
    /* Gets real index */
    u32Counter--;

    /* Updates counter */
    orxAnim_DecreaseTextureCounter(_pstAnim);

    /* Updates texture reference counter */
    orxStructure_DecreaseCounter((orxSTRUCTURE *)_pstAnim->pastTexture[u32Counter]);

    /* Removes the texture & cleans info */
    _pstAnim->pastTexture[u32Counter]   = orxNULL;
    _pstAnim->au32Time[u32Counter] = 0;
  }
  else
  {
    /* !!! MSG !!! */

    return orxSTATUS_FAILED;
  }

  /* Done! */
  return orxSTATUS_SUCCESS;
}

/***************************************************************************
 orxAnim_RemoveAllTextures
 Removes all referenced textures from a 2D Animation.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxAnim_RemoveAllTextures(orxANIM *_pstAnim)
{
  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_FLAG_READY);
  orxASSERT(_pstAnim != orxNULL);
  orxASSERT(orxAnim_TestFlag(_pstAnim, orxANIM_KU32_ID_FLAG_2D) != orxFALSE);

  /* Until there are no texture left */
  while(orxAnim_RemoveTexture(_pstAnim) != orxSTATUS_FAILED);

  /* Done! */
  return;
}

/***************************************************************************
 orxAnim_GetTexture
 Texture used by a 2D Animation get accessor, given its index.

 returns: orxTEXTURE *
 ***************************************************************************/
orxTEXTURE *orxAnim_GetTexture(orxANIM *_pstAnim, orxU32 _u32Index)
{
  orxU32 u32Counter;
  orxTEXTURE *pstTexture = orxNULL;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_FLAG_READY);
  orxASSERT(_pstAnim != orxNULL);
  orxASSERT(orxAnim_TestFlag(_pstAnim, orxANIM_KU32_ID_FLAG_2D) != orxFALSE);

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

/***************************************************************************
 orxAnim_ComputeTexture
 Computes active 2D texture given a timestamp.

 returns: orxU32 texture index
 ***************************************************************************/
orxTEXTURE *orxAnim_ComputeTexture(orxANIM *_pstAnim, orxU32 _u32Time)
{
  orxTEXTURE *pstTexture = orxNULL;
  orxU32 u32Index;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_FLAG_READY);
  orxASSERT(_pstAnim != orxNULL);
  orxASSERT(orxAnim_TestFlag(_pstAnim, orxANIM_KU32_ID_FLAG_2D) != orxFALSE);

  /* Finds corresponding texture index */
  u32Index = orxAnim_FindTextureIndex(_pstAnim, _u32Time);

  /* Found? */
  if(u32Index != orxU32_Undefined)
  {
    pstTexture = _pstAnim->pastTexture[u32Index];
  }

  return pstTexture;
}


/* *** Structure accessors *** */


/***************************************************************************
 orxAnim_GetTextureCounter
 2D Animation texture counter get accessor.

 returns: orxU32 counter
 ***************************************************************************/
orxU32 orxAnim_GetTextureCounter(orxANIM *_pstAnim)
{
  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_FLAG_READY);
  orxASSERT(_pstAnim != orxNULL);
  orxASSERT(orxAnim_TestFlag(_pstAnim, orxANIM_KU32_ID_FLAG_2D) != orxFALSE);

  /* Gets counter */
  return((_pstAnim->u32IDFlags & orxANIM_KU32_ID_MASK_COUNTER) >> orxANIM_KS32_ID_SHIFT_COUNTER);
}

/***************************************************************************
 orxAnim_GetTextureStorageSize
 2D Animation texture storage size get accessor.

 returns: orxU32 storage size
 ***************************************************************************/
orxU32 orxAnim_GetTextureStorageSize(orxANIM *_pstAnim)
{
  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_FLAG_READY);
  orxASSERT(_pstAnim != orxNULL);
  orxASSERT(orxAnim_TestFlag(_pstAnim, orxANIM_KU32_ID_FLAG_2D) != orxFALSE);

  /* Gets storage size */
  return((_pstAnim->u32IDFlags & orxANIM_KU32_ID_MASK_SIZE) >> orxANIM_KS32_ID_SHIFT_SIZE);
}  

/***************************************************************************
 orxAnim_GetLength
 Animation length get accessor.

 returns: orxU32 length
 ***************************************************************************/
orxU32 orxAnim_GetLength(orxANIM *_pstAnim)
{
  orxU32 u32Counter, u32Length = 0;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_FLAG_READY);
  orxASSERT(_pstAnim != orxNULL);

  /* 2D? */
  if(orxAnim_TestFlag(_pstAnim, orxANIM_KU32_ID_FLAG_2D) != orxFALSE)
  {
    /* Gets texture counter */
    u32Counter = orxAnim_GetTextureCounter(_pstAnim);

    /* Is animation non empty? */
    if(u32Counter != 0)
    {
      /* Gets length */
      u32Length = _pstAnim->au32Time[u32Counter - 1];
    }
  }
  else
  {
    /* !!! MSG !!! */

    return orxU32_Undefined;
  }

  return u32Length;
}

/***************************************************************************
 orxAnim_TestFlag
 Animation flag test accessor.

 returns: orxBOOL
 ***************************************************************************/
orxBOOL orxAnim_TestFlag(orxANIM *_pstAnim, orxU32 _u32Flag)
{
  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_FLAG_READY);
  orxASSERT(_pstAnim != orxNULL);

  return((_pstAnim->u32IDFlags & _u32Flag) == _u32Flag);
}

/***************************************************************************
 orxAnim_SetFlag
 Animation flag get/set accessor.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxAnim_SetFlag(orxANIM *_pstAnim, orxU32 _u32AddFlags, orxU32 _u32RemoveFlags)
{
  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_FLAG_READY);
  orxASSERT(_pstAnim != orxNULL);

  /* Updates flags */
  _pstAnim->u32IDFlags &= ~_u32RemoveFlags;
  _pstAnim->u32IDFlags |= _u32AddFlags;

  return;
}
