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
        libdirs {"$(ORX)/lib/dynamic"}

    configuration {"xcode*"}
        includedirs {"[code-path]/include"}
        libdirs {"[code-path]/lib/dynamic"}

    configuration {"not vs2015", "not vs2017", "not vs2019"}
        flags {"EnableSSE2"}

    configuration {"not x64"}
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


-- Linux

    configuration {"linux"}
        buildoptions {"-Wno-unused-function"}
        linkoptions {"-Wl,-rpath ./", "-Wl,--export-dynamic"}
        links
        {
            "dl",
            "m",
            "rt"
        }

    -- This prevents an optimization bug from happening with some versions of gcc on linux
    configuration {"linux", "not *Debug*"}
        buildoptions {"-fschedule-insns"}


-- Mac OS X

    configuration {"macosx"}
        buildoptions
        {
            "-mmacosx-version-min=10.9",
            "-stdlib=libc++",
            "-gdwarf-2",
            "-Wno-write-strings"
        }
        linkoptions
        {
            "-mmacosx-version-min=10.9",
            "-stdlib=libc++",
            "-dead_strip"
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

    configuration {"macosx", "x32"}
        buildoptions
        {
            "-mfix-and-continue"
        }


-- Windows

    configuration {"windows", "vs*"}
        buildoptions
        {
            "/MP",
            "/EHsc"
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


-- Linux

    configuration {"linux"}
        postbuildcommands {"cp -f $(ORX)/lib/dynamic/liborx*.so " .. copybase .. "/bin"}


-- Mac OS X

    configuration {"macosx", "xcode*"}
        postbuildcommands {"cp -f [code-path]/lib/dynamic/liborx*.dylib " .. copybase .. "/bin"}

    configuration {"macosx", "not xcode*"}
        postbuildcommands {"cp -f $(ORX)/lib/dynamic/liborx*.dylib " .. copybase .. "/bin"}


-- Windows

    configuration {"windows"}
        postbuildcommands {"cmd /c copy /Y $(ORX)\\lib\\dynamic\\orx*.dll " .. path.translate(copybase, "\\") .. "\\bin"}
