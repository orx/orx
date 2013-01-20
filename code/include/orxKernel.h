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
#include "core/orxCommand.h"
#include "core/orxConfig.h"
#include "core/orxConsole.h"
#include "core/orxEvent.h"
#include "core/orxLocale.h"
#include "core/orxResource.h"
#include "core/orxSystem.h"

#include "debug/orxDebug.h"
#include "debug/orxFPS.h"
#include "debug/orxProfiler.h"

#include "display/orxGraphic.h"
#include "display/orxDisplay.h"
#include "display/orxFont.h"
#include "display/orxScreenshot.h"
#include "display/orxText.h"
#include "display/orxTexture.h"

#include "io/orxFile.h"
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
#include "object/orxFX.h"
#include "object/orxFXPointer.h"
#include "object/orxObject.h"
#include "object/orxSpawner.h"
#include "object/orxStructure.h"
#include "object/orxTimeLine.h"

#include "plugin/orxPlugin.h"
#include "plugin/orxPluginCore.h"
#include "plugin/orxPluginUser.h"

#include "physics/orxBody.h"
#include "physics/orxPhysics.h"

#include "render/orxCamera.h"
#include "render/orxRender.h"
#include "render/orxShader.h"
#include "render/orxShaderPointer.h"
#include "render/orxViewport.h"

#include "sound/orxSound.h"
#include "sound/orxSoundPointer.h"
#include "sound/orxSoundSystem.h"


#endif /* _orxKERNEL_H_ */

/** @} */
