Orx - Portable Game Engine (Version 1.6)
========================================


Email iarwain [at] orx-project.org to contact the author; or, better,
check [orx's homepage](http://orx-project.org) for the most up-to-date contact information.

This engine is licensed under the very permissive [zlib license](http://en.wikipedia.org/wiki/Zlib_License),
see the [LICENSE](https://bitbucket.org/orx/orx/src/default/LICENSE) file for details.


Summary
-------

Orx is a 2D-oriented, data-driven, portable game engine focused primarily on ease of use and powerful features.
See below for a list of supported platforms.

This is the stable release for orx v1.6. If you find any bugs,
please report them on the [forum](http://forum.orx-project.org), in the "Bug report - Feature request" board,
or via orx's [issue page/tracker](https://bitbucket.org/orx/orx/issues?status=new&status=open).

The current features of orx engine are:

- hardware-accelerated rendering that provides: translation, anisotropic scale, rotation,
  transparency (alpha blending), different blending modes, coloring, tiling and mirroring
- advanced rendering features such as MRT (Multiple Render Targets) support and easy compositing
- powerful config system that makes orx data-driven and provides an easy to use load/save system
- powerful resource management allowing for easy multi-platform data support and patching
- automatic hotloading of resources upon modification on disk shortens drastically iteration times
- timelines and commands modules allow for config-driven scripting-like features
- interactive console can execute commands at runtime (very useful for tweaking/debug purposes)
- animation engine (including a chaining graph & custom animation events for synchronization)
- fragment (pixel) shader support
- visual FXs based on curve combinations
- integrated runtime profiler (with graphical display) to easily spot which parts of your game need to be optimized
- collision handling and rigid body physics
- camera/viewport scheme allowing multiple views displayed with camera translation, zoom and rotation
- generic input system that abstracts keyboard, mouse, joystick, touch and accelerometer inputs
- powerful localization module
- spawners (provides an easy way to create particles or projectiles)
- 3D positioning using "scene nodes"
- custom bitmap font support
- automatic differential scrolling and depth scaling upon request
- music and spatialized sound support
- clock system that provides time consistency and allows time stretching + high precision timers
- event manager
- unicode support with UTF-8 encoding
- plugin system
- screenshot capture tool (supports bmp, png, jpg, tga and dds)

See the [doc/html](https://bitbucket.org/orx/orx/src/default/doc/html) directory for the doxygen documentation of orx's API.
The documentation for the latest release version is [available here](http://orx-project.org/orx/doc/html) at orx's homepage.

You can find details about the tutorials (as well as community-made ones), the tools and the data-driven side of orx
(ie. how to use the config system) on [orx's wiki](http://wiki.orx-project.org).  
The English version is the most complete one.  
Chinese and Spanish translations are entirely written and maintained by community members,
and they might be less accurate/somewhat outdated.


Supported Platforms
-------------------

The engine compiles and is tested for:

- Linux (x86/x86-64 with codelite, codeblocks and GNU makefile)
- Windows (x86 with vs2012, vs2013 (x86/x86-64) and mingw32 with codelite, codeblocks and GNU makefile)
- MacOS X (x86/x86-64/ppc/ppc64 with xcode or x86/x86-64 with codelite, codeblocks and GNU makefile),  
  version 10.5+ for GLFW plugins (default version) and 10.4+ for SFML ones (no joystick support)
- iOS (iPhone/iPod Touch/iPad, simulator & device with xcode)
- Android (NDK build files, simulator & device)


Versions
--------

Those are not revision versions but link/run versions.
Orx library can be compiled as a static or a dynamic library.
In addition to these modes, orx can be compiled as an embedded version or not.

Orx's core is basically platform-independent. All the platform/OS-dependent features
are implemented via plugins. These plugins can be loaded at runtime (hotplug) or
they can be embedded at linktime.

If you use the non-embedded versions, you'll have to specify which plugins to use.
This is more flexible but also requires additional files (the plugins themselves).
The embedded version will store everything in orx's library, meaning you won't be able
to choose which plugin to use at runtime, but will also be more compact. This will
also make orx run considerably faster.

From the download page you'll find precompiled binaries for Windows (x86), Linux (x86/x86-64), MacOS X (ppc/x86/x86-64),
iOS and Android, using the dynamic embedded versions only.
If you want to use the non-embedded versions (to use with your own plugins), you'll need to compile orx yourself from the source.
Everything compiles out-of-the-box for the hardware platforms cited above.

The embedded versions currently use:

- GLFW-based (+SOIL) plugins for display, joystick, keyboard and mouse for all non-iOS/non-Android platforms
- OpenAL-based (+libsndfile/stb_vorbis/tremor) plugins for sound for all platforms
- Box2D-based plugin for physics
- homemade plugin for 2D rendering
- OpenGL ES plugins for display on iOS and Android
- Touch/MultiTouch-based plugin for mouse on iOS and Android
- Accelerometer-based plugin for joystick on iOS and Android

All the 11 basic and advanced official tutorials are shipped with the dev packages, including precompiled release binaries.

Some tools (precompiled binaries only) are also shipped with the dev packages:

- orxCrypt  : command line tool to encrypt/decrypt/merge multiple config files
- orxFontGen: command line tool (based on FreeType2) to generate custom bitmap fonts (.tga texture & .ini config file) from TrueType fonts


Packages
--------

You can download all the packages from [sourceforge](http://orx.sf.net) or [bitbucket](https://bitbucket.org/orx/orx/downloads).
Here is a list with a small description for each package.

- orx-doc-1.6.zip: orx's API doxygen documentation
- orx-src-1.6.zip: orx's source code including build projects for
    - codelite (Windows and Linux, all versions, Mac OS X, non-static versions)
    - codeblocks (Windows and Linux, all versions, Mac OS X, non-static versions)
    - GNU makefiles (Windows and Linux, all versions, Mac OS X, non-static versions)
    - vs2012 & vs2013 (Windows Visual Studio, all versions)
    - xcode (3.2+, for MacOS X, non-static versions; SDK 8.1 for iOS, static embedded versions)
    - ndk build files for Android (static embedded versions)
NB: You'll need orx-extern-1.6.zip (orx plugins' external dependencies) if you intend to compile orx yourself.
- orx-extern-1.6.zip: orx's external dependencies. You will *only NEED* these if you
  use orx-src-1.6.zip and you plan on compiling orx yourself. They are not needed otherwise.
  The external libraries are usually modified versions of the original ones.  
  __VERY IMPORTANT__: If you want to compile orx yourself, you'll need these versions and not the official ones.
- orx-dev-linux32-1.6.tar.bz2       : dynamic embedded binaries for Linux (x86), release/profile/debug + tools.
- orx-dev-linux64-1.6.tar.bz2       : dynamic embedded binaries for Linux (x86-64), release/profile/debug + tools.
- orx-dev-mac-1.6.zip               : dynamic embedded binaries for MacOS X (x86/x86-64), release/profile/debug + tools.
- orx-dev-mingw-1.6.zip             : dynamic embedded binaries for Windows (mingw), release/profile/debug + tools.
- orx-dev-vs2012-1.6.zip            : dynamic embedded binaries for Windows (Visual Studio 2012), release/profile/debug + tools.
- orx-dev-vs2013-32-1.6.zip         : dynamic embedded binaries for Windows (Visual Studio 2013, x86), release/profile/debug + tools.
- orx-dev-vs2013-64-1.6.zip         : dynamic embedded binaries for Windows (Visual Studio 2013, x86-64), release/profile/debug + tools.
- orx-full-ios-1.6.zip              : static embedded binaries for iOS, release/profile/debug, simulator/device + doc + source + XCode project file.
- orx-dev-android-1.6.zip           : static embedded binaries for Android, release/profile/debug (device).
- orx-dev-android-native-1.6.zip    : static embedded binaries for Android Native, release/profile/debug (device).
- orx-tutorial-linux32-1.6.tar.bz2  : tutorials for Linux (x86)
- orx-tutorial-linux64-1.6.tar.bz2  : tutorials for Linux (x86-64)
- orx-tutorial-mac-1.6.zip          : tutorials for MacOS X (x86/x86-64)
- orx-tutorial-mingw-1.6.zip        : tutorials for Windows (mingw)
- orx-tutorial-vs2012-1.6.zip       : tutorials for Windows (Visual Studio 2012)
- orx-tutorial-vs2013-1.6.zip       : tutorials for Windows (Visual Studio 2013)

All the `*-dev-*` packages above include:

- orx release/profile/debug libraries used for linking
- runtime release/profile/debug orx libraries
- headers to include at compile time
- template files exposing wich properties can be accessed using the config system (for user reference only, not needed by orx)

All the `*-tutorial-*` packages above include:

- heavily commented source code for 11 basic and advanced tutorials
- precompiled binaries (orx link library, orx runtime library)
- headers to include at compile time
- template files exposing wich properties can be accessed using the config system (for user reference only, not needed by orx)
- build/project files


Compiling
---------

###Important - first step:

**If you just cloned orx with mercurial or git, you will need to run the setup.bat/setup.sh script
that's located at its root before being able to compile the engine.
This script will download all the needed dependencies and generate all the project files for your platform.
The script will then hook itself to mercurial/git and you shouldn't have to run it manually after subsequent pulls.**

The easiest way to learn how to compile your project using orx for a given platform
is to look at the tutorial build project files.

NB: The debug version is far slower than the release one, but will output all the warning
and error messages useful during development.

Here's a quick list of the available compile preprocessor flags:

- `__orxDEBUG__`    : used to compile and link against the debug versions of orx library
                    (liborxd.a / orxd.lib / liborxd.dylib), if not specified it refers to
                    the release versions (liborx.a / orx.lib / liborx.dylib).  
                    NB: If you want to link against the debug versions of orx library, you need
                    to specify it to your compiler!

- `__orxPROFILER__` : used to enable the profiling push/pop macros for release builds
                    It's automatically enabled for debug builds and it has been enabled to
                    build liborxp.a / orxp.lib / orxp.dll / liborxp.dylib / liborxp.so  
                    NB: You can define it in your projects even when using regular release builds,
                    but you won't be able to see orx's internal profiling markers!

- `__orxSTATIC__`   : used to compile and link against the static versions of orx library.  
                    NB: If you want to link against the static versions of orx library, you need
                    to specify it to your compiler!

- `__orxEMBEDDED__` : used to compile the embedded versions of orx library.  
                    NB: this flag is *ONLY* needed when compiling orx library, not when linking
                    against it.


There are other preprocessor flags used when compiling the orx library,
but those should be easy enough to decipher.  
However, you might want to specify them manually for cross-compiling or
use them so that your code will behave differently depending on the architecture
for which you're compiling. Here's a quick list of these flags:

  Flag                    | Description
:------------------------:|--------------------------------------------------------
  `__orxARM__`            | orx is being compiled for an ARM architecture
  `__orxPPC__`            | orx is being compiled for a PowerPC architecture
  `__orxPPC64__`          | orx is being compiled for a PowerPC 64 architecture
  `__orxX86__`            | orx is being compiled for a x86 architecture
  `__orxX86_64__`         | orx is being compiled for a x86-64 architecture
  `__orxLITTLE_ENDIAN__`  | orx is being compiled for a little endian architecture
  `__orxBIG_ENDIAN__`     | orx is being compiled for a big endian architecture
  `__orxGCC__`            | orx is being compiled with gcc
  `__orxMSVC__`           | orx is being compiled with visual studio C/C
  `__orxLLVM__`           | orx is being compiled with llvm/clang
  `__orxWINDOWS__`        | orx is being compiled for Windows (x86
  `__orxMAC__`            | orx is being compiled for MacOS X (ppc/x86/x86-64
  `__orxLINUX__`          | orx is being compiled for Linux (x86/x86-64
  `__orxIOS__`            | orx is being compiled for iOS
  `__orxANDROID__`        | orx is being compiled for Android
  `__orxANDROID_NATIVE__` | orx is being compiled for Android Native
  `__orxRASPBERRY_PI__`   | orx is being compiled for Raspberry Pi
  `__orxCPP__`            | orx is being compiled with a C++ compiler
  `__orxOBJC__`           | orx is being compiled with an Objective-C compiler
  `__orxFREEBASIC__`      | orx is being compiled for FreeBasic, WIP
  `__orxPLUGIN__`         | a plugin for orx is being compiled
  `__orxEXTERN__`         | code using orx's library is being compiled


Comments
--------

If you have any questions, comments, ideas or reviews, feel free to post them
on [orx's forum](http://forum.orx-project.org) or send them directly by email to iarwain [at] orx-project.org

Enjoy!
