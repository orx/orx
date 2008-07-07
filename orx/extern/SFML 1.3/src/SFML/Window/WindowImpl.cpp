////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2008 Laurent Gomila (laurent.gom@gmail.com)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/WindowImpl.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/WindowListener.hpp>
#include <algorithm>
#include <cmath>

#if defined(SFML_SYSTEM_WINDOWS)

    #include <SFML/Window/Win32/WindowImplWin32.hpp>

#elif defined(SFML_SYSTEM_LINUX)

    #include <SFML/Window/Linux/WindowImplX11.hpp>

#elif defined(SFML_SYSTEM_MACOS)

	#if defined(SFML_IMPL_CARBON)

		#include <SFML/Window/OSXCarbon/WindowImplCarbon.hpp>

	#elif defined(SFML_IMPL_COCOA)

		#include <SFML/Window/OSXCocoa/WindowImplCocoa.hpp>

	#endif

#endif


namespace sf
{
namespace priv
{
////////////////////////////////////////////////////////////
// Static member data
////////////////////////////////////////////////////////////
const WindowImpl* WindowImpl::ourCurrent = NULL;


////////////////////////////////////////////////////////////
/// Create a new window depending on the current OS
////////////////////////////////////////////////////////////
WindowImpl* WindowImpl::New()
{
    WindowImpl* Window = NULL;

    #if defined(SFML_SYSTEM_WINDOWS)

        // Win32 window
        Window = new WindowImplWin32();

    #elif defined(SFML_SYSTEM_LINUX)

        // Unix window
        Window = new WindowImplX11();

    #elif defined(SFML_SYSTEM_MACOS)

		#if defined(SFML_IMPL_CARBON)
		
			// Carbon window
			Window = new WindowImplCarbon();
			
		#elif defined(SFML_IMPL_COCOA)
			
			// Cocoa window
			Window = new WindowImplCocoa();
			
		#endif

    #endif

    return Window;
}


////////////////////////////////////////////////////////////
/// Create a new window depending on the current OS
////////////////////////////////////////////////////////////
WindowImpl* WindowImpl::New(VideoMode Mode, const std::string& Title, unsigned long WindowStyle, WindowSettings& Params)
{
    #if defined(SFML_SYSTEM_WINDOWS)

        // Win32 window
        return new WindowImplWin32(Mode, Title, WindowStyle, Params);

    #elif defined(SFML_SYSTEM_LINUX)

        // Unix window
        return new WindowImplX11(Mode, Title, WindowStyle, Params);

    #elif defined(SFML_SYSTEM_MACOS)

		#if defined(SFML_IMPL_CARBON)
		
			// Carbon window
			return new WindowImplCarbon(Mode, Title, WindowStyle, Params);
		
		#elif defined(SFML_IMPL_COCOA)
		
			// Cocoa window
			return new WindowImplCocoa(Mode, Title, WindowStyle, Params);
		
		#endif

    #endif
}


////////////////////////////////////////////////////////////
/// Create a new window depending on the current OS
////////////////////////////////////////////////////////////
WindowImpl* WindowImpl::New(WindowHandle Handle, WindowSettings& Params)
{
    #if defined(SFML_SYSTEM_WINDOWS)

        // Win32 window
        return new WindowImplWin32(Handle, Params);

    #elif defined(SFML_SYSTEM_LINUX)

        // Unix window
        return new WindowImplX11(Handle, Params);

    #elif defined(SFML_SYSTEM_MACOS)

		#if defined(SFML_IMPL_CARBON)
		
			// Carbon window
			return new WindowImplCarbon(Handle, Params);
			
		#elif defined(SFML_IMPL_COCOA)
			
			// Cocoa window
			return new WindowImplCocoa(Handle, Params);
			
		#endif
    #endif
}


////////////////////////////////////////////////////////////
/// Default constructor
////////////////////////////////////////////////////////////
WindowImpl::WindowImpl() :
myWidth       (0),
myHeight      (0),
myJoyThreshold(0.1f)
{
    // Initialize the joysticks
    for (unsigned int i = 0; i < JoysticksCount; ++i)
    {
        myJoysticks[i].Initialize(i);
        myJoyStates[i] = myJoysticks[i].UpdateState();
    }
}


////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
WindowImpl::~WindowImpl()
{
    // Reset the active window
    if (ourCurrent == this)
        ourCurrent = NULL;
}


////////////////////////////////////////////////////////////
/// Add a listener to the window
////////////////////////////////////////////////////////////
void WindowImpl::AddListener(WindowListener* Listener)
{
    if (Listener)
        myListeners.insert(Listener);
}


////////////////////////////////////////////////////////////
/// Remove a listener from the window
////////////////////////////////////////////////////////////
void WindowImpl::RemoveListener(WindowListener* Listener)
{
    myListeners.erase(Listener);
}


////////////////////////////////////////////////////////////
/// Get the client width of the window
////////////////////////////////////////////////////////////
unsigned int WindowImpl::GetWidth() const
{
    return myWidth;
}


////////////////////////////////////////////////////////////
/// Get the client height of the window
////////////////////////////////////////////////////////////
unsigned int WindowImpl::GetHeight() const
{
    return myHeight;
}


////////////////////////////////////////////////////////////
/// Activate of deactivate the window as the current target
/// for rendering
////////////////////////////////////////////////////////////
void WindowImpl::SetActive(bool Active) const
{
    if (Active && (ourCurrent != this))
    {
        MakeActive(true);
        ourCurrent = this;
    }
    else if (!Active && (ourCurrent == this))
    {
        MakeActive(false);
        ourCurrent = NULL;
    }
}


////////////////////////////////////////////////////////////
/// Change the joystick threshold, ie. the value below which
/// no move event will be generated
////////////////////////////////////////////////////////////
void WindowImpl::SetJoystickThreshold(float Threshold)
{
    myJoyThreshold = Threshold;
}


////////////////////////////////////////////////////////////
/// Process incoming events from operating system
////////////////////////////////////////////////////////////
void WindowImpl::DoEvents()
{
    // Read the joysticks state and generate the appropriate events
    ProcessJoystickEvents();

    // Let the derived class process other events
    ProcessEvents();
}


////////////////////////////////////////////////////////////
/// Send an event to listeners
////////////////////////////////////////////////////////////
void WindowImpl::SendEvent(const Event& EventToSend)
{
    for (std::set<WindowListener*>::iterator i = myListeners.begin(); i != myListeners.end(); ++i)
    {
        (*i)->OnEvent(EventToSend);
    }
}


////////////////////////////////////////////////////////////
/// Convert an UTF-8 sequence to UTF-16
////////////////////////////////////////////////////////////
Uint16 WindowImpl::UTF8toUTF16(const Uint8* Source)
{
    // http://www.unicode.org/Public/PROGRAMS/CVTUTF/ConvertUTF.h
    // http://www.unicode.org/Public/PROGRAMS/CVTUTF/ConvertUTF.c

    Uint8 Byte = *Source;

    int    NbBytes = 0;
    Uint32 Offset  = 0;
    if      (Byte < 0xC0) {NbBytes = 0; Offset = 0x00000000;}
    else if (Byte < 0xE0) {NbBytes = 1; Offset = 0x00003080;}
    else if (Byte < 0xF0) {NbBytes = 2; Offset = 0x000E2080;}
    else if (Byte < 0xF8) {NbBytes = 3; Offset = 0x03C82080;}
    else if (Byte < 0xFC) {NbBytes = 4; Offset = 0xFA082080;}
    else                  {NbBytes = 5; Offset = 0x82082080;}

    Uint16 Result = 0;
    for (int i = 0; i < NbBytes; ++i)
    {
        Result = static_cast<Uint16>(Result + *Source++);
        Result <<= 6;
    }

    return static_cast<Uint16>(Result + *Source - Offset);
}


////////////////////////////////////////////////////////////
/// Evaluate a pixel format configuration.
/// This functions can be used by implementations that have
/// several valid formats and want to get the best one
////////////////////////////////////////////////////////////
int WindowImpl::EvaluateConfig(const VideoMode& Mode, const WindowSettings& Settings, int ColorBits, int DepthBits, int StencilBits, int Antialiasing)
{
    return abs(static_cast<int>(Mode.BitsPerPixel          - ColorBits))   +
           abs(static_cast<int>(Settings.DepthBits         - DepthBits))   +
           abs(static_cast<int>(Settings.StencilBits       - StencilBits)) +
           abs(static_cast<int>(Settings.AntialiasingLevel - Antialiasing));
}


////////////////////////////////////////////////////////////
/// Read the joysticks state and generate the appropriate events
////////////////////////////////////////////////////////////
void WindowImpl::ProcessJoystickEvents()
{
    for (unsigned int i = 0; i < JoysticksCount; ++i)
    {
        // Copy the previous state of the joystick and get the new one
        JoystickState PreviousState = myJoyStates[i];
        myJoyStates[i] = myJoysticks[i].UpdateState();

        // Axis
        for (unsigned int j = 0; j < myJoysticks[i].GetAxesCount(); ++j)
        {
            float PrevPos = PreviousState.Axis[j];
            float CurrPos = myJoyStates[i].Axis[j];
            if (fabs(CurrPos - PrevPos) >= myJoyThreshold)
            {
                Event Event;
                Event.Type               = Event::JoyMoved;
                Event.JoyMove.JoystickId = i;
                Event.JoyMove.Axis       = static_cast<Joy::Axis>(j);
                Event.JoyMove.Position   = CurrPos;
                SendEvent(Event);
            }
        }

        // Buttons
        for (unsigned int j = 0; j < myJoysticks[i].GetButtonsCount(); ++j)
        {
            bool PrevPressed = PreviousState.Buttons[j];
            bool CurrPressed = myJoyStates[i].Buttons[j];

            if ((!PrevPressed && CurrPressed) || (PrevPressed && !CurrPressed))
            {
                Event Event;
                Event.Type                 = CurrPressed ? Event::JoyButtonPressed : Event::JoyButtonReleased;
                Event.JoyButton.JoystickId = i;
                Event.JoyButton.Button     = j;
                SendEvent(Event);
            }
        }
    }
}


} // namespace priv

} // namespace sf
