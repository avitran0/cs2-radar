#!/bin/bash
cd backend
./build.sh
cd ..
cp backend/build/cs2-radar ./frontend/static
