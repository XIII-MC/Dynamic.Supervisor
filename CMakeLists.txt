cmake_minimum_required(VERSION 3.31)
project(Dynamic.Supervisor)

set(CMAKE_CXX_STANDARD 26)

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set(SRV_TARGET_NAME "Dynamic.Supervisor-SRV_x86_64")
    set(CLT_TARGET_NAME "Dynamic.Supervisor-CLT_x86_64")
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    set(SRV_TARGET_NAME "Dynamic.Supervisor-SRV_ARM64")
    set(CLT_TARGET_NAME "Dynamic.Supervisor-CLT_ARM64")
    set(CMAKE_C_COMPILER "aarch64-linux-gnu-gcc")
    set(CMAKE_CXX_COMPILER "aarch64-linux-gnu-g++")
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7l")
    set(SRV_TARGET_NAME "Dynamic.Supervisor-SRV_ARM32")
    set(CLT_TARGET_NAME "Dynamic.Supervisor-CLT_ARM32")
    set(CMAKE_C_COMPILER "arm-linux-gnueabihf-gcc")
    set(CMAKE_CXX_COMPILER "arm-linux-gnueabihf-g++")
else()
    message(FATAL_ERROR "Unsupported architecture: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

include_directories(${CMAKE_SOURCE_DIR}/include/Crow/include)

# Server executable
add_executable(${SRV_TARGET_NAME} server/main.cpp
        server/utils/json/JSONManager.cpp
        server/utils/json/JSONManager.h
        server/backend/BackendRunner.cpp
        server/backend/BackendRunner.h
)
set_target_properties(${SRV_TARGET_NAME} PROPERTIES OUTPUT_NAME ${SRV_TARGET_NAME})

find_package(Boost REQUIRED)
find_package(OpenSSL REQUIRED)

target_link_libraries(${SRV_TARGET_NAME}
        Boost::boost
        OpenSSL::SSL
        OpenSSL::Crypto
)

# Client executable
add_executable(${CLT_TARGET_NAME} client/main.cpp)
set_target_properties(${CLT_TARGET_NAME} PROPERTIES OUTPUT_NAME ${CLT_TARGET_NAME})