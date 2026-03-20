cmake_minimum_required(VERSION 3.10)
project(ImGUI)

# Cpp Files
set(ImGUI_SOURCES
        src/imgui.cpp
        src/imgui_impl_dx12.cpp
        src/imgui_demo.cpp
        src/imgui_draw.cpp
        src/imgui_impl_win32.cpp
        src/imgui_tables.cpp
        src/imgui_widgets.cpp
)

# Build Library
add_library(ImGUI STATIC ${ImGUI_SOURCES})

# Interface
target_include_directories(ImGUI PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)