add_library(nmutillib STATIC gl.cpp
        camera.h defs.h gl.h io.h log.h math.h matrix.h vector.h intersect.h
        util.h)

target_include_directories(nmutillib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

# link OpenGL and glad
# todo: this is kind of messy as the executable will also have to link this,
# todo:  and versions need to be coordinated
find_package(OpenGL REQUIRED)

add_library(nmglad STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../external/glad/src/gl.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../external/glad/src/wgl.c)
target_include_directories(nmglad PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../external/glad/include)

target_link_libraries(nmutillib OpenGL::GL OpenGL::GLU nmglad)