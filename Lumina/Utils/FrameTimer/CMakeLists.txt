cmake_minimum_required(VERSION 3.10)
project(FrameTimer)

# Cpp Files
set(TIMER_SOURCES src/FrameTimer.cpp)

# Build Library
add_library(FrameTimer STATIC ${TIMER_SOURCES})

# Interface
target_include_directories(FrameTimer PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)