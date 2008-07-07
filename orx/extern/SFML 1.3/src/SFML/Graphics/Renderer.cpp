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
#include <SFML/Graphics/Renderer.hpp>
#include <SFML/Graphics/VideoResource.hpp>
#include <SFML/Window/Window.hpp>


namespace sf
{
////////////////////////////////////////////////////////////
/// Default constructor
////////////////////////////////////////////////////////////
Renderer::Renderer()
{
    // Force context initialization
    Window::ForceContextInit();

    // Initialize GLEW
    glewInit();
}


////////////////////////////////////////////////////////////
/// Virtual destructor
////////////////////////////////////////////////////////////
Renderer::~Renderer()
{
    // Notify resources
    for (std::set<VideoResource*>::iterator i = myResources.begin(); i != myResources.end(); ++i)
        (*i)->DestroyVideoResources();
}


////////////////////////////////////////////////////////////
/// Add a video resource
////////////////////////////////////////////////////////////
void Renderer::AddVideoResource(VideoResource* Resource)
{
    if (Resource)
        myResources.insert(Resource);
}


////////////////////////////////////////////////////////////
/// Remove a video resource
////////////////////////////////////////////////////////////
void Renderer::RemoveVideoResource(VideoResource* Resource)
{
    if (Resource)
        myResources.erase(Resource);
}


////////////////////////////////////////////////////////////
/// Get the global instance of the renderer
////////////////////////////////////////////////////////////
Renderer& GetRenderer()
{
    static Renderer Instance;

    return Instance;
}

} // namespace sf
