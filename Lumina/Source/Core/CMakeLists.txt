cmake_minimum_required(VERSION 3.10)
project(LuminaCore)

# Cpp Files
set(CORE_SOURCES
        src/Profiler.cpp
        src/Logger.cpp
)

# Build Library
add_library(LuminaCore STATIC ${CORE_SOURCES})

# Interface
target_include_directories(LuminaCore PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)