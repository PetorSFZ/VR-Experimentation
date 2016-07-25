cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project("VR Experimentation")

# Sets build type to release if no build type is specified in a single-configuration generator.
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
if(MSVC)
	# MSVC flags
	set(CMAKE_CXX_FLAGS "/Wall /Zi /EHsc /wd4201 /wd4350 /wd4514 /wd4571 /wd4647 /wd4668 /wd4710 /wd4711 /wd4820 /D_CRT_SECURE_NO_WARNINGS")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O2 /fp:fast /DEBUG /DSFZ_NO_DEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "/O2 /fp:fast /DSFZ_NO_DEBUG")
	set(CMAKE_CXX_FLAGS_DEBUG "/Od /DEBUG")
else()
	# GCC / Clang flags
	set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-c++11-extensions -std=c++11 -pthread")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -ffast-math -g -DSFZ_NO_DEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math -DSFZ_NO_DEBUG")
	set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
endif()

# Directories
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(EXTERNALS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/externals)

# sfzCore
set(SFZ_CORE_OPENGL TRUE)
add_subdirectory(${EXTERNALS_DIR}/sfzCore)

# OpenVR (${OPENVR_INCLUDE_DIRS}, ${OPENVR_LIBRARIES}, ${OPENVR_DLL_PATH})
add_subdirectory(${EXTERNALS_DIR}/OpenVR)

# tinyobjloader
add_subdirectory(${EXTERNALS_DIR}/tinyobjloader)

# Include directories
include_directories(
	${SOURCE_DIR}
	${SFZ_CORE_INCLUDE_DIRS}
	${OPENVR_INCLUDE_DIRS}
	${TINYOBJLOADER_INCLUDE_DIRS}
)

set(SOURCE_ROOT_FILES
	${SOURCE_DIR}/GameScreen.hpp
	${SOURCE_DIR}/GameScreen.cpp
	${SOURCE_DIR}/Main.cpp
	${SOURCE_DIR}/SimpleModel.hpp
	${SOURCE_DIR}/SimpleModel.cpp
	${SOURCE_DIR}/VR.hpp
	${SOURCE_DIR}/VR.cpp)
source_group(root FILES ${SOURCE_ROOT_FILES})

set(SOURCE_ALL_FILES
	${SOURCE_ROOT_FILES})

# Main executable
add_executable(VRExperimentation ${SOURCE_ALL_FILES})

# Linking libraries to main executable
target_link_libraries(
	VRExperimentation

	${SFZ_CORE_LIBRARIES}
	${OPENVR_LIBRARIES}
	${TINYOBJLOADER_LIBRARIES}
)

# MSVC specific file copying
if(MSVC)
	# Create assets symlinks batch file
	file(COPY RuntimeDLLs/CREATE_ASSETS_SYMLINKS.BAT DESTINATION ${CMAKE_BINARY_DIR})

	# Runtime DLLs
	file(COPY RuntimeDLLs/SDL2.dll DESTINATION ${CMAKE_BINARY_DIR}/Debug)
	file(COPY RuntimeDLLs/SDL2_mixer.dll DESTINATION ${CMAKE_BINARY_DIR}/Debug)
	file(COPY ${OPENVR_DLL_PATH} DESTINATION ${CMAKE_BINARY_DIR}/Debug)

	file(COPY RuntimeDLLs/SDL2.dll DESTINATION ${CMAKE_BINARY_DIR}/Release)
	file(COPY RuntimeDLLs/SDL2_mixer.dll DESTINATION ${CMAKE_BINARY_DIR}/Release)
	file(COPY ${OPENVR_DLL_PATH} DESTINATION ${CMAKE_BINARY_DIR}/Release)

else()
	# Create assets symlinks bash script (for non windows platforms)
	file(COPY RuntimeDLLs/CREATE_ASSETS_SYMLINKS.SH DESTINATION ${CMAKE_BINARY_DIR})
endif()
