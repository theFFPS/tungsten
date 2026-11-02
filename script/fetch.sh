#!/bin/bash

if [ -e conanfile.py ]; then
    conan install . --build=missing -s compiler.cppstd=gnu26 -c tools.cmake.cmaketoolchain:extra_variables="{'CMAKE_IGNORE_PREFIX_PATH': '/opt/cuda'}"
else
    echo "You must run this command in root directory of tungsten"
fi
