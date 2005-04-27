/***************************************************************************
 orxVector.c
 Vector module
 
 begin                : 27/04/2005
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


#include "math/orxVector.h"

#include "math/orxMath.h"
#include "memory/orxMemory.h"


/*** Constants Definitions ***/
#define COORD_KS32_orxNULL        (int)0xFFFFFFFF

/*** Functions Definitions ***/
orxBOOL coord_is_null(orxVEC *_pst_coord)
{
  return(((_pst_coord->fX == COORD_KS32_orxNULL)
       && (_pst_coord->fY == COORD_KS32_orxNULL)
       && (_pst_coord->fZ == COORD_KS32_orxNULL))
       ? orxTRUE
       : orxFALSE);
}

orxVOID coord_set(orxVEC *_pst_coord, orxS32 _l_x, orxS32 _l_y, orxS32 _l_z)
{
  /* Non null?*/
  if(_pst_coord != orxNULL)
  {
    _pst_coord->fX = _l_x;
    _pst_coord->fY = _l_y;
    _pst_coord->fZ = _l_z;
  }

  return;
}

orxVOID coord_reset(orxVEC *_pst_coord)
{
  coord_set(_pst_coord, COORD_KS32_orxNULL, COORD_KS32_orxNULL, COORD_KS32_orxNULL);

  return;
}

orxVEC *coord_create(orxS32 _l_x, orxS32 _l_y, orxS32 _l_z)
{
  orxVEC *pst_coord;

  pst_coord = (orxVEC *)orxMemory_Allocate(sizeof(orxVEC), orxMEMORY_TYPE_MAIN);

  coord_set(pst_coord, _l_x, _l_y, _l_z);

  return pst_coord;
}

orxVOID coord_delete(orxVEC *_pst_coord)
{
  if(_pst_coord != orxNULL)
  {
    orxMemory_Free(_pst_coord);
  }

  return;
}

orxVOID coord_copy(orxVEC *_pst_dest, orxVEC *_pst_src)
{
  if(_pst_src != orxNULL)
  {
    coord_set(_pst_dest, _pst_src->fX, _pst_src->fY, _pst_src->fZ);
  }

  return;
}

orxVOID coord_add(orxVEC *_pst_result, orxVEC *_pst_op1, orxVEC *_pst_op2)
{
  if((_pst_op1 != orxNULL) && (_pst_op2 != orxNULL))
  {
    coord_set(_pst_result,
              _pst_op1->fX + _pst_op2->fX,
              _pst_op1->fY + _pst_op2->fY,
              _pst_op1->fZ + _pst_op2->fZ);
  }

  return;
}

orxVOID coord_sub(orxVEC *_pst_result, orxVEC *_pst_op1, orxVEC *_pst_op2)
{
  if((_pst_op1 != orxNULL) && (_pst_op2 != orxNULL))
  {
    coord_set(_pst_result,
              _pst_op1->fX - _pst_op2->fX,
              _pst_op1->fY - _pst_op2->fY,
              _pst_op1->fZ - _pst_op2->fZ);
  }

  return;
}

orxVOID coord_neg(orxVEC *_pst_result, orxVEC *_pst_op)
{
  if(_pst_op != orxNULL)
  {
    coord_set(_pst_result, -(_pst_op->fX), -(_pst_op->fY), -(_pst_op->fZ));
  }

  return;
}

orxVOID coord_mul(orxVEC *_pst_result, orxVEC *_pst_op1, orxFLOAT _f_op2)
{
  if(_pst_op1 != orxNULL)
  {
    coord_set(_pst_result,
              (int)rintf((orxFLOAT)(_pst_op1->fX) * _f_op2),
              (int)rintf((orxFLOAT)(_pst_op1->fY) * _f_op2),
              (int)rintf((orxFLOAT)(_pst_op1->fZ) * _f_op2));
  }

  return;
}

orxVOID coord_div(orxVEC *_pst_result, orxVEC *_pst_op1, orxFLOAT _f_op2)
{
  if(_pst_op1 != orxNULL)
  {
    coord_set(_pst_result,
              (int)rintf((orxFLOAT)(_pst_op1->fX) / _f_op2),
              (int)rintf((orxFLOAT)(_pst_op1->fY) / _f_op2),
              (int)rintf((orxFLOAT)(_pst_op1->fZ) / _f_op2));
  }

  return;
}

orxVOID coord_rotate(orxVEC *_pst_result, orxVEC *_pst_op1, orxFLOAT _f_op2)
{
  orxFLOAT fCos, fSin;
  orxFLOAT fX, fY;

  if(_pst_op1 != orxNULL)
  {
    fCos = cosf(_f_op2);
    fSin = sinf(_f_op2);
    fX = (orxFLOAT)_pst_op1->fX;
    fY = (orxFLOAT)_pst_op1->fY;

    coord_set(_pst_result,
              (int)rintf((fX * fCos) - (fY * fSin)),
              (int)rintf((fX * fSin) + (fY * fCos)),
              (_pst_op1->fZ));
  }

  return;
}

orxVOID coord_aabox_reorder(orxVEC *_pst_box_ul, orxVEC *_pst_box_br)
{
  /* Non null? */
  if((_pst_box_ul != orxNULL) && (_pst_box_br != orxNULL))
  {
    /* Reorders coordinates so as to have upper left & bottom right box corners */

    /* Z coord */
    if(_pst_box_ul->fZ > _pst_box_br->fZ)
    {
      /* Swap */
      orxSWAP32(_pst_box_ul->fZ, _pst_box_br->fZ);
    }

    /* Y coord */
    if(_pst_box_ul->fY > _pst_box_br->fY)
    {
      /* Swap */
      orxSWAP32(_pst_box_ul->fY, _pst_box_br->fY);
    }

    /* X coord */
    if(_pst_box_ul->fX > _pst_box_br->fX)
    {
      /* Swap */
      orxSWAP32(_pst_box_ul->fX, _pst_box_br->fX);
    }
  }

  return;
}

orxBOOL coord_aabox_intersection_test(orxVEC *_pst_box1_ul, orxVEC *_pst_box1_br, orxVEC *_pst_box2_ul, orxVEC *_pst_box2_br)
{
  /* Non null? */
  if((_pst_box1_ul != orxNULL)
  && (_pst_box1_br != orxNULL)
  && (_pst_box2_ul != orxNULL)
  && (_pst_box2_br != orxNULL))
  {
    /* Warning : Corners should be sorted otherwise test won't work! */

    /* Z intersection test */
    if((_pst_box2_br->fZ < _pst_box1_ul->fZ)
    || (_pst_box2_ul->fZ > _pst_box1_br->fZ))
    {
      /* Disjoint */
      return orxFALSE;
    }

    /* X intersection test */
    if((_pst_box2_br->fX < _pst_box1_ul->fX)
    || (_pst_box2_ul->fX > _pst_box1_br->fX))
    {
      /* Disjoint */
      return orxFALSE;
    }

    /* Y intersection test */
    if((_pst_box2_br->fY < _pst_box1_ul->fY)
    || (_pst_box2_ul->fY > _pst_box1_br->fY))
    {
      /* Disjoint */
      return orxFALSE;
    }
  }
  else
  {
    return orxFALSE;
  }

  /* Not disjoint */
  return orxTRUE;
}
