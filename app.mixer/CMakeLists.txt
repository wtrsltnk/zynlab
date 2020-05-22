cmake_minimum_required(VERSION 3.12)

project(app.mixer)

add_executable(app.mixer
    application.cpp
    realmixer.cpp
    realmixer.h
    glad.c
    imgui/imgui.cpp
    imgui/imgui.h
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_impl_opengl3.cpp
    imgui/imgui_impl_opengl3.h
    imgui/imgui_impl_win32.cpp
    imgui/imgui_impl_win32.h
    imgui/imgui_internal.h
    imgui/imgui_widgets.cpp
    main.cpp
    )

target_include_directories(app.mixer
    PUBLIC include
    PUBLIC imgui
    PRIVATE ${CMAKE_SOURCE_DIR})

target_compile_features(app.mixer
    PRIVATE cxx_auto_type
    PRIVATE cxx_nullptr
    PRIVATE cxx_range_for
    )

target_link_libraries(app.mixer
    PRIVATE
        ${OPENGL_LIBRARIES}
        zyn.nio
        zyn.mixer
        zyn.synth
        zyn.common)

target_compile_options(app.mixer
    PUBLIC 
        -DUNICODE
        -D_UNICODE
        -DIMGUI_IMPL_WIN32_DISABLE_GAMEPAD)

if(MINGW)
    target_link_options(app.mixer
        PUBLIC -static -mwindows -municode)
endif()
