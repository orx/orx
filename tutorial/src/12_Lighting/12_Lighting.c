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
 * As we are *NOT* using the default executable anymore for this tutorial, the tutorial
 * code will be directly built into the executable and not into an external library.
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
 * The performance of this tutorial can be greatly improved, for example, by calculating
 * the normal maps at load time in separate textures and transmitting them to the shader,
 * or by having tighter light control/restriction.
 *
 * Please note that the lighting shader is a basic one, far from any realistic lighting,
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
static  orxVIEWPORT  *pstViewport   = orxNULL;
static  orxOBJECT    *pstScene      = orxNULL;
static  orxU32        u32LightIndex = 0;
static  Light         astLightList[LIGHT_NUMBER];


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
    if(pstPayload->s32ParamIndex <= (orxS32)u32LightIndex)
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
    }
  }

  /* Done! */
  return eResult;
}

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

/** Init function
 */
orxSTATUS orxFASTCALL Init()
{
  orxINPUT_TYPE   eType;
  orxENUM         eID;
  const orxSTRING zInputClearLights;
  const orxSTRING zInputCreateLight;
  const orxSTRING zInputIncreaseRadius;
  const orxSTRING zInputDecreaseRadius;
  const orxSTRING zInputToggleAlpha;
  orxSTATUS       eResult;

  /* Gets binding names */
  orxInput_GetBinding("CreateLight", 0, &eType, &eID);
  zInputCreateLight     = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("ClearLights", 0, &eType, &eID);
  zInputClearLights     = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("IncreaseRadius", 0, &eType, &eID);
  zInputIncreaseRadius  = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("DecreaseRadius", 0, &eType, &eID);
  zInputDecreaseRadius  = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("ToggleAlpha", 0, &eType, &eID);
  zInputToggleAlpha     = orxInput_GetBindingName(eType, eID);

  /* Displays a small hint in console */
  orxLOG("\n- '%s' will create a new light under the cursor"
         "\n- '%s' will clear all the lights from the scene"         
         "\n- '%s' will increase the radius of the current light"
         "\n- '%s' will decrease the radius of the current light"
         "\n- '%s' will toggle alpha on light (ie. make holes in lit objects)",
         zInputCreateLight, zInputClearLights, zInputIncreaseRadius, zInputDecreaseRadius, zInputToggleAlpha);

  /* Creates viewport */
  pstViewport = orxViewport_CreateFromConfig("Viewport");

  /* Creates scene */
  pstScene = orxObject_CreateFromConfig("Scene");

  /* Adds event handler */
  eResult = orxEvent_AddHandler(orxEVENT_TYPE_SHADER, EventHandler);

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

  /* Gets mouse position */
  orxMouse_GetPosition(&(astLightList[u32LightIndex].vPosition));

  /* Creates a new light? */
  if((orxInput_IsActive("CreateLight") != orxFALSE) && (orxInput_HasNewStatus("CreateLight") != orxFALSE))
  {
    /* Updates light index */
    u32LightIndex = orxMIN(LIGHT_NUMBER - 1, u32LightIndex + 1);
  }
  /* Clears all lights? */
  else if((orxInput_IsActive("ClearLights") != orxFALSE) && (orxInput_HasNewStatus("ClearLights") != orxFALSE))
  {
    /* Clears all lights */
    ClearLights();

    /* Resets light index */
    u32LightIndex = 0;
  }
  /* Increases radius? */
  else if((orxInput_IsActive("IncreaseRadius") != orxFALSE) && (orxInput_HasNewStatus("IncreaseRadius") != orxFALSE))
  {
    astLightList[u32LightIndex].fRadius += orxInput_GetValue("IncreaseRadius") * orx2F(0.05f);
  }
  /* Decreases radius? */
  else if((orxInput_IsActive("DecreaseRadius") != orxFALSE) && (orxInput_HasNewStatus("DecreaseRadius") != orxFALSE))
  {
    astLightList[u32LightIndex].fRadius = orxMAX(orxFLOAT_0, astLightList[u32LightIndex].fRadius - orxInput_GetValue("DecreaseRadius") * orx2F(0.05f));
  }
  /* Toggle alpha? */
  else if((orxInput_IsActive("ToggleAlpha") != orxFALSE) && (orxInput_HasNewStatus("ToggleAlpha") != orxFALSE))
  {
    astLightList[u32LightIndex].stColor.fAlpha = orx2F(1.5f) - astLightList[u32LightIndex].stColor.fAlpha;
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
}

/** Main function
 */
int main(int argc, char **argv)
{
  /* Executes a new instance of tutorial */
  orx_Execute(argc, argv, Init, Run, Exit);

  return EXIT_SUCCESS;
}
