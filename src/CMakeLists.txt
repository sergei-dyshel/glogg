set(SOURCES
    session.cpp
    data/abstractlogdata.cpp
    data/logdata.cpp
    data/logfiltereddata.cpp
    data/logfiltereddataworkerthread.cpp
    data/logdataworkerthread.cpp
    data/compressedlinestorage.cpp
    mainwindow.cpp
    crawlerwidget.cpp
    abstractlogview.cpp
    logmainview.cpp
    filteredview.cpp
    optionsdialog.cpp
    persistentinfo.cpp
    configuration.cpp
    filtersdialog.cpp
    filterset.cpp
    savedsearches.cpp
    infoline.cpp
    menuactiontooltipbehavior.cpp
    selection.cpp
    quickfind.cpp
    quickfindpattern.cpp
    quickfindwidget.cpp
    sessioninfo.cpp
    recentfiles.cpp
    overview.cpp
    overviewwidget.cpp
    marks.cpp
    quickfindmux.cpp
    signalmux.cpp
    tabbedcrawlerwidget.cpp
    viewtools.cpp
    encodingspeculator.cpp
    gloggapp.cpp
    regexp_filter.cpp
    externalcom.cpp
    filewatcher.cpp
    struct_config_store.cpp
    settings.cpp
    qt_utils.cpp
    highlights.cpp
    tab_bar.cpp
    tab_info.cpp
)

set(COMMON_SRC
    log.cpp
    exception.cpp
)

set(SYNTAX_SRC
    colorizer.cpp
    config_node.cpp
    struct_config.cpp
    syntax.cpp
    color_scheme.cpp
    location.cpp
    struct_stream.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(UI optionsdialog.ui filtersdialog.ui)

# inotify/kqueue
if(UNIX AND NOT APPLE)
    list(APPEND SOURCES platformfilewatcher.cpp inotifywatchtowerdriver.cpp
        watchtower.cpp watchtowerlist.cpp)
    string(APPEND CMAKE_CXX_FLAGS " -DGLOGG_SUPPORTS_INOTIFY")
elseif(APPLE)
    list(APPEND SOURCES platformfilewatcher.cpp kqueuewatchtowerdriver.cpp
        watchtower.cpp watchtowerlist.cpp)
    string(APPEND CMAKE_CXX_FLAGS " -DGLOGG_SUPPORTS_KQUEUE")
endif()

# dbus/ipc
if(USE_DBUS)
    list(APPEND SOURCES dbusexternalcom.cpp)
elseif(APPLE)
    list(APPEND SOURCES socketexternalcom.cpp)
endif()

add_library(glogg_common
    ${COMMON_SRC}
)
target_link_libraries(glogg_common
    Qt5::Widgets
)

add_library(glogg_core
    ${SOURCES}
    ${UI}
)

target_link_libraries(glogg_core
    glogg_common ${LIBS} pthread Qt5::Widgets Boost::program_options
)

add_library(glogg_syntax
    ${SYNTAX_SRC}
)

target_link_libraries(glogg_syntax
    glogg_common yaml-cpp Qt5::Widgets
)
