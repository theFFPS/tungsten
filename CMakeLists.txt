cmake_minimum_required(VERSION 3.20)

project(tungsten)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_JOB_POOL_LINK link_pool)
set_property(GLOBAL PROPERTY JOB_POOLS link_pool=4)

file(GLOB_RECURSE TUNGSTEN_CORE_SOURCES "lib/*.cpp")

set(TUNGSTEN_ARGUMENT_BASE -Wall -Wextra -Werror -s -fPIC -march=x86-64-v3
  -Ofast -flto=auto -fno-rtti)
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
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(liburing CONFIG REQUIRED)
endif()

add_library(tungsten-core STATIC ${TUNGSTEN_CORE_SOURCES})
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(tungsten-core PUBLIC liburing::liburing)
endif()
target_link_libraries(tungsten-core PUBLIC
  zstd::libzstd_static zlib-ng::zlib-ng
  OpenSSL::SSL OpenSSL::Crypto libsodium::libsodium
  asio::asio
  glaze::glaze
  stduuid::stduuid
)
target_compile_options(tungsten-core PRIVATE ${TUNGSTEN_ARGUMENT})

add_executable(tungsten src/main.cpp)
target_link_libraries(tungsten PUBLIC tungsten-core)
target_compile_options(tungsten PRIVATE ${TUNGSTEN_ARGUMENT})
