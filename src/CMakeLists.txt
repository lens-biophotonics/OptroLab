set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)

find_package(Qt5 5.8 REQUIRED COMPONENTS
    Widgets
)
find_package(QtLab REQUIRED Core Widgets NI Hamamatsu)

option(FORCE_FUSION_STYLE "Force fusion style" OFF)

if (FORCE_FUSION_STYLE)
    add_definitions(-DFORCE_FUSION_STYLE)
    message(AUTHOR_WARNING "Forcing fusion style")
endif ()

set(SOURCE_FILES
    resources.qrc

    main.cpp
    mainwindow.cpp
    version.cpp
)

add_executable(OptroLab ${SOURCE_FILES})

target_link_libraries(OptroLab
    Qt5::Widgets
    QtLab::Core
    QtLab::NI
    QtLab::Hamamatsu
)
