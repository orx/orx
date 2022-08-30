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
 * @file orxPlugin_EmbeddedList.cpp
 * @date 03/05/2009
 * @author iarwain@orx-project.org
 *
 */


#include "orxInclude.h"


#ifdef __orxEMBEDDED__

/* iOS? */
#if defined(__orxIOS__)

#pragma GCC visibility push(hidden)

/* Display, joystick, mouse & sound system plugins are directly included in the XCode project file as they need to be compiled in Objective-C */
#include "../plugins/Keyboard/Dummy/orxKeyboard.c"
#include "../plugins/Physics/LiquidFun/orxPhysics.cpp"
#include "../plugins/Render/Home/orxRender.c"

#pragma GCC visibility pop

/* Android? */
#elif defined(__orxANDROID__) || defined(__orxANDROID_NATIVE__)

#include "../plugins/Display/android/orxDisplay.c"
#include "../plugins/Joystick/android/orxJoystick.c"
#include "../plugins/Keyboard/android/orxKeyboard.cpp"
#include "../plugins/Mouse/android/orxMouse.c"
#include "../plugins/Physics/LiquidFun/orxPhysics.cpp"
#include "../plugins/Render/Home/orxRender.c"
#include "../plugins/Sound/MiniAudio/orxSoundSystem.c"

/* Others */
#else /* __orxANDROID__ || __orxANDROID_NATIVE__ */

/** Includes all plugins to embed
 */

/* GLFW, Box2D, MiniAudio, stb_image, libsndfile & stb_vorbis */
#include "../plugins/Display/GLFW/orxDisplay.c"
#include "../plugins/Joystick/GLFW/orxJoystick.c"
#include "../plugins/Keyboard/GLFW/orxKeyboard.c"
#include "../plugins/Mouse/GLFW/orxMouse.c"
#include "../plugins/Physics/LiquidFun/orxPhysics.cpp"
#include "../plugins/Render/Home/orxRender.c"
#include "../plugins/Sound/MiniAudio/orxSoundSystem.c"

#endif /* __orxANDROID__ || __orxANDROID_NATIVE__ */

#endif /* __orxEMBEDDED__ */
