cmake_minimum_required(VERSION 3.10)

project(week01)

set(CMAKE_CXX_STANDARD 17)

add_executable(week01 main.cpp)

target_include_directories(week01 PRIVATE
    ../external/glad/include
)

target_sources(week01 PRIVATE
    ../external/glad/src/gl.c
)

target_link_libraries(week01
    glfw
    GL
    dl
)
