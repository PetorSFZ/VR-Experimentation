# Check if 32 or 64bit compiler
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(BITS_64 TRUE)
	set(BITS_32 FALSE)
else()
	set(BITS_64 FALSE)
	set(BITS_32 TRUE)
endif()

# Find precompiled library and path to dll
if(WIN32)
	if (BITS_64)
		find_library(OpenVRLibs openvr_api
			HINTS
			${CMAKE_CURRENT_SOURCE_DIR}/lib/win64)
		set(DLLPath ${CMAKE_CURRENT_SOURCE_DIR}/bin/win64/openvr_api.dll)
	elseif(BITS_32)
		find_library(OpenVRLibs openvr_api
			HINTS
			${CMAKE_CURRENT_SOURCE_DIR}/lib/win32)
		set(DLLPath ${CMAKE_CURRENT_SOURCE_DIR}/bin/win32/openvr_api.dll)
	endif()
elseif(LINUX)
	if (BITS_64)
		find_library(OpenVRLibs libopenvr_api
			HINTS
			${CMAKE_CURRENT_SOURCE_DIR}/lib/linux64)
		set(DLLPath ${CMAKE_CURRENT_SOURCE_DIR}/bin/linux64/libopenvr_api.so)
	elseif(BITS_32)
		find_library(OpenVRLibs libopenvr_api
			HINTS
			${CMAKE_CURRENT_SOURCE_DIR}/lib/linux32)
		set(DLLPath ${CMAKE_CURRENT_SOURCE_DIR}/bin/linux32/libopenvr_api.so)
	endif()
else()
	message(FATAL_ERROR "Unsupported platform for OpenVR")
endif()

# Return variables
set(OPENVR_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/headers PARENT_SCOPE)
set(OPENVR_LIBRARIES ${OpenVRLibs} PARENT_SCOPE)
set(OPENVR_DLL_PATH ${DLLPath} PARENT_SCOPE)
