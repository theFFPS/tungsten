cmake_minimum_required(VERSION 3.20)

project(tungsten)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_JOB_POOL_LINK link_pool)
set_property(GLOBAL PROPERTY JOB_POOLS link_pool=4)

file(GLOB_RECURSE TUNGSTEN_CORE_SOURCES "lib/*.cpp")
file(GLOB_RECURSE TUNGSTEN_CORE_HEADERS "inc/*.hpp")

set(TUNGSTEN_ARGUMENT_BASE -Wall -Wextra -Werror -s -fPIC -march=x86-64-v3
  -Ofast -flto=auto -freflection)
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
find_package(glaze REQUIRED)
find_package(libsodium REQUIRED)
find_package(VulkanHeaders REQUIRED)
find_package(vk-bootstrap REQUIRED)
find_package(vulkan-validationlayers REQUIRED)
find_package(VulkanMemoryAllocator REQUIRED)
find_package(SDL3 REQUIRED)
find_package(glm REQUIRED)
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(liburing CONFIG REQUIRED)
endif()

add_executable(license_check tools/license_check.cpp)
add_custom_target(license-check COMMAND $<TARGET_FILE:license_check>
                  meta/LICENSE_HEADER ${TUNGSTEN_CORE_SOURCES}
                  ${TUNGSTEN_CORE_HEADERS} src/main.cpp DEPENDS license_check
                  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" COMMENT
                  "Checking and applying C++ license headers..." VERBATIM)

add_library(tungsten-core STATIC ${TUNGSTEN_CORE_SOURCES})
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(tungsten-core PUBLIC liburing::liburing decor-0)
endif()
target_link_libraries(tungsten-core PUBLIC
  zstd::libzstd_static zlib-ng::zlib-ng
  OpenSSL::SSL OpenSSL::Crypto libsodium::libsodium
  asio::asio
  glaze::glaze
  stduuid::stduuid
  vulkan-headers::vulkan-headers
  vk-bootstrap::vk-bootstrap
  vulkan-validationlayers::vulkan-validationlayers
  GPUOpen::VulkanMemoryAllocator
  SDL3::SDL3-static glm::glm
)
target_compile_options(tungsten-core PRIVATE ${TUNGSTEN_ARGUMENT})

add_executable(tungsten src/main.cpp)
target_link_libraries(tungsten PUBLIC tungsten-core)
target_compile_options(tungsten PRIVATE ${TUNGSTEN_ARGUMENT})

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
