cmake_minimum_required(VERSION 3.8)

project(app.drummer)

find_package(GLFW3)

if(GLFW3_FOUND)
    find_package(OpenGL REQUIRED)
    find_package(GLM REQUIRED)

    add_executable(app.drummer
        app.drummer.cpp
        app.drummer.h
        appstate.cpp
        appstate.h
        glad.c
        imgui/examples/imgui_impl_glfw.cpp
        imgui/examples/imgui_impl_opengl3.cpp
        imgui/imconfig.h
        imgui/imgui.cpp
        imgui/imgui_demo.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_widgets.cpp
        imgui_addons/imgui_checkbutton.cpp
        imgui_addons/imgui_checkbutton.h
        imgui_addons/imgui_knob.cpp
        imgui_addons/imgui_knob.h
        imgui_addons/imgui_seq.cpp
        imgui_addons/imgui_seq.h
        imgui_addons/imgui_Timeline.h
        instrumentcategories.h
        main.cpp
        stb_image.cpp
        stb_image.h
        ui/ui.effect.cpp
        ui/ui.effect.h
        ui/ui.filter.cpp
        ui/ui.filter.h
        ui/ui.library.cpp
        ui/ui.library.h
        ui/ui.mixer.cpp
        ui/ui.mixer.h
        ui/ui.sequencer.cpp
        ui/ui.sequencer.h
        ui/ui.steppattern.cpp
        ui/ui.steppattern.h
        )

    target_include_directories(app.drummer
        PRIVATE
            ${CMAKE_SOURCE_DIR}
            ${GLFW3_INCLUDE_DIR}
            ${GLM_DIR}
            imgui
            include
        )

    target_link_libraries(app.drummer
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
        zyn.serialization
        zyn.common
        )

    target_compile_definitions(app.drummer
        PRIVATE -DIMGUI_IMPL_OPENGL_LOADER_GLAD
        )

    target_compile_features(app.drummer
        PRIVATE cxx_auto_type
        PRIVATE cxx_nullptr
        PRIVATE cxx_range_for
        PRIVATE cxx_thread_local
        )

endif(GLFW3_FOUND)
