cmake_minimum_required(VERSION 2.8)

project(app.threedee)

find_package(GLFW3)
if(GLFW3_FOUND)
    find_package(OpenGL REQUIRED)

    include_directories(
        imgui
        )

    set(app_threedee_SRCS
        main.cpp
        app.threedee.cpp
        )

    set(app_threedee_HDRS
        app.threedee.h
        )

    set(app_threedee_imgui_SRCS
        imgui/imgui.cpp
        imgui/imgui_draw.cpp
        imgui_impl_glfw_gl3.cpp
        )

    add_executable(app.threedee
        ${app_threedee_SRCS}
        ${app_threedee_HDRS}
        ${app_threedee_imgui_SRCS}
        )

    target_link_libraries(app.threedee
        ${ZLIB_LIBRARIES}
        ${OPENGL_LIBRARIES}
        ${GLFW3_LIBRARY}
        zyn.nio
        zyn.mixer
        zyn.synth
        zyn.fx
        zyn.dsp
        zyn.common
        )
endif(GLFW3_FOUND)