cmake_minimum_required(VERSION 3.12)

project(lib.imgui)

find_package(OpenGL REQUIRED)
find_package(GLM REQUIRED)

add_library(lib.imgui
    glad.c
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp
    imgui/imconfig.h
    imgui/imgui.cpp
    imgui/imgui.h
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui_addons/imgui_Timeline.h
    imgui_addons/imgui_checkbutton.cpp
    imgui_addons/imgui_checkbutton.h
    imgui_addons/imgui_common.cpp
    imgui_addons/imgui_common.h
    imgui_addons/imgui_knob.cpp
    imgui_addons/imgui_knob.h
    imgui_addons/imgui_seq.cpp
    imgui_addons/imgui_seq.h
    )

target_include_directories(lib.imgui
    PUBLIC include
    PUBLIC imgui
    PUBLIC imgui_addons)

target_link_libraries(lib.imgui
    PUBLIC
        glfw)

target_compile_definitions(lib.imgui
    PRIVATE
        -DIMGUI_IMPL_OPENGL_LOADER_GLAD
        -DIMGUI_IMPL_WIN32_DISABLE_GAMEPAD)
