#
# Set up PortAudio libraries for the sample app.
#
# To build with PortAudio, run the following command,
#     cmake <path-to-source> 
#       -DPORTAUDIO=ON 
#           -DPORTAUDIO_LIB_PATH=<path-to-portaudio-lib> 
#           -DPORTAUDIO_INCLUDE_DIR=<path-to-portaudio-include-dir>
#

option(PORTAUDIO "Enable PortAudio for the sample app." OFF)
set(PORTAUDIO_LIB_PATH "" CACHE FILEPATH "PortAudio library path.")
set(PORTAUDIO_INCLUDE_DIR "" CACHE PATH "PortAudio include directory.")

mark_as_dependent(PORTAUDIO_INCLUDE_DIR PORTAUDIO)
mark_as_dependent(PORTAUDIO_LIB_PATH PORTAUDIO)

if(OLLI_IMPL)
    find_package(PkgConfig)
    pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)
    message("Lib find ${PORTAUDIO_LDFLAGS}, ${PORTAUDIO_INCLUDE_DIRS}")
    set(PORTAUDIO ON)
    set(PORTAUDIO_LIB_PATH ${PORTAUDIO_LDFLAGS})
    set(PORTAUDIO_INCLUDE_DIR ${PORTAUDIO_INCLUDE_DIRS})
endif()

if(PORTAUDIO)
    if(NOT PORTAUDIO_LIB_PATH)
        message(FATAL_ERROR "Must pass library path of PortAudio to enable it.")
        return()
    endif()
    if(NOT PORTAUDIO_INCLUDE_DIR)
        message(FATAL_ERROR "Must pass include dir path of PortAudio to enable it.")
        return()
    endif()

    add_definitions("-DPORTAUDIO")
endif()
