cmake_minimum_required(VERSION 3.5)
project(MasoGradation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(raylib REQUIRED)

add_executable(MasoGradation src/blender.cpp)
target_link_libraries(MasoGradation raylib)
