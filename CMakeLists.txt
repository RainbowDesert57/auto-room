cmake_minimum_required(VERSION 3.10)
project(AutoRoom)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Source files
set(SOURCES src/logic-simulation.cpp)

# Create executable
add_executable(logic-simulation ${SOURCES})

