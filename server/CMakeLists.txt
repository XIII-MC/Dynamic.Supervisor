cmake_minimum_required(VERSION 3.25.1)
project(Dynamic.Supervisor)

set(CMAKE_CXX_STANDARD 26)

message(STATUS "Building for host architecture: ${CMAKE_SYSTEM_PROCESSOR}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(GLOB OLD_EXECUTABLES "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Dynamic.Supervisor-SRV*")
foreach(exe ${OLD_EXECUTABLES})
    file(REMOVE ${exe})
endforeach()

#32/64 bit
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    add_executable(Dynamic.Supervisor-SRV main.cpp)
    set_target_properties(Dynamic.Supervisor-SRV PROPERTIES OUTPUT_NAME "Dynamic.Supervisor-SRV_x86_64")
    message(STATUS "Building x86_64 target: Dynamic.Supervisor-SRV-x86_64")
endif()

#ARM64
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    add_executable(Dynamic.Supervisor-SRV main.cpp)
    set_target_properties(Dynamic.Supervisor-SRV PROPERTIES OUTPUT_NAME "Dynamic.Supervisor-SRV_ARM64")
    message(STATUS "Building ARM64 target: Dynamic.Supervisor-SRV-ARM64")
endif()

#ARM32
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7l" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
    add_executable(Dynamic.Supervisor-SRV main.cpp)
    set_target_properties(Dynamic.Supervisor-SRV PROPERTIES OUTPUT_NAME "Dynamic.Supervisor-SRV_ARM32")
    message(STATUS "Building ARM32 target: Dynamic.Supervisor-SRV-ARM32")
endif()
