cmake_minimum_required(VERSION 3.10)
project(FalloutDOS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define the source files
set(SOURCES
    main.cpp
    Game.cpp
    Player.cpp
    Room.cpp
    Item.cpp
)

# Define the header files
set(HEADERS
    Game.h
    Player.h
    Room.h
    Item.h
    Command.h
)

# Create the executable
add_executable(FalloutDOS ${SOURCES} ${HEADERS})
