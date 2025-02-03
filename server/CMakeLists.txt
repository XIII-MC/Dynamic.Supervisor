cmake_minimum_required(VERSION 3.25)
project(Dynamic.Supervisor)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 26)

# Detect system architecture
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set(TARGET_NAME "Dynamic.Supervisor-SRV_x86_64")
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    set(TARGET_NAME "Dynamic.Supervisor-SRV_ARM64")
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7l")
    set(TARGET_NAME "Dynamic.Supervisor-SRV_ARM32")
else()
    message(FATAL_ERROR "Unsupported architecture: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

# Set compiler for cross-compilation (if needed)
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    set(CMAKE_C_COMPILER "aarch64-linux-gnu-gcc")
    set(CMAKE_CXX_COMPILER "aarch64-linux-gnu-g++")
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7l")
    set(CMAKE_C_COMPILER "arm-linux-gnueabihf-gcc")
    set(CMAKE_CXX_COMPILER "arm-linux-gnueabihf-g++")
endif()

# Create executable
add_executable(${TARGET_NAME} main.cpp)

# Optionally, set output properties
set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME ${TARGET_NAME})
