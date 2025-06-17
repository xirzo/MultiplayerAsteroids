cmake_minimum_required(VERSION 3.24.5)

project(asteroids LANGUAGES C)

add_compile_options(-Wunused-function -Wall -Wextra)

find_package(logger REQUIRED)

add_subdirectory(bin)
add_subdirectory(src)
