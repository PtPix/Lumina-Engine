cmake_minimum_required(VERSION 3.10)
project(Logger)

# Cpp Files
set(LOGGER_SOURCES src/Logger.cpp)

# Build Library
add_library(Logger STATIC ${LOGGER_SOURCES})

# Interface
target_include_directories(Logger PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)