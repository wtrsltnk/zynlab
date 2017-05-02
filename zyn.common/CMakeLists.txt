
cmake_minimum_required(VERSION 2.8)

project(zyn.common)

set(zyn_common_SRCS
    Config.cpp
    Presets.cpp
    PresetsArray.cpp
    PresetsStore.cpp
    Stereo.cpp
    Util.cpp
    WaveShapeSmps.cpp
    WavFile.cpp
    XmlWrapper.cpp
)

set(zyn_common_HDRS
    Config.h
    globals.h
    Presets.h
    PresetsArray.h
    PresetsStore.h
    Stereo.h
    Util.h
    WaveShapeSmps.h
    WavFile.h
    XmlWrapper.h
)

add_library(zyn.common STATIC
    ${zyn_common_SRCS}
    ${zyn_common_HDRS}
    )

target_include_directories(zyn.common
    PRIVATE ${MXML_INCLUDE_DIRS}
    )

target_link_libraries(zyn.common
    ${MXML_LIBRARIES}
	${ZLIB_LIBRARIES}
    z
    )

target_compile_features(zyn.common
    PRIVATE cxx_auto_type
    PRIVATE cxx_nullptr
    )

install(FILES ${zyn_common_HDRS} DESTINATION include/zyn/common COMPONENT develop)
install(TARGETS zyn.common DESTINATION lib COMPONENT develop)
