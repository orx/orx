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
 * @file orxTest.m
 * @date 28/01/2010
 * @author iarwain@orx-project.org
 *
 * iPhone test executable
 *
 */


#include "orx.h"

orxOBJECT *spstObject;

void orxFASTCALL Log(const orxCLOCK_INFO *_pstInfo, void *_pContext)
{
  orxClock_AddGlobalTimer(Log, orxFLOAT_1, 1, (void *)!(orxBOOL)_pContext);

  orxLOG("%s (FPS=%ld)", _pContext ? "Tic!" : "Tac!", orxFPS_GetFPS());
}

static orxSTATUS orxFASTCALL Init()
{
  orxViewport_CreateFromConfig("Viewport");
  spstObject = orxObject_CreateFromConfig("Object");
  return orxClock_AddGlobalTimer(Log, orxFLOAT_1, 1, orxNULL);
}

static orxSTATUS orxFASTCALL Run()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  orxVECTOR vPos;

  orxRender_GetWorldPosition(orxMouse_GetPosition(&vPos), &vPos);
  vPos.fZ = orxFLOAT_0;
  orxObject_SetPosition(spstObject, &vPos);
  
  return eResult;
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
