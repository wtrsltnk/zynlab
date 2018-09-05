cmake_minimum_required(VERSION 3.8)

project(app.threedee)

find_package(GLFW3)

if(GLFW3_FOUND)
    find_package(OpenGL REQUIRED)
    find_package(GLM REQUIRED)

    add_executable(app.threedee
        main.cpp
        app.threedee.cpp
        app.threedee.h
        stb_image.cpp
        imgui/imgui.cpp
        imgui/imgui_draw.cpp
        imgui_impl_glfw_gl3.cpp
        )

    target_include_directories(app.threedee
        PRIVATE
            ${CMAKE_SOURCE_DIR}
            ${GLFW3_INCLUDE_DIR}
            ${GLM_DIR}
            imgui
            include
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

    target_compile_features(app.threedee
        PRIVATE cxx_auto_type
        PRIVATE cxx_nullptr
        PRIVATE cxx_range_for
        PRIVATE cxx_thread_local
        )

endif(GLFW3_FOUND)
