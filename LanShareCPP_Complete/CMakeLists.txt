cmake_minimum_required(VERSION 3.15)
project(LanShareCPP VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

# Find required packages
find_package(Boost 1.70 REQUIRED COMPONENTS system thread)
find_package(OpenSSL REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
)

# Common library (Protocol and Crypto)
add_library(LanShareCommon STATIC
    common/Protocol.h
    common/AESGCM.h
    common/AESGCM.cpp
)

target_link_libraries(LanShareCommon
    OpenSSL::SSL
    OpenSSL::Crypto
)

# Server library
add_library(LanShareServer STATIC
    server/ServerCore.h
    server/ServerCore.cpp
    server/ClientSession.h
    server/ClientSession.cpp
    server/AuthManager.h
    server/AuthManager.cpp
    server/GroupManager.h
    server/GroupManager.cpp
    server/MessageRouter.h
    server/MessageRouter.cpp
)

target_link_libraries(LanShareServer
    LanShareCommon
    Boost::system
    Boost::thread
    OpenSSL::SSL
    OpenSSL::Crypto
)

# Client library
add_library(LanShareClient STATIC
    client/ClientCore.h
    client/ClientCore.cpp
)

target_link_libraries(LanShareClient
    LanShareCommon
    Boost::system
    Boost::thread
    OpenSSL::SSL
    OpenSSL::Crypto
)

# Server executable
add_executable(lanshare_server
    server/main.cpp
)

target_link_libraries(lanshare_server
    LanShareServer
    LanShareCommon
    Boost::system
    Boost::thread
)

# Client executable  
add_executable(lanshare_client
    client/main.cpp
)

target_link_libraries(lanshare_client
    LanShareClient
    LanShareCommon
    Boost::system
    Boost::thread
)

# Windows-specific libraries
if(WIN32)
    target_link_libraries(lanshare_server ws2_32 wsock32)
    target_link_libraries(lanshare_client ws2_32 wsock32)
endif()

# Installation
install(TARGETS lanshare_server lanshare_client
    RUNTIME DESTINATION bin
)

# Print configuration summary
message(STATUS "")
message(STATUS "LanShareCPP Configuration Summary")
message(STATUS "================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Boost version: ${Boost_VERSION}")
message(STATUS "OpenSSL version: ${OPENSSL_VERSION}")
message(STATUS "")
message(STATUS "Building:")
message(STATUS "  - lanshare_server (Server application)")
message(STATUS "  - lanshare_client (Console client)")
message(STATUS "")
