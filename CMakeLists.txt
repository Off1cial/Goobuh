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

find_package(Vulkan REQUIRED)
find_package(Ktx CONFIG REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CheckCXXCompilerFlag)

# -------------------------
# Source files
# -------------------------

file(GLOB_RECURSE SOURCE_CORE
    src/core/*.c
    src/core/*.cpp
)

file(GLOB_RECURSE SOURCE_MATH
    src/math/*.asm
    src/math/*.c
    src/math/*.cpp
)

file(GLOB_RECURSE SOURCE_PLATFORM
    src/platform/*.c
    src/platform/*.cpp
)

file(GLOB_RECURSE SOURCE_RENDERER
    src/renderer/*.c
    src/renderer/*.cpp
    include/volk/volk.c
    include/tinyobj/tiny_obj_loader.cpp
)

file(GLOB_RECURSE SOURCE_ENGINE_SHARED
    src/common/*.c
    src/common/*.cpp
    src/engine/shared/*.c
    src/engine/shared/*.cpp
)

file(GLOB_RECURSE SOURCE_ENGINE
    src/engine/*.c
    src/engine/*.cpp
    src/game/*.cpp
)

# -------------------------
# fastgltf
# -------------------------

file(GLOB_RECURSE SOURCE_FASTGLTF
    include/fastgltf/fastgltfsrc/*.cpp
)

add_library(fastgltf STATIC
    ${SOURCE_FASTGLTF}
)

target_include_directories(fastgltf SYSTEM PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/JoltPhysics
)

target_compile_options(fastgltf PRIVATE
    -w
)

# -------------------------
# Goobuh engine library
# -------------------------

add_library(goobuh STATIC
    ${SOURCE_ENGINE}
    ${SOURCE_ENGINE_SHARED}
    ${SOURCE_PLATFORM}
    ${SOURCE_RENDERER}
    ${SOURCE_CORE}
    ${SOURCE_MATH}
)

target_compile_definitions(goobuh PRIVATE
    ASSET_DIR="${CMAKE_SOURCE_DIR}/Assets"
)

# -------------------------
# Engine executable
# -------------------------

add_executable(engine
    src/main.c
)

target_link_libraries(engine PRIVATE
    goobuh
)

# -------------------------
# Include paths
# -------------------------

target_include_directories(goobuh SYSTEM PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${SDL3_INCLUDE_DIRS}
    ${SDL3IMAGE_INCLUDE_DIRS}
    ${SDL3TTF_INCLUDE_DIRS}
    ${Vulkan_INCLUDE_DIRS}
)

target_include_directories(goobuh PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_include_directories(engine PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include

)

# -------------------------
# Libraries
# -------------------------

target_link_directories(goobuh PRIVATE
    ${SDL3_LIBRARY_DIRS}
    ${SDL3IMAGE_LIBRARY_DIRS}
    ${SDL3TTF_LIBRARY_DIRS}
)

target_link_libraries(goobuh PRIVATE
    ${SDL3_LIBRARIES}
    ${SDL3IMAGE_LIBRARIES}
    ${SDL3TTF_LIBRARIES}
    ${Vulkan_LIBRARIES}
    fastgltf
    ktx
    simdjson
)

if(WIN32)
    target_link_libraries(goobuh PRIVATE
        ${Vulkan_LIBRARIES}
        opengl32
    )
elseif(UNIX)
    target_link_libraries(goobuh PRIVATE
        ${Vulkan_LIBRARIES}
        dl
        pthread
        m
        GL
    )
endif()

# -------------------------
# Compiler settings
# -------------------------

set_target_properties(goobuh PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS NO
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

set_target_properties(engine PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS NO
)

# -------------------------
# Compiler optimization flags
# -------------------------

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(goobuh PRIVATE
        -g
        -O0
        -fno-inline
        -fno-omit-frame-pointer
    )

elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(goobuh PRIVATE
        -O3
        -march=native
        -mtune=native
        -ffast-math
        -funroll-loops
        -finline-functions
        -fomit-frame-pointer
        -fno-stack-protector
        -flto=auto
        -fvisibility=hidden
        -fno-rtti
        -fno-exceptions
        -fno-math-errno
        -fno-trapping-math
        -fno-signaling-nans
    )

    target_link_options(engine PRIVATE
        -flto=auto
        -Wl,-O3
        -Wl,--as-needed
        -Wl,--strip-all
    )

    target_compile_definitions(goobuh PRIVATE
        NDEBUG
        GOOBUH_RELEASE
        JPH_NO_DEBUG
    )

elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    target_compile_options(goobuh PRIVATE
        -O2
        -g
        -march=native
        -mtune=native
        -ffast-math
    )
endif()

# -------------------------
# SSE/AVX flags
# -------------------------

if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    check_cxx_compiler_flag("-msse" COMPILER_SUPPORTS_SSE)
    check_cxx_compiler_flag("-msse2" COMPILER_SUPPORTS_SSE2)
    check_cxx_compiler_flag("-msse3" COMPILER_SUPPORTS_SSE3)
    check_cxx_compiler_flag("-mssse3" COMPILER_SUPPORTS_SSSE3)
    check_cxx_compiler_flag("-msse4.1" COMPILER_SUPPORTS_SSE4_1)
    check_cxx_compiler_flag("-msse4.2" COMPILER_SUPPORTS_SSE4_2)
    check_cxx_compiler_flag("-mavx" COMPILER_SUPPORTS_AVX)
    check_cxx_compiler_flag("-mavx2" COMPILER_SUPPORTS_AVX2)
    check_cxx_compiler_flag("-mfma" COMPILER_SUPPORTS_FMA)

    if(COMPILER_SUPPORTS_SSE)
        target_compile_options(goobuh PRIVATE -msse)
        target_compile_definitions(goobuh PRIVATE __SSE__)
    endif()

    if(COMPILER_SUPPORTS_SSE2)
        target_compile_options(goobuh PRIVATE -msse2)
        target_compile_definitions(goobuh PRIVATE __SSE2__)
    endif()

    if(COMPILER_SUPPORTS_SSE3)
        target_compile_options(goobuh PRIVATE -msse3)
        target_compile_definitions(goobuh PRIVATE __SSE3__)
    endif()

    if(COMPILER_SUPPORTS_SSSE3)
        target_compile_options(goobuh PRIVATE -mssse3)
        target_compile_definitions(goobuh PRIVATE __SSSE3__)
    endif()

    if(COMPILER_SUPPORTS_SSE4_1)
        target_compile_options(goobuh PRIVATE -msse4.1)
        target_compile_definitions(goobuh PRIVATE __SSE4_1__)
    endif()

    if(COMPILER_SUPPORTS_SSE4_2)
        target_compile_options(goobuh PRIVATE -msse4.2)
        target_compile_definitions(goobuh PRIVATE __SSE4_2__)
    endif()

    if(COMPILER_SUPPORTS_AVX)
        target_compile_options(goobuh PRIVATE -mavx)
        target_compile_definitions(goobuh PRIVATE __AVX__)
    endif()

    if(COMPILER_SUPPORTS_AVX2)
        target_compile_options(goobuh PRIVATE -mavx2)
        target_compile_definitions(goobuh PRIVATE __AVX2__)
    endif()

    if(COMPILER_SUPPORTS_FMA)
        target_compile_options(goobuh PRIVATE -mfma)
        target_compile_definitions(goobuh PRIVATE __FMA__)
    endif()
endif()

# -------------------------
# Debug build flags
# -------------------------

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(goobuh PRIVATE
        -fno-omit-frame-pointer
        -fno-inline
        -fno-optimize-sibling-calls
        -fno-elide-constructors
        -fstack-protector-strong
        #-Werror
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        #-Wunused
        -Wcast-align
        #-Wconversion
        -Wsign-conversion
        #-Wdouble-promotion
        -Wformat=2
        -Wmissing-include-dirs
        -Wundef
        -fsanitize=address
        -fno-omit-frame-pointer
        -g
    )

    target_compile_definitions(goobuh PRIVATE
        DEBUG
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