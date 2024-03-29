cmake_minimum_required(VERSION 3.12)

project(app.threedee)

find_package(OpenGL REQUIRED)

add_executable(app.threedee
    app.threedee.cpp
    app.threedee.h
    appstate.cpp
    appstate.h
    instrumentcategories.h
    main.cpp
    stb_image.cpp
    stb_image.h
    ui/microtonal.cpp
    ui/resonance.cpp
    ui/ui.adnote.cpp
    ui/ui.adnote.h
    ui/ui.adnote.voice.cpp
    ui/ui.dialogs.cpp
    ui/ui.dialogs.h
    ui/ui.effect.cpp
    ui/ui.effect.h
    ui/ui.envelope.cpp
    ui/ui.envelope.h
    ui/ui.filter.cpp
    ui/ui.filter.h
    ui/ui.lfo.cpp
    ui/ui.lfo.h
    ui/ui.library.cpp
    ui/ui.library.h
    ui/ui.mixer.cpp
    ui/ui.mixer.h
    ui/ui.oscilgen.cpp
    ui/ui.oscilgen.h
    ui/ui.padnote.cpp
    ui/ui.padnote.h
    ui/ui.subnote.cpp
    ui/ui.subnote.h
    ui/ui.samplenote.cpp
    ui/ui.samplenote.h
    )

target_include_directories(app.threedee
    PRIVATE
        ${CMAKE_SOURCE_DIR}
        include
    )

target_link_libraries(app.threedee
    ${ZLIB_LIBRARIES}
    ${OPENGL_LIBRARIES}
    glfw
    glm
    zyn.nio
    zyn.net
    zyn.mixer
    zyn.synth
    zyn.fx
    zyn.dsp
    zyn.seq
    zyn.serialization
    zyn.common
    lib.imgui
    )

target_compile_features(app.threedee
    PRIVATE cxx_auto_type
    PRIVATE cxx_nullptr
    PRIVATE cxx_range_for
    PRIVATE cxx_thread_local
    PRIVATE cxx_std_17
    )

if(MINGW)
    target_link_options(app.threedee
        PUBLIC -static -mwindows)
endif()
