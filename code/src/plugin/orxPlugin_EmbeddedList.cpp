/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2011 Orx-Project
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

/* Wii? */
#if defined(__orxWII__)

//! TODO: Includes all plugins to embed for Wii
#include "../plugins/Physics/Box2D/orxPhysics.cpp"
#include "../plugins/Render/Home/orxRender.c"

/* iPhone? */
#elif defined(__orxIPHONE__)

#pragma GCC visibility push(hidden)

/* Display, joystick, mouse & sound system plugins are directly included in the XCode project file as they need to be compiled in Objective-C */
#include "../plugins/Keyboard/Dummy/orxKeyboard.c"
#include "../plugins/Physics/Box2D/orxPhysics.cpp"
#include "../plugins/Render/Home/orxRender.c"

#pragma GCC visibility pop

/* Android? */
#elif defined(__orxANDROID__)

#pragma GCC visibility push(hidden)

#include "../plugins/Display/android/orxDisplay.cpp"
#include "../plugins/Joystick/android/orxJoystick.c"
#include "../plugins/Keyboard/Dummy/orxKeyboard.c"
#include "../plugins/Mouse/android/orxMouse.c"
#include "../plugins/Physics/Box2D/orxPhysics.cpp"
#include "../plugins/Render/Home/orxRender.c"
#include "../plugins/Sound/android/orxSoundSystem.c"

#pragma GCC visibility pop

/* Others */
#else

/** Includes all plugins to embed
 */

/* GLFW, Box2D, OpenAL, SOIL, libsndfile & stb_vorbis */
#include "../plugins/Display/GLFW/orxDisplay.c"
#include "../plugins/Joystick/GLFW/orxJoystick.c"
#include "../plugins/Keyboard/GLFW/orxKeyboard.c"
#include "../plugins/Mouse/GLFW/orxMouse.c"
#include "../plugins/Physics/Box2D/orxPhysics.cpp"
#include "../plugins/Render/Home/orxRender.c"
#include "../plugins/Sound/OpenAL/orxSoundSystem.c"

/* SFML (bad/incomplete shader support, no rendering to texture) & Box2D */
//#include "../plugins/Display/SFML/orxDisplay.cpp"
//#include "../plugins/Joystick/SFML/orxJoystick.cpp"
//#include "../plugins/Keyboard/SFML/orxKeyboard.cpp"
//#include "../plugins/Mouse/SFML/orxMouse.cpp"
//#include "../plugins/Physics/Box2D/orxPhysics.cpp"
//#include "../plugins/Render/Home/orxRender.c"
//#include "../plugins/Sound/SFML/orxSoundSystem.cpp"

#ifndef __orxMAC__

/* SDL (no mac support, no rendering to texture, not optimized, deprecated), Box2D, OpenAL, SOIL, libsndfile & stb_vorbis */
//#include "../plugins/Display/SDL/orxDisplay.c"
//#include "../plugins/Joystick/SDL/orxJoystick.c"
//#include "../plugins/Keyboard/SDL/orxKeyboard.c"
//#include "../plugins/Mouse/SDL/orxMouse.c"
//#include "../plugins/Physics/Box2D/orxPhysics.cpp"
//#include "../plugins/Render/Home/orxRender.c"
//#include "../plugins/Sound/OpenAL/orxSoundSystem.c"

#endif /* __orxMAC__ */

#endif

#endif /* __orxEMBEDDED__ */
