cmake_minimum_required(VERSION 3.8)

project(app.threedee)

find_package(GLFW3)

if(GLFW3_FOUND)
    find_package(OpenGL REQUIRED)
    find_package(GLM REQUIRED)

    add_executable(app.threedee
        glad.c
        main.cpp
        app.threedee.cpp
        app.threedee.h
        imgui/imgui.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_widgets.cpp
        imgui/examples/imgui_impl_opengl3.cpp
        imgui/examples/imgui_impl_glfw.cpp
        imgui_addons/imgui_dock.cpp
        imgui_addons/imgui_dock.h
        imgui_addons/imgui_knob.cpp
        imgui_addons/imgui_knob.h
        ui/adnote.cpp
        ui/effect.cpp
        ui/envelope.cpp
        ui/filter.cpp
        ui/lfo.cpp
        ui/microtonal.cpp
        ui/oscilgen.cpp
        ui/padnote.cpp
        ui/resonance.cpp
        ui/subnote.cpp
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
        zyn.net
        zyn.mixer
        zyn.synth
        zyn.fx
        zyn.dsp
        zyn.seq
        zyn.common
        )

    target_compile_definitions(app.threedee
        PRIVATE -DIMGUI_IMPL_OPENGL_LOADER_GLAD
        )

    target_compile_features(app.threedee
        PRIVATE cxx_auto_type
        PRIVATE cxx_nullptr
        PRIVATE cxx_range_for
        PRIVATE cxx_thread_local
        )

endif(GLFW3_FOUND)
