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
 * @file orxKernel.h
 * @date 12/09/2005
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup Orx
 *
 * Kernel include file
 *
 * @{
 */


#ifndef _orxKERNEL_H_
#define _orxKERNEL_H_


/** Includes all the kernel modules of the engine
 */

#include "anim/orxAnim.h"
#include "anim/orxAnimPointer.h"
#include "anim/orxAnimSet.h"

#include "core/orxClock.h"
#include "core/orxConfig.h"
#include "core/orxEvent.h"
#include "core/orxLocale.h"
#include "core/orxSystem.h"

#include "debug/orxDebug.h"
#include "debug/orxFPS.h"

#include "display/orxGraphic.h"
#include "display/orxDisplay.h"
#include "display/orxText.h"
#include "display/orxTexture.h"

#include "io/orxFile.h"
#include "io/orxFileSystem.h"
#include "io/orxInput.h"
#include "io/orxJoystick.h"
#include "io/orxKeyboard.h"
#include "io/orxMouse.h"

#include "main/orxParam.h"

#include "math/orxAABox.h"
#include "math/orxMath.h"
#include "math/orxOBox.h"
#include "math/orxVector.h"

#include "memory/orxBank.h"
#include "memory/orxMemory.h"

#include "object/orxFrame.h"
#include "object/orxObject.h"
#include "object/orxSpawner.h"
#include "object/orxStructure.h"

#include "plugin/orxPlugin.h"
#include "plugin/orxPluginCore.h"
#include "plugin/orxPluginUser.h"

#include "physics/orxBody.h"
#include "physics/orxPhysics.h"

#include "render/orxCamera.h"
#include "render/orxFX.h"
#include "render/orxFXPointer.h"
#include "render/orxRender.h"
#include "render/orxShader.h"
#include "render/orxShaderPointer.h"
#include "render/orxViewport.h"

#include "sound/orxSound.h"
#include "sound/orxSoundPointer.h"
#include "sound/orxSoundSystem.h"


#endif /* _orxKERNEL_H_ */

/** @} */
