cmake_minimum_required(VERSION 3.8)

project(tests)

add_executable(tests
    common-tests.cpp
    catch.cpp
    )

target_include_directories(tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}
        include
    )

target_compile_features(tests
    PRIVATE cxx_auto_type
    PRIVATE cxx_nullptr
    PRIVATE cxx_lambdas
    )

target_compile_definitions(tests
    PRIVATE
        TEST_DATA_PATH="${CMAKE_CURRENT_SOURCE_DIR}/test-data/"
    )

target_link_libraries(tests
    ${ZLIB_LIBRARIES}
    zyn.nio
    zyn.net
    zyn.mixer
    zyn.synth
    zyn.fx
    zyn.dsp
    zyn.common
    )

