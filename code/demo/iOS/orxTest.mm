/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2022 Orx-Project
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
 * @file orxTest.mm
 * @date 28/01/2010
 * @author iarwain@orx-project.org
 *
 * iOS test
 *
 */


#include "orx.h"


static orxOBJECT   *spstGenerator;
static orxVIEWPORT *spstViewport;


static orxSTATUS orxFASTCALL EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(_pstEvent->eType == orxEVENT_TYPE_PHYSICS);

  /* Colliding? */
  if(_pstEvent->eID == orxPHYSICS_EVENT_CONTACT_ADD)
  {
    /* Adds bump FX on both objects */
    orxObject_AddUniqueFX(orxOBJECT(_pstEvent->hSender), "Bump");
    orxObject_AddUniqueFX(orxOBJECT(_pstEvent->hRecipient), "Bump");

    /* Adds bip sound on one of them */
    orxObject_AddSound(orxOBJECT(_pstEvent->hSender), "Bip");
  }

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL Init()
{
  /* Creates viewport */
  spstViewport = orxViewport_CreateFromConfig("Viewport");

  /* Creates generator */
  spstGenerator = orxObject_CreateFromConfig("Generator");

  /* Creates walls */
  orxObject_CreateFromConfig("Walls");

  /* Registers event handler */
  orxEvent_AddHandler(orxEVENT_TYPE_PHYSICS, EventHandler);

  /* Done! */
  return (spstViewport && spstGenerator) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
}

static orxSTATUS orxFASTCALL Run()
{
  orxVECTOR vMousePos, vGravity;

  /* Updates generator's status */
  orxObject_Enable(spstGenerator, orxInput_IsActive("Spawn"));

  /* Gets mouse position in world space */
  if(orxRender_GetWorldPosition(orxMouse_GetPosition(&vMousePos), orxNULL, &vMousePos))
  {
    orxVECTOR vGeneratorPos;

    /* Gets generator position */
    orxObject_GetPosition(spstGenerator, &vGeneratorPos);

    /* Keeps generator's Z coord */
    vMousePos.fZ = vGeneratorPos.fZ;

    /* Updates generator's position */
    orxObject_SetPosition(spstGenerator, &vMousePos);
  }

  /* Gets gravity vector from input */
  orxVector_Set(&vGravity, orxInput_GetValue("GravityX"), -orxInput_GetValue("GravityY"), orxFLOAT_0);

  /* Significant enough? */
  if(orxVector_GetSquareSize(&vGravity) > orx2F(0.5f))
  {
    static orxVECTOR svSmoothedGravity =
    {
      orxFLOAT_0, -orxFLOAT_1, orxFLOAT_0
    };

    /* Gets smoothed gravity from new value (low-pass filter) */
    orxVector_Lerp(&svSmoothedGravity, &svSmoothedGravity, &vGravity, orx2F(0.05f));

    /* Updates camera rotation */
    orxCamera_SetRotation(orxViewport_GetCamera(spstViewport), orxVector_FromCartesianToSpherical(&vGravity, &svSmoothedGravity)->fTheta);
  }

  return orxSTATUS_SUCCESS;
}

static void orxFASTCALL Exit()
{
}

int main(int argc, char *argv[])
{
  /* Launches application */
  orx_Execute(argc, argv, Init, Run, Exit);

  /* Done! */
  return EXIT_SUCCESS;
}
