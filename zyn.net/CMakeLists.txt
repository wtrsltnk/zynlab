cmake_minimum_required(VERSION 3.8)

project(zyn.net)

add_library(zyn.net STATIC
    HttpClient.cpp
    HttpClient.h
    HttpRequest.cpp
    HttpRequest.h
    HttpResponse.cpp
    HttpResponse.h
    HttpServer.cpp
    HttpServer.h
    )

target_include_directories(zyn.net
    PRIVATE
        ${CMAKE_SOURCE_DIR}
    )

target_compile_features(zyn.net
    PRIVATE
        cxx_auto_type
        cxx_nullptr
    )

target_link_libraries(zyn.net
    ws2_32
    wininet
    )

target_compile_definitions(zyn.net
    PUBLIC
        -DWIN32
    )

install(
    TARGETS
        zyn.net
    EXPORT
        zyn.net
    DESTINATION
        "lib"
    )

install(
    FILES
        ${zyn.net_SOURCE_DIR}/HttpClient.h
        ${zyn.net_SOURCE_DIR}/HttpRequest.h
        ${zyn.net_SOURCE_DIR}/HttpResponse.h
        ${zyn.net_SOURCE_DIR}/HttpServer.h
    DESTINATION
        "include/zyn/net"
    )

install(
    EXPORT
        zyn.net
    FILE
        zyn.net-config.cmake
    DESTINATION
        "cmake"
    )
