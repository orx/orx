/***************************************************************************
 orxLinkList.c
 List module
 
 begin                : 06/04/2005
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


#include "utils/orxLinkList.h"

#include "debug/orxDebug.h"
#include "memory/orxMemory.h"


/*
 * Platform independant defines
 */

#define orxLINKLIST_KU32_FLAG_NONE                0x00000000
#define orxLINKLIST_KU32_FLAG_READY               0x00000001


/*
 * Static structure
 */
typedef struct __orxLINKLIST_STATIC_t
{
  /* Control flags */
  orxU32 u32Flags;

} orxLINKLIST_STATIC;

/*
 * Static data
 */
static orxLINKLIST_STATIC sstLinkList;


/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/



/***************************************************************************
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 orxLinkList_Init
 Inits the link list system.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxLinkList_Init()
{
  /* Already Initialized? */
  if(sstLinkList.u32Flags & orxLINKLIST_KU32_FLAG_READY)
  {
    /* !!! MSG !!! */

    return orxSTATUS_FAILED;
  }

  /* Cleans static controller */
  orxMemory_Set(&sstLinkList, 0, sizeof(orxLINKLIST_STATIC));

  /* Inits ID Flags */
  sstLinkList.u32Flags = orxLINKLIST_KU32_FLAG_READY;

  /* Done! */
  return orxSTATUS_SUCCESS;
}

/***************************************************************************
 orxLinkList_Exit
 Exits from the link list system.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxLinkList_Exit()
{
  /* Not initialized? */
  if((sstLinkList.u32Flags & orxLINKLIST_KU32_FLAG_READY) == orxLINKLIST_KU32_FLAG_NONE)
  {
    /* !!! MSG !!! */

    return;
  }

  /* Updates flags */
  sstLinkList.u32Flags &= ~orxLINKLIST_KU32_FLAG_READY;

  return;
}

/***************************************************************************
 orxLinkList_Setup
 Setups a link list for use.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxLinkList_Setup(orxLINKLIST *_pstList)
{
  /* Checks */
  orxASSERT(sstLinkList.u32Flags & orxLINKLIST_KU32_FLAG_READY);
  orxASSERT(_pstList != orxNULL);

  /* Inits list */
  orxMemory_Set(_pstList, 0, sizeof(orxLINKLIST));

  /* Done! */
  return orxSTATUS_SUCCESS;
}

/***************************************************************************
 orxLinkList_AddStart
 Adds a new cell at the start of the corresponding list.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxLinkList_AddStart(orxLINKLIST *_pstList, orxLINKLIST_CELL *_pstCell)
{
  orxREGISTER orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstLinkList.u32Flags & orxLINKLIST_KU32_FLAG_READY);
  orxASSERT(_pstList != orxNULL);
  orxASSERT(_pstCell != orxNULL);

  /* Not already used in a list? */
  if(_pstCell->pstList == orxNULL)
  {
    /* Adds it at the start of the list */
    _pstCell->pstNext     = _pstList->pstFirst;
    _pstCell->pstPrevious = orxNULL;
    _pstCell->pstList     = _pstList;
  
    /* Updates old cell if needed */
    if(_pstList->pstFirst != orxNULL)
    {
      _pstList->pstFirst->pstPrevious = _pstCell;
    }
  
    /* Stores cell at the start of the list */
    _pstList->pstFirst    = _pstCell;

    /* Updates counter */
    _pstList->u32Counter++;
  }
  else
  {
    /* !!! MSG !!! */

    /* Not linked */
    eResult = orxSTATUS_FAILED;
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxLinkList_AddEnd
 Adds a new cell at the end of the corresponding list.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxLinkList_AddEnd(orxLINKLIST *_pstList, orxLINKLIST_CELL *_pstCell)
{
  orxREGISTER orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstLinkList.u32Flags & orxLINKLIST_KU32_FLAG_READY);
  orxASSERT(_pstList != orxNULL);
  orxASSERT(_pstCell != orxNULL);

  /* Not already used in a list? */
  if(_pstCell->pstList == orxNULL)
  {
    /* Adds it at the end of the list */
    _pstCell->pstPrevious = _pstList->pstLast;
    _pstCell->pstNext     = orxNULL;
    _pstCell->pstList     = _pstList;
  
    /* Updates old cell if needed */
    if(_pstList->pstLast != orxNULL)
    {
      _pstList->pstLast->pstNext = _pstCell;
    }
  
    /* Stores cell at the end of the list */
    _pstList->pstLast     = _pstCell;

    /* Updates counter */
    _pstList->u32Counter++;
  }
  else
  {
    /* !!! MSG !!! */

    /* Not linked */
    eResult = orxSTATUS_FAILED;
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxLinkList_Remove
 Removes a cell from its list.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxLinkList_Remove(orxLINKLIST_CELL *_pstCell)
{
  orxREGISTER orxLINKLIST *pstList;
  orxREGISTER orxLINKLIST_CELL *pstPrevious, *pstNext;
  orxREGISTER orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstLinkList.u32Flags & orxLINKLIST_KU32_FLAG_READY);
  orxASSERT(_pstCell != orxNULL);

  /* Gets list */
  pstList = _pstCell->pstList;

  /* Valid? */
  if(pstList != orxNULL)
  {
    /* Checks list is non empty */
    orxASSERT(pstList->u32Counter != 0);

    /* Gets neighbours pointers */
    pstPrevious = _pstCell->pstPrevious;
    pstNext     = _pstCell->pstNext;

    /* Not at the start of the list? */
    if(pstPrevious != orxNULL)
    {
      /* Updates previous cell */
      pstPrevious->pstNext  = pstNext;
    }
    else
    {
      /* Checks cell was at the start of the list */
      orxASSERT(pstList->pstFirst == _pstCell);

      /* Updates list first pointer */
      pstList->pstFirst     = pstNext;
    }

    /* Not at the end of the list? */
    if(pstNext != orxNULL)
    {
      /* Updates previous cell */
      pstNext->pstPrevious  = pstPrevious;
    }
    else
    {
      /* Checks cell was at the end of the list */
      orxASSERT(pstList->pstLast == _pstCell);

      /* Updates list last pointer */
      pstList->pstLast      = pstPrevious;
    }

    /* Udpates counter */
    pstList->u32Counter--;
  }
  else
  {
    /* !!! MSG !!! */
    
    /* Failed */
    eResult = orxSTATUS_FAILED;
  }

  /* Done! */
  return eResult;
}
