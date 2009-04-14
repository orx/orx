/* Orx - Portable Game Engine
 *
 * Orx is the legal property of its developers, whose names
 * are listed in the COPYRIGHT file distributed 
 * with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * @file orxDisplay.c
 * @date 23/04/2003
 * @author iarwain@orx-project.org
 *
 * @todo
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

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_CreateText, orxDISPLAY_TEXT *, void);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_DeleteText, void, orxDISPLAY_TEXT *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_TransformText, orxSTATUS, orxBITMAP *, const orxDISPLAY_TEXT *, const orxDISPLAY_TRANSFORM *, orxRGBA, orxDISPLAY_BLEND_MODE);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_SetTextString, orxSTATUS, orxDISPLAY_TEXT *, const orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_SetTextFont, orxSTATUS, orxDISPLAY_TEXT *, const orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_GetTextString, orxSTRING, const orxDISPLAY_TEXT *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_GetTextFont, orxSTRING, const orxDISPLAY_TEXT *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_GetTextSize, orxSTATUS, const orxDISPLAY_TEXT *, orxFLOAT *, orxFLOAT *);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_PrintString, orxSTATUS, const orxBITMAP *, const orxSTRING, const orxDISPLAY_TRANSFORM *, orxRGBA);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_CreateBitmap, orxBITMAP *, orxU32, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_DeleteBitmap, void, orxBITMAP *);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_GetScreenBitmap, orxBITMAP *, void);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_GetScreenSize, orxSTATUS, orxFLOAT *, orxFLOAT *);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_ClearBitmap, orxSTATUS, orxBITMAP *, orxRGBA);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_TransformBitmap, orxSTATUS, orxBITMAP *, const orxBITMAP *, const orxDISPLAY_TRANSFORM *, orxDISPLAY_SMOOTHING, orxDISPLAY_BLEND_MODE);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_SetBitmapColorKey, orxSTATUS, orxBITMAP *, orxRGBA, orxBOOL);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_SetBitmapColor, orxSTATUS, orxBITMAP *, orxRGBA);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_SetBitmapClipping, orxSTATUS, orxBITMAP *, orxU32, orxU32, orxU32, orxU32);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_BlitBitmap, orxSTATUS, orxBITMAP *, const orxBITMAP *, orxFLOAT, orxFLOAT, orxDISPLAY_BLEND_MODE);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_SaveBitmap, orxSTATUS, const orxBITMAP *, const orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_LoadBitmap, orxBITMAP *, const orxSTRING);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_GetBitmapColor, orxRGBA, const orxBITMAP *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_GetBitmapSize, orxSTATUS, const orxBITMAP *, orxFLOAT *, orxFLOAT *);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_CreateShader, orxHANDLE, const orxSTRING, const orxLINKLIST *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_DeleteShader, void, orxHANDLE);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_RenderShader, orxSTATUS, const orxHANDLE);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_SetShaderBitmap, orxSTATUS, orxHANDLE, const orxSTRING, orxBITMAP *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_SetShaderFloat, orxSTATUS, orxHANDLE, const orxSTRING, orxFLOAT);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_SetShaderVector, orxSTATUS, orxHANDLE, const orxSTRING, const orxVECTOR *);


orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_GetApplicationInput, orxHANDLE, void);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_EnableVSync, orxSTATUS, orxBOOL);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxDisplay_IsVSyncEnabled, orxBOOL, void);


/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(DISPLAY)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, INIT, orxDisplay_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, EXIT, orxDisplay_Exit)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SWAP, orxDisplay_Swap)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, CREATE_BITMAP, orxDisplay_CreateBitmap)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, DELETE_BITMAP, orxDisplay_DeleteBitmap)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SAVE_BITMAP, orxDisplay_SaveBitmap)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, LOAD_BITMAP, orxDisplay_LoadBitmap)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, TRANSFORM_BITMAP, orxDisplay_TransformBitmap)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, CLEAR_BITMAP, orxDisplay_ClearBitmap)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, BLIT_BITMAP, orxDisplay_BlitBitmap)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SET_BITMAP_COLOR_KEY, orxDisplay_SetBitmapColorKey)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SET_BITMAP_COLOR, orxDisplay_SetBitmapColor)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SET_BITMAP_CLIPPING, orxDisplay_SetBitmapClipping)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, GET_BITMAP_COLOR, orxDisplay_GetBitmapColor)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, GET_BITMAP_SIZE, orxDisplay_GetBitmapSize)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, GET_SCREEN_BITMAP, orxDisplay_GetScreenBitmap)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, GET_SCREEN_SIZE, orxDisplay_GetScreenSize)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, CREATE_SHADER, orxDisplay_CreateShader)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, DELETE_SHADER, orxDisplay_DeleteShader)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, RENDER_SHADER, orxDisplay_RenderShader)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SET_SHADER_BITMAP, orxDisplay_SetShaderBitmap)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SET_SHADER_FLOAT, orxDisplay_SetShaderFloat)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SET_SHADER_VECTOR, orxDisplay_SetShaderVector)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, CREATE_TEXT, orxDisplay_CreateText)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, DELETE_TEXT, orxDisplay_DeleteText)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, TRANSFORM_TEXT, orxDisplay_TransformText)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SET_TEXT_STRING, orxDisplay_SetTextString)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, SET_TEXT_FONT, orxDisplay_SetTextFont)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, GET_TEXT_STRING, orxDisplay_GetTextString)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, GET_TEXT_FONT, orxDisplay_GetTextFont)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, GET_TEXT_SIZE, orxDisplay_GetTextSize)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, PRINT_STRING, orxDisplay_PrintString)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, GET_APPLICATION_INPUT, orxDisplay_GetApplicationInput)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, ENABLE_VSYNC, orxDisplay_EnableVSync)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(DISPLAY, IS_VSYNC_ENABLED, orxDisplay_IsVSyncEnabled)

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

orxDISPLAY_TEXT *orxFASTCALL orxDisplay_CreateText()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_CreateText)();
}

void orxFASTCALL orxDisplay_DeleteText(orxDISPLAY_TEXT *_pstText)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_DeleteText)(_pstText);
}

orxSTATUS orxFASTCALL orxDisplay_TransformText(orxBITMAP *_pstDst, const orxDISPLAY_TEXT *_pstText, const orxDISPLAY_TRANSFORM *_pstTransform, orxRGBA _stColor, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_TransformText)(_pstDst, _pstText, _pstTransform, _stColor, _eBlendMode);
}

orxSTATUS orxFASTCALL orxDisplay_SetTextString(orxDISPLAY_TEXT *_pstText, const orxSTRING _zString)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_SetTextString)(_pstText, _zString);
}

orxSTATUS orxFASTCALL orxDisplay_SetTextFont(orxDISPLAY_TEXT *_pstText, const orxSTRING _zFont)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_SetTextFont)(_pstText, _zFont);
}

const orxSTRING orxFASTCALL orxDisplay_GetTextString(const orxDISPLAY_TEXT *_pstText)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_GetTextString)(_pstText);
}

const orxSTRING orxFASTCALL orxDisplay_GetTextFont(const orxDISPLAY_TEXT *_pstText)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_GetTextFont)(_pstText);
}

orxSTATUS orxFASTCALL orxDisplay_GetTextSize(const orxDISPLAY_TEXT *_pstText, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_GetTextSize)(_pstText, _pfWidth, _pfHeight);
}

orxSTATUS orxFASTCALL orxDisplay_PrintString(const orxBITMAP *_pstBitmap, const orxSTRING _zString, const orxDISPLAY_TRANSFORM *_pstTransform, orxRGBA _stColor)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_PrintString)(_pstBitmap, _zString, _pstTransform, _stColor);
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

orxSTATUS orxFASTCALL orxDisplay_TransformBitmap(orxBITMAP *_pstDst, const orxBITMAP *_pstSrc, const orxDISPLAY_TRANSFORM *_pstTransform, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_TransformBitmap)(_pstDst, _pstSrc, _pstTransform, _eSmoothing, _eBlendMode);
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

orxSTATUS orxFASTCALL orxDisplay_BlitBitmap(orxBITMAP *_pstDst, const orxBITMAP *_pstSrc, orxFLOAT _fPosX, orxFLOAT _fPosY, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_BlitBitmap)(_pstDst, _pstSrc, _fPosX, _fPosY, _eBlendMode);
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

orxSTATUS orxFASTCALL orxDisplay_RenderShader(const orxHANDLE _hShader)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_RenderShader)(_hShader);
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

orxHANDLE orxFASTCALL orxDisplay_GetApplicationInput()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_GetApplicationInput)();
}

orxSTATUS orxFASTCALL orxDisplay_EnableVSync(orxBOOL _bEnable)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_EnableVSync)(_bEnable);
}

orxBOOL orxFASTCALL orxDisplay_IsVSyncEnabled()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_IsVSyncEnabled)();
}
