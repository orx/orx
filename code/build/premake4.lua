-- This premake script should be used with orx-customized version of premake4.
-- Its Hg repository can be found at https://bitbucket.org/orx/premake-stable.
-- A copy, including binaries, can also be found in the extern/premake folder.

--
-- Globals
--

function islinux64 ()
    local pipe    = io.popen ("uname -m")
    local content = pipe:read ('*a')
    pipe:close ()

    local t64 =
    {
        'x86_64',
        'ia64',
        'amd64',
        'powerpc64',
        'sparc64'
    }

    for i, v in ipairs (t64) do
        if content:find (v) then
            return true
        end
    end

    return false
end

function initconfigurations ()
    if os.is ("macosx") then
        return
        {
            "Embedded Dynamic Debug",
            "Embedded Dynamic Profile",
            "Embedded Dynamic Release",
            "Dynamic Debug",
            "Dynamic Profile",
            "Dynamic Release"
        }
    else
        return
        {
            "Embedded Dynamic Debug",
            "Embedded Dynamic Profile",
            "Embedded Dynamic Release",
            "Embedded Static Debug",
            "Embedded Static Profile",
            "Embedded Static Release",
            "Dynamic Debug",
            "Dynamic Profile",
            "Dynamic Release",
            "Static Debug",
            "Static Profile",
            "Static Release"
        }
    end
end

function initplatforms ()
    if os.is ("windows") then
        return
        {
            "Native"
        }
    elseif os.is ("linux") then
        if islinux64 () then
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
            "x64",
            "x32"
        }
    end
end

function defaultaction (name, action)
   if os.is (name) then
      _ACTION = _ACTION or action
   end
end

defaultaction ("windows", "vs2010")
defaultaction ("linux", "gmake")
defaultaction ("macosx", "xcode3")

newoption
{
    trigger = "to",
    value   = "path",
    description = "Set the output location for the generated files"
}


--
-- Solution: orx
--

solution "orx"

    language ("C++")

    location (_OPTIONS["to"] or "./" .. _ACTION)

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
        "../../extern/dlmalloc",
        "../../extern/glfw-2.7/include",
        "../../extern/Box2D_2.1.3/include",
        "../../extern/SOIL/include",
        "../../extern/openal-soft/include",
        "../../extern/libsndfile-1.0.22/include",
        "../../extern/stb_vorbis"
    }

    excludes
    {
        "../src/main/orxAndroidNativeSupport.c",
        "../src/main/orxAndroidSupport.cpp"
    }

    flags
    {
        "NoPCH",
        "NoManifest",
        "EnableSSE2",
        "FloatFast",
        "NoNativeWChar",
        "NoExceptions",
        "Symbols",
        "StaticRuntime"
    }

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
            "../../extern/glfw-2.7/lib/linux",
            "../../extern/SOIL/lib/linux",
            "../../extern/libsndfile-1.0.22/lib/linux",
            "../../extern/Box2D_2.1.3/lib/linux"
        }

    configuration {"linux", "x64"}
        libdirs
        {
            "../../extern/glfw-2.7/lib/linux64",
            "../../extern/SOIL/lib/linux64",
            "../../extern/libsndfile-1.0.22/lib/linux64",
            "../../extern/Box2D_2.1.3/lib/linux64"
        }


-- Mac OS X

    configuration {"macosx"}
        libdirs
        {
            "../../extern/glfw-2.7/lib/mac",
            "../../extern/SOIL/lib/mac",
            "../../extern/libsndfile-1.0.22/lib/mac",
            "../../extern/Box2D_2.1.3/lib/mac"
        }
        buildoptions
        {
            "-isysroot /Developer/SDKs/MacOSX10.6.sdk",
            "-mmacosx-version-min=10.6",
            "-gdwarf-2"
        }
        linkoptions
        {
            "-isysroot /Developer/SDKs/MacOSX10.6.sdk",
            "-mmacosx-version-min=10.6",
            "-dead_strip"
        }


-- Windows

    configuration {"vs2008"}
        libdirs
        {
            "../../extern/glfw-2.7/lib/vc2008",
            "../../extern/SOIL/lib/msvs2008",
            "../../extern/openal-soft/lib/vc2008",
            "../../extern/libsndfile-1.0.22/lib/vc2008",
            "../../extern/Box2D_2.1.3/lib/msvs2008"
        }

    configuration {"vs2010"}
        libdirs
        {
            "../../extern/glfw-2.7/lib/vc2010",
            "../../extern/SOIL/lib/msvs2010",
            "../../extern/openal-soft/lib/vc2010",
            "../../extern/libsndfile-1.0.22/lib/vc2010",
            "../../extern/Box2D_2.1.3/lib/msvs2010"
        }

    configuration {"windows", "codelite or gmake"}
        libdirs
        {
            "../../extern/glfw-2.7/lib/mingw",
            "../../extern/SOIL/lib/mingw",
            "../../extern/openal-soft/lib/mingw",
            "../../extern/libsndfile-1.0.22/lib/mingw",
            "../../extern/Box2D_2.1.3/lib/mingw"
        }


--
-- Project: orx
--

project "orx"

    files {"../src/main/orxMain.c"}

    targetdir ("../bin/")

    kind ("ConsoleApp")

    links {"orxLIB"}

    configuration {"*Static*"}
        defines {"__orxSTATIC__"}

    configuration {"*Static*", "*Debug*"}
        links {"Box2Dd"}

    configuration {"*Static*", "not *Debug*"}
        links {"Box2D"}


-- Linux

    configuration {"linux"}
        linkoptions {"-Wl,-rpath ./", "-Wl,--export-dynamic"}

    configuration {"linux", "*Static*"}
        linkoptions {"-Wl,--no-whole-archive"}
        links
        {
            "glfw",
            "openal",
            "sndfile",
            "SOIL",
            "GL",
            "X11",
            "Xrandr",
            "dl",
            "m",
            "rt"
        }

    configuration {"linux", "x64", "*Static*"}
        links {"pthread"}

    -- This prevents an optimization bug from happening with some versions of gcc on linux
    configuration {"linux", "not *Debug*"}
        buildoptions {"-fschedule-insns"}


-- Mac OS X

    configuration {"macosx", "*Static*"}
        links
        {
            "Foundation.framework",
            "IOKit.framework",
            "AppKit.framework",
            "glfw",
            "SOIL",
            "sndfile",
            "OpenAL.framework",
            "OpenGL.framework"
        }


-- Windows

    configuration {"windows", "*Static*"}
        implibdir ("../lib/static")
        implibname ("imporx")
        implibextension (".lib")
        links
        {
            "glfw",
            "openal32",
            "SOIL",
            "winmm",
            "sndfile"
        }

    configuration {"windows", "vs*", "*Static*"}
        links {"OpenGL32"}


--
-- Project: orxLIB
--

project "orxLIB"

    files
    {
        "../src/**.cpp",
        "../src/**.c",
        "../include/**.h"
    }

    excludes {"../src/main/orxMain.c"}

    targetname ("orx")

    configuration {"*Embedded*"}
        defines
        {
            "__orxEMBEDDED__",
            "AL_LIBTYPE_STATIC"
        }

    -- Work around for codelite "default" configuration
    configuration {"codelite"}
        kind ("StaticLib")

    configuration {"*Static*"}
        targetdir ("../lib/static")
        kind ("StaticLib")

    configuration {"*Dynamic*"}
        targetdir ("../lib/dynamic")
        kind ("SharedLib")

    configuration {"*Debug*"}
        links {"Box2Dd"}

    configuration {"not *Debug*"}
        links {"Box2D"}


-- Linux

    configuration {"linux"}
        links
        {
            "glfw",
            "openal",
            "sndfile",
            "SOIL",
            "GL",
            "X11",
            "Xrandr",
            "dl",
            "m",
            "rt"
        }

    configuration {"linux", "*Static*"}
        buildoptions {"-fPIC"}

    configuration {"linux", "*Dynamic*"}
        postbuildcommands {"cp -f ../../lib/dynamic/liborx*.so ../../bin"}


-- Mac OS X

    configuration {"macosx"}
        links
        {
            "Foundation.framework",
            "IOKit.framework",
            "AppKit.framework",
            "glfw",
            "SOIL",
            "sndfile",
            "OpenAL.framework",
            "OpenGL.framework"
        }

    configuration {"macosx", "*Debug*"}
        linkoptions {"-install_name @executable_path/liborxd.dylib"}

    configuration {"macosx", "*Profile*"}
        linkoptions {"-install_name @executable_path/liborxp.dylib"}

    configuration {"macosx", "*Release*"}
        linkoptions {"-install_name @executable_path/liborx.dylib"}

    configuration {"macosx", "*Dynamic*"}
        postbuildcommands {"cp -f ../../lib/dynamic/liborx*.dylib ../../bin"}


-- Windows

    configuration {"windows"}
        links
        {
            "glfw",
            "openal32",
            "SOIL",
            "winmm",
            "sndfile"
        }

    configuration {"windows", "vs*"}
        links {"OpenGL32"}

    configuration {"windows", "*Dynamic*"}
        postbuildcommands {"cmd /c copy /Y ..\\..\\lib\\dynamic\\orx*.dll ..\\..\\bin"}


--
-- Project: bounce
--

project "Bounce"

    files {"../plugins/Demo/orxBounce.c"}

    targetdir ("../bin/plugins/demo/")

    targetprefix ("")

    kind ("SharedLib")

    links
    {
        "orxLIB",
        "orx"
    }


-- Linux


-- Mac OS X

    configuration {"macosx"}
        targetextension (".so")
        linkoptions {"-single_module"}


-- Windows

    configuration {"windows", "*Static*"}
        libdirs {"../lib/static"}

    configuration {"windows", "*Static*", "*Debug*"}
        links {"imporxd"}
    configuration {"windows", "*Static*", "*Profile*"}
        links {"imporxp"}
    configuration {"windows", "*Static*", "*Release*"}
        links {"imporx"}
