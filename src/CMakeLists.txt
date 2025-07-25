cmake_minimum_required(VERSION 3.16)
project(ubuntu-frame)
include(FindPkgConfig)
include(CMakeDependentOption)

set(CMAKE_CXX_STANDARD 20)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

option(ENABLE_TESTING "Specifies whether or not we should build the tests" ON)

cmake_dependent_option(ENABLE_COVERAGE "Enable code coverage" OFF ENABLE_TESTING OFF)
if(ENABLE_COVERAGE)
    include(CodeCoverage)
    append_coverage_compiler_flags()
    setup_target_for_coverage_lcov(
        NAME coverage_html
        BASE_DIRECTORY ${CMAKE_SOURCE_DIR}
        EXECUTABLE ctest --test-dir ${CMAKE_BINARY_DIR} --output-on-failure
        DEPENDENCIES ubuntu-frame-tests
        EXCLUDE "tests/*" "/usr/include/**"
        LCOV_ARGS --ignore-errors inconsistent --ignore-errors mismatch
    )

    setup_target_for_coverage_gcovr_xml(
        NAME coverage_xml
        EXECUTABLE ctest --test-dir ${CMAKE_BINARY_DIR} --output-on-failure
        DEPENDENCIES ubuntu-frame-tests
        EXCLUDE "tests/*"
    )

    add_custom_target(coverage
        DEPENDS coverage_html coverage_xml
    )
endif()

pkg_check_modules(MIRAL miral REQUIRED)
pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client)
pkg_check_modules(APPARMOR libapparmor REQUIRED)
pkg_check_modules(FREETYPE freetype2 REQUIRED)

add_library(frame-implementation
    frame_authorization.cpp frame_authorization.h
    frame_window_manager.cpp frame_window_manager.h
    egfullscreenclient.cpp egfullscreenclient.h
    background_client.cpp background_client.h
    snap_name_of.cpp snap_name_of.h
    layout_metadata.cpp layout_metadata.h
)

add_executable(frame
    frame_main.cpp
)

# Check for boost
find_package(Boost 1.48.0 COMPONENTS iostreams REQUIRED)

target_compile_definitions(frame PRIVATE MIR_LOG_COMPONENT="frame")
target_compile_definitions(frame-implementation PRIVATE MIR_LOG_COMPONENT="frame")

target_include_directories(frame-implementation
    PUBLIC 
        SYSTEM 
        ${MIRAL_INCLUDE_DIRS} 
    PRIVATE 
        ${FREETYPE_INCLUDE_DIRS}
)

target_include_directories(frame
    PRIVATE
    ${FREETYPE_INCLUDE_DIRS}
)

target_link_libraries(frame-implementation
    ${MIRAL_LDFLAGS}
    ${WAYLAND_CLIENT_LDFLAGS}
    ${APPARMOR_LDFLAGS}
    ${FREETYPE_LDFLAGS}
    ${Boost_LIBRARIES}
)

target_link_libraries(frame
    frame-implementation
)

install(PROGRAMS ${CMAKE_BINARY_DIR}/frame
    DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)

if (ENABLE_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
