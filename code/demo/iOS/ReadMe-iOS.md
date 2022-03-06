Orx - Portable Game Engine  (Version iOS 1.13)
================================================

Email iarwain [at] orx-project -dot- org to contact the author; or better, check
[orx's homepage](https://orx-project.org) for the most up-to-date contact information.
You can also come [chat with us](https://orx-project.org/discord)
if you have any questions or comments.

This engine is licensed under the zlib license, see the `LICENSE` file for details.

[Orx](https://orx-project.org) is a 2D-oriented, data-driven and portable game engine focused primarily on ease of use.

Please read the `README.md` file for general info about orx.


Intro
-----

This release contains the full source code of orx as well as an iOS-oriented demo featuring:

- accelerometer
- touch interaction
- physics
- particles
- music & sounds
- dynamic object creation
- visual FXs

For more generic examples on how to use orx, please get the tutorial package and/or have a look at [the wiki](https://orx-project.org/wiki).


Files
-----

    /dev-ios
      /build/ios/xcode                  - XCode project file for both orx and the iOS demo
      /demo                             - Source, config and data files for the iOS demo
      /include                          - Include files
      /lib/static/ios                   - Folder where the static orx library is built
      /src                              - Source files for compiling orx
      /plugins                          - Source files for iOS plugins

      CreationTemplate.ini              - List of all config parameters used when creating objects, sounds, graphics, ...
      SettingsTemplate.ini              - List of all config parameters for physics, display, ... setup

      liborx-fat-sdk9.2.zip             - Fat static library of orx,
                                          including both simulator & device (iPhone/iPod Touch/iPad) versions compiled with iOS SDK 9.2

    /doc/html                           - API documentation (doxygen)

    /extern                             - External dependencies needed when building its plugins


HowTo
-----

If you're not familiar with orx you can start by opening the XCode project in `/dev-ios/build/ios/xcode`.  
This project contains both orx library and a small iOS-oriented demo.

The demo uses iOS's accelerometer (using orx's joystick module) and iOS's touch system (using orx's mouse module).

Demo's code is found in /dev-ios/demo/iOS/orxTest.mm and is very short.  
Demo's config files can be found in `/dev-ios/demo/iOS/config`.

Fiddling with both demo's code and config is probably a good place to start understanding how orx works.  
[The wiki](https://orx-project.org/wiki) also contains useful information as well as some more generic tutorials.

When starting your own new project, you can either modify the demo project file (that also compiles orx) or use
the precompiled libraries found in `liborx-fat-sdk9.2.zip` along with the include files found in `/dev-ios/include`.

The precompiled libraries contain both device & simulator code for more convenience.


Comments
--------

If you have any questions, comments, ideas or reviews, feel free to either
post them on [orx's forum](https://forum.orx-project.org),
come [chat with us](https://orx-project.org/discord)
or send them directly by email to iarwain [at] orx-project.org


Enjoy!
