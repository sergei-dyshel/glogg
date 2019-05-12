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

# Boost
find_package(Boost REQUIRED COMPONENTS program_options)
include_directories(${Boost_INCLUDE_DIRS})

# yaml-cpp
SET(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
SET(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
SET(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
add_subdirectory(3rdparty/yaml-cpp EXCLUDE_FROM_ALL)
list(APPEND LIBS yaml-cpp)

# google re2
SET(RE2_BUILD_TESTING OFF CACHE BOOL "" FORCE)
add_subdirectory(3rdparty/re2 EXCLUDE_FROM_ALL)
include_directories(3rdparty/re2)
# list(APPEND LIBS re2)

# google bench
SET(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
SET(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(3rdparty/googlebench EXCLUDE_FROM_ALL)

# dbus
find_package(Qt5DBus REQUIRED)
string(APPEND CMAKE_CXX_FLAGS " -DGLOGG_SUPPORTS_DBUS")
list(APPEND LIBS Qt5::DBus)

execute_process(COMMAND git describe OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE VERSION)
string(APPEND CMAKE_CXX_FLAGS " -DGLOGG_VERSION=\\\"${VERSION}\\\"")

string(APPEND CMAKE_CXX_FLAGS " -Wall -pedantic -Wextra -Werror")

# debug
option(BUILD_TESTS "Compile integration and unit-tests")
string(APPEND CMAKE_CXX_FLAGS_DEBUG " -DENABLE_TRACE")
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

add_subdirectory(src)

include_directories(src)

add_executable(glogg
    src/main.cpp
    glogg.qrc
)

target_link_libraries(glogg ${LIBS} glogg_core glogg_syntax pthread Qt5::Widgets Boost::program_options)

add_executable(bench
    src/bench.cpp
)

target_link_libraries(bench benchmark re2 Qt5::Widgets)

add_executable(bench_grep
    src/bench_grep.cpp
)

target_link_libraries(bench_grep re2 Qt5::Widgets pthread)

include(GNUInstallDirs)

install(TARGETS glogg RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY config DESTINATION share/glogg)
install(FILES colors.schema.json syntax.schema.json DESTINATION share/glogg/schemas)
install(FILES images/hicolor/16x16/glogg.png DESTINATION share/icons/hicolor/16x16/apps)
install(FILES images/hicolor/32x32/glogg.png DESTINATION share/icons/hicolor/32x32/apps)
install(FILES images/hicolor/scalable/glogg.svg DESTINATION share/icons/hicolor/scalable/apps)
install(FILES glogg.desktop DESTINATION share/applications)