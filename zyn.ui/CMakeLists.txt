cmake_minimum_required(VERSION 3.12)

project(zyn.ui)

add_library(zyn.ui
    glad.c
    imgui/examples/imgui_impl_opengl3.cpp
    imgui/examples/imgui_impl_opengl3.h
    imgui/examples/imgui_impl_win32.cpp
    imgui/examples/imgui_impl_win32.h
    imgui/imgui.cpp
    imgui/imgui.h
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_internal.h
    imgui/imgui_widgets.cpp
    imgui_helpers.cpp
    imgui_plot.cpp
    include/imgui_helpers.h
    include/imgui_plot.h
    librarydialog.cpp
    librarydialog.h
    piano.cpp
    piano.h
    syntheditor.cpp
    syntheditor.h
    )

target_include_directories(zyn.ui
    PUBLIC include
    PUBLIC imgui
    PRIVATE ${CMAKE_SOURCE_DIR})

target_compile_features(zyn.ui
    PRIVATE cxx_auto_type
    PRIVATE cxx_nullptr
    PRIVATE cxx_range_for
    )

target_link_libraries(zyn.ui
    PRIVATE
        ${OPENGL_LIBRARIES}
        zyn.nio
        zyn.mixer
        zyn.synth
        zyn.serialization
        zyn.common)

target_compile_options(zyn.ui
    PUBLIC 
        -DUNICODE
        -D_UNICODE
        -DIMGUI_IMPL_WIN32_DISABLE_GAMEPAD
        -DIMGUI_IMPL_OPENGL_LOADER_GLAD)
