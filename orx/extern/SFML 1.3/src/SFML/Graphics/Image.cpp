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
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/ImageLoader.hpp>
#include <SFML/Graphics/Renderer.hpp>
#include <algorithm>
#include <iostream>
#include <vector>


namespace sf
{
////////////////////////////////////////////////////////////
/// Default constructor
////////////////////////////////////////////////////////////
Image::Image() :
myWidth        (0),
myHeight       (0),
myTextureWidth (0),
myTextureHeight(0),
myTexture      (0),
myIsSmooth     (true),
myUpdated      (false)
{

}


////////////////////////////////////////////////////////////
/// Copy constructor
////////////////////////////////////////////////////////////
Image::Image(const Image& Copy) :
VideoResource  (Copy),
myWidth        (Copy.myWidth),
myHeight       (Copy.myHeight),
myTextureWidth (Copy.myTextureWidth),
myTextureHeight(Copy.myTextureHeight),
myPixels       (Copy.myPixels),
myTexture      (0),
myIsSmooth     (Copy.myIsSmooth),
myUpdated      (false)
{
    CreateTexture();
}


////////////////////////////////////////////////////////////
/// Construct an empty image
////////////////////////////////////////////////////////////
Image::Image(unsigned int Width, unsigned int Height, const Color& Col) :
myWidth        (0),
myHeight       (0),
myTextureWidth (0),
myTextureHeight(0),
myTexture      (0),
myIsSmooth     (true),
myUpdated      (false)
{
    Create(Width, Height, Col);
}


////////////////////////////////////////////////////////////
/// Construct the image from pixels in memory
////////////////////////////////////////////////////////////
Image::Image(unsigned int Width, unsigned int Height, const Uint8* Data) :
myWidth        (0),
myHeight       (0),
myTextureWidth (0),
myTextureHeight(0),
myTexture      (0),
myIsSmooth     (true),
myUpdated      (false)
{
    LoadFromPixels(Width, Height, Data);
}


////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
Image::~Image()
{
    // Destroy video resources
    DestroyVideoResources();
}


////////////////////////////////////////////////////////////
/// Load the image from a file
////////////////////////////////////////////////////////////
bool Image::LoadFromFile(const std::string& Filename)
{
    // Let the image loader load the image into our pixel array
    bool Success = priv::ImageLoader::GetInstance().LoadImageFromFile(Filename, myPixels, myWidth, myHeight);

    if (Success)
    {
        // Loading succeeded : we can create the texture
        if (CreateTexture())
            return true;
    }

    // Oops... something failed
    Reset();

    return false;
}


////////////////////////////////////////////////////////////
/// Load the image from a file in memory
////////////////////////////////////////////////////////////
bool Image::LoadFromMemory(const char* Data, std::size_t SizeInBytes)
{
    // Check parameters
    if (!Data || (SizeInBytes == 0))
    {
        std::cerr << "Failed to image font from memory, no data provided" << std::endl;
        return false;
    }

    // Let the image loader load the image into our pixel array
    bool Success = priv::ImageLoader::GetInstance().LoadImageFromMemory(Data, SizeInBytes, myPixels, myWidth, myHeight);

    if (Success)
    {
        // Loading succeeded : we can create the texture
        if (CreateTexture())
            return true;
    }

    // Oops... something failed
    Reset();

    return false;
}


////////////////////////////////////////////////////////////
/// Load the image directly from an array of pixels
////////////////////////////////////////////////////////////
bool Image::LoadFromPixels(unsigned int Width, unsigned int Height, const Uint8* Data)
{
    if (Data)
    {
        // Store the texture dimensions
        myWidth  = Width;
        myHeight = Height;

        // Fill the pixel buffer with the specified raw data
        const Color* Ptr = reinterpret_cast<const Color*>(Data);
        myPixels.assign(Ptr, Ptr + Width * Height);

        // We can create the texture
        if (CreateTexture())
        {
            return true;
        }
        else
        {
            // Oops... something failed
            Reset();
            return false;
        }
    }
    else
    {
        // No data provided : create a white image
        return Create(Width, Height, Color(255, 255, 255, 255));
    }
}


////////////////////////////////////////////////////////////
/// Save the content of the image to a file
////////////////////////////////////////////////////////////
bool Image::SaveToFile(const std::string& Filename) const
{
    // Let the image loader save our pixel array into the image
    return priv::ImageLoader::GetInstance().SaveImageToFile(Filename, myPixels, myWidth, myHeight);
}


////////////////////////////////////////////////////////////
/// Create an empty image
////////////////////////////////////////////////////////////
bool Image::Create(unsigned int Width, unsigned int Height, Color Col)
{
    // Store the texture dimensions
    myWidth  = Width;
    myHeight = Height;

    // Recreate the pixel buffer and fill it with the specified color
    myPixels.clear();
    myPixels.resize(Width * Height, Col);

    // We can create the texture
    if (CreateTexture())
    {
        return true;
    }
    else
    {
        // Oops... something failed
        Reset();
        return false;
    }
}


////////////////////////////////////////////////////////////
/// Create transparency mask from a specified colorkey
////////////////////////////////////////////////////////////
void Image::CreateMaskFromColor(Color ColorKey, Uint8 Alpha)
{
    // Calculate the new color (old color with no alpha)
    Color NewColor(ColorKey.r, ColorKey.g, ColorKey.b, Alpha);

    // Replace the old color by the new one
    std::replace(myPixels.begin(), myPixels.end(), ColorKey, NewColor);

    // The texture will need to be updated
    myUpdated = false;
}


////////////////////////////////////////////////////////////
/// Resize the image - warning : this function does not scale the image,
/// it just adjusts its size (add padding or remove pixels)
////////////////////////////////////////////////////////////
bool Image::Resize(unsigned int Width, unsigned int Height, Color Col)
{
    // Check size
    if ((Width == 0) || (Height == 0))
    {
        std::cerr << "Invalid new size for image (width = " << Width << ", height = " << Height << ")" << std::endl;
        return false;
    }

    // Create a new pixel array with the desired size
    std::vector<Color> Pixels(Width * Height, Col);

    // Copy the old pixel buffer into the new one
    for (unsigned int i = 0; i < std::min(Width, myWidth); ++i)
        for (unsigned int j = 0; j < std::min(Height, myHeight); ++j)
            Pixels[i + j * Width] = myPixels[i + j * myWidth];
    Pixels.swap(myPixels);

    // Store the new texture dimensions
    myWidth  = Width;
    myHeight = Height;

    // We can create the texture
    if (CreateTexture())
    {
        return true;
    }
    else
    {
        // Oops... something failed
        Reset();
        return false;
    }
}


////////////////////////////////////////////////////////////
/// Change the color of a pixel
/// Don't forget to call Update when you end modifying pixels
////////////////////////////////////////////////////////////
void Image::SetPixel(unsigned int X, unsigned int Y, const Color& Col)
{
    // Check if pixel is whithin the image bounds
    if ((X >= myWidth) || (Y >= myHeight))
    {
        std::cerr << "Cannot set pixel (" << X << "," << Y << ") for image "
                  << "(width = " << myWidth << ", height = " << myHeight << ")" << std::endl;
        return;
    }

    myPixels[X + Y * myWidth] = Col;

    // The texture will need to be updated
    myUpdated = false;
}


////////////////////////////////////////////////////////////
/// Get a pixel from the image
////////////////////////////////////////////////////////////
const Color& Image::GetPixel(unsigned int X, unsigned int Y) const
{
    // Check if pixel is whithin the image bounds
    if ((X >= myWidth) || (Y >= myHeight))
    {
        std::cerr << "Cannot get pixel (" << X << "," << Y << ") for image "
                  << "(width = " << myWidth << ", height = " << myHeight << ")" << std::endl;
        return Color::Black;
    }

    return myPixels[X + Y * myWidth];
}


////////////////////////////////////////////////////////////
/// Get a read-only pointer to the array of pixels (RGBA 8 bits integers components)
/// Array size is GetWidth() x GetHeight() x 4
/// This pointer becomes invalid if you reload or resize the image
////////////////////////////////////////////////////////////
const Uint8* Image::GetPixelsPtr() const
{
    if (!myPixels.empty())
    {
        return reinterpret_cast<const Uint8*>(&myPixels[0]);
    }
    else
    {
        std::cerr << "Trying to access the pixels of an empty image" << std::endl;
        return NULL;
    }
}


////////////////////////////////////////////////////////////
/// Bind the image for rendering
////////////////////////////////////////////////////////////
void Image::Bind() const
{
    // First check if the texture needs to be updated
    if (!myUpdated)
        const_cast<Image*>(this)->Update();

    // Bind it
    if (myTexture)
    {
        GLCheck(glEnable(GL_TEXTURE_2D));
        GLCheck(glBindTexture(GL_TEXTURE_2D, myTexture));
    }
}


////////////////////////////////////////////////////////////
/// Enable or disable image smoothing filter
////////////////////////////////////////////////////////////
void Image::SetSmooth(bool Smooth)
{
    myIsSmooth = Smooth;

    if (myTexture)
    {
        GLint PreviousTexture;
        GLCheck(glGetIntegerv(GL_TEXTURE_BINDING_2D, &PreviousTexture));

        GLCheck(glBindTexture(GL_TEXTURE_2D, myTexture));
        GLCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, myIsSmooth ? GL_LINEAR : GL_NEAREST));
        GLCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, myIsSmooth ? GL_LINEAR : GL_NEAREST));

        GLCheck(glBindTexture(GL_TEXTURE_2D, PreviousTexture));
    }
}


////////////////////////////////////////////////////////////
/// Return the width of the image
////////////////////////////////////////////////////////////
unsigned int Image::GetWidth() const
{
    return myWidth;
}


////////////////////////////////////////////////////////////
/// Return the height of the image
////////////////////////////////////////////////////////////
unsigned int Image::GetHeight() const
{
    return myHeight;
}


////////////////////////////////////////////////////////////
/// Tells whether the smooth filtering is enabled or not
////////////////////////////////////////////////////////////
bool Image::IsSmooth() const
{
    return myIsSmooth;
}


////////////////////////////////////////////////////////////
/// Convert a subrect expressed in pixels, into float
/// texture coordinates
////////////////////////////////////////////////////////////
FloatRect Image::GetTexCoords(const IntRect& Rect, bool Adjust) const
{
    float Width  = static_cast<float>(myTextureWidth);
    float Height = static_cast<float>(myTextureHeight);

    if (Adjust && myIsSmooth)
    {
        return FloatRect((Rect.Left   + 0.5f) / Width,
                         (Rect.Top    + 0.5f) / Height,
                         (Rect.Right  - 0.5f) / Width,
                         (Rect.Bottom - 0.5f) / Height);
    }
    else
    {
        return FloatRect(Rect.Left   / Width,
                         Rect.Top    / Height,
                         Rect.Right  / Width,
                         Rect.Bottom / Height);
    }
}


////////////////////////////////////////////////////////////
/// Get a valid texture size according to hardware support
////////////////////////////////////////////////////////////
unsigned int Image::GetValidTextureSize(unsigned int Size)
{
    if (glewIsSupported("GL_ARB_texture_non_power_of_two") != 0)
    {
        // If hardware supports NPOT textures, then just return the unmodified size
        return Size;
    }
    else
    {
        // If hardware doesn't support NPOT textures, we calculate the nearest power of two
        unsigned int PowerOfTwo = 1;
        while (PowerOfTwo < Size)
            PowerOfTwo *= 2;

        return PowerOfTwo;
    }
}


////////////////////////////////////////////////////////////
/// Assignment operator
////////////////////////////////////////////////////////////
Image& Image::operator =(const Image& Other)
{
    Image Temp(Other);

    std::swap(myWidth,         Temp.myWidth);
    std::swap(myHeight,        Temp.myHeight);
    std::swap(myTextureWidth,  Temp.myTextureWidth);
    std::swap(myTextureHeight, Temp.myTextureHeight);
    std::swap(myTexture,       Temp.myTexture);
    std::swap(myIsSmooth,      Temp.myIsSmooth);
    std::swap(myUpdated,       Temp.myUpdated);
    myPixels.swap(Temp.myPixels);

    return *this;
}


////////////////////////////////////////////////////////////
/// Create the OpenGL texture
////////////////////////////////////////////////////////////
bool Image::CreateTexture()
{
    // Check if texture parameters are valid before creating it
    if (!myWidth || !myHeight || myPixels.empty())
        return false;

    // Adjust internal texture dimensions depending on NPOT textures support
    unsigned int TextureWidth  = GetValidTextureSize(myWidth);
    unsigned int TextureHeight = GetValidTextureSize(myHeight);

    // Check the maximum texture size
    GLint MaxSize;
    GLCheck(glGetIntegerv(GL_MAX_TEXTURE_SIZE, &MaxSize));
    if ((TextureWidth > static_cast<unsigned int>(MaxSize)) || (TextureHeight > static_cast<unsigned int>(MaxSize)))
    {
        std::cerr << "Failed to create image, its internal size is too high (" << TextureWidth << "x" << TextureHeight << ")" << std::endl;
        return false;
    }

    // Destroy the previous OpenGL texture if it already exists with another size
    if ((TextureWidth != myTextureWidth) || (TextureHeight != myTextureHeight))
    {
        DestroyVideoResources();
        myTextureWidth  = TextureWidth;
        myTextureHeight = TextureHeight;
    }

    // We don't actually create the OpenGL texture, to allow loading sf::Image instances
    // without the need for a valid OpenGL context ()
    // Creation is done in Image::Update()
    myUpdated = false;

    return true;
}


////////////////////////////////////////////////////////////
/// Update the whole image in video memory
////////////////////////////////////////////////////////////
void Image::Update()
{
    GLint PreviousTexture;
    GLCheck(glGetIntegerv(GL_TEXTURE_BINDING_2D, &PreviousTexture));

    // Create the OpenGL texture if it doesn't exist
    // Texture creation is delayed on purpose (see Image::CreateTexture())
    if (!myTexture)
    {
        GLuint Texture = 0;
        GLCheck(glGenTextures(1, &Texture));
        GLCheck(glBindTexture(GL_TEXTURE_2D, Texture));
        GLCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, myTextureWidth, myTextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
        GLCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP));
        GLCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP));
        GLCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, myIsSmooth ? GL_LINEAR : GL_NEAREST));
        GLCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, myIsSmooth ? GL_LINEAR : GL_NEAREST));
        myTexture = static_cast<unsigned int>(Texture);
    }

    // Copy the pixels
    if (myTexture)
    {
        const Uint8* Pixels = reinterpret_cast<const Uint8*>(&myPixels[0]);
        GLCheck(glBindTexture(GL_TEXTURE_2D, myTexture));
        GLCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, myWidth, myHeight, GL_RGBA, GL_UNSIGNED_BYTE, Pixels));
    }

    GLCheck(glBindTexture(GL_TEXTURE_2D, PreviousTexture));

    myUpdated = true;
}


////////////////////////////////////////////////////////////
/// Reset the image attributes
////////////////////////////////////////////////////////////
void Image::Reset()
{
    DestroyVideoResources();

    myWidth         = 0;
    myHeight        = 0;
    myTextureWidth  = 0;
    myTextureHeight = 0;
    myTexture       = 0;
    myIsSmooth      = true;
    myUpdated       = false;
    myPixels.clear();
}


////////////////////////////////////////////////////////////
/// Destroy all video resources that need a valid rendering context
////////////////////////////////////////////////////////////
void Image::DestroyVideoResources()
{
    // Destroy the internal texture
    if (myTexture)
    {
        GLuint Texture = static_cast<GLuint>(myTexture);
        GLCheck(glDeleteTextures(1, &Texture));
        myTexture = 0;
        myUpdated = false;
    }
}

} // namespace sf
