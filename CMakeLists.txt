cmake_minimum_required(VERSION 3.1.0)

project(OptroLab)

include_directories(${INCLUDE_DIRS})
message(STATUS "Custom include directories: ${INCLUDE_DIRS}")

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

set(OTHER_FILES
    .gitignore
    uncrustify.cfg
)

add_custom_target(project-related-files SOURCES ${OTHER_FILES})

add_subdirectory(src)
