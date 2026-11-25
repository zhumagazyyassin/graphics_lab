cmake_minimum_required(VERSION 3.10)
project(Week12)

if(POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

add_executable(Week12
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../external/glad/src/gl.c"
)

target_include_directories(Week12 PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../external/glad/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../external/stb"
)

target_compile_definitions(Week12 PRIVATE
    ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../assets"
)

target_link_libraries(Week12 PRIVATE
    glfw
    OpenGL::GL
    glm::glm
)