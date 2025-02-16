/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008- Orx-Project
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
 * @file orxDisplay.c
 * @date 08/08/2024
 * @author iarwain@orx-project.org
 *
 * Dummy display plugin implementation
 *
 */


#include "orxPluginAPI.h"

orxSTATUS orxFASTCALL orxDisplay_Dummy_Init()
{
  /* Done! */
  return orxSTATUS_SUCCESS;
}

void orxFASTCALL orxDisplay_Dummy_Exit()
{
  /* Done! */
  return;
}

orxSTATUS orxFASTCALL orxDisplay_Dummy_Swap()
{
  /* Done! */
  return orxSTATUS_SUCCESS;
}

orxBITMAP *orxFASTCALL orxDisplay_Dummy_GetScreenBitmap()
{
  /* Done! */
  return (orxBITMAP *)orxHANDLE_UNDEFINED;
}

orxSTATUS orxFASTCALL orxDisplay_Dummy_GetScreenSize(orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
{
  /* Clears values */
  *_pfWidth = *_pfHeight = orxFLOAT_1;

  /* Done! */
  return orxSTATUS_SUCCESS;
}

orxBITMAP *orxFASTCALL orxDisplay_Dummy_CreateBitmap(orxU32 _u32Width, orxU32 _u32Height)
{
  /* Done! */
  return (orxBITMAP *)orxHANDLE_UNDEFINED;
}

void orxFASTCALL orxDisplay_Dummy_DeleteBitmap(orxBITMAP *_pstBitmap)
{
  /* Done! */
  return;
}

orxBITMAP *orxFASTCALL orxDisplay_Dummy_LoadBitmap(const orxSTRING _zFileName)
{
  /* Done! */
  return (orxBITMAP *)orxHANDLE_UNDEFINED;
}

orxSTATUS orxFASTCALL orxDisplay_Dummy_SaveBitmap(const orxBITMAP *_pstBitmap, const orxSTRING _zFileName)
{
  /* Done! */
  return orxSTATUS_SUCCESS;
}

orxBITMAP *orxFASTCALL orxDisplay_Dummy_LoadFont(const orxSTRING _zFileName, const orxSTRING _zCharacterList, orxFLOAT _fHeight, const orxVECTOR *_pvCharacterSpacing, orxFLOAT *_afCharacterWidthList)
{
  /* Done! */
  return (orxBITMAP *)orxHANDLE_UNDEFINED;
}

orxSTATUS orxFASTCALL orxDisplay_Dummy_SetTempBitmap(const orxBITMAP *_pstBitmap)
{
  /* Done! */
  return orxSTATUS_SUCCESS;
}

const orxBITMAP *orxFASTCALL orxDisplay_Dummy_GetTempBitmap()
{
  /* Done! */
  return orxNULL;
}

orxSTATUS orxFASTCALL orxDisplay_Dummy_SetDestinationBitmaps(orxBITMAP **_apstBitmapList, orxU32 _u32Number)
{
  /* Done! */
  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL orxDisplay_Dummy_ClearBitmap(orxBITMAP *_pstBitmap, orxRGBA _stColor)
{
  /* Done! */
  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL orxDisplay_Dummy_SetBlendMode(orxDISPLAY_BLEND_MODE _eBlendMode)
{
  /* Done! */
  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL orxDisplay_Dummy_SetBitmapClipping(orxBITMAP *_pstBitmap, orxU32 _u32TLX, orxU32 _u32TLY, orxU32 _u32BRX, orxU32 _u32BRY)
{
  /* Done! */
  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL orxDisplay_Dummy_SetBitmapData(orxBITMAP *_pstBitmap, const orxU8 *_au8Data, orxU32 _u32ByteNumber)
{
  /* Done! */
  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL orxDisplay_Dummy_GetBitmapData(const orxBITMAP *_pstBitmap, orxU8 *_au8Data, orxU32 _u32ByteNumber)
{
  /* Done! */
  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL orxDisplay_Dummy_SetPartialBitmapData(orxBITMAP *_pstBitmap, const orxU8 *_au8Data, orxU32 _u32X, orxU32 _u32Y, orxU32 _u32Width, orxU32 _u32Height)
{
  /* Done! */
  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL orxDisplay_Dummy_GetBitmapSize(const orxBITMAP *_pstBitmap, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
{
  /* Clears values */
  *_pfWidth = *_pfHeight = orxFLOAT_1;

  /* Done! */
  return orxSTATUS_SUCCESS;
}

orxU32 orxFASTCALL orxDisplay_Dummy_GetBitmapID(const orxBITMAP *_pstBitmap)
{
  /* Done! */
  return orxU32_UNDEFINED;
}

orxSTATUS orxFASTCALL orxDisplay_Dummy_TransformBitmap(const orxBITMAP *_pstSrc, const orxDISPLAY_TRANSFORM *_pstTransform, orxRGBA _stColor, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  /* Done! */
  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL orxDisplay_Dummy_TransformText(const orxSTRING _zString, const orxBITMAP *_pstFont, const orxCHARACTER_MAP *_pstMap, const orxDISPLAY_TRANSFORM *_pstTransform, orxRGBA _stColor, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  /* Done! */
  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL orxDisplay_Dummy_DrawLine(const orxVECTOR *_pvStart, const orxVECTOR *_pvEnd, orxRGBA _stColor)
{
  /* Done! */
  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL orxDisplay_Dummy_DrawPolyline(const orxVECTOR *_avVertexList, orxU32 _u32VertexNumber, orxRGBA _stColor)
{
  /* Done! */
  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL orxDisplay_Dummy_DrawPolygon(const orxVECTOR *_avVertexList, orxU32 _u32VertexNumber, orxRGBA _stColor, orxBOOL _bFill)
{
  /* Done! */
  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL orxDisplay_Dummy_DrawCircle(const orxVECTOR *_pvCenter, orxFLOAT _fRadius, orxRGBA _stColor, orxBOOL _bFill)
{
  /* Done! */
  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL orxDisplay_Dummy_DrawOBox(const orxOBOX *_pstBox, orxRGBA _stColor, orxBOOL _bFill)
{
  /* Done! */
  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL orxDisplay_Dummy_DrawMesh(const orxDISPLAY_MESH *_pstMesh, const orxBITMAP *_pstBitmap, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  /* Done! */
  return orxSTATUS_SUCCESS;
}

orxBOOL orxFASTCALL orxDisplay_Dummy_HasShaderSupport()
{
  /* Done! */
  return orxFALSE;
}

orxHANDLE orxFASTCALL orxDisplay_Dummy_CreateShader(const orxSTRING *_azCodeList, orxU32 _u32Size, const orxLINKLIST *_pstParamList, orxBOOL _bUseCustomParam)
{
  /* Done! */
  return orxHANDLE_UNDEFINED;
}

void orxFASTCALL orxDisplay_Dummy_DeleteShader(orxHANDLE _hShader)
{
  /* Done! */
  return;
}

orxSTATUS orxFASTCALL orxDisplay_Dummy_StartShader(orxHANDLE _hShader)
{
  /* Done! */
  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL orxDisplay_Dummy_StopShader(orxHANDLE _hShader)
{
  /* Done! */
  return orxSTATUS_SUCCESS;
}

orxS32 orxFASTCALL orxDisplay_Dummy_GetParameterID(const orxHANDLE _hShader, const orxSTRING _zParam, orxS32 _s32Index, orxBOOL _bIsTexture)
{
  /* Done! */
  return -1;
}

orxSTATUS orxFASTCALL orxDisplay_Dummy_SetShaderBitmap(orxHANDLE _hShader, orxS32 _s32ID, const orxBITMAP *_pstValue)
{
  /* Done! */
  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL orxDisplay_Dummy_SetShaderFloat(orxHANDLE _hShader, orxS32 _s32ID, orxFLOAT _fValue)
{
  /* Done! */
  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL orxDisplay_Dummy_SetShaderVector(orxHANDLE _hShader, orxS32 _s32ID, const orxVECTOR *_pvValue)
{
  /* Done! */
  return orxSTATUS_SUCCESS;
}

orxU32 orxFASTCALL orxDisplay_Dummy_GetShaderID(const orxHANDLE _hShader)
{
  /* Done! */
  return orxU32_UNDEFINED;
}

orxSTATUS orxFASTCALL orxDisplay_Dummy_EnableVSync(orxBOOL _bEnable)
{
  /* Done! */
  return orxSTATUS_SUCCESS;
}

orxBOOL orxFASTCALL orxDisplay_Dummy_IsVSyncEnabled()
{
  /* Done! */
  return orxFALSE;
}

orxSTATUS orxFASTCALL orxDisplay_Dummy_SetFullScreen(orxBOOL _bFullScreen)
{
  /* Done! */
  return orxSTATUS_SUCCESS;
}

orxBOOL orxFASTCALL orxDisplay_Dummy_IsFullScreen()
{
  /* Done! */
  return orxFALSE;
}

orxU32 orxFASTCALL orxDisplay_Dummy_GetVideoModeCount()
{
  /* Done! */
  return 0;
}

orxDISPLAY_VIDEO_MODE *orxFASTCALL orxDisplay_Dummy_GetVideoMode(orxU32 _u32Index, orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  /* Done! */
  return orxNULL;
}

orxSTATUS orxFASTCALL orxDisplay_Dummy_SetVideoMode(const orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  /* Done! */
  return orxSTATUS_SUCCESS;
}

orxBOOL orxFASTCALL orxDisplay_Dummy_IsVideoModeAvailable(const orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  /* Done! */
  return orxFALSE;
}


/***************************************************************************
 * Plugin Related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(DISPLAY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_Init, DISPLAY, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_Exit, DISPLAY, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_Swap, DISPLAY, SWAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_GetScreenBitmap, DISPLAY, GET_SCREEN_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_GetScreenSize, DISPLAY, GET_SCREEN_SIZE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_CreateBitmap, DISPLAY, CREATE_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_DeleteBitmap, DISPLAY, DELETE_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_LoadBitmap, DISPLAY, LOAD_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_SaveBitmap, DISPLAY, SAVE_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_LoadFont, DISPLAY, LOAD_FONT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_SetTempBitmap, DISPLAY, SET_TEMP_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_GetTempBitmap, DISPLAY, GET_TEMP_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_SetDestinationBitmaps, DISPLAY, SET_DESTINATION_BITMAPS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_ClearBitmap, DISPLAY, CLEAR_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_SetBlendMode, DISPLAY, SET_BLEND_MODE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_SetBitmapClipping, DISPLAY, SET_BITMAP_CLIPPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_SetBitmapData, DISPLAY, SET_BITMAP_DATA);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_GetBitmapData, DISPLAY, GET_BITMAP_DATA);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_SetPartialBitmapData, DISPLAY, SET_PARTIAL_BITMAP_DATA);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_GetBitmapSize, DISPLAY, GET_BITMAP_SIZE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_GetBitmapID, DISPLAY, GET_BITMAP_ID);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_TransformBitmap, DISPLAY, TRANSFORM_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_TransformText, DISPLAY, TRANSFORM_TEXT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_DrawLine, DISPLAY, DRAW_LINE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_DrawPolyline, DISPLAY, DRAW_POLYLINE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_DrawPolygon, DISPLAY, DRAW_POLYGON);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_DrawCircle, DISPLAY, DRAW_CIRCLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_DrawOBox, DISPLAY, DRAW_OBOX);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_DrawMesh, DISPLAY, DRAW_MESH);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_HasShaderSupport, DISPLAY, HAS_SHADER_SUPPORT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_CreateShader, DISPLAY, CREATE_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_DeleteShader, DISPLAY, DELETE_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_StartShader, DISPLAY, START_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_StopShader, DISPLAY, STOP_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_GetParameterID, DISPLAY, GET_PARAMETER_ID);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_SetShaderBitmap, DISPLAY, SET_SHADER_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_SetShaderFloat, DISPLAY, SET_SHADER_FLOAT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_SetShaderVector, DISPLAY, SET_SHADER_VECTOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_GetShaderID, DISPLAY, GET_SHADER_ID);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_EnableVSync, DISPLAY, ENABLE_VSYNC);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_IsVSyncEnabled, DISPLAY, IS_VSYNC_ENABLED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_SetFullScreen, DISPLAY, SET_FULL_SCREEN);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_IsFullScreen, DISPLAY, IS_FULL_SCREEN);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_GetVideoModeCount, DISPLAY, GET_VIDEO_MODE_COUNT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_GetVideoMode, DISPLAY, GET_VIDEO_MODE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_SetVideoMode, DISPLAY, SET_VIDEO_MODE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Dummy_IsVideoModeAvailable, DISPLAY, IS_VIDEO_MODE_AVAILABLE);
orxPLUGIN_USER_CORE_FUNCTION_END();
