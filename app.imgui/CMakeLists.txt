cmake_minimum_required(VERSION 3.8)

project(app.imgui)

find_package(GLFW3)

if(GLFW3_FOUND)
    find_package(OpenGL REQUIRED)
    find_package(GLM REQUIRED)

    set(app_imgui_SRCS
        main.cpp
        app.imgui.cpp
        )

    set(app_imgui_HDRS
        app.imgui.h
        )

    set(app_imgui_imgui_SRCS
        imgui/imgui.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_demo.cpp
        imgui_impl_glfw_gl3.cpp
        )

    add_executable(app.imgui
        ${app_imgui_SRCS}
        ${app_imgui_HDRS}
        ${app_imgui_imgui_SRCS}
        )

    target_include_directories(app.imgui
        PRIVATE
            ${CMAKE_SOURCE_DIR}
            ${GLFW3_INCLUDE_DIR}
            ${GLM_DIR}
            imgui
        )

    target_link_libraries(app.imgui
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

    target_compile_features(app.imgui
        PRIVATE cxx_auto_type
        PRIVATE cxx_nullptr
        PRIVATE cxx_range_for
        PRIVATE cxx_thread_local
        )

endif(GLFW3_FOUND)
