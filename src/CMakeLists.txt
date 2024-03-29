set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)

# Make this a GUI application on Windows
if(WIN32)
  set(CMAKE_WIN32_EXECUTABLE ON)
  set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/icon.rc")
endif()

find_package(Qt5 5.8 REQUIRED COMPONENTS
    Svg SerialPort
)
find_package(QtLab REQUIRED Core IO Widgets Hamamatsu NI PI PI-Widgets)
find_package(Spinnaker REQUIRED)

option(FORCE_FUSION_STYLE "Force fusion style" OFF)
option(DEMO_MODE "Demo mode" OFF)

if (FORCE_FUSION_STYLE)
    add_definitions(-DFORCE_FUSION_STYLE)
    message(AUTHOR_WARNING "Forcing fusion style")
endif ()

if (DEMO_MODE)
    add_definitions(-DDEMO_MODE)
endif()

set(SOURCE_FILES
    resources.qrc

    main.cpp
    mainwindow.cpp
    settings.cpp
    version.cpp

    centralwidget.cpp
    controlswidget.cpp
    behavworker.cpp
    elreadoutworker.cpp
    displayworker.cpp
    savestackworker.cpp
    mainpage.cpp
    settingspage.cpp
    ddsdialog.cpp
    camdisplay.cpp

    optrode.cpp
    tasks.cpp
    chameleoncamera.cpp
    dds.cpp
)

add_executable(OptroLab ${SOURCE_FILES} ${APP_ICON_RESOURCE_WINDOWS})
target_include_directories(OptroLab PUBLIC ${Spinnaker_INCLUDE_DIR})

target_link_libraries(OptroLab
    Qt5::Svg
    Qt5::SerialPort
    QtLab::Core
    QtLab::IO
    QtLab::NI
    QtLab::PI
    QtLab::PI-Widgets
    QtLab::Hamamatsu
    ${Spinnaker_LIBRARY}
    ${SpinVideo_LIBRARY}
)
