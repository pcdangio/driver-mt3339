cmake_minimum_required(VERSION 3.2)

# Set up project.
project(mt3339
        VERSION 1.0
        DESCRIPTION "A library for interfacing with a MediaTek MT3339 GPS.")

# Add project subdirectories.
add_subdirectory("nmea")
add_subdirectory("mt3339")