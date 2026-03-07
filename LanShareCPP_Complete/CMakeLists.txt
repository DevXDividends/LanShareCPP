cmake_minimum_required(VERSION 3.15)
project(LanShareCPP VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(Boost 1.70 REQUIRED COMPONENTS system thread)
find_package(OpenSSL REQUIRED)
find_package(Qt6 COMPONENTS Core Widgets Network)
find_library(SQLITE3_LIB sqlite3 REQUIRED)
set(SQLITE3_LIBRARIES ${SQLITE3_LIB})
if(Qt6_FOUND)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)
    message(STATUS "Qt6 found - GUI will be built")
else()
    message(STATUS "Qt6 not found - GUI will be skipped")
endif()

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${SQLite3_INCLUDE_DIRS}
)

# ── Common ─────────────────────────────────────────────────────────────────
add_library(LanShareCommon STATIC
    common/Protocol.h
    common/AESGCM.h
    common/AESGCM.cpp
)
target_link_libraries(LanShareCommon
    OpenSSL::SSL OpenSSL::Crypto stdc++fs)

# ── Server ─────────────────────────────────────────────────────────────────
add_library(LanShareServer STATIC
    server/ServerCore.h   server/ServerCore.cpp
    server/ClientSession.h server/ClientSession.cpp
    server/AuthManager.h   server/AuthManager.cpp
    server/GroupManager.h  server/GroupManager.cpp
    server/MessageRouter.h server/MessageRouter.cpp
)
target_link_libraries(LanShareServer
    LanShareCommon Boost::system Boost::thread)

# ── Client ─────────────────────────────────────────────────────────────────
add_library(LanShareClient STATIC
    client/ClientCore.h
    client/ClientCore.cpp
)
target_link_libraries(LanShareClient
    LanShareCommon Boost::system Boost::thread stdc++fs)

# ── Server exe ─────────────────────────────────────────────────────────────
add_executable(lanshare_server server/main.cpp)
target_link_libraries(lanshare_server
    LanShareServer LanShareCommon Boost::system Boost::thread)

# ── Console client exe ─────────────────────────────────────────────────────
add_executable(lanshare_client client/main.cpp)
target_link_libraries(lanshare_client
    LanShareClient LanShareCommon Boost::system Boost::thread)

# ── GUI ────────────────────────────────────────────────────────────────────
if(Qt6_FOUND)
    add_executable(lanshare_gui WIN32
        ui/main.cpp
        ui/LoginWindow.h   ui/LoginWindow.cpp   ui/LoginWindow.ui
        ui/ChatWindow.h    ui/ChatWindow.cpp     ui/ChatWindow.ui
        ui/MessageBubble.h ui/MessageBubble.cpp
        ui/FileTransferWidget.h ui/FileTransferWidget.cpp
        ui/MessageDB.h     ui/MessageDB.cpp
    )
    target_link_libraries(lanshare_gui
        LanShareClient LanShareCommon
        Qt6::Core Qt6::Widgets Qt6::Network
        Boost::system Boost::thread
        stdc++fs
        ${SQLITE3_LIBRARIES}    )
    if(WIN32)
        target_link_libraries(lanshare_gui ws2_32 wsock32)
    endif()
    message(STATUS "GUI target: lanshare_gui")
endif()

# ── Windows extras ─────────────────────────────────────────────────────────
if(WIN32)
    target_link_libraries(lanshare_server ws2_32 wsock32)
    target_link_libraries(lanshare_client ws2_32 wsock32)
endif()

message(STATUS "")
message(STATUS "LanShareCPP Build Configuration")
message(STATUS "================================")
message(STATUS "Build type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard:   ${CMAKE_CXX_STANDARD}")
message(STATUS "Boost:          ${Boost_VERSION}")
message(STATUS "OpenSSL:        ${OPENSSL_VERSION}")
message(STATUS "SQLite3:        ${SQLite3_VERSION}")
if(Qt6_FOUND)
    message(STATUS "Qt:             ${Qt6_VERSION}")
endif()
message(STATUS "")