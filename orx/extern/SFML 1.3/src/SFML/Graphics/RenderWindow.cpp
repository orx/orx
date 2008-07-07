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
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Renderer.hpp>
#include <iostream>


namespace sf
{
////////////////////////////////////////////////////////////
/// Default constructor
////////////////////////////////////////////////////////////
RenderWindow::RenderWindow() :
myBackgroundColor(Color(0, 0, 0, 255)),
myCurrentView    (&myDefaultView),
myPreserveStates (false),
myIsDrawing      (false)
{

}


////////////////////////////////////////////////////////////
/// Construct the window
////////////////////////////////////////////////////////////
RenderWindow::RenderWindow(VideoMode Mode, const std::string& Title, unsigned long WindowStyle, const WindowSettings& Params) :
myBackgroundColor(Color(0, 0, 0, 255)),
myCurrentView    (&myDefaultView),
myPreserveStates (false),
myIsDrawing      (false)
{
    Create(Mode, Title, WindowStyle, Params);
}


////////////////////////////////////////////////////////////
/// Construct the window from an existing control
////////////////////////////////////////////////////////////
RenderWindow::RenderWindow(WindowHandle Handle, const WindowSettings& Params) :
myBackgroundColor(Color(0, 0, 0, 255)),
myCurrentView    (&myDefaultView),
myPreserveStates (false),
myIsDrawing      (false)
{
    Create(Handle, Params);
}


////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
RenderWindow::~RenderWindow()
{
    // Nothing to do...
}


////////////////////////////////////////////////////////////
/// Draw something on the window
////////////////////////////////////////////////////////////
void RenderWindow::Draw(const Drawable& Object) const
{
    // Check whether we are called from the outside or from a previous call to Draw
    if (!myIsDrawing)
    {
        myIsDrawing = true;

        // Set our window as the current target for rendering
        if (SetActive())
        {
            // Save the current render states and set the SFML ones
            if (myPreserveStates)
            {
                GLCheck(glMatrixMode(GL_MODELVIEW));  GLCheck(glPushMatrix());
                GLCheck(glMatrixMode(GL_PROJECTION)); GLCheck(glPushMatrix());
                GLCheck(glPushAttrib(GL_COLOR_BUFFER_BIT | GL_CURRENT_BIT   | GL_ENABLE_BIT  |
                                     GL_TEXTURE_BIT      | GL_TRANSFORM_BIT | GL_VIEWPORT_BIT));
                SetRenderStates();
            }

            // Set the window viewport and transform matrices
            GLCheck(glViewport(0, 0, GetWidth(), GetHeight()));
            GLCheck(glMatrixMode(GL_PROJECTION)); GLCheck(glLoadMatrixf(myCurrentView->GetMatrix().Get4x4Elements()));
            GLCheck(glMatrixMode(GL_MODELVIEW));  GLCheck(glLoadIdentity());

            // Let the object draw itself
            Object.Draw(*this);

            // Restore render states
            if (myPreserveStates)
            {
                GLCheck(glMatrixMode(GL_PROJECTION)); GLCheck(glPopMatrix());
                GLCheck(glMatrixMode(GL_MODELVIEW));  GLCheck(glPopMatrix());
                GLCheck(glPopAttrib());
            }
        }

        myIsDrawing = false;
    }
    else
    {
        // We are already called from a previous Draw : we don't need to set the states again, just draw the object
        Object.Draw(*this);
    }
}


////////////////////////////////////////////////////////////
/// Save the content of the window to an image
////////////////////////////////////////////////////////////
Image RenderWindow::Capture() const
{
    // Get the window dimensions
    const unsigned int Width  = GetWidth();
    const unsigned int Height = GetHeight();

    // Set our window as the current target for rendering
    if (SetActive())
    {
        // Get pixels from the backbuffer
        std::vector<Uint8> Pixels(Width * Height * 4);
        Uint8* PixelsPtr = &Pixels[0];
        GLCheck(glReadPixels(0, 0, Width, Height, GL_RGBA, GL_UNSIGNED_BYTE, PixelsPtr));

        // Flip the pixels
        unsigned int Pitch = Width * 4;
        for (unsigned int y = 0; y < Height / 2; ++y)
            std::swap_ranges(PixelsPtr + y * Pitch, PixelsPtr + (y + 1) * Pitch, PixelsPtr + (Height - y - 1) * Pitch);

        // Create an image from the pixel buffer and return it
        return Image(Width, Height, PixelsPtr);
    }
    else
    {
        return Image(Width, Height, Color::White);
    }
}


////////////////////////////////////////////////////////////
/// Change the background color of the window
////////////////////////////////////////////////////////////
void RenderWindow::SetBackgroundColor(const Color& Col)
{
    myBackgroundColor = Col;
}


////////////////////////////////////////////////////////////
/// Change the current active view
////////////////////////////////////////////////////////////
void RenderWindow::SetView(const View& NewView)
{
    myCurrentView = &NewView;
}


////////////////////////////////////////////////////////////
/// Get the current view
////////////////////////////////////////////////////////////
const View& RenderWindow::GetView() const
{
    return *myCurrentView;
}


////////////////////////////////////////////////////////////
/// Get the default view of the window for read / write
////////////////////////////////////////////////////////////
View& RenderWindow::GetDefaultView()
{
    return myDefaultView;
}


////////////////////////////////////////////////////////////
/// Convert a point in window coordinates into view coordinates
////////////////////////////////////////////////////////////
sf::Vector2f RenderWindow::ConvertCoords(unsigned int WindowX, unsigned int WindowY, const View* TargetView) const
{
    // Use the current view if none has been passed
    if (!TargetView)
        TargetView = myCurrentView;

    float Left   = TargetView->GetCenter().x - TargetView->GetHalfSize().x;
    float Top    = TargetView->GetCenter().y - TargetView->GetHalfSize().y;
    float Right  = TargetView->GetCenter().x + TargetView->GetHalfSize().x;
    float Bottom = TargetView->GetCenter().y + TargetView->GetHalfSize().y;

    return sf::Vector2f(Left + WindowX * (Right - Left) / GetWidth(),
                        Top  + WindowY * (Bottom - Top) / GetHeight());
}

////////////////////////////////////////////////////////////
/// Tell SFML to preserve external OpenGL states, at the expense of
/// more CPU charge. Use this function if you don't want SFML
/// to mess up your own OpenGL states (if any).
/// Don't enable state preservation if not needed, as it will allow
/// SFML to do internal optimizations and improve performances.
/// This parameter is false by default
////////////////////////////////////////////////////////////
void RenderWindow::PreserveOpenGLStates(bool Preserve)
{
    myPreserveStates = Preserve;
}


////////////////////////////////////////////////////////////
/// Called after the window has been created
////////////////////////////////////////////////////////////
void RenderWindow::OnCreate()
{
    // Set the default rendering states
    SetRenderStates();

    // Setup the default view
    myDefaultView.SetFromRect(FloatRect(0, 0, static_cast<float>(GetWidth()), static_cast<float>(GetHeight())));
    SetView(myDefaultView);
}


////////////////////////////////////////////////////////////
/// Called when the window displays its content on screen
////////////////////////////////////////////////////////////
void RenderWindow::OnDisplay()
{
    // Clear the frame buffer with the background color, for next frame
    GLCheck(glClearColor(myBackgroundColor.r / 255.f,
                         myBackgroundColor.g / 255.f,
                         myBackgroundColor.b / 255.f,
                         myBackgroundColor.a / 255.f));
    GLCheck(glClear(GL_COLOR_BUFFER_BIT));
}


////////////////////////////////////////////////////////////
/// Set the OpenGL render states needed for the SFML rendering
////////////////////////////////////////////////////////////
void RenderWindow::SetRenderStates() const
{
    GLCheck(glDisable(GL_ALPHA_TEST));
    GLCheck(glDisable(GL_DEPTH_TEST));
    GLCheck(glDisable(GL_LIGHTING)); 
}

} // namespace sf
