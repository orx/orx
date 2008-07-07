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
#include <SFML/Graphics/String.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Renderer.hpp>
#include <locale>


namespace sf
{
////////////////////////////////////////////////////////////
/// Construct the string from a multibyte text
////////////////////////////////////////////////////////////
String::String(const std::string& Text, const Font& CharFont, float Size) :
mySize          (Size),
myStyle         (Regular),
myNeedRectUpdate(true)
{
    SetFont(CharFont);
    SetText(Text);
}


////////////////////////////////////////////////////////////
/// Construct the string from a unicode text
////////////////////////////////////////////////////////////
String::String(const std::wstring& Text, const Font& CharFont, float Size) :
mySize          (Size),
myStyle         (Regular),
myNeedRectUpdate(true)
{
    SetFont(CharFont);
    SetText(Text);
}


////////////////////////////////////////////////////////////
/// Set the text (from a multibyte string)
////////////////////////////////////////////////////////////
void String::SetText(const std::string& Text)
{
    myNeedRectUpdate = true;

    if (!Text.empty())
    {
        std::vector<wchar_t> Buffer(Text.size());

        #ifdef __MINGW32__
                // MinGW has a bad support for wstring conversions (and wchar_t based standard classes in general)
                mbstowcs(&Buffer[0], Text.c_str(), Text.size());
        #else
                std::locale Locale("");
                std::use_facet<std::ctype<wchar_t> >(Locale).widen(Text.data(), Text.data() + Text.size(), &Buffer[0]);
        #endif

        myText.assign(&Buffer[0], Buffer.size());
    }
    else
    {
        myText = L"";
    }
}


////////////////////////////////////////////////////////////
/// Set the text (from a unicode string)
////////////////////////////////////////////////////////////
void String::SetText(const std::wstring& Text)
{
    myNeedRectUpdate = true;
    myText = Text;
}


////////////////////////////////////////////////////////////
/// Set the font of the string
////////////////////////////////////////////////////////////
void String::SetFont(const Font& CharFont)
{
    if (myFont != &CharFont)
    {
        myNeedRectUpdate = true;
        myFont = &CharFont;
    }
}


////////////////////////////////////////////////////////////
/// Set the size of the string
////////////////////////////////////////////////////////////
void String::SetSize(float Size)
{
    if (mySize != Size)
    {
        myNeedRectUpdate = true;
        mySize = Size;
    }
}


////////////////////////////////////////////////////////////
/// Set the style of the text
/// The default style is Regular
////////////////////////////////////////////////////////////
void String::SetStyle(unsigned long TextStyle)
{
    if (myStyle != TextStyle)
    {
        myNeedRectUpdate = true;
        myStyle = TextStyle;
    }
}


////////////////////////////////////////////////////////////
/// Get the text (returns a unicode string)
////////////////////////////////////////////////////////////
const std::wstring& String::GetUnicodeText() const
{
    return myText;
}


////////////////////////////////////////////////////////////
/// Get the text (returns a multibyte string)
////////////////////////////////////////////////////////////
std::string String::GetText() const
{
    if (!myText.empty())
    {
        std::vector<char> Buffer(myText.size());

        #ifdef __MINGW32__
                // MinGW has a bad support for wstring conversions (and wchar_t based standard classes in general)
                wcstombs(&Buffer[0], myText.c_str(), myText.size());
        #else
                std::locale Locale("");
                std::use_facet<std::ctype<wchar_t> >(Locale).narrow(myText.data(), myText.data() + myText.size(), '?', &Buffer[0]);
        #endif

        return std::string(&Buffer[0], Buffer.size());
    }
    else
    {
        return "";
    }
}


////////////////////////////////////////////////////////////
/// Get the font used by the string
////////////////////////////////////////////////////////////
const Font& String::GetFont() const
{
    return *myFont;
}


////////////////////////////////////////////////////////////
/// Get the size of the characters
////////////////////////////////////////////////////////////
float String::GetSize() const
{
    return mySize;
}


////////////////////////////////////////////////////////////
/// Get the style of the text
////////////////////////////////////////////////////////////
unsigned long String::GetStyle() const
{
    return myStyle;
}


////////////////////////////////////////////////////////////
/// Get the string rectangle on screen
////////////////////////////////////////////////////////////
FloatRect String::GetRect() const
{
    if (myNeedRectUpdate)
        const_cast<String*>(this)->RecomputeRect();

    FloatRect Rect;
    Rect.Left   = (myBaseRect.Left   - GetCenter().x) * GetScale().x + GetPosition().x;
    Rect.Top    = (myBaseRect.Top    - GetCenter().y) * GetScale().y + GetPosition().y;
    Rect.Right  = (myBaseRect.Right  - GetCenter().x) * GetScale().x + GetPosition().x;
    Rect.Bottom = (myBaseRect.Bottom - GetCenter().y) * GetScale().y + GetPosition().y;

    return Rect;
}


////////////////////////////////////////////////////////////
/// /see sfDrawable::Render
////////////////////////////////////////////////////////////
void String::Render(const RenderWindow&) const
{
    // No text, no rendering :)
    if (myText.empty())
        return;

    // Set the scaling factor to get the actual size
    float Factor = mySize / myFont->myCharSize;
    GLCheck(glScalef(Factor, Factor, 1.f));

    // Bind the font texture
    myFont->myTexture.Bind();

    // Initialize the rendering coordinates
    float X = 0.f;
    float Y = static_cast<float>(myFont->myCharSize);

    // Holds the lines to draw later, for underlined style
    std::vector<float> UnderlineCoords;
    UnderlineCoords.reserve(16);

    // Compute the shearing to apply if we're using the italic style
    float ItalicCoeff = (myStyle & Italic) ? 0.208f : 0.f; // 12 degrees

    // Draw one quad for each character
    glBegin(GL_QUADS);
    for (std::size_t i = 0; i < myText.size(); ++i)
    {
        // Get the current character
        wchar_t c = myText[i];

        // Check if the character is in the charset
        std::map<wchar_t, Font::Character>::const_iterator It = myFont->myCharacters.find(c);
        if (It == myFont->myCharacters.end())
        {
            // No : add a space and continue to the next character
            X += myFont->myCharSize;
            continue;
        }

        // Get the dimensions of the current character from the font description
        const Font::Character& CurChar = It->second;
        const IntRect&         Rect    = CurChar.Rect;
        const FloatRect&       Coord   = CurChar.Coord;

        // If we're using the underlined style and there's a new line,
        // we keep track of the previous line to draw it later
        if ((c == L'\n') && (myStyle & Underlined))
        {
            UnderlineCoords.push_back(X);
            UnderlineCoords.push_back(Y + 2);
        }

        // Handle special characters
        switch (c)
        {
            case L' ' :  X += CurChar.Advance;           continue;
            case L'\n' : Y += myFont->myCharSize; X = 0; continue;
            case L'\t' : X += CurChar.Advance    * 4;    continue;
            case L'\v' : Y += myFont->myCharSize * 4;    continue;
        }

        // Draw a textured quad for the current character
        glTexCoord2f(Coord.Left,  Coord.Top);    glVertex2f(X + Rect.Left  - ItalicCoeff * Rect.Top,    Y + Rect.Top);
        glTexCoord2f(Coord.Left,  Coord.Bottom); glVertex2f(X + Rect.Left  - ItalicCoeff * Rect.Bottom, Y + Rect.Bottom);
        glTexCoord2f(Coord.Right, Coord.Bottom); glVertex2f(X + Rect.Right - ItalicCoeff * Rect.Bottom, Y + Rect.Bottom);
        glTexCoord2f(Coord.Right, Coord.Top);    glVertex2f(X + Rect.Right - ItalicCoeff * Rect.Top,    Y + Rect.Top);

        // If we're using the bold style, we must render the character 4 more times,
        // slightly offseted, to simulate a higher weight
        if (myStyle & Bold)
        {
            static const float OffsetsX[] = {-0.5f, 0.5f, 0.f, 0.f};
            static const float OffsetsY[] = {0.f, 0.f, -0.5f, 0.5f};

            for (int j = 0; j < 4; ++j)
            {
                glTexCoord2f(Coord.Left,  Coord.Top);    glVertex2f(X + OffsetsX[j] + Rect.Left  - ItalicCoeff * Rect.Top,    Y + OffsetsY[j] + Rect.Top);
                glTexCoord2f(Coord.Left,  Coord.Bottom); glVertex2f(X + OffsetsX[j] + Rect.Left  - ItalicCoeff * Rect.Bottom, Y + OffsetsY[j] + Rect.Bottom);
                glTexCoord2f(Coord.Right, Coord.Bottom); glVertex2f(X + OffsetsX[j] + Rect.Right - ItalicCoeff * Rect.Bottom, Y + OffsetsY[j] + Rect.Bottom);
                glTexCoord2f(Coord.Right, Coord.Top);    glVertex2f(X + OffsetsX[j] + Rect.Right - ItalicCoeff * Rect.Top,    Y + OffsetsY[j] + Rect.Top);
            }
        }

        // Advance to the next character
        X += CurChar.Advance;
    }
    glEnd();

    // Draw the underlines if needed
    if (myStyle & Underlined)
    {
        // Compute the line thickness
        float Thickness = (myStyle & Bold) ? 3.f : 2.f;

        // Add the last line (which was not finished with a \n)
        UnderlineCoords.push_back(X);
        UnderlineCoords.push_back(Y + 2);

        // Draw the underlines as quads
        GLCheck(glDisable(GL_TEXTURE_2D));
        glBegin(GL_QUADS);
        for (std::size_t i = 0; i < UnderlineCoords.size(); i += 2)
        {
            glVertex2f(0,                  UnderlineCoords[i + 1]);
            glVertex2f(0,                  UnderlineCoords[i + 1] + Thickness);
            glVertex2f(UnderlineCoords[i], UnderlineCoords[i + 1] + Thickness);
            glVertex2f(UnderlineCoords[i], UnderlineCoords[i + 1]);
        }
        glEnd();
    }
}


////////////////////////////////////////////////////////////
/// Recompute the bounding rectangle of the text
////////////////////////////////////////////////////////////
void String::RecomputeRect()
{
    // Reset the "need update" state
    myNeedRectUpdate = false;

    // No text, empty box :)
    if (myText.empty())
    {
        myBaseRect = FloatRect(0, 0, 0, 0);
        return;
    }

    // Initial values
    float CurWidth  = 0;
    float CurHeight = 0;
    float Width     = 0;
    float Height    = 0;
    float Factor    = mySize / myFont->myCharSize;

    // Go through each character
    for (std::size_t i = 0; i < myText.size(); ++i)
    {
        // Get the current character
        wchar_t c = myText[i];

        // Check if the character is in the charset
        std::map<wchar_t, Font::Character>::const_iterator It = myFont->myCharacters.find(c);
        if (It == myFont->myCharacters.end())
        {
            // No : add a space and continue to the next character
            CurWidth += mySize;
            continue;
        }

        // Get the dimensions of the current character from the font description
        const Font::Character& CurChar = It->second;
        const IntRect& Rect = CurChar.Rect;
        float Advance = CurChar.Advance * Factor;

        // Handle special characters...
        switch (c)
        {
            case L' ' :  CurWidth += Advance;                   continue;
            case L'\t' : CurWidth += mySize * 4;                continue;
            case L'\v' : Height   += mySize * 4; CurHeight = 0; continue;

            case L'\n' :
                Height += mySize;
                CurHeight = 0;
                if (CurWidth > Width)
                    Width = CurWidth;
                CurWidth = 0;
                continue;
        }

        // Advance to the next character
        CurWidth += Advance;

        // Update the maximum height
        float CharHeight = (myFont->myCharSize + Rect.Bottom) * Factor;
        if (CharHeight > CurHeight)
            CurHeight = CharHeight;
    }

    // Update the last line
    if (CurWidth > Width)
        Width = CurWidth;
    Height += CurHeight;

    // Add a slight width / height if we're using the bold style
    if (myStyle & Bold)
    {
        Width  += 1 * Factor;
        Height += 1 * Factor;
    }

    // Add a slight width if we're using the italic style
    if (myStyle & Italic)
    {
        Width += 0.208f * mySize;
    }

    // Add a slight height if we're using the underlined style
    if (myStyle & Underlined)
    {
        if (CurHeight < mySize + 4 * Factor)
            Height += 4 * Factor;
    }

    // Finally update the rectangle
    myBaseRect.Left   = 0;
    myBaseRect.Top    = 0;
    myBaseRect.Right  = Width;
    myBaseRect.Bottom = Height;
}

} // namespace sf
