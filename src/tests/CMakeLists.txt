cmake_minimum_required(VERSION 3.16)

include_directories(
    ${PROJECT_SOURCE_DIR}
)

find_package(PkgConfig)
pkg_check_modules(MIRAL miral REQUIRED IMPORTED_TARGET)
pkg_check_modules(MIRSERVER mirserver REQUIRED IMPORTED_TARGET)
pkg_check_modules(MIRTEST mirtest REQUIRED IMPORTED_TARGET)
pkg_check_modules(MIRCOMMON mircommon REQUIRED IMPORTED_TARGET)
pkg_check_modules(MIRSERVER_INTERNAL REQUIRED mirserver-internal IMPORTED_TARGET)
find_package(GTest REQUIRED)

add_executable(ubuntu-frame-tests
    test_frame_authorization.cpp
    test_frame_window_manager.cpp
)

target_link_libraries(ubuntu-frame-tests
    frame-implementation
    PkgConfig::MIRAL
    PkgConfig::MIRSERVER
    PkgConfig::MIRSERVER_INTERNAL
    PkgConfig::MIRCOMMON
    PkgConfig::MIRTEST
    GTest::GTest
    GTest::Main
    GTest::gmock
)

gtest_discover_tests(ubuntu-frame-tests)
