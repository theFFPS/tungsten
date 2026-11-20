cmake_minimum_required(VERSION 4.0)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_MODULE_STD ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(tungsten LANGUAGES CXX)

set(CMAKE_JOB_POOL_LINK link_pool)
set_property(GLOBAL PROPERTY JOB_POOLS link_pool=4)

file(GLOB_RECURSE TUNGSTEN_CORE_SOURCES "lib/*.cpp")
file(GLOB_RECURSE TUNGSTEN_CORE_HEADERS "inc/*.hpp")
file(GLOB_RECURSE TUNGSTEN_CORE_MODULES "mod/*.cppm")
file(GLOB_RECURSE TUNGSTEN_CORE_MODULES_SOURCES "mod/*.cpp")

option(NATIVE "Optimize for host CPU microarchitecture" OFF)

if(NATIVE)
    set(TUNGSTEN_ARCH_FLAG -march=native)
else()
    set(TUNGSTEN_ARCH_FLAG -march=x86-64-v4)
endif()

set(TUNGSTEN_ARGUMENT_BASE -Wall -Wextra -Werror ${TUNGSTEN_ARCH_FLAG}
  -O3 -fno-math-errno -fno-trapping-math -flto=auto -freflection -fno-rtti
  -DASIO_NO_TYPEID)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(TUNGSTEN_ARGUMENT ${TUNGSTEN_ARGUMENT_BASE}
        -DASIO_HAS_IO_URING -DASIO_DISABLE_EPOLL -fuse-ld=mold)
else()
    set(TUNGSTEN_ARGUMENT ${TUNGSTEN_ARGUMENT_BASE})
endif()

include_directories(inc/)

find_package(zstd REQUIRED)
find_package(zlib-ng REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(spdlog REQUIRED)
find_package(asio REQUIRED)
find_package(libqrencode REQUIRED)
find_package(stduuid CONFIG REQUIRED)
find_package(libsodium REQUIRED)
find_package(VulkanHeaders REQUIRED)
find_package(vk-bootstrap REQUIRED)
find_package(vulkan-validationlayers REQUIRED)
find_package(VulkanMemoryAllocator REQUIRED)
find_package(SDL3 REQUIRED)
find_package(Ktx REQUIRED)
find_package(glm REQUIRED)
find_package(TBB REQUIRED)
find_package(FastNoise2 REQUIRED)
find_package(freetype CONFIG REQUIRED)
find_package(Freetype REQUIRED)
find_package(stb REQUIRED)
find_package(mimalloc REQUIRED)
find_package(meshoptimizer REQUIRED)
find_package(Tracy REQUIRED)
find_package(harfbuzz REQUIRED)
find_package(Jolt REQUIRED)
find_package(glaze REQUIRED)
find_package(miniz REQUIRED)
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(liburing CONFIG REQUIRED)
endif()

add_executable(license_check tools/license_check.cpp)
add_custom_target(license-check COMMAND $<TARGET_FILE:license_check>
                  meta/LICENSE_HEADER ${TUNGSTEN_CORE_SOURCES}
                  ${TUNGSTEN_CORE_HEADERS} ${TUNGSTEN_CORE_MODULES}
                  ${TUNGSTEN_CORE_MODULES_SOURCES}
                  src/main.cpp tests/datapack_tests.cpp
                  DEPENDS license_check
                  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" COMMENT
                  "Checking and applying C++ license headers..." VERBATIM)

add_library(tungsten-core STATIC)
target_sources(tungsten-core
    PRIVATE ${TUNGSTEN_CORE_SOURCES}
)
target_sources(tungsten-core
    PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS mod
        FILES ${TUNGSTEN_CORE_MODULES}
    PRIVATE
        ${TUNGSTEN_CORE_MODULES_SOURCES}
)
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(tungsten-core PUBLIC liburing::liburing decor-0)
endif()
target_link_libraries(tungsten-core PUBLIC
  zstd::libzstd_static zlib-ng::zlib-ng
  OpenSSL::SSL OpenSSL::Crypto libsodium::libsodium
  asio::asio
  stduuid::stduuid
  vulkan-headers::vulkan-headers
  vk-bootstrap::vk-bootstrap
  vulkan-validationlayers::vulkan-validationlayers
  GPUOpen::VulkanMemoryAllocator
  SDL3::SDL3-static glm::glm KTX::ktx
  spdlog::spdlog fastnoise2::fastnoise2
  Freetype::Freetype TBB::tbb stb::stb glaze::glaze
  mimalloc-static meshoptimizer::meshoptimizer Tracy::TracyClient
  Jolt::Jolt miniz::miniz harfbuzz::harfbuzz
)
target_compile_options(tungsten-core PRIVATE ${TUNGSTEN_ARGUMENT})

add_executable(tungsten src/main.cpp)
target_link_libraries(tungsten PUBLIC tungsten-core)
target_compile_options(tungsten PRIVATE ${TUNGSTEN_ARGUMENT})

add_executable(tungsten-datapack-tests tests/datapack_tests.cpp)
target_link_libraries(tungsten-datapack-tests PUBLIC tungsten-core)
target_compile_options(tungsten-datapack-tests PRIVATE ${TUNGSTEN_ARGUMENT})

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    set(HOOK_CONTENT
"#!/bin/sh
echo 'Running automatic license header check...'
cmake --build \"${CMAKE_BINARY_DIR}\" --target license-check
"
    )

    file(GENERATE
        OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/.git/hooks/pre-commit"
        CONTENT "${HOOK_CONTENT}"
    )

    if(NOT WIN32)
        add_custom_target(patch_git_hook_permissions ALL
            COMMAND chmod +x "${CMAKE_CURRENT_SOURCE_DIR}/.git/hooks/pre-commit"
            VERBATIM
        )
    endif()

endif()
