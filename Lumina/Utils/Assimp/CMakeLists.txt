cmake_minimum_required(VERSION 3.10)
project(Assimp LANGUAGES CXX)

add_library(Assimp SHARED IMPORTED GLOBAL)

set_target_properties(Assimp PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
        IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/lib/assimp-vc143-mtd.lib"
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/bin/assimp-vc143-mtd.dll"
)