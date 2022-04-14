cmake_minimum_required(VERSION 3.2)

# Set up project.
project(mt3339
        VERSION 1.0
        DESCRIPTION "A library for interfacing with a MediaTek MT3339 GPS.")

# Add source files.
file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# Create library.
add_library(mt3339 STATIC ${sources})

# Link library to NMEA.
target_link_libraries(mt3339
    nmea)

# Set include directories for internal/external use.
target_include_directories(mt3339 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)