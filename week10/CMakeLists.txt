cmake_minimum_required(VERSION 3.10)
project(Week10_Camera)

set(CMAKE_CXX_STANDARD 17)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(../external/glad/include)

add_executable(Week10 main.cpp ../external/glad/src/gl.c)

target_link_libraries(Week10 PRIVATE glfw OpenGL::GL)