////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007 Laurent Gomila (laurent.gom@gmail.com)
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

#ifndef SFML_CONFIG_HPP
#define SFML_CONFIG_HPP

////////////////////////////////////////////////////////////
// Identify the operating system
////////////////////////////////////////////////////////////
#if defined(_WIN32) || defined(__WIN32__)

    // Windows
    #define SFML_SYSTEM_WINDOWS
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif

#elif defined(linux) || defined(__linux)

    // Linux
    #define SFML_SYSTEM_LINUX

#elif defined(__APPLE__) || defined(MACOSX) || defined(macintosh) || defined(Macintosh)

    // MacOS
    #define SFML_SYSTEM_MACOS

#else

    // Unsupported system
    #error This operating system is not supported by SFML library

#endif


////////////////////////////////////////////////////////////
// Identify the platform
////////////////////////////////////////////////////////////
#if defined(__i386__) || defined(_M_IX86) || defined(_X86_) || defined(__INTEL__) || defined(__i386)

    // Intel x86
    #define SFML_PLATFORM_X86

#elif defined(__amd64__) || defined(__x86_64) || defined(__x86_64__) || defined(_M_AMD64)

    // AMD64
    #define SFML_PLATFORM_AMD64

#elif defined(__IA64__) || defined(_M_IA64)

    // Intel IA64
    #define SFML_PLATFORM_IA64

#elif defined(__powerpc__) || defined(_M_PPC) || defined(_ARCH_PPC)

    // Apple PowerPC
    #define SFML_PLATFORM_POWERPC

#else

    // Unsupported platform
    #error This platform is not supported by SFML library

#endif


////////////////////////////////////////////////////////////
// Define a portable debug macro
////////////////////////////////////////////////////////////
#if !defined(NDEBUG)

    #define SFML_DEBUG

#endif


////////////////////////////////////////////////////////////
// Define portable import / export macros
////////////////////////////////////////////////////////////
#if defined(SFML_SYSTEM_WINDOWS)

    #ifdef SFML_DYNAMIC

        // Windows platforms
        #ifdef SFML_EXPORTS

            // From DLL side, we must export
            #define SFML_API __declspec(dllexport)

        #else

            // From client application side, we must import
            #define SFML_API __declspec(dllimport)

        #endif

        // For Visual C++ compilers, we also need to turn off this annoying C4251 warning.
        // You can read lots ot different things about it, but the point is the code will
        // just work fine, and so the simplest way to get rid of this warning is to disable it
        #ifdef _MSC_VER

            #pragma warning(disable : 4251)

        #endif

    #else

        // No specific directive needed for static build
        #define SFML_API

    #endif

#else

    // Other platforms don't need to define anything
    #define SFML_API

#endif


////////////////////////////////////////////////////////////
// Define endianness depending on current platform
////////////////////////////////////////////////////////////
#ifdef SFML_PLATFORM_POWERPC

    // Apple PowerPC processors are big endian
    #define SFML_BIG_ENDIAN

#else

    // The other supported processors (x86, IA64, AMD64) are little endian
    #define SFML_LITTLE_ENDIAN

#endif


////////////////////////////////////////////////////////////
// Define portable types
////////////////////////////////////////////////////////////
#include <climits>

namespace sf
{
    // 8 bits integer types
    #if UCHAR_MAX == 0xFF
        typedef char          Int8;
        typedef unsigned char Uint8;
    #else
        #error No 8 bits integer type for this platform
    #endif

    // 16 bits integer types
    #if USHRT_MAX == 0xFFFF
        typedef short          Int16;
        typedef unsigned short Uint16;
    #elif UINT_MAX == 0xFFFF
        typedef int          Int16;
        typedef unsigned int Uint16;
    #elif ULONG_MAX == 0xFFFF
        typedef long          Int16;
        typedef unsigned long Uint16;
    #else
        #error No 16 bits integer type for this platform
    #endif

    // 32 bits integer types
    #if USHRT_MAX == 0xFFFFFFFF
        typedef short          Int32;
        typedef unsigned short Uint32;
    #elif UINT_MAX == 0xFFFFFFFF
        typedef int          Int32;
        typedef unsigned int Uint32;
    #elif ULONG_MAX == 0xFFFFFFFF
        typedef long          Int32;
        typedef unsigned long Uint32;
    #else
        #error No 32 bits integer type for this platform
    #endif

} // namespace sf


#endif // SFML_CONFIG_HPP
