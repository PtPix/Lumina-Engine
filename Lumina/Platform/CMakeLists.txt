cmake_minimum_required(VERSION 3.10)
project(Platform)

# Cpp Files
set(PLATFORM_SOURCES src/Window.cpp)

# Build Library
add_library(Platform STATIC ${PLATFORM_SOURCES})

# Interface
target_include_directories(Platform PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Dependency
target_link_libraries(Platform PRIVATE Renderer)
target_link_libraries(Platform PRIVATE Logger)