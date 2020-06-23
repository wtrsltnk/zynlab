cmake_minimum_required(VERSION 3.12)

project(app.tracker)

add_executable(app.tracker
    application.cpp
    application.h
    applicationsession.cpp
    applicationsession.h
    automationeditor.cpp
    automationeditor.h
    effectseditor.cpp
    effectseditor.h
    glad.c
    imgui/imgui.cpp
    imgui/imgui.h
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_impl_opengl3.cpp
    imgui/imgui_impl_opengl3.h
    imgui/imgui_impl_win32.cpp
    imgui/imgui_impl_win32.h
    imgui/imgui_internal.h
    imgui/imgui_widgets.cpp
    imgui_helpers.cpp
    imgui_helpers.h
    imgui_plot.cpp
    include/imgui_plot.h
    instrumentspanel.cpp
    instrumentspanel.h
    main.cpp
    note.cpp
    note.h
    pattern.cpp
    pattern.h
    patterneditor.cpp
    patterneditor.h
    patternsmanager.cpp
    patternsmanager.h
    playercontrolspanel.cpp
    playercontrolspanel.h
    song.cpp
    song.h
    syntheditor.cpp
    syntheditor.h
    )

target_include_directories(app.tracker
    PUBLIC include
    PUBLIC imgui
    PRIVATE ${CMAKE_SOURCE_DIR})

target_compile_features(app.tracker
    PRIVATE cxx_auto_type
    PRIVATE cxx_nullptr
    PRIVATE cxx_range_for
    )

target_link_libraries(app.tracker
    PRIVATE
        ${OPENGL_LIBRARIES}
        zyn.nio
        zyn.mixer
        zyn.synth
        zyn.serialization
        zyn.common)

target_compile_options(app.tracker
    PUBLIC 
        -DUNICODE
        -D_UNICODE
        -DIMGUI_IMPL_WIN32_DISABLE_GAMEPAD)

if(MINGW)
    target_link_options(app.tracker
        PUBLIC -static -mwindows -municode)
endif()

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

if(MINGW)
    target_link_options(app.tracker
        PUBLIC -static -mwindows)
endif()
