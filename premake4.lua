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
			"Rpcrt4.lib",
			"Ws2_32.lib",
			"Winmm.lib"
		}

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