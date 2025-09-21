cmake_minimum_required(VERSION 3.15)
project(TUIManager)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include headers
include_directories(src/include)

# Source files
set(SOURCES
    src/main.cpp
    src/manager.cpp
    src/actions.cpp
    src/os_detect.cpp
)

# Executable
add_executable(manager ${SOURCES})

# Link ncurses for TUI
find_package(Curses REQUIRED)
if(CURSES_FOUND)
    include_directories(${CURSES_INCLUDE_DIR})
    target_link_libraries(manager ${CURSES_LIBRARIES})
endif()

# Set output directories
set_target_properties(manager PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/linux
)

# Optional: Windows cross-compile (using mingw-w64 on Linux)
if(WIN32 OR MINGW)
    set_target_properties(manager PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/windows
    )
endif()

# Optional: macOS output
if(APPLE)
    set_target_properties(manager PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/mac
    )
endif()

