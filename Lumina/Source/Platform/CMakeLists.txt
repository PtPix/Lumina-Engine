cmake_minimum_required(VERSION 3.10)
project(Platform)

# Cpp Files
set(PLATFORM_SOURCES src/Window.cpp)

# Build Library
add_library(Platform STATIC ${PLATFORM_SOURCES})

# Interface
target_include_directories(Platform PUBLIC include)

# Dependency
target_link_libraries(Platform PRIVATE Renderer)
target_link_libraries(Platform PRIVATE LuminaCore)