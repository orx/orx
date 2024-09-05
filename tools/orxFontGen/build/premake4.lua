-- This premake script should be used with orx-customized version of premake4.
-- Its Hg repository can be found at https://bitbucket.org/orx/premake-stable.
-- A copy, including binaries, can also be found in the extern/premake folder.

--
-- Globals
--

function initconfigurations ()
    return
    {
        "Debug",
        "Profile",
        "Release"
    }
end

function initplatforms ()
    if os.is ("windows")
    or os.is ("linux") then
        if os.is64bit () then
            return
            {
                "x64",
                "x32"
            }
        else
            return
            {
                "x32",
                "x64"
            }
        end
    elseif os.is ("macosx") then
        return
        {
            "universal64",
            "x64"
        }
    end
end

function defaultaction (name, action)
   if os.is (name) then
      _ACTION = _ACTION or action
   end
end

defaultaction ("windows", "vs2022")
defaultaction ("linux", "gmake")
defaultaction ("macosx", "gmake")

newoption
{
    trigger = "to",
    value   = "path",
    description = "Set the output location for the generated files"
}

if os.is ("macosx") then
    osname = "mac"
else
    osname = os.get()
end

destination = _OPTIONS["to"] or "./" .. osname .. "/" .. _ACTION
copybase = path.rebase ("..", os.getcwd (), os.getcwd () .. "/" .. destination)


--
-- Solution: orx
--

solution "orxFontGen"

    language ("C")

    location (destination)

    kind ("ConsoleApp")

    configurations
    {
        initconfigurations ()
    }

    platforms
    {
        initplatforms ()
    }

    includedirs
    {
        "../include",
        "../../../code/include",
        "$(ORX)/include",
        "../../../extern/stb_image",
        "$(ORX)/../extern/stb_image",
        "../../../extern/freetype/include",
        "$(ORX)/../extern/freetype/include"
    }

    configuration {"not macosx"}
        libdirs
        {
            "../lib",
            "../../../code/lib/static",
            "$(ORX)/lib/static"
        }

    configuration {"macosx"}
        libdirs
        {
            "../../../code/lib/dynamic",
            "$(ORX)/lib/dynamic"
        }

    configuration {}

    targetdir ("../bin/")

    flags
    {
        "NoPCH",
        "NoManifest",
        "FloatFast",
        "NoNativeWChar",
        "NoExceptions",
        "Symbols",
        "StaticRuntime"
    }

    configuration {"x32"}
        flags {"EnableSSE2"}

    configuration {"not windows"}
        flags {"Unicode"}

    configuration {"*Debug*"}
        defines {"__orxDEBUG__"}
        links {"orxd"}

    configuration {"*Profile*"}
        defines {"__orxPROFILER__"}
        flags {"Optimize", "NoRTTI"}
        links {"orxp"}

    configuration {"*Release*"}
        flags {"Optimize", "NoRTTI"}
        links {"orx"}

    configuration {}
        defines {"__orxSTATIC__"}


-- Linux

    configuration {"linux", "x32"}
        libdirs
        {
            "../../../extern/freetype/lib/linux",
            "$(ORX)/../extern/freetype/lib/linux"
        }

    configuration {"linux", "x64"}
        libdirs
        {
            "../../../extern/freetype/lib/linux64",
            "$(ORX)/../extern/freetype/lib/linux64"
        }

    configuration {"linux"}
        buildoptions {"-Wno-unused-function"}

    -- This prevents an optimization bug from happening with some versions of gcc on linux
    configuration {"linux", "not *Debug*"}
        buildoptions {"-fschedule-insns"}


-- Mac OS X

    configuration {"macosx"}
        libdirs
        {
            "../../../extern/freetype/lib/mac",
            "$(ORX)/../extern/freetype/lib/mac"
        }
        buildoptions
        {
            "-stdlib=libc++",
            "-gdwarf-2",
            "-Wno-unused-function",
            "-Wno-write-strings"
        }
        linkoptions
        {
            "-stdlib=libc++",
            "-dead_strip"
        }
        postbuildcommands {"$(shell [ -f " .. copybase .. "/../../code/lib/dynamic/liborx.dylib ] && cp -f " .. copybase .. "/../../code/lib/dynamic/liborx*.dylib " .. copybase .. "/bin)"}

    configuration {"macosx", "x32"}
        buildoptions
        {
            "-mfix-and-continue"
        }


-- Windows

    configuration {"windows", "vs*"}
        buildoptions
        {
            "/MP"
        }

    configuration {"windows", "vs*", "*Debug*"}
        linkoptions {"/NODEFAULTLIB:LIBCMT"}

    configuration {"vs2017 or vs2019 or vs2022", "x32"}
        libdirs
        {
            "../../../extern/freetype/lib/vc2015/32",
            "$(ORX)/../extern/freetype/lib/vc2015/32"
        }

    configuration {"vs2017 or vs2019 or vs2022", "x64"}
        libdirs
        {
            "../../../extern/freetype/lib/vc2015/64",
            "$(ORX)/../extern/freetype/lib/vc2015/64"
        }

    configuration {"windows", "gmake or codelite or codeblocks", "x32"}
        libdirs
        {
            "../../../extern/freetype/lib/mingw/32",
            "$(ORX)/../extern/freetype/lib/mingw/32"
        }

    configuration {"windows", "gmake or codelite or codeblocks", "x64"}
        libdirs
        {
            "../../../extern/freetype/lib/mingw/64",
            "$(ORX)/../extern/freetype/lib/mingw/64"
        }

    configuration {"windows", "gmake", "x32"}
        prebuildcommands
        {
            "$(eval CC := i686-w64-mingw32-gcc)",
            "$(eval CXX := i686-w64-mingw32-g++)",
            "$(eval AR := i686-w64-mingw32-gcc-ar)"
        }

    configuration {"windows", "gmake", "x64"}
        prebuildcommands
        {
            "$(eval CC := x86_64-w64-mingw32-gcc)",
            "$(eval CXX := x86_64-w64-mingw32-g++)",
            "$(eval AR := x86_64-w64-mingw32-gcc-ar)"
        }

    configuration {"windows", "codelite or codeblocks", "x32"}
        envs
        {
            "CC=i686-w64-mingw32-gcc",
            "CXX=i686-w64-mingw32-g++",
            "AR=i686-w64-mingw32-gcc-ar"
        }

    configuration {"windows", "codelite or codeblocks", "x64"}
        envs
        {
            "CC=x86_64-w64-mingw32-gcc",
            "CXX=x86_64-w64-mingw32-g++",
            "AR=x86_64-w64-mingw32-gcc-ar"
        }


--
-- Project: orxFontGen
--

project "orxFontGen"

    files {"../src/orxFontGen.c"}
    targetname ("orxfontgen")
    links
    {
        "freetype"
    }


-- Linux

    configuration {"linux"}
        links
        {
            "dl",
            "m",
            "z",
            "rt",
            "pthread"
        }


-- Mac OS X

    configuration {"macosx", "not codelite", "not codeblocks"}
        links
        {
            "Foundation.framework",
            "AppKit.framework",
            "OpenGL.framework"
        }

    configuration {"macosx", "codelite or codeblocks"}
        linkoptions
        {
            "-framework Foundation",
            "-framework AppKit",
            "-framework OpenGL"
        }

    configuration {"macosx"}
        links
        {
            "z",
            "pthread"
        }


-- Windows

    configuration {"windows"}
        links
        {
            "winmm",
            "OpenGL32"
        }
