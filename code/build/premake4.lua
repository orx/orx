----
-- Function definitions
----

--Taken from a dev version of premake.
function linux_is_64_bit()
	local pipe = io.popen("uname -m")
	local contents =  pipe:read('*a')
	pipe:close()

	local t64 = 
	{
		'x86_64'
		,'ia64'
		,'amd64'
		,'powerpc64'
		,'sparc64'
	}
	for _,v in ipairs(t64) do
		if contents:find(v) then return true end
	end
	return false
end

function defaultaction(osName, actionName)
   if os.is(osName) then
      _ACTION = _ACTION or actionName
   end
end

function permute_concat(concat_str, table_of_tables)

	num = table.getn(table_of_tables)

	if num >= 2 then

		local subtable = { }
		for i = 2,num do
			if i ~= 1 then
				table.insert(subtable,table_of_tables[i])
			end
		end

		local permuted_table = permute_concat(concat_str,subtable)
		local result_table = { }

		for i,v in ipairs(table_of_tables[1]) do
			for j,w in ipairs(permuted_table) do
				table.insert(result_table, v .. concat_str .. w)
			end
		end

		return result_table

	elseif num == 1 then

		return table_of_tables[1]

	else

		return { }

	end

end

function concat_tables(table_one, table_two)

	result_table = table_one

	for i,v in ipairs(table_two) do
		table.insert(result_table,v)
	end

	return result_table

end

function make_config_strs(configs, string_table)

	return concat_tables(configs,permute_concat("_",string_table))

end


function gen_linux_configs()

	--This order allows for a sane default build
	local arch_order
	if linux_is_64_bit() then
		arch_order = { "amd64", "x86" }
	else
		arch_order = { "x86", "amd64" }
	end

	config_table = 
		{
			{ "linux" },
			arch_order,
			{ "dynamic", "static" },
			--{ "embedded", "plugin" },
			{ "embedded" },
			{ "release", "profile", "debug" }
		}

	return permute_concat("_",config_table)

end

function gen_windows_configs()

	config_table = 
		{
			{ "windows" },
			{ "x86" },
			{ "dynamic", "static" },
			--{ "embedded", "plugin" },
			{ "embedded" },
			{ "release", "profile", "debug" }
		}

	return permute_concat("_",config_table)

end

function gen_osx_dynamic_configs()

	config_table = 
		{
			{ "macosx" },
			{ "dynamic" },
			--{ "embedded", "plugin" },
			{ "embedded" },
			{ "release", "profile", "debug" },
		}

	return permute_concat("_",config_table)

end

function gen_osx_static_configs()

	config_table = 
		{
			{ "macosx" },
			{ "static" },
			--{ "embedded", "plugin" },
			{ "embedded" },
			{ "release", "profile", "debug" },
		}

	return permute_concat("_",config_table)

end

function gen_ios_configs()

	config_table = 
		{
			{ "ios" },
			{ "static" },
			{ "embedded" },
			{ "release", "profile", "debug" },
		}

	return permute_concat("_",config_table)

end 

function gen_android_configs()

	config_table = 
		{
			{ "android" },
			{ "static" },
			{ "embedded" },
			{ "release", "profile", "debug" },
		}

	return permute_concat("_",config_table)

end

function initconfig()

	if not _OPTIONS["build-type"] then
		_OPTIONS["build-type"] = "dynamic"
	end
	
	if _OPTIONS["build-type"] == "ios" then
		return gen_ios_configs()
	end

	-- if _OPTIONS["build-type"] == "android" then
	-- 	return gen_android_configs()
	-- end

	if _ACTION == "xcode3" then
		if _OPTIONS["build-type"] == "dynamic" then
			return gen_osx_dynamic_configs()
		else
			return gen_osx_static_configs()
		end
	end

	if string.find(_ACTION,'vs') == 1 then
		return gen_windows_configs()
	end

	if _ACTION == "codelite" or _ACTION == "gmake" then
		if os.is("linux") then
			return gen_linux_configs()
		elseif os.is("windows") then
			return gen_windows_configs()
		end
	end

	print("Don't know how to build for this platform!")
	return { }
end

function setup_options()

	newoption {
		trigger     = "build-type",
		value       = "dynamic",
		description = "Selects the solution type to be generated. Dynamic and static are only differntiated for XCode3",
		allowed = {
			{ "dynamic", "Dynamic (run-time linked) library [Default]" },
			{ "static",  "Static (compile-time linked) library" },
			{ "ios",     "iOS solution (static link only, embedded plugins)" },
			--{ "android", "Android solution (static link only, embedded plugins)"}
		}
	}

	--Gen both embedded and plugin configs for now; no need for this option. maybe in future
	-- newoption {
	-- 	trigger     = "plugins",
	-- 	value       = "embedded",
	-- 	description = "Specify weather to embed the plugins in orx, or compile and link them seperately.\n" ..
	-- 	              "This option is set automaticaly set to embedded for iOS and android builds",
	-- 	allowed = {
	-- 		{ "embedded", "Compile plugins into orx [Default]" },
	-- 		{ "external",  "Generate plugins, dynamically load them at runtime." }
	-- 	}
	-- }

end

-----
-- Actual build definitions begin here
-----

defaultaction("windows","vs2010")
defaultaction("linux","gmake")
defaultaction("macosx","xcode3")

build_plugins = { }; -- gen_*_configs will set this variable.

setup_options()

solution "orx"
	PATH = "./" .. _ACTION
	location (PATH)
	configurations {
			initconfig()
			}
	--
	--	Global solution settings
	--

	includedirs	"../include"
	includedirs	"../../extern/dlmalloc"

	configuration "not *android*"
		excludes "../src/main/orxAndroidNativeSupport.c"
		excludes "../src/main/orxAndroidSupport.cpp"


	configuration "*static*"
		targetdir ("../lib/static") 
		kind "StaticLib"
		defines "__orxSTATIC__"

	configuration "*dynamic*"
		targetdir ("../lib/dynamic")
		kind "SharedLib"

	--
	--	Arch specifics
	--
	configuration "linux*x86*"
		linkoptions "-m32"
		buildoptions "-m32"

	configuration "linux*amd64*" 
		linkoptions "-m64"
		buildoptions "-m64"

	--TODO: Mac, iOS, Android

	--
	--	Mode
	--
	configuration "*debug*"
		defines "__orxDEBUG__"
		flags "symbols"

	configuration "*profile*"
		defines "__orxPROFILER__"
		flags {"optimizespeed", "nortti"}

	configuration "*release*"
		flags {"optimizespeed", "nortti"}

	configuration "*embedded*"
		defines "__orxEMBEDDED__"
		defines "AL_LIBTYPE_STATIC"

		includedirs {
			"../../extern/glfw-2.7/include",
			"../../extern/sdl-1.2.14/include",
			"../../extern/Box2D_2.1.3/include",
			"../../extern/SOIL/include",
			"../../extern/openal-soft/include",
			"../../extern/libsndfile-1.0.22/include",
			"../../extern/stb_vorbis" }

	--
	-- Library Paths
	--

	configuration {"linux*x86*"}
		libdirs { "../../extern/glfw-2.7/lib/linux",
			"../../extern/SOIL/lib/linux",
			"../../extern/libsndfile-1.0.22/lib/linux",
			"../../extern/Box2D_2.1.3/lib/linux" }

	configuration {"linux*amd64*"}
		libdirs { "../../extern/glfw-2.7/lib/linux64",
			"../../extern/SOIL/lib/linux64",
			"../../extern/libsndfile-1.0.22/lib/linux64",
			"../../extern/Box2D_2.1.3/lib/linux64" }

	configuration { "vs2005" }
		libdirs { "../../extern/glfw-2.7/lib/vc2005", 
				"../../extern/SDL-1.2.14/lib/vc2005", 
				"../../extern/SOIL/lib/msvs2005", 
				"../../extern/openal-soft/lib/vc2005",
				"../../extern/libsndfile-1.0.22/lib/vc2005",
				"../../extern/Box2D_2.1.3/lib/msvs2005" }

	configuration { "vs2008" }
		libdirs { "../../extern/glfw-2.7/lib/vc2008", 
				"../../extern/SDL-1.2.14/lib/vc2008", 
				"../../extern/SOIL/lib/msvs2008", 
				"../../extern/openal-soft/lib/vc2008",
				"../../extern/libsndfile-1.0.22/lib/vc2008",
				"../../extern/Box2D_2.1.3/lib/msvs2008" }

	configuration { "vs2010" }
		libdirs { "../../extern/glfw-2.7/lib/vc2010", 
				"../../extern/SDL-1.2.14/lib/vc2010", 
				"../../extern/SOIL/lib/msvs2010", 
				"../../extern/openal-soft/lib/vc2010",
				"../../extern/libsndfile-1.0.22/lib/vc2010",
				"../../extern/Box2D_2.1.3/lib/msvs2010" }

	configuration { "codelite", "windows*" }
		libdirs { "../../extern/glfw-2.7/lib/mingw",
			"../../extern/SDL-1.2.14/lib/mingw",
			"../../extern/SOIL/lib/mingw",
			"../../extern/openal-soft/lib/mingw",
			"../../extern/libsndfile-1.0.22/lib/mingw",
			"../../extern/Box2D_2.1.3/lib/mingw" }

	--
	-- On post build, if we built a shared lib, copy it to ./bin
	-- I can't find an easy way to proceduraly get the build name, so 
	-- hardcoding it for now.
	--
	configuration "linux*dynamic*debug"
		postbuildcommands { "cp -f ../../lib/dynamic/liborxd.so ../../bin" }

	configuration "linux*dynamic*profile"
		postbuildcommands { "cp -f ../../lib/dynamic/liborxp.so ../../bin" }

	configuration "linux*dynamic*release"
		postbuildcommands { "cp -f ../../lib/dynamic/liborx.so ../../bin" }


	configuration "windows*dynamic*debug"
		postbuildcommands { "cmd /c copy /Y ..\\..\\lib\\dynamic\\orxd.dll ..\\..\\bin" }

	configuration "windows*dynamic*profile"
		postbuildcommands { "cmd /c copy /Y ..\\..\\lib\\dynamic\\orxp.dll ..\\..\\bin" }

	configuration "windows*dynamic*release"
		postbuildcommands { "cmd /c copy /Y ..\\..\\lib\\dynamic\\orx.dll ..\\..\\bin" }

	-- configuration "macosx*dynamic*debug"
	-- configuration "macosx*dynamic*profile"
	-- configuration "macosx*dynamic*release"


	--
	-- End global solution settings
	--

--
-- Orx Lib Project
--

project "orxLIB"
	targetname "orx"
	language "C++"
	files {"../src/**.cpp", "../src/**.c", "../include/**.h" }
	excludes "../src/main/orxMain.c"
	
	--
	--	Platform specifics
	--
	configuration "linux*"  
		buildoptions "-Wno-write-strings"  
		buildoptions "-fPIC"

	configuration "*debug*"
		targetsuffix ("d")

	configuration "*profile*"
		targetsuffix ("p")

	configuration "*static*" 
		kind "StaticLib"
	
	--
	--	Linking
	--

	configuration "*dynamic*" 
		kind "SharedLib" 
		links {
			"glfw",
			"SOIL"}

	configuration "*dynamic*debug*"
		links "Box2Dd"
	configuration {"*dynamic*", "not *debug*"}
		links "Box2D"

	configuration { "linux*dynamic*" }
		links { 
			"sndfile",
			"openal",
			"GL",
			"X11",
			"Xrandr",
			"dl",
			"m",
			"rt" }

	configuration {"windows*dynamic*"}
		links { 
			"openal32",
			"SDL",
			"winmm"}
			
	configuration {"codelite" , "windows*dynamic*"}
		links { "sndfile" }

	configuration {"vs*" , "windows*dynamic*"}
		links { 
			"OpenGL32",
			"libsndfile" }


	-- configuration {"vs*" , "windows*dynamic*"}
	-- 	links { 
	-- 		"SDL",
	-- 		"GLFW",
	-- 		"OpenAL32",
	-- 		"sndfile",
	-- 		"SOIL",
	-- 		"OpenGL32",
	-- 		"winmm" }

	-- configuration {"vs*" , "windows*dynamic*debug*"}
	-- 	links "box2dd"
	-- configuration {"vs*" , "windows*dynamic*", "not *debug*"}
	-- 	links "box2d"


--
-- End orxLib project
--

--
-- orx project
--

project "orx"
	language "C++"
	files { "../src/main/orxMain.c" }
	links { "orxLIB" }
	targetdir("../bin/")

	kind "WindowedApp"
	flags "WinMain" --causes WinMain to be used for windows builds, main otherwise.

	--
	--	Setup compiler flags, etc...
	--
	
	configuration "linux*"
		buildoptions "-fPIC"
		linkoptions {"-Wl,-rpath ./"," -Wl,--export-dynamic"}
		links "m"
	--
	-- End setup compiler flags, etc...
	--

	--
	-- Static Build Section:
	--

	configuration "*static*"
		links {
			"glfw",
			"SOIL"}

	configuration {"*static*debug*"}
		links "Box2Dd"
	configuration {"*static*", "not *debug*"}
		links "Box2D"

	configuration { "linux*static*" }
		links { 
			"sndfile",
			"openal",
			"GL",
			"X11",
			"Xrandr",
			"dl",}
		buildoptions "-Wno-write-strings"
		linkoptions "-Wl,--no-whole-archive"

	configuration "linux*amd64*static*"
		links { 
			"pthread",
			"rt" }

	configuration {"windows*static*"}
		links { 
			"openal32",
			"SDL",
			"winmm"}

	configuration {"codelite", "windows*static*"}
		links { "sndfile" }

	configuration {"vs*" , "windows*static*"}
		links {
			"libsndfile",
			"OpenGL32"}

	-- configuration {"vs*" , "windows*static*"}
	-- 	links { 
	-- 		"SDL",
	-- 		"GLFW",
	-- 		"OpenAL32",
	-- 		"sndfile",
	-- 		"SOIL",
	-- 		"OpenGL32",
	-- 		"winmm" }

	-- configuration {"vs*" , "windows*static*debug*"}
	-- 	links "box2dd"
	-- configuration {"vs*" , "windows*static*", "not *debug*"}
	-- 	links "box2d"

	--
	-- End Static build section
	--

--
-- End orx project
--

--
-- Bounce project
--

project "Bounce"
	language "C"
	files { "../plugins/Demo/orxBounce.c" }
	links { "orxLIB" }
	kind  "SharedLib"
	targetprefix ""
	targetsuffix ""
	targetdir("../bin/plugins/demo/")
	--
	--	Platform specifics
	--
	configuration "linux*"
		linkoptions {"-Wl,-rpath ./"," -Wl,--export-dynamic"}

	configuration {"codelite", "windows*"}
		linkoptions "-fPIC"

	configuration {"codelite", "windows*static*"}
		linkoptions "-Wl,--enable-auto-import"

--
-- End Bounce project
--

--
-- Scroll project
--
project "Scroll"
	language "C"
	files { "../plugins/Demo/orxScroll.c" }
	links { "orxLIB" }
	kind  "SharedLib"
	targetprefix ""
	targetsuffix ""
	targetdir("../bin/plugins/demo")

	--
	--	Platform specifics
	--
	configuration "linux*"
		linkoptions {"-Wl,-rpath ./"," -Wl,--export-dynamic"}

	configuration {"codelite", "windows*"}
		linkoptions "-fPIC"

	configuration {"codelite", "windows*static*"}
		linkoptions "-Wl,--enable-auto-import"

--
-- End Scroll project
--
