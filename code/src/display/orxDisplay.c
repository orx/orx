/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2010 Orx-Project
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


/***************************************************************************
 orxDisplay_Setup
 Display module setup.

 returns: nothing
 ***************************************************************************/
void orxFASTCALL orxDisplay_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_DISPLAY, orxMODULE_ID_PLUGIN);
  orxModule_AddDependency(orxMODULE_ID_DISPLAY, orxMODULE_ID_PARAM);
  orxModule_AddDependency(orxMODULE_ID_DISPLAY, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_DISPLAY, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_DISPLAY, orxMODULE_ID_EVENT);
  orxModule_AddDependency(orxMODULE_ID_DISPLAY, orxMODULE_ID_CLOCK);

  return;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_Init, orxSTATUS, void);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_Exit, void, void);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_Swap, orxSTATUS, void);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_TransformText, orxSTATUS, const orxSTRING, const orxBITMAP *, const orxCHARACTER_MAP *, const orxDISPLAY_TRANSFORM *, orxDISPLAY_SMOOTHING, orxDISPLAY_BLEND_MODE);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_CreateBitmap, orxBITMAP *, orxU32, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_DeleteBitmap, void, orxBITMAP *);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_GetScreenBitmap, orxBITMAP *, void);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_GetScreenSize, orxSTATUS, orxFLOAT *, orxFLOAT *);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_ClearBitmap, orxSTATUS, orxBITMAP *, orxRGBA);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_TransformBitmap, orxSTATUS, const orxBITMAP *, const orxDISPLAY_TRANSFORM *, orxDISPLAY_SMOOTHING, orxDISPLAY_BLEND_MODE);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_SetBitmapData, orxSTATUS, orxBITMAP *, const orxU8 *, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_SetBitmapColorKey, orxSTATUS, orxBITMAP *, orxRGBA, orxBOOL);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_SetBitmapColor, orxSTATUS, orxBITMAP *, orxRGBA);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_SetBitmapClipping, orxSTATUS, orxBITMAP *, orxU32, orxU32, orxU32, orxU32);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_SetDestinationBitmap, orxSTATUS, orxBITMAP *);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_BlitBitmap, orxSTATUS, const orxBITMAP *, orxFLOAT, orxFLOAT, orxDISPLAY_SMOOTHING, orxDISPLAY_BLEND_MODE);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_SaveBitmap, orxSTATUS, const orxBITMAP *, const orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_LoadBitmap, orxBITMAP *, const orxSTRING);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_GetBitmapColor, orxRGBA, const orxBITMAP *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_GetBitmapSize, orxSTATUS, const orxBITMAP *, orxFLOAT *, orxFLOAT *);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_CreateShader, orxHANDLE, const orxSTRING, const orxLINKLIST *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_DeleteShader, void, orxHANDLE);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_StartShader, orxSTATUS, const orxHANDLE);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_StopShader, orxSTATUS, const orxHANDLE);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_SetShaderBitmap, orxSTATUS, orxHANDLE, const orxSTRING, orxBITMAP *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_SetShaderFloat, orxSTATUS, orxHANDLE, const orxSTRING, orxFLOAT);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_SetShaderVector, orxSTATUS, orxHANDLE, const orxSTRING, const orxVECTOR *);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_EnableVSync, orxSTATUS, orxBOOL);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_IsVSyncEnabled, orxBOOL, void);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_SetFullScreen, orxSTATUS, orxBOOL);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_IsFullScreen, orxBOOL, void);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_GetVideoModeCounter, orxU32, void);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_GetVideoMode, orxDISPLAY_VIDEO_MODE *, orxU32, orxDISPLAY_VIDEO_MODE *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_SetVideoMode, orxSTATUS, const orxDISPLAY_VIDEO_MODE *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_IsVideoModeAvailable, orxBOOL, const orxDISPLAY_VIDEO_MODE *);


/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(DISPLAY)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, INIT, orxDisplay_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, EXIT, orxDisplay_Exit)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SWAP, orxDisplay_Swap)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, CREATE_BITMAP, orxDisplay_CreateBitmap)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, DELETE_BITMAP, orxDisplay_DeleteBitmap)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SAVE_BITMAP, orxDisplay_SaveBitmap)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, LOAD_BITMAP, orxDisplay_LoadBitmap)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SET_DESTINATION_BITMAP, orxDisplay_SetDestinationBitmap)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, TRANSFORM_BITMAP, orxDisplay_TransformBitmap)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, CLEAR_BITMAP, orxDisplay_ClearBitmap)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, BLIT_BITMAP, orxDisplay_BlitBitmap)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SET_BITMAP_DATA, orxDisplay_SetBitmapData)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SET_BITMAP_COLOR_KEY, orxDisplay_SetBitmapColorKey)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SET_BITMAP_COLOR, orxDisplay_SetBitmapColor)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SET_BITMAP_CLIPPING, orxDisplay_SetBitmapClipping)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, GET_BITMAP_COLOR, orxDisplay_GetBitmapColor)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, GET_BITMAP_SIZE, orxDisplay_GetBitmapSize)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, GET_SCREEN_BITMAP, orxDisplay_GetScreenBitmap)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, GET_SCREEN_SIZE, orxDisplay_GetScreenSize)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, CREATE_SHADER, orxDisplay_CreateShader)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, DELETE_SHADER, orxDisplay_DeleteShader)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, START_SHADER, orxDisplay_StartShader)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, STOP_SHADER, orxDisplay_StopShader)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SET_SHADER_BITMAP, orxDisplay_SetShaderBitmap)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SET_SHADER_FLOAT, orxDisplay_SetShaderFloat)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SET_SHADER_VECTOR, orxDisplay_SetShaderVector)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, TRANSFORM_TEXT, orxDisplay_TransformText)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, ENABLE_VSYNC, orxDisplay_EnableVSync)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, IS_VSYNC_ENABLED, orxDisplay_IsVSyncEnabled)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SET_FULL_SCREEN, orxDisplay_SetFullScreen)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, IS_FULL_SCREEN, orxDisplay_IsFullScreen)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, GET_VIDEO_MODE_COUNTER, orxDisplay_GetVideoModeCounter)
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

orxSTATUS orxFASTCALL orxDisplay_SetDestinationBitmap(orxBITMAP *_pstDst)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_SetDestinationBitmap)(_pstDst);
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

orxSTATUS orxFASTCALL orxDisplay_BlitBitmap(const orxBITMAP *_pstSrc, orxFLOAT _fPosX, orxFLOAT _fPosY, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_BlitBitmap)(_pstSrc, _fPosX, _fPosY, _eSmoothing, _eBlendMode);
}

orxSTATUS orxFASTCALL orxDisplay_SaveBitmap(const orxBITMAP *_pstBitmap, const orxSTRING _zFileName)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_SaveBitmap)(_pstBitmap, _zFileName);
}

orxBITMAP *orxFASTCALL orxDisplay_LoadBitmap(const orxSTRING _zFileName)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_LoadBitmap)(_zFileName);
}

orxSTATUS orxFASTCALL orxDisplay_GetBitmapSize(const orxBITMAP *_pstBitmap, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_GetBitmapSize)(_pstBitmap, _pfWidth, _pfHeight);
}

orxRGBA orxFASTCALL orxDisplay_GetBitmapColor(const orxBITMAP *_pstBitmap)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_GetBitmapColor)(_pstBitmap);
}

orxHANDLE orxFASTCALL orxDisplay_CreateShader(const orxSTRING _zCode, const orxLINKLIST *_pstParamList)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_CreateShader)(_zCode, _pstParamList);
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

orxSTATUS orxFASTCALL orxDisplay_SetShaderBitmap(orxHANDLE _hShader, const orxSTRING _zParam, orxBITMAP *_pstValue)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_SetShaderBitmap)(_hShader, _zParam, _pstValue);
}

orxSTATUS orxFASTCALL orxDisplay_SetShaderFloat(orxHANDLE _hShader, const orxSTRING _zParam, orxFLOAT _fValue)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_SetShaderFloat)(_hShader, _zParam, _fValue);
}

orxSTATUS orxFASTCALL orxDisplay_SetShaderVector(orxHANDLE _hShader, const orxSTRING _zParam, const orxVECTOR *_pvValue)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_SetShaderVector)(_hShader, _zParam, _pvValue);
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

orxU32 orxFASTCALL orxDisplay_GetVideoModeCounter()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_GetVideoModeCounter)();
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
