cmake_minimum_required(VERSION 2.8)

project(zyn.seq)

set(zyn_seq_SRCS
    Sequencer.cpp
    SequencerStrip.cpp
    SequencerStep.cpp
    )

set(zyn_seq_HDRS
    Sequencer.h
    SequencerStrip.h
    SequencerStep.h
    )

include_directories(${CMAKE_SOURCE_DIR})

add_library(zyn.seq STATIC
    ${zyn_seq_SRCS}
    ${zyn_seq_HDRS}
    )

target_link_libraries(zyn.seq
    zyn.common
    )

target_compile_features(zyn.seq
    PRIVATE cxx_thread_local
    )

install(FILES ${zyn_seq_HDRS} DESTINATION include/zyn/seq COMPONENT develop)
install(TARGETS zyn.seq DESTINATION lib COMPONENT develop)
