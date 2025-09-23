cmake_minimum_required(VERSION 3.10)
project(AutoRoom)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include headers
include_directories(include)

# Source files
set(SOURCES
    info-fetch.cpp
    dataManager.cpp
    liveStatFetch.cpp
)

# Create executable
add_executable(auto-room ${SOURCES})

# If on Raspberry Pi and using bcm2835 for real sensor, link it
find_library(BCM2835_LIB bcm2835)
if(BCM2835_LIB)
    target_link_libraries(auto-room ${BCM2835_LIB})
endif()

