/**
 * \file orxClock.h
 */

/***************************************************************************
 begin                : 28/01/2004
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

#ifndef _orxCLOCK_H_
#define _orxCLOCK_H_

#include "orxInclude.h"


#define orxCLOCK_KU32_CLOCK_BANK_SIZE       16          /**< Clock bank size */

#define orxCLOCK_KU32_FUNCTION_BANK_SIZE    16          /**< Function bank size */


/** Clock type enum */
typedef enum __orxCLOCK_TYPE_t
{
  orxCLOCK_TYPE_CORE = 0,
  orxCLOCK_TYPE_USER,
  
  orxCLOCK_TYPE_FPS,

  orxCLOCK_TYPE_NUMBER,

  orxCLOCK_TYPE_NONE = orxENUM_NONE

} orxCLOCK_TYPE;

/** Clock mod type enum */
typedef enum __orxCLOCK_MOD_TYPE_t
{
  orxCLOCK_MOD_TYPE_FIXED = 0,
  orxCLOCK_MOD_TYPE_MULTIPLY,

  orxCLOCK_MOD_TYPE_NUMBER,

  orxCLOCK_MOD_TYPE_NONE = orxENUM_NONE,
    
} orxCLOCK_MOD_TYPE;

/** Clock info structure. */
typedef struct __orxCLOCK_INFO_t
{
  orxCLOCK_TYPE     eType;                            /**< Clock type : 4 */
  orxU32            u32TickCounter;                   /**< Clock tick counter : 8 */
  orxFLOAT          fTickSize;                        /**< Clock tick size (in seconds) : 12 */
  orxFLOAT          fTickValue;                       /**< Clock current tick value ellapsed after last tick 0.0 - 1.0 : 16 */
  orxCLOCK_MOD_TYPE eModType;                         /**> Clock mod type : 20 */
  orxFLOAT          fModValue;                        /**> Clock mod value : 24 */
  orxFLOAT          fDT;                              /**> Clock DT (time ellapsed between 2 tick calls) : 28 */

  orxU8 au8Unused[4];                                 /**> 4 extra bytes of padding : 32 */

} orxCLOCK_INFO;


/** Clock structure. */
typedef struct __orxCLOCK_t                           orxCLOCK;

/** Clock callback function type for used with clock bindings. */
typedef orxVOID orxFASTCALL (*orxCLOCK_FUNCTION)(orxCONST orxCLOCK_INFO *_pstClockInfo, orxVOID *_pstContext);


/** Inits Clock module. */
extern orxDLLAPI orxSTATUS                            orxClock_Init();
/** Exits from Clock module. */
extern orxDLLAPI orxVOID                              orxClock_Exit();

/** Updates the clock system. */
extern orxDLLAPI orxSTATUS                            orxClock_Update();

/** Creates a Clock. */
extern orxDLLAPI orxCLOCK *orxFASTCALL                orxClock_Create(orxFLOAT _fTickSize, orxCLOCK_TYPE _eType);
/** Deletes a Clock. */
extern orxDLLAPI orxVOID                              orxClock_Delete(orxCLOCK *_pstClock);

/** Gets informations about a Clock. */
extern orxDLLAPI orxCONST orxCLOCK_INFO *orxFASTCALL  orxClock_GetInfo(orxCLOCK *_pstClock);

/** Registers a callback function to a clock. */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxClock_Register(orxCLOCK *_pstClock, orxCLOCK_FUNCTION _pfnCallback, orxVOID *_pstContext);
/** Unregisters a callback function from a clock. */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxClock_Unregister(orxCLOCK *_pstClock, orxCLOCK_FUNCTION _pfnCallback);

/** Finds a clock according to its tick size and its type. */
extern orxDLLAPI orxCLOCK *orxFASTCALL                orxClock_FindFirst(orxFLOAT _fTickSize, orxCLOCK_TYPE _eType);
/** Finds next clock of same type/tick size. */
extern orxDLLAPI orxCLOCK *orxFASTCALL                orxClock_FindNext(orxCLOCK *_pstClock);


#endif /* _orxCLOCK_H_ */
