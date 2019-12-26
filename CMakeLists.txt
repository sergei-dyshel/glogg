cmake_minimum_required(VERSION 3.1.0)

option(USE_HOMEBREW_LLVM "Use Homebrew-provided LLVM toolchain instead of Apple's" OFF)

if(APPLE AND USE_HOMEBREW_LLVM)
    execute_process(
        COMMAND brew --prefix llvm
        OUTPUT_VARIABLE llvm_prefix
        RESULT_VARIABLE homebrew_llvm_present
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if (NOT homebrew_llvm_present EQUAL 0)
        message(FATAL_ERROR "Could not find LLVM installed with Homebrew")
    endif()

    message(STATUS "Using LLVM installed with Homebrew...")
    include_directories(SYSTEM ${llvm_prefix}/include)
    link_directories(${llvm_prefix}/lib)
    set(CMAKE_C_COMPILER ${llvm_prefix}/bin/clang)
    set(CMAKE_CXX_COMPILER ${llvm_prefix}/bin/clang++)
    set(CMAKE_LINKER ${llvm_prefix}/bin/ld.ldd)
    set(CMAKE_AR ${llvm_prefix}/bin/llvm-ar)
    set(CMAKE_RANLIB ${llvm_prefix}/bin/llvm-ranlib)
endif()

project(glogg)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Concurrent REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Svg REQUIRED)

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


# dbus
option(USE_DBUS "Use D-Bus for interprocess communication" ON)
if(USE_DBUS)
    find_package(Qt5DBus REQUIRED)
    string(APPEND CMAKE_CXX_FLAGS " -DGLOGG_SUPPORTS_DBUS")
    list(APPEND LIBS Qt5::DBus)
elseif(APPLE)
    string(APPEND CMAKE_CXX_FLAGS " -DGLOGG_SUPPORTS_SOCKETIPC")
    list(APPEND LIBS Qt5::Network)
endif()

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

include(GNUInstallDirs)

install(TARGETS glogg RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY config DESTINATION share/glogg)
install(FILES colors.schema.json syntax.schema.json DESTINATION share/glogg/schemas)
install(FILES images/hicolor/16x16/glogg.png DESTINATION share/icons/hicolor/16x16/apps)
install(FILES images/hicolor/32x32/glogg.png DESTINATION share/icons/hicolor/32x32/apps)
install(FILES images/hicolor/scalable/glogg.svg DESTINATION share/icons/hicolor/scalable/apps)
install(FILES glogg.desktop DESTINATION share/applications)
