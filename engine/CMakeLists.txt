cmake_minimum_required(VERSION 3.20)

project(engine
    VERSION 0.1
    LANGUAGES C CXX ASM_NASM
)

if(WIN32)
    set(CMAKE_ASM_NASM_OBJECT_FORMAT win64)
else()
    set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
endif()

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Unix-only definitions
if(UNIX AND NOT APPLE)
    add_compile_definitions(_POSIX_C_SOURCE=200809L)
    add_compile_definitions(_GNU_SOURCE)
endif()

# Dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL3 REQUIRED IMPORTED_TARGET sdl3)


if(WIN32)
    set(Vulkan_INCLUDE_DIR
        "C:/msys64/ucrt64/include"
        CACHE PATH ""
    )

    set(Vulkan_LIBRARY
        "C:/msys64/ucrt64/lib/libvulkan-1.dll.a"
        CACHE FILEPATH ""
    )

    set(Vulkan_GLSLC_EXECUTABLE
        "C:/msys64/ucrt64/bin/glslc.exe"
        CACHE FILEPATH ""
    )
endif()

find_package(Vulkan REQUIRED)



find_package(Vulkan REQUIRED)

find_package(Ktx CONFIG REQUIRED)

# Sources
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    src/*.c
    src/*.cpp
	src/*.asm
)

file(GLOB_RECURSE SOURCE_FASTGLTF
    ${CMAKE_SOURCE_DIR}/include/fastgltf/fastgltfsrc/*.cpp
)

add_library(fastgltf STATIC
    ${SOURCE_FASTGLTF}
)

target_include_directories(fastgltf SYSTEM PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

list(FILTER ENGINE_SOURCES EXCLUDE REGEX ".*/main\\.c$")

# Engine library
add_library(engine_lib STATIC
    ${ENGINE_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/include/volk/volk.c
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tinyobj/tiny_obj_loader.cpp
)

# Platform
if(WIN32)
    set(PLATFORM_DEFINE WINDOWS)
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_DEFINE LINUX)
endif()

target_compile_definitions(engine_lib PUBLIC
    ${PLATFORM_DEFINE}
    ASSET_DIR="${CMAKE_SOURCE_DIR}/resource"
	LIB_DIR="${CMAKE_SOURCE_DIR}/lib"
)

target_link_libraries(engine_lib
    PUBLIC
        PkgConfig::SDL3
        Vulkan::Vulkan
        fastgltf
        simdjson
        ktx
)
if(WIN32)
    target_link_libraries(engine_lib PUBLIC ws2_32)
endif()


target_include_directories(engine_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/public
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Executable
add_executable(engine
    src/main.cpp
)

if(WIN32)
    set_target_properties(engine PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()

if(WIN32)
    target_link_options(engine PRIVATE
        -mconsole
    )
endif()

target_compile_definitions(engine PUBLIC
    ASSET_DIR="${CMAKE_SOURCE_DIR}/resource"
	LIB_DIR="${CMAKE_SOURCE_DIR}/lib"
)

target_link_libraries(engine
    PRIVATE
        engine_lib
)

target_include_directories(engine
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/public
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Warnings
target_compile_options(engine_lib PRIVATE
    $<$<COMPILE_LANGUAGE:C,CXX>:-Wall;-Wextra>
)

target_compile_options(engine PRIVATE
    -Wall
    -Wextra
)

# Debug
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(engine_lib PRIVATE
        $<$<COMPILE_LANGUAGE:C,CXX>:-g;-O0;-fno-omit-frame-pointer -Wno-unused-variable>
    )

    target_compile_options(engine PRIVATE
        -g
        -O0
        -fno-omit-frame-pointer
		-Wno-unused-variable
    )

    # Sanitizers on Linux only for now
    if(NOT WIN32)
        target_compile_options(engine_lib PRIVATE
        $<$<COMPILE_LANGUAGE:C,CXX>:-fsanitize=address,undefined>
        )

        target_compile_options(engine PRIVATE
        -fsanitize=address,undefined
        )

        target_link_options(engine PRIVATE
            -fsanitize=address,undefined
        )
    endif()
endif()

# Release
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(engine_lib PRIVATE
        $<$<COMPILE_LANGUAGE:C,CXX>:-O3;-march=native;-flto>
    )

    target_compile_options(engine PRIVATE
        -O3
        -march=native
        -flto
    )

    target_link_options(engine PRIVATE
        -flto
    )
endif()


# Tools
file(GLOB_RECURSE MAPREADER_SOURCES CONFIGURE_DEPENDS
    tools/MapReader/*.cpp
)

add_executable(map_reader ${MAPREADER_SOURCES})

target_include_directories(map_reader PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/MapReader
)

target_compile_options(map_reader PRIVATE -Wall -Wextra)

if(WIN32)
    set_target_properties(map_reader PROPERTIES WIN32_EXECUTABLE FALSE)
    target_link_options(map_reader PRIVATE -mconsole -static)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(map_reader PRIVATE -g -O0 -fno-omit-frame-pointer)
    if(NOT WIN32)
        target_compile_options(map_reader PRIVATE -fsanitize=address,undefined)
        target_link_options(map_reader PRIVATE -fsanitize=address,undefined)
    endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(map_reader PRIVATE -O3 -march=native -flto)
    target_link_options(map_reader PRIVATE -flto)
endif()

set_target_properties(map_reader PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tools/MapReader
)

# Convert the test map when building the "levels" target
add_custom_command(
    OUTPUT  ${CMAKE_SOURCE_DIR}/resource/levels/test.glb
            ${CMAKE_SOURCE_DIR}/resource/levels/test.json
    COMMAND map_reader ${CMAKE_SOURCE_DIR}/resource/maps/test.map
                       ${CMAKE_SOURCE_DIR}/resource/levels
                       ${CMAKE_SOURCE_DIR}/resource/textures
    DEPENDS map_reader ${CMAKE_SOURCE_DIR}/resource/maps/test.map
)

add_custom_target(levels ALL
    DEPENDS ${CMAKE_SOURCE_DIR}/resource/levels/test.glb
)
