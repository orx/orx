/**
 * @file orxGraphic.c
 * 
 * Graphic module
 * 
 */

 /***************************************************************************
 orxGraphic.c
 Graphic module
 
 begin                : 08/12/2003
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


#include "display/orxGraphic.h"

#include "debug/orxDebug.h"
#include "memory/orxMemory.h"


/** Module flags
 */

#define orxGRAPHIC_KU32_FLAG_NONE           0x00000000

#define orxGRAPHIC_KU32_FLAG_READY          0x00000001


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Graphic structure
 */
struct __orxGRAPHIC_t
{
  orxSTRUCTURE  stStructure;                /**< Public structure, first structure member : 16 */
  orxU32        u32IDFlags;                 /**< ID flags : 20 */
  orxSTRUCTURE *pstData;                    /**< Data structure : 24 */

  orxPAD(24)
};

/** Static structure
 */
typedef struct __orxGRAPHIC_STATIC_t
{
  orxU32 u32Flags;                          /**< Control flags : 4 */

} orxGRAPHIC_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
orxSTATIC orxGRAPHIC_STATIC sstGraphic;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Deletes all graphics
 */
orxSTATIC orxINLINE orxVOID orxGraphic_DeleteAll()
{
  orxREGISTER orxGRAPHIC *pstGraphic;

  /* Gets first graphic */
  pstGraphic = (orxGRAPHIC *)orxStructure_GetFirst(orxSTRUCTURE_ID_GRAPHIC);

  /* Non empty? */
  while(pstGraphic != orxNULL)
  {
    /* Deletes Graphic */
    orxGraphic_Delete(pstGraphic);

    /* Gets first Graphic */
    pstGraphic = (orxGRAPHIC *)orxStructure_GetFirst(orxSTRUCTURE_ID_GRAPHIC);
  }

  return;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Graphic module setup
 */
orxVOID orxGraphic_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_GRAPHIC, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_GRAPHIC, orxMODULE_ID_STRUCTURE);

  return;
}

/** Inits the Graphic module
 */
orxSTATUS orxGraphic_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;
  
  /* Not already Initialized? */
  if((sstGraphic.u32Flags & orxGRAPHIC_KU32_FLAG_READY) == orxGRAPHIC_KU32_FLAG_NONE)
  {
    /* Cleans static controller */
    orxMemory_Set(&sstGraphic, 0, sizeof(orxGRAPHIC_STATIC));

    /* Registers structure type */
    eResult = orxSTRUCTURE_REGISTER(orxSTRUCTURE_ID_GRAPHIC, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxNULL);
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
    sstGraphic.u32Flags = orxGRAPHIC_KU32_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return eResult;
}

/** Exits from the Graphic module
 */
orxVOID orxGraphic_Exit()
{
  /* Initialized? */
  if(sstGraphic.u32Flags & orxGRAPHIC_KU32_FLAG_READY)
  {
    /* Deletes graphic list */
    orxGraphic_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_GRAPHIC);

    /* Updates flags */
    sstGraphic.u32Flags &= ~orxGRAPHIC_KU32_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}

/** Creates an empty graphic
 * @return      Created orxGRAPHIC / orxNULL
 */
orxGRAPHIC *orxGraphic_Create()
{
  orxGRAPHIC *pstGraphic;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_FLAG_READY);

  /* Creates graphic */
  pstGraphic = (orxGRAPHIC *)orxStructure_Create(orxSTRUCTURE_ID_GRAPHIC);

  /* Valid? */
  if(pstGraphic != orxNULL)
  {
    /* Cleans it */
    orxMemory_Set(pstGraphic, 0, sizeof(orxGRAPHIC));
    
    /* Inits flags */
    orxGraphic_SetFlags(pstGraphic, orxGRAPHIC_KU32_ID_FLAG_NONE, orxGRAPHIC_KU32_ID_MASK_ALL);
  }

  /* Done! */
  return pstGraphic;
}

/** Deletes a graphic
 * @param[in]   _pstGraphic     Graphic to delete
 */
orxSTATUS orxFASTCALL orxGraphic_Delete(orxGRAPHIC *_pstGraphic)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_FLAG_READY);
  orxASSERT(_pstGraphic != orxNULL);

  /* Not referenced? */
  if(orxStructure_GetRefCounter((orxSTRUCTURE *)_pstGraphic) == 0)
  {
    /* Cleans data */
    orxGraphic_SetData(_pstGraphic, orxNULL);

    /* Deletes structure */
    orxStructure_Delete((orxSTRUCTURE *)_pstGraphic);
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

/** Sets graphic data
 * @param[in]   _pstGraphic     Graphic concerned
 * @param[in]   _pstData        Data structure to set / orxNULL
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxGraphic_SetData(orxGRAPHIC *_pstGraphic, orxSTRUCTURE *_pstData)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;
  
  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_FLAG_READY);
  orxASSERT(_pstGraphic != orxNULL);

  /* Had previously data? */
  if(_pstGraphic->pstData != orxNULL)
  {
    /* Updates structure reference counter */
    orxStructure_DecreaseCounter(_pstGraphic->pstData);
    
    /* Cleans reference */
    _pstGraphic->pstData = orxNULL;
  }

  /* Sets new data? */
  if(_pstData != orxNULL)
  {
    /* Stores it */
    _pstGraphic->pstData = _pstData;

    /* Updates structure reference counter */
    orxStructure_IncreaseCounter(_pstData);

    /* Is data a texture? */
    if(_pstData->eID == orxSTRUCTURE_ID_TEXTURE)
    {
      /* Updates ID flags */
      orxGraphic_SetFlags(_pstGraphic, orxGRAPHIC_KU32_ID_FLAG_2D, orxGRAPHIC_KU32_FLAG_NONE);
    }
    {
      /* !!! MSG !!! */
      
      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
    
    /* !!! TODO : Update internal flags given data type */
  }

  /* Done! */
  return eResult;
}

orxSTRUCTURE *orxFASTCALL orxGraphic_GetData(orxCONST orxGRAPHIC *_pstGraphic)
{
  orxSTRUCTURE *pstStructure;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_FLAG_READY);
  orxASSERT(_pstGraphic != orxNULL);

  /* Updates result */
  pstStructure = _pstGraphic->pstData;

  /* Done! */
  return pstStructure;
}

/** Graphic flags test accessor
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[in]   _u32Flags       Flags to test
 * @return      orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxGraphic_TestFlags(orxCONST orxGRAPHIC *_pstGraphic, orxU32 _u32Flags)
{
  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_FLAG_READY);
  orxASSERT(_pstGraphic != orxNULL);

  return((_pstGraphic->u32IDFlags & _u32Flags) != orxGRAPHIC_KU32_FLAG_NONE);
}

/** Graphic all flags test accessor
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[in]   _u32Flags       Flags to test
 * @return      orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxGraphic_TestAllFlags(orxCONST orxGRAPHIC *_pstGraphic, orxU32 _u32Flags)
{
  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_FLAG_READY);
  orxASSERT(_pstGraphic != orxNULL);

  return((_pstGraphic->u32IDFlags & _u32Flags) == _u32Flags);
}

/** Graphic flag set accessor
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[in]   _u32AddFlags    Flags to add
 * @param[in]   _u32RemoveFlags Flags to remove
 */
orxVOID orxFASTCALL orxGraphic_SetFlags(orxGRAPHIC *_pstGraphic, orxU32 _u32AddFlags, orxU32 _u32RemoveFlags)
{
  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_FLAG_READY);
  orxASSERT(_pstGraphic != orxNULL);

  /* Updates flags */
  _pstGraphic->u32IDFlags &= ~_u32RemoveFlags;
  _pstGraphic->u32IDFlags |= _u32AddFlags;

  return;
}
