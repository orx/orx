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

#define orxGRAPHIC_KU32_STATIC_FLAG_NONE    0x00000000

#define orxGRAPHIC_KU32_STATIC_FLAG_READY   0x00000001


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Graphic structure
 */
struct __orxGRAPHIC_t
{
  orxSTRUCTURE  stStructure;                /**< Public structure, first structure member : 16 */
  orxSTRUCTURE *pstData;                    /**< Data structure : 20 */
  orxVECTOR     vPivot;                     /**< Pivot : 36 */

  orxPAD(36)
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
  orxModule_AddDependency(orxMODULE_ID_GRAPHIC, orxMODULE_ID_TEXTURE);

  return;
}

/** Inits the Graphic module
 */
orxSTATUS orxGraphic_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;
  
  /* Not already Initialized? */
  if((sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY) == orxGRAPHIC_KU32_STATIC_FLAG_NONE)
  {
    /* Cleans static controller */
    orxMemory_Set(&sstGraphic, 0, sizeof(orxGRAPHIC_STATIC));

    /* Registers structure type */
    eResult = orxSTRUCTURE_REGISTER(GRAPHIC, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxNULL);
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
    sstGraphic.u32Flags = orxGRAPHIC_KU32_STATIC_FLAG_READY;
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
  if(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY)
  {
    /* Deletes graphic list */
    orxGraphic_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_GRAPHIC);

    /* Updates flags */
    sstGraphic.u32Flags &= ~orxGRAPHIC_KU32_STATIC_FLAG_READY;
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
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);

  /* Creates graphic */
  pstGraphic = (orxGRAPHIC *)orxStructure_Create(orxSTRUCTURE_ID_GRAPHIC);

  /* Valid? */
  if(pstGraphic != orxNULL)
  {
    /* Inits flags */
    orxStructure_SetFlags(pstGraphic, orxGRAPHIC_KU32_FLAG_NONE, orxGRAPHIC_KU32_MASK_ALL);
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
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);

  /* Not referenced? */
  if(orxStructure_GetRefCounter(_pstGraphic) == 0)
  {
    /* Cleans data */
    orxGraphic_SetData(_pstGraphic, orxNULL);

    /* Deletes structure */
    orxStructure_Delete(_pstGraphic);
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
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);

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
    if(orxSTRUCTURE_GET_POINTER(_pstData, TEXTURE) != orxNULL)
    {
      /* Updates flags */
      orxStructure_SetFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_2D, orxGRAPHIC_KU32_STATIC_FLAG_NONE);
    }
    else
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

/** Gets graphic data
 * @param[in]   _pstGraphic     Concerned graphic
 * @return      OrxSTRUCTURE / orxNULL
 */
orxSTRUCTURE *orxFASTCALL orxGraphic_GetData(orxCONST orxGRAPHIC *_pstGraphic)
{
  orxSTRUCTURE *pstStructure;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);

  /* Updates result */
  pstStructure = _pstGraphic->pstData;

  /* Done! */
  return pstStructure;
}

/** Sets graphic pivot
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[in]   _pvPivot        Pivot to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxGraphic_SetPivot(orxGRAPHIC *_pstGraphic, orxCONST orxVECTOR *_pvPivot)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);
  orxASSERT(_pvPivot != orxNULL);

  /* Stores pivot */
  orxVector_Copy(&(_pstGraphic->vPivot), _pvPivot);

  /* Done! */
  return eResult;
}

/** Gets graphic pivot
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[out]  _pvPivot        Graphic pivot
 * @return      orxPIVOT / orxNULL
 */
orxVECTOR *orxFASTCALL orxGraphic_GetPivot(orxCONST orxGRAPHIC *_pstGraphic, orxVECTOR *_pvPivot)
{
  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);
  orxASSERT(_pvPivot != orxNULL);

  /* Copies pivot */
  orxVector_Copy(_pvPivot, &(_pstGraphic->vPivot));

  /* Done! */
  return _pvPivot;  
}

/** Gets graphic size
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[out]  _pfWidth        Object's width
 * @param[out]  _pfHeight       Object's height
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxGraphic_GetSize(orxCONST orxGRAPHIC *_pstGraphic, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);
  orxASSERT(_pfWidth != orxNULL);
  orxASSERT(_pfHeight != orxNULL);

  /* Valid 2D data? */
  if(orxStructure_TestFlags((orxGRAPHIC *)_pstGraphic, orxGRAPHIC_KU32_FLAG_2D) != orxFALSE)
  {
    /* Gets its size */
    eResult = orxTexture_GetSize((orxTEXTURE *)_pstGraphic->pstData, _pfWidth, _pfHeight);
  }
  else
  {
    /* No size */
    *_pfWidth  = *_pfHeight = orx2F(-1.0f);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}
