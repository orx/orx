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
        "Release",
        "Core Debug",
        "Core Profile",
        "Core Release",
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

newoption
{
    trigger = "split-platforms",
    description = "Split target folders based on platforms"
}

newoption
{
    trigger = "gles",
    description = "Use OpenGL ES instead of OpenGL (Linux only)"
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

solution "orx"

    language ("C++")

    location (destination)

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
        "../../extern/rpmalloc/rpmalloc",
        "../../extern/xxHash",
        "../../extern/glfw-3/include",
        "../../extern/stb_image",
        "../../extern/libwebp/include",
        "../../extern/basisu/include",
        "../../extern/stb_vorbis",
        "../../extern/miniaudio",
        "../../extern/LiquidFun-1.1.0/include"
    }

    excludes
    {
        "../src/main/orxAndroidSupport.cpp",
        "../src/main/orxAndroidNativeSupport.cpp"
    }

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
        "StaticRuntime",
        "FatalWarnings"
    }

    configuration {"x32"}
        flags {"EnableSSE2"}

    configuration {"not windows"}
        flags {"Unicode"}

    configuration {"*Debug*"}
        targetsuffix ("d")
        defines {"__orxDEBUG__"}

    configuration {"*Profile*"}
        targetsuffix ("p")
        defines {"__orxPROFILER__"}
        flags {"Optimize", "NoRTTI"}

    configuration {"*Release*"}
        flags {"Optimize", "NoRTTI"}


-- Linux

    configuration {"linux", "x32"}
        libdirs
        {
            "../../extern/glfw-3/lib/linux",
            "../../extern/LiquidFun-1.1.0/lib/linux",
            "../../extern/libwebp/lib/linux",
            "../../extern/basisu/lib/linux/32",
        }
        buildoptions
        {
            "-Wno-unused-function",
            "-Wno-unused-but-set-variable"
        }

    configuration {"linux", "x64"}
        libdirs
        {
            "../../extern/glfw-3/lib/linux64",
            "../../extern/LiquidFun-1.1.0/lib/linux64",
            "../../extern/libwebp/lib/linux64",
            "../../extern/basisu/lib/linux/64",
        }
        buildoptions
        {
            "-Wno-unused-function",
            "-Wno-unused-but-set-variable"
        }


-- Mac OS X

    configuration {"macosx"}
        libdirs
        {
            "../../extern/glfw-3/lib/mac",
            "../../extern/LiquidFun-1.1.0/lib/mac",
            "../../extern/libwebp/lib/mac",
            "../../extern/basisu/lib/mac",
        }
        buildoptions
        {
            "-x c++",
            "-gdwarf-2",
            "-Wno-unused-function",
            "-Wno-write-strings",
            "-fvisibility-inlines-hidden",
            "-stdlib=libc++"
        }
        linkoptions
        {
            "-stdlib=libc++",
            "-dead_strip"
        }

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

    configuration {"vs2015 or vs2017 or vs2019", "x32"}
        libdirs
        {
            "../../extern/glfw-3/lib/vc2015/32",
            "../../extern/LiquidFun-1.1.0/lib/vc2015/32",
            "../../extern/libwebp/lib/vc2015/32",
            "../../extern/basisu/lib/vc2015/32",
        }

    configuration {"vs2015 or vs2017 or vs2019", "x64"}
        libdirs
        {
            "../../extern/glfw-3/lib/vc2015/64",
            "../../extern/LiquidFun-1.1.0/lib/vc2015/64",
            "../../extern/libwebp/lib/vc2015/64",
            "../../extern/basisu/lib/vc2015/64",
        }

    configuration {"windows", "gmake or codelite or codeblocks", "x32"}
        libdirs
        {
            "../../extern/glfw-3/lib/mingw/32",
            "../../extern/LiquidFun-1.1.0/lib/mingw/32",
            "../../extern/libwebp/lib/mingw/32",
            "../../extern/basisu/lib/mingw/32",
        }

    configuration {"windows", "gmake or codelite or codeblocks", "x64"}
        libdirs
        {
            "../../extern/glfw-3/lib/mingw/64",
            "../../extern/LiquidFun-1.1.0/lib/mingw/64",
            "../../extern/libwebp/lib/mingw/64",
            "../../extern/basisu/lib/mingw/64",
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

    configuration {"windows", "codeblocks", "x32"}
        envs
        {
            "CC=i686-w64-mingw32-gcc",
            "CXX=i686-w64-mingw32-g++",
            "AR=i686-w64-mingw32-gcc-ar"
        }

    configuration {"windows", "codeblocks", "x64"}
        envs
        {
            "CC=x86_64-w64-mingw32-gcc",
            "CXX=x86_64-w64-mingw32-g++",
            "AR=x86_64-w64-mingw32-gcc-ar"
        }


--
-- Project: orx
--

project "orx"

    files {"../src/main/orxMain.c"}

    targetdir ("../bin")
    if _OPTIONS["split-platforms"] then
        configuration {"x32"}
            targetdir ("../bin/x32")

        configuration {"x64"}
            targetdir ("../bin/x64")

        configuration {}
    end

    kind ("ConsoleApp")

    links {"orxLIB"}

    configuration {"not *Core*"}
        links {"webpdecoder"}

    configuration {"not *Core*", "not vs*"}
        links {"basisu"}

    configuration {"not *Debug*", "not *Core*", "vs*"}
        links {"basisu"}

    configuration {"*Debug*", "not *Core*", "vs*"}
        links {"basisud"}

    configuration {"not *Debug*", "not *Core*"}
        links {"liquidfun"}

    configuration {"*Debug*", "not *Core*"}
        links {"liquidfund"}

-- Linux

    configuration {"linux"}
        linkoptions {"-Wl,--export-dynamic", "-Wl,--no-whole-archive"}
        links
        {
            "dl",
            "m",
            "rt",
            "pthread"
        }

    configuration {"linux", "not *Core*"}
        links
        {
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

    -- This prevents an optimization bug from happening with some versions of gcc on linux
    configuration {"linux", "not *Debug*"}
        buildoptions {"-fschedule-insns"}


-- Mac OS X

    configuration {"macosx"}
        linkoptions {"-Wl,--export-dynamic"}

    configuration {"macosx", "not *Core*"}
        links
        {
            "glfw3",
            "m",
            "pthread"
        }

    configuration {"xcode* or macosx", "gmake"}
        links
        {
            "Foundation.framework",
            "IOKit.framework"
        }

    configuration {"macosx", "codelite or codeblocks"}
        linkoptions
        {
            "-framework Foundation",
            "-framework IOKit"
        }

    configuration {"xcode* or macosx", "not *Core*", "gmake"}
        links
        {
            "Foundation.framework",
            "CoreFoundation.framework",
            "CoreAudio.framework",
            "AudioUnit.framework",
            "IOKit.framework",
            "AppKit.framework",
            "CoreVideo.framework",
            "OpenGL.framework"
        }

    configuration {"macosx", "not *Core*", "codelite or codeblocks"}
        linkoptions
        {
            "-framework Foundation",
            "-framework CoreFoundation",
            "-framework CoreAudio",
            "-framework AudioUnit",
            "-framework IOKit",
            "-framework AppKit",
            "-framework CoreVideo",
            "-framework OpenGL"
        }


-- Windows

    configuration {"windows"}
        implibdir ("../lib/static")
        implibname ("imporx")
        implibextension (".lib")
        links
        {
            "winmm"
        }

    configuration {"windows", "not *Core*"}
        links
        {
            "glfw3"
        }

    configuration {"windows", "not *Core*", "vs*"}
        links {"OpenGL32"}

    configuration {"windows", "vs*", "*Debug*", "not *Core*"}
        linkoptions {"/NODEFAULTLIB:LIBCMT", "/ignore:4099"}


--
-- Project: orxLIB
--

project "orxLIB"

    files
    {
        "../src/**.cpp",
        "../src/**.c",
        "../include/**.h",
        "../include/**.inc"
    }

    excludes {"../src/main/orxMain.c"}

    targetname ("orx")

    configuration {"not *Core*"}
        defines
        {
            "__orxEMBEDDED__"
        }

    -- Work around for codelite "default" configuration
    configuration {"codelite"}
        kind ("StaticLib")

    configuration {}
        targetdir ("../lib/static")
        kind ("StaticLib")
        buildoptions {"$(ORXFLAGS)"}

    if _OPTIONS["split-platforms"] then
        configuration {"x32"}
            targetdir ("../lib/static/x32")

        configuration {"x64"}
            targetdir ("../lib/static/x64")

        configuration {}
    end


-- Linux

    configuration {"linux"}
        defines {"_GNU_SOURCE"}
        buildoptions {"-fPIC"}
        if _OPTIONS["gles"] then
            defines {"__orxDISPLAY_OPENGL_ES__"}
        end


-- Mac OS X

    configuration{"macosx"}
        buildoptions{"-Wno-deprecated-declarations", "-Wno-empty-body"}


-- Windows

    configuration {"windows", "vs*"}
        buildoptions {"/wd\"4577\""}

    configuration {"windows", "not vs*"}
        defines {"_WIN32_WINNT=_WIN32_WINNT_VISTA"}


--
-- Project: bounce
--

project "Bounce"

    files {"../plugins/Demo/orxBounce.c"}

    targetdir ("../bin/plugins/demo")

    if _OPTIONS["split-platforms"] then
        configuration {"x32"}
            targetdir ("../bin/plugins/demo/x32")

        configuration {"x64"}
            targetdir ("../bin/plugins/demo/x64")

        configuration {}
    end

    targetprefix ("")

    kind ("SharedLib")

    links
    {
        "orxLIB"
    }

    configuration {"not xcode*"}
        links {"orx"}


-- Linux


-- Mac OS X

    configuration {"macosx"}
        targetextension (".so")
        linkoptions {"-single_module"}


-- Windows

    configuration {"windows"}
        libdirs {"../lib/static"}

    configuration {"windows", "*Debug*"}
        links {"imporxd"}
    configuration {"windows", "*Profile*"}
        links {"imporxp"}
    configuration {"windows", "*Release*"}
        links {"imporx"}
