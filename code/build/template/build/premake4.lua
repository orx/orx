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
            "x64"
        }
    end
end

function defaultaction (name, action)
   if os.is (name) then
      _ACTION = _ACTION or action
   end
end

defaultaction ("windows", "vs2019")
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
-- Solution: [name]
--

solution "[name]"

    language ("C[+c++ ++]")

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

    targetdir ("../bin")

    flags
    {
        "NoPCH",
        "NoManifest",
        "FloatFast",
        "NoNativeWChar",
        "NoExceptions",
        "NoIncrementalLink",
        "NoEditAndContinue",
        "NoMinimalRebuild",
        "Symbols",
        "StaticRuntime"
    }

    configuration {"not xcode*"}
        includedirs {"$(ORX)/include"}
        libdirs {"$(ORX)/lib/static"}

    configuration {"xcode*"}
        includedirs {"[code-path]/include"}
        libdirs {"[code-path]/lib/static"}

    configuration {"x32"}
        flags {"EnableSSE2"}

    configuration {"not windows"}
        flags {"Unicode"}

    configuration {"*Debug*"}
        targetsuffix ("d")
        defines {"__orxDEBUG__"}
        links {"orxd"}

    configuration {"*Profile*"}
        targetsuffix ("p")
        defines {"__orxPROFILER__"}
        flags {"Optimize", "NoRTTI"}
        links {"orxp"}

    configuration {"*Release*"}
        flags {"Optimize", "NoRTTI"}
        links {"orx"}

    configuration {"windows", "*Release*"}
        kind ("WindowedApp")

    configuration {}
        links {"webpdecoder"}

    configuration {"not vs*"}
        links {"basisu"}

    configuration {"not *Debug*", "vs*"}
        links {"basisu"}

    configuration {"*Debug*", "vs*"}
        links {"basisud"}

    configuration {"not *Debug*"}
        links {"liquidfun"}

    configuration {"*Debug*"}
        links {"liquidfund"}


-- Linux

    configuration {"linux"}
        buildoptions {"-Wno-unused-function"}
        links
        {
            "dl",
            "m",
            "rt",
            "pthread",
            "glfw3",
            "X11",
            "Xrandr",
            "gcc"
        }
        if _OPTIONS["gles"] then
            links {"GLESv3"}
        else
            links {"GL"}
        end

    configuration {"linux", "x32"}
        libdirs
        {
            "$(ORX)/../extern/glfw-3/lib/linux",
            "$(ORX)/../extern/LiquidFun-1.1.0/lib/linux",
            "$(ORX)/../extern/libwebp/lib/linux",
            "$(ORX)/../extern/basisu/lib/linux/32",
        }

    configuration {"linux", "x64"}
        libdirs
        {
            "$(ORX)/../extern/glfw-3/lib/linux64",
            "$(ORX)/../extern/LiquidFun-1.1.0/lib/linux64",
            "$(ORX)/../extern/libwebp/lib/linux64",
            "$(ORX)/../extern/basisu/lib/linux/64",
        }

    -- This prevents an optimization bug from happening with some versions of gcc on linux
    configuration {"linux", "not *Debug*"}
        buildoptions {"-fschedule-insns"}


-- Mac OS X

    configuration {"macosx"}
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
        links
        {
            "m",
            "pthread",
            "glfw3"
        }

    configuration {"macosx", "not xcode*"}
        libdirs
        {
            "$(ORX)/../extern/glfw-3/lib/mac",
            "$(ORX)/../extern/LiquidFun-1.1.0/lib/mac",
            "$(ORX)/../extern/libwebp/lib/mac",
            "$(ORX)/../extern/basisu/lib/mac",
        }

    configuration {"macosx", "xcode*"}
        libdirs
        {
            "[code-path]/../extern/glfw-3/lib/mac",
            "[code-path]/../extern/LiquidFun-1.1.0/lib/mac",
            "[code-path]/../extern/libwebp/lib/mac",
            "[code-path]/../extern/basisu/lib/mac",
        }

    configuration {"macosx", "not codelite", "not codeblocks"}
        links
        {
            "Foundation.framework",
            "AppKit.framework"
        }

    configuration {"macosx", "codelite or codeblocks"}
        linkoptions
        {
            "-framework Foundation",
            "-framework AppKit"
        }

    configuration {"xcode* or macosx", "gmake"}
        links
        {
            "Foundation.framework",
            "AppKit.framework",
            "Foundation.framework",
            "CoreFoundation.framework",
            "CoreAudio.framework",
            "AudioUnit.framework",
            "IOKit.framework",
            "AppKit.framework",
            "CoreVideo.framework",
            "OpenGL.framework"
        }

    configuration {"macosx", "codelite or codeblocks"}
        linkoptions
        {
            "-framework Foundation",
            "-framework AppKit",
            "-framework Foundation",
            "-framework CoreFoundation",
            "-framework CoreAudio",
            "-framework AudioUnit",
            "-framework IOKit",
            "-framework AppKit",
            "-framework CoreVideo",
            "-framework OpenGL"
        }

    configuration {"macosx", "x32"}
        buildoptions
        {
            "-mfix-and-continue"
        }


-- Windows

[+remote
    configuration {"windows"}
        links
        {
            "ws2_32"
        }
]
    configuration {"windows"}
        links
        {
            "winmm",
            "glfw3"
        }

    configuration {"windows", "vs*"}
        links {"OpenGL32"}
        buildoptions
        {
            "/MP",
            "/EHsc"
        }

    configuration {"windows", "vs*", "*Debug*", "not *Core*"}
        linkoptions {"/NODEFAULTLIB:LIBCMT", "/ignore:4099"}


    configuration {"vs2015 or vs2017 or vs2019", "x32"}
        libdirs
        {
            "$(ORX)/../extern/glfw-3/lib/vc2015/32",
            "$(ORX)/../extern/LiquidFun-1.1.0/lib/vc2015/32",
            "$(ORX)/../extern/libwebp/lib/vc2015/32",
            "$(ORX)/../extern/basisu/lib/vc2015/32",
        }

    configuration {"vs2015 or vs2017 or vs2019", "x64"}
        libdirs
        {
            "$(ORX)/../extern/glfw-3/lib/vc2015/64",
            "$(ORX)/../extern/LiquidFun-1.1.0/lib/vc2015/64",
            "$(ORX)/../extern/libwebp/lib/vc2015/64",
            "$(ORX)/../extern/basisu/lib/vc2015/64",
        }

    configuration {"windows", "gmake or codelite or codeblocks", "x32"}
        libdirs
        {
            "$(ORX)/../extern/glfw-3/lib/mingw/32",
            "$(ORX)/../extern/LiquidFun-1.1.0/lib/mingw/32",
            "$(ORX)/../extern/libwebp/lib/mingw/32",
            "$(ORX)/../extern/basisu/lib/mingw/32",
        }

    configuration {"windows", "gmake or codelite or codeblocks", "x64"}
        libdirs
        {
            "$(ORX)/../extern/glfw-3/lib/mingw/64",
            "$(ORX)/../extern/LiquidFun-1.1.0/lib/mingw/64",
            "$(ORX)/../extern/libwebp/lib/mingw/64",
            "$(ORX)/../extern/basisu/lib/mingw/64",
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
-- Project: [name]
--

project "[name]"

    files
    {
[+c++
        "../src/**.cpp",]
        "../src/**.c",
        "../include/**.h",
[+scroll
        "../include/**.inl",]
        "../data/config/**.ini"
    }

    includedirs
    {
[+scroll
        "../include/Scroll",]
[+imgui
        "../include/imgui",]
[+nuklear
        "../include/nuklear",]
        "../include"
    }

    configuration {"windows", "vs*"}
        buildoptions {"/EHsc"}

    vpaths
    {
[+scroll
        ["inline"] = {"**.inl"},]
        ["config"] = {"**.ini"}
    }
