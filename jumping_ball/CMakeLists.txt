cmake_minimum_required(VERSION 3.16)

project(jumping_ball LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(jumping_ball
    main.cpp
    ../external/glad/src/gl.c
)

target_include_directories(jumping_ball PRIVATE
    ../external/glad/include
)

target_link_libraries(jumping_ball PRIVATE
    glfw
    OpenGL::GL
)