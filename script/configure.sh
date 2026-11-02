#!/bin/bash

if [ -e conanfile.py ]; then
    conan install . --build=missing -s compiler.cppstd=gnu26 -c tools.cmake.cmaketoolchain:extra_variables="{'CMAKE_IGNORE_PREFIX_PATH': '/opt/cuda'}"
    cmake -B build/Release -DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr . -G Ninja
else
    echo "You must run this command in root directory of tungsten"
fi
