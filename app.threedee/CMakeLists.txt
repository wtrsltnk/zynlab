cmake_minimum_required(VERSION 3.12)

project(app.threedee)

find_package(GLFW3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLM REQUIRED)

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
        ${GLFW3_INCLUDE_DIR}
        ${GLM_DIR}
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
    zyn.serialization
    zyn.common
    lib.imgui
    )

target_compile_features(app.threedee
    PRIVATE cxx_auto_type
    PRIVATE cxx_nullptr
    PRIVATE cxx_range_for
    PRIVATE cxx_thread_local
    )

if(WIN32)
    find_file(FFTW_BIN_PATH
        NAME "libfftw3-3.dll"
        PATHS ${CMAKE_PREFIX_PATH}/bin)

    configure_file(${FFTW_BIN_PATH} ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

    if(PaEnable)
        find_file(PORTAUDIO_BIN_PATH
            NAME "libportaudio.dll" "libportaudio_x86.dll"
            PATHS ${CMAKE_PREFIX_PATH}/bin)

        configure_file(${PORTAUDIO_BIN_PATH} ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
    endif(PaEnable)
endif(WIN32)
