set(Spinnaker_LIBNAME_STR "FLIR Spinnaker SDK")
set(Spinnaker_LIBRARY_NAMES Spinnaker Spinnaker_v140)

set(WIN_Spinnaker_DIR "$ENV{ProgramFiles}/FLIR Systems/Spinnaker")

find_library(Spinnaker_LIBRARY
    NAMES ${Spinnaker_LIBRARY_NAMES}
    PATH_SUFFIXES vs2015
    PATHS
    /usr/lib/x86_64-linux-gnu
    /usr/lib
    /usr/local/lib
    "$ENV{LIB_DIR}/lib"
    "$ENV{LIB}"
    "${WIN_Spinnaker_DIR}/lib64"
)

find_library(SpinVideo_LIBRARY
    NAMES SpinVideo SpinVideo_v140
	PATH_SUFFIXES vs2015
    PATHS
    /usr/lib/x86_64-linux-gnu
    /usr/lib
    /usr/local/lib
    "$ENV{LIB_DIR}/lib"
    "$ENV{LIB}"
    "${WIN_Spinnaker_DIR}/lib64"
)

find_path(Spinnaker_INCLUDE_DIR NAMES Spinnaker.h PATHS
    "${INCLUDE_DIRS}"
    /usr/include
    /usr/include/spinnaker
    /usr/local/include
    "$ENV{LIB_DIR}/include"
    "$ENV{INCLUDE}"
    "${WIN_Spinnaker_DIR}/include"
)

if (Spinnaker_INCLUDE_DIR)
    message(STATUS "Found ${Spinnaker_LIBNAME_STR} headers in: ${Spinnaker_INCLUDE_DIR}")
endif ()

if (Spinnaker_INCLUDE_DIR AND Spinnaker_LIBRARY)
    set(Spinnaker_FOUND TRUE)
    message(STATUS "Found ${Spinnaker_LIBNAME_STR}: ${Spinnaker_LIBRARY} ${SpinVideo_LIBRARY}")
endif ()

if (NOT Spinnaker_FOUND AND Spinnaker_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find ${Spinnaker_LIBNAME_STR}")
endif ()
