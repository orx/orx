/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/**
 * @file
 * @date 02/09/2005
 * @author (C) Arcallians
 * 
 */

#ifndef _orx_H_
#define _orx_H_

/* Include all the module of the project.
 * This file will be used to generate a precompiled header for compiler that
 * support this feature.
 * Orx Engine :   This header must not be used by modules of the engine (only main program)
 * User Plugins : This header is the only one that have to be used
 */

#include "orxInclude.h"

#include "anim/orxAnim.h"
#include "anim/orxAnimPointer.h"
#include "anim/orxAnimSet.h"

#include "camera/orxCamera.h"
#include "camera/orxRender.h"
#include "camera/orxViewport.h"

#include "core/orxClock.h"
#include "core/orxEvent.h"
#include "core/orxTime.h"

#include "debug/orxDebug.h"
#include "debug/orxFps.h"

#include "display/graphic.h"
#include "display/orxDisplay.h"
#include "display/orxTexture.h"

#include "io/interaction.h"
#include "io/orxFile.h"
#include "io/orxJoystick.h"
#include "io/orxKeyboard.h"
#include "io/orxMouse.h"
#include "io/orxPackage.h"
#include "io/orxTextIO.h"

#include "object/orxFrame.h"
#include "object/orxObject.h"
#include "object/orxStructure.h"

#include "plugin/orxPlugin.h"
#include "plugin/orxPluginCore.h"
#include "plugin/orxPluginUser.h"

#include "script/orxScript.h"

#include "sound/orxSound.h"

#include "utils/orxBayes.h"
#include "utils/orxFSM.h"
#include "utils/orxHashTable.h"
#include "utils/orxLinkList.h"
#include "utils/orxQueue.h"
#include "utils/orxScreenshot.h"
#include "utils/orxString.h"
#include "utils/orxTest.h"
#include "utils/orxTree.h"
#include "utils/pathfinder.h"

#endif /*_orx_H_*/
