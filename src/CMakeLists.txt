cmake_minimum_required(VERSION 3.16)
project(ubuntu_core_kiosk)
include(FindPkgConfig)

set(CMAKE_CXX_STANDARD 20)
pkg_check_modules(MIRAL miral REQUIRED)

add_executable(ubuntu_core_kiosk
    ubuntu_core_kiosk_main.cpp
    kiosk_window_manager.cpp kiosk_window_manager.h
)

target_include_directories(ubuntu_core_kiosk PUBLIC SYSTEM ${MIRAL_INCLUDE_DIRS})
target_link_libraries(ubuntu_core_kiosk ${MIRAL_LDFLAGS})

install(PROGRAMS ${CMAKE_BINARY_DIR}/ubuntu_core_kiosk ${CMAKE_BINARY_DIR}/x11_kiosk_launch
    DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)
