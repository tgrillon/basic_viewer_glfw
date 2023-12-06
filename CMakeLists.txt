# Created by the script cgal_create_cmake_script
# This is the CMake script for compiling a CGAL application.

cmake_minimum_required(VERSION 3.1...3.23)
project(OpenGL)

find_package(CGAL REQUIRED COMPONENTS Core)

add_subdirectory (vendor/glfw)

include_directories (
    .
    vendor/glfw/include/
    vendor/glad/include/
    ${CGAL_DIR}/Basic_viewer/include/ # TEMPO
)


file (GLOB         VENDORS_SOURCES vendor/glad/src/glad.c)

file (GLOB_RECURSE PROJECT_HEADERS *.hpp
                                   *.h)
file (GLOB_RECURSE PROJECT_SOURCES *.cpp
                                   *.cxx
                                   *.cc
                                   *.c)

add_executable (${PROJECT_NAME} ${VENDORS_SOURCES} ${PROJET_SOURCES} ${PROJET_HEADERS} "main.cpp" "program.hpp" "program.cpp")

target_link_libraries(${PROJECT_NAME} glfw CGAL::CGAL)
add_definitions (-DGLFW_INCLUDE_NONE
                 -DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\")

