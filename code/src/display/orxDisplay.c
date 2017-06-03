/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2018 Orx-Project
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
 * @date 23/04/2003
 * @author iarwain@orx-project.org
 *
 */


#include "display/orxDisplay.h"
#include "plugin/orxPluginCore.h"


/** Misc defines
 */
#define orxDISPLAY_KZ_ALPHA                       "alpha"
#define orxDISPLAY_KZ_MULTIPLY                    "multiply"
#define orxDISPLAY_KZ_ADD                         "add"
#define orxDISPLAY_KZ_PREMUL                      "premul"


/** Display module setup
 */
void orxFASTCALL orxDisplay_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_DISPLAY, orxMODULE_ID_PLUGIN);
  orxModule_AddDependency(orxMODULE_ID_DISPLAY, orxMODULE_ID_PARAM);
  orxModule_AddDependency(orxMODULE_ID_DISPLAY, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_DISPLAY, orxMODULE_ID_PROFILER);
  orxModule_AddDependency(orxMODULE_ID_DISPLAY, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_DISPLAY, orxMODULE_ID_STRING);
  orxModule_AddDependency(orxMODULE_ID_DISPLAY, orxMODULE_ID_EVENT);
  orxModule_AddDependency(orxMODULE_ID_DISPLAY, orxMODULE_ID_CLOCK);
  orxModule_AddDependency(orxMODULE_ID_DISPLAY, orxMODULE_ID_RESOURCE);
  orxModule_AddDependency(orxMODULE_ID_DISPLAY, orxMODULE_ID_THREAD);

  /* Done! */
  return;
}

/** Gets blend mode from a string
 * @param[in]    _zBlendMode                          String to evaluate
 * @return orxDISPLAY_BLEND_MODE
 */
orxDISPLAY_BLEND_MODE orxFASTCALL orxDisplay_GetBlendModeFromString(const orxSTRING _zBlendMode)
{
  orxDISPLAY_BLEND_MODE eResult;

  /* Alpha blend mode? */
  if(orxString_ICompare(_zBlendMode, orxDISPLAY_KZ_ALPHA) == 0)
  {
    /* Updates blend mode */
    eResult = orxDISPLAY_BLEND_MODE_ALPHA;
  }
  /* Multiply blend mode? */
  else if(orxString_ICompare(_zBlendMode, orxDISPLAY_KZ_MULTIPLY) == 0)
  {
    /* Updates blend mode */
    eResult = orxDISPLAY_BLEND_MODE_MULTIPLY;
  }
  /* Add blend mode? */
  else if(orxString_ICompare(_zBlendMode, orxDISPLAY_KZ_ADD) == 0)
  {
    /* Updates blend mode */
    eResult = orxDISPLAY_BLEND_MODE_ADD;
  }
  /* Pre-multiplied alpha blend mode? */
  else if(orxString_ICompare(_zBlendMode, orxDISPLAY_KZ_PREMUL) == 0)
  {
    /* Updates blend mode */
    eResult = orxDISPLAY_BLEND_MODE_PREMUL;
  }
  else
  {
    /* Updates blend mode */
    eResult = orxDISPLAY_BLEND_MODE_NONE;
  }

  /* Done! */
  return eResult;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_Init, orxSTATUS, void);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_Exit, void, void);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_Swap, orxSTATUS, void);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_GetScreenBitmap, orxBITMAP *, void);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_GetScreenSize, orxSTATUS, orxFLOAT *, orxFLOAT *);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_CreateBitmap, orxBITMAP *, orxU32, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_DeleteBitmap, void, orxBITMAP *);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_LoadBitmap, orxBITMAP *, const orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_SaveBitmap, orxSTATUS, const orxBITMAP *, const orxSTRING);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_SetTempBitmap, orxSTATUS, const orxBITMAP *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_GetTempBitmap, const orxBITMAP *);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_SetDestinationBitmaps, orxSTATUS, orxBITMAP **, orxU32);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_ClearBitmap, orxSTATUS, orxBITMAP *, orxRGBA);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_SetBlendMode, orxSTATUS, orxDISPLAY_BLEND_MODE);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_SetBitmapClipping, orxSTATUS, orxBITMAP *, orxU32, orxU32, orxU32, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_SetBitmapColorKey, orxSTATUS, orxBITMAP *, orxRGBA, orxBOOL);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_SetBitmapData, orxSTATUS, orxBITMAP *, const orxU8 *, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_GetBitmapData, orxSTATUS, const orxBITMAP *, orxU8 *, orxU32);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_SetBitmapColor, orxSTATUS, orxBITMAP *, orxRGBA);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_GetBitmapColor, orxRGBA, const orxBITMAP *);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_GetBitmapSize, orxSTATUS, const orxBITMAP *, orxFLOAT *, orxFLOAT *);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_GetBitmapID, orxU32, const orxBITMAP *);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_TransformBitmap, orxSTATUS, const orxBITMAP *, const orxDISPLAY_TRANSFORM *, orxDISPLAY_SMOOTHING, orxDISPLAY_BLEND_MODE);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_TransformText, orxSTATUS, const orxSTRING, const orxBITMAP *, const orxCHARACTER_MAP *, const orxDISPLAY_TRANSFORM *, orxDISPLAY_SMOOTHING, orxDISPLAY_BLEND_MODE);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_DrawLine, orxSTATUS, const orxVECTOR *, const orxVECTOR *, orxRGBA);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_DrawPolyline, orxSTATUS, const orxVECTOR *, orxU32, orxRGBA);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_DrawPolygon, orxSTATUS, const orxVECTOR *, orxU32, orxRGBA, orxBOOL);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_DrawCircle, orxSTATUS, const orxVECTOR *, orxFLOAT, orxRGBA, orxBOOL);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_DrawOBox, orxSTATUS, const orxOBOX *, orxRGBA, orxBOOL);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_DrawMesh, orxSTATUS, const orxBITMAP *, orxDISPLAY_SMOOTHING, orxDISPLAY_BLEND_MODE, orxU32, const orxDISPLAY_VERTEX *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_DrawCustomMesh, orxSTATUS, orxCUSTOM_MESH *);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_HasShaderSupport, orxBOOL);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_CreateShader, orxHANDLE, const orxSTRING *, orxU32, const orxLINKLIST *, orxBOOL);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_DeleteShader, void, orxHANDLE);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_StartShader, orxSTATUS, const orxHANDLE);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_StopShader, orxSTATUS, const orxHANDLE);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_GetParameterID, orxS32, const orxHANDLE, const orxSTRING, orxS32, orxBOOL);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_SetShaderBitmap, orxSTATUS, orxHANDLE, orxS32, const orxBITMAP *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_SetShaderFloat, orxSTATUS, orxHANDLE, orxS32, orxFLOAT);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_SetShaderVector, orxSTATUS, orxHANDLE, orxS32, const orxVECTOR *);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_EnableVSync, orxSTATUS, orxBOOL);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_IsVSyncEnabled, orxBOOL, void);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_SetFullScreen, orxSTATUS, orxBOOL);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_IsFullScreen, orxBOOL, void);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_GetVideoModeCount, orxU32, void);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_GetVideoMode, orxDISPLAY_VIDEO_MODE *, orxU32, orxDISPLAY_VIDEO_MODE *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_SetVideoMode, orxSTATUS, const orxDISPLAY_VIDEO_MODE *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_IsVideoModeAvailable, orxBOOL, const orxDISPLAY_VIDEO_MODE *);


/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(DISPLAY)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, INIT, orxDisplay_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, EXIT, orxDisplay_Exit)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SWAP, orxDisplay_Swap)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, GET_SCREEN_BITMAP, orxDisplay_GetScreenBitmap)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, GET_SCREEN_SIZE, orxDisplay_GetScreenSize)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, CREATE_BITMAP, orxDisplay_CreateBitmap)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, DELETE_BITMAP, orxDisplay_DeleteBitmap)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, LOAD_BITMAP, orxDisplay_LoadBitmap)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SAVE_BITMAP, orxDisplay_SaveBitmap)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SET_TEMP_BITMAP, orxDisplay_SetTempBitmap)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, GET_TEMP_BITMAP, orxDisplay_GetTempBitmap)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SET_DESTINATION_BITMAPS, orxDisplay_SetDestinationBitmaps)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, CLEAR_BITMAP, orxDisplay_ClearBitmap)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SET_BLEND_MODE, orxDisplay_SetBlendMode)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SET_BITMAP_CLIPPING, orxDisplay_SetBitmapClipping)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SET_BITMAP_COLOR_KEY, orxDisplay_SetBitmapColorKey)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SET_BITMAP_DATA, orxDisplay_SetBitmapData)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, GET_BITMAP_DATA, orxDisplay_GetBitmapData)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SET_BITMAP_COLOR, orxDisplay_SetBitmapColor)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, GET_BITMAP_COLOR, orxDisplay_GetBitmapColor)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, GET_BITMAP_SIZE, orxDisplay_GetBitmapSize)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, GET_BITMAP_ID, orxDisplay_GetBitmapID)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, TRANSFORM_BITMAP, orxDisplay_TransformBitmap)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, TRANSFORM_TEXT, orxDisplay_TransformText)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, DRAW_LINE, orxDisplay_DrawLine)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, DRAW_POLYLINE, orxDisplay_DrawPolyline)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, DRAW_POLYGON, orxDisplay_DrawPolygon)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, DRAW_CIRCLE, orxDisplay_DrawCircle)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, DRAW_OBOX, orxDisplay_DrawOBox)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, DRAW_MESH, orxDisplay_DrawMesh)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, DRAW_CUSTOM_MESH, orxDisplay_DrawCustomMesh)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, HAS_SHADER_SUPPORT, orxDisplay_HasShaderSupport)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, CREATE_SHADER, orxDisplay_CreateShader)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, DELETE_SHADER, orxDisplay_DeleteShader)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, START_SHADER, orxDisplay_StartShader)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, STOP_SHADER, orxDisplay_StopShader)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, GET_PARAMETER_ID, orxDisplay_GetParameterID)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SET_SHADER_BITMAP, orxDisplay_SetShaderBitmap)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SET_SHADER_FLOAT, orxDisplay_SetShaderFloat)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SET_SHADER_VECTOR, orxDisplay_SetShaderVector)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, ENABLE_VSYNC, orxDisplay_EnableVSync)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, IS_VSYNC_ENABLED, orxDisplay_IsVSyncEnabled)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SET_FULL_SCREEN, orxDisplay_SetFullScreen)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, IS_FULL_SCREEN, orxDisplay_IsFullScreen)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, GET_VIDEO_MODE_COUNT, orxDisplay_GetVideoModeCount)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, GET_VIDEO_MODE, orxDisplay_GetVideoMode)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SET_VIDEO_MODE, orxDisplay_SetVideoMode)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, IS_VIDEO_MODE_AVAILABLE, orxDisplay_IsVideoModeAvailable)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(DISPLAY)


/* *** Core function implementations *** */

orxSTATUS orxFASTCALL orxDisplay_Init()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_Init)();
}

void orxFASTCALL orxDisplay_Exit()
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_Exit)();
}

orxSTATUS orxFASTCALL orxDisplay_Swap()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_Swap)();
}

orxSTATUS orxFASTCALL orxDisplay_TransformText(const orxSTRING _zString, const orxBITMAP *_pstFont, const orxCHARACTER_MAP *_pstMap, const orxDISPLAY_TRANSFORM *_pstTransform, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_TransformText)(_zString, _pstFont, _pstMap, _pstTransform, _eSmoothing, _eBlendMode);
}

orxSTATUS orxFASTCALL orxDisplay_DrawLine(const orxVECTOR *_pvStart, const orxVECTOR *_pvEnd, orxRGBA _stColor)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_DrawLine)(_pvStart, _pvEnd, _stColor);
}

orxSTATUS orxFASTCALL orxDisplay_DrawPolyline(const orxVECTOR *_avVertexList, orxU32 _u32VertexNumber, orxRGBA _stColor)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_DrawPolyline)(_avVertexList, _u32VertexNumber, _stColor);
}

orxSTATUS orxFASTCALL orxDisplay_DrawPolygon(const orxVECTOR *_avVertexList, orxU32 _u32VertexNumber, orxRGBA _stColor, orxBOOL _bFill)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_DrawPolygon)(_avVertexList, _u32VertexNumber, _stColor, _bFill);
}

orxSTATUS orxFASTCALL orxDisplay_DrawCircle(const orxVECTOR *_pvCenter, orxFLOAT _fRadius, orxRGBA _stColor, orxBOOL _bFill)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_DrawCircle)(_pvCenter, _fRadius, _stColor, _bFill);
}

orxSTATUS orxFASTCALL orxDisplay_DrawOBox(const orxOBOX *_pstBox, orxRGBA _stColor, orxBOOL _bFill)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_DrawOBox)(_pstBox, _stColor, _bFill);
}

orxSTATUS orxFASTCALL orxDisplay_DrawMesh(const orxBITMAP *_pstBitmap, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode, orxU32 _u32VertexCount, const orxDISPLAY_VERTEX *_astVertexList)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_DrawMesh)(_pstBitmap, _eSmoothing, _eBlendMode, _u32VertexCount, _astVertexList);
}

orxSTATUS orxFASTCALL orxDisplay_DrawCustomMesh(orxCUSTOM_MESH * _pstCustomMesh)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_DrawCustomMesh)(_pstCustomMesh);
}

orxBITMAP *orxFASTCALL orxDisplay_CreateBitmap(orxU32 _u32Width, orxU32 _u32Height)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_CreateBitmap)(_u32Width, _u32Height);
}

void orxFASTCALL orxDisplay_DeleteBitmap(orxBITMAP *_pstBitmap)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_DeleteBitmap)(_pstBitmap);
}

orxBITMAP *orxFASTCALL orxDisplay_GetScreenBitmap()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_GetScreenBitmap)();
}

orxSTATUS orxFASTCALL orxDisplay_GetScreenSize(orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_GetScreenSize)(_pfWidth, _pfHeight);
}

orxSTATUS orxFASTCALL orxDisplay_ClearBitmap(orxBITMAP *_pstBitmap, orxRGBA _stColor)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_ClearBitmap)(_pstBitmap, _stColor);
}

orxSTATUS orxFASTCALL orxDisplay_SetBlendMode(orxDISPLAY_BLEND_MODE _eBlendMode)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_SetBlendMode)(_eBlendMode);
}

orxSTATUS orxFASTCALL orxDisplay_SetDestinationBitmaps(orxBITMAP **_apstBitmapList, orxU32 _u32Number)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_SetDestinationBitmaps)(_apstBitmapList, _u32Number);
}

orxSTATUS orxFASTCALL orxDisplay_TransformBitmap(const orxBITMAP *_pstSrc, const orxDISPLAY_TRANSFORM *_pstTransform, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_TransformBitmap)(_pstSrc, _pstTransform, _eSmoothing, _eBlendMode);
}

orxSTATUS orxFASTCALL orxDisplay_SetBitmapData(orxBITMAP *_pstBitmap, const orxU8 *_au8Data, orxU32 _u32ByteNumber)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_SetBitmapData)(_pstBitmap, _au8Data, _u32ByteNumber);
}

orxSTATUS orxFASTCALL orxDisplay_SetBitmapColorKey(orxBITMAP *_pstBitmap, orxRGBA _stColor, orxBOOL _bEnable)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_SetBitmapColorKey)(_pstBitmap, _stColor, _bEnable);
}

orxSTATUS orxFASTCALL orxDisplay_SetBitmapColor(orxBITMAP *_pstBitmap, orxRGBA _stColor)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_SetBitmapColor)(_pstBitmap, _stColor);
}

orxSTATUS orxFASTCALL orxDisplay_SetBitmapClipping(orxBITMAP *_pstBitmap, orxU32 _u32TLX, orxU32 _u32TLY, orxU32 _u32BRX, orxU32 _u32BRY)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_SetBitmapClipping)(_pstBitmap, _u32TLX, _u32TLY, _u32BRX, _u32BRY);
}

orxU32 orxFASTCALL orxDisplay_GetBitmapID(const orxBITMAP *_pstBitmap)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_GetBitmapID)(_pstBitmap);
}

orxSTATUS orxFASTCALL orxDisplay_SaveBitmap(const orxBITMAP *_pstBitmap, const orxSTRING _zFileName)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_SaveBitmap)(_pstBitmap, _zFileName);
}

orxSTATUS orxFASTCALL orxDisplay_SetTempBitmap(const orxBITMAP *_pstBitmap)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_SetTempBitmap)(_pstBitmap);
}

const orxBITMAP *orxFASTCALL orxDisplay_GetTempBitmap()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_GetTempBitmap)();
}

orxBITMAP *orxFASTCALL orxDisplay_LoadBitmap(const orxSTRING _zFileName)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_LoadBitmap)(_zFileName);
}

orxSTATUS orxFASTCALL orxDisplay_GetBitmapData(const orxBITMAP *_pstBitmap, orxU8 *_au8Data, orxU32 _u32ByteNumber)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_GetBitmapData)(_pstBitmap, _au8Data, _u32ByteNumber);
}

orxRGBA orxFASTCALL orxDisplay_GetBitmapColor(const orxBITMAP *_pstBitmap)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_GetBitmapColor)(_pstBitmap);
}

orxSTATUS orxFASTCALL orxDisplay_GetBitmapSize(const orxBITMAP *_pstBitmap, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_GetBitmapSize)(_pstBitmap, _pfWidth, _pfHeight);
}

orxBOOL orxFASTCALL orxDisplay_HasShaderSupport()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_HasShaderSupport)();
}

orxHANDLE orxFASTCALL orxDisplay_CreateShader(const orxSTRING *_azCodeList, orxU32 _u32Size, const orxLINKLIST *_pstParamList, orxBOOL _bUseCustomParam)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_CreateShader)(_azCodeList, _u32Size, _pstParamList, _bUseCustomParam);
}

void orxFASTCALL orxDisplay_DeleteShader(orxHANDLE _hShader)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_DeleteShader)(_hShader);
}

orxSTATUS orxFASTCALL orxDisplay_StartShader(const orxHANDLE _hShader)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_StartShader)(_hShader);
}

orxSTATUS orxFASTCALL orxDisplay_StopShader(const orxHANDLE _hShader)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_StopShader)(_hShader);
}

orxS32 orxFASTCALL orxDisplay_GetParameterID(orxHANDLE _hShader, const orxSTRING _zParam, orxS32 _s32Index, orxBOOL _bIsTexture)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_GetParameterID)(_hShader, _zParam, _s32Index, _bIsTexture);
}

orxSTATUS orxFASTCALL orxDisplay_SetShaderBitmap(orxHANDLE _hShader, orxS32 _s32ID, const orxBITMAP *_pstValue)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_SetShaderBitmap)(_hShader, _s32ID, _pstValue);
}

orxSTATUS orxFASTCALL orxDisplay_SetShaderFloat(orxHANDLE _hShader, orxS32 _s32ID, orxFLOAT _fValue)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_SetShaderFloat)(_hShader, _s32ID, _fValue);
}

orxSTATUS orxFASTCALL orxDisplay_SetShaderVector(orxHANDLE _hShader, orxS32 _s32ID, const orxVECTOR *_pvValue)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_SetShaderVector)(_hShader, _s32ID, _pvValue);
}

orxSTATUS orxFASTCALL orxDisplay_EnableVSync(orxBOOL _bEnable)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_EnableVSync)(_bEnable);
}

orxBOOL orxFASTCALL orxDisplay_IsVSyncEnabled()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_IsVSyncEnabled)();
}

orxSTATUS orxFASTCALL orxDisplay_SetFullScreen(orxBOOL _bFullScreen)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_SetFullScreen)(_bFullScreen);
}

orxBOOL orxFASTCALL orxDisplay_IsFullScreen()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_IsFullScreen)();
}

orxU32 orxFASTCALL orxDisplay_GetVideoModeCount()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_GetVideoModeCount)();
}

orxDISPLAY_VIDEO_MODE *orxFASTCALL orxDisplay_GetVideoMode(orxU32 _u32Index, orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_GetVideoMode)(_u32Index, _pstVideoMode);
}

orxSTATUS orxFASTCALL orxDisplay_SetVideoMode(const orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_SetVideoMode)(_pstVideoMode);
}

orxBOOL orxFASTCALL orxDisplay_IsVideoModeAvailable(const orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_IsVideoModeAvailable)(_pstVideoMode);
}
