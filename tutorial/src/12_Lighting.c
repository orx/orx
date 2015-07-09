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
 * @file 12_Lighting.c
 * @date 15/01/2011
 * @author iarwain@orx-project.org
 *
 * Lighting tutorial
 */


/* This is a basic C tutorial showing how to use shaders to do some lighting.
 *
 * See previous tutorials for more info about the basic object creation, clock, animation,
 * viewport, sound, FX, physics/collision, differentiel scrolling handling, stand alone creation,
 * text, font & localization and spawners.
 *
 * This tutorial shows how to do some pixel based lighting in shaders with optional bumpmaps.
 * It's only one of the many possibilities you can achieve with shaders.
 *
 * The code simply deals with an array of lights and allow to change some of their properties
 * such as position or radius.
 *
 * The whole object lighting is done in the fragment shader defined in 12_Lighting.ini.
 *
 * For performance sake, the normap maps are computed for each object's texture
 * the first time the object is loaded.
 * This computation is made on the CPU but it could have been done on the GPU using viewports
 * that would have textures as render target, instead of the screen. Then all the objects would be
 * rendered separately once with a shader which would only compute the normal maps. This technique
 * would improve "loading/init" performances but requires more code to be written.
 * A more efficient way would be to batch the normal map creation: loading all the texture at once
 * and creating the associated normal maps in one pass. We chose to do it on objects creations instead
 * so as to keep this tutorial modular and allow new objects to be added in config by users
 * without any additional knowledge on how the textures will be processed at runtime by the code.
 *
 * Please note that the lighting shader is a very basic one, far from any realistic lighting,
 * and has been kept simple so as to provide a good base for newcomers.
 *
 */


/* Include orx.h */
#include "orx.h"


/* Misc defines
 */
#define LIGHT_NUMBER                  10

/* Light structure
 */
typedef struct __Light_t
{
  orxCOLOR  stColor;
  orxVECTOR vPosition;
  orxFLOAT  fRadius;

} Light;

/** Local storage
 */
static  orxHASHTABLE *pstTextureTable = orxNULL;
static  orxVIEWPORT  *pstViewport     = orxNULL;
static  orxOBJECT    *pstScene        = orxNULL;
static  orxS32        s32LightIndex   = 0;
static  Light         astLightList[LIGHT_NUMBER];


/** Clears all lights
 */
void ClearLights()
{
  orxU32 i;

  /* Pushes lighting config section */
  orxConfig_PushSection("Lighting");

  /* For all lights */
  for(i = 0; i < LIGHT_NUMBER; i++)
  {
    /* Inits it */
    orxConfig_GetVector("Color", &(astLightList[i].stColor.vRGB));
    astLightList[i].stColor.fAlpha = orxFLOAT_0;
    orxVector_Copy(&(astLightList[i].vPosition), &orxVECTOR_0);
    astLightList[i].fRadius = orxConfig_GetFloat("Radius");
  }

  /* Pops config section */
  orxConfig_PopSection();
}

/** Compute a grey image
 */
void ComputeGreyImage(orxU8 *_pu8Buffer, orxU32 _u32BufferSize)
{
  orxU32 i;

  /* For all pixels */
  for(i = 0; i < _u32BufferSize; i += sizeof(orxRGBA))
  {
    orxCOLOR        stColor;
    orxRGBA         u32Pixel;
    const orxVECTOR vBW = {orx2F(0.299f), orx2F(0.587f), orx2F(0.114f)};

    /* Gets pixel's value */
    u32Pixel = orx2RGBA(_pu8Buffer[i], _pu8Buffer[i + 1], _pu8Buffer[i + 2], _pu8Buffer[i + 3]);

    /* Gets normalized color */
    orxColor_SetRGBA(&stColor, u32Pixel);

    /* Gets its grey value */
    orxVector_SetAll(&stColor.vRGB, orxVector_Dot(&stColor.vRGB, &vBW));

    /* Updates pixel value */
    u32Pixel = orxColor_ToRGBA(&stColor);

    /* Stores it */
    _pu8Buffer[i]     = orxRGBA_R(u32Pixel);
    _pu8Buffer[i + 1] = orxRGBA_G(u32Pixel);
    _pu8Buffer[i + 2] = orxRGBA_B(u32Pixel);
    _pu8Buffer[i + 3] = orxRGBA_A(u32Pixel);
  }
}

/** Compute a normal map
 */
void ComputeNormalMap(const orxU8 *_pu8SrcBuffer, orxU8 *_pu8DstBuffer, orxS32 _s32Width, orxS32 _s32Height)
{
  orxS32 i, j;

  /* For all lines */
  for(i = 0; i < _s32Height; i++)
  {
    /* For all columns */
    for(j = 0; j < _s32Width; j++)
    {
      orxS32          s32Index, s32Left, s32Right, s32Up, s32Down;
      orxFLOAT        fLeft, fRight, fUp, fDown;
      orxCOLOR        stNormal;
      orxRGBA         stPixel;
      const orxVECTOR vHalf = {orx2F(0.5f), orx2F(0.5f), orx2F(0.5f)};

      /* Gets pixel's index */
      s32Index = (i * _s32Width + j) * sizeof(orxRGBA);

      /* Gets neighbour indices */
      s32Left   = (i * _s32Width + orxMAX(j - 1, 0)) * sizeof(orxRGBA);
      s32Right  = (i * _s32Width + orxMIN(j + 1, _s32Width - 1)) * sizeof(orxRGBA);
      s32Up     = (orxMAX(i - 1, 0) * _s32Width + j) * sizeof(orxRGBA);
      s32Down   = (orxMIN(i + 1, _s32Height - 1) * _s32Width + j) * sizeof(orxRGBA);

      /* Gets their normalized values */
      fLeft   = _pu8SrcBuffer[s32Left] * orxCOLOR_NORMALIZER;
      fRight  = _pu8SrcBuffer[s32Right] * orxCOLOR_NORMALIZER;
      fUp     = _pu8SrcBuffer[s32Up] * orxCOLOR_NORMALIZER;
      fDown   = _pu8SrcBuffer[s32Down] * orxCOLOR_NORMALIZER;

      /* Gets normal as color */
      orxVector_Add(&stNormal.vRGB, orxVector_Mulf(&stNormal.vRGB, orxVector_Set(&stNormal.vRGB, (fLeft - fRight), fDown - fUp, orx2F(0.5f)), orx2F(0.5f)), &vHalf);
      stNormal.fAlpha = orxFLOAT_1;

      /* Gets pixel value */
      stPixel = orxColor_ToRGBA(&stNormal);

      /* Stores it */
      _pu8DstBuffer[s32Index]     = stPixel.u8R;
      _pu8DstBuffer[s32Index + 1] = stPixel.u8G;
      _pu8DstBuffer[s32Index + 2] = stPixel.u8B;
      _pu8DstBuffer[s32Index + 3] = stPixel.u8A;
    }
  }
}

/** Create the normal map of a texture
 */
void CreateNormalMap(const orxTEXTURE *_pstTexture)
{
  const orxSTRING zName;

  /* Gets its name */
  zName = orxTexture_GetName(_pstTexture);

  /* Valid? */
  if(zName && zName != orxSTRING_EMPTY)
  {
    orxU32 u32CRC;

    /* Gets its CRC */
    u32CRC = orxString_ToCRC(zName);

    /* Does not already exist? */
    if(!orxHashTable_Get(pstTextureTable, u32CRC))
    {
      orxFLOAT    fWidth, fHeight;
      orxU32      u32BufferSize;
      orxBITMAP  *pstBitmap, *pstNMBitmap;
      orxTEXTURE *pstNMTexture;
      orxU8      *pu8SrcBuffer, *pu8DstBuffer;
      orxCHAR     acNMName[256];

      /* Gets its bitmap */
      pstBitmap = orxTexture_GetBitmap(_pstTexture);

      /* Gets it size */
      orxDisplay_GetBitmapSize(pstBitmap, &fWidth, &fHeight);
      u32BufferSize = (orxU32)(fWidth * fHeight) * sizeof(orxRGBA);

      /* Allocates buffers */
      pu8SrcBuffer = (orxU8 *)orxMemory_Allocate(u32BufferSize, orxMEMORY_TYPE_VIDEO);
      pu8DstBuffer = (orxU8 *)orxMemory_Allocate(u32BufferSize, orxMEMORY_TYPE_VIDEO);

      /* Gets its content */
      orxDisplay_GetBitmapData(pstBitmap, pu8SrcBuffer, u32BufferSize);

      /* Gets it in grey shades */
      ComputeGreyImage(pu8SrcBuffer, u32BufferSize);

      /* Computes normal map */
      ComputeNormalMap(pu8SrcBuffer, pu8DstBuffer, (orxS32)fWidth, (orxS32)fHeight);

      /* Creates a new bitmap */
      pstNMBitmap = orxDisplay_CreateBitmap((orxU32)fWidth, (orxU32)fHeight);

      /* Sets its content */
      orxDisplay_SetBitmapData(pstNMBitmap, pu8DstBuffer, u32BufferSize);

      /* Deletes buffers */
      orxMemory_Free(pu8SrcBuffer);
      orxMemory_Free(pu8DstBuffer);

      /* Creates new name with prefix NM_ */
      orxString_NPrint(acNMName, 256, "NM_%s", zName);

      /* Creates a texture for the normal map */
      pstNMTexture = orxTexture_Create();

      /* Sets its bitmap */
      orxTexture_LinkBitmap(pstNMTexture, pstNMBitmap, acNMName);

      /* Add it to the table using the CRC of the original as key */
      orxHashTable_Add(pstTextureTable, u32CRC, pstNMTexture);
    }
  }
}

/* Event handler
 */
orxSTATUS orxFASTCALL EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Set shader param? */
  if((_pstEvent->eType == orxEVENT_TYPE_SHADER) && (_pstEvent->eID == orxSHADER_EVENT_SET_PARAM))
  {
    orxSHADER_EVENT_PAYLOAD *pstPayload;

    /* Gets its payload */
    pstPayload = (orxSHADER_EVENT_PAYLOAD *)_pstEvent->pstPayload;

    /* Is active? */
    if(pstPayload->s32ParamIndex <= s32LightIndex)
    {
      /* Use bumpmap status? */
      if(!orxString_Compare(pstPayload->zParamName, "UseBumpMap"))
      {
        /* Pushes object's section */
        orxConfig_PushSection(orxObject_GetName(orxOBJECT(_pstEvent->hSender)));

        /* Updates bumpmap status */
        pstPayload->fValue = (orxConfig_GetBool("UseBumpMap") != orxFALSE) ? orxFLOAT_1 : orxFLOAT_0;

        /* Pops config section */
        orxConfig_PopSection();
      }
      /* Graphic size? */
      else if(!orxString_Compare(pstPayload->zParamName, "vSize"))
      {
        /* Updates size */
        orxObject_GetSize(orxOBJECT(_pstEvent->hSender), &(pstPayload->vValue));
      }
      /* Light color? */
      else if(!orxString_Compare(pstPayload->zParamName, "avLightColor"))
      {
        /* Updates light color */
        orxVector_Copy(&(pstPayload->vValue), &(astLightList[pstPayload->s32ParamIndex].stColor.vRGB));
      }
      /* Light alpha? */
      else if(!orxString_Compare(pstPayload->zParamName, "afLightAlpha"))
      {
        /* Updates light alpha */
        pstPayload->fValue = astLightList[pstPayload->s32ParamIndex].stColor.fAlpha;
      }
      /* Light position? */
      else if(!orxString_Compare(pstPayload->zParamName, "avLightPos"))
      {
        /* Updates light position */
        orxVector_Copy(&(pstPayload->vValue), &(astLightList[pstPayload->s32ParamIndex].vPosition));
      }
      /* Light radius? */
      else if(!orxString_Compare(pstPayload->zParamName, "afLightRadius"))
      {
        /* Updates light radius */
        pstPayload->fValue = astLightList[pstPayload->s32ParamIndex].fRadius;
      }
      /* Normal map texture? */
      else if(!orxString_Compare(pstPayload->zParamName, "NormalMap"))
      {
        /* Gets associated normal map */
        pstPayload->pstValue = (orxTEXTURE *)orxHashTable_Get(pstTextureTable, orxString_ToCRC(orxTexture_GetName(pstPayload->pstValue)));
      }
    }
  }
  /* Texture loaded? */
  else if((_pstEvent->eType == orxEVENT_TYPE_TEXTURE) && (_pstEvent->eID == orxTEXTURE_EVENT_LOAD))
  {
    /* Creates associated normal map */
    CreateNormalMap(orxTEXTURE(_pstEvent->hSender));
  }

  /* Done! */
  return eResult;
}

/** Init function
 */
orxSTATUS orxFASTCALL Init()
{
  orxINPUT_TYPE   eType;
  orxENUM         eID;
  orxINPUT_MODE   eMode;
  const orxSTRING zInputClearLights;
  const orxSTRING zInputCreateLight;
  const orxSTRING zInputIncreaseRadius;
  const orxSTRING zInputDecreaseRadius;
  const orxSTRING zInputToggleAlpha;
  orxSTATUS       eResult = orxSTATUS_SUCCESS;

  /* Gets binding names */
  orxInput_GetBinding("CreateLight", 0, &eType, &eID, &eMode);
  zInputCreateLight     = orxInput_GetBindingName(eType, eID, eMode);

  orxInput_GetBinding("ClearLights", 0, &eType, &eID, &eMode);
  zInputClearLights     = orxInput_GetBindingName(eType, eID, eMode);

  orxInput_GetBinding("IncreaseRadius", 0, &eType, &eID, &eMode);
  zInputIncreaseRadius  = orxInput_GetBindingName(eType, eID, eMode);

  orxInput_GetBinding("DecreaseRadius", 0, &eType, &eID, &eMode);
  zInputDecreaseRadius  = orxInput_GetBindingName(eType, eID, eMode);

  orxInput_GetBinding("ToggleAlpha", 0, &eType, &eID, &eMode);
  zInputToggleAlpha     = orxInput_GetBindingName(eType, eID, eMode);

  /* Displays a small hint in console */
  orxLOG("\n- '%s' will create a new light under the cursor"
         "\n- '%s' will clear all the lights from the scene"
         "\n- '%s' will increase the radius of the current light"
         "\n- '%s' will decrease the radius of the current light"
         "\n- '%s' will toggle alpha on light (ie. make holes in lit objects)",
         zInputCreateLight, zInputClearLights, zInputIncreaseRadius, zInputDecreaseRadius, zInputToggleAlpha);

  /* Adds event handler */
  orxEvent_AddHandler(orxEVENT_TYPE_SHADER, EventHandler);
  orxEvent_AddHandler(orxEVENT_TYPE_TEXTURE, EventHandler);

  /* Creates texture table */
  pstTextureTable = orxHashTable_Create(16, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

  /* Creates viewport */
  pstViewport = orxViewport_CreateFromConfig("Viewport");

  /* Creates scene */
  pstScene = orxObject_CreateFromConfig("Scene");

  /* Clear all lights */
  ClearLights();

  /* Done! */
  return eResult;
}

/** Run function
 */
orxSTATUS orxFASTCALL Run()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Stores mouse position as current light position */
  orxMouse_GetPosition(&(astLightList[s32LightIndex].vPosition));

  /* Creates a new light? */
  if(orxInput_IsActive("CreateLight") && orxInput_HasNewStatus("CreateLight"))
  {
    /* Updates light index */
    s32LightIndex = orxMIN(LIGHT_NUMBER - 1, s32LightIndex + 1);
  }
  /* Clears all lights? */
  else if(orxInput_IsActive("ClearLights") && orxInput_HasNewStatus("ClearLights"))
  {
    /* Clears all lights */
    ClearLights();

    /* Resets light index */
    s32LightIndex = 0;
  }
  /* Increases radius? */
  else if(orxInput_IsActive("IncreaseRadius") && orxInput_HasNewStatus("IncreaseRadius"))
  {
    astLightList[s32LightIndex].fRadius += orxInput_GetValue("IncreaseRadius") * orx2F(0.05f);
  }
  /* Decreases radius? */
  else if(orxInput_IsActive("DecreaseRadius") && orxInput_HasNewStatus("DecreaseRadius"))
  {
    astLightList[s32LightIndex].fRadius = orxMAX(orxFLOAT_0, astLightList[s32LightIndex].fRadius - orxInput_GetValue("DecreaseRadius") * orx2F(0.05f));
  }
  /* Toggle alpha? */
  else if(orxInput_IsActive("ToggleAlpha") && orxInput_HasNewStatus("ToggleAlpha"))
  {
    astLightList[s32LightIndex].stColor.fAlpha = orx2F(1.5f) - astLightList[s32LightIndex].stColor.fAlpha;
  }
  /* Should quit? */
  else if(orxInput_IsActive("Quit"))
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Exit function
 */
void orxFASTCALL Exit()
{
  /* This time we decided not to be lazy and we deleted what we created: much cleaner! :) */

  /* Deletes scene */
  orxObject_Delete(pstScene);

  /* Deletes viewport */
  orxViewport_Delete(pstViewport);

  /* Deletes texture table */
  orxHashTable_Delete(pstTextureTable);
}

/** Main function
 */
int main(int argc, char **argv)
{
  /* Executes a new instance of tutorial */
  orx_Execute(argc, argv, Init, Run, Exit);

  return EXIT_SUCCESS;
}
