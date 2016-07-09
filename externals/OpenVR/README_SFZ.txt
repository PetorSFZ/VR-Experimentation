# OpenVR for CMake

Trimmed down version of the OpenVR sdk (https://github.com/ValveSoftware/openvr), modified so it can easily be linked with CMake.

# Usage

Place this entire directory in a subdirectory of your CMake project, then add it with `add_subdirectory()`. Three CMake variables are returned:

`${OPENVR_INCLUDE_DIRS}`: The headers you need to include with `include_directories()`

`${OPENVR_LIBRARIES}`: The libraries you need to link your binary to with `target_link_libraries()`

`${OPENVR_DLL_PATH}`: The path to the bundled .dll. Useful so you can copy it to your build directory.