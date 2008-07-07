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

#ifndef SFML_RENDERWINDOW_HPP
#define SFML_RENDERWINDOW_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Window/Window.hpp>
#include <string>


namespace sf
{
class Drawable;

////////////////////////////////////////////////////////////
/// Simple wrapper for sf::Window that allows easy
/// 2D rendering
////////////////////////////////////////////////////////////
class SFML_API RenderWindow : public Window
{
public :

    ////////////////////////////////////////////////////////////
    /// Default constructor
    ///
    ////////////////////////////////////////////////////////////
    RenderWindow();

    ////////////////////////////////////////////////////////////
    /// Construct the window
    ///
    /// \param Mode :        Video mode to use
    /// \param Title :       Title of the window
    /// \param WindowStyle : Window style (Resize | Close by default)
    /// \param Params :      Creation parameters (see default constructor for default values)
    ///
    ////////////////////////////////////////////////////////////
    RenderWindow(VideoMode Mode, const std::string& Title, unsigned long WindowStyle = Style::Resize | Style::Close, const WindowSettings& Params = WindowSettings());

    ////////////////////////////////////////////////////////////
    /// Construct the window from an existing control
    ///
    /// \param Handle : Platform-specific handle of the control
    /// \param Params : Creation parameters (see default constructor for default values)
    ///
    ////////////////////////////////////////////////////////////
    RenderWindow(WindowHandle Handle, const WindowSettings& Params = WindowSettings());

    ////////////////////////////////////////////////////////////
    /// Destructor
    ///
    ////////////////////////////////////////////////////////////
    virtual ~RenderWindow();

    ////////////////////////////////////////////////////////////
    /// Draw something on the window
    ///
    /// \param Object : Object to draw
    ///
    ////////////////////////////////////////////////////////////
    void Draw(const Drawable& Object) const;

    ////////////////////////////////////////////////////////////
    /// Save the content of the window to an image
    ///
    /// \return Image instance containing the contents of the screen
    ///
    ////////////////////////////////////////////////////////////
    Image Capture() const;

    ////////////////////////////////////////////////////////////
    /// Change the background color of the window.
    /// The default color is black
    ///
    /// \param Col : New background color
    ///
    ////////////////////////////////////////////////////////////
    void SetBackgroundColor(const Color& Col);

    ////////////////////////////////////////////////////////////
    /// Change the current active view.
    ///
    /// \param NewView : New view to use (pass GetDefaultView() to set the default view)
    ///
    ////////////////////////////////////////////////////////////
    void SetView(const View& NewView);

    ////////////////////////////////////////////////////////////
    /// Get the current view
    ///
    /// \return Current view active in the window
    ///
    ////////////////////////////////////////////////////////////
    const View& GetView() const;

    ////////////////////////////////////////////////////////////
    /// Get the default view of the window for read / write
    ///
    /// \return Default view
    ///
    ////////////////////////////////////////////////////////////
    View& GetDefaultView();

    ////////////////////////////////////////////////////////////
    /// Convert a point in window coordinates into view coordinates
    ///
    /// \param WindowX :    X coordinate of the point to convert, relative to the window
    /// \param WindowY :    Y coordinate of the point to convert, relative to the window
    /// \param TargetView : Target view to convert the point to (NULL by default -- uses the current view)
    ///
    /// \return Converted point
    ///
    ////////////////////////////////////////////////////////////
    sf::Vector2f ConvertCoords(unsigned int WindowX, unsigned int WindowY, const View* TargetView = NULL) const;

    ////////////////////////////////////////////////////////////
    /// Tell SFML to preserve external OpenGL states, at the expense of
    /// more CPU charge. Use this function if you don't want SFML
    /// to mess up your own OpenGL states (if any).
    /// Don't enable state preservation if not needed, as it will allow
    /// SFML to do internal optimizations and improve performances.
    /// This parameter is false by default
    ///
    /// \param Preserve : True to preserve OpenGL states, false to let SFML optimize
    ///
    ////////////////////////////////////////////////////////////
    void PreserveOpenGLStates(bool Preserve);

private :

    ////////////////////////////////////////////////////////////
    /// /see Window::OnCreate
    ///
    ////////////////////////////////////////////////////////////
    virtual void OnCreate();

    ////////////////////////////////////////////////////////////
    /// /see Window::OnDisplay
    ///
    ////////////////////////////////////////////////////////////
    virtual void OnDisplay();

    ////////////////////////////////////////////////////////////
    /// Set the OpenGL render states needed for the SFML rendering
    ///
    ////////////////////////////////////////////////////////////
    void SetRenderStates() const;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    Color        myBackgroundColor; ///< Background color
    View         myDefaultView;     ///< Default view
    const View*  myCurrentView;     ///< Current active view
    bool         myPreserveStates;  ///< Should we preserve external OpenGL states ?
    mutable bool myIsDrawing;       ///< True when Draw is called from inside, to allow some renderstates optimizations
};

} // namespace sf


#endif // SFML_RENDERWINDOW_HPP
