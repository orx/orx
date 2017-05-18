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
    return
    {
        "Debug",
        "Profile",
        "Release",
        "Core Debug",
        "Core Profile",
        "Core Release"
    }
end

function initplatforms ()
    if os.is ("windows") then
        if string.lower(_ACTION) == "vs2013"
        or string.lower(_ACTION) == "vs2015"
        or string.lower(_ACTION) == "vs2017" then
            return
            {
                "x64",
                "x32"
            }
        else
            return
            {
                "Native"
            }
        end
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

defaultaction ("windows", "vs2015")
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
        "../../extern/LiquidFun-1.1.0/include",
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
        "FloatFast",
        "NoNativeWChar",
        "NoExceptions",
        "NoIncrementalLink",
        "NoEditAndContinue",
        "NoMinimalRebuild",
        "Symbols",
        "StaticRuntime"
    }

    configuration {"not vs2013", "not vs2015", "not vs2017"}
        flags {"EnableSSE2"}

    configuration {"not x64"}
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
            "../../extern/glfw-2.7/lib/linux",
            "../../extern/libsndfile-1.0.22/lib/linux",
            "../../extern/LiquidFun-1.1.0/lib/linux",
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
            "../../extern/LiquidFun-1.1.0/lib/linux64",
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
            "../../extern/LiquidFun-1.1.0/lib/mac",
            "../../extern/libwebp/lib/mac"
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

    configuration {"windows", "vs*"}
        buildoptions
        {
            "/MP"
        }

    configuration {"vs2012"}
        libdirs
        {
            "../../extern/glfw-2.7/lib/vc2012",
            "../../extern/openal-soft/lib/vc2012",
            "../../extern/libsndfile-1.0.22/lib/vc2012",
            "../../extern/LiquidFun-1.1.0/lib/vc2012",
            "../../extern/libwebp/lib/vc2012"
        }

    configuration {"vs2013", "x32"}
        libdirs
        {
            "../../extern/glfw-2.7/lib/vc2013/32",
            "../../extern/openal-soft/lib/vc2013/32",
            "../../extern/libsndfile-1.0.22/lib/vc2013/32",
            "../../extern/LiquidFun-1.1.0/lib/vc2013/32",
            "../../extern/libwebp/lib/vc2013/32"
        }

    configuration {"vs2013", "x64"}
        libdirs
        {
            "../../extern/glfw-2.7/lib/vc2013/64",
            "../../extern/openal-soft/lib/vc2013/64",
            "../../extern/libsndfile-1.0.22/lib/vc2013/64",
            "../../extern/LiquidFun-1.1.0/lib/vc2013/64",
            "../../extern/libwebp/lib/vc2013/64"
        }

    configuration {"vs2015 or vs2017", "x32"}
        libdirs
        {
            "../../extern/glfw-2.7/lib/vc2015/32",
            "../../extern/openal-soft/lib/vc2015/32",
            "../../extern/libsndfile-1.0.22/lib/vc2015/32",
            "../../extern/LiquidFun-1.1.0/lib/vc2015/32",
            "../../extern/libwebp/lib/vc2015/32"
        }

    configuration {"vs2015 or vs2017", "x64"}
        libdirs
        {
            "../../extern/glfw-2.7/lib/vc2015/64",
            "../../extern/openal-soft/lib/vc2015/64",
            "../../extern/libsndfile-1.0.22/lib/vc2015/64",
            "../../extern/LiquidFun-1.1.0/lib/vc2015/64",
            "../../extern/libwebp/lib/vc2015/64"
        }

    configuration {"windows", "codeblocks or codelite or gmake"}
        libdirs
        {
            "../../extern/glfw-2.7/lib/mingw",
            "../../extern/openal-soft/lib/mingw",
            "../../extern/libsndfile-1.0.22/lib/mingw",
            "../../extern/LiquidFun-1.1.0/lib/mingw",
            "../../extern/libwebp/lib/mingw"
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

    configuration {"not xcode*", "*Core*"}
        defines {"__orxSTATIC__"}


-- Linux

    configuration {"linux"}
        linkoptions {"-Wl,-rpath ./", "-Wl,--export-dynamic"}

    configuration {"linux", "*Core*"}
        linkoptions {"-Wl,--no-whole-archive"}
        links
        {
            "dl",
            "m",
            "rt",
            "pthread"
        }

    -- This prevents an optimization bug from happening with some versions of gcc on linux
    configuration {"linux", "not *Debug*"}
        buildoptions {"-fschedule-insns"}


-- Mac OS X

    configuration {"macosx", "not xcode*", "*Core*"}
        links
        {
            "Foundation.framework",
            "IOKit.framework",
            "AppKit.framework",
            "pthread"
        }


-- Windows

    configuration {"windows", "*Core*"}
        implibdir ("../lib/static")
        implibname ("imporx")
        implibextension (".lib")
        links
        {
            "winmm"
        }


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

    configuration {"not *Core*"}
        defines
        {
            "__orxEMBEDDED__",
            "AL_LIBTYPE_STATIC"
        }

    -- Work around for codelite "default" configuration
    configuration {"codelite"}
        kind ("StaticLib")

    configuration {}
        targetdir ("../lib/dynamic")
        kind ("SharedLib")
        buildoptions {"$(ORXFLAGS)"}

    configuration {"not xcode*", "*Core*"}
        targetdir ("../lib/static")
        kind ("StaticLib")

    if _OPTIONS["split-platforms"] then
        configuration {"x32"}
        targetdir ("../lib/dynamic/x32")

        configuration {"not xcode*", "*Core*", "x32"}
            targetdir ("../lib/static/x32")

        configuration {"x64"}
        targetdir ("../lib/dynamic/x64")

        configuration {"not xcode*", "*Core*", "x64"}
            targetdir ("../lib/static/x64")

        configuration {}
    end

    configuration {"not *Core*"}
        links {"webpdecoder"}

    configuration {"*Debug*", "not *Core*"}
        links {"liquidfund"}

    configuration {"not *Debug*", "not *Core*"}
        links {"liquidfun"}


-- Linux

    configuration {"linux"}
        defines {"_GNU_SOURCE"}
        prebuildcommands {"if [ ! -e " .. copybase .. "/include/base/orxBuild.h ]; then hg log -l 1 --template \"#define __orxVERSION_BUILD__ {rev}\" > " .. copybase .. "/include/base/orxBuild.h; fi"}

    configuration {"linux", "not *Core*"}
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
            "pthread",
            "gcc"
        }

    configuration {"linux", "*Core*"}
        buildoptions {"-fPIC"}

    if _OPTIONS["split-platforms"] then
        configuration {"linux", "not *Core*", "x32"}
            postbuildcommands {"mkdir " .. copybase .. "/bin/x32 ; cp -f " .. copybase .. "/lib/dynamic/x32/liborx*.so " .. copybase .. "/bin/x32"}

        configuration {"linux", "not *Core*", "x64"}
            postbuildcommands {"mkdir " .. copybase .. "/bin/x64 ; cp -f " .. copybase .. "/lib/dynamic/x64/liborx*.so " .. copybase .. "/bin/x64"}

        configuration {"linux", "not *Core*", "not x32", "not x64"}
            postbuildcommands {"cp -f " .. copybase .. "/lib/dynamic/liborx*.so " .. copybase .. "/bin"}

        configuration {}
    else
        configuration {"linux", "not *Core*"}
            postbuildcommands {"cp -f " .. copybase .. "/lib/dynamic/liborx*.so " .. copybase .. "/bin"}
    end


-- Mac OS X

    configuration {"macosx", "not *Core*"}
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
        prebuildcommands {"if [ ! -e " .. copybase .. "/include/base/orxBuild.h ]; then hg log -l 1 --template \"#define __orxVERSION_BUILD__ {rev}\" > " .. copybase .. "/include/base/orxBuild.h; fi"}

    configuration {"macosx", "*Debug*"}
        linkoptions {"-install_name @executable_path/liborxd.dylib"}

    configuration {"macosx", "*Profile*"}
        linkoptions {"-install_name @executable_path/liborxp.dylib"}

    configuration {"macosx", "*Release*"}
        linkoptions {"-install_name @executable_path/liborx.dylib"}

    if _OPTIONS["split-platforms"] then
        configuration {"macosx", "xcode*", "x32"}
            postbuildcommands {"mkdir " .. copybase .. "/bin/x32 ; cp -f " .. copybase .. "/lib/dynamic/x32/liborx*.dylib " .. copybase .. "/bin/x32"}

        configuration {"macosx", "not xcode*", "not *Core*", "x32"}
            postbuildcommands {"mkdir " .. copybase .. "/bin/x32 ; cp -f " .. copybase .. "/lib/dynamic/x32/liborx*.dylib " .. copybase .. "/bin/x32"}

        configuration {"macosx", "xcode*", "x64"}
            postbuildcommands {"mkdir " .. copybase .. "/bin/x64 ; cp -f " .. copybase .. "/lib/dynamic/x64/liborx*.dylib " .. copybase .. "/bin/x64"}

        configuration {"macosx", "not xcode*", "not *Core*", "x64"}
            postbuildcommands {"mkdir " .. copybase .. "/bin/x64 ; cp -f " .. copybase .. "/lib/dynamic/x64/liborx*.dylib " .. copybase .. "/bin/x64"}

        configuration {"macosx", "xcode*", "not x32", "not x64"}
            postbuildcommands {"cp -f " .. copybase .. "/lib/dynamic/liborx*.dylib " .. copybase .. "/bin"}

        configuration {"macosx", "not xcode*", "not *Core*", "not x32", "not x64"}
            postbuildcommands {"cp -f " .. copybase .. "/lib/dynamic/liborx*.dylib " .. copybase .. "/bin"}

        configuration {}
    else
        configuration {"macosx", "not *Core*"}
            postbuildcommands {"cp -f " .. copybase .. "/lib/dynamic/liborx*.dylib " .. copybase .. "/bin"}
    end


-- Windows

    configuration {"windows"}
        prebuildcommands {"if not exist " .. path.translate(copybase, "\\") .. "\\include\\base\\orxBuild.h (hg log -l 1 --template \"#define __orxVERSION_BUILD__ {rev}\" > " .. path.translate(copybase, "\\") .. "\\include\\base\\orxBuild.h)"}

    configuration {"windows", "not *Core*"}
        links
        {
            "glfw",
            "openal32",
            "winmm",
            "sndfile"
        }

    configuration {"windows", "vs*", "not *Core*"}
        links {"OpenGL32"}

    configuration {"windows", "vs*", "*Debug*"}
        linkoptions {"/NODEFAULTLIB:LIBCMT", "/ignore:4099"}

    configuration {"windows", "vs*"}
        buildoptions {"/wd\"4577\""}

    configuration {"windows", "not *Core*"}
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

    configuration {"windows", "*Core*"}
        libdirs {"../lib/static"}

    configuration {"windows", "*Core*", "*Debug*"}
        links {"imporxd"}
    configuration {"windows", "*Core*", "*Profile*"}
        links {"imporxp"}
    configuration {"windows", "*Core*", "*Release*"}
        links {"imporx"}
