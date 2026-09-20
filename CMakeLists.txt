cmake_minimum_required(VERSION 3.16)

project(cg LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(cg
    main.cpp
    external/glad/src/gl.c
)

target_include_directories(cg PRIVATE
    external/glad/include
)

target_link_libraries(cg PRIVATE
    glfw
    OpenGL::GL
    dl
)
