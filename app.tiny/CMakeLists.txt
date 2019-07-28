cmake_minimum_required(VERSION 3.8)

project(app.tiny)

find_package(GLFW3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLM REQUIRED)

add_executable(app.tiny
    main.cpp
    app.tiny.cpp
    app.tiny.h
    glfwapplication.cpp
    glfwapplication.h
    glad.c
    )

target_include_directories(app.tiny
    PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${GLFW3_INCLUDE_DIR}
        ${GLM_DIR}
        include
    )

target_link_libraries(app.tiny
    ${ZLIB_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLFW3_LIBRARY}
    zyn.nio
    zyn.net
    zyn.mixer
    zyn.synth
    zyn.fx
    zyn.dsp
    zyn.serialization
    zyn.common
    )

target_compile_definitions(app.tiny
    PRIVATE -DIMGUI_IMPL_OPENGL_LOADER_GLAD
    )

target_compile_features(app.tiny
    PRIVATE cxx_auto_type
    PRIVATE cxx_nullptr
    PRIVATE cxx_range_for
    PRIVATE cxx_thread_local
    )
