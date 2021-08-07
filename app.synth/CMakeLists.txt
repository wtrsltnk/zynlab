cmake_minimum_required(VERSION 3.12)

project(app.synth)

add_executable(app.synth
    application.cpp
    application.h
    main.cpp
    )

target_include_directories(app.synth
    PUBLIC include
    PUBLIC imgui
    PRIVATE ${CMAKE_SOURCE_DIR})

target_compile_features(app.synth
    PRIVATE cxx_auto_type
    PRIVATE cxx_nullptr
    PRIVATE cxx_range_for
    )

target_link_libraries(app.synth
    PRIVATE
        ${OPENGL_LIBRARIES}
        zyn.nio
        zyn.mixer
        zyn.synth
        zyn.serialization
        zyn.common
        zyn.ui)

target_compile_options(app.synth
    PUBLIC 
        -DUNICODE
        -D_UNICODE
        -DIMGUI_IMPL_WIN32_DISABLE_GAMEPAD)

if(MINGW)
    target_link_options(app.synth
        PUBLIC -static -mwindows -municode)
endif()
