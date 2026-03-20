cmake_minimum_required(VERSION 3.10)
project(Engine)

# Cpp Files
set(ENGINE_SOURCES
        src/LuminaEngineMain.cpp
        src/LuminaEngineWindowEvents.cpp
        src/Camera.cpp
        src/LuminaApp.cpp
        src/Input.cpp
)

set(ASSETS_SOURCES
        src/ModelLoader.cpp
        src/StaticModel.cpp
)

# Build Library
add_library(Engine STATIC ${ENGINE_SOURCES} ${ASSETS_SOURCES})

# Interface
target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Dependency
target_link_libraries(Engine PRIVATE ImGUI)
target_link_libraries(Engine PUBLIC Renderer)
target_link_libraries(Engine PUBLIC FrameTimer)
target_link_libraries(Engine PRIVATE Logger)
target_link_libraries(Engine PRIVATE Assimp)

target_link_libraries(Engine PUBLIC Platform)

#target_include_directories(Engine PUBLIC
#        $<TARGET_PROPERTY:Platform,INTERFACE_INCLUDE_DIRECTORIES>
#)

target_link_libraries(Engine PRIVATE
        d3d12.lib
        dxgi.lib
        dxguid.lib
        d3dcompiler.lib
)

target_link_libraries(Engine PRIVATE
        user32.lib
        kernel32.lib
        winmm.lib
)