cmake_minimum_required(VERSION 3.16)

include_directories(
  ${PROJECT_SOURCE_DIR}
)

find_package(PkgConfig)
pkg_check_modules(MIRAL miral REQUIRED)
pkg_check_modules(MIRSERVER mirserver REQUIRED)
find_package(GTest REQUIRED)

add_executable(ubuntu-frame-tests
  test_frame_authorization.cpp)

target_include_directories(ubuntu-frame-tests PUBLIC SYSTEM
  ${MIRAL_INCLUDE_DIRS}
  ${MIRSERVER_INCLUDE_DIRS}
  ${GTEST_INCLUDE_DIRS}
  ${GMOCK_INCLUDE_DIRS})

target_link_libraries(ubuntu-frame-tests
  frame-implementation
  ${MIRAL_LDFLAGS}
  ${MIRSERVER_LDFLAGS}
  GTest::GTest
  GTest::Main
  GTest::gmock)

enable_testing()
