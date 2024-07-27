#!/bin/bash
cmake -DCMAKE_BUILD_TYPE:STRING=Debug -S. -B./build -G "Unix Makefiles"
cmake --build ./build --config Debug --target all -j 8
