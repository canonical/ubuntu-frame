cmake_minimum_required(VERSION 3.16)

include_directories(
    ${PROJECT_SOURCE_DIR}
)

find_package(PkgConfig)
pkg_check_modules(MIRAL miral REQUIRED)
pkg_check_modules(MIRSERVER mirserver REQUIRED)
pkg_check_modules(MIRTEST mirtest REQUIRED)
pkg_check_modules(MIRCOMMON mircommon REQUIRED)
pkg_check_modules(MIRCOMMON_INTERNAL REQUIRED mircommon-internal)
pkg_check_modules(MIRSERVER_INTERNAL REQUIRED mirserver-internal)
find_package(GTest REQUIRED)

add_executable(ubuntu-frame-tests
    test_frame_authorization.cpp
    test_frame_window_manager.cpp
)

target_include_directories(ubuntu-frame-tests PUBLIC SYSTEM
    ${MIRAL_INCLUDE_DIRS}
    ${MIRSERVER_INCLUDE_DIRS}
    ${MIRSERVER_INTERNAL_INCLUDE_DIRS}
    ${MIRCOMMON_INCLUDE_DIRS}
    ${MIRCOMMON_INTERNAL_INCLUDE_DIRS}
    ${MIRTEST_INCLUDE_DIRS}
    ${GTEST_INCLUDE_DIRS}
    ${GMOCK_INCLUDE_DIRS}
)

target_link_libraries(ubuntu-frame-tests
    frame-implementation
    ${MIRAL_LDFLAGS}
    ${MIRSERVER_LDFLAGS}
    ${MIRSERVER_INTERNAL_LDFLAGS_DIRS}
    ${MIRCOMMON_LDFLAGS_DIRS}
    ${MIRCOMMON_INTERNAL_LDFLAGS_DIRS}
    ${MIRTEST_LDFLAGS}
    GTest::GTest
    GTest::Main
    GTest::gmock
)

gtest_discover_tests(ubuntu-frame-tests)
