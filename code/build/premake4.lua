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
        if string.find(string.lower(_ACTION), "xcode") then
            return
            {
                "Universal"
            }
        else
            return
            {
                "x32", "x64"
            }
        end
    end
end

function defaultaction (name, action)
   if os.is (name) then
      _ACTION = _ACTION or action
   end
end

defaultaction ("windows", "vs2012")
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
        "../../extern/dlmalloc",
        "../../extern/glfw-2.7/include",
        "../../extern/Box2D_2.1.3/include",
        "../../extern/stb_image",
        "../../extern/openal-soft/include",
        "../../extern/libsndfile-1.0.22/include",
        "../../extern/stb_vorbis",
        "../../extern/libwebp/include"
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
        "EnableSSE2",
        "FloatFast",
        "NoNativeWChar",
        "NoExceptions",
        "NoIncrementalLink",
        "NoEditAndContinue",
        "NoMinimalRebuild",
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
            "../../extern/libsndfile-1.0.22/lib/linux",
            "../../extern/Box2D_2.1.3/lib/linux",
            "../../extern/libwebp/lib/linux"
        }
        buildoptions
        {
            "-Wno-unused-function"
        }

    configuration {"linux", "x64"}
        libdirs
        {
            "../../extern/glfw-2.7/lib/linux64",
            "../../extern/libsndfile-1.0.22/lib/linux64",
            "../../extern/Box2D_2.1.3/lib/linux64",
            "../../extern/libwebp/lib/linux64"
        }
        buildoptions
        {
            "-Wno-unused-function"
        }


-- Mac OS X

    configuration {"macosx"}
        libdirs
        {
            "../../extern/glfw-2.7/lib/mac",
            "../../extern/libsndfile-1.0.22/lib/mac",
            "../../extern/Box2D_2.1.3/lib/mac"
        }
        buildoptions
        {
            "-x c++",
            "-gdwarf-2",
            "-Wno-write-strings",
            "-fvisibility-inlines-hidden"
        }
        linkoptions
        {
            "-dead_strip"
        }

    configuration {"macosx", "x32"}
        buildoptions
        {
            "-mfix-and-continue"
        }


-- Windows

    configuration {"vs2008"}
        libdirs
        {
            "../../extern/glfw-2.7/lib/vc2008",
            "../../extern/openal-soft/lib/vc2008",
            "../../extern/libsndfile-1.0.22/lib/vc2008",
            "../../extern/Box2D_2.1.3/lib/msvs2008"
        }

    configuration {"vs2010"}
        libdirs
        {
            "../../extern/glfw-2.7/lib/vc2010",
            "../../extern/openal-soft/lib/vc2010",
            "../../extern/libsndfile-1.0.22/lib/vc2010",
            "../../extern/Box2D_2.1.3/lib/msvs2010"
        }

    configuration {"vs2012"}
        libdirs
        {
            "../../extern/glfw-2.7/lib/vc2012",
            "../../extern/openal-soft/lib/vc2012",
            "../../extern/libsndfile-1.0.22/lib/vc2012",
            "../../extern/Box2D_2.1.3/lib/msvs2012"
        }

    configuration {"windows", "codeblocks or codelite or gmake"}
        libdirs
        {
            "../../extern/glfw-2.7/lib/mingw",
            "../../extern/openal-soft/lib/mingw",
            "../../extern/libsndfile-1.0.22/lib/mingw",
            "../../extern/Box2D_2.1.3/lib/mingw"
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

    configuration {"*Static*"}
        defines {"__orxSTATIC__"}

    configuration {"*Static*", "*Debug*"}
        links {"Box2Dd", "webpdecoder"}

    configuration {"*Static*", "not *Debug*"}
        links {"Box2D", "webpdecoder"}


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
            "GL",
            "X11",
            "Xrandr",
            "dl",
            "m",
            "rt",
            "pthread"
        }

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
            "sndfile",
            "pthread",
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

    if _OPTIONS["split-platforms"] then
        configuration {"*Static*", "x32"}
            targetdir ("../lib/static/x32")

        configuration {"*Dynamic*", "x32"}
            targetdir ("../lib/dynamic/x32")

        configuration {"*Static*", "x64"}
            targetdir ("../lib/static/x64")

        configuration {"*Dynamic*", "x64"}
            targetdir ("../lib/dynamic/x64")

        configuration {}
    end

    configuration {"*Debug*"}
        links {"Box2Dd", "webpdecoder"}

    configuration {"not *Debug*"}
        links {"Box2D", "webpdecoder"}


-- Linux

    configuration {"linux"}
        links
        {
            "glfw",
            "openal",
            "sndfile",
            "GL",
            "X11",
            "Xrandr",
            "dl",
            "m",
            "rt",
            "pthread"
        }
        defines {"_GNU_SOURCE"}

    configuration {"linux", "*Static*"}
        buildoptions {"-fPIC"}

    if _OPTIONS["split-platforms"] then
        configuration {"linux", "*Dynamic*", "x32"}
            postbuildcommands {"mkdir " .. copybase .. "/bin/x32 ; cp -f " .. copybase .. "/lib/dynamic/x32/liborx*.so " .. copybase .. "/bin/x32"}

        configuration {"linux", "*Dynamic*", "x64"}
            postbuildcommands {"mkdir " .. copybase .. "/bin/x64 ; cp -f " .. copybase .. "/lib/dynamic/x64/liborx*.so " .. copybase .. "/bin/x64"}

        configuration {"linux", "*Dynamic*", "not x32", "not x64"}
            postbuildcommands {"cp -f " .. copybase .. "/lib/dynamic/liborx*.so " .. copybase .. "/bin"}

        configuration {}
    else
        configuration {"linux", "*Dynamic*"}
            postbuildcommands {"cp -f " .. copybase .. "/lib/dynamic/liborx*.so " .. copybase .. "/bin"}
    end


-- Mac OS X

    configuration {"macosx"}
        links
        {
            "Foundation.framework",
            "IOKit.framework",
            "AppKit.framework",
            "glfw",
            "sndfile",
            "pthread",
            "OpenAL.framework",
            "OpenGL.framework"
        }

    configuration{"macosx"}
        buildoptions{"-Wno-deprecated-declarations", "-Wno-empty-body"}

    configuration {"macosx", "*Debug*"}
        linkoptions {"-install_name @executable_path/liborxd.dylib"}

    configuration {"macosx", "*Profile*"}
        linkoptions {"-install_name @executable_path/liborxp.dylib"}

    configuration {"macosx", "*Release*"}
        linkoptions {"-install_name @executable_path/liborx.dylib"}

    if _OPTIONS["split-platforms"] then
        configuration {"macosx", "*Dynamic*", "x32"}
            postbuildcommands {"mkdir " .. copybase .. "/bin/x32 ; cp -f " .. copybase .. "/lib/dynamic/x32/liborx*.dylib " .. copybase .. "/bin/x32"}

        configuration {"macosx", "*Dynamic*", "x64"}
            postbuildcommands {"mkdir " .. copybase .. "/bin/x64 ; cp -f " .. copybase .. "/lib/dynamic/x64/liborx*.dylib " .. copybase .. "/bin/x64"}

        configuration {"macosx", "*Dynamic*", "not x32", "not x64"}
            postbuildcommands {"cp -f " .. copybase .. "/lib/dynamic/liborx*.dylib " .. copybase .. "/bin"}

        configuration {}
    else
        configuration {"macosx", "*Dynamic*"}
            postbuildcommands {"cp -f " .. copybase .. "/lib/dynamic/liborx*.dylib " .. copybase .. "/bin"}
    end


-- Windows

    configuration {"windows"}
        links
        {
            "glfw",
            "openal32",
            "winmm",
            "sndfile"
        }

    configuration {"windows", "vs*"}
        links {"OpenGL32"}

    configuration {"windows", "vs*", "*Debug*"}
        linkoptions {"/NODEFAULTLIB:LIBCMT"}

    configuration {"windows", "*Dynamic*"}
        postbuildcommands {"cmd /c copy /Y " .. path.translate(copybase, "\\") .. "\\lib\\dynamic\\orx*.dll " .. path.translate(copybase, "\\") .. "\\bin"}


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

    configuration {"windows", "*Static*"}
        libdirs {"../lib/static"}

    configuration {"windows", "*Static*", "*Debug*"}
        links {"imporxd"}
    configuration {"windows", "*Static*", "*Profile*"}
        links {"imporxp"}
    configuration {"windows", "*Static*", "*Release*"}
        links {"imporx"}
