set(EXECUTABLE_NAME Lumina)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

set(MAIN_SOURCES main.cpp src/ModelApp.cpp)

add_executable(${EXECUTABLE_NAME} WIN32 ${MAIN_SOURCES})

set(SHADER_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../Shaders")

add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${SHADER_SRC_DIR}"
        "$<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/Shaders"
)

add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_PROPERTY:Assimp,IMPORTED_LOCATION>
        $<TARGET_FILE_DIR:${EXECUTABLE_NAME}>

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/Assimp/bin/assimp-vc143-mtd.pdb"
        $<TARGET_FILE_DIR:${EXECUTABLE_NAME}>

        COMMENT "Auto-copying Assimp DLL and PDB to output directory..."
)

add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_PROPERTY:DXC,IMPORTED_LOCATION>
        $<TARGET_FILE_DIR:${EXECUTABLE_NAME}>

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/DXCompiler/bin/dxil.dll"
        $<TARGET_FILE_DIR:${EXECUTABLE_NAME}>

        COMMENT "Auto-copying DXC DLLs (dxcompiler & dxil) to output directory..."
)

target_link_libraries(${EXECUTABLE_NAME} PRIVATE LuminaCore)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE ImGUI)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE Renderer)

target_link_libraries(${EXECUTABLE_NAME} PRIVATE Engine)

# Interface
target_include_directories(${EXECUTABLE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${EXECUTABLE_NAME} PRIVATE
        d3d12.lib
        dxgi.lib
        dxguid.lib
        d3dcompiler.lib
)

target_link_libraries(${EXECUTABLE_NAME} PRIVATE
        user32.lib
        kernel32.lib
        winmm.lib
)