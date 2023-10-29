cmake_minimum_required(VERSION 3.26)
project(terrain3)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)

# WIN32 to build an executable with a WinMain entry point on windows
add_executable(${PROJECT_NAME} WIN32
        src/main.cpp src/app.cpp src/terrain.cpp src/noise.h src/geometry.cpp
        src/heightmap.cpp src/mesh.cpp src/win.cpp src/gui.cpp src/axis.cpp
        src/stb_wrapper.cpp)

add_subdirectory(nmutil)

add_library(glad STATIC
        ${PROJECT_SOURCE_DIR}/external/glad/src/gl.c
        ${PROJECT_SOURCE_DIR}/external/glad/src/wgl.c)
target_include_directories(glad PUBLIC
        ${PROJECT_SOURCE_DIR}/external/glad/include)

target_link_libraries(${PROJECT_NAME} OpenGL::GL OpenGL::GLU glad nmutillib)

# build imgui (perhaps a bit messy)
set(IMGUI_SOURCES
        ${PROJECT_SOURCE_DIR}/external/imgui-1.85/imgui.cpp
        ${PROJECT_SOURCE_DIR}/external/imgui-1.85/imgui_demo.cpp
        ${PROJECT_SOURCE_DIR}/external/imgui-1.85/imgui_draw.cpp
        ${PROJECT_SOURCE_DIR}/external/imgui-1.85/imgui_tables.cpp
        ${PROJECT_SOURCE_DIR}/external/imgui-1.85/imgui_widgets.cpp
        ${PROJECT_SOURCE_DIR}/external/imgui-1.85/backends/imgui_impl_win32.cpp
        ${PROJECT_SOURCE_DIR}/external/imgui-1.85/backends/imgui_impl_opengl3.cpp)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC
        ${PROJECT_SOURCE_DIR}/external/imgui-1.85
        ${PROJECT_SOURCE_DIR}/external/imgui-1.85/backends)

# link imgui
target_link_libraries(${PROJECT_NAME} imgui)

# copy resource directory to binary directory
set(RESOURCE_DIR "${PROJECT_BINARY_DIR}/res")
add_custom_command(
  TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND}
  ARGS -E copy_directory_if_different "${PROJECT_SOURCE_DIR}/res" ${RESOURCE_DIR}
  COMMENT "Copying resources")
target_compile_definitions(${PROJECT_NAME} PRIVATE RESOURCE_DIR="${RESOURCE_DIR}")