function SetupManifests()
	excludes { "include/wx/msw/*.manifest" }

	local c = configuration { "windows", "vs*", "x32" }
		files { "include/wx/msw/wx.manifest" }

	configuration { "windows", "vs*", "x64" }
		files { "include/wx/msw/amd64.manifest" }

	configuration(c)
end

project "wxWidgets"

	SetupNativeDependencyProject()
	DoNotBuildInNativeClient()
	
	kind "SharedLib"

	includedirs 
	{
		"include/setup",
		"include",
		"src/png",
		"../zlib/include",
	}
	
	deps { "zlib" }
	
	pchheader "wx/wxprec.h"
	pchsource "src/common/dummy.cpp"
	files { "src/common/dummy.cpp" }

	defines
	{
		"_UNICODE",
		"WXBUILDING",
		"WXMAKINGDLL",
		"wxMONOLITHIC=1",
		"wxUSE_BASE=1",
		"wxUSE_GUI=1",
		"wxUSE_AUI=0",
		"wxUSE_PROPGRID=0",
	}

	configuration { "windows" }
		defines { "__WXMSW__" }
		files { "src/msw/glcanvas.cpp" }
		links
		{
			"Comctl32.lib",
			"Comdlg32.lib",	
			"Gdi32.lib",
			"Opengl32.lib",
			"Ole32.lib",
			"Oleaut32.lib",
			"Rpcrt4.lib",
			"uuid.lib",
			"Winspool.lib",
			"Ws2_32.lib",
			"Winmm.lib"
		}

	configuration { "windows", "not vs*", "x32" }
		resoptions { "-Fpe-i386" }

	configuration { "windows", "not vs*", "x64" }
		resoptions { "-Fpe-x86-64" }

	configuration "msvc"
		defines
		{
			"_CRT_SECURE_NO_WARNINGS",
			"_CRT_SECURE_NO_DEPRECATE",
			"_SCL_SECURE_NO_WARNINGS"
		}

	configuration {}
	
	--if cfg.flags.NoRTTI then
		defines { "wxNO_RTTI" }
	--end
	
	--if cfg.flags.NoExceptions then
		defines { "wxNO_EXCEPTIONS" }
	--end
	
	dofile "wx.lua"

	SetupManifests()

	excludes
	{
		"include/wx/arrimpl.cpp",
		"include/wx/listimpl.cpp",
		"include/wx/thrimpl.cpp"
	}
	
	files { "src/common/glcmn.cpp" }
	
	files { "src/png/png*.c", }
	excludes { "src/png/pngtest.c", }
	configuration "src/png/*.c"
		flags { "NoPCH" }
	
	configuration "**/extended.c"
		flags { "NoPCH" }