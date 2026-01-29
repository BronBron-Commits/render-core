cmake_minimum_required(VERSION 3.16)

project(lumen_social LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)

add_executable(lumen_social
    src/main.cpp
    src/core/app.cpp
    src/render/camera.cpp
)

target_include_directories(lumen_social PRIVATE
    src
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(lumen_social PRIVATE
    SDL2::SDL2
    OpenGL::GL
    GLEW::GLEW
    glm::glm
)

