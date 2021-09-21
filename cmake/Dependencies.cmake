
CPMAddPackage(
    NAME zlib
    GITHUB_REPOSITORY "madler/zlib"
    GIT_TAG v1.2.11
    GIT_SHALLOW ON
#    OPTIONS
#        "ASM686 Off"
#        "AMD64 On"
)

CPMAddPackage(
    NAME fftw3
    VERSION 3.3.5
    URL ftp://ftp.fftw.org/pub/fftw/fftw-3.3.5-dll64.zip
)

if (fftw3_ADDED)
    get_filename_component(_vs_bin_path "${CMAKE_LINKER}" DIRECTORY)

    if(MINGW)
        if(NOT EXISTS "${fftw3_SOURCE_DIR}/libfftw3-3.lib")
            execute_process(RESULT_VARIABLE LIB_RESULT
                COMMAND "${_vs_bin_path}/dlltool.exe" --def libfftw3-3.def --dllname libfftw3-3.dll --output-lib libfftw3-3.lib
                WORKING_DIRECTORY ${fftw3_SOURCE_DIR})
        endif()

        if(NOT EXISTS "${fftw3_SOURCE_DIR}/libfftw3f-3.lib")
            execute_process(RESULT_VARIABLE LIB_F_RESULT
                COMMAND "${_vs_bin_path}/dlltool.exe" --def libfftw3f-3.def --dllname libfftw3f-3.dll --output-lib libfftw3f-3.lib
                WORKING_DIRECTORY ${fftw3_SOURCE_DIR})
        endif()

        if(NOT EXISTS "${fftw3_SOURCE_DIR}/libfftw3l-3.lib")
            execute_process(RESULT_VARIABLE LIB_L_RESULT
                COMMAND "${_vs_bin_path}/dlltool.exe" --def libfftw3l-3.def --dllname libfftw3l-3.dll --output-lib libfftw3l-3.lib
                WORKING_DIRECTORY ${fftw3_SOURCE_DIR})
        endif()
    elseif(MSVC)
        if(NOT EXISTS "${fftw3_SOURCE_DIR}/libfftw3-3.lib")
            execute_process(RESULT_VARIABLE LIB_RESULT
                COMMAND "${_vs_bin_path}/lib.exe" /machine:x64 /def:libfftw3-3.def
                WORKING_DIRECTORY ${fftw3_SOURCE_DIR})
        endif()

        if(NOT EXISTS "${fftw3_SOURCE_DIR}/libfftw3f-3.lib")
            execute_process(RESULT_VARIABLE LIB_RESULT
                COMMAND "${_vs_bin_path}/lib.exe" /machine:x64 /def:libfftw3f-3.def
                WORKING_DIRECTORY ${fftw3_SOURCE_DIR})
        endif()

        if(NOT EXISTS "${fftw3_SOURCE_DIR}/libfftw3l-3.lib")
            execute_process(RESULT_VARIABLE LIB_RESULT
                COMMAND "${_vs_bin_path}/lib.exe" /machine:x64 /def:libfftw3l-3.def
                WORKING_DIRECTORY ${fftw3_SOURCE_DIR})
        endif()
     endif()

    add_library(fftw3 INTERFACE)

    target_link_libraries(fftw3
        INTERFACE
            "${fftw3_SOURCE_DIR}/libfftw3-3.lib"
            "${fftw3_SOURCE_DIR}/libfftw3f-3.lib"
            "${fftw3_SOURCE_DIR}/libfftw3l-3.lib"
            "${fftw3_SOURCE_DIR}/libfftw3-3.dll"
            "${fftw3_SOURCE_DIR}/libfftw3f-3.dll"
            "${fftw3_SOURCE_DIR}/libfftw3l-3.dll")

    find_file(fftw3_HEADER_FILE_PATH "fftw3.h" HINT ${fftw3_SOURCE_DIR})
    get_filename_component(fftw3_HEADER_FILE_LOCATION ${fftw3_HEADER_FILE_PATH} DIRECTORY)

    target_include_directories(fftw3
        INTERFACE
            $<BUILD_INTERFACE:${fftw3_HEADER_FILE_LOCATION}>
            $<INSTALL_INTERFACE:include>)

endif()

if(BUILD_APP_THREEDEE)

    CPMAddPackage(
        NAME glfw
        GITHUB_REPOSITORY "glfw/glfw"
        GIT_TAG 3.3.4
        GIT_SHALLOW ON
        OPTIONS
            "GLFW_BUILD_EXAMPLES Off"
            "GLFW_BUILD_TESTS Off"
            "GLFW_BUILD_DOCS Off"
            "GLFW_INSTALL Off"
    )

endif()

#if (BUILD_APP_ZYNADSUBFX)
#    CPMAddPackage(
#        NAME fltk
#        GITHUB_REPOSITORY "fltk/fltk"
#        GIT_TAG release-1.3.7
#        GIT_SHALLOW ON
#        OPTIONS
#            "FLTK_BUILD_TEST OFF"
#            "OPTION_BUILD_EXAMPLES OFF"
#            "OPTION_BUILD_HTML_DOCUMENTATION OFF"
#            "OPTION_INSTALL_HTML_DOCUMENTATION OFF"
#    )

#    if (fltk_ADDED)
#        find_file(fltk_FLUID_EXECUTABLE "fluid.exe"
#            NO_DEFAULT_PATH
#            NO_CMAKE_SYSTEM_PATH
#            NO_CACHE
#            PATHS
#                ${fltk_BINARY_DIR}
#                ${fltk_BINARY_DIR}/bin
#            )

#        message(STATUS "fltk_FLUID_EXECUTABLE = ${fltk_FLUID_EXECUTABLE}")

#        find_file(fltk_HEADER_FILE "FL/FL.h"
#            NO_DEFAULT_PATH
#            NO_CMAKE_SYSTEM_PATH
#            NO_CACHE
#            PATHS
#                ${fltk_SOURCE_DIR}
#                ${fltk_SOURCE_DIR}/include
#            )
#        get_filename_component(fltk_HEADER_FILE_LOCATION ${fltk_HEADER_FILE} DIRECTORY)

#        message(STATUS "fltk_HEADER_FILE = ${fltk_HEADER_FILE}")

#        target_include_directories(fltk
#            INTERFACE
#                $<BUILD_INTERFACE:${fltk_HEADER_FILE_LOCATION}>)
#    endif()
#endif(BUILD_APP_ZYNADSUBFX)

CPMAddPackage(
    NAME portaudio
    GITHUB_REPOSITORY "PortAudio/portaudio"
    GIT_TAG v19.7.0
    GIT_SHALLOW ON
    OPTIONS
        "PA_BUILD_TESTS Off"
        "PA_BUILD_EXAMPLES Off"
)

CPMAddPackage(
  NAME glm
  GITHUB_REPOSITORY g-truc/glm
  GIT_TAG 0.9.9.7
)
