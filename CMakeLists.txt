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

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        7b6aead9fb88b3623e3b3725ebb42670cbe4c579 #3.4
    GIT_PROGRESS   TRUE
    CMAKE_ARGS     "-DGLFW_LIBRARY_TYPE=STATIC -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF -DGLFW_INSTALL=OFF -DGLFW_BUILD_EXAMPLES=OFF"
)
FetchContent_MakeAvailable(glfw)

target_link_libraries(${PROJECT_NAME} glfw)

# build imgui (perhaps a bit messy)
set(IMGUI_SOURCES
    ${PROJECT_SOURCE_DIR}/external/imgui-1.85/imgui.cpp
    ${PROJECT_SOURCE_DIR}/external/imgui-1.85/imgui_demo.cpp
    ${PROJECT_SOURCE_DIR}/external/imgui-1.85/imgui_draw.cpp
    ${PROJECT_SOURCE_DIR}/external/imgui-1.85/imgui_tables.cpp
    ${PROJECT_SOURCE_DIR}/external/imgui-1.85/imgui_widgets.cpp
    ${PROJECT_SOURCE_DIR}/external/imgui-1.85/backends/imgui_impl_glfw.cpp
    ${PROJECT_SOURCE_DIR}/external/imgui-1.85/backends/imgui_impl_opengl3.cpp)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC
    ${PROJECT_SOURCE_DIR}/external/imgui-1.85
    ${PROJECT_SOURCE_DIR}/external/imgui-1.85/backends)
target_link_libraries(imgui PRIVATE glfw)

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
