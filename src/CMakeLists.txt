set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)

find_package(Qt5 5.8 REQUIRED COMPONENTS
    Widgets
)
find_package(QtLab REQUIRED Core Widgets Hamamatsu NI)
find_package(Spinnaker REQUIRED)

option(FORCE_FUSION_STYLE "Force fusion style" OFF)
option(DEMO_MODE "Demo mode" OFF)

if (FORCE_FUSION_STYLE)
    add_definitions(-DFORCE_FUSION_STYLE)
    message(AUTHOR_WARNING "Forcing fusion style")
endif ()

if (NOT DEMO_MODE)
    add_definitions(-DWITH_HARDWARE)
endif()

set(SOURCE_FILES
    resources.qrc

    main.cpp
    mainwindow.cpp
    settings.cpp
    version.cpp

    centralwidget.cpp
    controlswidget.cpp
    behavdispworker.cpp
    mainpage.cpp

    optrode.cpp
    tasks.cpp
    chameleoncamera.cpp
)

add_executable(OptroLab ${SOURCE_FILES})
target_include_directories(OptroLab PUBLIC ${Spinnaker_INCLUDE_DIR})

target_link_libraries(OptroLab
    Qt5::Widgets
    QtLab::Core
    QtLab::Widgets
    QtLab::NI
    QtLab::Hamamatsu
    ${Spinnaker_LIBRARY}
    ${SpinVideo_LIBRARY}
)
