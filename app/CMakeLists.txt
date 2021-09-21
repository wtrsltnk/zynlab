cmake_minimum_required(VERSION 3.12)

project(app)

find_package(FLTK)

add_executable(app
    main.cpp
    ui/common.cpp
    ui/common.h
    ui/NioUI.cpp
    ui/NioUI.H
    ui/WidgetPDial.cpp
    ui/WidgetPDial.h
    )

set(UI_fl_files
    ui/ADnoteUI.fl
    ui/BankUI.fl
    ui/ConfigUI.fl
    ui/EffUI.fl
    ui/EnvelopeUI.fl
    ui/FilterUI.fl
    ui/LFOUI.fl
    ui/MasterUI.fl
    ui/MicrotonalUI.fl
    ui/OscilGenUI.fl
    ui/PADnoteUI.fl
    ui/PartUI.fl
    ui/PresetsUI.fl
    ui/ResonanceUI.fl
    ui/SUBnoteUI.fl
    ui/VirKeyboard.fl
    #ui/SequencerUI.fl
)

set_source_files_properties(ui/MasterUI.h PROPERTIES GENERATED 1)

fltk_wrap_ui(app ${UI_fl_files})

    find_program (FLTK_CONFIG fltk-config)

    if (FLTK_CONFIG)
        execute_process (COMMAND ${FLTK_CONFIG} --use-images --ldflags OUTPUT_VARIABLE FLTK_LDFLAGS)

        if (FLTK_LDFLAGS)
                string(STRIP ${FLTK_LDFLAGS} FLTK_LIBRARIES)
        endif()
    endif()

target_sources(app
    PRIVATE ${app_FLTK_UI_SRCS}
    PRIVATE ${UI_fl_files}
    )

target_compile_features(app
    PRIVATE
        cxx_auto_type
        cxx_nullptr
        cxx_lambdas
        cxx_std_17
    )

target_include_directories(app
    PRIVATE
        ${FLTK_INCLUDE_DIR}
        "${CMAKE_CURRENT_SOURCE_DIR}/.."
        "${CMAKE_CURRENT_SOURCE_DIR}/ui"
        "${CMAKE_CURRENT_BINARY_DIR}"
        "../zyn.common"
    )

target_compile_definitions(app
    PRIVATE
        -DFLTK_GUI
    )

target_link_libraries(app
    ${FLTK_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${ZLIB_LIBRARIES}
    zyn.nio
    zyn.net
    zyn.mixer
    zyn.synth
    zyn.fx
    zyn.dsp
    zyn.seq
    zyn.serialization
    zyn.common)
