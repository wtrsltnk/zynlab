cmake_minimum_required(VERSION 2.8)

project(app.threedee)

find_package(GLFW3)
if(GLFW3_FOUND)
    find_package(OpenGL REQUIRED)
    find_package(GLM REQUIRED)

    set(app_threedee_SRCS
        main.cpp
        app.threedee.cpp
        stb_image.cpp
        scenenode.cpp
        )

    set(app_threedee_HDRS
        app.threedee.h
        scenenode.h
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

    target_include_directories(app.threedee
        PRIVATE ${GLFW3_INCLUDE_DIR}
        PRIVATE ${GLM_DIR}
        PRIVATE imgui
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
