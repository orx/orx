/** 
 * \file orxFrame.h
 * 
 * Frame Module.
 * Allows to creates and handle frames.
 * Frames are structures used to give position, orientation & scale to objects.
 * They thus can be referenced by other structures.
 * 
 * \todo
 * Use matrix instead of disjoint pos vector/angle float/scale float for frame data structure.
 * 3D system (later).
 */


/***************************************************************************
 orxFrame.h
 Frame module
 
 begin                : 02/12/2003
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

#ifndef _orxFRAME_H_
#define _orxFRAME_H_

#include "orxInclude.h"

#include "math/orxMath.h"


/** Internal frame structure. */
typedef struct __orxFRAME_t orxFRAME;


/** Inits the frame system. */
extern orxSTATUS            orxFrame_Init();
/** Ends the frame system. */
extern orxVOID              orxFrame_Exit();

/** Creates a frame. */
extern orxFRAME            *orxFrame_Create();
/** Deletes a frame. */
extern orxSTATUS            orxFrame_Delete(orxFRAME *_pstFrame);

/** Cleans all frames render status */
extern orxVOID              orxFrame_CleanAllRenderStatus();
/** Test frame render status (TRUE : clean / orxFALSE : dirty)*/
extern orxBOOL              orxFrame_IsRenderStatusClean(orxFRAME *_pstFrame);

/** Does frame has differential scrolling? */
extern orxBOOL              orxFrame_HasDifferentialScrolling(orxFRAME *_pstFrame);
/** Sets frame differential scrolling values (X & Y axis / orx2F(0.0f) : none) */
extern orxVOID              orxFrame_SetDifferentialScrolling(orxFRAME * _pstFrame, orxVEC *_pvScroll);
/** Gets frame differential scrolling values (X & Y axis / orx2F(0.0f) : none) */
extern orxVOID              orxFrame_GetDifferentialScrolling(orxFRAME * _pstFrame, orxVEC *_pvScroll);

/** Sets a frame parent. */
extern orxVOID              orxFrame_SetParent(orxFRAME *_pstFrame, orxFRAME * _pstParent);

/** Sets 2D local position */
extern orxVOID              orxFrame_SetPosition(orxFRAME *_pstFrame, orxVEC *_pvPos);
/** Sets 2D local rotation */
extern orxVOID              orxFrame_SetRotation(orxFRAME *_pstFrame, orxFLOAT _fAngle);
/** Sets 2D local scale */
extern orxVOID              orxFrame_SetScale(orxFRAME *_pstFrame, orxFLOAT _fScale);

/** Gets 2D position global/local according to param */
extern orxVOID              orxFrame_GetPosition(orxFRAME *_pstFrame, orxVEC *_pvPos, orxBOOL _bLocal);
/** Gets 2D global rotation global/local according to param */
extern orxFLOAT             orxFrame_GetRotation(orxFRAME *_pstFrame, orxBOOL _bLocal);
/** Gets 2D scale global/local according to param */
extern orxFLOAT             orxFrame_GetScale(orxFRAME *_pstFrame, orxBOOL _bLocal);

/** Computes a frame global data using parent's global & self local ones. Result is stored in another frame. */
extern orxVOID              orxFrame_ComputeGlobalData(orxFRAME *_pstDstFrame, orxFRAME *_pstSrcFrame, orxFRAME *_pstParentFrame);


#endif /* _orxFRAME_H_ */
