cmake_minimum_required(VERSION 3.10.0)
project(Lumina VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Lumina/Utils/D3D12MA")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Lumina/Utils/DXCompiler")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Lumina/Utils/Logger")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Lumina/Utils/FrameTimer")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Lumina/Utils/ImGUI")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Lumina/Utils/Assimp")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Lumina/Platform")

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Lumina/Renderer")

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Lumina/Engine")

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Lumina/Samples")


