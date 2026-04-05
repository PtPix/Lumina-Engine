cmake_minimum_required(VERSION 3.10.0)
project(Lumina VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

# Compile ThirdParty
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Lumina/ThirdParty/Assimp")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Lumina/ThirdParty/ImGUI")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Lumina/ThirdParty/DXCompiler")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Lumina/ThirdParty/D3D12MA")

# Compile Core Files
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Lumina/Source/Core")


add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Lumina/Platform")

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Lumina/Renderer")

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Lumina/Engine")

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Lumina/Samples")


