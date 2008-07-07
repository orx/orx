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
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/FontLoader.hpp>
#include <iostream>


namespace sf
{
////////////////////////////////////////////////////////////
/// Default constructor
////////////////////////////////////////////////////////////
Font::Font() :
myCharSize(0)
{

}


////////////////////////////////////////////////////////////
/// Load the font from a file
////////////////////////////////////////////////////////////
bool Font::LoadFromFile(const std::string& Filename, unsigned int CharSize, std::wstring Charset)
{
    return priv::FontLoader::GetInstance().LoadFontFromFile(Filename, CharSize, Charset, *this);
}


////////////////////////////////////////////////////////////
/// Load the font from a file in memory
////////////////////////////////////////////////////////////
bool Font::LoadFromMemory(const char* Data, std::size_t SizeInBytes, unsigned int CharSize, std::wstring Charset)
{
    // Check parameters
    if (!Data || (SizeInBytes == 0))
    {
        std::cerr << "Failed to load font from memory, no data provided" << std::endl;
        return false;
    }

    return priv::FontLoader::GetInstance().LoadFontFromMemory(Data, SizeInBytes, CharSize, Charset, *this);
}


////////////////////////////////////////////////////////////
/// Get the SFML default built-in font (Arial)
////////////////////////////////////////////////////////////
const Font& Font::GetDefaultFont()
{
    static Font       DefaultFont;
    static bool       DefaultFontLoaded = false;
    static const char DefaultFontData[] =
    {
        #include <SFML/Graphics/Arial.hpp>
    };

    // Load the default font on first call
    if (!DefaultFontLoaded)
    {
        DefaultFont.LoadFromMemory(DefaultFontData, sizeof(DefaultFontData), 30);
        DefaultFontLoaded = true;
    }

    return DefaultFont;
}

} // namespace sf
