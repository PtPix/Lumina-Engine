cmake_minimum_required(VERSION 3.10)
project(DXC LANGUAGES CXX)

add_library(DXC SHARED IMPORTED GLOBAL)

set_target_properties(DXC PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
        IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/lib/dxcompiler.lib"
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/bin/dxcompiler.dll"
)

target_link_libraries(DXC INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/lib/dxil.lib")