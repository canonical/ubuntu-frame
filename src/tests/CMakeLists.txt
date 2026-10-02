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

if(MIRAL_VERSION VERSION_GREATER_EQUAL "6.1.0")
    target_sources(ubuntu-frame-tests PRIVATE
        test_cec_manager.cpp
        test_cec_output.cpp
        test_cec_output_configuration.cpp
        test_libcec_adapter.cpp
        test_libcec_adapter_factory.cpp
        mock_icec_adapter.h)
endif()

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
