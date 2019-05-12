find_package(Qt5Test REQUIRED)

enable_testing()

add_subdirectory(googletest)

set(LIBS gmock gtest gtest_main yaml-cpp pthread Qt5::Widgets Qt5::Test Boost::program_options)

include_directories(../src)

# Unit tests
add_executable(glogg_tests
    watchtowerTest.cpp
    linepositionarrayTest.cpp
    encodingspeculatorTest.cpp
    utests.cpp
)

# Link test executable against gtest & gtest_main
target_link_libraries(glogg_tests
    glogg_core ${LIBS})

add_executable(glogg_syntax_tests
    colorizer_test.cpp
    colors_test.cpp
    syntax_test.cpp
    utests.cpp
)

target_link_libraries(glogg_syntax_tests glogg_syntax ${LIBS})

# Integration tests
add_executable(glogg_itests
    logdataTest.cpp
    logfiltereddataTest.cpp
    itests.cpp
)

target_link_libraries(glogg_itests glogg_core glogg_syntax gtest_main ${LIBS})

# Performance tests
add_executable(glogg_ptests
    logdataPerfTest.cpp
    logfiltereddataPerfTest.cpp
)

target_link_libraries(glogg_ptests glogg_core glogg_syntax ${LIBS})

add_test(
    NAME glogg_tests
    COMMAND glogg_tests
)
