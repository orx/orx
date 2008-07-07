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
#include <SFML/Graphics/PostFX.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Renderer.hpp>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>


namespace sf
{
////////////////////////////////////////////////////////////
/// Default constructor
////////////////////////////////////////////////////////////
PostFX::PostFX() :
myShaderProgram(0)
{
    // No filtering on frame buffer
    myFrameBuffer.SetSmooth(false);
}


////////////////////////////////////////////////////////////
/// Copy constructor
////////////////////////////////////////////////////////////
PostFX::PostFX(const PostFX& Copy) :
Drawable        (Copy),
VideoResource   (Copy),
myShaderProgram (0),
mySamplers      (Copy.mySamplers),
myFragmentShader(Copy.myFragmentShader),
myFrameBuffer   (Copy.myFrameBuffer)
{
    // No filtering on frame buffer
    myFrameBuffer.SetSmooth(false);

    // Create the shaders and the program
    if (Copy.myShaderProgram)
        CreateProgram();
}


////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
PostFX::~PostFX()
{
    DestroyVideoResources();
}


////////////////////////////////////////////////////////////
/// Load the effect from a file
////////////////////////////////////////////////////////////
bool PostFX::LoadFromFile(const std::string& Filename)
{
    // Open the file
    std::ifstream File(Filename.c_str());
    if (!File)
    {
        std::cerr << "Failed to open effect file \"" << Filename << "\"" << std::endl;
        return false;
    }

    // Apply the preprocessing pass to the fragment shader code
    myFragmentShader = PreprocessEffect(File);

    // Create the shaders and the program
    CreateProgram();

    return myShaderProgram != 0;
}


////////////////////////////////////////////////////////////
/// Load the effect from a text in memory
////////////////////////////////////////////////////////////
bool PostFX::LoadFromMemory(const std::string& Effect)
{
    // Open a stream and copy the effect code
    std::istringstream Stream(Effect.c_str());

    // Apply the preprocessing pass to the fragment shader code
    myFragmentShader = PreprocessEffect(Stream);

    // Create the shaders and the program
    CreateProgram();

    return myShaderProgram != 0;
}


////////////////////////////////////////////////////////////
/// Change a parameter of the effect (1 float)
////////////////////////////////////////////////////////////
void PostFX::SetParameter(const std::string& Name, float X)
{
    if (myShaderProgram)
    {
        // Enable program
        GLCheck(glUseProgramObjectARB(myShaderProgram));

        // Get parameter location and assign it new values
        GLint Location = glGetUniformLocationARB(myShaderProgram, Name.c_str());
        if (Location != -1)
            GLCheck(glUniform1fARB(Location, X));
        else
            std::cerr << "Parameter \"" << Name << "\" not found in post-effect" << std::endl;

        // Disable program
        GLCheck(glUseProgramObjectARB(0));
    }
}


////////////////////////////////////////////////////////////
/// Change a parameter of the effect (2 floats)
////////////////////////////////////////////////////////////
void PostFX::SetParameter(const std::string& Name, float X, float Y)
{
    if (myShaderProgram)
    {
        // Enable program
        GLCheck(glUseProgramObjectARB(myShaderProgram));

        // Get parameter location and assign it new values
        GLint Location = glGetUniformLocationARB(myShaderProgram, Name.c_str());
        if (Location != -1)
            GLCheck(glUniform2fARB(Location, X, Y));
        else
            std::cerr << "Parameter \"" << Name << "\" not found in post-effect" << std::endl;

        // Disable program
        GLCheck(glUseProgramObjectARB(0));
    }
}


////////////////////////////////////////////////////////////
/// Change a parameter of the effect (3 floats)
////////////////////////////////////////////////////////////
void PostFX::SetParameter(const std::string& Name, float X, float Y, float Z)
{
    if (myShaderProgram)
    {
        // Enable program
        GLCheck(glUseProgramObjectARB(myShaderProgram));

        // Get parameter location and assign it new values
        GLint Location = glGetUniformLocationARB(myShaderProgram, Name.c_str());
        if (Location != -1)
            GLCheck(glUniform3fARB(Location, X, Y, Z));
        else
            std::cerr << "Parameter \"" << Name << "\" not found in post-effect" << std::endl;

        // Disable program
        GLCheck(glUseProgramObjectARB(0));
    }
}


////////////////////////////////////////////////////////////
/// Change a parameter of the effect (4 floats)
////////////////////////////////////////////////////////////
void PostFX::SetParameter(const std::string& Name, float X, float Y, float Z, float W)
{
    if (myShaderProgram)
    {
        // Enable program
        GLCheck(glUseProgramObjectARB(myShaderProgram));

        // Get parameter location and assign it new values
        GLint Location = glGetUniformLocationARB(myShaderProgram, Name.c_str());
        if (Location != -1)
            GLCheck(glUniform4fARB(Location, X, Y, Z, W));
        else
            std::cerr << "Parameter \"" << Name << "\" not found in post-effect" << std::endl;

        // Disable program
        GLCheck(glUseProgramObjectARB(0));
    }
}


////////////////////////////////////////////////////////////
/// Set a texture parameter
////////////////////////////////////////////////////////////
void PostFX::SetTexture(const std::string& Name, Image* Texture)
{
    // Just store the texture for later use
    mySamplers[Name] = Texture ? Texture : &myFrameBuffer;
}


////////////////////////////////////////////////////////////
/// Assignment operator
////////////////////////////////////////////////////////////
PostFX& PostFX::operator =(const PostFX& Other)
{
    PostFX Temp(Other);

    std::swap(myShaderProgram,  Temp.myShaderProgram);
    std::swap(mySamplers,       Temp.mySamplers);
    std::swap(myFragmentShader, Temp.myFragmentShader);
    std::swap(myFrameBuffer,    Temp.myFrameBuffer);

    return *this;
}


////////////////////////////////////////////////////////////
/// Tell whether or not the system supports post-effects
////////////////////////////////////////////////////////////
bool PostFX::CanUsePostFX()
{
    // Make sure the renderer exists
    GetRenderer();

    return glewIsSupported("GL_ARB_shading_language_100") != 0 &&
           glewIsSupported("GL_ARB_shader_objects")       != 0 &&
           glewIsSupported("GL_ARB_vertex_shader")        != 0 &&
           glewIsSupported("GL_ARB_fragment_shader")      != 0;
}


////////////////////////////////////////////////////////////
/// /see sfDrawable::Render
////////////////////////////////////////////////////////////
void PostFX::Render(const RenderWindow& Window) const
{
    // Check that we have a valid program
    if (!myShaderProgram)
        return;

    // If window dimensions have changed, recreate the frame buffer texture
    if ((Window.GetWidth() != myFrameBuffer.GetWidth()) || (Window.GetHeight() != myFrameBuffer.GetHeight()))
    {
        myFrameBuffer.Resize(Window.GetWidth(), Window.GetHeight());
    }

    // Copy the current framebuffer pixels to our frame buffer texture
    myFrameBuffer.Bind();
    GLCheck(glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, myFrameBuffer.GetWidth(), myFrameBuffer.GetHeight()));

    // Enable program
    GLCheck(glUseProgramObjectARB(myShaderProgram));

    // Bind textures
    GLint MaxUnits;
    GLCheck(glGetIntegerv(GL_MAX_TEXTURE_COORDS_ARB, &MaxUnits));
    unsigned int Unit = 0;
    for (std::map<std::string, Image*>::const_iterator i = mySamplers.begin(); i != mySamplers.end(); ++i)
    {
        // Check that the current texture unit is available
        if (Unit >= static_cast<unsigned int>(MaxUnits))
        {
            // Error : we have reached the maximum texture units count
            std::cerr << "Impossible to use texture \"" << i->first << "\" for post-effect : all available texture units are used" << std::endl;
            break;
        }

        // Get texture location and assign it the given unit
        int Location = glGetUniformLocationARB(myShaderProgram, i->first.c_str());
        if (Location != -1)
        {
            GLCheck(glUniform1iARB(Location, Unit));
            GLCheck(glActiveTextureARB(GL_TEXTURE0_ARB + Unit));
            i->second->Bind();
            Unit++;
        }
        else
        {
            std::cerr << "Texture \"" << i->first << "\" not found in post-effect" << std::endl;
        }
    }

    // Compute the texture coordinates (in case the texture is larger than the screen)
    IntRect FrameBufferRect(0, 0, myFrameBuffer.GetWidth(), myFrameBuffer.GetHeight());
    FloatRect TexCoords = myFrameBuffer.GetTexCoords(FrameBufferRect);

    // Render a fullscreen quad using the effect on our framebuffer
    FloatRect Screen = Window.GetView().GetRect();
    glBegin(GL_QUADS);
         glTexCoord2f(TexCoords.Left,  TexCoords.Top);    glVertex2f(Screen.Left,  Screen.Bottom);
         glTexCoord2f(TexCoords.Right, TexCoords.Top);    glVertex2f(Screen.Right, Screen.Bottom);
         glTexCoord2f(TexCoords.Right, TexCoords.Bottom); glVertex2f(Screen.Right, Screen.Top);
         glTexCoord2f(TexCoords.Left,  TexCoords.Bottom); glVertex2f(Screen.Left,  Screen.Top);
    glEnd();

    // Disable program
    GLCheck(glUseProgramObjectARB(0));

    // Disable texture units
    for (unsigned int i = 0; i < Unit; ++i)
    {
        GLCheck(glActiveTextureARB(GL_TEXTURE0_ARB + i));
        GLCheck(glBindTexture(GL_TEXTURE_2D, 0));
    }
    GLCheck(glActiveTextureARB(GL_TEXTURE0_ARB));
}


////////////////////////////////////////////////////////////
/// Preprocess a SFML effect file
/// to convert it to a valid GLSL fragment shader
////////////////////////////////////////////////////////////
std::string PostFX::PreprocessEffect(std::istream& File)
{
    // Initialize output string
    std::set<std::string> myTextures;
    std::string Out = "";

    // Variable declarations
    std::string Line;
    while (std::getline(File, Line) && (Line.substr(0, 6) != "effect"))
    {
        // Remove the ending '\r', if any
        if (!Line.empty() && (Line[Line.size() - 1] == '\r'))
            Line.erase(Line.size() - 1);

        // Skip empty lines
        if (Line == "")
            continue;

        // Extract variables type and name and convert them
        std::string Type, Name;
        std::istringstream iss(Line);
        if (!(iss >> Type >> Name))
        {
            std::cerr << "Post-effect error : invalid declaration (should be \"[type][name]\")" << std::endl
                      << "> " << Line << std::endl;
            return "";
        }

        if (Type == "texture")
        {
            // Textures need some checking and conversion
            if (myTextures.find(Name) != myTextures.end())
            {
                std::cerr << "Post-effect error : texture \"" << Name << "\" already exists" << std::endl;
                return "";
            }

            Out += "uniform sampler2D " + Name + ";\n";
            myTextures.insert(Name);
        }
        else
        {
            // Other types are just copied to output with "uniform" prefix
            Out += "uniform " + Type + " " + Name + ";\n";
        }
    }

    // Effect code
    Out += "void main()\n";
    while (std::getline(File, Line))
    {
        // Replace any texture lookup "T(" by "texture2D(T, "
        for (std::set<std::string>::const_iterator i = myTextures.begin(); i != myTextures.end(); ++i)
        {
            std::string::size_type Pos = Line.find(*i);
            if (Pos != std::string::npos)
                Line.replace(Pos, i->size() + 1, "texture2D(" + *i + ", ");
        }

        // Replace "_in" by "gl_TexCoord[0].xy"
        for (std::string::size_type Pos = Line.find("_in"); Pos != std::string::npos; Pos = Line.find("_in"))
            Line.replace(Pos, 3, "gl_TexCoord[0].xy");

        // Replace "_out" by "gl_FragColor"
        for (std::string::size_type Pos = Line.find("_out"); Pos != std::string::npos; Pos = Line.find("_out"))
            Line.replace(Pos, 4, "gl_FragColor");

        // Write modified line to output string
        Out += Line + "\n";
    }

    return Out;
}


////////////////////////////////////////////////////////////
/// Create the program and attach the shaders
////////////////////////////////////////////////////////////
void PostFX::CreateProgram()
{
    // Check that we can use post-FX !
    if (!CanUsePostFX())
    {
        std::cerr << "Failed to create a PostFX : your system doesn't support effects" << std::endl;
        return;
    }

    // Destroy effect program if it was already created
    if (myShaderProgram)
        GLCheck(glDeleteObjectARB(myShaderProgram));

    // Define vertex shader source (we provide it directly as it doesn't have to change)
    static const std::string VertexShaderSrc =
        "void main()"
        "{"
        "    gl_TexCoord[0] = gl_MultiTexCoord0;"
        "    gl_Position = ftransform();"
        "}";

    // Create the program
    myShaderProgram = glCreateProgramObjectARB();

    // Create the shaders
    GLhandleARB VertexShader   = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
    GLhandleARB FragmentShader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

    // Compile them
    const char* VertexSrc   = VertexShaderSrc.c_str();
    const char* FragmentSrc = myFragmentShader.c_str();
    GLCheck(glShaderSourceARB(VertexShader,   1, &VertexSrc,   NULL));
    GLCheck(glShaderSourceARB(FragmentShader, 1, &FragmentSrc, NULL));
    GLCheck(glCompileShaderARB(VertexShader));
    GLCheck(glCompileShaderARB(FragmentShader));

    // Check the compile logs
    GLint Success;
    GLCheck(glGetObjectParameterivARB(VertexShader, GL_OBJECT_COMPILE_STATUS_ARB, &Success));
    if (Success == GL_FALSE)
    {
        char CompileLog[1024];
        GLCheck(glGetInfoLogARB(VertexShader, sizeof(CompileLog), 0, CompileLog));
        std::cerr << "Failed to compile post-effect :" << std::endl
                  << CompileLog << std::endl;
        GLCheck(glDeleteObjectARB(VertexShader));
        GLCheck(glDeleteObjectARB(FragmentShader));
        GLCheck(glDeleteObjectARB(myShaderProgram));
        myShaderProgram = 0;
        return;
    }
    GLCheck(glGetObjectParameterivARB(FragmentShader, GL_OBJECT_COMPILE_STATUS_ARB, &Success));
    if (Success == GL_FALSE)
    {
        char CompileLog[1024];
        GLCheck(glGetInfoLogARB(FragmentShader, sizeof(CompileLog), 0, CompileLog));
        std::cerr << "Failed to compile post-effect :" << std::endl
                  << CompileLog << std::endl;
        GLCheck(glDeleteObjectARB(VertexShader));
        GLCheck(glDeleteObjectARB(FragmentShader));
        GLCheck(glDeleteObjectARB(myShaderProgram));
        myShaderProgram = 0;
        return;
    }

    // Attach the shaders to the program
    GLCheck(glAttachObjectARB(myShaderProgram, VertexShader));
    GLCheck(glAttachObjectARB(myShaderProgram, FragmentShader));

    // We can now delete the shaders
    GLCheck(glDeleteObjectARB(VertexShader));
    GLCheck(glDeleteObjectARB(FragmentShader));

    // Link the program
    GLCheck(glLinkProgramARB(myShaderProgram));

    // Get link log
    GLCheck(glGetObjectParameterivARB(myShaderProgram, GL_OBJECT_LINK_STATUS_ARB, &Success));
    if (Success == GL_FALSE)
    {
        // Oops... link errors
        char LinkLog[1024];
        GLCheck(glGetInfoLogARB(myShaderProgram, sizeof(LinkLog), 0, LinkLog));
        std::cerr << "Failed to link post-effect :" << std::endl
                  << LinkLog << std::endl;
        GLCheck(glDeleteObjectARB(myShaderProgram));
        myShaderProgram = 0;
        return;
    }
}


////////////////////////////////////////////////////////////
/// Destroy all video resources that need a valid rendering context
////////////////////////////////////////////////////////////
void PostFX::DestroyVideoResources()
{
    // Destroy effect program
    if (myShaderProgram)
    {
        GLCheck(glDeleteObjectARB(myShaderProgram));
        myShaderProgram = 0;
    }
}

} // namespace sf
