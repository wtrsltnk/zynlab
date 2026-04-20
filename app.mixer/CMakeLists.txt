cmake_minimum_required(VERSION 3.12)

project(app.mixer)

add_executable(app.mixer
    application.cpp
    realmixer.cpp
    realmixer.h
    glad.c
    main.cpp
    )

target_include_directories(app.mixer
    PUBLIC include
    PRIVATE ${CMAKE_SOURCE_DIR})

target_compile_features(app.mixer
    PRIVATE cxx_auto_type
    PRIVATE cxx_nullptr
    PRIVATE cxx_range_for
    PRIVATE cxx_std_17
    )

target_link_libraries(app.mixer
    PRIVATE
        ${OPENGL_LIBRARIES}
        zyn.nio
        zyn.net
        zyn.mixer
        zyn.synth
        zyn.fx
        zyn.dsp
        zyn.seq
        zyn.serialization
        zyn.common
        zyn.ui)

target_compile_options(app.mixer
    PUBLIC 
        -DUNICODE
        -D_UNICODE
        -DIMGUI_IMPL_WIN32_DISABLE_GAMEPAD)

if(MINGW)
    target_link_options(app.mixer
        PUBLIC -static -mwindows -municode)
endif()
