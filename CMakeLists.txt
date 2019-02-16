cmake_minimum_required(VERSION 3.1.0)

project(glogg)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Concurrent REQUIRED)
find_package(Qt5Network REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(src)

set(SOURCES
    src/main.cpp
    src/session.cpp
    src/data/abstractlogdata.cpp
    src/data/logdata.cpp
    src/data/logfiltereddata.cpp
    src/data/logfiltereddataworkerthread.cpp
    src/data/logdataworkerthread.cpp
    src/data/compressedlinestorage.cpp
    src/mainwindow.cpp
    src/crawlerwidget.cpp
    src/abstractlogview.cpp
    src/logmainview.cpp
    src/filteredview.cpp
    src/optionsdialog.cpp
    src/persistentinfo.cpp
    src/configuration.cpp
    src/filtersdialog.cpp
    src/filterset.cpp
    src/savedsearches.cpp
    src/infoline.cpp
    src/menuactiontooltipbehavior.cpp
    src/selection.cpp
    src/quickfind.cpp
    src/quickfindpattern.cpp
    src/quickfindwidget.cpp
    src/sessioninfo.cpp
    src/recentfiles.cpp
    src/overview.cpp
    src/overviewwidget.cpp
    src/marks.cpp
    src/quickfindmux.cpp
    src/signalmux.cpp
    src/tabbedcrawlerwidget.cpp
    src/viewtools.cpp
    src/encodingspeculator.cpp
    src/gloggapp.cpp
    src/regexp_filter.cpp
    src/color_scheme.cpp
    src/colorizer.cpp
    src/config_node.cpp
    src/struct_config.cpp
    src/syntax.cpp
    src/log.cpp
    src/exception.cpp
    src/enum.cpp
    src/externalcom.cpp
    src/filewatcher.cpp
)

# Boost
find_package(Boost REQUIRED COMPONENTS program_options)
include_directories(${Boost_INCLUDE_DIRS})

# yaml-cpp
SET(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
SET(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
SET(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
add_subdirectory(3rdparty/yaml-cpp EXCLUDE_FROM_ALL)
list(APPEND LIBS yaml-cpp)

# inotify
list(APPEND SOURCES src/platformfilewatcher.cpp src/inotifywatchtowerdriver.cpp
    src/watchtower.cpp src/watchtowerlist.cpp)
string(APPEND CMAKE_CXX_FLAGS " -DGLOGG_SUPPORTS_INOTIFY")

# dbus
find_package(Qt5DBus REQUIRED)
list(APPEND SOURCES src/dbusexternalcom.cpp)
string(APPEND CMAKE_CXX_FLAGS " -DGLOGG_SUPPORTS_DBUS")
list(APPEND LIBS Qt5::DBus)

string(APPEND CMAKE_CXX_FLAGS " -Wall -pedantic -Wextra -Werror")

execute_process(COMMAND git describe OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE VERSION)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGLOGG_VERSION=\\\"${VERSION}\\\"")

set(UI src/optionsdialog.ui src/filtersdialog.ui)
set(RESOURCES glogg.qrc)

set(TARGET ${CMAKE_PROJECT_NAME})
add_executable(${TARGET}
    ${SOURCES}
    ${UI}
    ${RESOURCES}
)

target_link_libraries(${TARGET} ${LIBS} pthread Qt5::Widgets Boost::program_options)

install(TARGETS ${TARGET} RUNTIME DESTINATION bin ${CMAKE_INSTALL_BINDIR})
install(FILES images/hicolor/16x16/glogg.png DESTINATION share/icons/hicolor/16x16/apps)
install(FILES images/hicolor/32x32/glogg.png DESTINATION share/icons/hicolor/32x32/apps)
install(FILES images/hicolor/scalable/glogg.svg DESTINATION share/icons/hicolor/scalable/apps)
install(FILES glogg.desktop DESTINATION share/applications)