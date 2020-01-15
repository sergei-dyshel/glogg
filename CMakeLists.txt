cmake_minimum_required(VERSION 3.1.0)

option(USE_HOMEBREW_LLVM "Use Homebrew-provided LLVM toolchain instead of Apple's" ON)

if(APPLE AND USE_HOMEBREW_LLVM)
    execute_process(
        COMMAND brew --prefix llvm
        OUTPUT_VARIABLE llvm_prefix
        RESULT_VARIABLE homebrew_llvm_present
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if (homebrew_llvm_present EQUAL 0)
        message("Using LLVM installed with Homebrew...")
        include_directories(SYSTEM ${llvm_prefix}/include)
        link_directories(${llvm_prefix}/lib)
        set(CMAKE_C_COMPILER ${llvm_prefix}/bin/clang)
        set(CMAKE_CXX_COMPILER ${llvm_prefix}/bin/clang++)
        set(CMAKE_LINKER ${llvm_prefix}/bin/ld.ldd)
        set(CMAKE_AR ${llvm_prefix}/bin/llvm-ar)
        set(CMAKE_RANLIB ${llvm_prefix}/bin/llvm-ranlib)
    endif()
endif()

project(glogg)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

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
# TEMPORARY: see https://github.com/boostorg/boost_install/issues/13
set(Boost_NO_BOOST_CMAKE ON)
find_package(Boost REQUIRED COMPONENTS program_options)
include_directories(${Boost_INCLUDE_DIRS})

# yaml-cpp
SET(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
SET(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
SET(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
set(cxx_flags_backup "${CMAKE_CXX_FLAGS}")
string(APPEND CMAKE_CXX_FLAGS " -Wno-shadow")
add_subdirectory(3rdparty/yaml-cpp EXCLUDE_FROM_ALL)
set(CMAKE_CXX_FLAGS "${cxx_flags_backup}")
list(APPEND LIBS yaml-cpp)

# dbus
include(CMakeDependentOption)
CMAKE_DEPENDENT_OPTION(USE_DBUS "Use D-Bus for interprocess communication" OFF "APPLE" ON)
if(USE_DBUS)
    message("Using DBus-based IPC")
    find_package(Qt5DBus REQUIRED)
    string(APPEND CMAKE_CXX_FLAGS " -DGLOGG_SUPPORTS_DBUS")
    list(APPEND LIBS Qt5::DBus)
elseif(APPLE)
    message("Using socket-based IPC")
    string(APPEND CMAKE_CXX_FLAGS " -DGLOGG_SUPPORTS_SOCKETIPC")
    list(APPEND LIBS Qt5::Network)
endif()

execute_process(COMMAND git describe OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE VERSION)
string(APPEND CMAKE_CXX_FLAGS " -DGLOGG_VERSION=\\\"${VERSION}\\\"")

string(APPEND CMAKE_CXX_FLAGS " -Wall -pedantic -Wextra -Werror")
string(APPEND CMAKE_CXX_FLAGS " -Wno-c99-extensions")

# debug
option(BUILD_TESTS "Compile integration and unit-tests")
string(APPEND CMAKE_CXX_FLAGS_DEBUG " -DENABLE_TRACE")
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

option(MAC_BUNDLE "Build application bundle on MacOS" ON)

add_subdirectory(src)

include_directories(src)


if (APPLE AND MAC_BUNDLE)
    set(OS_BUNDLE MACOSX_BUNDLE)
    set(ICON images/glogg.icns)
    set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/output")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIR}")

    set(BUNDLE_DIR ${OUTPUT_DIR}/${PROJECT_NAME}.app)

    set_source_files_properties(${ICON} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
endif()

add_executable(glogg
    ${OS_BUNDLE}
    src/main.cpp
    glogg.qrc
    qdarkstyle/style.qrc
    ${ICON}
)

target_link_libraries(glogg ${LIBS} glogg_core glogg_syntax pthread Qt5::Widgets Boost::program_options)

if (APPLE)
    install(DIRECTORY config DESTINATION "${BUNDLE_DIR}/Contents/Resources")

    function(install_bundle_plugin name subdir)
        get_target_property(src_path ${name} LOCATION)
        get_filename_component(basename ${src_path} NAME)
        set(dst_dir "${BUNDLE_DIR}/Contents/PlugIns/${subdir}")
        list(APPEND BUNDLE_PLUGINS "${dst_dir}/${basename}")
        set(BUNDLE_PLUGINS ${BUNDLE_PLUGINS} PARENT_SCOPE)
        install(FILES ${src_path} DESTINATION "${dst_dir}")
    endfunction()

    install_bundle_plugin(Qt5::QCocoaIntegrationPlugin platforms)
    install_bundle_plugin(Qt5::QMacStylePlugin styles)
    install_bundle_plugin(Qt5::QGifPlugin imageformats)
    install_bundle_plugin(Qt5::QICNSPlugin imageformats)
    install_bundle_plugin(Qt5::QJpegPlugin imageformats)
    install_bundle_plugin(Qt5::QICOPlugin imageformats)
    install_bundle_plugin(Qt5::QMacHeifPlugin imageformats)
    install_bundle_plugin(Qt5::QMacJp2Plugin imageformats)
    install_bundle_plugin(Qt5::QTgaPlugin imageformats)
    install_bundle_plugin(Qt5::QTiffPlugin imageformats)
    install_bundle_plugin(Qt5::QWbmpPlugin imageformats)
    install_bundle_plugin(Qt5::QWebpPlugin imageformats)
    install_bundle_plugin(Qt5::QSvgPlugin imageformats)

    set(MACOSX_BUNDLE_BUNDLE_NAME "Glogg")
    set(MACOSX_BUNDLE_BUNDLE_DISPLAY_NAME "Glogg log viewer")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.github.sergei-dyshel.glogg")
    set(MACOSX_BUNDLE_ICON_FILE glogg.icns)

    install(CODE "
        include(BundleUtilities)
        set(BU_CHMOD_BUNDLE_ITEMS TRUE)
        fixup_bundle(\"${BUNDLE_DIR}\"   \"${BUNDLE_PLUGINS}\"   \"\")
        ")

    set(CPACK_INSTALL_CMAKE_PROJECTS "") # skip install before cpack install
    set(CPACK_INSTALL_COMMANDS "rm -rf ${OUTPUT_DIR}/Applications")
    set(CPACK_TEMPORARY_DIRECTORY ${OUTPUT_DIR})
    set(CMAKE_INSTALL_PREFIX ${OUTPUT_DIR})
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_BUNDLE_NAME "Glogg")
    set(CPACK_BINARY_DRAGNDROP ON)
    set(CPACK_MONOLITHIC_INSTALL ON)
    set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
    set(CPACK_PACKAGE_VERSION ${VERSION})
    set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION})
    include(CPack)
else()
    include(GNUInstallDirs)
    install(TARGETS glogg RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} BUNDLE DESTINATION .)
    install(DIRECTORY config DESTINATION share/glogg)
    install(FILES colors.schema.json syntax.schema.json DESTINATION share/glogg/schemas)
    install(FILES images/hicolor/16x16/glogg.png DESTINATION share/icons/hicolor/16x16/apps)
    install(FILES images/hicolor/32x32/glogg.png DESTINATION share/icons/hicolor/32x32/apps)
    install(FILES images/hicolor/scalable/glogg.svg DESTINATION share/icons/hicolor/scalable/apps)
    install(FILES glogg.desktop DESTINATION share/applications)
endif()
