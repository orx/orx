/***************************************************************************
 graphic.c
 Graphic module
 
 begin                : 08/12/2003
 author               : (C) Gdp
 email                : iarwain@ifrance.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "graph/graphic.h"

#include "anim/orxAnimPointer.h"
#include "debug/orxDebug.h"
#include "memory/orxMemory.h"


/*
 * Platform independant defines
 */

#define GRAPHIC_KU32_FLAG_NONE                 0x00000000
#define GRAPHIC_KU32_FLAG_READY                0x00000001
#define GRAPHIC_KU32_FLAG_DEFAULT              0x00000000

#define GRAPHIC_KU32_ID_FLAG_RENDER_DIRTY      0x20000000

#define GRAPHIC_KS32_STRUCT_NUMBER              2

#define GRAPHIC_KS32_STRUCT_OFFSET_INVALID     -1
#define GRAPHIC_KS32_STRUCT_OFFSET_TEXTURE      0
#define GRAPHIC_KS32_STRUCT_OFFSET_ANIMPOINTER  1


/*
 * Object structure
 */
struct st_graphic_t
{
  /* Public structure, first structure member : 16 */
  orxSTRUCTURE stStructure;

  /* Used structures ids : 20 */
  orxU32 u32LinkedStructures;

  /* Id flags : 24 */
  orxU32 u32IDFlags;

  /* Used structures : 32 */
  orxSTRUCTURE *pastStructure[GRAPHIC_KS32_STRUCT_NUMBER];
};



/*
 * Static members
 */
static orxU32 graphic_su32Flags = GRAPHIC_KU32_FLAG_DEFAULT;


/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 graphic_struct_offset_get
 Gets a structure offset given its id

 returns: requested structure offset
 ***************************************************************************/
inline orxS32 graphic_struct_offset_get(orxSTRUCTURE_ID _eStructureID)
{
  /* Gets structure offset according to id */
  switch(_eStructureID)
  {
    /* Texture structure */
    case orxSTRUCTURE_ID_TEXTURE:
      return GRAPHIC_KS32_STRUCT_OFFSET_TEXTURE;

    /* AnimationPointer structure*/
    case orxSTRUCTURE_ID_ANIMPOINTER:
      return GRAPHIC_KS32_STRUCT_OFFSET_ANIMPOINTER;

    default:
      return GRAPHIC_KS32_STRUCT_OFFSET_INVALID;
  }
}

/***************************************************************************
 graphic_list_delete
 Deletes all graphics.

 returns: orxVOID
 ***************************************************************************/
inline orxVOID graphic_list_delete()
{
  graphic_st_graphic *pstGraphic = (graphic_st_graphic *)orxStructure_GetFirst(orxSTRUCTURE_ID_GRAPHIC);

  /* Non empty? */
  while(pstGraphic != orxNULL)
  {
    /* Deletes graphic */
    graphic_delete(pstGraphic);

    /* Gets first graphic */
    pstGraphic = (graphic_st_graphic *)orxStructure_GetFirst(orxSTRUCTURE_ID_GRAPHIC);
  }

  return;
}


/***************************************************************************
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/


/***************************************************************************
 graphic_init
 Inits graphic system.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxU32 graphic_init()
{
  /* Not already Initialized? */
  if(!(graphic_su32Flags & GRAPHIC_KU32_FLAG_READY))
  {
    /* Inits Flags */
    graphic_su32Flags = GRAPHIC_KU32_FLAG_READY;

    return orxSTATUS_SUCCESS;
  }

  return orxSTATUS_FAILED;
}

/***************************************************************************
 graphic_exit
 Exits from the graphic system.

 returns: orxVOID
 ***************************************************************************/
orxVOID graphic_exit()
{
  /* Initialized? */
  if(graphic_su32Flags & GRAPHIC_KU32_FLAG_READY)
  {
    graphic_su32Flags &= ~GRAPHIC_KU32_FLAG_READY;

    /* Deletes graphic list */
    graphic_list_delete();
  }

  return;
}

/***************************************************************************
 graphic_create
 Creates a new empty graphic.

 returns: Created graphic.
 ***************************************************************************/
graphic_st_graphic *graphic_create()
{
  graphic_st_graphic *pstGraphic;
  orxS32 i;

  /* Creates graphic */
  pstGraphic = (graphic_st_graphic *) orxMemory_Allocate(sizeof(graphic_st_graphic), orxMEMORY_TYPE_MAIN);

  /* Non null? */
  if(pstGraphic != orxNULL)
  {
    /* Inits structure */
    if(orxStructure_Setup((orxSTRUCTURE *)pstGraphic, orxSTRUCTURE_ID_GRAPHIC) != orxSTATUS_SUCCESS)
    {
      /* !!! MSG !!! */

      /* Frees partially allocated texture */
      orxMemory_Free(pstGraphic);

      /* Returns nothing */
      return orxNULL;
    }

    /* Inits flags */
    pstGraphic->u32LinkedStructures = orxSTRUCTURE_ID_NONE;
    pstGraphic->u32IDFlags = GRAPHIC_KU32_ID_FLAG_NONE;

    /* Cleans structure pointers */
    for(i = 0; i < GRAPHIC_KS32_STRUCT_NUMBER; i++)
    {
      pstGraphic->pastStructure[i] = orxNULL;
    }
  }

  return pstGraphic;
}

/***************************************************************************
 graphic_delete
 Deletes a graphic.

 returns: orxVOID
 ***************************************************************************/
orxVOID graphic_delete(graphic_st_graphic *_pstGraphic)
{
  /* Non null? */
  if(_pstGraphic != orxNULL)
  {
    orxU32 i;

    /* Unlink all structures */
    for(i = 0; i < orxSTRUCTURE_ID_NUMBER; i++)
    {
      /* Cleans members */
      graphic_struct_unlink(_pstGraphic, (orxSTRUCTURE_ID)i);
    }

    /* Cleans structure */
    orxStructure_Clean((orxSTRUCTURE *)_pstGraphic);

    /* Frees graphic memory */
    orxMemory_Free(_pstGraphic);
  }

  return;
}

/***************************************************************************
 graphic_struct_link
 Links a structure to a graphic given.

 returns: orxVOID
 ***************************************************************************/
orxVOID graphic_struct_link(graphic_st_graphic *_pstGraphic, orxSTRUCTURE *_pstStructure)
{
  orxU32 u32StructureIndex;
  orxSTRUCTURE_ID eStructureID;

   /* Checks */
  orxASSERT(_pstGraphic != orxNULL);
  orxASSERT(_pstStructure != orxNULL);

  /* Gets structure id & offset */
  eStructureID      = orxStructure_GetID(_pstStructure);
  u32StructureIndex = graphic_struct_offset_get(eStructureID);

  /* Valid? */
  if(u32StructureIndex != orxU32_Undefined)
  {
    /* Unlink previous structure if needed */
    graphic_struct_unlink(_pstGraphic, eStructureID);

    /* Updates structure reference counter */
    orxStructure_IncreaseCounter(_pstStructure);

    /* Links new structure to graphic */
    _pstGraphic->pastStructure[u32StructureIndex] = _pstStructure;
    _pstGraphic->u32LinkedStructures |= eStructureID;

    /* Depends on structure type */
    switch(eStructureID)
    {
      /* Texture */
      case orxSTRUCTURE_ID_TEXTURE:
        /* Updates flag */
        graphic_flag_set(_pstGraphic, GRAPHIC_KU32_ID_FLAG_2D, 0);
        break;

      /* Animpointer */
      case orxSTRUCTURE_ID_ANIMPOINTER:
        /* Checks current anim */
        if(orxAnim_TestFlag(orxAnimPointer_GetAnim((orxANIM_POINTER *)_pstStructure), orxANIM_KU32_ID_FLAG_2D) != orxFALSE)
        {
          /* Updates flag */
          graphic_flag_set(_pstGraphic, GRAPHIC_KU32_ID_FLAG_2D | GRAPHIC_KU32_ID_FLAG_ANIM, GRAPHIC_KU32_ID_FLAG_NONE);

          break;
        }

      default:
        /* !!! MSG !!! */

        break;
    }
  }

  return;
}

/***************************************************************************
 graphic_struct_unlink
 Unlinks structures from a graphic given their IDs.
 Ids can be OR'ed.

 returns: orxVOID
 ***************************************************************************/
orxVOID graphic_struct_unlink(graphic_st_graphic *_pstGraphic, orxSTRUCTURE_ID _eStructureID)
{
  orxSTRUCTURE *pstStructure;
  orxU32 u32ID, u32StructureIndex;

   /* Checks */
  orxASSERT(_pstGraphic != orxNULL);

  /* Gets used struct ids */
  u32ID = (1 << _eStructureID);

  /* Needs to be processed? */
  if(u32ID & _pstGraphic->u32LinkedStructures)
  {
    /* Gets structure index */
    u32StructureIndex = graphic_struct_offset_get(_eStructureID);

    /* Decreases structure reference counter */
    pstStructure = _pstGraphic->pastStructure[u32StructureIndex];
    orxStructure_DecreaseCounter(pstStructure);

    /* Unlinks structure */
    _pstGraphic->pastStructure[u32StructureIndex] = orxNULL;

    /* Updates structures ids */
    _pstGraphic->u32LinkedStructures &= ~u32ID;

    /* Updates flags */
    if(_eStructureID == orxSTRUCTURE_ID_ANIMPOINTER)
    {
      graphic_flag_set(_pstGraphic, GRAPHIC_KU32_ID_FLAG_NONE, GRAPHIC_KU32_ID_FLAG_ANIM);
    }
  }

  return;
}

/***************************************************************************
 graphic_render_status_ok
 Test graphic render status (TRUE : clean / orxFALSE : dirty)

 returns: orxTRUE (clean) / orxFALSE (dirty)
 ***************************************************************************/
inline orxBOOL graphic_render_status_ok(graphic_st_graphic *_pstGraphic)
{
  /* Non null? */
  if(_pstGraphic != orxNULL)
  {
    /* Test render dirty flag */
    if(_pstGraphic->u32IDFlags & GRAPHIC_KU32_ID_FLAG_RENDER_DIRTY)
    {
      return orxFALSE;
    }
    else
    {
      return orxTRUE;
    }
  }

  return orxTRUE;
}

/***************************************************************************
 graphic_render_status_clean
 Cleans all graphics render status

 returns: orxVOID
 ***************************************************************************/
orxVOID graphic_render_status_clean()
{
  graphic_st_graphic *pstGraphic = (graphic_st_graphic *)orxStructure_GetFirst(orxSTRUCTURE_ID_GRAPHIC);

  /* Non empty? */
  while(pstGraphic != orxNULL)
  {
    /* Removes render dirty flag from all graphic */
    pstGraphic->u32IDFlags &= ~GRAPHIC_KU32_ID_FLAG_RENDER_DIRTY;

    /* Gets next graphic */
    pstGraphic = (graphic_st_graphic *)orxStructure_GetNext((orxSTRUCTURE *)pstGraphic);
  }

  return;
}


/* *** Structure accessors *** */


/***************************************************************************
 graphic_struct_get
 Gets a structure used by a graphic, given its structure ID.

 returns: pointer to the requested structure (must be cast correctly)
 ***************************************************************************/
inline orxSTRUCTURE *graphic_struct_get(graphic_st_graphic *_pstGraphic, orxSTRUCTURE_ID _eStructureID)
{
  orxSTRUCTURE *pstStructure = orxNULL;
  orxS32 i_struct_offset;

  /* Gets offset */
  i_struct_offset = graphic_struct_offset_get(_eStructureID);

  /* Offset is valid? */
  if(i_struct_offset != GRAPHIC_KS32_STRUCT_OFFSET_INVALID)
  {
    /* Gets requested structure */
    pstStructure = _pstGraphic->pastStructure[i_struct_offset];
  }

  return pstStructure;
}    

/***************************************************************************
 graphic_2d_data_get
 Gets current 2D data.

 returns: pointer to the current data
 ***************************************************************************/
inline orxTEXTURE *graphic_2d_data_get(graphic_st_graphic *_pstGraphic)
{
  /* Use an animation? */
  if(_pstGraphic->u32LinkedStructures & orxSTRUCTURE_ID_ANIMPOINTER)
  {
    orxANIM_POINTER *pstAnimpointer;
    orxANIM *pstAnim;

    /* Gets animpointer */
    pstAnimpointer = (orxANIM_POINTER *)graphic_struct_get(_pstGraphic, orxSTRUCTURE_ID_ANIMPOINTER);

    /* Gets current animation */
    pstAnim = orxAnimPointer_GetAnim(pstAnimpointer);

    /* Is animation 2D? */
    if(orxAnim_TestFlag(pstAnim, orxANIM_KU32_ID_FLAG_2D) != orxFALSE)
    {
      orxU32 u32Time;

      /* Gets timestamp */
      u32Time = orxAnimPointer_GetTime(pstAnimpointer);

      /* returns texture */
      return(orxAnim_ComputeTexture(pstAnim, u32Time));
    }
    else
    {
      /* !!! MSG !!! */
 
      return orxNULL;
    }
  }
  /* Use single texture? */
  else if(_pstGraphic->u32LinkedStructures & orxSTRUCTURE_ID_TEXTURE)
  {
    return((orxTEXTURE *)graphic_struct_get(_pstGraphic, orxSTRUCTURE_ID_TEXTURE));
  }

  /* No data */
  /* !!! MSG !!! */

  return orxNULL;
}

/***************************************************************************
 graphic_2d_size_get
 Gets current 2d data size.

 returns: orxVOID
 ***************************************************************************/
inline orxVOID graphic_2d_size_get(graphic_st_graphic *_pstGraphic, orxVEC *_pst_coord)
{
  orxTEXTURE *pstTexture;

  /* Gets texture */
  pstTexture = graphic_2d_data_get(_pstGraphic);

  /* Data found? */
  if(pstTexture != orxNULL)
  {
    texture_size_get(pstTexture, _pst_coord);
  }
  else
  {
    /* !!! MSG !!! */

    coord_set(_pst_coord, 0, 0, 0);
  }

  return;
}

/***************************************************************************
 graphic_2d_ref_coord_get
 Gets graphic current 2d data ref coord.

 returns: orxVOID
 ***************************************************************************/
inline orxVOID graphic_2d_ref_coord_get(graphic_st_graphic *_pstGraphic, orxVEC *_pst_coord)
{
  orxTEXTURE *pstTexture;

  /* Gets texture */
  pstTexture = graphic_2d_data_get(_pstGraphic);

  /* Data found? */
  if(pstTexture != orxNULL)
  {
      texture_ref_coord_get(pstTexture, _pst_coord);
  }
  else
  {
    /* No data */
    /* !!! MSG !!! */

    coord_set(_pst_coord, 0, 0, 0);
  }

  return;
}

/***************************************************************************
 graphic_2d_max_size_get
 Gets maximum size (used for object bounding box).

 returns: orxVOID
 ***************************************************************************/
inline orxVOID graphic_2d_max_size_get(graphic_st_graphic *_pstGraphic, orxVEC *_pst_coord)
{
  orxTEXTURE *pstTexture;

  /* Gets texture */
  pstTexture = graphic_2d_data_get(_pstGraphic);

  /* Data found? */
  if(pstTexture != orxNULL)
  {
    texture_size_get(pstTexture, _pst_coord);
  }
  else
  {
    /* No data */
    /* !!! MSG !!! */
  }

  return;
}

/***************************************************************************
 graphic_flag_test
 Graphic flag test accessor.

 returns: orxBOOL
 ***************************************************************************/
orxBOOL graphic_flag_test(graphic_st_graphic *_pstGraphic, orxU32 _u32Flag)
{
  if(_pstGraphic->u32IDFlags & _u32Flag)
  {
    return orxTRUE;
  }

  return orxFALSE;
}

/***************************************************************************
 graphic_flag_set
 Graphic flag get/set accessor.

 returns: orxVOID
 ***************************************************************************/
orxVOID graphic_flag_set(graphic_st_graphic *_pstGraphic, orxU32 _u32AddFlags, orxU32 _u32RemoveFlags)
{
  _pstGraphic->u32IDFlags &= ~_u32RemoveFlags;
  _pstGraphic->u32IDFlags |= _u32AddFlags;

  return;
}
