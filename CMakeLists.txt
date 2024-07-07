# 3.11 FetchContent
cmake_minimum_required(VERSION 3.26)
project(terrain3)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

find_package(OpenGL REQUIRED)

add_executable(${PROJECT_NAME}
    src/app.cpp 
    src/axis.cpp
    src/geometry.cpp
    src/gui.cpp 
    src/heightmap.cpp 
    src/log.cpp
    src/main.cpp 
    src/mesh.cpp 
    src/stb_wrapper.cpp
    src/terrain.cpp 
    src/window.cpp) 

add_subdirectory(nmutil)
target_link_libraries(${PROJECT_NAME} nmutillib)

# glad
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG        658f48e72aee3c6582e80b05ac0f8787a64fe6bb #v2.0.6
    GIT_PROGRESS   TRUE
    SOURCE_SUBDIR  cmake
)
FetchContent_MakeAvailable(glad)
glad_add_library(glad_gl_core_43 STATIC REPRODUCIBLE LOADER API gl:core=4.3)
target_link_libraries(${PROJECT_NAME} glad_gl_core_43)

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        7b6aead9fb88b3623e3b3725ebb42670cbe4c579 #3.4
    GIT_PROGRESS   TRUE
    CMAKE_ARGS     "-DGLFW_LIBRARY_TYPE=STATIC -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF -DGLFW_INSTALL=OFF -DGLFW_BUILD_EXAMPLES=OFF"
)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
set(GLFW_LIBRARY_TYPE "STATIC")
FetchContent_MakeAvailable(glfw)
target_link_libraries(${PROJECT_NAME} glfw)

# imgui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        cb16be3a3fc1f9cd146ae24d52b615f8a05fa93d #v1.90.9
    GIT_PROGRESS   TRUE
)
FetchContent_Populate(imgui)
# https://github.com/ocornut/imgui/wiki/Getting-Started#compilinglinking
add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp)
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}/
    ${imgui_SOURCE_DIR}/backends)
target_link_libraries(imgui PRIVATE glfw)
target_link_libraries(${PROJECT_NAME} imgui)

# stb
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG        013ac3beddff3dbffafd5177e7972067cd2b5083 # master@2024-07-08
    GIT_PROGRESS   TRUE
)
FetchContent_Populate(stb)
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} stb)

# copy resource directory to binary directory
set(RESOURCE_DIR "${PROJECT_BINARY_DIR}/res")
add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E copy_directory_if_different "${PROJECT_SOURCE_DIR}/res" ${RESOURCE_DIR}
    COMMENT "Copying resources")
target_compile_definitions(${PROJECT_NAME} PRIVATE RESOURCE_DIR="${RESOURCE_DIR}")
