#!/bin/bash

echo "Compiling FalloutDOS for Windows..."
x86_64-w64-mingw32-g++ -std=c++17 main.cpp Game.cpp Player.cpp Room.cpp Item.cpp -o falloutdos.exe -static-libgcc -static-libstdc++

if [ $? -eq 0 ]; then
    echo "Success! Created falloutdos.exe"
else
    echo "Compilation failed."
fi
