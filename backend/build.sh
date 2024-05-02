#!/bin/bash
cmake -DCMAKE_BUILD_TYPE:STRING=Release -S. -B./build -G "Unix Makefiles"
cmake --build ./build --config Release --target all -j 8