cmake_minimum_required(VERSION 3.8)

project(app.imgui)

find_package(GLFW3)

if(GLFW3_FOUND)
    find_package(OpenGL REQUIRED)
    find_package(GLM REQUIRED)

    add_executable(app.imgui
        main.cpp
        app.imgui.cpp
        app.imgui.h
        ImSequencer.cpp
        ImSequencer.h
        glad.c
        imgui/imgui.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_demo.cpp
        imgui/imgui_widgets.cpp
        imgui/examples/imgui_impl_glfw.cpp
        #imgui/examples/imgui_impl_opengl3.cpp
        )

    target_include_directories(app.imgui
        PRIVATE
            ${CMAKE_SOURCE_DIR}
            ${GLFW3_INCLUDE_DIR}
            ${GLM_DIR}
            include
            imgui
        )

    target_link_libraries(app.imgui
        ${ZLIB_LIBRARIES}
        ${OPENGL_LIBRARIES}
        ${GLFW3_LIBRARY}
        zyn.nio
        zyn.net
        zyn.mixer
        zyn.synth
        zyn.fx
        zyn.dsp
        zyn.serialization
        zyn.common
        )

    target_compile_features(app.imgui
        PRIVATE cxx_auto_type
        PRIVATE cxx_nullptr
        PRIVATE cxx_range_for
        PRIVATE cxx_thread_local
        )

endif(GLFW3_FOUND)
