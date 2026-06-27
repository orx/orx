Orx - Portable Game Engine  (Version iOS 1.17)
================================================

Email iarwain [at] orx-project -dot- org to contact the author; or better, check
[orx's homepage](https://orx-project.org) for the most up-to-date contact information.
You can also come [chat with us](https://orx-project.org/discord)
if you have any questions or comments.

This engine is licensed under the very permissive zlib license, see the `LICENSE` file for details.

[Orx](https://orx-project.org) is a 2D-oriented, heavily data-driven game engine, focused primarily on ease of use.

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

    orx
      /code
        /build/ios/xcode                  - XCode project file for both orx and the iOS demo
        /demo/iOS                         - Source, config and data files for the iOS demo
        /include                          - Include files
        /lib/static/ios                   - Folder where the static orx library gets built
        /src                              - Source files for compiling orx
        /plugins                          - Source files for iOS plugins

        CreationTemplate.ini              - List of all config parameters used when creating objects, sounds, graphics, ...
        SettingsTemplate.ini              - List of all config parameters for physics, display, ... setup

      /doc/html                           - API documentation (doxygen)

      /extern                             - External dependencies needed when building its plugins


HowTo
-----

If you're not familiar with orx you can start by opening the XCode project in `orx/code/build/ios/xcode`.  
This project contains both orx library and a small iOS-oriented demo.

The demo uses iOS's accelerometer (using orx's joystick module) and iOS's touch system (using orx's mouse module).

Demo's code is found in orx/code/demo/iOS/orxTest.mm and is very short.  
Demo's config files can be found in `orx/code/demo/iOS/config`.

Fiddling with both demo's code and config is probably a good place to start understanding how orx works.  
[The wiki](https://orx-project.org/wiki) also contains useful information as well as some more generic tutorials.

When starting your own new project, you can either modify the demo project file (that also compiles orx) or use the `init` utility to create a new project.


Comments
--------

If you have any questions, comments or ideas, feel free to come [chat with us](https://orx-project.org/discord).


Enjoy!
