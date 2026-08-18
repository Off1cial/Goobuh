cmake_minimum_required(VERSION 3.10)

project(Goobuh VERSION 0.1 LANGUAGES CXX C ASM_NASM)
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

add_compile_definitions(_POSIX_C_SOURCE=200112L)

find_package(PkgConfig REQUIRED)

pkg_check_modules(SDL3 REQUIRED sdl3)
pkg_check_modules(SDL3IMAGE REQUIRED sdl3-image)
pkg_check_modules(SDL3TTF REQUIRED sdl3-ttf)

# Find Vulkan
find_package(Vulkan REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -------------------------
# Source files
# -------------------------

file(GLOB_RECURSE SOURCE_ENGINE
  src/*.c
  src/*.cpp
)

# -------------------------
# Engine executable
# -------------------------

add_executable(engine
    ${SOURCE_ENGINE}
)

target_compile_definitions(engine PRIVATE
    ASSET_DIR="${CMAKE_SOURCE_DIR}/Assets"
)

# -------------------------
# Include paths
# -------------------------

target_include_directories(engine PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${SDL3_INCLUDE_DIRS}
    ${SDL3IMAGE_INCLUDE_DIRS}
    ${SDL3TTF_INCLUDE_DIRS}
    ${Vulkan_INCLUDE_DIRS}
)

# -------------------------
# Libraries
# -------------------------

target_link_directories(engine PRIVATE
    ${SDL3_LIBRARY_DIRS}
    ${SDL3IMAGE_LIBRARY_DIRS}
    ${SDL3TTF_LIBRARY_DIRS}
)

target_link_libraries(engine PRIVATE
    ${SDL3_LIBRARIES}
    ${SDL3IMAGE_LIBRARIES}
    ${SDL3TTF_LIBRARIES}
    ${Vulkan_LIBRARIES}
)

if(WIN32)
    target_link_libraries(engine PRIVATE
        ${Vulkan_LIBRARIES}
    )
elseif(UNIX)
    target_link_libraries(engine PRIVATE
        ${Vulkan_LIBRARIES}
        dl
        pthread
        m
    )
endif()

# -------------------------
# Compiler settings
# -------------------------

set_target_properties(engine PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS NO
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# -------------------------
# Address sanitizer (Debug only)
# -------------------------

if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT WIN32)
    target_compile_options(engine PRIVATE
        -fsanitize=address
        -fno-omit-frame-pointer
        -g
    )
    target_link_options(engine PRIVATE
        -fsanitize=address
    )
endif()

# -------------------------
# Debug output
# -------------------------

message(STATUS "SDL3 Include Dir: ${SDL3_INCLUDE_DIRS}")
message(STATUS "SDL3 Libraries: ${SDL3_LIBRARIES}")
message(STATUS "SDL3_image Include Dir: ${SDL3IMAGE_INCLUDE_DIRS}")
message(STATUS "SDL3_image Libraries: ${SDL3IMAGE_LIBRARIES}")
message(STATUS "SDL3_ttf Include Dir: ${SDL3TTF_INCLUDE_DIRS}")
message(STATUS "SDL3_ttf Libraries: ${SDL3TTF_LIBRARIES}")
message(STATUS "Vulkan Include Dir: ${Vulkan_INCLUDE_DIRS}")
message(STATUS "Vulkan Libraries: ${Vulkan_LIBRARIES}")