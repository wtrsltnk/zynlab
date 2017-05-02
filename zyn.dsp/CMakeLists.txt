cmake_minimum_required(VERSION 2.8)

project(zyn.dsp)

set(zyn_dsp_SRCS
    AnalogFilter.cpp
    Filter.cpp
    FilterParams.cpp
    FormantFilter.cpp
    SVFilter.cpp
    Unison.cpp
)

set(zyn_dsp_HDRS
    AnalogFilter.h
    Filter.h
    FilterParams.h
    FormantFilter.h
    SVFilter.h
    Unison.h
)

add_library(zyn.dsp STATIC
    ${zyn_dsp_SRCS}
    ${zyn_dsp_HDRS}
    )

target_link_libraries(zyn.dsp
    zyn.common
    )

install(FILES ${zyn_dsp_HDRS} DESTINATION include/zyn/dsp COMPONENT develop)
install(TARGETS zyn.dsp DESTINATION lib COMPONENT develop)
