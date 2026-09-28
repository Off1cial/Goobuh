cmake_minimum_required(VERSION 3.20)

project(engine
  VERSION 0.1
  LANGUAGES C CXX ASM_NASM
)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)


set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
add_compile_definitions(_POSIX_C_SOURCE=200809L)
add_compile_definitions(_GNU_SOURCE) # Added 17/09/26 17:50 for struct ip_mreq 



set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL3 REQUIRED IMPORTED_TARGET sdl3)

find_package(Vulkan REQUIRED)
find_package(Ktx CONFIG REQUIRED)

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

add_library(engine_lib STATIC
  ${ENGINE_SOURCES}
  ${CMAKE_CURRENT_SOURCE_DIR}/include/volk/volk.c
  ${CMAKE_CURRENT_SOURCE_DIR}/include/tinyobj/tiny_obj_loader.cpp
)

if(WIN32)
  set(PLATFORM_DEFINE WINDOWS)
elseif(UNIX AND NOT APPLE)
  set(PLATFORM_DEFINE LINUX)
endif()

target_compile_definitions(engine_lib PUBLIC ${PLATFORM_DEFINE})
target_compile_definitions(engine_lib PUBLIC
ASSET_DIR="${CMAKE_SOURCE_DIR}/resource"
)

target_link_libraries(engine_lib
  PUBLIC
    PkgConfig::SDL3
    Vulkan::Vulkan
    fastgltf
    simdjson
    ktx
)

target_include_directories(engine_lib 
  PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/public
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(engine
  src/main.cpp
)

target_compile_definitions(engine PUBLIC
ASSET_DIR="${CMAKE_SOURCE_DIR}/resource"
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

target_compile_options(engine_lib PRIVATE
  -Wall
  -Wextra
  #-Wpedantic
  #-Wconversion
  #-Wshadow
)

target_compile_options(engine PRIVATE
  -Wall
  -Wextra
  #-Wpedantic
  #-Wconversion
  #-Wshadow
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_options(engine_lib PRIVATE
    -g
    -O0
    -fsanitize=address,undefined
    -fno-omit-frame-pointer
  )


  target_compile_options(engine PRIVATE
    -g
    -O0
    -fsanitize=address,undefined
    -fno-omit-frame-pointer
  )

  target_link_options(engine PRIVATE
    -fsanitize=address,undefined
  )
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  target_compile_options(engine_lib PRIVATE
    -O3
    -march=native
    -flto
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
