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

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Unix-only definitions
if(UNIX AND NOT APPLE)
    add_compile_definitions(_POSIX_C_SOURCE=200809L)
    add_compile_definitions(_GNU_SOURCE)
endif()

# The exe and the game module land in the same folder, so the engine can
# find the module next to itself at runtime.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Headers that BOTH sides may include: the API structs, header-only helpers.
# Nothing in here may define global state.
set(SHARED_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(PUBLIC_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/public")

# ---------------------------------------------------------------------------
# Flags shared by engine and game. Both must be built the same way
# (sanitizers, LTO, definitions), or the boundary between them breaks.
# ---------------------------------------------------------------------------
add_library(build_flags INTERFACE)
if(UNIX)
    target_link_libraries(build_flags INTERFACE m)
endif()
target_compile_options(build_flags INTERFACE
    $<$<COMPILE_LANGUAGE:C, CXX>:-Wall;-Wextra>
)


if(WIN32)
    target_compile_definitions(build_flags INTERFACE WINDOWS)
elseif(UNIX AND NOT APPLE)
    target_compile_definitions(build_flags INTERFACE
        LINUX
        _POSIX_C_SOURCE=200809L
        _GNU_SOURCE
    )
endif()




if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(build_flags INTERFACE
        $<$<COMPILE_LANGUAGE:C, CXX>:-g;-O3;-fno-omit-frame-pointer;-Wno-unused-variable>
    )
    if(NOT WIN32)
        target_compile_options(build_flags INTERFACE
            $<$<COMPILE_LANGUAGE:C, CXX>:-fsanitize=address,undefined>
        )
        target_link_options(build_flags INTERFACE
            -fsanitize=address,undefined
        )
    endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(build_flags INTERFACE
        $<$<COMPILE_LANGUAGE:C, CXX>:-O3;-march=native;-flto>
    )
    target_link_options(build_flags INTERFACE -flto)
endif()

# ---------------------------------------------------------------------------
# Game module: loaded by the engine at runtime. Sees ONLY the shared headers.
# ---------------------------------------------------------------------------
file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/game/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/game/*.cpp"
)

add_library(game MODULE ${GAME_SOURCES})

set_target_properties(game PROPERTIES
    PREFIX ""                      # game.dll / game.so, not libgame.so
    C_VISIBILITY_PRESET hidden     # export only what is marked GAME_API
)

target_include_directories(game PRIVATE ${SHARED_INCLUDE_DIR} ${PUBLIC_INCLUDE_DIR})
target_link_libraries(game PRIVATE build_flags)

# ---------------------------------------------------------------------------
# Engine: the executable. Owns main(), the loop, SDL, OpenGL.
# ---------------------------------------------------------------------------
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL3 REQUIRED IMPORTED_TARGET sdl3)
find_package(OpenGL REQUIRED)

find_package(Vulkan REQUIRED)
include(FetchContent)
FetchContent_Declare(
    vk-bootstrap
    GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap
    GIT_TAG main
)
FetchContent_MakeAvailable(vk-bootstrap)


file(GLOB ENGINE_TOP_SOURCES CONFIGURE_DEPENDS    # e.g. src/main.c, if it lives there
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/*.asm"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.asm"
)

add_executable(engine ${ENGINE_TOP_SOURCES} ${ENGINE_SOURCES})

target_compile_definitions( engine PUBLIC
    RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resource/"
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions( engine PUBLIC _DEBUG)
endif()

target_include_directories(engine PRIVATE
    ${SHARED_INCLUDE_DIR}
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(engine PRIVATE
    build_flags
    PkgConfig::SDL3
    vk-bootstrap::vk-bootstrap
    Vulkan::Vulkan
    stdc++
)
if (WIN32)
target_link_libraries(engine PRIVATE ws2_32)
endif()

# Not linked: the engine never links the game. It loads it at runtime.
# This only makes sure building engine also builds the module.
add_dependencies(engine game)

if(WIN32)
    set_target_properties(engine PROPERTIES WIN32_EXECUTABLE FALSE)
    target_link_options(engine PRIVATE -mconsole)
    target_link_libraries(engine PRIVATE opengl32)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(engine PRIVATE OpenGL::GL)
elseif(APPLE)
    target_link_libraries(engine PRIVATE "-framework OpenGL")
endif()
