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

# Find required packages
find_package(Boost 1.70 REQUIRED COMPONENTS system thread)
find_package(OpenSSL REQUIRED)
find_package(Qt6 COMPONENTS Core Widgets Network)

# Qt setup
if(Qt6_FOUND)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)
    message(STATUS "Qt6 found - GUI will be built")
else()
    message(STATUS "Qt6 not found - GUI will be skipped")
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
)

# Common library
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

# Console client executable
add_executable(lanshare_client
    client/main.cpp
)

target_link_libraries(lanshare_client
    LanShareClient
    LanShareCommon
    Boost::system
    Boost::thread
)

# GUI client (only if Qt6 is found)
if(Qt6_FOUND)
    add_executable(lanshare_gui WIN32
        ui/main.cpp
        ui/LoginWindow.h
        ui/LoginWindow.cpp
        ui/LoginWindow.ui
        ui/ChatWindow.h
        ui/ChatWindow.cpp
        ui/ChatWindow.ui
        ui/MessageBubble.h
        ui/MessageBubble.cpp
        ui/FileTransferWidget.h
        ui/FileTransferWidget.cpp
    )

    target_link_libraries(lanshare_gui
        LanShareClient
        LanShareCommon
        Qt6::Core
        Qt6::Widgets
        Qt6::Network
        Boost::system
        Boost::thread
    )
    
    # Windows-specific
    if(WIN32)
        target_link_libraries(lanshare_gui ws2_32 wsock32)
    endif()
    
    message(STATUS "GUI target: lanshare_gui")
endif()

# Windows-specific libraries for other targets
if(WIN32)
    target_link_libraries(lanshare_server ws2_32 wsock32)
    target_link_libraries(lanshare_client ws2_32 wsock32)
endif()

# Print summary
message(STATUS "")
message(STATUS "LanShareCPP Build Configuration")
message(STATUS "================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Boost version: ${Boost_VERSION}")
message(STATUS "OpenSSL version: ${OPENSSL_VERSION}")
if(Qt6_FOUND)
    message(STATUS "Qt version: ${Qt6_VERSION}")
endif()
message(STATUS "")
message(STATUS "Targets:")
message(STATUS "  - lanshare_server (Server)")
message(STATUS "  - lanshare_client (Console client)")
if(Qt6_FOUND)
    message(STATUS "  - lanshare_gui (GUI client)")
endif()
message(STATUS "")
