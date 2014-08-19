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
        "Release"
    }
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

defaultaction ("windows", "vs2012")
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
        "../../../extern/stb_image",
        "../../../extern/freetype-2.4.1/include"
    }

    libdirs
    {
        "../lib"
    }

    configuration {"not macosx"}
        libdirs {"../../../code/lib/static"}

    configuration {"macosx"}
        libdirs {"../../../code/lib/dynamic"}

    configuration {}

    targetdir ("../bin/")

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
            "../../../extern/freetype-2.4.1/lib/linux",
        }

    configuration {"linux", "x64"}
        libdirs
        {
            "../../../extern/freetype-2.4.1/lib/linux64"
        }

    -- This prevents an optimization bug from happening with some versions of gcc on linux
    configuration {"linux", "not *Debug*"}
        buildoptions {"-fschedule-insns"}


-- Mac OS X

    configuration {"macosx"}
        libdirs
        {
            "../../../extern/freetype-2.4.1/lib/mac"
        }
        buildoptions
        {
            "-isysroot /Developer/SDKs/MacOSX10.6.sdk",
            "-mmacosx-version-min=10.6",
            "-gdwarf-2",
            "-Wno-write-strings"
        }
        linkoptions
        {
            "-isysroot /Developer/SDKs/MacOSX10.6.sdk",
            "-mmacosx-version-min=10.6",
            "-dead_strip"
        }
        postbuildcommands {"$(shell [ -f " .. copybase .. "/../../code/lib/dynamic/liborx.dylib ] && cp -f " .. copybase .. "/../../code/lib/dynamic/liborx*.dylib " .. copybase .. "/bin)"}

    configuration {"macosx", "x32"}
        buildoptions
        {
            "-mfix-and-continue"
        }


-- Windows

    configuration {"windows", "vs*", "*Debug*"}
        linkoptions {"/NODEFAULTLIB:LIBCMT"}

    configuration {"vs2008"}
        libdirs
        {
            "../../../extern/freetype-2.4.1/lib/vc2008"
        }

    configuration {"vs2010"}
        libdirs
        {
            "../../../extern/freetype-2.4.1/lib/vc2010"
        }

    configuration {"vs2012"}
        libdirs
        {
            "../../../extern/freetype-2.4.1/lib/vc2012"
        }

    configuration {"windows", "codeblocks or codelite or gmake"}
        libdirs
        {
            "../../../extern/freetype-2.4.1/lib/mingw"
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

    configuration {"macosx"}
        links
        {
            "Foundation.framework",
            "AppKit.framework",
            "OpenGL.framework",
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
