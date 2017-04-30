cmake_minimum_required(VERSION 2.8)

project(app)

if(FltkGui)
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
        ui/SequencerUI.fl
	)

set_source_files_properties(ui/MasterUI.h PROPERTIES GENERATED 1)
fltk_wrap_ui(app ${UI_fl_files})

	#UGLY WORKAROUND
	find_program (FLTK_CONFIG fltk-config)
	if (FLTK_CONFIG)
		execute_process (COMMAND ${FLTK_CONFIG} --use-images --ldflags OUTPUT_VARIABLE FLTK_LDFLAGS)
		if (FLTK_LDFLAGS)
			string(STRIP ${FLTK_LDFLAGS} FLTK_LIBRARIES)
		endif()
	endif()

	set(GUI_LIBRARIES ${FLTK_LIBRARIES} ${OPENGL_LIBRARIES})

	add_definitions(-DFLTK_GUI)
	message(STATUS "Will build FLTK gui")

	include_directories(
		${FLTK_INCLUDE_DIR}
		"${CMAKE_CURRENT_SOURCE_DIR}/ui"
		"${CMAKE_CURRENT_BINARY_DIR}"
		)
endif()

set(app_SRCS
    main.cpp
    )

set(app_HDRS
    )

add_executable(app
    ${app_SRCS}
    ${app_HDRS}
    ${UI_objs}
    ${app_FLTK_UI_SRCS}
    ${UI_fl_files}
    ui/NioUI.cpp
    ui/WidgetPDial.cpp
	)

target_compile_features(app
    PRIVATE cxx_auto_type
    PRIVATE cxx_nullptr
    PRIVATE cxx_lambdas
    )

target_link_libraries(app
    ${GUI_LIBRARIES}
    ${ZLIB_LIBRARIES}
    zyn.nio
    zyn.mixer
    zyn.synth
    zyn.fx
    zyn.dsp
    zyn.common
    zyn.seq
    )
