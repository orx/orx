#include "math/coord.h"

/*** Constants Definitions ***/
#define COORD_KI_NULL        (int)0xFFFFFFFF

/*** Functions Definitions ***/
inline bool coord_is_null(coord_st_coord *_pst_coord)
{
  return(((_pst_coord->s32_x == COORD_KI_NULL)
       && (_pst_coord->s32_y == COORD_KI_NULL)
       && (_pst_coord->s32_z == COORD_KI_NULL))
       ? TRUE
       : FALSE);
}

inline void coord_set(coord_st_coord *_pst_coord, int32 _l_x, int32 _l_y, int32 _l_z)
{
  /* Non null?*/
  if(_pst_coord != NULL)
  {
    _pst_coord->s32_x = _l_x;
    _pst_coord->s32_y = _l_y;
    _pst_coord->s32_z = _l_z;
  }

  return;
}

inline void coord_reset(coord_st_coord *_pst_coord)
{
  coord_set(_pst_coord, COORD_KI_NULL, COORD_KI_NULL, COORD_KI_NULL);

  return;
}

inline coord_st_coord *coord_create(int32 _l_x, int32 _l_y, int32 _l_z)
{
  coord_st_coord *pst_coord;

  pst_coord = (coord_st_coord *)malloc(sizeof(coord_st_coord));

  coord_set(pst_coord, _l_x, _l_y, _l_z);

  return pst_coord;
}

inline void coord_delete(coord_st_coord *_pst_coord)
{
  if(_pst_coord != NULL)
  {
    free(_pst_coord);
  }

  return;
}

inline void coord_copy(coord_st_coord *_pst_dest, coord_st_coord *_pst_src)
{
  if(_pst_src != NULL)
  {
    coord_set(_pst_dest, _pst_src->s32_x, _pst_src->s32_y, _pst_src->s32_z);
  }

  return;
}

inline void coord_add(coord_st_coord *_pst_result, coord_st_coord *_pst_op1, coord_st_coord *_pst_op2)
{
  if((_pst_op1 != NULL) && (_pst_op2 != NULL))
  {
    coord_set(_pst_result,
              _pst_op1->s32_x + _pst_op2->s32_x,
              _pst_op1->s32_y + _pst_op2->s32_y,
              _pst_op1->s32_z + _pst_op2->s32_z);
  }

  return;
}

inline void coord_sub(coord_st_coord *_pst_result, coord_st_coord *_pst_op1, coord_st_coord *_pst_op2)
{
  if((_pst_op1 != NULL) && (_pst_op2 != NULL))
  {
    coord_set(_pst_result,
              _pst_op1->s32_x - _pst_op2->s32_x,
              _pst_op1->s32_y - _pst_op2->s32_y,
              _pst_op1->s32_z - _pst_op2->s32_z);
  }

  return;
}

inline void coord_neg(coord_st_coord *_pst_result, coord_st_coord *_pst_op)
{
  if(_pst_op != NULL)
  {
    coord_set(_pst_result, -(_pst_op->s32_x), -(_pst_op->s32_y), -(_pst_op->s32_z));
  }

  return;
}

inline void coord_mul(coord_st_coord *_pst_result, coord_st_coord *_pst_op1, float _f_op2)
{
  if(_pst_op1 != NULL)
  {
    coord_set(_pst_result,
              (int)rintf((float)(_pst_op1->s32_x) * _f_op2),
              (int)rintf((float)(_pst_op1->s32_y) * _f_op2),
              (int)rintf((float)(_pst_op1->s32_z) * _f_op2));
  }

  return;
}

inline void coord_div(coord_st_coord *_pst_result, coord_st_coord *_pst_op1, float _f_op2)
{
  if(_pst_op1 != NULL)
  {
    coord_set(_pst_result,
              (int)rintf((float)(_pst_op1->s32_x) / _f_op2),
              (int)rintf((float)(_pst_op1->s32_y) / _f_op2),
              (int)rintf((float)(_pst_op1->s32_z) / _f_op2));
  }

  return;
}

inline void coord_rotate(coord_st_coord *_pst_result, coord_st_coord *_pst_op1, float _f_op2)
{
  float f_cos, f_sin;
  float f_x, f_y;

  if(_pst_op1 != NULL)
  {
    f_cos = cosf(_f_op2);
    f_sin = sinf(_f_op2);
    f_x = (float)_pst_op1->s32_x;
    f_y = (float)_pst_op1->s32_y;

    coord_set(_pst_result,
              (int)rintf((f_x * f_cos) - (f_y * f_sin)),
              (int)rintf((f_x * f_sin) + (f_y * f_cos)),
              (_pst_op1->s32_z));
  }

  return;
}

inline void coord_aabox_reorder(coord_st_coord *_pst_box_ul, coord_st_coord *_pst_box_br)
{
  /* Non null? */
  if((_pst_box_ul != NULL) && (_pst_box_br != NULL))
  {
    /* Reorders coordinates so as to have upper left & bottom right box corners */

    /* Z coord */
    if(_pst_box_ul->s32_z > _pst_box_br->s32_z)
    {
      /* Swap */
      _pst_box_ul->s32_z = _pst_box_ul->s32_z ^ _pst_box_br->s32_z;
      _pst_box_br->s32_z = _pst_box_ul->s32_z ^ _pst_box_br->s32_z;
      _pst_box_ul->s32_z = _pst_box_ul->s32_z ^ _pst_box_br->s32_z;
    }

    /* Y coord */
    if(_pst_box_ul->s32_y > _pst_box_br->s32_y)
    {
      /* Swap */
      _pst_box_ul->s32_y = _pst_box_ul->s32_y ^ _pst_box_br->s32_y;
      _pst_box_br->s32_y = _pst_box_ul->s32_y ^ _pst_box_br->s32_y;
      _pst_box_ul->s32_y = _pst_box_ul->s32_y ^ _pst_box_br->s32_y;
    }

    /* X coord */
    if(_pst_box_ul->s32_x > _pst_box_br->s32_x)
    {
      /* Swap */
      _pst_box_ul->s32_x = _pst_box_ul->s32_x ^ _pst_box_br->s32_x;
      _pst_box_br->s32_x = _pst_box_ul->s32_x ^ _pst_box_br->s32_x;
      _pst_box_ul->s32_x = _pst_box_ul->s32_x ^ _pst_box_br->s32_x;
    }
  }

  return;
}

inline bool coord_aabox_intersection_test(coord_st_coord *_pst_box1_ul, coord_st_coord *_pst_box1_br, coord_st_coord *_pst_box2_ul, coord_st_coord *_pst_box2_br)
{
  /* Non null? */
  if((_pst_box1_ul != NULL)
  && (_pst_box1_br != NULL)
  && (_pst_box2_ul != NULL)
  && (_pst_box2_br != NULL))
  {
    /* Warning : Corners should be sorted otherwise test won't work! */

    /* Z intersection test */
    if((_pst_box2_br->s32_z < _pst_box1_ul->s32_z)
    || (_pst_box2_ul->s32_z > _pst_box1_br->s32_z))
    {
      /* Disjoint */
      return FALSE;
    }

    /* X intersection test */
    if((_pst_box2_br->s32_x < _pst_box1_ul->s32_x)
    || (_pst_box2_ul->s32_x > _pst_box1_br->s32_x))
    {
      /* Disjoint */
      return FALSE;
    }

    /* Y intersection test */
    if((_pst_box2_br->s32_y < _pst_box1_ul->s32_y)
    || (_pst_box2_ul->s32_y > _pst_box1_br->s32_y))
    {
      /* Disjoint */
      return FALSE;
    }
  }
  else
  {
    return FALSE;
  }

  /* Not disjoint */
  return TRUE;
}
