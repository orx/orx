/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2013 Orx-Project
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 */

/**
 * @file orxGraphic.h
 * @date 08/12/2003
 * @author iarwain@orx-project.org
 *
 * @todo
 * - Add 3D support
 */

/**
 * @addtogroup orxGraphic
 *
 * Graphic Module.
 * Allows to creates and handle 2D/3D Graphics.
 * They are used as texture/animation container, with associated properties.
 * 2D Graphics are used by objects.
 * They thus can be referenced by objects as structures.
 *
 * @{
 */


#ifndef _orxGRAPHIC_H_
#define _orxGRAPHIC_H_

#include "orxInclude.h"

#include "object/orxStructure.h"
#include "display/orxDisplay.h"


/** Graphic flags
 */
#define orxGRAPHIC_KU32_FLAG_NONE             0x00000000  /**< No flags */

#define orxGRAPHIC_KU32_FLAG_2D               0x00000001  /**< 2D type graphic flag  */
#define orxGRAPHIC_KU32_FLAG_TEXT             0x00000002  /**< Text type graphic flag  */

#define orxGRAPHIC_KU32_MASK_TYPE             0x00000003  /**< Type mask */

#define orxGRAPHIC_KU32_FLAG_FLIP_X           0x00000004  /**< X axis flipping flag */
#define orxGRAPHIC_KU32_FLAG_FLIP_Y           0x00000008  /**< Y axis flipping flag */
#define orxGRAPHIC_KU32_MASK_FLIP_BOTH        0x0000000C  /**< Both axis flipping mask */

#define orxGRAPHIC_KU32_FLAG_ALIGN_CENTER     0x00000000  /**< Center vertical / horizontal alignment */
#define orxGRAPHIC_KU32_FLAG_ALIGN_LEFT       0x00000010  /**< Left horizontal alignment */
#define orxGRAPHIC_KU32_FLAG_ALIGN_RIGHT      0x00000020  /**< Right horizontal alignment */
#define orxGRAPHIC_KU32_FLAG_ALIGN_TOP        0x00000040  /**< Top vertical alignment */
#define orxGRAPHIC_KU32_FLAG_ALIGN_BOTTOM     0x00000080  /**< Bottom vertical alignment */
#define orxGRAPHIC_KU32_FLAG_ALIGN_TRUNCATE   0x00000100  /**< Truncate alignment value */
#define orxGRAPHIC_KU32_FLAG_ALIGN_ROUND      0x00000200  /**< Round alignment value */

#define orxGRAPHIC_KU32_MASK_USER_ALL         0x00000FFF  /**< User all ID mask */


/** Internal Graphic structure
 */
typedef struct __orxGRAPHIC_t                 orxGRAPHIC;


/** Graphic module setup
 */
extern orxDLLAPI void orxFASTCALL             orxGraphic_Setup();

/** Inits the Graphic module
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxGraphic_Init();

/** Exits from the Graphic module
 */
extern orxDLLAPI void orxFASTCALL             orxGraphic_Exit();


/** Creates an empty graphic
 * @return      Created orxGRAPHIC / orxNULL
 */
extern orxDLLAPI orxGRAPHIC *orxFASTCALL      orxGraphic_Create();

/** Creates a graphic from config
 * @param[in]   _zConfigID            Config ID
 * @ return orxGRAPHIC / orxNULL
 */
extern orxDLLAPI orxGRAPHIC *orxFASTCALL      orxGraphic_CreateFromConfig(const orxSTRING _zConfigID);

/** Deletes a graphic
 * @param[in]   _pstGraphic       Graphic to delete
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxGraphic_Delete(orxGRAPHIC *_pstGraphic);


/** Sets graphic data
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[in]   _pstData        Data structure to set / orxNULL
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxGraphic_SetData(orxGRAPHIC *_pstGraphic, orxSTRUCTURE *_pstData);

/** Gets graphic data
 * @param[in]   _pstGraphic     Concerned graphic
 * @return      OrxSTRUCTURE / orxNULL
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL    orxGraphic_GetData(const orxGRAPHIC *_pstGraphic);


/** Sets graphic flipping
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[in]   _bFlipX         Flip it on X axis
 * @param[in]   _bFlipY         Flip it on Y axis
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxGraphic_SetFlip(orxGRAPHIC *_pstGraphic, orxBOOL _bFlipX, orxBOOL _bFlipY);

/** Gets graphic flipping
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[in]   _pbFlipX        X axis flipping
 * @param[in]   _pbFlipY        Y axis flipping
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxGraphic_GetFlip(const orxGRAPHIC *_pstGraphic, orxBOOL *_pbFlipX, orxBOOL *_pbFlipY);


/** Sets graphic pivot
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[in]   _pvPivot        Pivot to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxGraphic_SetPivot(orxGRAPHIC *_pstGraphic, const orxVECTOR *_pvPivot);

/** Sets relative graphic pivot
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[in]   _u32AlignFlags  Alignment flags
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxGraphic_SetRelativePivot(orxGRAPHIC *_pstGraphic, orxU32 _u32AlignFlags);

/** Gets graphic pivot
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[out]  _pvPivot        Graphic pivot
 * @return      orxPIVOT / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL       orxGraphic_GetPivot(const orxGRAPHIC *_pstGraphic, orxVECTOR *_pvPivot);


/** Sets graphic size
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[in]   _pvSize         Size to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxGraphic_SetSize(orxGRAPHIC *_pstGraphic, const orxVECTOR *_pvSize);

/** Gets graphic size
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[out]  _pvSize         Object's size
 * @return      orxVECTOR / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL       orxGraphic_GetSize(const orxGRAPHIC *_pstGraphic, orxVECTOR *_pvSize);

/** Sets graphic color
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[in]   _pstColor       Color to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxGraphic_SetColor(orxGRAPHIC *_pstGraphic, const orxCOLOR *_pstColor);

/** Sets graphic repeat (wrap) value
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[in]   _fRepeatX       X-axis repeat value
 * @param[in]   _fRepeatY       Y-axis repeat value
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxGraphic_SetRepeat(orxGRAPHIC *_pstGraphic, orxFLOAT _fRepeatX, orxFLOAT _fRepeatY);

/** Clears graphic color
 * @param[in]   _pstGraphic     Concerned graphic
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxGraphic_ClearColor(orxGRAPHIC *_pstGraphic);

/** Graphic has color accessor
 * @param[in]   _pstGraphic     Concerned graphic
 * @return      orxTRUE / orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL          orxGraphic_HasColor(const orxGRAPHIC *_pstGraphic);

/** Gets graphic color
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[out]  _pstColor       Object's color
 * @return      orxCOLOR / orxNULL
 */
extern orxDLLAPI orxCOLOR *orxFASTCALL        orxGraphic_GetColor(const orxGRAPHIC *_pstGraphic, orxCOLOR *_pstColor);

/** Gets graphic repeat (wrap) values
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[out]  _pfRepeatX      X-axis repeat value
 * @param[out]  _pfRepeatY      Y-axis repeat value
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxGraphic_GetRepeat(const orxGRAPHIC *_pstGraphic, orxFLOAT *_pfRepeatX, orxFLOAT *_pfRepeatY);

/** Sets graphic origin
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[in]   _pvOrigin       Origin coordinates
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxGraphic_SetOrigin(orxGRAPHIC *_pstGraphic, const orxVECTOR *_pvOrigin);

/** Gets graphic origin
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[out]  _pvOrigin       Origin coordinates
 * @return      Origin coordinates
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL       orxGraphic_GetOrigin(const orxGRAPHIC *_pstGraphic, orxVECTOR *_pvOrigin);

/** Updates graphic size (recompute)
 * @param[in]   _pstGraphic     Concerned graphic
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxGraphic_UpdateSize(orxGRAPHIC *_pstGraphic);

/** Sets graphic smoothing
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[in]   _eSmoothing     Smoothing type (enabled, default or none)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxGraphic_SetSmoothing(orxGRAPHIC *_pstGraphic, orxDISPLAY_SMOOTHING _eSmoothing);

/** Gets graphic smoothing
 * @param[in]   _pstGraphic     Concerned graphic
 * @return Smoothing type (enabled, default or none)
 */
extern orxDLLAPI orxDISPLAY_SMOOTHING orxFASTCALL orxGraphic_GetSmoothing(const orxGRAPHIC *_pstGraphic);


/** Sets object blend mode
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[in]   _eBlendMode     Blend mode (alpha, multiply, add or none)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxGraphic_SetBlendMode(orxGRAPHIC *_pstGraphic, orxDISPLAY_BLEND_MODE _eBlendMode);

/** Gets graphic blend mode
 * @param[in]   _pstGraphic     Concerned graphic
 * @return Blend mode (alpha, multiply, add or none)
 */
extern orxDLLAPI orxDISPLAY_BLEND_MODE orxFASTCALL orxGraphic_GetBlendMode(const orxGRAPHIC *_pstGraphic);

#endif /* _orxGRAPHIC_H_ */

/** @} */
