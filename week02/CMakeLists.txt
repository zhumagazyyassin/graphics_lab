cmake_minimum_required(VERSION 3.10)

project(week02)

set(CMAKE_CXX_STANDARD 17)

add_executable(week02 main.cpp)

target_include_directories(week02 PRIVATE
    ../external/glad/include
)

target_sources(week02 PRIVATE
    ../external/glad/src/gl.c
)

target_link_libraries(week02
    glfw
    GL
    dl
)
