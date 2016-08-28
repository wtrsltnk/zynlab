cmake_minimum_required(VERSION 2.8)

project(app.threedee)

find_package(GLFW3 REQUIRED)
find_package(OpenGL REQUIRED)

set(app_threedee_SRCS
	main.cpp
	app.threedee.cpp
    )

set(app_threedee_HDRS
	app.threedee.h
    )

add_executable(app.threedee
    ${app_threedee_SRCS}
    ${app_threedee_HDRS}
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
