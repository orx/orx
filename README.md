<p align="center">
    <a href="https://orx-project.org"><img src="./logo.png" alt="Orx Engine" width="400" height="auto"></a>
    <br />
    <strong>A (HEAVILY) DATA-DRIVEN 2D GAME ENGINE</strong>
</p>

<p align="center">
    <a href="LICENSE"><img src="https://img.shields.io/github/license/orx/orx" alt="license"></a>
    <a href="https://github.com/orx/orx/graphs/commit-activity"><img src="https://img.shields.io/github/commit-activity/t/orx/orx/master" alt="total-commit-activity"></a>
    <a href="https://github.com/orx/orx"><img src="https://img.shields.io/github/stars/orx/orx" alt="stars"></a>
    <a href="https://github.com/orx/orx/forks"><img src="https://img.shields.io/github/forks/orx/orx" alt="forks"></a>
    <a href="https://orx-project.org/discord"><img src="https://discordapp.com/api/guilds/522167736823185418/widget.png?style=shield" alt="discord"></a>
</p>

<p align="center">
    <a href="https://orx-project.org"><img src="https://img.shields.io/website?url=https%3A%2F%2Forx-project.org" alt="site"></a>
    <a href="https://wiki.orx-project.org"><img src="https://img.shields.io/badge/platforms-windows%20%7C%20linux%20%7C%20mac%20%7C%20web/html5%20%7C%20android%20%7C%20ios-red" alt="platforms"></a>
    <a href="https://github.com/orx/orx/releases"><img src="https://img.shields.io/github/v/release/orx/orx?color=orange" alt="releases"></a>
</p>

<p align="center">
    <a href="https://github.com/orx/orx/actions/workflows/main.yml"><img src="https://img.shields.io/github/actions/workflow/status/orx/orx/main.yml" alt="build-travis"></a>
    <a href="https://ci.appveyor.com/project/iarwain/orx"><img src="https://ci.appveyor.com/api/projects/status/alwm4il7x1w6g1o4?svg=true" alt="build-appveyor"></a>
    <a href="https://github.com/orx/orx/commits/master"><img src="https://img.shields.io/github/last-commit/orx/orx/master" alt="last-commit"></a>
    <a href="https://github.com/orx/orx/graphs/commit-activity"><img src="https://img.shields.io/github/commit-activity/m/orx/orx/master" alt="monthly-commit-activity"></a>
</p>

<p align="center">
    <a href="https://github.com/orx/orx"><img src="https://img.shields.io/github/repo-size/orx/orx.svg" alt="repo-size"></a>
    <a href="https://github.com/orx/orx/issues"><img src="https://img.shields.io/github/issues/orx/orx" alt="issues"></a>
    <a href="https://github.com/orx/orx/pulls"><img src="https://img.shields.io/github/issues-pr/orx/orx" alt="pull-requests"></a>
</p>


Orx (Version 1.16)
========================================

This engine is licensed under the very permissive [zlib license](https://en.wikipedia.org/wiki/Zlib_License) (i.e. it's free for any commercial or non-commercial use),
see the [LICENSE](https://github.com/orx/orx/blob/master/LICENSE) file for details.

Email iarwain [at] orx-project.org to contact the author or come [chat with us on Discord](https://orx-project.org/discord) if you have any questions, feedback or comments.


Summary
-------

[Orx](https://orx-project.org) is a heavily data-driven, 2D-oriented, portable game engine, focused primarily on ease of use and powerful features.
(See below for a list of supported platforms.)

This is a release candidate for orx v1.16. If you find any bugs, please report them using orx's [issue page/tracker](https://github.com/orx/orx/issues).

Some notable features of the engine are:

- powerful config system that makes orx data-driven and provides an easy to use load/save system
- hardware-accelerated rendering that provides: translation, anisotropic scale, rotation,
  transparency (alpha blending), different blending modes, coloring, tiling and mirroring
- advanced rendering features such as MRT (Multiple Render Targets), offscreen rendering, flexible pipeline & easy compositing
- advanced resource management allowing for easy multi-platform data support and patching
- C++ extension with semi-automated C++/config binding
- bundle support (automatic resource encryption & packaging)
- automatic hotloading of resources upon modification on disk drastically shortens iteration times
- timelines and commands modules allow for config-driven scripting-like features
- interactive console can execute commands at runtime (very useful for tweaking/debugging purposes)
- advanced animation engine (including a chaining graph & custom animation events for synchronization)
- fragment (pixel) shader support
- visual FXs based on curve combinations
- integrated runtime profiler (with graphical display) to easily spot which parts of your game need to be optimized
- collision handling and rigid body physics
- camera/viewport scheme allowing multiple views displayed with camera translation, zoom and rotation
- generic input system that abstracts keyboard, mouse, joystick, touch and accelerometer inputs
- powerful localization module (for localized texts, audio, graphics, etc.)
- spawners (provides an easy way to create particles or projectiles)
- 3D positioning using "scene nodes"
- custom bitmap, truetype & SDF font support
- automatic differential scrolling and depth scaling upon request
- fully featured and extensible sound system (WAV/MP3/OGG/QOA support, streams, hierarchical buses, filters, spatialization, etc.)
- clock system that provides time consistency and allows time stretching + high precision timers
- event manager
- unicode support with UTF-8 encoding
- plugin system
- screenshot capture tool (supports BMP, PNG, JPG, QOI, TGA, PNM, GIF, PSD and DDS)


Getting Started
---------------

Check [orx's wiki](https://orx-project.org/wiki) for detailed information, including:
- initial setup
- beginner's guide
- tools
- samples
- detailed tutorials (both official and community-made ones)

Check the [doc/html](./doc/html/index.html) directory for orx's API doxygen documentation.


Supported Platforms
-------------------

The engine compiles for, and is tested on:

- Linux (x86/x86-64 with codelite, codeblocks and GNU makefile)
- Windows (x86/x86-64 with vs2017, vs2019 & vs2022, x86/x86-64 mingw-w64 (14.2.0) with codelite, codeblocks and GNU makefile)
- MacOS X (x86/x86-64 with xcode or x86/x86-64 with codelite, codeblocks and GNU makefile), version 10.6+
- Web/HTML5 (Emscripten)
- Android (Gradle build files, simulator & device)
- iOS (iPhone/iPod Touch/iPad, simulator & device with xcode)


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

- GLFW-based (+stb_image/webp/qoi/BasisU) display, joystick, keyboard and mouse plugins for all non-iOS/non-Android platforms
- miniaudio (+stb_vorbis) sound plugin for all platforms
- LiquidFun-based physics plugin
- Homemade rendering plugin (2D)
- OpenGL ES display plugins for iOS and Android
- Touch/MultiTouch-based mouse plugin for iOS and Android
- Accelerometer-based joystick plugin for iOS and Android

If the headless variants of the embedded versions are requested, dummy plugins will be used for the display, joystick, mouse, keyboard and soundsystem plugins.
The render & physics plugins will be the same as the ones used for the non-headless versions.

Some tools are also available:

- `orxCrypt`  : command line tool to encrypt/decrypt/merge multiple config files
- `orxFontGen`: command line tool to generate custom bitmap & SDF fonts offline, from TrueType/OpenType fonts


Compiling
---------

### IMPORTANT - FIRST STEP:

**If you just cloned orx's repository, you will need to run the `setup.bat` / `setup.sh` script located at its root before being able to compile the engine.
This script will download all the required dependencies and generate the project files for your platform.
Finally, the script will hook itself to mercurial/git so you won't have to run it manually ever again.**

The easiest way to learn how to build your project using orx for a given platform
is to check [orx's wiki](https://wiki.orx-project.org).

The generated project files will contain 6 different configurations to build orx: `Debug`, `Profile` & `Release` in both regular and `Core` flavors. Here's what they do:

- `Debug`: These configurations will build orx in debug mode. They are the only configurations that will output warning and error messages resulting from incorrect data, code and behaviors. These configurations are also much slower than the other ones but is recommended during development to catch any mistake. Note: the internal profiler is also available with these configurations.

- `Profile`: These configurations will build orx in optimized mode but will keep the internal profiler, slightly slowing down the execution but allowing the user to get high level profiling information which can be very useful during development.

- `Release`: These configurations will build orx in optimized mode, *without* the internal profiler. These are the configurations one should be using when shipping their project. Note: on Windows, the OS terminal will also be hidden with this configuration.

- `Core`: Those configurations will only build orx's core as a static library. With these configurations, no plugins will be compiled/embedded. In non-`Core` configurations, all the plugins are built and embedded inside orx's library, and should be the configurations used by most users. The `Core` configurations are typically used to build orx's tools that do not require any plugin (`orxCrypt` & `orxFontGen`).


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

- `__orxHEADLESS__` : used to embed plugins that are compatible with a headless mode.  
                    NB: this flag is *ONLY* used when compiling orx library
                    and when `__orxEMBEDDED__` is also defined!


There are other preprocessor flags used when compiling the orx library,
but those should be easy enough to decipher.  markdo
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
  `__orxMSVC__`           | orx is being compiled with visual studio C/C++
  `__orxLLVM__`           | orx is being compiled with llvm/clang
  `__orxWINDOWS__`        | orx is being compiled for Windows
  `__orxMAC__`            | orx is being compiled for MacOS X
  `__orxLINUX__`          | orx is being compiled for Linux
  `__orxWEB__`            | orx is being compiled for Web/HTML5
  `__orxIOS__`            | orx is being compiled for iOS
  `__orxANDROID__`        | orx is being compiled for Android
  `__orxCPP__`            | orx is being compiled with a C++ compiler
  `__orxOBJC__`           | orx is being compiled with an Objective-C compiler
  `__orxPLUGIN__`         | a plugin for orx is being compiled
  `__orxEXTERN__`         | code using orx's library is being compiled


Comments
--------

If you have any questions, comments, ideas or feedback, feel free to come [chat with us on Discord](https://orx-project.org/discord).

Enjoy!
