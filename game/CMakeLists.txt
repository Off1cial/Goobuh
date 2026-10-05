cmake_minimum_required(VERSION 3.20)

project(game LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


file (GLOB_RECURSE SOURCE_ALL
  src/*.c
  src/*.cpp
  src/weapons/*.c
)

add_library(game SHARED
${SOURCE_ALL}
)

target_include_directories(game
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/src/public
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(game PRIVATE cxx_std_23)

set_target_properties(game PROPERTIES
    OUTPUT_NAME "game"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)